
#include "include/public_header.h"
#include "src/Participant.h"
#include "src/Server.h"
#include "src/Servers.h"
#include "src/process_noise.h"
#include "src/time_evaluation.h"
#include "public_func.h"

#include "evaluation/testing2.cpp"
#include "evaluation/testing-PV-submission-verify.cpp"
#include "evaluation/test-runtime.cpp"
#include "evaluation/test_query_testfunction_process.cpp"
#include "evaluation/testFunction_estimation.cpp"
#include "evaluation/fakePV_fakeResponse_test.cpp"
#include "evaluation/runtime_comparison.cpp"
#include "evaluation/varying_params.cpp"
#include "evaluation/release_answer.cpp"
#include "evaluation/testing-PartialView_verification_selfPV.cpp"
#include "evaluation/test_query_testfunction_process_new_PV_gen_method.cpp"




int main(int argc, char **argv)
{
	
	srand(time(NULL));


    // test_mult_opt(1, 1000);

	// runtime_testing(argc, argv);
	
	// phase_3_test(argc, argv);

	// phase_3_test_new_PVGen_method(argc,argv);

	
	partialView_verification(argc, argv);
	
	// partialView_verification_selfPV(argc,argv);

	// test_key_switch_new(3);
	// test_estimation(argc, argv);
	// fakePV_fakeResponse_test(argc, argv);
	// runtime_comparison(argc, argv);
	// varying_params(argc, argv);

	// release_answer(argc, argv);
	return 0;
}

