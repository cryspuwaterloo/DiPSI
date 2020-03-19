//
// Created by overholt on 04/03/19.
//

#ifndef FHE_PSICLIENT_H
#define FHE_PSICLIENT_H

#include "outputs.h"
#include "ClientRequest.h"
#include "ServerResponse.h"
#include "HashingStrategy.h"

#include <iomanip> /* Fixed precision couts */

using namespace std;

struct TmpFHEContext {
    std::unique_ptr<FHEcontext> context;
    std::unique_ptr<FHESecKey> secKey;
    FHEPubKey* pubKey;
};

class PSIClient {

public:
    /**
     * Returns encrypted client data according to the specification from the
     * ExperimentParams class for the PSI request
     */
    ClientRequest gen_psi_request(ExperimentParams &params, TmpFHEContext*, ExperimentLogs&);

    /**
     * Processes a server response by finding out number of matches
     */
    void process_psi_response(ServerResponse &res, ExperimentParams &p, TmpFHEContext*, ExperimentLogs&);

};

#endif //FHE_PSICLIENT_H
