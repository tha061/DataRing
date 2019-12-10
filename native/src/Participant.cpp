#include "Participant.h"
#include "../public_func.h"

static void _printEncData(int index, gamal_ciphertext_t *enc_list)
{
    extern EC_GROUP *init_group;
    BIGNUM *x = BN_new();
    BIGNUM *y = BN_new();

    cout << "Print encryption of row index" << index << endl;
    printf("encryption of row index #%d->C1:\n", index);
    if (EC_POINT_get_affine_coordinates_GFp(init_group, enc_list[index]->C1, x, y, NULL))
    {
        BN_print_fp(stdout, x);
        putc('\n', stdout);
        BN_print_fp(stdout, y);
        putc('\n', stdout);
    }
    else
    {
        std::cerr << "Can't get point coordinates." << std::endl;
    }
    printf("\n");
    printf("encryption of row index #%d->C2:\n", index);
    if (EC_POINT_get_affine_coordinates_GFp(init_group, enc_list[index]->C2, x, y, NULL))
    {
        BN_print_fp(stdout, x);
        putc('\n', stdout);
        BN_print_fp(stdout, y);
        putc('\n', stdout);
    }
    else
    {
        std::cerr << "Can't get point coordinates." << std::endl;
    }

    printf("\n");
}

static int _getRandomInRange(int min, int max)
{
    return min + (rand() % (max - min + 1));
}

// const string Participant::DATA_DIR = "./data/unique_domains.csv";

Participant::Participant()
{
    size_dataset = 0;
}

Participant::Participant(string data_dir)
{
    size_dataset = 0;
    DATA_DIR = data_dir;
}

// Participant Generates original Histogram
void Participant::create_TrueHistogram(int datasize_row)
{
    std::ifstream data(DATA_DIR);
    if (!data.is_open())
    {
        std::exit(EXIT_FAILURE);
    }

    int i = 0;
    std::string str;
    std::getline(data, str); // skip the first line

    while (!data.eof())
    {
        if (i >= datasize_row)
        {
            break;
        }
        getline(data, str);
        if (str.empty())
        {
            continue;
        }
        string id, name;
        int count;
        istringstream iss(str);
        getline(iss, id, ',');
        getline(iss, name, ',');
        iss >> count;

        string id_domain = id + " " + name;

        hashMap.insert({make_pair(id, name), count});
        i++;
    }

    for (hash_pair_map::iterator itr = hashMap.begin(); itr != hashMap.end(); ++itr)
    {
        size_dataset += itr->second;
    }
}

// initialize ciphertext stack for participant
void Participant::initializePreStack(gamal_key_t coll_key)
{
    pre_enc_stack = ENC_Stack(hashMap.size(), coll_key);
    pre_enc_stack.initializeStack_E0();
    pre_enc_stack.initializeStack_E1();
}

void Participant::print_hash_map()
{
    cout << "\nHash map\n";
    int i = 0;
    hash_pair_map::iterator itr;
    for (itr = hashMap.begin(); itr != hashMap.end(); ++itr)
    {
        i += 1;
        if (i > 10)
        {
            break;
        }
        cout << itr->first.first << "|" << itr->first.second << "|" << itr->second << endl;
    }
}

string getDummyDomain()
{
    const int MAX_COL_1 = 40000;
    const int MIN_COL_1 = 1000;
    const int MAX_COL_2 = 40000;
    const int MIN_COL_2 = 1000;
    const int MAX_COL_3 = 40000;
    const int MIN_COL_3 = 0;
    const float MAX_COL_4 = 110000000.0;
    const float MIN_COL_4 = 0.0;
    const int DISTINCT_COL_5 = 2;
    const int DISTINCT_COL_7 = 7;
    const int DISTINCT_COL_8 = 12;
    const int DISTINCT_COL_9 = 3;
    const int DISTINCT_COL_10 = 6;

    int col1 = _getRandomInRange(MIN_COL_1, MAX_COL_1);
    int col2 = _getRandomInRange(MIN_COL_2, MAX_COL_2);
    int col3 = _getRandomInRange(MIN_COL_3, MAX_COL_3);
    float col4 = 0.0 + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (110000000.0 - 0.0)));
    int col5 = _getRandomInRange(0, DISTINCT_COL_5 - 1);
    float col6 = 5.31 + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (30.99 - 5.31)));
    int col7 = _getRandomInRange(0, DISTINCT_COL_7 - 1);
    int col8 = _getRandomInRange(-1, 10);
    int col9 = _getRandomInRange(0, DISTINCT_COL_9 - 1);
    int col10 = _getRandomInRange(0, DISTINCT_COL_10 - 1);

    string dummy_domain = to_string(col1) + " " + to_string(col2) + " " + to_string(col3) + " " + to_string(col4) + " " + to_string(col5) + " " + to_string(col6) + " " + to_string(col7) + " " + to_string(col8) + " " + to_string(col9) + " " + to_string(col10);

    return dummy_domain;
}

// Participant adds dummy bins to the original histogram -> true histogram
void Participant::addDummy_TrueHistogram(int factorSize)
{
    int domain_size = hashMap.size();
    // cout << "Size of original histogram: " << domain_size << endl;
    int pv_size = factorSize * size_dataset;

    int dummy_id = size_dataset;
    while (hashMap.size() < pv_size)
    {
        string dummy_domain = getDummyDomain();
        hashMap.insert({make_pair(to_string(dummy_id), dummy_domain), 0});
        dummy_id++;
    }

    // cout << "Total size of partial view histogram: " << hashMap.size() << endl
    //      << endl;

    fakeHashMap = hashMap;
}

/*
    Dishonest participant strategy 1: in n original rows, participant A keeps x rows and replace
    n-x rows as dummy rows. Values of bin associated with n original rows (n original domains) are set to 1
    participant add a*n dummy rows and set their bin values to 0
    Then A applies the sampling vector received from S:
    there are still V enc(1) and a*n - V enc(0).
    However, with this strategy, there are only x original domains are kept intact. If S 
    checks for the domains associated with L known records (regardless of the bin values are enc(1)
    or enc(0)), 
    there is high chance that there is not enough L domains are kept. 
*/

void Participant::addDummy_FakeHist(int keepDomainS, int factorSize)
{
    int domain_size = hashMap.size();
    cout << "Size of original histogram: " << domain_size << endl;
    int pv_size = factorSize * size_dataset;

    int dummy_id = size_dataset;
    while (hashMap.size() < pv_size)
    {
        string dummy_domain = getDummyDomain();
        hashMap.insert({make_pair(to_string(dummy_id), dummy_domain), 0});
        dummy_id++;
    }

    int replaceDomainS = size_dataset - keepDomainS;
    int counter = 0;

    hash_pair_map::iterator itr = hashMap.begin();
    for (itr; itr != hashMap.end(); itr++)
    {
        if (counter >= replaceDomainS)
        {
            break;
        }
        int count = itr->second;
        if (count > 0)
        {
            hashMap.erase(itr);
            string dummy_domain = getDummyDomain();
            dummy_id++;
            hashMap.insert({make_pair(to_string(dummy_id), dummy_domain), 1});
            counter++;
        }
    }
    cout << "Total size of fake partial view histogram: " << hashMap.size() << endl
         << endl;

    fakeHashMap = hashMap;
}

/*
    Dishonest participant strategy 2: A randomly keeps x (x<n) original rows in histogram,
    replace the bin value of (n-x) original rows from 1 to 0. For added dummy domains,
    A set bin values of (n-x) dummy rows to 1. All other dummy rows have bin value of 0 in
    the historgram.
    Hence, there still n bins with 1 and (a-1)*n bins with 0.
    Then A applies the sampling vector to its histogram, there still V enc(1) and all other 
    elements in PV are with enc(0)
    TO DO: add randomese feature to this function.
*/

void Participant::addDummy_FakeHist_random(int keepDomainS, int factorSize)
{
    // int domain_size = hashMap.size();
    // cout << "Size of original histogram: " << domain_size << endl;

    // // add dummy
    // int dummy_id = size_dataset;
    // while (hashMap.size() < pv_size)
    // {
    //     string dummy_domain = getDummyDomain();
    //     int random_id = _getRandomInRange(0, 1);
    //     hashMap.insert({make_pair(to_string(dummy_id), dummy_domain), 0});
    //     dummy_id++;
    // }

    // fakeHashMap = hashMap;
    Participant::addDummy_TrueHistogram(factorSize);
    int replaceDomainS = size_dataset - keepDomainS;
    int pv_size = factorSize * size_dataset;

    int replace_counter = 0;
    while (replace_counter < replaceDomainS)
    {
        int random_id = _getRandomInRange(size_dataset, pv_size - 1);
        // cout<<"random_id_fake_enc1 = "<< random_id <<endl;
        hash_pair_map::iterator find = fakeHashMap.find({to_string(random_id), ""});
        if (find != fakeHashMap.end() && find->second == 0)
        {
            find->second = 1;
            replace_counter++;
        }
    }

    replace_counter = 0;
    while (replace_counter < replaceDomainS)
    {
        int random_id = _getRandomInRange(0, size_dataset - 1);
        // cout<<"random_id_enc0 = "<< random_id <<endl;
        hash_pair_map::iterator find = fakeHashMap.find({to_string(random_id), ""});
        if (find != fakeHashMap.end() && find->second == 1)
        {
            find->second = 0;
            replace_counter++;
        }
    }

    // cout << "Total size of fake partial view histogram: " << fakeHashMap.size() << endl
    //      << endl;
    // cout << "Total size of original partial view histogram: " << hashMap.size() << endl
    //      << endl;
}

/*
   Dishonest participant A creates PV by itself:
    A does not apply the sampling vector to its histogram. Instead, A randomly chooses v rows in original dataset and keeps their bin
   bin value of 1; all other (n-v) original rows' bin value is set to 0
   add in n dummy rows and choose V-v rows in these dummy and set their bin value to 1;
   all other dummy bins value set to 0
   Then participant encrypts bins' value. Hence there are still V enc(1) and 2n-V enc(0), 
   and participant controls which domains were included in PV.
   Also, participant keeps all original domains name in PV, if server wants to see if the domain
   of L records are in PV, the participant passes this test.
*/

void Participant::selfCreateFakePV(int keepDomainS, int factorSize)
{
    Participant::addDummy_TrueHistogram(factorSize);
    int replaceDomainS = (size_dataset / 100) - keepDomainS;
    int pv_size = factorSize * size_dataset;

    int replace_counter = 0;
    while (replace_counter < replaceDomainS)
    {
        int random_id = _getRandomInRange(size_dataset, pv_size - 1);
        hash_pair_map::iterator find = fakeHashMap.find({to_string(random_id), ""});
        if (find != fakeHashMap.end() && find->second == 0)
        {
            find->second = 1;
            replace_counter++;
        }
    }

    replace_counter = 0;
    int total_replace_actual = size_dataset - keepDomainS;
    while (replace_counter < total_replace_actual)
    {
        int random_id = _getRandomInRange(0, size_dataset - 1);
        hash_pair_map::iterator find = fakeHashMap.find({to_string(random_id), ""});
        if (find != fakeHashMap.end() && find->second == 1)
        {
            find->second = 0;
            replace_counter++;
        }
    }

    int counter_row = 0;
    for (hash_pair_map::iterator itr = fakeHashMap.begin(); itr != fakeHashMap.end(); ++itr)
    {
        int decypt_cip = 0;

        id_domain_pair domain = itr->first;
        int domain_count = itr->second;

        gamal_ciphertext_t *mul_enc_ciphertext = new gamal_ciphertext_t[1];

        int track = 0;

        if (domain_count > 0)
        {
            decypt_cip = 1;
            pre_enc_stack.pop_E1(mul_enc_ciphertext[0]);
            counter_row++;
        }
        else
        {
            decypt_cip = 0;
            pre_enc_stack.pop_E0(mul_enc_ciphertext[0]);
        }

        enc_domain_map.insert({domain, mul_enc_ciphertext[0]});
        plain_domain_map.insert({domain, decypt_cip});
    }

    cout << "Counter row " << counter_row << endl;

    cout << "Total size of fake partial view histogram: " << fakeHashMap.size() << endl
         << endl;
    cout << "Total size of original partial view histogram: " << hashMap.size() << endl
         << endl;
}

void _printCiphertext(gamal_ciphertext_ptr ciphertext)
{
    extern EC_GROUP *init_group;
    BIGNUM *x = BN_new();
    BIGNUM *y = BN_new();

    if (EC_POINT_get_affine_coordinates_GFp(init_group, ciphertext->C1, x, y, NULL))
    {
        BN_print_fp(stdout, x);
        putc('\n', stdout);
        BN_print_fp(stdout, y);
        putc('\n', stdout);
    }
    else
    {
        std::cerr << "Can't get point coordinates." << std::endl;
    }
    printf("\n");
    if (EC_POINT_get_affine_coordinates_GFp(init_group, ciphertext->C2, x, y, NULL))
    {
        BN_print_fp(stdout, x);
        putc('\n', stdout);
        BN_print_fp(stdout, y);
        putc('\n', stdout);
    }
    else
    {
        std::cerr << "Can't get point coordinates." << std::endl;
    }

    printf("\n");
}

// plain_track_list: 1, 0 vector encrypted from Server
// enc_list: E1, E0 vector encrypted from Server
void Participant::generatePV(int *plain_track_list, gamal_ciphertext_t *enc_list, bool useTruth)
{
    hash_pair_map tmp_hashMap = useTruth ? hashMap : fakeHashMap;
    int counter_row = 0;
    // cout << "PV SIZE " << tmp_hashMap.size() << ", VECTOR FROM SERVER SIZE " << size_dataset << endl;
    for (hash_pair_map::iterator itr = tmp_hashMap.begin(); itr != tmp_hashMap.end(); ++itr)
    {
        int decypt_cip = 0;

        id_domain_pair domain = itr->first;
        int domain_count = itr->second;

        gamal_ciphertext_t *mul_enc_ciphertext = new gamal_ciphertext_t[1];

        int track = 0;
        if (domain_count > 0)
        {
            decypt_cip = plain_track_list[counter_row] * domain_count;
            gamal_mult_opt(mul_enc_ciphertext[0], enc_list[counter_row], domain_count);
            counter_row++;
        }
        else
        {
            decypt_cip = 0;
            Participant::pre_enc_stack.pop_E0(mul_enc_ciphertext[0]);
        }

        enc_domain_map.insert({domain, mul_enc_ciphertext[0]});
        plain_domain_map.insert({domain, decypt_cip});
    }

    // cout << "Counter row " << counter_row << endl;
    tmp_hashMap.clear();
}

void Participant::test_cleartext()
{
    fstream fout;
    // Open an existing file
    // fout.open("./data/report.csv", ios::out | ios::trunc);
    // fout << "ID, DOMAIN, SUM\n";
    int count = 0;
    int sum_value;
    for (hash_pair_map::iterator itr = plain_domain_map.begin(); itr != plain_domain_map.end(); ++itr)
    {
        sum_value = itr->second;
        // if (sum_value > 0)
        // {
        count += sum_value;
        // Insert the data to file
        // fout << itr->first.first << ", " << itr->first.second << ", " << sum_value << "\n";

        // cout << "Decrypt: " << sum_value << " of key " << itr->first << endl;
        // }
    }
    // fout.close();
    cout << "Total count of chosen plaintext 1 from server: " << count << endl;
}

void Participant::computeAnswer(ENC_DOMAIN_MAP &enc_test_map, gamal_ciphertext_t sum_cipher, bool useTruth, gamal_key_t &coll_key, double prob)
{
    hash_pair_map tmp_hashMap = useTruth ? hashMap : fakeHashMap;

    const int size_test_map = enc_test_map.size();
    // gamal_ciphertext_t *enc_list = new gamal_ciphertext_t[size_test_map];

    int counter = 0;

    gamal_ciphertext_t tmp, mul_tmp;
    gamal_cipher_new(tmp);
    gamal_cipher_new(mul_tmp);

    // cout << "Start multiply and add " << endl;
    int i = 0;
    for (ENC_DOMAIN_MAP::iterator itr = enc_test_map.begin(); itr != enc_test_map.end(); itr++)
    {
        string key = itr->first.first;
        string domain = itr->first.second;

        int value = tmp_hashMap[{key, domain}];
        if (value > 0)
        {
            gamal_mult_opt(mul_tmp, itr->second, value);

            if (counter == 0)
            {
                sum_cipher->C1 = mul_tmp->C1;
                sum_cipher->C2 = mul_tmp->C2;
            }
            else
            {
                tmp->C1 = sum_cipher->C1;
                tmp->C2 = sum_cipher->C2;
                gamal_add(sum_cipher, tmp, mul_tmp);
            }

            counter++;
        }
    }
    cout << "Counter " << counter << endl;

    float epsilon = 0.1;
    float sensitivity = 1.0;

    double maxNoise = getLaplaceNoiseRange(sensitivity, epsilon, prob);
    double minNoise = -maxNoise;
    cout << "Min noise: " << minNoise << endl;
    cout << "Max noise: " << maxNoise << endl;

    int randomNoise = (int)getLaplaceNoise(sensitivity, epsilon);
    // cout << "Random noise: " << randomNoise << endl;

    if (randomNoise < minNoise)
    {
        randomNoise = (int)(minNoise);
    }
    else if (randomNoise > maxNoise)
    {
        randomNoise = (int)(maxNoise);
    }

    if(randomNoise < 0)
    {
        randomNoise = -randomNoise;
    }

    cout << "Random noise: " << randomNoise << endl;

    gamal_ciphertext_t noiseEnc;
    gamal_cipher_new(noiseEnc);
    gamal_encrypt(noiseEnc, coll_key, randomNoise);

    gamal_cipher_new(tmp);
    tmp->C1 = sum_cipher->C1;
    tmp->C2 = sum_cipher->C2;
    gamal_add(sum_cipher, tmp, noiseEnc);
    
    tmp_hashMap.clear();
}
