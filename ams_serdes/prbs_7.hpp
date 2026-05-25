#include <systemc>

SC_MODULE(prbs7_gen_32)
{
    //--------------------------------
    // Ports
    //--------------------------------
    sc_core::sc_in<bool> clk;

    sc_core::sc_out<
        sc_dt::sc_uint<32>
    > data_out;

    //--------------------------------
    // Internal PRBS state
    //--------------------------------
    sc_dt::sc_uint<7> lfsr;

    SC_CTOR(prbs7_gen_32)
    : lfsr(0x7F)
    {
        SC_METHOD(process);

        sensitive << clk.pos();
    }

    //--------------------------------
    // Generate next PRBS bit
    //--------------------------------
    bool next_prbs_bit()
    {
        bool newbit =
            lfsr[6] ^ lfsr[5];

        lfsr =
            (lfsr << 1) | newbit;

        return lfsr[0];
    }

    //--------------------------------
    // Main process
    //--------------------------------
    void process()
    {
        sc_dt::sc_uint<32> word = 0;

        //--------------------------------
        // Generate 32 bits
        //--------------------------------
        for(int i = 0; i < 32; i++)
        {
            word[i] =
                next_prbs_bit();
        }

        data_out.write(word);
    }
};