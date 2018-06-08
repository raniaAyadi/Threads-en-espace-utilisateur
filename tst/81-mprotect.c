#include <assert.h>
#include "thread.h"

int count = 0;

static void* thfunc(void* arg) {
  count++;

  thfunc(NULL);

  return NULL;
}

int main(int argc, char* argv[]) {
  thread_t th;
  int err;

  err = thread_create(&th, thfunc, NULL);
  assert(!err);

  err = thread_join(th, NULL);
  assert(!err);

  assert(count);

  return 0;
}
