// _  _ ____ _    ____ ___ _
//  \/  |  | |    |  |  |  |
// _/\_ |__| |___ |__|  |  |___
//
// abstract class for defining synapses
// synapses connect two compartments
// typically, neurons.

#ifndef SYNAPSE
#define SYNAPSE
#include <cmath>
class compartment;

class synapse {
protected:


    
public:
    compartment *post_syn; // pointer to pre-synaptic compartment
    compartment *pre_syn; // pointer to pre-synaptic compartment
    double gmax = 0;
    double g = 0;
    double E = 0;
    
    // dynamic variables
    double s = 0;
    double k_s[4] = {0,0,0,0};

    bool is_electrical = false;

    double dt = 0;
    double verbosity = 0;
    double temperature_ref = 11;
    double temperature = 11;


    synapse()
    {
        pre_syn = 0; //null pointer for safety
        post_syn = 0;
    }
    ~synapse() {}

    virtual void integrate(void);
    virtual void integrateMS(int, double, double);

    virtual void checkSolvers(int);

    virtual void connect(compartment*, compartment*) = 0;// tells compiler they will be overridden by derived class
    virtual int getFullStateSize(void) = 0;
    virtual int getFullState(double*, int) = 0;


};


void synapse::integrate() {
    mexErrMsgTxt("[synapse] Unimplemented integration method\n");
}

void synapse::integrateMS(int k, double V, double Ca) {
    mexErrMsgTxt("[synapse] Unimplemented integration method\n");
}

void synapse::checkSolvers(int k){
    mexErrMsgTxt("[synapse] Unsupported solver order\n");
}


#endif
