#include <systemc>
#include <systemc-ams>
#include <deque>

SCA_TDF_MODULE(tx_isi_50g)
{
    sca_tdf::sca_out<double> tx;

    //---------------------------------
    // Parameters
    //---------------------------------
    double bitrate;
    double vhigh;
    double vlow;

    //---------------------------------
    // UI
    //---------------------------------
    double ui;
    double timestep;

    //---------------------------------
    // PRBS7
    //---------------------------------
    uint8_t lfsr;

    //---------------------------------
    // FIR channel taps
    //---------------------------------
    double a0;
    double a1;
    double a2;

    //---------------------------------
    // Symbol history
    //---------------------------------
    std::deque<double> symbols;

    //---------------------------------
    // Timing
    //---------------------------------
    double next_ui;

    SCA_CTOR(tx_isi_50g)
    : bitrate(50e9),
      vhigh(0.7),
      vlow(0.0),
      lfsr(0x7F),
      a0(1.0),
      a1(0.35),
      a2(0.15)
    {
    }

    //---------------------------------
    // Setup
    //---------------------------------
    void set_attributes()
    {
        timestep = 1e-12;

        set_timestep(
            sc_core::sc_time(
                timestep,
                sc_core::SC_SEC));

        ui = 1.0 / bitrate;

        next_ui = 0.0;

        // Initialize history
        symbols.push_front(vlow);
        symbols.push_front(vlow);
        symbols.push_front(vlow);
    }

    //---------------------------------
    // PRBS7
    //---------------------------------
    int prbs7()
    {
        int newbit =
            ((lfsr >> 6) ^
             (lfsr >> 5)) & 1;

        lfsr =
            ((lfsr << 1) |
             newbit) & 0x7F;

        return lfsr & 1;
    }

    //---------------------------------
    // Main processing
    //---------------------------------
    void processing()
    {
        double t =
            sc_core::sc_time_stamp()
            .to_seconds();

        //--------------------------------
        // New symbol each UI
        //--------------------------------
        if(t >= next_ui)
        {
            int bit = prbs7();

            double sym =
                bit ? vhigh : vlow;

            symbols.push_front(sym);

            while(symbols.size() > 3)
                symbols.pop_back();

            next_ui += ui;
        }

        //--------------------------------
        // FIR ISI channel
        //--------------------------------

        double y =
              a0 * symbols[0]
            + a1 * symbols[1]
            + a2 * symbols[2];

        tx.write(y);
    }
};

int sc_main(int argc, char* argv[])
{
    sca_tdf::sca_signal<double> tx_sig;

    tx_isi_50g tx("tx");

    tx.tx(tx_sig);

    //--------------------------------
    // Trace
    //--------------------------------
    sca_util::sca_trace_file* tf =
        sca_util::sca_create_vcd_trace_file(
            "isi_wave");

    sca_util::sca_trace(tf, tx_sig, "tx");

    //--------------------------------
    // Run
    //--------------------------------
    sc_core::sc_start(
        5,
        sc_core::SC_NS);

    sca_util::sca_close_vcd_trace_file(tf);

    return 0;
}