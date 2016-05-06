; NOTE: Assertions have been autogenerated by utils/update_test_checks.py
; RUN: opt < %s -mtriple=x86_64-unknown -basicaa -slp-vectorizer -S | FileCheck %s --check-prefix=CHECK --check-prefix=SSE
; RUN: opt < %s -mtriple=x86_64-unknown -mcpu=corei7-avx -basicaa -slp-vectorizer -S | FileCheck %s --check-prefix=CHECK --check-prefix=AVX
; RUN: opt < %s -mtriple=x86_64-unknown -mcpu=core-avx2 -basicaa -slp-vectorizer -S | FileCheck %s --check-prefix=CHECK --check-prefix=AVX

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

@src64 = common global [4 x i64] zeroinitializer, align 32
@dst64 = common global [4 x i64] zeroinitializer, align 32
@src32 = common global [8 x i32] zeroinitializer, align 32
@dst32 = common global [8 x i32] zeroinitializer, align 32
@src16 = common global [16 x i16] zeroinitializer, align 32
@dst16 = common global [16 x i16] zeroinitializer, align 32

declare i64 @llvm.bswap.i64(i64)
declare i32 @llvm.bswap.i32(i32)
declare i16 @llvm.bswap.i16(i16)

define void @bswap_2i64() #0 {
; CHECK-LABEL: @bswap_2i64(
; CHECK-NEXT:    [[LD0:%.*]] = load i64, i64* getelementptr inbounds ([4 x i64], [4 x i64]* @src64, i32 0, i64 0), align 8
; CHECK-NEXT:    [[LD1:%.*]] = load i64, i64* getelementptr inbounds ([4 x i64], [4 x i64]* @src64, i32 0, i64 1), align 8
; CHECK-NEXT:    [[BSWAP0:%.*]] = call i64 @llvm.bswap.i64(i64 [[LD0]])
; CHECK-NEXT:    [[BSWAP1:%.*]] = call i64 @llvm.bswap.i64(i64 [[LD1]])
; CHECK-NEXT:    store i64 [[BSWAP0]], i64* getelementptr inbounds ([4 x i64], [4 x i64]* @dst64, i32 0, i64 0), align 8
; CHECK-NEXT:    store i64 [[BSWAP1]], i64* getelementptr inbounds ([4 x i64], [4 x i64]* @dst64, i32 0, i64 1), align 8
; CHECK-NEXT:    ret void
;
  %ld0 = load i64, i64* getelementptr inbounds ([4 x i64], [4 x i64]* @src64, i32 0, i64 0), align 8
  %ld1 = load i64, i64* getelementptr inbounds ([4 x i64], [4 x i64]* @src64, i32 0, i64 1), align 8
  %bswap0 = call i64 @llvm.bswap.i64(i64 %ld0)
  %bswap1 = call i64 @llvm.bswap.i64(i64 %ld1)
  store i64 %bswap0, i64* getelementptr inbounds ([4 x i64], [4 x i64]* @dst64, i32 0, i64 0), align 8
  store i64 %bswap1, i64* getelementptr inbounds ([4 x i64], [4 x i64]* @dst64, i32 0, i64 1), align 8
  ret void
}

define void @bswap_4i64() #0 {
; CHECK-LABEL: @bswap_4i64(
; CHECK-NEXT:    [[LD0:%.*]] = load i64, i64* getelementptr inbounds ([4 x i64], [4 x i64]* @src64, i64 0, i64 0), align 4
; CHECK-NEXT:    [[LD1:%.*]] = load i64, i64* getelementptr inbounds ([4 x i64], [4 x i64]* @src64, i64 0, i64 1), align 4
; CHECK-NEXT:    [[LD2:%.*]] = load i64, i64* getelementptr inbounds ([4 x i64], [4 x i64]* @src64, i64 0, i64 2), align 4
; CHECK-NEXT:    [[LD3:%.*]] = load i64, i64* getelementptr inbounds ([4 x i64], [4 x i64]* @src64, i64 0, i64 3), align 4
; CHECK-NEXT:    [[BSWAP0:%.*]] = call i64 @llvm.bswap.i64(i64 [[LD0]])
; CHECK-NEXT:    [[BSWAP1:%.*]] = call i64 @llvm.bswap.i64(i64 [[LD1]])
; CHECK-NEXT:    [[BSWAP2:%.*]] = call i64 @llvm.bswap.i64(i64 [[LD2]])
; CHECK-NEXT:    [[BSWAP3:%.*]] = call i64 @llvm.bswap.i64(i64 [[LD3]])
; CHECK-NEXT:    store i64 [[BSWAP0]], i64* getelementptr inbounds ([4 x i64], [4 x i64]* @dst64, i64 0, i64 0), align 4
; CHECK-NEXT:    store i64 [[BSWAP1]], i64* getelementptr inbounds ([4 x i64], [4 x i64]* @dst64, i64 0, i64 1), align 4
; CHECK-NEXT:    store i64 [[BSWAP2]], i64* getelementptr inbounds ([4 x i64], [4 x i64]* @dst64, i64 0, i64 2), align 4
; CHECK-NEXT:    store i64 [[BSWAP3]], i64* getelementptr inbounds ([4 x i64], [4 x i64]* @dst64, i64 0, i64 3), align 4
; CHECK-NEXT:    ret void
;
  %ld0 = load i64, i64* getelementptr inbounds ([4 x i64], [4 x i64]* @src64, i64 0, i64 0), align 4
  %ld1 = load i64, i64* getelementptr inbounds ([4 x i64], [4 x i64]* @src64, i64 0, i64 1), align 4
  %ld2 = load i64, i64* getelementptr inbounds ([4 x i64], [4 x i64]* @src64, i64 0, i64 2), align 4
  %ld3 = load i64, i64* getelementptr inbounds ([4 x i64], [4 x i64]* @src64, i64 0, i64 3), align 4
  %bswap0 = call i64 @llvm.bswap.i64(i64 %ld0)
  %bswap1 = call i64 @llvm.bswap.i64(i64 %ld1)
  %bswap2 = call i64 @llvm.bswap.i64(i64 %ld2)
  %bswap3 = call i64 @llvm.bswap.i64(i64 %ld3)
  store i64 %bswap0, i64* getelementptr inbounds ([4 x i64], [4 x i64]* @dst64, i64 0, i64 0), align 4
  store i64 %bswap1, i64* getelementptr inbounds ([4 x i64], [4 x i64]* @dst64, i64 0, i64 1), align 4
  store i64 %bswap2, i64* getelementptr inbounds ([4 x i64], [4 x i64]* @dst64, i64 0, i64 2), align 4
  store i64 %bswap3, i64* getelementptr inbounds ([4 x i64], [4 x i64]* @dst64, i64 0, i64 3), align 4
  ret void
}

define void @bswap_4i32() #0 {
; CHECK-LABEL: @bswap_4i32(
; CHECK-NEXT:    [[LD0:%.*]] = load i32, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @src32, i32 0, i64 0), align 4
; CHECK-NEXT:    [[LD1:%.*]] = load i32, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @src32, i32 0, i64 1), align 4
; CHECK-NEXT:    [[LD2:%.*]] = load i32, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @src32, i32 0, i64 2), align 4
; CHECK-NEXT:    [[LD3:%.*]] = load i32, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @src32, i32 0, i64 3), align 4
; CHECK-NEXT:    [[BSWAP0:%.*]] = call i32 @llvm.bswap.i32(i32 [[LD0]])
; CHECK-NEXT:    [[BSWAP1:%.*]] = call i32 @llvm.bswap.i32(i32 [[LD1]])
; CHECK-NEXT:    [[BSWAP2:%.*]] = call i32 @llvm.bswap.i32(i32 [[LD2]])
; CHECK-NEXT:    [[BSWAP3:%.*]] = call i32 @llvm.bswap.i32(i32 [[LD3]])
; CHECK-NEXT:    store i32 [[BSWAP0]], i32* getelementptr inbounds ([8 x i32], [8 x i32]* @dst32, i32 0, i64 0), align 4
; CHECK-NEXT:    store i32 [[BSWAP1]], i32* getelementptr inbounds ([8 x i32], [8 x i32]* @dst32, i32 0, i64 1), align 4
; CHECK-NEXT:    store i32 [[BSWAP2]], i32* getelementptr inbounds ([8 x i32], [8 x i32]* @dst32, i32 0, i64 2), align 4
; CHECK-NEXT:    store i32 [[BSWAP3]], i32* getelementptr inbounds ([8 x i32], [8 x i32]* @dst32, i32 0, i64 3), align 4
; CHECK-NEXT:    ret void
;
  %ld0 = load i32, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @src32, i32 0, i64 0), align 4
  %ld1 = load i32, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @src32, i32 0, i64 1), align 4
  %ld2 = load i32, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @src32, i32 0, i64 2), align 4
  %ld3 = load i32, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @src32, i32 0, i64 3), align 4
  %bswap0 = call i32 @llvm.bswap.i32(i32 %ld0)
  %bswap1 = call i32 @llvm.bswap.i32(i32 %ld1)
  %bswap2 = call i32 @llvm.bswap.i32(i32 %ld2)
  %bswap3 = call i32 @llvm.bswap.i32(i32 %ld3)
  store i32 %bswap0, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @dst32, i32 0, i64 0), align 4
  store i32 %bswap1, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @dst32, i32 0, i64 1), align 4
  store i32 %bswap2, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @dst32, i32 0, i64 2), align 4
  store i32 %bswap3, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @dst32, i32 0, i64 3), align 4
  ret void
}

define void @bswap_8i32() #0 {
; CHECK-LABEL: @bswap_8i32(
; CHECK-NEXT:    [[LD0:%.*]] = load i32, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @src32, i32 0, i64 0), align 2
; CHECK-NEXT:    [[LD1:%.*]] = load i32, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @src32, i32 0, i64 1), align 2
; CHECK-NEXT:    [[LD2:%.*]] = load i32, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @src32, i32 0, i64 2), align 2
; CHECK-NEXT:    [[LD3:%.*]] = load i32, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @src32, i32 0, i64 3), align 2
; CHECK-NEXT:    [[LD4:%.*]] = load i32, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @src32, i32 0, i64 4), align 2
; CHECK-NEXT:    [[LD5:%.*]] = load i32, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @src32, i32 0, i64 5), align 2
; CHECK-NEXT:    [[LD6:%.*]] = load i32, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @src32, i32 0, i64 6), align 2
; CHECK-NEXT:    [[LD7:%.*]] = load i32, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @src32, i32 0, i64 7), align 2
; CHECK-NEXT:    [[BSWAP0:%.*]] = call i32 @llvm.bswap.i32(i32 [[LD0]])
; CHECK-NEXT:    [[BSWAP1:%.*]] = call i32 @llvm.bswap.i32(i32 [[LD1]])
; CHECK-NEXT:    [[BSWAP2:%.*]] = call i32 @llvm.bswap.i32(i32 [[LD2]])
; CHECK-NEXT:    [[BSWAP3:%.*]] = call i32 @llvm.bswap.i32(i32 [[LD3]])
; CHECK-NEXT:    [[BSWAP4:%.*]] = call i32 @llvm.bswap.i32(i32 [[LD4]])
; CHECK-NEXT:    [[BSWAP5:%.*]] = call i32 @llvm.bswap.i32(i32 [[LD5]])
; CHECK-NEXT:    [[BSWAP6:%.*]] = call i32 @llvm.bswap.i32(i32 [[LD6]])
; CHECK-NEXT:    [[BSWAP7:%.*]] = call i32 @llvm.bswap.i32(i32 [[LD7]])
; CHECK-NEXT:    store i32 [[BSWAP0]], i32* getelementptr inbounds ([8 x i32], [8 x i32]* @dst32, i32 0, i64 0), align 2
; CHECK-NEXT:    store i32 [[BSWAP1]], i32* getelementptr inbounds ([8 x i32], [8 x i32]* @dst32, i32 0, i64 1), align 2
; CHECK-NEXT:    store i32 [[BSWAP2]], i32* getelementptr inbounds ([8 x i32], [8 x i32]* @dst32, i32 0, i64 2), align 2
; CHECK-NEXT:    store i32 [[BSWAP3]], i32* getelementptr inbounds ([8 x i32], [8 x i32]* @dst32, i32 0, i64 3), align 2
; CHECK-NEXT:    store i32 [[BSWAP4]], i32* getelementptr inbounds ([8 x i32], [8 x i32]* @dst32, i32 0, i64 4), align 2
; CHECK-NEXT:    store i32 [[BSWAP5]], i32* getelementptr inbounds ([8 x i32], [8 x i32]* @dst32, i32 0, i64 5), align 2
; CHECK-NEXT:    store i32 [[BSWAP6]], i32* getelementptr inbounds ([8 x i32], [8 x i32]* @dst32, i32 0, i64 6), align 2
; CHECK-NEXT:    store i32 [[BSWAP7]], i32* getelementptr inbounds ([8 x i32], [8 x i32]* @dst32, i32 0, i64 7), align 2
; CHECK-NEXT:    ret void
;
  %ld0 = load i32, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @src32, i32 0, i64 0), align 2
  %ld1 = load i32, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @src32, i32 0, i64 1), align 2
  %ld2 = load i32, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @src32, i32 0, i64 2), align 2
  %ld3 = load i32, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @src32, i32 0, i64 3), align 2
  %ld4 = load i32, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @src32, i32 0, i64 4), align 2
  %ld5 = load i32, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @src32, i32 0, i64 5), align 2
  %ld6 = load i32, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @src32, i32 0, i64 6), align 2
  %ld7 = load i32, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @src32, i32 0, i64 7), align 2
  %bswap0 = call i32 @llvm.bswap.i32(i32 %ld0)
  %bswap1 = call i32 @llvm.bswap.i32(i32 %ld1)
  %bswap2 = call i32 @llvm.bswap.i32(i32 %ld2)
  %bswap3 = call i32 @llvm.bswap.i32(i32 %ld3)
  %bswap4 = call i32 @llvm.bswap.i32(i32 %ld4)
  %bswap5 = call i32 @llvm.bswap.i32(i32 %ld5)
  %bswap6 = call i32 @llvm.bswap.i32(i32 %ld6)
  %bswap7 = call i32 @llvm.bswap.i32(i32 %ld7)
  store i32 %bswap0, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @dst32, i32 0, i64 0), align 2
  store i32 %bswap1, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @dst32, i32 0, i64 1), align 2
  store i32 %bswap2, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @dst32, i32 0, i64 2), align 2
  store i32 %bswap3, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @dst32, i32 0, i64 3), align 2
  store i32 %bswap4, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @dst32, i32 0, i64 4), align 2
  store i32 %bswap5, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @dst32, i32 0, i64 5), align 2
  store i32 %bswap6, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @dst32, i32 0, i64 6), align 2
  store i32 %bswap7, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @dst32, i32 0, i64 7), align 2
  ret void
}

define void @bswap_8i16() #0 {
; CHECK-LABEL: @bswap_8i16(
; CHECK-NEXT:    [[LD0:%.*]] = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64 0), align 2
; CHECK-NEXT:    [[LD1:%.*]] = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64 1), align 2
; CHECK-NEXT:    [[LD2:%.*]] = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64 2), align 2
; CHECK-NEXT:    [[LD3:%.*]] = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64 3), align 2
; CHECK-NEXT:    [[LD4:%.*]] = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64 4), align 2
; CHECK-NEXT:    [[LD5:%.*]] = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64 5), align 2
; CHECK-NEXT:    [[LD6:%.*]] = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64 6), align 2
; CHECK-NEXT:    [[LD7:%.*]] = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64 7), align 2
; CHECK-NEXT:    [[BSWAP0:%.*]] = call i16 @llvm.bswap.i16(i16 [[LD0]])
; CHECK-NEXT:    [[BSWAP1:%.*]] = call i16 @llvm.bswap.i16(i16 [[LD1]])
; CHECK-NEXT:    [[BSWAP2:%.*]] = call i16 @llvm.bswap.i16(i16 [[LD2]])
; CHECK-NEXT:    [[BSWAP3:%.*]] = call i16 @llvm.bswap.i16(i16 [[LD3]])
; CHECK-NEXT:    [[BSWAP4:%.*]] = call i16 @llvm.bswap.i16(i16 [[LD4]])
; CHECK-NEXT:    [[BSWAP5:%.*]] = call i16 @llvm.bswap.i16(i16 [[LD5]])
; CHECK-NEXT:    [[BSWAP6:%.*]] = call i16 @llvm.bswap.i16(i16 [[LD6]])
; CHECK-NEXT:    [[BSWAP7:%.*]] = call i16 @llvm.bswap.i16(i16 [[LD7]])
; CHECK-NEXT:    store i16 [[BSWAP0]], i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64 0), align 2
; CHECK-NEXT:    store i16 [[BSWAP1]], i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64 1), align 2
; CHECK-NEXT:    store i16 [[BSWAP2]], i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64 2), align 2
; CHECK-NEXT:    store i16 [[BSWAP3]], i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64 3), align 2
; CHECK-NEXT:    store i16 [[BSWAP4]], i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64 4), align 2
; CHECK-NEXT:    store i16 [[BSWAP5]], i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64 5), align 2
; CHECK-NEXT:    store i16 [[BSWAP6]], i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64 6), align 2
; CHECK-NEXT:    store i16 [[BSWAP7]], i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64 7), align 2
; CHECK-NEXT:    ret void
;
  %ld0 = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64 0), align 2
  %ld1 = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64 1), align 2
  %ld2 = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64 2), align 2
  %ld3 = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64 3), align 2
  %ld4 = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64 4), align 2
  %ld5 = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64 5), align 2
  %ld6 = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64 6), align 2
  %ld7 = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64 7), align 2
  %bswap0 = call i16 @llvm.bswap.i16(i16 %ld0)
  %bswap1 = call i16 @llvm.bswap.i16(i16 %ld1)
  %bswap2 = call i16 @llvm.bswap.i16(i16 %ld2)
  %bswap3 = call i16 @llvm.bswap.i16(i16 %ld3)
  %bswap4 = call i16 @llvm.bswap.i16(i16 %ld4)
  %bswap5 = call i16 @llvm.bswap.i16(i16 %ld5)
  %bswap6 = call i16 @llvm.bswap.i16(i16 %ld6)
  %bswap7 = call i16 @llvm.bswap.i16(i16 %ld7)
  store i16 %bswap0, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64 0), align 2
  store i16 %bswap1, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64 1), align 2
  store i16 %bswap2, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64 2), align 2
  store i16 %bswap3, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64 3), align 2
  store i16 %bswap4, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64 4), align 2
  store i16 %bswap5, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64 5), align 2
  store i16 %bswap6, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64 6), align 2
  store i16 %bswap7, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64 7), align 2
  ret void
}

define void @bswap_16i16() #0 {
; CHECK-LABEL: @bswap_16i16(
; CHECK-NEXT:    [[LD0:%.*]] = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64 0), align 2
; CHECK-NEXT:    [[LD1:%.*]] = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64 1), align 2
; CHECK-NEXT:    [[LD2:%.*]] = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64 2), align 2
; CHECK-NEXT:    [[LD3:%.*]] = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64 3), align 2
; CHECK-NEXT:    [[LD4:%.*]] = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64 4), align 2
; CHECK-NEXT:    [[LD5:%.*]] = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64 5), align 2
; CHECK-NEXT:    [[LD6:%.*]] = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64 6), align 2
; CHECK-NEXT:    [[LD7:%.*]] = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64 7), align 2
; CHECK-NEXT:    [[LD8:%.*]] = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64 8), align 2
; CHECK-NEXT:    [[LD9:%.*]] = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64 9), align 2
; CHECK-NEXT:    [[LD10:%.*]] = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64 10), align 2
; CHECK-NEXT:    [[LD11:%.*]] = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64 11), align 2
; CHECK-NEXT:    [[LD12:%.*]] = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64 12), align 2
; CHECK-NEXT:    [[LD13:%.*]] = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64 13), align 2
; CHECK-NEXT:    [[LD14:%.*]] = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64 14), align 2
; CHECK-NEXT:    [[LD15:%.*]] = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64 15), align 2
; CHECK-NEXT:    [[BSWAP0:%.*]] = call i16 @llvm.bswap.i16(i16 [[LD0]])
; CHECK-NEXT:    [[BSWAP1:%.*]] = call i16 @llvm.bswap.i16(i16 [[LD1]])
; CHECK-NEXT:    [[BSWAP2:%.*]] = call i16 @llvm.bswap.i16(i16 [[LD2]])
; CHECK-NEXT:    [[BSWAP3:%.*]] = call i16 @llvm.bswap.i16(i16 [[LD3]])
; CHECK-NEXT:    [[BSWAP4:%.*]] = call i16 @llvm.bswap.i16(i16 [[LD4]])
; CHECK-NEXT:    [[BSWAP5:%.*]] = call i16 @llvm.bswap.i16(i16 [[LD5]])
; CHECK-NEXT:    [[BSWAP6:%.*]] = call i16 @llvm.bswap.i16(i16 [[LD6]])
; CHECK-NEXT:    [[BSWAP7:%.*]] = call i16 @llvm.bswap.i16(i16 [[LD7]])
; CHECK-NEXT:    [[BSWAP8:%.*]] = call i16 @llvm.bswap.i16(i16 [[LD8]])
; CHECK-NEXT:    [[BSWAP9:%.*]] = call i16 @llvm.bswap.i16(i16 [[LD9]])
; CHECK-NEXT:    [[BSWAP10:%.*]] = call i16 @llvm.bswap.i16(i16 [[LD10]])
; CHECK-NEXT:    [[BSWAP11:%.*]] = call i16 @llvm.bswap.i16(i16 [[LD11]])
; CHECK-NEXT:    [[BSWAP12:%.*]] = call i16 @llvm.bswap.i16(i16 [[LD12]])
; CHECK-NEXT:    [[BSWAP13:%.*]] = call i16 @llvm.bswap.i16(i16 [[LD13]])
; CHECK-NEXT:    [[BSWAP14:%.*]] = call i16 @llvm.bswap.i16(i16 [[LD14]])
; CHECK-NEXT:    [[BSWAP15:%.*]] = call i16 @llvm.bswap.i16(i16 [[LD15]])
; CHECK-NEXT:    store i16 [[BSWAP0]], i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64 0), align 2
; CHECK-NEXT:    store i16 [[BSWAP1]], i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64 1), align 2
; CHECK-NEXT:    store i16 [[BSWAP2]], i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64 2), align 2
; CHECK-NEXT:    store i16 [[BSWAP3]], i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64 3), align 2
; CHECK-NEXT:    store i16 [[BSWAP4]], i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64 4), align 2
; CHECK-NEXT:    store i16 [[BSWAP5]], i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64 5), align 2
; CHECK-NEXT:    store i16 [[BSWAP6]], i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64 6), align 2
; CHECK-NEXT:    store i16 [[BSWAP7]], i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64 7), align 2
; CHECK-NEXT:    store i16 [[BSWAP8]], i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64 8), align 2
; CHECK-NEXT:    store i16 [[BSWAP9]], i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64 9), align 2
; CHECK-NEXT:    store i16 [[BSWAP10]], i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64 10), align 2
; CHECK-NEXT:    store i16 [[BSWAP11]], i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64 11), align 2
; CHECK-NEXT:    store i16 [[BSWAP12]], i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64 12), align 2
; CHECK-NEXT:    store i16 [[BSWAP13]], i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64 13), align 2
; CHECK-NEXT:    store i16 [[BSWAP14]], i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64 14), align 2
; CHECK-NEXT:    store i16 [[BSWAP15]], i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64 15), align 2
; CHECK-NEXT:    ret void
;
  %ld0  = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64  0), align 2
  %ld1  = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64  1), align 2
  %ld2  = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64  2), align 2
  %ld3  = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64  3), align 2
  %ld4  = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64  4), align 2
  %ld5  = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64  5), align 2
  %ld6  = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64  6), align 2
  %ld7  = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64  7), align 2
  %ld8  = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64  8), align 2
  %ld9  = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64  9), align 2
  %ld10 = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64 10), align 2
  %ld11 = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64 11), align 2
  %ld12 = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64 12), align 2
  %ld13 = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64 13), align 2
  %ld14 = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64 14), align 2
  %ld15 = load i16, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @src16, i16 0, i64 15), align 2
  %bswap0  = call i16 @llvm.bswap.i16(i16 %ld0)
  %bswap1  = call i16 @llvm.bswap.i16(i16 %ld1)
  %bswap2  = call i16 @llvm.bswap.i16(i16 %ld2)
  %bswap3  = call i16 @llvm.bswap.i16(i16 %ld3)
  %bswap4  = call i16 @llvm.bswap.i16(i16 %ld4)
  %bswap5  = call i16 @llvm.bswap.i16(i16 %ld5)
  %bswap6  = call i16 @llvm.bswap.i16(i16 %ld6)
  %bswap7  = call i16 @llvm.bswap.i16(i16 %ld7)
  %bswap8  = call i16 @llvm.bswap.i16(i16 %ld8)
  %bswap9  = call i16 @llvm.bswap.i16(i16 %ld9)
  %bswap10 = call i16 @llvm.bswap.i16(i16 %ld10)
  %bswap11 = call i16 @llvm.bswap.i16(i16 %ld11)
  %bswap12 = call i16 @llvm.bswap.i16(i16 %ld12)
  %bswap13 = call i16 @llvm.bswap.i16(i16 %ld13)
  %bswap14 = call i16 @llvm.bswap.i16(i16 %ld14)
  %bswap15 = call i16 @llvm.bswap.i16(i16 %ld15)
  store i16 %bswap0 , i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64  0), align 2
  store i16 %bswap1 , i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64  1), align 2
  store i16 %bswap2 , i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64  2), align 2
  store i16 %bswap3 , i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64  3), align 2
  store i16 %bswap4 , i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64  4), align 2
  store i16 %bswap5 , i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64  5), align 2
  store i16 %bswap6 , i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64  6), align 2
  store i16 %bswap7 , i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64  7), align 2
  store i16 %bswap8 , i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64  8), align 2
  store i16 %bswap9 , i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64  9), align 2
  store i16 %bswap10, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64 10), align 2
  store i16 %bswap11, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64 11), align 2
  store i16 %bswap12, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64 12), align 2
  store i16 %bswap13, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64 13), align 2
  store i16 %bswap14, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64 14), align 2
  store i16 %bswap15, i16* getelementptr inbounds ([16 x i16], [16 x i16]* @dst16, i16 0, i64 15), align 2
  ret void
}

attributes #0 = { nounwind }
