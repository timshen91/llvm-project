; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=x86_64-unknown -mattr=+sse4.2 | FileCheck %s

; Test based on pr5626 to load/store
;

%i32vec3 = type <3 x i32>
define void @add3i32(%i32vec3*  sret %ret, %i32vec3* %ap, %i32vec3* %bp)  {
; CHECK-LABEL: add3i32:
; CHECK:       # BB#0:
; CHECK-NEXT:    movdqa (%rsi), %xmm0
; CHECK-NEXT:    paddd (%rdx), %xmm0
; CHECK-NEXT:    pextrd $2, %xmm0, 8(%rdi)
; CHECK-NEXT:    movq %xmm0, (%rdi)
; CHECK-NEXT:    movq %rdi, %rax
; CHECK-NEXT:    retq
	%a = load %i32vec3, %i32vec3* %ap, align 16
	%b = load %i32vec3, %i32vec3* %bp, align 16
	%x = add %i32vec3 %a, %b
	store %i32vec3 %x, %i32vec3* %ret, align 16
	ret void
}

define void @add3i32_2(%i32vec3*  sret %ret, %i32vec3* %ap, %i32vec3* %bp)  {
; CHECK-LABEL: add3i32_2:
; CHECK:       # BB#0:
; CHECK-NEXT:    movq {{.*#+}} xmm0 = mem[0],zero
; CHECK-NEXT:    pinsrd $2, 8(%rsi), %xmm0
; CHECK-NEXT:    movq {{.*#+}} xmm1 = mem[0],zero
; CHECK-NEXT:    pinsrd $2, 8(%rdx), %xmm1
; CHECK-NEXT:    paddd %xmm0, %xmm1
; CHECK-NEXT:    pextrd $2, %xmm1, 8(%rdi)
; CHECK-NEXT:    movq %xmm1, (%rdi)
; CHECK-NEXT:    movq %rdi, %rax
; CHECK-NEXT:    retq
	%a = load %i32vec3, %i32vec3* %ap, align 8
	%b = load %i32vec3, %i32vec3* %bp, align 8
	%x = add %i32vec3 %a, %b
	store %i32vec3 %x, %i32vec3* %ret, align 8
	ret void
}

%i32vec7 = type <7 x i32>
define void @add7i32(%i32vec7*  sret %ret, %i32vec7* %ap, %i32vec7* %bp)  {
; CHECK-LABEL: add7i32:
; CHECK:       # BB#0:
; CHECK-NEXT:    movdqa (%rsi), %xmm0
; CHECK-NEXT:    movdqa 16(%rsi), %xmm1
; CHECK-NEXT:    paddd (%rdx), %xmm0
; CHECK-NEXT:    paddd 16(%rdx), %xmm1
; CHECK-NEXT:    pextrd $2, %xmm1, 24(%rdi)
; CHECK-NEXT:    movq %xmm1, 16(%rdi)
; CHECK-NEXT:    movdqa %xmm0, (%rdi)
; CHECK-NEXT:    movq %rdi, %rax
; CHECK-NEXT:    retq
	%a = load %i32vec7, %i32vec7* %ap, align 16
	%b = load %i32vec7, %i32vec7* %bp, align 16
	%x = add %i32vec7 %a, %b
	store %i32vec7 %x, %i32vec7* %ret, align 16
	ret void
}

%i32vec12 = type <12 x i32>
define void @add12i32(%i32vec12*  sret %ret, %i32vec12* %ap, %i32vec12* %bp)  {
; CHECK-LABEL: add12i32:
; CHECK:       # BB#0:
; CHECK-NEXT:    movdqa (%rsi), %xmm0
; CHECK-NEXT:    movdqa 16(%rsi), %xmm1
; CHECK-NEXT:    movdqa 32(%rsi), %xmm2
; CHECK-NEXT:    paddd (%rdx), %xmm0
; CHECK-NEXT:    paddd 16(%rdx), %xmm1
; CHECK-NEXT:    paddd 32(%rdx), %xmm2
; CHECK-NEXT:    movdqa %xmm2, 32(%rdi)
; CHECK-NEXT:    movdqa %xmm1, 16(%rdi)
; CHECK-NEXT:    movdqa %xmm0, (%rdi)
; CHECK-NEXT:    movq %rdi, %rax
; CHECK-NEXT:    retq
	%a = load %i32vec12, %i32vec12* %ap, align 16
	%b = load %i32vec12, %i32vec12* %bp, align 16
	%x = add %i32vec12 %a, %b
	store %i32vec12 %x, %i32vec12* %ret, align 16
	ret void
}


%i16vec3 = type <3 x i16>
define void @add3i16(%i16vec3* nocapture sret %ret, %i16vec3* %ap, %i16vec3* %bp) nounwind {
; CHECK-LABEL: add3i16:
; CHECK:       # BB#0:
; CHECK-NEXT:    pmovzxwd {{.*#+}} xmm0 = mem[0],zero,mem[1],zero,mem[2],zero,mem[3],zero
; CHECK-NEXT:    pmovzxwd {{.*#+}} xmm1 = mem[0],zero,mem[1],zero,mem[2],zero,mem[3],zero
; CHECK-NEXT:    paddd %xmm0, %xmm1
; CHECK-NEXT:    pextrw $4, %xmm1, 4(%rdi)
; CHECK-NEXT:    pshufb {{.*#+}} xmm1 = xmm1[0,1,4,5,8,9,12,13,8,9,12,13,12,13,14,15]
; CHECK-NEXT:    pmovzxdq {{.*#+}} xmm0 = xmm1[0],zero,xmm1[1],zero
; CHECK-NEXT:    movd %xmm0, (%rdi)
; CHECK-NEXT:    movq %rdi, %rax
; CHECK-NEXT:    retq
	%a = load %i16vec3, %i16vec3* %ap, align 16
	%b = load %i16vec3, %i16vec3* %bp, align 16
	%x = add %i16vec3 %a, %b
	store %i16vec3 %x, %i16vec3* %ret, align 16
	ret void
}

%i16vec4 = type <4 x i16>
define void @add4i16(%i16vec4* nocapture sret %ret, %i16vec4* %ap, %i16vec4* %bp) nounwind {
; CHECK-LABEL: add4i16:
; CHECK:       # BB#0:
; CHECK-NEXT:    movq {{.*#+}} xmm0 = mem[0],zero
; CHECK-NEXT:    movq {{.*#+}} xmm1 = mem[0],zero
; CHECK-NEXT:    paddw %xmm0, %xmm1
; CHECK-NEXT:    movq %xmm1, (%rdi)
; CHECK-NEXT:    movq %rdi, %rax
; CHECK-NEXT:    retq
	%a = load %i16vec4, %i16vec4* %ap, align 16
	%b = load %i16vec4, %i16vec4* %bp, align 16
	%x = add %i16vec4 %a, %b
	store %i16vec4 %x, %i16vec4* %ret, align 16
	ret void
}

%i16vec12 = type <12 x i16>
define void @add12i16(%i16vec12* nocapture sret %ret, %i16vec12* %ap, %i16vec12* %bp) nounwind {
; CHECK-LABEL: add12i16:
; CHECK:       # BB#0:
; CHECK-NEXT:    movdqa (%rsi), %xmm0
; CHECK-NEXT:    movdqa 16(%rsi), %xmm1
; CHECK-NEXT:    paddw (%rdx), %xmm0
; CHECK-NEXT:    paddw 16(%rdx), %xmm1
; CHECK-NEXT:    movq %xmm1, 16(%rdi)
; CHECK-NEXT:    movdqa %xmm0, (%rdi)
; CHECK-NEXT:    movq %rdi, %rax
; CHECK-NEXT:    retq
	%a = load %i16vec12, %i16vec12* %ap, align 16
	%b = load %i16vec12, %i16vec12* %bp, align 16
	%x = add %i16vec12 %a, %b
	store %i16vec12 %x, %i16vec12* %ret, align 16
	ret void
}

%i16vec18 = type <18 x i16>
define void @add18i16(%i16vec18* nocapture sret %ret, %i16vec18* %ap, %i16vec18* %bp) nounwind {
; CHECK-LABEL: add18i16:
; CHECK:       # BB#0:
; CHECK-NEXT:    movdqa (%rsi), %xmm0
; CHECK-NEXT:    movdqa 16(%rsi), %xmm1
; CHECK-NEXT:    movdqa 32(%rsi), %xmm2
; CHECK-NEXT:    paddw (%rdx), %xmm0
; CHECK-NEXT:    paddw 16(%rdx), %xmm1
; CHECK-NEXT:    paddw 32(%rdx), %xmm2
; CHECK-NEXT:    movd %xmm2, 32(%rdi)
; CHECK-NEXT:    movdqa %xmm1, 16(%rdi)
; CHECK-NEXT:    movdqa %xmm0, (%rdi)
; CHECK-NEXT:    movq %rdi, %rax
; CHECK-NEXT:    retq
	%a = load %i16vec18, %i16vec18* %ap, align 16
	%b = load %i16vec18, %i16vec18* %bp, align 16
	%x = add %i16vec18 %a, %b
	store %i16vec18 %x, %i16vec18* %ret, align 16
	ret void
}


%i8vec3 = type <3 x i8>
define void @add3i8(%i8vec3* nocapture sret %ret, %i8vec3* %ap, %i8vec3* %bp) nounwind {
; CHECK-LABEL: add3i8:
; CHECK:       # BB#0:
; CHECK-NEXT:    pmovzxbd {{.*#+}} xmm0 = mem[0],zero,zero,zero,mem[1],zero,zero,zero,mem[2],zero,zero,zero,mem[3],zero,zero,zero
; CHECK-NEXT:    pmovzxbd {{.*#+}} xmm1 = mem[0],zero,zero,zero,mem[1],zero,zero,zero,mem[2],zero,zero,zero,mem[3],zero,zero,zero
; CHECK-NEXT:    paddd %xmm0, %xmm1
; CHECK-NEXT:    pextrb $8, %xmm1, 2(%rdi)
; CHECK-NEXT:    pshufb {{.*#+}} xmm1 = xmm1[0,4,8,12,u,u,u,u,u,u,u,u,u,u,u,u]
; CHECK-NEXT:    pmovzxwq {{.*#+}} xmm0 = xmm1[0],zero,zero,zero,xmm1[1],zero,zero,zero
; CHECK-NEXT:    movd %xmm0, %eax
; CHECK-NEXT:    movw %ax, (%rdi)
; CHECK-NEXT:    movq %rdi, %rax
; CHECK-NEXT:    retq
	%a = load %i8vec3, %i8vec3* %ap, align 16
	%b = load %i8vec3, %i8vec3* %bp, align 16
	%x = add %i8vec3 %a, %b
	store %i8vec3 %x, %i8vec3* %ret, align 16
	ret void
}

%i8vec31 = type <31 x i8>
define void @add31i8(%i8vec31* nocapture sret %ret, %i8vec31* %ap, %i8vec31* %bp) nounwind {
; CHECK-LABEL: add31i8:
; CHECK:       # BB#0:
; CHECK-NEXT:    movdqa (%rsi), %xmm0
; CHECK-NEXT:    movdqa 16(%rsi), %xmm1
; CHECK-NEXT:    paddb (%rdx), %xmm0
; CHECK-NEXT:    paddb 16(%rdx), %xmm1
; CHECK-NEXT:    pextrb $14, %xmm1, 30(%rdi)
; CHECK-NEXT:    pextrw $6, %xmm1, 28(%rdi)
; CHECK-NEXT:    pextrd $2, %xmm1, 24(%rdi)
; CHECK-NEXT:    movq %xmm1, 16(%rdi)
; CHECK-NEXT:    movdqa %xmm0, (%rdi)
; CHECK-NEXT:    movq %rdi, %rax
; CHECK-NEXT:    retq
	%a = load %i8vec31, %i8vec31* %ap, align 16
	%b = load %i8vec31, %i8vec31* %bp, align 16
	%x = add %i8vec31 %a, %b
	store %i8vec31 %x, %i8vec31* %ret, align 16
	ret void
}


%i8vec3pack = type { <3 x i8>, i8 }
define void @rot(%i8vec3pack* nocapture sret %result, %i8vec3pack* %X, %i8vec3pack* %rot) nounwind {
; CHECK-LABEL: rot:
; CHECK:       # BB#0: # %entry
; CHECK-NEXT:    movdqa {{.*#+}} xmm0 = <0,4,8,128,u,u,u,u,u,u,u,u,u,u,u,u>
; CHECK-NEXT:    movdqa {{.*#+}} xmm1 = <158,158,158,u>
; CHECK-NEXT:    pshufb %xmm0, %xmm1
; CHECK-NEXT:    pmovzxwq {{.*#+}} xmm1 = xmm1[0],zero,zero,zero,xmm1[1],zero,zero,zero
; CHECK-NEXT:    movd %xmm1, %eax
; CHECK-NEXT:    movw %ax, (%rsi)
; CHECK-NEXT:    movb $-98, 2(%rsi)
; CHECK-NEXT:    movdqa {{.*#+}} xmm1 = <1,1,1,u>
; CHECK-NEXT:    pshufb %xmm0, %xmm1
; CHECK-NEXT:    pmovzxwq {{.*#+}} xmm0 = xmm1[0],zero,zero,zero,xmm1[1],zero,zero,zero
; CHECK-NEXT:    movd %xmm0, %eax
; CHECK-NEXT:    movw %ax, (%rdx)
; CHECK-NEXT:    movb $1, 2(%rdx)
; CHECK-NEXT:    pmovzxbd {{.*#+}} xmm0 = mem[0],zero,zero,zero,mem[1],zero,zero,zero,mem[2],zero,zero,zero,mem[3],zero,zero,zero
; CHECK-NEXT:    movdqa %xmm0, %xmm1
; CHECK-NEXT:    psrld $1, %xmm1
; CHECK-NEXT:    pblendw {{.*#+}} xmm1 = xmm1[0,1,2,3,4,5],xmm0[6,7]
; CHECK-NEXT:    pextrb $8, %xmm1, 2(%rdi)
; CHECK-NEXT:    pshufb {{.*#+}} xmm1 = xmm1[0,4,8,12,u,u,u,u,u,u,u,u,u,u,u,u]
; CHECK-NEXT:    pmovzxwq {{.*#+}} xmm0 = xmm1[0],zero,zero,zero,xmm1[1],zero,zero,zero
; CHECK-NEXT:    movd %xmm0, %eax
; CHECK-NEXT:    movw %ax, (%rdi)
; CHECK-NEXT:    movq %rdi, %rax
; CHECK-NEXT:    retq
entry:
  %storetmp = bitcast %i8vec3pack* %X to <3 x i8>*
  store <3 x i8> <i8 -98, i8 -98, i8 -98>, <3 x i8>* %storetmp
  %storetmp1 = bitcast %i8vec3pack* %rot to <3 x i8>*
  store <3 x i8> <i8 1, i8 1, i8 1>, <3 x i8>* %storetmp1
  %tmp = load %i8vec3pack, %i8vec3pack* %X
  %extractVec = extractvalue %i8vec3pack %tmp, 0
  %tmp2 = load %i8vec3pack, %i8vec3pack* %rot
  %extractVec3 = extractvalue %i8vec3pack %tmp2, 0
  %shr = lshr <3 x i8> %extractVec, %extractVec3
  %storetmp4 = bitcast %i8vec3pack* %result to <3 x i8>*
  store <3 x i8> %shr, <3 x i8>* %storetmp4
  ret void
}

