#ifndef SERVERS
#define SERVERS

#include "../include/public_header.h"
#include "Server.h"

class Servers
{
public:
    gamal_key_t coll_key;
    vector<Server> server_vect;
    int server_size, data_size;

    gamal_ciphertext_t * s_myPIR_enc;
    int * s_plain_track_list;

    // Servers();
    Servers(int server_size, int data_size);
    void generateCollKey();
    void fusionDecrypt(ENC_DOMAIN_MAP enc_domain_map, bsgs_table_t table);
    void createServersEncrypVector(ENC_Stack &pre_enc_stack);
};

#endif