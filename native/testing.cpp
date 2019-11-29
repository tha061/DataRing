
#include "include/public_header.h"
#include "src/Participant.h"
#include "src/Server.h"
#include "src/Servers.h"

int getRandomInRange(int min, int max);
void decryptFind(map<string, gamal_ciphertext_t *> enc_domain_map, gamal_key_t key, bsgs_table_t table);

void timeEvaluate(string task_name, high_resolution_clock::time_point t1, high_resolution_clock::time_point t2)
{
	double time_diff = duration_cast<nanoseconds>(t2 - t1).count();
	cout << "\n -------------------------------------------------------------------- \n";
	// cout << "\nTime Evaluation \n";
	cout << task_name << " : " << time_diff / 1000000.0 << " ms";
	cout << "\n -------------------------------------------------------------------- \n";
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

	laplace_distribution<> lp_dist(loc, scale);
	cout << "loc " << lp_dist.location() << endl;
	cout << "scale " << lp_dist.scale() << endl;

	// Distributional properties
	float probability = 0.001 / 2;

	float max_noise = quantile(lp_dist, 1 - probability);

	float min_noise = quantile(lp_dist, probability);

	cout << "max_noise: " << max_noise << endl;
	cout << "min_noise: " << min_noise << endl;

	float prob100 = 0.9999999;
	float max_100 = quantile(lp_dist, prob100);
	float min_100 = quantile(lp_dist, 1 - prob100);
	cout << "range_max: " << max_100 << endl;
	cout << "range_min: " << min_100 << endl;

	// int n= -9+ int((2* 9+ 1)* 1.* rand()/ (RAND_MAX+ 1.));
	// cout << n << endl;
	return 0;
}

int main()
{
	const int SERVER_SIZE = 3;

	high_resolution_clock::time_point t1, t2;

	// srand(time(NULL));
	srand(5);
	// initialize key & table
	// gamal_key_t key;
	bsgs_table_t table;
	gamal_init(CURVE_256_SEC);
	// gamal_generate_keys(key);
	gamal_init_bsgs_table(table, (dig_t)1L << 16);

	//========================= PARTICIPANT ==========================/

	Participant part_A;

	t1 = high_resolution_clock::now();
	part_A.processData();
	t2 = high_resolution_clock::now();
	timeEvaluate("Participant import data", t1, t2);

	cout << "Total rows: " << part_A.size_dataset << endl;
	cout << "Total domains: " << part_A.hashMap.size() << endl;
	part_A.print_hash_map();

	int size_dataset = part_A.size_dataset;

	//========================= SERVER ==========================/
	//============= SET UP COLLECTIVE KEY ============/
	Servers servers(SERVER_SIZE, size_dataset);

	servers.generateCollKey();

	// ============= INITIALIZE CIPHERTEXT STACK ============/
	ENC_Stack pre_enc_stack(4 * size_dataset, servers.coll_key);

	t1 = high_resolution_clock::now();
	pre_enc_stack.initializeStack_E0();
	t2 = high_resolution_clock::now();
	timeEvaluate("Initialize E0 Stack", t1, t2);
	cout << endl;

	t1 = high_resolution_clock::now();
	pre_enc_stack.initializeStack_E1();
	t2 = high_resolution_clock::now();
	timeEvaluate("Initialize E1 Stack", t1, t2);
	cout << endl;

	servers.createServersEncrypVector(pre_enc_stack);

	//========================= PARTICIPANT ==========================/
	// part_A.addDummy(2);
	// part_A.addDummyFake_1(300000, 2);
	// part_A.addDummyFake_2(200000, 2);
	// part_A.multiply_enc_map(servers.s_plain_track_list, servers.s_myPIR_enc, pre_enc_stack);
	// part_A.testWithoutDecrypt();

	part_A.selfIntializePV(pre_enc_stack, 100, 2);

	// ========================= SERVER ==========================/
	// servers.fusionDecrypt(part_A.enc_domain_map, table);

	// ========================= SERVER 1 VERIFICATION =============/
	int server_id = 0; // Server 1
	t1 = high_resolution_clock::now();
	servers.verificationPV(part_A.enc_domain_map, table, server_id, pre_enc_stack);
	t2 = high_resolution_clock::now();
	timeEvaluate("verificationPV", t1, t2);

	// Server server1 = servers.server_vect[0];

	// // ========================= TEST FUNCTION 1 ===================/
	// t1 = high_resolution_clock::now();

	// server1.generateTestHashMap_1(pre_enc_stack, part_A.enc_domain_map);

	// t2 = high_resolution_clock::now();
	// timeEvaluate("generateTestHashMap_1", t1, t2);

	// // ========================= PARTICIPANT ==========================/
	// gamal_ciphertext_t sum_cipher;
	// gamal_cipher_new(sum_cipher);
	// part_A.proceedTestFunction(server1.enc_test_map, sum_cipher);

	// // ========================= SERVER ==========================/
	// dig_t decrypt_test_f1 = servers._fusionDecrypt(sum_cipher, table, 0);
	// cout << "Test function - Count of L known data: " << decrypt_test_f1 << endl;

	// // ========================= TEST FUNCTION 2 ===================/
	// t1 = high_resolution_clock::now();

	// server1.generateTestHashMap_2(pre_enc_stack, part_A.enc_domain_map);

	// t2 = high_resolution_clock::now();
	// timeEvaluate("generateTestHashMap_2", t1, t2);

	// // ========================= PARTICIPANT ==========================/
	// gamal_ciphertext_t sum_cipher2;
	// gamal_cipher_new(sum_cipher2);
	// part_A.proceedTestFunction(server1.enc_test_map_2, sum_cipher2);

	// // ========================= SERVER ==========================/
	// dig_t decrypt_test_f2 = servers._fusionDecrypt(sum_cipher2, table, 0);
	// cout << "Test function - Count of V known data: " << decrypt_test_f2 << endl;

	// // ========================= TEST FUNCTION 3 ===================/
	// // ========================= SERVER ==========================/
	// t1 = high_resolution_clock::now();

	// server1.generateTestHashMap_3(pre_enc_stack, part_A.enc_domain_map);

	// t2 = high_resolution_clock::now();
	// timeEvaluate("IgenerateTestHashMap_3", t1, t2);

	// // ========================= PARTICIPANT ==========================/
	// gamal_ciphertext_t sum_cipher3;
	// gamal_cipher_new(sum_cipher3);
	// part_A.proceedTestFunction(server1.enc_test_map_3, sum_cipher3);

	// // ========================= SERVER ==========================/
	// dig_t decrypt_test_f3 = servers._fusionDecrypt(sum_cipher3, table, 0);
	// cout << "Test function - Count of V - r0 data: " << decrypt_test_f3 << endl;

	// // ========================= TEST FUNCTION 4 ===================/
	// // ========================= SERVER ==========================/
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
	// timeEvaluate("IgenerateTestHashMap_Attr", t1, t2);

	// // ========================= PARTICIPANT ==========================/
	// gamal_ciphertext_t sum_cipher4;
	// gamal_cipher_new(sum_cipher4);
	// part_A.proceedTestFunction(server1.enc_test_map_4, sum_cipher4);

	// // ========================= SERVER ==========================/
	// dig_t decrypt_test_f4 = servers._fusionDecrypt(sum_cipher4, table, 0);
	// cout << "Test function - Count of specified attribute: " << decrypt_test_f4 << endl;

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
