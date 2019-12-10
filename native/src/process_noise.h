#ifndef NOISES
#define NOISES

#include "../include/public_header.h"

// float generatePVTestCondition(int dataset, int PV, int known_records, double eta);
int getLaplaceNoise(double sensitivity, double epsilon);
double getLaplaceNoiseRange(float sensitivity, float epsilon, float prob);
// vector<double> estimate_conf_interval(double alpha, int PV_answer, int dataset_size, int PV_size);

#endif