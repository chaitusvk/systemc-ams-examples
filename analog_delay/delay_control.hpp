#include <systemc-ams>
#include <deque>
#include <cmath>

using namespace sc_core;


SCA_TDF_MODULE(delay_control)
{
    sca_tdf::sca_out<double> delay_out;

    SCA_CTOR(delay_control)
    {
        set_timestep(100.0, SC_FS);
    }

    void processing()
    {
        // 5 ps delay
        delay_out.write(5e-12);
    }
};