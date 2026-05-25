#ifndef BER_MONITOR_H
#define BER_MONITOR_H

// ============================================================
//  ber_monitor.h  –  Bit-Error-Rate monitor (SC_MODULE)
//
//  Compares PAR_WIDTH-bit words from RX against the reference
//  PRBS stream (with programmable latency compensation).
//  Prints BER summary at end of simulation.
// ============================================================

#include "serdes_types.h"
#include <deque>
#include <cstdint>
#include <iostream>
#include <iomanip>

SC_MODULE(ber_monitor)
{
    // ── Ports ────────────────────────────────────────────────
    sc_core::sc_in<bool>   clk;
    sc_core::sc_in<sc_dt::sc_uint<serdes::PAR_WIDTH>> rx_data;
    sc_core::sc_in<sc_dt::sc_uint<serdes::PAR_WIDTH>> ref_data;

    // ── Constructor ──────────────────────────────────────────
    SC_CTOR(ber_monitor)
        : clk("clk"),
          rx_data("rx_data"),
          ref_data("ref_data"),
          m_total_bits(0),
          m_error_bits(0),
          m_latency_words(4),   // pipeline latency in words
          m_lock(false),
          m_lock_cnt(0)
    {
        SC_METHOD(monitor_method);
        sensitive << clk.pos();
        dont_initialize();
    }

    void end_of_simulation() override
    {
        print_summary();
    }

    // Accessors
    uint64_t total_bits() const { return m_total_bits; }
    uint64_t error_bits() const { return m_error_bits; }
    double   ber()        const {
        return m_total_bits ? static_cast<double>(m_error_bits) / m_total_bits
                            : 0.0;
    }

private:
    void monitor_method()
    {
        // Buffer reference to model pipeline latency
        m_ref_fifo.push_back(ref_data.read());
        if (static_cast<int>(m_ref_fifo.size()) <= m_latency_words) return;

        auto ref_word = m_ref_fifo.front();
        m_ref_fifo.pop_front();
        auto rx_word  = rx_data.read();

        // XOR to find bit errors
        auto err_bits = ref_word ^ rx_word;
        int  errs     = 0;
        for (int i = 0; i < serdes::PAR_WIDTH; ++i)
            errs += err_bits.bit(i);

        // Lock detection: require <2 errors per word for 16 words
        if (!m_lock) {
            if (errs < 2) ++m_lock_cnt;
            else m_lock_cnt = 0;
            if (m_lock_cnt >= 16) {
                m_lock = true;
                std::cout << "[BER] CDR locked at t="
                          << sc_core::sc_time_stamp() << "\n";
            }
            return;  // don't count pre-lock errors
        }

        m_total_bits += serdes::PAR_WIDTH;
        m_error_bits += errs;
    }

    void print_summary()
    {
        std::cout << "\n===== BER Summary =====================================\n";
        std::cout << "  Total bits  : " << m_total_bits << "\n";
        std::cout << "  Error bits  : " << m_error_bits << "\n";
        std::cout << "  BER         : ";
        if (m_total_bits > 0)
            std::cout << std::scientific << std::setprecision(3) << ber() << "\n";
        else
            std::cout << "N/A (no lock)\n";
        std::cout << "=======================================================\n";
    }

    // State
    uint64_t              m_total_bits;
    uint64_t              m_error_bits;
    int                   m_latency_words;
    bool                  m_lock;
    int                   m_lock_cnt;
    std::deque<sc_dt::sc_uint<serdes::PAR_WIDTH>> m_ref_fifo;
};

#endif // BER_MONITOR_H
