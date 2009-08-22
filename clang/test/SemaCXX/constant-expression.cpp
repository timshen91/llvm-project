// RUN: clang-cc -fsyntax-only -verify -std=c++98 %s

// C++ [expr.const]p1:
//   In several places, C++ requires expressions that evaluate to an integral
//   or enumeration constant: as array bounds, as case expressions, as
//   bit-field lengths, as enumerator initializers, as static member
//   initializers, and as integral or enumeration non-type template arguments.
//   An integral constant-expression can involve only literals, enumerators,
//   const variables or static data members of integral or enumeration types
//   initialized with constant expressions, and sizeof expressions. Floating
//   literals can appear only if they are cast to integral or enumeration types.

enum Enum { eval = 1 };
const int cval = 2;
const Enum ceval = eval;
struct Struct {
  static const int sval = 3;
  static const Enum seval = eval;
};

template <int itval, Enum etval> struct C {
  enum E {
    v1 = 1,
    v2 = eval,
    v3 = cval,
    v4 = ceval,
    v5 = Struct::sval,
    v6 = Struct::seval,
    v7 = itval,
    v8 = etval,
    v9 = (int)1.5,
    v10 = sizeof(Struct),
    v11 = true? 1 + cval * Struct::sval ^ itval / (int)1.5 - sizeof(Struct) : 0
  };
  unsigned
    b1 : 1,
    b2 : eval,
    b3 : cval,
    b4 : ceval,
    b5 : Struct::sval,
    b6 : Struct::seval,
    b7 : itval,
    b8 : etval,
    b9 : (int)1.5,
    b10 : sizeof(Struct),
    b11 : true? 1 + cval * Struct::sval ^ itval / (int)1.5 - sizeof(Struct) : 0
    ;
  static const int
    i1 = 1,
    i2 = eval,
    i3 = cval,
    i4 = ceval,
    i5 = Struct::sval,
    i6 = Struct::seval,
    i7 = itval,
    i8 = etval,
    i9 = (int)1.5,
    i10 = sizeof(Struct),
    i11 = true? 1 + cval * Struct::sval ^ itval / (int)1.5 - sizeof(Struct) : 0
    ;
  void f() {
    switch(0) {
    case    0 + 1:
    case  100 + eval:
    case  200 + cval:
    case  300 + ceval:
    case  400 + Struct::sval:
    case  500 + Struct::seval:
    case  600 + itval:
    case  700 + etval:
    case  800 + (int)1.5:
    case  900 + sizeof(Struct):
    case 1000 + (true? 1 + cval * Struct::sval ^
                 itval / (int)1.5 - sizeof(Struct) : 0):
      ;
    }
  }
  typedef C<itval, etval> T0;
};

template struct C<1, eval>;
//template struct C<cval, ceval>;
//template struct C<Struct::sval, Struct::seval>;
