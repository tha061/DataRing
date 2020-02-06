#ifndef PARTICIPANT
#define PARTICIPANT

#include "../include/public_header.h"
#include "ENC_Stack.h"
#include "process_noise.h"

class Participant
{

public:
    string DATA_DIR;
    ENC_Stack pre_enc_stack_participant;
    ENC_DOMAIN_MAP enc_domain_map; //partial view encrypted
    hash_pair_map hashMap, plain_domain_map, fakeHashMap;
    gamal_key_t keys; //added by Tham 21 Jan
    int size_dataset;
    double pv_ratio;
    float epsilon_q;
    float epsilon_test;
    float sensitivity;
    double maxNoise_q;
    double minNoise_q;
    double maxNoise_test;
    double minNoise_test;
    int no_lied_answer; 

    Participant();
    Participant(string data_dir);

    //RENAME THIS FUNCTION to addDummy_to_Histogram()
    void addDummy_to_Histogram(int factorSize); //add dummyy '0' to the histogram
    void addDummy_ones_FakeHistogram(int factorSize, float adding_ones); //added by Tham 29 Jan
    // replace n-keepDomainS with dummy of E(1)
    void addDummy_FakeHist(int keepDomainS, int factorSize);

    // replace n-keepDomainS by E(0) and make n-keepDomainS dummy E(1)
    void addDummy_FakeHist_random(int keepDomainS, int factorSize);

    // make vector by self and make PV View without Servers
    void selfCreateFakePV(int fakeEnc1, int factorSize);
    void selfCreateFakePV_opt(bool useTruth);
    void selfCreate_Fake_Historgram(int fakeEnc1, int factorSize);

    void initializePreStack(gamal_key_t coll_key);

    //RENAME THIS FUNCTION to create_OriginalHistogram()
    void create_OriginalHistogram(int datasize_row);
    void print_hash_map();

    ////RENAME THIS FUNCTION to generatePV()
    void pre_process_generatePV(bool useTruth);
    void generatePV_opt(gamal_ciphertext_t *myPIR_enc, bool useTruth);
    void generatePV(int *plain_track_list, gamal_ciphertext_t *myPIR_enc, bool useTruth);
    void test_cleartext();
    
    //RENAME THIS FUNCTION to computeAnswer()
    // adding a function to generate Laplace noise and then add this noise to the computed answer
    void computeAnswer(ENC_DOMAIN_MAP &enc_test_map, gamal_ciphertext_t sum_cipher, bool useTruth, gamal_key_t &coll_key);
    void computeAnswer_opt(ENC_DOMAIN_MAP &enc_test_map, gamal_ciphertext_t sum_cipher, bool useTruth, gamal_key_t &coll_key, float epsilon_i);
};

#endif