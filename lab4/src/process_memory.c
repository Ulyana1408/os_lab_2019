#include <stdio.h>
#include <stdlib.h>

extern char etext, edata, end;

int global_init = 42;
int global_uninit;

int main(void) {
  int local = 10;
  int *heap = malloc(sizeof(int));

  printf("=== Адреса сегментов ===\n");
  printf("&etext (конец кода)        = %p\n", &etext);
  printf("&edata (конец иниц. данных) = %p\n", &edata);
  printf("&end   (конец BSS)          = %p\n", &end);
  printf("&global_init (иниц. данные) = %p\n", &global_init);
  printf("&global_uninit (BSS)        = %p\n", &global_uninit);
  printf("&local (стек)               = %p\n", &local);
  printf("heap (куча)                 = %p\n", heap);

  free(heap);
  return 0;
}
