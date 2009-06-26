//===-- fixunsdfdi.c - Implement __fixunsdfdi -----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements __fixunsdfdi for the compiler_rt library.
//
//===----------------------------------------------------------------------===//

#include "int_lib.h"

// Returns: convert a to a unsigned long long, rounding toward zero.
//          Negative values all become zero.

// Assumption: double is a IEEE 64 bit floating point type 
//             du_int is a 64 bit integral type
//             value in double is representable in du_int or is negative 
//                 (no range checking performed)

// seee eeee eeee mmmm mmmm mmmm mmmm mmmm | mmmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm

du_int
__fixunsdfdi(double a)
{
    double_bits fb;
    fb.f = a;
    int e = ((fb.u.high & 0x7FF00000) >> 20) - 1023;
    if (e < 0 || (fb.u.high & 0x80000000))
        return 0;
    udwords r;
    r.high = (fb.u.high & 0x000FFFFF) | 0x00100000;
    r.low = fb.u.low;
    if (e > 52)
        r.all <<= (e - 52);
    else
        r.all >>= (52 - e);
    return r.all;
}
