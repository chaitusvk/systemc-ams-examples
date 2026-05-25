#include<systemc>
#include <systemc-ams>
#include <cmath>


using namespace sc_core;


SC_MODULE(thermo_ctrl)
{
    sc_core::sc_vector<sc_signal<bool>> ctrl;

    SC_CTOR(thermo_ctrl)
    : ctrl("ctrl", 8)
    {
        SC_THREAD(run);
    }

    void run()
    {
        while(true)
        {
            // Example: 4 bits ON
            for(int i = 0; i < 8; i++)
                ctrl[i].write(i < 4);

            wait(50, SC_PS);

            // Example: 6 bits ON
            for(int i = 0; i < 8; i++)
                ctrl[i].write(i < 6);

            wait(50, SC_PS);
        }
    }
};