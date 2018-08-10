// xolotl
// this template file is used by xolotl.transpile() to convert
// the xolotl pseudo-object in MATLAB into a C++
// file that can be compiled from mex
// DON'T TOUCH ANYTHING HERE!

#include <cmath>
#include <vector>
#include <typeinfo>
#include "mex.h"
//xolotl:include_headers_here


using namespace std;

// declare global variable
// so that other code can access verbosity
// double verbosity;

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    // declare pointers to outputs
    double *output_state;
    double *output_V;
    double *output_Ca;
    double *output_I_clamp;
    double *output_curr_state; // currents
    double *output_syn_state;  // synapses
    double *output_cont_state; // mechanisms


    //xolotl:define_v_clamp_idx

    // make an empty network
    network xolotl_network;

    int n_synapses = 0;

    //xolotl:input_declarations


    // temperature wire-ups
    xolotl_network.temperature = temperature;
    xolotl_network.temperature_ref = temperature_ref;
    xolotl_network.verbosity = verbosity;
    double delta_temperature = (temperature - temperature_ref)/10;

    //xolotl:insert_constructors


    //xolotl:add_neurons_to_network

    //xolotl:add_conductances_here


    vector<synapse*> synapses; // pointers to all synapses
    //xolotl:add_synapses_here


    //xolotl:add_mechanisms_here



    //xolotl:call_methods_here
    int nsteps = (int) floor(t_end/sim_dt);
    int progress_report = (int) floor(nsteps/10);

    int nsteps_out = (int) floor(t_end/dt);
    int n_comp = (int) (xolotl_network.comp).size(); // these many compartments


    // ask each controller (nicely) what their
    // full state size is
    int full_controller_sizes[n_comp];
    int full_controller_size = 0;
    for (int i = 0; i < n_comp; i ++)
    {
        int n_cont = (xolotl_network.comp[i])->n_cont;

        full_controller_sizes[i] = xolotl_network.comp[i]->getFullMechanismSize();
        full_controller_size += full_controller_sizes[i];
    }


    // compute ionic current state dimensions
    // assumed to be the same for all conductances
    int full_current_size = 0;
    for (int i = 0; i < n_comp; i ++)
    {
        full_current_size += (xolotl_network.comp[i])->n_cond;
    }

    // compute synapse state dim
    int full_synaptic_sizes[n_comp];
    int full_synaptic_size = 0;
    for (int i = 0; i < n_comp; i ++)
    {
        int n_syn = (xolotl_network.comp[i])->n_syn;

        full_synaptic_sizes[i] = xolotl_network.comp[i]->getFullSynapseSize();
        full_synaptic_size += full_synaptic_sizes[i];
    }

    // set up outputs as mex objects
    int res = dt/sim_dt;
    if (verbosity > 0)
    {
        mexPrintf("[C++] res = %i\n",res);
        mexPrintf("[C++] nsteps = %i\n",nsteps);
    }

    plhs[0] = mxCreateDoubleMatrix(param_size, 1, mxREAL);
    output_state = mxGetPr(plhs[0]);

    if (nlhs > 1) {
        plhs[1] = mxCreateDoubleMatrix(n_comp, nsteps_out, mxREAL);
        output_V = mxGetPr(plhs[1]);
    }
    if (nlhs > 2) {
        plhs[2] = mxCreateDoubleMatrix(2*n_comp, nsteps_out, mxREAL);
        output_Ca = mxGetPr(plhs[2]);
    }

    if (nlhs > 3) {
        plhs[3] = mxCreateDoubleMatrix(full_controller_size, nsteps_out, mxREAL);
        output_cont_state = mxGetPr(plhs[3]);
    }

    if (nlhs > 4) {
        plhs[4] = mxCreateDoubleMatrix(full_current_size, nsteps_out, mxREAL);
        output_curr_state = mxGetPr(plhs[4]);
    }

    if (nlhs > 5) {
        plhs[5] = mxCreateDoubleMatrix(full_synaptic_size, nsteps_out, mxREAL);
        output_syn_state = mxGetPr(plhs[5]);
    }

    // link up I_ext and V_clamp
    double * I_ext = new double[n_comp];
    double * V_clamp = new double[n_comp];
    double * I_ext_in = mxGetPr(prhs[1]);
    double * V_clamp_in = mxGetPr(prhs[2]);


    // figure out the sizes of the arrays
    // for V_clamp and I_ext
    const mwSize *I_ext_dim, *V_clamp_dim;
    I_ext_dim = mxGetDimensions(prhs[1]);
    V_clamp_dim = mxGetDimensions(prhs[2]);

    int I_ext_size_1 = I_ext_dim[0];
    int I_ext_size_2 = I_ext_dim[1];
    int V_clamp_size_1 = V_clamp_dim[0];
    int V_clamp_size_2 = V_clamp_dim[1];



    if (verbosity > 0)
    {
        if (I_ext_size_2 == nsteps)
        {
            mexPrintf("[C++] dynamically changing I_ext\n");
        } else {
            mexPrintf("[C++] fixed I_ext\n");
        }

        if (V_clamp_size_2 == nsteps)
        {
            mexPrintf("[C++] dynamically changing V_clamp\n");
        } else {
            mexPrintf("[C++] fixed V_clamp\n");
        }
    }



    // copy I_ext so we can use it
    for(int q = 0; q < n_comp; q++)
    {
        I_ext[q] = I_ext_in[q];
        V_clamp[q] = V_clamp_in[q];
        // mexPrintf("I_ext =  %f ", I_ext_in[q]);
    }



    // figure out if we're voltage clamping
    // if any V_clamp is non-NaN, then we are
    bool is_voltage_clamped = false;
    for (int j = 0; j < n_comp; j++)
    {
        if (!isnan(V_clamp[j]))
        {
            is_voltage_clamped = true;
        }
    }


    // resolve the tree (for multi-compartment models)
    xolotl_network.resolveTree();

    mexEvalString("drawnow;");

    int percent_complete = 10;

    if (is_voltage_clamped)
    {
        // do the integration respecting V_clamp
        int output_idx = 0;
        for(int i = 0; i < nsteps; i++)
        {



            if (V_clamp_size_2 == nsteps)
            {
                // I_ext is dynamically changing
                for(int q = 0; q < n_comp; q++)
                {
                    V_clamp[q] = V_clamp_in[i*n_comp + q];
                }
            }

            xolotl_network.integrateClamp(sim_dt, V_clamp, delta_temperature);


            if (i%progress_report == 0 & verbosity > 0)
            {
                mexPrintf("[C++] integration %i %", percent_complete);
                mexPrintf(" complete\n");
                percent_complete += 10;
                mexEvalString("drawnow;");
            }

            // here we're getting the state of every compartment -- V, Ca, and all conductances
            if (i%res == 0)
            {

                for (int j = 0; j < n_comp; j++)
                {

                    if (nlhs > 1) {
                        if (isnan(V_clamp[j]))
                        {
                            output_V[output_idx*n_comp + j] = xolotl_network.comp[j]->V;
                        }
                        else {
                            output_V[output_idx*n_comp + j] = xolotl_network.comp[j]->I_clamp;
                        }
                    }

                    if (nlhs > 2) {
                        output_Ca[output_idx*2*n_comp + j] = xolotl_network.comp[j]->Ca;
                        output_Ca[output_idx*2*n_comp + j + n_comp] = xolotl_network.comp[j]->E_Ca;
                    }


                }
                output_idx ++;
            }
        } // end for loop over nsteps
    }
    else

    {

        // voltage is not clamped
        // do the integration
        int output_idx = 0;
        int cont_idx = 0;
        int cond_idx = 0;
        int syn_idx = 0;
        for(int i = 0; i < nsteps; i++)
        {


            if (I_ext_size_2 == nsteps)
            {
                // I_ext is dynamically changing
                for(int q = 0; q < n_comp; q++)
                {
                    I_ext[q] = I_ext_in[i*n_comp + q];
                }
            }


            xolotl_network.integrate(sim_dt,I_ext, delta_temperature);


            if (i%progress_report == 0 & verbosity > 0)
            {
                mexPrintf("[C++] integration %i %", percent_complete);
                mexPrintf(" complete\n");
                percent_complete += 10;
                mexEvalString("drawnow;");
            }

            // here we're getting the state of every compartment -- V, Ca, and all conductances
            if (i%res == 0)
            {

                for (int j = 0; j < n_comp; j++)
                {
                    // read out voltages
                    if (nlhs > 1) {
                        output_V[output_idx*n_comp + j] = xolotl_network.comp[j]->V;
                    }


                    // read out calcium + E_Ca
                    if (nlhs > 2) {
                        output_Ca[output_idx*2*n_comp + j] = xolotl_network.comp[j]->Ca;
                        output_Ca[output_idx*2*n_comp + j + n_comp] = xolotl_network.comp[j]->E_Ca;
                    }

                    // read out controllers
                    if (nlhs > 3) {
                        cont_idx = (xolotl_network.comp[j]->getFullMechanismState(output_cont_state,cont_idx));
                    }

                    // read out ionic currents
                    if (nlhs > 4)
                    {
                        cond_idx = (xolotl_network.comp[j]->getFullCurrentState(output_curr_state,cond_idx));
                    }


                } // end j loop over compartments

                // read out synaptic currents and full
                // state of all synapses
                if (nlhs > 5)
                {
                    for (int k = 0; k < n_synapses; k++)
                    {
                        syn_idx = (synapses[k]->getFullState(output_syn_state,syn_idx));
                    }

                }


                output_idx ++;
            }
        } // end for loop over nsteps
    }




    // now measure the mean Ca in every compartment
    for(int j = 0; j < n_comp; j++)
    {
        (xolotl_network.comp[j]->Ca_average) = (xolotl_network.comp[j]->Ca_average)/nsteps;
    }

    // here, we are reading the full state back, so we
    // can easily update the xolotl object in the MATLAB wrapper
    //xolotl:read_state_back


}
