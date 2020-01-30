
#include "include/public_header.h"
#include "src/Participant.h"
#include "src/Server.h"
#include "src/Servers.h"
#include "src/process_noise.h"
#include "src/time_evaluation.h"
#include "public_func.h"

#include "testing2.cpp"
#include "testing-PV-submission-verify.cpp"
#include "test-runtime.cpp"
#include "test_query_testfunction_process.cpp"




int main(int argc, char **argv)
{
	
	srand(time(NULL));
	// runtime_testing(argc, argv);
	// phase_3_test(argc, argv);
	partialView_verification(argc, argv);

	// test_key_switch_new(3);


	return 0;
}

