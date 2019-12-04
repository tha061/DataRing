#ifndef ENC_STACK
#define ENC_STACK

#include "../include/public_header.h"

class ENC_Stack
{
    // high_resolution_clock::time_point t1, t2;

public:
    int top, max_size, top_E1, top_E0;
    gamal_key_t key;
    gamal_ciphertext_t *myPIR_enc0;
    gamal_ciphertext_t *myPIR_enc1;

    ENC_Stack();
    ENC_Stack(int size, gamal_key_t i_key);

    bool isEmpty();
    // bool push(int plain);
    void initializeStack_E0();
    void initializeStack_E1();
    void pop_E1(gamal_ciphertext_t ciphertext);
    void pop_E0(gamal_ciphertext_t ciphertext);
    void reFillStack_E0(int new_size);
    void reFillStack_E1(int new_size);
};

#endif