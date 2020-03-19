#ifndef PARAMS_H
#define PARAMS_H

#include <string>
#include <iostream>
#include <bitset>
#include <vector>
#include <cmath>
#include <set>
#include <assert.h>

#include "HashingStrategy.h"
#include "FixedBinningHash.h"

using namespace std;

enum Mode {
    CARDINALITY,      // Count number of matches
    INTERSECTION,     // For each client element, check if it is in the intersection
};

/* Generates non-duplicate data. User can specify values it has to contain */
vector<long> gen_rnd_non_dup_data(long n, int bitlen, vector<long> values_to_contain={});

struct ExperimentParams {
    void init() {
        if(client_input.empty()) {
            client_input = gen_rnd_non_dup_data(n_client, bitlen, client_subset);
        }
        if(server_input.empty()) {
            server_input = gen_rnd_non_dup_data(n_server, bitlen, server_subset);
        }
        // Check that no_match is really unique
        long max_val = (long) pow(2, bitlen);
        bool appeared_once = false;
        for(long dat : server_input) {
            if(!appeared_once && dat > max_val) {
                cerr << "Warning: You chose values larger than 2^BITLEN=" << max_val << ". They will be reduced"
                                                                                        " modulo the plaintext modulus!" << endl;
                appeared_once = true;
            }
            if(dat == no_match || dat == no_match2) {
		no_match = (long) pow(2, bitlen)-2;
                cerr << "Invalid choice of parameters. Please re-assign ExperimentParams.no_match. (other than " << no_match << " and " << no_match2 << " bitlen is " << bitlen << ")";

                exit(EXIT_FAILURE);
            }
        }
        assert(no_match < max_val);
        assert(no_match2 < max_val);
    }

    // How many bits used to represent a number
    int bitlen = 16;

    // How many threads to use
    int nthreads = 10;

    // INTERSECTION or CARDINALITY
    Mode mode = INTERSECTION;

    // Apply the differential privacy on the server
    bool apply_diff_privacy = false;

    // How many client and server elements to generate (random draw without putting back)
    long n_client=4096, n_server=1000;
    // Elements guaranteed to be in server and client set to force matches
    vector<long> client_subset = {772, 824},
            server_subset = {824};

    // Unbounded binning approach
    int max_capacity = 2;

    HashingStrategy* binning_strategy = new FixedBinningHash();

    /** Encryption scheme parameters */
    long p_mod = 40961, // Plaintext modulus (IMPORTANT FOR SLOTCOUNT) (1021, 40961,..)
            s = 100,      // Minimum number of slots (m is chosen according to s, so m should be -1 if s is chosen)
            r = 1,         // Lifting polynomials used for bootstrapping
            d = 1,         // Degree of field extension
            c = 2,         // Number of columns in the key-switching matrices (custom rotation sizes)
            k = 128,       // k-bit secure scheme
            L = 450,       // Modulus chain (larger values means more noise budget available) (450 is fine)
            pm = 0,        // find the first m satisfying phi(m)>=N and d | ord(p) in Z_m^* and phi(m)/ord(p) >= s
            m = -1;        // Leave -1 if s is chosen.

    /** Differential Privacy Params **/
    double p = .5,          // (INTERSECTION) Randomized response params (used in intersection only)
            q = .5,          // If p=1 => send exact bit, else throw q. If q=1 => send match, else send no match
            lambda = 1;      // (CARDINALITY) Lambda for exponential distribution sample.

    long    no_match = (long) pow(2, bitlen)-2,                    // Element that is DEFINITELY NOT in server data (for dp) (CLIENT)
            no_match2 = (long) pow(2, bitlen)-1;                   // Element that is DEFINITELY NOT in server data (for server padding)

    /** Debug outputs **/
    bool print_client_data = true;           // Whether to print the data of the client
    bool debug_print_m = true;               // Print selection of m
    bool debug_print_batchsize = true;       // Print number of server elements that can be batched into one ciphertext
    bool debug_cardinality_dp_sample = true; // Print choice for differential private dp sample in cardinality
    bool print_pq_samples = true;            // Print choice for p and q
    bool print_number_of_server_rounds = true; // How many rounds used in the server for matching
    bool debug_server_matching_progress = true; // Output the progress bar for the server matching process

    // Client/Server data definition (Change their initialization in the constructor)
    vector<long> client_input, server_input;
    ExperimentParams() {

    }
};

struct ExperimentLogs {
    /** This class will be filled during the execution of the protocol and makes all values outputted on the
     * console accessible programatically for benchmarks. Values are only filled if they are outputted to the consolse
     * so make sure you have the associated params set in the ExperimentParams.
     */

    /** Client generate request measures **/
    // Encrypted data size of client
    double client_context_size,
            client_ctxt_size,
            client_total_size;

    long slot_count;

    // Timing values for client
    double client_create_context_time,
            client_encrypt_data_time,
            client_binning_data_time,
            client_serialize_data_time,
            client_total_time;

    /** Server measures **/
    double server_data_received_size;
    long server_elems_per_ctxt;
    long server_rounds;
    vector<pair<long,long>> server_pq_samples;
    long server_cardinality_dp_sample;

    double server_deserialize_time,
            server_binning_time,
            server_matching_time,
            server_dp_for_intersection_time,
            server_cardinality_time,
            server_dp_for_cardinality_time,
            server_total_time_server;

    vector<long> server_bit_capacity_matches;
    long server_bit_capacity_cardin;
    double server_response_size;

    /** Client handle response **/
    vector<long> client_matches;
    double client_cardinality;
};

/** Override printout operator to print the params */
ostream& operator<<(ostream& os, const ExperimentParams& p);

#endif /* PARAMS_H */
