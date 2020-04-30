
#include "../include/public_header.h"
#include "../src/Participant.h"
#include "../src/Server.h"
#include "../src/Servers.h"
#include "../src/process_noise.h"
#include "../src/time_evaluation.h"
#include "../public_func.h"


int runtime_testing(int argc, char **argv)
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
	float noise_budget = 1;
	float sensitivity = 1.0; //for COUNT query
	int PV_size = (int)dataset_size*pv_ratio; // actual V, not the PV histogram form

	float epsilon_q = noise_budget/20; //if there are 20 questions
	float epsilon_test = epsilon_q; //if there are 20 questions
	float epsilon = noise_budget/20;  //if there are 20 questions
	
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

	//key for partial view generation
	// gamal_key_t coll_key_PV_phase;
	
	servers.generateCollKey(servers.coll_key);
	
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

	// cout<<"Party A: maxNoise query = "<< part_A.maxNoise_q<<endl;
	// cout<<"Party A: maxNoise test = "<< part_A.maxNoise_test<<endl;

	// INITIALIZE CIPHERTEXT STACK FOR SERVERS
	ENC_Stack pre_enc_stack(4*size_dataset, servers.coll_key);//four times of the dataset size enc0 and enc1 pre-computed

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
	// servers.createPVsamplingVector(pre_enc_stack);
	servers.createPVsamplingVector_size_N_plus_1(pre_enc_stack);
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
	// // part_A.generatePV_opt(servers.s_myPIR_enc, true);
	// part_A.generatePV_fixed_scheme(servers.s_myPIR_enc, part_A.histogram, part_A.size_dataset);
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
	int keep_row = 487742; 
	// // t1 = high_resolution_clock::now();
	part_A.addDummy_FakeHist_random(keep_row, a);
	// // t2 = high_resolution_clock::now();
	// // trackTaskPerformance(time_track_list, "Fake Dummy Histog (ms)", t1, t2);

	// //pre process:
	// // t1 = high_resolution_clock::now();
	// part_A.pre_process_generatePV(false);
	// // t2 = high_resolution_clock::now();
	// // trackTaskPerformance(time_track_list, "Pre Gen PV (ms)", t1, t2);

	// // t1 = high_resolution_clock::now();
	part_A.generatePV_fixed_scheme(servers.s_myPIR_enc, part_A.fake_histogram, part_A.size_dataset);
	// // t2 = high_resolution_clock::now();
	// // trackTaskPerformance(time_track_list, "Gen PV (ms)", t1, t2);

	//======= strategy 3: participant does not use PV sampling vector from server
	
	// int true_record_PV = (int)PV_size*0.9;
	// // int true_record_PV = 1498;
	// // t1 = high_resolution_clock::now();
	// part_A.self_create_PV_prepare(true_record_PV, a);
	// // t2 = high_resolution_clock::now();
	// // trackTaskPerformance(time_track_list, "Part_A keep x rows: ", t1, t2);

	// // //pre process:
	// // // t1 = high_resolution_clock::now();
	// part_A.pre_process_generatePV(false);
	// // // t2 = high_resolution_clock::now();
	// // // trackTaskPerformance(time_track_list, "Pre Gen PV (ms)", t1, t2);

	// // // t1 = high_resolution_clock::now();
	//  part_A.selfCreateFakePV_opt(false);
	// // part_A.selfCreateFakePV_opt(false);
	// // t2 = high_resolution_clock::now();
	// // trackTaskPerformance(time_track_list, "Gen PV (ms)", t1, t2);


	//=====SERVER VERIFIES SUBMITTED PV
	int server_id = 0; // Server 1
	Server server1 = servers.server_vect[server_id];

	
	t1 = high_resolution_clock::now();
	
	bool verify_status = servers.verifyingPV(part_A.enc_domain_map, table, server_id, pre_enc_stack, eta);
	
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_list, "Verify PV (ms)", t1, t2);


	// t1 = high_resolution_clock::now();
	// if (verify_status) servers.open_true_PV( part_A.enc_domain_map, table, server_id, pre_enc_stack);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Open v bins in PV (ms)", t1, t2);

	
	// const int number_known_after_phase2 = servers.known_rows_after_phase2.size();
	// // const int number_known_after_phase2 = server1.known_rows_after_phase2.size();
	// cout<<"number_known_after_phase2 = "<<number_known_after_phase2<<endl;

	// // int r_found_in_PV = server1.verified_set.size();
	// int r_found_in_PV = servers.verified_set.size();
	// cout<<"r_found_in_PV = "<<r_found_in_PV<<endl;

	
	// int num_opened_rows = servers.opened_rows_set.size();
	// cout<<"num_opened_rows = "<< num_opened_rows<<endl;
	
	
	
	bool test_status;
	int threshold;
	gamal_ciphertext_t sum_cipher;

	// Servers send a cipher_1 = enc(1) to party for encryptig noise

	// pre_enc_stack.pop_E1(part_A.cipher1);


	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	//                          QUERY & TEST PHASE                              //
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	
	//Server determines maxNoise
	servers.maxNoise = getLaplaceNoiseRange(sensitivity, epsilon, percentile_noise);
	servers.minNoise = -servers.maxNoise;

	
	
    //+++++++++++++++++++RUNTIME OPTIMIZED++++++++++++++++++++++++++++++//

	// // //====TEST FUNCTION KNOWN RECORDS NEW USING PRE_COMPUTE TEST FUCTION =====//
	// // Pre process

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
	
	// part_A.computeAnswer_modified(server1.enc_question_map, sum_cipher, part_A.histogram, part_A.cipher1, epsilon);
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


	// // //===== TEST FUNCTION BASED on known records after phase 2 =====

	// t1 = high_resolution_clock::now();
	// server1.prepareTestFuntion_Query_Vector(pre_enc_stack, part_A.enc_domain_map);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Prepare Test function (ms)", t1, t2);

	// t1 = high_resolution_clock::now();
	// server1.generateTest_known_records_after_phase2(pre_enc_stack, part_A.enc_domain_map, servers.known_rows_after_phase2);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Gen Test L+ opened_record (ms)", t1, t2);

	// t1 = high_resolution_clock::now();
	// gamal_cipher_new(sum_cipher);
	// // part_A.computeAnswer_opt(server1.enc_question_map, sum_cipher, part_A.histogram, servers.coll_key, epsilon);
	// part_A.computeAnswer_modified(server1.enc_question_map, sum_cipher, part_A.histogram, part_A.cipher1, epsilon);

	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Compute ans L+ opened_record (ms)", t1, t2);

	// //Tham added 15 Jan, delete test at server side
	// server1.enc_question_map.clear();

	
	// threshold = servers.known_rows_after_phase2.size();
	
	// cout<<"threshold = "<<threshold<<endl;
	// t1 = high_resolution_clock::now();
	// test_status = servers.verifyingTestResult("Test target L + opened_record found:", sum_cipher, table, server_id, threshold);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Verify test L+ opened_record result  (ms)", t1, t2);
	// trackTaskStatus(time_track_list, "Test target L+ opened_record status", test_status);



	//===== TEST FUNCTION BASED PV OPTIMAL =====

	// t1 = high_resolution_clock::now();
	// server1.generateTestBasedPartialView_opt(pre_enc_stack, part_A.enc_domain_map); //partial view is re-encryted already
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Gen Test V optimal (ms)", t1, t2);

	// t1 = high_resolution_clock::now();
	// gamal_cipher_new(sum_cipher);
	// part_A.computeAnswer_opt(server1.enc_question_map, sum_cipher, part_A.histogram, servers.coll_key, epsilon);
	// // part_A.computeAnswer_modified(server1.enc_question_map, sum_cipher, part_A.histogram, part_A.cipher1, epsilon);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Compute ans V (ms)", t1, t2);

	// //Tham added 15 Jan, delete test at server side
	// server1.enc_question_map.clear();

	// threshold = PV_size; //actual PV size = V (not the PV histogram)
	// t1 = high_resolution_clock::now();
	// test_status = servers.verifyingTestResult("Test target V rows found:", sum_cipher, table, server_id, threshold);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Verify test V result (ms)", t1, t2);
	// trackTaskStatus(time_track_list, "Test target V status", test_status);


	//=========== TEST all rows===========

	// t1 = high_resolution_clock::now();
	// server1.generateTest_Target_All_Records(pre_enc_stack, part_A.enc_domain_map);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Gen Test n (ms)", t1, t2);


	// gamal_cipher_new(sum_cipher);

	// t1 = high_resolution_clock::now();
	// part_A.computeAnswer_opt(server1.enc_question_map, sum_cipher, part_A.histogram, servers.coll_key, epsilon);
	// // part_A.computeAnswer_modified(server1.enc_question_map, sum_cipher, part_A.histogram, part_A.cipher1, epsilon);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Compute ans Test n (ms)", t1, t2);	

	
	// threshold = size_dataset;
	// t1 = high_resolution_clock::now();
	// test_status = servers.verifyingTestResult("Test target n rows found:", sum_cipher, table, server_id, threshold);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Verify ans Test n (ms)", t1, t2);

	
	//====== RUNTIME OPTIMAL TEST FUNCTION 4 targeting specific attributes ==========//

	// t1 = high_resolution_clock::now();
	// server1.prepareTestFuntion_Query_Vector(pre_enc_stack, part_A.enc_domain_map);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Preprare Test (ms)", t1, t2);

	// t1 = high_resolution_clock::now();
	// server1.generateMatchDomain(1);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Matching Domain (ms)", t1, t2);

	// t1 = high_resolution_clock::now();
	// server1.generateTest_Target_Attr_opt(pre_enc_stack);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Gen Test Attr (ms)", t1, t2);

	// t1 = high_resolution_clock::now();
	// gamal_cipher_new(sum_cipher);
	// part_A.computeAnswer_opt(server1.enc_question_map, sum_cipher, part_A.histogram, servers.coll_key, epsilon);
	// // part_A.computeAnswer_modified(server1.enc_question_map, sum_cipher, part_A.histogram, part_A.cipher1, epsilon);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Compute ans Test Attr opt (ms)", t1, t2);

	// //Tham added 15 Jan, delete test at server side
	// server1.enc_question_map.clear();

	// t1 = high_resolution_clock::now();
	// server1.generateServerDomain_Test_Target_Attr(pre_enc_stack);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Gen test attr clear (ms)", t1, t2);

	// gamal_ciphertext_t enc_PV_answer;
	// gamal_cipher_new(enc_PV_answer);
	// server1.getTestResult_fromPV(part_A.enc_domain_map, enc_PV_answer); //partial view is encrypted with coll_key_PV_phase

	// t1 = high_resolution_clock::now();
	// cout<<"test ok"<<endl;
	// test_status = servers.verifyingTestResult_Estimate("Test attr found:", sum_cipher, table, server_id, enc_PV_answer, alpha);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Verify test attr result (ms)", t1, t2);
	// trackTaskStatus(time_track_list, "Test attr status", test_status);


    // // ==== NORMAL QUERY PRE_COMPUTE TO OPTIMIZE RUNTIME ============//
	// //=====SUM query=====
	// for(int i =1; i<=2;i++)
	// {
	// 	t1 = high_resolution_clock::now();
	// 	server1.prepareTestFuntion_Query_Vector(pre_enc_stack, part_A.enc_domain_map);
	// 	t2 = high_resolution_clock::now();
	// 	trackTaskPerformance(time_track_list, "Preprare SUM Query (ms)", t1, t2);


	// 	t1 = high_resolution_clock::now();
	// 	server1.generateMatchDomain(0);
	// 	t2 = high_resolution_clock::now();
	// 	trackTaskPerformance(time_track_list, "Gen Match Domain (ms)", t1, t2);


	// 	t1 = high_resolution_clock::now();
	// 	// server1.generateNormalQuery_opt(pre_enc_stack);
	// 	int attr_to_sum = server1.generateNormalQuery_sum(pre_enc_stack, 3);
	// 	t2 = high_resolution_clock::now();
	// 	trackTaskPerformance(time_track_list, "Gen SUM Query (ms)", t1, t2);
	// 	cout<<"attr_to_sum = "<<attr_to_sum<<endl;

	// 	t1 = high_resolution_clock::now();
	// 	gamal_cipher_new(sum_cipher);
	// 	part_A.computeAnswer_sum(server1.enc_question_map, sum_cipher, part_A.histogram, servers.coll_key, epsilon, attr_to_sum);
	// 	t2 = high_resolution_clock::now();
	// 	trackTaskPerformance(time_track_list, "Compute ans SUM Query (ms)", t1, t2);

	// 	// Re-encryption
	// 	gamal_ciphertext_t sum_cipher_update;

	// 	gamal_generate_keys(part_B.keys); //part_B keys pair   

		
	// 	gama_key_switch_lead(sum_cipher_update, sum_cipher, server1.key, part_B.keys);

	// 	for (int i=1; i< number_servers; i++)
	// 	{
	// 		gama_key_switch_follow(sum_cipher_update, sum_cipher, servers.server_vect[server_id+i].key, part_B.keys);
	// 	}
	
	// }
	

	// // threshold = PV_size; //actual PV size = V (not the PV histogram)
	// // t1 = high_resolution_clock::now();
	// // test_status = servers.verifyingTestResult("Test sum income:", sum_cipher, table, server_id, threshold);
	// // t2 = high_resolution_clock::now();
	// // trackTaskPerformance(time_track_list, "Verify test V result (ms)", t1, t2);
	// // trackTaskStatus(time_track_list, "Test target V status", test_status);

	
	
	//====COUNT query===
	// for (int i=1; i<=1; i++)
	// {

	// 	t1 = high_resolution_clock::now();
	// 	server1.prepareTestFuntion_Query_Vector(pre_enc_stack, part_A.enc_domain_map);
	// 	t2 = high_resolution_clock::now();
	// 	trackTaskPerformance(time_track_list, "Preprare COUNT Query (ms)", t1, t2);


	// 	t1 = high_resolution_clock::now();
	// 	server1.generateMatchDomain(0);
	// 	t2 = high_resolution_clock::now();
	// 	trackTaskPerformance(time_track_list, "Gen Match Domain (ms)", t1, t2);


	// 	t1 = high_resolution_clock::now();
	// 	// server1.generateNormalQuery_opt(pre_enc_stack);
	// 	server1.generateNormalQuery_opt(pre_enc_stack);
	// 	t2 = high_resolution_clock::now();
	// 	trackTaskPerformance(time_track_list, "Gen COUNT Query (ms)", t1, t2);
		

	// 	t1 = high_resolution_clock::now();
	// 	gamal_cipher_new(sum_cipher);
	// 	part_A.computeAnswer_opt(server1.enc_question_map, sum_cipher, part_A.histogram, servers.coll_key, epsilon);
	//     // part_A.computeAnswer_modified(server1.enc_question_map, sum_cipher, part_A.histogram, part_A.cipher1, epsilon);
	// 	t2 = high_resolution_clock::now();
	// 	trackTaskPerformance(time_track_list, "Compute ans COUNT Query (ms)", t1, t2);

	// 	// Re-encryption
	// 	gamal_ciphertext_t sum_cipher_update;

	// 	gamal_generate_keys(part_B.keys); //part_B keys pair   

	// 	t1 = high_resolution_clock::now();
	// 	gama_key_switch_lead(sum_cipher_update, sum_cipher, server1.key, part_B.keys);

	// 	for (int i=1; i< number_servers; i++)
	// 	{
	// 		gama_key_switch_follow(sum_cipher_update, sum_cipher, servers.server_vect[server_id+i].key, part_B.keys);
	// 	}
	// 	t2 = high_resolution_clock::now();
	// 	trackTaskPerformance(time_track_list, "Re-encryption (ms)", t1, t2);
		
	// }

	high_resolution_clock::time_point t2_e2e  = high_resolution_clock::now();

	trackTaskPerformance(time_track_list, "E2E delay", t1_e2e, t2_e2e);

	// // Re-encrypt query answer to participant B's public key

	// gamal_ciphertext_t sum_cipher_update;

	// gamal_generate_keys(part_B.keys); //part_B keys pair   

	// t1 = high_resolution_clock::now();
	// gama_key_switch_lead(sum_cipher_update, sum_cipher, server1.key, part_B.keys);
	// for (int i=1; i< number_servers; i++)
	// {
	// 	gama_key_switch_follow(sum_cipher_update, sum_cipher, servers.server_vect[server_id+i].key, part_B.keys);
	// }
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Re-encryption (ms)", t1, t2);

	// // dig_t after;
	// // gamal_decrypt(&after, part_B.keys, sum_cipher_update, table);	
	// // std::cout<<"Check after re-encryption: "<<after<<std::endl;



	// //Tham added 15 Jan, delete query at the server side
	// server1.enc_question_map.clear();





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

	// storeTimeEvaluation(argc, argv, time_track_list, verify_status);


	if (argc > 1)
	{
		fstream fout;
		if (strcmp(argv[9], "1") == 0)
		{
			// fout.open("./results/E2Eruntime_sum_query_20K_attr_income_filtering_loan5_20K_empLength_term_4Query_4Test_a_10_20runs.csv", ios::out | ios::trunc);
			fout.open("./results/runtime_new_scheme_500K_nopt_testL.csv", ios::out | ios::trunc);
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
			fout.open("./results/runtime_new_scheme_500K_nopt_testL.csv", ios::out | ios::app);
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