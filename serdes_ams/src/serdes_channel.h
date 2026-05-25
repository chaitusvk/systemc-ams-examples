#ifndef SERDES_CHANNEL_H
#define SERDES_CHANNEL_H

// ============================================================
//  serdes_channel.h  –  Single-ended Analog Channel Model
//
//  Models:
//    1. Single-pole RC low-pass (bandwidth = CHANNEL_RC_POLE_HZ)
//    2. Frequency-dependent insertion loss (approximated as
//       additional first-order attenuation at Nyquist)
//    3. Single reflection stub (FIR delay-line model)
//    4. Additive white Gaussian noise (thermal + coupling)
//
//  MoC: TDF – timestep = UI (serial bit period)
// ============================================================

#include "serdes_types.h"
#include <random>
#include <deque>
#include <cmath>

SCA_TDF_MODULE(serdes_channel)
{
    // ── Ports ────────────────────────────────────────────────
    sca_tdf::sca_in<double>  vin;   // TX single-ended voltage
    sca_tdf::sca_out<double> vout;  // RX single-ended voltage

    // ── Constructor ──────────────────────────────────────────
    SCA_CTOR(serdes_channel)
        : vin("vin"),
          vout("vout"),
          m_lp_state(0.0),
          m_rng(42),
          m_noise_dist(0.0, serdes::THERMAL_NOISE_VRMS)
    {
        // Pre-compute RC coefficient  α = exp(-2π·f_pole·Ts)
        m_alpha = std::exp(-2.0 * M_PI * serdes::CHANNEL_RC_POLE_HZ
                           * serdes::CLK_SERIAL_SEC);

        // Insertion loss at Nyquist: dB = loss_per_GHz * (DR/2 in GHz) * length
        double f_nyq_ghz = (serdes::DATA_RATE_BPS / 2.0) * 1e-9;
        double loss_db   = serdes::CHANNEL_LOSS_DB_PER_GHZ
                           * f_nyq_ghz
                           * serdes::CHANNEL_LENGTH_M;
        m_il_linear = std::pow(10.0, -loss_db / 20.0);  // amplitude factor

        // Reflection delay: 1 UI (stub at ~half channel)
        m_refl_delay.assign(1, 0.0);   // 1-tap delay line
    }

    // ── Elaboration ──────────────────────────────────────────
    void set_attributes() override
    {
        set_timestep(serdes::CLK_SERIAL_SEC, sc_core::SC_SEC);
    }

    void initialize() override
    {
        m_lp_state = 0.0;
        std::fill(m_refl_delay.begin(), m_refl_delay.end(), 0.0);
    }

    // ── Processing ───────────────────────────────────────────
    void processing() override
    {
        double v_in = vin.read();

        // ── 1. Frequency-dependent insertion loss ─────────
        //    Simple approximation: scale by DC→Nyquist factor
        //    (a proper model would use a convolution kernel)
        double v_att = v_in * m_il_linear;

        // ── 2. Single-pole RC low-pass filter ─────────────
        //    y[n] = α·y[n-1] + (1-α)·x[n]
        m_lp_state = m_alpha * m_lp_state + (1.0 - m_alpha) * v_att;

        // ── 3. Stub reflection (1-tap FIR echo) ───────────
        double v_refl = m_refl_delay.back();
        m_refl_delay.pop_back();
        m_refl_delay.push_front(m_lp_state);

        double v_with_refl = m_lp_state
                             + serdes::REFLECTIONS_COEFF * v_refl;

        // ── 4. Additive Gaussian thermal noise ────────────
        double noise = m_noise_dist(m_rng);

        vout.write(v_with_refl + noise);
    }

private:
    double m_alpha;       // RC pole coefficient
    double m_il_linear;   // insertion-loss amplitude factor
    double m_lp_state;    // RC integrator state

    std::deque<double>           m_refl_delay;   // stub delay line
    std::mt19937                 m_rng;
    std::normal_distribution<double> m_noise_dist;
};

#endif // SERDES_CHANNEL_H
