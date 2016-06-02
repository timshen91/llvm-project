; NOTE: Assertions have been autogenerated by update_llc_test_checks.py
; RUN: llc < %s -mtriple=x86_64-apple-darwin -mcpu=knl -mattr=+avx512vl --show-mc-encoding| FileCheck %s

declare void @llvm.x86.avx512.mask.store.pd.128(i8*, <2 x double>, i8)

define void@test_int_x86_avx512_mask_store_pd_128(i8* %ptr1, i8* %ptr2, <2 x double> %x1, i8 %x2) {
; CHECK-LABEL: test_int_x86_avx512_mask_store_pd_128:
; CHECK:       ## BB#0:
; CHECK-NEXT:    kmovw %edx, %k1 ## encoding: [0xc5,0xf8,0x92,0xca]
; CHECK-NEXT:    vmovapd %xmm0, (%rdi) {%k1} ## encoding: [0x62,0xf1,0xfd,0x09,0x29,0x07]
; CHECK-NEXT:    vmovapd %xmm0, (%rsi) ## encoding: [0x62,0xf1,0xfd,0x08,0x29,0x06]
; CHECK-NEXT:    retq ## encoding: [0xc3]
  call void @llvm.x86.avx512.mask.store.pd.128(i8* %ptr1, <2 x double> %x1, i8 %x2)
  call void @llvm.x86.avx512.mask.store.pd.128(i8* %ptr2, <2 x double> %x1, i8 -1)
  ret void
}

declare void @llvm.x86.avx512.mask.store.pd.256(i8*, <4 x double>, i8)

define void@test_int_x86_avx512_mask_store_pd_256(i8* %ptr1, i8* %ptr2, <4 x double> %x1, i8 %x2) {
; CHECK-LABEL: test_int_x86_avx512_mask_store_pd_256:
; CHECK:       ## BB#0:
; CHECK-NEXT:    kmovw %edx, %k1 ## encoding: [0xc5,0xf8,0x92,0xca]
; CHECK-NEXT:    vmovapd %ymm0, (%rdi) {%k1} ## encoding: [0x62,0xf1,0xfd,0x29,0x29,0x07]
; CHECK-NEXT:    vmovapd %ymm0, (%rsi) ## encoding: [0x62,0xf1,0xfd,0x28,0x29,0x06]
; CHECK-NEXT:    retq ## encoding: [0xc3]
  call void @llvm.x86.avx512.mask.store.pd.256(i8* %ptr1, <4 x double> %x1, i8 %x2)
  call void @llvm.x86.avx512.mask.store.pd.256(i8* %ptr2, <4 x double> %x1, i8 -1)
  ret void
}

declare void @llvm.x86.avx512.mask.storeu.pd.128(i8*, <2 x double>, i8)

define void@test_int_x86_avx512_mask_storeu_pd_128(i8* %ptr1, i8* %ptr2, <2 x double> %x1, i8 %x2) {
; CHECK-LABEL: test_int_x86_avx512_mask_storeu_pd_128:
; CHECK:       ## BB#0:
; CHECK-NEXT:    kmovw %edx, %k1 ## encoding: [0xc5,0xf8,0x92,0xca]
; CHECK-NEXT:    vmovupd %xmm0, (%rdi) {%k1} ## encoding: [0x62,0xf1,0xfd,0x09,0x11,0x07]
; CHECK-NEXT:    vmovupd %xmm0, (%rsi) ## encoding: [0x62,0xf1,0xfd,0x08,0x11,0x06]
; CHECK-NEXT:    retq ## encoding: [0xc3]
  call void @llvm.x86.avx512.mask.storeu.pd.128(i8* %ptr1, <2 x double> %x1, i8 %x2)
  call void @llvm.x86.avx512.mask.storeu.pd.128(i8* %ptr2, <2 x double> %x1, i8 -1)
  ret void
}

declare void @llvm.x86.avx512.mask.storeu.pd.256(i8*, <4 x double>, i8)

define void@test_int_x86_avx512_mask_storeu_pd_256(i8* %ptr1, i8* %ptr2, <4 x double> %x1, i8 %x2) {
; CHECK-LABEL: test_int_x86_avx512_mask_storeu_pd_256:
; CHECK:       ## BB#0:
; CHECK-NEXT:    kmovw %edx, %k1 ## encoding: [0xc5,0xf8,0x92,0xca]
; CHECK-NEXT:    vmovupd %ymm0, (%rdi) {%k1} ## encoding: [0x62,0xf1,0xfd,0x29,0x11,0x07]
; CHECK-NEXT:    vmovupd %ymm0, (%rsi) ## encoding: [0x62,0xf1,0xfd,0x28,0x11,0x06]
; CHECK-NEXT:    retq ## encoding: [0xc3]
  call void @llvm.x86.avx512.mask.storeu.pd.256(i8* %ptr1, <4 x double> %x1, i8 %x2)
  call void @llvm.x86.avx512.mask.storeu.pd.256(i8* %ptr2, <4 x double> %x1, i8 -1)
  ret void
}

declare void @llvm.x86.avx512.mask.store.ps.128(i8*, <4 x float>, i8)

define void@test_int_x86_avx512_mask_store_ps_128(i8* %ptr1, i8* %ptr2, <4 x float> %x1, i8 %x2) {
; CHECK-LABEL: test_int_x86_avx512_mask_store_ps_128:
; CHECK:       ## BB#0:
; CHECK-NEXT:    kmovw %edx, %k1 ## encoding: [0xc5,0xf8,0x92,0xca]
; CHECK-NEXT:    vmovaps %xmm0, (%rdi) {%k1} ## encoding: [0x62,0xf1,0x7c,0x09,0x29,0x07]
; CHECK-NEXT:    vmovaps %xmm0, (%rsi) ## encoding: [0x62,0xf1,0x7c,0x08,0x29,0x06]
; CHECK-NEXT:    retq ## encoding: [0xc3]
    call void @llvm.x86.avx512.mask.store.ps.128(i8* %ptr1, <4 x float> %x1, i8 %x2)
    call void @llvm.x86.avx512.mask.store.ps.128(i8* %ptr2, <4 x float> %x1, i8 -1)
    ret void
}

declare void @llvm.x86.avx512.mask.store.ps.256(i8*, <8 x float>, i8)

define void@test_int_x86_avx512_mask_store_ps_256(i8* %ptr1, i8* %ptr2, <8 x float> %x1, i8 %x2) {
; CHECK-LABEL: test_int_x86_avx512_mask_store_ps_256:
; CHECK:       ## BB#0:
; CHECK-NEXT:    kmovw %edx, %k1 ## encoding: [0xc5,0xf8,0x92,0xca]
; CHECK-NEXT:    vmovaps %ymm0, (%rdi) {%k1} ## encoding: [0x62,0xf1,0x7c,0x29,0x29,0x07]
; CHECK-NEXT:    vmovaps %ymm0, (%rsi) ## encoding: [0x62,0xf1,0x7c,0x28,0x29,0x06]
; CHECK-NEXT:    retq ## encoding: [0xc3]
    call void @llvm.x86.avx512.mask.store.ps.256(i8* %ptr1, <8 x float> %x1, i8 %x2)
    call void @llvm.x86.avx512.mask.store.ps.256(i8* %ptr2, <8 x float> %x1, i8 -1)
    ret void
}

declare void @llvm.x86.avx512.mask.storeu.ps.128(i8*, <4 x float>, i8)

define void@test_int_x86_avx512_mask_storeu_ps_128(i8* %ptr1, i8* %ptr2, <4 x float> %x1, i8 %x2) {
; CHECK-LABEL: test_int_x86_avx512_mask_storeu_ps_128:
; CHECK:       ## BB#0:
; CHECK-NEXT:    kmovw %edx, %k1 ## encoding: [0xc5,0xf8,0x92,0xca]
; CHECK-NEXT:    vmovups %xmm0, (%rdi) {%k1} ## encoding: [0x62,0xf1,0x7c,0x09,0x11,0x07]
; CHECK-NEXT:    vmovups %xmm0, (%rsi) ## encoding: [0x62,0xf1,0x7c,0x08,0x11,0x06]
; CHECK-NEXT:    retq ## encoding: [0xc3]
    call void @llvm.x86.avx512.mask.storeu.ps.128(i8* %ptr1, <4 x float> %x1, i8 %x2)
    call void @llvm.x86.avx512.mask.storeu.ps.128(i8* %ptr2, <4 x float> %x1, i8 -1)
    ret void
}

declare void @llvm.x86.avx512.mask.storeu.ps.256(i8*, <8 x float>, i8)

define void@test_int_x86_avx512_mask_storeu_ps_256(i8* %ptr1, i8* %ptr2, <8 x float> %x1, i8 %x2) {
; CHECK-LABEL: test_int_x86_avx512_mask_storeu_ps_256:
; CHECK:       ## BB#0:
; CHECK-NEXT:    kmovw %edx, %k1 ## encoding: [0xc5,0xf8,0x92,0xca]
; CHECK-NEXT:    vmovups %ymm0, (%rdi) {%k1} ## encoding: [0x62,0xf1,0x7c,0x29,0x11,0x07]
; CHECK-NEXT:    vmovups %ymm0, (%rsi) ## encoding: [0x62,0xf1,0x7c,0x28,0x11,0x06]
; CHECK-NEXT:    retq ## encoding: [0xc3]
    call void @llvm.x86.avx512.mask.storeu.ps.256(i8* %ptr1, <8 x float> %x1, i8 %x2)
    call void @llvm.x86.avx512.mask.storeu.ps.256(i8* %ptr2, <8 x float> %x1, i8 -1)
    ret void
}

declare void @llvm.x86.avx512.mask.storeu.q.128(i8*, <2 x i64>, i8)

define void@test_int_x86_avx512_mask_storeu_q_128(i8* %ptr1, i8* %ptr2, <2 x i64> %x1, i8 %x2) {
; CHECK-LABEL: test_int_x86_avx512_mask_storeu_q_128:
; CHECK:       ## BB#0:
; CHECK-NEXT:    kmovw %edx, %k1 ## encoding: [0xc5,0xf8,0x92,0xca]
; CHECK-NEXT:    vmovdqu64 %xmm0, (%rdi) {%k1} ## encoding: [0x62,0xf1,0xfe,0x09,0x7f,0x07]
; CHECK-NEXT:    vmovdqu64 %xmm0, (%rsi) ## encoding: [0x62,0xf1,0xfe,0x08,0x7f,0x06]
; CHECK-NEXT:    retq ## encoding: [0xc3]
  call void @llvm.x86.avx512.mask.storeu.q.128(i8* %ptr1, <2 x i64> %x1, i8 %x2)
  call void @llvm.x86.avx512.mask.storeu.q.128(i8* %ptr2, <2 x i64> %x1, i8 -1)
  ret void
}

declare void @llvm.x86.avx512.mask.storeu.q.256(i8*, <4 x i64>, i8)

define void@test_int_x86_avx512_mask_storeu_q_256(i8* %ptr1, i8* %ptr2, <4 x i64> %x1, i8 %x2) {
; CHECK-LABEL: test_int_x86_avx512_mask_storeu_q_256:
; CHECK:       ## BB#0:
; CHECK-NEXT:    kmovw %edx, %k1 ## encoding: [0xc5,0xf8,0x92,0xca]
; CHECK-NEXT:    vmovdqu64 %ymm0, (%rdi) {%k1} ## encoding: [0x62,0xf1,0xfe,0x29,0x7f,0x07]
; CHECK-NEXT:    vmovdqu64 %ymm0, (%rsi) ## encoding: [0x62,0xf1,0xfe,0x28,0x7f,0x06]
; CHECK-NEXT:    retq ## encoding: [0xc3]
  call void @llvm.x86.avx512.mask.storeu.q.256(i8* %ptr1, <4 x i64> %x1, i8 %x2)
  call void @llvm.x86.avx512.mask.storeu.q.256(i8* %ptr2, <4 x i64> %x1, i8 -1)
  ret void
}

declare void @llvm.x86.avx512.mask.storeu.d.128(i8*, <4 x i32>, i8)

define void@test_int_x86_avx512_mask_storeu_d_128(i8* %ptr1, i8* %ptr2, <4 x i32> %x1, i8 %x2) {
; CHECK-LABEL: test_int_x86_avx512_mask_storeu_d_128:
; CHECK:       ## BB#0:
; CHECK-NEXT:    kmovw %edx, %k1 ## encoding: [0xc5,0xf8,0x92,0xca]
; CHECK-NEXT:    vmovdqu32 %xmm0, (%rdi) {%k1} ## encoding: [0x62,0xf1,0x7e,0x09,0x7f,0x07]
; CHECK-NEXT:    vmovdqu32 %xmm0, (%rsi) ## encoding: [0x62,0xf1,0x7e,0x08,0x7f,0x06]
; CHECK-NEXT:    retq ## encoding: [0xc3]
  call void @llvm.x86.avx512.mask.storeu.d.128(i8* %ptr1, <4 x i32> %x1, i8 %x2)
  call void @llvm.x86.avx512.mask.storeu.d.128(i8* %ptr2, <4 x i32> %x1, i8 -1)
  ret void
}

declare void @llvm.x86.avx512.mask.storeu.d.256(i8*, <8 x i32>, i8)

define void@test_int_x86_avx512_mask_storeu_d_256(i8* %ptr1, i8* %ptr2, <8 x i32> %x1, i8 %x2) {
; CHECK-LABEL: test_int_x86_avx512_mask_storeu_d_256:
; CHECK:       ## BB#0:
; CHECK-NEXT:    kmovw %edx, %k1 ## encoding: [0xc5,0xf8,0x92,0xca]
; CHECK-NEXT:    vmovdqu32 %ymm0, (%rdi) {%k1} ## encoding: [0x62,0xf1,0x7e,0x29,0x7f,0x07]
; CHECK-NEXT:    vmovdqu32 %ymm0, (%rsi) ## encoding: [0x62,0xf1,0x7e,0x28,0x7f,0x06]
; CHECK-NEXT:    retq ## encoding: [0xc3]
  call void @llvm.x86.avx512.mask.storeu.d.256(i8* %ptr1, <8 x i32> %x1, i8 %x2)
  call void @llvm.x86.avx512.mask.storeu.d.256(i8* %ptr2, <8 x i32> %x1, i8 -1)
  ret void
}

declare void @llvm.x86.avx512.mask.store.q.128(i8*, <2 x i64>, i8)

define void@test_int_x86_avx512_mask_store_q_128(i8* %ptr1, i8* %ptr2, <2 x i64> %x1, i8 %x2) {
; CHECK-LABEL: test_int_x86_avx512_mask_store_q_128:
; CHECK:       ## BB#0:
; CHECK-NEXT:    kmovw %edx, %k1 ## encoding: [0xc5,0xf8,0x92,0xca]
; CHECK-NEXT:    vmovdqa64 %xmm0, (%rdi) {%k1} ## encoding: [0x62,0xf1,0xfd,0x09,0x7f,0x07]
; CHECK-NEXT:    vmovdqa64 %xmm0, (%rsi) ## encoding: [0x62,0xf1,0xfd,0x08,0x7f,0x06]
; CHECK-NEXT:    retq ## encoding: [0xc3]
  call void @llvm.x86.avx512.mask.store.q.128(i8* %ptr1, <2 x i64> %x1, i8 %x2)
  call void @llvm.x86.avx512.mask.store.q.128(i8* %ptr2, <2 x i64> %x1, i8 -1)
  ret void
}

declare void @llvm.x86.avx512.mask.store.q.256(i8*, <4 x i64>, i8)

define void@test_int_x86_avx512_mask_store_q_256(i8* %ptr1, i8* %ptr2, <4 x i64> %x1, i8 %x2) {
; CHECK-LABEL: test_int_x86_avx512_mask_store_q_256:
; CHECK:       ## BB#0:
; CHECK-NEXT:    kmovw %edx, %k1 ## encoding: [0xc5,0xf8,0x92,0xca]
; CHECK-NEXT:    vmovdqa64 %ymm0, (%rdi) {%k1} ## encoding: [0x62,0xf1,0xfd,0x29,0x7f,0x07]
; CHECK-NEXT:    vmovdqa64 %ymm0, (%rsi) ## encoding: [0x62,0xf1,0xfd,0x28,0x7f,0x06]
; CHECK-NEXT:    retq ## encoding: [0xc3]
  call void @llvm.x86.avx512.mask.store.q.256(i8* %ptr1, <4 x i64> %x1, i8 %x2)
  call void @llvm.x86.avx512.mask.store.q.256(i8* %ptr2, <4 x i64> %x1, i8 -1)
  ret void
}

declare void @llvm.x86.avx512.mask.store.d.128(i8*, <4 x i32>, i8)

define void@test_int_x86_avx512_mask_store_d_128(i8* %ptr1, i8* %ptr2, <4 x i32> %x1, i8 %x2) {
; CHECK-LABEL: test_int_x86_avx512_mask_store_d_128:
; CHECK:       ## BB#0:
; CHECK-NEXT:    kmovw %edx, %k1 ## encoding: [0xc5,0xf8,0x92,0xca]
; CHECK-NEXT:    vmovdqa32 %xmm0, (%rdi) {%k1} ## encoding: [0x62,0xf1,0x7d,0x09,0x7f,0x07]
; CHECK-NEXT:    vmovdqa32 %xmm0, (%rsi) ## encoding: [0x62,0xf1,0x7d,0x08,0x7f,0x06]
; CHECK-NEXT:    retq ## encoding: [0xc3]
  call void @llvm.x86.avx512.mask.store.d.128(i8* %ptr1, <4 x i32> %x1, i8 %x2)
  call void @llvm.x86.avx512.mask.store.d.128(i8* %ptr2, <4 x i32> %x1, i8 -1)
  ret void
}

declare void @llvm.x86.avx512.mask.store.d.256(i8*, <8 x i32>, i8)

define void@test_int_x86_avx512_mask_store_d_256(i8* %ptr1, i8* %ptr2, <8 x i32> %x1, i8 %x2) {
; CHECK-LABEL: test_int_x86_avx512_mask_store_d_256:
; CHECK:       ## BB#0:
; CHECK-NEXT:    kmovw %edx, %k1 ## encoding: [0xc5,0xf8,0x92,0xca]
; CHECK-NEXT:    vmovdqa32 %ymm0, (%rdi) {%k1} ## encoding: [0x62,0xf1,0x7d,0x29,0x7f,0x07]
; CHECK-NEXT:    vmovdqa32 %ymm0, (%rsi) ## encoding: [0x62,0xf1,0x7d,0x28,0x7f,0x06]
; CHECK-NEXT:    retq ## encoding: [0xc3]
  call void @llvm.x86.avx512.mask.store.d.256(i8* %ptr1, <8 x i32> %x1, i8 %x2)
  call void @llvm.x86.avx512.mask.store.d.256(i8* %ptr2, <8 x i32> %x1, i8 -1)
  ret void
}

define <8 x float> @test_mask_load_aligned_ps_256(<8 x float> %data, i8* %ptr, i8 %mask) {
; CHECK-LABEL: test_mask_load_aligned_ps_256:
; CHECK:       ## BB#0:
; CHECK-NEXT:    vmovaps (%rdi), %ymm0 ## encoding: [0x62,0xf1,0x7c,0x28,0x28,0x07]
; CHECK-NEXT:    kmovw %esi, %k1 ## encoding: [0xc5,0xf8,0x92,0xce]
; CHECK-NEXT:    vmovaps (%rdi), %ymm0 {%k1} ## encoding: [0x62,0xf1,0x7c,0x29,0x28,0x07]
; CHECK-NEXT:    vmovaps (%rdi), %ymm1 {%k1} {z} ## encoding: [0x62,0xf1,0x7c,0xa9,0x28,0x0f]
; CHECK-NEXT:    vaddps %ymm0, %ymm1, %ymm0 ## encoding: [0x62,0xf1,0x74,0x28,0x58,0xc0]
; CHECK-NEXT:    retq ## encoding: [0xc3]
  %res = call <8 x float> @llvm.x86.avx512.mask.load.ps.256(i8* %ptr, <8 x float> zeroinitializer, i8 -1)
  %res1 = call <8 x float> @llvm.x86.avx512.mask.load.ps.256(i8* %ptr, <8 x float> %res, i8 %mask)
  %res2 = call <8 x float> @llvm.x86.avx512.mask.load.ps.256(i8* %ptr, <8 x float> zeroinitializer, i8 %mask)
  %res4 = fadd <8 x float> %res2, %res1
  ret <8 x float> %res4
}

declare <8 x float> @llvm.x86.avx512.mask.load.ps.256(i8*, <8 x float>, i8)

define <8 x float> @test_mask_load_unaligned_ps_256(<8 x float> %data, i8* %ptr, i8 %mask) {
; CHECK-LABEL: test_mask_load_unaligned_ps_256:
; CHECK:       ## BB#0:
; CHECK-NEXT:    vmovups (%rdi), %ymm0 ## encoding: [0x62,0xf1,0x7c,0x28,0x10,0x07]
; CHECK-NEXT:    kmovw %esi, %k1 ## encoding: [0xc5,0xf8,0x92,0xce]
; CHECK-NEXT:    vmovups (%rdi), %ymm0 {%k1} ## encoding: [0x62,0xf1,0x7c,0x29,0x10,0x07]
; CHECK-NEXT:    vmovups (%rdi), %ymm1 {%k1} {z} ## encoding: [0x62,0xf1,0x7c,0xa9,0x10,0x0f]
; CHECK-NEXT:    vaddps %ymm0, %ymm1, %ymm0 ## encoding: [0x62,0xf1,0x74,0x28,0x58,0xc0]
; CHECK-NEXT:    retq ## encoding: [0xc3]
  %res = call <8 x float> @llvm.x86.avx512.mask.loadu.ps.256(i8* %ptr, <8 x float> zeroinitializer, i8 -1)
  %res1 = call <8 x float> @llvm.x86.avx512.mask.loadu.ps.256(i8* %ptr, <8 x float> %res, i8 %mask)
  %res2 = call <8 x float> @llvm.x86.avx512.mask.loadu.ps.256(i8* %ptr, <8 x float> zeroinitializer, i8 %mask)
  %res4 = fadd <8 x float> %res2, %res1
  ret <8 x float> %res4
}

declare <8 x float> @llvm.x86.avx512.mask.loadu.ps.256(i8*, <8 x float>, i8)

define <4 x double> @test_mask_load_aligned_pd_256(<4 x double> %data, i8* %ptr, i8 %mask) {
; CHECK-LABEL: test_mask_load_aligned_pd_256:
; CHECK:       ## BB#0:
; CHECK-NEXT:    vmovapd (%rdi), %ymm0 ## encoding: [0x62,0xf1,0xfd,0x28,0x28,0x07]
; CHECK-NEXT:    kmovw %esi, %k1 ## encoding: [0xc5,0xf8,0x92,0xce]
; CHECK-NEXT:    vmovapd (%rdi), %ymm0 {%k1} ## encoding: [0x62,0xf1,0xfd,0x29,0x28,0x07]
; CHECK-NEXT:    vmovapd (%rdi), %ymm1 {%k1} {z} ## encoding: [0x62,0xf1,0xfd,0xa9,0x28,0x0f]
; CHECK-NEXT:    vaddpd %ymm0, %ymm1, %ymm0 ## encoding: [0x62,0xf1,0xf5,0x28,0x58,0xc0]
; CHECK-NEXT:    retq ## encoding: [0xc3]
  %res = call <4 x double> @llvm.x86.avx512.mask.load.pd.256(i8* %ptr, <4 x double> zeroinitializer, i8 -1)
  %res1 = call <4 x double> @llvm.x86.avx512.mask.load.pd.256(i8* %ptr, <4 x double> %res, i8 %mask)
  %res2 = call <4 x double> @llvm.x86.avx512.mask.load.pd.256(i8* %ptr, <4 x double> zeroinitializer, i8 %mask)
  %res4 = fadd <4 x double> %res2, %res1
  ret <4 x double> %res4
}

declare <4 x double> @llvm.x86.avx512.mask.load.pd.256(i8*, <4 x double>, i8)

define <4 x double> @test_mask_load_unaligned_pd_256(<4 x double> %data, i8* %ptr, i8 %mask) {
; CHECK-LABEL: test_mask_load_unaligned_pd_256:
; CHECK:       ## BB#0:
; CHECK-NEXT:    vmovupd (%rdi), %ymm0 ## encoding: [0x62,0xf1,0xfd,0x28,0x10,0x07]
; CHECK-NEXT:    kmovw %esi, %k1 ## encoding: [0xc5,0xf8,0x92,0xce]
; CHECK-NEXT:    vmovupd (%rdi), %ymm0 {%k1} ## encoding: [0x62,0xf1,0xfd,0x29,0x10,0x07]
; CHECK-NEXT:    vmovupd (%rdi), %ymm1 {%k1} {z} ## encoding: [0x62,0xf1,0xfd,0xa9,0x10,0x0f]
; CHECK-NEXT:    vaddpd %ymm0, %ymm1, %ymm0 ## encoding: [0x62,0xf1,0xf5,0x28,0x58,0xc0]
; CHECK-NEXT:    retq ## encoding: [0xc3]
  %res = call <4 x double> @llvm.x86.avx512.mask.loadu.pd.256(i8* %ptr, <4 x double> zeroinitializer, i8 -1)
  %res1 = call <4 x double> @llvm.x86.avx512.mask.loadu.pd.256(i8* %ptr, <4 x double> %res, i8 %mask)
  %res2 = call <4 x double> @llvm.x86.avx512.mask.loadu.pd.256(i8* %ptr, <4 x double> zeroinitializer, i8 %mask)
  %res4 = fadd <4 x double> %res2, %res1
  ret <4 x double> %res4
}

declare <4 x double> @llvm.x86.avx512.mask.loadu.pd.256(i8*, <4 x double>, i8)

define <4 x float> @test_mask_load_aligned_ps_128(<4 x float> %data, i8* %ptr, i8 %mask) {
; CHECK-LABEL: test_mask_load_aligned_ps_128:
; CHECK:       ## BB#0:
; CHECK-NEXT:    vmovaps (%rdi), %xmm0 ## encoding: [0x62,0xf1,0x7c,0x08,0x28,0x07]
; CHECK-NEXT:    kmovw %esi, %k1 ## encoding: [0xc5,0xf8,0x92,0xce]
; CHECK-NEXT:    vmovaps (%rdi), %xmm0 {%k1} ## encoding: [0x62,0xf1,0x7c,0x09,0x28,0x07]
; CHECK-NEXT:    vmovaps (%rdi), %xmm1 {%k1} {z} ## encoding: [0x62,0xf1,0x7c,0x89,0x28,0x0f]
; CHECK-NEXT:    vaddps %xmm0, %xmm1, %xmm0 ## encoding: [0x62,0xf1,0x74,0x08,0x58,0xc0]
; CHECK-NEXT:    retq ## encoding: [0xc3]
  %res = call <4 x float> @llvm.x86.avx512.mask.load.ps.128(i8* %ptr, <4 x float> zeroinitializer, i8 -1)
  %res1 = call <4 x float> @llvm.x86.avx512.mask.load.ps.128(i8* %ptr, <4 x float> %res, i8 %mask)
  %res2 = call <4 x float> @llvm.x86.avx512.mask.load.ps.128(i8* %ptr, <4 x float> zeroinitializer, i8 %mask)
  %res4 = fadd <4 x float> %res2, %res1
  ret <4 x float> %res4
}

declare <4 x float> @llvm.x86.avx512.mask.load.ps.128(i8*, <4 x float>, i8)

define <4 x float> @test_mask_load_unaligned_ps_128(<4 x float> %data, i8* %ptr, i8 %mask) {
; CHECK-LABEL: test_mask_load_unaligned_ps_128:
; CHECK:       ## BB#0:
; CHECK-NEXT:    vmovups (%rdi), %xmm0 ## encoding: [0x62,0xf1,0x7c,0x08,0x10,0x07]
; CHECK-NEXT:    kmovw %esi, %k1 ## encoding: [0xc5,0xf8,0x92,0xce]
; CHECK-NEXT:    vmovups (%rdi), %xmm0 {%k1} ## encoding: [0x62,0xf1,0x7c,0x09,0x10,0x07]
; CHECK-NEXT:    vmovups (%rdi), %xmm1 {%k1} {z} ## encoding: [0x62,0xf1,0x7c,0x89,0x10,0x0f]
; CHECK-NEXT:    vaddps %xmm0, %xmm1, %xmm0 ## encoding: [0x62,0xf1,0x74,0x08,0x58,0xc0]
; CHECK-NEXT:    retq ## encoding: [0xc3]
  %res = call <4 x float> @llvm.x86.avx512.mask.loadu.ps.128(i8* %ptr, <4 x float> zeroinitializer, i8 -1)
  %res1 = call <4 x float> @llvm.x86.avx512.mask.loadu.ps.128(i8* %ptr, <4 x float> %res, i8 %mask)
  %res2 = call <4 x float> @llvm.x86.avx512.mask.loadu.ps.128(i8* %ptr, <4 x float> zeroinitializer, i8 %mask)
  %res4 = fadd <4 x float> %res2, %res1
  ret <4 x float> %res4
}

declare <4 x float> @llvm.x86.avx512.mask.loadu.ps.128(i8*, <4 x float>, i8)

define <2 x double> @test_mask_load_aligned_pd_128(<2 x double> %data, i8* %ptr, i8 %mask) {
; CHECK-LABEL: test_mask_load_aligned_pd_128:
; CHECK:       ## BB#0:
; CHECK-NEXT:    vmovapd (%rdi), %xmm0 ## encoding: [0x62,0xf1,0xfd,0x08,0x28,0x07]
; CHECK-NEXT:    kmovw %esi, %k1 ## encoding: [0xc5,0xf8,0x92,0xce]
; CHECK-NEXT:    vmovapd (%rdi), %xmm0 {%k1} ## encoding: [0x62,0xf1,0xfd,0x09,0x28,0x07]
; CHECK-NEXT:    vmovapd (%rdi), %xmm1 {%k1} {z} ## encoding: [0x62,0xf1,0xfd,0x89,0x28,0x0f]
; CHECK-NEXT:    vaddpd %xmm0, %xmm1, %xmm0 ## encoding: [0x62,0xf1,0xf5,0x08,0x58,0xc0]
; CHECK-NEXT:    retq ## encoding: [0xc3]
  %res = call <2 x double> @llvm.x86.avx512.mask.load.pd.128(i8* %ptr, <2 x double> zeroinitializer, i8 -1)
  %res1 = call <2 x double> @llvm.x86.avx512.mask.load.pd.128(i8* %ptr, <2 x double> %res, i8 %mask)
  %res2 = call <2 x double> @llvm.x86.avx512.mask.load.pd.128(i8* %ptr, <2 x double> zeroinitializer, i8 %mask)
  %res4 = fadd <2 x double> %res2, %res1
  ret <2 x double> %res4
}

declare <2 x double> @llvm.x86.avx512.mask.load.pd.128(i8*, <2 x double>, i8)

define <2 x double> @test_mask_load_unaligned_pd_128(<2 x double> %data, i8* %ptr, i8 %mask) {
; CHECK-LABEL: test_mask_load_unaligned_pd_128:
; CHECK:       ## BB#0:
; CHECK-NEXT:    vmovupd (%rdi), %xmm0 ## encoding: [0x62,0xf1,0xfd,0x08,0x10,0x07]
; CHECK-NEXT:    kmovw %esi, %k1 ## encoding: [0xc5,0xf8,0x92,0xce]
; CHECK-NEXT:    vmovupd (%rdi), %xmm0 {%k1} ## encoding: [0x62,0xf1,0xfd,0x09,0x10,0x07]
; CHECK-NEXT:    vmovupd (%rdi), %xmm1 {%k1} {z} ## encoding: [0x62,0xf1,0xfd,0x89,0x10,0x0f]
; CHECK-NEXT:    vaddpd %xmm0, %xmm1, %xmm0 ## encoding: [0x62,0xf1,0xf5,0x08,0x58,0xc0]
; CHECK-NEXT:    retq ## encoding: [0xc3]
  %res = call <2 x double> @llvm.x86.avx512.mask.loadu.pd.128(i8* %ptr, <2 x double> zeroinitializer, i8 -1)
  %res1 = call <2 x double> @llvm.x86.avx512.mask.loadu.pd.128(i8* %ptr, <2 x double> %res, i8 %mask)
  %res2 = call <2 x double> @llvm.x86.avx512.mask.loadu.pd.128(i8* %ptr, <2 x double> zeroinitializer, i8 %mask)
  %res4 = fadd <2 x double> %res2, %res1
  ret <2 x double> %res4
}

declare <2 x double> @llvm.x86.avx512.mask.loadu.pd.128(i8*, <2 x double>, i8)

declare <4 x i32> @llvm.x86.avx512.mask.loadu.d.128(i8*, <4 x i32>, i8)

define <4 x i32> @test_mask_load_unaligned_d_128(i8* %ptr, i8* %ptr2, <4 x i32> %data, i8 %mask) {
; CHECK-LABEL: test_mask_load_unaligned_d_128:
; CHECK:       ## BB#0:
; CHECK-NEXT:    vmovdqu32 (%rdi), %xmm0 ## encoding: [0x62,0xf1,0x7e,0x08,0x6f,0x07]
; CHECK-NEXT:    kmovw %edx, %k1 ## encoding: [0xc5,0xf8,0x92,0xca]
; CHECK-NEXT:    vmovdqu32 (%rsi), %xmm0 {%k1} ## encoding: [0x62,0xf1,0x7e,0x09,0x6f,0x06]
; CHECK-NEXT:    vmovdqu32 (%rdi), %xmm1 {%k1} {z} ## encoding: [0x62,0xf1,0x7e,0x89,0x6f,0x0f]
; CHECK-NEXT:    vpaddd %xmm0, %xmm1, %xmm0 ## encoding: [0x62,0xf1,0x75,0x08,0xfe,0xc0]
; CHECK-NEXT:    retq ## encoding: [0xc3]
  %res = call <4 x i32> @llvm.x86.avx512.mask.loadu.d.128(i8* %ptr, <4 x i32> zeroinitializer, i8 -1)
  %res1 = call <4 x i32> @llvm.x86.avx512.mask.loadu.d.128(i8* %ptr2, <4 x i32> %res, i8 %mask)
  %res2 = call <4 x i32> @llvm.x86.avx512.mask.loadu.d.128(i8* %ptr, <4 x i32> zeroinitializer, i8 %mask)
  %res4 = add <4 x i32> %res2, %res1
  ret <4 x i32> %res4
}

declare <8 x i32> @llvm.x86.avx512.mask.loadu.d.256(i8*, <8 x i32>, i8)

define <8 x i32> @test_mask_load_unaligned_d_256(i8* %ptr, i8* %ptr2, <8 x i32> %data, i8 %mask) {
; CHECK-LABEL: test_mask_load_unaligned_d_256:
; CHECK:       ## BB#0:
; CHECK-NEXT:    vmovdqu32 (%rdi), %ymm0 ## encoding: [0x62,0xf1,0x7e,0x28,0x6f,0x07]
; CHECK-NEXT:    kmovw %edx, %k1 ## encoding: [0xc5,0xf8,0x92,0xca]
; CHECK-NEXT:    vmovdqu32 (%rsi), %ymm0 {%k1} ## encoding: [0x62,0xf1,0x7e,0x29,0x6f,0x06]
; CHECK-NEXT:    vmovdqu32 (%rdi), %ymm1 {%k1} {z} ## encoding: [0x62,0xf1,0x7e,0xa9,0x6f,0x0f]
; CHECK-NEXT:    vpaddd %ymm0, %ymm1, %ymm0 ## encoding: [0x62,0xf1,0x75,0x28,0xfe,0xc0]
; CHECK-NEXT:    retq ## encoding: [0xc3]
  %res = call <8 x i32> @llvm.x86.avx512.mask.loadu.d.256(i8* %ptr, <8 x i32> zeroinitializer, i8 -1)
  %res1 = call <8 x i32> @llvm.x86.avx512.mask.loadu.d.256(i8* %ptr2, <8 x i32> %res, i8 %mask)
  %res2 = call <8 x i32> @llvm.x86.avx512.mask.loadu.d.256(i8* %ptr, <8 x i32> zeroinitializer, i8 %mask)
  %res4 = add <8 x i32> %res2, %res1
  ret <8 x i32> %res4
}

declare <2 x i64> @llvm.x86.avx512.mask.loadu.q.128(i8*, <2 x i64>, i8)

define <2 x i64> @test_mask_load_unaligned_q_128(i8* %ptr, i8* %ptr2, <2 x i64> %data, i8 %mask) {
; CHECK-LABEL: test_mask_load_unaligned_q_128:
; CHECK:       ## BB#0:
; CHECK-NEXT:    vmovdqu64 (%rdi), %xmm0 ## encoding: [0x62,0xf1,0xfe,0x08,0x6f,0x07]
; CHECK-NEXT:    kmovw %edx, %k1 ## encoding: [0xc5,0xf8,0x92,0xca]
; CHECK-NEXT:    vmovdqu64 (%rsi), %xmm0 {%k1} ## encoding: [0x62,0xf1,0xfe,0x09,0x6f,0x06]
; CHECK-NEXT:    vmovdqu64 (%rdi), %xmm1 {%k1} {z} ## encoding: [0x62,0xf1,0xfe,0x89,0x6f,0x0f]
; CHECK-NEXT:    vpaddq %xmm0, %xmm1, %xmm0 ## encoding: [0x62,0xf1,0xf5,0x08,0xd4,0xc0]
; CHECK-NEXT:    retq ## encoding: [0xc3]
  %res = call <2 x i64> @llvm.x86.avx512.mask.loadu.q.128(i8* %ptr, <2 x i64> zeroinitializer, i8 -1)
  %res1 = call <2 x i64> @llvm.x86.avx512.mask.loadu.q.128(i8* %ptr2, <2 x i64> %res, i8 %mask)
  %res2 = call <2 x i64> @llvm.x86.avx512.mask.loadu.q.128(i8* %ptr, <2 x i64> zeroinitializer, i8 %mask)
  %res4 = add <2 x i64> %res2, %res1
  ret <2 x i64> %res4
}

declare <4 x i64> @llvm.x86.avx512.mask.loadu.q.256(i8*, <4 x i64>, i8)

define <4 x i64> @test_mask_load_unaligned_q_256(i8* %ptr, i8* %ptr2, <4 x i64> %data, i8 %mask) {
; CHECK-LABEL: test_mask_load_unaligned_q_256:
; CHECK:       ## BB#0:
; CHECK-NEXT:    vmovdqu64 (%rdi), %ymm0 ## encoding: [0x62,0xf1,0xfe,0x28,0x6f,0x07]
; CHECK-NEXT:    kmovw %edx, %k1 ## encoding: [0xc5,0xf8,0x92,0xca]
; CHECK-NEXT:    vmovdqu64 (%rsi), %ymm0 {%k1} ## encoding: [0x62,0xf1,0xfe,0x29,0x6f,0x06]
; CHECK-NEXT:    vmovdqu64 (%rdi), %ymm1 {%k1} {z} ## encoding: [0x62,0xf1,0xfe,0xa9,0x6f,0x0f]
; CHECK-NEXT:    vpaddq %ymm0, %ymm1, %ymm0 ## encoding: [0x62,0xf1,0xf5,0x28,0xd4,0xc0]
; CHECK-NEXT:    retq ## encoding: [0xc3]
  %res = call <4 x i64> @llvm.x86.avx512.mask.loadu.q.256(i8* %ptr, <4 x i64> zeroinitializer, i8 -1)
  %res1 = call <4 x i64> @llvm.x86.avx512.mask.loadu.q.256(i8* %ptr2, <4 x i64> %res, i8 %mask)
  %res2 = call <4 x i64> @llvm.x86.avx512.mask.loadu.q.256(i8* %ptr, <4 x i64> zeroinitializer, i8 %mask)
  %res4 = add <4 x i64> %res2, %res1
  ret <4 x i64> %res4
}

declare <4 x i32> @llvm.x86.avx512.mask.load.d.128(i8*, <4 x i32>, i8)

define <4 x i32> @test_mask_load_aligned_d_128(<4 x i32> %data, i8* %ptr, i8 %mask) {
; CHECK-LABEL: test_mask_load_aligned_d_128:
; CHECK:       ## BB#0:
; CHECK-NEXT:    vmovdqa32 (%rdi), %xmm0 ## encoding: [0x62,0xf1,0x7d,0x08,0x6f,0x07]
; CHECK-NEXT:    kmovw %esi, %k1 ## encoding: [0xc5,0xf8,0x92,0xce]
; CHECK-NEXT:    vmovdqa32 (%rdi), %xmm0 {%k1} ## encoding: [0x62,0xf1,0x7d,0x09,0x6f,0x07]
; CHECK-NEXT:    vmovdqa32 (%rdi), %xmm1 {%k1} {z} ## encoding: [0x62,0xf1,0x7d,0x89,0x6f,0x0f]
; CHECK-NEXT:    vpaddd %xmm0, %xmm1, %xmm0 ## encoding: [0x62,0xf1,0x75,0x08,0xfe,0xc0]
; CHECK-NEXT:    retq ## encoding: [0xc3]
  %res = call <4 x i32> @llvm.x86.avx512.mask.load.d.128(i8* %ptr, <4 x i32> zeroinitializer, i8 -1)
  %res1 = call <4 x i32> @llvm.x86.avx512.mask.load.d.128(i8* %ptr, <4 x i32> %res, i8 %mask)
  %res2 = call <4 x i32> @llvm.x86.avx512.mask.load.d.128(i8* %ptr, <4 x i32> zeroinitializer, i8 %mask)
  %res4 = add <4 x i32> %res2, %res1
  ret <4 x i32> %res4
}

declare <8 x i32> @llvm.x86.avx512.mask.load.d.256(i8*, <8 x i32>, i8)

define <8 x i32> @test_mask_load_aligned_d_256(<8 x i32> %data, i8* %ptr, i8 %mask) {
; CHECK-LABEL: test_mask_load_aligned_d_256:
; CHECK:       ## BB#0:
; CHECK-NEXT:    vmovdqa32 (%rdi), %ymm0 ## encoding: [0x62,0xf1,0x7d,0x28,0x6f,0x07]
; CHECK-NEXT:    kmovw %esi, %k1 ## encoding: [0xc5,0xf8,0x92,0xce]
; CHECK-NEXT:    vmovdqa32 (%rdi), %ymm0 {%k1} ## encoding: [0x62,0xf1,0x7d,0x29,0x6f,0x07]
; CHECK-NEXT:    vmovdqa32 (%rdi), %ymm1 {%k1} {z} ## encoding: [0x62,0xf1,0x7d,0xa9,0x6f,0x0f]
; CHECK-NEXT:    vpaddd %ymm0, %ymm1, %ymm0 ## encoding: [0x62,0xf1,0x75,0x28,0xfe,0xc0]
; CHECK-NEXT:    retq ## encoding: [0xc3]
  %res = call <8 x i32> @llvm.x86.avx512.mask.load.d.256(i8* %ptr, <8 x i32> zeroinitializer, i8 -1)
  %res1 = call <8 x i32> @llvm.x86.avx512.mask.load.d.256(i8* %ptr, <8 x i32> %res, i8 %mask)
  %res2 = call <8 x i32> @llvm.x86.avx512.mask.load.d.256(i8* %ptr, <8 x i32> zeroinitializer, i8 %mask)
  %res4 = add <8 x i32> %res2, %res1
  ret <8 x i32> %res4
}

declare <2 x i64> @llvm.x86.avx512.mask.load.q.128(i8*, <2 x i64>, i8)

define <2 x i64> @test_mask_load_aligned_q_128(<2 x i64> %data, i8* %ptr, i8 %mask) {
; CHECK-LABEL: test_mask_load_aligned_q_128:
; CHECK:       ## BB#0:
; CHECK-NEXT:    vmovdqa64 (%rdi), %xmm0 ## encoding: [0x62,0xf1,0xfd,0x08,0x6f,0x07]
; CHECK-NEXT:    kmovw %esi, %k1 ## encoding: [0xc5,0xf8,0x92,0xce]
; CHECK-NEXT:    vmovdqa64 (%rdi), %xmm0 {%k1} ## encoding: [0x62,0xf1,0xfd,0x09,0x6f,0x07]
; CHECK-NEXT:    vmovdqa64 (%rdi), %xmm1 {%k1} {z} ## encoding: [0x62,0xf1,0xfd,0x89,0x6f,0x0f]
; CHECK-NEXT:    vpaddq %xmm0, %xmm1, %xmm0 ## encoding: [0x62,0xf1,0xf5,0x08,0xd4,0xc0]
; CHECK-NEXT:    retq ## encoding: [0xc3]
  %res = call <2 x i64> @llvm.x86.avx512.mask.load.q.128(i8* %ptr, <2 x i64> zeroinitializer, i8 -1)
  %res1 = call <2 x i64> @llvm.x86.avx512.mask.load.q.128(i8* %ptr, <2 x i64> %res, i8 %mask)
  %res2 = call <2 x i64> @llvm.x86.avx512.mask.load.q.128(i8* %ptr, <2 x i64> zeroinitializer, i8 %mask)
  %res4 = add <2 x i64> %res2, %res1
  ret <2 x i64> %res4
}

declare <4 x i64> @llvm.x86.avx512.mask.load.q.256(i8*, <4 x i64>, i8)

define <4 x i64> @test_mask_load_aligned_q_256(<4 x i64> %data, i8* %ptr, i8 %mask) {
; CHECK-LABEL: test_mask_load_aligned_q_256:
; CHECK:       ## BB#0:
; CHECK-NEXT:    vmovdqa64 (%rdi), %ymm0 ## encoding: [0x62,0xf1,0xfd,0x28,0x6f,0x07]
; CHECK-NEXT:    kmovw %esi, %k1 ## encoding: [0xc5,0xf8,0x92,0xce]
; CHECK-NEXT:    vmovdqa64 (%rdi), %ymm0 {%k1} ## encoding: [0x62,0xf1,0xfd,0x29,0x6f,0x07]
; CHECK-NEXT:    vmovdqa64 (%rdi), %ymm1 {%k1} {z} ## encoding: [0x62,0xf1,0xfd,0xa9,0x6f,0x0f]
; CHECK-NEXT:    vpaddq %ymm0, %ymm1, %ymm0 ## encoding: [0x62,0xf1,0xf5,0x28,0xd4,0xc0]
; CHECK-NEXT:    retq ## encoding: [0xc3]
  %res = call <4 x i64> @llvm.x86.avx512.mask.load.q.256(i8* %ptr, <4 x i64> zeroinitializer, i8 -1)
  %res1 = call <4 x i64> @llvm.x86.avx512.mask.load.q.256(i8* %ptr, <4 x i64> %res, i8 %mask)
  %res2 = call <4 x i64> @llvm.x86.avx512.mask.load.q.256(i8* %ptr, <4 x i64> zeroinitializer, i8 %mask)
  %res4 = add <4 x i64> %res2, %res1
  ret <4 x i64> %res4
}
