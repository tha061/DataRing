/*
 * Copyright (c) 2018, Institute for Pervasive Computing, ETH Zurich.
 * All rights reserved.
 *
 * Author:
 *       Lukas Burkhalter <lubu@inf.ethz.ch>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ECELGAMAL_ECELGAMAL_H
#define ECELGAMAL_ECELGAMAL_H

#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/objects.h>
#include <inttypes.h>
#include "uthash.h"

/** @file ecelgamal.h
    @brief Open API for EC-Elgamal crypto system. 
    @details Tha basic API is developed by Institute for Pervasive Computing, ETH Zurich and re-distributed by the authors of Dataring.
    Data Ring reused basic encryption and developed more functions to use in DataRing.
    @author Lukas Burkhalter <lubu@inf.ethz.ch>, Tham Nguyen tham.nguyen@mq.edu.au, Nam Bui, Data Ring
    @date April 2020
*/
 

// Tham extracted from ecparam.c file
// workaround for the SECG curve names secp192r1 and secp256r1 (which are the same as the curves prime192v1 and prime256v1 defined in X9.62

/** @def DEFAULT_CURVE NID_X9_62_prime192v1
    @brief A macro that returns the default eliptic curve used for EC-Elgamal encryption
*/
#define DEFAULT_CURVE NID_X9_62_prime192v1

/** @def CURVE_256_SEC NID_X9_62_prime256v1
    @brief A macro that returns the an eliptic curve used for EC-Elgamal encryption
*/

#define CURVE_256_SEC NID_X9_62_prime256v1

/** @def MAX_BITS 32
    @brief A macro that returns maximum bit for an integer to be encrypted and decrypted in EC-Elgamal
*/
#define MAX_BITS 32


// KEY
/**
 * @brief This struct defines an EC-Elgamal key pair
 * @details An EC-Elgaml key pair
 * @author Lukas Burkhalter <lubu@inf.ethz.ch>
 * @param is_public
 * @param Y: public key
 * @param secret: private key
*/
struct gamal_key
{
    char is_public;
    EC_POINT *Y; //public_key for encrypt
    BIGNUM *secret; //private_key for decrypt
};

typedef struct gamal_key *gamal_key_ptr; 
typedef struct gamal_key gamal_key_t[1]; 
typedef uint64_t dig_t;

/**
 * @brief  Get the size of the encoded key
 */
size_t get_encoded_key_size(gamal_key_t key, int compressed);
/**
 * @brief  Encode a key
 */
int encode_key(unsigned char *buff, int size, gamal_key_t key, int compressed);
/**
 * @brief  Decode a key
 */
int decode_key(gamal_key_t key, unsigned char *buff, int size);

// EC POINTS - tuple of Encrytion of x
/**
 * @brief This struct defines an EC-Elgamal ciphertext
 * @details An EC-Elgaml ciphertext. EC POINTS - tuple of Encrytion of x
 * @author Lukas Burkhalter <lubu@inf.ethz.ch>
 * @param C1: point in the x axis
 * @param C2: point in the y axis
*/
struct gamal_ciphertext
{
    EC_POINT *C1;
    EC_POINT *C2;
};

typedef struct gamal_ciphertext *gamal_ciphertext_ptr;
typedef struct gamal_ciphertext gamal_ciphertext_t[1];
/**
 * @brief  Get the size of the encoded ciphertext
 */
size_t get_encoded_ciphertext_size(gamal_ciphertext_t ciphertext);
/**
 * @brief  Encode a ciphertext
 */
int encode_ciphertext(unsigned char *buff, int size, gamal_ciphertext_t ciphertext);
/**
 * @brief  Decode a ciphertext
 */
int decode_ciphertext(gamal_ciphertext_t ciphertext, unsigned char *buff, int size);

/**
 * @brief This struct defines a hash table for the EC Elgamal decryption algorithm
 * @details A table to map a value to a key
 * @author Lukas Burkhalter <lubu@inf.ethz.ch>
 * @param key: to be mapped
 * @param value: the matched value to key
*/
typedef struct bsgs_hash_table_entry
{
    unsigned char *key;
    uint32_t value;
    UT_hash_handle hh;
} bsgs_hash_table_entry_t;

/**
 * @brief This struct defines a hash table for the EC Elgamal decryption algorithm
 * @details A table to map an EC_POINT to a value 
 * @author Lukas Burkhalter <lubu@inf.ethz.ch>
 * @param table: the lookup table including pre-computed matching
 * @param mG: the value to be mapped to m
 * @param group:
 * @param tablesize: size of the lookup table
*/

struct bsgs_table_s
{
    bsgs_hash_table_entry_t *table;
    EC_POINT *mG, *mG_inv;
    EC_GROUP *group;
    dig_t tablesize;
};
typedef struct bsgs_table_s *bsgs_table_ptr;
typedef struct bsgs_table_s bsgs_table_t[1];

 

/**
 * @brief  Inits the library with the given curve
 * @param cure_id: ID of given elliptic curve
 */
int gamal_init(int curve_id);

/**
 * @brief Deinits the library
 * @return
 */
int gamal_deinit();


/**
 * @brief Returns the EC_Group (elliptic curve group) struct if initialized
 */
EC_GROUP *gamal_get_current_group();


/**
 * @brief Returns the encded size of an EC-Point in this group.
 */
int gamal_get_point_compressed_size();

/** 
 * @brief Inititlaizes the baby-step-giant-step table.
 * @param the table
 * @param the number of elemnts to store in the table
 * @return
 */
int gamal_init_bsgs_table(bsgs_table_t table, dig_t size);

/**
 * @brief Frees the memory of the table
 * @param table
 * @return
 */
int gamal_free_bsgs_table(bsgs_table_t table);
/**
 * @brief  Clear a key pair
 */
int gamal_key_clear(gamal_key_t keys);

int gamal_key_to_public(gamal_key_t pub, gamal_key_t priv);
int gamal_cipher_clear(gamal_ciphertext_t cipher);

/**
 * @brief Create a new ciphertext
 * @param cipher: new cipher text, C1 and C2 is the \O point of the x0y
 * @return
 */
int gamal_cipher_new(gamal_ciphertext_t cipher);

/**
 * @brief Generates an EC-Elgamal key pair
 * @param keys the EC-ElGamal keypair
 * @return
 */
int gamal_generate_keys(gamal_key_t keys);

/**
 * @brief Encrypts an Integer with additadive homomorphic EC-ElGamal
 * @param ciphertext
 * @param key
 * @param plaintext
 * @return
 */
int gamal_encrypt(gamal_ciphertext_t ciphertext, gamal_key_t key, dig_t plaintext);

/**
 * @brief Decrypts an EC-Elgamal ciphertext
 * @param res the resulting plaintext integer
 * @param key
 * @param ciphertext
 * @param table if NULL bruteforce is used
 * @return
 */

int gamal_decrypt(dig_t *res, gamal_key_t key, gamal_ciphertext_t ciphertext, bsgs_table_t table);

/**
 * @brief Adds two EC-Elgamal ciphertext and stores it in res.
 * @param res the resulting ciphertext
 * @param ciphertext1
 * @param ciphertext2
 * @return
 */
int gamal_add(gamal_ciphertext_t res, gamal_ciphertext_t ciphertext1, gamal_ciphertext_t ciphertext2);


//==========================================================================================//
//=============== Below are functions added by Tham =======================================//

/**
 * @brief Multiplication of a ciphertext by a scalar.
 * @details Implemented by authors of Data Ring.
 * This function use the double-and-add scheme to provide the scalar multipication of a ciphertext.
 * @param res: the resulted ciphertext
 * @param ciphertext: ciphertext to be scalar
 * @param pt: scalar is an integer >=1 
 */

int gamal_mult_opt(gamal_ciphertext_t res, gamal_ciphertext_t ciphertext, dig_t pt);



/**
 * @brief Generate a collective public key from a set of public keys.
 * @details Implemented by authors of Data Ring.
 * Generates an collective EC-ElGamal key pair from number of parties.
 * @param coll_keys: the collective keypair
 * @param p_key_list: point to the list of public keys of servers
 * @param size_key_list: number of public keys = number of servers
 * @return a colective
 */
int gamal_collective_publickey_gen(gamal_key_t coll_keys, EC_POINT **p_key_list, int size_key_list);


/**
 * @brief A leading server initiates the key switching of an encryption to under a new collective public key
 * @details Implemented by authors of Data Ring.
 * This function for a leading server to intitiate the key switching for re-encryption
 * @param cipher_update: partially re-encrypted cipher
 * @param cipher: original ciphertext to be re-encrypted
 * @param keys_lead: key pair of the leading server (both public key and secret key)
 * @param ketsNew: new public key
*/

int gama_key_switch_lead(gamal_ciphertext_t cipher_update, gamal_ciphertext_t cipher, gamal_key_t keys_lead, gamal_key_t keysNew);

/**
 * @brief A following server operations in the key switching of an encryption to under a new collective public key
 * @details Implemented by authors of Data Ring.
 * This function for a leading server to continue the key switching for re-encryption.
 * @param cipher_update: partially re-encrypted cipher
 * @param cipher: original ciphertext to be re-encrypted
 * @param keys_follow: key pair of the following servers (both public key and secret key)
 * @param ketsNew: new public key
*/
int gama_key_switch_follow(gamal_ciphertext_t cipher_update, gamal_ciphertext_t cipher, gamal_key_t keys_follow, gamal_key_t keysNew);


/**
 * @brief Simple re-encrytion function
 * @details Implemented by authors of Data Ring.
 * Re-encrypt a ciphertext under one (1) server's public key to a ciphertext under a new public key
 * @param new_cipher
 * @param cipher: ciphertext under server's public key
 * @param keys: server's key pair
 * @param keySNew: new public key
 */

int gamal_re_encrypt(gamal_ciphertext_t new_cipher, gamal_ciphertext_t cipher, gamal_key_t keys, gamal_key_t keysNew);


/**
 * @brief A leading server initiate the threshold decryption
 * @details Implemented by authors of Data Ring.
 * Collective decrypts an EC-ElGamal ciphertext by all parties
 * This function for a leading server to initiate the threshold decryption
 * @param ciphertext_update: partially decrypted by the leading server
 * @param keys: the key pair of the leading server (pk and sk)
 * @param ciphertext: original ciphertext to be decrypted
 * @param table: lookup table for decryption
 *
 */
int gamal_coll_decrypt_lead(gamal_ciphertext_t ciphertext_update, gamal_key_t keys, gamal_ciphertext_t ciphertext, bsgs_table_t table);

/**
 * @brief Following servers join the threshold decryption.
 * @details Implemented by authors of Data Ring.
 * Collective decrypts an EC-ElGamal ciphertext by all parties
 * This function for a following server to continue the threshold decryption
 * @param ciphertext_update: partially decrypted by the leading server
 * @param keys: the key pair of the following servers (pk and sk)
 * @param ciphertext: original ciphertext to be decrypted
 * @param table: lookup table for decryption
 *
 */

int gamal_coll_decrypt_follow(gamal_ciphertext_t ciphertext_update, gamal_key_t keys, gamal_ciphertext_t ciphertext, bsgs_table_t table);

/**
 * @brief All servers jointly compute the final plaintext of a ciphertext from the threshold decryption.
 * @details Implemented by authors of Data Ring.
 * Collective decrypts an EC-ElGamal ciphertext by all parties
 * This function for all server to join and decrypt a ciphertext
 * this function will call the gamal_coll_decrypt_lead() and  gamal_coll_decrypt_lead() functions
 * @param res: result is a plaintext
 * @param num_server: the number of servers to jointly decrypt
 * @param ciphertext_update: partially decrypted by the leading server
 * @param key_lead: the key pair of the leading server (pk and sk)
 *  @param key_follow[]: the set of key pair of the following servers (pk and sk)
 * @param ciphertext: original ciphertext to be decrypted
 * @param table: lookup table for decryption
 *
 */
int gamal_fusion_decrypt(dig_t *res, int num_server, gamal_key_t key_lead, gamal_key_t key_follow[], 
                        gamal_ciphertext_t ciphertext_update, gamal_ciphertext_t ciphertext, bsgs_table_t table);



/**
 * @brief Helper function for the scalar multiplication
 * @details Implemented by authors of Data Ring.
 * For utility functions
 */
int *convert_to_bin(int binary_arr[64], dig_t number, int bit_num);
/**
 * @brief Helper function for the scalar multiplication
 * @details Implemented by authors of Data Ring.
 * For utility functions
 */
int *convert_to_NAF(int naf_arr[64], dig_t number, int bit_num[2]);
/**
 * @brief Helper function for the scalar multiplication
 * @details Implemented by authors of Data Ring.
 * For utility functions
 */
int mods_func(int number);
/**
 * @brief Helper function for the scalar multiplication
 * @details Implemented by authors of Data Ring.
 * For utility functions
 */
int findCeil(int arr[], int r, int l, int h);
/**
 * @brief Helper function for the scalar multiplication
 * @details Implemented by authors of Data Ring.
 * For utility functions
 */
int myRand(int arr[], int freq[], int n);
/**
 * @brief Helper function for generating a histogram
 * @details Implemented by authors of Data Ring.
 * For utility functions
 */
int *hist_gen(int histogr[], int arr[], int freq[], int datasize, int scale_up);
// int *pir_gen(int myPIR_arr[], int arr[], int freq[], int datasize, double pv_ratio);


//=================================================================//

// /**
//  * Implemented by authors of Data Ring.
//  * This function add the ciphertext to itself for (scalar - 1) times
//  * Multiply EC-Elgamal ciphertext and a plaintext and stores it in res.
//  * @param res the resulting ciphertext
//  * @param ciphertext
//  * @param pt: scalar is an integer >=1 
//  * @return res
//  */

int gamal_mult(gamal_ciphertext_t res, gamal_ciphertext_t ciphertext1, dig_t pt);




// /**
//  * Implemented by authors of Data Ring.
//  * Substract two ciphertexts 
//  * Might not be able to decrypt the result, need to double check
//  */

int gamal_subtract(gamal_ciphertext_t res, gamal_ciphertext_t ciphertext1, gamal_ciphertext_t ciphertext2);



// /**
//  * @brief Key switching process of three servers
//  * Implemented by authors of Data Ring.
//  * Re-encrypt (switching)) a ciphertext under a collective public key of a group of 3 servers to a ciphertext under a new public key
//  * @param new_cipher
//  * @param cipher: ciphertext under server's public key
//  * @param keys1, keys2, keys3: three servers' key pairs
//  * @param keySNew: new public key
//  */
int gamal_key_switching(gamal_ciphertext_t new_cipher, gamal_ciphertext_t cipher, gamal_key_t keys1, gamal_key_t keys2, 
                        gamal_key_t keys3, gamal_key_t keysNew);



// /**
//  * Implemented by authors of Data Ring.
//  * Collective decrypts an EC-ElGamal ciphertext by 3 servers
//  *
//  */

int gamal_coll_decrypt(dig_t *res, gamal_key_t keys1, gamal_key_t keys2, gamal_key_t keys3, gamal_ciphertext_t ciphertext, bsgs_table_t table);


// /**
//  * Implemented by authors of Data Ring.
//  * Generates an collective EC-ElGamal key pair from 3 servers
//  * @param coll_keys: the collective keypair
//  * @return
//  */
//int gamal_collective_key_gen(gamal_key_t coll_keys);
int gamal_collective_key_gen(gamal_key_t coll_keys, gamal_key_t keys1, gamal_key_t keys2, gamal_key_t keys3);



#endif //ECELGAMAL_ECELGAMAL_H
