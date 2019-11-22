#include "Server.h"

Server::Server()
{
    myPIR_enc = NULL;
    plain_track_list = NULL;
    size_dataset = 0;
}

Server::Server(int size)
{
    gamal_generate_keys(key);

    myPIR_enc = new gamal_ciphertext_t[size];
    plain_track_list = new int[size];
    size_dataset = size;

    const string KNOWN_DOMAIN_DIR = "./data/known_domains.csv";
    Server::importFile(KNOWN_DOMAIN_DIR);
}

void Server::importFile(string file_url)
{

    std::ifstream data(file_url);
    if (!data.is_open())
    {
        std::exit(EXIT_FAILURE);
    }

    int i = 0;
    std::string str;
    std::getline(data, str); // skip the first line

    while (!data.eof())
    {
        getline(data, str);
        // cout << str << endl;
        if (str.empty())
        {
            continue;
        }
        string id, name;
        int count;
        istringstream iss(str);
        getline(iss, id, ',');
        getline(iss, name, ',');
        string id_domain = id + " " + name;
        known_vector.push_back(id);
    }
}

void Server::createRandomEncrypVector(ENC_Stack &pre_enc_stack)
{
    int *myPIR_arr; //final array with value of encryp type after reverting from shuffle array

    int enc_types[] = {1, 0};
    int freq[] = {1, 99};
    int pv_ratio = 100;

    myPIR_arr = new int[size_dataset];

    pir_gen(myPIR_arr, enc_types, freq, size_dataset, pv_ratio); // function that server place 1 or 0 randomly

    // ========== Encrypt the vector =============== //

    int plain1 = 1, plain0 = 0;

    for (int i = 0; i < size_dataset; i++)
    {
        plain_track_list[i] = myPIR_arr[i];
        if (myPIR_arr[i] == 1)
        {
            pre_enc_stack.pop_E1(myPIR_enc[i]);
        }
        else
        {
            pre_enc_stack.pop_E0(myPIR_enc[i]);
        }
    }

    delete[] myPIR_arr;
}

bool Server::verificationPV(ENC_DOMAIN_MAP &enc_domain_map)
{
    const int LEAST_DOMAIN = known_vector.size() / 3;
    int count = 0;

    for(int i=0; i<known_vector.size(); i++)
    {
        if(enc_domain_map.count(known_vector[i]) > 0)
        {
            count++;
        }
    }

    if (count >= LEAST_DOMAIN)
    {
        cout << "Pass the verification" << endl;
        return true;
    }
    else
    {
        cout << "Fail the verification" << endl;
        return false;
    }
}

// gamal_key_t Server::coll_key;

// void Server::setCollKey(gamal_key_t new_coll_key)
// {
//     coll_key->Y = new_coll_key->Y;
//     coll_key->secret = new_coll_key->secret;
//     coll_key->is_public = new_coll_key->is_public;
// }
