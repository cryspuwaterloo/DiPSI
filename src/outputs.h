#ifndef OUTPUTS_H
#define OUTPUTS_H

#include <vector>
#include <fstream>
#include <iostream>

#include <NTL/ZZ.h>
#include "FHE.h"
#include "EncryptedArray.h"
#include <NTL/BasicThreadPool.h>

using namespace std;
// us = microseconds

    struct ClientOutputs {
        // Whether it's a cardinality or intersection request.
        bool cardinality;
        uint64_t num_bytes_sent;
        uint64_t num_bytes_received;
        // Time to initialize SEAL objects and data structures.
        uint64_t setup_us;
        // Time to encode and encrypt client inputs.
        uint64_t encode_us;
        // Time to receive request results from server.
        uint64_t request_us;
        // Time to decrypt received ciphertexts.
        uint64_t decrypt_us;
        // Total time.
        uint64_t total_us;
        // Remaining noise budget of each output.
        // If 0, very likely the result is incorrect.
        vector<int> remaining_noise_budget;

        // Request outputs. Can be used to ensure correctness.
        // For cardinality requests, cardinality.
        // For intersection requests, only the values in intersection.
        vector<int> results;
    };
    struct ServerOutput {

        bool cardinality;       // Cardinality or Intersection

        Ctxt cardinality_ctxt; // Encrypted vector with the sum being the differentially private cardinality
        vector<Ctxt> matches;  // Encrypted vectors about matches that have at most one '1' entry

        // Time to initialize SEAL objects and data structures.
        uint64_t setup_us;
        // Total time for request.
        // Excludes time to write to file, so might be more accurate than client's.
        uint64_t total_us;
        // Once per client input
        // Preparing rotated plaintexts and DP encryptions
        vector<uint64_t> encode_us;

        // Once per client input
        // Combining results of different client inputs
        vector<uint64_t> combine_us;
        // Once per round
        // Bitwise equality checking
        vector<uint64_t> equality_us;

        // Once per round
        // ANDing together equality results
        vector<uint64_t> and_us;

        // Once per round
        // Adding over batches
        vector<uint64_t> add_us;
        // For cardinality requests, the noise sample added.
        long card_noise;
        // For intersection requests, one pair per round.
        // p and q samples, to determine correctness.
        vector<bool> ps;
        vector<bool> qs;
    };

#endif /* OUTPUTS_H */