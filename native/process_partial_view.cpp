
#include <iostream>
#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cmath>
#include <stack>

extern "C"
{
#include "ecelgamal.h"
#include "crtecelgamal.h"
#include "generate_enc_vectors.c"
}

using namespace std::chrono;
using namespace std;

void _timeEvaluate(string task_name, high_resolution_clock::time_point t1, high_resolution_clock::time_point t2)
{
	double time_diff = duration_cast<nanoseconds>(t2 - t1).count();
	cout << "\n -------------------------------------------------------------------- \n";
	cout << "\nTime Evaluation \n";
	cout << task_name << " : " << time_diff / 1000000.0 << " ms" << endl;
	cout << "\n -------------------------------------------------------------------- \n";
}

gamal_ciphertext_t *createRandomEncrypVector(gamal_key_t key, bsgs_table_t table, int datasize, int *plain_track_list, ENC_Stack pre_enc_stack)
{
    int *myPIR_arr; //final array with value of encryp type after reverting from shuffle array

    int enc_types[] = {1, 0};
    int freq[] = {1, 99};
    int pv_ratio = 100;

    gamal_ciphertext_t *myPIR_enc; // (C1, C2) - Encryption List

    myPIR_arr = new int[datasize];
    myPIR_enc = new gamal_ciphertext_t[datasize];

    // Use a different seed value for every run.
    // srand(time(NULL));

    pir_gen(myPIR_arr, enc_types, freq, datasize, pv_ratio); // function that server place 1 or 0 randomly


    // ========== Encrypt the vector =============== //

    int plain1 = 1, plain0 = 0;

    for (int i = 0; i < datasize; i++)
    {
        plain_track_list[i] = myPIR_arr[i];
        if (myPIR_arr[i] == 1)
        {
            pre_enc_stack.pop_E1(myPIR_enc[i]);
        }
        else
        {
            pre_enc_stack.pop_E0(myPIR_enc[i]);
        }
    }

    delete[] myPIR_arr;

    return myPIR_enc;
}

void printEncMap(string key, map<string, gamal_ciphertext_t *> enc_domain_map)
{
    extern EC_GROUP *init_group;
    BIGNUM *x = BN_new();
    BIGNUM *y = BN_new();

    cout << "Print summation of all encryption in key " << key << endl;
    printf("encryption of row index #%d->C1:\n", key);
    if (EC_POINT_get_affine_coordinates_GFp(init_group, enc_domain_map[key][0]->C1, x, y, NULL))
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
    printf("encryption of row index #%d->C2:\n", key);
    if (EC_POINT_get_affine_coordinates_GFp(init_group, enc_domain_map[key][0]->C2, x, y, NULL))
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

// func partialViewCollect_histogram
// datasize: size of input dataset
// scale_up: for adding dummy rows (domain) for final partial view
void partialViewCollect_histogram(int datasize, int scale_up)
{
    gamal_key_t key;
    //gamal_ciphertext_t  myPIR_enc[datasize], histogr_encrypt[scale_up*datasize];//, temp_ciph;
    int *myPIR_arr, *histogr;
    gamal_ciphertext_t *myPIR_enc, *histogr_encrypt; // (C1, C2) - Encryption List

    myPIR_arr = new int[datasize];
    myPIR_enc = new gamal_ciphertext_t[datasize];
    histogr = new int[scale_up * datasize];
    histogr_encrypt = new gamal_ciphertext_t[scale_up * datasize];

    //int myPIR_arr[2*datasize] = {0};
    //int histogr[scale_up*datasize], data[datasize] = {1}, dummy[(scale_up - 1)*datasize] = {0};
    bsgs_table_t table;

    // Use a different seed value for every run.
    srand(time(NULL));
    int randomBit;

    // time evaluation
    double pir_clear_gen_time = 0, pir_enc_time = 0, hist_gen_time = 0, pir_apply_hist_time = 0; //, avg_dec = 0;

    int plain1 = 1, plain0 = 0;

    gamal_init(CURVE_256_SEC);
    gamal_generate_keys(key);
    gamal_init_bsgs_table(table, (dig_t)1L << 16);

    //generate pir vector in clear : V of (1) and n-V of (0)

    int arr[] = {1, 0};
    int freq1[] = {1, 99};
    int freq2[] = {1, scale_up};
    int pv_ratio = 100;

    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    pir_gen(myPIR_arr, arr, freq1, datasize, pv_ratio); // function that server place 1 or 0 randomly

    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    pir_clear_gen_time = duration_cast<nanoseconds>(t2 - t1).count();

    //#if 0
    //==================== Generate encrypted pir vector: V enc(1) and n-V enc(0) ==================
    ///**
    t1 = high_resolution_clock::now();

    for (int it = 0; it <= datasize - 1; it++)
    {

        if (myPIR_arr[it] == 1)
        {
            gamal_encrypt(myPIR_enc[it], key, plain1); //this step can use pre-encryption of 1 from the setup phase
        }
        else
        {
            gamal_encrypt(myPIR_enc[it], key, plain0); //this step can use pre-encryption of 0 from the setup phase
        }
    }

    t2 = high_resolution_clock::now();
    pir_enc_time = duration_cast<nanoseconds>(t2 - t1).count();

    delete[] myPIR_arr;

    extern EC_GROUP *init_group;
    BIGNUM *x = BN_new();
    BIGNUM *y = BN_new();

    //for (int it=0; it<3; it++){
    printf("encryption of row index #%d->C1:\n", datasize - 1);
    if (EC_POINT_get_affine_coordinates_GFp(init_group, myPIR_enc[datasize - 1]->C1, x, y, NULL))
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
    printf("encryption of row index #%d->C2:\n", datasize - 1);
    if (EC_POINT_get_affine_coordinates_GFp(init_group, myPIR_enc[datasize - 1]->C2, x, y, NULL))
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

    //}

    //*/
    //================ dataset and dummy data to histogram ======================

    t1 = high_resolution_clock::now();

    hist_gen(histogr, arr, freq2, datasize, scale_up);

    t2 = high_resolution_clock::now();
    hist_gen_time = duration_cast<nanoseconds>(t2 - t1).count();

    //applying pir_enc to histogram by participant
    int index_pir = 0;
    t1 = high_resolution_clock::now();

    // cout << histogr[datasize * scale_up -1] << endl;
    // cout << myPIR_arr[datasize-1] << endl;

    for (int it = 0; it < datasize * scale_up; it++)
    {
        if (histogr[it] == 1)
        {
            gamal_mult_opt(histogr_encrypt[it], myPIR_enc[index_pir], histogr[it]);
            index_pir++;
        }
        else
        {
            gamal_encrypt(histogr_encrypt[it], key, histogr[it]); //this step can use pre-encryption of 0 from the setup phase
        }
    }

    t2 = high_resolution_clock::now();
    pir_apply_hist_time = duration_cast<nanoseconds>(t2 - t1).count();

    delete[] histogr;

    std::cout << "PIR clear gen time: " << pir_clear_gen_time / 1000000.0 << " ms" << std::endl;
    std::cout << "PIR enc time: " << pir_enc_time / 1000000.0 << " ms" << std::endl;
    std::cout << "Hist gen time: " << hist_gen_time / 1000000.0 << " ms" << std::endl;
    std::cout << "pir_apply_hist_time: " << pir_apply_hist_time / 1000000.0 << " ms" << std::endl;
    std::cout << "histogr_encrypt [1] address = " << histogr_encrypt[0] << std::endl;

    //#endif
}

void partialViewCollect_histogram_map(int datasize, int scale_up)
{
    gamal_key_t key;
    //gamal_ciphertext_t  myPIR_enc[datasize], histogr_encrypt[scale_up*datasize];//, temp_ciph;
    int *myPIR_arr, *histogr;
    gamal_ciphertext_t *myPIR_enc, *histogr_encrypt;
    myPIR_arr = new int[datasize];
    myPIR_enc = new gamal_ciphertext_t[datasize];
    histogr = new int[scale_up * datasize];
    histogr_encrypt = new gamal_ciphertext_t[scale_up * datasize];

    //int myPIR_arr[2*datasize] = {0};
    //int histogr[scale_up*datasize], data[datasize] = {1}, dummy[(scale_up - 1)*datasize] = {0};
    bsgs_table_t table;

    // Use a different seed value for every run.
    srand(time(NULL));
    int randomBit;
    double pir_clear_gen_time = 0, pir_enc_time = 0, hist_gen_time = 0, pir_apply_hist_time = 0; //, avg_dec = 0;

    int plain1 = 1, plain0 = 0;

    gamal_init(CURVE_256_SEC);
    gamal_generate_keys(key);
    gamal_init_bsgs_table(table, (dig_t)1L << 16);

    //generate pir vector in clear : V of (1) and n-V of (0)

    int arr[] = {1, 0};
    int freq1[] = {1, 99};
    int freq2[] = {1, scale_up};
    int pv_ratio = 100;

    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    pir_gen(myPIR_arr, arr, freq1, datasize, pv_ratio);
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    pir_clear_gen_time = duration_cast<nanoseconds>(t2 - t1).count();

    //#if 0
    //generate encrypted pir vector: V enc(1) and n-V enc(0)
    ///**
    t1 = high_resolution_clock::now();
    for (int it = 0; it <= datasize - 1; it++)
    {

        if (myPIR_arr[it] == 1)
        {
            gamal_encrypt(myPIR_enc[it], key, plain1); //this step can use pre-encryption of 1 from the setup phase
        }
        else
        {
            gamal_encrypt(myPIR_enc[it], key, plain0); //this step can use pre-encryption of 0 from the setup phase
        }
    }
    t2 = high_resolution_clock::now();
    pir_enc_time = duration_cast<nanoseconds>(t2 - t1).count();

    delete[] myPIR_arr;

    extern EC_GROUP *init_group;
    BIGNUM *x = BN_new();
    BIGNUM *y = BN_new();

    //for (int it=0; it<3; it++){
    printf("encryption #%d->C1:\n", datasize - 1);
    if (EC_POINT_get_affine_coordinates_GFp(init_group, myPIR_enc[datasize - 1]->C1, x, y, NULL))
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
    printf("encryption #%d->C2:\n", datasize - 1);
    if (EC_POINT_get_affine_coordinates_GFp(init_group, myPIR_enc[datasize - 1]->C2, x, y, NULL))
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

    //}

    //*/
    // dataset and dummy data to histogram

    t1 = high_resolution_clock::now();
    hist_gen(histogr, arr, freq2, datasize, scale_up);
    t2 = high_resolution_clock::now();
    hist_gen_time = duration_cast<nanoseconds>(t2 - t1).count();
    //printf("Histogram start: \n");
    //for(int it=0; it<=datasize*scale_up -1; it++){
    //	printf("%d",histogr[it]);
    //}
    // printf("\n");

    //applying pir_enc to histogram by participant
    int index_pir = 0;
    t1 = high_resolution_clock::now();
    for (int it = 0; it < datasize * scale_up; it++)
    {
        if (histogr[it] == 1)
        {
            gamal_mult_opt(histogr_encrypt[it], myPIR_enc[index_pir], histogr[it]);
            index_pir++;
        }
        else
        {
            gamal_encrypt(histogr_encrypt[it], key, histogr[it]); //this step can use pre-encryption of 0 from the setup phase
        }
    }
    t2 = high_resolution_clock::now();
    pir_apply_hist_time = duration_cast<nanoseconds>(t2 - t1).count();

    delete[] histogr;

    std::cout << "PIR clear gen time: " << pir_clear_gen_time / 1000000.0 << " ms" << std::endl;
    std::cout << "PIR enc time: " << pir_enc_time / 1000000.0 << " ms" << std::endl;
    std::cout << "Hist gen time: " << hist_gen_time / 1000000.0 << " ms" << std::endl;
    std::cout << "pir_apply_hist_time: " << pir_apply_hist_time / 1000000.0 << " ms" << std::endl;
    std::cout << "histogr_encrypt [1] address = " << histogr_encrypt[0] << std::endl;

    //#endif
}