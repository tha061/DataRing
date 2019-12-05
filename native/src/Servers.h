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
    dig_t _fusionDecrypt(gamal_ciphertext_t ciphertext, bsgs_table_t table, int serverId);
    void fusionDecrypt(ENC_DOMAIN_MAP enc_domain_map, bsgs_table_t table);
    void createServersEncrypVector(ENC_Stack &pre_enc_stack);
    bool verificationPV(ENC_DOMAIN_MAP enc_domain_map, bsgs_table_t table, int serverId, ENC_Stack &pre_enc_stack);
    bool verificationTestResult(string testName, gamal_ciphertext_t sum_cipher, bsgs_table_t table, int serverId, int threshold);
};

#endif