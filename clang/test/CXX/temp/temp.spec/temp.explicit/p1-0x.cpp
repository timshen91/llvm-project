// RUN: %clang_cc1 -fsyntax-only -std=c++11 -verify %s

template<typename T>
struct X {
  void f() {}
};

template inline void X<int>::f(); // expected-error{{'inline'}}

// FIXME: test constexpr
