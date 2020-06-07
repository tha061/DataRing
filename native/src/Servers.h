#ifndef SERVERS
#define SERVERS

#include "../include/public_header.h"
#include "process_noise.h"
#include "Server.h"


/** 
 * @brief This class provides functions for servers to jointly perform activities in the Data Ring system.
 * @details Functions include generating a collective public key from all servers;
 * threshold decrypt of a ciphertext;
 * creating a sampling vector for a participant to sample a partial view from its dataset;
 * verfifying the partial view from a participant; 
 * verifying an answer for a test function from the participant.
 * @author Tham Nguyen tham.nguyen@mq.edu.au, Nam Bui, Data Ring
 * @date April 2020
*/


class Servers
{
public:
    /// Collective public key
    gamal_key_t coll_key; 
    //// Collective public key used in partial view phase
    gamal_key_t coll_key_PV_phase; 
    //// Collective public key used in query evaluation phase
    gamal_key_t coll_key_query_phase; 
    /// Set of servers 
    vector<Server> server_vect; 
    /// Number of servers
    int server_size; 
    /// Participant's dataset size
    int data_size; 
    /// Scaled-up factor of histogram size
    int scale_up_factor; 
    /// Total number of possible records of a dataset
    int domain_size; 
    /// Encrypted sampling vector
    gamal_ciphertext_t *s_myPIR_enc; 
    /// Clear sampling vector
    int *s_plain_track_list; 
    /// Maximum noise can be added to participant's answer for DP guarantee
    double maxNoise; 
    /// Minimum noise can be added to participant's answer
    double minNoise; 
    /// Ratio of partial view size to dataset size
    double pv_ratio; 

    id_domain_set known_rows_after_phase2, verified_set, opened_rows_set, known_record_set, rows_set_in_opened_PV;
    
/**
 * @brief Constructor function.
*/
    Servers();
    // Servers(int server_size, int data_size, string known_domain_dir);

/**
 * @brief Initiate a vector of servers.
 * @param server_size: number of servers
 * @param data_size: participant's dataset size
 * @param known_domain_dir: directory to the background knowledge subset
 * @param scale_up_factor: "a" used by participant to make a histogram of size aN
*/
    Servers(int server_size, int data_size, string known_domain_dir, int scale_up_factor);

    

/**
 * @brief Generate a collective public key from a set of servers
 * @param collective_key: generated collective public key
*/
    void generateCollKey(gamal_key_t collective_key);


    void generateCollKey_projectBased(gamal_key_t collective_key, bool coll_key_phase);


/**
 * @brief All servers to jointly decrypt a ciphertext
 * @param ciphertext to be decrypted
 * @param table: lookup table for mapping a EC point to original integer
 * @param serverID: the server initiates the threshold decryption
 * @param key_for_decrypt: which collective public key is used
 * @return the plaintext
*/
    dig_t _fusionDecrypt(gamal_ciphertext_t ciphertext, bsgs_table_t table, int serverId);

// /**
//  * @brief All servers to jointly decrypt a ciphertext
//  * @param ciphertext to be decrypted
//  * @param table: lookup table for mapping a EC point to original integer
//  * @param serverID: the server initiates the threshold decryption
//  * @param key_for_decrypt: which collective public key is used
//  * @return the plaintext
// */
    dig_t _fusionDecrypt_projectBased(gamal_ciphertext_t ciphertext, bsgs_table_t table, int serverId, bool key_for_decrypt);

/**
 * @brief A server to generate a encrypted sampling vector
 * @param pre_enc_stack: pre computed enc(0) and enc(1) stack
 * @return an encrypted sampling vector of size N+1 with V enc(1) and N-V enc(0) and last one is enc(0)
*/

    void createPVsamplingVector_size_N_plus_1(ENC_Stack &pre_enc_stack);

/**
 * @brief All servers verify the submitted partial view
 * @details Condition for passing the verification is generated inside the function
 * @param enc_domain_map: encrypted partial view sent be a participant
 * @param table: a lookup table for decryption
 * @param serverID: the server to initiate the threshold decryption
 * @param pre_enc_stack: stack of precomputed encryption of servers to get an enc(0) for the special case where there is no known record found in the PV, 
 * we need to decrypt the encrypted total number of known records found in PV, 
 * the encrypted total number of known records is initiated with enc(0)
 * @param eta: the tolerated false positve rate that the verification can have
 * @return a true/fale boolean: 1=pass; 0=fail
*/
    bool verifyingPV(ENC_DOMAIN_MAP enc_domain_map, bsgs_table_t table, int serverId, ENC_Stack &pre_enc_stack, double eta);

// /**
//  * @brief All servers re-encrypt the partial view to a new collective public key used for query evaluation phase
//  * @param coll_key_for_query_phase: new collective public key
//  * @param enc_domain_map: encrypted partial view under coll_key_for_PV_phase
//  * @param serverId: server lead the re-encryption
// */    

    void re_encrypt_PV(ENC_DOMAIN_MAP enc_domain_map, int serverId, gamal_key_t coll_key_for_query_phase);


/**
 * @brief All servers jointly decrypt the test answer from the participant and check if the decrypted answer satisfies the condition of the test
 * @param testName: specify the test to the server
 * @param sum_cipher: the encrypted noisy answer from participant
 * @param table: lookup table for decryption
 * @param serverID: the server which initiate (lead) the decryption
 * @param threshold: expected answer
 * @return a status that pass/fail the test
*/
    // bool verifyingTestResult(string testName, gamal_ciphertext_t sum_cipher, bsgs_table_t table, int serverId, int threshold);
    bool verifyingTestResult(gamal_ciphertext_t sum_cipher, bsgs_table_t table, int serverId, int threshold);

/** 
 * @brief Verify if the test answer from the participant is within the confidence interval +/- noise
 * @details The servers decrypt answer from participant, then find conf_interval +- maxnoise.
 * The answer from participant must be in [min_interval - maxnoise; max_interval + maxnoise]
 * @param testName: specify the test to the server
 * @param sum_cipher: encrypted answer from participant 
 * @param table: lookup table for decryption
 * @param enc_PV_answer: encrypted answer from the function getTestResult_fromPV()
 * @param alpha = 0.05 // 95% confidence level
 * @return a status that pass/fail the test
 */
    bool verifyingTestResult_Estimate(string testName, gamal_ciphertext_t sum_cipher, bsgs_table_t table, int serverId, gamal_ciphertext_t enc_PV_answer, double alpha);

/**
 * @brief Helper function to estimate the test results computed over the dataset based on
 * the result the servers achieved from the encrypted PV.
 * @param alpha: confidence level
 * @param PV_answer: query answer computed from the partial view
 * @param dataset_size: dataset's size
 * @param PV_size: size of the partial view
 */
    vector<double> estimate_conf_interval(double alpha, int PV_answer, int dataset_size, int PV_size);




//===================== Supportive functions or unused functions ====================================================//

// /**
//  * This function is for a server to create a sampling vector of size N
//  * @param ciphertext to be decrypted
//  * @return an encrypted sample vector of length N
// */    
    
    void createPVsamplingVector(ENC_Stack &pre_enc_stack);

    void decryptPV(ENC_DOMAIN_MAP unPermutePV, bsgs_table_t table, int serverId);


// /**
//  * This function saves known records in background knowledge found in the partial view
//  * @param known records found in the PV
//  * @return a subset of known records found in the PV
//  */    
    void save_knownRow_found_in_PV(id_domain_pair verified_domain_pair);

// /**
//  * This function saves rows were opened by the server during the partial view verification
//  */
    void save_opened_rows(id_domain_pair opened_rows);

// /**
//  * This fuction saves all rows that serves know about the dataset after PV phase
//  */    
    void save_knownRow_after_phase2(id_domain_pair domain_pair);    


    void open_true_PV(ENC_DOMAIN_MAP enc_domain_map, bsgs_table_t table, int serverId, ENC_Stack &pre_enc_stack);

    void fusionDecrypt(ENC_DOMAIN_MAP enc_domain_map, bsgs_table_t table);

};

#endif