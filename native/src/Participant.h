#ifndef PARTICIPANT
#define PARTICIPANT

#include "../include/public_header.h"

class Participant
{
    static const string DATA_DIR;

public:
    ENC_DOMAIN_MAP enc_domain_map;
    hash_pair_map hashMap, plain_domain_map;
    int size_dataset;

    Participant();

    void addDummy(int factorSize);
    // replace n-keepDomainS with dummy of E(1)
    void addDummyFake_1(int keepDomainS, int factorSize);

    // replace n-keepDomainS by E(0) and make n-keepDomainS dummy E(1)
    void addDummyFake_2(int keepDomainS, int factorSize);

    // make vector by self and make PV View without Servers
    void selfIntializePV(ENC_Stack &pre_enc_stack, int fakeEnc1, int factorSize);

    void processData(int datasize_row);
    void print_hash_map();
    void multiply_enc_map(int *plain_track_list, gamal_ciphertext_t *myPIR_enc, ENC_Stack &pre_enc_stack);
    void testWithoutDecrypt();
    void proceedTestFunction(ENC_DOMAIN_MAP &enc_test_map, gamal_ciphertext_t sum_cipher);
};

#endif
