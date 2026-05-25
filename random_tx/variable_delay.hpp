#include <systemc-ams>
#include <deque>
#include <cmath>

using namespace sc_core;

SCA_TDF_MODULE(variable_delay)
{
    // Ports
    sca_tdf::sca_in<double> in_sig;
    sca_tdf::sca_in<double> delay_ctrl;

    sca_tdf::sca_out<double> out_sig;

    // Internal buffer
    std::deque<double> buffer;

    // timestep
    double ts;

    SCA_CTOR(variable_delay)
    {
        // 100 fs timestep
        set_timestep(100.0, SC_FS);
    }

    void initialize()
    {
        ts = get_timestep().to_seconds();

        // Pre-fill buffer
        for(int i = 0; i < 10000; i++)
            buffer.push_back(0.0);
    }

    void processing()
    {
        // Read input sample
        double x = in_sig.read();

        // Read desired delay (seconds)
        double delay_s = delay_ctrl.read();

        // Convert delay to samples
        unsigned delay_samples =
            (unsigned)std::round(delay_s / ts);

        // Store newest sample
        buffer.push_back(x);

        // Safety limit
        if(delay_samples >= buffer.size())
            delay_samples = buffer.size() - 1;

        // Read delayed sample
        double y =
            buffer[buffer.size() - 1 - delay_samples];

        out_sig.write(y);

        // Prevent unlimited growth
        if(buffer.size() > 20000)
            buffer.pop_front();
    }
};