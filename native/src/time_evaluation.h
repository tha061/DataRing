#ifndef TIME_EVALUATION
#define TIME_EVALUATION

#include "../include/public_header.h"

double timeEvaluate(string task_name, high_resolution_clock::time_point t1, high_resolution_clock::time_point t2);
void trackTaskPerformance(TRACK_LIST &time_track_list, string task_name, high_resolution_clock::time_point t1, high_resolution_clock::time_point t2);
int computeTimeEvaluation();
void storeTimeEvaluation(int argc, char **argv, TRACK_LIST &time_track_list, bool verify_status);


#endif