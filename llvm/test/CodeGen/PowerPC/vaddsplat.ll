; RUN: llc -O0 -mcpu=pwr7 <%s | FileCheck %s

; Test optimization of build_vector into vadd/vsplt for 6-bit immediates.

target datalayout = "E-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f128:128:128-v128:128:128-n32:64"
target triple = "powerpc64-unknown-linux-gnu"

%v4i32 = type <4 x i32>
%v8i16 = type <8 x i16>
%v16i8 = type <16 x i8>

define void @test_v4i32_pos(%v4i32* %P, %v4i32* %S) {
       %p = load %v4i32* %P
       %r = add %v4i32 %p, < i32 18, i32 18, i32 18, i32 18 >
       store %v4i32 %r, %v4i32* %S
       ret void
}

; CHECK: test_v4i32_pos:
; CHECK: vspltisw [[REG1:[0-9]+]], 9
; CHECK: vadduwm {{[0-9]+}}, [[REG1]], [[REG1]]

define void @test_v4i32_neg(%v4i32* %P, %v4i32* %S) {
       %p = load %v4i32* %P
       %r = add %v4i32 %p, < i32 -28, i32 -28, i32 -28, i32 -28 >
       store %v4i32 %r, %v4i32* %S
       ret void
}

; CHECK: test_v4i32_neg:
; CHECK: vspltisw [[REG1:[0-9]+]], -14
; CHECK: vadduwm {{[0-9]+}}, [[REG1]], [[REG1]]

define void @test_v8i16_pos(%v8i16* %P, %v8i16* %S) {
       %p = load %v8i16* %P
       %r = add %v8i16 %p, < i16 30, i16 30, i16 30, i16 30, i16 30, i16 30, i16 30, i16 30 >
       store %v8i16 %r, %v8i16* %S
       ret void
}

; CHECK: test_v8i16_pos:
; CHECK: vspltish [[REG1:[0-9]+]], 15
; CHECK: vadduhm {{[0-9]+}}, [[REG1]], [[REG1]]

define void @test_v8i16_neg(%v8i16* %P, %v8i16* %S) {
       %p = load %v8i16* %P
       %r = add %v8i16 %p, < i16 -32, i16 -32, i16 -32, i16 -32, i16 -32, i16 -32, i16 -32, i16 -32 >
       store %v8i16 %r, %v8i16* %S
       ret void
}

; CHECK: test_v8i16_neg:
; CHECK: vspltish [[REG1:[0-9]+]], -16
; CHECK: vadduhm {{[0-9]+}}, [[REG1]], [[REG1]]

define void @test_v16i8_pos(%v16i8* %P, %v16i8* %S) {
       %p = load %v16i8* %P
       %r = add %v16i8 %p, < i8 16, i8 16, i8 16, i8 16, i8 16, i8 16, i8 16, i8 16, i8 16, i8 16, i8 16, i8 16, i8 16, i8 16, i8 16, i8 16 >
       store %v16i8 %r, %v16i8* %S
       ret void
}

; CHECK: test_v16i8_pos:
; CHECK: vspltisb [[REG1:[0-9]+]], 8
; CHECK: vaddubm {{[0-9]+}}, [[REG1]], [[REG1]]

define void @test_v16i8_neg(%v16i8* %P, %v16i8* %S) {
       %p = load %v16i8* %P
       %r = add %v16i8 %p, < i8 -18, i8 -18, i8 -18, i8 -18, i8 -18, i8 -18, i8 -18, i8 -18, i8 -18, i8 -18, i8 -18, i8 -18, i8 -18, i8 -18, i8 -18, i8 -18 >
       store %v16i8 %r, %v16i8* %S
       ret void
}

; CHECK: test_v16i8_neg:
; CHECK: vspltisb [[REG1:[0-9]+]], -9
; CHECK: vaddubm {{[0-9]+}}, [[REG1]], [[REG1]]

