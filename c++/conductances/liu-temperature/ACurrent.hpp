// _  _ ____ _    ____ ___ _
//  \/  |  | |    |  |  |  |
// _/\_ |__| |___ |__|  |  |___
//
// the A current, a potassium current
// for some reason I don't understand, I get compiler
// errors when I name this "A" or "Ka"
// so we'll have to live with this awkward name
// http://www.jneurosci.org/content/jneuro/18/7/2309.full.pdf
#ifndef ACURRENT
#define ACURRENT
#include "conductance.hpp"

//inherit conductance class spec
class ACurrent: public conductance {

public:

    double Q_g;
    double Q_tau_m;
    double Q_tau_h;


    // specify parameters + initial conditions
    ACurrent(double g_, double E_, double m_, double h_, double Q_g_, double Q_tau_m_, double Q_tau_h_)
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

    double m_inf(double, double);
    double h_inf(double, double);
    double tau_m(double, double);
    double tau_h(double, double);
    string getClass(void);


};
string ACurrent::getClass(){return "ACurrent";}

void ACurrent::integrate(double V, double Ca)
{

    double delta_temp = (temperature - temperature_ref)/10;

    m = m_inf(V,Ca) + (m - m_inf(V,Ca))*exp(-(dt*pow(Q_tau_m, delta_temp))/tau_m(V,Ca));
    h = h_inf(V,Ca) + (h - h_inf(V,Ca))*exp(-(dt*pow(Q_tau_h, delta_temp))/tau_h(V,Ca));
    g = pow(Q_g, delta_temp)*gbar*m*m*m*h;
}

double ACurrent::m_inf(double V, double Ca) {return 1.0/(1.0+exp((V+27.2)/-8.7)); }
double ACurrent::h_inf(double V, double Ca) {return 1.0/(1.0+exp((V+56.9)/4.9)); }
double ACurrent::tau_m(double V, double Ca) {return 11.6 - 10.4/(1.0+exp((V+32.9)/-15.2));}
double ACurrent::tau_h(double V, double Ca) {return 38.6 - 29.2/(1.0+exp((V+38.9)/-26.5));}

#endif
