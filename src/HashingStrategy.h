//
// Created by Nils Lukas on 15/04/19.
//

#ifndef FHE_HASHINGSTRATEGY_H
#define FHE_HASHINGSTRATEGY_H

#include <list>
#include "vector"

using namespace std;

class HashingStrategy {
public:
    struct binned_vec {
        // The bins encoded horizontally (e.g. for max_cap=3 we have at most three elements)
        vector<vector<long>> bin0, bin1;
    };

    /** Applies binning to data. Hashes all data across all bins
     * @param dat: The raw client data to encode
     * @param n_bins: How many bins should be created
     * @param dummy: The dummy element to fill empty spots
     * @param max_capacity: The maximum capacity for each bin
     * @param determine_max_capacity: Flag to treat max_capacity as part of the output in case the server encodes
     **/
    virtual binned_vec hash_data(vector<long> dat, int n_bins, long dummy, int* max_capacity,
            bool determine_max_capacity, bool all_data_per_bin) = 0;


};


#endif //FHE_HASHINGSTRATEGY_H
