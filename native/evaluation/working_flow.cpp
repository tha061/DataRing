//  #include "include/public_header.h"
//  #include "src/Participant.h"
//  #include "src/Server.h"
//  #include "src/Servers.h"
//  #include "src/process_noise.h"
//  #include "src/time_evaluation.h"
//  #include "public_func.h"
  

#include "../include/public_header.h"
 #include "../src/Participant.h"
 #include "../src/Server.h"
 #include "../src/Servers.h"
 #include "../src/process_noise.h"
 #include "../src/time_evaluation.h"
 #include "../public_func.h"
  
 // #include <bits/stdc++.h> 
 // #include "./src/time_evaluation.h"
 #include <algorithm> 
 #include <iostream> 
 #include <map>
 using namespace std; 

/** 
 * @file working_flow.cpp
 * @brief Evaluation of system performance: computational overhead
 * @author Tham Nguyen tham.nguyen@mq.edu.au, Nam Bui, Data Ring
 * @date April 2020
*/ 
  

 int working_flow(int argc, char **argv)
 {
     
    int dataset_size = 5000; //make it an argument to use to determine dataset_size
     int number_servers = 3;    //make it an argument use to setup number of servers
     int a = 2;                 //make it an argument to scale up the histogram for adding dummy
     double eta;                //make it an argument use to determine r0
     double alpha = 0.05;
     double pv_ratio = 0.5;
     
     string dataset_directory, background_knowledge_directory;
     if (argc > 1)
     {
         if (strstr(argv[1], ".csv") != NULL && strstr(argv[2], ".csv") != NULL)
         {
             dataset_directory = argv[1]; //dataset is a csv file
             background_knowledge_directory = argv[2]; //background knolwedge is a csv file
             dataset_size = stoi(argv[3]); //size of dataset
             pv_ratio = stod(argv[4]); // pv sample rate
             number_servers = stoi(argv[5]);
             a = stoi(argv[6]);
             eta = stod(argv[7]); //make it an argument use to determine r0
             alpha = stod(argv[8]); //for estimate confidence interval of an answer
         }
         else
         {
             cout << "Please enter path to your data" << endl;
             return -1;
         }
     }
     else
     {
         cout << "Please enter path to your data" << endl;
         return -1;
     }
  
     
     int PV_size = (int)dataset_size*pv_ratio; // actual V, not the PV histogram form
  
     
     
    TRACK_LIST time_track_list;
    high_resolution_clock::time_point t1, t2;
     
     srand(time(NULL)); // for randomness 
  
     // for encryption scheme
     bsgs_table_t table; // lookup table for decryption
     gamal_init(CURVE_256_SEC); //eliptic curve
     gamal_init_bsgs_table(table, (dig_t)1L << 16); //table size = 2^16
  
  
     //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
     //                          SETUP PHASE                                     //
     //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
  
     // PARTICIPANT CONVERT DATASET TO HISTOGRAM
     Participant part_A(dataset_directory);
     Participant part_B(dataset_directory);
     part_A.pv_ratio = pv_ratio;
     // Participant represent its dataset over a histogram of <label, value(label)>; label is one of all possible records that a dataset can take
     t1 = high_resolution_clock::now();
     part_A.create_OriginalHistogram(dataset_size, a);
     t2 = high_resolution_clock::now();
     trackTaskPerformance(time_track_list, "gen_Histo (ms)", t1, t2);
     
     
      
     int size_dataset = part_A.size_dataset;
  
     
  
     // SERVER SETUP COLLECTIVE KEY
     int server_id = 0; // Server 1
     
     Servers servers(number_servers, size_dataset, background_knowledge_directory, a);
     Server server1 = servers.server_vect[server_id];
     Server server2 = servers.server_vect[server_id+1];
  
     t1 = high_resolution_clock::now();
     servers.generateCollKey(servers.coll_key); //generate collective key for Partial View Collection Phase
     t2 = high_resolution_clock::now();
     trackTaskPerformance(time_track_list, "gen_coll_PK (ms)", t1, t2);
     servers.pv_ratio = pv_ratio;
  
  
     // INITIALIZE CIPHERTEXT STACK FOR SERVERS
     ENC_Stack pre_enc_stack(4*size_dataset, servers.coll_key);
     pre_enc_stack.initializeStack_E0(); //pre-computed enc(0)
     pre_enc_stack.initializeStack_E1(); //pre-compted enc(1)
         
  
     //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
     //          PARTIAL VIEW COLLECTION                                        //
     //++++++++++++++++++++++++++++++++++++++++++++++ ++++++++++++++++++++++++++++//
  
       
     int n = dataset_size;
  
  
     //-----------------------------------------------------------------------------------//
     //extract the id and label from the pair<id,name> and the flag from the <count> for permutation,
     
     vector<string> v(dataset_size*a);
     vector<string> v_temp(dataset_size*a);
     vector<int> flag(dataset_size*a);
     
     
     int it = 0;
     
  
     for (hash_pair_map::iterator itr = part_A.histogram.begin(); itr != part_A.histogram.end(); ++itr)
     {
         v[it] = itr->first.first + ' ' + itr->first.second;
         flag[it] = itr->second;
         it++;
     }

  
     
     
     part_A.vector_endcoded_label = v;
     part_A.vector_flag = flag;
  
     
  
     vector<string> v_un_permute_sort(n*a);
     
     part_A.vector_un_permute_sort = v_un_permute_sort;
  
     
  
     t1 = high_resolution_clock::now();
     part_A.getPermutationOfHistogram(part_A.vector_endcoded_label, part_A.vector_flag); 
     t2 = high_resolution_clock::now();
     trackTaskPerformance(time_track_list, "gen_permute_labs (ms)", t1, t2);

     t1 = high_resolution_clock::now();
     part_A.getInversePermutationVector(part_A.vector_endcoded_label, part_A.map_v_permute);
     t2 = high_resolution_clock::now();
     trackTaskPerformance(time_track_list, "gen_inverse_permute (ms)", t1, t2);
  
     
  
  
     //-----------------------------------------------------------------------------------//
  
    
  
     server1.pv_ratio = pv_ratio;
     server1.size_dataset = dataset_size;
  
     t1 = high_resolution_clock::now();
     server1.createEncryptedPVSamplingVector(pre_enc_stack, server1.server_sample_vector_clear);
     t2 = high_resolution_clock::now();
     trackTaskPerformance(time_track_list, "create_PV_vector (ms)", t1, t2);
  
     t1 = high_resolution_clock::now();
     server1.generatePVfromPermutedHistogram(part_A.map_v_permute_to_send_flag,
                                                 server1.server_sample_vector_encrypted, pre_enc_stack);
     t2 = high_resolution_clock::now();
     trackTaskPerformance(time_track_list, "genPV_from_labs (ms)", t1, t2);
    
    
    //  t1 = high_resolution_clock::now();
    //  server2.rerandomizePVSampleFromPermutedHistogram(server1.PV_sample_from_permuted_map, pre_enc_stack);
    //  t2 = high_resolution_clock::now();
    //  trackTaskPerformance(time_track_list, "re-random PV (ms)", t1, t2);
     
     t1 = high_resolution_clock::now();
     server2.getUnPermutePV(server1.PV_sample_from_permuted_map, part_A.vector_un_permute_sort);
     t2 = high_resolution_clock::now();
    
     trackTaskPerformance(time_track_list, "getUnPermutePV (ms)", t1, t2);
  
  
     //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
     //          PARTIAL VIEW VERIFICATION                                      //
     //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
  
  
     t1 = high_resolution_clock::now();
     bool verify_status = servers.verifyingPV(server2.un_permute_PV, table, server_id, pre_enc_stack, eta);//all servers jointly decrypt
     t2 = high_resolution_clock::now();
     trackTaskPerformance(time_track_list, "Verify PV (ms)", t1, t2);
  
     
     double percentile_noise = 0.95;             //use to determine Laplace max_noise
     float noise_budget = 1.5;
     float sensitivity = 1.0;
     float epsilon_q = noise_budget/20; //if there are 20 questions
     float epsilon_test = epsilon_q; //if there are 20 questions
     float epsilon = noise_budget/20;  //if there are 20 questions
  
     //Server determines maxNoise
     servers.maxNoise = getLaplaceNoiseRange(sensitivity, epsilon_test, percentile_noise);
     servers.minNoise = -servers.maxNoise;
  
  
     //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
     //                          QUERY & TEST PHASE                              //
     //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
  
     bool test_status;
     int threshold;
     gamal_ciphertext_t sum_cipher;
     
     //Server determines maxNoise
     servers.maxNoise = getLaplaceNoiseRange(sensitivity, epsilon, percentile_noise);
     servers.minNoise = -servers.maxNoise;
  
     //Servers generate a collective key pair for Query Evaluation Phase
     // servers.generateCollKey(); 
  
     //====TEST L: TEST FUNCTION KNOWN RECORDS NEW USING PRE_COMPUTE TEST FUCTION =====//
     
     // SERVER: Pre-compute test L
  
     server1.prepareTestFuntion_Query_Vector(pre_enc_stack, server2.un_permute_PV);
  
     //SERVER: gen test L
     t1 = high_resolution_clock::now();
     server1.generateTestKnownRecords_opt(pre_enc_stack, server2.un_permute_PV);
     t2 = high_resolution_clock::now();
     trackTaskPerformance(time_track_list, "gen_test_L (ms)", t1, t2);
  
     //PARTY compute answer:
     gamal_cipher_new(sum_cipher);
     t1 = high_resolution_clock::now();
     part_A.computeAnswer_opt(server1.enc_question_map, sum_cipher, part_A.histogram, servers.coll_key, epsilon);
     t2 = high_resolution_clock::now();
     trackTaskPerformance(time_track_list, "com_ans_test_L (ms)", t1, t2);
  
     // SERVER: verify test's answer by comparing it with L
     
     threshold = server1.known_record_subset.size();
    //  cout<<"threshold L = "<<threshold;
     t1 = high_resolution_clock::now();
    //  test_status = servers.verifyingTestResult("Test target L:", sum_cipher, table, server_id, threshold);
    test_status = servers.verifyingTestResult(sum_cipher, table, server_id, threshold);
     t2 = high_resolution_clock::now();
     trackTaskPerformance(time_track_list, "verify_ans_L (ms)", t1, t2);
     trackTaskStatus(time_track_list, "Test target L status", test_status);
     server1.enc_question_map.clear();
  
  
  
     // //===== TEST FUNCTION BASED PV OPTIMAL =====
  
     //SERVER gen test V:
     t1 = high_resolution_clock::now();
     server1.generateTestBasedPartialView_opt(pre_enc_stack, server2.un_permute_PV);
     t2 = high_resolution_clock::now();
     trackTaskPerformance(time_track_list, "gen_test_V (ms)", t1, t2);

     // PARTY compute answer
     gamal_cipher_new(sum_cipher);
     t1 = high_resolution_clock::now();
     part_A.computeAnswer_opt(server1.enc_question_map, sum_cipher, part_A.histogram, servers.coll_key, epsilon);
     t2 = high_resolution_clock::now();
     trackTaskPerformance(time_track_list, "com_ans_test_V (ms)", t1, t2);
  
     //SERVER: verify test's answer by comparing it with V
     
     threshold = PV_size; //actual PV size = V (not the PV histogram)
    //  cout<<"threshold V = "<<threshold;
     t1 = high_resolution_clock::now();
    //  test_status = servers.verifyingTestResult("Test target V:", sum_cipher, table, server_id, threshold);
     test_status = servers.verifyingTestResult(sum_cipher, table, server_id, threshold);
     t2 = high_resolution_clock::now();
     trackTaskPerformance(time_track_list, "verify_ans_V (ms)", t1, t2);
     trackTaskStatus(time_track_list, "Test target V status", test_status);
     server1.enc_question_map.clear();
  
     
     
     //====== TEST estimation: targeting specific attributes ==========//
  
     //SERVER gen test
    //  server1.prepareTestFuntion_Query_Vector(pre_enc_stack, server2.un_permute_PV);
     
    //  server1.generateMatchDomain(1); // 1: for test function; 0: for normal query
    
    // t1 = high_resolution_clock::now();
    // server1.generateTest_Target_Attr_opt(pre_enc_stack);
    // t2 = high_resolution_clock::now();
    // trackTaskPerformance(time_track_list, "gen_test_attr (ms)", t1, t2);
    //  //PARTY compute answer:
     
    //  gamal_cipher_new(sum_cipher);
    //  t1 = high_resolution_clock::now();
    //  part_A.computeAnswer_opt(server1.enc_question_map, sum_cipher, part_A.histogram, servers.coll_key, epsilon);
    //  t2 = high_resolution_clock::now();
    //  trackTaskPerformance(time_track_list, "com_ans_V (ms)", t1, t2);
    //  //SERVER verify test's answer by comparing it with estimated answer based on PV
     
    //  gamal_ciphertext_t enc_PV_answer;
    //  gamal_cipher_new(enc_PV_answer);

    //  t1 = high_resolution_clock::now();
    //  server1.generate_Test_Target_Attr_Clear(pre_enc_stack);  
    //  server1.getTestResult_fromPV(server2.un_permute_PV, enc_PV_answer);
    //  test_status = servers.verifyingTestResult_Estimate("Test attr found:", sum_cipher, table, server_id, enc_PV_answer, alpha);
    //  t2 = high_resolution_clock::now();
    //  trackTaskPerformance(time_track_list, "verify_ans_test_attr (ms)", t1, t2);
     
    //  server1.enc_question_map.clear();

     // Test target N
    t1 = high_resolution_clock::now();
    server1.generateTest_Target_All_Records(pre_enc_stack, server2.un_permute_PV);
    t2 = high_resolution_clock::now();
    trackTaskPerformance(time_track_list, "gen_test_N (ms)", t1, t2);

    gamal_cipher_new(sum_cipher);
    t1 = high_resolution_clock::now();
    part_A.computeAnswer_opt(server1.enc_question_map, sum_cipher, part_A.histogram, servers.coll_key, epsilon);
    t2 = high_resolution_clock::now();
    trackTaskPerformance(time_track_list, "com_ans_test_N (ms)", t1, t2);

    threshold = size_dataset;
    // cout<<"threshold N = "<<threshold<<endl;
    t1 = high_resolution_clock::now();
    // test_status = servers.verifyingTestResult("Test target N:", sum_cipher, table, server_id, threshold);
    test_status = servers.verifyingTestResult(sum_cipher, table, server_id, threshold);
    t2 = high_resolution_clock::now();
    trackTaskPerformance(time_track_list, "verify_ans_test_N (ms)", t1, t2);
    trackTaskStatus(time_track_list, "Test target N status", test_status);


    // //==== NORMAL QUERY PRE_COMPUTE TO OPTIMIZE RUNTIME ============//
  
     // SERVER gen query
     server1.prepareTestFuntion_Query_Vector(pre_enc_stack, server2.un_permute_PV);
     
     server1.generateMatchDomain(0); // 1: for test function; 0: for normal query
    t1 = high_resolution_clock::now();
    server1.generateNormalQuery_opt(pre_enc_stack);
    t2 = high_resolution_clock::now();
    trackTaskPerformance(time_track_list, "gen_normal_query (ms)", t1, t2);
     // PARTY compute query's answer
     gamal_cipher_new(sum_cipher);

    t1 = high_resolution_clock::now();
    part_A.computeAnswer_opt(server1.enc_question_map, sum_cipher, part_A.histogram, servers.coll_key, epsilon);
    t2 = high_resolution_clock::now();
    trackTaskPerformance(time_track_list, "com_ans_normal_query (ms)", t1, t2);  
  
  
     //======Re-encrypt query answer to participant B's public key
  
    //  gamal_ciphertext_t sum_cipher_update;
  
    //  gamal_generate_keys(part_B.keys); //part_B keys pair   
  
    //  t1 = high_resolution_clock::now();
    //  gama_key_switch_lead(sum_cipher_update, sum_cipher, server1.key, part_B.keys);
  
    //  for (int i=1; i< number_servers; i++)
    //  {
    //      gama_key_switch_follow(sum_cipher_update, sum_cipher, servers.server_vect[server_id+i].key, part_B.keys);
    //  }
    //  t2 = high_resolution_clock::now();
    // trackTaskPerformance(time_track_list, "re_encryption (ms)", t1, t2);  
  
    //  dig_t after;

    //  t1 = high_resolution_clock::now();
    //  gamal_decrypt(&after, part_B.keys, sum_cipher_update, table);   
    //  t2 = high_resolution_clock::now();
    //  trackTaskPerformance(time_track_list, "decryption (ms)", t1, t2);  
    //  std::cout<<"Check after re-encryption: "<<after<<std::endl;
  
  
     //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
     //                          FINISHED SHARING                               //
     //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
     
    if (argc > 1)
     {
         fstream fout;
         // std::ofstream fout;
  
         string filename = "./results/runtime_cost_";
     
         stringstream ss;
     
         if (strcmp(argv[9], "1") == 0)
         {
             
             ss << filename <<"dataset_"<<dataset_size <<"_PV_size_"<<PV_size<<".csv"; // add your stuff to the stream
             fout.open(ss.str().c_str(),ios::out | ios::trunc);
         
             fout << "Iteration, PV Verification";
             for (auto itr = time_track_list.begin(); itr != time_track_list.end(); itr++)
             {
                 string column = itr->first;
                 fout << ", " << column;
             }
             fout << "\n";
         }
         else
         {
             
             ss << filename <<"dataset_"<<dataset_size <<"_PV_size_"<<PV_size<<".csv";
              
             fout.open(ss.str().c_str(),ios::out | ios::app);
             
         }
  
         // Insert the data to file
         fout << argv[9] << ", " << verify_status;
         for (auto itr = time_track_list.begin(); itr != time_track_list.end(); itr++)
         {
             string time_diff = itr->second;
             fout << ", " << time_diff;
         }
         fout << "\n";
         fout.close();
     }
  
     return 0;
 }