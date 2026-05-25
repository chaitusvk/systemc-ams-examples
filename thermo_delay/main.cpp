#include <systemc>
#include <systemc-ams>

#include "square_wave.hpp"
#include "thermo_delay.hpp"
#include "thermo_ctrl.hpp"


SC_MODULE(tb)
{
    sca_tdf::sca_signal<double> sig_in;
    sca_tdf::sca_signal<double> sig_out;

    square_wave_gen gen;
    thermo_delay    dly;
    thermo_ctrl     ctrl;

    SC_CTOR(tb)
    : gen("gen")
    , dly("dly", 8)
    , ctrl("ctrl")
    {
        gen.out(sig_in);

        dly.in_sig(sig_in);
        dly.out_sig(sig_out);

        for(int i = 0; i < 8; i++)
            dly.thermo[i](ctrl.ctrl[i]);
    }
};


int sc_main(int argc, char* argv[])
{
    sc_set_time_resolution(1, SC_FS);

    tb t("t");

    sca_util::sca_trace_file* tf =
        sca_util::sca_create_vcd_trace_file("thermo_delay");

    sca_util::sca_trace(tf, t.sig_in,  "input");
    sca_util::sca_trace(tf, t.sig_out, "output");

    sc_start(200, SC_PS);

    sca_util::sca_close_vcd_trace_file(tf);

    return 0;
}