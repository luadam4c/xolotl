// _  _ ____ _    ____ ___ _    
//  \/  |  | |    |  |  |  |    
// _/\_ |__| |___ |__|  |  |___ 
//
// class defining one compartment

#ifndef COMPARTMENT
#define COMPARTMENT
#include <cmath>
#include <vector>
#include "conductance.hpp"
#include "synapse.hpp"
#include "controller.hpp"
class network;

#define F 96485

using namespace std;

class compartment
{
protected:
    
    vector<conductance*> cond; // pointers to all conductances in compartment
    vector<synapse*> syn; // pointers to synapses onto this neuron. 
    vector<controller*> cont; // pointers to controllers 
    vector<int> controller_sizes; // stores sizes of each controller's full state
    
    // voltage and other state variables (calcium, ..
    double sigma_g;
    double sigma_gE;
    double V_inf;         
    double Ca_inf; 


    double RT_by_nF;

public:

    // pointer to network that contains this
    network * container;

    // this int stores an integer that indicates 
    // the hierarchy of this compartment in a multi-comp
    // neuron tree. tree_idx of 0 means it is a soma
    // compartment
    double tree_idx; 

    // pointers to upstream and downstream compartments
    // (will be generated on initialization)
    compartment * upstream;
    compartment * downstream;

    // neuron parameters
    double Cm; // specific capacitance 
    double A; 
    double phi; 
    double Ca_in;
    double Ca_out; 
    double tau_Ca;
    double vol;
    double Ca_target; // for homeostatic control 


    // stores the average Ca over the integration
    // window. useful for quickly determining 
    // if integral control has worked without 
    // pulling out the full trace 
    double Ca_average;

    double V;          
    double Ca; 
    double E_Ca;
    double i_Ca; // specific calcium current (current/area. nA/mm^2)
    double I_ext; // all external currents are summed here
    double I_clamp; // this is the current required to clamp it 
    int n_cond; // this keep tracks of the # channels
    int n_cont; // # of controllers 
    int n_syn; // # of synapses

    // constructor with all parameters 
    compartment(double V_, double Ca_, double Cm_, double A_, double vol_, double phi_, double Ca_out_, double Ca_in_, double tau_Ca_, double Ca_target_, double Ca_average_, double tree_idx_)
    {
        V = V_;
        vol = vol_;
        Ca = Ca_;

        sigma_g = 0;
        sigma_gE = 0;
        V_inf = 0;
        Ca_inf = 0;

        Cm = Cm_;
        A = A_; 
        phi = phi_; 
        Ca_in = Ca_in_; 
        Ca_out = Ca_out_;
        tau_Ca = tau_Ca_;
        Ca_target = Ca_target_;

        Ca_average = Ca_average_;
        Ca_average = 0; // reset it every time

        RT_by_nF = 500.0*(8.6174e-5)*(10 + 273.15);

        tree_idx = tree_idx_;

        // defaults
        if (isnan (V)) { V = -60; }
        if (isnan (Ca)) { Ca = Ca_in; }
        if (isnan (Ca_target)) { Ca_target = Ca_in; }     

        // housekeeping
        E_Ca = 0;
        i_Ca = 0; // this is the current density (nA/mm^2)
        n_cond = 0;
        n_cont = 0;
        n_syn = 0; 
        container = NULL;
        upstream = NULL;
        downstream = NULL;

    }
    // begin function declarations 
    void addConductance(conductance*);
    void addSynapse(synapse*);
    void addController(controller*);

    void integrate(double, double);
    void integrateControllers(double, double);
    void integrateChannels(double, double, double, double);
    void integrateSynapses(double, double, double);
    void integrateVC(double, double, double, double);
    void integrateC_V_clamp(double, double, double, double);
    void get_cond_state(double*);
    int getFullControllerState(double*, int);


    int getFullControllerSize(void);

    controller* getControllerPointer(int);
    void connect(network*);

    compartment* getConnectedCompartment(int);

};


// returns a list of connected compartments.
// this function returns only one pointer at a time
// to get them all, iterative over the integer argument
compartment* compartment::getConnectedCompartment(int idx)
{
    compartment* neighbour = NULL;
    for (int i = 0; i < n_syn; i ++)
    {
        neighbour = syn[i]->pre_syn;
    }
    return neighbour;
}

void compartment::connect(network *pnetwork_) {container = pnetwork_; }


int compartment::getFullControllerSize(void)
{
    int full_size = 0;
    for (int i=0; i<n_cont; i++)
    {
        full_size += cont[i]->getFullStateSize();
    }
    return full_size;
}

// simple integrate; use this only for 
// single-compartment non-clamped neurons
void compartment::integrate(double dt, double delta_temperature)
{
    double V_prev = V;
    double Ca_prev = Ca;
    i_Ca = 0;
    I_ext = 0;
    I_clamp = 0;

    integrateChannels(V_prev, Ca_prev, dt, delta_temperature);
    integrateVC(V_prev, Ca_prev, dt, delta_temperature);
}

void compartment::integrateControllers(double Ca_prev, double dt)
{

    // integrate all controllers
    for (int i=0; i<n_cont; i++)
    {
        cont[i]->integrate(Ca_target - Ca_prev, dt);
    }
   
}

void compartment::integrateChannels(double V_prev, double Ca_prev, double dt, double delta_temperature)
{

    sigma_g = 0.0;
    sigma_gE = 0.0;


    // compute E_Ca
    E_Ca = RT_by_nF*log((Ca_out)/(Ca_prev));

    // integrate all channels
    for (int i=0; i<n_cond; i++)
    {
        cond[i]->integrate(V_prev, Ca_prev, dt, delta_temperature);
        sigma_g += cond[i]->g;
        sigma_gE += (cond[i]->g)*(cond[i]->E);
    }

    // update the running total Ca
    Ca_average += Ca;
}

void compartment::integrateSynapses(double V_prev, double dt, double delta_temperature)
{

    // we treat synapses identically to any other conductance 

    // integrate all synapses
    for (int i=0; i<n_syn; i++)
    {
        // mexPrintf("integrating synapse in comp: =  %i\n",&(syn[i]));
        syn[i]->integrate(dt);
        sigma_g += abs((syn[i]->gbar)*(syn[i]->s)/(1000*A)); // now uS/mm^2
        sigma_gE += ((syn[i]->gbar)*(syn[i]->s)*(syn[i]->E)/(1000*A));

    }
}

void compartment::integrateVC(double V_prev, double Ca_prev, double dt, double delta_temperature)
{
    // compute infinity values for V and Ca
    //mexPrintf("sigma_gE =  %f\n",sigma_gE);
    if (sigma_g == 0)
        V_inf = V_prev;
    else
        V_inf = (sigma_gE + (I_ext/A))/sigma_g;

    Ca_inf = Ca_in - (tau_Ca*phi*i_Ca*A*.5)/(F*vol); // microM


    // integrate V and Ca
    V = V_inf + (V_prev - V_inf)*exp(-dt/(Cm/(sigma_g)));
    Ca = Ca_inf + (Ca_prev - Ca_inf)*exp(-dt/tau_Ca);
}


// integrates calcium, but not voltage
// assumes cell is voltage clamped, and 
// returns the current required to clamp it
void compartment::integrateC_V_clamp(double V_clamp, double Ca_prev, double dt, double delta_temperature)
{
    // compute infinity values for Ca
    Ca_inf = Ca_in - (tau_Ca*phi*i_Ca*A*.5)/(F*vol); // microM

    // 

    // integrate Ca
    Ca = Ca_inf + (Ca_prev - Ca_inf)*exp(-dt/tau_Ca);

    // calculate I_clamp, and set voltage to the clamped
    // voltage 
    double E = exp(-dt/(Cm/(sigma_g)));
    V_inf = (V_clamp - V*E)/(1 - E);
    //mexPrintf("sigma_g=  %f\n",sigma_g);
    I_clamp =  A*(V_inf*sigma_g - sigma_gE);

    // normalize by conductance
    // I_clamp =  (V_inf*sigma_g - sigma_gE)/sigma_gE;

    V = V_clamp;
    
}


// add conductance and provide pointer back to compartment 
void compartment::addConductance(conductance *cond_)
{
    cond.push_back(cond_);
    cond_->connect(this);
    n_cond ++; 
}

// add synapse to this compartment (this compartment is after synapse)
void compartment::addSynapse(synapse *syn_)
{
    syn.push_back(syn_);
    n_syn ++; 
}

// add controller to this compartment 
void compartment::addController(controller *cont_)
{
    // mexPrintf("adding controller @  %p\n",cont_);
    cont.push_back(cont_);
    cont_->controller_idx = n_cont; // tell the controller what rank it has
    n_cont ++;

    // also store the controller's full state size
    controller_sizes.push_back(cont_->getFullStateSize());

    // mexPrintf("this controller has size =  %i ", cont_->getFullStateSize());
   
}


// returns a vector of the state of every conductance 
// cond_state is a pointer to a matrix that is hopefully of
// the right size 
void compartment::get_cond_state(double *cond_state)
{
    for (int i = 0; i < n_cond; i ++) 
    {
        cond_state[i*2] = cond[i]->m;
        cond_state[(i*2)+1] = cond[i]->h;
    }
}

// returns a vector of the state of every controller 
// cont_state is a pointer to a matrix that is hopefully of
// the right size 
int compartment::getFullControllerState(double *cont_state, int idx)
{
    for (int i = 0; i < n_cont; i ++) 
    {

        cont[i]->getFullState(cont_state, idx);
        idx += controller_sizes[i];

    }
    return idx;
}

// returns a pointer to a controller in this compartment 
controller * compartment::getControllerPointer(int cont_idx)
{
    // mexPrintf("In compartment, the controllers m is  %f\n",(cont[cont_idx])->m);
    return cont[cont_idx];
}



#endif