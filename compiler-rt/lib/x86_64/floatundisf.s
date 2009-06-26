// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.

// float __floatundisf(du_int a);

#ifdef __x86_64__

.literal4
two: .single 2.0

#define REL_ADDR(_a)	(_a)(%rip)

.text
.align 4
.globl ___floatundisf
___floatundisf:
	movq		$1,			%rsi
	testq		%rdi,		%rdi
	js			1f
	cvtsi2ssq	%rdi,		%xmm0
	ret
	
1:	andq		%rdi,		%rsi
	shrq		%rdi
	orq			%rsi,		%rdi
	cvtsi2ssq	%rdi,		%xmm0
	mulss	REL_ADDR(two),	%xmm0
	ret
	
#endif // __x86_64__