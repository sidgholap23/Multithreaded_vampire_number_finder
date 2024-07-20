#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <pthread.h>

#define max_count 10000000
#define max_threads 1000

// Define a mutex for thread safety
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;

struct thread_data
{
    int thread_id;
    int num_count;
    int numbers[max_count];
};

// Declare a counter variable for vampire numbers
int vampire_count = 0;

// Swap two elements in an array
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Reverse the elements in the array from start to end
void reverse(int arr[], int start, int end) {
    while (start < end) {
        swap(&arr[start], &arr[end]);
        start++;
        end--;
    }
}

// Find the next lexicographically greater permutation
bool my_next_permutation(int arr[], int size) {
    int i = size - 2;

    // Find the first element from the right that is smaller than its adjacent element
    while (i >= 0 && arr[i] >= arr[i + 1]) {
        i--;
    }

    // If no such element is found, the entire sequence is in descending order,
    // and there is no next permutation
    if (i < 0) {
        return false;
    }

    // Find the smallest element to the right of arr[i] that is greater than arr[i]
    int j = size - 1;
    while (arr[j] <= arr[i]) {
        j--;
    }

    // Swap arr[i] and arr[j]
    swap(&arr[i], &arr[j]);

    // Reverse the elements to the right of arr[i]
    reverse(arr, i + 1, size - 1);

    return true;
}


bool isVampireNumber(int num) {
    int originalNum = num;
    int numDigits = log10(num) + 1;

    if (numDigits % 2 != 0) {
        // Odd number of digits cannot be a vampire number
        return false;
    }

    int halfDigits = numDigits / 2;
    int* digits = (int*)malloc(numDigits * sizeof(int));

    // Extract digits from the number
    for (int i = numDigits - 1; i >= 0; i--) {
        digits[i] = num % 10;
        num /= 10;
    }

    // Generate all permutations of factors
    int *perm = (int*)malloc(numDigits * sizeof(int));
    for (int i = 0; i < numDigits; i++) {
        perm[i] = i;
    }

    do {
        int factor1 = 0, factor2 = 0;

        for (int i = 0; i < halfDigits; i++) {
            factor1 = factor1 * 10 + digits[perm[i]];
            factor2 = factor2 * 10 + digits[perm[halfDigits + i]];
        }

        // Check if the factors are valid and form the original number
        if (factor1 * factor2 == originalNum) {
            free(digits);
            free(perm);
            return true;
        }

    } while (my_next_permutation(perm, numDigits));

    // Free memory after the loop
    free(digits);
    free(perm);

    return false;
}


void *thread_call(void *arg) {
    struct thread_data *data = (struct thread_data*)arg;

    int x;

    FILE *outfile;
    outfile = fopen("OutFile.txt", "a");

    for (int i = 0; i < data->num_count; i++) 
    {
        x = data->numbers[i];
        
        if(isVampireNumber(x))
        {
            // Increment the counter in a thread-safe manner
            pthread_mutex_lock(&count_mutex);
            vampire_count++;
            pthread_mutex_unlock(&count_mutex);

            fprintf(outfile, "%d found by thread %d\n", x, data->thread_id);
        }
    }

    fclose(outfile);

    pthread_exit(NULL);
}

int main() 
{
    int N, M;

    FILE *infile;
    infile = fopen("InFile.txt", "r");

    if (infile == NULL) {
        fprintf(stderr, "Error opening InFile.txt\n");
        return EXIT_FAILURE;
    }

    fscanf(infile, "%d %d", &N, &M);
    fclose(infile);

    if(M>N)   //if number of threads are greater than numbers, then use only N number of threads.
    {
        M=N;
    }

    if(M!=0 && N!=0)
    {
        int num_per_thread = N / M;
        int extra_num = N % M;

        // Create an array of thread_data pointers
        struct thread_data* thread_info[M];

        // Allocate memory for each thread_data structure
        for (int i = 0; i < M; ++i) 
        {
            thread_info[i] = (struct thread_data*)malloc(sizeof(struct thread_data));

            if (thread_info[i] == NULL) 
            {
                fprintf(stderr, "Memory allocation error\n");
            // Handle error, e.g., free allocated memory and exit
            exit(EXIT_FAILURE);
            }
        }

        pthread_t Thread[M];

        // Initialize ThreadData structures
        for (int i = 0; i < M; i++) 
        {
            thread_info[i]->thread_id = i + 1;
            thread_info[i]->num_count = num_per_thread + ((i < extra_num) ? 1 : 0);
        }

        // Partitioning the threads
        int count = 0;
        int j = 0;
        for (int i = 0; i < N; i++) 
        {
            thread_info[i % M]->numbers[j] = (i + 1);

            count++;

            if (count == M) 
            {
                j++;
                count = 0;
            } 
        }

        FILE *outfile;
        outfile = fopen("OutFile.txt", "w");

        fprintf(outfile, "N = %d, M = %d\n", N, M);

        fclose(outfile);

        for (int i = 0; i < M; i++) 
        {
            pthread_create(&Thread[i], NULL, thread_call, (void *)thread_info[i]);
        }

        for (int i = 0; i < M; i++) 
        {
            pthread_join(Thread[i], NULL);
        }

        //FILE *outfile;
        outfile = fopen("OutFile.txt", "a");

        // Print the total number of vampire numbers found in the OutFile
        fprintf(outfile, "Total vampire numbers found: %d\n", vampire_count);

        fclose(outfile);

        // Free allocated memory
        for (int i = 0; i < M; ++i) 
        {
            free(thread_info[i]);
        }

    }

    return EXIT_SUCCESS;
}
