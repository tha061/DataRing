
#include "../include/public_header.h"
#include "../src/Participant.h"
#include "../src/Server.h"
#include "../src/Servers.h"
#include "../src/process_noise.h"
#include "../src/time_evaluation.h"
#include "../public_func.h"


int partialView_verification_selfPV(int argc, char **argv)
{
	// if (argc > 1 && strcmp(argv[1], "-1") == 0)
	// {
	// 	return computeTimeEvaluation();
	// }

	int dataset_size = 500000; //make it an argument to use to determine dataset_size
	int number_servers = 3;	   //make it an argument use to setup number of servers
	int a = 2;				   //make it an argument to scale up the histogram for adding dummy
	double eta;				   //make it an argument use to determine r0
	double alpha = 0.05;
	double pv_ratio = 0.01;
	
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
	float noise_budget = 0.1;
	float sensitivity = 1.0; //for COUNT query
	int PV_size = (int)dataset_size*pv_ratio; // actual V, not the PV histogram form

	float epsilon_q = noise_budget; //only for test runtime, noise is not a matter
	float epsilon_test = epsilon_q; //only for test runtime, noise is not a matter
	float epsilon = noise_budget;  //only for test runtime, noise is not a matter
	
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

	t1 = high_resolution_clock::now();
	part_A.create_OriginalHistogram(dataset_size);
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_list, "Create Hist (ms)", t1, t2);

	// cout << "Total rows: " << part_A.size_dataset << endl;
	// cout << "Total domains: " << part_A.histogram.size() << endl;

	// part_A.print_Histogram();

	t1 = high_resolution_clock::now();
	t1 = high_resolution_clock::now();
	part_A.addDummy_to_Histogram(a);
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_list, "Add dummy to true Hist (ms)", t1, t2);

	int size_dataset = part_A.size_dataset;

	

	// SERVER SETUP COLLECTIVE KEY
	Servers servers(number_servers, size_dataset, background_knowledge_directory, a); //modified to size aN


	servers.generateCollKey(servers.coll_key);
	servers.pv_ratio = pv_ratio;

	// INITIALIZE CIPHERTEXT STACK FOR PARTY
	part_A.initializePreStack(servers.coll_key);
	


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

	// int keep_row = int(dataset_size*0.9); //lie about 50% rows
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
	
	// // int keep_row = int(dataset_size*0.6); //lie about 50% rows
	// int keep_row = 428027; 
	// // t1 = high_resolution_clock::now();
	// part_A.addDummy_FakeHist_random(keep_row, a);
	// // t2 = high_resolution_clock::now();
	// // trackTaskPerformance(time_track_list, "Fake Dummy Histog (ms)", t1, t2);

	// //pre process:
	// // t1 = high_resolution_clock::now();
	// part_A.pre_process_generatePV(false);
	// // t2 = high_resolution_clock::now();
	// // trackTaskPerformance(time_track_list, "Pre Gen PV (ms)", t1, t2);

	// // t1 = high_resolution_clock::now();
	// part_A.generatePV_opt(servers.s_myPIR_enc, false);
	// // t2 = high_resolution_clock::now();
	// // trackTaskPerformance(time_track_list, "Gen PV (ms)", t1, t2);

	//======= strategy 3: participant does not use PV sampling vector from server
	
	int true_record_PV = (int)PV_size*1;
	// int true_record_PV = 1498;
	// t1 = high_resolution_clock::now();
	part_A.self_create_PV_prepare(true_record_PV, a);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Part_A keep x rows: ", t1, t2);

	// //pre process:
	// // t1 = high_resolution_clock::now();
	part_A.pre_process_generatePV(false);
	// // t2 = high_resolution_clock::now();
	// // trackTaskPerformance(time_track_list, "Pre Gen PV (ms)", t1, t2);

	// // t1 = high_resolution_clock::now();
	part_A.selfCreateFakePV_opt(false);
	// // part_A.selfCreateFakePV_opt(false);
	// // t2 = high_resolution_clock::now();
	// // trackTaskPerformance(time_track_list, "Gen PV (ms)", t1, t2);


	//// after creating PV by itself, create a histogram included V bins in PV and hide all other bins (N-V) bins
    part_A.generate_Histogram_included_self_PV(false, PV_size, a);

	//=====SERVER VERIFIES SUBMITTED PV
	int server_id = 0; // Server 1
	Server server1 = servers.server_vect[server_id];

	
	t1 = high_resolution_clock::now();
	bool verify_status = servers.verifyingPV(part_A.enc_domain_map, table, server_id, pre_enc_stack, eta);
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_list, "Verify PV (ms)", t1, t2);


    ////===== if PV passed, open V bins in PV

	t1 = high_resolution_clock::now();
	if (verify_status) servers.open_true_PV( part_A.enc_domain_map, table, server_id, pre_enc_stack);
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_list, "Open v bins in PV (ms)", t1, t2);

	
	const int number_known_after_phase2 = servers.known_rows_after_phase2.size();
	// const int number_known_after_phase2 = server1.known_rows_after_phase2.size();
	cout<<"number_known_after_phase2 = "<<number_known_after_phase2<<endl;

	// int r_found_in_PV = server1.verified_set.size();
	int r_found_in_PV = servers.verified_set.size();
	cout<<"r_found_in_PV = "<<r_found_in_PV<<endl;

	
	int num_opened_rows = servers.opened_rows_set.size();
	cout<<"num_opened_rows = "<< num_opened_rows<<endl;
	
	

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	//                          QUERY & TEST PHASE                              //
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


	bool test_status;
	int threshold;
	gamal_ciphertext_t sum_cipher;
	
	//Server determines maxNoise
	servers.maxNoise = getLaplaceNoiseRange(sensitivity, epsilon, percentile_noise);
	servers.minNoise = -servers.maxNoise;

    //+++++++++++++++++++RUNTIME OPTIMIZED++++++++++++++++++++++++++++++//

	// // //====TEST FUNCTION KNOWN RECORDS NEW USING PRE_COMPUTE TEST FUCTION =====//
	// // Pre process
	cout<<"Test L ......."<<endl;
	high_resolution_clock::time_point t1_e2e = high_resolution_clock::now();
	
	t1 = high_resolution_clock::now();
	server1.prepareTestFuntion_Query_Vector(pre_enc_stack, part_A.enc_domain_map);
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_list, "Prepare Test function (ms)", t1, t2);

	t1 = high_resolution_clock::now();
	server1.generateTestKnownRecords_opt(pre_enc_stack, part_A.enc_domain_map);
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_list, "Gen Test L (ms)", t1, t2);

	t1 = high_resolution_clock::now();
	gamal_cipher_new(sum_cipher);
	part_A.computeAnswer_opt(server1.enc_question_map, sum_cipher, part_A.fake_histogram, servers.coll_key, epsilon);
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_list, "Compute ans L opt (ms)", t1, t2);

	//Tham added 15 Jan, delete test at server side
	server1.enc_question_map.clear();

	threshold = server1.known_record_subset.size();
	t1 = high_resolution_clock::now();
	test_status = servers.verifyingTestResult("Test target L known rows found:", sum_cipher, table, server_id, threshold);
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_list, "Verify test L result  (ms)", t1, t2);
	trackTaskStatus(time_track_list, "Test target L status", test_status);

	// //===== TEST FUNCTION BASED on known records after phase 2 =====
	// 	//target L + opened records in PV (servers randomly opened V bins and found a number of enc(1))
	
	cout<<"Test L + opened bins from PV ......."<<endl;
	t1 = high_resolution_clock::now();
	server1.prepareTestFuntion_Query_Vector(pre_enc_stack, part_A.enc_domain_map);
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_list, "Prepare Test function (ms)", t1, t2);

	t1 = high_resolution_clock::now();
	server1.generateTest_known_records_after_phase2(pre_enc_stack, part_A.enc_domain_map, servers.known_rows_after_phase2);
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_list, "Gen Test L+ opened_record (ms)", t1, t2);

	t1 = high_resolution_clock::now();
	gamal_cipher_new(sum_cipher);
	part_A.computeAnswer_opt(server1.enc_question_map, sum_cipher, part_A.fake_histogram, servers.coll_key, epsilon);
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_list, "Compute ans L+ opened_record (ms)", t1, t2);

	//Tham added 15 Jan, delete test at server side
	server1.enc_question_map.clear();

	
	threshold = servers.known_rows_after_phase2.size();
	
	cout<<"threshold = "<<threshold<<endl;
	t1 = high_resolution_clock::now();
	test_status = servers.verifyingTestResult("Test target L + opened_record found:", sum_cipher, table, server_id, threshold);
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_list, "Verify test L+ opened_record result  (ms)", t1, t2);
	trackTaskStatus(time_track_list, "Test target L+ opened_record status", test_status);

    //===== TEST FUNCTION BASED PV OPTIMAL =====

	cout<<"Test PV ......."<<endl;
	t1 = high_resolution_clock::now();
	server1.generateTestBasedPartialView_opt(pre_enc_stack, part_A.enc_domain_map);
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_list, "Gen Test V optimal (ms)", t1, t2);

	t1 = high_resolution_clock::now();
	gamal_cipher_new(sum_cipher);
	part_A.computeAnswer_opt(server1.enc_question_map, sum_cipher, part_A.fake_histogram, servers.coll_key, epsilon);
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_list, "Compute ans V (ms)", t1, t2);

	//Tham added 15 Jan, delete test at server side
	server1.enc_question_map.clear();

	threshold = PV_size; 
	t1 = high_resolution_clock::now();
	test_status = servers.verifyingTestResult("Test target V rows found:", sum_cipher, table, server_id, threshold);
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_list, "Verify test V result (ms)", t1, t2);
	trackTaskStatus(time_track_list, "Test target V status", test_status);

    
    //==== TEST PV - r0 =======//
    
	// server1.prepareTestFuntion_Query_Vector(pre_enc_stack, part_A.enc_domain_map);
	
	cout<<"Test PV - r0 ......."<<endl;
	server1.generateTest_PV_r0(pre_enc_stack, part_A.enc_domain_map, servers.verified_set);

	
	gamal_cipher_new(sum_cipher);
	part_A.computeAnswer_opt(server1.enc_question_map, sum_cipher, part_A.fake_histogram, servers.coll_key, epsilon);
	

	//Tham added 15 Jan, delete test at server side
	server1.enc_question_map.clear();

	
	threshold = PV_size - servers.verified_set.size();
	
	cout<<"threshold of test PV - r0= "<<threshold<<endl;
	
	test_status = servers.verifyingTestResult("Test target PV - r0 found:", sum_cipher, table, server_id, threshold);
	
	trackTaskStatus(time_track_list, "Test target PV - r0 status", test_status);


	//======= Test PV+(L-r0) ========//

	// server1.prepareTestFuntion_Query_Vector(pre_enc_stack, part_A.enc_domain_map);
	
	cout<<"Test PV + L - r0 ......."<<endl;
	server1.generateTest_PV_L_r0(pre_enc_stack, part_A.enc_domain_map, servers.verified_set, server1.known_record_subset);
	
	
	gamal_cipher_new(sum_cipher);
	part_A.computeAnswer_opt(server1.enc_question_map, sum_cipher, part_A.histogram, servers.coll_key, epsilon);
	

	//Tham added 15 Jan, delete test at server side
	server1.enc_question_map.clear();

	
	threshold = PV_size + server1.known_record_subset.size() - servers.verified_set.size();
	
	cout<<"threshold of test PV + L - r0= "<<threshold<<endl;
	
	test_status = servers.verifyingTestResult("Test target PV + L - r0 found:", sum_cipher, table, server_id, threshold);
	
	trackTaskStatus(time_track_list, "Test target PV + L - r0 status", test_status);




	//=========== TEST all N rows===========

	cout<<"Test N ......."<<endl;
	server1.generateTest_Target_All_Records(pre_enc_stack, part_A.enc_domain_map);

	gamal_cipher_new(sum_cipher);

	part_A.computeAnswer_opt(server1.enc_question_map, sum_cipher, part_A.fake_histogram, servers.coll_key, epsilon_test);
	
	threshold = size_dataset;
	
	test_status = servers.verifyingTestResult("Test target n rows found:", sum_cipher, table, server_id, threshold);
	
/**	
	//====== RUNTIME OPTIMAL TEST FUNCTION 4 targeting specific attributes ==========//


	server1.prepareTestFuntion_Query_Vector(pre_enc_stack, part_A.enc_domain_map);
	
	server1.generateMatchDomain(1);


	server1.generateTest_Target_Attr_opt(pre_enc_stack);


	
	gamal_cipher_new(sum_cipher);
	part_A.computeAnswer_opt(server1.enc_question_map, sum_cipher, false, servers.coll_key, epsilon);
	

	//Tham added 15 Jan, delete test at server side
	server1.enc_question_map.clear();

	
	server1.generateServerDomain_Test_Target_Attr(pre_enc_stack);
	

	gamal_ciphertext_t enc_PV_answer;
	gamal_cipher_new(enc_PV_answer);
	server1.getTestResult_fromPV(part_A.enc_domain_map, enc_PV_answer);

	
	test_status = servers.verifyingTestResult_Estimate("Test attr found:", sum_cipher, table, server_id, enc_PV_answer, alpha);
	
	trackTaskStatus(time_track_list, "Test attr status", test_status);

	// storeTimeEvaluation(argc, argv, time_track_list, verify_status);
**/

    if (argc > 1)
	{
		fstream fout;
		if (strcmp(argv[9], "1") == 0)
		{
			fout.open("./results/Dishonest_PV_n_500K_selfPV_100pc_eta_095_ratio_1pc.csv", ios::out | ios::trunc);
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
			fout.open("./results/Dishonest_PV_n_500K_selfPV_100pc_eta_095_ratio_1pc.csv", ios::out | ios::app);
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