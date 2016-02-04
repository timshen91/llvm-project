; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=x86_64-unknown -mattr=+avx512f | FileCheck %s --check-prefix=ALL --check-prefix=AVX512F
; RUN: llc < %s -mtriple=x86_64-unknown -mattr=+avx512f,+avx512bw | FileCheck %s --check-prefix=ALL --check-prefix=AVX512BW
;
; Just one 32-bit run to make sure we do reasonable things.
; RUN: llc < %s -mtriple=i686-unknown-unknown -mattr=+avx512f | FileCheck %s --check-prefix=X32-AVX512F

define <8 x double> @merge_8f64_2f64_12u4(<2 x double>* %ptr) nounwind uwtable noinline ssp {
; ALL-LABEL: merge_8f64_2f64_12u4:
; ALL:       # BB#0:
; ALL-NEXT:    vinsertf128 $1, 64(%rdi), %ymm0, %ymm0
; ALL-NEXT:    vinsertf64x4 $0, 16(%rdi), %zmm0, %zmm1
; ALL-NEXT:    vinsertf64x4 $1, %ymm0, %zmm1, %zmm0
; ALL-NEXT:    retq
;
; X32-AVX512F-LABEL: merge_8f64_2f64_12u4:
; X32-AVX512F:       # BB#0:
; X32-AVX512F-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-AVX512F-NEXT:    vinsertf128 $1, 64(%eax), %ymm0, %ymm0
; X32-AVX512F-NEXT:    vinsertf64x4 $0, 16(%eax), %zmm0, %zmm1
; X32-AVX512F-NEXT:    vinsertf64x4 $1, %ymm0, %zmm1, %zmm0
; X32-AVX512F-NEXT:    retl
  %ptr0 = getelementptr inbounds <2 x double>, <2 x double>* %ptr, i64 1
  %ptr1 = getelementptr inbounds <2 x double>, <2 x double>* %ptr, i64 2
  %ptr3 = getelementptr inbounds <2 x double>, <2 x double>* %ptr, i64 4
  %val0 = load <2 x double>, <2 x double>* %ptr0
  %val1 = load <2 x double>, <2 x double>* %ptr1
  %val3 = load <2 x double>, <2 x double>* %ptr3
  %res01 = shufflevector <2 x double> %val0, <2 x double> %val1, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %res23 = shufflevector <2 x double> undef, <2 x double> %val3, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %res = shufflevector <4 x double> %res01, <4 x double> %res23, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  ret <8 x double> %res
}

define <8 x double> @merge_8f64_2f64_23z5(<2 x double>* %ptr) nounwind uwtable noinline ssp {
; ALL-LABEL: merge_8f64_2f64_23z5:
; ALL:       # BB#0:
; ALL-NEXT:    vxorpd %xmm0, %xmm0, %xmm0
; ALL-NEXT:    vinsertf128 $1, 80(%rdi), %ymm0, %ymm0
; ALL-NEXT:    vinsertf64x4 $0, 32(%rdi), %zmm0, %zmm1
; ALL-NEXT:    vinsertf64x4 $1, %ymm0, %zmm1, %zmm0
; ALL-NEXT:    retq
;
; X32-AVX512F-LABEL: merge_8f64_2f64_23z5:
; X32-AVX512F:       # BB#0:
; X32-AVX512F-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-AVX512F-NEXT:    vxorpd %xmm0, %xmm0, %xmm0
; X32-AVX512F-NEXT:    vinsertf128 $1, 80(%eax), %ymm0, %ymm0
; X32-AVX512F-NEXT:    vinsertf64x4 $0, 32(%eax), %zmm0, %zmm1
; X32-AVX512F-NEXT:    vinsertf64x4 $1, %ymm0, %zmm1, %zmm0
; X32-AVX512F-NEXT:    retl
  %ptr0 = getelementptr inbounds <2 x double>, <2 x double>* %ptr, i64 2
  %ptr1 = getelementptr inbounds <2 x double>, <2 x double>* %ptr, i64 3
  %ptr3 = getelementptr inbounds <2 x double>, <2 x double>* %ptr, i64 5
  %val0 = load <2 x double>, <2 x double>* %ptr0
  %val1 = load <2 x double>, <2 x double>* %ptr1
  %val3 = load <2 x double>, <2 x double>* %ptr3
  %res01 = shufflevector <2 x double> %val0, <2 x double> %val1, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %res23 = shufflevector <2 x double> zeroinitializer, <2 x double> %val3, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %res = shufflevector <4 x double> %res01, <4 x double> %res23, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  ret <8 x double> %res
}

define <8 x double> @merge_8f64_4f64_z2(<4 x double>* %ptr) nounwind uwtable noinline ssp {
; ALL-LABEL: merge_8f64_4f64_z2:
; ALL:       # BB#0:
; ALL-NEXT:    vxorpd %ymm0, %ymm0, %ymm0
; ALL-NEXT:    vinsertf64x4 $1, 64(%rdi), %zmm0, %zmm0
; ALL-NEXT:    retq
;
; X32-AVX512F-LABEL: merge_8f64_4f64_z2:
; X32-AVX512F:       # BB#0:
; X32-AVX512F-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-AVX512F-NEXT:    vxorpd %ymm0, %ymm0, %ymm0
; X32-AVX512F-NEXT:    vinsertf64x4 $1, 64(%eax), %zmm0, %zmm0
; X32-AVX512F-NEXT:    retl
  %ptr1 = getelementptr inbounds <4 x double>, <4 x double>* %ptr, i64 2
  %val1 = load <4 x double>, <4 x double>* %ptr1
  %res = shufflevector <4 x double> zeroinitializer, <4 x double> %val1, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  ret <8 x double> %res
}

define <8 x double> @merge_8f64_f64_23uuuuu9(double* %ptr) nounwind uwtable noinline ssp {
; ALL-LABEL: merge_8f64_f64_23uuuuu9:
; ALL:       # BB#0:
; ALL-NEXT:    vmovupd 16(%rdi), %zmm0
; ALL-NEXT:    retq
;
; X32-AVX512F-LABEL: merge_8f64_f64_23uuuuu9:
; X32-AVX512F:       # BB#0:
; X32-AVX512F-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-AVX512F-NEXT:    vmovupd 16(%eax), %zmm0
; X32-AVX512F-NEXT:    retl
  %ptr0 = getelementptr inbounds double, double* %ptr, i64 2
  %ptr1 = getelementptr inbounds double, double* %ptr, i64 3
  %ptr7 = getelementptr inbounds double, double* %ptr, i64 9
  %val0 = load double, double* %ptr0
  %val1 = load double, double* %ptr1
  %val7 = load double, double* %ptr7
  %res0 = insertelement <8 x double> undef, double %val0, i32 0
  %res1 = insertelement <8 x double> %res0, double %val1, i32 1
  %res7 = insertelement <8 x double> %res1, double %val7, i32 7
  ret <8 x double> %res7
}

define <8 x double> @merge_8f64_f64_12zzuuzz(double* %ptr) nounwind uwtable noinline ssp {
; ALL-LABEL: merge_8f64_f64_12zzuuzz:
; ALL:       # BB#0:
; ALL-NEXT:    vmovupd 8(%rdi), %xmm0
; ALL-NEXT:    vxorpd %xmm1, %xmm1, %xmm1
; ALL-NEXT:    vinsertf128 $1, %xmm1, %ymm0, %ymm0
; ALL-NEXT:    vxorpd %ymm1, %ymm1, %ymm1
; ALL-NEXT:    vinsertf64x4 $1, %ymm1, %zmm0, %zmm0
; ALL-NEXT:    retq
;
; X32-AVX512F-LABEL: merge_8f64_f64_12zzuuzz:
; X32-AVX512F:       # BB#0:
; X32-AVX512F-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-AVX512F-NEXT:    vmovupd 8(%eax), %xmm0
; X32-AVX512F-NEXT:    vxorpd %xmm1, %xmm1, %xmm1
; X32-AVX512F-NEXT:    vinsertf128 $1, %xmm1, %ymm0, %ymm0
; X32-AVX512F-NEXT:    vxorpd %ymm1, %ymm1, %ymm1
; X32-AVX512F-NEXT:    vinsertf64x4 $1, %ymm1, %zmm0, %zmm0
; X32-AVX512F-NEXT:    retl
  %ptr0 = getelementptr inbounds double, double* %ptr, i64 1
  %ptr1 = getelementptr inbounds double, double* %ptr, i64 2
  %val0 = load double, double* %ptr0
  %val1 = load double, double* %ptr1
  %res0 = insertelement <8 x double> undef, double %val0, i32 0
  %res1 = insertelement <8 x double> %res0, double %val1, i32 1
  %res2 = insertelement <8 x double> %res1, double   0.0, i32 2
  %res3 = insertelement <8 x double> %res2, double   0.0, i32 3
  %res6 = insertelement <8 x double> %res3, double   0.0, i32 6
  %res7 = insertelement <8 x double> %res6, double   0.0, i32 7
  ret <8 x double> %res7
}

define <8 x double> @merge_8f64_f64_1u3u5zu8(double* %ptr) nounwind uwtable noinline ssp {
; ALL-LABEL: merge_8f64_f64_1u3u5zu8:
; ALL:       # BB#0:
; ALL-NEXT:    vmovsd {{.*#+}} xmm0 = mem[0],zero
; ALL-NEXT:    vmovddup {{.*#+}} xmm1 = mem[0,0]
; ALL-NEXT:    vinsertf128 $1, %xmm1, %ymm0, %ymm0
; ALL-NEXT:    vmovsd {{.*#+}} xmm1 = mem[0],zero
; ALL-NEXT:    vmovsd {{.*#+}} xmm2 = mem[0],zero
; ALL-NEXT:    vinsertf128 $1, %xmm2, %ymm1, %ymm1
; ALL-NEXT:    vinsertf64x4 $1, %ymm0, %zmm1, %zmm0
; ALL-NEXT:    retq
;
; X32-AVX512F-LABEL: merge_8f64_f64_1u3u5zu8:
; X32-AVX512F:       # BB#0:
; X32-AVX512F-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-AVX512F-NEXT:    vmovsd {{.*#+}} xmm0 = mem[0],zero
; X32-AVX512F-NEXT:    vmovddup {{.*#+}} xmm1 = mem[0,0]
; X32-AVX512F-NEXT:    vinsertf128 $1, %xmm1, %ymm0, %ymm0
; X32-AVX512F-NEXT:    vmovsd {{.*#+}} xmm1 = mem[0],zero
; X32-AVX512F-NEXT:    vmovsd {{.*#+}} xmm2 = mem[0],zero
; X32-AVX512F-NEXT:    vinsertf128 $1, %xmm2, %ymm1, %ymm1
; X32-AVX512F-NEXT:    vinsertf64x4 $1, %ymm0, %zmm1, %zmm0
; X32-AVX512F-NEXT:    retl
  %ptr0 = getelementptr inbounds double, double* %ptr, i64 1
  %ptr2 = getelementptr inbounds double, double* %ptr, i64 3
  %ptr4 = getelementptr inbounds double, double* %ptr, i64 5
  %ptr7 = getelementptr inbounds double, double* %ptr, i64 8
  %val0 = load double, double* %ptr0
  %val2 = load double, double* %ptr2
  %val4 = load double, double* %ptr4
  %val7 = load double, double* %ptr7
  %res0 = insertelement <8 x double> undef, double %val0, i32 0
  %res2 = insertelement <8 x double> %res0, double %val2, i32 2
  %res4 = insertelement <8 x double> %res2, double %val4, i32 4
  %res5 = insertelement <8 x double> %res4, double   0.0, i32 5
  %res7 = insertelement <8 x double> %res5, double %val7, i32 7
  ret <8 x double> %res7
}

define <8 x i64> @merge_8i64_4i64_z3(<4 x i64>* %ptr) nounwind uwtable noinline ssp {
; ALL-LABEL: merge_8i64_4i64_z3:
; ALL:       # BB#0:
; ALL-NEXT:    vpxor %ymm0, %ymm0, %ymm0
; ALL-NEXT:    vinserti64x4 $1, 96(%rdi), %zmm0, %zmm0
; ALL-NEXT:    retq
;
; X32-AVX512F-LABEL: merge_8i64_4i64_z3:
; X32-AVX512F:       # BB#0:
; X32-AVX512F-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-AVX512F-NEXT:    vpxor %ymm0, %ymm0, %ymm0
; X32-AVX512F-NEXT:    vinserti64x4 $1, 96(%eax), %zmm0, %zmm0
; X32-AVX512F-NEXT:    retl
  %ptr1 = getelementptr inbounds <4 x i64>, <4 x i64>* %ptr, i64 3
  %val1 = load <4 x i64>, <4 x i64>* %ptr1
  %res = shufflevector <4 x i64> zeroinitializer, <4 x i64> %val1, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  ret <8 x i64> %res
}

define <8 x i64> @merge_8i64_i64_56zz9uzz(i64* %ptr) nounwind uwtable noinline ssp {
; ALL-LABEL: merge_8i64_i64_56zz9uzz:
; ALL:       # BB#0:
; ALL-NEXT:    vmovdqu 40(%rdi), %xmm0
; ALL-NEXT:    vpxor %xmm1, %xmm1, %xmm1
; ALL-NEXT:    vinserti128 $1, %xmm1, %ymm0, %ymm0
; ALL-NEXT:    vmovq {{.*#+}} xmm1 = mem[0],zero
; ALL-NEXT:    vinserti64x4 $1, %ymm1, %zmm0, %zmm0
; ALL-NEXT:    retq
;
; X32-AVX512F-LABEL: merge_8i64_i64_56zz9uzz:
; X32-AVX512F:       # BB#0:
; X32-AVX512F-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-AVX512F-NEXT:    vmovdqu 40(%eax), %xmm0
; X32-AVX512F-NEXT:    vpxor %xmm1, %xmm1, %xmm1
; X32-AVX512F-NEXT:    vinserti128 $1, %xmm1, %ymm0, %ymm0
; X32-AVX512F-NEXT:    vmovq {{.*#+}} xmm1 = mem[0],zero
; X32-AVX512F-NEXT:    vinserti64x4 $1, %ymm1, %zmm0, %zmm0
; X32-AVX512F-NEXT:    retl
  %ptr0 = getelementptr inbounds i64, i64* %ptr, i64 5
  %ptr1 = getelementptr inbounds i64, i64* %ptr, i64 6
  %ptr4 = getelementptr inbounds i64, i64* %ptr, i64 9
  %val0 = load i64, i64* %ptr0
  %val1 = load i64, i64* %ptr1
  %val4 = load i64, i64* %ptr4
  %res0 = insertelement <8 x i64> undef, i64 %val0, i32 0
  %res1 = insertelement <8 x i64> %res0, i64 %val1, i32 1
  %res2 = insertelement <8 x i64> %res1, i64     0, i32 2
  %res3 = insertelement <8 x i64> %res2, i64     0, i32 3
  %res4 = insertelement <8 x i64> %res3, i64 %val4, i32 4
  %res6 = insertelement <8 x i64> %res4, i64     0, i32 6
  %res7 = insertelement <8 x i64> %res6, i64     0, i32 7
  ret <8 x i64> %res7
}

define <8 x i64> @merge_8i64_i64_1u3u5zu8(i64* %ptr) nounwind uwtable noinline ssp {
; ALL-LABEL: merge_8i64_i64_1u3u5zu8:
; ALL:       # BB#0:
; ALL-NEXT:    vmovq {{.*#+}} xmm0 = mem[0],zero
; ALL-NEXT:    vpbroadcastq 64(%rdi), %xmm1
; ALL-NEXT:    vinserti128 $1, %xmm1, %ymm0, %ymm0
; ALL-NEXT:    vmovq {{.*#+}} xmm1 = mem[0],zero
; ALL-NEXT:    vmovq {{.*#+}} xmm2 = mem[0],zero
; ALL-NEXT:    vinserti128 $1, %xmm2, %ymm1, %ymm1
; ALL-NEXT:    vinserti64x4 $1, %ymm0, %zmm1, %zmm0
; ALL-NEXT:    retq
;
; X32-AVX512F-LABEL: merge_8i64_i64_1u3u5zu8:
; X32-AVX512F:       # BB#0:
; X32-AVX512F-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-AVX512F-NEXT:    vpinsrd $2, 64(%eax), %xmm0, %xmm0
; X32-AVX512F-NEXT:    vpinsrd $3, 68(%eax), %xmm0, %xmm0
; X32-AVX512F-NEXT:    vmovq {{.*#+}} xmm1 = mem[0],zero
; X32-AVX512F-NEXT:    vinserti128 $1, %xmm0, %ymm1, %ymm0
; X32-AVX512F-NEXT:    vmovq {{.*#+}} xmm1 = mem[0],zero
; X32-AVX512F-NEXT:    vmovq {{.*#+}} xmm2 = mem[0],zero
; X32-AVX512F-NEXT:    vinserti128 $1, %xmm2, %ymm1, %ymm1
; X32-AVX512F-NEXT:    vinserti64x4 $1, %ymm0, %zmm1, %zmm0
; X32-AVX512F-NEXT:    retl
  %ptr0 = getelementptr inbounds i64, i64* %ptr, i64 1
  %ptr2 = getelementptr inbounds i64, i64* %ptr, i64 3
  %ptr4 = getelementptr inbounds i64, i64* %ptr, i64 5
  %ptr7 = getelementptr inbounds i64, i64* %ptr, i64 8
  %val0 = load i64, i64* %ptr0
  %val2 = load i64, i64* %ptr2
  %val4 = load i64, i64* %ptr4
  %val7 = load i64, i64* %ptr7
  %res0 = insertelement <8 x i64> undef, i64 %val0, i32 0
  %res2 = insertelement <8 x i64> %res0, i64 %val2, i32 2
  %res4 = insertelement <8 x i64> %res2, i64 %val4, i32 4
  %res5 = insertelement <8 x i64> %res4, i64     0, i32 5
  %res7 = insertelement <8 x i64> %res5, i64 %val7, i32 7
  ret <8 x i64> %res7
}

define <16 x float> @merge_16f32_f32_89zzzuuuuuuuuuuuz(float* %ptr) nounwind uwtable noinline ssp {
; ALL-LABEL: merge_16f32_f32_89zzzuuuuuuuuuuuz:
; ALL:       # BB#0:
; ALL-NEXT:    vmovq {{.*#+}} xmm0 = mem[0],zero
; ALL-NEXT:    retq
;
; X32-AVX512F-LABEL: merge_16f32_f32_89zzzuuuuuuuuuuuz:
; X32-AVX512F:       # BB#0:
; X32-AVX512F-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-AVX512F-NEXT:    vmovq {{.*#+}} xmm0 = mem[0],zero
; X32-AVX512F-NEXT:    retl
  %ptr0 = getelementptr inbounds float, float* %ptr, i64 8
  %ptr1 = getelementptr inbounds float, float* %ptr, i64 9
  %val0 = load float, float* %ptr0
  %val1 = load float, float* %ptr1
  %res0 = insertelement <16 x float> undef, float %val0, i32 0
  %res1 = insertelement <16 x float> %res0, float %val1, i32 1
  %res2 = insertelement <16 x float> %res1, float   0.0, i32 2
  %res3 = insertelement <16 x float> %res2, float   0.0, i32 3
  %res4 = insertelement <16 x float> %res3, float   0.0, i32 4
  %resF = insertelement <16 x float> %res4, float   0.0, i32 15
  ret <16 x float> %resF
}

define <16 x float> @merge_16f32_f32_45u7uuuuuuuuuuuu(float* %ptr) nounwind uwtable noinline ssp {
; ALL-LABEL: merge_16f32_f32_45u7uuuuuuuuuuuu:
; ALL:       # BB#0:
; ALL-NEXT:    vmovups 16(%rdi), %xmm0
; ALL-NEXT:    retq
;
; X32-AVX512F-LABEL: merge_16f32_f32_45u7uuuuuuuuuuuu:
; X32-AVX512F:       # BB#0:
; X32-AVX512F-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-AVX512F-NEXT:    vmovups 16(%eax), %xmm0
; X32-AVX512F-NEXT:    retl
  %ptr0 = getelementptr inbounds float, float* %ptr, i64 4
  %ptr1 = getelementptr inbounds float, float* %ptr, i64 5
  %ptr3 = getelementptr inbounds float, float* %ptr, i64 7
  %val0 = load float, float* %ptr0
  %val1 = load float, float* %ptr1
  %val3 = load float, float* %ptr3
  %res0 = insertelement <16 x float> undef, float %val0, i32 0
  %res1 = insertelement <16 x float> %res0, float %val1, i32 1
  %res3 = insertelement <16 x float> %res1, float %val3, i32 3
  ret <16 x float> %res3
}

define <16 x float> @merge_16f32_f32_0uu3uuuuuuuuCuEF(float* %ptr) nounwind uwtable noinline ssp {
; ALL-LABEL: merge_16f32_f32_0uu3uuuuuuuuCuEF:
; ALL:       # BB#0:
; ALL-NEXT:    vmovups (%rdi), %zmm0
; ALL-NEXT:    retq
;
; X32-AVX512F-LABEL: merge_16f32_f32_0uu3uuuuuuuuCuEF:
; X32-AVX512F:       # BB#0:
; X32-AVX512F-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-AVX512F-NEXT:    vmovups (%eax), %zmm0
; X32-AVX512F-NEXT:    retl
  %ptr0 = getelementptr inbounds float, float* %ptr, i64 0
  %ptr3 = getelementptr inbounds float, float* %ptr, i64 3
  %ptrC = getelementptr inbounds float, float* %ptr, i64 12
  %ptrE = getelementptr inbounds float, float* %ptr, i64 14
  %ptrF = getelementptr inbounds float, float* %ptr, i64 15
  %val0 = load float, float* %ptr0
  %val3 = load float, float* %ptr3
  %valC = load float, float* %ptrC
  %valE = load float, float* %ptrE
  %valF = load float, float* %ptrF
  %res0 = insertelement <16 x float> undef, float %val0, i32 0
  %res3 = insertelement <16 x float> %res0, float %val3, i32 3
  %resC = insertelement <16 x float> %res3, float %valC, i32 12
  %resE = insertelement <16 x float> %resC, float %valE, i32 14
  %resF = insertelement <16 x float> %resE, float %valF, i32 15
  ret <16 x float> %resF
}

define <16 x float> @merge_16f32_f32_0uu3zzuuuuuzCuEF(float* %ptr) nounwind uwtable noinline ssp {
; ALL-LABEL: merge_16f32_f32_0uu3zzuuuuuzCuEF:
; ALL:       # BB#0:
; ALL-NEXT:    vmovq {{.*#+}} xmm0 = mem[0],zero
; ALL-NEXT:    vmovss {{.*#+}} xmm1 = mem[0],zero,zero,zero
; ALL-NEXT:    vunpcklpd {{.*#+}} xmm0 = xmm1[0],xmm0[0]
; ALL-NEXT:    vinsertf128 $1, %xmm0, %ymm0, %ymm0
; ALL-NEXT:    vmovupd (%rdi), %xmm1
; ALL-NEXT:    vxorpd %xmm2, %xmm2, %xmm2
; ALL-NEXT:    vinsertf128 $1, %xmm2, %ymm1, %ymm1
; ALL-NEXT:    vinsertf64x4 $1, %ymm0, %zmm1, %zmm0
; ALL-NEXT:    retq
;
; X32-AVX512F-LABEL: merge_16f32_f32_0uu3zzuuuuuzCuEF:
; X32-AVX512F:       # BB#0:
; X32-AVX512F-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-AVX512F-NEXT:    vmovq {{.*#+}} xmm0 = mem[0],zero
; X32-AVX512F-NEXT:    vmovss {{.*#+}} xmm1 = mem[0],zero,zero,zero
; X32-AVX512F-NEXT:    vunpcklpd {{.*#+}} xmm0 = xmm1[0],xmm0[0]
; X32-AVX512F-NEXT:    vinsertf128 $1, %xmm0, %ymm0, %ymm0
; X32-AVX512F-NEXT:    vmovupd (%eax), %xmm1
; X32-AVX512F-NEXT:    vxorpd %xmm2, %xmm2, %xmm2
; X32-AVX512F-NEXT:    vinsertf128 $1, %xmm2, %ymm1, %ymm1
; X32-AVX512F-NEXT:    vinsertf64x4 $1, %ymm0, %zmm1, %zmm0
; X32-AVX512F-NEXT:    retl
  %ptr0 = getelementptr inbounds float, float* %ptr, i64 0
  %ptr3 = getelementptr inbounds float, float* %ptr, i64 3
  %ptrC = getelementptr inbounds float, float* %ptr, i64 12
  %ptrE = getelementptr inbounds float, float* %ptr, i64 14
  %ptrF = getelementptr inbounds float, float* %ptr, i64 15
  %val0 = load float, float* %ptr0
  %val3 = load float, float* %ptr3
  %valC = load float, float* %ptrC
  %valE = load float, float* %ptrE
  %valF = load float, float* %ptrF
  %res0 = insertelement <16 x float> undef, float %val0, i32 0
  %res3 = insertelement <16 x float> %res0, float %val3, i32 3
  %res4 = insertelement <16 x float> %res3, float   0.0, i32 4
  %res5 = insertelement <16 x float> %res4, float   0.0, i32 5
  %resC = insertelement <16 x float> %res5, float %valC, i32 12
  %resD = insertelement <16 x float> %resC, float   0.0, i32 13
  %resE = insertelement <16 x float> %resD, float %valE, i32 14
  %resF = insertelement <16 x float> %resE, float %valF, i32 15
  ret <16 x float> %resF
}

define <16 x i32> @merge_16i32_i32_12zzzuuuuuuuuuuuz(i32* %ptr) nounwind uwtable noinline ssp {
; ALL-LABEL: merge_16i32_i32_12zzzuuuuuuuuuuuz:
; ALL:       # BB#0:
; ALL-NEXT:    vmovq {{.*#+}} xmm0 = mem[0],zero
; ALL-NEXT:    retq
;
; X32-AVX512F-LABEL: merge_16i32_i32_12zzzuuuuuuuuuuuz:
; X32-AVX512F:       # BB#0:
; X32-AVX512F-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-AVX512F-NEXT:    vmovq {{.*#+}} xmm0 = mem[0],zero
; X32-AVX512F-NEXT:    retl
  %ptr0 = getelementptr inbounds i32, i32* %ptr, i64 1
  %ptr1 = getelementptr inbounds i32, i32* %ptr, i64 2
  %val0 = load i32, i32* %ptr0
  %val1 = load i32, i32* %ptr1
  %res0 = insertelement <16 x i32> undef, i32 %val0, i32 0
  %res1 = insertelement <16 x i32> %res0, i32 %val1, i32 1
  %res2 = insertelement <16 x i32> %res1, i32     0, i32 2
  %res3 = insertelement <16 x i32> %res2, i32     0, i32 3
  %res4 = insertelement <16 x i32> %res3, i32     0, i32 4
  %resF = insertelement <16 x i32> %res4, i32     0, i32 15
  ret <16 x i32> %resF
}

define <16 x i32> @merge_16i32_i32_23u5uuuuuuuuuuuu(i32* %ptr) nounwind uwtable noinline ssp {
; ALL-LABEL: merge_16i32_i32_23u5uuuuuuuuuuuu:
; ALL:       # BB#0:
; ALL-NEXT:    vmovups 8(%rdi), %xmm0
; ALL-NEXT:    retq
;
; X32-AVX512F-LABEL: merge_16i32_i32_23u5uuuuuuuuuuuu:
; X32-AVX512F:       # BB#0:
; X32-AVX512F-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-AVX512F-NEXT:    vmovups 8(%eax), %xmm0
; X32-AVX512F-NEXT:    retl
  %ptr0 = getelementptr inbounds i32, i32* %ptr, i64 2
  %ptr1 = getelementptr inbounds i32, i32* %ptr, i64 3
  %ptr3 = getelementptr inbounds i32, i32* %ptr, i64 5
  %val0 = load i32, i32* %ptr0
  %val1 = load i32, i32* %ptr1
  %val3 = load i32, i32* %ptr3
  %res0 = insertelement <16 x i32> undef, i32 %val0, i32 0
  %res1 = insertelement <16 x i32> %res0, i32 %val1, i32 1
  %res3 = insertelement <16 x i32> %res1, i32 %val3, i32 3
  ret <16 x i32> %res3
}

define <16 x i32> @merge_16i32_i32_0uu3uuuuuuuuCuEF(i32* %ptr) nounwind uwtable noinline ssp {
; ALL-LABEL: merge_16i32_i32_0uu3uuuuuuuuCuEF:
; ALL:       # BB#0:
; ALL-NEXT:    vmovdqu32 (%rdi), %zmm0
; ALL-NEXT:    retq
;
; X32-AVX512F-LABEL: merge_16i32_i32_0uu3uuuuuuuuCuEF:
; X32-AVX512F:       # BB#0:
; X32-AVX512F-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-AVX512F-NEXT:    vmovdqu32 (%eax), %zmm0
; X32-AVX512F-NEXT:    retl
  %ptr0 = getelementptr inbounds i32, i32* %ptr, i64 0
  %ptr3 = getelementptr inbounds i32, i32* %ptr, i64 3
  %ptrC = getelementptr inbounds i32, i32* %ptr, i64 12
  %ptrE = getelementptr inbounds i32, i32* %ptr, i64 14
  %ptrF = getelementptr inbounds i32, i32* %ptr, i64 15
  %val0 = load i32, i32* %ptr0
  %val3 = load i32, i32* %ptr3
  %valC = load i32, i32* %ptrC
  %valE = load i32, i32* %ptrE
  %valF = load i32, i32* %ptrF
  %res0 = insertelement <16 x i32> undef, i32 %val0, i32 0
  %res3 = insertelement <16 x i32> %res0, i32 %val3, i32 3
  %resC = insertelement <16 x i32> %res3, i32 %valC, i32 12
  %resE = insertelement <16 x i32> %resC, i32 %valE, i32 14
  %resF = insertelement <16 x i32> %resE, i32 %valF, i32 15
  ret <16 x i32> %resF
}

define <16 x i32> @merge_16i32_i32_0uu3zzuuuuuzCuEF(i32* %ptr) nounwind uwtable noinline ssp {
; ALL-LABEL: merge_16i32_i32_0uu3zzuuuuuzCuEF:
; ALL:       # BB#0:
; ALL-NEXT:    vmovq {{.*#+}} xmm0 = mem[0],zero
; ALL-NEXT:    vmovd {{.*#+}} xmm1 = mem[0],zero,zero,zero
; ALL-NEXT:    vpunpcklqdq {{.*#+}} xmm0 = xmm1[0],xmm0[0]
; ALL-NEXT:    vinserti128 $1, %xmm0, %ymm0, %ymm0
; ALL-NEXT:    vmovdqu (%rdi), %xmm1
; ALL-NEXT:    vpxor %xmm2, %xmm2, %xmm2
; ALL-NEXT:    vinserti128 $1, %xmm2, %ymm1, %ymm1
; ALL-NEXT:    vinserti64x4 $1, %ymm0, %zmm1, %zmm0
; ALL-NEXT:    retq
;
; X32-AVX512F-LABEL: merge_16i32_i32_0uu3zzuuuuuzCuEF:
; X32-AVX512F:       # BB#0:
; X32-AVX512F-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-AVX512F-NEXT:    vmovq {{.*#+}} xmm0 = mem[0],zero
; X32-AVX512F-NEXT:    vmovd {{.*#+}} xmm1 = mem[0],zero,zero,zero
; X32-AVX512F-NEXT:    vpunpcklqdq {{.*#+}} xmm0 = xmm1[0],xmm0[0]
; X32-AVX512F-NEXT:    vinserti128 $1, %xmm0, %ymm0, %ymm0
; X32-AVX512F-NEXT:    vmovdqu (%eax), %xmm1
; X32-AVX512F-NEXT:    vpxor %xmm2, %xmm2, %xmm2
; X32-AVX512F-NEXT:    vinserti128 $1, %xmm2, %ymm1, %ymm1
; X32-AVX512F-NEXT:    vinserti64x4 $1, %ymm0, %zmm1, %zmm0
; X32-AVX512F-NEXT:    retl
  %ptr0 = getelementptr inbounds i32, i32* %ptr, i64 0
  %ptr3 = getelementptr inbounds i32, i32* %ptr, i64 3
  %ptrC = getelementptr inbounds i32, i32* %ptr, i64 12
  %ptrE = getelementptr inbounds i32, i32* %ptr, i64 14
  %ptrF = getelementptr inbounds i32, i32* %ptr, i64 15
  %val0 = load i32, i32* %ptr0
  %val3 = load i32, i32* %ptr3
  %valC = load i32, i32* %ptrC
  %valE = load i32, i32* %ptrE
  %valF = load i32, i32* %ptrF
  %res0 = insertelement <16 x i32> undef, i32 %val0, i32 0
  %res3 = insertelement <16 x i32> %res0, i32 %val3, i32 3
  %res4 = insertelement <16 x i32> %res3, i32     0, i32 4
  %res5 = insertelement <16 x i32> %res4, i32     0, i32 5
  %resC = insertelement <16 x i32> %res5, i32 %valC, i32 12
  %resD = insertelement <16 x i32> %resC, i32     0, i32 13
  %resE = insertelement <16 x i32> %resD, i32 %valE, i32 14
  %resF = insertelement <16 x i32> %resE, i32 %valF, i32 15
  ret <16 x i32> %resF
}

define <32 x i16> @merge_32i16_i16_12u4uuuuuuuuuuuuuuuuuuuuuuuuuuzz(i16* %ptr) nounwind uwtable noinline ssp {
; AVX512F-LABEL: merge_32i16_i16_12u4uuuuuuuuuuuuuuuuuuuuuuuuuuzz:
; AVX512F:       # BB#0:
; AVX512F-NEXT:    vmovq {{.*#+}} xmm0 = mem[0],zero
; AVX512F-NEXT:    vxorps %ymm1, %ymm1, %ymm1
; AVX512F-NEXT:    retq
;
; AVX512BW-LABEL: merge_32i16_i16_12u4uuuuuuuuuuuuuuuuuuuuuuuuuuzz:
; AVX512BW:       # BB#0:
; AVX512BW-NEXT:    vmovq {{.*#+}} xmm0 = mem[0],zero
; AVX512BW-NEXT:    retq
;
; X32-AVX512F-LABEL: merge_32i16_i16_12u4uuuuuuuuuuuuuuuuuuuuuuuuuuzz:
; X32-AVX512F:       # BB#0:
; X32-AVX512F-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-AVX512F-NEXT:    vmovq {{.*#+}} xmm0 = mem[0],zero
; X32-AVX512F-NEXT:    vxorps %ymm1, %ymm1, %ymm1
; X32-AVX512F-NEXT:    retl
  %ptr0 = getelementptr inbounds i16, i16* %ptr, i64 1
  %ptr1 = getelementptr inbounds i16, i16* %ptr, i64 2
  %ptr3 = getelementptr inbounds i16, i16* %ptr, i64 4
  %val0 = load i16, i16* %ptr0
  %val1 = load i16, i16* %ptr1
  %val3 = load i16, i16* %ptr3
  %res0 = insertelement <32 x i16> undef, i16 %val0, i16 0
  %res1 = insertelement <32 x i16> %res0, i16 %val1, i16 1
  %res3 = insertelement <32 x i16> %res1, i16 %val3, i16 3
  %res30 = insertelement <32 x i16> %res3, i16 0, i16 30
  %res31 = insertelement <32 x i16> %res30, i16 0, i16 31
  ret <32 x i16> %res31
}

define <32 x i16> @merge_32i16_i16_45u7uuuuuuuuuuuuuuuuuuuuuuuuuuuu(i16* %ptr) nounwind uwtable noinline ssp {
; ALL-LABEL: merge_32i16_i16_45u7uuuuuuuuuuuuuuuuuuuuuuuuuuuu:
; ALL:       # BB#0:
; ALL-NEXT:    vmovq {{.*#+}} xmm0 = mem[0],zero
; ALL-NEXT:    retq
;
; X32-AVX512F-LABEL: merge_32i16_i16_45u7uuuuuuuuuuuuuuuuuuuuuuuuuuuu:
; X32-AVX512F:       # BB#0:
; X32-AVX512F-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-AVX512F-NEXT:    vmovq {{.*#+}} xmm0 = mem[0],zero
; X32-AVX512F-NEXT:    retl
  %ptr0 = getelementptr inbounds i16, i16* %ptr, i64 4
  %ptr1 = getelementptr inbounds i16, i16* %ptr, i64 5
  %ptr3 = getelementptr inbounds i16, i16* %ptr, i64 7
  %val0 = load i16, i16* %ptr0
  %val1 = load i16, i16* %ptr1
  %val3 = load i16, i16* %ptr3
  %res0 = insertelement <32 x i16> undef, i16 %val0, i16 0
  %res1 = insertelement <32 x i16> %res0, i16 %val1, i16 1
  %res3 = insertelement <32 x i16> %res1, i16 %val3, i16 3
  ret <32 x i16> %res3
}

define <32 x i16> @merge_32i16_i16_23uzuuuuuuuuuuzzzzuuuuuuuuuuuuuu(i16* %ptr) nounwind uwtable noinline ssp {
; AVX512F-LABEL: merge_32i16_i16_23uzuuuuuuuuuuzzzzuuuuuuuuuuuuuu:
; AVX512F:       # BB#0:
; AVX512F-NEXT:    vmovd {{.*#+}} xmm0 = mem[0],zero,zero,zero
; AVX512F-NEXT:    vxorps %ymm1, %ymm1, %ymm1
; AVX512F-NEXT:    retq
;
; AVX512BW-LABEL: merge_32i16_i16_23uzuuuuuuuuuuzzzzuuuuuuuuuuuuuu:
; AVX512BW:       # BB#0:
; AVX512BW-NEXT:    vmovd {{.*#+}} xmm0 = mem[0],zero,zero,zero
; AVX512BW-NEXT:    retq
;
; X32-AVX512F-LABEL: merge_32i16_i16_23uzuuuuuuuuuuzzzzuuuuuuuuuuuuuu:
; X32-AVX512F:       # BB#0:
; X32-AVX512F-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-AVX512F-NEXT:    vmovd {{.*#+}} xmm0 = mem[0],zero,zero,zero
; X32-AVX512F-NEXT:    vxorps %ymm1, %ymm1, %ymm1
; X32-AVX512F-NEXT:    retl
  %ptr0 = getelementptr inbounds i16, i16* %ptr, i64 2
  %ptr1 = getelementptr inbounds i16, i16* %ptr, i64 3
  %val0 = load i16, i16* %ptr0
  %val1 = load i16, i16* %ptr1
  %res0 = insertelement <32 x i16> undef, i16 %val0, i16 0
  %res1 = insertelement <32 x i16> %res0, i16 %val1, i16 1
  %res3 = insertelement <32 x i16> %res1, i16     0, i16 3
  %resE = insertelement <32 x i16> %res3, i16     0, i16 14
  %resF = insertelement <32 x i16> %resE, i16     0, i16 15
  %resG = insertelement <32 x i16> %resF, i16     0, i16 16
  %resH = insertelement <32 x i16> %resG, i16     0, i16 17
  ret <32 x i16> %resH
}

define <64 x i8> @merge_64i8_i8_12u4uuu8uuuuuuzzzzuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuz(i8* %ptr) nounwind uwtable noinline ssp {
; AVX512F-LABEL: merge_64i8_i8_12u4uuu8uuuuuuzzzzuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuz:
; AVX512F:       # BB#0:
; AVX512F-NEXT:    vmovq {{.*#+}} xmm0 = mem[0],zero
; AVX512F-NEXT:    vxorps %ymm1, %ymm1, %ymm1
; AVX512F-NEXT:    retq
;
; AVX512BW-LABEL: merge_64i8_i8_12u4uuu8uuuuuuzzzzuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuz:
; AVX512BW:       # BB#0:
; AVX512BW-NEXT:    vmovq {{.*#+}} xmm0 = mem[0],zero
; AVX512BW-NEXT:    retq
;
; X32-AVX512F-LABEL: merge_64i8_i8_12u4uuu8uuuuuuzzzzuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuz:
; X32-AVX512F:       # BB#0:
; X32-AVX512F-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-AVX512F-NEXT:    vmovq {{.*#+}} xmm0 = mem[0],zero
; X32-AVX512F-NEXT:    vxorps %ymm1, %ymm1, %ymm1
; X32-AVX512F-NEXT:    retl
  %ptr0 = getelementptr inbounds i8, i8* %ptr, i64 1
  %ptr1 = getelementptr inbounds i8, i8* %ptr, i64 2
  %ptr3 = getelementptr inbounds i8, i8* %ptr, i64 4
  %ptr7 = getelementptr inbounds i8, i8* %ptr, i64 8
  %val0 = load i8, i8* %ptr0
  %val1 = load i8, i8* %ptr1
  %val3 = load i8, i8* %ptr3
  %val7 = load i8, i8* %ptr7
  %res0  = insertelement <64 x i8> undef,  i8 %val0, i8 0
  %res1  = insertelement <64 x i8> %res0,  i8 %val1, i8 1
  %res3  = insertelement <64 x i8> %res1,  i8 %val3, i8 3
  %res7  = insertelement <64 x i8> %res3,  i8 %val7, i8 7
  %res14 = insertelement <64 x i8> %res7,  i8     0, i8 14
  %res15 = insertelement <64 x i8> %res14, i8     0, i8 15
  %res16 = insertelement <64 x i8> %res15, i8     0, i8 16
  %res17 = insertelement <64 x i8> %res16, i8     0, i8 17
  %res63 = insertelement <64 x i8> %res17, i8     0, i8 63
  ret <64 x i8> %res63
}

define <64 x i8> @merge_64i8_i8_12u4uuuuuuuuuuzzzzuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuz(i8* %ptr) nounwind uwtable noinline ssp {
; AVX512F-LABEL: merge_64i8_i8_12u4uuuuuuuuuuzzzzuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuz:
; AVX512F:       # BB#0:
; AVX512F-NEXT:    vmovd {{.*#+}} xmm0 = mem[0],zero,zero,zero
; AVX512F-NEXT:    vxorps %ymm1, %ymm1, %ymm1
; AVX512F-NEXT:    retq
;
; AVX512BW-LABEL: merge_64i8_i8_12u4uuuuuuuuuuzzzzuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuz:
; AVX512BW:       # BB#0:
; AVX512BW-NEXT:    vmovd {{.*#+}} xmm0 = mem[0],zero,zero,zero
; AVX512BW-NEXT:    retq
;
; X32-AVX512F-LABEL: merge_64i8_i8_12u4uuuuuuuuuuzzzzuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuz:
; X32-AVX512F:       # BB#0:
; X32-AVX512F-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-AVX512F-NEXT:    vmovd {{.*#+}} xmm0 = mem[0],zero,zero,zero
; X32-AVX512F-NEXT:    vxorps %ymm1, %ymm1, %ymm1
; X32-AVX512F-NEXT:    retl
  %ptr0 = getelementptr inbounds i8, i8* %ptr, i64 1
  %ptr1 = getelementptr inbounds i8, i8* %ptr, i64 2
  %ptr3 = getelementptr inbounds i8, i8* %ptr, i64 4
  %val0 = load i8, i8* %ptr0
  %val1 = load i8, i8* %ptr1
  %val3 = load i8, i8* %ptr3
  %res0  = insertelement <64 x i8> undef,  i8 %val0, i8 0
  %res1  = insertelement <64 x i8> %res0,  i8 %val1, i8 1
  %res3  = insertelement <64 x i8> %res1,  i8 %val3, i8 3
  %res14 = insertelement <64 x i8> %res3,  i8     0, i8 14
  %res15 = insertelement <64 x i8> %res14, i8     0, i8 15
  %res16 = insertelement <64 x i8> %res15, i8     0, i8 16
  %res17 = insertelement <64 x i8> %res16, i8     0, i8 17
  %res63 = insertelement <64 x i8> %res17, i8     0, i8 63
  ret <64 x i8> %res63
}
