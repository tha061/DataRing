
#include "include/public_header.h"
#include "src/Participant.h"
#include "src/Server.h"
#include "src/Servers.h"
#include "src/process_noise.h"
#include "src/time_evaluation.h"
#include "public_func.h"

#include "testing2.cpp"

int main(int argc, char **argv)
{
	// if (argc > 1 && strcmp(argv[1], "-1") == 0)
	// {
	// 	return computeTimeEvaluation();
	// }

	int datasize_row = 500000; //make it an argument to use to determine dataset_size
	int SERVER_SIZE = 3;	   //make it an argument use to setup number of servers
	int a = 2;				   //make it an argument to scale up the histogram for adding dummy
	double eta;				   //make it an argument use to determine r0
	double alpha = 0.05;
	
	string UNIQUE_DOMAIN_DIR, KNOWN_DOMAIN_DIR;
	if (argc > 1)
	{
		if (strstr(argv[1], ".csv") != NULL && strstr(argv[2], ".csv") != NULL)
		{
			UNIQUE_DOMAIN_DIR = argv[1];
			KNOWN_DOMAIN_DIR = argv[2];
			datasize_row = stoi(argv[3]); //size of dataset
			SERVER_SIZE = stoi(argv[4]);
			a = stoi(argv[5]);
			eta = stod(argv[6]); //make it an argument use to determine r0
			alpha = stod(argv[7]);
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
	float epsilon = 0.1;
	float sensitivity = 1.0;
	int PV_size = int(datasize_row / 100); // actual V, not the PV histogram form
	int keep_rows = 400000;
	TRACK_LIST time_track_list;

	high_resolution_clock::time_point t1, t2;

	srand(time(NULL));
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

	cout << "Total rows: " << part_A.size_dataset << endl;
	cout << "Total domains: " << part_A.hashMap.size() << endl;

	t1 = high_resolution_clock::now();
	part_A.addDummy_TrueHistogram(a);
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_list, "Add dummy to Hist (ms)", t1, t2);

	int size_dataset = part_A.size_dataset;

	

	// SERVER SETUP COLLECTIVE KEY
	Servers servers(SERVER_SIZE, size_dataset, KNOWN_DOMAIN_DIR);

	servers.generateCollKey();

	// INITIALIZE CIPHERTEXT STACK FOR PARTY
	part_A.initializePreStack(servers.coll_key);


	// Participant pre process
	//pre process PV:
	t1 = high_resolution_clock::now();
	part_A.pre_process_generatePV(true);
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_list, "Pre Gen PV (ms)", t1, t2);

	//Participant determine maxNoise
	part_A.maxNoise = getLaplaceNoiseRange(sensitivity, epsilon, percentile_noise);
	part_A.epsilon = epsilon;
	part_A.sensitivity = sensitivity;
	part_A.minNoise = -part_A.maxNoise;

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

	// PARTY IS HONEST

	// t1 = high_resolution_clock::now();
	// part_A.generatePV(servers.s_plain_track_list, servers.s_myPIR_enc, true);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Gen PV (ms)", t1, t2);

	//Gen PV optimal by pre-compte Enc(0) to all dummy domains at offline
	// //pre process:
	// t1 = high_resolution_clock::now();
	// part_A.pre_process_generatePV(true);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Pre Gen PV (ms)", t1, t2);
	
	t1 = high_resolution_clock::now();
	part_A.generatePV_opt(servers.s_myPIR_enc, true);
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_list, "Gen PV (ms)", t1, t2);

	//// PARTY IS DISHONEST
	// strategy 1

	// int keep_row = int(datasize_row*0.9); //lie about 50% rows
	// t1 = high_resolution_clock::now();
	// part_A.addDummy_FakeHist(keep_row, a);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Fake Dummy Histog (ms)", t1, t2);

	// t1 = high_resolution_clock::now();
	// part_A.generatePV(servers.s_plain_track_list, servers.s_myPIR_enc, false);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Gen PV (ms)", t1, t2);

	// strategy 2
	// int keep_row = int(datasize_row*0.9); //lie about 50% rows
	// t1 = high_resolution_clock::now();
	// part_A.addDummy_FakeHist_random(keep_rows, a);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Fake Dummy Histog (ms)", t1, t2);

	// t1 = high_resolution_clock::now();
	// part_A.generatePV(servers.s_plain_track_list, servers.s_myPIR_enc, false);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Gen PV (ms)", t1, t2);

	// strategy 3

	// t1 = high_resolution_clock::now();
	// part_A.selfCreateFakePV(4000, a);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Part_A Generate PartialView", t1, t2);

	// part_A.test_cleartext();

	// SERVER VERIFIES SUBMITTED PV
	int server_id = 0; // Server 1
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

	//===== TEST FUNCTION 1 targeting L records in dataset =====//
	// t1 = high_resolution_clock::now();
	// server1.generateTestFunction(pre_enc_stack, part_A.enc_domain_map, 1);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Gen Test L (ms)", t1, t2);

	// t1 = high_resolution_clock::now();
	// gamal_cipher_new(sum_cipher);
	// part_A.computeAnswer(server1.enc_test_map, sum_cipher, true, servers.coll_key, percentile_noise);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Compute ans L (ms)", t1, t2);

	// threshold = server1.known_vector.size();
	// test_status = servers.verifyingTestResult("Test target L known rows found:", sum_cipher, table, server_id, threshold, percentile_noise);
	// trackTaskStatus(time_track_list, "Test target L status", test_status);

	// //====TEST FUNCTION KNOWN RECORDS NEW USING PRE_COMPUTE TEST FUCTION =====//
	// Pre process
	Server server1 = servers.server_vect[0];
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
	part_A.computeAnswer(server1.enc_test_map, sum_cipher, true, servers.coll_key);
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_list, "Compute ans L (ms)", t1, t2);

	threshold = server1.known_vector.size();
	test_status = servers.verifyingTestResult("Test target L known rows found:", sum_cipher, table, server_id, threshold);
	trackTaskStatus(time_track_list, "Test target L status", test_status);

	//====== TEST FUNCTION 2 targeting V records in V ======//
	// t1 = high_resolution_clock::now();
	// server1.generateTestFunction(pre_enc_stack, part_A.enc_domain_map, 2);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Gen Test V (ms)", t1, t2);

	// t1 = high_resolution_clock::now();
	// gamal_cipher_new(sum_cipher);
	// part_A.computeAnswer(server1.enc_test_map, sum_cipher, true, servers.coll_key, percentile_noise);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Compute ans V (ms)", t1, t2);

	// threshold = PV_size; //actual PV size = V (not the PV histogram)
	// test_status = servers.verifyingTestResult("Test target V rows found:", sum_cipher, table, server_id, threshold, percentile_noise);
	// trackTaskStatus(time_track_list, "Test target V status", test_status);

	// // ==== TEST FUNCTION BASED PV OPTIMAL

	// t1 = high_resolution_clock::now();
	// server1.generateTestBasedPartialView_opt(pre_enc_stack, part_A.enc_domain_map);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Gen Test V optimal (ms)", t1, t2);

	// t1 = high_resolution_clock::now();
	// gamal_cipher_new(sum_cipher);
	// part_A.computeAnswer(server1.enc_test_map, sum_cipher, true, servers.coll_key);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Compute ans V (ms)", t1, t2);

	// threshold = PV_size; //actual PV size = V (not the PV histogram)
	// test_status = servers.verifyingTestResult("Test target V rows found:", sum_cipher, table, server_id, threshold);
	// trackTaskStatus(time_track_list, "Test target V status", test_status);

	//===== TEST FUNCTION 3 targeting V - r0 records in PV ====//

	// t1 = high_resolution_clock::now();
	// server1.generateTestHashMap_3(pre_enc_stack, part_A.enc_domain_map);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Gen Test V - r0 (ms)", t1, t2);

	// t1 = high_resolution_clock::now();
	// gamal_cipher_new(sum_cipher);
	// part_A.computeAnswer(server1.enc_test_map, sum_cipher, false, servers.coll_key, percentile_noise);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Compute ans V - r0 (ms)", t1, t2);

	// threshold = PV_size - server1.verified_set.size();
	// test_status = servers.verifyingTestResult("Test target V - r0 rows found:", sum_cipher, table, server_id, threshold, percentile_noise);
	// trackTaskStatus(time_track_list, "Test target V - r0 status", test_status);

	// //====== TEST FUNCTION 4 targeting specific attributes ==========//

	// t1 = high_resolution_clock::now();
	// server1.generateTestFunction(pre_enc_stack, part_A.enc_domain_map, 4);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Gen Test Attr (ms)", t1, t2);

	// t1 = high_resolution_clock::now();
	// gamal_cipher_new(sum_cipher);
	// part_A.computeAnswer(server1.enc_test_map, sum_cipher, false, servers.coll_key, percentile_noise);
	// t2 = high_resolution_clock::now();
	
	// trackTaskPerformance(time_track_list, "Compute ans Test Attr (ms)", t1, t2);

	// gamal_ciphertext_t enc_PV_answer;
	// gamal_cipher_new(enc_PV_answer);
	// server1.getTestResult_fromPV(part_A.enc_domain_map, enc_PV_answer);

	// test_status = servers.verifyingTestResult_Estimate("Test attr found:", sum_cipher, table, server_id, enc_PV_answer, alpha, percentile_noise);
	// trackTaskStatus(time_track_list, "Test attr status", test_status);

	//====== RUNTIME OPTIMAL TEST FUNCTION 4 targeting specific attributes ==========//

	// t1 = high_resolution_clock::now();
	// server1.prepareTestFuntion_Query_Vector(pre_enc_stack, part_A.enc_domain_map);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Preprare Test (ms)", t1, t2);

	// t1 = high_resolution_clock::now();
	// server1.generateMatchDomain();
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Gen Match DOmain (ms)", t1, t2);

	// t1 = high_resolution_clock::now();
	// server1.generateTest_Target_Attr_opt(pre_enc_stack);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Gen Test Attr (ms)", t1, t2);

	// t1 = high_resolution_clock::now();
	// gamal_cipher_new(sum_cipher);
	// part_A.computeAnswer(server1.enc_test_map, sum_cipher, false, servers.coll_key);
	// t2 = high_resolution_clock::now();
	
	// trackTaskPerformance(time_track_list, "Compute ans Test Attr (ms)", t1, t2);


	// // ===================== //
	// t1 = high_resolution_clock::now();
	// server1.generateServerDomain_Test_Target_Attr(pre_enc_stack);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Gen Server Domain from test function (ms)", t1, t2);

	// gamal_ciphertext_t enc_PV_answer;
	// gamal_cipher_new(enc_PV_answer);
	// server1.getTestResult_fromPV(part_A.enc_domain_map, enc_PV_answer);

	// test_status = servers.verifyingTestResult_Estimate("Test attr found:", sum_cipher, table, server_id, enc_PV_answer, alpha);
	// trackTaskStatus(time_track_list, "Test attr status", test_status);


	// ====== NORMAL QUERY =============//

	//// Servers will not verify the answer,
	//// but re-encrypt the encrypted answer to public key of querying party (participant B)

	// t1 = high_resolution_clock::now();
	// server1.generateNormalQuery(pre_enc_stack, part_A.enc_domain_map);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Gen Query (ms)", t1, t2);

	// t1 = high_resolution_clock::now();
	// gamal_cipher_new(sum_cipher);
	// part_A.computeAnswer(server1.enc_test_map, sum_cipher, true, servers.coll_key, percentile_noise);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Compute ans Query (ms)", t1, t2);


	// // ==== NORMAL QUERY PRE_COMPUTE TO OPTIMIZE RUNTIME ============//

	// t1 = high_resolution_clock::now();
	// server1.prepareTestFuntion_Query_Vector(pre_enc_stack, part_A.enc_domain_map);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Preprare Query (ms)", t1, t2);


	// t1 = high_resolution_clock::now();
	// server1.generateMatchDomain();
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Gen Match DOmain (ms)", t1, t2);


	// t1 = high_resolution_clock::now();
	// server1.generateNormalQuery_opt(pre_enc_stack);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Gen Query (ms)", t1, t2);

	// t1 = high_resolution_clock::now();
	// gamal_cipher_new(sum_cipher);
	// part_A.computeAnswer(server1.enc_test_map, sum_cipher, true, servers.coll_key);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_list, "Compute ans Query (ms)", t1, t2);

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

	return 0;
}