#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

unsigned long long result = 1;
unsigned long long factorial_result = 1; 
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* calculate_part(void* arg) {
    int thread_id = *(int*)arg;
    int k = ((int*)arg)[1];
    int mod = ((int*)arg)[2];
    int pnum = ((int*)arg)[3];
    
    unsigned long long partial = 1;
    unsigned long long partial_factorial = 1;
    
    for (int i = thread_id + 1; i <= k; i += pnum) {
        partial = (partial * i) % mod;
        partial_factorial = partial_factorial * i;
    }
    
    pthread_mutex_lock(&mutex);
    result = (result * partial) % mod;
    factorial_result = factorial_result * partial_factorial;
    pthread_mutex_unlock(&mutex);
    
    return NULL;
}

int main(int argc, char* argv[]) {
    int k = 0, pnum = 0, mod = 0;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-k") == 0 && i + 1 < argc) {
            k = atoi(argv[i + 1]);
            i++; 
        }
        else if (strncmp(argv[i], "--pnum=", 7) == 0) {
            pnum = atoi(argv[i] + 7);
        }
        else if (strncmp(argv[i], "--mod=", 6) == 0) {
            mod = atoi(argv[i] + 6);
        }
    }
    
    if (k == 0 || pnum == 0 || mod == 0) {
        printf("Usage: %s -k <number> --pnum=<threads> --mod=<modulus>\n", argv[0]);
        printf("Example: %s -k 10 --pnum=4 --mod=10\n", argv[0]);
        return 1;
    }
    
    printf("Calculating %d! mod %d using %d threads\n", k, mod, pnum);
    
    pthread_t threads[pnum];
    int thread_data[pnum][4]; 
    
    for (int i = 0; i < pnum; i++) {
        thread_data[i][0] = i;      
        thread_data[i][1] = k;   
        thread_data[i][2] = mod;   
        thread_data[i][3] = pnum;   
        
        pthread_create(&threads[i], NULL, calculate_part, thread_data[i]);
    }
    
    for (int i = 0; i < pnum; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("\nResults:\n");
    printf("%d! = %llu\n", k, factorial_result);
    printf("%d! mod %d = %llu\n", k, mod, result);
    
    return 0;
}