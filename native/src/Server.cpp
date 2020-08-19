#include "Server.h"

/** 
 * @file Server.cpp
 * @brief Implementation of a single Server's functions
 * @author Tham Nguyen tham.nguyen@mq.edu.au, Nam Bui, Data Ring
 * @date April 2020
*/ 

Server::Server()
{
    
}

Server::Server(int size, string known_domain_dir)
{
    gamal_generate_keys(key); //key pair
    
    size_dataset = size;

    Server::importFile(known_domain_dir);
    server_sample_vector_encrypted = new gamal_ciphertext_t[size_dataset];
    server_sample_vector_clear = new int[size_dataset];

   
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
        
    }

  
}


void Server::createEncryptedPVSamplingVector(ENC_Stack &pre_enc_stack, int* server_sample_vector_clear)
{
    
    
    int enc_types[] = {1, 0};
    int up_freq = (int)(1 / pv_ratio - 1);
    int freq[] = {1, up_freq};
    
    int PV_size = (int)size_dataset*pv_ratio; 
    
    int n = sizeof(enc_types) / sizeof(enc_types[0]);  
    
    int count_1 = 0;
    int i;

    for (i = 0; i < size_dataset; i++)
    {
        
        if (count_1 >= PV_size)
        {
            server_sample_vector_clear[i] = 0;
        }
        else
        {
            
            int test = myRand(enc_types, freq, n);
            
            server_sample_vector_clear[i] = test;
            
            if (server_sample_vector_clear[i] == 1)
                count_1++;
        }

       
    }
    if (count_1 < PV_size)
    {
        for (int i = size_dataset - 1; i >= 0; i--)
        {
            if (server_sample_vector_clear[i] != 1)
            {
                server_sample_vector_clear[i] = 1;
                count_1++;
            }
            i--;
            i--;
            if (count_1 >= PV_size)
            {
                
                break;
            }
        }
    }
    
    //========== Encrypt the vector =============== //
    int plain1 = 1, plain0 = 0;
    //among N elements: V elements of enc(1) and N-V elements of enc(0)
    for (int i = 0; i < size_dataset; i++)
    {
        if (server_sample_vector_clear[i] == 1)
        {
            pre_enc_stack.pop_E1(server_sample_vector_encrypted[i]);
        }
        else
        {
            pre_enc_stack.pop_E0(server_sample_vector_encrypted[i]);
        }
    }

      
}

void Server::generatePVfromPermutedHistogram(hash_pair_map permuted_map_from_participant, gamal_ciphertext_t *server_sample_vector_encrypted, ENC_Stack &pre_enc_stack)
{
    
    int sample_vector_index = 0; 
    gamal_ciphertext_t cipher0;
    
    for (hash_pair_map::iterator itr = permuted_map_from_participant.begin(); itr != permuted_map_from_participant.end(); ++itr) 
	{ 

		id_domain_pair domain = itr->first;
		int domain_count = itr->second;
        gamal_ciphertext_t *enc_position = new gamal_ciphertext_t[1];
		
		if (domain_count == 1) 
		{
					
            enc_position[0]->C1 = server_sample_vector_encrypted[sample_vector_index]->C1;
            enc_position[0]->C2 = server_sample_vector_encrypted[sample_vector_index]->C2;
		    sample_vector_index++;     
			
		}
		else 
		{
			pre_enc_stack.pop_E0(cipher0);
            enc_position[0]->C1 = cipher0->C1;
			enc_position[0]->C2 = cipher0->C2;

		}
		
		PV_sample_from_permuted_map.insert({domain, enc_position[0]});
		
    }


}

void Server::rerandomizePVSampleFromPermutedHistogram(ENC_DOMAIN_MAP PV_sample_from_permuted_map, ENC_Stack &pre_enc_stack)
{
    
    gamal_ciphertext_t cipher0;
    for(ENC_DOMAIN_MAP::iterator itr=PV_sample_from_permuted_map.begin(); itr != PV_sample_from_permuted_map.end(); ++itr)
	{
		gamal_ciphertext_t *tmp = new gamal_ciphertext_t[1];
		
		tmp[0]->C1 = itr->second->C1;
		tmp[0]->C2 = itr->second->C2;
        pre_enc_stack.pop_E0(cipher0);
		gamal_add(itr->second, tmp[0], cipher0);
    }

    

}


void Server::getUnPermutePV(ENC_DOMAIN_MAP PV_sample_from_permuted_map, vector<string> vector_un_permute_sort)
{
	
    int i=0;
	string tmp_label_pv;
	id_domain_pair domain_un_permute_pv;
	for(ENC_DOMAIN_MAP::iterator itr=PV_sample_from_permuted_map.begin(); itr!=PV_sample_from_permuted_map.end(); ++itr)
	{
		
		tmp_label_pv = vector_un_permute_sort[i];
		string id_pv, name_pv;
		istringstream iss(tmp_label_pv);
        getline(iss, id_pv, ' ');
        getline(iss, name_pv);
		domain_un_permute_pv.first = id_pv;
		domain_un_permute_pv.second = name_pv;

		un_permute_PV.insert({domain_un_permute_pv, itr->second});

		i++;
	}

}


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


void Server::generateTestBasedPartialView_opt(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map)
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
        itr->second = add_enc_ciphertext[0];
        
    }

    enc_question_map = enc_domain_map;

}



//To use separate file directory for normal query and test function (6 Feb 2020)
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

            cols_map.insert({col_id, value});
        }

    }
    else
    {
        
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
            cols_map.insert({col_id, value});
        }
    }
    
}


//Releasing phase
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

    
}


//generate clear test attribute for server to query the submitted partial view
void Server::generate_Test_Target_Attr_Clear(ENC_Stack &pre_enc_stack)
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

}


void Server::generateTest_Target_Attr_opt(ENC_Stack &pre_enc_stack)
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

           
            int col_value_int_min = stoi(query_arr[0]);
            int col_value_int_max = stoi(query_arr[1]);
            int o_col_value_int = stoi(col_arr[col_index]);

            //matched when in a range 
            if (o_col_value_int < col_value_int_min || o_col_value_int > col_value_int_max)
            {
                match = false;
                break; 
            }
        }

        if (match)
        {
            match_query_domain_vect.push_back(domain_pair);
            counter++;
        }
    }

}


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

    
}


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

    
}


float Server::generatePVTestCondition(int dataset, int PV, int known_records, double eta)
{
    hypergeometric_distribution<> hyper_dist(known_records, PV, dataset);
    float result = quantile(complement(hyper_dist, 1-eta)) + 1;
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


//Get Query answer from PV if release answer to real query from verified PV
void Server::getQueryResult_fromPV(ENC_DOMAIN_MAP enc_PV, gamal_ciphertext_t enc_PV_query_answer)
{
    int counter = 0;

    gamal_ciphertext_t tmp, mul_tmp;
    gamal_cipher_new(tmp);
    gamal_cipher_new(mul_tmp);

    int i = 0;
    for (ENC_DOMAIN_MAP::iterator itr = enc_PV.begin(); itr != enc_PV.end(); itr++)
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
