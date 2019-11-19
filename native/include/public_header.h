
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
#include<iterator> // for iterators 
using namespace std::chrono;


extern "C"
{
#include "ecelgamal.h"
#include "crtecelgamal.h"
}


#include "backup_enc.cpp"

using namespace std;

typedef vector<int> int_vector;
typedef std::map<pair<string, string>, int> hash_map;