//
// Created by Nils Lukas on 15/04/19.
//

#ifndef FHE_FIXEDBINNINGHASH_H
#define FHE_FIXEDBINNINGHASH_H

#include "HashingStrategy.h"
#include <openssl/ossl_typ.h>
#include "openssl/bn.h"
#include <iostream>
#include <sstream>
#include <cmath>
#include <openssl/sha.h>
#include <vector>

using namespace std;

#define HASH_1_DIGEST_SIZE_BYTES 20
#define HASH_2_DIGEST_SIZE_BYTES 32
#define NUMBER_OF_HASHES 2

class FixedBinningHash : public HashingStrategy {
public:
    /** Inherited method to bin data elements across all bins */
    binned_vec hash_data(vector<long> dat, int n_bins, long dummy, int* max_capacity,
            bool det_max_capacity, bool all_data_per_bin) override;

    /** Initialize all bins **/
    vector<vector<vector<long>>> init_bins(int numberOfBins);

    /** Inserts an element into a bin.
        @param element: the item that is being inserted into a bin.
        @throws std::runtime_error: if a bin exceeds maximum capacity.  */
    void insert(BN_CTX* ctx, vector<vector<vector<long>>> &allBins, long element, int numberOfBins, int* max_cap,
            bool determine_cap, int preferred_bin=-1);

    /** Fills each slot in each table with a dummy element,
        until each slot reaches its maximum capacity.
        @param dummyElement: the integer used to represent the client's dummy element.  */
    void fillBins(vector<vector<vector<long>>> &allBins, long dummyElement, int numberOfBins, int* max_cap0, int* max_cap1);

    int hash1(BN_CTX* ctx, long element, int modulus) {
        if(element == 0) element = 1; //hack
        long arraySize = std::log10(element) + 2;
        char numberArray[arraySize];
        sprintf(numberArray, "%ld", element);

        unsigned char hash[HASH_1_DIGEST_SIZE_BYTES];
        SHA1((unsigned char*) numberArray, sizeof(numberArray) - 1, hash);
        char converted[2 * HASH_1_DIGEST_SIZE_BYTES + 1];
        for(int i = 0; i < HASH_1_DIGEST_SIZE_BYTES; i++) {
            sprintf(&converted[2 * i], "%02x", hash[i]);
        }
        BIGNUM *digestBN = NULL;
        BN_hex2bn(&digestBN, converted);
        BIGNUM *remainder;
        remainder = BN_CTX_get(ctx);
        BIGNUM *divisor = BN_CTX_get(ctx);
        std::ostringstream modulusStream;
        modulusStream << modulus;
        BN_dec2bn(&divisor, modulusStream.str().c_str());
        BN_mod(remainder, digestBN, divisor, ctx);
        char* result = BN_bn2dec(remainder);
        BN_free(digestBN);
        BN_free(divisor);
        BN_free(remainder);
        return atoi(result);
    }

    int hash2(BN_CTX* ctx, long element, int modulus) {
        if(element == 0) element = 1; // hack
        int digits = (int) std::log10(element) + 2;
        char numberArray[digits];
        sprintf(numberArray, "%ld", element);

        unsigned char hash[HASH_2_DIGEST_SIZE_BYTES];
        SHA256((unsigned char*) numberArray, sizeof(numberArray) - 1, hash);
        char converted[2 * HASH_2_DIGEST_SIZE_BYTES + 1];
        for(int i = 0; i < HASH_2_DIGEST_SIZE_BYTES; i++) {
            sprintf(&converted[2 * i], "%02x", hash[i]);
        }
        BIGNUM *digestBN = NULL;
        BN_hex2bn(&digestBN, converted);
        BIGNUM *remainder;
        remainder = BN_CTX_get(ctx);
        BIGNUM *divisor = BN_new();
        std::ostringstream modulusStream;
        modulusStream << modulus;
        BN_dec2bn(&divisor, modulusStream.str().c_str());
        BN_mod(remainder, digestBN, divisor, ctx);
        char* result = BN_bn2dec(remainder);

        BN_free(digestBN);
        BN_free(divisor);
        BN_free(remainder);
        return atoi(result);
    }
};

#endif //FHE_FIXEDBINNINGHASH_H
