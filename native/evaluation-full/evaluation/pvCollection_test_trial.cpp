
// #include "include/public_header.h"
// #include "src/Participant.h"
// #include "src/Server.h"
// #include "src/Servers.h"
// #include "src/process_noise.h"
// #include "src/time_evaluation.h"
// #include "public_func.h"

// #include <bits/stdc++.h> 
// #include "./src/time_evaluation.h"
// #include <algorithm> 
// #include <iostream> 
// #include <map>
// using namespace std; 

// // // to choose one string from a list of strings randomly
// // string getString(vector<string>& v) 
// // { 

// // 	// Size of the vector 
// // 	int n = v.size(); 
// // 	// int index_arr[n];

// // 	// cout<<"size of vector: "<<n<<endl;

// // 	// Generate a random number 
// // 	srand(time(NULL)); 

// // 	// Make sure the number is within 
// // 	// the index range 
// // 	int index = rand() % n; 
// // 	// cout<<"index = "<<index<<endl;

// // 	// Get random number from the vector 
// // 	string str = v[index]; 
// // 	// cout<<"num = "<<num<<endl;

// // 	// Remove the number from the vector 
// // 	swap(v[index], v[n - 1]); 
// // 	v.pop_back(); 

// // 	// Return the removed number 
// // 	return str; 
// // } 

// // // generate a permutation
// // hash_pair_map getPermutationOfHistogram(hash_pair_map  permute_map, hash_pair_map  permute_map_with_flag, vector<string> v, vector<int> flag) 
// // { 
	
	
// // 	// int index=0;
// // 	// string temp;

// // 	// while (v.size()) { 
// // 	// 	temp=getString(v);
// // 	// 	cout << temp <<endl; 
// // 	// 	string id, name;
// // 	// 	istringstream iss(temp);
// //     //     getline(iss, id, ' ');
// //     //     getline(iss, name);
		        
// //     //     permute_map.insert({make_pair(id, name), index});

// // 	// 	index++;
// // 	// } 

// // 	// return permute_map;



// //     //permutation the histogram to shuffle all the id,label with flags

// // 	int index=0;
// // 	string temp;

// // 	vector<string> v_temp = v;

// // 	while (v_temp.size()) { 
// // 		temp=getString(v_temp);
// // 		cout << "temp = " <<temp<<endl; 
		
// // 		string id, name;
// // 		istringstream iss(temp);
// //         getline(iss, id, ' ');
// //         getline(iss, name);
// // 		// iss >> name;
// //         cout<<"id = "<<id<<endl;
        
// //         cout<<"name = "<<name<<endl;

// //         cout<<"flag = "<<flag[index]<<endl;
        
// //         permute_map.insert({make_pair(id, name), index}); //keep this for un-permutation vector generation
// // 		permute_map_with_flag.insert({make_pair(id, name), flag[index]});//send this to S1: id,label and corresponding flag

// // 		index++;
// // 	} 
	
// // } 

// // 	//---------------------------------------------------------------
// // 	// high_resolution_clock::time_point t1, t2;
// // 	// int n = 8; 
// // 	// t1 = high_resolution_clock::now();
// // 	// generateRandom(n); 
// // 	// t2 = high_resolution_clock::now();
// // 	// double time_diff = duration_cast<nanoseconds>(t2 - t1).count();
// // 	// cout<<"time for permutation (ms) = "<<(time_diff)/1000000.0<<endl;

// // Driver code 
// int pvCollection(int argc, char **argv)
// { 

//     int dataset_size = 20; //make it an argument to use to determine dataset_size
// 	int number_servers = 3;	   //make it an argument use to setup number of servers
// 	int a = 2;				   //make it an argument to scale up the histogram for adding dummy
// 	double eta;				   //make it an argument use to determine r0
// 	double alpha = 0.05;
// 	double pv_ratio = 0.5;
	
// 	string dataset_directory, background_knowledge_directory;
// 	if (argc > 1)
// 	{
// 		if (strstr(argv[1], ".csv") != NULL && strstr(argv[2], ".csv") != NULL)
// 		{
// 			dataset_directory = argv[1]; //dataset is a csv file
// 			background_knowledge_directory = argv[2]; //background knolwedge is a csv file
// 			dataset_size = stoi(argv[3]); //size of dataset
// 			pv_ratio = stod(argv[4]); // pv sample rate
// 			number_servers = stoi(argv[5]);
// 			a = stoi(argv[6]);
// 			eta = stod(argv[7]); //make it an argument use to determine r0
// 			alpha = stod(argv[8]); //for estimate confidence interval of an answer
// 		}
// 		else
// 		{
// 			cout << "Please enter path to your data" << endl;
// 			return -1;
// 		}
// 	}
// 	else
// 	{
// 		cout << "Please enter path to your data" << endl;
// 		return -1;
// 	}

	
// 	int PV_size = (int)dataset_size*pv_ratio; // actual V, not the PV histogram form

	
// 	TRACK_LIST time_track_list;

// 	high_resolution_clock::time_point t1, t2;

// 	srand(time(NULL)); // for randomness 

// 	// for encryption scheme
// 	bsgs_table_t table; // lookup table for decryption
// 	gamal_init(CURVE_256_SEC); //eliptic curve
// 	gamal_init_bsgs_table(table, (dig_t)1L << 16); //table size = 2^16


//     //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
// 	//                          SETUP PHASE                                     //
// 	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

// 	// PARTICIPANT CONVERT DATASET TO HISTOGRAM
// 	Participant part_A(dataset_directory);
// 	Participant part_B;

// 	// Participant represent its dataset over a histogram of <label, value(label)>; label is one of all possible records that a dataset can take
// 	part_A.create_OriginalHistogram(dataset_size, a);
// 	// part_A.addDummy_to_Histogram(a); //for reduce overhead: limit the histogram to size of aN instead of size |D|
	

// 	int size_dataset = part_A.size_dataset;

	

// 	// SERVER SETUP COLLECTIVE KEY
// 	Servers servers(number_servers, size_dataset, background_knowledge_directory, a);

// 	servers.generateCollKey(servers.coll_key); //generate collective key for Partial View Collection Phase
// 	servers.pv_ratio = pv_ratio;

// 	// INITIALIZE CIPHERTEXT STACK FOR PARTY
// 	// part_A.initializePreStack(servers.coll_key);
	
// 	part_A.pv_ratio = pv_ratio;

	

// 	// INITIALIZE CIPHERTEXT STACK FOR SERVERS
// 	ENC_Stack pre_enc_stack(size_dataset, servers.coll_key);
// 	pre_enc_stack.initializeStack_E0(); //pre-computed enc(0)
// 	pre_enc_stack.initializeStack_E1(); //pre-compted enc(1)
		

//     //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
// 	//          PARTIAL VIEW COLLECTION                          //
// 	//++++++++++++++++++++++++++++++++++++++++++++++ ++++++++++++++++++++++++++++//

	  
// 	// insert histogram map
// 	int n = dataset_size;
// 	map<int, int>::iterator itr; 

// 	// vector<int> v(n); 
// 	vector<int> v_permute(n); 
// 	map<int,int> map_permute;
// 	map<int, int> map_data_with_flag;
// 	vector<int> trace(n);

	
// 	hash_pair_map map_v;
// 	// hash_pair_map map_v_permute;
// 	// hash_pair_map map_v_permute_to_send_flag;
// 	hash_pair_map map_v_permute_to_send;

	
// 	// string id0 = "0";
// 	// string label0 = "a";
// 	// string id1 = "1";
// 	// string label1 = "ab";
// 	// string id2 = "2";
// 	// string label2 = "abc";
// 	// string id3 = "3";
// 	// string label3 = "abcd";
// 	// string id4 = "4";
// 	// string label4 = "abcde";
// 	// string id5 = "5";
// 	// string label5 = "abcde";
// 	// string id6 = "6";
// 	// string label6 = "abcde";
// 	// string id7 = "7";
// 	// string label7 = "abcde";
// 	// string id8 = "8";
// 	// string label8 = "abcde";
// 	// string id9 = "9";
// 	// string label9 = "abcde";
	

// 	// map_v.insert({make_pair(id0,label0),1});
// 	// map_v.insert({make_pair(id1, label1), 0});
// 	// map_v.insert({make_pair(id2, label2), 1});
// 	// map_v.insert({make_pair(id3, label3), 0});
// 	// map_v.insert({make_pair(id4, label4), 1});
// 	// map_v.insert({make_pair(id5, label5), 0});
// 	// map_v.insert({make_pair(id6, label6), 1});
// 	// map_v.insert({make_pair(id7, label7), 0});
// 	// map_v.insert({make_pair(id8, label8), 1});
// 	// map_v.insert({make_pair(id9, label9), 0});

// 	//-----------------------------------------------------------------------------------//
// 	//extract the id and label from the pair<id,name> and the flag from the <count> for permutation,
// 	int it = 0;
// 	vector<string> v(dataset_size*a);
// 	vector<string> v_temp(dataset_size*a);
// 	vector<int> flag(dataset_size*a);
    
// 	cout<<"print map_v_original: "<<endl;
// 	for (hash_pair_map::iterator itr = part_A.histogram.begin(); itr != part_A.histogram.end(); ++itr)
//     {
//         cout << '\t' << itr->first.first
// 			 << '\t' << itr->first.second
//              << '\t' << itr->second << '\n'; 

// 		// cout<<"text = "<<itr->first.first + ' ' + itr->first.second<<endl;
// 		v[it] = itr->first.first + ' ' + itr->first.second;
// 		flag[it] = itr->second;
// 		it++;
//     }


// 	cout<<"print v: "<<endl;
// 	for(int i=0; i<n*a; i++)
// 	{
// 		cout<<v[i]<<"\n";
// 	}
// 	cout<<endl;

// 	cout<<"print flag vector: "<<endl;
// 	for(int i=0; i<n*a; i++)
// 	{
// 		cout<<flag[i]<<"\n";
// 	}
// 	cout<<endl;

// 	//-----------------------------------------------------------------------------------//

// 	//permutation the histogram to shuffle all the id,label with flags

// 	// int index=0;
// 	// string temp;

// 	// v_temp=v;

// 	// while (v_temp.size()) { 
// 	// 	temp=getString(v_temp);
// 	// 	cout << "temp = " <<temp<<endl; 
		
// 	// 	string id, name;
// 	// 	istringstream iss(temp);
//     //     getline(iss, id, ' ');
//     //     getline(iss, name);
// 	// 	// iss >> name;
//     //     cout<<"id = "<<id<<endl;
        
//     //     cout<<"name = "<<name<<endl;

//     //     cout<<"flag = "<<flag[index]<<endl;
        
//     //     map_v_permute.insert({make_pair(id, name), index}); //keep this for un-permutation vector generation
// 	// 	map_v_permute_to_send_flag.insert({make_pair(id, name), flag[index]});//send this to S1: id,label and corresponding flag

// 	// 	index++;
// 	// } 
//     part_A.vector_endcoded_label = v;
//     part_A.vector_flag = flag;

//     cout<<"print v in partA: "<<endl;
// 	for(int i=0; i<n*a; i++)
// 	{
// 		cout<<part_A.vector_endcoded_label[i]<<"\n";
// 	}
// 	cout<<endl;

//     // vector<string> v_permute_sort(n*a);
// 	// vector<int> match_back_to_v(n*a);
// 	vector<string> v_un_permute_sort(n*a);

//     part_A.vector_un_permute_sort = v_un_permute_sort;

//     part_A.getPermutationOfHistogram(part_A.map_v_permute, part_A.map_v_permute_to_send_flag, part_A.vector_endcoded_label, 
//                            part_A.vector_flag, part_A.vector_un_permute_sort); 



	
// 	// int i =0;
// 	// cout<<"\nmap_v_permute: "<<endl;
// 	// for (hash_pair_map::iterator itr = part_A.map_v_permute.begin(); itr != part_A.map_v_permute.end(); ++itr) { 
//     //     // cout << '\t' << itr->first.first
// 	// 	// 	<< "\t" << itr->first.second	 
//     //     //      << '\t' << itr->second << '\n'; 
// 	// 	v_permute_sort[i] = itr->first.first;//just for check  bug
// 	// 	match_back_to_v[i] = itr->second; //take this to get the un-permute vector
// 	// 	i++;
//     // }
// 	// cout << endl; 


	
// 	// cout<<"vector v_permute_sort: "<<"\n ";
// 	// for (int i =0; i<n*a; i++)
// 	// {
		
// 	// 	cout<<v_permute_sort[i]<<"\n";
// 	// }
// 	// cout<<"\n";

// 	// cout<<"vector match back "<<" ";
// 	// for (int i =0; i<n*a; i++)
// 	// {
		
// 	// 	cout<<match_back_to_v[i]<<" ";
// 	// }
// 	// cout<<"\n";

// 	// //un-permutation vector generation
// 	// for (int i=0; i<n*a; i++)
// 	// {
// 	// 	int ind = match_back_to_v[i];
// 	// 	cout<<"ind = "<<ind<<"\n";
// 	// 	v_un_permute_sort[i] = v[ind];
// 	// 	cout<<"test = "<<v_un_permute_sort[i]<<endl;
// 	// }
// 	// cout<<endl;	

// 	// cout<<"vector v_un_permute_sort: "<<" ";
// 	// for (int i =0; i<n*a; i++)
// 	// {
		
// 	// 	cout<<v_un_permute_sort[i]<<" ";
// 	// }
// 	// cout<<"\n";

// 	//-----------------------------------------------------------------------------------//
// 	// cout<<"print map_permute_to_send_with_flag:"<<endl;
// 	// // map_v_permute_to_send =  map_v_permute;
// 	// // int ii = 0;
// 	// for (hash_pair_map::iterator itr = part_A.map_v_permute_to_send_flag.begin(); itr != part_A.map_v_permute_to_send_flag.end(); ++itr) { 

// 	// 	// itr->second = flag[ii];
// 	// 	// ii++;
// 	// 	cout << '\t' << itr->first.first
// 	// 		<< "\t" << itr->first.second	 
//     //         << '\t' << itr->second << '\n'; 
//     // }

// 	// cout<<endl;

// 	//-----------------------------------------------------------------------------------//

// 	//S1 selects V 1s and encrypts all

//     int server_id = 0; // Server 1
// 	Server server1 = servers.server_vect[server_id];
//     Server server2 = servers.server_vect[server_id+1];

//     server1.pv_ratio = pv_ratio;
//     server1.size_dataset = dataset_size;

// 	// ENC_DOMAIN_MAP sample_by_server; //vector \mu selected by S1
	

// 	// gamal_key_t key;
// 	// gamal_ciphertext_t cipher1, cipher0;
// 	// dig_t res;
	
// 	// gamal_ciphertext_t *cipher = new gamal_ciphertext_t[1];

	
// 	// gamal_generate_keys(key);
// 	// gamal_encrypt(cipher1, key, 1);
// 	// gamal_encrypt(cipher0, key, 0);

// 	// gamal_decrypt(&res,key, cipher0, table);
// 	// cout<<"cpher0 decr = "<<res<<endl;

// 	// // generate vector \mu = sample_by_server map
// 	// int pv_size = n;
// 	// int pv_select [pv_size] = {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0};
// 	// int pv_count = 0;

//     server1.createPVsamplingVector(pre_enc_stack, server1.server_sample_vector_clear);
//     server1.generatePVfromPermutedHistogram(server1.PV_sample_from_permuted_map,part_A.map_v_permute_to_send_flag,
//                                                 server1.server_sample_vector_encrypted, pre_enc_stack);

//     server2.rerandomizePVSampleFromPermutedHistogram(server1.PV_sample_from_permuted_map, pre_enc_stack);
//     server2.getUnPermutePV(server2.un_permute_PV, server1.PV_sample_from_permuted_map, part_A.vector_un_permute_sort);

// 	// for (hash_pair_map::iterator itr = part_A.map_v_permute_to_send_flag.begin(); itr != part_A.map_v_permute_to_send_flag.end(); ++itr) 
// 	// { 

// 	// 	id_domain_pair domain = itr->first;
// 	// 	int domain_count = itr->second;
// 	// 	// cout<<"domain = "<<domain_count<<endl;
// 	// 	gamal_ciphertext_t *enc_position = new gamal_ciphertext_t[1];
// 	// 	if (domain_count == 1) 
// 	// 	{
					
// 	// 		if (pv_select[pv_count] == 1) 
// 	// 		{
// 	// 			gamal_encrypt(cipher1, key, 1);
// 	// 			enc_position[0]->C1 = cipher1->C1;
// 	// 			enc_position[0]->C2 = cipher1->C2;
			
// 	// 		}
// 	// 		else
// 	// 		{
// 	// 			gamal_encrypt(cipher0, key, 0);
// 	// 			enc_position[0]->C1 = cipher0->C1;
// 	// 			enc_position[0]->C2 = cipher0->C2;
				
				
// 	// 		}

// 	// 	    pv_count++;     
			
// 	// 	}
// 	// 	else 
// 	// 	{
// 	// 		gamal_encrypt(cipher0, key, 0);
// 	// 		enc_position[0]->C1 = cipher0->C1;
// 	// 		enc_position[0]->C2 = cipher0->C2;

// 	// 	}
// 	// 	// gamal_decrypt(&res, key, enc_position[0], table);	 
// 	// 	// cout<< "res before insert= " <<res<<'\n';
// 	// 	sample_by_server.insert({domain, enc_position[0]});
		
//     // }

// 	// cout<<"Print the pv sampled by S: "<<endl;

// 	// for(ENC_DOMAIN_MAP::iterator itr=sample_by_server.begin(); itr != sample_by_server.end(); ++itr)
// 	// {
// 	// 	cout << '\t' << itr->first.first
// 	// 		<< "\t" << itr->first.second<< '\t'; 

// 	// 	gamal_decrypt(&res, key, itr->second, table);	 
// 	// 	cout<< "res= " <<res<<'\n';
		
// 	// }

// 	//S2 rerandomize all ciphertexts

// 	// for(ENC_DOMAIN_MAP::iterator itr=sample_by_server.begin(); itr != sample_by_server.end(); ++itr)
// 	// {
// 	// 	gamal_ciphertext_t *tmp = new gamal_ciphertext_t[1];
// 	// 	gamal_encrypt(cipher0, key, 0);
// 	// 	tmp[0]->C1 = itr->second->C1;
// 	// 	tmp[0]->C2 = itr->second->C2;
// 	// 	gamal_add(itr->second, tmp[0], cipher0);
//     // }

// 	// cout<<"print map after S2 re-encrypt: "<<endl;

// 	// for(ENC_DOMAIN_MAP::iterator itr=sample_by_server.begin(); itr != sample_by_server.end(); ++itr)
// 	// {
// 	// 	cout << '\t' << itr->first.first
// 	// 		<< "\t" << itr->first.second<< '\t'; 

// 	// 	gamal_decrypt(&res, key, itr->second, table);	 
// 	// 	cout<< "res= " <<res<<'\n';
//     // }

// 	//----------------------------------------------------------------------
// 	// A sends un-permute vector, S2 un-permutes sample_by_server to get the final PV: after_un_permute_PV map

// 	// ENC_DOMAIN_MAP after_un_permute_PV;
// 	// int iiii=0;
// 	// string tmp_label_pv;
// 	// id_domain_pair domain_un_permute_pv;
// 	// for(ENC_DOMAIN_MAP::iterator itr=sample_by_server.begin(); itr!=sample_by_server.end(); ++itr)
// 	// {
// 	// 	// cout<<"vector_un_permute = "<< v_un_permute_sort[iiii]<<endl;
// 	// 	tmp_label_pv = part_A.vector_un_permute_sort[iiii];
// 	// 	string id_pv, name_pv;
// 	// 	istringstream iss(tmp_label_pv);
//     //     getline(iss, id_pv, ' ');
//     //     getline(iss, name_pv);
// 	// 	domain_un_permute_pv.first = id_pv;
// 	// 	domain_un_permute_pv.second = name_pv;

// 	// 	after_un_permute_PV.insert({domain_un_permute_pv, itr->second});

// 	// 	iiii++;
// 	// }

// 	// //print the after un-permute PV
// 	// cout<<"\n un-permuted PV: "<<endl;
// 	// for (ENC_DOMAIN_MAP::iterator itr = after_un_permute_PV.begin(); itr != after_un_permute_PV.end(); ++itr) 
// 	// { 

// 	// 	// itr->second = flag[ii];
// 	// 	// ii++;
// 	// 	cout << '\t' << itr->first.first
// 	// 		<< "\t" << itr->first.second<< '\t'; 	 
        
// 	// 	gamal_decrypt(&res, key, itr->second, table);	 
// 	// 	cout<< "resPV= " <<res<<'\n';
//     // }


//      //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
// 	//          PARTIAL VIEW VERIFICATION                      //
// 	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


// 	servers.decryptPV(server2.un_permute_PV, table, server_id);

// 	return 0; 
// } 

   


// //-------------------------------------------------------------------------------//
// //-------------------------------------------------------------------------------//
// //Trial functions
// // Function to return the next random number 
// int getNum(vector<int>& v) 
// { 

// 	// Size of the vector 
// 	int n = v.size(); 
// 	// int index_arr[n];

// 	// cout<<"size of vector: "<<n<<endl;

// 	// Generate a random number 
// 	srand(time(NULL)); 

// 	// Make sure the number is within 
// 	// the index range 
// 	int index = rand() % n; 
// 	// cout<<"index = "<<index<<endl;

// 	// Get random number from the vector 
// 	int num = v[index]; 
// 	// cout<<"num = "<<num<<endl;

// 	// Remove the number from the vector 
// 	swap(v[index], v[n - 1]); 
// 	v.pop_back(); 

// 	// Return the removed number 
// 	return num; 
// } 

// // Function to generate n non-repeating random numbers 
// void generateRandom(int n) 
// { 
// 	vector<int> v(n); 
// 	vector<int> permute_v(n);

// 	// Fill the vector with the values 
// 	// 1, 2, 3, ..., n 
// 	for (int i = 0; i < n; i++) 
// 		v[i] = i + 1; 

// 	// While vector has elements 
// 	// get a random number from the vector and print it 
// 	//save it in another vector
// 	int index=0;
// 	while (v.size()) { 
// 		// getNum(v);
// 		// cout << getNum(v) << " "; 
// 		permute_v[index] = getNum(v);
// 		index++;
// 	} 
// 	// cout<<"\n";
// } 

// // vector<int> getPermutationOfHistogram(vector<int> permute_v, vector<int> v) 
// // { 
	
// // 	// vector<int> permute_v(8);

// // 	// Fill the vector with the values 
// // 	// 1, 2, 3, ..., n 
// // 	// for (int i = 0; i < n; i++) 
// // 	// 	v[i] = i + 1; 

// // 	// While vector has elements 
// // 	// get a random number from the vector and print it 
// // 	//save it in another vector
// // 	int index=0;
// // 	while (v.size()) { 
// // 		// getNum(v);
// // 		// cout << getNum(v) << " "; 
// // 		permute_v[index] = getNum(v);
// // 		// cout<<permute_v[index]<<" ";
// // 		index++;
// // 	} 

// // 	return permute_v;
	
// // } 


// map<int, int> getPermutation_1(map<int, int>  permute_map, map<int, int> map_datapoint) 
// { 

// 	int index=0;
// 	int temp;
// 	int n;
// 	vector<int> v(n);
// 	map<int, int>::iterator itr; 
// 	for (itr = map_datapoint.begin(); itr != map_datapoint.end(); ++itr) { 
//         cout << '\t' << itr->first 
//              << '\t' << itr->second << '\n'; 

// 		v[index] = itr->first;
// 		cout<<"v[index] = "<<v[index];
// 		index++;
//     } 
//     cout << endl; 
// 	int i =0;
// 	// while (v.size()) { 
// 	for (itr = map_datapoint.begin(); itr != map_datapoint.end(); ++itr) { 	
// 		temp=getNum(v);
// 		cout << temp << " "; 
// 		// permute_map.insert(pair<int, int>(temp,index));
// 		permute_map.insert({temp,itr->second});
// 		// cout<<permute_v[index]<<" ";
// 		i++;
// 	} 

// 	return permute_map;
	
// } 



// map<int, int> getPermutationOfHistogram(map<int, int>  permute_map, vector<int> v) 
// { 
	
// 	// vector<int> permute_v(8);

// 	// Fill the vector with the values 
// 	// 1, 2, 3, ..., n 
// 	// for (int i = 0; i < n; i++) 
// 	// 	v[i] = i + 1; 

// 	// While vector has elements 
// 	// get a random number from the vector and print it 
// 	//save it in another vector
// 	int index=0;
// 	int temp;

// 	while (v.size()) { 
// 		temp=getNum(v);
// 		cout << temp << " "; 
// 		// permute_map.insert(pair<int, int>(temp,index));
// 		permute_map.insert({temp,index});
// 		// cout<<permute_v[index]<<" ";
// 		index++;
// 	} 

// 	return permute_map;
	
// } 


// //=======================================================================//
// // True process of pv collection
// //----------------------------------------------------------------

// 	// // insert histogram map
// 	// int n = 10;
// 	// map<int, int>::iterator itr; 

// 	// // vector<int> v(n); 
// 	// vector<int> v_permute(n); 
// 	// map<int,int> map_permute;
// 	// map<int, int> map_data_with_flag;
// 	// vector<int> trace(n);

	
// 	// hash_pair_map map_v;
// 	// hash_pair_map map_v_permute;
// 	// hash_pair_map map_v_permute_to_send_flag;
// 	// hash_pair_map map_v_permute_to_send;

	
// 	// string id0 = "0";
// 	// string label0 = "a";
// 	// string id1 = "1";
// 	// string label1 = "ab";
// 	// string id2 = "2";
// 	// string label2 = "abc";
// 	// string id3 = "3";
// 	// string label3 = "abcd";
// 	// string id4 = "4";
// 	// string label4 = "abcde";
// 	// string id5 = "5";
// 	// string label5 = "abcde";
// 	// string id6 = "6";
// 	// string label6 = "abcde";
// 	// string id7 = "7";
// 	// string label7 = "abcde";
// 	// string id8 = "8";
// 	// string label8 = "abcde";
// 	// string id9 = "9";
// 	// string label9 = "abcde";
	

// 	// map_v.insert({make_pair(id0,label0),1});
// 	// map_v.insert({make_pair(id1, label1), 0});
// 	// map_v.insert({make_pair(id2, label2), 1});
// 	// map_v.insert({make_pair(id3, label3), 0});
// 	// map_v.insert({make_pair(id4, label4), 1});
// 	// map_v.insert({make_pair(id5, label5), 0});
// 	// map_v.insert({make_pair(id6, label6), 1});
// 	// map_v.insert({make_pair(id7, label7), 0});
// 	// map_v.insert({make_pair(id8, label8), 1});
// 	// map_v.insert({make_pair(id9, label9), 0});

// 	// //-----------------------------------------------------------------------------------//
// 	// //extract the id and label from the pair<id,name> and the flag from the <count> for permutation,
// 	// int it = 0;
// 	// vector<string> v(10);
// 	// vector<string> v_temp(10);
// 	// vector<int> flag(n);
// 	// cout<<"print map_v_original: "<<endl;
// 	// for (hash_pair_map::iterator itr = map_v.begin(); itr != map_v.end(); ++itr)
//     // {
//     //     cout << '\t' << itr->first.first
// 	// 		 << '\t' << itr->first.second
//     //          << '\t' << itr->second << '\n'; 

// 	// 	// cout<<"text = "<<itr->first.first + ' ' + itr->first.second<<endl;
// 	// 	v[it] = itr->first.first + ' ' + itr->first.second;
// 	// 	flag[it] = itr->second;
// 	// 	it++;
//     // }


// 	// cout<<"print v: "<<endl;
// 	// for(int i=0; i<n; i++)
// 	// {
// 	// 	cout<<v[i]<<"\t";
// 	// }
// 	// cout<<endl;

// 	// cout<<"print flag vector: "<<endl;
// 	// for(int i=0; i<n; i++)
// 	// {
// 	// 	cout<<flag[i]<<"\t";
// 	// }
// 	// cout<<endl;

// 	// //-----------------------------------------------------------------------------------//

// 	// //permutation the histogram to shuffle all the id,label with flags

// 	// int index=0;
// 	// string temp;

// 	// v_temp=v;

// 	// while (v_temp.size()) { 
// 	// 	temp=getString(v_temp);
// 	// 	cout << "temp = " <<temp<<endl; 
// 	// 	cout<<"flag = "<<flag[index]<<endl;
// 	// 	string id, name;
// 	// 	istringstream iss(temp);
//     //     getline(iss, id, ' ');
// 	// 	iss >> name;
//     //     // cout<<"id = "<<id<<endl;
        
//     //     // cout<<"name = "<<name<<endl;
        
//     //     map_v_permute.insert({make_pair(id, name), index}); //keep this for un-permutation vector generation
// 	// 	map_v_permute_to_send_flag.insert({make_pair(id, name), flag[index]});//send this to S1: id,label and corresponding flag

// 	// 	index++;
// 	// } 



// 	// vector<string> v_permute_sort(n);
// 	// vector<int> match_back_to_v(n);
// 	// vector<string> v_un_permute_sort(n);
// 	// int i =0;
// 	// cout<<"\nmap_v_permute: "<<endl;
// 	// for (hash_pair_map::iterator itr = map_v_permute.begin(); itr != map_v_permute.end(); ++itr) { 
//     //     cout << '\t' << itr->first.first
// 	// 		<< "\t" << itr->first.second	 
//     //          << '\t' << itr->second << '\n'; 
// 	// 	v_permute_sort[i] = itr->first.first;//just for check  bug
// 	// 	match_back_to_v[i] = itr->second; //take this to get the un-permute vector
// 	// 	i++;
//     // }
// 	// cout << endl; 


	
// 	// cout<<"vector v_permute_sort: "<<" ";
// 	// for (int i =0; i<n; i++)
// 	// {
		
// 	// 	cout<<v_permute_sort[i]<<" ";
// 	// }
// 	// cout<<"\n";

// 	// cout<<"vector match back "<<" ";
// 	// for (int i =0; i<n; i++)
// 	// {
		
// 	// 	cout<<match_back_to_v[i]<<" ";
// 	// }
// 	// cout<<"\n";

// 	// //un-permutation vector generation
// 	// for (int i=0; i<n; i++)
// 	// {
// 	// 	int ind = match_back_to_v[i];
// 	// 	cout<<"ind = "<<ind<<"\t";
// 	// 	v_un_permute_sort[i] = v[ind];
// 	// 	cout<<"test = "<<v_un_permute_sort[i]<<" ";
// 	// }
// 	// cout<<endl;	

// 	// cout<<"vector v_un_permute_sort: "<<" ";
// 	// for (int i =0; i<n; i++)
// 	// {
		
// 	// 	cout<<v_un_permute_sort[i]<<" ";
// 	// }
// 	// cout<<"\n";

// 	// //-----------------------------------------------------------------------------------//
// 	// cout<<"print map_permute_to_send_with_flag:"<<endl;
// 	// // map_v_permute_to_send =  map_v_permute;
// 	// // int ii = 0;
// 	// for (hash_pair_map::iterator itr = map_v_permute_to_send_flag.begin(); itr != map_v_permute_to_send_flag.end(); ++itr) { 

// 	// 	// itr->second = flag[ii];
// 	// 	// ii++;
// 	// 	cout << '\t' << itr->first.first
// 	// 		<< "\t" << itr->first.second	 
//     //         << '\t' << itr->second << '\n'; 
//     // }

// 	// cout<<endl;

// 	// //-----------------------------------------------------------------------------------//

// 	// //S1 selects V 1s and encrypts all

// 	// ENC_DOMAIN_MAP sample_by_server; //vector \mu selected by S1
	
	

	
// 	// gamal_key_t key;
// 	// gamal_ciphertext_t cipher1, cipher0;
// 	// dig_t res;
	
// 	// // gamal_ciphertext_t *cipher = new gamal_ciphertext_t[1];

	
// 	// gamal_generate_keys(key);
// 	// gamal_encrypt(cipher1, key, 1);
// 	// gamal_encrypt(cipher0, key, 0);

// 	// gamal_decrypt(&res,key, cipher0, table);
// 	// cout<<"cpher0 decr = "<<res<<endl;

// 	// // generate vector \mu = sample_by_server map
// 	// int pv_size = 5;
// 	// int pv_select [pv_size] = {1,0,1,0,1};
// 	// int pv_count = 0;

// 	// for (hash_pair_map::iterator itr = map_v_permute_to_send_flag.begin(); itr != map_v_permute_to_send_flag.end(); ++itr) 
// 	// { 

// 	// 	id_domain_pair domain = itr->first;
// 	// 	int domain_count = itr->second;
// 	// 	// cout<<"domain = "<<domain_count<<endl;
// 	// 	gamal_ciphertext_t *enc_position = new gamal_ciphertext_t[1];
// 	// 	if (domain_count == 1) 
// 	// 	{
					
// 	// 		if (pv_select[pv_count] == 1) 
// 	// 		{
// 	// 			gamal_encrypt(cipher1, key, 1);
// 	// 			enc_position[0]->C1 = cipher1->C1;
// 	// 			enc_position[0]->C2 = cipher1->C2;
			
// 	// 		}
// 	// 		else
// 	// 		{
// 	// 			gamal_encrypt(cipher0, key, 0);
// 	// 			enc_position[0]->C1 = cipher0->C1;
// 	// 			enc_position[0]->C2 = cipher0->C2;
				
				
// 	// 		}

// 	// 	    pv_count++;     
			
// 	// 	}
// 	// 	else 
// 	// 	{
// 	// 		gamal_encrypt(cipher0, key, 0);
// 	// 		enc_position[0]->C1 = cipher0->C1;
// 	// 		enc_position[0]->C2 = cipher0->C2;

// 	// 	}
// 	// 	gamal_decrypt(&res, key, enc_position[0], table);	 
// 	// 	cout<< "res before insert= " <<res<<'\n';
// 	// 	sample_by_server.insert({domain, enc_position[0]});
		
//     // }

// 	// cout<<"Print the pv sampled by S: "<<endl;

// 	// for(ENC_DOMAIN_MAP::iterator itr=sample_by_server.begin(); itr != sample_by_server.end(); ++itr)
// 	// {
// 	// 	cout << '\t' << itr->first.first
// 	// 		<< "\t" << itr->first.second<< '\t'; 

// 	// 	gamal_decrypt(&res, key, itr->second, table);	 
// 	// 	cout<< "res= " <<res<<'\n';
		
// 	// }

// 	// //S2 rerandomize all ciphertexts

// 	// for(ENC_DOMAIN_MAP::iterator itr=sample_by_server.begin(); itr != sample_by_server.end(); ++itr)
// 	// {
// 	// 	gamal_ciphertext_t *tmp = new gamal_ciphertext_t[1];
// 	// 	gamal_encrypt(cipher0, key, 0);
// 	// 	tmp[0]->C1 = itr->second->C1;
// 	// 	tmp[0]->C2 = itr->second->C2;
// 	// 	gamal_add(itr->second, tmp[0], cipher0);
//     // }

// 	// cout<<"print map after S2 re-encrypt: "<<endl;

// 	// for(ENC_DOMAIN_MAP::iterator itr=sample_by_server.begin(); itr != sample_by_server.end(); ++itr)
// 	// {
// 	// 	cout << '\t' << itr->first.first
// 	// 		<< "\t" << itr->first.second<< '\t'; 

// 	// 	gamal_decrypt(&res, key, itr->second, table);	 
// 	// 	cout<< "res= " <<res<<'\n';
//     // }

// 	// //----------------------------------------------------------------------
// 	// // A sends un-permute vector, S2 un-permutes sample_by_server to get the final PV: after_un_permute_PV map

// 	// ENC_DOMAIN_MAP after_un_permute_PV;
// 	// int iiii=0;
// 	// string tmp_label_pv;
// 	// id_domain_pair domain_un_permute_pv;
// 	// for(ENC_DOMAIN_MAP::iterator itr=sample_by_server.begin(); itr!=sample_by_server.end(); ++itr)
// 	// {
// 	// 	cout<<"vector_un_permute = "<< v_un_permute_sort[iiii]<<endl;
// 	// 	tmp_label_pv = v_un_permute_sort[iiii];
// 	// 	string id_pv, name_pv;
// 	// 	istringstream iss(tmp_label_pv);
//     //     getline(iss, id_pv, ' ');
// 	// 	// cout<<"id= "<<id<<endl;
// 	// 	iss >> name_pv;
// 	// 	domain_un_permute_pv.first = id_pv;
// 	// 	domain_un_permute_pv.second = name_pv;

// 	// 	after_un_permute_PV.insert({domain_un_permute_pv, itr->second});

// 	// 	iiii++;
// 	// }

// 	// //print the after un-permute PV
// 	// cout<<"\n un-permuted PV: "<<endl;
// 	// for (ENC_DOMAIN_MAP::iterator itr = after_un_permute_PV.begin(); itr != after_un_permute_PV.end(); ++itr) 
// 	// { 

// 	// 	// itr->second = flag[ii];
// 	// 	// ii++;
// 	// 	cout << '\t' << itr->first.first
// 	// 		<< "\t" << itr->first.second<< '\t'; 	 
        
// 	// 	gamal_decrypt(&res, key, itr->second, table);	 
// 	// 	cout<< "resPV= " <<res<<'\n';
//     // }
//     //================================================================================