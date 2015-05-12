//===-- lib/extendhfsf2.c - half -> single conversion -------------*- C -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//

#define SRC_HALF
#define DST_SINGLE
#include "fp_extend_impl.inc"

COMPILER_RT_ABI float __extendhfsf2(uint16_t a) {
    return __extendXfYf2__(a);
}

COMPILER_RT_ABI float __gnu_h2f_ieee(uint16_t a) {
    return __extendXfYf2__(a);
}
