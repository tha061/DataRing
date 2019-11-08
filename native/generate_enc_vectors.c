/**
 * Added by Tham to generate random PIR vector in clear
 * Utility function to find ceiling of r in arr[l..h]
 */
int findCeil(int arr[], int r, int l, int h)
{
    int mid;
    while (l < h)
    {
        mid = l + ((h - l) >> 1); // Same as mid = (l+h)/2
        (r > arr[mid]) ? (l = mid + 1) : (h = mid);
    }
    return (arr[l] >= r) ? l : -1;
}

/**
 * Added by Tham to generate random PIR vector in clear
 * The main function that returns a random number from arr[] according to
 * distribution array defined by freq[]. n is size of arrays.
 */

int myRand(int arr[], int freq[], int n)
{
    // Create and fill prefix array
    int prefix[n], i;
    prefix[0] = freq[0];
    for (i = 1; i < n; ++i)
    {
        prefix[i] = prefix[i - 1] + freq[i];
    }
    // prefix[n-1] is sum of all frequencies. Generate a random number
    // with value from 1 to this sum
    int r = (rand() % prefix[n - 1]) + 1;

    // Find index of ceiling of r in prefix arrat
    int indexc = findCeil(prefix, r, 0, n - 1);

    return arr[indexc]; // return arr[1] -> 0 | arr[0] -> 1
}

// histgen()
// histogr (scale_up x datasize) added dummy data
// arr {1, 0}
// freq {1, scale_up}
int *hist_gen(int histogr[], int arr[], int freq[], int datasize, int scale_up)
{
    int n = sizeof(arr) / sizeof(arr[0]); // = 2
    int count_bin1 = 0;
    int index;
    for (index = 0; index < datasize * scale_up; index++)
    {
        histogr[index] = myRand(arr, freq, 2);
        // printf("%d\n", myrand_arr[i]);
        if (histogr[index] == 1)
            count_bin1++;
        if (count_bin1 >= datasize)
        {
            // printf("break when i = %d\n", index --);
            break;
        }
        //i++;
    }
    // printf("i= %d\n", i);

    int i;
    if (count_bin1 < datasize)
    {
        for (i = datasize * scale_up - 1; i >= 0; i--)
        {
            histogr[i] = 1;
            count_bin1++;
            i--;
            i--;
            if (count_bin1 >= datasize)
            {
                // printf("second break when i = %d\n", i--);
                break;
            }
        }
    }
    // printf("%d\n",histogr[i]);
    printf("number of bins '1' after adding dummy = %d\n", count_bin1);

    //	  for (index = 0; index < datasize*scale_up; index++){
    // printf("%d", histogr[index]);
    // }
    //printf("\n");
}

int *pir_gen(int* myPIR_arr, int arr[], int freq[], int datasize, int pv_ratio)
{
    // printf("freq[1] %d\n", freq[1]);
    int pv_size = (int)datasize / pv_ratio; //1% of dataset
    int n = sizeof(arr) / sizeof(arr[0]);   // 2
    // printf("sizeof(arr) = %d\n", sizeof(arr));
    // printf("sizeof(arr[0]) = %d\n", sizeof(arr[0]));
    // printf("n = %d\n", n);

    int count_1 = 0;
    int i;

    for (i = 0; i < datasize; i++)
    {
        if (count_1 >= pv_size) // if total number of E(1) > threshold 
        {
            myPIR_arr[i] = 0;
            
        }
        myPIR_arr[i] = myRand(arr, freq, n);
        if (myPIR_arr[i] == 1)
            count_1++;
        //i++;
    }
    // printf("i= %d\n", i);

    if (count_1 < pv_size)
    {
        for (int i = datasize - 1; i >= 0; i--)
        {
            myPIR_arr[i] = 1;
            count_1++;
            i--;
            i--;
            if (count_1 >= pv_size)
            {
                //printf("second break when i = %d\n", i--);
                break;
            }
        }
    }

    printf("\n/=======================pir_gen=====================/\n");
    printf("pv size = %d\n", pv_size);
    printf("number of '1' for PV collect = %d\n", count_1);
    printf("\n/============================================/\n");
}