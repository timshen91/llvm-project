// RUN: %clang_cc1 %s -triple=x86_64-pc-linux -emit-llvm -o - | FileCheck --check-prefix=X86 %s
// RUN: %clang_cc1 %s -triple=wasm32 -emit-llvm -o - | FileCheck --check-prefix=WASM %s

// Test that destructors are not passed directly to __cxa_atexit when their
// signatures do not match the type of its first argument.
// e.g. ARM and WebAssembly have destructors that return this instead of void.


class Foo {
 public:
  ~Foo() {
  }
};

Foo global;

// X86 destructors have void return, and are registered directly with __cxa_atexit.
// X86: define internal void @__cxx_global_var_init()
// X86-NEXT: entry:
// X86-NEXT:   %0 = call i32 @__cxa_atexit(void (i8*)* bitcast (void (%class.Foo*)* @_ZN3FooD1Ev to void (i8*)*), i8* getelementptr inbounds (%class.Foo, %class.Foo* @global, i32 0, i32 0), i8* @__dso_handle)

// Wasm destructors return this, and use a wrapper function, which is registered
// with __cxa_atexit.
// WASM: define internal void @__cxx_global_var_init()
// WASM-NEXT: entry:
// WASM-NEXT: %0 = call i32 @__cxa_atexit(void (i8*)* @__cxx_global_array_dtor, i8* null, i8* @__dso_handle)

// WASM: define internal void @__cxx_global_array_dtor(i8*)
// WASM: %call = call %class.Foo* @_ZN3FooD1Ev(%class.Foo* @global)
