#include "Servers.h"
#include "process_noise.h"

// Servers::Servers(int server_size, int data_size, string known_domain_dir)
// {
//     gamal_generate_keys(coll_key);
//     for (int i = 0; i < server_size; i++)
//     {
//         Server server(data_size, known_domain_dir);
//         server_vect.push_back(server);
//     }

    

//     Servers::server_size = server_size;
//     Servers::data_size = data_size;
    

//     s_myPIR_enc = new gamal_ciphertext_t[data_size];
//     s_plain_track_list = new int[data_size];
// }

Servers::Servers(int server_size, int data_size, string known_domain_dir, int scale_up_factor)
{
    gamal_generate_keys(coll_key);//not used
    gamal_generate_keys(coll_key_PV_phase); //gen collective key pairs for PV phase
    gamal_generate_keys(coll_key); //gen collective key pairs for Query phase

    for (int i = 0; i < server_size; i++)
    {
        Server server(data_size, known_domain_dir); // initiate all servers with their key pair
        server_vect.push_back(server);
    }

    

    Servers::server_size = server_size;
    Servers::data_size = data_size;
    Servers::domain_size = data_size*scale_up_factor; //aN

    s_myPIR_enc = new gamal_ciphertext_t[domain_size];
    s_plain_track_list = new int[domain_size];
}

void Servers::createPVsamplingVector(ENC_Stack &pre_enc_stack)
{
    int enc_types[] = {1, 0};
    int up_freq = (int)(1 / pv_ratio - 1);
    int freq[] = {1, up_freq};
    // int pv_ratio = 50; // V = 2% of n

    pir_gen(s_plain_track_list, enc_types, freq, data_size, pv_ratio); // function that server place 1 or 0 randomly

    //========== Encrypt the vector =============== //
    int plain1 = 1, plain0 = 0;

    for (int i = 0; i < data_size; i++)
    {
        if (s_plain_track_list[i] == 1)
        {
            pre_enc_stack.pop_E1(s_myPIR_enc[i]);
        }
        else
        {
            pre_enc_stack.pop_E0(s_myPIR_enc[i]);
        }
    }
}

void Servers::createPVsamplingVector_size_N_plus_1(ENC_Stack &pre_enc_stack)
{
    int enc_types[] = {1, 0};
    int up_freq = (int)(1 / pv_ratio - 1);
    int freq[] = {1, up_freq};
    // int pv_ratio = 50; // V = 2% of n

    pir_gen(s_plain_track_list, enc_types, freq, data_size, pv_ratio); // function that server place 1 or 0 randomly

    //========== Encrypt the vector =============== //
    int plain1 = 1, plain0 = 0;
    //among N elements: V elements of enc(1) and N-V elements of enc(0)
    for (int i = 0; i < data_size; i++)
    {
        if (s_plain_track_list[i] == 1)
        {
            pre_enc_stack.pop_E1(s_myPIR_enc[i]);
        }
        else
        {
            pre_enc_stack.pop_E0(s_myPIR_enc[i]);
        }
    }

    pre_enc_stack.pop_E0(s_myPIR_enc[data_size]); //element (N+1)th = 0

    // for (int i = data_size; i < domain_size; i++)
    // {
       
    //     pre_enc_stack.pop_E0(s_myPIR_enc[i]);
    
    // }
}



void Servers::generateCollKey_projectBased(gamal_key_t collective_key, bool coll_key_phase)
{
    int server_size = server_vect.size();
    EC_POINT **p_key_list = new EC_POINT *[server_size];
    // BIGNUM **secret_key_list = new BIGNUM *[server_size]; //added 21 Jan by Tham: for re-encryption
   
    if (coll_key_phase) //=1: gen key for PV phase
    {
        // cout<<"test Ok"<<endl;
        for (int i = 0; i < server_size; i++)
        {
            
            p_key_list[i] = server_vect[i].key_PV_phase->Y;
            // p_key_list[i] = server_vect[i].key->Y;
            // cout<<"test gen keyOk"<<endl;
            // secret_key_list[i] = server_vect[i].key->secret; //added 21 Jan by Tham: for re-encryption
        }

        gamal_collective_publickey_gen(coll_key_PV_phase, p_key_list, server_size);
    }
    else
    {
       
        for (int i = 0; i < server_size; i++)
        {
            p_key_list[i] = server_vect[i].key_query_phase->Y;
            // p_key_list[i] = server_vect[i].key->Y;
            // secret_key_list[i] = server_vect[i].key->secret; //added 21 Jan by Tham: for re-encryption
        }

        gamal_collective_publickey_gen(coll_key, p_key_list, server_size);
    }
    
    // cout<<"test collective gen key Ok"<<endl;
    // gamal_collective_publickey_gen(collective_key, p_key_list, server_size);
    // cout<<"test collective gen key Ok"<<endl;
}


void Servers::generateCollKey(gamal_key_t collective_key)
{
    int server_size = server_vect.size();
    EC_POINT **p_key_list = new EC_POINT *[server_size];
    // BIGNUM **secret_key_list = new BIGNUM *[server_size]; //added 21 Jan by Tham: for re-encryption
   
   
        for (int i = 0; i < server_size; i++)
        {
            
            p_key_list[i] = server_vect[i].key->Y;
            // p_key_list[i] = server_vect[i].key->Y;
            // cout<<"test gen keyOk"<<endl;
            // secret_key_list[i] = server_vect[i].key->secret; //added 21 Jan by Tham: for re-encryption
        }

        gamal_collective_publickey_gen(coll_key, p_key_list, server_size);
   
}

dig_t Servers ::_fusionDecrypt_projectBased(gamal_ciphertext_t ciphertext, bsgs_table_t table, int serverId, bool key_for_decrypt)
{
    // cout<<"decryption begin "<<endl;
    int server_size = server_vect.size();
    // cout<<"server_size = "<<server_size <<endl;
    gamal_key_t key_follow[server_size - 1];

    int counter = 0;
    dig_t res;
    gamal_ciphertext_t ciphertext_update;

    if (key_for_decrypt) //key_for_decrypt == 1: use key for PV to decrypt
    {
        
        for (int i = 0; i < server_size; i++)
        {
            
            if (i != serverId)
            {
                key_follow[counter]->Y = server_vect[i].key_PV_phase->Y;
                key_follow[counter]->secret = server_vect[i].key_PV_phase->secret;
                key_follow[counter]->is_public = server_vect[i].key_PV_phase->is_public;
                counter++;
            }
            
        }
    
        gamal_fusion_decrypt(&res, server_size, server_vect[serverId].key_PV_phase, key_follow, ciphertext_update, ciphertext, table);
    }
    else // 0 for using key for query phase
    {
        for (int i = 0; i < server_size; i++)
        {
            if (i != serverId)
            {
                key_follow[counter]->Y = server_vect[i].key_query_phase->Y;
                key_follow[counter]->secret = server_vect[i].key_query_phase->secret;
                key_follow[counter]->is_public = server_vect[i].key_query_phase->is_public;
                counter++;
            }
        }
    
        gamal_fusion_decrypt(&res, server_size, server_vect[serverId].key_query_phase, key_follow, ciphertext_update, ciphertext, table);
    }
    
    

    // cout<<"decrypted answer = "<<res<<endl;

    return res;
}


dig_t Servers ::_fusionDecrypt(gamal_ciphertext_t ciphertext, bsgs_table_t table, int serverId)
{
    // cout<<"decryption begin "<<endl;
    int server_size = server_vect.size();
    // cout<<"server_size = "<<server_size <<endl;
    gamal_key_t key_follow[server_size - 1];

    int counter = 0;
    dig_t res;
    gamal_ciphertext_t ciphertext_update;

   
        
    for (int i = 0; i < server_size; i++)
    {
        
        if (i != serverId)
        {
            key_follow[counter]->Y = server_vect[i].key->Y;
            key_follow[counter]->secret = server_vect[i].key->secret;
            key_follow[counter]->is_public = server_vect[i].key->is_public;
            counter++;
        }
        
    }

    gamal_fusion_decrypt(&res, server_size, server_vect[serverId].key, key_follow, ciphertext_update, ciphertext, table);



    // cout<<"decrypted answer = "<<res<<endl;

    return res;
}

void Servers::fusionDecrypt(ENC_DOMAIN_MAP enc_domain_map, bsgs_table_t table)
{
    int server_size = server_vect.size();
    gamal_key_t key_follow[server_size - 1];
    for (int i = 0; i < server_size - 1; i++)
    {
        key_follow[i]->Y = server_vect[i + 1].key->Y;
        key_follow[i]->secret = server_vect[i + 1].key->secret;
        key_follow[i]->is_public = server_vect[i + 1].key->is_public;
    }

    dig_t res;
    gamal_ciphertext_t ciphertext_update;
    int count = 0;

    for (ENC_DOMAIN_MAP::iterator itr = enc_domain_map.begin(); itr != enc_domain_map.end(); ++itr)
    {
        gamal_fusion_decrypt(&res, server_size, server_vect[0].key, key_follow, ciphertext_update, itr->second, table);
        if (res > 0)
        {
            count += res;
        }
    }

    // cout << "Total domain encrypted as 1: " << count << endl;
}


void Servers::decryptPV(ENC_DOMAIN_MAP unPermutePV, bsgs_table_t table, int serverId)
{
    // cout<<"decryptPV: start"<<endl;
    for(ENC_DOMAIN_MAP::iterator itr = unPermutePV.begin(); itr != unPermutePV.end(); ++itr)
    {
        // itr->second = flag[ii];
		// ii++;
		cout << '\t' << itr->first.first
			<< "\t" << itr->first.second<< '\t'; 	 
        
		dig_t position_selected_decrypt = Servers::_fusionDecrypt(itr->second, table, serverId);
		cout<< "resPV= " <<position_selected_decrypt<<'\n';
    }

    // cout<<"decryptPV: end"<<endl;    
}

/*
 * to verify the PV
 * eta is the probability s.t the honest participant will pass the test at eta
 * higher eta is, smaller r0 is
 * 
 */
bool Servers::verifyingPV(ENC_DOMAIN_MAP enc_domain_map, bsgs_table_t table, int serverId, ENC_Stack &pre_enc_stack, double eta)
{
    Server server = server_vect[serverId]; // shallow copy to specify the first server initiate the verification
    const int number_known_records = server.known_record_subset.size();
    int v = (int)data_size * pv_ratio;
    const int req_known_record_found = server.generatePVTestCondition(data_size, v, number_known_records, eta);
    gamal_ciphertext_t sum, tmp, encrypt_E0;
    gamal_cipher_new(sum);
    gamal_cipher_new(tmp);
    gamal_cipher_new(encrypt_E0);

    pre_enc_stack.pop_E0(encrypt_E0);

    int counter = 0;
   

    //========= Checking both the PV size is V of enc(1), and the known records >= r0

    // gamal_ciphertext_t total_v, tmp_v;
    // gamal_cipher_new(tmp_v);
    // gamal_cipher_new(total_v);
    // total_v->C1 = encrypt_E0->C1;
    // total_v->C2 = encrypt_E0->C2;
    // tmp_v->C1 = total_v->C1;
    // tmp_v->C2 = total_v->C2;
    
    // for (ENC_DOMAIN_MAP::iterator ind = enc_domain_map.begin(); ind != enc_domain_map.end(); ind++)
    // {
    //     gamal_add(total_v, tmp_v, ind->second);
    //     tmp_v->C1 = total_v->C1;
    //     tmp_v->C2 = total_v->C2;
    // }

    // dig_t total_v_decrypt = Servers::_fusionDecrypt(total_v, table, serverId);
    // cout << "Total number of domain encrypted as 1 in PV: " << total_v_decrypt << endl;

    
    // if (total_v_decrypt != v)
    // {
    //     cout << "Fail the verification" << endl;
    //     return false;
    // }
    // else
    // {
    //     for (id_domain_set::iterator itr = server.known_record_subset.begin(); itr != server.known_record_subset.end(); itr++)
    //     {
    //         id_domain_pair domain_pair = *itr;
    //         ENC_DOMAIN_MAP::iterator find = enc_domain_map.find(domain_pair);
    //         if (find != enc_domain_map.end())
    //         {
    //             if (counter == 0)
    //             {
    //                 sum->C1 = find->second->C1;
    //                 sum->C2 = find->second->C2;
    //             }
    //             else
    //             {
    //                 tmp->C1 = sum->C1;
    //                 tmp->C2 = sum->C2;
    //                 gamal_add(sum, tmp, find->second);
    //             }

    //             //======= To find exactly which known rows are in PV 

    //             gamal_ciphertext_t tmp_decrypt;
    //             gamal_add(tmp_decrypt, find->second, encrypt_E0); // to fix the decryption function
    //             dig_t domain_decrypt = Servers::_fusionDecrypt(tmp_decrypt, table, serverId);
    //             if (domain_decrypt > 0)
    //             {
    //                  // cout << "Domain decrypt: " << domain_decrypt << endl;
    //                  server_vect[serverId].save_knownRow_found_in_PV(domain_pair);
    //             }
    //             //===========================================================

    //             counter++;
    //         }

    //     }

    //     dig_t sum_res = Servers::_fusionDecrypt(sum, table, serverId);

    //     if (sum_res >= req_known_record_found)
    //     {
    //         cout << "Total known rows found in PV: " << sum_res << endl;
    //         cout << "Pass the verification" << endl;
    //         return true;
    //     }
    //     else
    //     {
    //         cout << "Total known rows found in PV: " << sum_res << endl;
    //         cout << "Fail the verification" << endl;
    //         return false;
    //     }
    // }

    //============================================


    //====== if do not check PV size = V, only check the known records >= r0 ========

    for (id_domain_set::iterator itr = server.known_record_subset.begin(); itr != server.known_record_subset.end(); itr++)
        {
            id_domain_pair domain_pair = *itr;
            ENC_DOMAIN_MAP::iterator find = enc_domain_map.find(domain_pair);
            if (find != enc_domain_map.end())
            {
                if (counter == 0)
                {
                    sum->C1 = find->second->C1;
                    sum->C2 = find->second->C2;
                }
                else
                {
                    tmp->C1 = sum->C1;
                    tmp->C2 = sum->C2;
                    gamal_add(sum, tmp, find->second);
                }

                //======= To find exactly which known rows are in PV ====
                //To open each bin with label matching label of known records
                gamal_ciphertext_t tmp_decrypt;
                gamal_add(tmp_decrypt, find->second, encrypt_E0); // to fix the decryption function
                
                dig_t domain_decrypt = Servers::_fusionDecrypt(tmp_decrypt, table, serverId); 
                
                if (domain_decrypt > 0)
                {
                    // cout << "Domain decrypt: " << domain_decrypt << endl;
                    // server.save_knownRow_found_in_PV(domain_pair);
                    save_knownRow_found_in_PV(domain_pair);
                    
                }
                //===========================================================

                counter++;
            }

        }
    //decrypt the sum of all encryptions with matched label
    
    dig_t sum_res = Servers::_fusionDecrypt(sum, table, serverId); //serverId to specify the first server initiate the decryption;
     
    if (sum_res >= req_known_record_found)
    {
        cout << "Total known rows found in PV: " << sum_res << endl;
        cout << "Pass the verification" << endl;
        return true;
    }
    else
    {
        cout << "Total known rows found in PV: " << sum_res << endl;
        cout << "Fail the verification" << endl;
        return false;
    }


    //=====================================================
    
    
}

void Servers::re_encrypt_PV(ENC_DOMAIN_MAP enc_domain_map, int serverId, gamal_key_t coll_key_for_query_phase)
{
    Server server = server_vect[serverId]; // shallow copy
    gamal_ciphertext_t temp, cipher_update;

    for (ENC_DOMAIN_MAP::iterator itr = enc_domain_map.begin(); itr != enc_domain_map.end(); itr++)
    {
        // itr->second = 
        

        // gamal_ciphertext_t temp, cipher_update;
        temp->C1 = itr->second->C1;
        temp->C2 = itr->second->C2;

                
        gama_key_switch_lead(cipher_update, temp, server.key_PV_phase, coll_key_for_query_phase);

        for (int i=1; i< server_vect.size(); i++)
        {
            gama_key_switch_follow(cipher_update, temp, server_vect[serverId+i].key_PV_phase, coll_key_for_query_phase);
        }

        itr->second->C1 = cipher_update->C1;
        itr->second->C2 = cipher_update->C2;


    }
    // cout<<"re-encrypt end"<<endl;

}


//Tham
void Servers::open_true_PV(ENC_DOMAIN_MAP enc_domain_map, bsgs_table_t table, int serverId, ENC_Stack &pre_enc_stack)
{
    Server server = server_vect[serverId]; // shallow copy

    // id_domain_set found_rows_set_in_verifed_PV = server.verified_set;
    
    int v = (int)data_size * pv_ratio;

    gamal_ciphertext_t encrypt_E0;
    gamal_cipher_new(encrypt_E0);
    pre_enc_stack.pop_E0(encrypt_E0);

    int counter = 0;
    int counter_rows_PV = 0;
    ENC_DOMAIN_MAP::iterator itr =enc_domain_map.begin();

    known_rows_after_phase2 = server.known_record_subset;
    // server.known_rows_after_phase2 = server.known_record_subset;
    while (counter <= v)
    {
    
        //======= To find exactly which known rows are in PV ====
        id_domain_pair domain_pair = itr->first;
        
        gamal_ciphertext_t tmp_decrypt;
        gamal_add(tmp_decrypt, itr->second, encrypt_E0); // to fix the decryption function
        dig_t domain_decrypt = Servers::_fusionDecrypt(tmp_decrypt, table, serverId);
        if (domain_decrypt > 0)
        {
            // cout << "Domain decrypt: " << domain_decrypt << endl;
            // server.save_opened_rows(domain_pair);
            save_opened_rows(domain_pair);
            known_rows_after_phase2.insert(domain_pair);
            // server.known_rows_after_phase2.insert(domain_pair);
            counter_rows_PV++;
        }
        //===========================================================

        itr++;        
    
        counter++;
    }
      
    cout<<"number of opened domains in PV = "<<counter_rows_PV<<endl;
    cout<<"counter = "<<counter<<endl;

    const int number_known_records_updated = known_rows_after_phase2.size();

    cout<<"number_known_records_after_phase2 = "<<number_known_records_updated<<endl;

}

bool Servers::verifyingTestResult(string testName, gamal_ciphertext_t sum_cipher, bsgs_table_t table, int serverId, int threshold)
{
    dig_t decrypt_test_f = Servers::_fusionDecrypt(sum_cipher, table, serverId); 
    

    cout << testName << " " << decrypt_test_f << endl;

    // float epsilon = 0.1;
    // float sensitivity = 1.0;

    // int maxNoise = (int)(getLaplaceNoiseRange(sensitivity, epsilon, prob));
    cout << "Max noise: " << maxNoise << endl;

    if (decrypt_test_f >= (threshold - maxNoise) && decrypt_test_f <= (threshold + maxNoise))
    {
        cout << "Pass test function" << endl;
        return true;
    }
    else
    {
        cout << "Fail test function" << endl;
        return false;
    }
}

bool Servers::verifyingTestResult_Estimate(string testName, gamal_ciphertext_t sum_cipher, bsgs_table_t table, int serverId, gamal_ciphertext_t enc_PV_answer, double alpha)
{
    dig_t PV_answer = Servers::_fusionDecrypt(enc_PV_answer, table, serverId); 
    
    
    int int_PV_answer = (int)PV_answer;

    cout << "PV_answer = " << PV_answer << endl;

    vector<double> conf_range = Servers::estimate_conf_interval(alpha, int_PV_answer, data_size, data_size / 100); // confident interval of estimate answer

    int min_conf = (int)(conf_range[0]);
    int max_conf = (int)(conf_range[1]);

    cout<<"test OK"<<endl;

    dig_t decrypt_test_f = Servers::_fusionDecrypt(sum_cipher, table, serverId); 
    cout << testName << " " << decrypt_test_f << endl;
    cout << "min_conf " << min_conf << ", "
         << "max_conf " << max_conf << endl;

    // cout << "maxNoise " << (int)(maxNoise) << endl;
    if (decrypt_test_f >= min_conf - (int)(maxNoise) && decrypt_test_f <= max_conf + (int)(maxNoise))
    {
        cout << "Pass test function estimate" << endl;
        return true;
    }
    else
    {
        cout << "Fail test function estimate" << endl;
        return false;
    }
}

vector<double> Servers::estimate_conf_interval(double alpha, int PV_answer, int dataset_size, int PV_size)
{
    chi_squared_distribution<> chi_squared_distribution_min(PV_answer * 2);
    chi_squared_distribution<> chi_squared_distribution_max(PV_answer * 2 + 2);
    float min_answer = (dataset_size / (2 * PV_size)) * quantile(chi_squared_distribution_min, alpha / 2);
    float max_answer = (dataset_size / (2 * PV_size)) * quantile(chi_squared_distribution_max, 1 - alpha / 2);
    // cout << "min answer= " << min_answer << "; max answer = " << max_answer << endl;
    vector<double> answers = {min_answer, max_answer};
    return answers;
}

//Tham
void Servers::save_knownRow_found_in_PV(id_domain_pair verified_domain_pair)
{
    verified_set.insert(verified_domain_pair);
}


//Tham
void Servers::save_opened_rows(id_domain_pair opened_rows)
{
    opened_rows_set.insert(opened_rows);
}

//Tham: open the true PV for v bins to find more records and save to the known-records-set for testing

void Servers::save_knownRow_after_phase2(id_domain_pair domain_pair)
{
    known_rows_after_phase2.insert(domain_pair);
}