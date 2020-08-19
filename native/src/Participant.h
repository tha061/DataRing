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
    hash_pair_map map_v_permute;
    /// Permutation sorted with flags of datapoints
    hash_pair_map map_v_permute_to_send_flag;
    /// Vector of encoded labels
    vector<string> vector_endcoded_label;
    /// Vector of flags to datapoints: 1 or 0
    vector<int> vector_flag;
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
 * @param v: a vector of encoded labels corresponding to possible records
 * @param flag: a binary vector indicates the which position in the pemutation refering a data point
 * @return a permutation of the histogram (labels of data records are shuffled)
*/
    void getPermutationOfHistogram(vector<string> v, vector<int> flag);

/**
 * @brief Stores the un-permutation corresponding to the permutation used for shuffling labels' order.
 * @param v: a vector of encoded labels corresponding to possible records
 * @param map_v_permute: the permutation
 * @return an inverse permutation
*/

    void getInversePermutationVector(vector<string> v, hash_pair_map map_v_permute); 


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


    
};

#endif