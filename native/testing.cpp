/*
 * Copyright (c) 2018, Institute for Pervasive Computing, ETH Zurich.
 * All rights reserved.
 *
 * Author:
 *       Lukas Burkhalter <lubu@inf.ethz.ch>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <iostream>
#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cmath>
/**
#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/objects.h>
#include <inttypes.h>
#include "uthash.h"
*/
extern "C" {
    #include "ecelgamal.h"
    #include "crtecelgamal.h"
}

using namespace std::chrono;
//EC_GROUP *init_group = NULL;

/*============================ Plain EC-ElGamal ===================================*/
int test1() {
    gamal_key_t key, key_decoded;
    gamal_ciphertext_t cipher, cipher_after;
    bsgs_table_t table;
    dig_t plain = exp2(32), res, res2; //36435345, res, res2;
    unsigned char *buff;
    size_t size;

    gamal_init(DEFAULT_CURVE);

    gamal_generate_keys(key);
    gamal_encrypt(cipher, key, plain);

    
    std::cout << "Plain EC-ElGamal encryption, decryption" << std::endl;
    std::cout << "key gen + enc ok" << std::endl;
	
    buff = (unsigned char *) malloc(get_encoded_ciphertext_size(cipher));
    encode_ciphertext(buff, 100000, cipher);
    decode_ciphertext(cipher_after, buff, 1000000);
    free(buff);

    size = get_encoded_key_size(key, 0);
    buff = (unsigned char *) malloc(size);
    encode_key(buff, size, key, 0);
    decode_key(key_decoded, buff, size);


    gamal_init_bsgs_table(table, 1L << 16);

    gamal_decrypt(&res, key, cipher_after, table);

    std::cout << "Before:  " << plain << " After: " << res << std::endl;
    gamal_free_bsgs_table(table);
    gamal_deinit();
    return 0;
}

void bench_elgamal(int num_entries, int tablebits) {
    gamal_key_t key;
    gamal_ciphertext_t cipher;
    dig_t plain, after;
    bsgs_table_t table;
    srand (time(NULL));
    double avg_enc = 0, avg_dec = 0;

    gamal_init(CURVE_256_SEC);
    gamal_generate_keys(key);
    gamal_init_bsgs_table(table, (dig_t) 1L << tablebits);

    //plain = ((dig_t) rand()) % (((dig_t)1L) << 32);
    plain = 10;

    for (int iter=0; iter<num_entries; iter++) {
        //plain = ((dig_t) rand()) * iter % (((dig_t)1L) << 32);
    	//plain = 0;

        high_resolution_clock::time_point t1 = high_resolution_clock::now();
        gamal_encrypt(cipher, key, plain);
        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        auto enc_time = duration_cast<nanoseconds>(t2-t1).count();
        std::cout<<"ciphertext: "<<cipher<<std::endl;
        avg_enc += enc_time;
///**
        t1 = high_resolution_clock::now();
        gamal_decrypt(&after, key, cipher, table);
        t2 = high_resolution_clock::now();
        auto dec_time = duration_cast<nanoseconds>(t2-t1).count();
        std::cout<<"Decrypt: "<<after<<" of plaintext "<<plain<<std::endl;
        avg_dec += dec_time;
        std::cout << "ENC Time: " <<  enc_time / 1000000.0 << "ms DEC Time: " << dec_time / 1000000.0 << " ms" << std::endl;
//*/
    }
    std::cout<< "Data points: "<< num_entries << std::endl;
    std::cout << "ENC Time " <<  avg_enc / 1000000.0 <<  " ms" <<std::endl; //"ms Avg DEC Time " << avg_dec / 1000000.0 << " ms" << std::endl;
    avg_enc = avg_enc / num_entries;
    //avg_dec = avg_dec / num_entries;
    std::cout << "Avg ENC Time " <<  avg_enc / 1000000.0 << " ms"<<std::endl;//"ms Avg DEC Time " << avg_dec / 1000000.0 << " ms" << std::endl;

    gamal_cipher_clear(cipher);
    gamal_key_clear(key);
    gamal_free_bsgs_table(table);
    gamal_deinit();
}
//Test ElGamal addition performance
void bench_elgamal_add(int num_entries, int tablebits) {
    gamal_key_t key;
    gamal_ciphertext_t cipher1, cipher2, res_add_cipher;
    dig_t plain1, plain2, plain3, res_add;
    bsgs_table_t table;
    srand (time(NULL));
    double avg_enc = 0, avg_dec = 0, avg_add = 0;//, avg_dec = 0;

    gamal_init(CURVE_256_SEC);
    gamal_generate_keys(key);
    gamal_init_bsgs_table(table, (dig_t) 1L << tablebits);

    for (int iter=0; iter<num_entries; iter++) {
        plain1 = ((dig_t) rand()) * iter % (((dig_t)1L) << 32);
        plain2 = ((dig_t) rand()) * iter % (((dig_t)1L) << 32);
        plain3 = 0;
        //gamal_encrypt(res_add_cipher, key, plain3);
        high_resolution_clock::time_point t1 = high_resolution_clock::now();
        gamal_encrypt(cipher1, key, plain1);
        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        auto enc_time = duration_cast<nanoseconds>(t2-t1).count();

        avg_enc += enc_time;

        gamal_encrypt(cipher2, key, plain2);


        t1 = high_resolution_clock::now();
		gamal_add(res_add_cipher, cipher1, cipher2);
		//gamal_add(cipher2, cipher1, cipher2);
		t2 = high_resolution_clock::now();
		auto add_time = duration_cast<nanoseconds>(t2-t1).count();

        avg_add += add_time;

        t1 = high_resolution_clock::now();
        gamal_decrypt(&res_add, key, res_add_cipher, table);
        //gamal_decrypt(&res_add, key, cipher2, table);
        t2 = high_resolution_clock::now();
        auto dec_time = duration_cast<nanoseconds>(t2-t1).count();
      // std::cout<<"ADD res: "<<res_add<< "= "<< plain1 << "+" << plain2 << std::endl;
        avg_dec += dec_time;
      // std::cout << "ENC Time: " <<  enc_time / 1000000.0 << "ms DEC Time: " << dec_time / 1000000.0 << " ms ADD Time: " << add_time / 1000000.0<< std::endl;

        if (res_add != (plain1 + plain2))
                   std::cout << "ERROR" << std::endl;
        else std::cout << "CORRECT" <<std::endl;
    }
    std::cout<<"Data points: "<<num_entries<<std::endl;
    std::cout << "Avg ENC Time " <<  avg_enc / 1000000.0 << "ms Avg DEC Time " << avg_dec / 1000000.0 << " ms Avg ADD Time: " << avg_add / 1000000.0<<std::endl;
    avg_enc = avg_enc / num_entries;
    avg_dec = avg_dec / num_entries;
    avg_add = avg_add / num_entries;
    std::cout << "Avg ENC Time " <<  avg_enc / 1000000.0 << "ms Avg DEC Time " << avg_dec / 1000000.0 << " ms Avg ADD Time: " << avg_add / 1000000.0<<std::endl;

    gamal_cipher_clear(cipher1);
    gamal_cipher_clear(cipher2);
    gamal_cipher_clear(res_add_cipher);
    gamal_key_clear(key);
    gamal_free_bsgs_table(table);
    gamal_deinit();
}
//Test ElGamal_multiplication performance
void bench_elgamal_mult(int num_entries, int tablebits, dig_t pt) {
    gamal_key_t key;
    gamal_ciphertext_t cipher1, res_mult_cipher;//, temp_ciph;
    dig_t plain1, plain3, res_mult;
    bsgs_table_t table;
    srand (time(NULL));
    double avg_enc = 0, avg_dec = 0, avg_mult = 0;//, avg_dec = 0;

    gamal_init(CURVE_256_SEC);
    gamal_generate_keys(key);
    gamal_init_bsgs_table(table, (dig_t) 1L << tablebits);


    for (int iter=0; iter<num_entries; iter++) {
        plain1 = 1; //((dig_t) rand()) * iter % (((dig_t)1L) << 32);
       // pt = 1;
        //plain3 = 0;
        //gamal_encrypt(res_mult_cipher, key, plain3);
        high_resolution_clock::time_point t1 = high_resolution_clock::now();
        gamal_encrypt(cipher1, key, plain1);
        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        auto enc_time = duration_cast<nanoseconds>(t2-t1).count();

        avg_enc += enc_time;


        t1 = high_resolution_clock::now();
		gamal_mult(res_mult_cipher, cipher1, pt);
        //gamal_mult(cipher1, cipher1, pt, temp_ciph);

		t2 = high_resolution_clock::now();
		auto mult_time = duration_cast<nanoseconds>(t2-t1).count();
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
    std::cout << "Avg ENC Time " <<  avg_enc / 1000000.0 << "ms Avg DEC Time " << avg_dec / 1000000.0 << " ms Avg MULT Time: " << avg_mult / 1000000.0<<" ms"<<std::endl;

    //gamal_cipher_clear(cipher1);
    //gamal_cipher_clear(cipher2);
    //gamal_cipher_clear(res_mult_cipher);
   // gamal_cipher_clear(temp_ciph);
   // gamal_key_clear(key);
   // gamal_free_bsgs_table(table);
   // gamal_deinit();
}


//Test collective key

void test_coll_key_gen (int num_entries) {
		gamal_key_t coll_keys;
		gamal_ciphertext_t cipher;
		dig_t plain, after;
		bsgs_table_t table;
		size_t coll_key_size;
		int compressed = 0;//public:2,uncomp=0, compr=1;
		double avg_coll_decryp = 0, avg_coll_key_gen = 0;


		//gamal_key_t coll_keys;
		gamal_key_t keys1, keys2, keys3;//, key3;//, key3;

		gamal_init(CURVE_256_SEC);
		gamal_generate_keys(keys1);
		gamal_generate_keys(keys2);
		gamal_generate_keys(keys3);


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
		gamal_collective_key_gen(coll_keys, keys1, keys2, keys3);//collective key gen
		high_resolution_clock::time_point t2 = high_resolution_clock::now();
		auto coll_key_time = duration_cast<nanoseconds>(t2-t1).count();
		avg_coll_key_gen += coll_key_time;
		//*/

		//std::cout<<"Avg Coll key gen time: "<< avg_coll_key_gen / (num_entries*1000000.0)<<" ms"<<std::endl;
		//coll_key_size = get_encoded_key_size(coll_keys, compressed);

		//std::cout<<"Coll key size, no compressed: "<<coll_key_size<<std::endl;
		//std::cout<<"test OK"<<std::endl;

		gamal_init_bsgs_table(table, (dig_t) 1L << 16);

		//std::cout<<"test OK"<<std::endl;
//#if 0
		for (int iter=0; iter<num_entries; iter++) {
			plain = ((dig_t) rand()) * iter % (((dig_t)1L) << 32);//10000; //((dig_t) rand()) * iter % (((dig_t)1L) << 32);
			//plain = 1;
			//std::cout<<"test OK"<<std::endl;
			gamal_encrypt(cipher, coll_keys, plain);
			//gamal_encrypt(cipher, keys3, plain);
			//std::cout<<"test OK"<<std::endl;
			//std::cout<<"test OK"<<std::endl;

			//gamal_decrypt(&after, coll_keys, cipher, table);
			t1 = high_resolution_clock::now();
			gamal_coll_decrypt(&after, keys1, keys2, keys3, cipher, table);
			t2 = high_resolution_clock::now();
			auto coll_decrypt_time = duration_cast<nanoseconds>(t2-t1).count();
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
	    std::cout << "Evg Collective Decrypt Time: " <<  avg_coll_decryp / 1000000.0 << "ms"<<std::endl;
//#endif

}

/**
void test_coll_decrypt(){
	        std::cout<<"test begin"<<std::endl;
			gamal_key_t keys1, keys2, coll_keys;
			gamal_ciphertext_t cipher;
			dig_t plain, after;
			bsgs_table_t table;

			std::cout<<"test OK"<<std::endl;

			gamal_init(CURVE_256_SEC);
			gamal_generate_keys(keys1);
			gamal_generate_keys(keys2);
			//gamal_generate_keys(key3);
			std::cout<<"test OK"<<std::endl;

			//coll keys gen
			BN_CTX *ctx = BN_CTX_new();
			BIGNUM *ord, *coll_key;
			EC_GROUP *init_group;

			std::cout<<"test OK"<<std::endl;

			ord = BN_new();
			std::cout<<"test OK"<<std::endl;
			coll_key = BN_new(); //coll secret key
			std::cout<<"test OK"<<std::endl;
			coll_keys->Y = EC_POINT_new(init_group);
			std::cout<<"test OK"<<std::endl;

			EC_GROUP_get_order(init_group, ord, ctx);
			std::cout<<"test OK"<<std::endl;

			coll_keys->is_public = 0;
			std::cout<<"test OK"<<std::endl;
			//EC_POINT_add(const EC_GROUP *group, EC_POINT *r, const EC_POINT *a, const EC_POINT *b, BN_CTX *ctx); //in ec.h

			EC_POINT_add(init_group, coll_keys->Y, keys2->Y, keys1->Y, ctx);
			std::cout<<"test OK"<<std::endl;
			//EC_POINT_add(init_group, coll_keys->Y, coll_keys->Y, key3->Y, ctx);


			//int BN_add(BIGNUM *r, const BIGNUM *a, const BIGNUM *b); //in bn.h

			BN_add(coll_key, keys1->secret, keys2->secret);
			coll_keys->secret = coll_key;
			std::cout<<"test OK"<<std::endl;
			//BN_add(coll_key, coll_keys->secret, key3->secret);
			//coll_keys->secret = coll_key;

			BN_free(ord);
			BN_CTX_free(ctx);


			//encrypt with coll_keys
			gamal_init_bsgs_table(table, (dig_t) 1L << 16);
			plain = 10000; //((dig_t) rand()) * iter % (((dig_t)1L) << 32);


			gamal_encrypt(cipher, coll_keys, plain);
			std::cout<<"test OK"<<std::endl;

			//coll decrypt
			gamal_coll_decrypt(&after, keys1, keys2, cipher, table);
			std::cout<<"test OK"<<std::endl;

			std::cout<<"after = "<<after<<std::endl;

			if (after != plain)
				   std::cout << "ERROR" << std::endl;
			else std::cout << "OK" << std::endl;


}
*/
//int gamal_coll_decrypt(dig_t *res, gamal_key_t keys1, gamal_key_t keys2, gamal_ciphertext_t ciphertext, bsgs_table_t table)

void test_key_switch(int num_entries){
	gamal_key_t coll_keys;
	gamal_ciphertext_t cipher, new_cipher;
	dig_t plain, after;
	bsgs_table_t table;

	double avg_re_encrypt = 0;


	//gamal_key_t coll_keys;
	gamal_key_t keys1, keys2, keys3, keysNew;//, key3;//, key3;

	gamal_init(CURVE_256_SEC);
	gamal_generate_keys(keys1);
	gamal_generate_keys(keys2);
	gamal_generate_keys(keys3);
	gamal_generate_keys(keysNew);
	//gamal_generate_keys(keys3);

	gamal_collective_key_gen(coll_keys, keys1, keys2, keys3);//collective key gen
	gamal_init_bsgs_table(table, (dig_t) 1L << 16);
	for(int iter = 0; iter<=num_entries; iter++){
		plain = ((dig_t) rand()) * iter % (((dig_t)1L) << 32);//10000; //((dig_t) rand()) * iter % (((dig_t)1L) << 32);
		//plain = 100;

		gamal_encrypt(cipher, coll_keys, plain);

		//std::cout<<"test OK"<<std::endl;

		high_resolution_clock::time_point t1 = high_resolution_clock::now();
		gamal_key_switching(new_cipher, cipher, keys1, keys2, keys3, keysNew);
		//std::cout<<"test OK"<<std::endl;
		high_resolution_clock::time_point t2 = high_resolution_clock::now();
		auto re_encrypt_time = duration_cast<nanoseconds>(t2-t1).count();
		avg_re_encrypt += re_encrypt_time;
		//std::cout<<"test OK"<<std::endl;

		//std::cout<<"cipher = "<<cipher<<" new_cipher = "<<new_cipher<<std::endl;

		gamal_decrypt(&after, keysNew, new_cipher, table);
		//std::cout<<"test OK"<<std::endl;


		//std::cout<<"after: "<<after<<" = plain ="<<plain<<std::endl;


		if (after != plain)
			   std::cout << "ERROR" << std::endl;
		//else std::cout << "OK" << std::endl;

	}


	std::cout<<"Avg Re-encryption time: "<<avg_re_encrypt/(num_entries*1000000.0)<<"ms"<<std::endl;


}

void test_binary_inter(dig_t number){


	double x = log2(number);
	int bit_num = ceil(x);
	printf("%d\n", bit_num);

	int binary_arr[bit_num];
	high_resolution_clock::time_point t1 = high_resolution_clock::now();

		convert_to_bin(binary_arr, number, bit_num);

	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	auto convert_time = duration_cast<nanoseconds>(t2-t1).count();
	//convert_time = convert_time/1000000.0;


	for(int it=0; it<=bit_num -1; it++){
	  printf("%d", binary_arr[it]);
	   }
	printf("\n");
	std::cout<<"convert_time:"<<convert_time/1000000.0<<" ms"<<std::endl;

}

void test_NAF_decode(dig_t number){
	//dig_t number = 7;
	int naf_arr[64]; int bit_num[2];

	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	convert_to_NAF(naf_arr, number, bit_num);
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	auto convert_time = duration_cast<nanoseconds>(t2-t1).count();

	std::cout<<"convert_time:"<<convert_time/1000000.0<<" ms"<<std::endl;
	//size_t size = sizeof(naf_arr)/sizeof(naf_arr[0]);
	//std::cout<<"size: "<<size<<std::endl;
	//for(int it=0; it<=size-1; it++){
		//std::cout<<naf_arr[it]<<std::endl;
	//}

	std::cout<<"bit num = "<<bit_num[0]<<std::endl;

}

void test_mult_opt(int num_entries, dig_t big_value){
	//std::cout<<"BEGIN"<<std::endl;
	gamal_key_t key;
	gamal_ciphertext_t cipher1, res_mult_cipher;//, temp_ciph;
	dig_t res_mult, pt, plain;
	bsgs_table_t table;
	srand (time(NULL));
	double avg_mult = 0;//, avg_dec = 0;
	//std::cout<<"Test OK"<<std::endl;

	gamal_init(DEFAULT_CURVE);//(CURVE_256_SEC);
	gamal_generate_keys(key);
	gamal_init_bsgs_table(table, (dig_t) 1L << 16);

	 //pt = ((dig_t) rand())% (((dig_t)1L) << 32);
	 //pt = 1000;//exp2(33);
	 std::cout<<"plaintext = "<<pt<<std::endl;
	//std::cout<<"Test OK"<<std::endl;
	 plain = 1;
	 gamal_encrypt(cipher1, key, plain);

	for (int iter=0; iter<num_entries; iter++) {


		//pt = ((dig_t) rand()) * iter % (((dig_t)1L) << 32); //what happens here, why core dumped
		high_resolution_clock::time_point t1 = high_resolution_clock::now();



		high_resolution_clock::time_point t2 = high_resolution_clock::now();

		//printf("DEBUG OK1\n");
		pt = ((dig_t) rand())% big_value +1;

		t1 = high_resolution_clock::now();
		gamal_mult_opt(res_mult_cipher, cipher1, pt);
		t2 = high_resolution_clock::now();
		auto mult_time = duration_cast<nanoseconds>(t2-t1).count();

		//std::cout<<"multi cipher: "<<res_mult_cipher<<std::endl;
		//printf("DEBUG OK2\n");

		avg_mult += mult_time;

		//gamal_decrypt(&res_mult, key, res_mult_cipher, table);

		//printf("DEBUG OK3\n");
		//std::cout<<"after= "<<res_mult<<std::endl;



		  // if (res_mult != (plain * pt))
			//   std::cout << "ERROR" << std::endl;
	}

	avg_mult = avg_mult;
	std::cout << "MULT Time: " << avg_mult / (1000000000.0 *60) <<" min"<<std::endl;

}

void test_COUNT_query_computation(int num_entries){
	//multiplication HE(1)*data_point in histogram which is the count of some target attribute's value
	//add up all HE(xxx)
	//decrypt the result

		gamal_key_t key;
		gamal_ciphertext_t cipher1, res_mult_cipher, answer_cipher, temp;//, temp_ciph;
		dig_t answer, pt, plain, pt1, pt2;
		bsgs_table_t table;
		srand (time(NULL));
		double avg_mult = 0, avg_add = 0, e2e_time = 0;//, avg_dec = 0;
		//std::cout<<"Test OK"<<std::endl;

		gamal_init(CURVE_256_SEC);
		gamal_generate_keys(key);
		gamal_init_bsgs_table(table, (dig_t) 1L << 16);

		//pt = ((dig_t) rand())% (((dig_t)1L) << 32);

		pt1 = 1; pt2 = 0;
		int bin_actual = (int)(num_entries / pt1);
		//int bin_actual = (int) floor(bin);

		//std::cout<<"pt = "<<pt1<<", bins of 100 = " << bin_actual<<std::endl;

		plain = 1;
		printf("Test ok1\n");
		gamal_encrypt(cipher1, key, plain);//PHE of 1 in the question vector
		//printf("Test ok");


		for (int iter=0; iter < num_entries - bin_actual; iter++) {
			// gamal_encrypt(cipher1, key, plain);
			high_resolution_clock::time_point t1 = high_resolution_clock::now();

			gamal_mult_opt(res_mult_cipher, cipher1, pt2);

			high_resolution_clock::time_point t2 = high_resolution_clock::now();

			auto mult_time = duration_cast<nanoseconds>(t2-t1).count();

		 	avg_mult += mult_time;
		 }
		 printf("Test ok2\n");

		//bin_actual = 5000000;
		 for (int iter=0; iter < bin_actual; iter++) {

			 //gamal_encrypt(cipher1, key, plain);//PHE of 1 in the question vector
			 high_resolution_clock::time_point t1 = high_resolution_clock::now();

				gamal_mult_opt(res_mult_cipher, cipher1, pt1);

				high_resolution_clock::time_point t2 = high_resolution_clock::now();

				auto mult_time = duration_cast<nanoseconds>(t2-t1).count();

		 		 	avg_mult += mult_time;
		 }
		 printf("Test ok3\n");

		 //answer_cipher = res_mult_cipher;
		 temp->C1 = res_mult_cipher->C1;
		 temp->C2 = res_mult_cipher->C2;
		 for (int iter=0; iter < num_entries - 1; iter++) {
			 //temp->C1 = EC_POINT_new(init_group);
			 //temp->C2 = EC_POINT_new(init_group);

			high_resolution_clock::time_point t1 = high_resolution_clock::now();

			gamal_add(answer_cipher, temp, res_mult_cipher);
			temp->C1 = answer_cipher->C1;
			temp-> C2 = answer_cipher->C2;

			high_resolution_clock::time_point t2 = high_resolution_clock::now();

			auto add_time = duration_cast<nanoseconds>(t2-t1).count();

			avg_add += add_time;
		 }
		 printf("Test ok4\n");
		 std::cout<<"answer_cipher: "<<answer_cipher<<std::endl;

		 high_resolution_clock::time_point t1 = high_resolution_clock::now();
		 gamal_decrypt(&answer, key, answer_cipher, table);
		 high_resolution_clock::time_point t2 = high_resolution_clock::now();
		 auto decrypt_time = duration_cast<nanoseconds>(t2-t1).count();

		std::cout<<"query answer: "<<answer<<std::endl;

		 e2e_time = avg_mult + avg_add;
		 std::cout<<"Multi time: "<<avg_mult/1000000000.0<<" sec"<<std::endl;
		 std::cout<<"Add time: "<<avg_add/1000000000.0<<" sec"<<std::endl;
		 std::cout<<"query_computation_time: "<<e2e_time /1000000000.0<<" sec"<<std::endl;
		 std::cout<<"decrypt time: "<< decrypt_time/1000000000.0<<" sec"<<std::endl;

}

void test_SUM_query_computation(int num_entries){
	//multiplication data_point in histogram and the target attribute = (res1): done in clear by the participant
	//multiplication HE(1)* res1 = HE(res1) or HE(0) * res1 = HE(0)
	//add up all HE(xxx)
	//decrypt the result

		gamal_key_t key;
		gamal_ciphertext_t cipher1, res_mult_cipher, answer_cipher, temp, res_mult_cipher_new;//, temp_ciph;
		dig_t answer, pt, plain, pt1, pt2;
		bsgs_table_t table;
		srand (time(NULL));
		double avg_mult = 0, avg_add = 0, avg_mult_clear = 0, e2e_time = 0;//, avg_dec = 0;
		//std::cout<<"Test OK"<<std::endl;

		gamal_init(CURVE_256_SEC);
		gamal_generate_keys(key);
		gamal_init_bsgs_table(table, (dig_t) 1L << 16);


		//pt = ((dig_t) rand())% (((dig_t)1L) << 32);
		dig_t count_hist = 1;
		dig_t updated_hist_count;
		dig_t attr_value = 100; //specific attribute values
		pt2 = 0;

		//int bin_actual = (int)(num_entries / pt1);
		//std::cout<<"pt = "<<pt<<std::endl;
		plain = 1;

		for (int iter=0; iter < num_entries; iter++) {

			high_resolution_clock::time_point t1 = high_resolution_clock::now();

			updated_hist_count = attr_value*count_hist;

			high_resolution_clock::time_point t2 = high_resolution_clock::now();

			auto mult_time_clear = duration_cast<nanoseconds>(t2-t1).count();

			avg_mult_clear += mult_time_clear;
		}
		std::cout<<"updated hist count: "<<updated_hist_count<<std::endl;
		printf("Test OK1\n");

		gamal_encrypt(cipher1, key, plain);

		std::cout<<"res_mult_cipher before loop: "<<res_mult_cipher<<std::endl;

		for (int iter=0; iter < num_entries; iter++) {

			high_resolution_clock::time_point t1 = high_resolution_clock::now();

			gamal_mult_opt(res_mult_cipher, cipher1, updated_hist_count);

			high_resolution_clock::time_point t2 = high_resolution_clock::now();

			auto mult_time = duration_cast<nanoseconds>(t2-t1).count();

		 	avg_mult += mult_time;
		 	std::cout<<"res_mult_cipher in loop: "<<res_mult_cipher<<std::endl;

		 }
		std::cout<<"res_mult_cipher after loop: "<<res_mult_cipher<<std::endl;

		gamal_mult_opt(res_mult_cipher_new, cipher1, updated_hist_count);

		std::cout<<"res_mult_cipher new: "<<res_mult_cipher_new<<std::endl;
		printf("Test OK2\n");
		gamal_decrypt(&answer, key, res_mult_cipher_new, table);
		std::cout<<"after multi: "<<answer<<std::endl;

		 //answer_cipher = res_mult_cipher;
		 temp->C1 = res_mult_cipher_new->C1;
		 temp->C2 = res_mult_cipher_new->C2;
		 for (int iter=0; iter<num_entries - 1; iter++) {
			 //temp->C1 = EC_POINT_new(init_group);
			 //temp->C2 = EC_POINT_new(init_group);

			high_resolution_clock::time_point t1 = high_resolution_clock::now();

			gamal_add(answer_cipher, temp, res_mult_cipher_new);
			temp->C1 = answer_cipher->C1;
			temp-> C2 = answer_cipher->C2;

			high_resolution_clock::time_point t2 = high_resolution_clock::now();

			auto add_time = duration_cast<nanoseconds>(t2-t1).count();

			avg_add += add_time;
		 }
		 printf("Test OK3\n");

		 high_resolution_clock::time_point t1 = high_resolution_clock::now();
		 gamal_decrypt(&answer, key, answer_cipher, table);
		 high_resolution_clock::time_point t2 = high_resolution_clock::now();
		 auto dec_time = duration_cast<nanoseconds>(t2-t1).count();
		 std::cout<<"query answer: "<<answer<<std::endl;
		 std::cout<<"Dec time: "<<dec_time/1000000000.0<<std::endl;

		 e2e_time = avg_mult + avg_add + avg_mult_clear;
		 std::cout<<"Multi_clear time: "<<avg_mult_clear/1000000000.0<<" sec"<<std::endl;
		 std::cout<<"Multi time: "<<avg_mult/1000000000.0<<" sec"<<std::endl;
		 std::cout<<"Add time: "<<avg_add/1000000000.0<<" sec"<<std::endl;
		 std::cout<<"query_computation_time: "<<e2e_time /1000000000.0<<" sec"<<std::endl;
}

void pre_encryption(int num_points){
	 	gamal_key_t key;
	    gamal_ciphertext_t cipher1, cipher0, cipher0_2, cipher0_3, cipher0_4, cipher0_5, cipher1_1, cipher1_2, cipher1_3, cipher1_4;
	    dig_t plain1, plain0, after, pt2, pt3, pt4, pt5;
	    bsgs_table_t table;
	    srand (time(NULL));
	    double avg_enc = 0, avg_dec = 0;

	    gamal_init(CURVE_256_SEC);
	    gamal_generate_keys(key);
	    gamal_init_bsgs_table(table, (dig_t) 1L << 16);//tablebits);

	    //plain = ((dig_t) rand()) % (((dig_t)1L) << 32);
	    plain1 = 1; plain0 = 0; pt2 = 2, pt3 = 3, pt4 = 4, pt5 = 5;

	    for (int iter=0; iter<num_points; iter++) {
	    	//srand (time(NULL));
	        high_resolution_clock::time_point t1 = high_resolution_clock::now();
	        gamal_encrypt(cipher1, key, plain1);
	        gamal_encrypt(cipher0, key, plain0);
	        gamal_mult_opt(cipher0_2, cipher0, pt2);
	        gamal_mult_opt(cipher0_3, cipher0, pt3);
	        gamal_mult_opt(cipher0_4, cipher0, pt4);
	        gamal_mult_opt(cipher0_5, cipher0, pt5);
	        gamal_add(cipher1_1, cipher0, cipher1);
	        gamal_add(cipher1_2, cipher0, cipher1_1);
	        gamal_add(cipher1_3, cipher0, cipher1_2);
	        gamal_add(cipher1_4, cipher0, cipher1_3);
	        high_resolution_clock::time_point t2 = high_resolution_clock::now();
	        auto enc_time = duration_cast<nanoseconds>(t2-t1).count();
	        //std::cout<<"ciphertext of 1: "<<cipher1->C1<<std::endl;
	        //std::cout<<"ciphertext of 0: "<<cipher0->C1<<std::endl;
	        //std::cout<<"ciphertext of 2*Enc(0): "<<&cipher0_2<<std::endl;
	        //std::cout<<"ciphertext of 3*Enc(0): "<<&cipher0_3<<std::endl;
	       // std::cout<<"ciphertext of 4*Enc(0): "<<&cipher0_4<<std::endl;
	        avg_enc += enc_time;
	/**
	        t1 = high_resolution_clock::now();
	        gamal_decrypt(&after, key, cipher, table);
	        t2 = high_resolution_clock::now();
	        auto dec_time = duration_cast<nanoseconds>(t2-t1).count();
	        std::cout<<"Decrypt: "<<after<<" of plaintext "<<plain<<std::endl;
	        avg_dec += dec_time;
	        std::cout << "ENC Time: " <<  enc_time / 1000000.0 << "ms DEC Time: " << dec_time / 1000000.0 << " ms" << std::endl;
	*/
/**
	        extern EC_GROUP *init_group;
	       		BIGNUM *x = BN_new();
	       			BIGNUM *y = BN_new();
	       		std::cout << "cipher1->C1=" << std::endl;
	       		if(EC_POINT_get_affine_coordinates_GFp(init_group, cipher1->C1, x, y, NULL)) {
	       			BN_print_fp(stdout, x);
	       			putc('\n', stdout);
	       			BN_print_fp(stdout, y);
	       			putc('\n', stdout);
	       		} else {
	    			std::cerr << "Can't get point coordinates." << std::endl;
	    		}

	       		std::cout << "cipher0->C1=" << std::endl;
	       		if(EC_POINT_get_affine_coordinates_GFp(init_group, cipher0->C1, x, y, NULL)) {
	       			BN_print_fp(stdout, x);
	       			putc('\n', stdout);
	       			BN_print_fp(stdout, y);
	       			putc('\n', stdout);
	       		} else {
	    			std::cerr << "Can't get point coordinates." << std::endl;
	    		}

	       		std::cout << "cipher0_2->C1=" << std::endl;
					if(EC_POINT_get_affine_coordinates_GFp(init_group, cipher0_2->C1, x, y, NULL)) {
						BN_print_fp(stdout, x);
						putc('\n', stdout);
						BN_print_fp(stdout, y);
						putc('\n', stdout);
					} else {
		    			std::cerr << "Can't get point coordinates." << std::endl;
		    		}

					std::cout << "cipher0_3->C1=" << std::endl;
						if(EC_POINT_get_affine_coordinates_GFp(init_group, cipher0_3->C1, x, y, NULL)) {
							BN_print_fp(stdout, x);
							putc('\n', stdout);
							BN_print_fp(stdout, y);
							putc('\n', stdout);
						} else {
			    			std::cerr << "Can't get point coordinates." << std::endl;
			    		}

						std::cout << "cipher0_4->C1=" << std::endl;
							if(EC_POINT_get_affine_coordinates_GFp(init_group, cipher0_4->C1, x, y, NULL)) {
								BN_print_fp(stdout, x);
								putc('\n', stdout);
								BN_print_fp(stdout, y);
								putc('\n', stdout);
							} else {
				    			std::cerr << "Can't get point coordinates." << std::endl;
				    		}
						std::cout << "cipher0_5->C1=" << std::endl;
								if(EC_POINT_get_affine_coordinates_GFp(init_group, cipher0_5->C1, x, y, NULL)) {
									BN_print_fp(stdout, x);
									putc('\n', stdout);
									BN_print_fp(stdout, y);
									putc('\n', stdout);
								} else {
									std::cerr << "Can't get point coordinates." << std::endl;
								}
						std::cout << "cipher1_1->C1=" << std::endl;
							if(EC_POINT_get_affine_coordinates_GFp(init_group, cipher1_1->C1, x, y, NULL)) {
								BN_print_fp(stdout, x);
								putc('\n', stdout);
								BN_print_fp(stdout, y);
								putc('\n', stdout);
							} else {
								std::cerr << "Can't get point coordinates." << std::endl;
							}
						std::cout << "cipher1_2->C1=" << std::endl;
							if(EC_POINT_get_affine_coordinates_GFp(init_group, cipher1_2->C1, x, y, NULL)) {
								BN_print_fp(stdout, x);
								putc('\n', stdout);
								BN_print_fp(stdout, y);
								putc('\n', stdout);
							} else {
								std::cerr << "Can't get point coordinates." << std::endl;
							}
					std::cout << "cipher1_3->C1=" << std::endl;
							if(EC_POINT_get_affine_coordinates_GFp(init_group, cipher1_3->C1, x, y, NULL)) {
								BN_print_fp(stdout, x);
								putc('\n', stdout);
								BN_print_fp(stdout, y);
								putc('\n', stdout);
							} else {
								std::cerr << "Can't get point coordinates." << std::endl;
							}
				std::cout << "cipher1_4->C1=" << std::endl;
							if(EC_POINT_get_affine_coordinates_GFp(init_group, cipher1_4->C1, x, y, NULL)) {
								BN_print_fp(stdout, x);
								putc('\n', stdout);
								BN_print_fp(stdout, y);
								putc('\n', stdout);
							} else {
								std::cerr << "Can't get point coordinates." << std::endl;
							}
*/

	    }




	    //std::cout<<"avg_enc "<<avg_enc<<std::endl;
/**
	    for (int iter=0; iter<num_points; iter++) {
	    	high_resolution_clock::time_point t1 = high_resolution_clock::now();
			gamal_mult_opt(cipher0_2, cipher0, pt2);
			//gamal_mult_opt(cipher0_2, cipher0, pt2);
			//gamal_mult_opt(cipher0_2, cipher0, pt2);
			high_resolution_clock::time_point t2 = high_resolution_clock::now();
			auto enc_time = duration_cast<nanoseconds>(t2-t1).count();
			//std::cout<<"ciphertext of 2*Enc(0): "<<cipher0_2<<std::endl;
			avg_enc += enc_time;
			extern EC_GROUP *init_group;
				       		BIGNUM *x = BN_new();
				       			BIGNUM *y = BN_new();
			std::cout << "cipher0_2->C1=" << std::endl;
								if(EC_POINT_get_affine_coordinates_GFp(init_group, cipher0_2->C1, x, y, NULL)) {
									BN_print_fp(stdout, x);
									putc('\n', stdout);
									BN_print_fp(stdout, y);
									putc('\n', stdout);
								} else {
									std::cerr << "Can't get point coordinates." << std::endl;
								}
	    }


	    for (int iter=0; iter<num_points; iter++) {
			high_resolution_clock::time_point t1 = high_resolution_clock::now();
			gamal_mult_opt(cipher0_3, cipher0, pt3);
			//gamal_mult_opt(cipher0_3, cipher0, pt3);
			//gamal_mult_opt(cipher0_3, cipher0, pt3);
			high_resolution_clock::time_point t2 = high_resolution_clock::now();
			auto enc_time = duration_cast<nanoseconds>(t2-t1).count();
			//std::cout<<"ciphertext of 3*Enc(0): "<<cipher0_3<<std::endl;
			avg_enc += enc_time;
			extern EC_GROUP *init_group;
				       		BIGNUM *x = BN_new();
				       			BIGNUM *y = BN_new();
			std::cout << "cipher0_3->C1=" << std::endl;
								if(EC_POINT_get_affine_coordinates_GFp(init_group, cipher0_3->C1, x, y, NULL)) {
									BN_print_fp(stdout, x);
									putc('\n', stdout);
									BN_print_fp(stdout, y);
									putc('\n', stdout);
								} else {
									std::cerr << "Can't get point coordinates." << std::endl;
								}
	    }
	    for (int iter=0; iter<num_points; iter++) {
			high_resolution_clock::time_point t1 = high_resolution_clock::now();
			gamal_mult_opt(cipher0_4, cipher0, pt4);
			//gamal_mult_opt(cipher0_4, cipher0, pt4);
			high_resolution_clock::time_point t2 = high_resolution_clock::now();
			auto enc_time = duration_cast<nanoseconds>(t2-t1).count();
			//std::cout<<"ciphertext of 4*Enc(0): "<<cipher0_4<<std::endl;
			avg_enc += enc_time;
			extern EC_GROUP *init_group;
				       		BIGNUM *x = BN_new();
				       			BIGNUM *y = BN_new();
			std::cout << "cipher0_4->C1=" << std::endl;
								if(EC_POINT_get_affine_coordinates_GFp(init_group, cipher0_4->C1, x, y, NULL)) {
									BN_print_fp(stdout, x);
									putc('\n', stdout);
									BN_print_fp(stdout, y);
									putc('\n', stdout);
								}
								else {
											std::cerr << "Can't get point coordinates." << std::endl;
										}
	   	    }
*/

	    std::cout<<"avg_enc "<<avg_enc<<std::endl;
	    std::cout<< "Data points: "<< num_points*10 << std::endl;
	    std::cout << "ENC Time " <<  avg_enc / 1000000000.0 <<  " s" <<std::endl; //"ms Avg DEC Time " << avg_dec / 1000000.0 << " ms" << std::endl;
	    //avg_enc = avg_enc / num_entries;
	    //avg_dec = avg_dec / num_entries;
	   // std::cout << "Avg ENC Time " <<  avg_enc / 1000000000.0 << " s"<<std::endl;//"ms Avg DEC Time " << avg_dec / 1000000.0 << " ms" << std::endl;

	    //gamal_cipher_clear(cipher1);
	    //gamal_cipher_clear(cipher2);
	    //gamal_cipher_clear(cipher2_new);
	    gamal_key_clear(key);
	    gamal_free_bsgs_table(table);
	    gamal_deinit();
}


void partialViewCollect(int datasize, int attr_num, dig_t big_data_point){
		gamal_key_t key;
		//gamal_ciphertext_t pir[10000], data_encrypt[1000][10], answer_cipher, temp;//, temp_ciph;
		//dig_t answer, pt, plain1, plain0, pt1, pt2, data_clear[1000][10];
		//int pir_size = 2*datasize;
		gamal_ciphertext_t pir[datasize], data_encrypt[datasize][attr_num], answer_cipher, temp, pir_point;//, temp_ciph;
		dig_t answer, pt, plain1, plain0, pt1, pt2, data_clear[datasize][attr_num];
		bsgs_table_t table;
		dig_t data_point;
		//dig_t big_data_point = 10000;
		srand (time(NULL));
		double avg_mult = 0, avg_add = 0, e2e_time = 0;//, avg_dec = 0;

		plain1 = 1; plain0 = 0;

		gamal_init(CURVE_256_SEC);
		gamal_generate_keys(key);
		gamal_init_bsgs_table(table, (dig_t) 1L << 16);


		//generate pir vector: V enc(1) and n-V enc(0)
		int n, m;
		//n= datasize, m = attr_num;
		int V = (int)n/100;
		int it=0;
		printf("test OK\n");

		for (it=0; it<=datasize-1; it++){
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

		for(int i=0; i<=datasize-1; i++) {
			for(int j=0; j<=attr_num-1; j++){
						//printf("test OK\n");

						data_point = ((dig_t) rand())% big_data_point + 1;
						data_clear[i][j] = data_point;
						//std::cout<<"data point: "<<data_clear[i][j]<<std::endl;
					}
		}


		for (int i=0; i<=datasize-1; i++ ){
			//std::cout<<"Test ok i= "<<i<<std::endl;
			for (int j=0; j<= attr_num-1; j++) {
				//std::cout<<"Test ok j = "<<j<<std::endl;
							// gamal_encrypt(cipher1, key, plain);
					high_resolution_clock::time_point t1 = high_resolution_clock::now();

					gamal_mult_opt(data_encrypt[i][j], pir[i], data_clear[i][j]);

					high_resolution_clock::time_point t2 = high_resolution_clock::now();

					auto mult_time = duration_cast<nanoseconds>(t2-t1).count();

					avg_mult += mult_time;
			}
			//printf("Test ok1-2\n");
		}

			 //printf("Test ok2\n");
			 std::cout<<"time: "<<avg_mult/1000000000.0<<" sec"<<std::endl;

}




int main() {
    std::cout << OPENSSL_VERSION_TEXT << std::endl;
    //EC_GROUP *init_group = NULL;

    //====== SETUP ========
    //pre_encryption(1000000);
    //partialViewCollect(int datasize, int attr_num, dig_t big_data_point)
    //partialViewCollect(100000, 10, 1000);

    // ======= Plain EC-ElGamal =====

   // test1();
  // std::cout << "Plain EC-ElGamal 32-bit integers" << std::endl;
   //bench_elgamal(10, 16);
    //std::cout << "Plain EC-ElGamal 32-bit integers" << std::endl;
    //bench_elgamal_add(10, 16);
   // std::cout << "Plain EC-ElGamal 32-bit integers" << std::endl;
   // bench_elgamal_mult(10, 16, 1000000);
    //std::cout<<"Test mult opt: "<<std::endl;
    test_mult_opt(100,1000);

   //std::cout<<"Test query computation: "<<std::endl;
   // test_COUNT_query_computation(1000000);
    //test_SUM_query_computation(10);

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

    //==== Collective Key Gen for EC-ElGamal=====
    //std::cout<< "EC ElGamal test collective key gen "<<std::endl;
   // test_coll_key_gen(10000);
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
