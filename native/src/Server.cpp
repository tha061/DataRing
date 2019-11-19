#include "include/public_header.h"

using namespace std;

class Server
{
public:
    gamal_ciphertext_t *myPIR_enc;
    int *plain_track_list;
    int size_dataset;
    Server()
    {
        myPIR_enc = NULL;
        plain_track_list = NULL;
        size_dataset = 0;
    }

    Server(int size)
    {
        myPIR_enc = new gamal_ciphertext_t[size];
        plain_track_list = new int[size];
        size_dataset = size;
    }

    void createRandomEncrypVector(gamal_key_t key, bsgs_table_t table, ENC_Stack pre_enc_stack);
};

void Server::createRandomEncrypVector(gamal_key_t key, bsgs_table_t table, ENC_Stack pre_enc_stack)
{
    int *myPIR_arr; //final array with value of encryp type after reverting from shuffle array

    int enc_types[] = {1, 0};
    int freq[] = {1, 99};
    int pv_ratio = 100;

    myPIR_arr = new int[size_dataset];

    pir_gen(myPIR_arr, enc_types, freq, size_dataset, pv_ratio); // function that server place 1 or 0 randomly

    // ========== Encrypt the vector =============== //

    int plain1 = 1, plain0 = 0;

    for (int i = 0; i < size_dataset; i++)
    {
        plain_track_list[i] = myPIR_arr[i];
        if (myPIR_arr[i] == 1)
        {
            pre_enc_stack.pop_E1(myPIR_enc[i]);
        }
        else
        {
            pre_enc_stack.pop_E0(myPIR_enc[i]);
        }
    }

    delete[] myPIR_arr;
}