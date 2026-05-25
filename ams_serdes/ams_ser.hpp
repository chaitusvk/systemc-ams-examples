#include <systemc>
#include <systemc-ams>
#include <cmath>

SCA_TDF_MODULE(serializer_32to1_ams)
{
    //--------------------------------
    // Inputs
    //--------------------------------

    sca_tdf::sca_de::sca_in<
        sc_dt::sc_uint<32>
    > din;

    //--------------------------------
    // Analog output
    //--------------------------------

    sca_tdf::sca_out<double> tx;

    //--------------------------------
    // Parameters
    //--------------------------------

    double bitrate;

    double vhigh;
    double vlow;

    double rise_ps;

    //--------------------------------
    // Timing
    //--------------------------------

    double ui;
    double timestep;

    //--------------------------------
    // Serializer state
    //--------------------------------

    sc_dt::sc_uint<32> shift_reg;

    int bit_idx;

    //--------------------------------
    // Analog state
    //--------------------------------

    double target_v;
    double vout;

    //--------------------------------
    // UI counter
    //--------------------------------

    int samples_per_ui;
    int sample_count;

    SCA_CTOR(serializer_32to1_ams)
    : bitrate(50e9),
      vhigh(0.7),
      vlow(0.0),
      rise_ps(5.0),
      bit_idx(31),
      target_v(0.0),
      vout(0.0)
    {
    }

    //--------------------------------
    // Setup
    //--------------------------------

    void set_attributes()
    {
        //--------------------------------
        // 1 ps timestep
        //--------------------------------

        timestep = 1e-12;

        set_timestep(
            sc_core::sc_time(
                timestep,
                sc_core::SC_SEC));

        //--------------------------------
        // 20 ps UI
        //--------------------------------

        ui = 1.0 / bitrate;

        //--------------------------------
        // Samples/UI
        //--------------------------------

        samples_per_ui =
            (int)(ui / timestep);

        sample_count = 0;
    }

    //--------------------------------
    // Main processing
    //--------------------------------

    void processing()
    {
        //--------------------------------
        // Load new word?
        //--------------------------------

        if(sample_count == 0)
        {
            shift_reg = din.read();

            bit_idx = 31;
        }

        //--------------------------------
        // Bit boundary?
        //--------------------------------

        if((sample_count %
            samples_per_ui) == 0)
        {
            bool bit =
                shift_reg[bit_idx];

            //--------------------------------
            // Convert bit to voltage
            //--------------------------------

            target_v =
                bit ? vhigh : vlow;

            //--------------------------------
            // Next bit
            //--------------------------------

            if(bit_idx == 0)
                bit_idx = 31;
            else
                bit_idx--;
        }

        //--------------------------------
        // Slew-limited edge
        //--------------------------------

        double tau =
            rise_ps *
            1e-12 / 2.2;

        vout +=
            (target_v - vout) *
            (1.0 -
             exp(-timestep / tau));

        //--------------------------------
        // Output analog waveform
        //--------------------------------

        tx.write(vout);

        //--------------------------------
        // Advance time
        //--------------------------------

        sample_count++;

        //--------------------------------
        // Wrap every 32 bits
        //--------------------------------

        if(sample_count >=
           samples_per_ui * 32)
        {
            sample_count = 0;
        }
    }
};