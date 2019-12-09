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

    // Servers();
    Servers(int server_size, int data_size, string known_domain_dir);
    void generateCollKey();
    dig_t _fusionDecrypt(gamal_ciphertext_t ciphertext, bsgs_table_t table, int serverId);
    void fusionDecrypt(ENC_DOMAIN_MAP enc_domain_map, bsgs_table_t table);
    void createServersEncrypVector(ENC_Stack &pre_enc_stack);

    //UPDATE THIS FUNCTION:
    // add a function to determine LEAST DOMAIN = r0 from hypergeometric distribution, using eta value = 0.9
    // as the probability in the quantile function at the moment.
    bool verificationPV(ENC_DOMAIN_MAP enc_domain_map, bsgs_table_t table, int serverId, ENC_Stack &pre_enc_stack, double prob);

    // UPDATE THIS FUNCTION
    // adding an indication for choosing the verification for
    // i) exact test (know the threshold) e.g test for L: threshold = number of rows known by the servers
    // ii) estimated answer (for test function (4) - targeting arbitrary attribute)
    // the estimation function is to be implemented later
    bool verificationTestResult(string testName, gamal_ciphertext_t sum_cipher, bsgs_table_t table, int serverId, int threshold, double prob);
    bool verificationTestResult_Estimate(string testName, gamal_ciphertext_t sum_cipher, bsgs_table_t table, int serverId, int min_conf, int max_conf);
};

#endif