#include "Participant.h"
#include "../public_func.h"
  
 /** 
 * @file Participant.cpp
 * @brief Implementation of Participant's functionality
 * @author Tham Nguyen tham.nguyen@mq.edu.au, Nam Bui, Data Ring
 * @date April 2020
*/  
  
 
  
 Participant::Participant()
 {
     size_dataset = 0;
 }
  
 Participant::Participant(string data_dir)
 {
     size_dataset = 0;
     DATA_DIR = data_dir; //dataset directory
 }
  
 static int _getRandomInRange(int min, int max)
 {
     return min + (rand() % (max - min + 1));
 }
  
 string getDummyDomain()
 {
     const int MAX_COL_1 = 40000;
     const int MIN_COL_1 = 1000;
     const int MAX_COL_2 = 40000;
     const int MIN_COL_2 = 1000;
     const int MAX_COL_3 = 40000;
     const int MIN_COL_3 = 0;
     const float MAX_COL_4 = 110000000.0;
     const float MIN_COL_4 = 0.0;
     const int DISTINCT_COL_5 = 2;
     const int DISTINCT_COL_7 = 7;
     const int DISTINCT_COL_8 = 12;
     const int DISTINCT_COL_9 = 3;
     const int DISTINCT_COL_10 = 6;
  
     int col1 = _getRandomInRange(MIN_COL_1, MAX_COL_1);
     int col2 = _getRandomInRange(MIN_COL_2, MAX_COL_2);
     int col3 = _getRandomInRange(MIN_COL_3, MAX_COL_3);
     float col4 = 0.0 + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (110000000.0 - 0.0)));
     int col5 = _getRandomInRange(0, DISTINCT_COL_5 - 1);
     float col6 = 5.31 + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (30.99 - 5.31)));
     int col7 = _getRandomInRange(0, DISTINCT_COL_7 - 1);
     int col8 = _getRandomInRange(-1, 10);
     int col9 = _getRandomInRange(0, DISTINCT_COL_9 - 1);
     int col10 = _getRandomInRange(0, DISTINCT_COL_10 - 1);
  
     string dummy_domain = to_string(col1) + " " + to_string(col2) + " " + to_string(col3) + " " + to_string(col4) + " " + to_string(col5) + " " + to_string(col6) + " " + to_string(col7) + " " + to_string(col8) + " " + to_string(col9) + " " + to_string(col10);
  
     return dummy_domain;
 }
  
 // Participant Generates original Histogram
 void Participant::create_OriginalHistogram(int dataset_size, int domainCoefficient)
 {
     std::ifstream data(DATA_DIR);
     if (!data.is_open())
     {
         cout << "Original File is not defined" << endl;
         std::exit(EXIT_FAILURE);
     }
  
     int i = 0;
     std::string str;
     std::getline(data, str); // skip the first line
  
     while (!data.eof())
     {
         if (i >= dataset_size)
         {
             break;
         }
         getline(data, str);
         if (str.empty())
         {
             continue;
         }
         string id, name;
         int count;
         istringstream iss(str);
         getline(iss, id, ',');
         // cout<<"id = "<<id<<endl;
         getline(iss, name, ',');
         // cout<<"name = "<<name<<endl;
         iss >> count;
         // cout<<"count = "<<count<<endl;
  
         string id_domain = id + " " + name;
  
         histogram.insert({make_pair(id, name), count});
         i++;
     }
  
     for (hash_pair_map::iterator itr = histogram.begin(); itr != histogram.end(); ++itr)
     {
         size_dataset += itr->second;
     }
  
  
     int domain_size = histogram.size();
     // cout << "Size of original histogram: " << domain_size << endl;
     int Histo_size = domainCoefficient * size_dataset;
  
     int dummy_id = size_dataset;
     
     while (histogram.size() < Histo_size)
     {
         string dummy_domain = getDummyDomain();
         histogram.insert({make_pair(to_string(dummy_id), dummy_domain), 0}); //honest Histogram: dummy are bin 0s, insert to a map
         dummy_id++;
     }
  
     
 }
    
  
 void Participant::addDummy_FakeHist_random(int keepDomainS, int factorSize)
 {
     
     
     fake_histogram = histogram; //added by Tham
     int replaceDomainS = size_dataset - keepDomainS;
     int Histo_size = factorSize * size_dataset;
  
     int replace_counter = 0;
     while (replace_counter < replaceDomainS)
     {
         
         int random_id = _getRandomInRange(size_dataset, Histo_size - 1);
         
         hash_pair_map::iterator find = fake_histogram.find({to_string(random_id), ""});
         if (find != fake_histogram.end() && find->second == 0)
         {
             find->second = 1;
             replace_counter++;
         }
     }
     
     replace_counter = 0;
     while (replace_counter < replaceDomainS)
     {
         
         int random_id = _getRandomInRange(0, size_dataset - 1);
         
         hash_pair_map::iterator find = fake_histogram.find({to_string(random_id), ""});
         if (find != fake_histogram.end() && find->second == 1)
         {
             find->second = 0;
             replace_counter++;
         }
     }
     
 }
  
  
 
 // Dishonest participant makes a histogram of (1+scaled_up)*n bins "1" 
 
 void Participant::addDummy_ones_FakeHistogram(int factorSize, float adding_ones)
 {
  
     fake_histogram = histogram; 
     
     int Histo_size = factorSize * size_dataset;
  
     int replace_counter = 0;
     int adding_ones_num = int(adding_ones*size_dataset);
  
     int random_id = size_dataset;
     while(replace_counter < adding_ones_num)
     {
         hash_pair_map::iterator find = fake_histogram.find({to_string(random_id), ""});
         if (find != fake_histogram.end() && find->second == 0)
         {
             find->second = 1;
             replace_counter++;
             random_id++;
         }
     }
         
 }
  
 void Participant::computeAnswer_opt(ENC_DOMAIN_MAP &enc_question_map, gamal_ciphertext_t sum_cipher, hash_pair_map hist, gamal_key_t &coll_key, float epsilon_i)
 {
  
     gamal_ciphertext_t tmp, mul_tmp;
     gamal_cipher_new(tmp);
     gamal_cipher_new(mul_tmp);
     
     
     gamal_encrypt(sum_cipher, coll_key, 0); 
     
     int count = 0;
     for(hash_pair_map::iterator itr = hist.begin(); itr != hist.end(); itr++)
     {
         id_domain_pair domain_pair = itr->first;
         int value = itr->second;
         ENC_DOMAIN_MAP::iterator find = enc_question_map.find(domain_pair);
  
         if(find != enc_question_map.end() && value > 0)
         {
             if(value == 1)
             {
                 tmp->C1 = sum_cipher->C1;
                 tmp->C2 = sum_cipher->C2;
                 gamal_add(sum_cipher, tmp, find->second); 
                 count++;            
             }
             else //value >= 2
             {
                 gamal_mult_opt(mul_tmp, find->second, value);
                 tmp->C1 = sum_cipher->C1;
                 tmp->C2 = sum_cipher->C2;
                 gamal_add(sum_cipher, tmp, mul_tmp);   
             }
  
             
         }
  
            
                 
     }
    
     //noise generation
  
     int randomNoise = (int)getLaplaceNoise(sensitivity, epsilon_i);
     
     int randomNoise_to_enc;

    if (randomNoise < minNoise)
    {
        randomNoise = (int)(minNoise);                
    }
    else if (randomNoise > maxNoise)
    {
        randomNoise = (int)(maxNoise);  

    }
           
     if(randomNoise < 0)
     {
         randomNoise_to_enc = -randomNoise;
     }
     else {
         randomNoise_to_enc = randomNoise;
     }
  
     gamal_ciphertext_t noiseEnc;
     gamal_cipher_new(noiseEnc);
     gamal_encrypt(noiseEnc, coll_key, randomNoise_to_enc); //encrypt a positive noise
  
    
     gamal_cipher_new(tmp);
     tmp->C1 = sum_cipher->C1;
     tmp->C2 = sum_cipher->C2;
     gamal_add(sum_cipher, tmp, noiseEnc); 
   
     
 }
  
  
 //Compute answer for sum query
  
 void Participant::computeAnswer_sum(ENC_DOMAIN_MAP &enc_question_map, gamal_ciphertext_t sum_cipher, hash_pair_map hist, gamal_key_t &coll_key, float epsilon_i, int attr_to_sum)
 {
      
     int counter = 0;
  
     gamal_ciphertext_t tmp, mul_tmp, tmp_mult_attr, cipher_0, cipher_1, mul_tmp2;
     gamal_cipher_new(tmp);
     gamal_cipher_new(mul_tmp);
     gamal_cipher_new(mul_tmp2);
     gamal_cipher_new(tmp_mult_attr);
     gamal_cipher_new(cipher_0);
     gamal_cipher_new(cipher_1);
     
    
     gamal_encrypt(sum_cipher, coll_key, 0); 
     gamal_encrypt(cipher_0, coll_key, 0);
     gamal_encrypt(cipher_1, coll_key, 1);
     
     int count = 0;
     int attr_to_sum_value;
     for(hash_pair_map::iterator itr = hist.begin(); itr != hist.end(); itr++)
     {
  
         vector<string> col_arr;
         id_domain_pair domain_pair = itr->first;
         string domain = domain_pair.second;
         char delim = ' ';
         stringstream ss(domain);
         // cout<<"domain ="<<domain<<endl;
         string token;
         while (getline(ss, token, delim))
         {
             col_arr.push_back(token);
         }
  
         attr_to_sum_value = stoi(col_arr[attr_to_sum]);
         if (attr_to_sum_value == 0 || attr_to_sum_value == 1) 
         {
             cout<<"attr_to_sum_value = "<<attr_to_sum_value<<endl;
         }
         
  
         int value = itr->second;
         ENC_DOMAIN_MAP::iterator find = enc_question_map.find(domain_pair);
  
         if(find != enc_question_map.end() && value > 0)
         {
             if(value == 1)
             {
                 tmp->C1 = sum_cipher->C1;
                 tmp->C2 = sum_cipher->C2;
                 if (attr_to_sum_value == 0)
                 {
                     gamal_add(sum_cipher, tmp, cipher_0);
                 }
                 else if (attr_to_sum_value == 1)
                 {
                     gamal_add(sum_cipher, tmp, cipher_1);
                 }
                 else
                 {
                     gamal_mult_opt(tmp_mult_attr, find->second, attr_to_sum_value);
                     gamal_add(sum_cipher, tmp, tmp_mult_attr); 
                 }
                 
                
                 count++;            
             }
             
             else //value >= 2
             {
                 tmp->C1 = sum_cipher->C1;
                 tmp->C2 = sum_cipher->C2;
                 if (attr_to_sum_value == 0)
                 {
                     gamal_add(sum_cipher, tmp, cipher_0);
                 }
                 else if (attr_to_sum_value == 1)
                 {
                     gamal_mult_opt(mul_tmp2, find->second, value); //attr_to_sum_value = 1
                     gamal_add(sum_cipher, tmp, mul_tmp2);
                 }
                 else
                 {
                 
                     gamal_mult_opt(mul_tmp, find->second, value);
                     gamal_mult_opt(find->second, find->second, attr_to_sum_value);
                     
                     gamal_add(sum_cipher, tmp, mul_tmp);  
                 } 
             }
  
         }
  
            
                 
     }
  
  
  
     //noise generation
  
     int randomNoise = (int)getLaplaceNoise(sensitivity, epsilon_i);
     
     int randomNoise_to_enc;
     
    
    if (randomNoise < minNoise)
    {
        randomNoise = (int)(minNoise);                
    }
    else if (randomNoise > maxNoise)
    {
        randomNoise = (int)(maxNoise);  

    }
     
  
     if(randomNoise < 0)
     {
         randomNoise_to_enc = -randomNoise;
     }
     else {
         randomNoise_to_enc = randomNoise;
     }
  
     
     gamal_ciphertext_t noiseEnc;
     gamal_cipher_new(noiseEnc);
     gamal_encrypt(noiseEnc, coll_key, randomNoise_to_enc);
  
    
     gamal_cipher_new(tmp);
     tmp->C1 = sum_cipher->C1;
     tmp->C2 = sum_cipher->C2;
     gamal_add(sum_cipher, tmp, noiseEnc); 
     
 }
  
  
 // Generate a permutation of the domain 
 void Participant::getPermutationOfHistogram(vector<string> v, vector<int> flag) 
 { 
     
     int index=0;
     string temp;
  
     vector<string> v_temp = v;
     
     while (v_temp.size()) 
     { 
         temp=getString(v_temp);
         
         
         string id, name;
         istringstream iss(temp);
         getline(iss, id, ' ');
         getline(iss, name);
         
         
         map_v_permute.insert({make_pair(id, name), index}); //keep this for un-permutation vector generation
         
         map_v_permute_to_send_flag.insert({make_pair(id, name), flag[index]});//send this to S1: id,label and corresponding flag
           
         index++;
     } 
  
     
 }
     
 // Generate permutation and corresponding inverse permutation vector
  
 void Participant::getInversePermutationVector(vector<string> v, hash_pair_map map_v_permute) 
 { 
         
     vector<string> v_permute_sort(map_v_permute.size());
     vector<int> match_back_to_v(map_v_permute.size());
     
     int i =0;
     for (hash_pair_map::iterator itr = map_v_permute.begin(); itr != map_v_permute.end(); ++itr) { 
         
         v_permute_sort[i] = itr->first.first;//just for check  bug
         match_back_to_v[i] = itr->second; //take this to get the un-permute vector
         i++;
     }
  
  
  
     //inverse permutation vector generation
     for (int i=0; i<map_v_permute.size(); i++)
     {
         int ind = match_back_to_v[i];
         
         vector_un_permute_sort[i] = v[ind];
         
     }
  
    
 }
   
  
 static void _printEncData(int index, gamal_ciphertext_t *enc_list)
 {
     extern EC_GROUP *init_group;
     BIGNUM *x = BN_new();
     BIGNUM *y = BN_new();
  
     cout << "Print encryption of row index" << index << endl;
     printf("encryption of row index #%d->C1:\n", index);
     if (EC_POINT_get_affine_coordinates_GFp(init_group, enc_list[index]->C1, x, y, NULL))
     {
         BN_print_fp(stdout, x);
         putc('\n', stdout);
         BN_print_fp(stdout, y);
         putc('\n', stdout);
     }
     else
     {
         std::cerr << "Can't get point coordinates." << std::endl;
     }
     printf("\n");
     printf("encryption of row index #%d->C2:\n", index);
     if (EC_POINT_get_affine_coordinates_GFp(init_group, enc_list[index]->C2, x, y, NULL))
     {
         BN_print_fp(stdout, x);
         putc('\n', stdout);
         BN_print_fp(stdout, y);
         putc('\n', stdout);
     }
     else
     {
         std::cerr << "Can't get point coordinates." << std::endl;
     }
  
     printf("\n");
 }
  
  
  
 void Participant::print_Histogram(string filename, hash_pair_map histo)
 {
     cout << "\nPrint Histogram\n";
     int i = 0;
     hash_pair_map::iterator itr;
     
     stringstream ss;
     fstream fout;
  
     ss << filename <<".csv";
     fout.open(ss.str().c_str(),ios::out | ios::app);
  
     for (itr = histo.begin(); itr != histo.end(); ++itr)
     {
         fout << itr->first.first << "|" << itr->first.second << "|" << itr->second << endl;
     }
  
     fout.close();
 }
  
 void _printCiphertext(gamal_ciphertext_ptr ciphertext)
 {
     extern EC_GROUP *init_group;
     BIGNUM *x = BN_new();
     BIGNUM *y = BN_new();
  
     if (EC_POINT_get_affine_coordinates_GFp(init_group, ciphertext->C1, x, y, NULL))
     {
         BN_print_fp(stdout, x);
         putc('\n', stdout);
         BN_print_fp(stdout, y);
         putc('\n', stdout);
     }
     else
     {
         std::cerr << "Can't get point coordinates." << std::endl;
     }
     printf("\n");
     if (EC_POINT_get_affine_coordinates_GFp(init_group, ciphertext->C2, x, y, NULL))
     {
         BN_print_fp(stdout, x);
         putc('\n', stdout);
         BN_print_fp(stdout, y);
         putc('\n', stdout);
     }
     else
     {
         std::cerr << "Can't get point coordinates." << std::endl;
     }
  
     printf("\n");
 }
  
 