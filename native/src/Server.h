#ifndef SERVER
#define SERVER

#include "../include/public_header.h"
#include "./process_noise.h"



/** 
 * @brief This class provides functionalities of a single server in the Data Ring system.
 * @details Functions include form an encrypted question from a query or a test, compute a answer for a test function from the encrypted partial view.
 * @author Tham Nguyen tham.nguyen@mq.edu.au, Nam Bui, Data Ring
 * @date April 2020
*/


class Server
{
public:
    // static gamal_key_t coll_key;
    /// A key pair
    gamal_key_t key; 
    // //// A key pair of one server used in Partial View Phase
    // gamal_key_t key_PV_phase; 
    // //// A key pair of one server used in Query Evaluation Phase
    // gamal_key_t key_query_phase; 
    /// The participant's dataset size
    int size_dataset; 
    /// ratio of partial view to dataset size
    double pv_ratio; 
    /// Encrypted query/test function
    ENC_DOMAIN_MAP enc_question_map;
    /// Precomputed encrypted query/test function for performance improvement
    ENC_DOMAIN_MAP enc_question_map_pre; 

    /// Sample vector in clear
    int *server_sample_vector_clear; 

    /// Sample Vector encrypted
    gamal_ciphertext_t *server_sample_vector_encrypted; 

    /// Partial view sampled from permuted histogram
    ENC_DOMAIN_MAP PV_sample_from_permuted_map;

    /// Encrypted PV from the participant to be shared to all servers
    ENC_DOMAIN_MAP un_permute_PV;

    /// Indicating which attribute to be summed in a sum query
    int index_attr_sum; 

    /// Subset of records included in the servers' backround knowledge
    id_domain_set known_record_subset;

    /// Set of labels in clear of the histogram
    hash_pair_map plain_domain_map; 

    /// Vector of matching records from SQL query to query formulation
    id_domain_vector match_query_domain_vect; 
     
    // ENC_DOMAIN_MAP enc_question_map_pre, enc_question_map_tmp; // for preprare a test funtion
    
    // id_domain_set verified_set, known_rows_after_phase2;

    // hash_pair_map plain_domain_map; 

    // id_domain_vector match_query_domain_vect; // matching actual query to a query vector    

    // int *plain_track_list; //used for tracking bug

    
/**
 * @brief Default constructor.
*/
    Server(); 

/**
 * @brief Initiate a server
 * @param size dataset's size
 * @param known_domain_dir Directory to background knowledge shared by all servers
*/    
    Server(int size, string known_domain_dir);


/**
 * @brief Import .csv files from the file_url directory
 * @param file_url link to the file of background knowledge
 * @return a set of records of the participan't dataset known by all servers
*/
    void importFile(string file_url);

/**
 * @brief Finding matching labels to form a query
 * @param test_or_query specify matching condition for a test or a query
 * 1: test --> go to the test matching condition file
 * 0: query --> go to the query matching condiftion file
*/
    
    void generateMatchDomain(bool test_or_query);

/**
 * @brief One server generate the sampling vector and encrypt it
 * @param pre_enc_stack: pre-computed stack of enc(1) and enc(0)
 * @param pv_ratio: ratio of partial view size to dataset size
 * @param data_size: dataset size
 * @return server_sample_vector_encrypted: an encrypted sampling vector
*/
    // void createPVsamplingVector(ENC_Stack &pre_enc_stack, double pv_ratio, int data_size);
    void createEncryptedPVSamplingVector(ENC_Stack &pre_enc_stack, int* server_sample_vector_clear);

/**
 * @brief One server applies the encrypted sampling vector to PV from the permuted histogram map
 * @param permuted_map_from_participant: ratio of partial view size to dataset size
 * @param server_sample_vector_encrypted: an encrypted sampling vector
 * @param pre_enc_stack: pre-computed stack of enc(1) and enc(0)
 * @return PV_sample_from_permuted_map: an encrypted PV from the permuted histogram
*/
    void generatePVfromPermutedHistogram(hash_pair_map permuted_map_from_participant, 
                                             gamal_ciphertext_t *server_sample_vector_encrypted, ENC_Stack &pre_enc_stack);


/**
 * @brief Another server rerandomize the encrypted PV to prevent the server generated this PV link to the original label order
 * @param PV_sample_from_permuted_map: the PV to be re-rendomised
 * @param pre_enc_stack: pre-computed stack of enc(1) and enc(0)
 * @return PV_sample_from_permuted_map and rerandomized.
*/
    void rerandomizePVSampleFromPermutedHistogram(ENC_DOMAIN_MAP PV_sample_from_permuted_map, ENC_Stack &pre_enc_stack);


/**
 * @brief A server un-permute the permuted PV to get the PV with orginal order of the label
 * @param PV_sample_from_permuted_map: the PV to be un-permuted
 * @param vector_un_permute_sort: the vector for un-permutation sent by participant
 * @return un_permute_PV: the resulted PV
*/
    void getUnPermutePV(ENC_DOMAIN_MAP PV_sample_from_permuted_map, vector<string> vector_un_permute_sort);
/**
 * @brief Determine mininum number of known rows needs to be found in PV
 * @param dataset: size of the dataset N
 * @param int PV: size of the partial view
 * @param known_records: number of known records in background knowledge: L
 * @param $1-\eta$ the tolerated false positive rate
*/    
    float generatePVTestCondition(int dataset, int PV, int known_records, double eta);

/** 
 * @brief Pre-compute a test function or a query function
 * @details This function pre-prepare a vector for a test function/query function, each element in the vector
 * is a domain and its value is an enc(0).
 * This is pre-computed so that when the server wishes to generate a test,
 * it only needs to find the element where the domain is satisfied the condition
 * of the test and changes it from enc(0) to enc(1)
 * @param pre_enc_stack: point to the precomputed encryption stack
 * @param enc_domain_map: partial view for server to get labels
 * @return a precomputed test function/query function with all enc(0)
 */  

    void prepareTestFuntion_Query_Vector(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map);

/**
 * @brief Generate the test that counts all records in the dataset that are known by the servers
 * @details This function modifed the pre-computed test function for reduce runtime. The expected answer is L+/- noise
 * @param pre_enc_stack: point to precomputed encryption stack of the serve
 * @param enc_domain_map: set of labels in enc partial view 
 * @return an encrypted vector of enc(0) and enc(1), labels matched with partial view's labels
 * enc(1) is placed to element with labels matched the L known records
 * enc(0) is placed to all other elements
*/
    void generateTestKnownRecords_opt(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map);

    

/**
 * @brief Generates the test that counts all records in the dataset that are sampled in the partial view
 * @details The expected answer is V+/- noise
 * This function only re-randomises some of encryptions in the partial view map to reduce runtime
 * @param pre_enc_stack: point to precomputed encryption stack of the serve
 * @param enc_domain_map: set of labels in enc partial view 
 * @return an encrypted vector of enc(0) and enc(1), labels matched with partial view's labels
 * enc(1) becomes a new ciphertext of 1
 * enc(0) becomes a new ciphertext of 0
*/

    void generateTestBasedPartialView_opt(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map);// added by Tham 16Dec



/**
 * @brief Generates the test that counts all records in the dataset that have attributes satisfied given values
 * @details This function modified a pre-computed test function and a matching domain function to reduce runtime
 * server matches all records that have the attributes sastifing given values to labels in the pre-computed test function
 * Then server add enc(1) to matched labels, enc(0) are remained as it in the pre-computed test version
 * @param pre_enc_stack: point to precomputed encryption stack of the serve
 * @return an encrypted vector of enc(0) and enc(1), labels matched with partial view's labels
*/

    void generateTest_Target_Attr_opt(ENC_Stack &pre_enc_stack); 

/**
 * @brief Generates clear test function to compute answer from encrypted partial view,
 * @details This test count all records in partial view that satify some condition
 * server matches the labels of condition with the label in the partial view
 * server matches all records that have the attributes sastifing given values using a matching function
 * Then server place 1 to matched labels, 0 to non matched labels
 * @param pre_enc_stack: point to precomputed encryption stack of the server
 * @return an clear vector of 0 and 1, labels matched with partial view's labels
*/    
    
    void generate_Test_Target_Attr_Clear(ENC_Stack &pre_enc_stack); //not using pre_enc_stack at all

/**
 * @brief Generates the test that counts all records in the dataset 
 * @details The expected answer is N+/- noise
 * @param pre_enc_stack: point to precomputed encryption stack of the serve
 * @param enc_domain_map: set of labels in enc partial view 
 * @return an encrypted vector of all enc(1), labels matched with partial view's labels
*/

    
    void generateTest_Target_All_Records(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map);


/**
 * @brief Generates encrypted query that counts all records in the dataset that match the query selection.
 * @details This function modified a pre-computed query function and a matching domain function to reduce runtime
 * server matches all records that have the attributes sastifing given values to labels in the pre-computed query function
 * Then server add enc(1) to matched labels, enc(0) are remained as it in the pre-computed query version
 * @param pre_enc_stack: point to precomputed encryption stack of the serve
 * @return an encrypted vector of enc(0) and enc(1), labels matched with partial view's labels
*/
    
    void generateNormalQuery_opt(ENC_Stack &pre_enc_stack); 

/**
 * @brief Generates encrypted query that counts all records in the dataset that match the query selection and then sum their attribute's value
 * @details The answer is a sum of attribute's values of those matched records. This function modified a pre-computed query function and a matching domain function to reduce runtime
 * The server matches all records that have the attributes sastifing given values to labels in the pre-computed query function
 * Then server add enc(1) to matched labels, enc(0) are remained as it in the pre-computed query version
 * @param pre_enc_stack: point to precomputed encryption stack of the serve
 * @return an encrypted vector of enc(0) and enc(1), labels matched with partial view's labels and also the index of attribute to be sum up
*/

    int generateNormalQuery_sum(ENC_Stack &pre_enc_stack, int index_attr_to_sum);



/**
 * @brief Generates clear query to compute answer from encrypted partial view, in case a cheating party is detected
 * @details The server matches all records that have the attributes sastifing given values using a matching function
 * Then server add 1 to matched labels, 0 to non matched labels
 * @param pre_enc_stack: point to precomputed encryption stack of the server
 * @return an clear vector of 0 and 1, labels matched with partial view's labels
*/    

    void generateNormalQuery_Clear(ENC_Stack &pre_enc_stack); //Added by Tham 17 Feb 2020 for releasing phase



/**
 * @brief Compute test result from the encrypted partial view
 * @details servers then use this decrypted result as the input of function estimate_conf_interval()
 * @param enc_domain_map: encrypted partial view
 * @param enc_PV_answer: to store the encrypted answer 
 * @return an encrypted result from the partial view for the test
 */ 
    void getTestResult_fromPV(ENC_DOMAIN_MAP enc_domain_map, gamal_ciphertext_t enc_PV_answer); 


/**
 * @brief This function is to get quer result from the encrypted PV in case a cheating party is detected
 * @details servers then use this decrypted result as the input of function estimate_conf_interval()
 * @param enc_PV: enc partial view 
 * @param enc_PV_answer: to store the encrypted answer 
 * @return an encrypted result from the partial view for the query
 */ 
    void getQueryResult_fromPV(ENC_DOMAIN_MAP enc_PV, gamal_ciphertext_t enc_PV_query_answer); 


//===============================================================================================================//
//===================== Supportive functions or unused functions ====================================================//


// /**
//  * @brief Generates the test that counts all records in the dataset that are known by the servers
//  * @details The expected answer is L+/- noise
//  * @param pre_enc_stack: point to precomputed encryption stack of the serve
//  * @param enc_domain_map: partial view for server to get labels
//  * @return an encrypted vector of enc(0) and enc(1), labels matched with partial view's labels
//  * enc(1) is placed to element with labels matched the L known records
//  * enc(0) is placed to all other elements
// */
    void generateTestKnownRecords(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map); // test the existence of L known domains


// /**
//  * @brief Generates the test that counts all records in the dataset that have attributes satisfied given values
//  * @details The server matches all records that have the attributes sastifing given values to labels in the pre-computed test function using a matching function inside it
//  * Then server add enc(1) to matched labels and enc(0) to other non matched labels
//  * @param pre_enc_stack: point to precomputed encryption stack of the serve
//  * @return an encrypted vector of enc(0) and enc(1), labels matched with partial view's labels
// */  

    void generateTest_Target_Attr(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map); 


// /**
//  * @brief Generates encrypted query that counts all records in the dataset that match the query selection
//  * @details The server matches all records that have the attributes sastifing given values using a matching function
//  * Then server add enc(1) to matched labels, enc(0) to non matched labels
//  * @param pre_enc_stack: point to precomputed encryption stack of the server
//  * @param enc_domain_map: partial view for servers to get all labels
//  * @return an encrypted vector of enc(0) and enc(1), labels matched with partial view's labels
// */    

    void generateNormalQuery(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map);


// /**
//  * @brief Generates the test that counts all records in the dataset that are sampled in the partial view
//  * @details The expected answer is V+/- noise
//  * @param pre_enc_stack: point to precomputed encryption stack of the serve
//  * @param enc_domain_map: using the partial view with both labels and encryptions
//  * @return an encrypted vector of enc(0) and enc(1), labels matched with partial view's labels
//  * enc(1) becomes a new ciphertext of 1
//  * enc(0) becomes a new ciphertext of 0
// */
    void generateTestBasedPartialView(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP enc_domain_map); // test the existence of V domains


// /**
//  * This function is to generate encrypted test function, there are many options
//  * if indicate 1: test for L known rows
//  * indicate 2: test for V rows in PV
//  * indicate 3: test for (V - r0)
//  * indicate 4: test for arbitrary attributes
//  */ 

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

// /**
//  * This function for a server to create a binary vector of V 0s and N-V 1s
//  * and encrypt them using precomputed encryption stack
// */
    void createRandomEncrypVector(ENC_Stack &pre_enc_stack);

};

#endif