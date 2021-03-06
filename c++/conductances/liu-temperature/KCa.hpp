// _  _ ____ _    ____ ___ _
//  \/  |  | |    |  |  |  |
// _/\_ |__| |___ |__|  |  |___
//
// Slow Calcium conductance
// http://www.jneurosci.org/content/jneuro/18/7/2309.full.pdf
#ifndef KCA
#define KCA
#include "conductance.hpp"

//inherit conductance class spec
class KCa: public conductance {

public:

    double Q_g;
    double Q_tau_m;
    double Q_tau_h;
    
    // specify parameters + initial conditions
    KCa(double g_, double E_, double m_, double h_, double Q_g_, double Q_tau_m_, double Q_tau_h_)
    {
        gbar = g_;
        E = E_;
        m = m_;
        h = h_;

        Q_g = Q_g_;
        Q_tau_m = Q_tau_m_;
        Q_tau_h = Q_tau_h_;

        // defaults 
        if (isnan(gbar)) { gbar = 0; }
        if (isnan (m)) { m = 0; }
        if (isnan (h)) { h = 1; }
        if (isnan (Q_g)) { Q_g = 1; }
        if (isnan (Q_tau_m)) { Q_tau_m = 1; }
        if (isnan (Q_tau_h)) { Q_tau_h = 1; }
        if (isnan (E)) { E = -80; }
    }

    void integrate(double, double);

    double m_inf(double V, double Ca);
    double tau_m(double, double);
    string getClass(void);


};

string KCa::getClass(){return "KCa";}

void KCa::integrate(double V, double Ca)
{

    double delta_temp = (temperature - temperature_ref)/10;

    m = m_inf(V,Ca) + (m - m_inf(V,Ca))*exp(-dt/tau_m(V,Ca));
    g = pow(Q_g, delta_temp)*gbar*m*m*m*m;
}

double KCa::m_inf(double V, double Ca) { return (Ca/(Ca+3.0))/(1.0+exp((V+28.3)/-12.6)); }
double KCa::tau_m(double V, double Ca) {return 90.3 - 75.1/(1.0+exp((V+46.0)/-22.7));}

#endif
