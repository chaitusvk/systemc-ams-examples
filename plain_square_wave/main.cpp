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
    tb t("t");

    // Trace
    sca_util::sca_trace_file* tf =
        sca_util::sca_create_vcd_trace_file("sq_50ghz");

    sca_util::sca_trace(tf, t.sig, "square_50ghz");

    // Run 200 ps
    sc_start(200, SC_PS);

    sca_util::sca_close_vcd_trace_file(tf);

    return 0;
}