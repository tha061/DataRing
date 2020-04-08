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
        cout << "Known Domains File is not defined" << endl;
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
        known_record_subset.insert(make_pair(id, name));
        // known_record_subset_new.push_back(make_pair(id, name));
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

//added by Tham 14 Dec to improve runtime
void Server::prepareTestFuntion_Query_Vector(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map)
{
    enc_question_map_pre.clear();
    for (ENC_DOMAIN_MAP::iterator itr = enc_domain_map.begin(); itr != enc_domain_map.end(); itr++)
    {
        id_domain_pair domain = itr->first;
        gamal_ciphertext_t *enc_0 = new gamal_ciphertext_t[1];
        pre_enc_stack.pop_E0(enc_0[0]);
        enc_question_map_pre.insert({domain, enc_0[0]});
    }
}

void Server::generateTestFunction(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map, int type)
{
    switch (type)
    {
    case 1: // targeting L known rows
    {
        Server::generateTestKnownRecords(pre_enc_stack, enc_domain_map);
        break;
    }

    case 2: // targetting V rows in PV
    {
        Server::generateTestBasedPartialView(pre_enc_stack, enc_domain_map);
        break;
    }

    case 3: // targeting V - r0 rows in PV
    {
        Server::generateTestHashMap_3(pre_enc_stack, enc_domain_map);
        break;
    }

    case 4: // targeting specific attributes
    {
        Server::generateTest_Target_Attr(pre_enc_stack, enc_domain_map);
        break;
    }
    default:
    {
        cout << "Please enter type of test function" << endl;
        break;
    }
    }
}

//test target L added by Tham 14 Dec for optimize runtime
void Server::generateTestKnownRecords_opt(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map)
{
    enc_question_map = enc_question_map_pre;
    gamal_ciphertext_t encrypt_1;
    gamal_cipher_new(encrypt_1);
    pre_enc_stack.pop_E1(encrypt_1);

    for (ENC_DOMAIN_MAP::iterator itr = enc_question_map.begin(); itr != enc_question_map.end(); itr++)
    {
        id_domain_pair domain = itr->first;

        if (known_record_subset.find(domain) != known_record_subset.end())
        {

            gamal_add(itr->second, itr->second, encrypt_1);
        }
    }
   
}

//target L
void Server::generateTestKnownRecords(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map)
{
    for (ENC_DOMAIN_MAP::iterator itr = enc_domain_map.begin(); itr != enc_domain_map.end(); itr++)
    {
        id_domain_pair domain = itr->first;
        gamal_ciphertext_t *mul_enc_ciphertext = new gamal_ciphertext_t[1];
        if (known_record_subset.find(domain) != known_record_subset.end())
        {
            pre_enc_stack.pop_E1(mul_enc_ciphertext[0]);
        }
        else
        {
            pre_enc_stack.pop_E0(mul_enc_ciphertext[0]);
        }
        enc_question_map.insert({domain, mul_enc_ciphertext[0]});
    }
}


//Tham: test L + rows opened in PV
void Server::generateTest_known_records_after_phase2(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map, id_domain_set known_rows_after_phase2)
{
    enc_question_map = enc_question_map_pre;
    gamal_ciphertext_t encrypt_1;
    gamal_cipher_new(encrypt_1);
    pre_enc_stack.pop_E1(encrypt_1);

    for (ENC_DOMAIN_MAP::iterator itr = enc_question_map.begin(); itr != enc_question_map.end(); itr++)
    {
        id_domain_pair domain = itr->first;

        if (known_rows_after_phase2.find(domain) != known_rows_after_phase2.end())
        {

            gamal_add(itr->second, itr->second, encrypt_1);
        }
    }
   
}


//test target V
void Server::generateTestBasedPartialView(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map)
{
    enc_question_map.clear();
    gamal_ciphertext_t encrypt_0;
    gamal_cipher_new(encrypt_0);
    pre_enc_stack.pop_E0(encrypt_0);

    for (ENC_DOMAIN_MAP::iterator itr = enc_domain_map.begin(); itr != enc_domain_map.end(); itr++)
    {
        id_domain_pair domain = itr->first;
        gamal_ciphertext_t *add_enc_ciphertext = new gamal_ciphertext_t[1];
        gamal_add(add_enc_ciphertext[0], itr->second, encrypt_0);
        enc_question_map.insert({domain, add_enc_ciphertext[0]});
    }
}
//added by Tham to reduce runtime
void Server::generateTestBasedPartialView_opt(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map)
{
    enc_question_map.clear();
    // enc_question_map = enc_domain_map;
    gamal_ciphertext_t encrypt_0;
    gamal_cipher_new(encrypt_0);
    pre_enc_stack.pop_E0(encrypt_0);

    for (ENC_DOMAIN_MAP::iterator itr = enc_domain_map.begin(); itr != enc_domain_map.end(); itr++)
    {
        id_domain_pair domain = itr->first;
        gamal_ciphertext_t *add_enc_ciphertext = new gamal_ciphertext_t[1];
        gamal_add(add_enc_ciphertext[0], itr->second, encrypt_0);
        itr->second = add_enc_ciphertext[0];
        // enc_question_map.insert({domain, add_enc_ciphertext[0]});
        itr++; //only process for half of the domains to save time
    }

    enc_question_map = enc_domain_map;
}

// This test target PV without r0 found in submitted PV
void Server::generateTest_PV_r0(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map, id_domain_set known_rows_in_pv)
{
    enc_question_map.clear();
    // enc_question_map = enc_domain_map;
    gamal_ciphertext_t encrypt_0;
    gamal_cipher_new(encrypt_0);
    pre_enc_stack.pop_E0(encrypt_0);

    for (ENC_DOMAIN_MAP::iterator itr = enc_domain_map.begin(); itr != enc_domain_map.end(); itr++)
    {
        id_domain_pair domain = itr->first;
        gamal_ciphertext_t *add_enc_ciphertext = new gamal_ciphertext_t[1];
        gamal_add(add_enc_ciphertext[0], itr->second, encrypt_0);
        itr->second = add_enc_ciphertext[0];
        // enc_question_map.insert({domain, add_enc_ciphertext[0]});
        itr++; //only process for half of the domains to save time
    }

    
    // removed r0 records from the test
    
    gamal_ciphertext_t encrypt_1;
    gamal_cipher_new(encrypt_1);
    pre_enc_stack.pop_E0(encrypt_1);

    for (ENC_DOMAIN_MAP::iterator itr = enc_question_map.begin(); itr != enc_question_map.end(); itr++)
    {
        id_domain_pair domain = itr->first;

        if (known_rows_in_pv.find(domain) != known_rows_in_pv.end())
        {

            cout<<"found one record of r0"<<endl;
            // itr->second = encrypt_0;
            gamal_subtract(itr->second, itr->second, encrypt_1);
        }
    }

    enc_question_map = enc_domain_map;
   
}


// This test target PV and (L- r0) records
// known_rows_in_PV: r0
// known_record_subset: L (background knowledge)
void Server::generateTest_PV_L_r0(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map, id_domain_set known_rows_in_pv, id_domain_set known_record_subset)
{
    enc_question_map.clear();
    // enc_question_map = enc_domain_map;
    gamal_ciphertext_t encrypt_0;
    gamal_cipher_new(encrypt_0);
    pre_enc_stack.pop_E0(encrypt_0);

    gamal_ciphertext_t encrypt_1;
    gamal_cipher_new(encrypt_1);
    pre_enc_stack.pop_E0(encrypt_1);

    for (ENC_DOMAIN_MAP::iterator itr = enc_domain_map.begin(); itr != enc_domain_map.end(); itr++)
    {
        id_domain_pair domain = itr->first;
        gamal_ciphertext_t *add_enc_ciphertext = new gamal_ciphertext_t[1];
        gamal_add(add_enc_ciphertext[0], itr->second, encrypt_0);
        itr->second = add_enc_ciphertext[0];
        // enc_question_map.insert({domain, add_enc_ciphertext[0]});
        itr++; //only process for half of the domains to save time
    }

    // Added targeting L records in background knowledge

    int count_L = 0;
    for (id_domain_set::iterator itr = known_record_subset.begin(); itr != known_record_subset.end(); itr++)
        {
            id_domain_pair domain_pair = *itr;
            ENC_DOMAIN_MAP::iterator find = enc_domain_map.find(domain_pair);
            if (find != enc_domain_map.end())
            {
               count_L++;
               gamal_add(find->second,find->second,encrypt_1);
            }

        }

    cout<<"count_L= "<<count_L<<endl;

    // removed r0 records from the test
    

    // for (ENC_DOMAIN_MAP::iterator itr = enc_question_map.begin(); itr != enc_question_map.end(); itr++)
    // {
    //     id_domain_pair domain = itr->first;

    //     if (known_rows_in_pv.find(domain) != known_rows_in_pv.end())
    //     {

    //         cout<<"found one record of r0"<<endl;
    //         // itr->second = encrypt_0;
    //         gamal_subtract(itr->second, itr->second, encrypt_1);
    //     }
    // }

    int count_r0 = 0;
    for (id_domain_set::iterator itr = known_rows_in_pv.begin(); itr != known_rows_in_pv.end(); itr++)
        {
            id_domain_pair domain_pair = *itr;
            ENC_DOMAIN_MAP::iterator find = enc_domain_map.find(domain_pair);
            if (find != enc_domain_map.end())
            {
               count_r0++;
               gamal_subtract(find->second,find->second,encrypt_1);
            }

        }

    cout<<"count_r0 = "<<count_r0<<endl;
    

    enc_question_map = enc_domain_map;
   
}

// //test target V - r0
void Server::generateTestHashMap_3(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map)
{
    enc_question_map.clear();
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
        enc_question_map.insert({domain, enc_ciphertext[0]});
    }
}



//updated by Tham to use separate file directory for normal query and test function (6 Feb 2020)
void _importQuery(map<int, string> &cols_map, bool test_or_query)
{
    string QUERY_DIR = "./data/query_data.csv";
    string TEST_DIR = "./data/test_estimate_sql.csv";

    if (test_or_query == 1) { //gen test estimation function
        std::ifstream data(TEST_DIR);
        if (!data.is_open())
        {
            cout << "Test File is not defined" << endl;
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
    else
    {
        // cout<<"check normal query bool"<<endl;
        std::ifstream data(QUERY_DIR); //gen query function
        if (!data.is_open())
        {
            cout << "Query File is not defined" << endl;
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
    
}


//Added by Tham 17 Feb 2020 for releasing phase
void Server::generateNormalQuery_Clear(ENC_Stack &pre_enc_stack)
{
    for (ENC_DOMAIN_MAP::iterator itr = enc_question_map_pre.begin(); itr != enc_question_map_pre.end(); itr++)
    {
        id_domain_pair domain_pair = itr->first;
        plain_domain_map.insert({domain_pair, 0});
    }


    int counter = 0;
    for (int i = 0; i < match_query_domain_vect.size(); i++)
    {
        id_domain_pair match_domain = match_query_domain_vect[i];
        hash_pair_map::iterator find = plain_domain_map.find(match_domain);
        if (find != plain_domain_map.end() && find->second == 0)
        {
            counter++;
            find->second = 1;
        }
    }

    // cout << "Total match plain domain: " << counter << endl;
}


//generate clear test attribute for server to query the submitted partial view
void Server::generateServerDomain_Test_Target_Attr(ENC_Stack &pre_enc_stack)
{
    for (ENC_DOMAIN_MAP::iterator itr = enc_question_map_pre.begin(); itr != enc_question_map_pre.end(); itr++)
    {
        id_domain_pair domain_pair = itr->first;
        plain_domain_map.insert({domain_pair, 0});
    }


    int counter = 0;
    for (int i = 0; i < match_query_domain_vect.size(); i++)
    {
        id_domain_pair match_domain = match_query_domain_vect[i];
        hash_pair_map::iterator find = plain_domain_map.find(match_domain);
        if (find != plain_domain_map.end() && find->second == 0)
        {
            counter++;
            find->second = 1;
        }
    }

    // cout << "Total match plain domain: " << counter << endl;
}

//added by Tham in 14 Dec 18 to optimize runtime

void Server::generateTest_Target_Attr_opt(ENC_Stack &pre_enc_stack)
{
    enc_question_map.clear();
    enc_question_map = enc_question_map_pre;

    gamal_ciphertext_t encrypt_1;
    gamal_cipher_new(encrypt_1);
    pre_enc_stack.pop_E1(encrypt_1);

    int counter = 0;

    // cout << "Mach domains: " << match_query_domain_vect.size() << endl;

    for (int i = 0; i < match_query_domain_vect.size(); i++)
    {
        id_domain_pair match_domain = match_query_domain_vect[i];
        ENC_DOMAIN_MAP::iterator find = enc_question_map.find(match_domain);
        if (find != enc_question_map.end())
        {
            counter++;
            gamal_add(find->second, find->second, encrypt_1);
        }
    }

    // cout << "Total match domains in query: " << counter << endl;
}

void Server::generateTest_Target_Attr(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map)
{
    enc_question_map.clear();

    map<int, string> columns_map;
    _importQuery(columns_map, 1); // test_or_query = 1 to generate test estimation function
    const int COLUMN_SIZE = 10;
    int counter = 0;
    int plain;

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
            plain = 1;
        }
        else
        {
            pre_enc_stack.pop_E0(enc_ciphertext[0]);
            plain = 0;
        }
        plain_domain_map.insert({domain_pair, plain}); //for server to get answer from encrypted
        enc_question_map.insert({domain_pair, enc_ciphertext[0]});
    }

    // cout << "Total match row in attribute test func: " << counter << endl;
}


void Server::generateTest_Target_All_Records(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map)
{
    enc_question_map.clear();
    int count = 0;
    for (ENC_DOMAIN_MAP::iterator itr = enc_domain_map.begin(); itr != enc_domain_map.end(); itr++)
    {
        id_domain_pair domain = itr->first;
        gamal_ciphertext_t *ciphertext = new gamal_ciphertext_t[1];
        
        pre_enc_stack.pop_E1(ciphertext[0]);
        enc_question_map.insert({domain,ciphertext[0]});
        count++;
    }
    // cout<<"\nnumber of enc(1) in the test target all rows: "<<count<<endl;
}


void Server::save_knownRow_found_in_PV(id_domain_pair verified_domain_pair)
{
    verified_set.insert(verified_domain_pair);
}


//Tham
void Server::save_opened_rows(id_domain_pair opened_rows)
{
    opened_rows_set.insert(opened_rows);
}

//Tham: open the true PV for v bins to find more records and save to the known-records-set for testing

void Server::save_knownRow_after_phase2(id_domain_pair domain_pair)
{
    known_rows_after_phase2.insert(domain_pair);
}




//updated by Tham to use separate file directory for normal query and test function (6 Feb 2020) and
// for making range query
void Server::generateMatchDomain(bool test_or_query)
{
    map<int, string> columns_map;
    _importQuery(columns_map,test_or_query);

    match_query_domain_vect.clear();
    int counter = 0;

    for (ENC_DOMAIN_MAP::iterator itr = enc_question_map_pre.begin(); itr != enc_question_map_pre.end(); itr++)
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
            
            vector<string> query_arr;
            
            string query = colItr->second;
            char delim = ' ';
            stringstream ss(query);
            string token;
            while (getline(ss, token, delim))
            {
                query_arr.push_back(token);
            }
            
            
            int col_index = colItr->first;

            // string col_value_min = query_arr[0];
            // string col_value_max = query_arr[1];
            // string o_col_value = col_arr[col_index];

            // int col_value_int_min = stoi(col_value_min);
            // int col_value_int_max = stoi(col_value_max);
            // int o_col_value_int = stoi(o_col_value);


            int col_value_int_min = stoi(query_arr[0]);
            int col_value_int_max = stoi(query_arr[1]);
            int o_col_value_int = stoi(col_arr[col_index]);

            //matched when in a range 
            if (o_col_value_int < col_value_int_min || o_col_value_int > col_value_int_max)
            {
                match = false;
                break; // Tham fixed Feb 2020 
            }
        }

        

        if (match)
        {
            match_query_domain_vect.push_back(domain_pair);
            counter++;
        }
    }

    // cout<<"\nNo. of matched rows: "<<counter<<endl;
}




//added by Tham 14 Dec 19 to optimize runtime
void Server::generateNormalQuery_opt(ENC_Stack &pre_enc_stack)
{
    enc_question_map.clear();
    enc_question_map = enc_question_map_pre;

    gamal_ciphertext_t encrypt_1;
    gamal_cipher_new(encrypt_1);
    pre_enc_stack.pop_E1(encrypt_1);

    int counter = 0;

    for (int i = 0; i < match_query_domain_vect.size(); i++)
    {
        id_domain_pair match_domain = match_query_domain_vect[i];
        ENC_DOMAIN_MAP::iterator find = enc_question_map.find(match_domain);
        if (find != enc_question_map.end())
        {
            counter++;
            gamal_add(find->second, find->second, encrypt_1);
        }
    }

    // cout << "Total match domains in query: " << counter << endl;
}

//Tha
//added by Tham for sum query
int Server::generateNormalQuery_sum(ENC_Stack &pre_enc_stack, int index_attr_to_sum)
{
    enc_question_map.clear();
    enc_question_map = enc_question_map_pre;

    gamal_ciphertext_t encrypt_1;
    gamal_cipher_new(encrypt_1);
    pre_enc_stack.pop_E1(encrypt_1);

    int counter = 0;

    for (int i = 0; i < match_query_domain_vect.size(); i++)
    {
        id_domain_pair match_domain = match_query_domain_vect[i];
        ENC_DOMAIN_MAP::iterator find = enc_question_map.find(match_domain);
        if (find != enc_question_map.end())
        {
            counter++;
            gamal_add(find->second, find->second, encrypt_1);
        }
    }

    index_attr_sum = index_attr_to_sum;
    return index_attr_sum;

    // cout << "Total match domains in query: " << counter << endl;
}

void Server::generateNormalQuery(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map)
{
    enc_question_map.clear();

    map<int, string> columns_map;
    _importQuery(columns_map, 0); //test_or_query = 1 to gen normal query
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
        enc_question_map.insert({domain_pair, enc_ciphertext[0]});
    }

    // cout << "Total match domains in query: " << counter << endl;
}

float Server::generatePVTestCondition(int dataset, int PV, int known_records, double eta)
{
    hypergeometric_distribution<> hyper_dist(known_records, PV, dataset);
    float result = quantile(complement(hyper_dist, eta)) + 1;
    cout << "eta value = " << eta << ", r0 = " << result << endl;
    return result;
}

//Get test answer from PV
void Server::getTestResult_fromPV(ENC_DOMAIN_MAP enc_domain_map, gamal_ciphertext_t enc_PV_answer)
{
    int counter = 0;

    gamal_ciphertext_t tmp, mul_tmp;
    gamal_cipher_new(tmp);
    gamal_cipher_new(mul_tmp);

    // cout << "Start multiply and add " << endl;
    int i = 0;
    for (ENC_DOMAIN_MAP::iterator itr = enc_domain_map.begin(); itr != enc_domain_map.end(); itr++)
    {
        string key = itr->first.first;
        string domain = itr->first.second;

        int value = plain_domain_map[{key, domain}];
        if (value == 1)
        {
             if (counter == 0)
            {
                enc_PV_answer->C1 = tmp->C1;
                enc_PV_answer->C2 = tmp->C2;
            }
            else
            {
                tmp->C1 = enc_PV_answer->C1;
                tmp->C2 = enc_PV_answer->C2;
                gamal_add(enc_PV_answer, tmp, itr->second);
            }

            counter++;
        
        }
        else if (value > 1)
        {
            gamal_mult_opt(mul_tmp, itr->second, value);

            if (counter == 0)
            {
                enc_PV_answer->C1 = mul_tmp->C1;
                enc_PV_answer->C2 = mul_tmp->C2;
            }
            else
            {
                tmp->C1 = enc_PV_answer->C1;
                tmp->C2 = enc_PV_answer->C2;
                gamal_add(enc_PV_answer, tmp, mul_tmp);
            }

            counter++;
        }
    }

    plain_domain_map.clear();
}


//Get Query answer from PV
void Server::getQueryResult_fromPV(ENC_DOMAIN_MAP enc_domain_map, gamal_ciphertext_t enc_PV_query_answer)
{
    int counter = 0;

    gamal_ciphertext_t tmp, mul_tmp;
    gamal_cipher_new(tmp);
    gamal_cipher_new(mul_tmp);

    // cout << "Start multiply and add " << endl;
    int i = 0;
    for (ENC_DOMAIN_MAP::iterator itr = enc_domain_map.begin(); itr != enc_domain_map.end(); itr++)
    {
        string key = itr->first.first;
        string domain = itr->first.second;

        int value = plain_domain_map[{key, domain}];
        if (value == 1)
        {
             if (counter == 0)
            {
                enc_PV_query_answer->C1 = tmp->C1;
                enc_PV_query_answer->C2 = tmp->C2;
            }
            else
            {
                tmp->C1 = enc_PV_query_answer->C1;
                tmp->C2 = enc_PV_query_answer->C2;
                gamal_add(enc_PV_query_answer, tmp, itr->second);
            }

            counter++;
        
        }
        else if (value > 1)
        {
            gamal_mult_opt(mul_tmp, itr->second, value);

            if (counter == 0)
            {
                enc_PV_query_answer->C1 = mul_tmp->C1;
                enc_PV_query_answer->C2 = mul_tmp->C2;
            }
            else
            {
                tmp->C1 = enc_PV_query_answer->C1;
                tmp->C2 = enc_PV_query_answer->C2;
                gamal_add(enc_PV_query_answer, tmp, mul_tmp);
            }

            counter++;
        }
    }

    plain_domain_map.clear();
}