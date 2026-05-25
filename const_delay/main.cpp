#include <systemc>
#include <systemc-ams>

#include "square_wave.hpp"
#include "delay.hpp"

SC_MODULE(tb)
{
    sca_tdf::sca_signal<double> sig_in;
    sca_tdf::sca_signal<double> sig_out;

    square_wave_gen gen;
    delay_tdf       dly;

    SC_CTOR(tb)
    : gen("gen")
    , dly("dly")
    {
        gen.out(sig_in);

        dly.in(sig_in);
        dly.out(sig_out);
    }
};

int sc_main(int argc, char* argv[])
{
    sc_set_time_resolution(1, SC_FS);

    tb t("t");

    sca_util::sca_trace_file* tf =
        sca_util::sca_create_vcd_trace_file("delay");

    sca_util::sca_trace(tf, t.sig_in,  "input");
    sca_util::sca_trace(tf, t.sig_out, "delayed");

    sc_start(100, SC_PS);

    sca_util::sca_close_vcd_trace_file(tf);

    return 0;
}