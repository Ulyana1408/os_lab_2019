#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

// глобальные переменные — чтобы обработчик сигнала мог их видеть
static pid_t *child_pids = NULL;
static int child_count = 0;
static volatile sig_atomic_t timeout_flag = 0;

void alarm_handler(int sig) {
  (void)sig;
  timeout_flag = 1;
}

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  int timeout = 0;          // 0 = не задан
  bool with_files = false;

  while (true) {
    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"timeout", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            if (seed <= 0) { printf("seed must be positive\n"); return 1; }
            break;
          case 1:
            array_size = atoi(optarg);
            if (array_size <= 0) { printf("array_size must be positive\n"); return 1; }
            break;
          case 2:
            pnum = atoi(optarg);
            if (pnum <= 0) { printf("pnum must be positive\n"); return 1; }
            break;
          case 3:
            timeout = atoi(optarg);
            if (timeout < 0) { printf("timeout must be non-negative\n"); return 1; }
            break;
          case 4:
            with_files = true;
            break;
          default:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case 'f':
        with_files = true;
        break;
      case '?':
        break;
      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" [--timeout \"num\"] [--by_files]\n", argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);

  int chunk = array_size / pnum;
  int remainder = array_size % pnum;

  int pipes[2];
  if (!with_files) {
    if (pipe(pipes) == -1) { perror("pipe"); return 1; }
  }

  // === таймаут ===
  if (timeout > 0) {
    signal(SIGALRM, alarm_handler);
    alarm(timeout);
  }

  child_pids = malloc(sizeof(pid_t) * pnum);
  child_count = 0;

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork();
    if (child_pid >= 0) {
      if (child_pid == 0) {
        // ребёнок
        int begin = i * chunk + (i < remainder ? i : remainder);
        int end = begin + chunk + (i < remainder ? 1 : 0);

        struct MinMax mm = GetMinMax(array, begin, end);

        if (with_files) {
          char filename[64];
          snprintf(filename, sizeof(filename), "result_%d.txt", i);
          FILE *f = fopen(filename, "w");
          if (f) {
            fprintf(f, "%d %d\n", mm.min, mm.max);
            fclose(f);
          }
        } else {
          close(pipes[0]);
          write(pipes[1], &mm, sizeof(struct MinMax));
          close(pipes[1]);
        }
        return 0;
      } else {
        child_pids[child_count++] = child_pid;
      }
    } else {
      printf("Fork failed!\n");
      return 1;
    }
  }

  if (!with_files) close(pipes[1]);

  // === ожидание с WNOHANG + убийство по таймауту ===
  int active = child_count;

  while (active > 0) {
    pid_t done = waitpid(-1, NULL, WNOHANG);
    if (done > 0) {
      active -= 1;
      continue;
    }

    if (timeout_flag) {
      // время вышло — убиваем всех оставшихся
      for (int i = 0; i < child_count; i++) {
        if (child_pids[i] > 0) {
          kill(child_pids[i], SIGKILL);
          child_pids[i] = -1;
        }
      }
      // добираем завершения
      while (waitpid(-1, NULL, 0) > 0) {}
      active = 0;
      break;
    }

    usleep(1000); // небольшая пауза, чтобы не крутить CPU
  }

  // отменить таймер, если ещё тикает
  if (timeout > 0) alarm(0);

  // === объединение результатов ===
  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    int min = INT_MAX;
    int max = INT_MIN;

    if (with_files) {
      char filename[64];
      snprintf(filename, sizeof(filename), "result_%d.txt", i);
      FILE *f = fopen(filename, "r");
      if (f) {
        fscanf(f, "%d %d", &min, &max);
        fclose(f);
        remove(filename);
      }
    } else {
      struct MinMax mm;
      read(pipes[0], &mm, sizeof(struct MinMax));
      min = mm.min;
      max = mm.max;
    }

    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
  }

  if (!with_files) close(pipes[0]);

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);
  free(child_pids);

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}