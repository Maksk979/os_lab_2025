CC = gcc
CFLAGS = -I. 
LDFLAGS = -lpthread

all: parallel_min_max process_memory parallel_sum

parallel_min_max: utils.o find_min_max.o parallel_min_max.c
	$(CC) $(CFLAGS) -o parallel_min_max parallel_min_max.c utils.o find_min_max.o

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c -o utils.o utils.c

find_min_max.o: find_min_max.c find_min_max.h
	$(CC) $(CFLAGS) -c -o find_min_max.o find_min_max.c

process_memory: process_memory.c
	$(CC) $(CFLAGS) -o process_memory process_memory.c

parallel_sum: parallel_sum.o sum.o utils.o
	$(CC) $(CFLAGS) -o parallel_sum parallel_sum.o sum.o utils.o $(LDFLAGS)

parallel_sum.o: parallel_sum.c sum.h utils.h
	$(CC) $(CFLAGS) -c -o parallel_sum.o parallel_sum.c

sum.o: sum.c sum.h
	$(CC) $(CFLAGS) -c -o sum.o sum.c

clean:
	rm -f parallel_min_max process_memory parallel_sum *.o

.PHONY: all clean