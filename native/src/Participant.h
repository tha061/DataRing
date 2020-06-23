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

    //-----------------------------------------// 
    // An answer has been scaled up by the participant
    // int scale_up_answer; 
    // hash_pair_map original_histogram, plain_domain_map;
    // Encrrypted partial view
    // ENC_DOMAIN_MAP enc_domain_map;
    // Pre-computed encyption of 1
    // gamal_ciphertext_t cipher1;  
    // Precomputed encryption stack if participant knows a collective Public Key
    // ENC_Stack pre_enc_stack_participant; 


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


 // 
//  void create_OriginalHistogram(int dataset_size);   

// /**
//  * @brief Adds dummy records to the histogram.
//  * @param factorSize: scale_up factor to limit the domain size to a*N
//  * @return a histgram of size = dataset size * scale_up
// */   
//     void addDummy_to_Histogram(int factorSize); //add dummyy '0' to the histogram



// // /**
// //  * @brief Generates a partial view by applying the sampling vector to the histogram.
// //  * @param enc_list: encrypted sampling vector size aN from serveer
// //  * @param hist: a histogram of the party to generate partial view
// //  * @param dataset_size: size of the dataset
// //  * @return a vector of size aN with V enc(1) and aN-V enc(0)
// // */
//     void generatePV_fixed_scheme(gamal_ciphertext_t *enc_list, hash_pair_map hist, int dataset_size);

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

    void print_Histogram(string filename, hash_pair_map histo);

// // /**
// //  * @brief Computes query answer when party doesnt know collective public key
// //  * @details Servers send over an encryption of 1 to the participants for them to encrypt Laplace noise
// //  * @param enc_question_map: encrypted query/test function 
// //  * @param sum_cipher: encrypted answer
// //  * @param hist: histogram to apply the encrypted query/test function  to
// //  * @param cipher_1: encryption of 1 from servers, to encrypt noise
// //  * @param epsilon_i: privacy budget for a query/test
// //  * 
// //  */
//     void computeAnswer_modified(ENC_DOMAIN_MAP &enc_question_map, gamal_ciphertext_t sum_cipher, hash_pair_map hist, gamal_ciphertext_t cipher_1, float epsilon_i);   



    // //party compute answer from the fake hist and scale up the answer
    // void computeAnswer_scaled_up_answer(ENC_DOMAIN_MAP &enc_question_map, gamal_ciphertext_t sum_cipher, int scale_up_answer, hash_pair_map hist, gamal_key_t &coll_key, float epsilon_i);

    //Tham: sum query
    void computeAnswer_sum(ENC_DOMAIN_MAP &enc_question_map, gamal_ciphertext_t sum_cipher, hash_pair_map hist, gamal_key_t &coll_key, float epsilon_i, int attr_to_sum);

    // void getArbitraryUnPermutationVector(vector<string> v, hash_pair_map map_v_permute);

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


//===================== Supportive functions or unused functions ====================================================//


    // // void print_Histogram(string filename);
    

    // // replace n-keepDomainS with dummy of E(1)
    // void addDummy_FakeHist(int keepDomainS, int factorSize);

    
    // // make vector by self and make PV View without Servers
    // void selfCreateFakePV(int fakeEnc1, int factorSize);
    // void selfCreateFakePV_opt(bool useTruth);
    // void self_create_PV_prepare(int fakeEnc1, int factorSize);

    // // after creating PV by itself, create a histogram included V bins in PV and hide all other bins (N-V) bins
    // void generate_Histogram_included_self_PV(bool useTruth, int PV_size, int factorSize);

    
    // void initializePreStack(gamal_key_t coll_key);


    // void pre_process_generatePV(bool useTruth);
    // void generatePV_opt(gamal_ciphertext_t *myPIR_enc, bool useTruth);
    // void generatePV(int *plain_track_list, gamal_ciphertext_t *myPIR_enc, bool useTruth);


    // void test_cleartext();
    
    // //RENAME THIS FUNCTION to computeAnswer()
    // //compute query answer
    // void computeAnswer(ENC_DOMAIN_MAP &enc_question_map, gamal_ciphertext_t sum_cipher, hash_pair_map hist, gamal_key_t &coll_key);

    
    // // party compute answer using original hist
    // void computeAnswer_use_orig_histogram(ENC_DOMAIN_MAP &enc_question_map, gamal_ciphertext_t sum_cipher, hash_pair_map hist, gamal_key_t &coll_key, float epsilon_i);

    
};

#endif