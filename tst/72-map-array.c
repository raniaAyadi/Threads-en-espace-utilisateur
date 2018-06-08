#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "thread.h"

unsigned long size;

static void* factorial(void* i) {
  thread_yield();

  unsigned long i_long = (unsigned long) i;

  return (void*) (i_long ? (i_long * (unsigned long) factorial((void*) (i_long - 1))) : 1);
}

void map(void* array, void *(*func)(void *)) {
  thread_t* pool = malloc(size * sizeof(thread_t));
  int err;
  unsigned long* array_long = (unsigned long*) array;

  int i;
  for (i = 0; i < size; i++) {
    err = thread_create(&pool[i], func, (void*) array_long[i]);
    assert(!err);
  }

  for (i = 0; i < size; i++) {
    err = thread_join(pool[i], (void*) &array_long[i]);
    assert(!err);
  }

  free(pool);
}

int main(int argc, char *argv[])
{
  if (argc < 2) {
    printf("argument manquant: taille du tableau sur lequel il faut appliquer la factorielle\n");
    return -1;
  }

  size = atoi(argv[1]);
  unsigned long* array = malloc(size * sizeof(unsigned long));

  int i;
  for (i = 0; i < size; i++) {
    array[i] = i+1;
  }

  map(array, factorial);

  assert(array[2] == 6);

  printf("Tableau de taille %ld bien calculé\n", size);

  free(array);

  return 0;
}
