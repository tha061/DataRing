#ifndef PUBLIC_HEADER
#define PUBLIC_HEADER

#include <stdio.h>
#include <chrono>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <vector>
#include <iterator>
#include <utility>
#include <algorithm>
#include <iterator> // for iterators
using namespace std::chrono;
using namespace std;

extern "C"
{
#include "../ecelgamal.h"
#include "../crtecelgamal.h"
#include "../src/generate_enc_vectors.h"
}

#include "../src/ENC_Stack.h"
#include "../process_data.cpp"
#include "../process_partial_view.cpp"

using namespace std;

typedef vector<int> int_vector;

struct comparer
{
public:
    bool operator()(string str1, string str2) const
    {
        vector<string> words1, words2;
        string temp1, temp2;

        // Convert the first string to a list of words
        std::stringstream stringstream1(str1);
        stringstream1 >> temp1;

        std::stringstream stringstream2(str2);
        stringstream2 >> temp2;

        return temp1.compare(temp2) != 0;
    }
};

typedef std::map<string, int> hash_map;
typedef map<string, gamal_ciphertext_t *> ENC_DOMAIN_MAP;
typedef vector<string> id_domain_vector;

#endif