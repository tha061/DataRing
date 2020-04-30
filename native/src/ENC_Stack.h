#ifndef ENC_STACK
#define ENC_STACK

#include "../include/public_header.h"

/** 
 * @brief This class defines functions for pre-computating encryptions to improve performance.
 * @details Servers generate two stacks of pre-computed encryption of 1 and 0 and store them offline.
 * Functions in this class including defining stack, initiate stack of encryption, 
 * pop an encrytion from the stack and re-fill extra encryption once 
 * the stack becomes empty.
 * @author Tham Nguyen tham.nguyen@mq.edu.au, Nam Bui, Data Ring
 * @date April 2020
*/


class ENC_Stack
{
    // high_resolution_clock::time_point t1, t2;

public:
    int top; /**<parameters define the stack to store pre-computed encryption*/
    int max_size; /**<size of the stack to store pre-computed encryption*/
    int top_E1; /**<parameters define the stack to store pre-computed encryption*/
    int top_E0; /**<parameters define the stack to store pre-computed encryption*/
    gamal_key_t key; /**<key pairs for encryption*/
    gamal_ciphertext_t *myPIR_enc0; /**<an encryption of 0*/
    gamal_ciphertext_t *myPIR_enc1; /**<an encryption of 1*/

    ENC_Stack(); /**<Default constructor of stack of pre-computed encryption*/
    ENC_Stack(int size, gamal_key_t i_key); /**<Constructor of stack of pre-computed encryption*/

    bool isEmpty(); /**<checking if the stack if empty*/
    // bool push(int plain);
/**
 * This function is to generate a stack to store pre-computed encryption of 0 under collective public key
*/    
    void initializeStack_E0(); 

/**
 * This function is to generate a stack to store pre-computed encryption of 1 under collective public key
*/
    void initializeStack_E1();

/**
 * This function is to take an encryption of 1 from the stack
 * @param ciphertext: a pre-computed encryption
*/
    void pop_E1(gamal_ciphertext_t ciphertext);

/**
 * This function is to take an encryption of 0 from the stack
 * @param ciphertext: a pre-computed encryption
*/
    void pop_E0(gamal_ciphertext_t ciphertext);

/**
 * This function is to pre-compute extra ciphertexts of 0, once the stack is empty
 * @param new_size: number of ciphertexts to refill 
*/

    void reFillStack_E0(int new_size);

/**
 *This function is to pre-compute extra ciphertexts of 1, once the stack is empty
 * @param new_size: number of ciphertexts to refill 
*/
    void reFillStack_E1(int new_size);
};

#endif