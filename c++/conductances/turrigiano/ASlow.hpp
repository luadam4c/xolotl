// _  _ ____ _    ____ ___ _
//  \/  |  | |    |  |  |  |
// _/\_ |__| |___ |__|  |  |___
//
// the A current, a potassium current
// for some reason I don't understand, I get compiler
// errors when I name this "A" or "Ka"
// so we'll have to live with this awkward name
// http://www.jneurosci.org/content/jneuro/18/7/2309.full.pdf
#ifndef ASLOW
#define ASLOW
#include "conductance.hpp"

//inherit conductance class spec
class ASlow: public conductance {

public:

    // specify parameters + initial conditions
    ASlow(double g_, double E_, double m_, double h_)
    {
        gbar = g_;
        E = E_;
        m = m_;
        h = h_;

        // defaults 
 if (isnan(gbar)) { gbar = 0; }
        if (isnan (m)) { m = 0; }
        if (isnan (h)) { h = 1; }
        if (isnan (E)) { E = -20; }
    }

    void integrate(double, double);

    double m_inf(double, double);
    double h_inf(double, double);
    double tau_m(double, double);
    double tau_h(double, double);
    string getClass(void);
};


string ASlow::getClass(){
    return "ASlow";
}

void ASlow::integrate(double V, double Ca)
{
    m = m_inf(V,Ca) + (m - m_inf(V,Ca))*exp(-dt/tau_m(V,Ca));
    h = h_inf(V,Ca) + (h - h_inf(V,Ca))*exp(-dt/tau_h(V,Ca));
    g = gbar*m*m*m*h;
}

double ASlow::m_inf(double V, double Ca) {return 1.0/(1.0+exp((V+24.3)/-9.4)); }
double ASlow::h_inf(double V, double Ca) {return 1.0/(1.0+exp((V+61.3)/6.6)); }
double ASlow::tau_m(double V, double Ca) {return 13.3 - 9/(1.0+exp((V+50.3)/-11.8));}
double ASlow::tau_h(double V, double Ca) {return 9821 - 9269/(1.0+exp((V+69.9)/-4.6));}


#endif
