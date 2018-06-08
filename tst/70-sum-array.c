#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "thread.h"

unsigned long* array;

struct arrayBounds {
  unsigned long min;
  unsigned long max; // On ne lit jamais le max
};

static void * sumArray(void *_value)
{
  thread_t th, th2;
  int err;
  void *res = NULL, *res2 = NULL;
  unsigned long answer = 0;

  struct arrayBounds* bounds = (struct arrayBounds*) _value;

  /* on passe un peu la main aux autres pour eviter de faire uniquement la partie gauche de l'arbre */
  thread_yield();


  if ((bounds->max - bounds->min) <= 2) { // 0 1 ou 0
    answer = array[bounds->min]; // 0

    if ((bounds->min+1) != bounds->max) { // 0 1
      answer += array[bounds->min + 1];
    }

    return (void*) answer;
  }

  struct arrayBounds boundsLeft = {
    .min = bounds->min,
    .max = bounds->min + ((bounds->max - bounds->min) / 2)
  };

  struct arrayBounds boundsRight = {
    .min = bounds->min + ((bounds->max - bounds->min) / 2),
    .max = bounds->max
  };


  err = thread_create(&th, sumArray, (void*) &boundsLeft);
  assert(!err);
  err = thread_create(&th2, sumArray, (void*) &boundsRight);
  assert(!err);

  err = thread_join(th, &res);
  assert(!err);
  err = thread_join(th2, &res2);
  assert(!err);



  return (void*)((unsigned long) res + (unsigned long) res2);
}

int main(int argc, char *argv[])
{
  unsigned long size, res;

  if (argc < 2) {
    printf("argument manquant: taille du tableau dont il faut calculer la somme\n");
    return -1;
  }

  size = atoi(argv[1]);

  array = malloc(size * sizeof(unsigned long));

  int i;
  for (i = 0; i < size; i++) {
    array[i] = i+1;
  }

  struct arrayBounds bounds = {
    .min = 0,
    .max = size
  };

  res = (unsigned long) sumArray((void *) &bounds);

  assert(res == (size * (size + 1) / 2));

  printf("Somme de 1 à %ld: %ld\n", size, res);

  free(array);

  return 0;
}
