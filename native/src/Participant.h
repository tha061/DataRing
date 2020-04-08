#ifndef PARTICIPANT
#define PARTICIPANT

#include "../include/public_header.h"
#include "ENC_Stack.h"
#include "process_noise.h"

class Participant
{

public:
    string DATA_DIR; // dataset directory
    ENC_Stack pre_enc_stack_participant; //Precomputed encryption stack if participant knows a collective Public Key
    ENC_DOMAIN_MAP enc_domain_map; // partial view encrypted
    hash_pair_map histogram, plain_domain_map, fake_histogram, original_histogram; //histogram map
    gamal_key_t keys; //added by Tham 21 Jan
    gamal_ciphertext_t cipher1; //encyption of 1 sent by servers
    int size_dataset; //size of the data set
    double pv_ratio; //partial view size to dataset size
    float epsilon_q; //epsilon for compute noise for a query answer
    float epsilon_test; //epsilon for compute noise for a test answer
    float sensitivity; //sensitivity of a query
    double maxNoise_q; //maximum noise to add to a query answer
    double minNoise_q; //minim noise to add to a query answer
    double maxNoise_test; //maximum noise to add to a test answer
    double minNoise_test; //minmum noise to add to a test answer
    int no_lied_answer; //number of lied answer during the query evaluation phase
    int scale_up_answer; //an answer has been scaled up by the participant


/**
 * This function create an object Participant
*/
    Participant();


/**
 * This function get the directory to the participant's dataset
*/    
    Participant(string data_dir);

/**
 * This function tranfer the original data into histogram of size N (dataset size)
 * each data records is represented by a label
 * there are N labels representing true data
 * @para: dataset_size: dataset size
 * @return: a map including N elements of <label, binValue = 1>
 */ 
    void create_OriginalHistogram(int dataset_size);

/**
 * This function generate a histgram from the true dataset
 * @para: factorSize: scale_up factor to limit the domain size to a*N
 * @return: a histgram of size = dataset size * scale_up
*/   
    void addDummy_to_Histogram(int factorSize); //add dummyy '0' to the histogram

/**
 * To simulaye a cheating participant in Partial View Collection
 * This function generate a fake histgram from the true histogram by 
 * replace n-keepDomainS by 0 and make n-keepDomainS dummy 1
 * @para: keepDomainS: the true records to be kept in the fake histogram
 * @para: factorSize: scale_up factor to limit the domain size to a*N
 * @return: a fake histgram of size = dataset size * scale_up including only keepDomainS bins are true
 * a map with aN elements of <label, binValue={1,0}>
*/     
    void addDummy_FakeHist_random(int keepDomainS, int factorSize);


/**
 * To simulate a cheating participant in PV Collection
 * This function generate a fake histogram where many dummy labels have binvalue = 1
 * @para: factorSize: scaled-up factor (a) to limit the histogram size to aN
 * @para: adding_ones: the number of dummy bins are added which have binValue=1 
 * @return: a fake histogram where there are more than N labels having binValue =1, 
 * the original data records are kept, adding more arbitrary records with label = 1
 * then the query answer is scaled up
*/

    void addDummy_ones_FakeHistogram(int factorSize, float adding_ones); //added by Tham 29 Jan


/**
 * Function for generating a partial view by applying the vector size aN to the histogram size aN
 * @para: *enc_list: encrypted sampling vector size aN from serveer
 * @para: histogram: a histogram of the party
 * @return: a vector of size aN with V enc(1) and aN-V enc(0)
*/
    void generatePV_fixed_scheme(gamal_ciphertext_t *enc_list, hash_pair_map hist, int dataset_size);



/**
 * function compute answer for a query when servers use different collective key for Query Evaluation Phase
 * @para: enc_question_map: question = query/test
 * @para: sum_cipher: encrypted answer
 * @para: hist: histogram to apply the enc_question_map to
 * @para: coll_key: collective PK to encrypt noise
 * @para: epsilon_i: privacy budget for a query/test
 * 
 */
    void computeAnswer_opt(ENC_DOMAIN_MAP &enc_question_map, gamal_ciphertext_t sum_cipher, hash_pair_map hist, gamal_key_t &coll_key, float epsilon_i);



/**
 * function compute answer for a query when party doesnt know collective public key
 * servers send over an encryption of 1 for party to encrypt its noise
 * @para: enc_question_map: question = query/test
 * @para: sum_cipher: encrypted answer
 * @para: hist: histogram to apply the enc_question_map to
 * @para: cipher_1: encryption of 1 from server, to encrypt noise
 * @para: epsilon_i: privacy budget for a query/test
 * 
 */
    void computeAnswer_modified(ENC_DOMAIN_MAP &enc_question_map, gamal_ciphertext_t sum_cipher, hash_pair_map hist, gamal_ciphertext_t cipher_1, float epsilon_i);   



    //party compute answer from the fake hist and scale up the answer
    void computeAnswer_scaled_up_answer(ENC_DOMAIN_MAP &enc_question_map, gamal_ciphertext_t sum_cipher, int scale_up_answer, hash_pair_map hist, gamal_key_t &coll_key, float epsilon_i);

    //Tham: sum query
    void computeAnswer_sum(ENC_DOMAIN_MAP &enc_question_map, gamal_ciphertext_t sum_cipher, hash_pair_map hist, gamal_key_t &coll_key, float epsilon_i, int attr_to_sum);


//===================== Supportive functions or unused functions ====================================================//


    void print_Histogram();


    // replace n-keepDomainS with dummy of E(1)
    void addDummy_FakeHist(int keepDomainS, int factorSize);

    
    // make vector by self and make PV View without Servers
    void selfCreateFakePV(int fakeEnc1, int factorSize);
    void selfCreateFakePV_opt(bool useTruth);
    void self_create_PV_prepare(int fakeEnc1, int factorSize);

    // after creating PV by itself, create a histogram included V bins in PV and hide all other bins (N-V) bins
    void generate_Histogram_included_self_PV(bool useTruth, int PV_size, int factorSize);

    
    void initializePreStack(gamal_key_t coll_key);


    void pre_process_generatePV(bool useTruth);
    void generatePV_opt(gamal_ciphertext_t *myPIR_enc, bool useTruth);
    void generatePV(int *plain_track_list, gamal_ciphertext_t *myPIR_enc, bool useTruth);


    void test_cleartext();
    
    //RENAME THIS FUNCTION to computeAnswer()
    //compute query answer
    void computeAnswer(ENC_DOMAIN_MAP &enc_question_map, gamal_ciphertext_t sum_cipher, hash_pair_map hist, gamal_key_t &coll_key);

    
    // party compute answer using original hist
    void computeAnswer_use_orig_histogram(ENC_DOMAIN_MAP &enc_question_map, gamal_ciphertext_t sum_cipher, hash_pair_map hist, gamal_key_t &coll_key, float epsilon_i);

    
};

#endif