
#ifndef PSI_HEADER_H
#define PSI_HEADER_H

#include <NTL/ZZ.h>
#include "FHE.h"
#include "EncryptedArray.h"
#include <NTL/BasicThreadPool.h>
#include <bitset>
#include <chrono>
#include <math.h>       /* pow */

#include "HELibBitVec.h"

NTL_CLIENT

using namespace std;

    /**
     * Creates a context for the HE
     *
     * @param p Plaintext modulus
     * @param r Lifting polynomials used for bootstrapping
     * @param d Degree of field extension used for finding m
     * @param c Number of columns in the key-switching matrices
     * @param k Security parameter
     * @param L Bits in the modulus chain
     * @param s Minimum number of slots [default=0]
     */
    std::unique_ptr<FHEcontext> create_context(long p = 40961, long r = 1, long d = 1, long c = 2, long k = 128, long L = 300, long s = 2048,
                                long m = -1, long pm = 4096, bool verbose=false);

    /**
     * Helper function to convert a vector of longs to its string bit representation
     */
    vector<string> convert_to_bit_representation(vector<long> &data, int bitlen);

    /**
     * PArallel encoding of one batch of client data (all elements bitwise in one batch)
     * Creates 1 ciphertext for floor(batchsize/bitlength) ciphertexts
     */
    vector<vector<long>> encode_data_p(vector<string> &bins, long n_slots, long no_match);

#endif

