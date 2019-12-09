
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

	double probability = 0.9;
	string UNIQUE_DOMAIN_DIR, KNOWN_DOMAIN_DIR;
	if (argc > 1)
	{
		if (strstr(argv[1], ".csv") != NULL && strstr(argv[2], ".csv") != NULL&& strstr(argv[3], "0.") != NULL)
		{
			UNIQUE_DOMAIN_DIR = argv[1];
			KNOWN_DOMAIN_DIR = argv[2];
			probability = stod(argv[3]);
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

	const int SERVER_SIZE = 3;
	TRACK_MAP time_track_map;

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

	int datasize_row = 500000;

	t1 = high_resolution_clock::now();
	part_A.create_TrueHistogram(datasize_row);
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_map, "Create Hist (ms)", t1, t2);

	cout << "Total rows: " << part_A.size_dataset << endl;
	cout << "Total domains: " << part_A.hashMap.size() << endl;

	int size_dataset = part_A.size_dataset;

	// SERVER SETUP COLLECTIVE KEY
	Servers servers(SERVER_SIZE, size_dataset, KNOWN_DOMAIN_DIR);

	servers.generateCollKey();

	// INITIALIZE CIPHERTEXT STACK FOR PARTY
	part_A.initializePreStack(servers.coll_key);

	// INITIALIZE CIPHERTEXT STACK FOR SERVERS
	ENC_Stack pre_enc_stack(size_dataset, servers.coll_key);

	t1 = high_resolution_clock::now();
	pre_enc_stack.initializeStack_E0();
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_map, "Precompute Enc0 (ms)", t1, t2);
	cout << endl;

	t1 = high_resolution_clock::now();
	pre_enc_stack.initializeStack_E1();
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_map, "Precompute Enc1 (ms)", t1, t2);
	cout << endl;


	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	//          PV SUBMISSION AND VERIFICATION PHASE                            //
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

	// SERVER CREATE PV SAMPLING VECTOR
	t1 = high_resolution_clock::now();
	servers.createServersEncrypVector(pre_enc_stack);
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_map, "Gen sample vector(ms)", t1, t2);

	// PART IS HONEST

	// t1 = high_resolution_clock::now();
	part_A.addDummy_TrueHistogram(2);
	part_A.generatePV(servers.s_plain_track_list, servers.s_myPIR_enc, true);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_map, "Gen PV (ms)", t1, t2);

	// PARTY IS DISHONEST

	// t1 = high_resolution_clock::now();
	// part_A.addDummyFake_1(300000, 2);
	// t1 = high_resolution_clock::now();
	// part_A.addDummyFake_2(100000, 2);
	// part_A.generatePV(servers.s_plain_track_list, servers.s_myPIR_enc, false);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_map, "Gen PV (ms)", t1, t2);

	// t1 = high_resolution_clock::now();
	// part_A.selfIntializePV(4000, 2);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_map, "Part_A Generate PartialView", t1, t2);

	// part_A.testWithoutDecrypt();

	// SERVER VERIFIES SUBMITTED PV
	int server_id = 0; // Server 1
	t1 = high_resolution_clock::now();
	bool verify_status = servers.verificationPV(part_A.enc_domain_map, table, server_id, pre_enc_stack, probability);
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_map, "Verify PV (ms)", t1, t2);

	bool test_status;
	int threshold;
	Server server1 = servers.server_vect[0];
	gamal_ciphertext_t sum_cipher;
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	//                          QUERY & TEST PHASE                              //
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

	// //===== TEST FUNCTION 1 targeting L records in dataset =====//
	// t1 = high_resolution_clock::now();
	// server1.generateTestFunction(pre_enc_stack, part_A.enc_domain_map, 1);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_map, "Gen Test L rows (ms)", t1, t2);

	// t1 = high_resolution_clock::now();
	// gamal_cipher_new(sum_cipher);
	// part_A.computeAnswer(server1.enc_test_map, sum_cipher, false, servers.coll_key, probability);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_map, "Compute answer L (ms)", t1, t2);

	// threshold = server1.known_vector.size();
	// test_status = servers.verificationTestResult("Test function - Count of L known data:", sum_cipher, table, server_id, threshold, probability);
	// time_track_map.insert({"Test function - L", to_string(test_status)});

	// //====== TEST FUNCTION 2 targeting V records in V ======//
	// t1 = high_resolution_clock::now();
	// server1.generateTestFunction(pre_enc_stack, part_A.enc_domain_map, 2);
	// t2 = high_resolution_clock::now();
	// // timeEvaluate("generateTestHashMap_V", t1, t2);
	// trackTaskPerformance(time_track_map, "Gen Test V rows (ms)", t1, t2);

	// t1 = high_resolution_clock::now();
	// gamal_cipher_new(sum_cipher);
	// part_A.computeAnswer(server1.enc_test_map, sum_cipher, false, servers.coll_key, probability);
	// t2 = high_resolution_clock::now();
	// timeEvaluate("proceedTestFunction_V", t1, t2);
	// trackTaskPerformance(time_track_map, "Compute ans V (ms)", t1, t2);

	// threshold = servers.data_size; //actual PV size
	// test_status = servers.verificationTestResult("Test function - Count of V known data:", sum_cipher, table, server_id, threshold, probability);
	// time_track_map.insert({"Test function - V", to_string(test_status)});

	// //===== TEST FUNCTION 3 targeting V - r0 records in PV ====//

	// t1 = high_resolution_clock::now();
	// server1.generateTestHashMap_3(pre_enc_stack, part_A.enc_domain_map);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_map, "GenerateTestHashMap_3", t1, t2);
	// time_track_map.insert({"Test function - L", to_string(test_status)});

	// gamal_cipher_new(sum_cipher);
	// part_A.computeAnswer(server1.enc_test_map, sum_cipher, false, servers.coll_key, probability);

	// threshold = servers.data_size - server1.verified_set.size();
	// test_status = servers.verificationTestResult("Test function - Count of V - r0 data:", sum_cipher, table, server_id, threshold, probability);
	// time_track_map.insert({"Test function - V - r0", to_string(test_status)});

	// ====== NORMAL QUERY ======= //

	// Servers will not verify the answer,
	// but re-encrypt the encrypted answer to public key of querying party (participant B)

	t1 = high_resolution_clock::now();
	server1.generateTestFunction(pre_enc_stack, part_A.enc_domain_map, 4);
	t2 = high_resolution_clock::now();
	timeEvaluate("generateTestHashMap_Attr", t1, t2);
	trackTaskPerformance(time_track_map, "Gen normal Query (ms)", t1, t2);

	t1 = high_resolution_clock::now();
	gamal_cipher_new(sum_cipher);
	part_A.computeAnswer(server1.enc_test_map, sum_cipher, false, servers.coll_key, probability);
	t2 = high_resolution_clock::now();
	// timeEvaluate("proceedTestFunction_Attr", t1, t2);
	trackTaskPerformance(time_track_map, "Compute answer Query (ms)", t1, t2);

	int conf_min = (int)(server1.conf_range[0]);
	int conf_max = (int)(server1.conf_range[1]);

	servers.verificationTestResult_Estimate("Test function - QUERY:", sum_cipher, table, server_id, conf_min, conf_max);


	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	//                          FINISHED SHARING                               //
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

	// cout << "Track map size: " << time_track_map.size() << endl;

	storeTimeEvaluation(argc, argv, time_track_map, verify_status);

	return 0;
}