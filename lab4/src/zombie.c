#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void) {
  pid_t pid = fork();

  if (pid == 0) {
    printf("Ребёнок: завершаюсь\n");
    exit(0);
  }

  // сразу вызываем wait() — зомби не появится
  wait(NULL);
  printf("Родитель: wait() вызван, зомби нет\n");
  printf("Родитель: сплю 30 секунд\n");
  sleep(30);
  printf("Родитель: завершаюсь\n");
  return 0;
}
