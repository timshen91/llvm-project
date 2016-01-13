/* ===-- ucmpti2.c - Implement __ucmpti2 -----------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __ucmpti2 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */

#include "int_lib.h"

#ifdef CRT_HAS_128BIT

/* Returns:  if (a <  b) returns 0
 *           if (a == b) returns 1
 *           if (a >  b) returns 2
 */

COMPILER_RT_ABI si_int
__ucmpti2(tu_int a, tu_int b)
{
    utwords x;
    x.all = a;
    utwords y;
    y.all = b;
    if (x.s.high < y.s.high)
        return 0;
    if (x.s.high > y.s.high)
        return 2;
    if (x.s.low < y.s.low)
        return 0;
    if (x.s.low > y.s.low)
        return 2;
    return 1;
}

#endif /* CRT_HAS_128BIT */
