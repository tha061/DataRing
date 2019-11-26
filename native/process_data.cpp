// #include <stdio.h>
// #include <iostream>
// #include <string>
// #include <fstream>
// #include <sstream>
// #include <map>
// #include <set>
// #include <vector>
// #include <iterator>
// #include <utility>
// #include <algorithm>
// #include<iterator> // for iterators 
// #include "process_data.h"

// using namespace std;

// typedef vector<int> int_vector;
// typedef std::map<pair<string, string>, int> hash_map;

// static const string DATA_DIR = "./data/unique_domains.csv";

// void print_hash_map (hash_map hashMap);

// void shuffle_vector(int_vector &index_vector)
// {
//     // shuffle vector
//     random_shuffle(index_vector.begin(), index_vector.end());
// }

// void processData(hash_map &hashMap)
// {
//     std::ifstream data(DATA_DIR);
//     if (!data.is_open())
//     {
//         std::exit(EXIT_FAILURE);
//     }

//     int i = 0;
//     std::string str;
//     std::getline(data, str); // skip the first line

//     while (!data.eof())
//     {
//         getline(data, str);
//         // cout << str << endl;
//         if(str.empty()){
//            continue; 
//         }
//         string id, name;
//         int count;
//         istringstream iss(str);
//         getline(iss, id, ',');
//         getline(iss, name, ',');
//         iss >> count;

//         // cout << name << ", " << count << endl;

//         hashMap.insert({make_pair(id, name), count});
//     }

//     // shuffle_vector(index_vector);
// }

// void print_hash_map (hash_map hashMap){
//     cout << "\nHash map\n";
//     int i = 0;
//     hash_map::iterator itr;
//     for (itr = hashMap.begin(); itr != hashMap.end(); ++itr)
//     {
//         i+=1;
//         if(i > 10){
//             break ;
//         }
//         cout << itr->first.second << "|" << itr->second << endl;
//     }
// }