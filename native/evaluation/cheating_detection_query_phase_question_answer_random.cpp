
#include "../include/public_header.h"
#include "../src/Participant.h"
#include "../src/Server.h"
#include "../src/Servers.h"
#include "../src/process_noise.h"
#include "../src/time_evaluation.h"
#include "../public_func.h"

/** 
 * @file cheating_detection_query_phase_question_answer_random.cpp
 * @brief Evaluation of the cheating detection in Query Evaluation Phase
 * @author Tham Nguyen tham.nguyen@mq.edu.au, Nam Bui, Data Ring
 * @date April 2020
*/ 

int query_evaluation_with_cheating_detection_random_query_ans(int argc, char **argv)
{
	int dataset_size = 500000; //make it an argument to use to determine dataset_size
	int number_servers = 3;	   //make it an argument use to setup number of servers
	int a = 2;				   //make it an argument to scale up the histogram for adding dummy
	double eta;				   //make it an argument use to determine r0
	double alpha = 0.05;
	double pv_ratio = 0.01;
    int num_query;
    double test_frequency;
	int fre_lie;
	
	string dataset_directory, background_knowledge_directory;
	if (argc > 1)
	{
		if (strstr(argv[1], ".csv") != NULL && strstr(argv[2], ".csv") != NULL)
		{
			dataset_directory = argv[1];
			background_knowledge_directory = argv[2];
			dataset_size = stoi(argv[3]); //size of dataset
			pv_ratio = stod(argv[4]); // pv sample rate
			number_servers = stoi(argv[5]);
			a = stoi(argv[6]);
			eta = stod(argv[7]); //make it an argument use to determine r0
			alpha = stod(argv[8]);
            num_query = stoi(argv[9]);
            test_frequency = stod(argv[10]);
			fre_lie = stoi(argv[11]);
			

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

	double percentile_noise = 0.95;			   //use to determine Laplace max_noise
	float noise_budget_server = 0.5;
	float noise_budget_party = 0.5;
	float sensitivity = 1.0; //count query
	int PV_size = (int)dataset_size*pv_ratio; // actual V, not the PV histogram form

    int num_test = (int)(num_query*test_frequency/(1-test_frequency));
    int total_queries = num_query + num_test;

	
	
	float epsilon = noise_budget_party/num_query;  //As num_test <= number_query, party adds same noise to all answers
	


	double num_test_each = (double)num_test/4; // 4 test function types
	int num_test_each_type_rounded = ceil(num_test_each);
	cout << "num_test_each_type = "<< num_test_each_type_rounded <<endl;
	// cout << "num_test = " << num_test_each <<endl;
	
	TRACK_LIST time_track_list;

	high_resolution_clock::time_point t1, t2;

	srand(time(NULL));
	bsgs_table_t table;
	gamal_init(CURVE_256_SEC);
	gamal_init_bsgs_table(table, (dig_t)1L << 16);

    



	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	//                          SETUP PHASE                                     //
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


     // SERVER SETUP COLLECTIVE KEY
     int server_id = 0; // Server 1
     
     Servers servers(number_servers, dataset_size, background_knowledge_directory, a);
     Server server1 = servers.server_vect[server_id];
     Server server2 = servers.server_vect[server_id+1];

	servers.generateCollKey(servers.coll_key);

	servers.pv_ratio = pv_ratio;

	//Server determines maxNoise
     servers.maxNoise = getLaplaceNoiseRange(sensitivity, epsilon, percentile_noise);
     servers.minNoise = -servers.maxNoise;



	//=========Question strategy=========//
	
	int number_real_query = num_query;
	int query_type[] = {1, 0};
	int freq_query[] = {50, 50}; //using fake dataset with p2 = 0.1

	int n_query = sizeof(query_type)/sizeof(query_type[0]);

	int counter_query = 0;
	int query[total_queries];
	for (int i =0; i< total_queries; i++)
	{
		query[i] =  myRand(query_type, freq_query, n_query);
		if (query[i] == 0) counter_query++;
	}
	cout<<"counter_query before modify: "<<counter_query<<endl;


	int ii;
	while (counter_query < number_real_query)
	{
	
		ii = rand()%total_queries;
		// cout<<"which one to fix = "<<i<<endl;
		if (query[ii] == 1)
		{
			query[ii] = 0;
			counter_query++;
		}
		// i++;
		
	}

	while (counter_query > number_real_query)
	{
	
		ii = rand()%total_queries;
		// cout<<"which zeros to fix = "<<i<<endl;
		if (query[ii] == 0)
		{
			query[ii] = 1;
			counter_query--;
		}
		// i++;
		
	}

    for (int i =0; i< total_queries; i++)
	{
		cout<<"query = "<<query[i]<<endl;
		
	}

	cout<<"counter of real query '1' = "<< counter_query <<endl;


	// INITIALIZE CIPHERTEXT STACK FOR SERVERS
	ENC_Stack pre_enc_stack(2*dataset_size, servers.coll_key);

	pre_enc_stack.initializeStack_E0();
		
	pre_enc_stack.initializeStack_E1();
	

	/////////////////////////////////////////////////////////


	// PARTICIPANT 
     Participant part_A(dataset_directory);
     Participant part_B(dataset_directory);
	 gamal_generate_keys(part_A.keys); //part_A keys pair   
	
	 gamal_generate_keys(part_B.keys); //part_B keys pair   
	
     part_A.pv_ratio = pv_ratio;
     // Participant represent its dataset over a histogram of <label, value(label)>; label is one of all possible records that a dataset can take
     
     part_A.create_OriginalHistogram(dataset_size, a);
      
     int size_dataset = part_A.size_dataset;
  
	//Participant determine maxNoise
	
	part_A.maxNoise = getLaplaceNoiseRange(sensitivity, epsilon, percentile_noise);
	part_A.minNoise = -part_A.maxNoise;

	part_A.sensitivity = sensitivity;
	part_A.pv_ratio = pv_ratio;

	cout<<"Party A: maxNoise to answer = "<< part_A.maxNoise<<endl;



	//=========Setup answer strategy=========//
	
	int answer_strategy;
    int query_random;
	int true_fake[] = {1, 0};
	int fake_freq = fre_lie*5; //25; //freq of lie
	int freq[] = {100 - fake_freq, fake_freq}; //using fake dataset with p2 = 0.1

	float lie_freq = (float)fake_freq/100;
	int n = sizeof(true_fake)/sizeof(true_fake[0]);
	int answer_strategy_arr[total_queries];

	int no_lied_answer = (int)(lie_freq*total_queries);
	cout<<"no_lied_answer = "<<no_lied_answer<<endl;

	int counter = 0;
	for (int i =0; i< total_queries; i++)
	{
		answer_strategy_arr[i] =  myRand(true_fake, freq, n);
		if (answer_strategy_arr[i] ==  0)
		{
			counter++;
		}
	}

	
	int i;
	while (counter < no_lied_answer)
	{
	
		i = rand()%total_queries;
		// cout<<"which one to fix = "<<i<<endl;
		if (answer_strategy_arr[i] == 1)
		{
			answer_strategy_arr[i] = 0;
			counter++;
		}
		// i++;
		
	}

	while (counter > no_lied_answer)
	{
	
		i = rand()%total_queries;
		// cout<<"which one to fix = "<<i<<endl;
		if (answer_strategy_arr[i] == 0)
		{
			answer_strategy_arr[i] = 1;
			counter--;
		}
		// i++;
		
	}

    for (int i =0; i< total_queries; i++)
    {
        cout<<"answer = "<<answer_strategy_arr[i]<<endl;
    }

    cout<<"number of lied answer = "<<counter<<endl;

    /////////////////////////////////////////////////////////


	

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	//          PARTIAL VIEW PHASE                                             //
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

	    
     // PARTIAL VIEW COLLECTION  //
    
    
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
  
     vector<string> v_un_permute_sort(dataset_size*a);
     
     part_A.vector_un_permute_sort = v_un_permute_sort;

     part_A.getPermutationOfHistogram(part_A.vector_endcoded_label, part_A.vector_flag); 
     
     part_A.getInversePermutationVector(part_A.vector_endcoded_label, part_A.map_v_permute);
     
	
  
     server1.pv_ratio = pv_ratio;
     server1.size_dataset = dataset_size;
  
     
     server1.createEncryptedPVSamplingVector(pre_enc_stack, server1.server_sample_vector_clear);
       
     
     server1.generatePVfromPermutedHistogram(part_A.map_v_permute_to_send_flag,
                                                 server1.server_sample_vector_encrypted, pre_enc_stack);
     
    
    
   
     server2.rerandomizePVSampleFromPermutedHistogram(server1.PV_sample_from_permuted_map, pre_enc_stack);
    
    
     server2.getUnPermutePV(server1.PV_sample_from_permuted_map, part_A.vector_un_permute_sort);
     
  
  
   
     // PARTIAL VIEW VERIFICATION                                      //
     
     bool verify_status = servers.verifyingPV(server2.un_permute_PV, table, server_id, pre_enc_stack, eta);//all servers jointly decrypt
     
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	//                          QUERY & TEST PHASE                              //
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


	if (verify_status)
	{

	//party create fake histogram (not the fake histogram used in PV)

	//======= Strategy 1: replacing true data by fake data ===========

	float amt_lie = 0.5; 
	int keep_row = int(dataset_size*(1-amt_lie)); //amount of lie
	
	part_A.addDummy_FakeHist_random(keep_row, a);
	
	
	//=== Strategy 2: adding fake data to scale up dataset
	// float adding_ones = 0.25; //omega = 1
	
	// part_A.addDummy_ones_FakeHistogram(a, adding_ones);
	

	bool test_status;
	int count_lied_ans = 0;
	int no_lied_detected = 0;
	int threshold;
	gamal_ciphertext_t sum_cipher, sum_cipher_realquery;


	part_A.no_lied_answer = (int)(lie_freq*total_queries);

	int index = 0;
	int itr = 1;

    int index_query = 0;
    int itr_query = 0;
    int itr_test_L = 1;
    int itr_test_V = 1;
    int itr_test_n = 1;

    t1 = high_resolution_clock::now();
    while (itr_query < total_queries)
    {

        query_random=query[itr_query];
        itr_query++;

        cout<<"query_random = "<<query_random<<endl;

        if (query_random == 1) //actual query
        {
            server1.prepareTestFuntion_Query_Vector(pre_enc_stack, server2.un_permute_PV);
          
            server1.generateMatchDomain(0);
            
            // t1 = high_resolution_clock::now();
            server1.generateNormalQuery_opt(pre_enc_stack);
            
            
            ////====answer using fake dataset with probability p2

            
            answer_strategy = answer_strategy_arr[index];
            // answer_strategy = 0;

            gamal_cipher_new(sum_cipher);
			gamal_cipher_new(sum_cipher_realquery);

            if (answer_strategy == 0)
            {
                
                part_A.computeAnswer_opt(server1.enc_question_map, sum_cipher, part_A.fake_histogram, servers.coll_key, epsilon);
                count_lied_ans++;
			
		    }
            else
            {
                part_A.computeAnswer_opt(server1.enc_question_map, sum_cipher, part_A.histogram, servers.coll_key, epsilon);
                
            }	
            trackTaskStatus(time_track_list, "Query:Dataset true/false", answer_strategy);
		

			
			// Servers save real query answer
			sum_cipher_realquery->C1 = sum_cipher->C1; 
			sum_cipher_realquery->C2 = sum_cipher->C2;

            
            server1.enc_question_map.clear();
            server1.match_query_domain_vect.clear();
            itr++;
            index++;

        }
        else // test function
        {
            if(itr_test_L <= num_test_each_type_rounded)
            {
                //test L
                itr_test_L++;

                server1.prepareTestFuntion_Query_Vector(pre_enc_stack, server2.un_permute_PV);
		
                // t1 = high_resolution_clock::now();
                server1.generateTestKnownRecords_opt(pre_enc_stack, server2.un_permute_PV);
                
                
                
                //====answer using fake dataset with probability p2
            
                
                answer_strategy = answer_strategy_arr[index];
                // answer_strategy = 0;
                
                gamal_cipher_new(sum_cipher);

                if (answer_strategy == 0)
                {
                
                    part_A.computeAnswer_opt(server1.enc_question_map, sum_cipher, part_A.fake_histogram, servers.coll_key, epsilon);
                    count_lied_ans++;
                    
                }
                else
                {
                    part_A.computeAnswer_opt(server1.enc_question_map, sum_cipher, part_A.histogram, servers.coll_key, epsilon);
                    
                }	
                trackTaskStatus(time_track_list, "Test:Dataset true/false", answer_strategy);

                
                
                threshold = server1.known_record_subset.size();
                // cout<< "Threshold L = " << threshold <<endl;

                test_status = servers.verifyingTestResult(sum_cipher, table, server_id, threshold);

                if (test_status == 0)
                {
                    no_lied_detected++;
                }
                trackTaskStatus(time_track_list, "Test target L status", test_status);
                // trackTaskStatus(time_track_list, "No of lied ans", count_lied_ans);
                server1.enc_question_map_pre.clear();
                
                server1.enc_question_map.clear();
                index++;
                // cout<< "Test L verify done" << endl;

            }
            else if (itr_test_V <= num_test_each_type_rounded)
            {
                //test V
                itr_test_V++;

                server1.generateTestBasedPartialView_opt(pre_enc_stack, server2.un_permute_PV);
		
		
                //====answer using fake dataset with probability p2
            
                
                answer_strategy = answer_strategy_arr[index];
                // answer_strategy = 0;
                gamal_cipher_new(sum_cipher);

                if (answer_strategy == 0)
                {
                
                    part_A.computeAnswer_opt(server1.enc_question_map, sum_cipher, part_A.fake_histogram, servers.coll_key, epsilon);
                    count_lied_ans++;
                    
                }
                else
                {
                    part_A.computeAnswer_opt(server1.enc_question_map, sum_cipher, part_A.histogram, servers.coll_key, epsilon);
                    
                }	
                trackTaskStatus(time_track_list, "Test:Dataset true/false", answer_strategy);

                threshold = PV_size; //actual PV size = V (not the PV histogram)
                // cout<<"Threshold V = " << threshold << endl;
                test_status = servers.verifyingTestResult(sum_cipher, table, server_id, threshold);
                
                trackTaskStatus(time_track_list, "Test target V status", test_status);

                if (test_status == 0)
                {
                    no_lied_detected++;
                }
                
                
                server1.enc_question_map_pre.clear();
                
                server1.enc_question_map.clear();
                index++;

            }
            else if (itr_test_n <= num_test_each_type_rounded)
            {
                //test n
                itr_test_n++;
                server1.generateTest_Target_All_Records(pre_enc_stack, server2.un_permute_PV);
                
                gamal_cipher_new(sum_cipher);

                //===== Party answer truth or lie at random p2

                answer_strategy = answer_strategy_arr[index];
                // answer_strategy = 0;

                
                if (answer_strategy == 0)
                {
                    part_A.computeAnswer_opt(server1.enc_question_map, sum_cipher, part_A.fake_histogram, servers.coll_key, epsilon);
                    count_lied_ans++;
                }
                else
                {
                    part_A.computeAnswer_opt(server1.enc_question_map, sum_cipher, part_A.histogram, servers.coll_key, epsilon);
                }
                
                trackTaskStatus(time_track_list, "Test:Dataset true/false", answer_strategy);

                threshold = size_dataset;
                test_status = servers.verifyingTestResult(sum_cipher, table, server_id, threshold);
                trackTaskStatus(time_track_list, "Test target n rows status", test_status);

                
                if (test_status == 0)
                {
                    no_lied_detected++;
                }

                server1.enc_question_map.clear();
                index++;

            }
            else
            {
                //test estimate
                server1.prepareTestFuntion_Query_Vector(pre_enc_stack, server2.un_permute_PV);

                server1.generateMatchDomain(1); // 1: for test function; 0: for normal query
                
                // t1 = high_resolution_clock::now();
                server1.generateTest_Target_Attr_opt(pre_enc_stack);
                
                //====answer using fake dataset with probability p2
            
                // answer_strategy = myRand(true_fake, freq, n);
                // cout<<	"answer_strategy = " <<answer_strategy<<endl;

                answer_strategy = answer_strategy_arr[index];
                // answer_strategy = 0;
                gamal_cipher_new(sum_cipher);
                
                // cout<<	"answer_strategy = " <<answer_strategy<<endl;

                if (answer_strategy == 0)
                {
                
                    part_A.computeAnswer_opt(server1.enc_question_map, sum_cipher, part_A.fake_histogram, servers.coll_key, epsilon);
                    count_lied_ans++;
                    
                }
                else
                {
                    part_A.computeAnswer_opt(server1.enc_question_map, sum_cipher, part_A.histogram, servers.coll_key, epsilon);
                    
                }	
                trackTaskStatus(time_track_list, "Test:Dataset true/false", answer_strategy);

                server1.generate_Test_Target_Attr_Clear(pre_enc_stack);                

                gamal_ciphertext_t enc_PV_answer;
                gamal_cipher_new(enc_PV_answer);
				server1.getTestResult_fromPV(server2.un_permute_PV, enc_PV_answer);
                

                test_status = servers.verifyingTestResult_Estimate("Test attr found:", sum_cipher, table, server_id, enc_PV_answer, alpha);
                trackTaskStatus(time_track_list, "Test attr status", test_status);
                if (test_status == 0)
                {
                    no_lied_detected++;
                }

                server1.match_query_domain_vect.clear();
                server1.enc_question_map_pre.clear();
                
                server1.enc_question_map.clear();
                index++;

            }
            
        }
        
    }
    

	t2 = high_resolution_clock::now();

    trackTaskPerformance(time_track_list, "E2E delay", t1, t2);
   

	

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	//                          FINISHED SHARING                               //
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

	// cout << "Track map size: " << time_track_list.size() << endl;

	

	// cout<<"no of lied ans = "<< count_lied_ans<<endl;
	// cout<<"no of lied detected = "<< no_lied_detected<<endl;
	cout<<"index = "<<index<<endl;
    // cout<<"index_question ="<<index_question<<endl;


	trackTestAccu(time_track_list, "No. of lied answer", count_lied_ans);
	trackTestAccu(time_track_list, "No. of lied detected", no_lied_detected);


	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	//                          RELEASE SHARED DATA                             //
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

	// After mulitple tests and normal queries, servers compute the final results
	// by aggregating answers from partiticipants,
	// re-ecrypt the ciphertext to targeted participant's public key
	// and send the re-encrypted result to each participant
	// depeding on the behavior of each participant, the result
	// is computed so that the protocol ensuring fairness for honest parties
	// and penalizing the lied parties

	if (no_lied_detected == 0)
	{
		//re-encryption
		gamal_ciphertext_t sum_cipher_update;
  		
		gama_key_switch_lead(sum_cipher_update, sum_cipher_realquery, server1.key, part_B.keys);
	
		for (int i=1; i< number_servers; i++)
		{
			gama_key_switch_follow(sum_cipher_update, sum_cipher_realquery, servers.server_vect[server_id+i].key, part_B.keys);
		}
		
	
		dig_t after;
		gamal_decrypt(&after, part_B.keys, sum_cipher_update, table);   

		std::cout<<"Check after re-encryption: "<<after<<std::endl;
  
	}
	
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

	// storeTimeEvaluation(argc, argv, time_track_list, verify_status);


	if (argc > 1)
	{
		fstream fout;
		// std::ofstream fout;

		string filename = "./results/cheating_detection_query_random_";
	
		stringstream ss;
	
		if (strcmp(argv[12], "1") == 0)
		{
			
			ss << filename <<"dataset_"<<dataset_size <<"_"<<num_query<<"_query_"<<num_test<<"_test_"<<"10runs_freq_lie_"<< fake_freq << ".csv"; // add your stuff to the stream
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
			
			ss << filename <<"dataset_"<<dataset_size <<"_"<<num_query<<"_query_"<<num_test<<"_test_"<<"10runs_freq_lie_"<< fake_freq << ".csv";
			 
			fout.open(ss.str().c_str(),ios::out | ios::app);
			
		}

		// Insert the data to file
		fout << argv[12] << ", " << verify_status;
		for (auto itr = time_track_list.begin(); itr != time_track_list.end(); itr++)
		{
			string time_diff = itr->second;
			fout << ", " << time_diff;
		}
		fout << "\n";
		fout.close();
	}

	return 0;


	} //not going to next phase
	else
	{
		
		if (argc > 1)
		{
			fstream fout;
			// std::ofstream fout;

			string filename = "./results/cheating_detection_query_random_";
		
			stringstream ss;
		
			if (strcmp(argv[12], "1") == 0)
			{
				
				ss << filename <<"dataset_"<<dataset_size <<"_"<<num_query<<"_query_"<<num_test<<"_test_"<<"10runs_freq_lie_"<< fake_freq << ".csv"; // add your stuff to the stream
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
				
				ss << filename <<"dataset_"<<dataset_size <<"_"<<num_query<<"_query_"<<num_test<<"_test_"<<"10runs_freq_lie_"<< fake_freq << ".csv";
				
				fout.open(ss.str().c_str(),ios::out | ios::app);
				
			}

			// Insert the data to file
			fout << argv[12] << ", " << verify_status;
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
	

	
}