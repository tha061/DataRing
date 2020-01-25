#include "Servers.h"
#include "process_noise.h"

Servers::Servers(int server_size, int data_size, string known_domain_dir)
{
    gamal_generate_keys(coll_key);
    for (int i = 0; i < server_size; i++)
    {
        Server server(data_size, known_domain_dir);
        server_vect.push_back(server);
    }

    s_myPIR_enc = new gamal_ciphertext_t[data_size];
    s_plain_track_list = new int[data_size];

    Servers::server_size = server_size;
    Servers::data_size = data_size;
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

void Servers::generateCollKey()
{
    int server_size = server_vect.size();
    EC_POINT **p_key_list = new EC_POINT *[server_size];
    // BIGNUM **secret_key_list = new BIGNUM *[server_size]; //added 21 Jan by Tham: for re-encryption
    for (int i = 0; i < server_size; i++)
    {
        p_key_list[i] = server_vect[i].key->Y;
        // secret_key_list[i] = server_vect[i].key->secret; //added 21 Jan by Tham: for re-encryption
    }

    gamal_collective_publickey_gen(coll_key, p_key_list, server_size);
}

dig_t Servers ::_fusionDecrypt(gamal_ciphertext_t ciphertext, bsgs_table_t table, int serverId)
{
    int server_size = server_vect.size();
    gamal_key_t key_follow[server_size - 1];

    int counter = 0;
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

    dig_t res;
    gamal_ciphertext_t ciphertext_update;

    gamal_fusion_decrypt(&res, server_size, server_vect[serverId].key, key_follow, ciphertext_update, ciphertext, table);

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
/**
 * to verify the PV
 * @paras: eta is the probability s.t the honest participant will pass the test at eta
 * higher eta is, smaller r0 is
 * 
 */
bool Servers::verifyingPV(ENC_DOMAIN_MAP enc_domain_map, bsgs_table_t table, int serverId, ENC_Stack &pre_enc_stack, double eta)
{
    Server server = server_vect[serverId]; // shallow copy
    const int KNOWN_VECT_SIZE = server.known_vector.size();
    int v = (int)data_size * pv_ratio;
    const int LEAST_DOMAIN = server_vect[serverId].generatePVTestCondition(data_size, v, KNOWN_VECT_SIZE, eta);
    gamal_ciphertext_t sum, tmp, encrypt_E0;
    gamal_cipher_new(sum);
    gamal_cipher_new(tmp);
    gamal_cipher_new(encrypt_E0);

    pre_enc_stack.pop_E0(encrypt_E0);

    int counter = 0;

    gamal_ciphertext_t total_v_decrypt, tmp_v_decrypt;
    gamal_cipher_new(tmp_v_decrypt);
    gamal_cipher_new(total_v_decrypt);
    int count = 0;

    for (id_domain_set::iterator itr = server.known_vector.begin(); itr != server.known_vector.end(); itr++)
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

            gamal_ciphertext_t tmp_decrypt;
            gamal_add(tmp_decrypt, find->second, encrypt_E0); // to fix the decryption function
            dig_t domain_decrypt = Servers::_fusionDecrypt(tmp_decrypt, table, serverId);
            if (domain_decrypt > 0)
            {
                // cout << "Domain decrypt: " << domain_decrypt << endl;
                server_vect[serverId].save_knownRow_found_in_PV(domain_pair);
            }

            counter++;
        }

    }

    // dig_t total_v = Servers::_fusionDecrypt(total_v_decrypt, table, serverId);
    // cout << "V - Total number of domain encrypted as 1: " << total_v << endl;
    // if (total_v == (data_size * 0.01))
    // {
    //     cout << "Pass V size verification" << endl;
    // }
    // else
    // {
    //     cout << "Fail V size verification" << endl;
    // }

    // cout << "L - TOtal number of known domain ecrypted from partial view: " << counter << endl;
    // if (counter == server.known_vector.size())
    // {
    //     cout << "Pass L size verification" << endl;
    // }
    // else
    // {
    //     cout << "Fail L size verification" << endl;
    // }

    // cout << "Total matching domain: " << counter << endl;

    dig_t sum_res = Servers::_fusionDecrypt(sum, table, serverId);

    if (sum_res >= LEAST_DOMAIN)
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

bool Servers::verifyingTestResult(string testName, gamal_ciphertext_t sum_cipher, bsgs_table_t table, int serverId, int threshold)
{
    dig_t decrypt_test_f = Servers::_fusionDecrypt(sum_cipher, table, serverId);
    cout << testName << " " << decrypt_test_f << endl;

    // float epsilon = 0.1;
    // float sensitivity = 1.0;

    // int maxNoise = (int)(getLaplaceNoiseRange(sensitivity, epsilon, prob));
    // cout << "Max noise: " << maxNoise << endl;

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