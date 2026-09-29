#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/time.h>
#include <getopt.h>

#include "sum_lib.h"
#include "../../lab3/src/utils.h"

typedef struct {
  int *array;
  int start;
  int end;
  long long result;
} ThreadArg;

void *thread_func(void *arg) {
  ThreadArg *t = (ThreadArg *)arg;
  t->result = ParallelSum(t->array, t->start, t->end);
  return NULL;
}

int main(int argc, char **argv) {
  int threads_num = -1;
  int seed = -1;
  int array_size = -1;

  while (true) {
    static struct option options[] = {{"threads_num", required_argument, 0, 0},
                                      {"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            threads_num = atoi(optarg);
            if (threads_num <= 0) { printf("threads_num must be positive\n"); return 1; }
            break;
          case 1:
            seed = atoi(optarg);
            if (seed <= 0) { printf("seed must be positive\n"); return 1; }
            break;
          case 2:
            array_size = atoi(optarg);
            if (array_size <= 0) { printf("array_size must be positive\n"); return 1; }
            break;
          default:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case '?':
        break;
      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (threads_num == -1 || seed == -1 || array_size == -1) {
    printf("Usage: %s --threads_num \"num\" --seed \"num\" --array_size \"num\"\n", argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);

  pthread_t *threads = malloc(sizeof(pthread_t) * threads_num);
  ThreadArg *args = malloc(sizeof(ThreadArg) * threads_num);

  int chunk = array_size / threads_num;
  int remainder = array_size % threads_num;

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  for (int i = 0; i < threads_num; i++) {
    int begin = i * chunk + (i < remainder ? i : remainder);
    int end = begin + chunk + (i < remainder ? 1 : 0);

    args[i].array = array;
    args[i].start = begin;
    args[i].end = end;
    args[i].result = 0;

    if (pthread_create(&threads[i], NULL, thread_func, &args[i]) != 0) {
      perror("pthread_create");
      return 1;
    }
  }

  long long total = 0;
  for (int i = 0; i < threads_num; i++) {
    pthread_join(threads[i], NULL);
    total += args[i].result;
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  printf("Sum: %lld\n", total);
  printf("Elapsed time: %fms\n", elapsed_time);

  free(array);
  free(threads);
  free(args);

  return 0;
}
