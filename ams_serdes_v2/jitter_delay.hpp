#include <systemc-ams>
#include <systemc>
#include <deque>
#include <random>
#include <cmath>

using namespace sc_core;

SCA_TDF_MODULE(thermo_delay_jitter)
{
    // Ports
    sca_tdf::sca_in<double> in_sig;
    sca_tdf::sca_out<double> out_sig;

    // Thermometer digital control
    sc_core::sc_vector<
        sca_tdf::sca_de::sca_in<bool>
    > thermo;

    // Parameters
    double delay_step;

    // RMS jitter
    double jitter_rms;

    // Voltage noise RMS
    double noise_rms;

    // Internal
    std::deque<double> buffer;

    double ts;

    // Random generators
    std::default_random_engine gen;

    std::normal_distribution<double> jitter_dist;
    std::normal_distribution<double> noise_dist;

    thermo_delay_jitter(
        sc_core::sc_module_name nm,
        int nbits = 8)
    :
      in_sig("in_sig"),
      out_sig("out_sig"),
      thermo("thermo", nbits),
      jitter_dist(0.0, 1.0),
      noise_dist(0.0, 1.0)
    {
        delay_step = 1e-12;   // 1 ps/bit

        jitter_rms = 200e-15; // 200 fs RMS
        noise_rms  = 0.01;    // 10 mV RMS

        set_timestep(10.0, SC_FS);
    }

    void initialize()
    {
        ts = get_timestep().to_seconds();

        for(int i = 0; i < 100000; i++)
            buffer.push_back(0.0);
    }

    void processing()
    {
        // Count thermometer bits
        unsigned ones = 0;

        for(unsigned i = 0; i < thermo.size(); i++)
        {
            if(thermo[i].read())
                ones++;
        }

        // Base delay
        double delay =
            ones * delay_step;

        // Add Gaussian jitter
        double jitter =
            jitter_dist(gen) * jitter_rms;

        double total_delay =
            delay + jitter;

        // Prevent negative delay
        if(total_delay < 0.0)
            total_delay = 0.0;

        // Convert to samples
        unsigned delay_samples =
            (unsigned)std::round(total_delay / ts);

        // Store input
        buffer.push_back(in_sig.read());

        // Safety
        if(delay_samples >= buffer.size())
            delay_samples =
                buffer.size() - 1;

        // Delayed sample
        double y =
            buffer[
                buffer.size()
                - 1
                - delay_samples
            ];

        // Add voltage noise
        y += noise_dist(gen) * noise_rms;

        out_sig.write(y);

        // Prevent unlimited growth
        if(buffer.size() > 150000)
            buffer.pop_front();
    }
};