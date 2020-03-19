//
// Created by Nils Lukas on 06/03/19.
//

#include <algorithm>
#include "PSI.h"

std::unique_ptr<FHEcontext> create_context(long p , long r , long d , long c , long k, long L, long s ,
                            long m , long pm , bool verbose) {
    // Find appropriate parameters based on our choices
    if (m <= 0) {
	// FindM(k, L, c, 2, 1, 0, chosen_m, true);
        m = FindM(k, L, c, p, d, s, pm);
        if(verbose) cout << "(Debug) m was set to " << m << endl;
    }
    std::unique_ptr<FHEcontext> context(new FHEcontext(m, p, r));
    buildModChain(*context, L, c);
    return context;
}

/** Helper method to turn a long to a binary string **/
string to_binary( long n, int bitlen )
{
    string binary = bitset<sizeof( long ) * 8>(n).to_string(); //to binary
    return binary.substr(sizeof( long ) * 8-bitlen, sizeof( long ) * 8);
}

vector<string> convert_to_bit_representation(vector<long> &data, int bitlen) {
    vector<string> res;
    for(long dat : data) {
        long dat_cyclic = (dat % ((long) pow(2, bitlen)));
        res.push_back(to_binary(dat_cyclic, bitlen));
    }
    return res;
}

vector<vector<long>> encode_data_p(vector<string> &bins, long n_slots, long no_match) {
    vector<vector<long>> res;
    if(bins.empty()) return res;
    string no_match_bitstr = to_binary(no_match, static_cast<int>(bins.at(0).size()));
    for (int i = 0; i < bins.at(0).size(); i++) {
        res.emplace_back();
        for(int j=0;j<bins.size();j++){
            int bit = (int) bins.at(j).at(i) - 48;
            res[i].push_back(bit);
        }
        // Fill up with no matches
        for(long j=res[i].size(); j<n_slots;j++) {
            res[i].push_back(no_match_bitstr.at(i));
        }
    }
    return res;
}

