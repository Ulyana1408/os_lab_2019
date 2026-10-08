#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <getopt.h>
#include <stdbool.h>

long long result = 1;
long long mod = 1;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
  int start;
  int end;
} ThreadArg;

void *thread_func(void *arg) {
  ThreadArg *t = (ThreadArg *)arg;
  long long local = 1;

  for (int i = t->start; i <= t->end; i++) {
    local = (local * i) % mod;
  }

  pthread_mutex_lock(&mutex);
  result = (result * local) % mod;
  pthread_mutex_unlock(&mutex);

  return NULL;
}

int main(int argc, char **argv) {
  int k = -1;
  int pnum = -1;
  long long mod_arg = -1;

  while (true) {
    static struct option options[] = {{"pnum", required_argument, 0, 0},
                                      {"mod", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "k:", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 'k':
        k = atoi(optarg);
        if (k < 0) { printf("k must be non-negative\n"); return 1; }
        break;
      case 0:
        switch (option_index) {
          case 0:
            pnum = atoi(optarg);
            if (pnum <= 0) { printf("pnum must be positive\n"); return 1; }
            break;
          case 1:
            mod_arg = atoll(optarg);
            if (mod_arg <= 0) { printf("mod must be positive\n"); return 1; }
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

  if (k == -1 || pnum == -1 || mod_arg == -1) {
    printf("Usage: %s -k \"num\" --pnum \"num\" --mod \"num\"\n", argv[0]);
    return 1;
  }

  mod = mod_arg;

  if (k == 0 || k == 1) {
    printf("Result: %lld\n", 1 % mod);
    return 0;
  }

  pthread_t *threads = malloc(sizeof(pthread_t) * pnum);
  ThreadArg *args = malloc(sizeof(ThreadArg) * pnum);

  int chunk = k / pnum;
  int remainder = k % pnum;

  for (int i = 0; i < pnum; i++) {
    int begin = i * chunk + (i < remainder ? i : remainder) + 1;
    int end = begin + chunk + (i < remainder ? 1 : 0) - 1;
    if (end > k) end = k;

    args[i].start = begin;
    args[i].end = end;

    if (pthread_create(&threads[i], NULL, thread_func, &args[i]) != 0) {
      perror("pthread_create");
      return 1;
    }
  }

  for (int i = 0; i < pnum; i++) {
    pthread_join(threads[i], NULL);
  }

  printf("Result: %lld\n", result);

  free(threads);
  free(args);
  return 0;
}