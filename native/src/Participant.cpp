#include "Participant.h"

static void _printEncData(int index, gamal_ciphertext_t *myPIR_enc)
{
    extern EC_GROUP *init_group;
    BIGNUM *x = BN_new();
    BIGNUM *y = BN_new();

    cout << "Print encryption of row index" << index << endl;
    printf("encryption of row index #%d->C1:\n", index);
    if (EC_POINT_get_affine_coordinates_GFp(init_group, myPIR_enc[index]->C1, x, y, NULL))
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
    if (EC_POINT_get_affine_coordinates_GFp(init_group, myPIR_enc[index]->C2, x, y, NULL))
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
        getline(data, str);
        // cout << str << endl;
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

        hashMap.insert({id, count});
    }

    for (hash_map::iterator itr = hashMap.begin(); itr != hashMap.end(); ++itr)
    {
        size_dataset += itr->second;
    }
}

void Participant::print_hash_map()
{
    cout << "\nHash map\n";
    int i = 0;
    hash_map::iterator itr;
    for (itr = hashMap.begin(); itr != hashMap.end(); ++itr)
    {
        i += 1;
        if (i > 10)
        {
            break;
        }
        cout << itr->first << "|" << itr->second << endl;
    }
}

void Participant::addDummy()
{
    int domain_size = hashMap.size();
    cout << "Size of original histogram: " << domain_size << endl;
    int pv_size = 2 * size_dataset;
    // int pv_size = 10 + size_dataset;
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

    int dummy_id = size_dataset;
    while (hashMap.size() < pv_size)
    {
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

        string dummy_domain = to_string(dummy_id) + " " + to_string(col1) + " " + to_string(col2) + " " + to_string(col3) + " " + to_string(col4) + " " + to_string(col5) + " " + to_string(col6) + " " + to_string(col7) + " " + to_string(col8) + " " + to_string(col9) + " " + to_string(col10);

        hashMap.insert({to_string(dummy_id), 0});
        dummy_id++;
    }

    cout << "Total size of partial view histogram: " << hashMap.size() << endl
         << endl;
}

// plain_track_list: 1, 0 vector encrypted from Server
// myPIR_enc: E1, E0 vector encrypted from Server
void Participant::multiply_enc_map(int *plain_track_list, gamal_ciphertext_t *myPIR_enc, ENC_Stack &pre_enc_stack)
{
    int counter_row = 0;
    cout << "PV SIZE " << hashMap.size() << ", VECTOR FROM SERVER SIZE " << size_dataset << endl;
    for (hash_map::iterator itr = hashMap.begin(); itr != hashMap.end(); ++itr)
    {
        int decypt_cip = 0;

        string domain = itr->first;
        int domain_count = itr->second;

        gamal_ciphertext_t *mul_enc_ciphertext = new gamal_ciphertext_t[1];

        int track = 0;
        if (domain_count > 0)
        {
            decypt_cip = plain_track_list[counter_row] * domain_count;
            gamal_mult_opt(mul_enc_ciphertext[0], myPIR_enc[counter_row], domain_count);
            counter_row++;
        }
        else
        {
            decypt_cip = 0;
            pre_enc_stack.pop_E0(mul_enc_ciphertext[0]);
        }

        enc_domain_map.insert({domain, mul_enc_ciphertext});
        plain_domain_map.insert({domain, decypt_cip});
    }

    cout << "Counter row " << counter_row << endl;
}

void Participant::testWithoutDecrypt()
{
    fstream fout;
    // Open an existing file
    fout.open("./data/report.csv", ios::out | ios::trunc);
    fout << "ID, DOMAIN, SUM\n";
    int count = 0;
    int sum_value;
    for (hash_map::iterator itr = plain_domain_map.begin(); itr != plain_domain_map.end(); ++itr)
    {
        sum_value = itr->second;
        // if (sum_value > 0)
        // {
        count += sum_value;
        // Insert the data to file
        fout << itr->first << ", " << sum_value << "\n";
        // cout << "Decrypt: " << sum_value << " of key " << itr->first << endl;
        // }
    }
    fout.close();
    cout << "Total count of chosen plaintext 1 from server: " << count << endl;
}