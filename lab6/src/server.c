#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include "common.h"

uint64_t Factorial(const struct FactorialArgs *args) {
    uint64_t ans = 1;
    for (uint64_t i = args->begin; i <= args->end; i++) {
        ans = MultModulo(ans, i, args->mod);
    }
    return ans;
}

void *ThreadFactorial(void *args) {
    struct FactorialArgs *fargs = (struct FactorialArgs *)args;
    uint64_t *result = malloc(sizeof(uint64_t));
    if (result != NULL) {
        *result = Factorial(fargs);
    }
    return (void *)result;
}

int main(int argc, char **argv) {
    int tnum = 0;
    int port = 0;
    bool tnum_set = false, port_set = false;

    while (true) {
        static struct option options[] = {
            {"port", required_argument, 0, 0},
            {"tnum", required_argument, 0, 0},
            {0, 0, 0, 0}
        };

        int option_index = 0;
        int c = getopt_long(argc, argv, "", options, &option_index);

        if (c == -1) break;

        switch (c) {
            case 0: {
                switch (option_index) {
                    case 0:
                        port = atoi(optarg);
                        if (port <= 0) {
                            fprintf(stderr, "Port must be positive number\n");
                            return 1;
                        }
                        port_set = true;
                        break;
                    case 1:
                        tnum = atoi(optarg);
                        if (tnum <= 0) {
                            fprintf(stderr, "Thread number must be positive\n");
                            return 1;
                        }
                        tnum_set = true;
                        break;
                    default:
                        printf("Index %d is out of options\n", option_index);
                }
            } break;

            case '?':
                printf("Unknown argument\n");
                break;
            default:
                fprintf(stderr, "getopt returned character code 0%o?\n", c);
        }
    }

    if (!port_set || !tnum_set) {
        fprintf(stderr, "Using: %s --port 20001 --tnum 4\n", argv[0]);
        return 1;
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        fprintf(stderr, "Can not create server socket!");
        return 1;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons((uint16_t)port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    int opt_val = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));

    if (bind(server_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        fprintf(stderr, "Can not bind to socket!");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 128) < 0) {
        fprintf(stderr, "Could not listen on socket\n");
        close(server_fd);
        return 1;
    }

    printf("Server listening at %d\n", port);

    while (true) {
        struct sockaddr_in client;
        socklen_t client_len = sizeof(client);
        int client_fd = accept(server_fd, (struct sockaddr *)&client, &client_len);

        if (client_fd < 0) {
            fprintf(stderr, "Could not establish new connection\n");
            continue;
        }

        while (true) {
            unsigned int buffer_size = sizeof(uint64_t) * 3;
            char from_client[buffer_size];
            ssize_t read_bytes = recv(client_fd, from_client, buffer_size, 0);

            if (read_bytes == 0) break;
            if (read_bytes < 0) {
                fprintf(stderr, "Client read failed\n");
                break;
            }
            if ((size_t)read_bytes < buffer_size) {
                fprintf(stderr, "Client send wrong data format\n");
                break;
            }

            // Извлечение данных от клиента
            struct FactorialArgs main_args;
            memcpy(&main_args.begin, from_client, sizeof(uint64_t));
            memcpy(&main_args.end, from_client + sizeof(uint64_t), sizeof(uint64_t));
            memcpy(&main_args.mod, from_client + 2 * sizeof(uint64_t), sizeof(uint64_t));

            printf("Receive: %lu %lu %lu\n", main_args.begin, main_args.end, main_args.mod);

            // Распределение работы между потоками
            struct FactorialArgs *args = malloc(tnum * sizeof(struct FactorialArgs));
            pthread_t *threads = malloc(tnum * sizeof(pthread_t));

            if (args == NULL || threads == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                free(args);
                free(threads);
                break;
            }

            uint64_t segment = (main_args.end - main_args.begin + 1) / tnum;
            for (uint32_t i = 0; i < (uint32_t)tnum; i++) {
                args[i].begin = main_args.begin + i * segment;
                args[i].end = (i == (uint32_t)tnum - 1) ? main_args.end : args[i].begin + segment - 1;
                args[i].mod = main_args.mod;

                if (pthread_create(&threads[i], NULL, ThreadFactorial, (void *)&args[i])) {
                    fprintf(stderr, "Error: pthread_create failed!\n");
                    free(args);
                    free(threads);
                    break;
                }
            }

            uint64_t total = 1;
            for (uint32_t i = 0; i < (uint32_t)tnum; i++) {
                uint64_t *result = NULL;
                pthread_join(threads[i], (void **)&result);
                if (result != NULL) {
                    total = MultModulo(total, *result, main_args.mod);
                    free(result);
                }
            }

            free(args);
            free(threads);

            printf("Total: %lu\n", total);

            char buffer[sizeof(total)];
            memcpy(buffer, &total, sizeof(total));
            if (send(client_fd, buffer, sizeof(total), 0) < 0) {
                fprintf(stderr, "Can't send data to client\n");
                break;
            }
        }

        close(client_fd);
    }

    close(server_fd);
    return 0;
}