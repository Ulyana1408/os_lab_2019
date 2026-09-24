#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
  if (argc != 3) {
    printf("Usage: %s seed array_size\n", argv[0]);
    return 1;
  }

  pid_t pid = fork();

  if (pid < 0) {
    perror("fork");
    return 1;
  }

  if (pid == 0) {
    // ребёнок — запускаем sequential_min_max через exec
    execl("./sequential_min_max", "sequential_min_max", argv[1], argv[2], NULL);

    // если exec вернулся — значит, ошибка
    perror("execl");
    return 1;
  }

  // родитель — ждёт завершения ребёнка
  int status;
  waitpid(pid, &status, 0);

  if (WIFEXITED(status)) {
    printf("sequential_min_max завершился с кодом %d\n", WEXITSTATUS(status));
  }

  return 0;
}
