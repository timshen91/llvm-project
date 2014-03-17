; RUN: opt < %s -S -globalopt -instcombine | FileCheck %s
;; check that global opt annotates loads from global variales that only hold 0 or 1
;; such that instcombine can optimize accordingly.

@G = internal addrspace(1) global i32 0
; CHECK: @G
; CHECK: addrspace(1)
; CHECK: global i32 0

define void @set1() {
  store i32 0, i32 addrspace(1)* @G
; CHECK: store i32 0
  ret void
}

define void @set2() {
  store i32 1, i32 addrspace(1)* @G
; CHECK: store i32 1
  ret void
}

define i1 @get() {
; CHECK-LABEL: @get(
  %A = load i32 addrspace(1) * @G
  %C = icmp slt i32 %A, 2
  ret i1 %C
; CHECK: ret i1 true
}

