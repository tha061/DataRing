#include "Participant.h"
#include "../public_func.h"
  
 /** 
 * @file Participant.cpp
 * @brief Implementation of Participant's functionality
 * @author Tham Nguyen tham.nguyen@mq.edu.au, Nam Bui, Data Ring
 * @date April 2020
*/  
  
 // const string Participant::DATA_DIR = "./data/unique_domains.csv";
  
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
     // fake_histogram = histogram;
     while (histogram.size() < Histo_size)
     {
         string dummy_domain = getDummyDomain();
         histogram.insert({make_pair(to_string(dummy_id), dummy_domain), 0}); //honest Histogram: dummy are bin 0s, insert to a map
         dummy_id++;
     }
  
     // original_histogram = histogram; //added by Tham 12 Feb 2020 for improving the compute answer for honest participant
 }
  
  




  
  
 // /*
 //     Honest participant adds dummy bins (all zeroes) to the original histogram
 //     -> true histogram: n bins "1" from dataset and (a-1)n bins "0" from dummy
 // */
  
  
 // void Participant::addDummy_to_Histogram(int factorSize)
 // {
 //     int domain_size = histogram.size();
 //     // cout << "Size of original histogram: " << domain_size << endl;
 //     int Histo_size = factorSize * size_dataset;
  
 //     int dummy_id = size_dataset;
 //     // fake_histogram = histogram;
 //     while (histogram.size() < Histo_size)
 //     {
 //         string dummy_domain = getDummyDomain();
 //         histogram.insert({make_pair(to_string(dummy_id), dummy_domain), 0}); //honest Histogram: dummy are bin 0s, insert to a map
 //         dummy_id++;
 //     }
  
 //     // after this function: histogram size is a*n; orig_histogram size is still n
  
 // }
  
  
  
  
 void Participant::addDummy_FakeHist_random(int keepDomainS, int factorSize)
 {
     
     // fake_histogram.clear();
     fake_histogram = histogram; //added by Tham
     int replaceDomainS = size_dataset - keepDomainS;
     // cout<<"replaceDomainS = "<<replaceDomainS<<endl;
     int Histo_size = factorSize * size_dataset;
  
     int replace_counter = 0;
     while (replace_counter < replaceDomainS)
     {
         // cout<<"do somthing"<<endl;
         int random_id = _getRandomInRange(size_dataset, Histo_size - 1);
         // cout<<"random_id_fake_hist_enc1 = "<< random_id <<endl;
         hash_pair_map::iterator find = fake_histogram.find({to_string(random_id), ""});
         if (find != fake_histogram.end() && find->second == 0)
         {
             find->second = 1;
             replace_counter++;
         }
     }
     // cout<<"check replace_counter 1 = "<<replace_counter<<endl;
     replace_counter = 0;
     while (replace_counter < replaceDomainS)
     {
         // cout<<"do somthing"<<endl;
         int random_id = _getRandomInRange(0, size_dataset - 1);
         // cout<<"random_id_fake_hist_enc0 = "<< random_id <<endl;
         hash_pair_map::iterator find = fake_histogram.find({to_string(random_id), ""});
         if (find != fake_histogram.end() && find->second == 1)
         {
             find->second = 0;
             replace_counter++;
         }
     }
     // cout<<"check replace_counter 2 = "<<replace_counter<<endl;
  
     // cout << "Total size of fake partial view histogram: " << fake_histogram.size() << endl
     //      << endl;
     // cout << "Total size of original partial view histogram: " << histogram.size() << endl
     //      << endl;
 }
  
  
 // /*
 //     Dishonest participant makes a histogram of (1+scaled_up)*n bins "1" 
 //     Fake histogram = n bins "1" from orig and scaled_up*n bins "1" from dummy
 //     NOtE: not be able to make the domain names similar with the true histogram
 // */
 void Participant::addDummy_ones_FakeHistogram(int factorSize, float adding_ones)
 {
  
     fake_histogram = histogram; 
     
     int Histo_size = factorSize * size_dataset;
  
     int replace_counter = 0;
     int adding_ones_num = int(adding_ones*size_dataset);
  
     // cout<<"num_scaled_up = "<<adding_ones_num<<endl;
  
  
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
         
          
     // cout<<"check replace_counter = "<<replace_counter<<endl;
    
     
 }
  
 void Participant::computeAnswer_opt(ENC_DOMAIN_MAP &enc_question_map, gamal_ciphertext_t sum_cipher, hash_pair_map hist, gamal_key_t &coll_key, float epsilon_i)
 {
  
     gamal_ciphertext_t tmp, mul_tmp;
     gamal_cipher_new(tmp);
     gamal_cipher_new(mul_tmp);
     
     // gamal_encrypt(mul_tmp, coll_key, 0); // added by Tham to deal with 100% fake
     gamal_encrypt(sum_cipher, coll_key, 0); // added by Tham to deal with 100% fake
     
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
     // cout << "Random noise: " << randomNoise << endl;
     int randomNoise_to_enc;
     // cout << "Party: max noise: " << maxNoise_test << endl;
     // cout << "Party: min noise: " << minNoise_test << endl;
  
    
         if (randomNoise < minNoise)
         {
             randomNoise = (int)(minNoise);                
         }
         else if (randomNoise > maxNoise)
         {
             randomNoise = (int)(maxNoise);  
     
         }
         
     
     
     
     // cout << "Random noise: " << randomNoise << endl;
  
     if(randomNoise < 0)
     {
         randomNoise_to_enc = -randomNoise;
     }
     else {
         randomNoise_to_enc = randomNoise;
     }
  
     // cout << "Random noise to enc: " << randomNoise_to_enc << endl;
     //randomNoise = 0; //to test
     gamal_ciphertext_t noiseEnc;
     gamal_cipher_new(noiseEnc);
     gamal_encrypt(noiseEnc, coll_key, randomNoise_to_enc);
  
    
     gamal_cipher_new(tmp);
     tmp->C1 = sum_cipher->C1;
     tmp->C2 = sum_cipher->C2;
     gamal_add(sum_cipher, tmp, noiseEnc); 
   
     
 }
  
  
 //Tham - compute answer for sum query
  
 void Participant::computeAnswer_sum(ENC_DOMAIN_MAP &enc_question_map, gamal_ciphertext_t sum_cipher, hash_pair_map hist, gamal_key_t &coll_key, float epsilon_i, int attr_to_sum)
 {
     //generatePV_opt
     // hash_pair_map tmp_histogram = useTruth ? histogram : fake_histogram;
  
     int counter = 0;
  
     gamal_ciphertext_t tmp, mul_tmp, tmp_mult_attr, cipher_0, cipher_1, mul_tmp2;
     gamal_cipher_new(tmp);
     gamal_cipher_new(mul_tmp);
     gamal_cipher_new(mul_tmp2);
     gamal_cipher_new(tmp_mult_attr);
     gamal_cipher_new(cipher_0);
     gamal_cipher_new(cipher_1);
     
    
     gamal_encrypt(sum_cipher, coll_key, 0); // added by Tham to deal with 100% fake
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
     // cout << "Random noise: " << randomNoise << endl;
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
  
  
 // Generate a permutation of the histogram 
 void Participant::getPermutationOfHistogram(vector<string> v, vector<int> flag) 
 { 
     //permutation the histogram to shuffle all the id,label with flags
  
     // cout<<"test permutation function ok"<<endl;
     // cout<<"getPermutationOfHistogram: start"<<endl;
     int index=0;
     string temp;
  
     vector<string> v_temp = v;
     
     while (v_temp.size()) { 
         temp=getString(v_temp);
         // cout << "temp = " <<temp<<endl; 
         
         string id, name;
         istringstream iss(temp);
         getline(iss, id, ' ');
         getline(iss, name);
         // iss >> name;
         // cout<<"id = "<<id<<endl;
         
         // cout<<"name = "<<name<<endl;
  
         // cout<<"flag = "<<flag[index]<<endl;
         
         map_v_permute.insert({make_pair(id, name), index}); //keep this for un-permutation vector generation
         // cout<<"test permutation function ok"<<endl;
         map_v_permute_to_send_flag.insert({make_pair(id, name), flag[index]});//send this to S1: id,label and corresponding flag
         // cout<<"test permutation function ok"<<endl;
  
         index++;
     } 
  
     // vector<string> v_permute_sort(map_v_permute.size());
     // vector<int> match_back_to_v(map_v_permute.size());
     // // cout<<"print map_v_permute"<<endl;
     // int i =0;
     // for (hash_pair_map::iterator itr = map_v_permute.begin(); itr != map_v_permute.end(); ++itr) { 
     //     // cout << '\t' << itr->first.first
     //  //  << "\t" << itr->first.second     
     //     //      << '\t' << itr->second << '\n'; 
     //     v_permute_sort[i] = itr->first.first;//just for check  bug
     //  match_back_to_v[i] = itr->second; //take this to get the un-permute vector
     //  i++;
     // }
  
     // // cout<<"print permute_map_with_flag"<<endl;
     // // for (hash_pair_map::iterator itr = map_v_permute_to_send_flag.begin(); itr != map_v_permute_to_send_flag.end(); ++itr) { 
     // //     cout << '\t' << itr->first.first
     // //       << "\t" << itr->first.second     
     // //          << '\t' << itr->second << '\n'; 
     // // }
  
  
     // //un-permutation vector generation
     // for (int i=0; i<map_v_permute.size(); i++)
     // {
     //  int ind = match_back_to_v[i];
     //  // cout<<"ind = "<<ind<<"\n";
     //  vector_un_permute_sort[i] = v[ind];
     //  // cout<<"test = "<<v_un_permute_sort[i]<<endl;
     // }
  
     
     // // map_v_permute.clear();
     // // v_permute_sort.clear();
     // // match_back_to_v.clear();
  
     // // cout<<"getPermutationOfHistogram: end"<<endl;
 }
     
 // Generate permutation and corresponding un-permute vector
  
 void Participant::getInversePermutationVector(vector<string> v, hash_pair_map map_v_permute) 
 { 
         
     vector<string> v_permute_sort(map_v_permute.size());
     vector<int> match_back_to_v(map_v_permute.size());
     // cout<<"print map_v_permute"<<endl;
     int i =0;
     for (hash_pair_map::iterator itr = map_v_permute.begin(); itr != map_v_permute.end(); ++itr) { 
         
         v_permute_sort[i] = itr->first.first;//just for check  bug
         match_back_to_v[i] = itr->second; //take this to get the un-permute vector
         i++;
     }
  
  
  
     //un-permutation vector generation
     for (int i=0; i<map_v_permute.size(); i++)
     {
         int ind = match_back_to_v[i];
         // cout<<"ind = "<<ind<<"\n";
         vector_un_permute_sort[i] = v[ind];
         // cout<<"test = "<<v_un_permute_sort[i]<<endl;
     }
  
    
 }
  
 // Generate permutation and arbitrary un-permute vector
  
//  void Participant::getArbitraryUnPermutationVector(vector<string> v, hash_pair_map map_v_permute) 
//  { 
         
//      vector<string> v_permute_sort(map_v_permute.size());
//      vector<int> match_back_to_v(map_v_permute.size());
//      // cout<<"print map_v_permute"<<endl;
//      // int i =0;
//      // for (hash_pair_map::iterator itr = map_v_permute.begin(); itr != map_v_permute.end(); ++itr) { 
         
//      //     v_permute_sort[i] = itr->first.first;//just for check  bug
//      //  match_back_to_v[i] = itr->second; //take this to get the un-permute vector
//      //  i++;
//      // }
  
//      for(int i=0; i< map_v_permute.size();i++)
//      {
//          match_back_to_v[i] = i;
//      }
  
  
  
//      //un-permutation vector generation
//      for (int i=0; i<map_v_permute.size(); i++)
//      {
//          // int ind = match_back_to_v[map_v_permute.size() - i];
  
//          int ind = getRandomNumber(match_back_to_v);
//          // cout<<"ind = "<<ind<<"\n";
         
//          vector_un_permute_sort[i] = v[ind];
//          // cout<<"test = "<<v_un_permute_sort[i]<<endl;
//      }
  
    
//  }
  
  
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
         // i += 1;
         // if (i > 100)
         // {
         //     break;
         // }
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
  
  
 //=============================================================================================//
  
 // modified PV generation - Mar 2020: party cannot make encrption as it does not know public key
 // void Participant::generatePV_fixed_scheme(gamal_ciphertext_t *enc_list, hash_pair_map hist, int dataset_size)
 // {
  
 // //    /** 
 // //     int counter_bin_1s = 0;
 // //     int count_bin_0s = 0;
 // //     // cout << "PV SIZE " << tmp_histogram.size() << ", VECTOR FROM SERVER SIZE " << size_dataset << endl;
 // //     int index_cipher0 = hist.size();
  
 // //     cout<<"Hist size ="<<index_cipher0<<endl;
  
 // //     gamal_ciphertext_t tmp, tmp2;
 // //     tmp->C1 = enc_list[index_cipher0-1]->C1;
 // //     tmp->C2 = enc_list[index_cipher0-1]->C2;
  
 // //     tmp2->C1 = enc_list[index_cipher0-2]->C1;
 // //     tmp2->C2 = enc_list[index_cipher0-2]->C2;
  
 // //     // cout<<"test genPV ok1\n";
 // //     for (hash_pair_map::iterator itr = hist.begin(); itr != hist.end(); ++itr)
 // //     {
         
 // //         id_domain_pair domain = itr->first;
 // //         int domain_count = itr->second;
 // //         // cout<<"domain_count = "<<domain_count<<endl;
  
 // //         gamal_ciphertext_t *mul_enc_ciphertext = new gamal_ciphertext_t[1];
  
         
 // //         if (domain_count == 0)
 // //         {
 // //             gamal_add(mul_enc_ciphertext[0], mul_enc_ciphertext[0], tmp);
 // //             count_bin_0s++;
 // //             tmp->C1 = enc_list[index_cipher0-1-count_bin_0s]->C1;
 // //             tmp->C2 = enc_list[index_cipher0-1-count_bin_0s]->C2;
 // //             // cout<<"test genPV ok2\n";
 // //         }
 // //         else if (domain_count == 1)
 // //         {
 // //             gamal_add(mul_enc_ciphertext[0], enc_list[counter_bin_1s], tmp2);
 // //             counter_bin_1s++;
 // //             // cout<<"test genPV ok3\n";
             
 // //         } 
 // //         else
 // //         {
 // //             gamal_mult_opt(mul_enc_ciphertext[0], enc_list[counter_bin_1s], domain_count);
 // //             counter_bin_1s++;
 // //             // cout<<"test genPV ok4\n";
 // //         }
         
         
 // //         enc_domain_map.insert({domain, mul_enc_ciphertext[0]});
 // //         // cout<<"test genPV ok5\n";
     
 // //     }
  
 // //     cout << "Count 1s in Hist " << counter_bin_1s << endl;
 // //     // tmp_histogram.clear();
  
 // //     **/
  
  
 //     int counter_bin_1s = 0;
 //     int count_bin_0s = 0;
 //     // cout << "PV SIZE " << tmp_histogram.size() << ", VECTOR FROM SERVER SIZE " << size_dataset << endl;
     
  
  
 //     gamal_ciphertext_t tmp, new_tmp, new_tmp2;
 //     tmp->C1 = enc_list[dataset_size + 1 - 1]->C1;
 //     tmp->C2 = enc_list[dataset_size + 1 - 1]->C2;
  
 //     gamal_cipher_new(new_tmp);
 //     // dig_t scalar = ((dig_t) rand()) % (((dig_t)1L) << 32); 
 //     dig_t scalar = ((dig_t) rand()) % (((dig_t)1L) << 20);
 //     gamal_mult_opt(new_tmp, tmp, scalar);
  
 //     // tmp2->C1 = enc_list[index_cipher0-2]->C1;
 //     // tmp2->C2 = enc_list[index_cipher0-2]->C2;
  
 //     // cout<<"test genPV ok1\n";
 //     for (hash_pair_map::iterator itr = hist.begin(); itr != hist.end(); ++itr)
 //     {
         
 //         id_domain_pair domain = itr->first;
 //         int domain_count = itr->second;
 //         // cout<<"domain_count = "<<domain_count<<endl;
  
 //         gamal_ciphertext_t *mul_enc_ciphertext = new gamal_ciphertext_t[1];
  
         
 //         if (domain_count == 0)
 //         {
 //             gamal_cipher_new(new_tmp2);
 //             // dig_t scalar = ((dig_t) rand()) % (((dig_t)1L) << 32); 
 //             dig_t scalar = ((dig_t) rand()) % (((dig_t)1L) << 20) + 1;
 //             gamal_mult_opt(new_tmp2, tmp, scalar);
 //             // gamal_add(mul_enc_ciphertext[0], mul_enc_ciphertext[0], new_tmp);
 //             mul_enc_ciphertext[0]->C1 = new_tmp2->C1;
 //             mul_enc_ciphertext[0]->C2 = new_tmp2->C2;
 //             // count_bin_0s++;
 //             // cout<<"test genPV ok2\n";
 //         }
 //         else if (domain_count == 1)
 //         {
 //             gamal_add(mul_enc_ciphertext[0], enc_list[counter_bin_1s], new_tmp);
 //             counter_bin_1s++;
 //             // cout<<"test genPV ok3\n";
             
 //         } 
 //         else
 //         {
 //             gamal_mult_opt(mul_enc_ciphertext[0], enc_list[counter_bin_1s], domain_count);
 //             counter_bin_1s++;
 //             // cout<<"test genPV ok4\n";
 //         }
         
         
 //         enc_domain_map.insert({domain, mul_enc_ciphertext[0]});
 //         // cout<<"test genPV ok5\n";
     
 //     }
  
 //     cout << "Count 1s in Hist " << counter_bin_1s << endl;
 //     // tmp_histogram.clear();
  
  
  
 // }
  
  
  
  
 // initialize ciphertext stack for participant
 // void Participant::initializePreStack(gamal_key_t coll_key)
 // {
 //     // pre_enc_stack_participant = ENC_Stack(histogram.size(), coll_key);
 //     pre_enc_stack_participant = ENC_Stack(size_dataset, coll_key);
 //     pre_enc_stack_participant.initializeStack_E0();
 //     pre_enc_stack_participant.initializeStack_E1();
 // }
  
  
  
  
 // void Participant::computeAnswer_modified(ENC_DOMAIN_MAP &enc_question_map, gamal_ciphertext_t sum_cipher, hash_pair_map hist, gamal_ciphertext_t cipher_1, float epsilon_i)  
 // {
     
 //     int counter = 0;
  
 //     gamal_ciphertext_t tmp, mul_tmp;
 //     gamal_cipher_new(tmp);
 //     gamal_cipher_new(mul_tmp);
     
     
     
 //     int count = 0;
 //     for(hash_pair_map::iterator itr = hist.begin(); itr != hist.end(); itr++)
 //     {
 //         id_domain_pair domain_pair = itr->first;
 //         int value = itr->second;
 //         ENC_DOMAIN_MAP::iterator find = enc_question_map.find(domain_pair);
  
         
  
 //         if(find != enc_question_map.end() && value > 0)
 //         {
             
             
 //             if(value == 1)
 //             {
 //                 tmp->C1 = sum_cipher->C1;
 //                 tmp->C2 = sum_cipher->C2;
                   
 //                 gamal_add(sum_cipher, tmp, find->second); 
                 
 //                 count++;    
                     
                 
 //             }
 //             else //value >= 2
 //             {
 //                 gamal_mult_opt(mul_tmp, find->second, value);
 //                 tmp->C1 = sum_cipher->C1;
 //                 tmp->C2 = sum_cipher->C2;
 //                 gamal_add(sum_cipher, tmp, mul_tmp);   
 //             }
  
             
 //         }
  
            
                 
 //     }
  
 //     //noise generation
  
 //     int randomNoise = (int)getLaplaceNoise(sensitivity, epsilon_i);
 //     // cout << "Random noise: " << randomNoise << endl;
 //     int randomNoise_to_enc;
 //     // cout << "Party: max noise: " << maxNoise_test << endl;
 //     // cout << "Party: min noise: " << minNoise_test << endl;
  
 //     if (epsilon_i == epsilon_test) 
 //     {
 //         if (randomNoise < minNoise_test)
 //         {
 //             randomNoise = (int)(minNoise_test);                
 //         }
 //         else if (randomNoise > maxNoise_test)
 //         {
 //             randomNoise = (int)(maxNoise_test);  
     
 //         }
         
 //     }
 //     else
 //     {
 //         if (randomNoise < minNoise_q)
 //         {
 //             randomNoise = (int)(minNoise_q);                
 //         }
 //          else if (randomNoise > maxNoise_test)
 //         {
 //             randomNoise = (int)(maxNoise_test);  
     
 //         }
         
 //     }
     
     
 //     // cout << "Random noise: " << randomNoise << endl;
  
 //     if(randomNoise < 0)
 //     {
 //         randomNoise_to_enc = -randomNoise;
 //     }
 //     else if(randomNoise==0)
 //     {
 //         randomNoise_to_enc=1;//fix error of scalar multi with 0
 //     }
 //     else
 //     {
 //         randomNoise_to_enc = randomNoise;
 //     }
  
 //     // cout << "Random noise to enc: " << randomNoise_to_enc << endl;
 //     //randomNoise = 0; //to test
 //     gamal_ciphertext_t noiseEnc;
 //     gamal_cipher_new(noiseEnc);
 //     gamal_mult_opt(noiseEnc, cipher_1, randomNoise_to_enc);
  
    
 //     gamal_cipher_new(tmp);
 //     tmp->C1 = sum_cipher->C1;
 //     tmp->C2 = sum_cipher->C2;
 //     gamal_add(sum_cipher, tmp, noiseEnc); //28 Jan 2020: Tham fixed the issue of negative ans because sometimes enc(ans) before add noise = enc(0)
  
     
 // }
  
  
 // void Participant::computeAnswer_scaled_up_answer(ENC_DOMAIN_MAP &enc_question_map, gamal_ciphertext_t sum_cipher, int scale_up_answer, hash_pair_map hist, gamal_key_t &coll_key, float epsilon_i)
 // {
     
 //     // hash_pair_map tmp_histogram = useTruth ? histogram : fake_histogram;
  
    
  
 //     int counter = 0;
  
 //     gamal_ciphertext_t tmp, mul_tmp;
 //     gamal_cipher_new(tmp);
 //     gamal_cipher_new(mul_tmp);
     
 //     gamal_encrypt(sum_cipher, coll_key, 0); // added by Tham to deal with 100% fake
     
 //     int count = 0;
 //     for(hash_pair_map::iterator itr = hist.begin(); itr != hist.end(); itr++)
 //     {
 //         id_domain_pair domain_pair = itr->first;
 //         int value = itr->second;
 //         ENC_DOMAIN_MAP::iterator find = enc_question_map.find(domain_pair);
  
 //         if(find != enc_question_map.end() && value > 0)
 //         {
 //             if(value == 1)
 //             {
 //                 tmp->C1 = sum_cipher->C1;
 //                 tmp->C2 = sum_cipher->C2;
 //                 gamal_add(sum_cipher, tmp, find->second); 
 //                 count++;            
 //             }
 //             else //value >= 2
 //             {
 //                 gamal_mult_opt(mul_tmp, find->second, value);
 //                 tmp->C1 = sum_cipher->C1;
 //                 tmp->C2 = sum_cipher->C2;
 //                 gamal_add(sum_cipher, tmp, mul_tmp);   
 //             }
  
             
 //         }          
 //     }
 //     tmp->C1 = sum_cipher->C1;
 //     tmp->C2 = sum_cipher->C2;
 //     gamal_mult_opt(sum_cipher, tmp, scale_up_answer);
  
  
 //     // cout<<"\nNumber of bins 1 in histogram = "<<count<<endl;
  
  
 //     //noise generation
  
 //     int randomNoise = (int)getLaplaceNoise(sensitivity, epsilon_i);
 //     // cout << "Random noise: " << randomNoise << endl;
 //     int randomNoise_to_enc;
 //     // cout << "Party: max noise: " << maxNoise_test << endl;
 //     // cout << "Party: min noise: " << minNoise_test << endl;
  
 //     if (epsilon_i == epsilon_test) 
 //     {
 //         if (randomNoise < minNoise_test)
 //         {
 //             randomNoise = (int)(minNoise_test);                
 //         }
 //         else if (randomNoise > maxNoise_test)
 //         {
 //             randomNoise = (int)(maxNoise_test);  
     
 //         }
         
 //     }
 //     else
 //     {
 //         if (randomNoise < minNoise_q)
 //         {
 //             randomNoise = (int)(minNoise_q);                
 //         }
 //          else if (randomNoise > maxNoise_test)
 //         {
 //             randomNoise = (int)(maxNoise_test);  
     
 //         }
         
 //     }
     
     
 //     // cout << "Random noise: " << randomNoise << endl;
  
 //     if(randomNoise < 0)
 //     {
 //         randomNoise_to_enc = -randomNoise;
 //     }
 //     else {
 //         randomNoise_to_enc = randomNoise;
 //     }
  
 //     // cout << "Random noise to enc: " << randomNoise_to_enc << endl;
 //     //randomNoise = 0; //to test
 //     gamal_ciphertext_t noiseEnc;
 //     gamal_cipher_new(noiseEnc);
 //     gamal_encrypt(noiseEnc, coll_key, randomNoise_to_enc);
  
    
 //     gamal_cipher_new(tmp);
 //     tmp->C1 = sum_cipher->C1;
 //     tmp->C2 = sum_cipher->C2;
 //     gamal_add(sum_cipher, tmp, noiseEnc); //28 Jan 2020: Tham fixed the issue of negative ans because sometimes enc(ans) before add noise = enc(0)
     
 //     // if (randomNoise >= 0)
 //     // {
 //     //     gamal_add(sum_cipher, tmp, noiseEnc);
 //     // }
 //     // else{
 //     //     gamal_subtract(sum_cipher, tmp, noiseEnc); //Tham fixed the issue of negative noise and enc(ans) = enc(0)
 //     // }
     
     
     
 //     // tmp_histogram.clear();
     
 // }
  
  
  
  
  
 // /*
 //     Dishonest participant strategy 1: in n original rows, participant A keeps x rows and replace
 //     n-x rows as dummy rows. Values of bin associated with n original rows (n original domains) are set to 1
 //     participant add a*n dummy rows and set their bin values to 0
 //     Then A applies the sampling vector received from S:
 //     there are still V enc(1) and a*n - V enc(0).
 //     However, with this strategy, there are only x original domains are kept intact. If S 
 //     checks for the domains associated with L known records (regardless of the bin values are enc(1)
 //     or enc(0)), 
 //     there is high chance that there is not enough L domains are kept. 
 // */
  
 // void Participant::addDummy_FakeHist(int keepDomainS, int factorSize)
 // {
 //     fake_histogram = histogram; // Participant always gen a true histo plus dummy at setup phase
 //     int domain_size = fake_histogram.size();
 //     cout << "Size of original histogram: " << domain_size << endl;
 //     int Histo_size = factorSize * size_dataset;
  
 //     int dummy_id = size_dataset;
 //     while (fake_histogram.size() < Histo_size)
 //     {
 //         string dummy_domain = getDummyDomain();
 //         fake_histogram.insert({make_pair(to_string(dummy_id), dummy_domain), 0});
 //         dummy_id++;
 //     }
  
 //     int replaceDomainS = size_dataset - keepDomainS;
 //     int counter = 0;
  
 //     hash_pair_map::iterator itr = fake_histogram.begin();
 //     for (itr; itr != fake_histogram.end(); itr++)
 //     {
 //         if (counter >= replaceDomainS)
 //         {
 //             break;
 //         }
 //         int count = itr->second;
 //         if (count > 0)
 //         {
 //             fake_histogram.erase(itr);
 //             string dummy_domain = getDummyDomain();
 //             dummy_id++;
 //             fake_histogram.insert({make_pair(to_string(dummy_id), dummy_domain), 1});
 //             counter++;
 //         }
 //     }
 //     cout << "Total size of fake partial view histogram: " << fake_histogram.size() << endl
 //          << endl;
  
     
 // }
  
 // /*
 //     Dishonest participant strategy 2: A randomly keeps x (x<n) original rows in histogram,
 //     replace the bin value of (n-x) original rows from 1 to 0. For added dummy domains,
 //     A set bin values of (n-x) dummy rows to 1. All other dummy rows have bin value of 0 in
 //     the historgram.
 //     Hence, there still n bins with 1 and (a-1)*n bins with 0.
 //     Then A applies the sampling vector to its histogram, there still V enc(1) and all other 
 //     elements in PV are with enc(0)
 //     TO DO: add randomese feature to this function.
 // */
  
  
 // /*
 //    Dishonest participant A creates PV by itself:
 //     A does not apply the sampling vector to its histogram. Instead, A randomly chooses v rows in original dataset and keeps their bin
 //    bin value of 1; all other (n-v) original rows' bin value is set to 0
 //    add in n dummy rows and choose V-v rows in these dummy and set their bin value to 1;
 //    all other dummy bins value set to 0
 //    Then participant encrypts bins' value. Hence there are still V enc(1) and 2n-V enc(0), 
 //    and participant controls which domains were included in PV.
 //    Also, participant keeps all original domains name in PV, if server wants to see if the domain
 //    of L records are in PV, the participant passes this test.
 // */
  
 // void Participant::selfCreateFakePV(int keepDomainS, int factorSize)
 // {
 //     Participant::addDummy_to_Histogram(factorSize);
    
 //     int replaceDomainS = (int)(size_dataset*pv_ratio) - keepDomainS;
 //     int Histo_size = factorSize * size_dataset;
  
 //     int replace_counter = 0;
 //     while (replace_counter < replaceDomainS)
 //     {
 //         int random_id = _getRandomInRange(size_dataset, Histo_size - 1);
 //         hash_pair_map::iterator find = fake_histogram.find({to_string(random_id), ""});
 //         if (find != fake_histogram.end() && find->second == 0)
 //         {
 //             find->second = 1;
 //             replace_counter++;
 //         }
 //     }
  
 //     replace_counter = 0;
 //     int total_replace_actual = size_dataset - keepDomainS;
 //     while (replace_counter < total_replace_actual)
 //     {
 //         int random_id = _getRandomInRange(0, size_dataset - 1);
 //         hash_pair_map::iterator find = fake_histogram.find({to_string(random_id), ""});
 //         if (find != fake_histogram.end() && find->second == 1)
 //         {
 //             find->second = 0;
 //             replace_counter++;
 //         }
 //     }
  
 //     int counter_row = 0;
 //     for (hash_pair_map::iterator itr = fake_histogram.begin(); itr != fake_histogram.end(); ++itr)
 //     {
 //         int decypt_cip = 0;
  
 //         id_domain_pair domain = itr->first;
 //         int domain_count = itr->second;
  
 //         gamal_ciphertext_t *mul_enc_ciphertext = new gamal_ciphertext_t[1];
  
 //         int track = 0;
  
 //         if (domain_count > 0)
 //         {
 //             decypt_cip = 1;
 //             pre_enc_stack_participant.pop_E1(mul_enc_ciphertext[0]);
 //             counter_row++;
 //         }
 //         else
 //         {
 //             decypt_cip = 0;
 //             pre_enc_stack_participant.pop_E0(mul_enc_ciphertext[0]);
 //         }
  
 //         enc_domain_map.insert({domain, mul_enc_ciphertext[0]});
 //         plain_domain_map.insert({domain, decypt_cip});
 //     }
  
 //     // cout << "Counter row " << counter_row << endl;
  
 //     // cout << "Total size of fake partial view histogram: " << fake_histogram.size() << endl
 //         //  << endl;
 //     // cout << "Total size of original partial view histogram: " << histogram.size() << endl
 //         //  << endl;
 // }
  
 // void Participant::selfCreateFakePV_opt(bool useTruth)
 // {
 //     hash_pair_map tmp_histogram = useTruth ? histogram : fake_histogram;
 //     int counter_row = 0;
 //     // cout << "PV SIZE " << tmp_histogram.size() << ", VECTOR FROM SERVER SIZE " << size_dataset << endl;
 //     for (hash_pair_map::iterator itr = tmp_histogram.begin(); itr != tmp_histogram.end(); ++itr)
 //     {
  
 //         id_domain_pair domain = itr->first;
 //         int domain_count = itr->second;
  
 //         gamal_ciphertext_t *enc_1 = new gamal_ciphertext_t[1];
  
 //         //using pre-generatePV to reduce runtime:
 //         if (domain_count > 0)
 //         {
             
 //             pre_enc_stack_participant.pop_E1(enc_1[0]);    
 //             enc_domain_map.insert({domain, enc_1[0]});
 //             counter_row++;
 //         }
         
  
 //     }
  
 //     // cout << "Number of enc(1) is processed: " << counter_row << endl;
 //     tmp_histogram.clear();
 // }
  
 //randomly choose v_opt bins for `1' among N true bins and (V-v_opt) bins for `1' among (a-1)N fake bins
 //v_opt = keepDomains
 // void Participant::self_create_PV_prepare(int keepDomainS, int factorSize) 
 // {
    
 //     fake_histogram = histogram;
 //     int replaceDomainS = (int)(size_dataset*pv_ratio) - keepDomainS; 
 //     int Histo_size = factorSize * size_dataset;
  
  
 //     int replace_counter = 0;
 //     while (replace_counter < replaceDomainS)
 //     {
 //         int random_id = _getRandomInRange(size_dataset, Histo_size - 1);
 //         hash_pair_map::iterator find = fake_histogram.find({to_string(random_id), ""});
 //         if (find != fake_histogram.end() && find->second == 0)
 //         {
 //             find->second = 1;
 //             replace_counter++;
 //         }
 //     }
  
     
  
 //     replace_counter = 0;
 //     int total_replace_actual = size_dataset - keepDomainS;
 //     while (replace_counter < total_replace_actual)
 //     {
 //         int random_id = _getRandomInRange(0, size_dataset - 1);
 //         hash_pair_map::iterator find = fake_histogram.find({to_string(random_id), ""});
 //         if (find != fake_histogram.end() && find->second == 1)
 //         {
 //             find->second = 0;
 //             replace_counter++;
 //         }
 //     }
  
  
 // }
  
 // after creating PV by itself, create a histogram included V bins in PV and hide all other bins (N-V) bins
 //update the fake_histogram
 // void Participant::generate_Histogram_included_self_PV(bool useTruth, int PV_size, int factorSize) 
 // {
 //     hash_pair_map tmp_histogram = useTruth ? histogram : fake_histogram;
 //     // fake_histogram = histogram;
 //     int replaceDomainS = size_dataset - PV_size;
 //     int Histo_size = factorSize * size_dataset;
  
  
 //     int replace_counter = 0;
 //     while (replace_counter < replaceDomainS)
 //     {
 //         int random_id = _getRandomInRange(size_dataset, Histo_size - 1);
 //         hash_pair_map::iterator find = fake_histogram.find({to_string(random_id), ""});
 //         if (find != fake_histogram.end() && find->second == 0)
 //         {
 //             find->second = 1;
 //             replace_counter++;
 //         }
 //     }
  
  
 // }
  
  
  
 //added by Tham to reduce runtime for PV generation 16 Dec 2019
 // void Participant::pre_process_generatePV(bool useTruth)
 // {
 //     hash_pair_map tmp_histogram = useTruth ? histogram : fake_histogram;
 //     int count =0;
 //     for (hash_pair_map::iterator itr = tmp_histogram.begin(); itr != tmp_histogram.end(); ++itr)
 //     {
  
 //         id_domain_pair domain = itr->first;
 //         int domain_count = itr->second;
  
 //         gamal_ciphertext_t *mul_enc_ciphertext = new gamal_ciphertext_t[1];
         
 //         if (domain_count == 0)
 //         {
 //             Participant::pre_enc_stack_participant.pop_E0(mul_enc_ciphertext[0]); //multiple with 0 will become ciphertext of 0 - Tham
 //             enc_domain_map.insert({domain, mul_enc_ciphertext[0]});
 //             count++;
 //         }
         
 //     }
 //     // cout << "Number of enc(0) is pre processed: " << count << endl;
  
 //     tmp_histogram.clear();
 // }
  
 // plain_track_list: 1, 0 vector encrypted from Server
 // enc_list: E1, E0 vector encrypted from Server
  
 // void Participant::generatePV(int *plain_track_list, gamal_ciphertext_t *enc_list, bool useTruth)
 // {
 //     hash_pair_map tmp_histogram = useTruth ? histogram : fake_histogram;
 //     int counter_row = 0;
 //     // cout << "PV SIZE " << tmp_histogram.size() << ", VECTOR FROM SERVER SIZE " << size_dataset << endl;
 //     for (hash_pair_map::iterator itr = tmp_histogram.begin(); itr != tmp_histogram.end(); ++itr)
 //     {
 //         int decypt_cip = 0;
  
 //         id_domain_pair domain = itr->first;
 //         int domain_count = itr->second;
  
 //         gamal_ciphertext_t *mul_enc_ciphertext = new gamal_ciphertext_t[1];
  
 //         int track = 0;
 //         if (domain_count > 0)
 //         {
 //             decypt_cip = plain_track_list[counter_row] * domain_count;
 //             gamal_mult_opt(mul_enc_ciphertext[0], enc_list[counter_row], domain_count);
 //             counter_row++;
 //         }
 //         else
 //         {
 //             decypt_cip = 0;
 //             Participant::pre_enc_stack_participant.pop_E0(mul_enc_ciphertext[0]);
 //         } 
         
 //         enc_domain_map.insert({domain, mul_enc_ciphertext[0]});
 //         plain_domain_map.insert({domain, decypt_cip});
     
 //     }
  
 //     // cout << "Counter row " << counter_row << endl;
 //     tmp_histogram.clear();
 // }
  
  
  
  
 //added by Tham to reduce runtime for PV generation 16 Dec 19
 // void Participant::generatePV_opt(gamal_ciphertext_t *enc_list, bool useTruth)
 // {
 //     hash_pair_map tmp_histogram = useTruth ? histogram : fake_histogram;
 //     int counter_row = 0;
 //     gamal_ciphertext_t tmp; // Tham added Jan 24, for improve the mult of cipher with 1
 //     // cout << "PV SIZE " << tmp_histogram.size() << ", VECTOR FROM SERVER SIZE " << size_dataset << endl;
  
     
 //     for (hash_pair_map::iterator itr = tmp_histogram.begin(); itr != tmp_histogram.end(); ++itr)
 //     {
  
 //         id_domain_pair domain = itr->first;
 //         int domain_count = itr->second;
 //         // cout<< "domain count = " << domain_count<<endl;
  
 //         gamal_ciphertext_t *mul_enc_ciphertext = new gamal_ciphertext_t[1];
  
 //         //using pre-generatePV to reduce runtime: domain = 0, already done in pre-generate
         
 //         if (domain_count == 1)
 //         {
 //             Participant::pre_enc_stack_participant.pop_E0(tmp); //take enc(0) - Tham
 //             gamal_add(mul_enc_ciphertext[0], enc_list[counter_row], tmp); //enc(0) added to make different ciphertext - Tham
 //             counter_row++;
 //         }
 //         else if (domain_count > 1)
 //         {
 //             gamal_mult_opt(mul_enc_ciphertext[0], enc_list[counter_row], domain_count);
 //             counter_row++;
  
 //         }
  
 //          enc_domain_map.insert({domain, mul_enc_ciphertext[0]});
           
         
  
 //     }
  
 //     // cout << "Number of enc(1) is processed: " << counter_row << endl;
 //     tmp_histogram.clear();
 // }
  
 // void Participant::test_cleartext()
 // {
 //     fstream fout;
 //     // Open an existing file
 //     // fout.open("./data/report.csv", ios::out | ios::trunc);
 //     // fout << "ID, DOMAIN, SUM\n";
 //     int count = 0;
 //     int sum_value;
 //     for (hash_pair_map::iterator itr = plain_domain_map.begin(); itr != plain_domain_map.end(); ++itr)
 //     {
 //         sum_value = itr->second;
 //         // if (sum_value > 0)
 //         // {
 //         count += sum_value;
 //         // Insert the data to file
 //         // fout << itr->first.first << ", " << itr->first.second << ", " << sum_value << "\n";
  
 //         // cout << "Decrypt: " << sum_value << " of key " << itr->first << endl;
 //         // }
 //     }
 //     // fout.close();
 //     cout << "Total count of chosen plaintext 1 from server: " << count << endl;
 // }
  
  
 // void Participant::computeAnswer(ENC_DOMAIN_MAP &enc_question_map, gamal_ciphertext_t sum_cipher, hash_pair_map hist, gamal_key_t &coll_key)
 // {
 //     //generatePV_opt
 //     // hash_pair_map tmp_histogram = useTruth ? histogram : fake_histogram;
  
 //     // const int size_test_map = enc_question_map.size();
 //     // gamal_ciphertext_t *enc_list = new gamal_ciphertext_t[size_test_map];
  
 //     hash_pair_map tmp_histogram = hist;
  
 //     int counter = 0;
  
 //     gamal_ciphertext_t tmp, mul_tmp;
 //     gamal_cipher_new(tmp);
 //     gamal_cipher_new(mul_tmp);
  
 //     for(hash_pair_map::iterator itr = tmp_histogram.begin(); itr != tmp_histogram.end(); itr++)
 //     {
 //         id_domain_pair domain_pair = itr->first;
 //         int value = itr->second;
 //         ENC_DOMAIN_MAP::iterator find = enc_question_map.find(domain_pair);
 //         if(find != enc_question_map.end() && value > 0)
 //         {
 //             gamal_mult_opt(mul_tmp, find->second, value);
  
 //             if (counter == 0)
 //             {
 //                 sum_cipher->C1 = mul_tmp->C1;
 //                 sum_cipher->C2 = mul_tmp->C2;
 //             }
 //             else
 //             {
 //                 tmp->C1 = sum_cipher->C1;
 //                 tmp->C2 = sum_cipher->C2;
 //                 gamal_add(sum_cipher, tmp, mul_tmp);
 //             }
  
 //             counter++;
 //         }
 //     }
  
 //     cout << "Counter " << counter << endl;
  
 //     int randomNoise = (int)getLaplaceNoise(sensitivity, epsilon);
 //     int randomNoise_to_enc;
 //     cout << "max noise: " << maxNoise << endl;
 //     cout << "min noise: " << minNoise << endl;
  
 //     if (randomNoise < minNoise)
 //     {
 //         randomNoise = (int)(minNoise);
 //     }
 //     else if (randomNoise > maxNoise)
 //     {
 //         randomNoise = (int)(maxNoise);
 //     }
  
     
 //     cout << "Random noise: " << randomNoise << endl;
  
 //     if(randomNoise < 0)
 //     {
 //         randomNoise_to_enc = -randomNoise;
 //     }
 //     else {
 //         randomNoise_to_enc = randomNoise;
 //     }
  
 //     cout << "Random noise to enc: " << randomNoise_to_enc << endl;
  
 //     //randomNoise = 0; //to test
 //     gamal_ciphertext_t noiseEnc;
 //     gamal_cipher_new(noiseEnc);
 //     gamal_encrypt(noiseEnc, coll_key, randomNoise_to_enc);
  
 //     gamal_cipher_new(tmp);
 //     tmp->C1 = sum_cipher->C1;
 //     tmp->C2 = sum_cipher->C2;
  
 //     if (randomNoise >= 0)
 //     {
 //         gamal_add(sum_cipher, tmp, noiseEnc);
 //     }
 //     else{
 //         gamal_subtract(sum_cipher, tmp, noiseEnc); //Tham fixed the issue of negative noise
 //     }
     
 //     tmp_histogram.clear();
 // }
  
  
 // void Participant::computeAnswer_use_orig_histogram(ENC_DOMAIN_MAP &enc_question_map, gamal_ciphertext_t sum_cipher, hash_pair_map hist, gamal_key_t &coll_key, float epsilon_i)
 // {
 //     //generatePV_opt
 //     // hash_pair_map tmp_histogram = useTruth ? original_histogram : fake_histogram;
  
 //     // const int size_test_map = enc_question_map.size();
 //     // gamal_ciphertext_t *enc_list = new gamal_ciphertext_t[size_test_map];
  
 //     int counter = 0;
  
 //     gamal_ciphertext_t tmp, mul_tmp;
 //     gamal_cipher_new(tmp);
 //     gamal_cipher_new(mul_tmp);
     
 //     // gamal_encrypt(mul_tmp, coll_key, 0); // added by Tham to deal with 100% fake
 //     gamal_encrypt(sum_cipher, coll_key, 0); // added by Tham to deal with 100% fake
     
 //     int count = 0;
 //     for(hash_pair_map::iterator itr = hist.begin(); itr != hist.end(); itr++)
 //     {
 //         id_domain_pair domain_pair = itr->first;
 //         int value = itr->second;
 //         ENC_DOMAIN_MAP::iterator find = enc_question_map.find(domain_pair);
  
 //         if(find != enc_question_map.end() && value > 0)
 //         {
 //             if(value == 1)
 //             {
 //                 tmp->C1 = sum_cipher->C1;
 //                 tmp->C2 = sum_cipher->C2;
 //                 gamal_add(sum_cipher, tmp, find->second); 
 //                 count++;            
 //             }
 //             else //value >= 2
 //             {
 //                 gamal_mult_opt(mul_tmp, find->second, value);
 //                 tmp->C1 = sum_cipher->C1;
 //                 tmp->C2 = sum_cipher->C2;
 //                 gamal_add(sum_cipher, tmp, mul_tmp);   
 //             }
  
             
 //         }
  
            
                 
 //     }
  
  
 //     int randomNoise = (int)getLaplaceNoise(sensitivity, epsilon_i);
 //     // cout << "Random noise: " << randomNoise << endl;
 //     int randomNoise_to_enc;
    
  
 //     if (epsilon_i == epsilon_test) 
 //     {
 //         if (randomNoise < minNoise_test)
 //         {
 //             randomNoise = (int)(minNoise_test);                
 //         }
 //         else if (randomNoise > maxNoise_test)
 //         {
 //             randomNoise = (int)(maxNoise_test);  
     
 //         }
         
 //     }
 //     else
 //     {
 //         if (randomNoise < minNoise_q)
 //         {
 //             randomNoise = (int)(minNoise_q);                
 //         }
 //          else if (randomNoise > maxNoise_test)
 //         {
 //             randomNoise = (int)(maxNoise_test);  
     
 //         }
         
 //     }
     
     
 //     // cout << "Random noise: " << randomNoise << endl;
  
 //     if(randomNoise < 0)
 //     {
 //         randomNoise_to_enc = -randomNoise;
 //     }
 //     else {
 //         randomNoise_to_enc = randomNoise;
 //     }
  
 //     // cout << "Random noise to enc: " << randomNoise_to_enc << endl;
 //     //randomNoise = 0; //to test
 //     gamal_ciphertext_t noiseEnc;
 //     gamal_cipher_new(noiseEnc);
 //     gamal_encrypt(noiseEnc, coll_key, randomNoise_to_enc);
  
    
 //     gamal_cipher_new(tmp);
 //     tmp->C1 = sum_cipher->C1;
 //     tmp->C2 = sum_cipher->C2;
 //     gamal_add(sum_cipher, tmp, noiseEnc); //28 Jan 2020: Tham fixed the issue of negative ans because sometimes enc(ans) before add noise = enc(0)
     
     
     
     
 //     // tmp_histogram.clear();
  
  
  
 // }