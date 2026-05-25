#include <systemc-ams>
#include <systemc>
#include <deque>
#include <vector>
#include <cmath>

using namespace sc_core;

SCA_TDF_MODULE(thermo_delay)
{
    // Analog signal
    sca_tdf::sca_in<double> in_sig;
    sca_tdf::sca_out<double> out_sig;

    // Thermometer-coded digital inputs
    sc_core::sc_vector<
        sca_tdf::sca_de::sca_in<bool>
    > thermo;

    // Delay per bit
    double delay_step;

    // Internal
    std::deque<double> buffer;
    double ts;

    // Constructor
    thermo_delay(sc_core::sc_module_name nm,
                 int nbits = 8)
    : in_sig("in_sig")
    , out_sig("out_sig")
    , thermo("thermo", nbits)
    {
        delay_step = 1e-12; // 1 ps/bit

        set_timestep(100.0, SC_FS);
    }

    void initialize()
    {
        ts = get_timestep().to_seconds();

        // Buffer initialization
        for(int i = 0; i < 50000; i++)
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

        // Total delay
        double total_delay =
            ones * delay_step;

        // Convert to samples
        unsigned delay_samples =
            (unsigned)std::round(total_delay / ts);

        // Store input
        buffer.push_back(in_sig.read());

        // Limit
        if(delay_samples >= buffer.size())
            delay_samples = buffer.size() - 1;

        // Read delayed sample
        double y =
            buffer[buffer.size()-1-delay_samples];

        out_sig.write(y);

        // Prevent infinite growth
        if(buffer.size() > 100000)
            buffer.pop_front();
    }
};