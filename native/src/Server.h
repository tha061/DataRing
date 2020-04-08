#ifndef SERVER
#define SERVER

#include "../include/public_header.h"
#include "./process_noise.h"

class Server
{
public:
    // static gamal_key_t coll_key;
    gamal_key_t key; // key pair of one server
    gamal_ciphertext_t *myPIR_enc; //not used any where
    id_domain_set known_record_subset, known_rows_after_phase2, verified_set, opened_rows_set, known_record_set, rows_set_in_opened_PV, known_rows_in_pv;
    ENC_DOMAIN_MAP enc_question_map; //encrypted question done by one of the servers
    // ENC_DOMAIN_MAP enc_question_map_pre, enc_question_map_tmp; // for preprare a test funtion
    ENC_DOMAIN_MAP enc_question_map_pre; // for preprare an encrypted question for performance improvement
    // id_domain_set verified_set, known_rows_after_phase2;

    hash_pair_map plain_domain_map; //used for tracking bug

    id_domain_vector match_query_domain_vect; // matching actual query to a query vector    

    int *plain_track_list; //used for tracking bug
    int size_dataset; // participant's dataset size

    int index_attr_sum; //specify which attribute to be sum up: for a sum query

    Server(); //create a server

/**
 * This function initial a server
 * @para: size of the dataset
 * @para: known_domain_dir: directory to background knowledge shared by all servers
*/    
    Server(int size, string known_domain_dir);


/**
 * This function imports csv files from the file_url directory
 * @para: link to the file of background knowledge
 * @return: a set of records of the participan't dataset known by all servers
*/
    void importFile(string file_url);

/**
 * function is resposiblle for find matching domain
 * to reduce runtime
 * @para: test_or_query: to specify matching condition for a test or a query
 * 1: test --> go to the test matching condition file
 * 0: query --> go to the query matching condiftion file
*/
    
    void generateMatchDomain(bool test_or_query);


/**
 * this function is to determine mininum number of known rows needs to be found in PV
 * @para: dataset: size of the dataset N
 * @para: int PV: size of the partial view
 * @para: known_records: number of known records in background knowledge: L
 * @para: 1-eta: the tolerated false positive rate
*/    
    float generatePVTestCondition(int dataset, int PV, int known_records, double eta);

/** added by Tham 14 Dec 19
 * pre-compute a test function or a query function
 * This function pre-prepare a vector for a test function/query function, each element in the vector
 * is a domain and its value is an enc(0)
 * This is pre-computed so that when the server wishes to generate a test,
 * it only needs to find the element where the domain is satisfied the condition
 * of the test and changes it from enc(0) to enc(1)
 * @para: &pre_enc_stack: point to the precomputed encryption stack
 * @para: enc_domain_maP: partial view sent by the participant to get the labels
 * @return: a precomputed test function/query function with all enc(0)
 */  

    void prepareTestFuntion_Query_Vector(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map);

/**
 * This function generates the test that counts all records in the dataset that are known by the servers
 * The expected answer is L+/- noise
 * This function modifed the pre-computed test function for reduce runtime
 * @para: &pre_enc_stack: point to precomputed encryption stack of the serve
 * @para: enc_domain_map: partial view for server to get labels
 * @return: an encrypted vector of enc(0) and enc(1), labels matched with partial view's labels
 * enc(1) is placed to element with labels matched the L known records
 * enc(0) is placed to all other elements
*/
    void generateTestKnownRecords_opt(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map);

    
/**
 * This function generates the test that counts all records in the dataset that are known by the servers
 * The expected answer is L+/- noise
 * @para: &pre_enc_stack: point to precomputed encryption stack of the serve
 * @para: enc_domain_map: partial view for server to get labels
 * @return: an encrypted vector of enc(0) and enc(1), labels matched with partial view's labels
 * enc(1) is placed to element with labels matched the L known records
 * enc(0) is placed to all other elements
*/
    void generateTestKnownRecords(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map); // test the existence of L known domains


/**
 * This function generates the test that counts all records in the dataset that are sampled in the partial view
 * The expected answer is V+/- noise
 * @para: &pre_enc_stack: point to precomputed encryption stack of the serve
 * @para: enc_domain_map: using the partial view with both labels and encryptions
 * server adds enc(0) to all encryptions in partial view to re-randomise them so participant cannot recognised ciphertexts in its submitted PV
 * @return: an encrypted vector of enc(0) and enc(1), labels matched with partial view's labels
 * enc(1) becomes a new ciphertext of 1
 * enc(0) becomes a new ciphertext of 0
*/
    void generateTestBasedPartialView(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map); // test the existence of V domains

/**
 * This function generates the test that counts all records in the dataset that are sampled in the partial view
 * The expected answer is V+/- noise
 * This function only re-randomises some of encryptions in the partial view map to reduce runtime
 * @para: &pre_enc_stack: point to precomputed encryption stack of the serve
 * @para: enc_domain_map: using the partial view with both labels and encryptions
 * server adds enc(0) to some encryptions in partial view to re-randomise them so participant cannot recognised ciphertexts in its submitted PV
 * @return: an encrypted vector of enc(0) and enc(1), labels matched with partial view's labels
 * enc(1) becomes a new ciphertext of 1
 * enc(0) becomes a new ciphertext of 0
*/

    void generateTestBasedPartialView_opt(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map);// added by Tham 16Dec


/**
 * This function generates the test that counts all records in the dataset that have attributes satisfied given values
 * @para: &pre_enc_stack: point to precomputed encryption stack of the serve
 * server matches all records that have the attributes sastifing given values to labels in the pre-computed test function using a matching function inside it
 * Then server add enc(1) to matched labels and enc(0) to other non matched labels
 * @return: an encrypted vector of enc(0) and enc(1), labels matched with partial view's labels
*/  

    void generateTest_Target_Attr(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map); 

/**
 * This function generates the test that counts all records in the dataset that have attributes satisfied given values
 * This function modified a pre-computed test function and a matching domain function to reduce runtime
 * @para: &pre_enc_stack: point to precomputed encryption stack of the serve
 * server matches all records that have the attributes sastifing given values to labels in the pre-computed test function
 * Then server add enc(1) to matched labels, enc(0) are remained as it in the pre-computed test version
 * @return: an encrypted vector of enc(0) and enc(1), labels matched with partial view's labels
*/

    void generateTest_Target_Attr_opt(ENC_Stack &pre_enc_stack); 

/**
 * This function generates clear test function to compute answer from encrypted partial view,
 * this test count all records in partial view that satify some condition
 * server matches the labels of condition with the label in the partial view
 * @para: &pre_enc_stack: point to precomputed encryption stack of the server
 * server matches all records that have the attributes sastifing given values using a matching function
 * Then server place 1 to matched labels, 0 to non matched labels
 * @return: an clear vector of 0 and 1, labels matched with partial view's labels
*/    
    
    void generateServerDomain_Test_Target_Attr(ENC_Stack &pre_enc_stack); //not using pre_enc_stack at all

/**
 * This function generates the test that counts all records in the dataset 
 * The expected answer is N+/- noise
 * @para: &pre_enc_stack: point to precomputed encryption stack of the serve
 * @para: enc_domain_map: partial view for server to get all labels
 * @return: an encrypted vector of all enc(1), labels matched with partial view's labels
*/

    
    void generateTest_Target_All_Records(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map);


/**
 * //added by Tham in 14 Dec 19 to optimize runtime
 * This function generates encrypted query that counts all records in the dataset that match the query selection
  This function modified a pre-computed query function and a matching domain function to reduce runtime
 * @para: &pre_enc_stack: point to precomputed encryption stack of the serve
 * server matches all records that have the attributes sastifing given values to labels in the pre-computed query function
 * Then server add enc(1) to matched labels, enc(0) are remained as it in the pre-computed query version
 * @return: an encrypted vector of enc(0) and enc(1), labels matched with partial view's labels
*/
    
    void generateNormalQuery_opt(ENC_Stack &pre_enc_stack); 

/**
 * This function generates encrypted query that counts all records in the dataset that match the query selection and then sum their attribute's value
 * the answer is a sum of attribute's values of those matched records
  This function modified a pre-computed query function and a matching domain function to reduce runtime
 * @para: &pre_enc_stack: point to precomputed encryption stack of the serve
 * server matches all records that have the attributes sastifing given values to labels in the pre-computed query function
 * Then server add enc(1) to matched labels, enc(0) are remained as it in the pre-computed query version
 * @return: an encrypted vector of enc(0) and enc(1), labels matched with partial view's labels
 * @rerurn: and also the index of attribute to be sum up
*/

    int generateNormalQuery_sum(ENC_Stack &pre_enc_stack, int index_attr_to_sum);

/**
 * This function generates encrypted query that counts all records in the dataset that match the query selection
 * @para: &pre_enc_stack: point to precomputed encryption stack of the server
 * @para: enc_domain_map: partial view for servers to get all labels
 * server matches all records that have the attributes sastifing given values using a matching function
 * Then server add enc(1) to matched labels, enc(0) to non matched labels
 * @return: an encrypted vector of enc(0) and enc(1), labels matched with partial view's labels
*/    

    void generateNormalQuery(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map);

/**
 * This function generates clear query to compute answer from encrypted partial view, in case a cheating party is detected
 * @para: &pre_enc_stack: point to precomputed encryption stack of the server
 * @para: enc_domain_map: partial view for servers to get all labels
 * server matches all records that have the attributes sastifing given values using a matching function
 * Then server add 1 to matched labels, 0 to non matched labels
 * @return: an clear vector of 0 and 1, labels matched with partial view's labels
*/    

    void generateNormalQuery_Clear(ENC_Stack &pre_enc_stack); //Added by Tham 17 Feb 2020 for releasing phase



/**
 * This function is to get test result from the encrypted PV
 * servers then use this decrypted result as the input of function estimate_conf_interval()
 * @para: enc_domain_map: encrypted partial view
 * @para: enc_PV_answer: to store the encrypted answer 
 * @return: an encrypted result from the partial view for the test
 */ 
    void getTestResult_fromPV(ENC_DOMAIN_MAP enc_domain_map, gamal_ciphertext_t enc_PV_answer); 


/**
 * This function is to get quer result from the encrypted PV in case a cheating party is detected
 * servers then use this decrypted result as the input of function estimate_conf_interval()
 * @para: enc_domain_map: encrypted partial view
 * @para: enc_PV_answer: to store the encrypted answer 
 * @return: an encrypted result from the partial view for the query
 */ 
    void getQueryResult_fromPV(ENC_DOMAIN_MAP enc_domain_map, gamal_ciphertext_t enc_PV_query_answer); 



//===================== Supportive functions or unused functions ====================================================//

/**
 * This function is to generate encrypted test function, there are many options
 * if indicate 1: test for L known rows
 * indicate 2: test for V rows in PV
 * indicate 3: test for (V - r0)
 * indicate 4: test for arbitrary attributes
 */ 

    void generateTestFunction(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map, int type);

    void save_knownRow_found_in_PV(id_domain_pair verified_domain_pair);
    //Tham
    void save_knownRow_after_phase2(id_domain_pair domain_pair);
    //Tham
    void save_opened_rows(id_domain_pair opened_rows);


    // test func() V - r0 domains
    void generateTestHashMap_3(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map); // test the existence of V domains


    //Tham
    void generateTest_known_records_after_phase2(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map, id_domain_set known_rows_after_phase2);

    //Tham Mar 2020
    void generateTest_PV_r0(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map, id_domain_set known_rows_in_pv);
    void generateTest_PV_L_r0(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map, id_domain_set known_rows_in_pv, id_domain_set known_record_subset);

/**
 * This function for a server to create a binary vector of V 0s and N-V 1s
 * and encrypt them using precomputed encryption stack
*/
    void createRandomEncrypVector(ENC_Stack &pre_enc_stack);

};

#endif