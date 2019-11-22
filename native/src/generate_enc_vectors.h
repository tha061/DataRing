#ifndef GENERATE_ENV_VECTOR
#define GENERATE_ENV_VECTOR

int findCeil(int arr[], int r, int l, int h);
int myRand(int arr[], int freq[], int n);
int *hist_gen(int histogr[], int arr[], int freq[], int datasize, int scale_up);
int *pir_gen(int *myPIR_arr, int arr[], int freq[], int datasize, int pv_ratio);

#endif