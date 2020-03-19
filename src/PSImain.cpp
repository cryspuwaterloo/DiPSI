//
// Created by overholt on 04/03/19.
//

#include "PSIClient.h"
#include "PSIServer.h"

#include <string.h> 
#include "outputs.h"
#include "params.h"

ExperimentLogs run_psi(ExperimentParams &p) {
    // Needed for transferring the HELib context to the client receive function
    TmpFHEContext tmpFHEContext = {};
    ExperimentLogs logs = {};

    // Build the client and generate the PSI
    PSIClient client;
    ClientRequest req = client.gen_psi_request(p, &tmpFHEContext, logs);

    cout << "<---- TRANSMISSION ---->" << endl;

    // Build the server and do the PSI with the client request and the experiment parameters
    PSIServer server;
    ServerResponse res = server.perform_psi(req, p, logs);

    cout << "<---- TRANSMISSION ---->" << endl;

    // Evaluate the response of the PSI and with the stored context from before
    client.process_psi_response(res, p, &tmpFHEContext, logs);

    return logs;
}

int main(int argc, char *argv[]) {
    // INPUT: Specify the experiment parameters here
    ExperimentParams p;

    for (int i = 1; i < argc; ++i) {
        string cmd = argv[i];
        if(i+1 >= argc) {
            cout << "Error, not sufficient parameters provided!" << endl;
            return -1;
        }
        if(cmd == "-b") {
            // Bitlength
            p.bitlen = stoi(argv[++i]);
	    p.no_match = (long) pow(2, p.bitlen)-2,                    // Element that is DEFINITELY NOT in server data (for dp) (CLIENT)
            p.no_match2 = (long) pow(2, p.bitlen)-1;                   // Element that is DEFINITELY NOT in server data (for server padding)
        }else if(cmd == "-n") {
            // Number of client elements
            p.n_client = stoi(argv[++i]);
        }else if(cmd == "-m") {
            // Number of server elements
            p.n_server = stoi(argv[++i]);
	}else if(cmd == "-k") {
	    // Security paramter
	    p.k = stoi(argv[++i]);
	} else if(cmd == "-N") {
	    // Plaintext modulus
	    p.p_mod = stoi(argv[++i]);
        }else if(cmd == "-L") {
            // Length of modulus chain
            p.L = stoi(argv[++i]);
        }else if(cmd == "-dp") {
            // Apply dp
            p.apply_diff_privacy = strcmp(argv[++i], "true") == 1;
        }else if(cmd == "--mode") {
            // Apply dp
            p.mode = strcmp(argv[++i], "intersection") == 1 ? INTERSECTION : CARDINALITY;
        }else if(cmd == "-t") {
            // Number of threads
            p.nthreads = stoi(argv[++i]);
        }
    }
    p.init();

    // Set the number of threads
    if(p.nthreads > 1) NTL::SetNumThreads(p.nthreads);

    // OUTPUT: Everything you see in the console is also accessible through this struct
    ExperimentLogs logs = run_psi(p);

}
