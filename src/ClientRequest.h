#ifndef CLIENT_REQUEST_H
#define CLIENT_REQUEST_H

#include <NTL/ZZ.h>
#include "HELibBitVec.h"
#include "FHE.h"

using namespace std;

struct ClientRequest {
    long n_client_elems; // Number of encoded client elements
    long helibvec_depth; // How many ciphertexts are grouped
    string data;         // Serialized as following: context, pubkey, seckey, data

};

#endif /* CLIENT_REQUEST_H */