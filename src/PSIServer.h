//
// Created by overholt on 04/03/19.
//

#ifndef FHE_PSISERVER_H
#define FHE_PSISERVER_H

#include "outputs.h"

#include "ClientRequest.h"
#include "ServerResponse.h"

#include <random>   /* exponential_distribution */
#include <iomanip> /* Fixed precision prints */

using namespace std;

class PSIServer {
public:
    /** Storage for randomized response */
    enum RandResponse {
        KEEP = 0,
        NO_MATCH = 1,
        MATCH = 2,

    };

    /**
     * Returns encrypted client data according to the specification from the
     * ExperimentParams class
     */
    ServerResponse perform_psi(ClientRequest &req, const ExperimentParams& params, ExperimentLogs &logs);


};

#endif //FHE_PSISERVER_H
