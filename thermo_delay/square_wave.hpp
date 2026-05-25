#include <systemc-ams>
#include <cmath>

using namespace sc_core;

SCA_TDF_MODULE(square_wave_gen)
{
    sca_tdf::sca_out<double> out;

    // Parameters
    double v_high;
    double v_low;
    double frequency;

    double rise_time;
    double fall_time;

    double period;
    double half_period;

    SCA_CTOR(square_wave_gen)
    {
        v_high = 1.0;
        v_low  = -1.0;

        frequency  = 50e9;   // 50 GHz
        rise_time  = 2e-12;  // 2 ps
        fall_time  = 2e-12;  // 2 ps

        period      = 1.0 / frequency;
        half_period = period / 2.0;

        // 100 fs timestep
        set_timestep(100.0, SC_FS);
        //set_timestep(1.0, SC_PS);

    }

    void processing()
    {
        double t = sc_time_stamp().to_seconds();

        // Time within one cycle
        double local_t = fmod(t, period);

        double val;

        // Rising edge
        if(local_t < rise_time)
        {
            val = v_low +
                  (v_high - v_low) * (local_t / rise_time);
        }

        // High level
        else if(local_t < (half_period - fall_time))
        {
            val = v_high;
        }

        // Falling edge
        else if(local_t < half_period)
        {
            double tf = local_t - (half_period - fall_time);

            val = v_high -
                  (v_high - v_low) * (tf / fall_time);
        }

        // Low level
        else
        {
            val = v_low;
        }

        out.write(val);
    }
};