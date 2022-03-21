#include "PSIServer.h"
#include "PSI.h"

void print_percentage(int percentage) {
    cout << "[";
    for(int i=0;i<100; i++) {
        if (i < percentage) {
            cout << "=";
        }else if( i == (int) percentage) {
            cout << ">";
        }else {
            cout << " ";
        }
    }
    cout << "] " << percentage << "%" << endl;
}

void print_fixed_size_text(string str, int size) {
    int sz = str.size();
    cout << str;
    for(int i=sz; i<=size; i++) {
        cout << " ";
    }
}

void print_progress(int ctr, int hi, int j, int hash_nr, int max_ctr, int max_hi, int max_j, int max_hash, clock_t begin) {
    int elapsed_secs = static_cast<int>((clock() - begin) / CLOCKS_PER_SEC);
    double nominator =  ctr +
                        hi*max_ctr+
                        j*max_hi*max_ctr+
                        hash_nr*max_j*max_hi*max_ctr,
            denominator = max_ctr*max_hash*max_j*max_hi;
    int percentage = static_cast<int>(100*(nominator/denominator));

    ostringstream strs;
    strs << "[" << (elapsed_secs) << "s] Matching: ";

    print_fixed_size_text(strs.str(), 20);
    print_percentage((int) percentage);
}

vector<long> get_chunk(vector<long> &server_batch, int n_slots, int r) {
    auto first = server_batch.begin() + (r * n_slots), last = server_batch.end();
    if ((r + 1) * n_slots <= server_batch.size()) {
        last = server_batch.begin() + ((r + 1) * n_slots);
    }
    vector<long> server_chunk(first, last);
    return server_chunk;
}

vector<HELibBitVec> load_client_bin(int hash_nr, vector<HELibBitVec>& cdat) {
    vector<HELibBitVec> client_bin_data;
    int start, end;

    if(hash_nr==0) {
        start=0, end=cdat.size()/2;
    }else {
        start=cdat.size()/2, end=cdat.size();
    }
    for(int i=start;i<end;i++) {
        client_bin_data.push_back(cdat.at(i));
    }
    return client_bin_data;
}

void apply_diff_privacy(vector<long>& chunk, vector<PSIServer::RandResponse> fake_x, long no_match) {
    for (int j = 0; j < chunk.size(); j++) {
        PSIServer::RandResponse response = fake_x.at(j);
        if (response != PSIServer::KEEP) {
            chunk.at(j) = no_match;
        }
    }
}

ServerResponse PSIServer::perform_psi(ClientRequest &req, const ExperimentParams& params, ExperimentLogs &logs) {
    FHE_NTIMER_START(perform_psi_func);
    ////// Client Raw data////////
    long n_cdat = req.n_client_elems;                               // How many client data samples are encoded
    long helibvec_depth = req.helibvec_depth;                       // Depth of ciphertexts for HELibBitVec
    stringstream istream(req.data);                                 // The stream encoding all client data
    //////////////////////////////

    // Compute the total data size received from client in MB
    double server_received_data = istream.str().size()/1000000.;
    logs.server_data_received_size = server_received_data;
    cout << "(Server) Received data from client: " <<  server_received_data << " MB" << endl;

    FHE_NTIMER_START(deserialize);

    std::unique_ptr<FHEcontext> context = buildContextFromBinary(istream);
    readContextBinary(istream, *context);
    std::unique_ptr<FHESecKey> secKey(new FHESecKey(*context));
    FHEPubKey* pubKey = (FHEPubKey*) secKey.get();

    readPubKeyBinary(istream, *pubKey);
    //readSecKeyBinary(istream, *secKey);

    const EncryptedArray& ea(*context);

    // In the parallel case we just have to read all the ciphertexts bit by bit and load them into the vector
    // One HELibBitVec now represents at most batchsize many client elements
    vector<HELibBitVec> cdat;
    for(int i=0;i<n_cdat;i++){
        auto vec_r = new HELibBitVec(&ea, pubKey);
        HELibBitVec::readHELibBitVec(istream, *vec_r, helibvec_depth);
        cdat.push_back(*vec_r);
    }

    FHE_NTIMER_STOP(deserialize);

    ///// Extract encryption parameters  /////
    long n_slots = ea.size();                           // Slot count for batching
    long plain_mod = pubKey->getPtxtSpace();            // Plaintext modulus
    long server_elems_per_ctxt = n_slots;               // Number of server elements that fit in one batch

    Mode mode = params.mode;                            // Cardinality or Intersection
    vector<long> sdat_in = params.server_input;         // Input server data
    HashingStrategy* strat = params.binning_strategy;   // Binning strategy
    //////////////////////////////////////////

    random_device rd;                           // Source of randomness
    vector<Ctxt> matches;                       // Result vector that will be filled with all matches

    if (params.debug_print_batchsize){
        logs.server_elems_per_ctxt = server_elems_per_ctxt;
        cout << "(Debug) Server elements per ctxt: " << server_elems_per_ctxt << endl;
    }

    FHE_NTIMER_START(differential_privacy);
    /* Differential privacy strategy for intersection: randomized response
           Generate random bits p and q. If p, we send the exact match bit.
           Otherwise, we generate fake input with a guaranteed match (q) or no match (!q). */
    vector<vector<RandResponse>> fake_x_vec; // Store results per client element
    if (mode == INTERSECTION && params.apply_diff_privacy) {
        for(int i=0;i<cdat.size();i++) {
            vector<RandResponse> fake_x;    // RandResponse per level
            bernoulli_distribution pd(params.p), qd(params.q);

            for(int j=0;j<params.n_client;j++) {
                bool p_sample = pd(rd), q_sample = qd(rd);
                if(p_sample) {
                    fake_x.push_back(KEEP);
                }else {
                    fake_x.push_back((q_sample ? MATCH : NO_MATCH));
                }
                logs.server_pq_samples.emplace_back(pair<long, long>(p_sample, q_sample));
            }
            if(params.print_pq_samples) {
                cout << "(Debug) Randomized Response Round " << i << ": "  << fake_x <<  endl;
            }
            fake_x_vec.push_back(fake_x);
        }
    }
    FHE_NTIMER_STOP(differential_privacy);

    /* Now, we perform the actual PSI
     * 1.) Bin server elements twice (for hash1 and hash2) into batchsize many bins
     * 2.) Slice server elements by their level and encode them bitwise
     *     => We have exactly one HELibBitVec per level
     *     => Remember to use randomized response to replace server elements with no_matches
     * 3.) Compare each server plaintext with all client elements from that hash
     * 4.) Add up all results per client element and add 1 to all forced matches
     * 5.) If mode == CARDINALITY apply differential privacy  */

    FHE_NTIMER_START(binning_data);
    // Apply binning to all server elements
    int* max_cap = new int(0);
    HashingStrategy::binned_vec bin_sdat = strat->hash_data(/*raw data*/sdat_in, params.n_client, /*dummy*/params.no_match2,
            max_cap, /*determine_capacity*/true, /* all_data_per_bin */true);
    FHE_NTIMER_STOP(binning_data);

    if(params.print_number_of_server_rounds) {
        logs.server_rounds = *max_cap ;
        cout <<"(Server) Longest binning chain has capacity: " << *max_cap << endl;
    }

    // Take one batch of client elements, take all server elements in the same hash, replace server elements with
    // randomized response, do all comparisons, finally add up and store result
    clock_t begin = clock();
    int horizontal_slots = (int) ceil(params.n_client/(double) n_slots);
    FHE_NTIMER_START(do_matching);
    for(int hash_nr=0;hash_nr<2;hash_nr++) {                                        // For each hash function
        auto server_bin = hash_nr == 0? bin_sdat.bin0: bin_sdat.bin1;
        vector<HELibBitVec> client_bin = load_client_bin(hash_nr, cdat);
        for (int j=0;j<server_bin.size();j++) {                                     // For each server element
            vector<long> server_batch = server_bin.at(j);
            for (int hi = 0; hi < horizontal_slots; hi++) {                         // For each horizontal slot
                int ctr = 0;
                for (int vi = (hi%horizontal_slots); vi < client_bin.size(); vi+=horizontal_slots) {   // For each client element from bin
                    if(params.debug_server_matching_progress) {
                        print_progress(++ctr, hi, j, hash_nr, (client_bin.size()/horizontal_slots), horizontal_slots,
                                       server_bin.size(), 2, begin);
                    }
                    // Load the correct horizontal chunk from the server batch
                    vector<long> chunk = get_chunk(server_batch, n_slots, hi);
                    if (mode == INTERSECTION && params.apply_diff_privacy) {
                        apply_diff_privacy(chunk, fake_x_vec.at(vi), params.no_match);
                    }
                    // cout << "client index: " << vi  << " hash: " << hash_nr << " j: " << j << " hi: " << hi << " vi: " << vi << " Chunk: " << chunk << endl;
                    // Encode and encrypt server data chunk as plaintext
                    HELibBitVec sdat_ptxt(&ea, pubKey);
                    vector<string> sdat_raw = convert_to_bit_representation(chunk, params.bitlen);
                    vector<vector<long>> sdat_enc = encode_data_p(sdat_raw, n_slots, params.no_match2);
                    sdat_ptxt = *(new HELibBitVec(&ea, pubKey))->encrypt(sdat_enc, true);

                    Ctxt match = (client_bin.at(vi) ^ -sdat_ptxt).reduce(HELibBitVec::AND);

                    int match_index = (hash_nr*(client_bin.size())) + vi;
                    if (matches.size() <= match_index) {
                        matches.push_back(match);
                    } else {
                        // Add matches across rounds
                        matches.at(match_index) += match;
                    }
                }
            }
        }
    }
    FHE_NTIMER_STOP(do_matching);

    // Sample Laplace noise and encode it as a plaintext vector so that the sum of the noise cancels out to 0
    FHE_NTIMER_START(do_cardinality);
    Ctxt cardinality_ctxt(*pubKey);
    FHE_NTIMER_START(diff_privacy_cardinality);
    if(mode == CARDINALITY) {
        // Add up all matches and add the noise vector onto it
        cardinality_ctxt = matches[0];
        for(int i=1;i<matches.size();i++) {
            cardinality_ctxt += matches[i];
        }
        if (params.apply_diff_privacy) {
            long noise_sum = 0;

            // Create a uniform noise vector that adds up to 0
            vector<long> noise;
            for (int i = 0; i < n_slots - 1; i++) {
                long sample = rd() % plain_mod;
                noise.push_back(sample);
                noise_sum += sample;
            }
            noise.push_back(-noise_sum % plain_mod);

            // Generate Laplace noise for sample.
            // We take an exponential sample and add it at a random position
            exponential_distribution<double> d(params.lambda);
            long sample = (long) floor(d(rd));
            if (rd() % 2) {
                sample = -sample;
            }
            if(params.debug_cardinality_dp_sample){
                logs.server_cardinality_dp_sample = sample;
                cout << "(Debug) Choose cardinality dp sample: " << sample << endl;
            }

            // Add sample to random position
            int rnd_index = rd() % n_slots;
            noise[rnd_index] = (noise[rnd_index] + sample) % plain_mod;

            // Encode the noise vector
            ZZX noise_ptxt;
            ea.encode(noise_ptxt, noise);
            cardinality_ctxt.addConstant(noise_ptxt);
        }
    }

    FHE_NTIMER_STOP(diff_privacy_cardinality);  // Differential privacy for cardinality
    FHE_NTIMER_STOP(do_cardinality);    // Time for cardinality
    FHE_NTIMER_STOP(perform_psi_func);  // Total time

    double t2 = getTimerByName("deserialize")->getTime();
    double t3 = getTimerByName("do_matching")->getTime();
    double t4 = getTimerByName("differential_privacy")->getTime();
    double t5 = getTimerByName("do_cardinality")->getTime();
    double t6 = getTimerByName("diff_privacy_cardinality")->getTime();
    double t7 = getTimerByName("perform_psi_func")->getTime();
    double t8 = getTimerByName("binning_data")->getTime();

    logs.server_deserialize_time = t2; logs.server_matching_time = t3; logs.server_dp_for_intersection_time = t4;
    logs.server_cardinality_time = t5; logs.server_dp_for_cardinality_time = t6; logs.server_total_time_server = t7;
    logs.server_binning_time = t8;
    cout << fixed << setprecision(2) <<
         "(Server) Timing Summary:   " << endl <<
         "   Deserialize:            " << t2 << "s" << endl <<
         "   Binning Data:           " << t8 << "s" << endl <<
         "   Matching:               " << t3 << "s" << endl <<
         "   DP for Intersection     " << t4 << "s" << endl <<
         "   Cardinality:            " << t5 << "s" << endl <<
         "   DP for Cardinality      " << t6 << "s" << endl <<
         "   Total:                  " << t7 << "s" << endl;

    // Fill the remaining noises
    vector<long> match_noises; bool error_appeared = false;
    for (Ctxt &match : matches) {
        long noise_left = match.bitCapacity();
        if(!error_appeared && noise_left >= 100) {
            cerr << "[WARNING] Ciphertexts still have a large noise capacity (" << noise_left << ") as leftover!"" Consider lowering L!" << endl;
            error_appeared = true;
        }
        match_noises.push_back(noise_left);
    }

    // Serialize data
    stringstream ostream;
    if(mode == INTERSECTION) {
        for (Ctxt &match : matches) {
            ostream << match;
        }
    }else if(mode == CARDINALITY) {
        ostream << cardinality_ctxt;
    }

    double response_size_server = ostream.str().size()/1000000.;

    logs.server_bit_capacity_matches = match_noises; logs.server_bit_capacity_cardin = cardinality_ctxt.bitCapacity();
    logs.server_response_size = response_size_server;
    cout << fixed << setprecision(2) <<
         "(Server) PSI Summary:      " << endl <<
         "   Bit capacity (matches): " << match_noises << endl <<
         "   Bit capacity (cardin):  " << cardinality_ctxt.bitCapacity() << endl <<
         "   Response size:          " << response_size_server << " MB " << endl;

    return {
            .n_items = (mode == INTERSECTION ? (int) matches.size() : 1),
            .data = ostream.str()
    };
}
