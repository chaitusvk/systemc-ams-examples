#ifndef SERDES_TX_H
#define SERDES_TX_H

// ============================================================
//  serdes_tx.h  –  Transmitter: Serializer + Pre-emphasis
//
//  Architecture:
//    parallel[PAR_WIDTH] → shift-register serialiser
//    → 1-tap FIR pre-emphasis driver
//    → single-ended voltage output (sca_tdf port)
//
//  MoC: SystemC-AMS Timed Data Flow (TDF)
// ============================================================

#include "serdes_types.h"
#include <deque>

SCA_TDF_MODULE(serdes_tx)
{
    // ── Ports ────────────────────────────────────────────────
    // Parallel data input (one word per parallel clock cycle)
    sca_tdf::sca_in<sc_dt::sc_uint<serdes::PAR_WIDTH>> par_in;

    // Single-ended voltage output  (sampled at serial rate)
    sca_tdf::sca_out<double> se_out;

    // ── Constructor ──────────────────────────────────────────
    SCA_CTOR(serdes_tx)
        : par_in("par_in"),
          se_out("se_out"),
          m_shift_reg(0),
          m_bit_cnt(0),
          m_prev_bit(0.0)
    {}

    // ── Elaboration ──────────────────────────────────────────
    void set_attributes() override
    {
        // par_in consumed at 1 sample / PAR_WIDTH serial steps
        par_in.set_rate(1);
        se_out.set_rate(serdes::PAR_WIDTH);  // produce PAR_WIDTH bits per call
        set_timestep(serdes::CLK_PARALLEL_SEC, sc_core::SC_SEC);
    }

    void initialize() override
    {
        m_shift_reg = 0;
        m_bit_cnt   = 0;
        m_prev_bit  = 0.0;
    }

    // ── Processing ───────────────────────────────────────────
    void processing() override
    {
        // Latch new parallel word
        sc_dt::sc_uint<serdes::PAR_WIDTH> word = par_in.read();

        // Serialise MSB first, apply 1-tap pre-emphasis
        for (int i = serdes::PAR_WIDTH - 1; i >= 0; --i)
        {
            double cur_bit = word.bit(i) ? +serdes::TX_SWING_V
                                         : -serdes::TX_SWING_V;

            // Pre-emphasis: boost transitions
            double emph = (cur_bit != m_prev_bit)
                          ? serdes::TX_PREEMP_COEFF * serdes::TX_SWING_V
                          : 0.0;

            // Sign of boost follows current bit polarity
            double v_out = cur_bit + ((cur_bit > 0) ? +emph : -emph);

            se_out.write(v_out, serdes::PAR_WIDTH - 1 - i);
            m_prev_bit = cur_bit;
        }
    }

private:
    sc_dt::sc_uint<serdes::PAR_WIDTH> m_shift_reg;
    int    m_bit_cnt;
    double m_prev_bit;
};

#endif // SERDES_TX_H
