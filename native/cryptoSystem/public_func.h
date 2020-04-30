#ifndef PUBLIC_FUNCTION
#define PUBLIC_FUNCTION

#include "./include/public_header.h"


/** 
 * @file public_func.h
 * @brief Functions facilitate ciphertext presentation.
 * @author Tham Nguyen tham.nguyen@mq.edu.au, Nam Bui, Data Ring
 * @date April 2020
*/


/**
 * This function prints ciphertexts in a vector
 * @param index: index of the vector
 * @param myPIR_enc: vector includes ciphertext
 * @return: representation of all ciphertexts including EC point C1 and C2
*/

void printEncData(int index, gamal_ciphertext_t *myPIR_enc);

/**
 * This function prints a ciphertext
 * @param ciphertext: the ciphertext to print
 * @return: representation of a ciphertext including EC point C1 and C2
*/

void printCiphertext(gamal_ciphertext_t ciphertext);

/**
 * This function produces a random number in a range of (min:max)
 * @param min: 
 * @param max: 
 * @return: a random number
*/

int getRandomInRange(int min, int max);

// void decryptFind(map<string, gamal_ciphertext_t *> enc_domain_map, gamal_key_t key, bsgs_table_t table);


#endif