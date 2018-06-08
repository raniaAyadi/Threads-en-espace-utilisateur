#include <assert.h>
#include "thread.h"

int count;

void handler() {
  count++;
}

static void* thfunc(void* arg) {
  int err = thread_signal_register(1, handler);
  assert(!err);

  while (count < 10) {
    thread_yield();
  }

  thread_exit(NULL);
}

int main(int argc, char* argv[]) {
  thread_t th;
  int err;
  int i;

  count = 0;

  err = thread_create(&th, thfunc, NULL);
  assert(!err);

  for (i = 0; i < 10; i++) {
    thread_yield();
    thread_kill(th, 1);
    thread_yield();
  }

  err = thread_join(th, NULL);
  assert(!err);

  assert(count == 10);

  return 0;
}
