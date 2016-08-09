; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=i686-apple-darwin -mattr=+avx | FileCheck %s --check-prefix=CHECK --check-prefix=X32
; RUN: llc < %s -mtriple=x86_64-apple-darwin -mattr=+avx | FileCheck %s --check-prefix=CHECK --check-prefix=X64

define void @and_masks(<8 x float>* %a, <8 x float>* %b, <8 x float>* %c) nounwind uwtable noinline ssp {
; X32-LABEL: and_masks:
; X32:       ## BB#0:
; X32-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-NEXT:    movl {{[0-9]+}}(%esp), %ecx
; X32-NEXT:    movl {{[0-9]+}}(%esp), %edx
; X32-NEXT:    vmovups (%edx), %ymm0
; X32-NEXT:    vmovups (%ecx), %ymm1
; X32-NEXT:    vcmpltps %ymm0, %ymm1, %ymm1
; X32-NEXT:    vmovups (%eax), %ymm2
; X32-NEXT:    vcmpltps %ymm0, %ymm2, %ymm0
; X32-NEXT:    vandps %ymm1, %ymm0, %ymm0
; X32-NEXT:    vandps LCPI0_0, %ymm0, %ymm0
; X32-NEXT:    vmovaps %ymm0, (%eax)
; X32-NEXT:    vzeroupper
; X32-NEXT:    retl
;
; X64-LABEL: and_masks:
; X64:       ## BB#0:
; X64-NEXT:    vmovups (%rdi), %ymm0
; X64-NEXT:    vmovups (%rsi), %ymm1
; X64-NEXT:    vcmpltps %ymm0, %ymm1, %ymm1
; X64-NEXT:    vmovups (%rdx), %ymm2
; X64-NEXT:    vcmpltps %ymm0, %ymm2, %ymm0
; X64-NEXT:    vandps %ymm1, %ymm0, %ymm0
; X64-NEXT:    vandps {{.*}}(%rip), %ymm0, %ymm0
; X64-NEXT:    vmovaps %ymm0, (%rax)
; X64-NEXT:    vzeroupper
; X64-NEXT:    retq
  %v0 = load <8 x float>, <8 x float>* %a, align 16
  %v1 = load <8 x float>, <8 x float>* %b, align 16
  %m0 = fcmp olt <8 x float> %v1, %v0
  %v2 = load <8 x float>, <8 x float>* %c, align 16
  %m1 = fcmp olt <8 x float> %v2, %v0
  %mand = and <8 x i1> %m1, %m0
  %r = zext <8 x i1> %mand to <8 x i32>
  store <8 x i32> %r, <8 x i32>* undef, align 32
  ret void
}

define void @neg_masks(<8 x float>* %a, <8 x float>* %b, <8 x float>* %c) nounwind uwtable noinline ssp {
; X32-LABEL: neg_masks:
; X32:       ## BB#0:
; X32-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-NEXT:    movl {{[0-9]+}}(%esp), %ecx
; X32-NEXT:    vmovups (%ecx), %ymm0
; X32-NEXT:    vcmpltps (%eax), %ymm0, %ymm0
; X32-NEXT:    vmovaps {{.*#+}} ymm1 = [1,1,1,1,1,1,1,1]
; X32-NEXT:    vxorps %ymm1, %ymm0, %ymm0
; X32-NEXT:    vandps %ymm1, %ymm0, %ymm0
; X32-NEXT:    vmovaps %ymm0, (%eax)
; X32-NEXT:    vzeroupper
; X32-NEXT:    retl
;
; X64-LABEL: neg_masks:
; X64:       ## BB#0:
; X64-NEXT:    vmovups (%rsi), %ymm0
; X64-NEXT:    vcmpltps (%rdi), %ymm0, %ymm0
; X64-NEXT:    vmovaps {{.*#+}} ymm1 = [1,1,1,1,1,1,1,1]
; X64-NEXT:    vxorps %ymm1, %ymm0, %ymm0
; X64-NEXT:    vandps %ymm1, %ymm0, %ymm0
; X64-NEXT:    vmovaps %ymm0, (%rax)
; X64-NEXT:    vzeroupper
; X64-NEXT:    retq
  %v0 = load <8 x float>, <8 x float>* %a, align 16
  %v1 = load <8 x float>, <8 x float>* %b, align 16
  %m0 = fcmp olt <8 x float> %v1, %v0
  %mand = xor <8 x i1> %m0, <i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1>
  %r = zext <8 x i1> %mand to <8 x i32>
  store <8 x i32> %r, <8 x i32>* undef, align 32
  ret void
}

