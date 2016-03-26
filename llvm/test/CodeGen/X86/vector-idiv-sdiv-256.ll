; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=x86_64-unknown-unknown -mattr=+avx | FileCheck %s --check-prefix=AVX --check-prefix=AVX1
; RUN: llc < %s -mtriple=x86_64-unknown-unknown -mattr=+avx2 | FileCheck %s --check-prefix=AVX --check-prefix=AVX2

;
; sdiv by 7
;

define <4 x i64> @test_div7_4i64(<4 x i64> %a) nounwind {
; AVX1-LABEL: test_div7_4i64:
; AVX1:       # BB#0:
; AVX1-NEXT:    vextractf128 $1, %ymm0, %xmm1
; AVX1-NEXT:    vpextrq $1, %xmm1, %rax
; AVX1-NEXT:    movabsq $5270498306774157605, %rcx # imm = 0x4924924924924925
; AVX1-NEXT:    imulq %rcx
; AVX1-NEXT:    movq %rdx, %rax
; AVX1-NEXT:    shrq $63, %rax
; AVX1-NEXT:    sarq %rdx
; AVX1-NEXT:    addq %rax, %rdx
; AVX1-NEXT:    vmovq %rdx, %xmm2
; AVX1-NEXT:    vmovq %xmm1, %rax
; AVX1-NEXT:    imulq %rcx
; AVX1-NEXT:    movq %rdx, %rax
; AVX1-NEXT:    shrq $63, %rax
; AVX1-NEXT:    sarq %rdx
; AVX1-NEXT:    addq %rax, %rdx
; AVX1-NEXT:    vmovq %rdx, %xmm1
; AVX1-NEXT:    vpunpcklqdq {{.*#+}} xmm1 = xmm1[0],xmm2[0]
; AVX1-NEXT:    vpextrq $1, %xmm0, %rax
; AVX1-NEXT:    imulq %rcx
; AVX1-NEXT:    movq %rdx, %rax
; AVX1-NEXT:    shrq $63, %rax
; AVX1-NEXT:    sarq %rdx
; AVX1-NEXT:    addq %rax, %rdx
; AVX1-NEXT:    vmovq %rdx, %xmm2
; AVX1-NEXT:    vmovq %xmm0, %rax
; AVX1-NEXT:    imulq %rcx
; AVX1-NEXT:    movq %rdx, %rax
; AVX1-NEXT:    shrq $63, %rax
; AVX1-NEXT:    sarq %rdx
; AVX1-NEXT:    addq %rax, %rdx
; AVX1-NEXT:    vmovq %rdx, %xmm0
; AVX1-NEXT:    vpunpcklqdq {{.*#+}} xmm0 = xmm0[0],xmm2[0]
; AVX1-NEXT:    vinsertf128 $1, %xmm1, %ymm0, %ymm0
; AVX1-NEXT:    retq
;
; AVX2-LABEL: test_div7_4i64:
; AVX2:       # BB#0:
; AVX2-NEXT:    vextracti128 $1, %ymm0, %xmm1
; AVX2-NEXT:    vpextrq $1, %xmm1, %rax
; AVX2-NEXT:    movabsq $5270498306774157605, %rcx # imm = 0x4924924924924925
; AVX2-NEXT:    imulq %rcx
; AVX2-NEXT:    movq %rdx, %rax
; AVX2-NEXT:    shrq $63, %rax
; AVX2-NEXT:    sarq %rdx
; AVX2-NEXT:    addq %rax, %rdx
; AVX2-NEXT:    vmovq %rdx, %xmm2
; AVX2-NEXT:    vmovq %xmm1, %rax
; AVX2-NEXT:    imulq %rcx
; AVX2-NEXT:    movq %rdx, %rax
; AVX2-NEXT:    shrq $63, %rax
; AVX2-NEXT:    sarq %rdx
; AVX2-NEXT:    addq %rax, %rdx
; AVX2-NEXT:    vmovq %rdx, %xmm1
; AVX2-NEXT:    vpunpcklqdq {{.*#+}} xmm1 = xmm1[0],xmm2[0]
; AVX2-NEXT:    vpextrq $1, %xmm0, %rax
; AVX2-NEXT:    imulq %rcx
; AVX2-NEXT:    movq %rdx, %rax
; AVX2-NEXT:    shrq $63, %rax
; AVX2-NEXT:    sarq %rdx
; AVX2-NEXT:    addq %rax, %rdx
; AVX2-NEXT:    vmovq %rdx, %xmm2
; AVX2-NEXT:    vmovq %xmm0, %rax
; AVX2-NEXT:    imulq %rcx
; AVX2-NEXT:    movq %rdx, %rax
; AVX2-NEXT:    shrq $63, %rax
; AVX2-NEXT:    sarq %rdx
; AVX2-NEXT:    addq %rax, %rdx
; AVX2-NEXT:    vmovq %rdx, %xmm0
; AVX2-NEXT:    vpunpcklqdq {{.*#+}} xmm0 = xmm0[0],xmm2[0]
; AVX2-NEXT:    vinserti128 $1, %xmm1, %ymm0, %ymm0
; AVX2-NEXT:    retq
  %res = sdiv <4 x i64> %a, <i64 7, i64 7, i64 7, i64 7>
  ret <4 x i64> %res
}

define <8 x i32> @test_div7_8i32(<8 x i32> %a) nounwind {
; AVX1-LABEL: test_div7_8i32:
; AVX1:       # BB#0:
; AVX1-NEXT:    vextractf128 $1, %ymm0, %xmm1
; AVX1-NEXT:    vpextrd $1, %xmm1, %eax
; AVX1-NEXT:    cltq
; AVX1-NEXT:    imulq $-1840700269, %rax, %rcx # imm = 0xFFFFFFFF92492493
; AVX1-NEXT:    shrq $32, %rcx
; AVX1-NEXT:    addl %ecx, %eax
; AVX1-NEXT:    movl %eax, %ecx
; AVX1-NEXT:    shrl $31, %ecx
; AVX1-NEXT:    sarl $2, %eax
; AVX1-NEXT:    addl %ecx, %eax
; AVX1-NEXT:    vmovd %xmm1, %ecx
; AVX1-NEXT:    movslq %ecx, %rcx
; AVX1-NEXT:    imulq $-1840700269, %rcx, %rdx # imm = 0xFFFFFFFF92492493
; AVX1-NEXT:    shrq $32, %rdx
; AVX1-NEXT:    addl %edx, %ecx
; AVX1-NEXT:    movl %ecx, %edx
; AVX1-NEXT:    shrl $31, %edx
; AVX1-NEXT:    sarl $2, %ecx
; AVX1-NEXT:    addl %edx, %ecx
; AVX1-NEXT:    vmovd %ecx, %xmm2
; AVX1-NEXT:    vpinsrd $1, %eax, %xmm2, %xmm2
; AVX1-NEXT:    vpextrd $2, %xmm1, %eax
; AVX1-NEXT:    cltq
; AVX1-NEXT:    imulq $-1840700269, %rax, %rcx # imm = 0xFFFFFFFF92492493
; AVX1-NEXT:    shrq $32, %rcx
; AVX1-NEXT:    addl %ecx, %eax
; AVX1-NEXT:    movl %eax, %ecx
; AVX1-NEXT:    shrl $31, %ecx
; AVX1-NEXT:    sarl $2, %eax
; AVX1-NEXT:    addl %ecx, %eax
; AVX1-NEXT:    vpinsrd $2, %eax, %xmm2, %xmm2
; AVX1-NEXT:    vpextrd $3, %xmm1, %eax
; AVX1-NEXT:    cltq
; AVX1-NEXT:    imulq $-1840700269, %rax, %rcx # imm = 0xFFFFFFFF92492493
; AVX1-NEXT:    shrq $32, %rcx
; AVX1-NEXT:    addl %ecx, %eax
; AVX1-NEXT:    movl %eax, %ecx
; AVX1-NEXT:    shrl $31, %ecx
; AVX1-NEXT:    sarl $2, %eax
; AVX1-NEXT:    addl %ecx, %eax
; AVX1-NEXT:    vpinsrd $3, %eax, %xmm2, %xmm1
; AVX1-NEXT:    vpextrd $1, %xmm0, %eax
; AVX1-NEXT:    cltq
; AVX1-NEXT:    imulq $-1840700269, %rax, %rcx # imm = 0xFFFFFFFF92492493
; AVX1-NEXT:    shrq $32, %rcx
; AVX1-NEXT:    addl %ecx, %eax
; AVX1-NEXT:    movl %eax, %ecx
; AVX1-NEXT:    shrl $31, %ecx
; AVX1-NEXT:    sarl $2, %eax
; AVX1-NEXT:    addl %ecx, %eax
; AVX1-NEXT:    vmovd %xmm0, %ecx
; AVX1-NEXT:    movslq %ecx, %rcx
; AVX1-NEXT:    imulq $-1840700269, %rcx, %rdx # imm = 0xFFFFFFFF92492493
; AVX1-NEXT:    shrq $32, %rdx
; AVX1-NEXT:    addl %edx, %ecx
; AVX1-NEXT:    movl %ecx, %edx
; AVX1-NEXT:    shrl $31, %edx
; AVX1-NEXT:    sarl $2, %ecx
; AVX1-NEXT:    addl %edx, %ecx
; AVX1-NEXT:    vmovd %ecx, %xmm2
; AVX1-NEXT:    vpinsrd $1, %eax, %xmm2, %xmm2
; AVX1-NEXT:    vpextrd $2, %xmm0, %eax
; AVX1-NEXT:    cltq
; AVX1-NEXT:    imulq $-1840700269, %rax, %rcx # imm = 0xFFFFFFFF92492493
; AVX1-NEXT:    shrq $32, %rcx
; AVX1-NEXT:    addl %ecx, %eax
; AVX1-NEXT:    movl %eax, %ecx
; AVX1-NEXT:    shrl $31, %ecx
; AVX1-NEXT:    sarl $2, %eax
; AVX1-NEXT:    addl %ecx, %eax
; AVX1-NEXT:    vpinsrd $2, %eax, %xmm2, %xmm2
; AVX1-NEXT:    vpextrd $3, %xmm0, %eax
; AVX1-NEXT:    cltq
; AVX1-NEXT:    imulq $-1840700269, %rax, %rcx # imm = 0xFFFFFFFF92492493
; AVX1-NEXT:    shrq $32, %rcx
; AVX1-NEXT:    addl %ecx, %eax
; AVX1-NEXT:    movl %eax, %ecx
; AVX1-NEXT:    shrl $31, %ecx
; AVX1-NEXT:    sarl $2, %eax
; AVX1-NEXT:    addl %ecx, %eax
; AVX1-NEXT:    vpinsrd $3, %eax, %xmm2, %xmm0
; AVX1-NEXT:    vinsertf128 $1, %xmm1, %ymm0, %ymm0
; AVX1-NEXT:    retq
;
; AVX2-LABEL: test_div7_8i32:
; AVX2:       # BB#0:
; AVX2-NEXT:    vpbroadcastd {{.*}}(%rip), %ymm1
; AVX2-NEXT:    vpshufd {{.*#+}} ymm2 = ymm1[1,1,3,3,5,5,7,7]
; AVX2-NEXT:    vpshufd {{.*#+}} ymm3 = ymm0[1,1,3,3,5,5,7,7]
; AVX2-NEXT:    vpmuldq %ymm2, %ymm3, %ymm2
; AVX2-NEXT:    vpmuldq %ymm1, %ymm0, %ymm1
; AVX2-NEXT:    vpshufd {{.*#+}} ymm1 = ymm1[1,1,3,3,5,5,7,7]
; AVX2-NEXT:    vpblendd {{.*#+}} ymm1 = ymm1[0],ymm2[1],ymm1[2],ymm2[3],ymm1[4],ymm2[5],ymm1[6],ymm2[7]
; AVX2-NEXT:    vpaddd %ymm0, %ymm1, %ymm0
; AVX2-NEXT:    vpsrld $31, %ymm0, %ymm1
; AVX2-NEXT:    vpsrad $2, %ymm0, %ymm0
; AVX2-NEXT:    vpaddd %ymm1, %ymm0, %ymm0
; AVX2-NEXT:    retq
  %res = sdiv <8 x i32> %a, <i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7>
  ret <8 x i32> %res
}

define <16 x i16> @test_div7_16i16(<16 x i16> %a) nounwind {
; AVX1-LABEL: test_div7_16i16:
; AVX1:       # BB#0:
; AVX1-NEXT:    vextractf128 $1, %ymm0, %xmm1
; AVX1-NEXT:    vmovdqa {{.*#+}} xmm2 = [18725,18725,18725,18725,18725,18725,18725,18725]
; AVX1-NEXT:    vpmulhw %xmm2, %xmm1, %xmm1
; AVX1-NEXT:    vpsrlw $15, %xmm1, %xmm3
; AVX1-NEXT:    vpsraw $1, %xmm1, %xmm1
; AVX1-NEXT:    vpaddw %xmm3, %xmm1, %xmm1
; AVX1-NEXT:    vpmulhw %xmm2, %xmm0, %xmm0
; AVX1-NEXT:    vpsrlw $15, %xmm0, %xmm2
; AVX1-NEXT:    vpsraw $1, %xmm0, %xmm0
; AVX1-NEXT:    vpaddw %xmm2, %xmm0, %xmm0
; AVX1-NEXT:    vinsertf128 $1, %xmm1, %ymm0, %ymm0
; AVX1-NEXT:    retq
;
; AVX2-LABEL: test_div7_16i16:
; AVX2:       # BB#0:
; AVX2-NEXT:    vpmulhw {{.*}}(%rip), %ymm0, %ymm0
; AVX2-NEXT:    vpsrlw $15, %ymm0, %ymm1
; AVX2-NEXT:    vpsraw $1, %ymm0, %ymm0
; AVX2-NEXT:    vpaddw %ymm1, %ymm0, %ymm0
; AVX2-NEXT:    retq
  %res = sdiv <16 x i16> %a, <i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7>
  ret <16 x i16> %res
}

define <32 x i8> @test_div7_32i8(<32 x i8> %a) nounwind {
; AVX1-LABEL: test_div7_32i8:
; AVX1:       # BB#0:
; AVX1-NEXT:    vextractf128 $1, %ymm0, %xmm1
; AVX1-NEXT:    vpmovsxbw %xmm1, %xmm2
; AVX1-NEXT:    vpmovsxbw {{.*}}(%rip), %xmm3
; AVX1-NEXT:    vpmullw %xmm3, %xmm2, %xmm2
; AVX1-NEXT:    vpsrlw $8, %xmm2, %xmm2
; AVX1-NEXT:    vpshufd {{.*#+}} xmm4 = xmm1[2,3,0,1]
; AVX1-NEXT:    vpmovsxbw %xmm4, %xmm4
; AVX1-NEXT:    vpmullw %xmm3, %xmm4, %xmm4
; AVX1-NEXT:    vpsrlw $8, %xmm4, %xmm4
; AVX1-NEXT:    vpackuswb %xmm4, %xmm2, %xmm2
; AVX1-NEXT:    vpaddb %xmm1, %xmm2, %xmm1
; AVX1-NEXT:    vpsrlw $7, %xmm1, %xmm2
; AVX1-NEXT:    vmovdqa {{.*#+}} xmm4 = [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]
; AVX1-NEXT:    vpand %xmm4, %xmm2, %xmm2
; AVX1-NEXT:    vpsrlw $2, %xmm1, %xmm1
; AVX1-NEXT:    vmovdqa {{.*#+}} xmm5 = [63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63]
; AVX1-NEXT:    vpand %xmm5, %xmm1, %xmm1
; AVX1-NEXT:    vmovdqa {{.*#+}} xmm6 = [32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32]
; AVX1-NEXT:    vpxor %xmm6, %xmm1, %xmm1
; AVX1-NEXT:    vpsubb %xmm6, %xmm1, %xmm1
; AVX1-NEXT:    vpaddb %xmm2, %xmm1, %xmm1
; AVX1-NEXT:    vpmovsxbw %xmm0, %xmm2
; AVX1-NEXT:    vpmullw %xmm3, %xmm2, %xmm2
; AVX1-NEXT:    vpsrlw $8, %xmm2, %xmm2
; AVX1-NEXT:    vpshufd {{.*#+}} xmm7 = xmm0[2,3,0,1]
; AVX1-NEXT:    vpmovsxbw %xmm7, %xmm7
; AVX1-NEXT:    vpmullw %xmm3, %xmm7, %xmm3
; AVX1-NEXT:    vpsrlw $8, %xmm3, %xmm3
; AVX1-NEXT:    vpackuswb %xmm3, %xmm2, %xmm2
; AVX1-NEXT:    vpaddb %xmm0, %xmm2, %xmm0
; AVX1-NEXT:    vpsrlw $7, %xmm0, %xmm2
; AVX1-NEXT:    vpand %xmm4, %xmm2, %xmm2
; AVX1-NEXT:    vpsrlw $2, %xmm0, %xmm0
; AVX1-NEXT:    vpand %xmm5, %xmm0, %xmm0
; AVX1-NEXT:    vpxor %xmm6, %xmm0, %xmm0
; AVX1-NEXT:    vpsubb %xmm6, %xmm0, %xmm0
; AVX1-NEXT:    vpaddb %xmm2, %xmm0, %xmm0
; AVX1-NEXT:    vinsertf128 $1, %xmm1, %ymm0, %ymm0
; AVX1-NEXT:    retq
;
; AVX2-LABEL: test_div7_32i8:
; AVX2:       # BB#0:
; AVX2-NEXT:    vmovdqa {{.*#+}} ymm1 = [147,147,147,147,147,147,147,147,147,147,147,147,147,147,147,147,147,147,147,147,147,147,147,147,147,147,147,147,147,147,147,147]
; AVX2-NEXT:    vextracti128 $1, %ymm1, %xmm2
; AVX2-NEXT:    vpmovsxbw %xmm2, %ymm2
; AVX2-NEXT:    vextracti128 $1, %ymm0, %xmm3
; AVX2-NEXT:    vpmovsxbw %xmm3, %ymm3
; AVX2-NEXT:    vpmullw %ymm2, %ymm3, %ymm2
; AVX2-NEXT:    vpsrlw $8, %ymm2, %ymm2
; AVX2-NEXT:    vpmovsxbw %xmm1, %ymm1
; AVX2-NEXT:    vpmovsxbw %xmm0, %ymm3
; AVX2-NEXT:    vpmullw %ymm1, %ymm3, %ymm1
; AVX2-NEXT:    vpsrlw $8, %ymm1, %ymm1
; AVX2-NEXT:    vperm2i128 {{.*#+}} ymm3 = ymm1[2,3],ymm2[2,3]
; AVX2-NEXT:    vinserti128 $1, %xmm2, %ymm1, %ymm1
; AVX2-NEXT:    vpackuswb %ymm3, %ymm1, %ymm1
; AVX2-NEXT:    vpaddb %ymm0, %ymm1, %ymm0
; AVX2-NEXT:    vpsrlw $2, %ymm0, %ymm1
; AVX2-NEXT:    vpand {{.*}}(%rip), %ymm1, %ymm1
; AVX2-NEXT:    vmovdqa {{.*#+}} ymm2 = [32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32]
; AVX2-NEXT:    vpxor %ymm2, %ymm1, %ymm1
; AVX2-NEXT:    vpsubb %ymm2, %ymm1, %ymm1
; AVX2-NEXT:    vpsrlw $7, %ymm0, %ymm0
; AVX2-NEXT:    vpand {{.*}}(%rip), %ymm0, %ymm0
; AVX2-NEXT:    vpaddb %ymm0, %ymm1, %ymm0
; AVX2-NEXT:    retq
  %res = sdiv <32 x i8> %a, <i8 7, i8 7, i8 7, i8 7,i8 7, i8 7, i8 7, i8 7, i8 7, i8 7, i8 7, i8 7,i8 7, i8 7, i8 7, i8 7, i8 7, i8 7, i8 7, i8 7,i8 7, i8 7, i8 7, i8 7, i8 7, i8 7, i8 7, i8 7,i8 7, i8 7, i8 7, i8 7>
  ret <32 x i8> %res
}

;
; srem by 7
;

define <4 x i64> @test_rem7_4i64(<4 x i64> %a) nounwind {
; AVX1-LABEL: test_rem7_4i64:
; AVX1:       # BB#0:
; AVX1-NEXT:    vextractf128 $1, %ymm0, %xmm1
; AVX1-NEXT:    vpextrq $1, %xmm1, %rcx
; AVX1-NEXT:    movabsq $5270498306774157605, %rsi # imm = 0x4924924924924925
; AVX1-NEXT:    movq %rcx, %rax
; AVX1-NEXT:    imulq %rsi
; AVX1-NEXT:    movq %rdx, %rax
; AVX1-NEXT:    shrq $63, %rax
; AVX1-NEXT:    sarq %rdx
; AVX1-NEXT:    addq %rax, %rdx
; AVX1-NEXT:    leaq (,%rdx,8), %rax
; AVX1-NEXT:    subq %rdx, %rax
; AVX1-NEXT:    subq %rax, %rcx
; AVX1-NEXT:    vmovq %rcx, %xmm2
; AVX1-NEXT:    vmovq %xmm1, %rcx
; AVX1-NEXT:    movq %rcx, %rax
; AVX1-NEXT:    imulq %rsi
; AVX1-NEXT:    movq %rdx, %rax
; AVX1-NEXT:    shrq $63, %rax
; AVX1-NEXT:    sarq %rdx
; AVX1-NEXT:    addq %rax, %rdx
; AVX1-NEXT:    leaq (,%rdx,8), %rax
; AVX1-NEXT:    subq %rdx, %rax
; AVX1-NEXT:    subq %rax, %rcx
; AVX1-NEXT:    vmovq %rcx, %xmm1
; AVX1-NEXT:    vpunpcklqdq {{.*#+}} xmm1 = xmm1[0],xmm2[0]
; AVX1-NEXT:    vpextrq $1, %xmm0, %rcx
; AVX1-NEXT:    movq %rcx, %rax
; AVX1-NEXT:    imulq %rsi
; AVX1-NEXT:    movq %rdx, %rax
; AVX1-NEXT:    shrq $63, %rax
; AVX1-NEXT:    sarq %rdx
; AVX1-NEXT:    addq %rax, %rdx
; AVX1-NEXT:    leaq (,%rdx,8), %rax
; AVX1-NEXT:    subq %rdx, %rax
; AVX1-NEXT:    subq %rax, %rcx
; AVX1-NEXT:    vmovq %rcx, %xmm2
; AVX1-NEXT:    vmovq %xmm0, %rcx
; AVX1-NEXT:    movq %rcx, %rax
; AVX1-NEXT:    imulq %rsi
; AVX1-NEXT:    movq %rdx, %rax
; AVX1-NEXT:    shrq $63, %rax
; AVX1-NEXT:    sarq %rdx
; AVX1-NEXT:    addq %rax, %rdx
; AVX1-NEXT:    leaq (,%rdx,8), %rax
; AVX1-NEXT:    subq %rdx, %rax
; AVX1-NEXT:    subq %rax, %rcx
; AVX1-NEXT:    vmovq %rcx, %xmm0
; AVX1-NEXT:    vpunpcklqdq {{.*#+}} xmm0 = xmm0[0],xmm2[0]
; AVX1-NEXT:    vinsertf128 $1, %xmm1, %ymm0, %ymm0
; AVX1-NEXT:    retq
;
; AVX2-LABEL: test_rem7_4i64:
; AVX2:       # BB#0:
; AVX2-NEXT:    vextracti128 $1, %ymm0, %xmm1
; AVX2-NEXT:    vpextrq $1, %xmm1, %rcx
; AVX2-NEXT:    movabsq $5270498306774157605, %rsi # imm = 0x4924924924924925
; AVX2-NEXT:    movq %rcx, %rax
; AVX2-NEXT:    imulq %rsi
; AVX2-NEXT:    movq %rdx, %rax
; AVX2-NEXT:    shrq $63, %rax
; AVX2-NEXT:    sarq %rdx
; AVX2-NEXT:    addq %rax, %rdx
; AVX2-NEXT:    leaq (,%rdx,8), %rax
; AVX2-NEXT:    subq %rdx, %rax
; AVX2-NEXT:    subq %rax, %rcx
; AVX2-NEXT:    vmovq %rcx, %xmm2
; AVX2-NEXT:    vmovq %xmm1, %rcx
; AVX2-NEXT:    movq %rcx, %rax
; AVX2-NEXT:    imulq %rsi
; AVX2-NEXT:    movq %rdx, %rax
; AVX2-NEXT:    shrq $63, %rax
; AVX2-NEXT:    sarq %rdx
; AVX2-NEXT:    addq %rax, %rdx
; AVX2-NEXT:    leaq (,%rdx,8), %rax
; AVX2-NEXT:    subq %rdx, %rax
; AVX2-NEXT:    subq %rax, %rcx
; AVX2-NEXT:    vmovq %rcx, %xmm1
; AVX2-NEXT:    vpunpcklqdq {{.*#+}} xmm1 = xmm1[0],xmm2[0]
; AVX2-NEXT:    vpextrq $1, %xmm0, %rcx
; AVX2-NEXT:    movq %rcx, %rax
; AVX2-NEXT:    imulq %rsi
; AVX2-NEXT:    movq %rdx, %rax
; AVX2-NEXT:    shrq $63, %rax
; AVX2-NEXT:    sarq %rdx
; AVX2-NEXT:    addq %rax, %rdx
; AVX2-NEXT:    leaq (,%rdx,8), %rax
; AVX2-NEXT:    subq %rdx, %rax
; AVX2-NEXT:    subq %rax, %rcx
; AVX2-NEXT:    vmovq %rcx, %xmm2
; AVX2-NEXT:    vmovq %xmm0, %rcx
; AVX2-NEXT:    movq %rcx, %rax
; AVX2-NEXT:    imulq %rsi
; AVX2-NEXT:    movq %rdx, %rax
; AVX2-NEXT:    shrq $63, %rax
; AVX2-NEXT:    sarq %rdx
; AVX2-NEXT:    addq %rax, %rdx
; AVX2-NEXT:    leaq (,%rdx,8), %rax
; AVX2-NEXT:    subq %rdx, %rax
; AVX2-NEXT:    subq %rax, %rcx
; AVX2-NEXT:    vmovq %rcx, %xmm0
; AVX2-NEXT:    vpunpcklqdq {{.*#+}} xmm0 = xmm0[0],xmm2[0]
; AVX2-NEXT:    vinserti128 $1, %xmm1, %ymm0, %ymm0
; AVX2-NEXT:    retq
  %res = srem <4 x i64> %a, <i64 7, i64 7, i64 7, i64 7>
  ret <4 x i64> %res
}

define <8 x i32> @test_rem7_8i32(<8 x i32> %a) nounwind {
; AVX1-LABEL: test_rem7_8i32:
; AVX1:       # BB#0:
; AVX1-NEXT:    vextractf128 $1, %ymm0, %xmm1
; AVX1-NEXT:    vpextrd $1, %xmm1, %eax
; AVX1-NEXT:    cltq
; AVX1-NEXT:    imulq $-1840700269, %rax, %rcx # imm = 0xFFFFFFFF92492493
; AVX1-NEXT:    shrq $32, %rcx
; AVX1-NEXT:    addl %eax, %ecx
; AVX1-NEXT:    movl %ecx, %edx
; AVX1-NEXT:    shrl $31, %edx
; AVX1-NEXT:    sarl $2, %ecx
; AVX1-NEXT:    addl %edx, %ecx
; AVX1-NEXT:    leal (,%rcx,8), %edx
; AVX1-NEXT:    subl %ecx, %edx
; AVX1-NEXT:    subl %edx, %eax
; AVX1-NEXT:    vmovd %xmm1, %ecx
; AVX1-NEXT:    movslq %ecx, %rcx
; AVX1-NEXT:    imulq $-1840700269, %rcx, %rdx # imm = 0xFFFFFFFF92492493
; AVX1-NEXT:    shrq $32, %rdx
; AVX1-NEXT:    addl %ecx, %edx
; AVX1-NEXT:    movl %edx, %esi
; AVX1-NEXT:    shrl $31, %esi
; AVX1-NEXT:    sarl $2, %edx
; AVX1-NEXT:    addl %esi, %edx
; AVX1-NEXT:    leal (,%rdx,8), %esi
; AVX1-NEXT:    subl %edx, %esi
; AVX1-NEXT:    subl %esi, %ecx
; AVX1-NEXT:    vmovd %ecx, %xmm2
; AVX1-NEXT:    vpinsrd $1, %eax, %xmm2, %xmm2
; AVX1-NEXT:    vpextrd $2, %xmm1, %eax
; AVX1-NEXT:    cltq
; AVX1-NEXT:    imulq $-1840700269, %rax, %rcx # imm = 0xFFFFFFFF92492493
; AVX1-NEXT:    shrq $32, %rcx
; AVX1-NEXT:    addl %eax, %ecx
; AVX1-NEXT:    movl %ecx, %edx
; AVX1-NEXT:    shrl $31, %edx
; AVX1-NEXT:    sarl $2, %ecx
; AVX1-NEXT:    addl %edx, %ecx
; AVX1-NEXT:    leal (,%rcx,8), %edx
; AVX1-NEXT:    subl %ecx, %edx
; AVX1-NEXT:    subl %edx, %eax
; AVX1-NEXT:    vpinsrd $2, %eax, %xmm2, %xmm2
; AVX1-NEXT:    vpextrd $3, %xmm1, %eax
; AVX1-NEXT:    cltq
; AVX1-NEXT:    imulq $-1840700269, %rax, %rcx # imm = 0xFFFFFFFF92492493
; AVX1-NEXT:    shrq $32, %rcx
; AVX1-NEXT:    addl %eax, %ecx
; AVX1-NEXT:    movl %ecx, %edx
; AVX1-NEXT:    shrl $31, %edx
; AVX1-NEXT:    sarl $2, %ecx
; AVX1-NEXT:    addl %edx, %ecx
; AVX1-NEXT:    leal (,%rcx,8), %edx
; AVX1-NEXT:    subl %ecx, %edx
; AVX1-NEXT:    subl %edx, %eax
; AVX1-NEXT:    vpinsrd $3, %eax, %xmm2, %xmm1
; AVX1-NEXT:    vpextrd $1, %xmm0, %eax
; AVX1-NEXT:    cltq
; AVX1-NEXT:    imulq $-1840700269, %rax, %rcx # imm = 0xFFFFFFFF92492493
; AVX1-NEXT:    shrq $32, %rcx
; AVX1-NEXT:    addl %eax, %ecx
; AVX1-NEXT:    movl %ecx, %edx
; AVX1-NEXT:    shrl $31, %edx
; AVX1-NEXT:    sarl $2, %ecx
; AVX1-NEXT:    addl %edx, %ecx
; AVX1-NEXT:    leal (,%rcx,8), %edx
; AVX1-NEXT:    subl %ecx, %edx
; AVX1-NEXT:    subl %edx, %eax
; AVX1-NEXT:    vmovd %xmm0, %ecx
; AVX1-NEXT:    movslq %ecx, %rcx
; AVX1-NEXT:    imulq $-1840700269, %rcx, %rdx # imm = 0xFFFFFFFF92492493
; AVX1-NEXT:    shrq $32, %rdx
; AVX1-NEXT:    addl %ecx, %edx
; AVX1-NEXT:    movl %edx, %esi
; AVX1-NEXT:    shrl $31, %esi
; AVX1-NEXT:    sarl $2, %edx
; AVX1-NEXT:    addl %esi, %edx
; AVX1-NEXT:    leal (,%rdx,8), %esi
; AVX1-NEXT:    subl %edx, %esi
; AVX1-NEXT:    subl %esi, %ecx
; AVX1-NEXT:    vmovd %ecx, %xmm2
; AVX1-NEXT:    vpinsrd $1, %eax, %xmm2, %xmm2
; AVX1-NEXT:    vpextrd $2, %xmm0, %eax
; AVX1-NEXT:    cltq
; AVX1-NEXT:    imulq $-1840700269, %rax, %rcx # imm = 0xFFFFFFFF92492493
; AVX1-NEXT:    shrq $32, %rcx
; AVX1-NEXT:    addl %eax, %ecx
; AVX1-NEXT:    movl %ecx, %edx
; AVX1-NEXT:    shrl $31, %edx
; AVX1-NEXT:    sarl $2, %ecx
; AVX1-NEXT:    addl %edx, %ecx
; AVX1-NEXT:    leal (,%rcx,8), %edx
; AVX1-NEXT:    subl %ecx, %edx
; AVX1-NEXT:    subl %edx, %eax
; AVX1-NEXT:    vpinsrd $2, %eax, %xmm2, %xmm2
; AVX1-NEXT:    vpextrd $3, %xmm0, %eax
; AVX1-NEXT:    cltq
; AVX1-NEXT:    imulq $-1840700269, %rax, %rcx # imm = 0xFFFFFFFF92492493
; AVX1-NEXT:    shrq $32, %rcx
; AVX1-NEXT:    addl %eax, %ecx
; AVX1-NEXT:    movl %ecx, %edx
; AVX1-NEXT:    shrl $31, %edx
; AVX1-NEXT:    sarl $2, %ecx
; AVX1-NEXT:    addl %edx, %ecx
; AVX1-NEXT:    leal (,%rcx,8), %edx
; AVX1-NEXT:    subl %ecx, %edx
; AVX1-NEXT:    subl %edx, %eax
; AVX1-NEXT:    vpinsrd $3, %eax, %xmm2, %xmm0
; AVX1-NEXT:    vinsertf128 $1, %xmm1, %ymm0, %ymm0
; AVX1-NEXT:    retq
;
; AVX2-LABEL: test_rem7_8i32:
; AVX2:       # BB#0:
; AVX2-NEXT:    vpbroadcastd {{.*}}(%rip), %ymm1
; AVX2-NEXT:    vpshufd {{.*#+}} ymm2 = ymm1[1,1,3,3,5,5,7,7]
; AVX2-NEXT:    vpshufd {{.*#+}} ymm3 = ymm0[1,1,3,3,5,5,7,7]
; AVX2-NEXT:    vpmuldq %ymm2, %ymm3, %ymm2
; AVX2-NEXT:    vpmuldq %ymm1, %ymm0, %ymm1
; AVX2-NEXT:    vpshufd {{.*#+}} ymm1 = ymm1[1,1,3,3,5,5,7,7]
; AVX2-NEXT:    vpblendd {{.*#+}} ymm1 = ymm1[0],ymm2[1],ymm1[2],ymm2[3],ymm1[4],ymm2[5],ymm1[6],ymm2[7]
; AVX2-NEXT:    vpaddd %ymm0, %ymm1, %ymm1
; AVX2-NEXT:    vpsrld $31, %ymm1, %ymm2
; AVX2-NEXT:    vpsrad $2, %ymm1, %ymm1
; AVX2-NEXT:    vpaddd %ymm2, %ymm1, %ymm1
; AVX2-NEXT:    vpbroadcastd {{.*}}(%rip), %ymm2
; AVX2-NEXT:    vpmulld %ymm2, %ymm1, %ymm1
; AVX2-NEXT:    vpsubd %ymm1, %ymm0, %ymm0
; AVX2-NEXT:    retq
  %res = srem <8 x i32> %a, <i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7>
  ret <8 x i32> %res
}

define <16 x i16> @test_rem7_16i16(<16 x i16> %a) nounwind {
; AVX1-LABEL: test_rem7_16i16:
; AVX1:       # BB#0:
; AVX1-NEXT:    vextractf128 $1, %ymm0, %xmm1
; AVX1-NEXT:    vmovdqa {{.*#+}} xmm2 = [18725,18725,18725,18725,18725,18725,18725,18725]
; AVX1-NEXT:    vpmulhw %xmm2, %xmm1, %xmm3
; AVX1-NEXT:    vpsrlw $15, %xmm3, %xmm4
; AVX1-NEXT:    vpsraw $1, %xmm3, %xmm3
; AVX1-NEXT:    vpaddw %xmm4, %xmm3, %xmm3
; AVX1-NEXT:    vmovdqa {{.*#+}} xmm4 = [7,7,7,7,7,7,7,7]
; AVX1-NEXT:    vpmullw %xmm4, %xmm3, %xmm3
; AVX1-NEXT:    vpsubw %xmm3, %xmm1, %xmm1
; AVX1-NEXT:    vpmulhw %xmm2, %xmm0, %xmm2
; AVX1-NEXT:    vpsrlw $15, %xmm2, %xmm3
; AVX1-NEXT:    vpsraw $1, %xmm2, %xmm2
; AVX1-NEXT:    vpaddw %xmm3, %xmm2, %xmm2
; AVX1-NEXT:    vpmullw %xmm4, %xmm2, %xmm2
; AVX1-NEXT:    vpsubw %xmm2, %xmm0, %xmm0
; AVX1-NEXT:    vinsertf128 $1, %xmm1, %ymm0, %ymm0
; AVX1-NEXT:    retq
;
; AVX2-LABEL: test_rem7_16i16:
; AVX2:       # BB#0:
; AVX2-NEXT:    vpmulhw {{.*}}(%rip), %ymm0, %ymm1
; AVX2-NEXT:    vpsrlw $15, %ymm1, %ymm2
; AVX2-NEXT:    vpsraw $1, %ymm1, %ymm1
; AVX2-NEXT:    vpaddw %ymm2, %ymm1, %ymm1
; AVX2-NEXT:    vpmullw {{.*}}(%rip), %ymm1, %ymm1
; AVX2-NEXT:    vpsubw %ymm1, %ymm0, %ymm0
; AVX2-NEXT:    retq
  %res = srem <16 x i16> %a, <i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7>
  ret <16 x i16> %res
}

define <32 x i8> @test_rem7_32i8(<32 x i8> %a) nounwind {
; AVX1-LABEL: test_rem7_32i8:
; AVX1:       # BB#0:
; AVX1-NEXT:    vextractf128 $1, %ymm0, %xmm2
; AVX1-NEXT:    vpmovsxbw %xmm2, %xmm3
; AVX1-NEXT:    vpmovsxbw {{.*}}(%rip), %xmm1
; AVX1-NEXT:    vpmullw %xmm1, %xmm3, %xmm3
; AVX1-NEXT:    vpsrlw $8, %xmm3, %xmm3
; AVX1-NEXT:    vpshufd {{.*#+}} xmm4 = xmm2[2,3,0,1]
; AVX1-NEXT:    vpmovsxbw %xmm4, %xmm4
; AVX1-NEXT:    vpmullw %xmm1, %xmm4, %xmm4
; AVX1-NEXT:    vpsrlw $8, %xmm4, %xmm4
; AVX1-NEXT:    vpackuswb %xmm4, %xmm3, %xmm3
; AVX1-NEXT:    vpaddb %xmm2, %xmm3, %xmm3
; AVX1-NEXT:    vpsrlw $7, %xmm3, %xmm4
; AVX1-NEXT:    vmovdqa {{.*#+}} xmm8 = [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]
; AVX1-NEXT:    vpand %xmm8, %xmm4, %xmm4
; AVX1-NEXT:    vpsrlw $2, %xmm3, %xmm3
; AVX1-NEXT:    vmovdqa {{.*#+}} xmm9 = [63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63]
; AVX1-NEXT:    vpand %xmm9, %xmm3, %xmm3
; AVX1-NEXT:    vmovdqa {{.*#+}} xmm7 = [32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32]
; AVX1-NEXT:    vpxor %xmm7, %xmm3, %xmm3
; AVX1-NEXT:    vpsubb %xmm7, %xmm3, %xmm3
; AVX1-NEXT:    vpaddb %xmm4, %xmm3, %xmm3
; AVX1-NEXT:    vpmovsxbw %xmm3, %xmm4
; AVX1-NEXT:    vpmovsxbw {{.*}}(%rip), %xmm5
; AVX1-NEXT:    vpmullw %xmm5, %xmm4, %xmm4
; AVX1-NEXT:    vmovdqa {{.*#+}} xmm6 = [255,255,255,255,255,255,255,255]
; AVX1-NEXT:    vpand %xmm6, %xmm4, %xmm4
; AVX1-NEXT:    vpshufd {{.*#+}} xmm3 = xmm3[2,3,0,1]
; AVX1-NEXT:    vpmovsxbw %xmm3, %xmm3
; AVX1-NEXT:    vpmullw %xmm5, %xmm3, %xmm3
; AVX1-NEXT:    vpand %xmm6, %xmm3, %xmm3
; AVX1-NEXT:    vpackuswb %xmm3, %xmm4, %xmm3
; AVX1-NEXT:    vpsubb %xmm3, %xmm2, %xmm2
; AVX1-NEXT:    vpmovsxbw %xmm0, %xmm3
; AVX1-NEXT:    vpmullw %xmm1, %xmm3, %xmm3
; AVX1-NEXT:    vpsrlw $8, %xmm3, %xmm3
; AVX1-NEXT:    vpshufd {{.*#+}} xmm4 = xmm0[2,3,0,1]
; AVX1-NEXT:    vpmovsxbw %xmm4, %xmm4
; AVX1-NEXT:    vpmullw %xmm1, %xmm4, %xmm1
; AVX1-NEXT:    vpsrlw $8, %xmm1, %xmm1
; AVX1-NEXT:    vpackuswb %xmm1, %xmm3, %xmm1
; AVX1-NEXT:    vpaddb %xmm0, %xmm1, %xmm1
; AVX1-NEXT:    vpsrlw $7, %xmm1, %xmm3
; AVX1-NEXT:    vpand %xmm8, %xmm3, %xmm3
; AVX1-NEXT:    vpsrlw $2, %xmm1, %xmm1
; AVX1-NEXT:    vpand %xmm9, %xmm1, %xmm1
; AVX1-NEXT:    vpxor %xmm7, %xmm1, %xmm1
; AVX1-NEXT:    vpsubb %xmm7, %xmm1, %xmm1
; AVX1-NEXT:    vpaddb %xmm3, %xmm1, %xmm1
; AVX1-NEXT:    vpmovsxbw %xmm1, %xmm3
; AVX1-NEXT:    vpmullw %xmm5, %xmm3, %xmm3
; AVX1-NEXT:    vpand %xmm6, %xmm3, %xmm3
; AVX1-NEXT:    vpshufd {{.*#+}} xmm1 = xmm1[2,3,0,1]
; AVX1-NEXT:    vpmovsxbw %xmm1, %xmm1
; AVX1-NEXT:    vpmullw %xmm5, %xmm1, %xmm1
; AVX1-NEXT:    vpand %xmm6, %xmm1, %xmm1
; AVX1-NEXT:    vpackuswb %xmm1, %xmm3, %xmm1
; AVX1-NEXT:    vpsubb %xmm1, %xmm0, %xmm0
; AVX1-NEXT:    vinsertf128 $1, %xmm2, %ymm0, %ymm0
; AVX1-NEXT:    retq
;
; AVX2-LABEL: test_rem7_32i8:
; AVX2:       # BB#0:
; AVX2-NEXT:    vmovdqa {{.*#+}} ymm1 = [147,147,147,147,147,147,147,147,147,147,147,147,147,147,147,147,147,147,147,147,147,147,147,147,147,147,147,147,147,147,147,147]
; AVX2-NEXT:    vextracti128 $1, %ymm1, %xmm2
; AVX2-NEXT:    vpmovsxbw %xmm2, %ymm2
; AVX2-NEXT:    vextracti128 $1, %ymm0, %xmm3
; AVX2-NEXT:    vpmovsxbw %xmm3, %ymm3
; AVX2-NEXT:    vpmullw %ymm2, %ymm3, %ymm2
; AVX2-NEXT:    vpsrlw $8, %ymm2, %ymm2
; AVX2-NEXT:    vpmovsxbw %xmm1, %ymm1
; AVX2-NEXT:    vpmovsxbw %xmm0, %ymm3
; AVX2-NEXT:    vpmullw %ymm1, %ymm3, %ymm1
; AVX2-NEXT:    vpsrlw $8, %ymm1, %ymm1
; AVX2-NEXT:    vperm2i128 {{.*#+}} ymm3 = ymm1[2,3],ymm2[2,3]
; AVX2-NEXT:    vinserti128 $1, %xmm2, %ymm1, %ymm1
; AVX2-NEXT:    vpackuswb %ymm3, %ymm1, %ymm1
; AVX2-NEXT:    vpaddb %ymm0, %ymm1, %ymm1
; AVX2-NEXT:    vpsrlw $2, %ymm1, %ymm2
; AVX2-NEXT:    vpand {{.*}}(%rip), %ymm2, %ymm2
; AVX2-NEXT:    vmovdqa {{.*#+}} ymm3 = [32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32]
; AVX2-NEXT:    vpxor %ymm3, %ymm2, %ymm2
; AVX2-NEXT:    vpsubb %ymm3, %ymm2, %ymm2
; AVX2-NEXT:    vpsrlw $7, %ymm1, %ymm1
; AVX2-NEXT:    vpand {{.*}}(%rip), %ymm1, %ymm1
; AVX2-NEXT:    vpaddb %ymm1, %ymm2, %ymm1
; AVX2-NEXT:    vextracti128 $1, %ymm1, %xmm2
; AVX2-NEXT:    vpmovsxbw %xmm2, %ymm2
; AVX2-NEXT:    vpmovsxbw {{.*}}(%rip), %ymm3
; AVX2-NEXT:    vpmullw %ymm3, %ymm2, %ymm2
; AVX2-NEXT:    vextracti128 $1, %ymm2, %xmm4
; AVX2-NEXT:    vmovdqa {{.*#+}} xmm5 = <0,2,4,6,8,10,12,14,u,u,u,u,u,u,u,u>
; AVX2-NEXT:    vpshufb %xmm5, %xmm4, %xmm4
; AVX2-NEXT:    vpshufb %xmm5, %xmm2, %xmm2
; AVX2-NEXT:    vpunpcklqdq {{.*#+}} xmm2 = xmm2[0],xmm4[0]
; AVX2-NEXT:    vpmovsxbw %xmm1, %ymm1
; AVX2-NEXT:    vpmullw %ymm3, %ymm1, %ymm1
; AVX2-NEXT:    vextracti128 $1, %ymm1, %xmm3
; AVX2-NEXT:    vpshufb %xmm5, %xmm3, %xmm3
; AVX2-NEXT:    vpshufb %xmm5, %xmm1, %xmm1
; AVX2-NEXT:    vpunpcklqdq {{.*#+}} xmm1 = xmm1[0],xmm3[0]
; AVX2-NEXT:    vinserti128 $1, %xmm2, %ymm1, %ymm1
; AVX2-NEXT:    vpsubb %ymm1, %ymm0, %ymm0
; AVX2-NEXT:    retq
  %res = srem <32 x i8> %a, <i8 7, i8 7, i8 7, i8 7,i8 7, i8 7, i8 7, i8 7, i8 7, i8 7, i8 7, i8 7,i8 7, i8 7, i8 7, i8 7, i8 7, i8 7, i8 7, i8 7,i8 7, i8 7, i8 7, i8 7, i8 7, i8 7, i8 7, i8 7,i8 7, i8 7, i8 7, i8 7>
  ret <32 x i8> %res
}
