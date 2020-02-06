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
    id_domain_set known_record_subset;
    ENC_DOMAIN_MAP enc_test_map;
    // ENC_DOMAIN_MAP enc_test_map_pre, enc_test_map_tmp; // for preprare a test funtion
    ENC_DOMAIN_MAP enc_test_map_pre; // for preprare a test funtion
    id_domain_set verified_set;

    hash_pair_map plain_domain_map;
    id_domain_vector match_query_domain_vect;    

    int *plain_track_list;
    int size_dataset;

    Server();
    Server(int size, string known_domain_dir);
    void createRandomEncrypVector(ENC_Stack &pre_enc_stack);
    void importFile(string file_url);

    // function is resposiblle for find matching domain from query
    void generateMatchDomain(bool test_or_query);

    
    // this function is to determine mininum number of known rows needs to be found in PV
    float generatePVTestCondition(int dataset, int PV, int known_records, double eta);

    /** added by Tham 14 Dec 19
     * pre-compute a test function
     * This function pre-prepare a vector for a test function, each element in the vector
     * is a domain and its value is an enc(0)
     * This is pre-computed so that when the server wishes to generate a test,
     * it only needs to find the element where the domain is satisfied the condition
     * of the test and changes it from enc(0) to enc(1)
     */  

    void prepareTestFuntion_Query_Vector(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map);

    // generate Test Function targeting known records in the prior knowledge set
    // number of known records is from [1 .. L] records
    void generateTestKnownRecords_opt(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map);
  
    
    
    
    // test func() L domains
    void generateTestKnownRecords(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map); // test the existence of L known domains
    // test func() V domains
    void generateTestBasedPartialView(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map); // test the existence of V domains
    void generateTestBasedPartialView_opt(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map);// added by Tham 16Dec
    // test func() V - r0 domains
    void generateTestHashMap_3(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map); // test the existence of V domains

    // test func() specific attributes
    //TO DO: separate the mapping domains satisfied the query to a different function
    // this function is only put enc(1) to satisified domains, enc(0) to other domains
    // remove the clear test function part from this function, write a separate function for generating the
    // clear test function
    void generateTest_Target_Attr(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map); 

    void generateTest_Target_Attr_opt(ENC_Stack &pre_enc_stack); 

    void generateServerDomain_Test_Target_Attr(ENC_Stack &pre_enc_stack);

    // added by Tham 29 Jan 2020, to check for all rows in the histogram, expected answer is n+noise
    void generateTest_Target_All_Records(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map);

    /**
     * This function is to generate test function, there are many options
     * if indicate 1: test for L known rows
     * indicate 2: test for V rows in PV
     * indicate 3: test for (V - r0)
     * indicate 4: test for arbitrary attributes
     */ 

    void generateTestFunction(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map, int type);

    void save_knownRow_found_in_PV(id_domain_pair verified_domain_pair);


    //added by Tham in 14 Dec 19 to optimize runtime
    void generateNormalQuery_opt(ENC_Stack &pre_enc_stack); 

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