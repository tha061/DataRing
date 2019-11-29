#include "Participant.h"

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

const string Participant::DATA_DIR = "./data/unique_domains.csv";

Participant::Participant()
{
    size_dataset = 0;
}

void Participant::processData()
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
        if (i >= 500000)
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

void Participant::addDummy(int factorSize)
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

    cout << "Total size of partial view histogram: " << hashMap.size() << endl
         << endl;
}

void Participant::addDummyFake_1(int keepDomainS, int factorSize)
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
}

void Participant::addDummyFake_2(int keepDomainS, int factorSize)
{

    int domain_size = hashMap.size();
    cout << "Size of original histogram: " << domain_size << endl;
    int pv_size = factorSize * size_dataset;
    int replaceDomainS = size_dataset - keepDomainS;

    int dummy_id = size_dataset;
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
            itr->second = 0;
            counter++;
        }
    }

    int counterDummy = 0;
    while (hashMap.size() < pv_size)
    {
        string dummy_domain = getDummyDomain();
        if (counterDummy >= replaceDomainS)
        {
            hashMap.insert({make_pair(to_string(dummy_id), dummy_domain), 0});
        }
        else
        {
            hashMap.insert({make_pair(to_string(dummy_id), dummy_domain), 1});
        }

        dummy_id++;
        counterDummy++;
    }
    cout << "Total size of fake partial view histogram: " << hashMap.size() << endl
         << endl;
}

void Participant::selfIntializePV(ENC_Stack &pre_enc_stack, int keepDomainS, int factorSize)
{
    int replaceDomainS = (size_dataset / 100) - keepDomainS;
    // cout << "keepDomainS " << keepDomainS << endl;

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

    int counter_row = 0;
    cout << "PV SIZE " << hashMap.size() << ", VECTOR FROM SERVER SIZE " << size_dataset << endl;

    for (hash_pair_map::iterator itr = hashMap.begin(); itr != hashMap.end(); ++itr)
    {
        int decypt_cip = 0;

        id_domain_pair domain = itr->first;
        int domain_count = itr->second;

        gamal_ciphertext_t *mul_enc_ciphertext = new gamal_ciphertext_t[1];

        int track = 0;
        if (domain_count > 0)
        {
            if (counter_row < replaceDomainS)
            {
                pre_enc_stack.pop_E1(mul_enc_ciphertext[0]);
                decypt_cip = 1;
                string dummy_domain = getDummyDomain();
                dummy_id++;
                enc_domain_map.insert({make_pair(to_string(dummy_id), dummy_domain), mul_enc_ciphertext[0]});
                plain_domain_map.insert({make_pair(to_string(dummy_id), dummy_domain), decypt_cip});
                counter_row++;
            }
            else if (counter_row < (size_dataset / 100) && counter_row >= replaceDomainS)
            {
                pre_enc_stack.pop_E1(mul_enc_ciphertext[0]);
                decypt_cip = 1;
                enc_domain_map.insert({domain, mul_enc_ciphertext[0]});
                plain_domain_map.insert({domain, decypt_cip});
                counter_row++;
            }
            else
            {
                pre_enc_stack.pop_E0(mul_enc_ciphertext[0]);
                decypt_cip = 0;
                enc_domain_map.insert({domain, mul_enc_ciphertext[0]});
                plain_domain_map.insert({domain, decypt_cip});
            }
        }
        else
        {
            decypt_cip = 0;
            pre_enc_stack.pop_E0(mul_enc_ciphertext[0]);
            enc_domain_map.insert({domain, mul_enc_ciphertext[0]});
            plain_domain_map.insert({domain, decypt_cip});
        }
    }

    cout << "Counter row " << counter_row << endl;
    cout << "PV size " << hashMap.size() << endl;
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
void Participant::multiply_enc_map(int *plain_track_list, gamal_ciphertext_t *enc_list, ENC_Stack &pre_enc_stack)
{
    int counter_row = 0;
    cout << "PV SIZE " << hashMap.size() << ", VECTOR FROM SERVER SIZE " << size_dataset << endl;
    for (hash_pair_map::iterator itr = hashMap.begin(); itr != hashMap.end(); ++itr)
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
            pre_enc_stack.pop_E0(mul_enc_ciphertext[0]);
        }

        enc_domain_map.insert({domain, mul_enc_ciphertext[0]});
        plain_domain_map.insert({domain, decypt_cip});
    }

    cout << "Counter row " << counter_row << endl;
}

void Participant::testWithoutDecrypt()
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

void Participant::proceedTestFunction(ENC_DOMAIN_MAP &enc_test_map, gamal_ciphertext_t sum_cipher)
{
    const int size_test_map = enc_test_map.size();
    // gamal_ciphertext_t *enc_list = new gamal_ciphertext_t[size_test_map];

    int counter = 0;

    gamal_ciphertext_t tmp, mul_tmp;
    gamal_cipher_new(tmp);
    gamal_cipher_new(mul_tmp);

    cout << "Start multiply and add " << endl;
    int i = 0;
    for (ENC_DOMAIN_MAP::iterator itr = enc_test_map.begin(); itr != enc_test_map.end(); itr++)
    {
        string key = itr->first.first;
        string domain = itr->first.second;

        int value = hashMap[{key, domain}];
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
}
