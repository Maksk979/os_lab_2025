#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stdint.h>
#include <netinet/in.h>

struct FactorialArgs {
    uint64_t begin;
    uint64_t end;
    uint64_t mod;
};

struct Server {
    char ip[255];
    int port;
};

uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod);

bool ConvertStringToUI64(const char *str, uint64_t *val);

struct Server* ReadServersFromFile(const char* filename, int* servers_count);

void FreeServers(struct Server* servers);

#endif 