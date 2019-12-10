#include "Server.h"

Server::Server()
{
    myPIR_enc = NULL;
    plain_track_list = NULL;
    size_dataset = 0;
}

Server::Server(int size, string known_domain_dir)
{
    gamal_generate_keys(key);

    myPIR_enc = new gamal_ciphertext_t[size];
    plain_track_list = new int[size];
    size_dataset = size;

    Server::importFile(known_domain_dir);
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
        known_vector.insert(make_pair(id, name));
        // known_vector_new.push_back(make_pair(id, name));
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

void Server::generateTestHashMap_1(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map)
{
    for (ENC_DOMAIN_MAP::iterator itr = enc_domain_map.begin(); itr != enc_domain_map.end(); itr++)
    {
        id_domain_pair domain = itr->first;
        gamal_ciphertext_t *mul_enc_ciphertext = new gamal_ciphertext_t[1];
        if (known_vector.find(domain) != known_vector.end())
        {
            pre_enc_stack.pop_E1(mul_enc_ciphertext[0]);
        }
        else
        {
            pre_enc_stack.pop_E0(mul_enc_ciphertext[0]);
        }
        enc_test_map.insert({domain, mul_enc_ciphertext[0]});
    }
}

void Server::generateTestHashMap_2(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map)
{
    enc_test_map.clear();
    gamal_ciphertext_t encrypt_0;
    gamal_cipher_new(encrypt_0);
    pre_enc_stack.pop_E0(encrypt_0);

    for (ENC_DOMAIN_MAP::iterator itr = enc_domain_map.begin(); itr != enc_domain_map.end(); itr++)
    {
        id_domain_pair domain = itr->first;
        gamal_ciphertext_t *add_enc_ciphertext = new gamal_ciphertext_t[1];
        gamal_add(add_enc_ciphertext[0], itr->second, encrypt_0);
        enc_test_map.insert({domain, add_enc_ciphertext[0]});
    }
}

void Server::generateTestHashMap_3(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map)
{
    enc_test_map.clear();
    gamal_ciphertext_t encrypt_0;
    gamal_cipher_new(encrypt_0);
    pre_enc_stack.pop_E0(encrypt_0);

    for (ENC_DOMAIN_MAP::iterator itr = enc_domain_map.begin(); itr != enc_domain_map.end(); itr++)
    {
        id_domain_pair domain = itr->first;
        gamal_ciphertext_t *enc_ciphertext = new gamal_ciphertext_t[1];
        if (verified_set.find(domain) != verified_set.end())
        {
            pre_enc_stack.pop_E0(enc_ciphertext[0]);
        }
        else
        {
            gamal_add(enc_ciphertext[0], itr->second, encrypt_0);
        }
        enc_test_map.insert({domain, enc_ciphertext[0]});
    }
}

void _importQuery(map<int, string> &cols_map)
{
    string QUERY_DIR = "./data/query_data.csv";

    std::ifstream data(QUERY_DIR);
    if (!data.is_open())
    {
        std::exit(EXIT_FAILURE);
    }
    std::string str;
    std::getline(data, str); // skip the first line
    while (!data.eof())
    {
        getline(data, str);
        if (str.empty())
        {
            continue;
        }

        string value;
        int col_id;

        istringstream iss(str);
        getline(iss, value, ',');
        iss >> col_id;

        // cout << col_id << " " << value << endl;
        cols_map.insert({col_id, value});
    }
}

void Server::generateTestHashMap_Attr(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map)
{
    enc_test_map.clear();

    map<int, string> columns_map;
    _importQuery(columns_map);
    const int COLUMN_SIZE = 10;
    int counter = 0;

    for (ENC_DOMAIN_MAP::iterator itr = enc_domain_map.begin(); itr != enc_domain_map.end(); itr++)
    {
        bool match = true;

        vector<string> col_arr;
        id_domain_pair domain_pair = itr->first;
        string domain = domain_pair.second;
        char delim = ' ';
        stringstream ss(domain);
        string token;
        while (getline(ss, token, delim))
        {
            col_arr.push_back(token);
        }

        for (map<int, string>::iterator colItr = columns_map.begin(); colItr != columns_map.end(); colItr++)
        {
            int col_index = colItr->first;
            string col_value = colItr->second;
            string o_col_value = col_arr[col_index];

            if (o_col_value != col_value)
            {
                match = false;
            }
        }

        gamal_ciphertext_t *enc_ciphertext = new gamal_ciphertext_t[1];
        if (match)
        {
            counter++;
            pre_enc_stack.pop_E1(enc_ciphertext[0]);
        }
        else
        {
            pre_enc_stack.pop_E0(enc_ciphertext[0]);
        }
        enc_test_map.insert({domain_pair, enc_ciphertext[0]});
    }

    cout << "Total match row in attribute test func: " << counter << endl;

}




void Server::save_knownRow_found_in_PV(id_domain_pair verified_domain_pair)
{
    verified_set.insert(verified_domain_pair);
}

void Server::generateTestFunction(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map, int type)
{
    switch (type)
    {
        case 1: // targeting L known rows
        {
            Server::generateTestHashMap_1(pre_enc_stack, enc_domain_map);
            break;
        }

        case 2: // targetting V rows in PV
        {
            Server::generateTestHashMap_2(pre_enc_stack, enc_domain_map);
            break;
        }

        case 3: // targeting V - r0 rows in PV
        {
            Server::generateTestHashMap_3(pre_enc_stack, enc_domain_map);
            break;
        }

        case 4: // targeting specific attributes
        {
            Server::generateTestHashMap_Attr(pre_enc_stack, enc_domain_map);
            break;
        }
        default:
        {
            cout << "Please enter type of test function" << endl;
            break;
        }
    }
}


void Server::generateNormalQuery(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map)
{
    enc_test_map.clear();

    map<int, string> columns_map;
    _importQuery(columns_map);
    const int COLUMN_SIZE = 10;
    int counter = 0;

    for (ENC_DOMAIN_MAP::iterator itr = enc_domain_map.begin(); itr != enc_domain_map.end(); itr++)
    {
        bool match = true;

        vector<string> col_arr;
        id_domain_pair domain_pair = itr->first;
        string domain = domain_pair.second;
        char delim = ' ';
        stringstream ss(domain);
        string token;
        while (getline(ss, token, delim))
        {
            col_arr.push_back(token);
        }

        for (map<int, string>::iterator colItr = columns_map.begin(); colItr != columns_map.end(); colItr++)
        {
            int col_index = colItr->first;
            string col_value = colItr->second;
            string o_col_value = col_arr[col_index];

            if (o_col_value != col_value)
            {
                match = false;
            }
        }

        gamal_ciphertext_t *enc_ciphertext = new gamal_ciphertext_t[1];
        if (match)
        {
            counter++;
            pre_enc_stack.pop_E1(enc_ciphertext[0]);
        }
        else
        {
            pre_enc_stack.pop_E0(enc_ciphertext[0]);
        }
        enc_test_map.insert({domain_pair, enc_ciphertext[0]});
    }

    cout << "Total match domains in query: " << counter << endl;

}


vector<double> Server::estimate_conf_interval(double alpha, int PV_answer, int dataset_size, int PV_size)
{
    chi_squared_distribution<> chi_squared_distribution_min(PV_answer * 2);
    chi_squared_distribution<> chi_squared_distribution_max(PV_answer * 2 + 2);
    float min_answer = (dataset_size / (2 * PV_size)) * quantile(chi_squared_distribution_min, alpha / 2);
    float max_answer = (dataset_size / (2 * PV_size)) * quantile(chi_squared_distribution_max, 1 - alpha / 2);
    cout << "min answer= " << min_answer << "; max answer = " << max_answer << endl;
    vector<double> answers = {min_answer, max_answer};
    return answers;
}

float Server::generatePVTestCondition(int dataset, int PV, int known_records, double eta)
{
    hypergeometric_distribution<> hyper_dist(known_records, PV, dataset);
    float result = quantile(complement(hyper_dist, eta)) + 1;
    cout << "eta value = " << eta << ", r0 = " << result << endl;
    return result;
}