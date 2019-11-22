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
    
    int *plain_track_list;
    int size_dataset;

    Server();
    Server(int size);
    void createRandomEncrypVector(ENC_Stack &pre_enc_stack);
    void importFile(string file_url);
    bool verificationPV(ENC_DOMAIN_MAP &enc_domain_map);
    // static void setCollKey(gamal_key_t new_coll_key);
    // void decryptEncMap(ENC_DOMAIN_MAP enc_domain_map);
};

#endif