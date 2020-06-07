
  
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
 * @file pvCollection_Verification.cpp
 * @brief Robustness of Partial View Phase
 * @author Tham Nguyen tham.nguyen@mq.edu.au, Nam Bui, Data Ring
 * @date April 2020
*/ 
  
 int pvCollection(int argc, char **argv)
 { 
  
     int dataset_size = 20; //make it an argument to use to determine dataset_size
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
     part_A.create_OriginalHistogram(dataset_size, a);
     // part_A.addDummy_to_Histogram(a); //for reduce overhead: limit the histogram to size of aN instead of size |D|
     // part_A.print_Histogram("fake_original_Hist_keep_99K", part_A.histogram);
  
    
     //====== strategy 2: participant generate fake histogram randomly
     // float amount_true = 0.5;
     // int keep_row = int(dataset_size*amount_true); 
    //  int keep_row = 500000;
     // t1 = high_resolution_clock::now();
    //  part_A.addDummy_FakeHist_random(keep_row, a);
     // t2 = high_resolution_clock::now();
     // trackTaskPerformance(time_track_list, "Fake Dummy Histog (ms)", t1, t2);
  
     // part_A.print_Histogram("fake_Hist_keep_99K", part_A.fake_histogram);
  
     int size_dataset = part_A.size_dataset;
  
     
  
     // SERVER SETUP COLLECTIVE KEY
     int server_id = 0; // Server 1
     
     Servers servers(number_servers, size_dataset, background_knowledge_directory, a);
     Server server1 = servers.server_vect[server_id];
     Server server2 = servers.server_vect[server_id+1];
  
  
     servers.generateCollKey(servers.coll_key); //generate collective key for Partial View Collection Phase
     servers.pv_ratio = pv_ratio;
  
  
     // INITIALIZE CIPHERTEXT STACK FOR SERVERS
     ENC_Stack pre_enc_stack(size_dataset, servers.coll_key);
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
     // for (hash_pair_map::iterator itr = part_A.histogram.begin(); itr != part_A.histogram.end(); ++itr)
     // {
     //  v[it] = itr->first.first + ' ' + itr->first.second;
     //  flag[it] = itr->second;
     //  it++;
     // }
  
     for (hash_pair_map::iterator itr = part_A.histogram.begin(); itr != part_A.histogram.end(); ++itr)
     {
         v[it] = itr->first.first + ' ' + itr->first.second;
         flag[it] = itr->second;
         it++;
     }

  
     // cout<<"print v: "<<endl;
     // for(int i=0; i<n*a; i++)
     // {
     //  cout<<v[i]<<"\n";
     // }
     // cout<<endl;
  
     // cout<<"print flag vector: "<<endl;
     // for(int i=0; i<n*a; i++)
     // {
     //  cout<<flag[i]<<"\n";
     // }
     // cout<<endl;
  
     
     part_A.vector_endcoded_label = v;
     part_A.vector_flag = flag;
  
     // cout<<"print v in partA: "<<endl;
     // for(int i=0; i<n*a; i++)
     // {
     //  cout<<part_A.vector_endcoded_label[i]<<"\n";
     // }
     // cout<<endl;
  
     vector<string> v_un_permute_sort(n*a);
     
     part_A.vector_un_permute_sort = v_un_permute_sort;
  
     high_resolution_clock::time_point t1, t2;
  
     // t1 = high_resolution_clock::now();
     part_A.getPermutationOfHistogram(part_A.vector_endcoded_label, part_A.vector_flag); 
     part_A.getUnPermutationVector(part_A.vector_endcoded_label, part_A.map_v_permute);
     // part_A.getArbitraryUnPermutationVector(part_A.vector_endcoded_label, part_A.map_v_permute);
     // t2 = high_resolution_clock::now();
     // double time_diff = duration_cast<nanoseconds>(t2 - t1).count();
     // cout<<"time for permutation (ms) = "<<(time_diff)/1000000.0<<endl;    
     // trackTaskPerformance(time_track_list, "permute_Hist (ms)", t1, t2);
     // cout<<"size of permute_map_with_flag = "<<part_A.map_v_permute_to_send_flag.size()<<endl;
     // cout<<"size of permute_map = "<<part_A.map_v_permute.size()<<endl;
     // cout<<"size of v_un_permute_sort = "<<part_A.vector_un_permute_sort.size()<<endl;
  
     
  
  
     //-----------------------------------------------------------------------------------//
  
     //S1 selects V 1s and encrypts all
  
    
  
     server1.pv_ratio = pv_ratio;
     server1.size_dataset = dataset_size;
  
     // t1 = high_resolution_clock::now();
     server1.createEncryptedPVSamplingVector(pre_enc_stack, server1.server_sample_vector_clear);
     // t2 = high_resolution_clock::now();
     // time_diff = duration_cast<nanoseconds>(t2 - t1).count();
     // cout<<"createPVsamplingVector (ms) = "<<(time_diff)/1000000.0<<endl;
     // trackTaskPerformance(time_track_list, "createEncSamplVect (ms)", t1, t2); 
  
     // t1 = high_resolution_clock::now();
     server1.generatePVfromPermutedHistogram(part_A.map_v_permute_to_send_flag,
                                                 server1.server_sample_vector_encrypted, pre_enc_stack);
     // t2 = high_resolution_clock::now();
     // time_diff = duration_cast<nanoseconds>(t2 - t1).count();
     // cout<<"generatePVfromPermutedHistogram (ms) = "<<(time_diff)/1000000.0<<endl; 
     // trackTaskPerformance(time_track_list, "genPV_permute (ms)", t1, t2);
    
    
     // t1 = high_resolution_clock::now();
     server2.rerandomizePVSampleFromPermutedHistogram(server1.PV_sample_from_permuted_map, pre_enc_stack);
     // t2 = high_resolution_clock::now();
     // // time_diff = duration_cast<nanoseconds>(t2 - t1).count();
     // // cout<<"rerandomizePVSampleFromPermutedHistogram (ms) = "<<(time_diff)/1000000.0<<endl; 
     // trackTaskPerformance(time_track_list, "re-random PV (ms)", t1, t2);
     
     // t1 = high_resolution_clock::now();
     server2.getUnPermutePV(server1.PV_sample_from_permuted_map, part_A.vector_un_permute_sort);
     // t2 = high_resolution_clock::now();
     // time_diff = duration_cast<nanoseconds>(t2 - t1).count();
     // cout<<"getUnPermutePV (ms) = "<<(time_diff)/1000000.0<<endl; 
     // trackTaskPerformance(time_track_list, "getUnPermutePV (ms)", t1, t2);
  
  
     //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
     //          PARTIAL VIEW VERIFICATION                                      //
     //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
  
  
     // servers.decryptPV(server2.un_permute_PV, table, server_id);
     // t1 = high_resolution_clock::now();
     bool verify_status = servers.verifyingPV(server2.un_permute_PV, table, server_id, pre_enc_stack, eta);//all servers jointly decrypt
     // t2 = high_resolution_clock::now();
     // trackTaskPerformance(time_track_list, "Verify PV (ms)", t1, t2);
  

  
     if (argc > 1)
     {
         fstream fout;
         if (strcmp(argv[9], "1") == 0)
         {
             fout.open("./results/PVcollection_500K_eta_092.csv", ios::out | ios::trunc);
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
             fout.open("./results/PVcollection_500K_eta_092.csv", ios::out | ios::app);
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