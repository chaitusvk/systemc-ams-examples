#ifndef SERDES_TYPES_H
#define SERDES_TYPES_H

// ============================================================
//  serdes_types.h  –  Common types & parameters
//  Single-ended SerDes  |  SystemC-AMS (TDF MoC)
// ============================================================

#include <systemc-ams>

// ─── Link Parameters ────────────────────────────────────────
namespace serdes {

// Data-rate: 10 Gbps  →  UI = 100 ps
static constexpr double DATA_RATE_BPS   = 10e9;
static constexpr double UI_SEC          = 1.0 / DATA_RATE_BPS;   // 100 ps

// Serialization ratio (parallel word width)
static constexpr int    PAR_WIDTH       = 8;   // 8b parallel → 1b serial

// Clock periods
static constexpr double CLK_SERIAL_SEC  = UI_SEC;                 // 100 ps
static constexpr double CLK_PARALLEL_SEC= UI_SEC * PAR_WIDTH;     // 800 ps

// Channel parameters (lossy single-ended trace)
static constexpr double CHANNEL_LOSS_DB_PER_GHZ  = 2.0;   // dB/GHz per metre
static constexpr double CHANNEL_LENGTH_M         = 0.10;   // 10 cm PCB trace
static constexpr double CHANNEL_ZO_OHM           = 50.0;   // characteristic Z
static constexpr double CHANNEL_RC_POLE_HZ       = 8e9;    // single-pole BW
static constexpr double REFLECTIONS_COEFF        = 0.05;   // stub reflection

// TX pre-emphasis
static constexpr double TX_SWING_V       = 0.8;    // single-ended swing (V)
static constexpr double TX_PREEMP_COEFF  = 0.20;   // 1-tap FIR pre-emphasis

// RX CTLE pole/zero (continuous-time linear equaliser)
static constexpr double CTLE_ZERO_HZ     = 4e9;
static constexpr double CTLE_POLE_HZ     = 10e9;
static constexpr double CTLE_DC_GAIN     = 0.7;

// CDR / DFE
static constexpr double CDR_KP           = 0.01;   // proportional gain
static constexpr double CDR_KI           = 0.001;  // integral gain
static constexpr double DFE_TAP1_INIT    = 0.0;    // 1-tap DFE seed

// Noise
static constexpr double THERMAL_NOISE_VRMS = 0.002;  // 2 mV rms
static constexpr double JITTER_UI_RMS      = 0.005;  // 0.5% UI rms

} // namespace serdes
#endif // SERDES_TYPES_H
