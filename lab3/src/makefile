CC = gcc
CFLAGS = -I.

all: sequential_min_max parallel_min_max launcher

sequential_min_max: utils.o find_min_max.o sequential_min_max.c
	$(CC) $(CFLAGS) -o sequential_min_max sequential_min_max.c utils.o find_min_max.o

parallel_min_max: utils.o find_min_max.o parallel_min_max.c
	$(CC) $(CFLAGS) -o parallel_min_max parallel_min_max.c utils.o find_min_max.o

launcher: launcher.c
	$(CC) $(CFLAGS) -o launcher launcher.c

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c -o utils.o utils.c

find_min_max.o: find_min_max.c find_min_max.h utils.h
	$(CC) $(CFLAGS) -c -o find_min_max.o find_min_max.c

clean:
	rm -f utils.o find_min_max.o sequential_min_max parallel_min_max

.PHONY: all clean