#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "thread.h"

int *array;

struct arrayBounds {
  unsigned long min;
  unsigned long max; // On ne lit jamais le max
};


void afficheTab(int *array, int size){
  int i;
  printf("[");
  for (i = 0 ; i < size - 1; i++){
    printf("%d, ", array[i]);
  }
  printf("%d]\n", array[size-1]);

}


void merge(int i, int j)
{
  int mid = (i+j)/2;
  int ai = i;
  int bi = mid+1;

  int newa[j-i+1], newai = 0;
  
  while(ai <= mid && bi <= j) {
    if (array[ai] > array[bi])
      newa[newai++] = array[bi++];
    else
      newa[newai++] = array[ai++];
  }

  while(ai <= mid) {
    newa[newai++] = array[ai++];
  }

  while(bi <= j) {
    newa[newai++] = array[bi++];
  }

  for (ai = 0; ai < (j-i+1) ; ai++)
    array[i+ai] = newa[ai];
}

static void * merge_sort (void *_value){

  thread_t th1, th2;

  int err;
  
  struct arrayBounds* bounds = (struct arrayBounds*) _value;

  thread_yield();  
  
  int m = (bounds->min + bounds->max) / 2;

  struct arrayBounds boundsLeft = {
    .min = bounds->min,
    .max = m
  };

  struct arrayBounds boundsRight = {
    .min = m + 1,
    .max = bounds->max
  }; 


  if (bounds->min >= bounds->max) {
    thread_exit(NULL);
  }

  
  err = thread_create(&th1, merge_sort, (void*) &boundsLeft);
  assert(!err);
  err = thread_create(&th2, merge_sort, (void*) &boundsRight);
  assert(!err);

  err = thread_join(th1, NULL);
  assert(!err);
  err = thread_join(th2, NULL);
  assert(!err);  
  
  merge(bounds->min, bounds->max);

  thread_exit(NULL);
  
}

void is_sort(int *array, int size){
  int i;
  for (i = 1; i < size; i++){
    assert(array[i-1] <= array[i]);
  }
}

int main(int argc, char *argv[])
{
  int size;
  int i, res;
  thread_t tid;
  
  if (argc < 2) {
    printf("argument manquant: taille du tableau\n");
    return -1;
  }

  size = atoi(argv[1]);

  array = malloc(sizeof(int) * size);

  for (i = 0 ; i < size; i++){
    array[i] = random() % size;
  }
   
  struct arrayBounds bounds = {
    .min = 0,
    .max = size - 1
  };

  res = thread_create(&tid, merge_sort, &bounds);
  assert(!res);
  
  thread_join(tid, NULL);
  
  is_sort(array, size);

  printf("Tableau de taille %d bien trié\n", size);

  free(array);

  return 0;
}
