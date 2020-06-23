// #include "evaluation/testing2.cpp"
// #include "evaluation/testing-PV-submission-verify.cpp"
// #include "evaluation/test-runtime.cpp"
// #include "evaluation/test_query_testfunction_process.cpp"
// #include "evaluation/testFunction_estimation.cpp"
// #include "evaluation/fakePV_fakeResponse_test.cpp"
// #include "evaluation/runtime_comparison.cpp"
// #include "evaluation/varying_params.cpp"
// #include "evaluation/release_answer.cpp"
// #include "evaluation/testing-PartialView_verification_selfPV.cpp"
// #include "evaluation/test_query_testfunction_process_new_PV_gen_method.cpp"
// #include "evaluation/runtime_new_scheme.cpp"
// #include "evaluation/test_query_testfunction_process_question_answer_random.cpp"
#include "evaluation/pvCollection_Verification.cpp"
#include "evaluation/working_flow.cpp"
#include "evaluation/cheating_detection_query_phase_question_answer_random.cpp"



int main(int argc, char **argv)
{
	
	
	
	srand(time(NULL));
	// pvCollection(argc, argv);
	query_evaluation_with_cheating_detection_random_query_ans(argc, argv);
	// working_flow(argc, argv);
	// cheating_detection(argc, argv);


    // test_mult_opt(1, 1000);

	// runtime_testing(argc, argv);
	
	// runtime_new_scheme(argc, argv);
	
	// phase_3_test(argc, argv);

	// phase_3_test_new_PVGen_method(argc,argv);

	// phase_3_test_question_random(argc,argv);

	
	// partialView_verification(argc, argv);
	
	// partialView_verification_selfPV(argc,argv);

	// test_key_switch_new(3);
	// test_estimation(argc, argv);
	// fakePV_fakeResponse_test(argc, argv);
	// runtime_comparison(argc, argv);
	// varying_params(argc, argv);

	// release_answer(argc, argv);

	// bench_elgamal_add_2(1,16);
	
	return 0;
}


