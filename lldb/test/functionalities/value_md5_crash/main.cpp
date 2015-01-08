//===-- main.cpp ------------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

class A {
public:
    virtual int foo() { return 1; }
    virtual ~A () = default;
    A() = default;
};

class B : public A {
public:
    virtual int foo() { return 2; }
    virtual ~B () = default;
    B() = default;
};

int main() {
    A* a = new B();
    a->foo();  // break here
    return 0;  // break here
}

