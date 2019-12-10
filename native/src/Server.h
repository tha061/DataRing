#ifndef SERVER
#define SERVER

#include "../include/public_header.h"
#include "./process_noise.h"

class Server
{
public:
    // static gamal_key_t coll_key;
    gamal_key_t key;
    gamal_ciphertext_t *myPIR_enc;
    id_domain_set known_vector;
    ENC_DOMAIN_MAP enc_test_map;
    id_domain_set verified_set;

    hash_pair_map plain_domain_map;

    int *plain_track_list;
    int size_dataset;

    Server();
    Server(int size, string known_domain_dir);
    void createRandomEncrypVector(ENC_Stack &pre_enc_stack);
    void importFile(string file_url);

    
    // this function is to determine mininum number of known rows needs to be found in PV
    float generatePVTestCondition(int dataset, int PV, int known_records, double eta);

    //RENAME AND UPDATE FUNCTION
    /**
     * This function is to generate test function, there are many options
     * if indicate 1: test for L known rows
     * indicate 2: test for V rows in PV
     * indicate 3: test for (V - r0)
     * indicate 4: test for arbitrary attributes
     */ 
    
    // test func() L domains
    void generateTestHashMap_1(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map); // test the existence of L known domains
    
    // test func() V domains
    void generateTestHashMap_2(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map); // test the existence of V domains

    // test func() V - r0 domains
    void generateTestHashMap_3(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map); // test the existence of V domains

    // test func() specific attributes
    void generateTestHashMap_Attr(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map); 


    void generateTestFunction(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map, int type);

    void save_knownRow_found_in_PV(id_domain_pair verified_domain_pair);

    void generateNormalQuery(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map);

    /**
     * this function is to get test result from the encrypted PV
     * paras test function is a vector of 1s and 0s in clear
     * paras PV is of size a*n and be encrypted
     * output from this function is a encrypted result
     * servers use this decrypted result as the input of function estimate_conf_interval()
     */ 
    void getTestResult_fromPV(ENC_DOMAIN_MAP enc_domain_map, gamal_ciphertext_t enc_PV_answer); 
};

#endif