#include <systemc-ams>
#include <deque>

using namespace sc_core;

SCA_TDF_MODULE(delay_tdf)
{
    // Ports
    sca_tdf::sca_in<double>  in;
    sca_tdf::sca_out<double> out;

    // Parameters
    double delay_time;

    // Internal
    unsigned delay_samples;
    std::deque<double> buffer;

    SCA_CTOR(delay_tdf)
    {
        delay_time = 5e-12; // 5 ps

        // Timestep
        set_timestep(100.0, SC_FS);
    }

    void initialize()
    {
        // Convert delay into sample count
        double ts =
            get_timestep().to_seconds();

        delay_samples =
            (unsigned)(delay_time / ts);

        // Initialize buffer
        for(unsigned i = 0; i < delay_samples; i++)
            buffer.push_back(0.0);
    }

    void processing()
    {
        // Push new sample
        buffer.push_back(in.read());

        // Output oldest sample
        out.write(buffer.front());

        // Remove oldest
        buffer.pop_front();
    }
};