//
// Created by Nils Lukas on 06/03/19.
//

#include "params.h"

vector<long> gen_rnd_non_dup_data(long n, int bitlen, const vector<long> vals_to_contain) {
    assert(n < (long) pow(2, bitlen)+1);

    int rounds = 0;
    set<long> res;

    for(long v : vals_to_contain) {
        res.insert(v);
    }

    int draw = 0;
    while(res.size() < n){
        // Change with non-random draw
        res.insert(draw);
        draw = (draw + 1) % ((int) pow(2, bitlen)-2);

        if(rounds++ > pow(2, bitlen)) {  // Ensure termination in a rather ugly way
            throw "Too many collisions on random data generation! Choose larger BITLEN or smaller n";
        }
    }
    return vector<long>(res.begin(), res.end());
}

ostream& operator<<(ostream& os, const ExperimentParams& p) {
    return os <<
    "   Mode:                   " << (p.mode == INTERSECTION ? "Intersection" : "Cardinality")  << endl <<
    "   Max Binning Capacity:   " << p.max_capacity << endl <<
    "   Bitlength:              " << p.bitlen << endl <<
    "   Threads:                " << p.nthreads << endl <<
    "   Modulus Chain Length:   " << p.L << endl <<
    "   Apply Diff Privacy      " << (p.apply_diff_privacy? "Yes":"No") << endl <<
    "   n_client, n_server:     " << p.client_input.size() << ", " << p.server_input.size() << endl <<
    "   p, q, lambda:           " << p.p << ", " << p.q << ", " << p.lambda;
}