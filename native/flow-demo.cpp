
#include "include/public_header.h"
#include "src/Participant.h"
#include "src/Server.h"
#include "src/Servers.h"
#include "src/process_noise.h"
#include "src/time_evaluation.h"
#include "public_func.h"


int flow_demo(int argc, char **argv)
{
	

	int datasize_row = 500000; //make it an argument to use to determine dataset_size
	int SERVER_SIZE = 3;	   //make it an argument use to setup number of servers
	int a = 2;				   //make it an argument to scale up the histogram for adding dummy
	double eta;				   //make it an argument use to determine r0
	double alpha = 0.05;
	double pv_ratio = 0.01;
	
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

	float epsilon_q = noise_budget; //only for test runtime, noise is not a matter
	float epsilon_test = epsilon_q; //only for test runtime, noise is not a matter
	float epsilon = noise_budget;  //only for test runtime, noise is not a matter
	
	TRACK_LIST time_track_list;

	high_resolution_clock::time_point t1, t2;

	srand(time(NULL));
	bsgs_table_t table;
	gamal_init(CURVE_256_SEC);
	gamal_init_bsgs_table(table, (dig_t)1L << 16);

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	//                          SETUP PHASE                                     //
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

	// PARTICIPANT CONVERT DATASET TO HISTOGRAM
	Participant part_A(UNIQUE_DOMAIN_DIR);
	Participant part_B;

	
	part_A.create_OriginalHistogram(datasize_row);
	
	
	part_A.addDummy_to_Histogram(a);
	

	int size_dataset = part_A.size_dataset;

	

	// SERVER SETUP COLLECTIVE KEY
	Servers servers(SERVER_SIZE, size_dataset, KNOWN_DOMAIN_DIR);

	servers.generateCollKey();
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

	// cout<<"Party A: maxNoise query = "<< part_A.maxNoise_q<<endl;
	// cout<<"Party A: maxNoise test = "<< part_A.maxNoise_test<<endl;

	// INITIALIZE CIPHERTEXT STACK FOR SERVERS
	ENC_Stack pre_enc_stack(size_dataset, servers.coll_key);

	
	pre_enc_stack.initializeStack_E0();
	
	pre_enc_stack.initializeStack_E1();
		


	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	//          PV SUBMISSION AND VERIFICATION PHASE                            //
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

	// SERVER CREATE PV SAMPLING VECTOR
	
	servers.createPVsamplingVector(pre_enc_stack);

	// PARTY gen PV

	//pre process: pre-compte Enc(0) to all dummy domains at offline
	
	part_A.pre_process_generatePV(true);
	
	//gen PV
	part_A.generatePV_opt(servers.s_myPIR_enc, true);
	


	// SERVER VERIFIES SUBMITTED PV

	int server_id = 0; // Server 1
	Server server1 = servers.server_vect[server_id];

	bool verify_status = servers.verifyingPV(part_A.enc_domain_map, table, server_id, pre_enc_stack, eta);


	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	//                          QUERY & TEST PHASE                              //
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

	bool test_status;
	int threshold;
	gamal_ciphertext_t sum_cipher;
	
	//Server determines maxNoise
	servers.maxNoise = getLaplaceNoiseRange(sensitivity, epsilon, percentile_noise);
	servers.minNoise = -servers.maxNoise;

    

	//====TEST L: TEST FUNCTION KNOWN RECORDS NEW USING PRE_COMPUTE TEST FUCTION =====//
	
	// SERVER: Pre-compute test L

	server1.prepareTestFuntion_Query_Vector(pre_enc_stack, part_A.enc_domain_map);

	//SERVER: gen test L
	server1.generateTestKnownRecords_opt(pre_enc_stack, part_A.enc_domain_map);
	

	//PARTY compute answer:
	gamal_cipher_new(sum_cipher);
	part_A.computeAnswer_opt(server1.enc_test_map, sum_cipher, true, servers.coll_key, epsilon);
	

	// SERVER: verify test's answer by comparing it with L
	
	threshold = server1.known_record_subset.size();
	
	test_status = servers.verifyingTestResult("Test target L known rows found:", sum_cipher, table, server_id, threshold);
	
	server1.enc_test_map.clear();

	// //===== TEST FUNCTION BASED PV OPTIMAL =====

	//SERVER gen test V:
	server1.generateTestBasedPartialView_opt(pre_enc_stack, part_A.enc_domain_map);
	
	// PARTY compute answer
	gamal_cipher_new(sum_cipher);
	part_A.computeAnswer_opt(server1.enc_test_map, sum_cipher, true, servers.coll_key, epsilon);
	

	//SERVER: verify test's answer by comparing it with V
	
	threshold = PV_size; //actual PV size = V (not the PV histogram)
	
	test_status = servers.verifyingTestResult("Test target V rows found:", sum_cipher, table, server_id, threshold);
	server1.enc_test_map.clear();

	
	
	//====== TEST estimation: targeting specific attributes ==========//

	//SERVER gen test
	server1.prepareTestFuntion_Query_Vector(pre_enc_stack, part_A.enc_domain_map);
	
	server1.generateMatchDomain(1); // 1: for test function; 0: for normal query

	server1.generateTest_Target_Attr_opt(pre_enc_stack);

	//PARTY compute answer:
	
	gamal_cipher_new(sum_cipher);
	part_A.computeAnswer_opt(server1.enc_test_map, sum_cipher, true, servers.coll_key, epsilon);
	
	//SERVER verify test's answer by comparing it with estimated answer based on PV
	
	server1.generateServerDomain_Test_Target_Attr(pre_enc_stack);
	
	gamal_ciphertext_t enc_PV_answer;
	gamal_cipher_new(enc_PV_answer);
	server1.getTestResult_fromPV(part_A.enc_domain_map, enc_PV_answer);

	test_status = servers.verifyingTestResult_Estimate("Test attr found:", sum_cipher, table, server_id, enc_PV_answer, alpha);
	
	server1.enc_test_map.clear();

    // //==== NORMAL QUERY PRE_COMPUTE TO OPTIMIZE RUNTIME ============//

	// SERVER gen query
	server1.prepareTestFuntion_Query_Vector(pre_enc_stack, part_A.enc_domain_map);
	
	server1.generateMatchDomain(0); // 1: for test function; 0: for normal query

	server1.generateNormalQuery_opt(pre_enc_stack);

	// PARTY compute query's answer
	gamal_cipher_new(sum_cipher);
	part_A.computeAnswer_opt(server1.enc_test_map, sum_cipher, true, servers.coll_key, epsilon);
		

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	//                          RELEASE SHARED DATA                             //
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

	//======Re-encrypt query answer to participant B's public key

	gamal_ciphertext_t sum_cipher_update;

	gamal_generate_keys(part_B.keys); //part_B keys pair   

	
	gama_key_switch_lead(sum_cipher_update, sum_cipher, server1.key, part_B.keys);

	for (int i=1; i< SERVER_SIZE; i++)
	{
		gama_key_switch_follow(sum_cipher_update, sum_cipher, servers.server_vect[server_id+i].key, part_B.keys);
	}
	

	dig_t after;
	gamal_decrypt(&after, part_B.keys, sum_cipher_update, table);	
	std::cout<<"Check after re-encryption: "<<after<<std::endl;


	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	//                          FINISHED SHARING                               //
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	


	return 0;
}