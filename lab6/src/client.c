#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "common.h"

int main(int argc, char **argv) {
    uint64_t k = 0;
    uint64_t mod = 0;
    char servers_file[255] = {'\0'};
    bool k_set = false, mod_set = false, servers_set = false;

    while (true) {
        static struct option options[] = {
            {"k", required_argument, 0, 0},
            {"mod", required_argument, 0, 0},
            {"servers", required_argument, 0, 0},
            {0, 0, 0, 0}
        };

        int option_index = 0;
        int c = getopt_long(argc, argv, "", options, &option_index);

        if (c == -1) break;

        switch (c) {
            case 0: {
                switch (option_index) {
                    case 0:
                        if (!ConvertStringToUI64(optarg, &k)) {
                            fprintf(stderr, "Invalid k value\n");
                            return 1;
                        }
                        k_set = true;
                        break;
                    case 1:
                        if (!ConvertStringToUI64(optarg, &mod)) {
                            fprintf(stderr, "Invalid mod value\n");
                            return 1;
                        }
                        mod_set = true;
                        break;
                    case 2:
                        strncpy(servers_file, optarg, sizeof(servers_file) - 1);
                        servers_file[sizeof(servers_file) - 1] = '\0';
                        servers_set = true;
                        break;
                    default:
                        printf("Index %d is out of options\n", option_index);
                }
            } break;

            case '?':
                printf("Arguments error\n");
                break;
            default:
                fprintf(stderr, "getopt returned character code 0%o?\n", c);
        }
    }

    if (!k_set || !mod_set || !servers_set) {
        fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/file\n", argv[0]);
        return 1;
    }

    int servers_num = 0;
    struct Server* servers = ReadServersFromFile(servers_file, &servers_num);
    
    if (servers == NULL || servers_num == 0) {
        fprintf(stderr, "No valid servers found in file\n");
        return 1;
    }

    printf("Found %d servers\n", servers_num);


    uint64_t total_result = 1;
    uint64_t segment = k / servers_num;
    uint64_t remainder = k % servers_num;
    
    for (int i = 0; i < servers_num; i++) {
        struct hostent *hostname = gethostbyname(servers[i].ip);
        if (hostname == NULL) {
            fprintf(stderr, "gethostbyname failed with %s\n", servers[i].ip);
            continue;
        }

        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(servers[i].port);
        
        if (hostname->h_addr_list[0] != NULL) {
            memcpy(&server_addr.sin_addr.s_addr, hostname->h_addr_list[0], hostname->h_length);
        } else {
            fprintf(stderr, "No address found for %s\n", servers[i].ip);
            continue;
        }

        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            fprintf(stderr, "Socket creation failed for server %s:%d\n", 
                    servers[i].ip, servers[i].port);
            continue;
        }

        if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            fprintf(stderr, "Connection failed to server %s:%d\n", 
                    servers[i].ip, servers[i].port);
            close(sock);
            continue;
        }

        uint64_t begin = i * segment + 1;
        uint64_t end = begin + segment - 1;
        
        if (i < remainder) {
            begin += i;
            end += i + 1;
        } else {
            begin += remainder;
            end += remainder;
        }

        if (end > k) end = k;

        printf("Sending to server %s:%d: begin=%lu, end=%lu, mod=%lu\n",
               servers[i].ip, servers[i].port, begin, end, mod);

        char task[sizeof(uint64_t) * 3];
        memcpy(task, &begin, sizeof(uint64_t));
        memcpy(task + sizeof(uint64_t), &end, sizeof(uint64_t));
        memcpy(task + 2 * sizeof(uint64_t), &mod, sizeof(uint64_t));

        if (send(sock, task, sizeof(task), 0) < 0) {
            fprintf(stderr, "Send failed to server %s:%d\n", 
                    servers[i].ip, servers[i].port);
            close(sock);
            continue;
        }

        // Получение результата
        char response[sizeof(uint64_t)];
        ssize_t recv_result = recv(sock, response, sizeof(response), 0);
        if (recv_result < 0) {
            fprintf(stderr, "Receive failed from server %s:%d\n", 
                    servers[i].ip, servers[i].port);
            close(sock);
            continue;
        }

        if (recv_result != sizeof(response)) {
            fprintf(stderr, "Incomplete response from server %s:%d\n", 
                    servers[i].ip, servers[i].port);
            close(sock);
            continue;
        }

        uint64_t partial_result = 0;
        memcpy(&partial_result, response, sizeof(uint64_t));
        printf("Received from server %s:%d: %lu\n", 
               servers[i].ip, servers[i].port, partial_result);

        total_result = MultModulo(total_result, partial_result, mod);
        close(sock);
    }

    printf("Final result: %lu! mod %lu = %lu\n", k, mod, total_result);

    FreeServers(servers);
    return 0;
}