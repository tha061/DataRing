#ifndef GENERATE_ENV_VECTOR
#define GENERATE_ENV_VECTOR

/**
 * @file generate_enc_vectors.h
 * @brief Helper functions to generate a vector of binary value 
 * @details The generated binary vector is then encrypted to serve as the sampling vector
 * @author Tham Nguyen tham.nguyen@mq.edu.au, Nam Bui, Data Ring
 * @date April 2020
*/
int findCeil(int arr[], int r, int l, int h);
int myRand(int arr[], int freq[], int n);
int *hist_gen(int histogr[], int arr[], int freq[], int datasize, int scale_up);
int *pir_gen(int *myPIR_arr, int arr[], int freq[], int datasize, double pv_ratio);

#endif