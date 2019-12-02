#ifndef SERVER
#define SERVER

#include "../include/public_header.h"

class Server
{
public:
    // static gamal_key_t coll_key;
    gamal_key_t key;
    gamal_ciphertext_t *myPIR_enc;
    id_domain_set known_vector;
    ENC_DOMAIN_MAP enc_test_map;
    id_domain_set verified_set;

    int *plain_track_list;
    int size_dataset;

    Server();
    Server(int size);
    void createRandomEncrypVector(ENC_Stack &pre_enc_stack);
    void importFile(string file_url);
    // test func() L domains
    void generateTestHashMap_1(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map); // test the existence of L known domains
    
    // test func() V domains
    void generateTestHashMap_2(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map); // test the existence of V domains

    // test func() specific attributes
    void generateTestHashMap_Attr(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map, map<int, string> cols); 

    // test func() V - r0 domains
    void generateTestHashMap_3(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map); // test the existence of V domains

    void addVerifiedDomain(id_domain_pair verified_domain_pair);
};

#endif