// _  _ ____ _    ____ ___ _    
//  \/  |  | |    |  |  |  |    
// _/\_ |__| |___ |__|  |  |___ 
//
// inward rectifying potassium conductance 
// this version does not support temperature dependence
// http://jn.physiology.org/content/jn/90/6/3998.full.pdf
#ifndef KD
#define KD
#include "../../conductance.hpp"

//inherit conductance class spec
class Kd: public conductance {
    
public:

    //specify both gbar and erev and initial conditions
    Kd(double g_, double E_, double m_, double h_, double Q_g_, double Q_tau_m_, double Q_tau_h_)
    {
        gbar = g_;
        E = E_;
        m = m_;
        h = 1;
        

        Q_g = Q_g_;
        Q_tau_m = Q_tau_m_;
        Q_tau_h = Q_tau_h_;
    }
    
    void integrate(double V, double Ca, double dt, double delta_temp);
    void connect(compartment *pcomp_);
    double m_inf(double V);
    double tau_m(double V);

};

void Kd::connect(compartment *pcomp_) { container = pcomp_; }

void Kd::integrate(double V, double Ca, double dt, double delta_temp)
{
    m = m_inf(V) + (m - m_inf(V))*exp(-dt/tau_m(V));
    g = gbar*m*m*m*m;

}



double Kd::m_inf(double V) {return 1.0/(1.0+exp((V+12.3)/-11.8));}
double Kd::tau_m(double V) {return 14.4 - 12.8/(1.0+exp((V+28.3)/-19.2));}


#endif
