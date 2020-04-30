#ifndef TIME_EVALUATION
#define TIME_EVALUATION


#include "../include/public_header.h"


/**
 * @file time_evaluation.h 
 * @brief Functions to trace runtime and test status of all operations.
 * @author Tham Nguyen tham.nguyen@mq.edu.au, Nam Bui, Data Ring
 * @date April 2020
*/


/**
 * This function computes runtime of an operation
 * @param task_name: operation name
 * @param t1: start
 * @param t2: end
 * @return runtime of an operation in milisecond
*/
double timeEvaluate(string task_name, high_resolution_clock::time_point t1, high_resolution_clock::time_point t2);

/**
 * This function computes runtime of an operation
 * @param task_name: operation name
 * @param test_status: verification result
 * @return 1 = "pass"; 0 = "fail"
*/
void trackTaskStatus(TRACK_LIST &time_track_list, string task_name, bool test_status);

/**
 * This function traces runtime of an operation
 * @param task_name: operation name
 * @param t1: start
 * @param t2: end
*/

void trackTaskPerformance(TRACK_LIST &time_track_list, string task_name, high_resolution_clock::time_point t1, high_resolution_clock::time_point t2);

/**
 * This function traces test result of an operation
 * @param task_name: operation name
 * @param test_result: verification result
*/
void trackTestAccu(TRACK_LIST &time_track_list, string task_name, int test_result);


// int computeTimeEvaluation();

/**
 * This function stores runtime and test result to a text file
*/
void storeTimeEvaluation(int argc, char **argv, TRACK_LIST &time_track_list, bool verify_status);


#endif