#include "./public_func.h"

void printEncData(int index, gamal_ciphertext_t *myPIR_enc)
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

void printCiphertext(gamal_ciphertext_t ciphertext)
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

void decryptFind(map<string, gamal_ciphertext_t *> enc_domain_map, gamal_key_t key, bsgs_table_t table)
{
	int count1 = 0;
	dig_t res;
	for (map<string, gamal_ciphertext_t *>::iterator itr = enc_domain_map.begin(); itr != enc_domain_map.end(); ++itr)
	{
		gamal_decrypt(&res, key, itr->second[0], table);
		if (res > 0)
		{
			count1 += res;
			cout << "Decrypt: " << res << " of key " << itr->first << endl;
		}
	}
	cout << "Total count of chosen plaintext 1 from server: " << count1 << endl;
}

int getRandomInRange(int min, int max)
{
	return min + (rand() % (max - min + 1));
}



string getString(vector<string>& v) 
{ 

	// Size of the vector 
	int n = v.size(); 
	
	// Generate a random number 
	srand(time(NULL)); 

	// Make sure the number is within the index range 
	int index = rand() % n; 
	

	// Get random number from the vector 
	string str = v[index]; 
	

	// Remove the number from the vector 
	swap(v[index], v[n - 1]); 
	v.pop_back(); 

	// Return the removed number 
	return str; 
} 


int getRandomNumber(vector<int>& v) 
{ 

	// Size of the vector 
	int n = v.size(); 
	

	// Generate a random number 
	srand(time(NULL)); 

	// Make sure the number is within the index range 
	int index = rand() % n; 
	

	// Get random number from the vector 
	int num = v[index]; 
	

	// Remove the number from the vector 
	swap(v[index], v[n - 1]); 
	v.pop_back(); 

	// Return the removed number 
	return num; 
} 
