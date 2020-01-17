
#include "include/public_header.h"
#include "src/Participant.h"
#include "src/Server.h"
#include "src/Servers.h"
#include "src/process_noise.h"
#include "src/time_evaluation.h"
#include "public_func.h"

// #include "testing2.cpp"

int phase_3_test(int argc, char **argv)
{
	int datasize_row = 500000; //make it an argument to use to determine dataset_size
	int SERVER_SIZE = 3;	   //make it an argument use to setup number of servers
	int a = 2;				   //make it an argument to scale up the histogram for adding dummy
	double eta;				   //make it an argument use to determine r0
	double alpha = 0.05;
	double pv_ratio = 0.01;
    int num_query;
    double test_frequency;
	
	string UNIQUE_DOMAIN_DIR, KNOWN_DOMAIN_DIR;
	if (argc > 1)
	{
		if (strstr(argv[1], ".csv") != NULL && strstr(argv[2], ".csv") != NULL)
		{
			UNIQUE_DOMAIN_DIR = argv[1];
			KNOWN_DOMAIN_DIR = argv[2];
			datasize_row = stoi(argv[3]); //size of dataset
			pv_ratio = stod(argv[4]); // pv sample rate
			SERVER_SIZE = stoi(argv[5]);
			a = stoi(argv[6]);
			eta = stod(argv[7]); //make it an argument use to determine r0
			alpha = stod(argv[8]);
            num_query = stoi(argv[9]);
            test_frequency = stod(argv[10]);

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
	float noise_budget = 0.5;
	float sensitivity = 1.0;
	int PV_size = (int)datasize_row*pv_ratio; // actual V, not the PV histogram form

    int num_test = (int)(num_query*test_frequency/(1-test_frequency));
    int iterations = num_query + num_test;

    float epsilon = noise_budget/iterations;
	
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
	Participant part_A(UNIQUE_DOMAIN_DIR);

	t1 = high_resolution_clock::now();
	part_A.create_OriginalHistogram(datasize_row);
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_list, "Create Hist (ms)", t1, t2);

	// cout << "Total rows: " << part_A.size_dataset << endl;
	// cout << "Total domains: " << part_A.hashMap.size() << endl;

	// part_A.print_hash_map();

	t1 = high_resolution_clock::now();
	t1 = high_resolution_clock::now();
	part_A.addDummy_TrueHistogram(a);
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_list, "Add dummy to true Hist (ms)", t1, t2);

	int size_dataset = part_A.size_dataset;

	

	// SERVER SETUP COLLECTIVE KEY
	Servers servers(SERVER_SIZE, size_dataset, KNOWN_DOMAIN_DIR);

	servers.generateCollKey();
	servers.pv_ratio = pv_ratio;

	// INITIALIZE CIPHERTEXT STACK FOR PARTY
	part_A.initializePreStack(servers.coll_key);



	//Participant determine maxNoise
	part_A.maxNoise = getLaplaceNoiseRange(sensitivity, epsilon, percentile_noise);
	part_A.epsilon = epsilon; // total budget is epsilon for all iterations
	part_A.sensitivity = sensitivity;
	part_A.minNoise = -part_A.maxNoise;
	part_A.pv_ratio = pv_ratio;

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
	t1 = high_resolution_clock::now();
	servers.createPVsamplingVector(pre_enc_stack);
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_list, "Gen PV sampling (ms)", t1, t2);

	//+++++++++++ PARTY IS HONEST +++++++++++++++//


	////Gen PV optimal by pre-compte Enc(0) to all dummy domains at offline
	// //pre process:
	// t1 = high_resolution_clock::now();
	// part_A.pre_process_generatePV(true);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Pre Gen PV (ms)", t1, t2);
	
	// t1 = high_resolution_clock::now();
	// part_A.generatePV_opt(servers.s_myPIR_enc, true);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Gen PV (ms)", t1, t2);

	//++++++++++ PARTY IS DISHONEST ++++++++++++//

	//====== strategy 1

	// int keep_row = int(datasize_row*0.9); //lie about 50% rows
	// t1 = high_resolution_clock::now();
	// part_A.addDummy_FakeHist(keep_row, a);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Fake Dummy Histog (ms)", t1, t2);

	// //pre process for reduce runtime
	// t1 = high_resolution_clock::now();
	// part_A.pre_process_generatePV(false);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Pre Gen PV (ms)", t1, t2);

	// t1 = high_resolution_clock::now();
	// part_A.generatePV_opt(servers.s_myPIR_enc, false);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Gen PV (ms)", t1, t2);

	//====== strategy 2: participant generate fake histogram randomly
	
	int keep_row = int(datasize_row*0.4); //lie about 50% rows
	// int keep_row = 428027; 
	t1 = high_resolution_clock::now();
	part_A.addDummy_FakeHist_random(keep_row, a);
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_list, "Fake Dummy Histog (ms)", t1, t2);

	// //pre process:
	t1 = high_resolution_clock::now();
	part_A.pre_process_generatePV(false);
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_list, "Pre Gen PV (ms)", t1, t2);

	t1 = high_resolution_clock::now();
	part_A.generatePV_opt(servers.s_myPIR_enc, false);
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_list, "Gen PV (ms)", t1, t2);

	//======= strategy 3: participant does not use PV sampling vector from server
	
	// // int true_record_PV = (int)PV_size*0.5;
	// int true_record_PV = 1498;
	// // t1 = high_resolution_clock::now();
	// part_A.selfCreate_Fake_Historgram(true_record_PV, a);
	// // t2 = high_resolution_clock::now();
	// // trackTaskPerformance(time_track_list, "Part_A keep x rows: ", t1, t2);

	// // //pre process:
	// // // t1 = high_resolution_clock::now();
	// part_A.pre_process_generatePV(false);
	// // // t2 = high_resolution_clock::now();
	// // // trackTaskPerformance(time_track_list, "Pre Gen PV (ms)", t1, t2);

	// // // t1 = high_resolution_clock::now();
	//  part_A.selfCreateFakePV_opt(false);
	// // // part_A.selfCreateFakePV_opt(false);
	// // // t2 = high_resolution_clock::now();
	// // // trackTaskPerformance(time_track_list, "Gen PV (ms)", t1, t2);


	//=====SERVER VERIFIES SUBMITTED PV
	int server_id = 0; // Server 1
	Server server1 = servers.server_vect[server_id];

	t1 = high_resolution_clock::now();
	t1 = high_resolution_clock::now();
	bool verify_status = servers.verifyingPV(part_A.enc_domain_map, table, server_id, pre_enc_stack, eta);
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_list, "Verify PV (ms)", t1, t2);


	bool test_status;
	int threshold;
	gamal_ciphertext_t sum_cipher;

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	//                          QUERY & TEST PHASE                              //
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	
	//Server determines maxNoise
	servers.maxNoise = getLaplaceNoiseRange(sensitivity, epsilon, percentile_noise);
	servers.minNoise = -servers.maxNoise;

	int itr = 1;
	while (itr <= num_query)
    {
         // // ==== NORMAL QUERY PRE_COMPUTE TO OPTIMIZE RUNTIME ============//
      
        cout<< "\nQuery: #" <<itr << endl;
		bool check = server1.enc_test_map.empty();
		cout<< "enc_test_map empty? "<<check<<endl;
		server1.prepareTestFuntion_Query_Vector(pre_enc_stack, part_A.enc_domain_map);
       
        server1.generateMatchDomain();
      
        t1 = high_resolution_clock::now();
		server1.generateNormalQuery_opt(pre_enc_stack);
		t2 = high_resolution_clock::now();
		trackTaskPerformance(time_track_list, "Gen Query (ms)", t1, t2);

		t1 = high_resolution_clock::now();
		gamal_cipher_new(sum_cipher);
		part_A.computeAnswer_opt(server1.enc_test_map, sum_cipher, true, servers.coll_key);
		t2 = high_resolution_clock::now();
		trackTaskPerformance(time_track_list, "Compute ans Query (ms)", t1, t2);

        //Tham added 15 Jan, delete query at the server side
        server1.enc_test_map.clear();
		server1.match_query_domain_vect.clear();
        itr++;
    }

	// //====TEST FUNCTION KNOWN RECORDS NEW USING PRE_COMPUTE TEST FUCTION =====//
    // Pre process
	

	for (int i = 1; i<=1; i++) 
	{
		cout<<"\nTest L: #"<<i<<endl;
		bool check = server1.enc_test_map.empty();
		cout<< "enc_test_map empty? "<<check<<endl;
		server1.prepareTestFuntion_Query_Vector(pre_enc_stack, part_A.enc_domain_map);
		
		t1 = high_resolution_clock::now();
		server1.generateTestKnownRecords_opt(pre_enc_stack, part_A.enc_domain_map);
		t2 = high_resolution_clock::now();
		trackTaskPerformance(time_track_list, "Gen Test L(ms)", t1, t2);

		t1 = high_resolution_clock::now();
		gamal_cipher_new(sum_cipher);
		// part_A.computeAnswer_opt(server1.enc_test_map, sum_cipher, true, servers.coll_key);
		part_A.computeAnswer_opt(server1.enc_test_map, sum_cipher, false, servers.coll_key);
		t2 = high_resolution_clock::now();
		trackTaskPerformance(time_track_list, "Compute ans L(ms)", t1, t2);
		

		//Tham added 15 Jan, delete test at server side
		server1.enc_test_map.clear();

		threshold = server1.known_vector.size();
		cout<< "Threshold L = " << threshold <<endl;
		test_status = servers.verifyingTestResult("Test target L known rows found:", sum_cipher, table, server_id, threshold);
		trackTaskStatus(time_track_list, "Test target L status", test_status);
		server1.enc_test_map_pre.clear();

	}
	
	// //===== TEST FUNCTION BASED PV OPTIMAL =====
	 
	
	for (int i=1; i<=1; i++)
	{
		cout<<"\nTest V: #"<<i<<endl;
		bool check = server1.enc_test_map.empty();
		cout<< "enc_test_map empty? "<<check<<endl;
		
		t1 = high_resolution_clock::now();
		server1.generateTestBasedPartialView_opt(pre_enc_stack, part_A.enc_domain_map);
		t2 = high_resolution_clock::now();
		trackTaskPerformance(time_track_list, "Gen Test V optimal (ms)", t1, t2);

		t1 = high_resolution_clock::now();
		gamal_cipher_new(sum_cipher);
		// part_A.computeAnswer_opt(server1.enc_test_map, sum_cipher, true, servers.coll_key);
		part_A.computeAnswer_opt(server1.enc_test_map, sum_cipher, false, servers.coll_key);
		t2 = high_resolution_clock::now();
		trackTaskPerformance(time_track_list, "Compute ans V (ms)", t1, t2);

		//Tham added 15 Jan, delete test at server side
		server1.enc_test_map.clear();

		threshold = PV_size; //actual PV size = V (not the PV histogram)
		cout<<"Threshold V = " << threshold << endl;
		test_status = servers.verifyingTestResult("Test target V rows found:", sum_cipher, table, server_id, threshold);
		trackTaskStatus(time_track_list, "Test target V status", test_status);
		
		server1.enc_test_map_pre.clear();
	}

	//===== TEST FUNCTION 3 targeting V - r0 records in PV ====//

	t1 = high_resolution_clock::now();
	server1.generateTestHashMap_3(pre_enc_stack, part_A.enc_domain_map);
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_list, "Gen Test V - r0 (ms)", t1, t2);

	t1 = high_resolution_clock::now();
	gamal_cipher_new(sum_cipher);
	part_A.computeAnswer_opt(server1.enc_test_map, sum_cipher, false, servers.coll_key);
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_list, "Compute ans V - r0 (ms)", t1, t2);

	threshold = PV_size - server1.verified_set.size();
	test_status = servers.verifyingTestResult("Test target V - r0 rows found:", sum_cipher, table, server_id, threshold);
	trackTaskStatus(time_track_list, "Test target V - r0 status", test_status);
	
	//====== RUNTIME OPTIMAL TEST FUNCTION 4 targeting specific attributes ==========//

	for (int i=1; i<=1; i++)
	{
		cout<<"\nTest estimate: #"<<i<<endl;
		bool check = server1.enc_test_map.empty();
		cout<< "enc_test_map empty? "<<check<<endl;
		server1.prepareTestFuntion_Query_Vector(pre_enc_stack, part_A.enc_domain_map);

		server1.generateMatchDomain();
		

		t1 = high_resolution_clock::now();
		server1.generateTest_Target_Attr_opt(pre_enc_stack);
		t2 = high_resolution_clock::now();
		trackTaskPerformance(time_track_list, "Gen Test Attr (ms)", t1, t2);

		t1 = high_resolution_clock::now();
		gamal_cipher_new(sum_cipher);
		part_A.computeAnswer_opt(server1.enc_test_map, sum_cipher, false, servers.coll_key);
		// part_A.computeAnswer_opt(server1.enc_test_map, sum_cipher, true, servers.coll_key);
		t2 = high_resolution_clock::now();
		trackTaskPerformance(time_track_list, "Compute ans Test Attr opt (ms)", t1, t2);

		//Tham added 15 Jan, delete test at server side
		server1.enc_test_map.clear();


		server1.generateServerDomain_Test_Target_Attr(pre_enc_stack);
		

		gamal_ciphertext_t enc_PV_answer;
		gamal_cipher_new(enc_PV_answer);
		server1.getTestResult_fromPV(part_A.enc_domain_map, enc_PV_answer);


		test_status = servers.verifyingTestResult_Estimate("Test attr found:", sum_cipher, table, server_id, enc_PV_answer, alpha);
		trackTaskStatus(time_track_list, "Test attr status", test_status);

		server1.match_query_domain_vect.clear();
		server1.enc_test_map_pre.clear();

	}

	// cout<<"DONE: "<<endl;
	// cout<<"====SERVER memory check=====\n";
	// bool S_check = server1.enc_test_map.empty();
	// cout<< "test/query = enc_test_map empty? "<<S_check<<endl;

	// bool S_check2 = server1.known_vector.empty();
	// cout << "server1.known_vector.empty(); "<<S_check2<<endl;


	// bool S_check3 = server1.enc_test_map_pre.empty();
	// cout << "server1.enc_test_map_pre.empty(); "<<S_check3<<endl;



	// bool S_check4 = server1.verified_set.empty();
	// cout << "server1.verified_set.empty(); "<<S_check4<<endl;

	
	// bool S_check5 = server1.plain_domain_map.empty();
	// cout << "server1.plain_domain_map.empty(); "<<S_check5<<endl;


	// bool S_check6 = server1.match_query_domain_vect.empty();
	// cout << "server1.match_query_domain_vect.empty(); "<<S_check6<<endl;

	
	// bool stack_check = pre_enc_stack.isEmpty();
	// cout <<"pre_enc_stack.isEmpty(); "<<stack_check<<endl;

	// cout<<"====PARTY memory check======\n";
	// bool P_check = part_A.enc_domain_map.empty();
	// cout<< "PV = enc_domain_map empty? "<<P_check<<endl;

	// bool P_check2 = part_A.pre_enc_stack_participant.isEmpty();
	// cout << "pre_enc_stack_participant.isEmpty() "<< P_check2<<endl;

	// bool P_check3 = part_A.hashMap.empty();
	// cout << "part_A.hashMap.empty() "<<P_check3<<endl;

	// bool P_check4 = part_A.plain_domain_map.empty();
	// cout <<"part_A.plain_domain_map.empty() "<<P_check4<<endl;

	// bool P_check5 = part_A.fakeHashMap.empty();
	// cout << "part_A.fakeHashMap.empty(); "<<P_check5<<endl;
   

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

	storeTimeEvaluation(argc, argv, time_track_list, verify_status);


	// // void storeTimeEvaluation(int argc, char **argv, TRACK_LIST &time_track_list, bool verify_status)
	// // {

		if (argc > 1)
		{
			fstream fout;
			if (strcmp(argv[11], "1") == 0)
			{
				fout.open("./results/runtime_phase3_malicious_fakePV_participant_500K_pv_001_L_1000_3query_4test.csv", ios::out | ios::trunc);
				// fout << "Iteration, PV Verification";
				
				fout << "Iteration" << ", "<<argv[11];
				fout<<"\n";
				fout <<"\n";
				fout << "PV Verification"<<", "<<verify_status;
				fout<<"\n";
				for (auto itr = time_track_list.begin(); itr != time_track_list.end(); itr++)
				{
					string column = itr->first;
					string time_diff = itr->second;
					// fout << ", " << column << ","<<time_diff;
					fout << column << ","<<time_diff;
					fout << "\n";
				}
				// fout << "\n";
			}
			else
			{
				fout.open("./results/runtime_phase3_malicious_fakePV_participant_500K_pv_001_L_1000_3query_4test.csv", ios::out | ios::app);
			}

			// Insert the data to file
			// fout << argv[11] << ", " << verify_status;
			// for (auto itr = time_track_list.begin(); itr != time_track_list.end(); itr++)
			// {
			// 	string time_diff = itr->second;
			// 	fout << ", " << time_diff;
			// 	fout << "\n";
			// }
			// fout << "\n";
			fout.close();
		}
	// // }

	return 0;
}