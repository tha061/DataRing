#include "include/public_header.h"
#include "public_func.h"


void bench_elgamal(int num_entries, int tablebits)
{
	gamal_key_t key;
	gamal_ciphertext_t cipher;
	dig_t plain, after;
	bsgs_table_t table;
	srand(time(NULL));
	double avg_enc = 0, avg_dec = 0;

	gamal_init(CURVE_256_SEC);
	gamal_generate_keys(key);
	gamal_init_bsgs_table(table, (dig_t)1L << tablebits);

	//plain = ((dig_t) rand()) % (((dig_t)1L) << 32);
	plain = 10;

	for (int iter = 0; iter < num_entries; iter++)
	{
		//plain = ((dig_t) rand()) * iter % (((dig_t)1L) << 32);
		//plain = 0;

		high_resolution_clock::time_point t1 = high_resolution_clock::now();
		gamal_encrypt(cipher, key, plain);
		high_resolution_clock::time_point t2 = high_resolution_clock::now();
		auto enc_time = duration_cast<nanoseconds>(t2 - t1).count();
		std::cout << "ciphertext: " << cipher << std::endl;
		avg_enc += enc_time;
		///**
		t1 = high_resolution_clock::now();
		gamal_decrypt(&after, key, cipher, table);
		t2 = high_resolution_clock::now();
		auto dec_time = duration_cast<nanoseconds>(t2 - t1).count();
		std::cout << "Decrypt: " << after << " of plaintext " << plain << std::endl;
		avg_dec += dec_time;
		std::cout << "ENC Time: " << enc_time / 1000000.0 << "ms DEC Time: " << dec_time / 1000000.0 << " ms" << std::endl;
		//*/
	}
	std::cout << "Data points: " << num_entries << std::endl;
	std::cout << "ENC Time " << avg_enc / 1000000.0 << " ms" << std::endl; //"ms Avg DEC Time " << avg_dec / 1000000.0 << " ms" << std::endl;
	avg_enc = avg_enc / num_entries;
	//avg_dec = avg_dec / num_entries;
	std::cout << "Avg ENC Time " << avg_enc / 1000000.0 << " ms" << std::endl; //"ms Avg DEC Time " << avg_dec / 1000000.0 << " ms" << std::endl;

	gamal_cipher_clear(cipher);
	gamal_key_clear(key);
	gamal_free_bsgs_table(table);
	gamal_deinit();
}
//Test ElGamal addition performance
void bench_elgamal_add(int num_entries, int tablebits)
{
	gamal_key_t key;
	gamal_ciphertext_t cipher1, cipher2, res_add_cipher;
	dig_t plain1, plain2, plain3, res_add;
	bsgs_table_t table;
	srand(time(NULL));
	double avg_enc = 0, avg_dec = 0, avg_add = 0; //, avg_dec = 0;

	gamal_init(CURVE_256_SEC);
	gamal_generate_keys(key);
	gamal_init_bsgs_table(table, (dig_t)1L << tablebits);

	for (int iter = 0; iter < num_entries; iter++)
	{
		plain1 = ((dig_t)rand()) * iter % (((dig_t)1L) << 32);
		plain2 = ((dig_t)rand()) * iter % (((dig_t)1L) << 32);
		plain3 = 0;
		//gamal_encrypt(res_add_cipher, key, plain3);
		high_resolution_clock::time_point t1 = high_resolution_clock::now();
		gamal_encrypt(cipher1, key, plain1);
		high_resolution_clock::time_point t2 = high_resolution_clock::now();
		auto enc_time = duration_cast<nanoseconds>(t2 - t1).count();

		avg_enc += enc_time;

		gamal_encrypt(cipher2, key, plain2);

		t1 = high_resolution_clock::now();
		gamal_add(res_add_cipher, cipher1, cipher2);
		//gamal_add(cipher2, cipher1, cipher2);
		t2 = high_resolution_clock::now();
		auto add_time = duration_cast<nanoseconds>(t2 - t1).count();

		avg_add += add_time;

		t1 = high_resolution_clock::now();
		gamal_decrypt(&res_add, key, res_add_cipher, table);
		//gamal_decrypt(&res_add, key, cipher2, table);
		t2 = high_resolution_clock::now();
		auto dec_time = duration_cast<nanoseconds>(t2 - t1).count();
		// std::cout<<"ADD res: "<<res_add<< "= "<< plain1 << "+" << plain2 << std::endl;
		avg_dec += dec_time;
		// std::cout << "ENC Time: " <<  enc_time / 1000000.0 << "ms DEC Time: " << dec_time / 1000000.0 << " ms ADD Time: " << add_time / 1000000.0<< std::endl;

		if (res_add != (plain1 + plain2))
			std::cout << "ERROR" << std::endl;
		else
			std::cout << "CORRECT" << std::endl;
	}
	std::cout << "Data points: " << num_entries << std::endl;
	std::cout << "Avg ENC Time " << avg_enc / 1000000.0 << "ms Avg DEC Time " << avg_dec / 1000000.0 << " ms Avg ADD Time: " << avg_add / 1000000.0 << std::endl;
	avg_enc = avg_enc / num_entries;
	avg_dec = avg_dec / num_entries;
	avg_add = avg_add / num_entries;
	std::cout << "Avg ENC Time " << avg_enc / 1000000.0 << "ms Avg DEC Time " << avg_dec / 1000000.0 << " ms Avg ADD Time: " << avg_add / 1000000.0 << std::endl;

	gamal_cipher_clear(cipher1);
	gamal_cipher_clear(cipher2);
	gamal_cipher_clear(res_add_cipher);
	gamal_key_clear(key);
	gamal_free_bsgs_table(table);
	gamal_deinit();
}
//Test ElGamal_multiplication performance
void bench_elgamal_mult(int num_entries, int tablebits, dig_t pt)
{
	gamal_key_t key;
	gamal_ciphertext_t cipher1, res_mult_cipher; //, temp_ciph;
	dig_t plain1, plain3, res_mult;
	bsgs_table_t table;
	srand(time(NULL));
	double avg_enc = 0, avg_dec = 0, avg_mult = 0; //, avg_dec = 0;

	gamal_init(CURVE_256_SEC);
	gamal_generate_keys(key);
	gamal_init_bsgs_table(table, (dig_t)1L << tablebits);

	for (int iter = 0; iter < num_entries; iter++)
	{
		plain1 = 1; //((dig_t) rand()) * iter % (((dig_t)1L) << 32);
					// pt = 1;
		//plain3 = 0;
		//gamal_encrypt(res_mult_cipher, key, plain3);
		high_resolution_clock::time_point t1 = high_resolution_clock::now();
		gamal_encrypt(cipher1, key, plain1);
		high_resolution_clock::time_point t2 = high_resolution_clock::now();
		auto enc_time = duration_cast<nanoseconds>(t2 - t1).count();

		avg_enc += enc_time;

		t1 = high_resolution_clock::now();
		gamal_mult(res_mult_cipher, cipher1, pt);
		//gamal_mult(cipher1, cipher1, pt, temp_ciph);

		t2 = high_resolution_clock::now();
		auto mult_time = duration_cast<nanoseconds>(t2 - t1).count();
		//std::cout<<"mult cipher = "<<res_mult_cipher<<" cipher1 = " << cipher1<< " temp = "<<temp_ciph<<std::endl;

		avg_mult += mult_time;
		/**
        t1 = high_resolution_clock::now();
        //gamal_decrypt(&res_mult, key, cipher1, table);
        gamal_decrypt(&res_mult, key, res_mult_cipher, table);
        t2 = high_resolution_clock::now();
        auto dec_time = duration_cast<nanoseconds>(t2-t1).count();
        //std::cout<< "Res_mult: "<<res_mult<<"="<<plain1 <<"*" << pt << std::endl;
       avg_dec += dec_time;
       //std::cout << "ENC Time: " <<  enc_time / 1000000.0 << "ms DEC Time: " << dec_time / 1000000.0 << " ms MULT Time: " << mult_time / 1000000.0<<" ms"<< std::endl;

       if (res_mult != (plain1 * pt))
                   std::cout << "ERROR" << std::endl;
      // else
    	  // std::cout << "CORRECT" << std::endl;
**/
	}

	avg_enc = avg_enc / num_entries;
	avg_dec = avg_dec / num_entries;
	avg_mult = avg_mult / num_entries;
	std::cout << "Avg ENC Time " << avg_enc / 1000000.0 << "ms Avg DEC Time " << avg_dec / 1000000.0 << " ms Avg MULT Time: " << avg_mult / 1000000.0 << " ms" << std::endl;

	//gamal_cipher_clear(cipher1);
	//gamal_cipher_clear(cipher2);
	//gamal_cipher_clear(res_mult_cipher);
	// gamal_cipher_clear(temp_ciph);
	// gamal_key_clear(key);
	// gamal_free_bsgs_table(table);
	// gamal_deinit();
}

//Test collective key and threshold decryption

void test_coll_key_gen(int num_entries)
{
	gamal_key_t coll_keys; //collective public key
	gamal_ciphertext_t cipher;
	dig_t plain, after;
	bsgs_table_t table;
	size_t coll_key_size;
	int compressed = 0;								  //public:2,uncomp=0, compr=1;
	double avg_coll_decryp = 0, avg_coll_key_gen = 0; //time

	//gamal_key_t coll_keys;
	gamal_key_t keys1, keys2, keys3; //, key3;//, key3; // key pairs of S1, S2, S3 (server)

	gamal_init(CURVE_256_SEC);
	gamal_generate_keys(keys1); //generate key pairs for S1
	gamal_generate_keys(keys2); //generate key pairs for S1
	gamal_generate_keys(keys3); //generate key pairs for S1

	//std::cout<<"test OK"<<std::endl;
	/**
		for(int it=0; it<=num_entries; it++){
			high_resolution_clock::time_point t1 = high_resolution_clock::now();
			gamal_collective_key_gen(coll_keys, keys1, keys2, keys3);//collective key gen
			high_resolution_clock::time_point t2 = high_resolution_clock::now();
			auto coll_key_time = duration_cast<nanoseconds>(t2-t1).count();
			avg_coll_key_gen += coll_key_time;
		}
		*/
	///**
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	gamal_collective_key_gen(coll_keys, keys1, keys2, keys3); //collective key gen
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	auto coll_key_time = duration_cast<nanoseconds>(t2 - t1).count();
	avg_coll_key_gen += coll_key_time;
	//*/

	std::cout << "Avg Coll key gen time: " << avg_coll_key_gen / (num_entries * 1000000.0) << " ms" << std::endl;
	//coll_key_size = get_encoded_key_size(coll_keys, compressed);

	//std::cout<<"Coll key size, no compressed: "<<coll_key_size<<std::endl;
	//std::cout<<"test OK"<<std::endl;

	gamal_init_bsgs_table(table, (dig_t)1L << 16);

	//std::cout<<"test OK"<<std::endl;
	//#if 0
	for (int iter = 0; iter < num_entries; iter++)
	{
		plain = ((dig_t)rand()) * (iter + 1) % (((dig_t)1L) << 32); //10000; //((dig_t) rand()) * iter % (((dig_t)1L) << 32);
		//plain = 1;
		std::cout << "Plaintext: " << plain << std::endl;
		gamal_encrypt(cipher, coll_keys, plain);
		//gamal_encrypt(cipher, keys3, plain);
		//std::cout<<"test OK"<<std::endl;
		//std::cout<<"test OK"<<std::endl;

		//gamal_decrypt(&after, coll_keys, cipher, table);
		t1 = high_resolution_clock::now();
		gamal_coll_decrypt(&after, keys1, keys2, keys3, cipher, table); //threshold decryption by 3 servers
		t2 = high_resolution_clock::now();
		auto coll_decrypt_time = duration_cast<nanoseconds>(t2 - t1).count();
		avg_coll_decryp += coll_decrypt_time;
		//std::cout<<"after: "<<after<<" = plain ="<<plain<<std::endl;

		//std::cout<<"Coll decrypt time: "<<coll_decrypt_time / 1000000.0<<" ms"<<std::endl;
		//std::cout<<"test OK"<<std::endl;

		//std::cout<<"after = "<<after<<std::endl;

		if (after != plain)
			std::cout << "ERROR" << std::endl;
		//else std::cout << "OK" << std::endl;
	}
	avg_coll_decryp = avg_coll_decryp / num_entries;
	std::cout << "Evg Collective Decrypt Time: " << avg_coll_decryp / 1000000.0 << "ms" << std::endl;
	//#endif
}

void test_threshold_decrypt(int num_entries)
{
	gamal_key_t coll_keys; //collective public key
	gamal_ciphertext_t cipher;
	gamal_ciphertext_t ciphertext_update;
	dig_t plain, after;
	bsgs_table_t table;
	size_t coll_key_size;
	int num_server = 3;
	int compressed = 0;								  //public:2,uncomp=0, compr=1;
	double avg_coll_decryp = 0, avg_coll_key_gen = 0; //time

	//gamal_key_t coll_keys;
	gamal_key_t keys1, keys2, keys3; //, key3;//, key3; // key pairs of S1, S2, S3 (server)
	gamal_key_t keys23[num_server - 1];

	gamal_init(CURVE_256_SEC);
	gamal_generate_keys(keys1);		//generate key pairs for S1
	gamal_generate_keys(keys23[0]); //generate key pairs for S1
	gamal_generate_keys(keys23[1]); //generate key pairs for S1

	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	//gamal_collective_key_gen(coll_keys, keys1, keys2, keys3);//collective key genZ
	gamal_collective_key_gen(coll_keys, keys1, keys23[0], keys23[1]); //collective key gen

	
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	auto coll_key_time = duration_cast<nanoseconds>(t2 - t1).count();
	avg_coll_key_gen += coll_key_time;
	//*/

	std::cout << "Avg Coll key gen time: " << avg_coll_key_gen / (num_entries * 1000000.0) << " ms" << std::endl;

	gamal_init_bsgs_table(table, (dig_t)1L << 16);

	for (int iter = 0; iter < num_entries; iter++)
	{
		plain = ((dig_t)rand()) * (iter + 1) % (((dig_t)1L) << 32); //10000; //((dig_t) rand()) * iter % (((dig_t)1L) << 32);
		//plain = 100;
		std::cout << "Plaintext: " << plain << std::endl;
		gamal_encrypt(cipher, coll_keys, plain);

		t1 = high_resolution_clock::now();
		//gamal_coll_decrypt(&after, keys1, keys2, keys3, cipher, table);//threshold decryption by 3 servers

		gamal_fusion_decrypt(&after, num_server, keys1, keys23, ciphertext_update, cipher, table);
		
		t2 = high_resolution_clock::now();
		auto coll_decrypt_time = duration_cast<nanoseconds>(t2 - t1).count();
		avg_coll_decryp += coll_decrypt_time;

		if (after != plain)
			std::cout << "ERROR" << std::endl;
		//else std::cout << "OK" << std::endl;
	}
	avg_coll_decryp = avg_coll_decryp / num_entries;
	std::cout << "Evg Collective Decrypt Time: " << avg_coll_decryp / 1000000.0 << "ms" << std::endl;
	//#endif
}

void test_key_switch(int num_entries)
{
	gamal_key_t coll_keys;
	gamal_ciphertext_t cipher, new_cipher;
	dig_t plain, after;
	bsgs_table_t table;

	double avg_re_encrypt = 0;

	//gamal_key_t coll_keys;
	gamal_key_t keys1, keys2, keys3, keysNew; //, key3;//, key3;

	gamal_init(CURVE_256_SEC);
	gamal_generate_keys(keys1);
	gamal_generate_keys(keys2);
	gamal_generate_keys(keys3);
	gamal_generate_keys(keysNew);
	//gamal_generate_keys(keys3);

	gamal_collective_key_gen(coll_keys, keys1, keys2, keys3); //collective key gen
	gamal_init_bsgs_table(table, (dig_t)1L << 16);
	for (int iter = 0; iter <= num_entries; iter++)
	{
		plain = ((dig_t)rand()) * iter % (((dig_t)1L) << 32); //10000; //((dig_t) rand()) * iter % (((dig_t)1L) << 32);
		//plain = 100;

		gamal_encrypt(cipher, coll_keys, plain);

		//std::cout<<"test OK"<<std::endl;

		high_resolution_clock::time_point t1 = high_resolution_clock::now();
		gamal_key_switching(new_cipher, cipher, keys1, keys2, keys3, keysNew);
		//std::cout<<"test OK"<<std::endl;
		high_resolution_clock::time_point t2 = high_resolution_clock::now();
		auto re_encrypt_time = duration_cast<nanoseconds>(t2 - t1).count();
		avg_re_encrypt += re_encrypt_time;
		//std::cout<<"test OK"<<std::endl;

		//std::cout<<"cipher = "<<cipher<<" new_cipher = "<<new_cipher<<std::endl;

		gamal_decrypt(&after, keysNew, new_cipher, table);
		//std::cout<<"test OK"<<std::endl;

		std::cout<<"after: "<<after<<" = plain ="<<plain<<std::endl;

		if (after != plain)
			std::cout << "ERROR" << std::endl;
		//else std::cout << "OK" << std::endl;
	}

	std::cout << "Avg Re-encryption time: " << avg_re_encrypt / (num_entries * 1000000.0) << "ms" << std::endl;
}

void test_key_switch_new(int num_entries)
{
	gamal_key_t coll_keys;
	gamal_ciphertext_t cipher, cipher_update;
	dig_t plain, after;
	bsgs_table_t table;
	int num_server = 3;

	double avg_re_encrypt = 0;

	//gamal_key_t coll_keys;
	gamal_key_t keys1, keys2, keys3, keysNew; //, key3;//, key3;

	gamal_key_t keys23[num_server - 1];

	gamal_init(CURVE_256_SEC);
	gamal_generate_keys(keys1);		//generate key pairs for S1
	gamal_generate_keys(keys23[0]); //generate key pairs for S2
	gamal_generate_keys(keys23[1]); //generate key pairs for S3

	gamal_generate_keys(keysNew);
	//gamal_generate_keys(keys3);

	gamal_collective_key_gen(coll_keys, keys1, keys23[0], keys23[1]); //collective key gen
	gamal_init_bsgs_table(table, (dig_t)1L << 16);

	for (int iter = 1; iter <= num_entries; iter++)
	{
		std::cout<<"Iteration: "<< iter << std::endl;
		plain = ((dig_t)rand()) * iter % (((dig_t)1L) << 32); //10000; //((dig_t) rand()) * iter % (((dig_t)1L) << 32);
		//plain = 1000;

		gamal_encrypt(cipher, coll_keys, plain);

		

		high_resolution_clock::time_point t1 = high_resolution_clock::now();
		//gamal_key_switching(cipher_update, cipher, keys1, keys2, keys3, keysNew);

		gama_key_switch_lead(cipher_update, cipher, keys1, keysNew);

		for(int i = 0; i<2; i++)
		{
			gama_key_switch_follow(cipher_update, cipher, keys23[i] ,keysNew);

		}

		//std::cout<<"test OK"<<std::endl;
		high_resolution_clock::time_point t2 = high_resolution_clock::now();
		auto re_encrypt_time = duration_cast<nanoseconds>(t2 - t1).count();
		avg_re_encrypt += re_encrypt_time;
		//std::cout<<"test OK"<<std::endl;

		//std::cout<<"cipher = "<<cipher<<" new_cipher = "<<new_cipher<<std::endl;

		gamal_decrypt(&after, keysNew, cipher_update, table);
		//std::cout<<"test OK"<<std::endl;

		std::cout<<"after: "<<after<<" = plain ="<<plain<<std::endl;

		if (after != plain)
			std::cout << "ERROR" << std::endl;
		//else std::cout << "OK" << std::endl;
	}

	std::cout << "Avg Re-encryption time: " << avg_re_encrypt / (num_entries * 1000000.0) << "ms" << std::endl;
}

void test_binary_inter(dig_t number)
{

	double x = log2(number);
	int bit_num = ceil(x);
	printf("%d\n", bit_num);

	int binary_arr[bit_num];
	high_resolution_clock::time_point t1 = high_resolution_clock::now();

	convert_to_bin(binary_arr, number, bit_num);

	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	auto convert_time = duration_cast<nanoseconds>(t2 - t1).count();
	//convert_time = convert_time/1000000.0;

	for (int it = 0; it <= bit_num - 1; it++)
	{
		printf("%d", binary_arr[it]);
	}
	printf("\n");
	std::cout << "convert_time:" << convert_time / 1000000.0 << " ms" << std::endl;
}

void test_NAF_decode(dig_t number)
{
	//dig_t number = 7;
	int naf_arr[64];
	int bit_num[2];

	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	convert_to_NAF(naf_arr, number, bit_num);
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	auto convert_time = duration_cast<nanoseconds>(t2 - t1).count();

	std::cout << "convert_time:" << convert_time / 1000000.0 << " ms" << std::endl;
	//size_t size = sizeof(naf_arr)/sizeof(naf_arr[0]);
	//std::cout<<"size: "<<size<<std::endl;
	//for(int it=0; it<=size-1; it++){
	//std::cout<<naf_arr[it]<<std::endl;
	//}

	std::cout << "bit num = " << bit_num[0] << std::endl;
}

void test_mult_opt(int num_entries, dig_t big_value)
{
	//std::cout<<"BEGIN"<<std::endl;
	gamal_key_t key;
	gamal_ciphertext_t cipher1, cipher0, cipher10,res_mult_cipher1, res_mult_cipher0,res_mult_cipher_scalar, res_add0; //, temp_ciph;
	dig_t res_mult1, res_mult0, res_mult_scalar, plain, res_add0_pt, pt;
	bsgs_table_t table;
	srand(time(NULL));
	double avg_mult = 0; //, avg_dec = 0;
	//std::cout<<"Test OK"<<std::endl;

	gamal_init(DEFAULT_CURVE); //(CURVE_256_SEC);
	gamal_generate_keys(key);
	gamal_init_bsgs_table(table, (dig_t)1L << 16); //lookup table size  2^16

	//pt = ((dig_t) rand())% (((dig_t)1L) << 32);
	//pt = 1000;//exp2(33);
	// std::cout << "plaintext = " << pt << std::endl;
	//std::cout<<"Test OK"<<std::endl;

	// plain = (((dig_t) rand()) % (((dig_t)1L) << 32)-1) % (((dig_t)1L) <<32); 
	plain = 100;

	cout<<"original plaintext: "<<plain<<endl;
	// gamal_encrypt(cipher1, key, plain);
	// gamal_encrypt(cipher10, key, plain);
	gamal_encrypt(cipher0, key, plain);


	cout<<"original ciphertext: \n"<<endl;
	printCiphertext(cipher0);

	for (int iter = 1; iter <= num_entries; iter++)
	{

		// pt = (((dig_t) rand()) * iter % (((dig_t)1L) << 32)-1) % (((dig_t)1L) <<32); 
		// pt = ((((dig_t)1L) << 32) - 1) % (((dig_t)1L) <<32);

		pt = (((dig_t)1L) << 32)-1;

		cout<<"random scalar = "<<pt<<endl;


		// high_resolution_clock::time_point t1 = high_resolution_clock::now();

		// high_resolution_clock::time_point t2 = high_resolution_clock::now();

		//printf("DEBUG OK1\n");
		// pt = ((dig_t)rand()) % big_value + 1;

		// int pt1 = 1;
		// int pt0 = 0;
		// int pt10 = 10;

		// t1 = high_resolution_clock::now();
		// gamal_mult_opt(res_mult_cipher1, cipher1, pt1);
		// cout<<"print cipher 1: \n";
		// printCiphertext(res_mult_cipher1);

		// gamal_mult_opt(res_mult_cipher0, cipher0, pt0);
		// cout<<"print cipher 0: \n";
		// printCiphertext(res_mult_cipher0);



		
		gamal_mult_opt(res_mult_cipher_scalar, cipher0, pt);
		cout<<"after scalar: new cipher: \n";
		printCiphertext(res_mult_cipher_scalar);

		// gamal_add(res_add0,res_mult_cipher_scalar, res_mult_cipher0);
		// cout<<"print cipher 0 add 10: \n";
		// printCiphertext(res_add0);


		// t2 = high_resolution_clock::now();
		// auto mult_time = duration_cast<nanoseconds>(t2 - t1).count();

		//std::cout<<"multi cipher: "<<res_mult_cipher<<std::endl;
		//printf("DEBUG OK2\n");

		// avg_mult += mult_time;

		gamal_decrypt(&res_mult_scalar, key, res_mult_cipher_scalar, table);

		cout<<"Decryted: "<<endl;
		std::cout<<"before = "<<plain<<"; after mult scalar = "<<res_mult_scalar<<std::endl;
		if (res_mult_scalar != (plain * pt))
		std::cout << "ERROR" << std::endl;
		

		// gamal_decrypt(&res_mult0, key, res_mult_cipher0, table);

		// // cout<<"DEBUG OK3"<<endl;
		// std::cout<<"before = "<<plain<<"; after mult 0= "<<res_mult0<<std::endl;

		// gamal_decrypt(&res_add0_pt, key, res_add0, table);

		// // cout<<"DEBUG OK3"<<endl;
		// std::cout<<"before = "<<10<<"; after add 0 and 10= "<<res_add0<<std::endl;


		// gamal_decrypt(&res_mult_scalar, key, res_mult_cipher_scalar, table);

		// // cout<<"DEBUG OK3"<<endl;
		// std::cout<<"before = "<<plain<<"; after mult 10 = "<<res_mult_scalar<<std::endl;

		// if (res_mult_scalar != (plain * pt10))
		// std::cout << "ERROR" << std::endl;
	}

	// avg_mult = avg_mult;
	// std::cout << "MULT Time: " << avg_mult / (1000000000.0) << " sec" << std::endl;
}

void test_COUNT_query_computation(int num_entries)
{
	//multiplication HE(1)*data_point in histogram which is the count of some target attribute's value
	//add up all HE(xxx)
	//decrypt the result

	gamal_key_t key;
	gamal_ciphertext_t cipher1, res_mult_cipher, answer_cipher, temp; //, temp_ciph;
	dig_t answer, pt, plain, pt1, pt2;
	bsgs_table_t table;
	srand(time(NULL));
	double avg_mult = 0, avg_add = 0, e2e_time = 0; //, avg_dec = 0;
	//std::cout<<"Test OK"<<std::endl;

	gamal_init(CURVE_256_SEC);
	gamal_generate_keys(key);
	gamal_init_bsgs_table(table, (dig_t)1L << 16);

	//pt = ((dig_t) rand())% (((dig_t)1L) << 32);

	pt1 = 1;
	pt2 = 0;
	int bin_actual = (int)(num_entries / pt1);
	//int bin_actual = (int) floor(bin);

	//std::cout<<"pt = "<<pt1<<", bins of 100 = " << bin_actual<<std::endl;

	plain = 1;
	printf("Test ok1\n");
	gamal_encrypt(cipher1, key, plain); //PHE of 1 in the question vector
	//printf("Test ok");

	for (int iter = 0; iter < num_entries - bin_actual; iter++)
	{
		// gamal_encrypt(cipher1, key, plain);
		high_resolution_clock::time_point t1 = high_resolution_clock::now();

		gamal_mult_opt(res_mult_cipher, cipher1, pt2);

		high_resolution_clock::time_point t2 = high_resolution_clock::now();

		auto mult_time = duration_cast<nanoseconds>(t2 - t1).count();

		avg_mult += mult_time;
	}
	printf("Test ok2\n");

	//bin_actual = 5000000;
	for (int iter = 0; iter < bin_actual; iter++)
	{

		//gamal_encrypt(cipher1, key, plain);//PHE of 1 in the question vector
		high_resolution_clock::time_point t1 = high_resolution_clock::now();

		gamal_mult_opt(res_mult_cipher, cipher1, pt1);

		high_resolution_clock::time_point t2 = high_resolution_clock::now();

		auto mult_time = duration_cast<nanoseconds>(t2 - t1).count();

		avg_mult += mult_time;
	}
	printf("Test ok3\n");

	//answer_cipher = res_mult_cipher;
	temp->C1 = res_mult_cipher->C1;
	temp->C2 = res_mult_cipher->C2;
	for (int iter = 0; iter < num_entries - 1; iter++)
	{
		//temp->C1 = EC_POINT_new(init_group);
		//temp->C2 = EC_POINT_new(init_group);

		high_resolution_clock::time_point t1 = high_resolution_clock::now();

		gamal_add(answer_cipher, temp, res_mult_cipher);
		temp->C1 = answer_cipher->C1;
		temp->C2 = answer_cipher->C2;

		high_resolution_clock::time_point t2 = high_resolution_clock::now();

		auto add_time = duration_cast<nanoseconds>(t2 - t1).count();

		avg_add += add_time;
	}
	printf("Test ok4\n");
	std::cout << "answer_cipher: " << answer_cipher << std::endl;

	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	gamal_decrypt(&answer, key, answer_cipher, table);
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	auto decrypt_time = duration_cast<nanoseconds>(t2 - t1).count();

	std::cout << "query answer: " << answer << std::endl;

	e2e_time = avg_mult + avg_add;
	std::cout << "Multi time: " << avg_mult / 1000000000.0 << " sec" << std::endl;
	std::cout << "Add time: " << avg_add / 1000000000.0 << " sec" << std::endl;
	std::cout << "query_computation_time: " << e2e_time / 1000000000.0 << " sec" << std::endl;
	std::cout << "decrypt time: " << decrypt_time / 1000000000.0 << " sec" << std::endl;
}

void test_SUM_query_computation(int num_entries)
{
	//multiplication data_point in histogram and the target attribute = (res1): done in clear by the participant
	//multiplication HE(1)* res1 = HE(res1) or HE(0) * res1 = HE(0)
	//add up all HE(xxx)
	//decrypt the result

	gamal_key_t key;
	gamal_ciphertext_t cipher1, res_mult_cipher, answer_cipher, temp, res_mult_cipher_new; //, temp_ciph;
	dig_t answer, pt, plain, pt1, pt2;
	bsgs_table_t table;
	srand(time(NULL));
	double avg_mult = 0, avg_add = 0, avg_mult_clear = 0, e2e_time = 0; //, avg_dec = 0;
	//std::cout<<"Test OK"<<std::endl;

	gamal_init(CURVE_256_SEC);
	gamal_generate_keys(key);
	gamal_init_bsgs_table(table, (dig_t)1L << 16);

	//pt = ((dig_t) rand())% (((dig_t)1L) << 32);
	dig_t count_hist = 1;
	dig_t updated_hist_count;
	dig_t attr_value = 100; //specific attribute values
	pt2 = 0;

	//int bin_actual = (int)(num_entries / pt1);
	//std::cout<<"pt = "<<pt<<std::endl;
	plain = 1;

	for (int iter = 0; iter < num_entries; iter++)
	{

		high_resolution_clock::time_point t1 = high_resolution_clock::now();

		updated_hist_count = attr_value * count_hist;

		high_resolution_clock::time_point t2 = high_resolution_clock::now();

		auto mult_time_clear = duration_cast<nanoseconds>(t2 - t1).count();

		avg_mult_clear += mult_time_clear;
	}
	std::cout << "updated hist count: " << updated_hist_count << std::endl;
	printf("Test OK1\n");

	gamal_encrypt(cipher1, key, plain);

	std::cout << "res_mult_cipher before loop: " << res_mult_cipher << std::endl;

	for (int iter = 0; iter < num_entries; iter++)
	{

		high_resolution_clock::time_point t1 = high_resolution_clock::now();

		gamal_mult_opt(res_mult_cipher, cipher1, updated_hist_count);

		high_resolution_clock::time_point t2 = high_resolution_clock::now();

		auto mult_time = duration_cast<nanoseconds>(t2 - t1).count();

		avg_mult += mult_time;
		std::cout << "res_mult_cipher in loop: " << res_mult_cipher << std::endl;
	}
	std::cout << "res_mult_cipher after loop: " << res_mult_cipher << std::endl;

	gamal_mult_opt(res_mult_cipher_new, cipher1, updated_hist_count);

	std::cout << "res_mult_cipher new: " << res_mult_cipher_new << std::endl;
	printf("Test OK2\n");
	gamal_decrypt(&answer, key, res_mult_cipher_new, table);
	std::cout << "after multi: " << answer << std::endl;

	//answer_cipher = res_mult_cipher;
	temp->C1 = res_mult_cipher_new->C1;
	temp->C2 = res_mult_cipher_new->C2;
	for (int iter = 0; iter < num_entries - 1; iter++)
	{
		//temp->C1 = EC_POINT_new(init_group);
		//temp->C2 = EC_POINT_new(init_group);

		high_resolution_clock::time_point t1 = high_resolution_clock::now();

		gamal_add(answer_cipher, temp, res_mult_cipher_new);
		temp->C1 = answer_cipher->C1;
		temp->C2 = answer_cipher->C2;

		high_resolution_clock::time_point t2 = high_resolution_clock::now();

		auto add_time = duration_cast<nanoseconds>(t2 - t1).count();

		avg_add += add_time;
	}
	printf("Test OK3\n");

	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	gamal_decrypt(&answer, key, answer_cipher, table);
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	auto dec_time = duration_cast<nanoseconds>(t2 - t1).count();
	std::cout << "query answer: " << answer << std::endl;
	std::cout << "Dec time: " << dec_time / 1000000000.0 << std::endl;

	e2e_time = avg_mult + avg_add + avg_mult_clear;
	std::cout << "Multi_clear time: " << avg_mult_clear / 1000000000.0 << " sec" << std::endl;
	std::cout << "Multi time: " << avg_mult / 1000000000.0 << " sec" << std::endl;
	std::cout << "Add time: " << avg_add / 1000000000.0 << " sec" << std::endl;
	std::cout << "query_computation_time: " << e2e_time / 1000000000.0 << " sec" << std::endl;
}

void pre_encryption(int num_points)
{
	gamal_key_t key;
	gamal_ciphertext_t *table_enc0, *table_enc1;
	gamal_ciphertext_t cipher1, cipher0, cipher0_2, cipher0_3, cipher0_4, cipher0_5, cipher1_1, cipher1_2, cipher1_3, cipher1_4;
	dig_t plain1, plain0, after, pt2, pt3, pt4, pt5;
	bsgs_table_t table;
	table_enc0 = new gamal_ciphertext_t[2 * num_points]; //table of enc(0)
	table_enc1 = new gamal_ciphertext_t[2 * num_points]; //table of enc(1)
	srand(time(NULL));
	double enc = 0, dec = 0;

	gamal_init(CURVE_256_SEC);
	gamal_generate_keys(key);
	gamal_init_bsgs_table(table, (dig_t)1L << 16); //tablebits);

	plain1 = 1;
	plain0 = 0;
	pt2 = 2, pt3 = 3, pt4 = 4, pt5 = 5;
	int iter;
	for (iter = 0; iter < num_points; iter++)
	{
		high_resolution_clock::time_point t1 = high_resolution_clock::now();
		gamal_encrypt(table_enc1[iter], key, plain1);
		gamal_encrypt(table_enc0[iter], key, plain0);
		high_resolution_clock::time_point t2 = high_resolution_clock::now();
		auto enc_time = duration_cast<nanoseconds>(t2 - t1).count();

		enc += enc_time;
	}
	iter--;

	//std::cout<<"avg_enc "<<avg_enc<<std::endl;
	int it;
	for (it = iter; it < 2 * num_points; it++)
	{
		high_resolution_clock::time_point t1 = high_resolution_clock::now();
		gamal_mult_opt(table_enc0[it], table_enc0[it - iter], pt2);
		gamal_add(table_enc1[it], table_enc0[it], table_enc1[it - iter]);
		high_resolution_clock::time_point t2 = high_resolution_clock::now();
		auto enc_time = duration_cast<nanoseconds>(t2 - t1).count();
		//std::cout<<"ciphertext of 2*Enc(0): "<<cipher0_2<<std::endl;
		enc += enc_time;
	}

	extern EC_GROUP *init_group;
	BIGNUM *x = BN_new();
	BIGNUM *y = BN_new();

	//check the last element in table of enc(0)
	printf("Enc(0) #%d->C1:\n", 2 * num_points - 1);
	if (EC_POINT_get_affine_coordinates_GFp(init_group, table_enc0[2 * num_points - 1]->C1, x, y, NULL))
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
	printf("Enc(0) #%d->C2:\n", 2 * num_points - 1);
	if (EC_POINT_get_affine_coordinates_GFp(init_group, table_enc0[2 * num_points - 1]->C2, x, y, NULL))
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

	//check the last element in table of enc(1)
	printf("Enc(1) #%d->C1:\n", 2 * num_points - 1);
	if (EC_POINT_get_affine_coordinates_GFp(init_group, table_enc1[2 * num_points - 1]->C1, x, y, NULL))
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
	printf("Enc(1) #%d->C2:\n", 2 * num_points - 1);
	if (EC_POINT_get_affine_coordinates_GFp(init_group, table_enc1[2 * num_points - 1]->C2, x, y, NULL))
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

	std::cout << "Data points: " << num_points * 2 << std::endl;
	std::cout << "ENC Time " << enc / 1000000000.0 << " sec" << std::endl; //"ms Avg DEC Time " << avg_dec / 1000000.0 << " ms" << std::endl;
	gamal_key_clear(key);
	gamal_free_bsgs_table(table);
	gamal_deinit();
}


// Do not use this function!!!!
void partialViewCollect_dataset(int datasize, int attr_num, dig_t big_data_point)
{
	gamal_key_t key;
	//gamal_ciphertext_t pir[10000], data_encrypt[1000][10], answer_cipher, temp;//, temp_ciph;
	//dig_t answer, pt, plain1, plain0, pt1, pt2, data_clear[1000][10];
	//int pir_size = 2*datasize;
	gamal_ciphertext_t pir[datasize], data_encrypt[datasize][attr_num], answer_cipher, temp, pir_point; //, temp_ciph;
	dig_t answer, pt, plain1, plain0, pt1, pt2, data_clear[datasize][attr_num];
	bsgs_table_t table;
	dig_t data_point;
	//dig_t big_data_point = 10000;
	srand(time(NULL));
	double avg_mult = 0, avg_add = 0, e2e_time = 0; //, avg_dec = 0;

	plain1 = 1;
	plain0 = 0;

	gamal_init(CURVE_256_SEC);
	gamal_generate_keys(key);
	gamal_init_bsgs_table(table, (dig_t)1L << 16);

	//generate pir vector: V enc(1) and n-V enc(0)
	int n, m;
	//n= datasize, m = attr_num;
	int V = (int)n / 100;
	int it = 0;
	printf("test OK\n");

	for (it = 0; it <= datasize - 1; it++)
	{
		gamal_encrypt(pir[it], key, plain1);
	}
	printf("test OK\n");

	//gamal_encrypt(pir_point, key, plain1);
	//std::cout<<"encrypted: "<<pir[999]<<std::endl;
	//for(int it2=it; it2<=n-1; it2++){

	//gamal_encrypt(pir[it], key, plain0);
	//}

	//std::cout<<"encrypted: "<<pir[10]<<std::endl;

	//generate synthetic data: nxm points of integers from 0 to 10000

	for (int i = 0; i <= datasize - 1; i++)
	{
		for (int j = 0; j <= attr_num - 1; j++)
		{
			//printf("test OK\n");

			data_point = ((dig_t)rand()) % big_data_point + 1;
			data_clear[i][j] = data_point;
			//std::cout<<"data point: "<<data_clear[i][j]<<std::endl;
		}
	}

	for (int i = 0; i <= datasize - 1; i++)
	{
		//std::cout<<"Test ok i= "<<i<<std::endl;
		for (int j = 0; j <= attr_num - 1; j++)
		{
			//std::cout<<"Test ok j = "<<j<<std::endl;
			// gamal_encrypt(cipher1, key, plain);
			high_resolution_clock::time_point t1 = high_resolution_clock::now();

			gamal_mult_opt(data_encrypt[i][j], pir[i], data_clear[i][j]);

			high_resolution_clock::time_point t2 = high_resolution_clock::now();

			auto mult_time = duration_cast<nanoseconds>(t2 - t1).count();

			avg_mult += mult_time;
		}
		//printf("Test ok1-2\n");
	}

	//printf("Test ok2\n");
	std::cout << "time: " << avg_mult / 1000000000.0 << " sec" << std::endl;
}