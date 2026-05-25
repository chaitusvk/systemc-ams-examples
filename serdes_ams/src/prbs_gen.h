#ifndef PRBS_GEN_H
#define PRBS_GEN_H

// ============================================================
//  prbs_gen.h  –  PRBS-31 parallel-word stimulus generator
//
//  Generates pseudo-random bit sequences (PRBS-31)
//  mapped onto PAR_WIDTH-bit parallel words, suitable as
//  TX stimulus.  Also used as reference for BER counting.
// ============================================================

#include "serdes_types.h"

SCA_TDF_MODULE(prbs_gen)
{
    sca_tdf::sca_out<sc_dt::sc_uint<serdes::PAR_WIDTH>> data_out;
    sca_tdf::sca_out<sc_dt::sc_uint<serdes::PAR_WIDTH>> ref_out;  // mirror for BER

    SCA_CTOR(prbs_gen)
        : data_out("data_out"),
          ref_out("ref_out"),
          m_lfsr(0x7FFF'FFFF)   // all-ones seed (PRBS-31)
    {}

    void set_attributes() override
    {
        set_timestep(serdes::CLK_PARALLEL_SEC, sc_core::SC_SEC);
    }

    void processing() override
    {
        sc_dt::sc_uint<serdes::PAR_WIDTH> word = 0;

        for (int i = serdes::PAR_WIDTH - 1; i >= 0; --i)
        {
            // PRBS-31: feedback = bit[30] XOR bit[27]
            uint32_t fb = ((m_lfsr >> 30) ^ (m_lfsr >> 27)) & 1u;
            m_lfsr      = ((m_lfsr << 1) | fb) & 0x7FFF'FFFFu;
            word.bit_select(i, i) = fb;
        }

        data_out.write(word);
        ref_out.write(word);
    }

private:
    uint32_t m_lfsr;
};

#endif // PRBS_GEN_H
