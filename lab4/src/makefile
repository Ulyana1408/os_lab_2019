CC=gcc
CFLAGS=-I../../lab3/src

all : process_memory parallel_min_max

process_memory : process_memory.c
	$(CC) -o process_memory process_memory.c $(CFLAGS)

parallel_min_max : ../../lab3/src/parallel_min_max.c ../../lab3/src/find_min_max.c ../../lab3/src/utils.c
	$(CC) -o parallel_min_max ../../lab3/src/parallel_min_max.c ../../lab3/src/find_min_max.c ../../lab3/src/utils.c $(CFLAGS)

clean :
	rm -f process_memory parallel_min_max *.o
