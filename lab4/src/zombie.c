#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void) {
  pid_t pid = fork();

    if (pid == 0) {
    perror("fork");
    return 1;
  }
  if (pid == 0) {
    printf("Ребёнок: PID=%d завершаюсь...\n");
    exit(0);
  }

  printf("Родитель: PID=%d, ребенок PID=%d\n", getpid(), pid);
  printf("Родитель: сплю 60 секунд, ребенок будет зомби\n");
  printf("проверьте в другом терминале:ps aux| grep Z \n")
  sleep(60); //зомби виден
  printf("Родитель: вызываю wait() - зомби исчезнет\n");
  wait(NULL); // зомби исчезает
  printf("Родитель: завершаюсь\n");
  return 0;
}
