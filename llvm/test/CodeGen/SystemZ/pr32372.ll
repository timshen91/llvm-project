; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc %s -o - -mtriple=s390x-linux-gnu | FileCheck %s

define void @pr32372(i8*) {
; CHECK-LABEL: pr32372:
; CHECK:       # BB#0: # %BB
; CHECK-NEXT:    llc %r1, 0(%r2)
; CHECK-NEXT:    mvhhi 0(%r1), -3825
; CHECK-NEXT:    llill %r0, 0
; CHECK-NEXT:    dlr %r0, %r1
; CHECK-NEXT:  .LBB0_1: # %CF251
; CHECK-NEXT:    # =>This Inner Loop Header: Depth=1
; CHECK-NEXT:    j .LBB0_1
BB:
  %L = load i8, i8* %0
  store i16 -3825, i16* undef
  %L5 = load i8, i8* %0
  %B9 = urem i8 %L5, %L
  %I107 = insertelement <8 x i8> zeroinitializer, i8 %B9, i32 7
  %ZE141 = zext i8 %L5 to i16
  br label %CF251

CF251:                                            ; preds = %CF258, %CF251, %BB
  %Shuff217 = shufflevector <8 x i8> zeroinitializer, <8 x i8> %I107, <8 x i32> <i32 0, i32 2, i32 undef, i32 6, i32 8, i32 undef, i32 12, i32 14>
  %Cmp227 = icmp sge i16 %ZE141, 0
  br i1 %Cmp227, label %CF251, label %CF258

CF258:                                            ; preds = %CF251
  %Shuff230 = shufflevector <2 x i16> undef, <2 x i16> undef, <2 x i32> <i32 3, i32 1>
  br label %CF251
}
