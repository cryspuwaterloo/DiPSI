#include "HELibBitVec.h"

void HELibBitVec::writeHELibBitVec(ostream& ostream, const HELibBitVec& vec) {
    for(const Ctxt &ctxt : vec.ctxt_data) {
         ostream << ctxt << endl;
    }
}

void HELibBitVec::readHELibBitVec(istream& istream, HELibBitVec& vec, long read_n) {
    for (long i = 0; i < read_n; i++) {
        Ctxt ctxt(*vec.pubKey);
        istream >> ctxt;
        vec.ctxt_data.push_back(ctxt);
    }
}

/** Class implementation **/

HELibBitVec* HELibBitVec::encrypt(vector<vector<long>> &plain_data, bool is_dummy) {
    ctxt_data.clear();
    for(vector<long>& elem : plain_data) {
        Ctxt ctxt(*pubKey);
        if(is_dummy) {
            // Allow fake encrypting plaintexts
            PlaintextArray pta(*ea);
            ZZX ppp0;
            encode(*ea, pta, elem);
            ea->encode(ppp0, pta);
            ctxt.DummyEncrypt(ppp0);
        }else {
            ea->encrypt(ctxt, *pubKey, elem);
        }
        ctxt_data.push_back(ctxt);
    }
    return this;
}

long HELibBitVec::size(){
    return ctxt_data.size();
}

Ctxt HELibBitVec::tree_multiply(vector<Ctxt>& data, int start, int end){
    int len = end-start;
    if(len > 1) {
        // Burst
        Ctxt a = tree_multiply(data, start, (start+end)/2);
        Ctxt b = tree_multiply(data, (start+end+1)/2, end);
        a *= b;
        return a;
    }else if(len == 1){
        // Multiply
        return data[start] *= data[end];
    }else if(len == 0) {
        // In case of odd, return singular value
        cout << " Odd case .. " << endl;
        return data[start];
    }else{
        throw "Empty list provided!";
    }
}

Ctxt HELibBitVec::reduce(he_ops op) {
    Ctxt sum(*pubKey);
    if(op == AND) {
        sum = tree_multiply(ctxt_data); // Recursive approach takes 3.99 seconds vs 4.69s for iterative approach
    }else {
        throw "Non-valid HE operation";
    }

    return sum;
}

HELibBitVec HELibBitVec::apply_op(const HELibBitVec &rhs, he_ops op){
    HELibBitVec res(ea, pubKey);

    for(int i=0;i<ctxt_data.size();i++) {
        Ctxt ctxt_i = ctxt_data[i];

        if(op == ADD) {
            ctxt_i += rhs.ctxt_data[i];
        }else if(op == MULT || op == AND){
            ctxt_i *= rhs.ctxt_data[i];
        }else if(op == XOR) {
            // xor = -(a+b-1)^2+1 = (a+b)-2ab
            // (a+b)
            ctxt_i += rhs.ctxt_data[i];
            // (a+b-1)
            ctxt_i.addConstant(to_ZZX(-1));
            // (a+b-1)^2
            ctxt_i.square();
            // -(a+b-1)^2
            ctxt_i.negate();
            // -(a+b-1)^2+1
            ctxt_i.addConstant(to_ZZX(1));
        }else {
            throw "Non-valid HE operation";
        }
        res.push_back(ctxt_i);
    }
    return res;
}







