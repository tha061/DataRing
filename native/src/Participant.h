#ifndef PARTICIPANT
#define PARTICIPANT

#include "../include/public_header.h"
#include "ENC_Stack.h"
#include "process_noise.h"

class Participant
{

public:
    string DATA_DIR;
    ENC_Stack pre_enc_stack;
    ENC_DOMAIN_MAP enc_domain_map;
    hash_pair_map hashMap, plain_domain_map, fakeHashMap;
    int size_dataset;

    Participant();
    Participant(string data_dir);

    void addDummy(int factorSize);
    // replace n-keepDomainS with dummy of E(1)
    void addDummyFake_1(int keepDomainS, int factorSize);

    // replace n-keepDomainS by E(0) and make n-keepDomainS dummy E(1)
    void addDummyFake_2(int keepDomainS, int factorSize);

    // make vector by self and make PV View without Servers
    void selfIntializePV(int fakeEnc1, int factorSize);

    void initializePreStack(gamal_key_t coll_key);
    void processData(int datasize_row);
    void print_hash_map();
    void multiply_enc_map(int *plain_track_list, gamal_ciphertext_t *myPIR_enc, bool useTruth);
    void testWithoutDecrypt();
    void proceedTestFunction(ENC_DOMAIN_MAP &enc_test_map, gamal_ciphertext_t sum_cipher, bool useTruth, gamal_key_t &coll_key, double prob);
};

#endif
