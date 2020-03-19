//
// Created by Nils Lukas on 04/03/19.
//

#ifndef FHE_HELIBBITVEC_H
#define FHE_HELIBBITVEC_H

#include <NTL/ZZ.h>
#include "FHE.h"
#include "EncryptedArray.h"
#include <NTL/BasicThreadPool.h>
#include "params.h"

NTL_CLIENT

using namespace std;

class HELibBitVec {
    vector<Ctxt> ctxt_data;                // The encrypted data
    const EncryptedArray* ea;              // Encrypted array instance to encrypt arrays

public:
    const FHEPubKey* pubKey;                // Public encryption key
    enum he_ops { MULT, ADD, XOR, AND };    // Available operation between bit vectors

    HELibBitVec(const EncryptedArray* ea, const FHEPubKey* publicKey) {
        this->ea = ea;
        this->pubKey = publicKey;
    }

    /** Get number of elements encrypted into vector */
    long size();

    /** Add new ciphertext **/
    void push_back(Ctxt ctxt) {
        ctxt_data.push_back(ctxt);
    }

    /** Retrieve ciphertext at position **/
    Ctxt at(int pos) {
        return ctxt_data.at(pos);
    }

    /** Encrypt your input and load it into the bit vector. Can be plaintext if is_dummy is true.*/
    HELibBitVec* encrypt(vector<vector<long>> &plain_data, bool is_dummy=false);

    /** Recursively multiplies data entries in a tree structure for best noise performance */
    Ctxt tree_multiply(vector<Ctxt>& data, int start, int end);

    /** Bootstrap tree_multiply */
    Ctxt tree_multiply(vector<Ctxt>& data) {
        return tree_multiply(data, 0, (int) ctxt_data.size()-1);
    }

    /** Applies an operator to all data elements of this class */
    HELibBitVec apply_op(const HELibBitVec&, he_ops);

    /** Apply a function along one dimension and reduce the bitvector to a single ctxt */
    Ctxt reduce(he_ops);

    /** Negation */
    HELibBitVec operator-() const {
        HELibBitVec res(ea, pubKey);
        for(int i=0;i<ctxt_data.size();i++) {
            Ctxt negate = ctxt_data[i];
            negate.negate();
            negate.addConstant(to_ZZX(1));
            res.push_back(negate);
        }
        return res;
    }

    /** ADD **/
    HELibBitVec operator+(const HELibBitVec &rhs) {
        return apply_op(rhs, ADD);
    }

    /** MULT **/
    HELibBitVec operator*(const HELibBitVec &rhs){
        return apply_op(rhs, MULT);
    }

    /** AND **/
    HELibBitVec operator&(const HELibBitVec &rhs){
        return apply_op(rhs, AND);
    }

    /** XOR **/
    HELibBitVec operator^(const HELibBitVec &rhs){
        return apply_op(rhs, XOR);
    }

    /** Serialize to stream **/
    static void writeHELibBitVec(ostream&, const HELibBitVec&);

    /** Deserialize from stream **/
    static void readHELibBitVec(istream& istream, HELibBitVec& vec, long read_n);

};

#endif //FHE_CIPHERTEXTS_H
