// RUN: %clang -target i686-pc-windows-msvc   -S -emit-llvm %s -o - | FileCheck %s --check-prefix=DEFAULT
// RUN: %clang -target i686-pc-windows-msvc19 -S -emit-llvm %s -o - | FileCheck %s --check-prefix=TARGET-19
// RUN: %clang -target i686-pc-windows-msvc   -S -emit-llvm %s -o - -fms-compatibility-version=19 | FileCheck %s --check-prefix=OVERRIDE-19

// DEFAULT:     target triple = "i686-pc-windows-msvc18.0.0"
// TARGET-19:   target triple = "i686-pc-windows-msvc19.0.0"
// OVERRIDE-19: target triple = "i686-pc-windows-msvc19.0.0"
