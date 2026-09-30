CC=gcc
CFLAGS=-I../../lab3/src
LDFLAGS=-lpthread

all : parallel_sum

parallel_sum : parallel_sum.c sum_lib.c ../../lab3/src/utils.c
	$(CC) -o parallel_sum parallel_sum.c sum_lib.c ../../lab3/src/utils.c $(CFLAGS) $(LDFLAGS)

clean :
	rm -f parallel_sum *.o
