#include <systemc>
#include <systemc-ams>

#include "square_wave.hpp"

SC_MODULE(tb)
{
    sca_tdf::sca_signal<double> sig;

    square_wave_gen gen;

    SC_CTOR(tb)
    : gen("gen")
    {
        gen.out(sig);
    }
};

int sc_main(int argc, char* argv[])
{
    sc_set_time_resolution(1, SC_FS);
    
    tb t("t");

    sca_util::sca_trace_file* tf =
        sca_util::sca_create_vcd_trace_file("square_50ghz_rf");

    sca_util::sca_trace(tf, t.sig, "sq_wave");

    // Simulate 200 ps
    sc_start(200, SC_PS);

    sca_util::sca_close_vcd_trace_file(tf);

    return 0;
}