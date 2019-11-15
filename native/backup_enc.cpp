
#include <iostream>
#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cmath>
#include <stack>



void _timeEvaluate(string task_name, high_resolution_clock::time_point t1, high_resolution_clock::time_point t2)
{
	double time_diff = duration_cast<nanoseconds>(t2 - t1).count();
	cout << "\n -------------------------------------------------------------------- \n";
	cout << "\nTime Evaluation \n";
	cout << task_name << " : " << time_diff / 1000000.0 << " ms" << endl;
	cout << "\n -------------------------------------------------------------------- \n";
}

using namespace std;
class ENC_Stack
{
	high_resolution_clock::time_point t1, t2;
    int top, max_size;
    gamal_key_t key;

public:
    gamal_ciphertext_t *myPIR_enc0;
    gamal_ciphertext_t *myPIR_enc1;

    ENC_Stack()
    {
        top = -1;
        myPIR_enc0 = NULL;
        myPIR_enc1 = NULL;
    };

    ENC_Stack(int size, gamal_key_t i_key)
    {
        top = -1;
        max_size = size;
        myPIR_enc0 = new gamal_ciphertext_t[max_size];
        myPIR_enc1 = new gamal_ciphertext_t[max_size];

        key->is_public = i_key->is_public;
        key->secret = i_key->secret;
        key->Y = i_key->Y;
    };

    bool isEmpty();
    bool push(int plain);
    void pop(gamal_ciphertext_t ciphertext);
    void initializeStack_E0();
    void initializeStack_E1();
};

bool ENC_Stack::isEmpty() 
{ 
    return (top < 0); 
} 

void ENC_Stack::initializeStack_E0()
{
    t1 = high_resolution_clock::now();
    int plain0 = 0;
    int upper = max_size;
    int lower = 2;
    for(int i=0; i<max_size; i++)
    {
        if(i == 0)
        {
            gamal_encrypt(myPIR_enc0[i], key, plain0);
        }
        else
        {
            int random_plain = (rand() % (upper - lower + 1)) + lower;
            gamal_mult_opt(myPIR_enc0[i], myPIR_enc0[0], random_plain);
        }
    }
    t2 = high_resolution_clock::now();
    _timeEvaluate("Initialize E0 Stack", t1, t2);
}

void ENC_Stack::initializeStack_E1()
{
    t1 = high_resolution_clock::now();
    int plain1 = 1;
    for(int i=0; i<max_size; i++)
    {
        if(i == 0)
        {
            gamal_encrypt(myPIR_enc1[i], key, plain1);
        }
        else
        {
            gamal_add(myPIR_enc1[i], myPIR_enc1[0], myPIR_enc0[i]);
        }
    }
    t2 = high_resolution_clock::now();
    _timeEvaluate("Initialize E1 Stack", t1, t2);
}

bool ENC_Stack::push(int plain)
{
    if (top >= (max_size - 1))
    {
        cout << "Stack Overflow";
        return false;
    }
    else
    {
        gamal_encrypt(myPIR_enc0[++top], key, plain);
        cout << " pushed into stack\n";
        return true;
    }
};

void ENC_Stack::pop(gamal_ciphertext_t ciphertext) 
{ 
    if (top < 0) { 
        cout << "Stack Underflow";
    } 
    else { 
        gamal_cipher_new(ciphertext);
        ciphertext->C1 = myPIR_enc0[top--]->C1; 
        ciphertext->C2 = myPIR_enc0[top--]->C2; 
    } 
};

