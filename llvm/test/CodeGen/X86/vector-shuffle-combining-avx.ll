; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=i686-unknown -mattr=+avx | FileCheck %s --check-prefix=X32
; RUN: llc < %s -mtriple=i686-unknown -mattr=+avx2 | FileCheck %s --check-prefix=X32
; RUN: llc < %s -mtriple=i686-unknown -mattr=+avx512f | FileCheck %s --check-prefix=X32
; RUN: llc < %s -mtriple=x86_64-unknown -mattr=+avx | FileCheck %s --check-prefix=X64
; RUN: llc < %s -mtriple=x86_64-unknown -mattr=+avx2 | FileCheck %s --check-prefix=X64
; RUN: llc < %s -mtriple=x86_64-unknown -mattr=+avx512f | FileCheck %s --check-prefix=X64
;
; Combine tests involving AVX target shuffles

declare <4 x float> @llvm.x86.avx.vpermil.ps(<4 x float>, i8)
declare <8 x float> @llvm.x86.avx.vpermil.ps.256(<8 x float>, i8)
declare <2 x double> @llvm.x86.avx.vpermil.pd(<2 x double>, i8)
declare <4 x double> @llvm.x86.avx.vpermil.pd.256(<4 x double>, i8)

declare <4 x float> @llvm.x86.avx.vpermilvar.ps(<4 x float>, <4 x i32>)
declare <8 x float> @llvm.x86.avx.vpermilvar.ps.256(<8 x float>, <8 x i32>)
declare <2 x double> @llvm.x86.avx.vpermilvar.pd(<2 x double>, <2 x i64>)
declare <4 x double> @llvm.x86.avx.vpermilvar.pd.256(<4 x double>, <4 x i64>)

declare <8 x i32> @llvm.x86.avx.vperm2f128.si.256(<8 x i32>, <8 x i32>, i8)
declare <8 x float> @llvm.x86.avx.vperm2f128.ps.256(<8 x float>, <8 x float>, i8)
declare <4 x double> @llvm.x86.avx.vperm2f128.pd.256(<4 x double>, <4 x double>, i8)

define <4 x float> @combine_vpermilvar_4f32_identity(<4 x float> %a0) {
; X32-LABEL: combine_vpermilvar_4f32_identity:
; X32:       # BB#0:
; X32-NEXT:    retl
;
; X64-LABEL: combine_vpermilvar_4f32_identity:
; X64:       # BB#0:
; X64-NEXT:    retq
  %1 = tail call <4 x float> @llvm.x86.avx.vpermilvar.ps(<4 x float> %a0, <4 x i32> <i32 3, i32 2, i32 1, i32 0>)
  %2 = tail call <4 x float> @llvm.x86.avx.vpermilvar.ps(<4 x float>  %1, <4 x i32> <i32 3, i32 2, i32 1, i32 0>)
  ret <4 x float> %2
}

define <4 x float> @combine_vpermilvar_4f32_movddup(<4 x float> %a0) {
; X32-LABEL: combine_vpermilvar_4f32_movddup:
; X32:       # BB#0:
; X32-NEXT:    vmovddup {{.*#+}} xmm0 = xmm0[0,0]
; X32-NEXT:    retl
;
; X64-LABEL: combine_vpermilvar_4f32_movddup:
; X64:       # BB#0:
; X64-NEXT:    vmovddup {{.*#+}} xmm0 = xmm0[0,0]
; X64-NEXT:    retq
  %1 = tail call <4 x float> @llvm.x86.avx.vpermilvar.ps(<4 x float> %a0, <4 x i32> <i32 0, i32 1, i32 0, i32 1>)
  ret <4 x float> %1
}
define <4 x float> @combine_vpermilvar_4f32_movddup_load(<4 x float> *%a0) {
; X32-LABEL: combine_vpermilvar_4f32_movddup_load:
; X32:       # BB#0:
; X32-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-NEXT:    vmovddup {{.*#+}} xmm0 = mem[0,0]
; X32-NEXT:    retl
;
; X64-LABEL: combine_vpermilvar_4f32_movddup_load:
; X64:       # BB#0:
; X64-NEXT:    vmovddup {{.*#+}} xmm0 = mem[0,0]
; X64-NEXT:    retq
  %1 = load <4 x float>, <4 x float> *%a0
  %2 = tail call <4 x float> @llvm.x86.avx.vpermilvar.ps(<4 x float> %1, <4 x i32> <i32 0, i32 1, i32 0, i32 1>)
  ret <4 x float> %2
}

define <4 x float> @combine_vpermilvar_4f32_movshdup(<4 x float> %a0) {
; X32-LABEL: combine_vpermilvar_4f32_movshdup:
; X32:       # BB#0:
; X32-NEXT:    vmovshdup {{.*#+}} xmm0 = xmm0[1,1,3,3]
; X32-NEXT:    retl
;
; X64-LABEL: combine_vpermilvar_4f32_movshdup:
; X64:       # BB#0:
; X64-NEXT:    vmovshdup {{.*#+}} xmm0 = xmm0[1,1,3,3]
; X64-NEXT:    retq
  %1 = tail call <4 x float> @llvm.x86.avx.vpermilvar.ps(<4 x float> %a0, <4 x i32> <i32 undef, i32 1, i32 3, i32 3>)
  ret <4 x float> %1
}

define <4 x float> @combine_vpermilvar_4f32_movsldup(<4 x float> %a0) {
; X32-LABEL: combine_vpermilvar_4f32_movsldup:
; X32:       # BB#0:
; X32-NEXT:    vmovsldup {{.*#+}} xmm0 = xmm0[0,0,2,2]
; X32-NEXT:    retl
;
; X64-LABEL: combine_vpermilvar_4f32_movsldup:
; X64:       # BB#0:
; X64-NEXT:    vmovsldup {{.*#+}} xmm0 = xmm0[0,0,2,2]
; X64-NEXT:    retq
  %1 = tail call <4 x float> @llvm.x86.avx.vpermilvar.ps(<4 x float> %a0, <4 x i32> <i32 0, i32 0, i32 2, i32 undef>)
  ret <4 x float> %1
}

define <4 x float> @combine_vpermilvar_4f32_unpckh(<4 x float> %a0) {
; X32-LABEL: combine_vpermilvar_4f32_unpckh:
; X32:       # BB#0:
; X32-NEXT:    vpermilps {{.*#+}} xmm0 = xmm0[2,2,3,3]
; X32-NEXT:    retl
;
; X64-LABEL: combine_vpermilvar_4f32_unpckh:
; X64:       # BB#0:
; X64-NEXT:    vpermilps {{.*#+}} xmm0 = xmm0[2,2,3,3]
; X64-NEXT:    retq
  %1 = tail call <4 x float> @llvm.x86.avx.vpermilvar.ps(<4 x float> %a0, <4 x i32> <i32 2, i32 2, i32 3, i32 3>)
  ret <4 x float> %1
}

define <4 x float> @combine_vpermilvar_4f32_unpckl(<4 x float> %a0) {
; X32-LABEL: combine_vpermilvar_4f32_unpckl:
; X32:       # BB#0:
; X32-NEXT:    vpermilps {{.*#+}} xmm0 = xmm0[0,0,1,1]
; X32-NEXT:    retl
;
; X64-LABEL: combine_vpermilvar_4f32_unpckl:
; X64:       # BB#0:
; X64-NEXT:    vpermilps {{.*#+}} xmm0 = xmm0[0,0,1,1]
; X64-NEXT:    retq
  %1 = tail call <4 x float> @llvm.x86.avx.vpermilvar.ps(<4 x float> %a0, <4 x i32> <i32 0, i32 0, i32 1, i32 1>)
  ret <4 x float> %1
}

define <8 x float> @combine_vpermilvar_8f32_identity(<8 x float> %a0) {
; X32-LABEL: combine_vpermilvar_8f32_identity:
; X32:       # BB#0:
; X32-NEXT:    retl
;
; X64-LABEL: combine_vpermilvar_8f32_identity:
; X64:       # BB#0:
; X64-NEXT:    retq
  %1 = tail call <8 x float> @llvm.x86.avx.vpermilvar.ps.256(<8 x float> %a0, <8 x i32> <i32 3, i32 2, i32 1, i32 0, i32 2, i32 3, i32 0, i32 undef>)
  %2 = tail call <8 x float> @llvm.x86.avx.vpermilvar.ps.256(<8 x float>  %1, <8 x i32> <i32 3, i32 2, i32 1, i32 0, i32 2, i32 3, i32 0, i32 1>)
  ret <8 x float> %2
}

define <8 x float> @combine_vpermilvar_8f32_10326u4u(<8 x float> %a0) {
; X32-LABEL: combine_vpermilvar_8f32_10326u4u:
; X32:       # BB#0:
; X32-NEXT:    vpermilps {{.*#+}} ymm0 = ymm0[1,0,3,2,6,u,4,u]
; X32-NEXT:    retl
;
; X64-LABEL: combine_vpermilvar_8f32_10326u4u:
; X64:       # BB#0:
; X64-NEXT:    vpermilps {{.*#+}} ymm0 = ymm0[1,0,3,2,6,u,4,u]
; X64-NEXT:    retq
  %1 = tail call <8 x float> @llvm.x86.avx.vpermilvar.ps.256(<8 x float> %a0, <8 x i32> <i32 3, i32 2, i32 1, i32 0, i32 0, i32 1, i32 2, i32 undef>)
  %2 = tail call <8 x float> @llvm.x86.avx.vpermilvar.ps.256(<8 x float>  %1, <8 x i32> <i32 2, i32 3, i32 0, i32 1, i32 2, i32 3, i32 0, i32 undef>)
  ret <8 x float> %2
}

define <8 x float> @combine_vpermilvar_vperm2f128_8f32(<8 x float> %a0) {
; X32-LABEL: combine_vpermilvar_vperm2f128_8f32:
; X32:       # BB#0:
; X32-NEXT:    vperm2f128 {{.*#+}} ymm0 = ymm0[2,3,0,1]
; X32-NEXT:    retl
;
; X64-LABEL: combine_vpermilvar_vperm2f128_8f32:
; X64:       # BB#0:
; X64-NEXT:    vperm2f128 {{.*#+}} ymm0 = ymm0[2,3,0,1]
; X64-NEXT:    retq
  %1 = tail call <8 x float> @llvm.x86.avx.vpermilvar.ps.256(<8 x float> %a0, <8 x i32> <i32 3, i32 2, i32 1, i32 0, i32 3, i32 2, i32 1, i32 0>)
  %2 = shufflevector <8 x float> %1, <8 x float> undef, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 0, i32 1, i32 2, i32 3>
  %3 = tail call <8 x float> @llvm.x86.avx.vpermilvar.ps.256(<8 x float>  %2, <8 x i32> <i32 3, i32 2, i32 1, i32 0, i32 3, i32 2, i32 1, i32 0>)
  ret <8 x float> %3
}

define <8 x float> @combine_vpermilvar_vperm2f128_zero_8f32(<8 x float> %a0) {
; X32-LABEL: combine_vpermilvar_vperm2f128_zero_8f32:
; X32:       # BB#0:
; X32-NEXT:    vperm2f128 {{.*#+}} ymm0 = zero,zero,ymm0[0,1]
; X32-NEXT:    retl
;
; X64-LABEL: combine_vpermilvar_vperm2f128_zero_8f32:
; X64:       # BB#0:
; X64-NEXT:    vperm2f128 {{.*#+}} ymm0 = zero,zero,ymm0[0,1]
; X64-NEXT:    retq
  %1 = tail call <8 x float> @llvm.x86.avx.vpermilvar.ps.256(<8 x float> %a0, <8 x i32> <i32 3, i32 2, i32 1, i32 0, i32 3, i32 2, i32 1, i32 0>)
  %2 = shufflevector <8 x float> %1, <8 x float> zeroinitializer, <8 x i32> <i32 8, i32 8, i32 8, i32 8, i32 0, i32 1, i32 2, i32 3>
  %3 = tail call <8 x float> @llvm.x86.avx.vpermilvar.ps.256(<8 x float>  %2, <8 x i32> <i32 3, i32 2, i32 1, i32 0, i32 3, i32 2, i32 1, i32 0>)
  ret <8 x float> %3
}

define <4 x double> @combine_vperm2f128_vpermilvar_as_vpblendpd(<4 x double> %a0) {
; X32-LABEL: combine_vperm2f128_vpermilvar_as_vpblendpd:
; X32:       # BB#0:
; X32-NEXT:    vxorpd %ymm1, %ymm1, %ymm1
; X32-NEXT:    vblendpd {{.*#+}} ymm0 = ymm0[0,1],ymm1[2,3]
; X32-NEXT:    retl
;
; X64-LABEL: combine_vperm2f128_vpermilvar_as_vpblendpd:
; X64:       # BB#0:
; X64-NEXT:    vxorpd %ymm1, %ymm1, %ymm1
; X64-NEXT:    vblendpd {{.*#+}} ymm0 = ymm0[0,1],ymm1[2,3]
; X64-NEXT:    retq
  %1 = tail call <4 x double> @llvm.x86.avx.vpermilvar.pd.256(<4 x double> %a0, <4 x i64> <i64 2, i64 0, i64 2, i64 0>)
  %2 = shufflevector <4 x double> %1, <4 x double> zeroinitializer, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %3 = tail call <4 x double> @llvm.x86.avx.vpermilvar.pd.256(<4 x double> %2, <4 x i64> <i64 2, i64 0, i64 2, i64 0>)
  ret <4 x double> %3
}

define <8 x float> @combine_vpermilvar_8f32_movddup(<8 x float> %a0) {
; X32-LABEL: combine_vpermilvar_8f32_movddup:
; X32:       # BB#0:
; X32-NEXT:    vmovddup {{.*#+}} ymm0 = ymm0[0,0,2,2]
; X32-NEXT:    retl
;
; X64-LABEL: combine_vpermilvar_8f32_movddup:
; X64:       # BB#0:
; X64-NEXT:    vmovddup {{.*#+}} ymm0 = ymm0[0,0,2,2]
; X64-NEXT:    retq
  %1 = tail call <8 x float> @llvm.x86.avx.vpermilvar.ps.256(<8 x float> %a0, <8 x i32> <i32 0, i32 1, i32 0, i32 1, i32 4, i32 5, i32 4, i32 5>)
  ret <8 x float> %1
}
define <8 x float> @combine_vpermilvar_8f32_movddup_load(<8 x float> *%a0) {
; X32-LABEL: combine_vpermilvar_8f32_movddup_load:
; X32:       # BB#0:
; X32-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-NEXT:    vmovddup {{.*#+}} ymm0 = mem[0,0,2,2]
; X32-NEXT:    retl
;
; X64-LABEL: combine_vpermilvar_8f32_movddup_load:
; X64:       # BB#0:
; X64-NEXT:    vmovddup {{.*#+}} ymm0 = mem[0,0,2,2]
; X64-NEXT:    retq
  %1 = load <8 x float>, <8 x float> *%a0
  %2 = tail call <8 x float> @llvm.x86.avx.vpermilvar.ps.256(<8 x float> %1, <8 x i32> <i32 0, i32 1, i32 0, i32 1, i32 4, i32 5, i32 4, i32 5>)
  ret <8 x float> %2
}

define <8 x float> @combine_vpermilvar_8f32_movshdup(<8 x float> %a0) {
; X32-LABEL: combine_vpermilvar_8f32_movshdup:
; X32:       # BB#0:
; X32-NEXT:    vmovshdup {{.*#+}} ymm0 = ymm0[1,1,3,3,5,5,7,7]
; X32-NEXT:    retl
;
; X64-LABEL: combine_vpermilvar_8f32_movshdup:
; X64:       # BB#0:
; X64-NEXT:    vmovshdup {{.*#+}} ymm0 = ymm0[1,1,3,3,5,5,7,7]
; X64-NEXT:    retq
  %1 = tail call <8 x float> @llvm.x86.avx.vpermilvar.ps.256(<8 x float> %a0, <8 x i32> <i32 1, i32 1, i32 3, i32 3, i32 undef, i32 5, i32 7, i32 7>)
  ret <8 x float> %1
}

define <8 x float> @combine_vpermilvar_8f32_movsldup(<8 x float> %a0) {
; X32-LABEL: combine_vpermilvar_8f32_movsldup:
; X32:       # BB#0:
; X32-NEXT:    vmovsldup {{.*#+}} ymm0 = ymm0[0,0,2,2,4,4,6,6]
; X32-NEXT:    retl
;
; X64-LABEL: combine_vpermilvar_8f32_movsldup:
; X64:       # BB#0:
; X64-NEXT:    vmovsldup {{.*#+}} ymm0 = ymm0[0,0,2,2,4,4,6,6]
; X64-NEXT:    retq
  %1 = tail call <8 x float> @llvm.x86.avx.vpermilvar.ps.256(<8 x float> %a0, <8 x i32> <i32 0, i32 0, i32 2, i32 2, i32 4, i32 4, i32 6, i32 6>)
  ret <8 x float> %1
}

define <2 x double> @combine_vpermilvar_2f64_identity(<2 x double> %a0) {
; X32-LABEL: combine_vpermilvar_2f64_identity:
; X32:       # BB#0:
; X32-NEXT:    retl
;
; X64-LABEL: combine_vpermilvar_2f64_identity:
; X64:       # BB#0:
; X64-NEXT:    retq
  %1 = tail call <2 x double> @llvm.x86.avx.vpermilvar.pd(<2 x double> %a0, <2 x i64> <i64 2, i64 0>)
  %2 = tail call <2 x double> @llvm.x86.avx.vpermilvar.pd(<2 x double>  %1, <2 x i64> <i64 2, i64 0>)
  ret <2 x double> %2
}

define <2 x double> @combine_vpermilvar_2f64_movddup(<2 x double> %a0) {
; X32-LABEL: combine_vpermilvar_2f64_movddup:
; X32:       # BB#0:
; X32-NEXT:    vmovddup {{.*#+}} xmm0 = xmm0[0,0]
; X32-NEXT:    retl
;
; X64-LABEL: combine_vpermilvar_2f64_movddup:
; X64:       # BB#0:
; X64-NEXT:    vmovddup {{.*#+}} xmm0 = xmm0[0,0]
; X64-NEXT:    retq
  %1 = tail call <2 x double> @llvm.x86.avx.vpermilvar.pd(<2 x double> %a0, <2 x i64> <i64 0, i64 0>)
  ret <2 x double> %1
}

define <4 x double> @combine_vpermilvar_4f64_identity(<4 x double> %a0) {
; X32-LABEL: combine_vpermilvar_4f64_identity:
; X32:       # BB#0:
; X32-NEXT:    retl
;
; X64-LABEL: combine_vpermilvar_4f64_identity:
; X64:       # BB#0:
; X64-NEXT:    retq
  %1 = tail call <4 x double> @llvm.x86.avx.vpermilvar.pd.256(<4 x double> %a0, <4 x i64> <i64 2, i64 0, i64 2, i64 0>)
  %2 = tail call <4 x double> @llvm.x86.avx.vpermilvar.pd.256(<4 x double>  %1, <4 x i64> <i64 2, i64 0, i64 2, i64 0>)
  ret <4 x double> %2
}

define <4 x double> @combine_vpermilvar_4f64_movddup(<4 x double> %a0) {
; X32-LABEL: combine_vpermilvar_4f64_movddup:
; X32:       # BB#0:
; X32-NEXT:    vmovddup {{.*#+}} ymm0 = ymm0[0,0,2,2]
; X32-NEXT:    retl
;
; X64-LABEL: combine_vpermilvar_4f64_movddup:
; X64:       # BB#0:
; X64-NEXT:    vmovddup {{.*#+}} ymm0 = ymm0[0,0,2,2]
; X64-NEXT:    retq
  %1 = tail call <4 x double> @llvm.x86.avx.vpermilvar.pd.256(<4 x double> %a0, <4 x i64> <i64 0, i64 0, i64 4, i64 4>)
  ret <4 x double> %1
}

define <4 x float> @combine_vpermilvar_4f32_4stage(<4 x float> %a0) {
; X32-LABEL: combine_vpermilvar_4f32_4stage:
; X32:       # BB#0:
; X32-NEXT:    vpermilps {{.*#+}} xmm0 = xmm0[2,0,3,1]
; X32-NEXT:    retl
;
; X64-LABEL: combine_vpermilvar_4f32_4stage:
; X64:       # BB#0:
; X64-NEXT:    vpermilps {{.*#+}} xmm0 = xmm0[2,0,3,1]
; X64-NEXT:    retq
  %1 = tail call <4 x float> @llvm.x86.avx.vpermilvar.ps(<4 x float> %a0, <4 x i32> <i32 3, i32 2, i32 1, i32 0>)
  %2 = tail call <4 x float> @llvm.x86.avx.vpermilvar.ps(<4 x float>  %1, <4 x i32> <i32 2, i32 3, i32 0, i32 1>)
  %3 = tail call <4 x float> @llvm.x86.avx.vpermilvar.ps(<4 x float>  %2, <4 x i32> <i32 0, i32 2, i32 1, i32 3>)
  %4 = tail call <4 x float> @llvm.x86.avx.vpermilvar.ps(<4 x float>  %3, <4 x i32> <i32 3, i32 2, i32 1, i32 0>)
  ret <4 x float> %4
}

define <8 x float> @combine_vpermilvar_8f32_4stage(<8 x float> %a0) {
; X32-LABEL: combine_vpermilvar_8f32_4stage:
; X32:       # BB#0:
; X32-NEXT:    vpermilps {{.*#+}} ymm0 = ymm0[2,0,3,1,6,4,7,5]
; X32-NEXT:    retl
;
; X64-LABEL: combine_vpermilvar_8f32_4stage:
; X64:       # BB#0:
; X64-NEXT:    vpermilps {{.*#+}} ymm0 = ymm0[2,0,3,1,6,4,7,5]
; X64-NEXT:    retq
  %1 = tail call <8 x float> @llvm.x86.avx.vpermilvar.ps.256(<8 x float> %a0, <8 x i32> <i32 3, i32 2, i32 1, i32 0, i32 3, i32 2, i32 1, i32 0>)
  %2 = tail call <8 x float> @llvm.x86.avx.vpermilvar.ps.256(<8 x float>  %1, <8 x i32> <i32 2, i32 3, i32 0, i32 1, i32 2, i32 3, i32 0, i32 1>)
  %3 = tail call <8 x float> @llvm.x86.avx.vpermilvar.ps.256(<8 x float>  %2, <8 x i32> <i32 0, i32 2, i32 1, i32 3, i32 0, i32 2, i32 1, i32 3>)
  %4 = tail call <8 x float> @llvm.x86.avx.vpermilvar.ps.256(<8 x float>  %3, <8 x i32> <i32 3, i32 2, i32 1, i32 0, i32 3, i32 2, i32 1, i32 0>)
  ret <8 x float> %4
}

define <4 x float> @combine_vpermilvar_4f32_as_insertps(<4 x float> %a0) {
; X32-LABEL: combine_vpermilvar_4f32_as_insertps:
; X32:       # BB#0:
; X32-NEXT:    vinsertps {{.*#+}} xmm0 = xmm0[1],zero,xmm0[2],zero
; X32-NEXT:    retl
;
; X64-LABEL: combine_vpermilvar_4f32_as_insertps:
; X64:       # BB#0:
; X64-NEXT:    vinsertps {{.*#+}} xmm0 = xmm0[1],zero,xmm0[2],zero
; X64-NEXT:    retq
  %1 = call <4 x float> @llvm.x86.avx.vpermilvar.ps(<4 x float> %a0, <4 x i32> <i32 3, i32 2, i32 1, i32 0>)
  %2 = shufflevector <4 x float> %1, <4 x float> zeroinitializer, <4 x i32> <i32 2, i32 4, i32 1, i32 4>
  ret <4 x float> %2
}

define <2 x double> @constant_fold_vpermilvar_pd() {
; X32-LABEL: constant_fold_vpermilvar_pd:
; X32:       # BB#0:
; X32-NEXT:    vpermilpd {{.*#+}} xmm0 = mem[1,0]
; X32-NEXT:    retl
;
; X64-LABEL: constant_fold_vpermilvar_pd:
; X64:       # BB#0:
; X64-NEXT:    vpermilpd {{.*#+}} xmm0 = mem[1,0]
; X64-NEXT:    retq
  %1 = call <2 x double> @llvm.x86.avx.vpermilvar.pd(<2 x double> <double 1.0, double 2.0>, <2 x i64> <i64 2, i64 0>)
  ret <2 x double> %1
}

define <4 x double> @constant_fold_vpermilvar_pd_256() {
; X32-LABEL: constant_fold_vpermilvar_pd_256:
; X32:       # BB#0:
; X32-NEXT:    vpermilpd {{.*#+}} ymm0 = mem[1,0,2,3]
; X32-NEXT:    retl
;
; X64-LABEL: constant_fold_vpermilvar_pd_256:
; X64:       # BB#0:
; X64-NEXT:    vpermilpd {{.*#+}} ymm0 = mem[1,0,2,3]
; X64-NEXT:    retq
  %1 = call <4 x double> @llvm.x86.avx.vpermilvar.pd.256(<4 x double> <double 1.0, double 2.0, double 3.0, double 4.0>, <4 x i64> <i64 2, i64 0, i64 0, i64 2>)
  ret <4 x double> %1
}

define <4 x float> @constant_fold_vpermilvar_ps() {
; X32-LABEL: constant_fold_vpermilvar_ps:
; X32:       # BB#0:
; X32-NEXT:    vpermilps {{.*#+}} xmm0 = mem[3,0,2,1]
; X32-NEXT:    retl
;
; X64-LABEL: constant_fold_vpermilvar_ps:
; X64:       # BB#0:
; X64-NEXT:    vpermilps {{.*#+}} xmm0 = mem[3,0,2,1]
; X64-NEXT:    retq
  %1 = call <4 x float> @llvm.x86.avx.vpermilvar.ps(<4 x float> <float 1.0, float 2.0, float 3.0, float 4.0>, <4 x i32> <i32 3, i32 0, i32 2, i32 1>)
  ret <4 x float> %1
}

define <8 x float> @constant_fold_vpermilvar_ps_256() {
; X32-LABEL: constant_fold_vpermilvar_ps_256:
; X32:       # BB#0:
; X32-NEXT:    vmovaps {{.*#+}} ymm0 = [1.000000e+00,2.000000e+00,3.000000e+00,4.000000e+00,5.000000e+00,6.000000e+00,7.000000e+00,8.000000e+00]
; X32-NEXT:    vpermilps {{.*#+}} ymm0 = ymm0[0,0,2,1,4,5,5,5]
; X32-NEXT:    retl
;
; X64-LABEL: constant_fold_vpermilvar_ps_256:
; X64:       # BB#0:
; X64-NEXT:    vmovaps {{.*#+}} ymm0 = [1.000000e+00,2.000000e+00,3.000000e+00,4.000000e+00,5.000000e+00,6.000000e+00,7.000000e+00,8.000000e+00]
; X64-NEXT:    vpermilps {{.*#+}} ymm0 = ymm0[0,0,2,1,4,5,5,5]
; X64-NEXT:    retq
  %1 = call <8 x float> @llvm.x86.avx.vpermilvar.ps.256(<8 x float> <float 1.0, float 2.0, float 3.0, float 4.0, float 5.0, float 6.0, float 7.0, float 8.0>, <8 x i32> <i32 4, i32 0, i32 2, i32 1, i32 0, i32 1, i32 1, i32 1>)
  ret <8 x float> %1
}
