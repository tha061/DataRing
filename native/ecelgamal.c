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

#include "ecelgamal.h"
#include <stdio.h>
#include <math.h>

#define KEY_UNCOMPRESSED 0
#define KEY_COMPRESSED 1
#define KEY_PUBLIC 2

EC_POINT *multiply_constant(const EC_POINT *in, const BIGNUM *n, EC_GROUP *curve_group)
{
    EC_POINT *res;
    BIGNUM *bn1;
    BN_CTX *ctx;

    bn1 = BN_new();
    ctx = BN_CTX_new();
    BN_zero(bn1);
    res = EC_POINT_new(curve_group);
    EC_POINT_mul(curve_group, res, bn1, in, n, ctx);

    BN_free(bn1);
    BN_CTX_free(ctx);
    return res;
}

EC_POINT *multiply_generator(const BIGNUM *n, EC_GROUP *curve_group)
{
    return multiply_constant(EC_GROUP_get0_generator(curve_group), n, curve_group);
}

char *point_to_string(EC_GROUP *curve_group, const EC_POINT *point)
{
    BN_CTX *ctx;
    char *s;
    point_conversion_form_t form = POINT_CONVERSION_COMPRESSED;
    ctx = BN_CTX_new();
    s = EC_POINT_point2hex(curve_group, point, form, ctx);
    BN_CTX_free(ctx);
    return s;
}

int add_value_to_table(bsgs_table_t table, EC_POINT *point, uint32_t value)
{
    unsigned char *point_key;
    BN_CTX *ctx = BN_CTX_new();
    size_t point_size = EC_POINT_point2oct(table->group, point, POINT_CONVERSION_COMPRESSED, NULL, 0, ctx);
    bsgs_hash_table_entry_t *new_entry = (bsgs_hash_table_entry_t *)malloc(sizeof(bsgs_hash_table_entry_t));
    point_key = (unsigned char *)malloc(point_size);
    EC_POINT_point2oct(table->group, point, POINT_CONVERSION_COMPRESSED, point_key, point_size, ctx);

    new_entry->key = point_key;
    new_entry->value = value;
    HASH_ADD_KEYPTR(hh, table->table, point_key, point_size, new_entry);
    BN_CTX_free(ctx);
    return 0;
}


int bsgs_table_init(EC_GROUP *curve_group, bsgs_table_t table, dig_t t_size)
{
    dig_t count = 0;
    BIGNUM *bn_size;
    EC_POINT *cur_point;
    const EC_POINT *gen;
    BN_CTX *ctx = BN_CTX_new();

    gen = EC_GROUP_get0_generator(curve_group);
    table->table = NULL;
    table->group = curve_group;

    //set Table metadata
    bn_size = BN_new();
    BN_set_word(bn_size, (BN_ULONG)t_size);
    table->tablesize = t_size;
    table->mG = multiply_constant(gen, bn_size, curve_group);
    BN_set_negative(bn_size, 1);
    table->mG_inv = multiply_constant(gen, bn_size, curve_group);
    BN_free(bn_size);

    gen = EC_GROUP_get0_generator(curve_group);
    cur_point = EC_POINT_new(curve_group);
    EC_POINT_set_to_infinity(curve_group, cur_point);
    for (; count <= t_size; count++)
    {
        add_value_to_table(table, cur_point, count);
        EC_POINT_add(curve_group, cur_point, cur_point, gen, ctx);
    }
    EC_POINT_free(cur_point);
    BN_CTX_free(ctx);
    return 0;
}

size_t bsgs_table_get_size(bsgs_table_t bsgs_table)
{
    return 0;
}

int bsgs_table_free(bsgs_table_t bsgs_table)
{
    bsgs_hash_table_entry_t *tmp, *current;
    HASH_ITER(hh, bsgs_table->table, current, tmp)
    {
        HASH_DEL(bsgs_table->table, current);
        free(current->key);
        free(current);
    }
    EC_POINT_free(bsgs_table->mG);
    EC_POINT_free(bsgs_table->mG_inv);
    return 0;
}

int get_power_from_table(uint64_t *power, bsgs_table_t bsgs_table, EC_POINT *lookup_point)
{
    unsigned char *point_key;
    BN_CTX *ctx = BN_CTX_new();
    size_t point_size = EC_POINT_point2oct(bsgs_table->group, lookup_point, POINT_CONVERSION_COMPRESSED, NULL, 0, ctx);
    bsgs_hash_table_entry_t *entry;
    point_key = (unsigned char *)malloc(point_size);
    EC_POINT_point2oct(bsgs_table->group, lookup_point, POINT_CONVERSION_COMPRESSED, point_key, point_size, ctx);

    HASH_FIND_BIN(bsgs_table->table, point_key, point_size, entry);
    BN_CTX_free(ctx);
    free(point_key);

    if (entry == NULL)
        return -1;
    *power = (uint64_t)entry->value;
    return 0;
}

int solve_ecdlp_bsgs(bsgs_table_t bsgs_table, uint64_t *result, EC_POINT *M, int64_t maxIt)
{
    uint64_t j = 0, i = 0;
    int ok;
    EC_GROUP *curve_group = bsgs_table->group;
    EC_POINT *curPoint = EC_POINT_dup(M, curve_group);
    EC_POINT *curPointNeg = EC_POINT_dup(M, curve_group);
    BN_CTX *ctx = BN_CTX_new();

    // printf("maxIt %ld\n", maxIt);
    // printf("i %ld\n", i);
    while (i <= maxIt)
    {
        // printf("maxIt %ld\n", maxIt);
        // printf("i %ld\n", i);
        // printf("error");
        ok = get_power_from_table(&j, bsgs_table, curPoint);
        // printf("ok %ld\n", ok);
        if (ok == 0)
        {
            *result = i * bsgs_table->tablesize + j;
            break;
        }
        EC_POINT_add(curve_group, curPoint, curPoint, bsgs_table->mG_inv, ctx);
        i = i + 1;
    }

    // printf("after maxIt %ld\n", maxIt);
    // printf("after i %ld\n", i);

    if (i > maxIt)
    {
        return -1;
    }

    EC_POINT_free(curPoint);
    EC_POINT_free(curPointNeg);
    BN_CTX_free(ctx);
    return 0;
}

// Finds the value x with brute force s.t. M=xG
int solve_dlog_brute(EC_GROUP *curve_group, EC_POINT *M, uint64_t *x, dig_t max_it)
{
    EC_POINT *cur;
    const EC_POINT *G;
    uint64_t max, x_local = 1;
    BN_CTX *ctx = BN_CTX_new();
    max = (int64_t)max_it;

    cur = EC_POINT_new(curve_group);
    G = EC_GROUP_get0_generator(curve_group);
    EC_POINT_set_to_infinity(curve_group, cur);

    if (EC_POINT_is_at_infinity(curve_group, M))
    {
        *x = 0;
        return 0;
    }
    else
    {
        for (; x_local < max; (*x) = x_local++)
        {
            EC_POINT_add(curve_group, cur, cur, G, ctx);
            if (EC_POINT_cmp(curve_group, cur, M, ctx) == 0)
            {
                break;
            }
        }
        *x = x_local;
    }
    EC_POINT_free(cur);
    BN_CTX_free(ctx);
    return 0;
}

// API IMPLEMENTATION

//the ec group used
EC_GROUP *init_group = NULL;

int gamal_init(int curve_id)
{
    init_group = EC_GROUP_new_by_curve_name(curve_id);
    return 0;
}

int gamal_deinit()
{
    if (init_group != NULL)
    {
        EC_GROUP_free(init_group);
        init_group = NULL;
    }
    return 0;
}

int gamal_init_bsgs_table(bsgs_table_t table, dig_t size)
{
    return bsgs_table_init(init_group, table, size);
}

int gamal_free_bsgs_table(bsgs_table_t table)
{
    bsgs_table_free(table);
    return 0;
}

int gamal_key_clear(gamal_key_t key)
{
    EC_POINT_clear_free(key->Y);
    if (!key->is_public)
    {
        BN_clear_free(key->secret);
    }
    return 0;
}

int gamal_key_to_public(gamal_key_t pub, gamal_key_t priv)
{
    pub->is_public = 1;
    pub->Y = EC_POINT_dup(priv->Y, init_group);
    return 0;
}

int gamal_cipher_clear(gamal_ciphertext_t cipher)
{
    EC_POINT_clear_free(cipher->C1);
    EC_POINT_clear_free(cipher->C2);
    return 0;
}

int gamal_cipher_new(gamal_ciphertext_t cipher)
{
    cipher->C1 = EC_POINT_new(init_group);
    cipher->C2 = EC_POINT_new(init_group);
    return 0;
}

int gamal_generate_keys(gamal_key_t keys)
{
    BN_CTX *ctx = BN_CTX_new();
    BIGNUM *ord, *key;

    ord = BN_new();
    key = BN_new();
    keys->Y = EC_POINT_new(init_group);

    EC_GROUP_get_order(init_group, ord, ctx);

    BN_rand_range(key, ord);
    keys->is_public = 0;
    keys->Y = multiply_generator(key, init_group);
    keys->secret = key;
    BN_free(ord);
    BN_CTX_free(ctx);
    return 0;
}


// Encryption
int gamal_encrypt(gamal_ciphertext_t ciphertext, gamal_key_t key, dig_t plaintext)
{
    BIGNUM *bn_plain, *ord, *rand;
    BN_CTX *ctx = BN_CTX_new();

    bn_plain = BN_new();
    ord = BN_new();
    rand = BN_new();
    // printf("ElGamal encryption: rand = %d \n", rand);
    ciphertext->C2 = EC_POINT_new(init_group);

    EC_GROUP_get_order(init_group, ord, ctx);
    BN_rand_range(rand, ord);

    BN_set_word(bn_plain, plaintext);

    ciphertext->C1 = multiply_generator(rand, init_group);
    EC_POINT_mul(init_group, ciphertext->C2, bn_plain, key->Y, rand, ctx);

    BN_clear_free(rand);
    BN_free(ord);
    BN_free(bn_plain);
    BN_CTX_free(ctx);
    return 0;
}




// if table == NULL use bruteforce
int gamal_decrypt(dig_t *res, gamal_key_t key, gamal_ciphertext_t ciphertext, bsgs_table_t table)
{
    EC_POINT *M;
    uint64_t plaintext;
    BN_CTX *ctx = BN_CTX_new();

    M = multiply_constant(ciphertext->C1, key->secret, init_group);

    EC_POINT_invert(init_group, M, ctx);

    EC_POINT_add(init_group, M, ciphertext->C2, M, ctx);

    if (table != NULL)
    {
            
        solve_ecdlp_bsgs(table, &plaintext, M, 1L << MAX_BITS);
    }
    else
    {
        solve_dlog_brute(init_group, M, &plaintext, 1L << MAX_BITS);
    }
    *res = (dig_t)plaintext;

    BN_CTX_free(ctx);
    EC_POINT_clear_free(M);
    return 0;
}



int gamal_add(gamal_ciphertext_t res, gamal_ciphertext_t ciphertext1, gamal_ciphertext_t ciphertext2)
{

    /*
		int EC_POINT_add(const EC_GROUP *group, EC_POINT *r, const EC_POINT *a,
		                 const EC_POINT *b, BN_CTX *ctx)
		*/
    res->C1 = EC_POINT_new(init_group);
    res->C2 = EC_POINT_new(init_group);

    BN_CTX *ctx = BN_CTX_new();
    EC_POINT_add(init_group, res->C1, ciphertext1->C1, ciphertext2->C1, ctx);
    EC_POINT_add(init_group, res->C2, ciphertext1->C2, ciphertext2->C2, ctx);
    BN_CTX_free(ctx);
    return 0;
}






EC_GROUP *gamal_get_current_group()
{
    return init_group;
}

int gamal_get_point_compressed_size()
{
    BN_CTX *ctx = BN_CTX_new();
    int res = (int)EC_POINT_point2oct(init_group, EC_GROUP_get0_generator(init_group),
                                      POINT_CONVERSION_COMPRESSED, NULL, 0, ctx);
    BN_CTX_free(ctx);
    return res;
}

// ENCODING + DECODING

void write_size(unsigned char *buffer, size_t size)
{
    buffer[0] = (unsigned char)((size >> 8) & 0xFF);
    buffer[1] = (unsigned char)(size & 0xFF);
}

size_t read_size(unsigned char *buffer)
{
    return ((uint8_t)buffer[0] << 8) | ((uint8_t)buffer[1]);
}

size_t get_encoded_ciphertext_size(gamal_ciphertext_t ciphertext)
{
    return (size_t)gamal_get_point_compressed_size() * 2;
}

int encode_ciphertext(unsigned char *buff, int size, gamal_ciphertext_t ciphertext)
{
    unsigned char *cur_ptr = buff;
    size_t len_point, tmp;
    BN_CTX *ctx = BN_CTX_new();
    len_point = (size_t)gamal_get_point_compressed_size();
    if (size < (len_point * 2))
        return -1;
    tmp = EC_POINT_point2oct(init_group, ciphertext->C1, POINT_CONVERSION_COMPRESSED, cur_ptr, len_point, ctx);
    cur_ptr += len_point;
    if (tmp != len_point)
        return -1;
    tmp = EC_POINT_point2oct(init_group, ciphertext->C2, POINT_CONVERSION_COMPRESSED, cur_ptr, len_point, ctx);
    if (tmp != len_point)
        return -1;
    BN_CTX_free(ctx);
    return 0;
}

int decode_ciphertext(gamal_ciphertext_t ciphertext, unsigned char *buff, int size)
{
    size_t len_point;
    BN_CTX *ctx = BN_CTX_new();
    unsigned char *cur_ptr = buff;
    len_point = (size_t)gamal_get_point_compressed_size();
    if (size < len_point * 2)
        return -1;

    ciphertext->C1 = EC_POINT_new(init_group);
    EC_POINT_oct2point(init_group, ciphertext->C1, cur_ptr, len_point, ctx);
    cur_ptr += len_point;

    ciphertext->C2 = EC_POINT_new(init_group);
    EC_POINT_oct2point(init_group, ciphertext->C2, cur_ptr, len_point, ctx);

    BN_CTX_free(ctx);
    return 0;
}

size_t get_encoded_key_size(gamal_key_t key, int compressed)
{
    size_t size = 1;
    BN_CTX *ctx = BN_CTX_new();
    if (!key->is_public)
    {
        if (compressed)
            size += BN_num_bytes(key->secret);
        else
            size += BN_num_bytes(key->secret) +
                    EC_POINT_point2oct(init_group, key->Y, POINT_CONVERSION_COMPRESSED, NULL, 0, ctx);
    }
    else
    {
        size += EC_POINT_point2oct(init_group, key->Y, POINT_CONVERSION_COMPRESSED, NULL, 0, ctx);
    }
    BN_CTX_free(ctx);
    return size;
}
int encode_key(unsigned char *buff, int size, gamal_key_t key, int compressed)
{
    size_t len_point;
    unsigned char *cur_ptr = buff;
    size_t size_data;
    BN_CTX *ctx = BN_CTX_new();

    len_point = (size_t)gamal_get_point_compressed_size();

    if (size < 3)
        return -1;

    if (key->is_public)
    {
        buff[0] = KEY_PUBLIC;
    }
    else
    {
        if (compressed)
            buff[0] = KEY_COMPRESSED;
        else
            buff[0] = KEY_UNCOMPRESSED;
    }

    cur_ptr++;
    if (key->is_public)
    {
        size_data = len_point;
    }
    else
    {
        if (compressed)
            size_data = (size_t)BN_num_bytes(key->secret);
        else
            size_data = (size_t)BN_num_bytes(key->secret) + len_point;
    }

    if (size < 1 + size_data)
        return -1;

    if (key->is_public)
    {
        EC_POINT_point2oct(init_group, key->Y, POINT_CONVERSION_COMPRESSED, cur_ptr, size_data, ctx);
    }
    else
    {
        if (compressed)
        {
            BN_bn2bin(key->secret, cur_ptr);
        }
        else
        {
            EC_POINT_point2oct(init_group, key->Y, POINT_CONVERSION_COMPRESSED, cur_ptr, len_point, ctx);
            cur_ptr += len_point;
            BN_bn2bin(key->secret, cur_ptr);
        }
    }
    BN_CTX_free(ctx);
    return 0;
}
int decode_key(gamal_key_t key, unsigned char *buff, int size)
{
    size_t len_point;
    char is_pub;
    int is_compressed = 0, decode_id = 0;
    unsigned char *cur_ptr = buff;
    size_t size_data;
    BN_CTX *ctx = BN_CTX_new();

    len_point = (size_t)gamal_get_point_compressed_size();

    if (size < 3)
        return -1;

    decode_id = (int)buff[0];

    if (decode_id == KEY_COMPRESSED)
        is_compressed = 1;

    if (decode_id == KEY_PUBLIC)
        is_pub = 1;
    else
        is_pub = 0;

    key->secret = BN_new();
    cur_ptr++;
    key->is_public = is_pub;
    if (key->is_public)
    {
        size_data = len_point;
    }
    else
    {
        size_data = (size_t)size - 1;
    }

    if (size < 1 + size_data)
        return -1;
    if (is_pub)
    {
        key->Y = EC_POINT_new(init_group);
        EC_POINT_oct2point(init_group, key->Y, cur_ptr, size_data, ctx);
    }
    else
    {
        if (is_compressed)
        {
            BN_bin2bn(cur_ptr, (int)size_data, key->secret);
            key->Y = multiply_generator(key->secret, init_group);
        }
        else
        {
            key->Y = EC_POINT_new(init_group);
            EC_POINT_oct2point(init_group, key->Y, cur_ptr, len_point, ctx);
            cur_ptr += len_point;
            BN_bin2bn(cur_ptr, (int)size_data - (int)len_point, key->secret);
        }
    }
    BN_CTX_free(ctx);
    return 0;
}



//===================================================================================//
//================== Below are functions implemented by Tham =========================//


//Added by Tham for better multiplication between a ciphertext and a plaintext (scalar multiplication)
int gamal_mult_opt(gamal_ciphertext_t res, gamal_ciphertext_t ciphertext, dig_t pt)
{

    //printf("Test begin\n");
    gamal_ciphertext_t temp, temp1, tmp;
    BN_CTX *ctx = BN_CTX_new();
    BIGNUM *X = BN_new();
	BIGNUM *Y = BN_new();
    EC_POINT *M1, *M2;
    double x = log2(pt);
    int num_bit = floor(x) + 1;
    int bit_num[2];
    int naf_binary_arr[64];
    //int counter1 = 0, counter2 = 0;

    gamal_cipher_new(tmp);
    
    
    // res->C1 = EC_POINT_new(init_group);
    // res->C2 = EC_POINT_new(init_group); 
   

    ///** n-1 doublings and m - 1 additions: double-and-add binary-correct
    // if (pt == 0) 
    // {
    //     res->C1 = 0;
    //     res->C2 = 0;
    // }
    // else if (pt == 1) 
    // {
    //     res->C1 = ciphertext->C1;
    //     res->C2 = ciphertext->C2;
    // }
    if (pt == 0 ) //change by Tham 24 Jan 2020
    {
       printf("gamal_mult_opt require scalar is greater than 0\n"); // updated Mar 2020 by Tham

        // printf("multiply with 0\n");  

        // int i = EC_POINT_get_affine_coordinates(init_group,res->C1, X, Y, ctx); 
        // printf("i = %d\n", i);
        // int j = EC_POINT_get_affine_coordinates(init_group,res->C2, X, Y, ctx); 
        // printf("j = %d\n", j);


        // gamal_ciphertext_t tmp;
        // gamal_cipher_new(tmp);
        // tmp->C1 = ciphertext->C1;
        // tmp->C2 = ciphertext->C2;

        // gamal_ciphertext_t tmp2;
        // gamal_cipher_new(tmp2);
        // tmp2->C1 = ciphertext->C1;
        // tmp2->C2 = ciphertext->C2;

        // res->C1 = ciphertext->C1;
        // res->C2 = ciphertext->C2;


        // BN_CTX *ctx = BN_CTX_new();
        // EC_POINT_invert(init_group, tmp->C1, ctx);
        // EC_POINT_invert(init_group, tmp->C2, ctx);

        // // //tmp2 = cipher(x) - cipher(x) = cipher(0)
        // EC_POINT_add(init_group, res->C1, ciphertext->C1, tmp->C1, ctx);
        // EC_POINT_add(init_group, res->C2, ciphertext->C2, tmp->C2, ctx);

        // res->C1 = EC_POINT_new(init_group);
        // res->C2 = EC_POINT_new(init_group);

        // gamal_cipher_new(res); 
        // gamal_add(res,res,res);

        return -1;

        // gamal_subtract(res,ciphertext,ciphertext);

    }
    else if (pt == 1)
    {
        // printf("multiply with 1\n"); 
        gamal_cipher_new(res);  
    
        EC_POINT_add(init_group, res->C1, res->C1, ciphertext->C1, ctx);
        EC_POINT_add(init_group, res->C2, res->C2, ciphertext->C2, ctx);


    }
    else
    {
        // printf("multiply with x >= 2\n");  
        gamal_cipher_new(res); 
        
        int binary_arr[num_bit];

        convert_to_bin(binary_arr, pt, num_bit); //binary_arr = 100010000....
        //for (int it = num_bit-1; it>=0; it--){
        //printf("%d", binary_arr[it]);
        //}
        //printf("\n");

        temp->C1 = ciphertext->C1;
        temp->C2 = ciphertext->C2;

        for (int it = num_bit - 2; it >= 0; it--)
        {

            EC_POINT_dbl(init_group, res->C1, temp->C1, ctx);
            EC_POINT_dbl(init_group, res->C2, temp->C2, ctx);

            if (binary_arr[it] == 1)
            {
                //counter1++;
                EC_POINT_add(init_group, res->C1, res->C1, ciphertext->C1, ctx);
                EC_POINT_add(init_group, res->C2, res->C2, ciphertext->C2, ctx);
            }
            temp->C1 = res->C1;
            temp->C2 = res->C2;
        }
    }

    BN_CTX_free(ctx);
    EC_POINT_clear_free(M1);
    EC_POINT_clear_free(M2);

    //printf("Test mult end\n");
    return 0;
}


//int gamal_mult(gamal_ciphertext_t res, gamal_ciphertext_t ciphertext1, int pt);
//added by Tham
int gamal_mult(gamal_ciphertext_t res, gamal_ciphertext_t ciphertext1, dig_t pt)
{

    gamal_ciphertext_t temp;
    /**
	res->C1 = EC_POINT_new(init_group);
	res->C2 = EC_POINT_new(init_group);
	temp->C1 = EC_POINT_new(init_group);
	temp->C2 = EC_POINT_new(init_group);
	if (pt == 1) {
		//res = ciphertext1;
				res->C1 = ciphertext1->C1;
				res->C2 = ciphertext1->C2;
	} else if (pt == 2){
		BN_CTX *ctx = BN_CTX_new();
		EC_POINT_dbl(init_group, res->C1, ciphertext1->C1, ctx);
		EC_POINT_dbl(init_group, res->C2, ciphertext1->C2, ctx);
		BN_CTX_free(ctx);

	} else {

		temp->C1 = ciphertext1->C1;
		temp->C2 = ciphertext1->C2;
		for(int it=0; it<pt-1; it++){
				gamal_add(res,temp,ciphertext1);
				temp->C1 = res->C1;
				temp->C2 = res->C2;
			}
	}
*/

    res->C1 = EC_POINT_new(init_group);
    res->C2 = EC_POINT_new(init_group);
    temp->C1 = EC_POINT_new(init_group);
    temp->C2 = EC_POINT_new(init_group);

    if (pt == 1)
    {
        //res = ciphertext1;
        res->C1 = ciphertext1->C1;
        res->C2 = ciphertext1->C2;
    }
    else
    {

        temp->C1 = ciphertext1->C1;
        temp->C2 = ciphertext1->C2;
        for (int it = 0; it < pt - 1; it++)
        {
            gamal_add(res, temp, ciphertext1);
            temp->C1 = res->C1;
            temp->C2 = res->C2;
        }
    }

    return 0;
}


//added by Tham for subtraction 
int gamal_subtract(gamal_ciphertext_t res, gamal_ciphertext_t ciphertext1, gamal_ciphertext_t ciphertext2)
{

    /*
		int EC_POINT_add(const EC_GROUP *group, EC_POINT *r, const EC_POINT *a,
		                 const EC_POINT *b, BN_CTX *ctx)
		*/
    res->C1 = EC_POINT_new(init_group);
    res->C2 = EC_POINT_new(init_group);

    gamal_ciphertext_t tmp;
    gamal_cipher_new(tmp);
    tmp->C1 = ciphertext2->C1;
    tmp->C2 = ciphertext2->C2;

    BN_CTX *ctx = BN_CTX_new();
    EC_POINT_invert(init_group, tmp->C1, ctx);
    EC_POINT_invert(init_group, tmp->C2, ctx);

    
    EC_POINT_add(init_group, res->C1, ciphertext1->C1, tmp->C1, ctx);
    EC_POINT_add(init_group, res->C2, ciphertext1->C2, tmp->C2, ctx);
    BN_CTX_free(ctx);
    return 0;
}



//added by Tham: function for fusion all partially decryption to get plaintext
int gamal_fusion_decrypt(dig_t *res, int num_server, gamal_key_t key_lead, gamal_key_t key_follow[], gamal_ciphertext_t ciphertext_update, gamal_ciphertext_t ciphertext, bsgs_table_t table)
{
    //EC_POINT *M1, *M2, *M3;
    uint64_t plaintext;
    BN_CTX *ctx = BN_CTX_new();

    gamal_coll_decrypt_lead(ciphertext_update, key_lead, ciphertext, table);
    // gamal_coll_decrypt_follow(ciphertext_update, key_follow[0], ciphertext,table);
    //gamal_coll_decrypt_follow(ciphertext_update, key_follow[1], ciphertext,table);
    for (int i = 0; i < num_server - 1; i++)
    {
        gamal_coll_decrypt_follow(ciphertext_update, key_follow[i], ciphertext, table);
    }

    EC_POINT_invert(init_group, ciphertext_update->C1, ctx);                              // -r(K1 + K2 + K3) = -rK
    EC_POINT_add(init_group, ciphertext->C2, ciphertext->C2, ciphertext_update->C1, ctx); // m + rK - r(K1+K2+K3) = m +rK - rK = m

    if (table != NULL)
    {
        solve_ecdlp_bsgs(table, &plaintext, ciphertext->C2, 1L << MAX_BITS);
    }
    else
    {
        solve_dlog_brute(init_group, ciphertext->C2, &plaintext, 1L << MAX_BITS);
    }

    *res = (dig_t)plaintext;

    // printf("Test Decrypt fusion OK\n");

    BN_CTX_free(ctx);

    return 0;
}


//added by Tham: this is for a server who takes lead the decryption
int gamal_coll_decrypt_lead(gamal_ciphertext_t ciphertext_update, gamal_key_t keys, gamal_ciphertext_t ciphertext, bsgs_table_t table)
{

    ciphertext_update->C1 = multiply_constant(ciphertext->C1, keys->secret, init_group); //rB*sk1
    return 0;
}
//added by Tham: function for other server partially decrypt the ciphertext
int gamal_coll_decrypt_follow(gamal_ciphertext_t ciphertext_update, gamal_key_t keys, gamal_ciphertext_t ciphertext, bsgs_table_t table)
{
    EC_POINT *M;
    //uint64_t plaintext;
    BN_CTX *ctx = BN_CTX_new();

    M = multiply_constant(ciphertext->C1, keys->secret, init_group); //rB*sk1
    EC_POINT_add(init_group, ciphertext_update->C1, ciphertext_update->C1, M, ctx);
    // printf("Testdecrypt_follow OK\n");

    BN_CTX_free(ctx);
    EC_POINT_clear_free(M);

    //EC_POINT_clear_free(M3);
    return 0;
}



/*----added by Tham for Threshold Decryption by multiple servers ---------------------------------------*/
int gamal_coll_decrypt(dig_t *res, gamal_key_t keys1, gamal_key_t keys2, gamal_key_t keys3, gamal_ciphertext_t ciphertext, bsgs_table_t table)
{
    EC_POINT *M1, *M2, *M3;
    uint64_t plaintext;
    BN_CTX *ctx = BN_CTX_new();

    M1 = multiply_constant(ciphertext->C1, keys1->secret, init_group); //rB*sk1
    M2 = multiply_constant(ciphertext->C1, keys2->secret, init_group); //rB*sk2
    M3 = multiply_constant(ciphertext->C1, keys3->secret, init_group); //rB*sk3
    //EC_POINT_add(init_group, M2, ciphertext->C1, M1, ctx); //rB*k1 + rB*k2 = r(K1 + K2)
    EC_POINT_add(init_group, M2, M2, M1, ctx);             //rB*k1 + rB*k2 = r(K1 + K2)
    EC_POINT_add(init_group, M3, M3, M2, ctx);             //rB*k1 + rB*k2 + rB*k3= r(K1 + K2 + K3)
    EC_POINT_invert(init_group, M3, ctx);                  // -r(K1 + K2 + K3) = -rK
    EC_POINT_add(init_group, M3, ciphertext->C2, M3, ctx); // m + rK - r(K1+K2+K3) = m +rK - rK = m

    if (table != NULL)
    {
        solve_ecdlp_bsgs(table, &plaintext, M3, 1L << MAX_BITS);
    }
    else
    {
        solve_dlog_brute(init_group, M3, &plaintext, 1L << MAX_BITS);
    }
    *res = (dig_t)plaintext;

    BN_CTX_free(ctx);
    EC_POINT_clear_free(M1);
    EC_POINT_clear_free(M2);
    EC_POINT_clear_free(M3);
    return 0;
}



/** added by Tham 7 Dec 2019
 * This piece of code for re-encryption (key switching) from PK_Ss to PK_B
 * idea: one server will take lead the key switching
 * other servers will partially switching collective public key to target's public key
 * 
 */ 

int gama_key_switch_lead(gamal_ciphertext_t cipher_update, gamal_ciphertext_t cipher, gamal_key_t keys_lead, gamal_key_t keysNew)
{
    
    BIGNUM *ord, *rand1;
    BN_CTX *ctx = BN_CTX_new();
    EC_POINT *M1, *temp1; 
    gamal_ciphertext_t temp;

    ord = BN_new();
    rand1 = BN_new();
    cipher_update->C1 = EC_POINT_new(init_group); 
    cipher_update->C2 = EC_POINT_new(init_group);
    temp->C1 = EC_POINT_new(init_group);


    temp->C1 = multiply_generator(rand1, init_group);
    EC_POINT_add(init_group, cipher_update->C1, cipher_update->C1, temp->C1, ctx); //v_1*B
    
    M1 = multiply_constant(cipher->C1, keys_lead->secret, init_group); //rB*k_1
    
    EC_POINT_invert(init_group, M1, ctx); // -rB*k_1
   
    EC_POINT_add(init_group, cipher_update->C2, cipher->C2, M1, ctx); //C2 - rB*k_1
    
    temp1 = multiply_constant(keysNew->Y, rand1, init_group); //v_1 * Knew
   
    EC_POINT_add(init_group, cipher_update->C2, cipher_update->C2, temp1, ctx); //c2 - rB*k_1 + v_1 * K_new
    

    BN_clear_free(rand1);
    BN_free(ord);
    BN_CTX_free(ctx);
    EC_POINT_clear_free(M1);
    
    EC_POINT_clear_free(temp1);
    
    return 0;
}
//added by Tham 7 Dec 2019
int gama_key_switch_follow(gamal_ciphertext_t cipher_update, gamal_ciphertext_t cipher, gamal_key_t keys_follow, gamal_key_t keysNew)
{
    BIGNUM *ord,  *rand2;
    BN_CTX *ctx = BN_CTX_new();
    EC_POINT *M2, *temp2; 
    gamal_ciphertext_t temp;

    ord = BN_new();
   
    rand2 = BN_new();
    temp->C1 = EC_POINT_new(init_group);

    temp->C1 = multiply_generator(rand2, init_group);
    EC_POINT_add(init_group, cipher_update->C1, cipher_update->C1, temp->C1, ctx); //v_1*B + v_2*B
    
    M2 = multiply_constant(cipher->C1, keys_follow->secret, init_group); //rB*k_2
    
    EC_POINT_invert(init_group, M2, ctx); // -rB*k_2
   
    EC_POINT_add(init_group, cipher_update->C2, cipher_update->C2, M2, ctx); //C2 - rB*k_1 + v_1*B - rB*k_2
   
    temp2 = multiply_constant(keysNew->Y, rand2, init_group);             //v_2 * Knew
    EC_POINT_add(init_group, cipher_update->C2, cipher_update->C2, temp2, ctx); //C2 - rB*k_1 + v_1*Knew - rB*k_2 + v_2*Knew = x + (v_1 + v_2)*Knew
       
    BN_clear_free(rand2);
    BN_free(ord);
    BN_CTX_free(ctx);
    EC_POINT_clear_free(M2);
    EC_POINT_clear_free(temp2);
    return 0;
}



/**
 * added by Tham for re-encrypt a ciphertext by a new public key, 
 * switching from collective public key of 3 servers
 * Need to improve, adapt to many servers
 */

int gamal_key_switching(gamal_ciphertext_t new_cipher, gamal_ciphertext_t cipher, gamal_key_t keys1, gamal_key_t keys2, gamal_key_t keys3, gamal_key_t keysNew)
{
    // (C1, C2) = (r*B, x + r*(K1 + K2)) => (C1, C2) = (v*B, x + v*Knew)
    //K1 = k_1*B; K2 = k_2*B
    //Knew = k_new*B
    //printf("key switch Test begin\n");
    BIGNUM *ord, *rand1, *rand2, *rand3;
    BN_CTX *ctx = BN_CTX_new();
    //gamal_ciphertext_t temp;
    EC_POINT *M1, *M2, *M3, *temp1, *temp2, *temp3; // *temp;
    gamal_ciphertext_t temp;

    //bn_plain = BN_new();
    ord = BN_new();
    rand1 = BN_new();
    rand2 = BN_new();
    rand3 = BN_new();
    new_cipher->C1 = EC_POINT_new(init_group); //nothing
    new_cipher->C2 = EC_POINT_new(init_group);
    temp->C1 = EC_POINT_new(init_group);

    //server 1 modifies the tuple:
    //printf("key switch Test OK\n");
    temp->C1 = multiply_generator(rand1, init_group);
    EC_POINT_add(init_group, new_cipher->C1, new_cipher->C1, temp->C1, ctx); //v_1*B
    //printf("key switch Test OK1\n");
    M1 = multiply_constant(cipher->C1, keys1->secret, init_group); //rB*k_1
    //printf("key switch Test OK2\n");
    EC_POINT_invert(init_group, M1, ctx); // -rB*k_1
    //printf("key switch Test OK3\n");
    EC_POINT_add(init_group, new_cipher->C2, cipher->C2, M1, ctx); //C2 - rB*k_1
    //printf("key switch Test OK4\n");
    temp1 = multiply_constant(keysNew->Y, rand1, init_group); //v_1 * Knew
    //printf("key switch Test OK5\n");
    EC_POINT_add(init_group, new_cipher->C2, new_cipher->C2, temp1, ctx); //c2 - rB*k_1 + v_1 * K_new
    //printf("key switch Test OK6\n");

    //server 2 modifies the tuple:
    temp->C1 = multiply_generator(rand2, init_group);
    EC_POINT_add(init_group, new_cipher->C1, new_cipher->C1, temp->C1, ctx); //v_1*B + v_2*B
    //printf("key switch Test OK7\n");
    M2 = multiply_constant(cipher->C1, keys2->secret, init_group); //rB*k_2
    //printf("key switch Test OK8\n");
    EC_POINT_invert(init_group, M2, ctx); // -rB*k_2
    //printf("key switch Test OK9\n");
    EC_POINT_add(init_group, new_cipher->C2, new_cipher->C2, M2, ctx); //C2 - rB*k_1 + v_1*B - rB*k_2
    //printf("key switch Test OK10\n");
    temp2 = multiply_constant(keysNew->Y, rand2, init_group);             //v_2 * Knew
    EC_POINT_add(init_group, new_cipher->C2, new_cipher->C2, temp2, ctx); //C2 - rB*k_1 + v_1*Knew - rB*k_2 + v_2*Knew = x + (v_1 + v_2)*Knew
    //printf("key switch Test OK11\n");

    //#if 0 // server 3 modifies the tuple
    temp->C1 = multiply_generator(rand3, init_group);
    EC_POINT_add(init_group, new_cipher->C1, new_cipher->C1, temp->C1, ctx); //v_1*B + v_2*B + v_3*B
    //printf("key switch Test OK7\n");
    M3 = multiply_constant(cipher->C1, keys3->secret, init_group); //rB*k_3
    //printf("key switch Test OK8\n");
    EC_POINT_invert(init_group, M3, ctx); // -rB*k_3
    //printf("key switch Test OK9\n");
    EC_POINT_add(init_group, new_cipher->C2, new_cipher->C2, M3, ctx); //C2 - rB*k_1 + v_1*B - rB*k_2 + v_2*Knew - rB*k_3
    //printf("key switch Test OK10\n");
    temp3 = multiply_constant(keysNew->Y, rand3, init_group);             //v_3 * Knew
    EC_POINT_add(init_group, new_cipher->C2, new_cipher->C2, temp3, ctx); //C2 - rB*k_1 + v_1*Knew - rB*k_2 + v_2*Knew - rB*k_3 + v3*Knew
    //printf("key switch Test OK11\n");
    //#endif
    BN_clear_free(rand1);
    BN_clear_free(rand2);
    BN_clear_free(rand3);
    BN_free(ord);
    BN_CTX_free(ctx);
    EC_POINT_clear_free(M1);
    EC_POINT_clear_free(M2);
    EC_POINT_clear_free(M3);
    EC_POINT_clear_free(temp1);
    EC_POINT_clear_free(temp2);
    EC_POINT_clear_free(temp3);
    //printf("key switch Test OK12\n");

    return 0;
}



/**
 * Added by Tham in 4 Feb 2020
 * Re-encrypt a ciphertext under a server's public key to a ciphertext under a new public key
 */
int gamal_re_encrypt(gamal_ciphertext_t new_cipher, gamal_ciphertext_t cipher, gamal_key_t keys, gamal_key_t keysNew)
{
    
    BIGNUM *ord, *rand1;
    BN_CTX *ctx = BN_CTX_new();
    //gamal_ciphertext_t temp;
    EC_POINT *M1, *temp1; // *temp;
    gamal_ciphertext_t temp;

    //bn_plain = BN_new();
    ord = BN_new();
    rand1 = BN_new();
    
    new_cipher->C1 = EC_POINT_new(init_group); //nothing
    new_cipher->C2 = EC_POINT_new(init_group);
    temp->C1 = EC_POINT_new(init_group);

    //printf("key switch Test OK\n");
    temp->C1 = multiply_generator(rand1, init_group);
    EC_POINT_add(init_group, new_cipher->C1, new_cipher->C1, temp->C1, ctx); //v_1*B = C1_new
    //printf("key switch Test OK1\n");
    M1 = multiply_constant(cipher->C1, keys->secret, init_group); //rB*k_1
    //printf("key switch Test OK2\n");
    EC_POINT_invert(init_group, M1, ctx); // -rB*k_1
    //printf("key switch Test OK3\n");
    EC_POINT_add(init_group, new_cipher->C2, cipher->C2, M1, ctx); //C2 - rB*k_1
    //printf("key switch Test OK4\n");
    temp1 = multiply_constant(keysNew->Y, rand1, init_group); //v_1 * Knew
    //printf("key switch Test OK5\n");
    EC_POINT_add(init_group, new_cipher->C2, new_cipher->C2, temp1, ctx); //C2 - rB*k_1 + v_1 * K_new = x + v1*K_new = C2_new
    //printf("key switch Test OK6\n");

    BN_clear_free(rand1);
    
    BN_free(ord);
    BN_CTX_free(ctx);
    EC_POINT_clear_free(M1);
    EC_POINT_clear_free(temp1);

    return 0;
}



// added by Nam: generate collective key with only public keys input
int gamal_collective_publickey_gen(gamal_key_t coll_keys, EC_POINT **p_key_list, int size_key_list)
{
    gamal_init(CURVE_256_SEC);

    BN_CTX *ctx = BN_CTX_new();
    BIGNUM *ord, *secret_key;

    ord = BN_new();
    // secret_key = BN_new(); //coll secret key
    coll_keys->Y = EC_POINT_new(init_group);
    EC_GROUP_get_order(init_group, ord, ctx);

    coll_keys->is_public = 0;

    EC_POINT *tmp;
    for (int i = 0; i < size_key_list; i++)
    {
        if (i == 0)
        {
            coll_keys->Y = p_key_list[i];
        }
        else
        {
            tmp = coll_keys->Y;
            EC_POINT_add(init_group, coll_keys->Y, tmp, p_key_list[i], ctx); //added up all public keys
        }
    }
    // coll_keys->secret = secret_key;

    BN_CTX_free(ctx);
    return 0;
}

//added by Tham for generate collective key => need to improve so that only public keys are the input of the function
int gamal_collective_key_gen(gamal_key_t coll_keys, gamal_key_t keys1, gamal_key_t keys2, gamal_key_t keys3)
{
    //gamal_key_t gamal_collective_key_gen(int party) {
    //party = 2;
    //gamal_key_t coll_keys;
    //gamal_key_t key1, key2, key3;//, key3;

    gamal_init(CURVE_256_SEC);
    //gamal_generate_keys(key1);
    //gamal_generate_keys(key2);
    //gamal_generate_keys(key3);

    BN_CTX *ctx = BN_CTX_new();
    BIGNUM *ord, *coll_key;

    ord = BN_new();
    //coll_key = BN_new(); //coll secret key
    coll_keys->Y = EC_POINT_new(init_group);
    EC_GROUP_get_order(init_group, ord, ctx);

    coll_keys->is_public = 0;

    //EC_POINT_add(const EC_GROUP *group, EC_POINT *r, const EC_POINT *a, const EC_POINT *b, BN_CTX *ctx); //in ec.h

    EC_POINT_add(init_group, coll_keys->Y, keys2->Y, keys1->Y, ctx);     //added up all public keys
    EC_POINT_add(init_group, coll_keys->Y, coll_keys->Y, keys3->Y, ctx); //added up all public keys

    //int BN_add(BIGNUM *r, const BIGNUM *a, const BIGNUM *b); //in bn.h

    //BN_add(coll_key, keys1->secret, keys2->secret);
    //coll_keys->secret = coll_key;
    //BN_add(coll_key, coll_keys->secret, key3->secret);
    //coll_keys->secret = coll_key;

    //BN_free(ord);
    BN_CTX_free(ctx);
    return 0;
}

//Suportive functions
// Added by Tham to convert an integer to its binary representation
int *convert_to_bin(int binary_arr[64], dig_t number, int bit_num)
{
    //dig_t binary = 0, counter = 0;
    //double x;
    //int bit_num;
    //bit_num = 32;
    //number = 100;
    // x = log2(number);
    //printf("%f\n", x);
    // bit_num = ceil(x);
    //printf("%d\n", bit_num);

    //int binary_arr[bit_num];
    //binary_arr[64]={0};
    int remainder, counter = 0;
    //int binary_arr[32];

    while (number > 0)
    {
        remainder = number % 2;
        number /= 2;
        //binary += pow(10, counter) * remainder;
        binary_arr[counter] = remainder;
        counter++;
    }

    //for(int it=counter; it<= bit_num - 1; it++){
    //	binary_arr[it] = 0;
    // }

    //binary_arr[counter] = 1;
    //#if 0
    // for(int it=0; it<=bit_num - 1; it++){
    //binary_arr_correct[it] = binary_arr[bit_num - 1 - it];
    //printf("%d", binary_arr[it]);
    // }
    // printf("\n");
    //#endif    //return 0;

    return binary_arr;
}

/**Added by Tham, function to decode an integer to the form of non-adjacent form,
 * to reduce the number of point addition in scalar multiplication
 * paras
 */
int *convert_to_NAF(int naf_arr[64], dig_t number, int bit_num[2])
{

    int i;
    while (number > 0)
    {
        if ((number % 2) == 1)
        {
            naf_arr[i] = mods_func(number); //number mods 2^2
            number = number - naf_arr[i];
        }
        else
            naf_arr[i] = 0;
        number = number / 2;
        i++;
    }
    bit_num[0] = i - 1;
    //int res_naf_arr[i];
    //for(int it=0; it<=i; it++){
    //  	res_naf_arr[i] = naf_arr[i];
    //  }

    //for(int it=bit_num[0]; it>=0; it--){
    //binary_arr_correct[it] = binary_arr[bit_num - 1 - it];
    // printf("%d", naf_arr[it]);
    // }
    //  printf("\n");

    //return naf_arr;
    return bit_num;
}

/**
 * added by Tham, compute the "mods" function
 */
int mods_func(int number)
{
    if ((number % 4) >= 2)
        return (number % 4) - 4; //(number mode 2^2) - 2^2
    else
        return (number % 4);
}

