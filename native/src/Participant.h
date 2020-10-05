#ifndef PARTICIPANT
#define PARTICIPANT

#include "../include/public_header.h"
#include "ENC_Stack.h"
#include "process_noise.h"

/** 
 * @file Participant.h
 * @brief Definition of functions in Participant class
 * @author Tham Nguyen tham.nguyen@mq.edu.au, Nam Bui, Data Ring
 * @date April 2020
*/ 

/** @brief This class provides functions for a participant in the Data Ring system.
 * @details The class implements functions for participant to convert its dataset in to a histogram, generate a partial view and compute query answer.
 * @author Tham Nguyen tham.nguyen@mq.edu.au, Nam Bui, Data Ring
 * @date April 2020
*/

class Participant
{

public:
    ///Dataset's directory 
    string DATA_DIR;  
    /// Permutation of the histogram map
    hash_pair_map map_public_data_domain_permute;
    /// Permutation sorted with flags of datapoints
    hash_pair_map map_public_data_domain_permute_to_send_flag;
    /// Vector of encoded labels
    vector<string> vector_endcoded_label; //based on public domain
    /// Vector of flags to datapoints: 1 or 0
    vector<int> vector_flag; //based on public domain and histogram
    ///vector of un-permutation of encoded labels
    vector<string> vector_un_permute_sort;
    /// True Histogram
    hash_pair_map histogram; 
    /// Fake Histogram
    hash_pair_map fake_histogram; 
    /// Key pair
    gamal_key_t keys; 
    ///Size of the dataset
    int size_dataset; 
    ///Ratio of partial view size to dataset size
    double pv_ratio; 
    /// Noise budget of a party for servers
    float noise_budget_server; 
    /// Noise budget of a party for another party
    float noise_budget_other_party; 
    ///Sensitivity of a query
    float sensitivity; 
    ///Maximum noise to add to any query answer
    double maxNoise; 
    ///Minimum noise to add to any query answer
    double minNoise; 
    ///Number of cheated answers during the query evaluation phase
    int no_lied_answer;

    /// Encrypted real query formulation
    ENC_DOMAIN_MAP enc_real_query_map;
    /// Encrypted real query pre-formulation
    ENC_DOMAIN_MAP enc_real_query_map_pre;

    id_domain_vector matching_query_domain_vec; 


   
/** Default constructor. Does something.
*/
    Participant();

/** My constructor. Initializes the coordinates.
*/   
    Participant(string data_dir);

/**
 * @brief Tranfers the original dataset into a histogram.
 * @details Each data record is represented by a label. 
 * @param dataset_size: dataset size
 * @return a map including N elements of <label, binValue = 1>
 */ 
    void create_OriginalHistogram(int dataset_size, int domainCoefficient);


 

/**
 * @brief Generates a permutation of the histogram of the dataset
 * @param public_data_domain: a vector of encoded labels corresponding to possible records
 * @param flag: a binary vector indicates the which position in the pemutation refering a data point
 * @return a permutation of the histogram (labels of data records are shuffled)
*/
    void getPermutationOfHistogram(vector<string> public_data_domain, vector<int> flag);

/**
 * @brief Stores the un-permutation corresponding to the permutation used for shuffling labels' order.
 * @param public_data_domain: a vector of encoded labels corresponding to possible records
 * @param map_public_data_domain_permute: the permutation
 * @return an inverse permutation
*/

    void getInversePermutationVector(vector<string> public_data_domain, hash_pair_map map_public_data_domain_permute); 


/**
 * @brief Computes query answer
 * @param enc_question_map: encrypted query/test function 
 * @param sum_cipher: encrypted answer
 * @param hist: histogram to apply the encrypted query to
 * @param coll_key: collective public key of servers used for encryption
 * @param epsilon_i: privacy budget for a query/test
 * 
 */
    void computeAnswer_opt(ENC_DOMAIN_MAP &enc_question_map, gamal_ciphertext_t sum_cipher, hash_pair_map hist, gamal_key_t &coll_key, float epsilon_i);



    //Sum query
    void computeAnswer_sum(ENC_DOMAIN_MAP &enc_question_map, gamal_ciphertext_t sum_cipher, hash_pair_map hist, gamal_key_t &coll_key, float epsilon_i, int attr_to_sum);


 
    /**
 * @brief Generates a fake histgram from the true histogram.
 * @details To simulate a cheating participant in Partial View Collection, cheating participant generates a fake histgram from the true histogram by 
 * replacing n-keepDomainS by 0 and making n-keepDomainS dummy 1
 * @param keepDomainS: the true records to be kept in the fake histogram
 * @param factorSize: scale_up factor to limit the domain size to a*N
 * @return a fake histgram of size = dataset size * scale_up including only keepDomainS bins are true
 * a map with aN elements of <label, binValue={1,0}>
*/     
    void addDummy_FakeHist_random(int keepDomainS, int factorSize);


/**
 * @brief Generate a fake histogram where many dummy labels have binvalue = 1
 * @details To simulate a cheating participant in PV Collection
 * @param factorSize: scaled-up factor (a) to limit the histogram size to aN
 * @param adding_ones: the number of dummy bins are added which have binValue=1 
 * @return a fake histogram where there are more than N labels having binValue =1, 
 * the original data records are kept, adding more arbitrary records with label = 1
 * then the query answer is scaled up
*/

    void addDummy_ones_FakeHistogram(int factorSize, float adding_ones); //added by Tham 29 Jan

    // print histogram: [(label, value)]
    void print_Histogram(string filename, hash_pair_map histo);


/** 
 * @brief Pre-compute a query function
 * @details This function pre-prepare a vector for a query function, each element in the vector
 * is a domain and its value is an enc(0).
 * This is pre-computed so that when the server wishes to generate a test,
 * it only needs to find the element where the domain is satisfied the condition
 * of the test and changes it from enc(0) to enc(1)
 * @param pre_enc_stack: point to the precomputed encryption stack
 * @param public_domain: histogram to get publicdomain 
 * @return a precomputed query function with all enc(0)
 */  

// void prepare_Real_Query(ENC_Stack &pre_enc_stack, vector<string> public_data_domain, ENC_DOMAIN_MAP party_enc_real_query_map_pre);

// void matchDomainForQuery(ENC_DOMAIN_MAP party_enc_real_query_map_pre, string query_directory, id_domain_vector party_matching_query_domain_vec);

// void generate_Real_Query(ENC_Stack &pre_enc_stack, ENC_DOMAIN_MAP party_enc_real_query_map_pre, id_domain_vector party_matching_query_domain_vec, ENC_DOMAIN_MAP party_enc_real_query_map);

void prepare_Real_Query(ENC_Stack &pre_enc_stack, vector<string> public_data_domain);

void matchDomainForQuery(string query_directoryc);

void generate_Real_Query(ENC_Stack &pre_enc_stack);


void get_data_domain_and_data_flag(hash_pair_map hist);
    
};

#endif