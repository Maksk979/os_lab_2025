// psum.c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <sys/time.h>
#include <pthread.h>

#include "sum.h"
#include "utils.h"

void *ThreadSum(void *args) {
    struct SumArgs *sum_args = (struct SumArgs *)args;
    int result = Sum(sum_args);
    return (void *)(intptr_t)result;
}

int main(int argc, char **argv) {
    uint32_t threads_num = 0;
    uint32_t array_size = 0;
    uint32_t seed = 0;

    while (1) {
        static struct option options[] = {
            {"threads_num", required_argument, 0, 0},
            {"seed", required_argument, 0, 0},
            {"array_size", required_argument, 0, 0},
            {0, 0, 0, 0}
        };

        int option_index = 0;
        int c = getopt_long(argc, argv, "", options, &option_index);

        if (c == -1) break;

        switch (c) {
            case 0:
                if (strcmp(options[option_index].name, "threads_num") == 0) {
                    threads_num = atoi(optarg);
                } else if (strcmp(options[option_index].name, "seed") == 0) {
                    seed = atoi(optarg);
                } else if (strcmp(options[option_index].name, "array_size") == 0) {
                    array_size = atoi(optarg);
                }
                break;
            default:
                fprintf(stderr, "Unknown option\n");
                return 1;
        }
    }

    if (threads_num == 0 || array_size == 0 || seed == 0) {
        fprintf(stderr, "Usage: %s --threads_num N --seed S --array_size M\n", argv[0]);
        return 1;
    }

    if (threads_num > array_size) {
        threads_num = array_size; 
    }

    int *array = malloc(sizeof(int) * array_size);
    if (!array) {
        perror("malloc");
        return 1;
    }
    GenerateArray(array, array_size, seed);

    struct SumArgs *args = malloc(threads_num * sizeof(struct SumArgs));
    if (!args) {
        perror("malloc args");
        free(array);
        return 1;
    }

    uint32_t chunk_size = array_size / threads_num;
    for (uint32_t i = 0; i < threads_num; i++) {
        args[i].array = array;
        args[i].begin = i * chunk_size;
        args[i].end = (i == threads_num - 1) ? array_size : (i + 1) * chunk_size;
    }

    struct timeval start_time, finish_time;
    gettimeofday(&start_time, NULL);

    pthread_t *threads = malloc(threads_num * sizeof(pthread_t));
    if (!threads) {
        perror("malloc threads");
        free(array);
        free(args);
        return 1;
    }

    for (uint32_t i = 0; i < threads_num; i++) {
        if (pthread_create(&threads[i], NULL, ThreadSum, &args[i]) != 0) {
            perror("pthread_create");
            free(array);
            free(args);
            free(threads);
            return 1;
        }
    }

    long long total_sum = 0; 
    for (uint32_t i = 0; i < threads_num; i++) {
        void *result;
        if (pthread_join(threads[i], &result) != 0) {
            perror("pthread_join");
            free(array);
            free(args);
            free(threads);
            return 1;
        }
        total_sum += (long long)(intptr_t)result;
    }

    gettimeofday(&finish_time, NULL);

    double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
    elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

    free(threads);
    free(args);
    free(array);

    printf("Total: %lld\n", total_sum);
    printf("Elapsed time: %.3fms\n", elapsed_time);
    return 0;
}