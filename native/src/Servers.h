#ifndef SERVERS
#define SERVERS

#include "../include/public_header.h"
#include "process_noise.h"
#include "Server.h"

class Servers
{
public:
    gamal_key_t coll_key;
    vector<Server> server_vect;
    int server_size, data_size;

    gamal_ciphertext_t *s_myPIR_enc;
    int *s_plain_track_list;

    double maxNoise;
    double minNoise;

    // Servers();
    Servers(int server_size, int data_size, string known_domain_dir);
    void generateCollKey();
    dig_t _fusionDecrypt(gamal_ciphertext_t ciphertext, bsgs_table_t table, int serverId);
    void fusionDecrypt(ENC_DOMAIN_MAP enc_domain_map, bsgs_table_t table);
    void createPVsamplingVector(ENC_Stack &pre_enc_stack);

    //UPDATE THIS FUNCTION:
    // add a function to determine LEAST DOMAIN = r0 from hypergeometric distribution, using eta value = 0.9
    // as the percentile_noise in the quantile function at the moment.
    bool verifyingPV(ENC_DOMAIN_MAP enc_domain_map, bsgs_table_t table, int serverId, ENC_Stack &pre_enc_stack, double eta);

    // UPDATE THIS FUNCTION
    // adding an indication for choosing the verification for
    // i) exact test (know the threshold) e.g test for L: threshold = number of rows known by the servers
    // ii) estimated answer (for test function (4) - targeting arbitrary attribute)
    // the estimation function is to be implemented later
    bool verifyingTestResult(string testName, gamal_ciphertext_t sum_cipher, bsgs_table_t table, int serverId, int threshold);

    /** fix this function
     * This function is to verify if the test answer from the participant is within the
     * confidence interval +/- noise
     * para1 answer from participant encrypted
     * para2 encrypted answer from the function getTestResult_fromPV()
     * para3 alpha = 0.05 // 95% confidence level
     * para4 dataset_size = n
     * para5 PV_size = V
     * paras maxnoise
     * the servers decrypt answer from participant
     * then find conf_interval +- maxnoise
     * the answer from participant must be in [min_interval - maxnoise; max_interval + maxnoise]
     */
    bool verifyingTestResult_Estimate(string testName, gamal_ciphertext_t sum_cipher, bsgs_table_t table, int serverId, gamal_ciphertext_t enc_PV_answer, double alpha);

    //this function is to estimate the test results computed over the dataset based on
    // the result the servers achieved from the encrypted PV
    vector<double> estimate_conf_interval(double alpha, int PV_answer, int dataset_size, int PV_size);
};

#endif