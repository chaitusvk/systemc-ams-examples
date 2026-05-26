#include <systemc>
#include <systemc-ams>
#include <cmath>
#include <random>
#include <fstream>
#include <vector>

SCA_TDF_MODULE(sink)
{
    sca_tdf::sca_in<double> rx;

    //--------------------------------
    // Timing
    //--------------------------------

    double ui;
    double timestep;
    double bitrate;

    int samples_per_ui;
    int sample_count;


    //------------------------------------
    // Eye diagram storage
    //------------------------------------
    std::ofstream eye_file;


    SCA_CTOR(sink): bitrate(50e9)
    {
    }

    void set_attributes()
    {
        //--------------------------------
        // 1 ps timestep
        //--------------------------------

        timestep = 10e-15;

        set_timestep(
            sc_core::sc_time(
                timestep,
                sc_core::SC_SEC));

        //--------------------------------
        // 20 ps UI
        //--------------------------------

        ui = 1.0 / bitrate;

        //--------------------------------
        // Samples/UI
        //--------------------------------

        samples_per_ui =
            (int)(ui / timestep);

        sample_count = 0;

        eye_file.open("eye_data.csv");

        eye_file << "time_ps,voltage\n";
    }

    void processing()
    {
        double t =
            sc_core::sc_time_stamp()
            .to_seconds();

        
        

        

      
        double ch_v = rx.read();
        
        //--------------------------------
        // Eye data capture
        //--------------------------------

        double eye_phase =
            fmod(t, 2.0 * ui);

        eye_file
            << eye_phase * 1e12
            << ","
            << ch_v
            << "\n";
    }

    //------------------------------------
    // Cleanup
    //------------------------------------
    void end_of_simulation()
    {
        eye_file.close();
    }


};