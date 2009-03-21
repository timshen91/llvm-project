// RUN: clang -arch i386 -emit-llvm -o %t %s &&
// RUN: grep '@_Z2f0i' %t &&
// RUN: grep '@_Z2f0l' %t &&

// Make sure we mangle overloadable, even in C system headers.

# 1 "somesystemheader.h" 1 3 4
void __attribute__((__overloadable__)) f0(int a) {}
void __attribute__((__overloadable__)) f0(long b) {}



// These should get merged.
void foo() __asm__("bar");
void foo2() __asm__("bar");

// RUN: grep '@"\\01foo"' %t &&
// RUN: grep '@"\\01bar"' %t

int nux __asm__("foo");
extern float nux2 __asm__("foo");

int test() { 
  foo();
  foo2();
  
  return nux + nux2;
}
