
#include "../include/public_header.h"
#include "../src/Participant.h"
#include "../src/Server.h"
#include "../src/Servers.h"
#include "../src/process_noise.h"
#include "../src/time_evaluation.h"
#include "../public_func.h"

// #include "testing2.cpp"

int phase_3_test_new_PVGen_method(int argc, char **argv)
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
	float noise_budget = 1;
	float sensitivity = 1.0;
	int PV_size = (int)dataset_size*pv_ratio; // actual V, not the PV histogram form

    int num_test = (int)(num_query*test_frequency/(1-test_frequency));
    int iterations = num_query + num_test;

	// Setup answer strategy
	int answer_strategy;
	int true_fake[] = {1, 0};
	int fake_freq = fre_lie*5;//25; //freq of lie
	int freq[] = {100 - fake_freq, fake_freq}; //using fake dataset with p2 = 0.1

	float lie_freq = (float)fake_freq/100;
	int n = sizeof(true_fake)/sizeof(true_fake[0]);
	int answer_strategy_arr[iterations];

	int no_lied_answer = (int)(lie_freq*iterations);
	cout<<"no_lied_answer = "<<no_lied_answer<<endl;

	int counter = 0;
	for (int i =0; i< iterations; i++)
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
	
		i = rand()%iterations;
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
	
		i = rand()%iterations;
		// cout<<"which one to fix = "<<i<<endl;
		if (answer_strategy_arr[i] == 0)
		{
			answer_strategy_arr[i] = 1;
			counter--;
		}
		// i++;
		
	}

	// participant computes epsilon for a query's answer and a test's answer
    float epsilon_q = noise_budget/(num_query+3*num_test);
	float epsilon_test = 3*epsilon_q;


	double num_test_each = (double)num_test/4; // 4 test function types
	int num_test_rounded = ceil(num_test_each);
	cout << "num_test_rounded = "<< num_test_rounded <<endl;
	// cout << "num_test = " << num_test_each <<endl;
	
	TRACK_LIST time_track_list;

	high_resolution_clock::time_point t1, t2;

	srand(time(NULL));
	// srand(100);
	// initialize key & table
	// gamal_key_t key;
	bsgs_table_t table;
	gamal_init(CURVE_256_SEC);
	// gamal_generate_keys(key);
	gamal_init_bsgs_table(table, (dig_t)1L << 16);

    

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	//                          SETUP PHASE                                     //
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

	// PARTICIPANT CONVERT DATASET TO HISTOGRAM
	Participant part_A(dataset_directory);
	Participant part_B;

	
	part_A.create_OriginalHistogram(dataset_size);
	
	

	//===== party generate true histogram =====
	
	// t1 = high_resolution_clock::now();
	part_A.addDummy_to_Histogram(a); 
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Add dummy to true Hist (ms)", t1, t2);

	int size_dataset = part_A.size_dataset;



	
	// SERVER SETUP COLLECTIVE KEY
	Servers servers(number_servers, size_dataset, background_knowledge_directory, a); //modified to size aN


	servers.generateCollKey();

	servers.pv_ratio = pv_ratio;

	// INITIALIZE CIPHERTEXT STACK FOR PARTY
	// part_A.initializePreStack(servers.coll_key);



	//Participant determine maxNoise
	part_A.maxNoise_q = getLaplaceNoiseRange(sensitivity, epsilon_q, percentile_noise);
	part_A.epsilon_q = epsilon_q; // total budget is epsilon for all iterations
	part_A.minNoise_q = -part_A.maxNoise_q;
	part_A.maxNoise_test = getLaplaceNoiseRange(sensitivity, epsilon_test, percentile_noise);
	part_A.epsilon_test = epsilon_test; // total budget is epsilon for all iterations
	part_A.minNoise_test = -part_A.maxNoise_test;

	part_A.sensitivity = sensitivity;
	part_A.pv_ratio = pv_ratio;

	cout<<"Party A: maxNoise query = "<< part_A.maxNoise_q<<endl;
	cout<<"Party A: maxNoise test = "<< part_A.maxNoise_test<<endl;

	// INITIALIZE CIPHERTEXT STACK FOR SERVERS
	ENC_Stack pre_enc_stack(size_dataset, servers.coll_key);

	// t1 = high_resolution_clock::now();
	pre_enc_stack.initializeStack_E0();
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Precompute Enc0 (ms)", t1, t2);
	// cout << endl;

	// t1 = high_resolution_clock::now();
	pre_enc_stack.initializeStack_E1();
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Precompute Enc1 (ms)", t1, t2);
	// cout << endl;



	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	//          PV SUBMISSION AND VERIFICATION PHASE                            //
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

	// SERVER CREATE PV SAMPLING VECTOR
	
	servers.createPVsamplingVector_size_N_plus_1(pre_enc_stack);
	

	//+++++++++++ PARTY IS HONEST +++++++++++++++//


	//Gen PV optimal by pre-compte Enc(0) to all dummy domains at offline
	//pre process:
	// t1 = high_resolution_clock::now();
	// part_A.pre_process_generatePV(true);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Pre Gen PV (ms)", t1, t2);
	
	// t1 = high_resolution_clock::now();
	// part_A.generatePV_opt(servers.s_myPIR_enc, true);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Gen PV (ms)", t1, t2);

	//++++++++++ PARTY IS DISHONEST ++++++++++++//

	//====== strategy 2: participant generate fake histogram randomly
	
	int keep_row_Hist = int(dataset_size*0.95); //lie about 50% rows
	// // int keep_row = 428027; 
	// t1 = high_resolution_clock::now();
	part_A.addDummy_FakeHist_random(keep_row_Hist, a);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Fake Dummy Histog (ms)", t1, t2);

	// // //pre process:
	// t1 = high_resolution_clock::now();
	// part_A.pre_process_generatePV(false);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Pre Gen PV (ms)", t1, t2);

	// t1 = high_resolution_clock::now();
	// part_A.generatePV_opt(servers.s_myPIR_enc, false);
	part_A.generatePV_fixed_scheme(servers.s_myPIR_enc, part_A.histogram, part_A.size_dataset);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Gen PV (ms)", t1, t2);

	

//======= strategy 3: participant does not use PV sampling vector from server and keep this in the fake histogram
	// int true_record_PV = (int)PV_size*1;
	
	// part_A.self_create_PV_prepare(true_record_PV, a);


	// part_A.pre_process_generatePV(false);
	
	// part_A.selfCreateFakePV_opt(false);

	// //// after creating PV by itself, create a histogram included V bins in PV and hide all other bins (N-V) bins
    // part_A.generate_Histogram_included_self_PV(false, PV_size, a);


	//=====SERVER VERIFIES SUBMITTED PV
	int server_id = 0; // Server 1
	Server server1 = servers.server_vect[server_id];

	
	// t1 = high_resolution_clock::now();
	bool verify_status = servers.verifyingPV(part_A.enc_domain_map, table, server_id, pre_enc_stack, eta);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Verify PV (ms)", t1, t2);

	if (verify_status)
	{

		//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	//                          QUERY & TEST PHASE                              //
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

	//party create fake histogram (not the fake histogram used in PV)

	//=======case 1 ===========

	// float amt_lie = 0.1; 
	// int keep_row = int(dataset_size*(1-amt_lie)); //amount of lie
	// // // // t1 = high_resolution_clock::now();
	// part_A.addDummy_FakeHist_random(keep_row, a);
	// // // // t2 = high_resolution_clock::now();
	// // // trackTaskPerformance(time_track_list, "Fake Dummy Histog (ms)", t1, t2);
	
	// //==case 2
	// float adding_ones = 1; //omega = 1
	// t1 = high_resolution_clock::now();
	// part_A.addDummy_ones_FakeHistogram(a, adding_ones);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Fake Dummy Histog (ms)", t1, t2);

	bool test_status;
	int count_lied_ans = 0;
	int no_lied_detected = 0;
	int threshold;
	gamal_ciphertext_t sum_cipher;

	// Servers send a cipher_1 = enc(1) to party for encryptig noise

	pre_enc_stack.pop_E1(part_A.cipher1);

	
	
	//Server determines maxNoise
	servers.maxNoise = getLaplaceNoiseRange(sensitivity, epsilon_test, percentile_noise);
	servers.minNoise = -servers.maxNoise;

	cout<<"Server: maxNoise = "<<servers.maxNoise<<endl;

	part_A.no_lied_answer = (int)(lie_freq*iterations);
	// cout<<"freq of lie = "<<lie_freq<<endl;
	// cout<<"iterations = "<<iterations<<endl;
	// cout<<"no of lies = "<<part_A.no_lied_answer<<endl;

	

	int index = 0;
	int itr = 1;

	// cout<<"test ok"<<endl;

	t1 = high_resolution_clock::now();
	while (itr <= num_query)
    {
         // // ==== NORMAL QUERY PRE_COMPUTE TO OPTIMIZE RUNTIME ============//
      
       
		server1.prepareTestFuntion_Query_Vector(pre_enc_stack, part_A.enc_domain_map);
       
       
        server1.generateMatchDomain(0);
       
      
        // t1 = high_resolution_clock::now();
		server1.generateNormalQuery_opt(pre_enc_stack);
        
		
		// //====answer using fake dataset with probability p2

		
		answer_strategy = answer_strategy_arr[index];
		// answer_strategy = 0;

		gamal_cipher_new(sum_cipher);

		if (answer_strategy == 0)
		{
			
            part_A.computeAnswer_modified(server1.enc_question_map, sum_cipher, part_A.fake_histogram, part_A.cipher1, epsilon_q);
            
			count_lied_ans++;
			
		}
		else
		{
			
			part_A.computeAnswer_modified(server1.enc_question_map, sum_cipher, part_A.histogram, part_A.cipher1, epsilon_q);
			
			
		}	
		trackTaskStatus(time_track_list, "Query:Dataset true/false", answer_strategy);
		

        //Tham added 15 Jan, delete query at the server side
        server1.enc_question_map.clear();
		server1.match_query_domain_vect.clear();
        itr++;
		index++;


		//======Re-encrypt query answer to participant B's public key

		gamal_ciphertext_t sum_cipher_update;

		gamal_generate_keys(part_B.keys); //part_B keys pair   

		
		gama_key_switch_lead(sum_cipher_update, sum_cipher, server1.key, part_B.keys);

		for (int i=1; i< number_servers; i++)
		{
			gama_key_switch_follow(sum_cipher_update, sum_cipher, servers.server_vect[server_id+i].key, part_B.keys);
		}
       
		
	}

	
	// //====TEST FUNCTION KNOWN RECORDS NEW USING PRE_COMPUTE TEST FUCTION =====//
    // Pre process
	
    // num_test_rounded =1;
	for (int i = 1; i<= num_test_rounded; i++) 
	{
		
		server1.prepareTestFuntion_Query_Vector(pre_enc_stack, part_A.enc_domain_map);
		
		// t1 = high_resolution_clock::now();
		server1.generateTestKnownRecords_opt(pre_enc_stack, part_A.enc_domain_map);
		
		
		
		//====answer using fake dataset with probability p2
	
		
		answer_strategy = answer_strategy_arr[index];
		// answer_strategy = 0;
		
		gamal_cipher_new(sum_cipher);

		if (answer_strategy == 0)
		{
			part_A.computeAnswer_modified(server1.enc_question_map, sum_cipher, part_A.fake_histogram, part_A.cipher1, epsilon_test);
			count_lied_ans++;
			
		}
		else
		{
			part_A.computeAnswer_modified(server1.enc_question_map, sum_cipher, part_A.histogram, part_A.cipher1, epsilon_test);
			
		}	
		trackTaskStatus(time_track_list, "Test:Dataset true/false", answer_strategy);

		
		
		threshold = server1.known_record_subset.size();
		// cout<< "Threshold L = " << threshold <<endl;

		test_status = servers.verifyingTestResult("Test target L known rows found:", sum_cipher, table, server_id, threshold);

		if (test_status == 0)
		{
			no_lied_detected++;
		}
		trackTaskStatus(time_track_list, "Test target L status", test_status);
		// trackTaskStatus(time_track_list, "No of lied ans", count_lied_ans);
		server1.enc_question_map_pre.clear();
		//Tham added 15 Jan, delete test at server side
		server1.enc_question_map.clear();
		index++;
		// cout<< "Test L verify done" << endl;

	}
	
	// //===== TEST FUNCTION BASED PV OPTIMAL =====
	 
	
	for (int i=1; i<=num_test_rounded; i++)
	{
		
		server1.generateTestBasedPartialView_opt(pre_enc_stack, part_A.enc_domain_map);
		
		
		//====answer using fake dataset with probability p2
	
		
		answer_strategy = answer_strategy_arr[index];
		// answer_strategy = 0;
		gamal_cipher_new(sum_cipher);

		if (answer_strategy == 0)
		{
			part_A.computeAnswer_modified(server1.enc_question_map, sum_cipher, part_A.fake_histogram, part_A.cipher1, epsilon_test);
			count_lied_ans++;
			
		}
		else
		{
			part_A.computeAnswer_modified(server1.enc_question_map, sum_cipher, part_A.histogram, part_A.cipher1, epsilon_test);
			
		}	
		trackTaskStatus(time_track_list, "Test:Dataset true/false", answer_strategy);

		threshold = PV_size; //actual PV size = V (not the PV histogram)
		// cout<<"Threshold V = " << threshold << endl;
		test_status = servers.verifyingTestResult("Test target V rows found:", sum_cipher, table, server_id, threshold);
		
		trackTaskStatus(time_track_list, "Test target V status", test_status);

		if (test_status == 0)
		{
			no_lied_detected++;
		}
		
		
		server1.enc_question_map_pre.clear();
		//Tham added 15 Jan, delete test at server side
		server1.enc_question_map.clear();
		index++;
	}

	//===== TEST FUNCTION  targeting all rows in dataset ====//


	
	for (int i=1; i<=num_test_rounded; i++)
	{
			
		server1.generateTest_Target_All_Records(pre_enc_stack, part_A.enc_domain_map);
		
		gamal_cipher_new(sum_cipher);

		//===== Party answer truth or lie at random p2

		answer_strategy = answer_strategy_arr[index];
		// answer_strategy = 0;

		
		if (answer_strategy == 0)
		{
            part_A.computeAnswer_modified(server1.enc_question_map, sum_cipher, part_A.fake_histogram, part_A.cipher1, epsilon_test);
			count_lied_ans++;
		}
		else
		{
			part_A.computeAnswer_modified(server1.enc_question_map, sum_cipher, part_A.histogram, part_A.cipher1, epsilon_test);
		}
		
		trackTaskStatus(time_track_list, "Test:Dataset true/false", answer_strategy);

		threshold = size_dataset;
		test_status = servers.verifyingTestResult("Test target n rows found:", sum_cipher, table, server_id, threshold);
		trackTaskStatus(time_track_list, "Test target n rows status", test_status);

		
		if (test_status == 0)
		{
			no_lied_detected++;
		}

		server1.enc_question_map.clear();
		index++;
	
	}

	//====== RUNTIME OPTIMAL TEST FUNCTION 4 targeting specific attributes ==========//


	for (int i=1; i<= num_test - 3*num_test_rounded; i++)
	// for (int i=1; i<= num_test_rounded +1; i++)
	{

		server1.prepareTestFuntion_Query_Vector(pre_enc_stack, part_A.enc_domain_map);

		server1.generateMatchDomain(1);
		
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
			part_A.computeAnswer_modified(server1.enc_question_map, sum_cipher, part_A.fake_histogram, part_A.cipher1, epsilon_test);
			count_lied_ans++;
			
		}
		else
		{
			part_A.computeAnswer_modified(server1.enc_question_map, sum_cipher, part_A.histogram, part_A.cipher1, epsilon_test);
			
		}	
		trackTaskStatus(time_track_list, "Test:Dataset true/false", answer_strategy);

		server1.generateServerDomain_Test_Target_Attr(pre_enc_stack);
		

		gamal_ciphertext_t enc_PV_answer;
		gamal_cipher_new(enc_PV_answer);
		server1.getTestResult_fromPV(part_A.enc_domain_map, enc_PV_answer);


		test_status = servers.verifyingTestResult_Estimate("Test attr found:", sum_cipher, table, server_id, enc_PV_answer, alpha);
		trackTaskStatus(time_track_list, "Test attr status", test_status);
		if (test_status == 0)
		{
			no_lied_detected++;
		}

		server1.match_query_domain_vect.clear();
		server1.enc_question_map_pre.clear();
		//Tham added 15 Jan, delete test at server side
		server1.enc_question_map.clear();
		index++;
		

	}


	  t2 = high_resolution_clock::now();

	  trackTaskPerformance(time_track_list, "E2E delay", t1, t2);
   

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

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	//                          FINISHED SHARING                               //
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

	// cout << "Track map size: " << time_track_list.size() << endl;

	

	// cout<<"no of lied ans = "<< count_lied_ans<<endl;
	// cout<<"no of lied detected = "<< no_lied_detected<<endl;
	cout<<"index = "<<index<<endl;



	trackTestAccu(time_track_list, "No. of lied answer", count_lied_ans);
	trackTestAccu(time_track_list, "No. of lied detected", no_lied_detected);


	// storeTimeEvaluation(argc, argv, time_track_list, verify_status);


	if (argc > 1)
	{
		fstream fout;
		// std::ofstream fout;

		string filename = "./results/lie_dectection_PV_disHist_95pc_new_";
	
		stringstream ss;
	
		if (strcmp(argv[12], "1") == 0)
		{
			
			ss << filename <<"dataset_"<<dataset_size <<"_"<<num_query<<"_query_"<<num_test<<"_test_"<<"30runs_freq_lie_"<< fake_freq << ".csv"; // add your stuff to the stream
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
			
			ss << filename <<"dataset_"<<dataset_size <<"_"<<num_query<<"_query_"<<num_test<<"_test_"<<"30runs_freq_lie_"<< fake_freq << ".csv";
			 
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
	else
	{
		
		if (argc > 1)
		{
			fstream fout;
			// std::ofstream fout;

			string filename = "./results/lie_dectection_PV_disHist_95pc_new_";
		
			stringstream ss;
		
			if (strcmp(argv[12], "1") == 0)
			{
				
				ss << filename <<"dataset_"<<dataset_size <<"_"<<num_query<<"_query_"<<num_test<<"_test_"<<"30runs_freq_lie_"<< fake_freq << ".csv"; // add your stuff to the stream
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
				
				ss << filename <<"dataset_"<<dataset_size <<"_"<<num_query<<"_query_"<<num_test<<"_test_"<<"30runs_freq_lie_"<< fake_freq << ".csv";
				
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