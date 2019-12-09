#ifndef NOISES
#define NOISES

#include "../include/public_header.h"


float getNoiseFromHyper(int N, int v, int L, double prob);
int laplace_noise(double sensitivity, double epsilon);
double getNoiseRangeFromLaplace(float sensitivity, float epsilon, float prob);
double estimate_conf_interval(double alpha, int PV_answer, int datasize, int PVsize);


#endif