; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=i686-unknown -mattr=+avx2 | FileCheck %s --check-prefix=X32
; RUN: llc < %s -mtriple=x86_64-unknown -mattr=+avx2 | FileCheck %s --check-prefix=X64

declare <8 x i32> @llvm.x86.avx2.permd(<8 x i32>, <8 x i32>)
declare <8 x float> @llvm.x86.avx2.permps(<8 x float>, <8 x i32>)
declare <16 x i8> @llvm.x86.ssse3.pshuf.b.128(<16 x i8>, <16 x i8>)
declare <32 x i8> @llvm.x86.avx2.pshuf.b(<32 x i8>, <32 x i8>)

define <32 x i8> @combine_pshufb_pslldq(<32 x i8> %a0) {
; X32-LABEL: combine_pshufb_pslldq:
; X32:       # BB#0:
; X32-NEXT:    vxorps %ymm0, %ymm0, %ymm0
; X32-NEXT:    retl
;
; X64-LABEL: combine_pshufb_pslldq:
; X64:       # BB#0:
; X64-NEXT:    vxorps %ymm0, %ymm0, %ymm0
; X64-NEXT:    retq
  %1 = tail call <32 x i8> @llvm.x86.avx2.pshuf.b(<32 x i8> %a0, <32 x i8> <i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 0, i8 1, i8 2, i8 3, i8 4, i8 5, i8 6, i8 7, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 0, i8 1, i8 2, i8 3, i8 4, i8 5, i8 6, i8 7>)
  %2 = shufflevector <32 x i8> %1, <32 x i8> zeroinitializer, <32 x i32> <i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23>
  ret <32 x i8> %2
}

define <32 x i8> @combine_pshufb_psrldq(<32 x i8> %a0) {
; X32-LABEL: combine_pshufb_psrldq:
; X32:       # BB#0:
; X32-NEXT:    vxorps %ymm0, %ymm0, %ymm0
; X32-NEXT:    retl
;
; X64-LABEL: combine_pshufb_psrldq:
; X64:       # BB#0:
; X64-NEXT:    vxorps %ymm0, %ymm0, %ymm0
; X64-NEXT:    retq
  %1 = tail call <32 x i8> @llvm.x86.avx2.pshuf.b(<32 x i8> %a0, <32 x i8> <i8 8, i8 9, i8 10, i8 11, i8 12, i8 13, i8 14, i8 15, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 8, i8 9, i8 10, i8 11, i8 12, i8 13, i8 14, i8 15, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128>)
  %2 = shufflevector <32 x i8> %1, <32 x i8> zeroinitializer, <32 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32>
  ret <32 x i8> %2
}

define <32 x i8> @combine_pshufb_vpermd(<8 x i32> %a) {
; X32-LABEL: combine_pshufb_vpermd:
; X32:       # BB#0:
; X32-NEXT:    vpshufb {{.*#+}} ymm0 = ymm0[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,16,17,18,18]
; X32-NEXT:    retl
;
; X64-LABEL: combine_pshufb_vpermd:
; X64:       # BB#0:
; X64-NEXT:    vpshufb {{.*#+}} ymm0 = ymm0[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,16,17,18,18]
; X64-NEXT:    retq
  %tmp0 = call <8 x i32> @llvm.x86.avx2.permd(<8 x i32> %a, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 4>)
  %tmp1 = bitcast <8 x i32> %tmp0 to <32 x i8>
  %tmp2 = shufflevector <32 x i8> %tmp1, <32 x i8> undef, <32 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 30>
  ret <32 x i8> %tmp2
}

define <32 x i8> @combine_pshufb_vpermps(<8 x float> %a) {
; X32-LABEL: combine_pshufb_vpermps:
; X32:       # BB#0:
; X32-NEXT:    vpshufb {{.*#+}} ymm0 = ymm0[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,16,17,18,18]
; X32-NEXT:    retl
;
; X64-LABEL: combine_pshufb_vpermps:
; X64:       # BB#0:
; X64-NEXT:    vpshufb {{.*#+}} ymm0 = ymm0[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,16,17,18,18]
; X64-NEXT:    retq
  %tmp0 = call <8 x float> @llvm.x86.avx2.permps(<8 x float> %a, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 4>)
  %tmp1 = bitcast <8 x float> %tmp0 to <32 x i8>
  %tmp2 = shufflevector <32 x i8> %tmp1, <32 x i8> undef, <32 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 30>
  ret <32 x i8> %tmp2
}

define <4 x i64> @combine_permq_pshufb_as_vperm2i128(<4 x i64> %a0) {
; X32-LABEL: combine_permq_pshufb_as_vperm2i128:
; X32:       # BB#0:
; X32-NEXT:    vperm2i128 {{.*#+}} ymm0 = ymm0[2,3],zero,zero
; X32-NEXT:    vpaddq {{\.LCPI.*}}, %ymm0, %ymm0
; X32-NEXT:    retl
;
; X64-LABEL: combine_permq_pshufb_as_vperm2i128:
; X64:       # BB#0:
; X64-NEXT:    vperm2i128 {{.*#+}} ymm0 = ymm0[2,3],zero,zero
; X64-NEXT:    vpaddq {{.*}}(%rip), %ymm0, %ymm0
; X64-NEXT:    retq
  %1 = shufflevector <4 x i64> %a0, <4 x i64> undef, <4 x i32> <i32 3, i32 2, i32 1, i32 0>
  %2 = bitcast <4 x i64> %1 to <32 x i8>
  %3 = call <32 x i8> @llvm.x86.avx2.pshuf.b(<32 x i8> %2, <32 x i8> <i8 8, i8 9, i8 10, i8 11, i8 12, i8 13, i8 14, i8 15, i8 0, i8 1, i8 2, i8 3, i8 4, i8 5, i8 6, i8 7, i8 255, i8 255, i8 255, i8 255, i8 255, i8 255, i8 255, i8 255, i8 255, i8 255, i8 255, i8 255, i8 255, i8 255, i8 255, i8 255>)
  %4 = bitcast <32 x i8> %3 to <4 x i64>
  %5 = add <4 x i64> %4, <i64 1, i64 1, i64 3, i64 3>
  ret <4 x i64> %5
}

define <8 x i32> @combine_as_vpermd(<8 x i32> %a0) {
; X32-LABEL: combine_as_vpermd:
; X32:       # BB#0:
; X32-NEXT:    vmovdqa {{.*#+}} ymm1 = [4,5,4,5,6,7,0,7]
; X32-NEXT:    vpermd %ymm0, %ymm1, %ymm0
; X32-NEXT:    retl
;
; X64-LABEL: combine_as_vpermd:
; X64:       # BB#0:
; X64-NEXT:    vmovdqa {{.*#+}} ymm1 = [4,5,4,5,6,7,0,7]
; X64-NEXT:    vpermd %ymm0, %ymm1, %ymm0
; X64-NEXT:    retq
  %1 = shufflevector <8 x i32> %a0, <8 x i32> undef, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 0, i32 1, i32 2, i32 3>
  %2 = tail call <8 x i32> @llvm.x86.avx2.permd(<8 x i32> %a0, <8 x i32> <i32 5, i32 4, i32 3, i32 2, i32 1, i32 0, i32 7, i32 6>)
  %3 = shufflevector <8 x i32> %1, <8 x i32> %2, <8 x i32> <i32 0, i32 8, i32 9, i32 1, i32 15, i32 14, i32 4, i32 3>
  ret <8 x i32> %3
}

define <8 x float> @combine_as_vpermps(<8 x float> %a0) {
; X32-LABEL: combine_as_vpermps:
; X32:       # BB#0:
; X32-NEXT:    vmovaps {{.*#+}} ymm1 = <6,4,7,5,1,u,4,7>
; X32-NEXT:    vpermps %ymm0, %ymm1, %ymm0
; X32-NEXT:    retl
;
; X64-LABEL: combine_as_vpermps:
; X64:       # BB#0:
; X64-NEXT:    vmovaps {{.*#+}} ymm1 = <6,4,7,5,1,u,4,7>
; X64-NEXT:    vpermps %ymm0, %ymm1, %ymm0
; X64-NEXT:    retq
  %1 = shufflevector <8 x float> %a0, <8 x float> undef, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7>
  %2 = tail call <8 x float> @llvm.x86.avx2.permps(<8 x float> %a0, <8 x i32> <i32 1, i32 undef, i32 3, i32 2, i32 5, i32 4, i32 7, i32 6>)
  %3 = shufflevector <8 x float> %1, <8 x float> %2, <8 x i32> <i32 15, i32 0, i32 14, i32 1, i32 8, i32 9, i32 4, i32 3>
  ret <8 x float> %3
}

define <32 x i8> @combine_permq_pshufb_as_vpblendd(<4 x i64> %a0) {
; X32-LABEL: combine_permq_pshufb_as_vpblendd:
; X32:       # BB#0:
; X32-NEXT:    vpxor %ymm1, %ymm1, %ymm1
; X32-NEXT:    vpblendd {{.*#+}} ymm0 = ymm0[0,1,2,3],ymm1[4,5,6,7]
; X32-NEXT:    retl
;
; X64-LABEL: combine_permq_pshufb_as_vpblendd:
; X64:       # BB#0:
; X64-NEXT:    vpxor %ymm1, %ymm1, %ymm1
; X64-NEXT:    vpblendd {{.*#+}} ymm0 = ymm0[0,1,2,3],ymm1[4,5,6,7]
; X64-NEXT:    retq
  %1 = shufflevector <4 x i64> %a0, <4 x i64> undef, <4 x i32> <i32 1, i32 0, i32 3, i32 2>
  %2 = bitcast <4 x i64> %1 to <32 x i8>
  %3 = call <32 x i8> @llvm.x86.avx2.pshuf.b(<32 x i8> %2, <32 x i8> <i8 8, i8 9, i8 10, i8 11, i8 12, i8 13, i8 14, i8 15, i8 0, i8 1, i8 2, i8 3, i8 4, i8 5, i8 6, i8 7, i8 255, i8 255, i8 255, i8 255, i8 255, i8 255, i8 255, i8 255, i8 255, i8 255, i8 255, i8 255, i8 255, i8 255, i8 255, i8 255>)
  ret <32 x i8> %3
}

define <16 x i8> @combine_pshufb_as_vpbroadcastb128(<16 x i8> %a) {
; X32-LABEL: combine_pshufb_as_vpbroadcastb128:
; X32:       # BB#0:
; X32-NEXT:    vpbroadcastb %xmm0, %xmm0
; X32-NEXT:    retl
;
; X64-LABEL: combine_pshufb_as_vpbroadcastb128:
; X64:       # BB#0:
; X64-NEXT:    vpbroadcastb %xmm0, %xmm0
; X64-NEXT:    retq
  %1 = call <16 x i8> @llvm.x86.ssse3.pshuf.b.128(<16 x i8> %a, <16 x i8> zeroinitializer)
  ret <16 x i8> %1
}

define <32 x i8> @combine_pshufb_as_vpbroadcastb256(<2 x i64> %a) {
; X32-LABEL: combine_pshufb_as_vpbroadcastb256:
; X32:       # BB#0:
; X32-NEXT:    # kill: %XMM0<def> %XMM0<kill> %YMM0<def>
; X32-NEXT:    vpbroadcastb %xmm0, %ymm0
; X32-NEXT:    retl
;
; X64-LABEL: combine_pshufb_as_vpbroadcastb256:
; X64:       # BB#0:
; X64-NEXT:    # kill: %XMM0<def> %XMM0<kill> %YMM0<def>
; X64-NEXT:    vpbroadcastb %xmm0, %ymm0
; X64-NEXT:    retq
  %1 = shufflevector <2 x i64> %a, <2 x i64> undef, <4 x i32> <i32 0, i32 undef, i32 undef, i32 undef>
  %2 = bitcast <4 x i64> %1 to <32 x i8>
  %3 = call <32 x i8> @llvm.x86.avx2.pshuf.b(<32 x i8> %2, <32 x i8> zeroinitializer)
  %4 = bitcast <32 x i8> %3 to <8 x i32>
  %5 = call <8 x i32> @llvm.x86.avx2.permd(<8 x i32> %4, <8 x i32> zeroinitializer)
  %6 = bitcast <8 x i32> %5 to <32 x i8>
  ret <32 x i8> %6
}

define <16 x i8> @combine_pshufb_as_vpbroadcastw128(<16 x i8> %a) {
; X32-LABEL: combine_pshufb_as_vpbroadcastw128:
; X32:       # BB#0:
; X32-NEXT:    vpbroadcastw %xmm0, %xmm0
; X32-NEXT:    retl
;
; X64-LABEL: combine_pshufb_as_vpbroadcastw128:
; X64:       # BB#0:
; X64-NEXT:    vpbroadcastw %xmm0, %xmm0
; X64-NEXT:    retq
  %1 = call <16 x i8> @llvm.x86.ssse3.pshuf.b.128(<16 x i8> %a, <16 x i8> <i8 0, i8 1, i8 0, i8 1, i8 0, i8 1, i8 0, i8 1, i8 0, i8 1, i8 0, i8 1, i8 0, i8 1, i8 0, i8 1>)
  ret <16 x i8> %1
}

define <32 x i8> @combine_pshufb_as_vpbroadcastw256(<2 x i64> %a) {
; X32-LABEL: combine_pshufb_as_vpbroadcastw256:
; X32:       # BB#0:
; X32-NEXT:    # kill: %XMM0<def> %XMM0<kill> %YMM0<def>
; X32-NEXT:    vpbroadcastw %xmm0, %ymm0
; X32-NEXT:    retl
;
; X64-LABEL: combine_pshufb_as_vpbroadcastw256:
; X64:       # BB#0:
; X64-NEXT:    # kill: %XMM0<def> %XMM0<kill> %YMM0<def>
; X64-NEXT:    vpbroadcastw %xmm0, %ymm0
; X64-NEXT:    retq
  %1 = shufflevector <2 x i64> %a, <2 x i64> undef, <4 x i32> <i32 0, i32 undef, i32 undef, i32 undef>
  %2 = bitcast <4 x i64> %1 to <32 x i8>
  %3 = call <32 x i8> @llvm.x86.avx2.pshuf.b(<32 x i8> %2, <32 x i8> <i8 0, i8 1, i8 0, i8 1, i8 0, i8 1, i8 0, i8 1, i8 0, i8 1, i8 0, i8 1, i8 0, i8 1, i8 0, i8 1, i8 0, i8 1, i8 0, i8 1, i8 0, i8 1, i8 0, i8 1, i8 0, i8 1, i8 0, i8 1, i8 0, i8 1, i8 0, i8 1>)
  %4 = bitcast <32 x i8> %3 to <8 x i32>
  %5 = call <8 x i32> @llvm.x86.avx2.permd(<8 x i32> %4, <8 x i32> zeroinitializer)
  %6 = bitcast <8 x i32> %5 to <32 x i8>
  ret <32 x i8> %6
}

define <16 x i8> @combine_pshufb_as_vpbroadcastd128(<16 x i8> %a) {
; X32-LABEL: combine_pshufb_as_vpbroadcastd128:
; X32:       # BB#0:
; X32-NEXT:    vpbroadcastd %xmm0, %xmm0
; X32-NEXT:    vpaddb {{\.LCPI.*}}, %xmm0, %xmm0
; X32-NEXT:    retl
;
; X64-LABEL: combine_pshufb_as_vpbroadcastd128:
; X64:       # BB#0:
; X64-NEXT:    vpbroadcastd %xmm0, %xmm0
; X64-NEXT:    vpaddb {{.*}}(%rip), %xmm0, %xmm0
; X64-NEXT:    retq
  %1 = call <16 x i8> @llvm.x86.ssse3.pshuf.b.128(<16 x i8> %a, <16 x i8> <i8 0, i8 1, i8 2, i8 3, i8 0, i8 1, i8 2, i8 3, i8 0, i8 1, i8 2, i8 3, i8 0, i8 1, i8 2, i8 3>)
  %2 = add <16 x i8> %1, <i8 0, i8 1, i8 2, i8 3, i8 0, i8 1, i8 2, i8 3, i8 0, i8 1, i8 2, i8 3, i8 0, i8 1, i8 2, i8 3>
  ret <16 x i8> %2
}

define <8 x i32> @combine_permd_as_vpbroadcastd256(<4 x i32> %a) {
; X32-LABEL: combine_permd_as_vpbroadcastd256:
; X32:       # BB#0:
; X32-NEXT:    # kill: %XMM0<def> %XMM0<kill> %YMM0<def>
; X32-NEXT:    vpbroadcastd %xmm0, %ymm0
; X32-NEXT:    vpaddd {{\.LCPI.*}}, %ymm0, %ymm0
; X32-NEXT:    retl
;
; X64-LABEL: combine_permd_as_vpbroadcastd256:
; X64:       # BB#0:
; X64-NEXT:    # kill: %XMM0<def> %XMM0<kill> %YMM0<def>
; X64-NEXT:    vpbroadcastd %xmm0, %ymm0
; X64-NEXT:    vpaddd {{.*}}(%rip), %ymm0, %ymm0
; X64-NEXT:    retq
  %1 = shufflevector <4 x i32> %a, <4 x i32> undef, <8 x i32> <i32 0, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %2 = call <8 x i32> @llvm.x86.avx2.permd(<8 x i32> %1, <8 x i32> zeroinitializer)
  %3 = add <8 x i32> %2, <i32 0, i32 1, i32 2, i32 3, i32 0, i32 1, i32 2, i32 3>
  ret <8 x i32> %3
}

define <16 x i8> @combine_pshufb_as_vpbroadcastq128(<16 x i8> %a) {
; X32-LABEL: combine_pshufb_as_vpbroadcastq128:
; X32:       # BB#0:
; X32-NEXT:    vpbroadcastq %xmm0, %xmm0
; X32-NEXT:    retl
;
; X64-LABEL: combine_pshufb_as_vpbroadcastq128:
; X64:       # BB#0:
; X64-NEXT:    vpbroadcastq %xmm0, %xmm0
; X64-NEXT:    retq
  %1 = call <16 x i8> @llvm.x86.ssse3.pshuf.b.128(<16 x i8> %a, <16 x i8> <i8 0, i8 1, i8 2, i8 3, i8 4, i8 5, i8 6, i8 7, i8 0, i8 1, i8 2, i8 3, i8 4, i8 5, i8 6, i8 7>)
  ret <16 x i8> %1
}

define <8 x i32> @combine_permd_as_vpbroadcastq256(<4 x i32> %a) {
; X32-LABEL: combine_permd_as_vpbroadcastq256:
; X32:       # BB#0:
; X32-NEXT:    # kill: %XMM0<def> %XMM0<kill> %YMM0<def>
; X32-NEXT:    vpbroadcastq %xmm0, %ymm0
; X32-NEXT:    vpaddd {{\.LCPI.*}}, %ymm0, %ymm0
; X32-NEXT:    retl
;
; X64-LABEL: combine_permd_as_vpbroadcastq256:
; X64:       # BB#0:
; X64-NEXT:    # kill: %XMM0<def> %XMM0<kill> %YMM0<def>
; X64-NEXT:    vpbroadcastq %xmm0, %ymm0
; X64-NEXT:    vpaddd {{.*}}(%rip), %ymm0, %ymm0
; X64-NEXT:    retq
  %1 = shufflevector <4 x i32> %a, <4 x i32> undef, <8 x i32> <i32 0, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %2 = call <8 x i32> @llvm.x86.avx2.permd(<8 x i32> %1, <8 x i32> <i32 0, i32 1, i32 0, i32 1, i32 0, i32 1, i32 0, i32 1>)
  %3 = add <8 x i32> %2, <i32 0, i32 1, i32 2, i32 3, i32 0, i32 1, i32 2, i32 3>
  ret <8 x i32> %3
}

define <4 x float> @combine_pshufb_as_vpbroadcastss128(<4 x float> %a) {
; X32-LABEL: combine_pshufb_as_vpbroadcastss128:
; X32:       # BB#0:
; X32-NEXT:    vbroadcastss %xmm0, %xmm0
; X32-NEXT:    retl
;
; X64-LABEL: combine_pshufb_as_vpbroadcastss128:
; X64:       # BB#0:
; X64-NEXT:    vbroadcastss %xmm0, %xmm0
; X64-NEXT:    retq
  %1 = bitcast <4 x float> %a to <16 x i8>
  %2 = call <16 x i8> @llvm.x86.ssse3.pshuf.b.128(<16 x i8> %1, <16 x i8> <i8 0, i8 1, i8 2, i8 3, i8 0, i8 1, i8 2, i8 3, i8 0, i8 1, i8 2, i8 3, i8 0, i8 1, i8 2, i8 3>)
  %3 = bitcast <16 x i8> %2 to <4 x float>
  ret <4 x float> %3
}

define <8 x float> @combine_permps_as_vpbroadcastss256(<4 x float> %a) {
; X32-LABEL: combine_permps_as_vpbroadcastss256:
; X32:       # BB#0:
; X32-NEXT:    # kill: %XMM0<def> %XMM0<kill> %YMM0<def>
; X32-NEXT:    vbroadcastss %xmm0, %ymm0
; X32-NEXT:    retl
;
; X64-LABEL: combine_permps_as_vpbroadcastss256:
; X64:       # BB#0:
; X64-NEXT:    # kill: %XMM0<def> %XMM0<kill> %YMM0<def>
; X64-NEXT:    vbroadcastss %xmm0, %ymm0
; X64-NEXT:    retq
  %1 = shufflevector <4 x float> %a, <4 x float> undef, <8 x i32> <i32 0, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %2 = call <8 x float> @llvm.x86.avx2.permps(<8 x float> %1, <8 x i32> zeroinitializer)
  ret <8 x float> %2
}

define <4 x double> @combine_permps_as_vpbroadcastsd256(<2 x double> %a) {
; X32-LABEL: combine_permps_as_vpbroadcastsd256:
; X32:       # BB#0:
; X32-NEXT:    # kill: %XMM0<def> %XMM0<kill> %YMM0<def>
; X32-NEXT:    vbroadcastsd %xmm0, %ymm0
; X32-NEXT:    retl
;
; X64-LABEL: combine_permps_as_vpbroadcastsd256:
; X64:       # BB#0:
; X64-NEXT:    # kill: %XMM0<def> %XMM0<kill> %YMM0<def>
; X64-NEXT:    vbroadcastsd %xmm0, %ymm0
; X64-NEXT:    retq
  %1 = shufflevector <2 x double> %a, <2 x double> undef, <4 x i32> <i32 0, i32 undef, i32 undef, i32 undef>
  %2 = bitcast <4 x double> %1 to <8 x float>
  %3 = call <8 x float> @llvm.x86.avx2.permps(<8 x float> %2, <8 x i32> <i32 0, i32 1, i32 0, i32 1, i32 0, i32 1, i32 0, i32 1>)
  %4 = bitcast <8 x float> %3 to <4 x double>
  ret <4 x double> %4
}

define <16 x i8> @combine_vpbroadcast_pshufb_as_vpbroadcastb128(<16 x i8> %a) {
; X32-LABEL: combine_vpbroadcast_pshufb_as_vpbroadcastb128:
; X32:       # BB#0:
; X32-NEXT:    vpbroadcastb %xmm0, %xmm0
; X32-NEXT:    retl
;
; X64-LABEL: combine_vpbroadcast_pshufb_as_vpbroadcastb128:
; X64:       # BB#0:
; X64-NEXT:    vpbroadcastb %xmm0, %xmm0
; X64-NEXT:    retq
  %1 = shufflevector <16 x i8> %a, <16 x i8> undef, <16 x i32> zeroinitializer
  %2 = call <16 x i8> @llvm.x86.ssse3.pshuf.b.128(<16 x i8> %1, <16 x i8> zeroinitializer)
  ret <16 x i8> %2
}

define <32 x i8> @combine_vpbroadcast_pshufb_as_vpbroadcastb256(<32 x i8> %a) {
; X32-LABEL: combine_vpbroadcast_pshufb_as_vpbroadcastb256:
; X32:       # BB#0:
; X32-NEXT:    vpbroadcastb %xmm0, %ymm0
; X32-NEXT:    retl
;
; X64-LABEL: combine_vpbroadcast_pshufb_as_vpbroadcastb256:
; X64:       # BB#0:
; X64-NEXT:    vpbroadcastb %xmm0, %ymm0
; X64-NEXT:    retq
  %1 = shufflevector <32 x i8> %a, <32 x i8> undef, <32 x i32> zeroinitializer
  %2 = call <32 x i8> @llvm.x86.avx2.pshuf.b(<32 x i8> %1, <32 x i8> zeroinitializer)
  ret <32 x i8> %2
}

define <4 x float> @combine_vpbroadcast_pshufb_as_vpbroadcastss128(<4 x float> %a) {
; X32-LABEL: combine_vpbroadcast_pshufb_as_vpbroadcastss128:
; X32:       # BB#0:
; X32-NEXT:    vbroadcastss %xmm0, %xmm0
; X32-NEXT:    retl
;
; X64-LABEL: combine_vpbroadcast_pshufb_as_vpbroadcastss128:
; X64:       # BB#0:
; X64-NEXT:    vbroadcastss %xmm0, %xmm0
; X64-NEXT:    retq
  %1 = shufflevector <4 x float> %a, <4 x float> undef, <4 x i32> zeroinitializer
  %2 = bitcast <4 x float> %1 to <16 x i8>
  %3 = call <16 x i8> @llvm.x86.ssse3.pshuf.b.128(<16 x i8> %2, <16 x i8> <i8 0, i8 1, i8 2, i8 3, i8 0, i8 1, i8 2, i8 3, i8 0, i8 1, i8 2, i8 3, i8 0, i8 1, i8 2, i8 3>)
  %4 = bitcast <16 x i8> %3 to <4 x float>
  ret <4 x float> %4
}

define <8 x float> @combine_vpbroadcast_permd_as_vpbroadcastss256(<4 x float> %a) {
; X32-LABEL: combine_vpbroadcast_permd_as_vpbroadcastss256:
; X32:       # BB#0:
; X32-NEXT:    vbroadcastss %xmm0, %ymm0
; X32-NEXT:    vbroadcastss %xmm0, %ymm0
; X32-NEXT:    retl
;
; X64-LABEL: combine_vpbroadcast_permd_as_vpbroadcastss256:
; X64:       # BB#0:
; X64-NEXT:    vbroadcastss %xmm0, %ymm0
; X64-NEXT:    vbroadcastss %xmm0, %ymm0
; X64-NEXT:    retq
  %1 = shufflevector <4 x float> %a, <4 x float> undef, <8 x i32> zeroinitializer
  %2 = call <8 x float> @llvm.x86.avx2.permps(<8 x float> %1, <8 x i32> zeroinitializer)
  ret <8 x float> %2
}

define <4 x double> @combine_vpbroadcast_permd_as_vpbroadcastsd256(<2 x double> %a) {
; X32-LABEL: combine_vpbroadcast_permd_as_vpbroadcastsd256:
; X32:       # BB#0:
; X32-NEXT:    vbroadcastsd %xmm0, %ymm0
; X32-NEXT:    vbroadcastsd %xmm0, %ymm0
; X32-NEXT:    retl
;
; X64-LABEL: combine_vpbroadcast_permd_as_vpbroadcastsd256:
; X64:       # BB#0:
; X64-NEXT:    vbroadcastsd %xmm0, %ymm0
; X64-NEXT:    vbroadcastsd %xmm0, %ymm0
; X64-NEXT:    retq
  %1 = shufflevector <2 x double> %a, <2 x double> undef, <4 x i32> zeroinitializer
  %2 = bitcast <4 x double> %1 to <8 x float>
  %3 = call <8 x float> @llvm.x86.avx2.permps(<8 x float> %2, <8 x i32> <i32 0, i32 1, i32 0, i32 1, i32 0, i32 1, i32 0, i32 1>)
  %4 = bitcast <8 x float> %3 to <4 x double>
  ret <4 x double> %4
}

define <8 x i32> @combine_permd_as_permq(<8 x i32> %a) {
; X32-LABEL: combine_permd_as_permq:
; X32:       # BB#0:
; X32-NEXT:    vpermq {{.*#+}} ymm0 = ymm0[0,2,2,1]
; X32-NEXT:    retl
;
; X64-LABEL: combine_permd_as_permq:
; X64:       # BB#0:
; X64-NEXT:    vpermq {{.*#+}} ymm0 = ymm0[0,2,2,1]
; X64-NEXT:    retq
  %1 = call <8 x i32> @llvm.x86.avx2.permd(<8 x i32> %a, <8 x i32> <i32 0, i32 1, i32 4, i32 5, i32 4, i32 5, i32 2, i32 3>)
  ret <8 x i32> %1
}

define <8 x float> @combine_permps_as_permpd(<8 x float> %a) {
; X32-LABEL: combine_permps_as_permpd:
; X32:       # BB#0:
; X32-NEXT:    vpermpd {{.*#+}} ymm0 = ymm0[3,2,0,1]
; X32-NEXT:    retl
;
; X64-LABEL: combine_permps_as_permpd:
; X64:       # BB#0:
; X64-NEXT:    vpermpd {{.*#+}} ymm0 = ymm0[3,2,0,1]
; X64-NEXT:    retq
  %1 = call <8 x float> @llvm.x86.avx2.permps(<8 x float> %a, <8 x i32> <i32 6, i32 7, i32 4, i32 5, i32 0, i32 1, i32 2, i32 3>)
  ret <8 x float> %1
}

define <4 x i64> @combine_pshufb_as_zext(<32 x i8> %a0) {
; X32-LABEL: combine_pshufb_as_zext:
; X32:       # BB#0:
; X32-NEXT:    vpermq {{.*#+}} ymm0 = ymm0[1,0,0,1]
; X32-NEXT:    vpshufb {{.*#+}} ymm0 = ymm0[8,9],zero,zero,zero,zero,zero,zero,ymm0[10,11],zero,zero,zero,zero,zero,zero,ymm0[20,21],zero,zero,zero,zero,zero,zero,ymm0[22,23],zero,zero,zero,zero,zero,zero
; X32-NEXT:    retl
;
; X64-LABEL: combine_pshufb_as_zext:
; X64:       # BB#0:
; X64-NEXT:    vpermq {{.*#+}} ymm0 = ymm0[1,0,0,1]
; X64-NEXT:    vpshufb {{.*#+}} ymm0 = ymm0[8,9],zero,zero,zero,zero,zero,zero,ymm0[10,11],zero,zero,zero,zero,zero,zero,ymm0[20,21],zero,zero,zero,zero,zero,zero,ymm0[22,23],zero,zero,zero,zero,zero,zero
; X64-NEXT:    retq
  %1 = shufflevector <32 x i8> %a0, <32 x i8> undef, <32 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %2 = call <32 x i8> @llvm.x86.avx2.pshuf.b(<32 x i8> %1, <32 x i8> <i8 8, i8 9, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 10, i8 11, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 4, i8 5, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 6, i8 7, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1>)
  %3 = bitcast <32 x i8> %2 to <4 x i64>
  ret <4 x i64> %3
}

define <4 x i64> @combine_pshufb_as_zext128(<32 x i8> %a0) {
; X32-LABEL: combine_pshufb_as_zext128:
; X32:       # BB#0:
; X32-NEXT:    vpshufb {{.*#+}} xmm0 = xmm0[15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0]
; X32-NEXT:    vpermq {{.*#+}} ymm0 = ymm0[0,1,0,1]
; X32-NEXT:    vpshufb {{.*#+}} ymm0 = ymm0[15,14],zero,zero,zero,zero,zero,zero,ymm0[13,12],zero,zero,zero,zero,zero,zero,ymm0[31,30],zero,zero,zero,zero,zero,zero,ymm0[29,28],zero,zero,zero,zero,zero,zero
; X32-NEXT:    retl
;
; X64-LABEL: combine_pshufb_as_zext128:
; X64:       # BB#0:
; X64-NEXT:    vpshufb {{.*#+}} xmm0 = xmm0[15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0]
; X64-NEXT:    vperm2i128 {{.*#+}} ymm0 = ymm0[0,1,0,1]
; X64-NEXT:    vpshufb {{.*#+}} ymm0 = ymm0[15,14],zero,zero,zero,zero,zero,zero,ymm0[13,12],zero,zero,zero,zero,zero,zero,ymm0[31,30],zero,zero,zero,zero,zero,zero,ymm0[29,28],zero,zero,zero,zero,zero,zero
; X64-NEXT:    retq
  %1 = shufflevector <32 x i8> %a0, <32 x i8> undef, <32 x i32> <i32 15, i32 14, i32 13, i32 12, i32 11, i32 10, i32 9, i32 8, i32 7, i32 6, i32 5, i32 4, i32 3, i32 2, i32 1, i32 0, i32 15, i32 14, i32 13, i32 12, i32 11, i32 10, i32 9, i32 8, i32 7, i32 6, i32 5, i32 4, i32 3, i32 2, i32 1, i32 0>
  %2 = call <32 x i8> @llvm.x86.avx2.pshuf.b(<32 x i8> %1, <32 x i8> <i8 15, i8 14, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 13, i8 12, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 15, i8 14, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 13, i8 12, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1>)
  %3 = bitcast <32 x i8> %2 to <4 x i64>
  ret <4 x i64> %3
}

define <4 x double> @combine_pshufb_as_vzmovl_64(<4 x double> %a0) {
; X32-LABEL: combine_pshufb_as_vzmovl_64:
; X32:       # BB#0:
; X32-NEXT:    vxorpd %ymm1, %ymm1, %ymm1
; X32-NEXT:    vblendpd {{.*#+}} ymm0 = ymm0[0],ymm1[1,2,3]
; X32-NEXT:    retl
;
; X64-LABEL: combine_pshufb_as_vzmovl_64:
; X64:       # BB#0:
; X64-NEXT:    vxorpd %ymm1, %ymm1, %ymm1
; X64-NEXT:    vblendpd {{.*#+}} ymm0 = ymm0[0],ymm1[1,2,3]
; X64-NEXT:    retq
  %1 = bitcast <4 x double> %a0 to <32 x i8>
  %2 = call <32 x i8> @llvm.x86.avx2.pshuf.b(<32 x i8> %1, <32 x i8> <i8 0, i8 1, i8 2, i8 3, i8 4, i8 5, i8 6, i8 7, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1>)
  %3 = bitcast <32 x i8> %2 to <4 x double>
  ret <4 x double> %3
}

define <8 x float> @combine_pshufb_as_vzmovl_32(<8 x float> %a0) {
; X32-LABEL: combine_pshufb_as_vzmovl_32:
; X32:       # BB#0:
; X32-NEXT:    vxorps %ymm1, %ymm1, %ymm1
; X32-NEXT:    vblendps {{.*#+}} ymm0 = ymm0[0],ymm1[1,2,3,4,5,6,7]
; X32-NEXT:    retl
;
; X64-LABEL: combine_pshufb_as_vzmovl_32:
; X64:       # BB#0:
; X64-NEXT:    vxorps %ymm1, %ymm1, %ymm1
; X64-NEXT:    vblendps {{.*#+}} ymm0 = ymm0[0],ymm1[1,2,3,4,5,6,7]
; X64-NEXT:    retq
  %1 = bitcast <8 x float> %a0 to <32 x i8>
  %2 = call <32 x i8> @llvm.x86.avx2.pshuf.b(<32 x i8> %1, <32 x i8> <i8 0, i8 1, i8 2, i8 3, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1>)
  %3 = bitcast <32 x i8> %2 to <8 x float>
  ret <8 x float> %3
}

define <32 x i8> @combine_pshufb_as_pslldq(<32 x i8> %a0) {
; X32-LABEL: combine_pshufb_as_pslldq:
; X32:       # BB#0:
; X32-NEXT:    vpslldq {{.*#+}} ymm0 = zero,zero,zero,zero,zero,zero,zero,zero,zero,zero,ymm0[0,1,2,3,4,5],zero,zero,zero,zero,zero,zero,zero,zero,zero,zero,ymm0[16,17,18,19,20,21]
; X32-NEXT:    retl
;
; X64-LABEL: combine_pshufb_as_pslldq:
; X64:       # BB#0:
; X64-NEXT:    vpslldq {{.*#+}} ymm0 = zero,zero,zero,zero,zero,zero,zero,zero,zero,zero,ymm0[0,1,2,3,4,5],zero,zero,zero,zero,zero,zero,zero,zero,zero,zero,ymm0[16,17,18,19,20,21]
; X64-NEXT:    retq
  %res0 = call <32 x i8> @llvm.x86.avx2.pshuf.b(<32 x i8> %a0, <32 x i8> <i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 0, i8 1, i8 2, i8 3, i8 4, i8 5, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 0, i8 1, i8 2, i8 3, i8 4, i8 5>)
  ret <32 x i8> %res0
}

define <32 x i8> @combine_pshufb_as_psrldq(<32 x i8> %a0) {
; X32-LABEL: combine_pshufb_as_psrldq:
; X32:       # BB#0:
; X32-NEXT:    vpsrldq {{.*#+}} ymm0 = ymm0[15],zero,zero,zero,zero,zero,zero,zero,zero,zero,zero,zero,zero,zero,zero,zero,ymm0[31],zero,zero,zero,zero,zero,zero,zero,zero,zero,zero,zero,zero,zero,zero,zero
; X32-NEXT:    retl
;
; X64-LABEL: combine_pshufb_as_psrldq:
; X64:       # BB#0:
; X64-NEXT:    vpsrldq {{.*#+}} ymm0 = ymm0[15],zero,zero,zero,zero,zero,zero,zero,zero,zero,zero,zero,zero,zero,zero,zero,ymm0[31],zero,zero,zero,zero,zero,zero,zero,zero,zero,zero,zero,zero,zero,zero,zero
; X64-NEXT:    retq
  %res0 = call <32 x i8> @llvm.x86.avx2.pshuf.b(<32 x i8> %a0, <32 x i8> <i8 15, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 15, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128, i8 128>)
  ret <32 x i8> %res0
}

define <32 x i8> @combine_pshufb_as_psrlw(<32 x i8> %a0) {
; X32-LABEL: combine_pshufb_as_psrlw:
; X32:       # BB#0:
; X32-NEXT:    vpsrlw $8, %ymm0, %ymm0
; X32-NEXT:    retl
;
; X64-LABEL: combine_pshufb_as_psrlw:
; X64:       # BB#0:
; X64-NEXT:    vpsrlw $8, %ymm0, %ymm0
; X64-NEXT:    retq
  %res0 = call <32 x i8> @llvm.x86.avx2.pshuf.b(<32 x i8> %a0, <32 x i8> <i8 1, i8 128, i8 3, i8 128, i8 5, i8 128, i8 7, i8 128, i8 9, i8 128, i8 11, i8 128, i8 13, i8 128, i8 15, i8 128, i8 17, i8 128, i8 19, i8 128, i8 21, i8 128, i8 23, i8 128, i8 25, i8 128, i8 27, i8 128, i8 29, i8 128, i8 31, i8 128>)
  ret <32 x i8> %res0
}

define <32 x i8> @combine_pshufb_as_pslld(<32 x i8> %a0) {
; X32-LABEL: combine_pshufb_as_pslld:
; X32:       # BB#0:
; X32-NEXT:    vpslld $24, %ymm0, %ymm0
; X32-NEXT:    retl
;
; X64-LABEL: combine_pshufb_as_pslld:
; X64:       # BB#0:
; X64-NEXT:    vpslld $24, %ymm0, %ymm0
; X64-NEXT:    retq
  %res0 = call <32 x i8> @llvm.x86.avx2.pshuf.b(<32 x i8> %a0, <32 x i8> <i8 128, i8 128, i8 128, i8 0, i8 128, i8 128, i8 128, i8 4, i8 128, i8 128, i8 128, i8 8, i8 128, i8 128, i8 128, i8 12, i8 128, i8 128, i8 128, i8 16, i8 128, i8 128, i8 128, i8 20, i8 128, i8 128, i8 128, i8 24, i8 128, i8 128, i8 128, i8 28>)
  ret <32 x i8> %res0
}

define <32 x i8> @combine_pshufb_as_psrlq(<32 x i8> %a0) {
; X32-LABEL: combine_pshufb_as_psrlq:
; X32:       # BB#0:
; X32-NEXT:    vpsrlq $40, %ymm0, %ymm0
; X32-NEXT:    retl
;
; X64-LABEL: combine_pshufb_as_psrlq:
; X64:       # BB#0:
; X64-NEXT:    vpsrlq $40, %ymm0, %ymm0
; X64-NEXT:    retq
  %res0 = call <32 x i8> @llvm.x86.avx2.pshuf.b(<32 x i8> %a0, <32 x i8> <i8 5, i8 6, i8 7, i8 128, i8 128, i8 128, i8 128, i8 128, i8 13, i8 14, i8 15, i8 128, i8 128, i8 128, i8 128, i8 128, i8 21, i8 22, i8 23, i8 128, i8 128, i8 128, i8 128, i8 128, i8 29, i8 30, i8 31, i8 128, i8 128, i8 128, i8 128, i8 128>)
  ret <32 x i8> %res0
}

define <32 x i8> @combine_pshufb_as_pshuflw(<32 x i8> %a0) {
; X32-LABEL: combine_pshufb_as_pshuflw:
; X32:       # BB#0:
; X32-NEXT:    vpshuflw {{.*#+}} ymm0 = ymm0[1,0,3,2,4,5,6,7,9,8,11,10,12,13,14,15]
; X32-NEXT:    retl
;
; X64-LABEL: combine_pshufb_as_pshuflw:
; X64:       # BB#0:
; X64-NEXT:    vpshuflw {{.*#+}} ymm0 = ymm0[1,0,3,2,4,5,6,7,9,8,11,10,12,13,14,15]
; X64-NEXT:    retq
  %res0 = call <32 x i8> @llvm.x86.avx2.pshuf.b(<32 x i8> %a0, <32 x i8> <i8 2, i8 3, i8 0, i8 1, i8 6, i8 7, i8 4, i8 5, i8 8, i8 9, i8 10, i8 11, i8 12, i8 13, i8 14, i8 15, i8 2, i8 3, i8 0, i8 1, i8 6, i8 7, i8 4, i8 5, i8 8, i8 9, i8 10, i8 11, i8 12, i8 13, i8 14, i8 15>)
  ret <32 x i8> %res0
}

define <32 x i8> @combine_pshufb_as_pshufhw(<32 x i8> %a0) {
; X32-LABEL: combine_pshufb_as_pshufhw:
; X32:       # BB#0:
; X32-NEXT:    vpshufhw {{.*#+}} ymm0 = ymm0[0,1,2,3,5,4,7,6,8,9,10,11,13,12,15,14]
; X32-NEXT:    retl
;
; X64-LABEL: combine_pshufb_as_pshufhw:
; X64:       # BB#0:
; X64-NEXT:    vpshufhw {{.*#+}} ymm0 = ymm0[0,1,2,3,5,4,7,6,8,9,10,11,13,12,15,14]
; X64-NEXT:    retq
  %res0 = call <32 x i8> @llvm.x86.avx2.pshuf.b(<32 x i8> %a0, <32 x i8> <i8 0, i8 1, i8 2, i8 3, i8 4, i8 5, i8 6, i8 7, i8 10, i8 11, i8 8, i8 9, i8 14, i8 15, i8 12, i8 13, i8 0, i8 1, i8 2, i8 3, i8 4, i8 5, i8 6, i8 7, i8 10, i8 11, i8 8, i8 9, i8 14, i8 15, i8 12, i8 13>)
  ret <32 x i8> %res0
}

define <32 x i8> @combine_pshufb_not_as_pshufw(<32 x i8> %a0) {
; X32-LABEL: combine_pshufb_not_as_pshufw:
; X32:       # BB#0:
; X32-NEXT:    vpshufb {{.*#+}} ymm0 = ymm0[2,3,0,1,6,7,4,5,10,11,8,9,14,15,12,13,18,19,16,17,22,23,20,21,26,27,24,25,30,31,28,29]
; X32-NEXT:    retl
;
; X64-LABEL: combine_pshufb_not_as_pshufw:
; X64:       # BB#0:
; X64-NEXT:    vpshufb {{.*#+}} ymm0 = ymm0[2,3,0,1,6,7,4,5,10,11,8,9,14,15,12,13,18,19,16,17,22,23,20,21,26,27,24,25,30,31,28,29]
; X64-NEXT:    retq
  %res0 = call <32 x i8> @llvm.x86.avx2.pshuf.b(<32 x i8> %a0, <32 x i8> <i8 2, i8 3, i8 0, i8 1, i8 6, i8 7, i8 4, i8 5, i8 8, i8 9, i8 10, i8 11, i8 12, i8 13, i8 14, i8 15, i8 2, i8 3, i8 0, i8 1, i8 6, i8 7, i8 4, i8 5, i8 8, i8 9, i8 10, i8 11, i8 12, i8 13, i8 14, i8 15>)
  %res1 = call <32 x i8> @llvm.x86.avx2.pshuf.b(<32 x i8> %res0, <32 x i8> <i8 0, i8 1, i8 2, i8 3, i8 4, i8 5, i8 6, i8 7, i8 10, i8 11, i8 8, i8 9, i8 14, i8 15, i8 12, i8 13, i8 0, i8 1, i8 2, i8 3, i8 4, i8 5, i8 6, i8 7, i8 10, i8 11, i8 8, i8 9, i8 14, i8 15, i8 12, i8 13>)
  ret <32 x i8> %res1
}

define <32 x i8> @combine_psrlw_pshufb(<16 x i16> %a0) {
; X32-LABEL: combine_psrlw_pshufb:
; X32:       # BB#0:
; X32-NEXT:    vpshufb {{.*#+}} ymm0 = zero,ymm0[1],zero,ymm0[3],zero,ymm0[5],zero,ymm0[7],zero,ymm0[9],zero,ymm0[11],zero,ymm0[13],zero,ymm0[15],zero,ymm0[17],zero,ymm0[19],zero,ymm0[21],zero,ymm0[23],zero,ymm0[25],zero,ymm0[27],zero,ymm0[29],zero,ymm0[31]
; X32-NEXT:    retl
;
; X64-LABEL: combine_psrlw_pshufb:
; X64:       # BB#0:
; X64-NEXT:    vpshufb {{.*#+}} ymm0 = zero,ymm0[1],zero,ymm0[3],zero,ymm0[5],zero,ymm0[7],zero,ymm0[9],zero,ymm0[11],zero,ymm0[13],zero,ymm0[15],zero,ymm0[17],zero,ymm0[19],zero,ymm0[21],zero,ymm0[23],zero,ymm0[25],zero,ymm0[27],zero,ymm0[29],zero,ymm0[31]
; X64-NEXT:    retq
  %1 = lshr <16 x i16> %a0, <i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8>
  %2 = bitcast <16 x i16> %1 to <32 x i8>
  %3 = tail call <32 x i8> @llvm.x86.avx2.pshuf.b(<32 x i8> %2, <32 x i8> <i8 1, i8 0, i8 3, i8 2, i8 5, i8 4, i8 7, i8 6, i8 9, i8 8, i8 11, i8 10, i8 13, i8 12, i8 15, i8 14, i8 17, i8 16, i8 19, i8 18, i8 21, i8 20, i8 23, i8 22, i8 25, i8 24, i8 27, i8 26, i8 29, i8 28, i8 31, i8 30>)
  ret <32 x i8> %3
}

define <32 x i8> @combine_pslld_pshufb(<8 x i32> %a0) {
; X32-LABEL: combine_pslld_pshufb:
; X32:       # BB#0:
; X32-NEXT:    vpshufb {{.*#+}} ymm0 = ymm0[0],zero,zero,zero,ymm0[4],zero,zero,zero,ymm0[8],zero,zero,zero,ymm0[12],zero,zero,zero,ymm0[16],zero,zero,zero,ymm0[20],zero,zero,zero,ymm0[24],zero,zero,zero,ymm0[28],zero,zero,zero
; X32-NEXT:    retl
;
; X64-LABEL: combine_pslld_pshufb:
; X64:       # BB#0:
; X64-NEXT:    vpshufb {{.*#+}} ymm0 = ymm0[0],zero,zero,zero,ymm0[4],zero,zero,zero,ymm0[8],zero,zero,zero,ymm0[12],zero,zero,zero,ymm0[16],zero,zero,zero,ymm0[20],zero,zero,zero,ymm0[24],zero,zero,zero,ymm0[28],zero,zero,zero
; X64-NEXT:    retq
  %1 = shl <8 x i32> %a0, <i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24>
  %2 = bitcast <8 x i32> %1 to <32 x i8>
  %3 = tail call <32 x i8> @llvm.x86.avx2.pshuf.b(<32 x i8> %2, <32 x i8> <i8 3, i8 2, i8 1, i8 0, i8 7, i8 6, i8 5, i8 4, i8 11, i8 10, i8 9, i8 8, i8 15, i8 14, i8 13, i8 12, i8 19, i8 18, i8 17, i8 16, i8 23, i8 22, i8 21, i8 20, i8 27, i8 26, i8 25, i8 24, i8 31, i8 30, i8 29, i8 28>)
  ret <32 x i8> %3
}

define <32 x i8> @combine_psrlq_pshufb(<4 x i64> %a0) {
; X32-LABEL: combine_psrlq_pshufb:
; X32:       # BB#0:
; X32-NEXT:    vpshufb {{.*#+}} ymm0 = zero,zero,zero,zero,ymm0[7,6,5,4],zero,zero,zero,zero,ymm0[15,14,13,12],zero,zero,zero,zero,ymm0[23,22,21],zero,zero,zero,zero,ymm0[31,30,29,28],zero
; X32-NEXT:    retl
;
; X64-LABEL: combine_psrlq_pshufb:
; X64:       # BB#0:
; X64-NEXT:    vpshufb {{.*#+}} ymm0 = zero,zero,zero,zero,ymm0[7,6,5,4],zero,zero,zero,zero,ymm0[15,14,13,12],zero,zero,zero,zero,ymm0[23,22,21],zero,zero,zero,zero,ymm0[31,30,29,28],zero
; X64-NEXT:    retq
  %1 = lshr <4 x i64> %a0, <i64 32, i64 32, i64 32, i64 32>
  %2 = bitcast <4 x i64> %1 to <32 x i8>
  %3 = tail call <32 x i8> @llvm.x86.avx2.pshuf.b(<32 x i8> %2, <32 x i8> <i8 7, i8 6, i8 5, i8 4, i8 3, i8 2, i8 1, i8 0, i8 15, i8 14, i8 13, i8 12, i8 11, i8 10, i8 9, i8 8, i8 23, i8 22, i8 21, i8 20, i8 19, i8 18, i8 17, i8 31, i8 30, i8 29, i8 28, i8 27, i8 26, i8 25, i8 24, i8 23>)
  ret <32 x i8> %3
}

define <8 x i32> @constant_fold_permd() {
; X32-LABEL: constant_fold_permd:
; X32:       # BB#0:
; X32-NEXT:    vmovdqa {{.*#+}} ymm0 = [4,6,2,1,7,1,5,0]
; X32-NEXT:    vpermd {{\.LCPI.*}}, %ymm0, %ymm0
; X32-NEXT:    retl
;
; X64-LABEL: constant_fold_permd:
; X64:       # BB#0:
; X64-NEXT:    vmovdqa {{.*#+}} ymm0 = [4,6,2,1,7,1,5,0]
; X64-NEXT:    vpermd {{.*}}(%rip), %ymm0, %ymm0
; X64-NEXT:    retq
  %1 = call <8 x i32> @llvm.x86.avx2.permd(<8 x i32> <i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8>, <8 x i32> <i32 4, i32 6, i32 2, i32 1, i32 7, i32 1, i32 5, i32 0>)
  ret <8 x i32> %1
}

define <8 x float> @constant_fold_permps() {
; X32-LABEL: constant_fold_permps:
; X32:       # BB#0:
; X32-NEXT:    vmovaps {{.*#+}} ymm0 = [4,6,2,1,7,1,5,0]
; X32-NEXT:    vpermps {{\.LCPI.*}}, %ymm0, %ymm0
; X32-NEXT:    retl
;
; X64-LABEL: constant_fold_permps:
; X64:       # BB#0:
; X64-NEXT:    vmovaps {{.*#+}} ymm0 = [4,6,2,1,7,1,5,0]
; X64-NEXT:    vpermps {{.*}}(%rip), %ymm0, %ymm0
; X64-NEXT:    retq
  %1 = call <8 x float> @llvm.x86.avx2.permps(<8 x float> <float 1.0, float 2.0, float 3.0, float 4.0, float 5.0, float 6.0, float 7.0, float 8.0>, <8 x i32> <i32 4, i32 6, i32 2, i32 1, i32 7, i32 1, i32 5, i32 0>)
  ret <8 x float> %1
}

define <32 x i8> @constant_fold_pshufb_256() {
; X32-LABEL: constant_fold_pshufb_256:
; X32:       # BB#0:
; X32-NEXT:    vmovdqa {{.*#+}} ymm0 = [15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,0,255,254,253,252,251,250,249,248,247,246,245,244,243,242,241]
; X32-NEXT:    vpshufb {{.*#+}} ymm0 = ymm0[1],zero,zero,zero,ymm0[u,u],zero,zero,ymm0[15],zero,zero,zero,zero,zero,ymm0[7,6,17],zero,zero,zero,ymm0[u,u],zero,zero,ymm0[31],zero,zero,zero,zero,zero,ymm0[23,22]
; X32-NEXT:    retl
;
; X64-LABEL: constant_fold_pshufb_256:
; X64:       # BB#0:
; X64-NEXT:    vmovdqa {{.*#+}} ymm0 = [15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,0,255,254,253,252,251,250,249,248,247,246,245,244,243,242,241]
; X64-NEXT:    vpshufb {{.*#+}} ymm0 = ymm0[1],zero,zero,zero,ymm0[u,u],zero,zero,ymm0[15],zero,zero,zero,zero,zero,ymm0[7,6,17],zero,zero,zero,ymm0[u,u],zero,zero,ymm0[31],zero,zero,zero,zero,zero,ymm0[23,22]
; X64-NEXT:    retq
  %1 = tail call <32 x i8> @llvm.x86.avx2.pshuf.b(<32 x i8> <i8 15, i8 14, i8 13, i8 12, i8 11, i8 10, i8 9, i8 8, i8 7, i8 6, i8 5, i8 4, i8 3, i8 2, i8 1, i8 0, i8 0, i8 -1, i8 -2, i8 -3, i8 -4, i8 -5, i8 -6, i8 -7, i8 -8, i8 -9, i8 -10, i8 -11, i8 -12, i8 -13, i8 -14, i8 -15>, <32 x i8> <i8 1, i8 -1, i8 -1, i8 -1, i8 undef, i8 undef, i8 -1, i8 -1, i8 15, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 7, i8 6, i8 1, i8 -1, i8 -1, i8 -1, i8 undef, i8 undef, i8 -1, i8 -1, i8 15, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 7, i8 6>)
  ret <32 x i8> %1
}
