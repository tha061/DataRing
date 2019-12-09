#include "Servers.h"

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

void Servers::createServersEncrypVector(ENC_Stack &pre_enc_stack)
{
    int enc_types[] = {1, 0};
    int freq[] = {1, 99};
    int pv_ratio = 100;

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

    cout << "Total domain encrypted as 1: " << count << endl;
}

bool Servers::verificationPV(ENC_DOMAIN_MAP enc_domain_map, bsgs_table_t table, int serverId, ENC_Stack &pre_enc_stack)
{
    Server server = server_vect[serverId]; // shallow copy
    const int KNOWN_VECT_SIZE = server.known_vector.size();
    const int LEAST_DOMAIN = 6; //r0

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

    for (ENC_DOMAIN_MAP::iterator itr = enc_domain_map.begin(); itr != enc_domain_map.end(); itr++)
    {
        id_domain_pair domain_pair = itr->first;
        id_domain_set::iterator find = server.known_vector.find(domain_pair);
        if (find != server.known_vector.end())
        {
            if (counter == 0)
            {
                sum->C1 = itr->second->C1;
                sum->C2 = itr->second->C2;
            }
            else
            {
                tmp->C1 = sum->C1;
                tmp->C2 = sum->C2;
                gamal_add(sum, tmp, itr->second);
            }

            gamal_ciphertext_t tmp_decrypt;
            gamal_add(tmp_decrypt, itr->second, encrypt_E0); // to fix the decryption function
            dig_t domain_decrypt = Servers::_fusionDecrypt(tmp_decrypt, table, serverId);
            if (domain_decrypt > 0)
            {
                // cout << "Domain decrypt: " << domain_decrypt << endl;
                server_vect[serverId].addVerifiedDomain(domain_pair);
            }

            counter++;
        }

        if (count == 0)
        {
            total_v_decrypt->C1 = itr->second->C1;
            total_v_decrypt->C2 = itr->second->C2;
        }
        else
        {
            tmp_v_decrypt->C1 = total_v_decrypt->C1;
            tmp_v_decrypt->C2 = total_v_decrypt->C2;
            gamal_add(total_v_decrypt, tmp_v_decrypt, itr->second);
        }

        count++;
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

    dig_t sum_res = Servers::_fusionDecrypt(sum, table, serverId);

    if (sum_res >= LEAST_DOMAIN)
    {
        // cout << "Total domain found in PV: " << sum_res << endl;
        // cout << "Pass the verification" << endl;
        return true;
    }
    else
    {
        // cout << "Total domain found in PV: " << sum_res << endl;
        // cout << "Fail the verification" << endl;
        return false;
    }
}

bool Servers::verificationTestResult(string testName, gamal_ciphertext_t sum_cipher, bsgs_table_t table, int serverId, int threshold)
{
	dig_t decrypt_test_f = Servers::_fusionDecrypt(sum_cipher, table, serverId);
	cout << testName << " " << decrypt_test_f << endl;
    if(decrypt_test_f < threshold)
    {
        // cout << "Test function fail" << endl;
        return false;
    }else{
        // cout << "Test function pass" << endl;
        return true;
    }
}