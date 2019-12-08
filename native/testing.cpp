
#include "include/public_header.h"
#include "src/Participant.h"
#include "src/Server.h"
#include "src/Servers.h"

#include "testing2.cpp"

typedef map<string, string> TRACK_MAP;

int getRandomInRange(int min, int max);
void decryptFind(map<string, gamal_ciphertext_t *> enc_domain_map, gamal_key_t key, bsgs_table_t table);

double timeEvaluate(string task_name, high_resolution_clock::time_point t1, high_resolution_clock::time_point t2)
{
	double time_diff = duration_cast<nanoseconds>(t2 - t1).count();
	cout << "\n -------------------------------------------------------------------- \n";
	// cout << "\nTime Evaluation \n";
	cout << task_name << " : " << time_diff / 1000000.0 << " ms";
	cout << "\n -------------------------------------------------------------------- \n";

	return time_diff / 1000000.0;
}

void printEncData(int index, gamal_ciphertext_t *myPIR_enc);
void testWithoutDecrypt(hash_map plain_domain_map);
void addDummy(hash_map &hashMap);
void printCiphertext(gamal_ciphertext_t ciphertext);
int data_ring();

// int main(int argc, char **argv)
// {
// 	cout << argv[1] << endl;
// 	cout << argv[2] << endl;
// 	return 0;
// }

int main()
{
	srand(time(NULL));
	float epsilon = 0.1;
	float sensitivity = 1.0;
	float laplace_quantitle = 0.1;
	float scale = sensitivity / epsilon;
	float loc = 0;

	hypergeometric_distribution<> hyper_dist(1000, 5000, 500000);
// complement(norm, 0.05)

	float max_noise = quantile(complement(hyper_dist, 0.9));

	cout << "max_noise: " << max_noise << endl;
	// cout << "min_noise: " << min_noise << endl;


	// laplace_distribution<> lp_dist(loc, scale);
	// cout << "loc " << lp_dist.location() << endl;
	// cout << "scale " << lp_dist.scale() << endl;

	// // Distributional properties
	// float probability = 0.001 / 2;

	// float max_noise = quantile(lp_dist, 1 - probability);

	// float min_noise = quantile(lp_dist, probability);

	// cout << "max_noise: " << max_noise << endl;
	// cout << "min_noise: " << min_noise << endl;

	// float prob100 = 0.9999999;
	// float max_100 = quantile(lp_dist, prob100);
	// float min_100 = quantile(lp_dist, 1 - prob100);
	// cout << "range_max: " << max_100 << endl;
	// cout << "range_min: " << min_100 << endl;

	// int n= -9+ int((2* 9+ 1)* 1.* rand()/ (RAND_MAX+ 1.));
	// cout << n << endl;
	return 0;
}

void trackTaskPerformance(TRACK_MAP &time_track_map, string task_name, high_resolution_clock::time_point t1, high_resolution_clock::time_point t2)
{
	double task_time_diff = timeEvaluate(task_name, t1, t2);
	time_track_map.insert({task_name, to_string(task_time_diff)});
}

int computeTimeEvaluation()
{
	std::ifstream data("./data/report_honestParty_1M.csv");
	if (!data.is_open())
	{
		std::exit(EXIT_FAILURE);
	}

	int i = 0;
	std::string str;
	std::getline(data, str); // skip the first line

	int verify_status_1 = 0;
	int verify_status_0 = 0;

	while (!data.eof())
	{
		getline(data, str);
		if (str.empty())
		{
			continue;
		}
		string id;
		int verify;
		istringstream iss(str);
		getline(iss, id, ',');
		iss >> verify;

		if (verify == 0)
		{
			verify_status_0++;
		}

		// string id_domain = id + " " + to_string(verify);
		// cout << id_domain << endl;
		i++;
	}

	verify_status_1 = i - verify_status_0;

	cout << "Total number of pass verification iteration: " << verify_status_1 << "/" << i << endl;
	cout << "Total number of fail verification iteration: " << verify_status_0 << "/" << i << endl;

	return -1;
}

void storeTimeEvaluation(int argc, char **argv, TRACK_MAP time_track_map, bool verify_status)
{

	if (argc > 1)
	{
		fstream fout;
		if (strcmp(argv[3], "1") == 0)
		{
			fout.open("./data/report_honestParty_500K.csv", ios::out | ios::trunc);
			fout << "Iteration, Verification Status";
			for (auto itr = time_track_map.begin(); itr != time_track_map.end(); itr++)
			{
				string column = itr->first;
				fout << ", " << column;
			}
			fout << "\n";
		}
		else
		{
			fout.open("./data/report_honestParty_500K.csv", ios::out | ios::app);
		}

		// Insert the data to file
		fout << argv[3] << ", " << verify_status;
		for (auto itr = time_track_map.begin(); itr != time_track_map.end(); itr++)
		{
			string time_diff = itr->second;
			fout << ", " << time_diff;
		}
		fout << "\n";
		fout.close();
	}
}

int main5(int argc, char **argv)
{
	if (argc > 1 && strcmp(argv[1], "-1") == 0)
	{
		return computeTimeEvaluation();
	}

	string UNIQUE_DOMAIN_DIR, KNOWN_DOMAIN_DIR;
	if (argc > 1)
	{
		if (strstr(argv[1], ".csv") != NULL && strstr(argv[2], ".csv") != NULL)
		{
			UNIQUE_DOMAIN_DIR = argv[1];
			KNOWN_DOMAIN_DIR = argv[2];
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

	// srand(time(NULL));
	srand(5);
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
	part_A.processData(datasize_row);
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

	t1 = high_resolution_clock::now();
	part_A.addDummy(2);
	part_A.multiply_enc_map(servers.s_plain_track_list, servers.s_myPIR_enc, true);
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_map, "Gen PV (ms)", t1, t2);

	// PARTY IS DISHONEST

	// part_A.addDummyFake_1(300000, 2);
	// part_A.addDummyFake_2(100000, 2);
	// part_A.multiply_enc_fake_map(servers.s_plain_track_list, servers.s_myPIR_enc);

	// t1 = high_resolution_clock::now();
	// part_A.selfIntializePV(4000, 2);
	// t2 = high_resolution_clock::now();
	// trackTaskPerformance(time_track_map, "Part_A Generate PartialView", t1, t2);

	// part_A.testWithoutDecrypt();

	// SERVER VERIFIES SUBMITTED PV
	int server_id = 0; // Server 1
	t1 = high_resolution_clock::now();
	bool verify_status = servers.verificationPV(part_A.enc_domain_map, table, server_id, pre_enc_stack);
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_map, "Verify PV (ms)", t1, t2);

	bool test_status;
	int threshold;
	Server server1 = servers.server_vect[0];
	gamal_ciphertext_t sum_cipher;
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	//                          QUERY & TEST PHASE                              //
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

	//===== TEST FUNCTION 1 targeting L records in dataset =====//
	t1 = high_resolution_clock::now();
	server1.generateTestHashMap_1(pre_enc_stack, part_A.enc_domain_map);
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_map, "Gen Test L rows (ms)", t1, t2);

	t1 = high_resolution_clock::now();
	gamal_cipher_new(sum_cipher);
	part_A.proceedTestFunction(server1.enc_test_map, sum_cipher, true);
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_map, "Compute answer L (ms)", t1, t2);

	threshold = server1.known_vector.size();
	test_status = servers.verificationTestResult("Test function - Count of L known data:", sum_cipher, table, server_id, threshold);
	time_track_map.insert({"Test function - L", to_string(test_status)});

	//====== TEST FUNCTION 2 targeting V records in V ======//
	t1 = high_resolution_clock::now();
	server1.generateTestHashMap_2(pre_enc_stack, part_A.enc_domain_map);
	t2 = high_resolution_clock::now();
	// timeEvaluate("generateTestHashMap_V", t1, t2);
	trackTaskPerformance(time_track_map, "Gen Test V rows (ms)", t1, t2);

	t1 = high_resolution_clock::now();
	gamal_cipher_new(sum_cipher);
	part_A.proceedTestFunction(server1.enc_test_map, sum_cipher, true);
	t2 = high_resolution_clock::now();
	timeEvaluate("proceedTestFunction_V", t1, t2);
	trackTaskPerformance(time_track_map, "Compute ans V (ms)", t1, t2);

	threshold = 5000;
	test_status = servers.verificationTestResult("Test function - Count of V known data:", sum_cipher, table, server_id, threshold);
	time_track_map.insert({"Test function - V", to_string(test_status)});

	//===== TEST FUNCTION 3 targeting V - r0 records in PV ====//

	t1 = high_resolution_clock::now();
	server1.generateTestHashMap_3(pre_enc_stack, part_A.enc_domain_map);
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_map, "GenerateTestHashMap_3", t1, t2);
	time_track_map.insert({"Test function - L", to_string(test_status)});

	// ========================= PARTICIPANT ==========================/
	gamal_cipher_new(sum_cipher);
	part_A.proceedTestFunction(server1.enc_test_map, sum_cipher, true);

	// ========================= SERVER ==========================/
	threshold = 5000 - server1.verified_set.size();
	test_status = servers.verificationTestResult("Test function - Count of V - r0 data:", sum_cipher, table, server_id, threshold);
	time_track_map.insert({"Test function - V - r0", to_string(test_status)});

	// ====== NORMAL QUERY ======= //

	// Servers will not verify the answer,
	// but re-encrypt the encrypted answer to public key of querying party (participant B)

	// t1 = high_resolution_clock::now();
	// server1.generateTestHashMap_Attr(pre_enc_stack, part_A.enc_domain_map);
	// t2 = high_resolution_clock::now();
	// timeEvaluate("generateTestHashMap_Attr", t1, t2);
	// trackTaskPerformance(time_track_map, "Gen normal Query (ms)", t1, t2);

	// t1 = high_resolution_clock::now();
	// gamal_cipher_new(sum_cipher);
	// part_A.proceedTestFunction(server1.enc_test_map, sum_cipher, true);
	// t2 = high_resolution_clock::now();
	// // timeEvaluate("proceedTestFunction_Attr", t1, t2);
	// trackTaskPerformance(time_track_map, "Compute answer Query (ms)", t1, t2);

	// //Need re-encryption here

	// threshold = 1;
	// test_status = servers.verificationTestResult("Test function - QUERY:", sum_cipher, table, server_id, threshold);
	// time_track_map.insert({"Test function - Query", to_string(test_status)});

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	//                          FINISHED SHARING                               //
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

	storeTimeEvaluation(argc, argv, time_track_map, verify_status);

	return 0;
}

void printEncData(int index, gamal_ciphertext_t *myPIR_enc)
{
	extern EC_GROUP *init_group;
	BIGNUM *x = BN_new();
	BIGNUM *y = BN_new();

	cout << "Print encryption of row index" << index << endl;
	printf("encryption of row index #%d->C1:\n", index);
	if (EC_POINT_get_affine_coordinates_GFp(init_group, myPIR_enc[index]->C1, x, y, NULL))
	{
		BN_print_fp(stdout, x);
		putc('\n', stdout);
		BN_print_fp(stdout, y);
		putc('\n', stdout);
	}
	else
	{
		std::cerr << "Can't get point coordinates." << std::endl;
	}
	printf("\n");
	printf("encryption of row index #%d->C2:\n", index);
	if (EC_POINT_get_affine_coordinates_GFp(init_group, myPIR_enc[index]->C2, x, y, NULL))
	{
		BN_print_fp(stdout, x);
		putc('\n', stdout);
		BN_print_fp(stdout, y);
		putc('\n', stdout);
	}
	else
	{
		std::cerr << "Can't get point coordinates." << std::endl;
	}

	printf("\n");
}

void printCiphertext(gamal_ciphertext_t ciphertext)
{
	extern EC_GROUP *init_group;
	BIGNUM *x = BN_new();
	BIGNUM *y = BN_new();

	if (EC_POINT_get_affine_coordinates_GFp(init_group, ciphertext->C1, x, y, NULL))
	{
		BN_print_fp(stdout, x);
		putc('\n', stdout);
		BN_print_fp(stdout, y);
		putc('\n', stdout);
	}
	else
	{
		std::cerr << "Can't get point coordinates." << std::endl;
	}
	printf("\n");
	if (EC_POINT_get_affine_coordinates_GFp(init_group, ciphertext->C2, x, y, NULL))
	{
		BN_print_fp(stdout, x);
		putc('\n', stdout);
		BN_print_fp(stdout, y);
		putc('\n', stdout);
	}
	else
	{
		std::cerr << "Can't get point coordinates." << std::endl;
	}

	printf("\n");
}

void decryptFind(map<string, gamal_ciphertext_t *> enc_domain_map, gamal_key_t key, bsgs_table_t table)
{
	int count1 = 0;
	dig_t res;
	for (map<string, gamal_ciphertext_t *>::iterator itr = enc_domain_map.begin(); itr != enc_domain_map.end(); ++itr)
	{
		gamal_decrypt(&res, key, itr->second[0], table);
		if (res > 0)
		{
			count1 += res;
			cout << "Decrypt: " << res << " of key " << itr->first << endl;
		}
	}
	cout << "Total count of chosen plaintext 1 from server: " << count1 << endl;
}

int getRandomInRange(int min, int max)
{
	return min + (rand() % (max - min + 1));
}

