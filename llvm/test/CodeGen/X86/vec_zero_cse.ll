; RUN: llc < %s -relocation-model=static -mtriple=i686-unknown -mattr=+mmx,+sse3 | FileCheck %s
; 64-bit stores here do not use MMX.

@M1 = external global <1 x i64>
@M2 = external global <2 x i32>

@S1 = external global <2 x i64>
@S2 = external global <4 x i32>

define void @test1() {
;CHECK-LABEL: @test1
;CHECK: xorpd
  store <1 x i64> zeroinitializer, <1 x i64>* @M1
  store <2 x i32> zeroinitializer, <2 x i32>* @M2
  ret void
}

define void @test2() {
;CHECK-LABEL: @test2
;CHECK: pshufd
  store <1 x i64> < i64 -1 >, <1 x i64>* @M1
  store <2 x i32> < i32 -1, i32 -1 >, <2 x i32>* @M2
  ret void
}

define void @test3() {
;CHECK-LABEL: @test3
;CHECK: xorps
  store <2 x i64> zeroinitializer, <2 x i64>* @S1
  store <4 x i32> zeroinitializer, <4 x i32>* @S2
  ret void
}

define void @test4() {
;CHECK-LABEL: @test4
;CHECK: pcmpeqd
  store <2 x i64> < i64 -1, i64 -1>, <2 x i64>* @S1
  store <4 x i32> < i32 -1, i32 -1, i32 -1, i32 -1 >, <4 x i32>* @S2
  ret void
}


