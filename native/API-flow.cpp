
#include "include/public_header.h"
#include "src/Participant.h"
#include "src/Server.h"
#include "src/Servers.h"
#include "src/process_noise.h"
#include "src/time_evaluation.h"
#include "public_func.h"

/**
 * @file API_flow.h 
 * @brief An example demonstrates operations of the Data Ring system.
 * @author Tham Nguyen tham.nguyen@mq.edu.au, Nam Bui, Data Ring
 * @date April 2020
*/

/**
 * @brief Working flow of the Data Ring system
 * @details The data sharing between two data owners under the monitoring of three (3) honest-but-curious servers. 
 * The process includes four phases:
 * 1) Setup Phase
 * 2) Partial View Phase
 * 3) Query Evaluation Phase
 * 4) Data Release Phase
*/
int API_flow(int argc, char **argv)
{
	

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

	double percentile_noise = 0.95;			   //use to determine Laplace max_noise
	float noise_budget = 0.5; //overall privacy budget = budget for queries and tests (no. of query and no. of test known by everyone)
	float sensitivity = 1.0; //for counting query
	int PV_size = (int)dataset_size*pv_ratio; // actual V, not the PV histogram form

	float epsilon_q = noise_budget; //only for test runtime, noise is not a matter
	float epsilon_test = epsilon_q; //only for test runtime, noise is not a matter
	float epsilon = noise_budget;  //only for test runtime, noise is not a matter
	
	TRACK_LIST time_track_list;

	high_resolution_clock::time_point t1, t2;

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
	Participant part_B;

	// Participant represent its dataset over a histogram of <label, value(label)>; label is one of all possible records that a dataset can take
	part_A.create_OriginalHistogram(dataset_size);
	part_A.addDummy_to_Histogram(a); //for reduce overhead: limit the histogram to size of aN instead of size |D|
	

	int size_dataset = part_A.size_dataset;

	

	// SERVER SETUP COLLECTIVE KEY
	Servers servers(number_servers, size_dataset, background_knowledge_directory,a);

	servers.generateCollKey(servers.coll_key); //generate collective key for Partial View Collection Phase
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
	ENC_Stack pre_enc_stack(size_dataset, servers.coll_key);
	pre_enc_stack.initializeStack_E0(); //pre-computed enc(0)
	pre_enc_stack.initializeStack_E1(); //pre-compted enc(1)
		


	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	//          PARTIAL VIEW COLLECTION AND VERIFICATION PHASE                  //
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

	// SERVER CREATE PV SAMPLING VECTOR: (can be done by one server)
	
	servers.createPVsamplingVector_size_N_plus_1(pre_enc_stack); //length N+1: V enc(1) and N-V enc(0) at random in first N, last one is enc(0)


	// PARTY generates Partial View

	//pre process: pre-compte Enc(0) to all dummy domains at offline
	
	// part_A.pre_process_generatePV(true);
	
	//gen PV
	part_A.generatePV_fixed_scheme(servers.s_myPIR_enc, part_A.histogram, part_A.size_dataset);   
	
	
	// SERVER VERIFIES SUBMITTED PV

	int server_id = 0; // Server 1
	Server server1 = servers.server_vect[server_id];

	bool verify_status = servers.verifyingPV(part_A.enc_domain_map, table, server_id, pre_enc_stack, eta);//all servers jointly decrypt


	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	//                          QUERY & TEST PHASE                              //
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

	bool test_status;
	int threshold;
	gamal_ciphertext_t sum_cipher;
	
	//Server determines maxNoise
	servers.maxNoise = getLaplaceNoiseRange(sensitivity, epsilon, percentile_noise);
	servers.minNoise = -servers.maxNoise;

    //Servers generate a collective key pair for Query Evaluation Phase
	// servers.generateCollKey(); 

	//====TEST L: TEST FUNCTION KNOWN RECORDS NEW USING PRE_COMPUTE TEST FUCTION =====//
	
	// SERVER: Pre-compute test L

	server1.prepareTestFuntion_Query_Vector(pre_enc_stack, part_A.enc_domain_map);

	//SERVER: gen test L
	server1.generateTestKnownRecords_opt(pre_enc_stack, part_A.enc_domain_map);
	

	//PARTY compute answer:
	gamal_cipher_new(sum_cipher);
	part_A.computeAnswer_opt(server1.enc_question_map, sum_cipher, part_A.histogram, servers.coll_key, epsilon);
	

	// SERVER: verify test's answer by comparing it with L
	
	threshold = server1.known_record_subset.size();
	
	test_status = servers.verifyingTestResult("Test target L known rows found:", sum_cipher, table, server_id, threshold);
	
	server1.enc_question_map.clear();
	
	

	// //===== TEST FUNCTION BASED PV OPTIMAL =====

	//SERVER gen test V:
	server1.generateTestBasedPartialView_opt(pre_enc_stack, part_A.enc_domain_map);
	
	// PARTY compute answer
	gamal_cipher_new(sum_cipher);
	part_A.computeAnswer_opt(server1.enc_question_map, sum_cipher, part_A.histogram, servers.coll_key, epsilon);
	

	//SERVER: verify test's answer by comparing it with V
	
	threshold = PV_size; //actual PV size = V (not the PV histogram)
	
	test_status = servers.verifyingTestResult("Test target V rows found:", sum_cipher, table, server_id, threshold);
	server1.enc_question_map.clear();

	
	
	//====== TEST estimation: targeting specific attributes ==========//

	//SERVER gen test
	server1.prepareTestFuntion_Query_Vector(pre_enc_stack, part_A.enc_domain_map);
	
	server1.generateMatchDomain(1); // 1: for test function; 0: for normal query

	server1.generateTest_Target_Attr_opt(pre_enc_stack);

	//PARTY compute answer:
	
	gamal_cipher_new(sum_cipher);
	part_A.computeAnswer_opt(server1.enc_question_map, sum_cipher, part_A.histogram, servers.coll_key, epsilon);
	
	//SERVER verify test's answer by comparing it with estimated answer based on PV
	
	server1.generateServerDomain_Test_Target_Attr(pre_enc_stack);
	
	gamal_ciphertext_t enc_PV_answer;
	gamal_cipher_new(enc_PV_answer);

	server1.getTestResult_fromPV(part_A.enc_domain_map, enc_PV_answer);

	test_status = servers.verifyingTestResult_Estimate("Test attr found:", sum_cipher, table, server_id, enc_PV_answer, alpha);
	
	server1.enc_question_map.clear();

    // //==== NORMAL QUERY PRE_COMPUTE TO OPTIMIZE RUNTIME ============//

	// SERVER gen query
	server1.prepareTestFuntion_Query_Vector(pre_enc_stack, part_A.enc_domain_map);
	
	server1.generateMatchDomain(0); // 1: for test function; 0: for normal query

	server1.generateNormalQuery_opt(pre_enc_stack);

	// PARTY compute query's answer
	gamal_cipher_new(sum_cipher);
	part_A.computeAnswer_opt(server1.enc_question_map, sum_cipher, part_A.histogram, servers.coll_key, epsilon);
		

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	//                          RELEASE SHARED DATA                             //
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

	//======Re-encrypt query answer to participant B's public key

	gamal_ciphertext_t sum_cipher_update;

	gamal_generate_keys(part_B.keys); //part_B keys pair   

	
	gama_key_switch_lead(sum_cipher_update, sum_cipher, server1.key, part_B.keys);

	for (int i=1; i< number_servers; i++)
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