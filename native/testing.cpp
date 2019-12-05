
#include "include/public_header.h"
#include "src/Participant.h"
#include "src/Server.h"
#include "src/Servers.h"

#include "testing2.cpp"

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

int main2()
{
	srand(time(NULL));
	float epsilon = 0.1;
	float sensitivity = 1.0;
	float laplace_quantitle = 0.1;
	float scale = sensitivity / epsilon;
	float loc = 0;
}

// int dataring();

// 	float probMax = 0.9999999;
// 	float max_100 = quantile(lp_dist, probMax);
// 	float min_100 = quantile(lp_dist, 1 - probMax);
// 	cout << "range_max: " << max_100 << endl;
// 	cout << "range_min: " << min_100 << endl;

// 	float n = min_100 + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max_100 - min_100)));
// 	cout << n << endl;
// 	return 0;
// }

void trackTaskPerformance(map<string, double> &time_track_map, string task_name, high_resolution_clock::time_point t1, high_resolution_clock::time_point t2)
{
	double task_time_diff = timeEvaluate(task_name, t1, t2);
	time_track_map.insert({task_name, task_time_diff});
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

void storeTimeEvaluation(int argc, char **argv, map<string, double> time_track_map, bool verify_status)
{

	if (argc > 1)
	{
		fstream fout;
		if (strcmp(argv[1], "1") == 0)
		{
			fout.open("./data/report_honestParty_1M.csv", ios::out | ios::trunc);
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
		fout << argv[1] << ", " << verify_status;
		for (auto itr = time_track_map.begin(); itr != time_track_map.end(); itr++)
		{
			double time_diff = itr->second;
			fout << ", " << time_diff;
		}
		fout << "\n";
		fout.close();
	}
}

int main(int argc, char **argv)
{
	if (argc > 1 && strcmp(argv[1], "-1") == 0)
	{
		return computeTimeEvaluation();
	}

	const int SERVER_SIZE = 3;
	map<string, double> time_track_map;

	high_resolution_clock::time_point t1, t2;

	srand(time(NULL));
	// srand(5);
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
	Participant part_A;

	int datasize_row = 500000;

	t1 = high_resolution_clock::now();
	part_A.processData(datasize_row);
	t2 = high_resolution_clock::now();
	trackTaskPerformance(time_track_map, "Create Hist (ms)", t1, t2);

	cout << "Total rows: " << part_A.size_dataset << endl;
	cout << "Total domains: " << part_A.hashMap.size() << endl;
	

	int size_dataset = part_A.size_dataset;


	// SERVER SETUP COLLECTIVE KEY 
	Servers servers(SERVER_SIZE, size_dataset);

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
	part_A.multiply_enc_map(servers.s_plain_track_list, servers.s_myPIR_enc);
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


	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	//                          QUERY & TEST PHASE                              // 
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

	//===== TEST FUNCTION 1 targeting L records in dataset =====//
	Server server1 = servers.server_vect[0];
	t1 = high_resolution_clock::now();
	server1.generateTestHashMap_1(pre_enc_stack, part_A.enc_domain_map);
	t2 = high_resolution_clock::now();
	// timeEvaluate("generateTestHashMap_L", t1, t2);
	trackTaskPerformance(time_track_map, "Gen Test L rows (ms)", t1, t2);
	
	t1 = high_resolution_clock::now();
	gamal_ciphertext_t sum_cipher;
	gamal_cipher_new(sum_cipher);
	part_A.proceedTestFunction(server1.enc_test_map, sum_cipher);
	t2 = high_resolution_clock::now();
	// timeEvaluate("proceedTestFunction_L", t1, t2);
	trackTaskPerformance(time_track_map, "Compute answer L (ms)", t1, t2);
	
	dig_t decrypt_test_f1 = servers._fusionDecrypt(sum_cipher, table, server_id);
	cout << "Test function - Count of L known data: " << decrypt_test_f1 << endl;


	//====== TEST FUNCTION 2 targeting V records in V ======//
	t1 = high_resolution_clock::now();
	server1.generateTestHashMap_2(pre_enc_stack, part_A.enc_domain_map);

	t2 = high_resolution_clock::now();
	// timeEvaluate("generateTestHashMap_V", t1, t2);
	trackTaskPerformance(time_track_map, "Gen Test V rows (ms)", t1, t2);

	t1 = high_resolution_clock::now();
	gamal_cipher_new(sum_cipher);
	part_A.proceedTestFunction(server1.enc_test_map, sum_cipher);
	t2 = high_resolution_clock::now();
	timeEvaluate("proceedTestFunction_V", t1, t2);
	trackTaskPerformance(time_track_map, "Compute ans V (ms)", t1, t2);

	dig_t decrypt_test_f2 = servers._fusionDecrypt(sum_cipher, table, server_id);
	cout << "Test function - Count of V known data: " << decrypt_test_f2 << endl;

	//===== TEST FUNCTION 3 targeting V - r0 records in PV ====//
	
	// t1 = high_resolution_clock::now();

	// server1.generateTestHashMap_3(pre_enc_stack, part_A.enc_domain_map);

	// t2 = high_resolution_clock::now();
	// timeEvaluate("IgenerateTestHashMap_3", t1, t2);

	
	// gamal_cipher_new(sum_cipher);
	// part_A.proceedTestFunction(server1.enc_test_map, sum_cipher);

	
	// dig_t decrypt_test_f3 = servers._fusionDecrypt(sum_cipher, table, 0);
	// cout << "Test function - Count of V - r0 data: " << decrypt_test_f3 << endl;

	// ===== TEST FUNCTION 4 targeting multiple attributes (not a whole row)===//

	// t1 = high_resolution_clock::now();
	// int col = 2;
	// string value = "5000";
	// map<int, string> cols_map;
	// cols_map.insert({0, "5000"});
	// cols_map.insert({1, "5000"});
	// cols_map.insert({2, "5000"});
	// cols_map.insert({3, "180000.0"});
	// cols_map.insert({5, "11.99"});
	// cols_map.insert({6, "B"});
	// server1.generateTestHashMap_Attr(pre_enc_stack, part_A.enc_domain_map, cols_map);

	// t2 = high_resolution_clock::now();
	// timeEvaluate("generateTestHashMap_Attr", t1, t2);

	// t1 = high_resolution_clock::now();
	// gamal_cipher_new(sum_cipher);
	// part_A.proceedTestFunction(server1.enc_test_map, sum_cipher);
	// t2 = high_resolution_clock::now();
	// timeEvaluate("proceedTestFunction_Attr", t1, t2);

	
	// dig_t decrypt_test_f4 = servers._fusionDecrypt(sum_cipher, table, 0);
	// cout << "Test function - Count of specified attribute: " << decrypt_test_f4 << endl;
	

	// ====== NORMAL QUERY ======= //

	// Servers will not verify the answer, 
	// but re-encrypt the encrypted answer to public key of querying party (participant B)

	t1 = high_resolution_clock::now();
	int col = 2;
	string value = "5000";
	map<int, string> cols_map;
	cols_map.insert({0, "5000"});
	cols_map.insert({1, "5000"});
	cols_map.insert({2, "5000"});
	cols_map.insert({3, "180000.0"});
	cols_map.insert({5, "11.99"});
	cols_map.insert({6, "B"});
	server1.generateTestHashMap_Attr(pre_enc_stack, part_A.enc_domain_map, cols_map);
	t2 = high_resolution_clock::now();
	timeEvaluate("generateTestHashMap_Attr", t1, t2);
	trackTaskPerformance(time_track_map, "Gen normal Query (ms)", t1, t2);

	t1 = high_resolution_clock::now();
	gamal_cipher_new(sum_cipher);
	part_A.proceedTestFunction(server1.enc_test_map, sum_cipher);
	t2 = high_resolution_clock::now();
	// timeEvaluate("proceedTestFunction_Attr", t1, t2);
	trackTaskPerformance(time_track_map, "Compute answer Query (ms)", t1, t2);

	//Need re-encryption here

	dig_t decrypt_test_f4 = servers._fusionDecrypt(sum_cipher, table, 0);
	cout << "Normal query - Count of specified attribute: " << decrypt_test_f4 << endl;
	
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

int main1()
{
	std::cout << OPENSSL_VERSION_TEXT << std::endl;

	//EC_GROUP *init_group = NULL;

	//====== SETUP ========
	// pre_encryption(100);

	// partialViewCollect_histogram(1000000, 5);

	// partialViewCollect_histogram_map(1000,2);

	// ======= Plain EC-ElGamal =====

	// test1();
	// std::cout << "Plain EC-ElGamal 32-bit integers" << std::endl;
	//bench_elgamal(10, 16);
	//std::cout << "Plain EC-ElGamal 32-bit integers" << std::endl;
	//bench_elgamal_add(10, 16);
	// std::cout << "Plain EC-ElGamal 32-bit integers" << std::endl;
	//bench_elgamal_mult(10, 16, 0);
	// std::cout<<"Test mult opt: "<<std::endl;
	test_mult_opt(10, 0);

	// std::cout<<"Test query computation: "<<std::endl;
	// test_COUNT_query_computation(1000000);
	//test_SUM_query_computation(10);

	//==== Collective Key Gen for EC-ElGamal=====
	// std::cout<< "EC ElGamal test collective key gen and threshold decryption"<<std::endl;
	// test_coll_key_gen(500);
	// test_threshold_decrypt(100);
	//test_coll_decrypt();
	//std::cout<< "EC ElGamal test key switching"<<std::endl;
	//test_key_switch(1000);

	//printf("08%d", convert_to_bin(13));
	//convert_to_bin(13);
	//std::cout<< convert_to_bin(16)<<std::endl;

	//std::cout<<"Test binary convert"<<std::endl;
	//test_binary_inter(15000);

	//test_NAF_decode(19);

	return 0;
}

#if 0
//#include <iostream>
#include <sys/time.h>
#include <sys/resource.h>
#include <errno.h>
using namespace std;

int main()
{
   struct rlimit sl;
   int returnVal = getrlimit(RLIMIT_STACK, &sl);
   if (returnVal == -1)
   {
      cout << "Error. errno: " << errno << endl;
   }
   else if (returnVal == 0)
   {
      cout << "stackLimit soft - max : " << sl.rlim_cur << " - " << sl.rlim_max << endl;
   }
}

//answer: stackLimit soft - max : 8388608 - 18446744073709551615

#endif

// ===== CRT EC-ElGamal ====
// test2();
//std::cout << "CRT optimized EC-ElGamal 32-bit integers" << std::endl;
// bench_crtelgamal(5000000, 16, 32);

//std::cout << "CRT optimized EC-ElGamal 64-bit integers" << std::endl;
//  bench_crtelgamal(1000, 17, 64);

//std::cout << "CRT optimized EC-ElGamal Addition 64 bits" << std::endl;
//bench_crtelgamal_add(100, 16, 64);

//std::cout << "CRT optimized EC-ElGamal Addition 32 bits" << std::endl;
//bench_crtelgamal_add(1000, 16, 32); //only work for integer very smaller than exp2(32)

//std::cout << "CRT optimized EC-ElGamal multi, 32-bit integer" << std::endl;
//bench_crtelgamal_mult(1000, 16, 32);

//std::cout << "CRT optimized EC-ElGamal multi, 64-bit integer" << std::endl;
//bench_crtelgamal_mult(1, 16, 64, 4);
