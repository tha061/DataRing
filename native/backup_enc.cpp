
#include <iostream>
#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cmath>
#include <stack>



// void _timeEvaluate(string task_name, high_resolution_clock::time_point t1, high_resolution_clock::time_point t2)
// {
// 	double time_diff = duration_cast<nanoseconds>(t2 - t1).count();
// 	cout << "\n -------------------------------------------------------------------- \n";
// 	cout << "\nTime Evaluation \n";
// 	cout << task_name << " : " << time_diff / 1000000.0 << " ms" << endl;
// 	cout << "\n -------------------------------------------------------------------- \n";
// }

using namespace std;
class ENC_Stack
{
	// high_resolution_clock::time_point t1, t2;
    int top, max_size, top_E1, top_E0;
    gamal_key_t key;

public:
    gamal_ciphertext_t *myPIR_enc0;
    gamal_ciphertext_t *myPIR_enc1;

    ENC_Stack()
    {
        top = -1;
        top_E1 = -1;
        top_E0 = -1;
        myPIR_enc0 = NULL;
        myPIR_enc1 = NULL;
    };

    ENC_Stack(int size, gamal_key_t i_key)
    {
        top = -1;
        top_E1 = -1;
        top_E0 = -1;
        max_size = size;
        myPIR_enc0 = new gamal_ciphertext_t[max_size];
        myPIR_enc1 = new gamal_ciphertext_t[max_size];

        key->is_public = i_key->is_public;
        key->secret = i_key->secret;
        key->Y = i_key->Y;
    };

    bool isEmpty();
    // bool push(int plain);
    void initializeStack_E0();
    void initializeStack_E1();
    void pop_E1(gamal_ciphertext_t ciphertext);
    void pop_E0(gamal_ciphertext_t ciphertext);
};

bool ENC_Stack::isEmpty() 
{ 
    return (top < 0); 
} 

// void ENC_Stack::initializeStack_E0()
// {
//     int plain0 = 0;
//     int upper = max_size;
//     int lower = 2;
//     for(int i=0; i<max_size; i++)
//     {
//         ++top_E0;
//         if (i%(max_size/10) == 0)
//         {
//             cout << "Initialize E0 at " << i << "th iteration" << endl;
//         }
//         if(i == 0)
//         {
//             gamal_encrypt(myPIR_enc0[i], key, plain0);
//         }
//         else
//         {
//             int random_plain = (rand() % (upper - lower + 1)) + lower;
//             gamal_mult_opt(myPIR_enc0[i], myPIR_enc0[0], random_plain);
//         }
//     }
// }

void ENC_Stack::initializeStack_E0()
{
    int plain0 = 0;
    int lower = 2;
    int upper = lower+1;
    int j = lower;
    gamal_ciphertext_t temp;
    gamal_cipher_new(temp);
    int root_enc = 0;
    for(int i=0; i<max_size; i++)
    {
        ++top_E0;
        // if (i%(max_size/10) == 0)
        // {
        //     cout << "Initialize E0 at " << i << "th iteration" << endl;
        // }
        if(i == 0)
        {
            gamal_encrypt(myPIR_enc0[i], key, plain0);
            temp -> C1 = myPIR_enc0[0]->C1;
            temp -> C2 = myPIR_enc0[0]->C2;
        }
        else
        {
            if(j==upper)
            {
                j = 2;
                ++root_enc;
                temp -> C1 = myPIR_enc0[root_enc]->C1;
                temp -> C2 = myPIR_enc0[root_enc]->C2;
            }

            gamal_mult_opt(myPIR_enc0[i], temp, j);
            j++;

        }
    }
}

void ENC_Stack::initializeStack_E1()
{
    int plain1 = 1;
    for(int i=0; i<max_size; i++)
    {
        ++top_E1;
        if(i == 0)
        {
            gamal_encrypt(myPIR_enc1[i], key, plain1);
        }
        else
        {
            gamal_add(myPIR_enc1[i], myPIR_enc1[0], myPIR_enc0[i]);
        }
    }
}

void ENC_Stack::pop_E1(gamal_ciphertext_t ciphertext) 
{ 
    if (top_E1 < 0) { 
        cout << "Stack E1 Underflow\n";
    } 
    else { 
        gamal_cipher_new(ciphertext);
        ciphertext->C1 = myPIR_enc1[top_E1]->C1; 
        ciphertext->C2 = myPIR_enc1[top_E1]->C2; 
        top_E1--;
    } 
};

void ENC_Stack::pop_E0(gamal_ciphertext_t ciphertext) 
{ 
    if (top_E0 < 0) { 
        cout << "Stack E0 Underflow\n";
    } 
    else { 
        gamal_cipher_new(ciphertext);
        ciphertext->C1 = myPIR_enc0[top_E0]->C1; 
        ciphertext->C2 = myPIR_enc0[top_E0]->C2; 
        top_E0--;
    } 
};