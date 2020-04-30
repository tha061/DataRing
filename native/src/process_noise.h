#ifndef NOISES
#define NOISES

#include "../include/public_header.h"

/**
 * @file process_noise.h
 * @brief Functions to generate a random Laplace noise and maximum noise value
 * @author Tham Nguyen tham.nguyen@mq.edu.au, Nam Bui, Data Ring
 * @date April 2020
*/

/**
 * @brief This function generates a random noise from a Lapace distribution
 * @param sensitivity: the query's sensitivity
 * @param epsilon: privacy budget
 * @return a noise scale to (sensitivity/epsilon)
*/
int getLaplaceNoise(double sensitivity, double epsilon);

/**
 * @brief This function determines the range of noises drawn from a Lapace distribution with a given percentile
 * @param sensitivity: the query's sensitivity
 * @param epsilon: privacy budget
 * @param prob: percentile point (all noises to be less than a value with a probability of prob)
 * @return a maximum value of Laplace noise
*/

double getLaplaceNoiseRange(float sensitivity, float epsilon, float prob);
// vector<double> estimate_conf_interval(double alpha, int PV_answer, int dataset_size, int PV_size);

#endif