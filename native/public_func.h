#ifndef PUBLIC_FUNCTION
#define PUBLIC_FUNCTION

#include "./include/public_header.h"

void printEncData(int index, gamal_ciphertext_t *myPIR_enc);
void printCiphertext(gamal_ciphertext_t ciphertext);
void decryptFind(map<string, gamal_ciphertext_t *> enc_domain_map, gamal_key_t key, bsgs_table_t table);
int getRandomInRange(int min, int max);

#endif