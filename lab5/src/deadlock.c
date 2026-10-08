#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

void *thread1_func(void *arg) {
  (void)arg;
  printf("Поток 1: захватываю mutex1\n");
  pthread_mutex_lock(&mutex1);

  sleep(1);

  printf("Поток 1: жду mutex2...\n");
  pthread_mutex_lock(&mutex2);

  printf("Поток 1: захватил оба\n");
  pthread_mutex_unlock(&mutex2);
  pthread_mutex_unlock(&mutex1);
  return NULL;
}

void *thread2_func(void *arg) {
  (void)arg;
  printf("Поток 2: захватываю mutex2\n");
  pthread_mutex_lock(&mutex2);

  sleep(1);

  printf("Поток 2: жду mutex1...\n");
  pthread_mutex_lock(&mutex1);

  printf("Поток 2: захватил оба\n");
  pthread_mutex_unlock(&mutex1);
  pthread_mutex_unlock(&mutex2);
  return NULL;
}

int main(void) {
  pthread_t t1, t2;

  printf("Запуск потоков...\n");

  pthread_create(&t1, NULL, thread1_func, NULL);
  pthread_create(&t2, NULL, thread2_func, NULL);

  pthread_join(t1, NULL);
  pthread_join(t2, NULL);

  printf("Программа завершилась\n");
  return 0;
}