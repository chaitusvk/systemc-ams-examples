#include <systemc>
#include <systemc-ams>
#include <cmath>
#include <random>
#include <fstream>
#include <vector>

using namespace sc_core;

SCA_TDF_MODULE(tx_eye_50g)
{
    sca_tdf::sca_out<double> tx;

    //------------------------------------
    // Parameters
    //------------------------------------
    double bitrate;
    double vlow;
    double vhigh;

    double rise_time_ps;
    double jitter_rms_ps;

    // Simple channel BW
    double channel_bw;

    //------------------------------------
    // Timing
    //------------------------------------
    double ui;
    double timestep;

    //------------------------------------
    // PRBS7
    //------------------------------------
    uint8_t lfsr;

    //------------------------------------
    // TX analog
    //------------------------------------
    double tx_out;
    double tx_target;

    //------------------------------------
    // Channel state
    //------------------------------------
    double ch_out;

    //------------------------------------
    // Timing control
    //------------------------------------
    double next_transition_time;

    //------------------------------------
    // RNG
    //------------------------------------
    std::default_random_engine gen;
    std::normal_distribution<double> jitter_dist;

    //------------------------------------
    // Eye diagram storage
    //------------------------------------
    std::ofstream eye_file;

    SCA_CTOR(tx_eye_50g)
    : bitrate(50e9),
      vlow(0.0),
      vhigh(0.7),
      rise_time_ps(5.0),
      jitter_rms_ps(0.3),
      channel_bw(18e9),
      lfsr(0x7F),
      tx_out(0.0),
      tx_target(0.0),
      ch_out(0.0),
      jitter_dist(0.0, 1.0)
    {
    }

    //------------------------------------
    // Setup
    //------------------------------------
    void set_attributes()
    {
        timestep = 100e-15;

        set_timestep(
            sc_core::sc_time(
                timestep,
                sc_core::SC_SEC));

        ui = 1.0 / bitrate;

        next_transition_time = 0.0;

        eye_file.open("eye_data.csv");

        eye_file << "time_ps,voltage\n";
    }

    //------------------------------------
    // PRBS7
    //------------------------------------
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

    //------------------------------------
    // Bit update
    //------------------------------------
    void update_target()
    {
        int bit = prbs7();

        tx_target =
            bit ? vhigh : vlow;

        // Gaussian jitter
        double jitter =
            jitter_dist(gen) *
            jitter_rms_ps * 1e-12;

        next_transition_time +=
            ui + jitter;
    }

    //------------------------------------
    // Main processing
    //------------------------------------
    void processing()
    {
        double t =
            sc_core::sc_time_stamp()
            .to_seconds();

        //--------------------------------
        // New bit?
        //--------------------------------
        if(t >= next_transition_time)
        {
            update_target();
        }

        //--------------------------------
        // TX edge shaping
        //--------------------------------
        double tau_tx =
            rise_time_ps *
            1e-12 / 2.2;

        tx_out +=
            (tx_target - tx_out) *
            (1.0 -
             exp(-timestep / tau_tx));

        //--------------------------------
        // Simple channel LPF
        //--------------------------------

        double tau_ch =
            1.0 /
            (2.0 * M_PI * channel_bw);

        ch_out +=
            (tx_out - ch_out) *
            (1.0 -
             exp(-timestep / tau_ch));

        //--------------------------------
        // Output
        //--------------------------------
        tx.write(ch_out);

        //--------------------------------
        // Eye data capture
        //--------------------------------

        double eye_phase =
            fmod(t, 2.0 * ui);

        eye_file
            << eye_phase * 1e12
            << ","
            << ch_out
            << "\n";
    }

    //------------------------------------
    // Cleanup
    //------------------------------------
    void end_of_simulation()
    {
        eye_file.close();
    }
};

int sc_main(int argc, char* argv[])
{
    sc_core::sc_set_time_resolution(1, SC_FS);

    sca_tdf::sca_signal<double> tx_sig;

    tx_eye_50g tx("tx");

    tx.tx(tx_sig);

    //--------------------------------
    // Trace waveform
    //--------------------------------
    sca_util::sca_trace_file* tf =
        sca_util::sca_create_vcd_trace_file(
            "tx_eye");

    sca_util::sca_trace(tf, tx_sig, "tx");

    //--------------------------------
    // Run
    //--------------------------------
    sc_core::sc_start(
        10,
        sc_core::SC_NS);

    sca_util::sca_close_vcd_trace_file(tf);

    return 0;
}