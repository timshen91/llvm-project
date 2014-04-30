// RUN: %clangxx_msan -m64 -O0 -g %s -o %t && %t >%t.out 2>&1
// RUN: FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NO-ORIGINS < %t.out

// RUN: %clangxx_msan -fsanitize-memory-track-origins -m64 -O0 -g %s -o %t && %t >%t.out 2>&1
// RUN: FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-ORIGINS < %t.out

// RUN: %clangxx_msan -fsanitize-memory-track-origins=2 -m64 -O0 -g %s -o %t && %t >%t.out 2>&1
// RUN: FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-ORIGINS < %t.out

#include <sanitizer/msan_interface.h>

int main(void) {
  char *p = new char[16];
  __msan_print_shadow(p, 1);
  __msan_print_shadow(p+1, 1);
  __msan_print_shadow(p+3, 1);
  __msan_print_shadow(p+15, 1);
  __msan_print_shadow(p, 0);
  delete[] p;
  const char *q = "abc";
  __msan_print_shadow(q, 3);
  return 0;
}

// CHECK: Shadow map of [0x{{.*}}, 0x{{.*}}), 1 bytes:
// CHECK-NO-ORIGINS:   0x{{.*}}: ff...... ........ ........ ........
// CHECK-ORIGINS:   0x{{.*}}: ff...... ........ ........ ........  |A . . .|
// CHECK-ORIGINS: Origin A (origin_id {{.*}}):

// CHECK: Shadow map of [0x{{.*}}, 0x{{.*}}), 1 bytes:
// CHECK-NO-ORIGINS:   0x{{.*}}: ..ff.... ........ ........ ........
// CHECK-ORIGINS:   0x{{.*}}: ..ff.... ........ ........ ........  |A . . .|
// CHECK-ORIGINS: Origin A (origin_id {{.*}}):

// CHECK: Shadow map of [0x{{.*}}, 0x{{.*}}), 1 bytes:
// CHECK-NO-ORIGINS:   0x{{.*}}: ......ff ........ ........ ........
// CHECK-ORIGINS:   0x{{.*}}: ......ff ........ ........ ........  |A . . .|
// CHECK-ORIGINS: Origin A (origin_id {{.*}}):

// CHECK: Shadow map of [0x{{.*}}, 0x{{.*}}), 1 bytes:
// CHECK-NO-ORIGINS:   0x{{.*}}: ......ff ........ ........ ........
// CHECK-ORIGINS:   0x{{.*}}: ......ff ........ ........ ........  |A . . .|
// CHECK-ORIGINS: Origin A (origin_id {{.*}}):

// CHECK: Shadow map of [0x{{.*}}, 0x{{.*}}), 0 bytes:

// CHECK: Shadow map of [0x{{.*}}, 0x{{.*}}), 3 bytes:
// CHECK-NO-ORIGINS:   0x{{.*}}: 000000.. ........ ........ ........
// CHECK-ORIGINS:   0x{{.*}}: 000000.. ........ ........ ........  |. . . .|
