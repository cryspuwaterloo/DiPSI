#include "PSIClient.h"
#include "PSI.h"

ClientRequest PSIClient::gen_psi_request(ExperimentParams &params, TmpFHEContext* tmpContext, ExperimentLogs& logs) {
        /// Read parameters given by user
        vector<long> cdat =params.client_input;
        long p = params.p_mod, r = params.r, d = params.d, c = params.c,
        k = params.k, L = params.L, s = params.s, m = params.m, pm = params.pm;
        HashingStrategy* strat = params.binning_strategy;
        int max_capacity = params.max_capacity;
        /////////////////////////////////

        cout << "(Client) Configuration: " << endl << params << endl;
        if(params.print_client_data) cout << "(Debug) Client Data Size: " << params.client_input.size() << endl;

        FHE_NTIMER_START(total);
        FHE_NTIMER_START(create_context);

        // Create a context and get the available slot count
        std::unique_ptr<FHEcontext> context = create_context(p, r, d, c, k, L, s, m, pm, params.debug_print_m);

        std::cout <<"Security level" << context->securityLevel() << endl;

        // Generate the public and secret key
        std::unique_ptr<FHESecKey> secKey(new FHESecKey(*context));
        auto pubKey = (FHEPubKey*) secKey.get();
        secKey->GenSecKey();
        // Compute key-switching matrices for rotations
        addSome1DMatrices(*secKey);
        // Compute batching parameter
        EncryptedArray ea(*context, context->alMod);
        long n_slots = pubKey->getContext().zMStar.getNSlots();

        logs.slot_count = n_slots;
        if( n_slots != params.s ) {
                cerr << "(Client) Could not accommodate desired slot size! Switched to " << n_slots << " instead!" << endl;
        }else {
                cout << "(Client) Slot count: " << n_slots << endl;
        }

        FHE_NTIMER_STOP(create_context);
        FHE_NTIMER_START(encrypt_data);
        FHE_NTIMER_START(binning_data);

        // First apply binning to the client data, returns two stashes of client size
        HashingStrategy::binned_vec bin_cdat = strat->hash_data(/*raw data*/cdat, params.n_client, /*dummy*/params.no_match,
                &max_capacity, /*determine_capacity*/false, /* all_data_per_bin */false);

        FHE_NTIMER_STOP(binning_data);

        vector<vector<long>> merged_list;
        merged_list.insert(merged_list.begin(), bin_cdat.bin0.begin(), bin_cdat.bin0.end());
        merged_list.insert(merged_list.end(), bin_cdat.bin1.begin(), bin_cdat.bin1.end());

        // For each level, extract m client elements, convert them to binary and encode them as one bit vector
        vector<HELibBitVec> cdat_vec_ctxt;
        for(auto & cdat_level_i : merged_list) {
                // In case batchsize < client_data_size, we need multiple ctxt per level
                for(int round=0; round<ceil(cdat_level_i.size()/(float) n_slots); round++) {
                        auto first = cdat_level_i.begin() + (round * n_slots), last = cdat_level_i.end();
                        if ((round + 1) * n_slots <= cdat_level_i.size()) {
                                last = cdat_level_i.begin() + ((round + 1) * n_slots);
                        }
                        vector<long> cdat_level_i_chunk(first, last);
                        vector<string> cdat_bits = convert_to_bit_representation(cdat_level_i_chunk, params.bitlen);
                        vector<vector<long>> cdat_enc = encode_data_p(cdat_bits, n_slots, params.no_match);
                        cdat_vec_ctxt.push_back(*(new HELibBitVec(&ea, pubKey))->encrypt(cdat_enc));
                }
        }

        FHE_NTIMER_STOP(encrypt_data);
        FHE_NTIMER_START(serialize_data);

        /// Serialize client context and client elements to send them off
        stringstream ostream;
        writeContextBaseBinary(ostream, *context);
        writeContextBinary(ostream, *context);
        writePubKeyBinary(ostream, *pubKey);
        //writeSecKeyBinary(ostream, *secKey);

        long size_context = ostream.str().size();
        double client_context_size = size_context/1000000.;

        logs.client_context_size = client_context_size;
        cout << "(Client) Serialized client context to: " << client_context_size << " MB" << endl;

        // Serialize client data
        for(HELibBitVec& vec : cdat_vec_ctxt) {
                HELibBitVec::writeHELibBitVec(ostream, vec);
        }

        FHE_NTIMER_STOP(serialize_data);
        FHE_NTIMER_STOP(total);

        // Log the size in MB of the whole serialization
        long size_data = ostream.str().size();
        double client_ctxt_size = (size_data-size_context)/1000000.;
        logs.client_ctxt_size = client_ctxt_size;
        cout << "(Client) Serialized " << cdat_vec_ctxt.size() << " data elements to: " << client_ctxt_size << " MB" << endl;

        // Fill the temporary context so we can handle the client response later
        tmpContext->pubKey = pubKey;
        tmpContext->context = std::unique_ptr<FHEcontext>(move(context));
        tmpContext->secKey =  std::unique_ptr<FHESecKey>(move(secKey));

        // Evaluate all the timer
        double t1 = getTimerByName("create_context")->getTime();
        double t2 = getTimerByName("encrypt_data")->getTime();
        double t3 = getTimerByName("binning_data")->getTime();
        double t4 = getTimerByName("serialize_data")->getTime();
        double t5 = getTimerByName("total")->getTime();

        logs.client_create_context_time = t1; logs.client_encrypt_data_time = t2;
        logs.client_binning_data_time = t3; logs.client_serialize_data_time = t4;
        logs.client_total_time = t5;
        cout << fixed << setprecision(2) <<
             "(Client) Timing Summary:   " << endl <<
             "   Create Context          " << t1 << "s" << endl <<
             "   Encrypt Data:           " << t2 << "s" << endl <<
             "   Binning Data            " << t3 << "s" << endl <<
             "   Serialize Data          " << t4 << "s" << endl <<
             "   Total:                  " << t5 << "s" << endl;

        long client_data_size = cdat_vec_ctxt.size();
        return {
                .n_client_elems = client_data_size,
                .helibvec_depth = params.bitlen,
                .data = ostream.str()
        };
}

void PSIClient::process_psi_response(ServerResponse &res, ExperimentParams &p, TmpFHEContext* tmpContext,
        ExperimentLogs& logs) {
        /////// Raw Data /////
        int n_items = res.n_items;
        stringstream istream(res.data);
        //////////////////////

        cout << "(Client) Received data from server: " << istream.str().size()/1000000. << " MB" << endl;

        std::unique_ptr<FHEcontext> context(move(tmpContext->context));
        std::unique_ptr<FHESecKey> secKey(move(tmpContext->secKey));
        FHEPubKey* pubKey = tmpContext->pubKey;

        EncryptedArray ea(*context, context->alMod);

        if(p.mode == INTERSECTION) {
                // Get client data
                HashingStrategy::binned_vec client_data = p.binning_strategy->hash_data(/*raw data*/p.client_input, p.n_client, /*dummy*/p.no_match,
                        &p.max_capacity, /*determine_capacity*/false, /* all_data_per_bin */false);
                cout << "(Client) Matches: [" << endl;
                for(int i=0;i<n_items;i++) {
                        Ctxt match(*pubKey);
                        istream >> match;
                        vector<long> y;
                        ea.decrypt(match, *secKey, y);

                        unsigned long offset = y.size()*(i % (int) ceil(p.n_client/(float) y.size()));
                        int real_i = (i / (int) ceil(p.n_client/(float) y.size()));

                        cout << "       Matches on level " << real_i << ": ";

                        for(int j=0;j<y.size();j++) {
                                if(j+offset >= client_data.bin0.at(0).size()) {
                                        break;
                                }
                                if(y[j] >= 1) {
                                        long matched_elem;
                                        if (real_i < client_data.bin0.size()) {     // hash 0
                                                matched_elem = client_data.bin0.at(real_i).at(j+offset);
                                        } else {         // hash1
                                                matched_elem = client_data.bin1.at(real_i-client_data.bin0.size()).at(j+offset);
                                        }

                                        logs.client_matches.push_back(matched_elem);
                                        cout << matched_elem << " ";

                                }
                        }
                        cout << endl;
                }
                cout << "]" << endl;
        }else if(p.mode == CARDINALITY) {
                Ctxt cardinality(*pubKey);
                istream >> cardinality;
                vector<long> y;
                ea.decrypt(cardinality, *secKey, y);
                long sum = 0;
                for(long dat : y) {
                        sum = (sum + dat);
                }
                sum = sum % pubKey->getPtxtSpace();
                // Decode negative numbers
                long real_sum = ((sum < pubKey->getPtxtSpace()/2) ? sum  : sum - pubKey->getPtxtSpace());
                cout << "(Client) Matches cardinality: " << real_sum << endl;
                logs.client_cardinality = real_sum;
        }
        cout << "Done!" << endl;
}