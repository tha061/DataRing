#include "Servers.h"
#include "process_noise.h"

/** 
 * @file Servers.cpp
 * @brief Implementation of Servers' functions
 * @author Tham Nguyen tham.nguyen@mq.edu.au, Nam Bui, Data Ring
 * @date April 2020
*/ 


Servers::Servers(int server_size, int data_size, string background_knowledge_dir, int domain_coefficient)
{
    gamal_generate_keys(coll_key);
    

    for (int i = 0; i < server_size; i++)
    {
        Server server(data_size, background_knowledge_dir); // initiate all servers with their key pair
        server_vect.push_back(server);
    }


    Servers::server_size = server_size;
    Servers::data_size = data_size;
    Servers::domain_size = data_size*domain_coefficient; //aN

    s_myPIR_enc = new gamal_ciphertext_t[domain_size];
    s_plain_track_list = new int[domain_size];
}


void Servers::generateCollKey(gamal_key_t collective_key)
{
    int server_size = server_vect.size();
    EC_POINT **p_key_list = new EC_POINT *[server_size];
      
   
        for (int i = 0; i < server_size; i++)
        {
            
            p_key_list[i] = server_vect[i].key->Y;
            
        }

        gamal_collective_publickey_gen(coll_key, p_key_list, server_size);
   
}



dig_t Servers ::_fusionDecrypt(gamal_ciphertext_t ciphertext, bsgs_table_t table, int serverId)
{
    
    int server_size = server_vect.size();
   
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


    return res;
}




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


                //======= To save known rows found in PV to a set====
                //To open each bin with label matching label of known records
                // gamal_ciphertext_t tmp_decrypt;
                // gamal_add(tmp_decrypt, find->second, encrypt_E0); // to fix the decryption function
                
                // dig_t domain_decrypt = Servers::_fusionDecrypt(tmp_decrypt, table, serverId); 
                
                // if (domain_decrypt > 0)
                // {
                //     // cout << "Domain decrypt: " << domain_decrypt << endl;
                //     // server.save_knownRow_found_in_PV(domain_pair);
                //     save_knownRow_found_in_PV(domain_pair);
                    
                // }
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
    
    
}



bool Servers::verifyingTestResult(gamal_ciphertext_t sum_cipher, bsgs_table_t table, int serverId, int threshold)
{
    dig_t decrypt_test_f = Servers::_fusionDecrypt(sum_cipher, table, serverId); 
    

    if (decrypt_test_f >= (threshold - maxNoise) && decrypt_test_f <= (threshold + maxNoise))
    {
        
        return true;
    }
    else
    {
    
        return false;
    }
}

//for Test Attribute
bool Servers::verifyingTestResult_Estimate(string testName, gamal_ciphertext_t sum_cipher, bsgs_table_t table, int serverId, gamal_ciphertext_t enc_PV_answer, double alpha)
{
    dig_t PV_answer = Servers::_fusionDecrypt(enc_PV_answer, table, serverId); 
    
    
    int int_PV_answer = (int)PV_answer;


    vector<double> conf_range = Servers::estimate_conf_interval(alpha, int_PV_answer, data_size, data_size / 100); // confident interval of estimate answer

    int min_conf = (int)(conf_range[0]);
    int max_conf = (int)(conf_range[1]);


    dig_t decrypt_test_f = Servers::_fusionDecrypt(sum_cipher, table, serverId); 
   
    if (decrypt_test_f >= min_conf - (int)(maxNoise) && decrypt_test_f <= max_conf + (int)(maxNoise))
    {
       
        return true;
    }
    else
    {

        return false;
    }
}

vector<double> Servers::estimate_conf_interval(double alpha, int PV_answer, int dataset_size, int PV_size)
{
    chi_squared_distribution<> chi_squared_distribution_min(PV_answer * 2);
    chi_squared_distribution<> chi_squared_distribution_max(PV_answer * 2 + 2);
    float min_answer = (dataset_size / (2 * PV_size)) * quantile(chi_squared_distribution_min, alpha / 2);
    float max_answer = (dataset_size / (2 * PV_size)) * quantile(chi_squared_distribution_max, 1 - alpha / 2);
    vector<double> answers = {min_answer, max_answer};
    return answers;
}


// 
// void Servers::save_knownRow_found_in_PV(id_domain_pair verified_domain_pair)
// {
//     verified_set.insert(verified_domain_pair);
// }

