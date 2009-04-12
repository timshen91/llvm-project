// RUN: clang-cc %s -fsyntax-only -verify -pedantic

extern int a1[];

void f0();
void f1(int [*]);
void f2(int [const *]);
void f3(int [volatile const*]);
int f4(*XX)(void); /* expected-error {{cannot return}} expected-warning {{type specifier missing, defaults to 'int'}} */

char ((((*X))));

void (*signal(int, void (*)(int)))(int);

int a, ***C, * const D, B(int);

int *A;

struct str;

int test2(int *P, int A) {
  struct str;

  // Hard case for array decl, not Array[*].
  int Array[*(int*)P+A];
}

typedef int atype;
int test3(x, 
          atype         /* expected-error {{unexpected type name 'atype': expected identifier}} */
         ) int x, atype; {}

int test4(x, x) int x; {} /* expected-error {{redefinition of parameter 'x'}} */


// PR3031
int (test5), ;  // expected-error {{expected identifier or '('}}



// PR3963 & rdar://6759604 - test error recovery for mistyped "typenames".

foo_t *d;      // expected-error {{unknown type name 'foo_t'}}
foo_t a = 4;   // expected-error {{unknown type name 'foo_t'}}
int test6() { return a; }  // a should be declared.

// Use of tagged type without tag. rdar://6783347
struct xyz { int y; };
enum myenum { ASDFAS };
xyz b;         // expected-error {{use of tagged type 'xyz' without 'struct' tag}}
myenum c;      // expected-error {{use of tagged type 'myenum' without 'enum' tag}}

float *test7() {
  // We should recover 'b' by parsing it with a valid type of "struct xyz", which
  // allows us to diagnose other bad things done with y, such as this.
  return &b.y;   // expected-warning {{incompatible pointer types returning 'int *', expected 'float *'}}
}


// Verify that implicit int still works.
static f;      // expected-warning {{type specifier missing, defaults to 'int'}}
static g = 4;  // expected-warning {{type specifier missing, defaults to 'int'}}
static h        // expected-warning {{type specifier missing, defaults to 'int'}} 
      __asm__("foo"); // expected-warning {{extension used}}

