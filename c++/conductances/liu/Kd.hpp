// _  _ ____ _    ____ ___ _
//  \/  |  | |    |  |  |  |
// _/\_ |__| |___ |__|  |  |___
//
// inward rectifying potassium conductance
// http://www.jneurosci.org/content/jneuro/18/7/2309.full.pdf
#ifndef KD
#define KD
#include "conductance.hpp"

//inherit conductance class spec
class Kd: public conductance {

public:

    //specify both gbar and erev and initial conditions
    Kd(double g_, double E_, double m_)
    {
        gbar = g_;
        E = E_;
        m = m_;

        // defaults 
        if (isnan(gbar)) { gbar = 0; }
        if (isnan (m)) { m = 0; }
        if (isnan (E)) { E = -80; }

        p = 4;

        // allow this channel to be approximated
        approx_m = 1;
    }

    double m_inf(double, double);
    double tau_m(double, double);
    string getClass(void);

};

string Kd::getClass(){
    return "Kd";
}


double Kd::m_inf(double V, double Ca) {return 1.0/(1.0+exp((V+12.3)/-11.8));}
double Kd::tau_m(double V, double Ca) {return 7.2 - 6.4/(1.0+exp((V+28.3)/-19.2));}


#endif
