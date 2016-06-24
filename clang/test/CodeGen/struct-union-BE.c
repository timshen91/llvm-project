// RUN: %clang -O2 -target mips-linux-gnu  -EB -S -emit-llvm %s -o - | FileCheck %s -check-prefix=MIPS
// RUN: %clang -O2 -target mips64-linux-gnu  -EB -S -emit-llvm %s -o - | FileCheck %s -check-prefix=MIPS64
// RUN: %clang -O2 -target armeb-linux-gnueabihf -march=armv7a  -EB -S -emit-llvm %s -o - | FileCheck %s -check-prefix=ARM

#include <stdarg.h>
#include <stdlib.h>

struct tiny {
  char c;
};

union data {
  char c;
};

void fstr(int n, ...) {
  struct tiny x;
  va_list ap;
  va_start (ap,n);
  x = va_arg (ap, struct tiny);
  if (x.c !=  10)
    abort();
  va_end (ap);
// MIPS-NOT: %{{[0-9]+}} = getelementptr inbounds i8, i8* %argp.cur, i32 3
// MIPS64-NOT: %{{[0-9]+}} = getelementptr inbounds i8, i8* %argp.cur, i64 7
// ARM-NOT: %{{[0-9]+}} = getelementptr inbounds i8, i8* %argp.cur, i32 3
}

void funi(int n, ...) {
  union data x;
  va_list ap;
  va_start (ap,n);
  x = va_arg (ap, union data);
  if (x.c !=  10)
    abort();
  va_end (ap);
// MIPS-NOT: %{{[0-9]+}} = getelementptr inbounds i8, i8* %argp.cur, i32 3
// MIPS64-NOT: %{{[0-9]+}} = getelementptr inbounds i8, i8* %argp.cur, i64 7
// ARM-NOT: %{{[0-9]+}} = getelementptr inbounds i8, i8* %argp.cur, i32 3
}

void foo() {
  struct tiny x[3];
  union data y;
  x[0].c = 10;
  fstr(1, x[0]);
  funi(1, y);
}
