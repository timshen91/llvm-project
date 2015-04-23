// RUN: %clang_tsan -O1 %s -o %t && %run %t 2>&1 | FileCheck %s
#include "test.h"
#include <sys/types.h>
#include <unistd.h>

// Setuid call used to hang because the background tsan thread did not handle
// SIGSETXID signal. Note that we don't care whether setuid call succeeds
// or not.

static void *thread(void *arg) {
  (void)arg;
  sleep(1);
  return 0;
}

int main() {
  // Create another thread just for completeness of the picture.
  pthread_t th;
  pthread_create(&th, 0, thread, 0);
  setuid(0);
  pthread_join(th, 0);
  fprintf(stderr, "DONE\n");
  return 0;
}

// CHECK: DONE
