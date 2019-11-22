#ifndef PARTICIPANT
#define PARTICIPANT

#include "../include/public_header.h"

class Participant
{
    static const string DATA_DIR;
public:
    ENC_DOMAIN_MAP enc_domain_map;
    hash_map hashMap, plain_domain_map;
    int size_dataset;

    Participant();
    void addDummy();
    void processData();
    void print_hash_map();
    void multiply_enc_map(int *plain_track_list, gamal_ciphertext_t *myPIR_enc, ENC_Stack &pre_enc_stack);
    void testWithoutDecrypt();
};

#endif
