// RUN: %clangxx_asan -O0 %s -Fe%t
// FIXME: 'cat' is needed due to PR19744.
// RUN: not %run %t 2>&1 | cat | FileCheck %s

struct C {
  int x;
  ~C() {}
};

int main() {
  C *buffer = new C[42];
  buffer[-2].x = 42;
// CHECK: AddressSanitizer: heap-buffer-overflow on address [[ADDR:0x[0-9a-f]+]]
// CHECK: WRITE of size 4 at [[ADDR]] thread T0
// CHECK-NEXT: {{#0 .* main .*operator_array_new_with_dtor_left_oob.cc}}:[[@LINE-3]]
//
// FIXME: Currently it says "4 bytes ... left of 172-byte region",
//        should be "8 bytes ... left of 168-byte region", see
//        https://code.google.com/p/address-sanitizer/issues/detail?id=314
// CHECK: [[ADDR]] is located {{.*}} bytes to the left of 172-byte region
// CHECK-LABEL: allocated by thread T0 here:
// CHECK-NEXT: {{#0 .* operator new}}[]
// CHECK-NEXT: {{#1 .* main .*operator_array_new_with_dtor_left_oob.cc}}:[[@LINE-12]]
  delete [] buffer;
}
