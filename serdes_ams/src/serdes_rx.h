#ifndef SERDES_RX_H
#define SERDES_RX_H

// ============================================================
//  serdes_rx.h  –  Receiver: CTLE + CDR + DFE + Deserializer
//
//  Pipeline (all in TDF):
//    se_in → CTLE (s-domain approximation via bilinear z)
//          → CDR  (Bang-bang PD + 2nd-order loop filter)
//          → DFE  (1-tap decision-feedback equaliser)
//          → slicer (threshold = 0 V)
//          → shift-register deserialiser
//          → par_out
// ============================================================

#include "serdes_types.h"
#include <deque>
#include <cmath>

SCA_TDF_MODULE(serdes_rx)
{
    // ── Ports ────────────────────────────────────────────────
    sca_tdf::sca_in<double>  se_in;    // analog single-ended input
    sca_tdf::sca_out<sc_dt::sc_uint<serdes::PAR_WIDTH>> par_out;

    // Observable analog nodes (for waveform probing)
    sca_tdf::sca_out<double> ctle_out_probe;
    sca_tdf::sca_out<double> dfe_out_probe;
    sca_tdf::sca_out<double> phase_err_probe;

    // ── Constructor ──────────────────────────────────────────
    SCA_CTOR(serdes_rx)
        : se_in("se_in"),
          par_out("par_out"),
          ctle_out_probe("ctle_out_probe"),
          dfe_out_probe("dfe_out_probe"),
          phase_err_probe("phase_err_probe"),
          m_ctle_x1(0.0), m_ctle_x2(0.0),
          m_ctle_y1(0.0), m_ctle_y2(0.0),
          m_cdr_phase(0.0), m_cdr_freq(0.0),
          m_dfe_tap1(serdes::DFE_TAP1_INIT),
          m_prev_decision(0.0),
          m_shift_reg(0),
          m_bit_cnt(0)
    {
        // ── CTLE bilinear transform coefficients ──────────
        // H(s) = G0 · (s/wz + 1) / (s/wp + 1)
        // Bilinear: s → 2/Ts · (z-1)/(z+1),  Ts = UI
        double Ts  = serdes::CLK_SERIAL_SEC;
        double wz  = 2.0 * M_PI * serdes::CTLE_ZERO_HZ;
        double wp  = 2.0 * M_PI * serdes::CTLE_POLE_HZ;
        double G0  = serdes::CTLE_DC_GAIN;

        // Pre-warp
        double wz_d = (2.0 / Ts) * std::tan(wz * Ts / 2.0);
        double wp_d = (2.0 / Ts) * std::tan(wp * Ts / 2.0);

        double k  = 2.0 / Ts;
        double b0 = G0 * (k + wz_d);
        double b1 = G0 * (-k + wz_d);
        double a0 = k + wp_d;
        double a1 = -k + wp_d;

        // Normalise
        m_cb0 =  b0 / a0;
        m_cb1 =  b1 / a0;
        m_ca1 = -a1 / a0;   // note sign: y[n] = b0·x[n] + b1·x[n-1] + a1_neg·y[n-1]
    }

    // ── Elaboration ──────────────────────────────────────────
    void set_attributes() override
    {
        // Consume PAR_WIDTH serial samples, produce one parallel word
        se_in.set_rate(serdes::PAR_WIDTH);
        par_out.set_rate(1);
        ctle_out_probe.set_rate(serdes::PAR_WIDTH);
        dfe_out_probe.set_rate(serdes::PAR_WIDTH);
        phase_err_probe.set_rate(serdes::PAR_WIDTH);
        set_timestep(serdes::CLK_PARALLEL_SEC, sc_core::SC_SEC);
    }

    void initialize() override
    {
        m_ctle_x1 = m_ctle_x2 = m_ctle_y1 = m_ctle_y2 = 0.0;
        m_cdr_phase = m_cdr_freq = 0.0;
        m_dfe_tap1  = serdes::DFE_TAP1_INIT;
        m_prev_decision = 0.0;
        m_shift_reg = 0;
        m_bit_cnt   = 0;
    }

    // ── Processing ───────────────────────────────────────────
    void processing() override
    {
        sc_dt::sc_uint<serdes::PAR_WIDTH> word = 0;

        for (int i = 0; i < serdes::PAR_WIDTH; ++i)
        {
            // ── CTLE (1st-order peaking) ──────────────────
            double x = se_in.read(i);
            double y_ctle = m_cb0 * x + m_cb1 * m_ctle_x1 + m_ca1 * m_ctle_y1;
            m_ctle_x1 = x;
            m_ctle_y1 = y_ctle;

            ctle_out_probe.write(y_ctle, i);

            // ── DFE (1-tap) ───────────────────────────────
            double y_dfe = y_ctle - m_dfe_tap1 * m_prev_decision;
            dfe_out_probe.write(y_dfe, i);

            // ── Slicer ────────────────────────────────────
            double decision = (y_dfe >= 0.0) ? +1.0 : -1.0;

            // ── Bang-Bang CDR phase detector ─────────────
            //    Uses early/late edge detector:
            //    phase_err = sign(edge) * sign(data_mid)
            double mid_sample = y_dfe;
            double edge_sample = (m_prev_decision != decision)
                                 ? 0.5 * (y_dfe + m_ctle_y1)
                                 : 0.0;
            double phase_err = (m_prev_decision != decision)
                                ? ((edge_sample > 0) ? +1.0 : -1.0)
                                  * ((mid_sample > 0) ? +1.0 : -1.0)
                                : 0.0;
            phase_err_probe.write(phase_err, i);

            // ── 2nd-order loop filter ─────────────────────
            m_cdr_freq  += serdes::CDR_KI * phase_err;
            m_cdr_phase += serdes::CDR_KP * phase_err + m_cdr_freq;
            // (Phase correction would offset sampling in a full model;
            //  here we expose it for analysis)

            // ── DFE tap adaptation (LMS, mu=0.01) ─────────
            double error = (decision > 0 ? +serdes::TX_SWING_V
                                         : -serdes::TX_SWING_V)
                           - y_dfe;
            m_dfe_tap1 += 0.01 * error * m_prev_decision;

            // ── Shift into deserialiser register ──────────
            //    MSB first
            word = (word << 1) | sc_dt::sc_uint<1>(decision > 0 ? 1 : 0);

            m_prev_decision = decision;
        }

        par_out.write(word);
    }

private:
    // CTLE IIR coefficients (bilinear)
    double m_cb0, m_cb1, m_ca1;
    // CTLE state
    double m_ctle_x1, m_ctle_x2, m_ctle_y1, m_ctle_y2;

    // CDR state
    double m_cdr_phase, m_cdr_freq;

    // DFE
    double m_dfe_tap1;
    double m_prev_decision;

    // Deserialiser
    sc_dt::sc_uint<serdes::PAR_WIDTH> m_shift_reg;
    int m_bit_cnt;
};

#endif // SERDES_RX_H
