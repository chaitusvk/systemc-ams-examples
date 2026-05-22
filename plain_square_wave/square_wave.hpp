#include <systemc-ams>
#include <cmath>

using namespace sc_core;

SCA_TDF_MODULE(square_wave_gen)
{
    sca_tdf::sca_out<double> out;

    double amplitude;
    double frequency;
    double period;

    SCA_CTOR(square_wave_gen)
    {
        amplitude = 1.0;          // +/-1V
        frequency = 50e9;         // 50 GHz

        period = 1.0 / frequency; // 20 ps

        // Simulation timestep
        // Must be much smaller than period
        set_timestep(1.0, SC_PS);
    }

    void processing()
    {
        double t = sc_time_stamp().to_seconds();

        if (fmod(t, period) < (period / 2.0))
            out.write(amplitude);
        else
            out.write(-amplitude);
    }
};