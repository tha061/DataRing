#ifndef SERVER
#define SERVER

#include "../include/public_header.h"

class Server
{
public:
    // static gamal_key_t coll_key;
    gamal_key_t key;
    gamal_ciphertext_t *myPIR_enc;
    id_domain_vector known_vector;
    ENC_DOMAIN_MAP enc_test_map;
    ENC_DOMAIN_MAP enc_test_map_2;

    int *plain_track_list;
    int size_dataset;

    Server();
    Server(int size);
    void createRandomEncrypVector(ENC_Stack &pre_enc_stack);
    void importFile(string file_url);
    void generateTestHashMap_1(ENC_Stack &pre_enc_stack); // test the existence of L known domains
    void generateTestHashMap_2(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map); // test the existence of V domains

    // bool verificationPV(ENC_DOMAIN_MAP &enc_domain_map, bsgs_table_t table);
    // static void setCollKey(gamal_key_t new_coll_key);
    // void decryptEncMap(ENC_DOMAIN_MAP enc_domain_map);
};

#endif