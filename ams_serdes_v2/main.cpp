#include <systemc>
#include <systemc-ams>

#include "ams_ser.hpp"
#include "prbs_7.hpp"
#include "sink.hpp"
#include "jitter_delay.hpp"
#include "thermo_ctrl.hpp"

using namespace sc_core;

int sc_main(int argc, char* argv[])
{
    sc_core::sc_set_time_resolution(1, SC_FS);

    //--------------------------------
    // Parallel clock
    //--------------------------------



    sc_core::sc_clock clk_par(
        "clk_par",
        sc_core::sc_time(
            640,
            sc_core::SC_PS));

    //--------------------------------
    // Signals
    //--------------------------------

    sc_core::sc_signal<
        sc_dt::sc_uint<32>
    > prbs_bus;

    sca_tdf::sca_signal<double> tx_sig;
    sca_tdf::sca_signal<double> dly_tx_sig;


    //--------------------------------
    // Modules
    //--------------------------------

    prbs7_gen_32 prbs("prbs");

    serializer_32to1_ams ser("ser");

    sink sink_mod("sink_mod");

    thermo_delay_jitter    dly("dly",8);
    thermo_ctrl            ctrl("ctrl");


    


    //--------------------------------
    // Connections
    //--------------------------------

    prbs.clk(clk_par);

    prbs.data_out(prbs_bus);

    ser.din(prbs_bus);

    ser.tx(tx_sig);

    sink_mod.rx(dly_tx_sig);

    dly.in_sig(tx_sig);
    dly.out_sig(dly_tx_sig);

    for(int i = 0; i < 8; i++)
            dly.thermo[i](ctrl.ctrl[i]);

    //--------------------------------
    // Trace
    //--------------------------------

    sca_util::sca_trace_file* tf =
        sca_util::sca_create_vcd_trace_file(
            "ams_ser");

    sca_util::sca_trace(
        tf,
        tx_sig,
        "tx");
     sca_util::sca_trace(
        tf,
        dly_tx_sig,
        "dly_tx");

    //--------------------------------
    // Run
    //--------------------------------

    sc_core::sc_start(
        5,
        sc_core::SC_NS);

    sca_util::sca_close_vcd_trace_file(tf);

    return 0;
}