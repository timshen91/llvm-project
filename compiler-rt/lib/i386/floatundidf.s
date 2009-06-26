//===-- floatundidf.s - Implement __floatundidf for i386 ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements __floatundidf for the compiler_rt library.
//
//===----------------------------------------------------------------------===//

// double __floatundidf(du_int a);

#ifdef __i386__

.const
.align 4
twop52: .quad 0x4330000000000000
twop84_plus_twop52:
		.quad 0x4530000000100000
twop84: .quad 0x4530000000000000

#define REL_ADDR(_a)	(_a)-0b(%eax)

.text
.align 4
.globl ___floatundidf
___floatundidf:
	movss	8(%esp),						%xmm1 // high 32 bits of a
	movss	4(%esp),						%xmm0 // low 32 bits of a
	calll	0f
0:	popl	%eax
	orpd	REL_ADDR(twop84),				%xmm1 // 0x1p84 + a_hi (no rounding occurs)
	subsd	REL_ADDR(twop84_plus_twop52),	%xmm1 // a_hi - 0x1p52 (no rounding occurs)
	orpd	REL_ADDR(twop52),				%xmm0 // 0x1p52 + a_lo (no rounding occurs)
	addsd	%xmm1,							%xmm0 // a_hi + a_lo   (round happens here)
	movsd	%xmm0,						   4(%esp)
	fldl   4(%esp)
	ret
	
#endif // __i386__
