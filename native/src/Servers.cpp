#include "Servers.h"

Servers::Servers(int server_size, int data_size)
{
    gamal_generate_keys(coll_key);
    for (int i = 0; i < server_size; i++)
    {
        Server server(data_size);
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

    gamal_collective_key_gen_fixed(coll_key, p_key_list, server_size);
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
        gamal_fusion_decrypt(&res, server_size, server_vect[0].key, key_follow, ciphertext_update, itr->second[0], table);
        if (res > 0)
        {
            count += res;
        }
    }

    cout << "Total domain encrypted as 1: " << count << endl;
}

bool Servers::verificationPV(ENC_DOMAIN_MAP enc_domain_map, bsgs_table_t table, int serverId)
{
    Server server = server_vect[serverId];
    const int KNOWN_VECT_SIZE = server.known_vector.size();
    const int LEAST_DOMAIN = 4;
    // const int LEAST_DOMAIN = KNOWN_VECT_SIZE / 3;
    int count = 0;

    gamal_ciphertext_t sum, tmp;
    gamal_cipher_new(sum);
    gamal_cipher_new(tmp);

    int counter = 0;
    for (int i = 0; i < KNOWN_VECT_SIZE; i++)
    {
        ENC_DOMAIN_MAP::iterator find = enc_domain_map.find(server.known_vector[i]);
        if (find != enc_domain_map.end())
        {
            if (counter == 0)
            {
                sum->C1 = find->second[0]->C1;
                sum->C2 = find->second[0]->C2;
            }
            else
            {
                tmp->C1 = sum->C1;
                tmp->C2 = sum->C2;
                gamal_add(sum, tmp, find->second[0]);
            }

            counter++;
        }
    }

    cout << "Verifiation counter " << counter << endl;

    dig_t sum_res = Servers::_fusionDecrypt(sum, table, serverId);

    if (sum_res >= LEAST_DOMAIN)
    {
        cout << "Total domain found in PV: " << sum_res << endl;
        cout << "Pass the verification" << endl;
        return true;
    }
    else
    {
        cout << "Total domain found in PV: " << sum_res << endl;
        cout << "Fail the verification" << endl;
        return false;
    }
}