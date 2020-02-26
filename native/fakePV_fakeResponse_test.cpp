#include "include/public_header.h"
#include "src/Participant.h"
#include "src/Server.h"
#include "src/Servers.h"
#include "src/process_noise.h"
#include "src/time_evaluation.h"
#include "public_func.h"

int fakePV_fakeResponse_test(int argc, char **argv)
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
	float noise_budget = 1;
	float sensitivity = 1.0;
	int PV_size = (int)datasize_row*pv_ratio; // actual V, not the PV histogram form

    int num_test = (int)(num_query*test_frequency/(1-test_frequency));
    int iterations = num_query + num_test;

	// Setup answer strategy
	int answer_strategy;
	int true_fake[] = {1, 0};
	int fake_freq = 100; //freq of lie
	int freq[] = {100 - fake_freq, fake_freq}; //using fake dataset with p2 = 0.1

	float lie_freq = (float)fake_freq/100;
	int n = sizeof(true_fake)/sizeof(true_fake[0]);
	int answer_strategy_arr[iterations];

	int no_lied_answer = (int)(lie_freq*iterations);

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
	
	// }

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
	
	bsgs_table_t table;
	gamal_init(CURVE_256_SEC);
	gamal_init_bsgs_table(table, (dig_t)1L << 16);


	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	//                          SETUP PHASE                                     //
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

	// PARTICIPANT CONVERT DATASET TO HISTOGRAM
	Participant part_A(UNIQUE_DOMAIN_DIR);

	part_A.create_OriginalHistogram(datasize_row);
	
	part_A.addDummy_to_Histogram(a);
	
	int size_dataset = part_A.size_dataset;


	// SERVER SETUP COLLECTIVE KEY
	Servers servers(SERVER_SIZE, size_dataset, KNOWN_DOMAIN_DIR);

	servers.generateCollKey();
	servers.pv_ratio = pv_ratio;

	// INITIALIZE CIPHERTEXT STACK FOR PARTY
	part_A.initializePreStack(servers.coll_key);


	part_A.maxNoise_q = getLaplaceNoiseRange(sensitivity, epsilon_q, percentile_noise);
	part_A.epsilon_q = epsilon_q; // total budget is epsilon for all iterations
	part_A.minNoise_q = -part_A.maxNoise_q;
	part_A.maxNoise_test = getLaplaceNoiseRange(sensitivity, epsilon_test, percentile_noise);
	part_A.epsilon_test = epsilon_test; // total budget is epsilon for all iterations
	part_A.minNoise_test = -part_A.maxNoise_test;

	part_A.sensitivity = sensitivity;
	part_A.pv_ratio = pv_ratio;
	// INITIALIZE CIPHERTEXT STACK FOR SERVERS
	ENC_Stack pre_enc_stack(size_dataset, servers.coll_key);

	
	pre_enc_stack.initializeStack_E0();
	
	pre_enc_stack.initializeStack_E1();



	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	//          PV SUBMISSION AND VERIFICATION PHASE                            //
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

	// SERVER CREATE PV SAMPLING VECTOR
	
	servers.createPVsamplingVector(pre_enc_stack);
	

	//++++++++++ PARTY IS DISHONEST ++++++++++++//


	//====== strategy 2: participant generate fake histogram randomly
	
	int keep_row = int(datasize_row*1); //lie about 50% rows
	
	part_A.addDummy_FakeHist_random(keep_row, a);
	
	//pre process:
	
	part_A.pre_process_generatePV(false);

	part_A.generatePV_opt(servers.s_myPIR_enc, false);
	

	//======= strategy 3: participant does not use PV sampling vector from server
	
	// int true_record_PV = (int)PV_size*0.2;

	// part_A.selfCreate_Fake_Historgram(true_record_PV, a);
	
	// // //pre process:
	
	// part_A.pre_process_generatePV(false);
	
	// part_A.selfCreateFakePV_opt(false);
	


	//=====SERVER VERIFIES SUBMITTED PV
	int server_id = 0; // Server 1
	Server server1 = servers.server_vect[server_id];

	bool verify_status = servers.verifyingPV(part_A.enc_domain_map, table, server_id, pre_enc_stack, eta);


	bool test_status;
	int count_lied_ans = 0;
	int no_lied_detected = 0;
	int threshold;
	gamal_ciphertext_t sum_cipher;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	//                          QUERY & TEST PHASE                              //
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	
	//Server determines maxNoise
	servers.maxNoise = getLaplaceNoiseRange(sensitivity, epsilon_test, percentile_noise);
	servers.minNoise = -servers.maxNoise;

	cout<<"Server: maxNoise = "<<servers.maxNoise<<endl;

	part_A.no_lied_answer = (int)(lie_freq*iterations);

    int scale_up_answer = int(1/pv_ratio);
    cout<<"scale_up_answer = "<<scale_up_answer<<endl;
	
    part_A.scale_up_answer = scale_up_answer;

	int index = 0;
	int itr = 1;

   
	
	while (itr <= num_query)
    {
         // // ==== NORMAL QUERY PRE_COMPUTE TO OPTIMIZE RUNTIME ============//
      
        cout<< "\nQuery: #" << itr << endl;
		
		server1.prepareTestFuntion_Query_Vector(pre_enc_stack, part_A.enc_domain_map);
       
        server1.generateMatchDomain(0);
      
		server1.generateNormalQuery_opt(pre_enc_stack);
		
		answer_strategy = answer_strategy_arr[index];
		
		// cout<<	"answer_strategy = " <<answer_strategy<<endl;

		if (answer_strategy == 0)
		{
			
			part_A.computeAnswer_opt(server1.enc_test_map, sum_cipher, false, servers.coll_key, epsilon_q);
            // part_A.computeAnswer_scaled_up_answer(server1.enc_test_map, sum_cipher, part_A.scale_up_answer, false, servers.coll_key, epsilon_q);
			
			count_lied_ans++;
			
		}
		else
		{
			
			part_A.computeAnswer_opt(server1.enc_test_map, sum_cipher, true, servers.coll_key, epsilon_q);
			
			
		}	
		trackTaskStatus(time_track_list, "Query:Dataset true/false", answer_strategy);
		
        //Tham added 15 Jan, delete query at the server side
        server1.enc_test_map.clear();
		server1.match_query_domain_vect.clear();
        itr++;
		index++;
    }

	
	// //====TEST FUNCTION KNOWN RECORDS NEW USING PRE_COMPUTE TEST FUCTION =====//
    // Pre process
	

	for (int i = 1; i<= num_test_rounded; i++) 
	{
		cout<<"\nTest L: #"<<i<<endl;
		
		server1.prepareTestFuntion_Query_Vector(pre_enc_stack, part_A.enc_domain_map);
		
	
		server1.generateTestKnownRecords_opt(pre_enc_stack, part_A.enc_domain_map);
		
	
		answer_strategy = answer_strategy_arr[index];

		if (answer_strategy == 0)
		{
			
			part_A.computeAnswer_opt(server1.enc_test_map, sum_cipher, false, servers.coll_key, epsilon_test);
            // part_A.computeAnswer_scaled_up_answer(server1.enc_test_map, sum_cipher, part_A.scale_up_answer, false, servers.coll_key, epsilon_q);
			
			count_lied_ans++;
			
		}
		else
		{
			
			part_A.computeAnswer_opt(server1.enc_test_map, sum_cipher, true, servers.coll_key, epsilon_test);
			
			
		}	
		trackTaskStatus(time_track_list, "Test:Dataset true/false", answer_strategy);

		threshold = server1.known_record_subset.size();
		cout<< "Threshold L = " << threshold <<endl;

		test_status = servers.verifyingTestResult("Test target L known rows found:", sum_cipher, table, server_id, threshold);
		if (test_status == 0)
		{
			no_lied_detected++;
		}
		trackTaskStatus(time_track_list, "Test target L status", test_status);
		
		server1.enc_test_map_pre.clear();
		
		server1.enc_test_map.clear();
		index++;
		

	}
	
	// //===== TEST FUNCTION BASED PV OPTIMAL =====
	 
	
	for (int i=1; i<=num_test_rounded; i++)
	{
		cout<<"\nTest V: #"<<i<<endl;
		
		server1.generateTestBasedPartialView_opt(pre_enc_stack, part_A.enc_domain_map);
		

		answer_strategy = answer_strategy_arr[index];
		

		if (answer_strategy == 0)
		{
			
			part_A.computeAnswer_opt(server1.enc_test_map, sum_cipher, false, servers.coll_key, epsilon_test);
            // part_A.computeAnswer_scaled_up_answer(server1.enc_test_map, sum_cipher, part_A.scale_up_answer, false, servers.coll_key, epsilon_q);
			count_lied_ans++;
			
		}
		else
		{
		
			part_A.computeAnswer_opt(server1.enc_test_map, sum_cipher, true, servers.coll_key, epsilon_test);
			
		}	
		trackTaskStatus(time_track_list, "Test:Dataset true/false", answer_strategy);

		threshold = PV_size; 
		test_status = servers.verifyingTestResult("Test target V rows found:", sum_cipher, table, server_id, threshold);
		
		trackTaskStatus(time_track_list, "Test target V status", test_status);

		if (test_status == 0)
		{
			no_lied_detected++;
		}
		
		
		server1.enc_test_map_pre.clear();
		//Tham added 15 Jan, delete test at server side
		server1.enc_test_map.clear();
		index++;
	}

	//===== TEST FUNCTION  targeting all rows in dataset ====//

	for (int i=1; i<=num_test_rounded; i++)
	{
		
		cout<<"\nTest all rows: #"<<i<<endl;
		
		server1.generateTest_Target_All_Records(pre_enc_stack, part_A.enc_domain_map);
		gamal_cipher_new(sum_cipher);

		//===== Party answer truth or lie at random p2

		answer_strategy = answer_strategy_arr[index];

		
		if (answer_strategy == 0)
		{

			part_A.computeAnswer_opt(server1.enc_test_map, sum_cipher, false, servers.coll_key, epsilon_test);
            // part_A.computeAnswer_scaled_up_answer(server1.enc_test_map, sum_cipher, part_A.scale_up_answer, false, servers.coll_key, epsilon_q);
	
			count_lied_ans++;
		}
		else
		{
			part_A.computeAnswer_opt(server1.enc_test_map, sum_cipher, true, servers.coll_key, epsilon_test);
		}
		
		trackTaskStatus(time_track_list, "Test:Dataset true/false", answer_strategy);

		
		threshold = size_dataset;
		test_status = servers.verifyingTestResult("Test target n rows found:", sum_cipher, table, server_id, threshold);
		trackTaskStatus(time_track_list, "Test target n rows status", test_status);

		
		if (test_status == 0)
		{
			no_lied_detected++;
		}

		server1.enc_test_map.clear();
		index++;
	
	}

	//====== TEST FUNCTION 4 targeting specific attributes ==========//


	for (int i=1; i<= num_test - 3*num_test_rounded; i++)
	{
		cout<<"\nTest estimate: #"<<i<<endl;
		
		server1.prepareTestFuntion_Query_Vector(pre_enc_stack, part_A.enc_domain_map);

		server1.generateMatchDomain(1);
		

		
		server1.generateTest_Target_Attr_opt(pre_enc_stack);
		
		answer_strategy = answer_strategy_arr[index];
		
		if (answer_strategy == 0)
		{
			
			part_A.computeAnswer_opt(server1.enc_test_map, sum_cipher, false, servers.coll_key, epsilon_test);
            // part_A.computeAnswer_scaled_up_answer(server1.enc_test_map, sum_cipher, part_A.scale_up_answer, false, servers.coll_key, epsilon_q);
			
			count_lied_ans++;
			
		}
		else
		{
		
			part_A.computeAnswer_opt(server1.enc_test_map, sum_cipher, true, servers.coll_key, epsilon_test);
		
			
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
		server1.enc_test_map_pre.clear();
		//Tham added 15 Jan, delete test at server side
		server1.enc_test_map.clear();
		index++;
		

	}

   

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

	
	cout<<"index = "<<index<<endl;



	trackTestAccu(time_track_list, "No. of lied answer", count_lied_ans);
	trackTestAccu(time_track_list, "No. of lied detected", no_lied_detected);


	storeTimeEvaluation(argc, argv, time_track_list, verify_status);




	return 0;
}