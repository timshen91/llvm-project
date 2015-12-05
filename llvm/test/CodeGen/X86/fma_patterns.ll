; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=x86_64-unknown-unknown -mattr=+avx,+fma -fp-contract=fast | FileCheck %s --check-prefix=ALL --check-prefix=FMA
; RUN: llc < %s -mtriple=x86_64-unknown-unknown -mattr=+avx,+fma4,+fma -fp-contract=fast | FileCheck %s --check-prefix=ALL --check-prefix=FMA4
; RUN: llc < %s -mtriple=x86_64-unknown-unknown -mattr=+avx,+fma4 -fp-contract=fast | FileCheck %s --check-prefix=ALL --check-prefix=FMA4
; RUN: llc < %s -mtriple=x86_64-unknown-unknown -mattr=+avx512dq,+avx512vl -fp-contract=fast | FileCheck %s --check-prefix=ALL --check-prefix=AVX512

;
; Pattern: (fadd (fmul x, y), z) -> (fmadd x,y,z)
;

define float @test_f32_fmadd(float %a0, float %a1, float %a2) {
; FMA-LABEL: test_f32_fmadd:
; FMA:       # BB#0:
; FMA-NEXT:    vfmadd213ss %xmm2, %xmm1, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_f32_fmadd:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfmaddss %xmm2, %xmm1, %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_f32_fmadd:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfmadd213ss %xmm2, %xmm0, %xmm1
; AVX512-NEXT:    vmovaps %zmm1, %zmm0
; AVX512-NEXT:    retq
  %x = fmul float %a0, %a1
  %res = fadd float %x, %a2
  ret float %res
}

define <4 x float> @test_4f32_fmadd(<4 x float> %a0, <4 x float> %a1, <4 x float> %a2) {
; FMA-LABEL: test_4f32_fmadd:
; FMA:       # BB#0:
; FMA-NEXT:    vfmadd213ps %xmm2, %xmm1, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_4f32_fmadd:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfmaddps %xmm2, %xmm1, %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_4f32_fmadd:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfmadd213ps %xmm2, %xmm1, %xmm0
; AVX512-NEXT:    retq
  %x = fmul <4 x float> %a0, %a1
  %res = fadd <4 x float> %x, %a2
  ret <4 x float> %res
}

define <8 x float> @test_8f32_fmadd(<8 x float> %a0, <8 x float> %a1, <8 x float> %a2) {
; FMA-LABEL: test_8f32_fmadd:
; FMA:       # BB#0:
; FMA-NEXT:    vfmadd213ps %ymm2, %ymm1, %ymm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_8f32_fmadd:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfmaddps %ymm2, %ymm1, %ymm0, %ymm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_8f32_fmadd:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfmadd213ps %ymm2, %ymm1, %ymm0
; AVX512-NEXT:    retq
  %x = fmul <8 x float> %a0, %a1
  %res = fadd <8 x float> %x, %a2
  ret <8 x float> %res
}

define double @test_f64_fmadd(double %a0, double %a1, double %a2) {
; FMA-LABEL: test_f64_fmadd:
; FMA:       # BB#0:
; FMA-NEXT:    vfmadd213sd %xmm2, %xmm1, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_f64_fmadd:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfmaddsd %xmm2, %xmm1, %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_f64_fmadd:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfmadd213sd %xmm2, %xmm0, %xmm1
; AVX512-NEXT:    vmovaps %zmm1, %zmm0
; AVX512-NEXT:    retq
  %x = fmul double %a0, %a1
  %res = fadd double %x, %a2
  ret double %res
}

define <2 x double> @test_2f64_fmadd(<2 x double> %a0, <2 x double> %a1, <2 x double> %a2) {
; FMA-LABEL: test_2f64_fmadd:
; FMA:       # BB#0:
; FMA-NEXT:    vfmadd213pd %xmm2, %xmm1, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_2f64_fmadd:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfmaddpd %xmm2, %xmm1, %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_2f64_fmadd:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfmadd213pd %xmm2, %xmm1, %xmm0
; AVX512-NEXT:    retq
  %x = fmul <2 x double> %a0, %a1
  %res = fadd <2 x double> %x, %a2
  ret <2 x double> %res
}

define <4 x double> @test_4f64_fmadd(<4 x double> %a0, <4 x double> %a1, <4 x double> %a2) {
; FMA-LABEL: test_4f64_fmadd:
; FMA:       # BB#0:
; FMA-NEXT:    vfmadd213pd %ymm2, %ymm1, %ymm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_4f64_fmadd:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfmaddpd %ymm2, %ymm1, %ymm0, %ymm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_4f64_fmadd:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfmadd213pd %ymm2, %ymm1, %ymm0
; AVX512-NEXT:    retq
  %x = fmul <4 x double> %a0, %a1
  %res = fadd <4 x double> %x, %a2
  ret <4 x double> %res
}

;
; Pattern: (fsub (fmul x, y), z) -> (fmsub x, y, z)
;

define float @test_f32_fmsub(float %a0, float %a1, float %a2) {
; FMA-LABEL: test_f32_fmsub:
; FMA:       # BB#0:
; FMA-NEXT:    vfmsub213ss %xmm2, %xmm1, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_f32_fmsub:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfmsubss %xmm2, %xmm1, %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_f32_fmsub:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfmsub213ss %xmm2, %xmm0, %xmm1
; AVX512-NEXT:    vmovaps %zmm1, %zmm0
; AVX512-NEXT:    retq
  %x = fmul float %a0, %a1
  %res = fsub float %x, %a2
  ret float %res
}

define <4 x float> @test_4f32_fmsub(<4 x float> %a0, <4 x float> %a1, <4 x float> %a2) {
; FMA-LABEL: test_4f32_fmsub:
; FMA:       # BB#0:
; FMA-NEXT:    vfmsub213ps %xmm2, %xmm1, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_4f32_fmsub:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfmsubps %xmm2, %xmm1, %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_4f32_fmsub:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfmsub213ps %xmm2, %xmm1, %xmm0
; AVX512-NEXT:    retq
  %x = fmul <4 x float> %a0, %a1
  %res = fsub <4 x float> %x, %a2
  ret <4 x float> %res
}

define <8 x float> @test_8f32_fmsub(<8 x float> %a0, <8 x float> %a1, <8 x float> %a2) {
; FMA-LABEL: test_8f32_fmsub:
; FMA:       # BB#0:
; FMA-NEXT:    vfmsub213ps %ymm2, %ymm1, %ymm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_8f32_fmsub:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfmsubps %ymm2, %ymm1, %ymm0, %ymm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_8f32_fmsub:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfmsub213ps %ymm2, %ymm1, %ymm0
; AVX512-NEXT:    retq
  %x = fmul <8 x float> %a0, %a1
  %res = fsub <8 x float> %x, %a2
  ret <8 x float> %res
}

define double @test_f64_fmsub(double %a0, double %a1, double %a2) {
; FMA-LABEL: test_f64_fmsub:
; FMA:       # BB#0:
; FMA-NEXT:    vfmsub213sd %xmm2, %xmm1, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_f64_fmsub:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfmsubsd %xmm2, %xmm1, %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_f64_fmsub:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfmsub213sd %xmm2, %xmm0, %xmm1
; AVX512-NEXT:    vmovaps %zmm1, %zmm0
; AVX512-NEXT:    retq
  %x = fmul double %a0, %a1
  %res = fsub double %x, %a2
  ret double %res
}

define <2 x double> @test_2f64_fmsub(<2 x double> %a0, <2 x double> %a1, <2 x double> %a2) {
; FMA-LABEL: test_2f64_fmsub:
; FMA:       # BB#0:
; FMA-NEXT:    vfmsub213pd %xmm2, %xmm1, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_2f64_fmsub:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfmsubpd %xmm2, %xmm1, %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_2f64_fmsub:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfmsub213pd %xmm2, %xmm1, %xmm0
; AVX512-NEXT:    retq
  %x = fmul <2 x double> %a0, %a1
  %res = fsub <2 x double> %x, %a2
  ret <2 x double> %res
}

define <4 x double> @test_4f64_fmsub(<4 x double> %a0, <4 x double> %a1, <4 x double> %a2) {
; FMA-LABEL: test_4f64_fmsub:
; FMA:       # BB#0:
; FMA-NEXT:    vfmsub213pd %ymm2, %ymm1, %ymm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_4f64_fmsub:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfmsubpd %ymm2, %ymm1, %ymm0, %ymm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_4f64_fmsub:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfmsub213pd %ymm2, %ymm1, %ymm0
; AVX512-NEXT:    retq
  %x = fmul <4 x double> %a0, %a1
  %res = fsub <4 x double> %x, %a2
  ret <4 x double> %res
}

;
; Pattern: (fsub z, (fmul x, y)) -> (fnmadd x, y, z)
;

define float @test_f32_fnmadd(float %a0, float %a1, float %a2) {
; FMA-LABEL: test_f32_fnmadd:
; FMA:       # BB#0:
; FMA-NEXT:    vfnmadd213ss %xmm2, %xmm1, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_f32_fnmadd:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfnmaddss %xmm2, %xmm1, %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_f32_fnmadd:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfnmadd213ss %xmm2, %xmm0, %xmm1
; AVX512-NEXT:    vmovaps %zmm1, %zmm0
; AVX512-NEXT:    retq
  %x = fmul float %a0, %a1
  %res = fsub float %a2, %x
  ret float %res
}

define <4 x float> @test_4f32_fnmadd(<4 x float> %a0, <4 x float> %a1, <4 x float> %a2) {
; FMA-LABEL: test_4f32_fnmadd:
; FMA:       # BB#0:
; FMA-NEXT:    vfnmadd213ps %xmm2, %xmm1, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_4f32_fnmadd:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfnmaddps %xmm2, %xmm1, %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_4f32_fnmadd:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfnmadd213ps %xmm2, %xmm1, %xmm0
; AVX512-NEXT:    retq
  %x = fmul <4 x float> %a0, %a1
  %res = fsub <4 x float> %a2, %x
  ret <4 x float> %res
}

define <8 x float> @test_8f32_fnmadd(<8 x float> %a0, <8 x float> %a1, <8 x float> %a2) {
; FMA-LABEL: test_8f32_fnmadd:
; FMA:       # BB#0:
; FMA-NEXT:    vfnmadd213ps %ymm2, %ymm1, %ymm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_8f32_fnmadd:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfnmaddps %ymm2, %ymm1, %ymm0, %ymm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_8f32_fnmadd:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfnmadd213ps %ymm2, %ymm1, %ymm0
; AVX512-NEXT:    retq
  %x = fmul <8 x float> %a0, %a1
  %res = fsub <8 x float> %a2, %x
  ret <8 x float> %res
}

define double @test_f64_fnmadd(double %a0, double %a1, double %a2) {
; FMA-LABEL: test_f64_fnmadd:
; FMA:       # BB#0:
; FMA-NEXT:    vfnmadd213sd %xmm2, %xmm1, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_f64_fnmadd:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfnmaddsd %xmm2, %xmm1, %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_f64_fnmadd:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfnmadd213sd %xmm2, %xmm0, %xmm1
; AVX512-NEXT:    vmovaps %zmm1, %zmm0
; AVX512-NEXT:    retq
  %x = fmul double %a0, %a1
  %res = fsub double %a2, %x
  ret double %res
}

define <2 x double> @test_2f64_fnmadd(<2 x double> %a0, <2 x double> %a1, <2 x double> %a2) {
; FMA-LABEL: test_2f64_fnmadd:
; FMA:       # BB#0:
; FMA-NEXT:    vfnmadd213pd %xmm2, %xmm1, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_2f64_fnmadd:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfnmaddpd %xmm2, %xmm1, %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_2f64_fnmadd:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfnmadd213pd %xmm2, %xmm1, %xmm0
; AVX512-NEXT:    retq
  %x = fmul <2 x double> %a0, %a1
  %res = fsub <2 x double> %a2, %x
  ret <2 x double> %res
}

define <4 x double> @test_4f64_fnmadd(<4 x double> %a0, <4 x double> %a1, <4 x double> %a2) {
; FMA-LABEL: test_4f64_fnmadd:
; FMA:       # BB#0:
; FMA-NEXT:    vfnmadd213pd %ymm2, %ymm1, %ymm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_4f64_fnmadd:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfnmaddpd %ymm2, %ymm1, %ymm0, %ymm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_4f64_fnmadd:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfnmadd213pd %ymm2, %ymm1, %ymm0
; AVX512-NEXT:    retq
  %x = fmul <4 x double> %a0, %a1
  %res = fsub <4 x double> %a2, %x
  ret <4 x double> %res
}

;
; Pattern: (fsub (fneg (fmul x, y)), z) -> (fnmsub x, y, z)
;

define float @test_f32_fnmsub(float %a0, float %a1, float %a2) {
; FMA-LABEL: test_f32_fnmsub:
; FMA:       # BB#0:
; FMA-NEXT:    vfnmsub213ss %xmm2, %xmm1, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_f32_fnmsub:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfnmsubss %xmm2, %xmm1, %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_f32_fnmsub:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfnmsub213ss %xmm2, %xmm0, %xmm1
; AVX512-NEXT:    vmovaps %zmm1, %zmm0
; AVX512-NEXT:    retq
  %x = fmul float %a0, %a1
  %y = fsub float -0.000000e+00, %x
  %res = fsub float %y, %a2
  ret float %res
}

define <4 x float> @test_4f32_fnmsub(<4 x float> %a0, <4 x float> %a1, <4 x float> %a2) {
; FMA-LABEL: test_4f32_fnmsub:
; FMA:       # BB#0:
; FMA-NEXT:    vfnmsub213ps %xmm2, %xmm1, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_4f32_fnmsub:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfnmsubps %xmm2, %xmm1, %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_4f32_fnmsub:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfnmsub213ps %xmm2, %xmm1, %xmm0
; AVX512-NEXT:    retq
  %x = fmul <4 x float> %a0, %a1
  %y = fsub <4 x float> <float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00>, %x
  %res = fsub <4 x float> %y, %a2
  ret <4 x float> %res
}

define <8 x float> @test_8f32_fnmsub(<8 x float> %a0, <8 x float> %a1, <8 x float> %a2) {
; FMA-LABEL: test_8f32_fnmsub:
; FMA:       # BB#0:
; FMA-NEXT:    vfnmsub213ps %ymm2, %ymm1, %ymm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_8f32_fnmsub:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfnmsubps %ymm2, %ymm1, %ymm0, %ymm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_8f32_fnmsub:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfnmsub213ps %ymm2, %ymm1, %ymm0
; AVX512-NEXT:    retq
  %x = fmul <8 x float> %a0, %a1
  %y = fsub <8 x float> <float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00>, %x
  %res = fsub <8 x float> %y, %a2
  ret <8 x float> %res
}

define double @test_f64_fnmsub(double %a0, double %a1, double %a2) {
; FMA-LABEL: test_f64_fnmsub:
; FMA:       # BB#0:
; FMA-NEXT:    vfnmsub213sd %xmm2, %xmm1, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_f64_fnmsub:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfnmsubsd %xmm2, %xmm1, %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_f64_fnmsub:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfnmsub213sd %xmm2, %xmm0, %xmm1
; AVX512-NEXT:    vmovaps %zmm1, %zmm0
; AVX512-NEXT:    retq
  %x = fmul double %a0, %a1
  %y = fsub double -0.000000e+00, %x
  %res = fsub double %y, %a2
  ret double %res
}

define <2 x double> @test_2f64_fnmsub(<2 x double> %a0, <2 x double> %a1, <2 x double> %a2) {
; FMA-LABEL: test_2f64_fnmsub:
; FMA:       # BB#0:
; FMA-NEXT:    vfnmsub213pd %xmm2, %xmm1, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_2f64_fnmsub:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfnmsubpd %xmm2, %xmm1, %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_2f64_fnmsub:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfnmsub213pd %xmm2, %xmm1, %xmm0
; AVX512-NEXT:    retq
  %x = fmul <2 x double> %a0, %a1
  %y = fsub <2 x double> <double -0.000000e+00, double -0.000000e+00>, %x
  %res = fsub <2 x double> %y, %a2
  ret <2 x double> %res
}

define <4 x double> @test_4f64_fnmsub(<4 x double> %a0, <4 x double> %a1, <4 x double> %a2) {
; FMA-LABEL: test_4f64_fnmsub:
; FMA:       # BB#0:
; FMA-NEXT:    vfnmsub213pd %ymm2, %ymm1, %ymm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_4f64_fnmsub:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfnmsubpd %ymm2, %ymm1, %ymm0, %ymm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_4f64_fnmsub:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfnmsub213pd %ymm2, %ymm1, %ymm0
; AVX512-NEXT:    retq
  %x = fmul <4 x double> %a0, %a1
  %y = fsub <4 x double> <double -0.000000e+00, double -0.000000e+00, double -0.000000e+00, double -0.000000e+00>, %x
  %res = fsub <4 x double> %y, %a2
  ret <4 x double> %res
}

;
; Load Folding Patterns
;

define <4 x float> @test_4f32_fmadd_load(<4 x float>* %a0, <4 x float> %a1, <4 x float> %a2) {
; FMA-LABEL: test_4f32_fmadd_load:
; FMA:       # BB#0:
; FMA-NEXT:    vfmadd132ps (%rdi), %xmm1, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_4f32_fmadd_load:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfmaddps %xmm1, (%rdi), %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_4f32_fmadd_load:
; AVX512:       # BB#0:
; AVX512-NEXT:    vmovaps (%rdi), %xmm2
; AVX512-NEXT:    vfmadd213ps %xmm1, %xmm0, %xmm2
; AVX512-NEXT:    vmovaps %zmm2, %zmm0
; AVX512-NEXT:    retq
  %x = load <4 x float>, <4 x float>* %a0
  %y = fmul <4 x float> %x, %a1
  %res = fadd <4 x float> %y, %a2
  ret <4 x float> %res
}

define <2 x double> @test_2f64_fmsub_load(<2 x double>* %a0, <2 x double> %a1, <2 x double> %a2) {
; FMA-LABEL: test_2f64_fmsub_load:
; FMA:       # BB#0:
; FMA-NEXT:    vfmsub132pd (%rdi), %xmm1, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_2f64_fmsub_load:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfmsubpd %xmm1, (%rdi), %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_2f64_fmsub_load:
; AVX512:       # BB#0:
; AVX512-NEXT:    vmovapd (%rdi), %xmm2
; AVX512-NEXT:    vfmsub213pd %xmm1, %xmm0, %xmm2
; AVX512-NEXT:    vmovaps %zmm2, %zmm0
; AVX512-NEXT:    retq
  %x = load <2 x double>, <2 x double>* %a0
  %y = fmul <2 x double> %x, %a1
  %res = fsub <2 x double> %y, %a2
  ret <2 x double> %res
}

;
; Patterns (+ fneg variants): mul(add(1.0,x),y), mul(sub(1.0,x),y), mul(sub(x,1.0),y)
;

define <4 x float> @test_v4f32_mul_add_x_one_y(<4 x float> %x, <4 x float> %y) {
; FMA-LABEL: test_v4f32_mul_add_x_one_y:
; FMA:       # BB#0:
; FMA-NEXT:    vfmadd213ps %xmm1, %xmm1, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_v4f32_mul_add_x_one_y:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfmaddps %xmm1, %xmm1, %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_v4f32_mul_add_x_one_y:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfmadd213ps %xmm1, %xmm1, %xmm0
; AVX512-NEXT:    retq
  %a = fadd <4 x float> %x, <float 1.0, float 1.0, float 1.0, float 1.0>
  %m = fmul <4 x float> %a, %y
  ret <4 x float> %m
}

define <4 x float> @test_v4f32_mul_y_add_x_one(<4 x float> %x, <4 x float> %y) {
; FMA-LABEL: test_v4f32_mul_y_add_x_one:
; FMA:       # BB#0:
; FMA-NEXT:    vfmadd213ps %xmm1, %xmm1, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_v4f32_mul_y_add_x_one:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfmaddps %xmm1, %xmm1, %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_v4f32_mul_y_add_x_one:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfmadd213ps %xmm1, %xmm1, %xmm0
; AVX512-NEXT:    retq
  %a = fadd <4 x float> %x, <float 1.0, float 1.0, float 1.0, float 1.0>
  %m = fmul <4 x float> %y, %a
  ret <4 x float> %m
}

define <4 x float> @test_v4f32_mul_add_x_negone_y(<4 x float> %x, <4 x float> %y) {
; FMA-LABEL: test_v4f32_mul_add_x_negone_y:
; FMA:       # BB#0:
; FMA-NEXT:    vfmsub213ps %xmm1, %xmm1, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_v4f32_mul_add_x_negone_y:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfmsubps %xmm1, %xmm1, %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_v4f32_mul_add_x_negone_y:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfmsub213ps %xmm1, %xmm1, %xmm0
; AVX512-NEXT:    retq
  %a = fadd <4 x float> %x, <float -1.0, float -1.0, float -1.0, float -1.0>
  %m = fmul <4 x float> %a, %y
  ret <4 x float> %m
}

define <4 x float> @test_v4f32_mul_y_add_x_negone(<4 x float> %x, <4 x float> %y) {
; FMA-LABEL: test_v4f32_mul_y_add_x_negone:
; FMA:       # BB#0:
; FMA-NEXT:    vfmsub213ps %xmm1, %xmm1, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_v4f32_mul_y_add_x_negone:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfmsubps %xmm1, %xmm1, %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_v4f32_mul_y_add_x_negone:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfmsub213ps %xmm1, %xmm1, %xmm0
; AVX512-NEXT:    retq
  %a = fadd <4 x float> %x, <float -1.0, float -1.0, float -1.0, float -1.0>
  %m = fmul <4 x float> %y, %a
  ret <4 x float> %m
}

define <4 x float> @test_v4f32_mul_sub_one_x_y(<4 x float> %x, <4 x float> %y) {
; FMA-LABEL: test_v4f32_mul_sub_one_x_y:
; FMA:       # BB#0:
; FMA-NEXT:    vfnmadd213ps %xmm1, %xmm1, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_v4f32_mul_sub_one_x_y:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfnmaddps %xmm1, %xmm1, %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_v4f32_mul_sub_one_x_y:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfnmadd213ps %xmm1, %xmm1, %xmm0
; AVX512-NEXT:    retq
  %s = fsub <4 x float> <float 1.0, float 1.0, float 1.0, float 1.0>, %x
  %m = fmul <4 x float> %s, %y
  ret <4 x float> %m
}

define <4 x float> @test_v4f32_mul_y_sub_one_x(<4 x float> %x, <4 x float> %y) {
; FMA-LABEL: test_v4f32_mul_y_sub_one_x:
; FMA:       # BB#0:
; FMA-NEXT:    vfnmadd213ps %xmm1, %xmm1, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_v4f32_mul_y_sub_one_x:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfnmaddps %xmm1, %xmm1, %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_v4f32_mul_y_sub_one_x:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfnmadd213ps %xmm1, %xmm1, %xmm0
; AVX512-NEXT:    retq
  %s = fsub <4 x float> <float 1.0, float 1.0, float 1.0, float 1.0>, %x
  %m = fmul <4 x float> %y, %s
  ret <4 x float> %m
}

define <4 x float> @test_v4f32_mul_sub_negone_x_y(<4 x float> %x, <4 x float> %y) {
; FMA-LABEL: test_v4f32_mul_sub_negone_x_y:
; FMA:       # BB#0:
; FMA-NEXT:    vfnmsub213ps %xmm1, %xmm1, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_v4f32_mul_sub_negone_x_y:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfnmsubps %xmm1, %xmm1, %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_v4f32_mul_sub_negone_x_y:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfnmsub213ps %xmm1, %xmm1, %xmm0
; AVX512-NEXT:    retq
  %s = fsub <4 x float> <float -1.0, float -1.0, float -1.0, float -1.0>, %x
  %m = fmul <4 x float> %s, %y
  ret <4 x float> %m
}

define <4 x float> @test_v4f32_mul_y_sub_negone_x(<4 x float> %x, <4 x float> %y) {
; FMA-LABEL: test_v4f32_mul_y_sub_negone_x:
; FMA:       # BB#0:
; FMA-NEXT:    vfnmsub213ps %xmm1, %xmm1, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_v4f32_mul_y_sub_negone_x:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfnmsubps %xmm1, %xmm1, %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_v4f32_mul_y_sub_negone_x:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfnmsub213ps %xmm1, %xmm1, %xmm0
; AVX512-NEXT:    retq
  %s = fsub <4 x float> <float -1.0, float -1.0, float -1.0, float -1.0>, %x
  %m = fmul <4 x float> %y, %s
  ret <4 x float> %m
}

define <4 x float> @test_v4f32_mul_sub_x_one_y(<4 x float> %x, <4 x float> %y) {
; FMA-LABEL: test_v4f32_mul_sub_x_one_y:
; FMA:       # BB#0:
; FMA-NEXT:    vfmsub213ps %xmm1, %xmm1, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_v4f32_mul_sub_x_one_y:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfmsubps %xmm1, %xmm1, %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_v4f32_mul_sub_x_one_y:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfmsub213ps %xmm1, %xmm1, %xmm0
; AVX512-NEXT:    retq
  %s = fsub <4 x float> %x, <float 1.0, float 1.0, float 1.0, float 1.0>
  %m = fmul <4 x float> %s, %y
  ret <4 x float> %m
}

define <4 x float> @test_v4f32_mul_y_sub_x_one(<4 x float> %x, <4 x float> %y) {
; FMA-LABEL: test_v4f32_mul_y_sub_x_one:
; FMA:       # BB#0:
; FMA-NEXT:    vfmsub213ps %xmm1, %xmm1, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_v4f32_mul_y_sub_x_one:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfmsubps %xmm1, %xmm1, %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_v4f32_mul_y_sub_x_one:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfmsub213ps %xmm1, %xmm1, %xmm0
; AVX512-NEXT:    retq
  %s = fsub <4 x float> %x, <float 1.0, float 1.0, float 1.0, float 1.0>
  %m = fmul <4 x float> %y, %s
  ret <4 x float> %m
}

define <4 x float> @test_v4f32_mul_sub_x_negone_y(<4 x float> %x, <4 x float> %y) {
; FMA-LABEL: test_v4f32_mul_sub_x_negone_y:
; FMA:       # BB#0:
; FMA-NEXT:    vfmadd213ps %xmm1, %xmm1, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_v4f32_mul_sub_x_negone_y:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfmaddps %xmm1, %xmm1, %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_v4f32_mul_sub_x_negone_y:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfmadd213ps %xmm1, %xmm1, %xmm0
; AVX512-NEXT:    retq
  %s = fsub <4 x float> %x, <float -1.0, float -1.0, float -1.0, float -1.0>
  %m = fmul <4 x float> %s, %y
  ret <4 x float> %m
}

define <4 x float> @test_v4f32_mul_y_sub_x_negone(<4 x float> %x, <4 x float> %y) {
; FMA-LABEL: test_v4f32_mul_y_sub_x_negone:
; FMA:       # BB#0:
; FMA-NEXT:    vfmadd213ps %xmm1, %xmm1, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_v4f32_mul_y_sub_x_negone:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfmaddps %xmm1, %xmm1, %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_v4f32_mul_y_sub_x_negone:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfmadd213ps %xmm1, %xmm1, %xmm0
; AVX512-NEXT:    retq
  %s = fsub <4 x float> %x, <float -1.0, float -1.0, float -1.0, float -1.0>
  %m = fmul <4 x float> %y, %s
  ret <4 x float> %m
}

;
; Interpolation Patterns: add(mul(x,t),mul(sub(1.0,t),y))
;

define float @test_f32_interp(float %x, float %y, float %t) {
; FMA-LABEL: test_f32_interp:
; FMA:       # BB#0:
; FMA-NEXT:    vfnmadd213ss %xmm1, %xmm2, %xmm1
; FMA-NEXT:    vfmadd213ss %xmm1, %xmm2, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_f32_interp:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfnmaddss %xmm1, %xmm1, %xmm2, %xmm1
; FMA4-NEXT:    vfmaddss %xmm1, %xmm2, %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_f32_interp:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfnmadd213ss %xmm1, %xmm2, %xmm1
; AVX512-NEXT:    vfmadd213ss %xmm1, %xmm0, %xmm2
; AVX512-NEXT:    vmovaps %zmm2, %zmm0
; AVX512-NEXT:    retq
  %t1 = fsub float 1.0, %t
  %tx = fmul float %x, %t
  %ty = fmul float %y, %t1
  %r = fadd float %tx, %ty
  ret float %r
}

define <4 x float> @test_v4f32_interp(<4 x float> %x, <4 x float> %y, <4 x float> %t) {
; FMA-LABEL: test_v4f32_interp:
; FMA:       # BB#0:
; FMA-NEXT:    vfnmadd213ps %xmm1, %xmm2, %xmm1
; FMA-NEXT:    vfmadd213ps %xmm1, %xmm2, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_v4f32_interp:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfnmaddps %xmm1, %xmm1, %xmm2, %xmm1
; FMA4-NEXT:    vfmaddps %xmm1, %xmm2, %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_v4f32_interp:
; AVX512:       # BB#0:
; AVX512-NEXT:    vmovaps %zmm2, %zmm3
; AVX512-NEXT:    vfnmadd213ps %xmm1, %xmm1, %xmm3
; AVX512-NEXT:    vfmadd213ps %xmm3, %xmm2, %xmm0
; AVX512-NEXT:    retq
  %t1 = fsub <4 x float> <float 1.0, float 1.0, float 1.0, float 1.0>, %t
  %tx = fmul <4 x float> %x, %t
  %ty = fmul <4 x float> %y, %t1
  %r = fadd <4 x float> %tx, %ty
  ret <4 x float> %r
}

define <8 x float> @test_v8f32_interp(<8 x float> %x, <8 x float> %y, <8 x float> %t) {
; FMA-LABEL: test_v8f32_interp:
; FMA:       # BB#0:
; FMA-NEXT:    vfnmadd213ps %ymm1, %ymm2, %ymm1
; FMA-NEXT:    vfmadd213ps %ymm1, %ymm2, %ymm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_v8f32_interp:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfnmaddps %ymm1, %ymm1, %ymm2, %ymm1
; FMA4-NEXT:    vfmaddps %ymm1, %ymm2, %ymm0, %ymm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_v8f32_interp:
; AVX512:       # BB#0:
; AVX512-NEXT:    vmovaps %zmm2, %zmm3
; AVX512-NEXT:    vfnmadd213ps %ymm1, %ymm1, %ymm3
; AVX512-NEXT:    vfmadd213ps %ymm3, %ymm2, %ymm0
; AVX512-NEXT:    retq
  %t1 = fsub <8 x float> <float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0>, %t
  %tx = fmul <8 x float> %x, %t
  %ty = fmul <8 x float> %y, %t1
  %r = fadd <8 x float> %tx, %ty
  ret <8 x float> %r
}

define double @test_f64_interp(double %x, double %y, double %t) {
; FMA-LABEL: test_f64_interp:
; FMA:       # BB#0:
; FMA-NEXT:    vfnmadd213sd %xmm1, %xmm2, %xmm1
; FMA-NEXT:    vfmadd213sd %xmm1, %xmm2, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_f64_interp:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfnmaddsd %xmm1, %xmm1, %xmm2, %xmm1
; FMA4-NEXT:    vfmaddsd %xmm1, %xmm2, %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_f64_interp:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfnmadd213sd %xmm1, %xmm2, %xmm1
; AVX512-NEXT:    vfmadd213sd %xmm1, %xmm0, %xmm2
; AVX512-NEXT:    vmovaps %zmm2, %zmm0
; AVX512-NEXT:    retq
  %t1 = fsub double 1.0, %t
  %tx = fmul double %x, %t
  %ty = fmul double %y, %t1
  %r = fadd double %tx, %ty
  ret double %r
}

define <2 x double> @test_v2f64_interp(<2 x double> %x, <2 x double> %y, <2 x double> %t) {
; FMA-LABEL: test_v2f64_interp:
; FMA:       # BB#0:
; FMA-NEXT:    vfnmadd213pd %xmm1, %xmm2, %xmm1
; FMA-NEXT:    vfmadd213pd %xmm1, %xmm2, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_v2f64_interp:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfnmaddpd %xmm1, %xmm1, %xmm2, %xmm1
; FMA4-NEXT:    vfmaddpd %xmm1, %xmm2, %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_v2f64_interp:
; AVX512:       # BB#0:
; AVX512-NEXT:    vmovaps %zmm2, %zmm3
; AVX512-NEXT:    vfnmadd213pd %xmm1, %xmm1, %xmm3
; AVX512-NEXT:    vfmadd213pd %xmm3, %xmm2, %xmm0
; AVX512-NEXT:    retq
  %t1 = fsub <2 x double> <double 1.0, double 1.0>, %t
  %tx = fmul <2 x double> %x, %t
  %ty = fmul <2 x double> %y, %t1
  %r = fadd <2 x double> %tx, %ty
  ret <2 x double> %r
}

define <4 x double> @test_v4f64_interp(<4 x double> %x, <4 x double> %y, <4 x double> %t) {
; FMA-LABEL: test_v4f64_interp:
; FMA:       # BB#0:
; FMA-NEXT:    vfnmadd213pd %ymm1, %ymm2, %ymm1
; FMA-NEXT:    vfmadd213pd %ymm1, %ymm2, %ymm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_v4f64_interp:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfnmaddpd %ymm1, %ymm1, %ymm2, %ymm1
; FMA4-NEXT:    vfmaddpd %ymm1, %ymm2, %ymm0, %ymm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_v4f64_interp:
; AVX512:       # BB#0:
; AVX512-NEXT:    vmovaps %zmm2, %zmm3
; AVX512-NEXT:    vfnmadd213pd %ymm1, %ymm1, %ymm3
; AVX512-NEXT:    vfmadd213pd %ymm3, %ymm2, %ymm0
; AVX512-NEXT:    retq
  %t1 = fsub <4 x double> <double 1.0, double 1.0, double 1.0, double 1.0>, %t
  %tx = fmul <4 x double> %x, %t
  %ty = fmul <4 x double> %y, %t1
  %r = fadd <4 x double> %tx, %ty
  ret <4 x double> %r
}

;
; Pattern: (fneg (fma x, y, z)) -> (fma x, -y, -z)
;

define <4 x float> @test_v4f32_fneg_fmadd(<4 x float> %a0, <4 x float> %a1, <4 x float> %a2) #0 {
; FMA-LABEL: test_v4f32_fneg_fmadd:
; FMA:       # BB#0:
; FMA-NEXT:    vfnmsub213ps %xmm2, %xmm1, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_v4f32_fneg_fmadd:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfnmsubps %xmm2, %xmm1, %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_v4f32_fneg_fmadd:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfnmsub213ps %xmm2, %xmm1, %xmm0
; AVX512-NEXT:    retq
  %mul = fmul <4 x float> %a0, %a1
  %add = fadd <4 x float> %mul, %a2
  %neg = fsub <4 x float> <float -0.0, float -0.0, float -0.0, float -0.0>, %add
  ret <4 x float> %neg
}

define <4 x double> @test_v4f64_fneg_fmsub(<4 x double> %a0, <4 x double> %a1, <4 x double> %a2) #0 {
; FMA-LABEL: test_v4f64_fneg_fmsub:
; FMA:       # BB#0:
; FMA-NEXT:    vfnmadd213pd %ymm2, %ymm1, %ymm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_v4f64_fneg_fmsub:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfnmaddpd %ymm2, %ymm1, %ymm0, %ymm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_v4f64_fneg_fmsub:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfnmadd213pd %ymm2, %ymm1, %ymm0
; AVX512-NEXT:    retq
  %mul = fmul <4 x double> %a0, %a1
  %sub = fsub <4 x double> %mul, %a2
  %neg = fsub <4 x double> <double -0.0, double -0.0, double -0.0, double -0.0>, %sub
  ret <4 x double> %neg
}

define <4 x float> @test_v4f32_fneg_fnmadd(<4 x float> %a0, <4 x float> %a1, <4 x float> %a2) #0 {
; FMA-LABEL: test_v4f32_fneg_fnmadd:
; FMA:       # BB#0:
; FMA-NEXT:    vfmsub213ps %xmm2, %xmm1, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_v4f32_fneg_fnmadd:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfmsubps %xmm2, %xmm1, %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_v4f32_fneg_fnmadd:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfmsub213ps %xmm2, %xmm1, %xmm0
; AVX512-NEXT:    retq
  %mul = fmul <4 x float> %a0, %a1
  %neg0 = fsub <4 x float> <float -0.0, float -0.0, float -0.0, float -0.0>, %mul
  %add = fadd <4 x float> %neg0, %a2
  %neg1 = fsub <4 x float> <float -0.0, float -0.0, float -0.0, float -0.0>, %add
  ret <4 x float> %neg1
}

define <4 x double> @test_v4f64_fneg_fnmsub(<4 x double> %a0, <4 x double> %a1, <4 x double> %a2) #0 {
; FMA-LABEL: test_v4f64_fneg_fnmsub:
; FMA:       # BB#0:
; FMA-NEXT:    vfmadd213pd %ymm2, %ymm1, %ymm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_v4f64_fneg_fnmsub:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfmaddpd %ymm2, %ymm1, %ymm0, %ymm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_v4f64_fneg_fnmsub:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfmadd213pd %ymm2, %ymm1, %ymm0
; AVX512-NEXT:    retq
  %mul = fmul <4 x double> %a0, %a1
  %neg0 = fsub <4 x double> <double -0.0, double -0.0, double -0.0, double -0.0>, %mul
  %sub = fsub <4 x double> %neg0, %a2
  %neg1 = fsub <4 x double> <double -0.0, double -0.0, double -0.0, double -0.0>, %sub
  ret <4 x double> %neg1
}

;
; Pattern: (fma x, c1, (fmul x, c2)) -> (fmul x, c1+c2)
;

define <4 x float> @test_v4f32_fma_x_c1_fmul_x_c2(<4 x float> %x) #0 {
; FMA-LABEL: test_v4f32_fma_x_c1_fmul_x_c2:
; FMA:       # BB#0:
; FMA-NEXT:    vmulps {{.*}}(%rip), %xmm0, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_v4f32_fma_x_c1_fmul_x_c2:
; FMA4:       # BB#0:
; FMA4-NEXT:    vmulps {{.*}}(%rip), %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_v4f32_fma_x_c1_fmul_x_c2:
; AVX512:       # BB#0:
; AVX512-NEXT:    vmulps {{.*}}(%rip){1to4}, %xmm0, %xmm0
; AVX512-NEXT:    retq
  %m0 = fmul <4 x float> %x, <float 1.0, float 2.0, float 3.0, float 4.0>
  %m1 = fmul <4 x float> %x, <float 4.0, float 3.0, float 2.0, float 1.0>
  %a  = fadd <4 x float> %m0, %m1
  ret <4 x float> %a
}

;
; Pattern: (fma (fmul x, c1), c2, y) -> (fma x, c1*c2, y)
;

define <4 x float> @test_v4f32_fma_fmul_x_c1_c2_y(<4 x float> %x, <4 x float> %y) #0 {
; FMA-LABEL: test_v4f32_fma_fmul_x_c1_c2_y:
; FMA:       # BB#0:
; FMA-NEXT:    vfmadd132ps {{.*}}(%rip), %xmm1, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_v4f32_fma_fmul_x_c1_c2_y:
; FMA4:       # BB#0:
; FMA4-NEXT:    vfmaddps %xmm1, {{.*}}(%rip), %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_v4f32_fma_fmul_x_c1_c2_y:
; AVX512:       # BB#0:
; AVX512-NEXT:    vfmadd231ps {{.*}}(%rip), %xmm0, %xmm1
; AVX512-NEXT:    vmovaps %zmm1, %zmm0
; AVX512-NEXT:    retq
  %m0 = fmul <4 x float> %x,  <float 1.0, float 2.0, float 3.0, float 4.0>
  %m1 = fmul <4 x float> %m0, <float 4.0, float 3.0, float 2.0, float 1.0>
  %a  = fadd <4 x float> %m1, %y
  ret <4 x float> %a
}

; Pattern: (fneg (fmul x, y)) -> (fnmsub x, y, 0)

define double @test_f64_fneg_fmul(double %x, double %y) #0 {
; FMA-LABEL: test_f64_fneg_fmul:
; FMA:       # BB#0:
; FMA-NEXT:    vxorps %xmm2, %xmm2, %xmm2
; FMA-NEXT:    vfnmsub213sd %xmm2, %xmm1, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_f64_fneg_fmul:
; FMA4:       # BB#0:
; FMA4-NEXT:    vxorpd %xmm2, %xmm2, %xmm2
; FMA4-NEXT:    vfnmsubsd %xmm2, %xmm1, %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_f64_fneg_fmul:
; AVX512:       # BB#0:
; AVX512-NEXT:    vxorps %xmm2, %xmm2, %xmm2
; AVX512-NEXT:    vfnmsub213sd %xmm2, %xmm0, %xmm1
; AVX512-NEXT:    vmovaps %zmm1, %zmm0
; AVX512-NEXT:    retq
  %m = fmul nsz double %x, %y
  %n = fsub double -0.0, %m
  ret double %n
}

define <4 x float> @test_v4f32_fneg_fmul(<4 x float> %x, <4 x float> %y) #0 {
; FMA-LABEL: test_v4f32_fneg_fmul:
; FMA:       # BB#0:
; FMA-NEXT:    vxorps %xmm2, %xmm2, %xmm2
; FMA-NEXT:    vfnmsub213ps %xmm2, %xmm1, %xmm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_v4f32_fneg_fmul:
; FMA4:       # BB#0:
; FMA4-NEXT:    vxorps %xmm2, %xmm2, %xmm2
; FMA4-NEXT:    vfnmsubps %xmm2, %xmm1, %xmm0, %xmm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_v4f32_fneg_fmul:
; AVX512:       # BB#0:
; AVX512-NEXT:    vxorps %xmm2, %xmm2, %xmm2
; AVX512-NEXT:    vfnmsub213ps %xmm2, %xmm1, %xmm0
; AVX512-NEXT:    retq
  %m = fmul nsz <4 x float> %x, %y
  %n = fsub <4 x float> <float -0.0, float -0.0, float -0.0, float -0.0>, %m
  ret <4 x float> %n
}

define <4 x double> @test_v4f64_fneg_fmul(<4 x double> %x, <4 x double> %y) #0 {
; FMA-LABEL: test_v4f64_fneg_fmul:
; FMA:       # BB#0:
; FMA-NEXT:    vxorpd %ymm2, %ymm2, %ymm2
; FMA-NEXT:    vfnmsub213pd %ymm2, %ymm1, %ymm0
; FMA-NEXT:    retq
;
; FMA4-LABEL: test_v4f64_fneg_fmul:
; FMA4:       # BB#0:
; FMA4-NEXT:    vxorpd %ymm2, %ymm2, %ymm2
; FMA4-NEXT:    vfnmsubpd %ymm2, %ymm1, %ymm0, %ymm0
; FMA4-NEXT:    retq
;
; AVX512-LABEL: test_v4f64_fneg_fmul:
; AVX512:       # BB#0:
; AVX512-NEXT:    vxorps %ymm2, %ymm2, %ymm2
; AVX512-NEXT:    vfnmsub213pd %ymm2, %ymm1, %ymm0
; AVX512-NEXT:    retq
  %m = fmul nsz <4 x double> %x, %y
  %n = fsub <4 x double> <double -0.0, double -0.0, double -0.0, double -0.0>, %m
  ret <4 x double> %n
}

define <4 x double> @test_v4f64_fneg_fmul_no_nsz(<4 x double> %x, <4 x double> %y) #0 {
; ALL-LABEL: test_v4f64_fneg_fmul_no_nsz:
; ALL:       # BB#0:
; ALL-NEXT:    vmulpd %ymm1, %ymm0, %ymm0
; ALL-NEXT:    vxorpd {{.*}}(%rip), %ymm0, %ymm0
; ALL-NEXT:    retq
  %m = fmul <4 x double> %x, %y
  %n = fsub <4 x double> <double -0.0, double -0.0, double -0.0, double -0.0>, %m
  ret <4 x double> %n
}

attributes #0 = { "unsafe-fp-math"="true" }
