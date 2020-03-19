//
// Created by Nils Lukas on 15/04/19.
//

#include "FixedBinningHash.h"

vector<vector<vector<long>>> FixedBinningHash::init_bins(int numberOfBins) {
    vector<vector<vector<long>>> allBins;
    for (unsigned long hashNumber = 0; hashNumber < NUMBER_OF_HASHES; hashNumber++) {
        allBins.emplace_back();
        for (int binNumber = 0; binNumber < numberOfBins; binNumber++) {
            allBins.at(hashNumber).emplace_back();
        }
    }
    return allBins;
}

void FixedBinningHash::insert(BN_CTX* ctx, vector<vector<vector<long>>> &allBins, long element, int numberOfBins,
        int* max_cap, bool determine_cap, int preferred_bin) {
    int index0 = hash1(ctx, element, numberOfBins),
        index1 = hash2(ctx, element, numberOfBins);

    // Add element to smaller bin
    long sizeBin0 = allBins[0][index0].size(),
         sizeBin1 = allBins[1][index1].size();

    // Determine the maximal and minimal size and the index for the minimal element
    long max_size, min_size, min_hash, min_index;
    if (preferred_bin != -1) {   // always hash into the preferred bin
        min_size = max_size = (preferred_bin == 0? sizeBin0:sizeBin1);
        min_index = (preferred_bin == 0? index0:index1);
        min_hash = preferred_bin;
    } else if (sizeBin0 > sizeBin1) {
        max_size = sizeBin0; min_size = sizeBin1; min_hash = 1; min_index = index1;
    }else {
        max_size = sizeBin1; min_size = sizeBin1; min_hash = 0; min_index = index0;
    }

    if(!determine_cap) {
        if(max_size > *max_cap) printf("[Error] Too many elements in bin 1!");
    }else {
        if(min_size+1 >= *max_cap) *max_cap = (int) min_size+1;
    }

    allBins[min_hash][min_index].push_back(element);
}

void FixedBinningHash::fillBins(vector<vector<vector<long>>> &allBins, long dummyElement, int numberOfBins, int* max_cap0, int* max_cap1) {
    for (int binNumber = 0; binNumber < numberOfBins; binNumber++) {
        while (allBins[0][binNumber].size() < *max_cap0) {
            allBins[0][binNumber].push_back(dummyElement);
        }
    }
    for (int binNumber = 0; binNumber < numberOfBins; binNumber++) {
        while (allBins[1][binNumber].size() < *max_cap1) {
            allBins[1][binNumber].push_back(dummyElement);
        }
    }
}

vector<vector<long>> transpose_data(vector<vector<long>> data, const int* max_capacity) {
    vector<vector<long>> res;
    for(int i=0;i<*max_capacity;i++) {
        res.emplace_back();
        for (auto &j : data) {
            res.at(i).push_back(j.at(i));
        }
    }
    return res;
}

FixedBinningHash::binned_vec FixedBinningHash::hash_data(vector<long> dat,
                                                    int n_bins,
                                                    long dummy,
                                                    int* max_capacity,
                                                    bool det_max_capacity,
                                                    bool all_data_per_bin) {
    BN_CTX* ctx = BN_CTX_new();
    auto allBins = init_bins(n_bins);

    int *max_cap0 = &(*max_capacity), *max_cap1 = &(*max_capacity);

    for(long element : dat) {
        if(all_data_per_bin) {
            insert(ctx, allBins, element, n_bins, max_cap0, det_max_capacity, 0);
            insert(ctx, allBins, element, n_bins, max_cap1, det_max_capacity, 1);
        }else {
            insert(ctx, allBins, element, n_bins, max_capacity, det_max_capacity,-1);
        }
    }

    // Debug longest chain
    for(int i=0;i<allBins.size();i++) {
        auto hashes = allBins.at(i);
        unsigned long max_len = 0;
        for(const auto &binlists : hashes ) {
            if(binlists.size() > max_len) {
                max_len = binlists.size();
            }
        }
        cout << "(Debug) Longest chain in hash " << i << " is " << max_len << "!" << endl;
    }

    binned_vec res;
    if(all_data_per_bin) {  // Different fill-up heights for bins
        fillBins(allBins, dummy, n_bins, max_cap0, max_cap1);
        res.bin0 = transpose_data(allBins.at(0), max_cap0);
        res.bin1 = transpose_data(allBins.at(1), max_cap1);
        max_capacity = (*max_cap0>*max_cap1? max_cap0: max_cap1);
    }else {     // Fill all bins up with to same height
        fillBins(allBins, dummy, n_bins, max_capacity, max_capacity);
        res.bin0 = transpose_data(allBins.at(0), max_capacity);
        res.bin1 = transpose_data(allBins.at(1), max_capacity);
    }

    return res;
}













