#include <systemc>
#include <systemc-ams>

#include "ams_ser.hpp"
#include "prbs_7.hpp"
#include "sink.hpp"

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

    //--------------------------------
    // Modules
    //--------------------------------

    prbs7_gen_32 prbs("prbs");

    serializer_32to1_ams ser("ser");

    sink sink_mod("sink_mod");

    


    //--------------------------------
    // Connections
    //--------------------------------

    prbs.clk(clk_par);

    prbs.data_out(prbs_bus);

    ser.din(prbs_bus);

    ser.tx(tx_sig);

    sink_mod.rx(tx_sig);

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

    //--------------------------------
    // Run
    //--------------------------------

    sc_core::sc_start(
        15,
        sc_core::SC_NS);

    sca_util::sca_close_vcd_trace_file(tf);

    return 0;
}