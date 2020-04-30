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
#include <list>
#include <vector>
#include <iterator>
#include <utility>
#include <algorithm>
#include <iterator> // for iterators
#include <boost/math/distributions/laplace.hpp> //dataring
#include <boost/math/distributions/hypergeometric.hpp>//dataring
#include <boost/math/distributions/chi_squared.hpp>//dataring
#include <boost/array.hpp>
#include <cstdlib> 

using namespace std::chrono;
using namespace std;

// using namespace boost;
using namespace boost::math;

extern "C"
{
#include "../ecelgamal.h"
#include "../crtecelgamal.h"
#include "../src/generate_enc_vectors.h"
}

#include "../src/ENC_Stack.h"

using namespace std;


/** 
 * @file public_header.h
 * @brief Functions that define data structure and math packages used in Data Ring protocol.
 * @details Define data structure and supportive mathematical packages for Laplace distribution, Hypergeometric distribution and Chi Squared distribution.
 * @author Tham Nguyen tham.nguyen@mq.edu.au, Nam Bui, Data Ring
 * @date April 2020
*/

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

struct comp
{
    template <typename T>
    bool operator()(const T &l, const T &r) const
    {
        return l.first < r.first;
    }
};


typedef pair<string, string> id_domain_pair;
typedef std::map<id_domain_pair, int, comp> hash_pair_map; //histogram
typedef std::map<string, int> hash_map; //not used


typedef map<id_domain_pair, gamal_ciphertext_ptr, comp> ENC_DOMAIN_MAP; //encrypted partial view

typedef vector<id_domain_pair> id_domain_vector;
typedef set<id_domain_pair, comp> id_domain_set;

typedef map<id_domain_pair, gamal_ciphertext_ptr, comp> CIPHER_ENC_DOMAIN_MAP;//not used

typedef list<pair<string, string>> TRACK_LIST;

#endif