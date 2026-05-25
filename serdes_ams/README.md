# Single-Ended SerDes – SystemC-AMS Simulation

## Architecture

```
┌──────────────┐  parallel[8]  ┌────────────────────────────────┐
│   PRBS-31    │──────────────▶│         TRANSMITTER            │
│  prbs_gen    │               │  • Serializer (8:1)            │
│              │               │  • 1-tap TX pre-emphasis FIR   │
└──────────────┘               │  • ±0.8 V single-ended swing   │
       │ ref                   └────────────────┬───────────────┘
       │                                se_voltage│ (serial, UI=100ps)
       ▼                                         ▼
┌──────────────┐               ┌────────────────────────────────┐
│  BER Monitor │               │     ANALOG CHANNEL             │
│  (SC_MODULE) │               │  • Insertion loss (freq-dep.)  │
│  • XOR count │               │  • Single-pole RC LPF          │
│  • Lock det. │               │  • Stub reflection (1-tap FIR) │
│  • BER print │               │  • AWGN thermal noise          │
└──────────────┘               └────────────────┬───────────────┘
       ▲                                         │ degraded se_voltage
       │ par[8]                                  ▼
       │               ┌────────────────────────────────────────┐
       └───────────────│          RECEIVER                      │
                       │  • CTLE (1st-order peaking, bilinear z)│
                       │  • Bang-bang CDR (2nd-order loop)      │
                       │  • 1-tap DFE (LMS adaptation)          │
                       │  • Slicer (0 V threshold)              │
                       │  • Deserializer (1:8)                  │
                       └────────────────────────────────────────┘
```

## Link Parameters

| Parameter          | Value          |
|--------------------|----------------|
| Data rate          | 10 Gbps        |
| UI                 | 100 ps         |
| Parallelism        | 8-bit words    |
| TX swing           | ±0.8 V SE      |
| TX pre-emphasis    | 1-tap, 20%     |
| Channel BW (-3dB)  | 8 GHz (RC pole)|
| Channel length     | 10 cm PCB trace|
| Insertion loss     | 2 dB/GHz/m     |
| Reflection coeff.  | 5%             |
| Thermal noise      | 2 mV rms       |
| CTLE zero / pole   | 4 GHz / 10 GHz |
| CDR Kp / Ki        | 0.01 / 0.001   |
| DFE taps           | 1 (LMS adapt.) |
| PRBS pattern       | PRBS-31        |

## File Structure

```
serdes_ams/
├── src/
│   ├── serdes_types.h     # Shared parameters & constants
│   ├── prbs_gen.h         # PRBS-31 stimulus generator
│   ├── serdes_tx.h        # TX serializer + pre-emphasis
│   ├── serdes_channel.h   # Analog channel (RC + noise + reflection)
│   ├── serdes_rx.h        # RX CTLE + CDR + DFE + deserializer
│   └── ber_monitor.h      # BER counting (SC_MODULE)
├── tb/
│   └── serdes_tb.cpp      # Top-level testbench
├── sim/                   # Build output directory
└── Makefile
```

## Build & Run

```bash
# Set toolchain paths
export SYSTEMC_HOME=/path/to/systemc-2.3.3
export SYSTEMCAMS_HOME=/path/to/systemc-ams-2.1

# Build
make

# Simulate (10 000 words = 80 000 UIs)
make run

# View waveforms
make waves        # opens serdes_tb.vcd in GTKWave
```

## Output Files

| File                | Content                              |
|---------------------|--------------------------------------|
| `sim/serdes_tb.vcd` | Digital waveforms (GTKWave)          |
| `sim/serdes_trace.dat` | Analog TDF traces (tabular)       |

### Analog traces available

- `tx_se_V`       – TX output voltage (pre-emphasis applied)
- `rx_se_V`       – Channel output voltage (degraded)
- `ctle_out_V`    – After CTLE equalization
- `dfe_out_V`     – After DFE cancellation
- `cdr_phase_err` – CDR bang-bang phase error
- `tx_par` / `rx_par` – Parallel TX/RX words

## Extending the Model

| Goal                         | Where to modify                     |
|------------------------------|-------------------------------------|
| Change data rate             | `serdes_types.h` → `DATA_RATE_BPS`  |
| Add more DFE taps            | `serdes_rx.h` → extend `m_dfe_tapN` |
| Add ISI from longer channel  | `serdes_channel.h` → extend FIR     |
| Add jitter injection         | `serdes_tx.h` → perturb sample time |
| Differential signaling       | Duplicate channel, invert one rail  |
| Eye diagram                  | Plot `dfe_out_V` vs mod(t, UI)      |
