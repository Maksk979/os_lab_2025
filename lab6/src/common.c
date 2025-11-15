#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod) {
    uint64_t result = 0;
    a = a % mod;
    while (b > 0) {
        if (b % 2 == 1)
            result = (result + a) % mod;
        a = (a * 2) % mod;
        b /= 2;
    }
    return result % mod;
}

bool ConvertStringToUI64(const char *str, uint64_t *val) {
    char *end = NULL;
    errno = 0; 
    unsigned long long i = strtoull(str, &end, 10);
    
    if (errno == ERANGE) {
        fprintf(stderr, "Out of uint64_t range: %s\n", str);
        return false;
    }

    if (errno != 0 || end == str || *end != '\0') {
        return false;
    }

    *val = i;
    return true;
}

struct Server* ReadServersFromFile(const char* filename, int* servers_count) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Cannot open servers file: %s\n", filename);
        return NULL;
    }

    struct Server *servers = NULL;
    int count = 0;
    char line[512];

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        
        if (strlen(line) == 0 || line[0] == '#')
            continue;

        char *colon = strchr(line, ':');
        if (colon == NULL) {
            fprintf(stderr, "Invalid server format in line: %s\n", line);
            continue;
        }

        *colon = '\0';
        char *ip = line;
        int port = atoi(colon + 1);

        if (port <= 0) {
            fprintf(stderr, "Invalid port in line: %s\n", line);
            continue;
        }

        struct Server *new_servers = realloc(servers, (count + 1) * sizeof(struct Server));
        if (new_servers == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            fclose(file);
            free(servers);
            return NULL;
        }
        servers = new_servers;

        strncpy(servers[count].ip, ip, sizeof(servers[count].ip) - 1);
        servers[count].ip[sizeof(servers[count].ip) - 1] = '\0';
        servers[count].port = port;
        count++;
    }

    fclose(file);
    *servers_count = count;
    return servers;
}

void FreeServers(struct Server* servers) {
    free(servers);
}