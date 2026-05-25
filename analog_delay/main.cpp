#include <systemc>
#include <systemc-ams>

#include "square_wave.hpp"
#include "variable_delay.hpp"
#include "delay_control.hpp"

SC_MODULE(tb)
{
    sca_tdf::sca_signal<double> sig_in;
    sca_tdf::sca_signal<double> sig_out;
    sca_tdf::sca_signal<double> delay_sig;

    square_wave_gen gen;
    variable_delay dly;
    delay_control ctrl;

    SC_CTOR(tb)
    : gen("gen")
    , dly("dly")
    , ctrl("ctrl")
    {
        gen.out(sig_in);

        ctrl.delay_out(delay_sig);

        dly.in_sig(sig_in);
        dly.delay_ctrl(delay_sig);
        dly.out_sig(sig_out);
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