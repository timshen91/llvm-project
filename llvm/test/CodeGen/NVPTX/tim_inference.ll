; RUN: opt -S -nvptx-infer-addrspace < %s | FileCheck %s
target triple = "nvptx64-unknown-unknown"

@global = external addrspace(3) global float

; CHECK-LABEL: TestArg
; CHECK: load float, float addrspace(3)*
define float @TestArg(float addrspace(3)* %f) {
bb:
  %t = addrspacecast float addrspace(3)* %f to float*
  %o = load float, float* %t
  ret float %o
}

; CHECK-LABEL: TestGlobal
; CHECK: load float, float addrspace(3)*
define float @TestGlobal() {
bb:
  %t = addrspacecast float addrspace(3)* @global to float*
  %o = load float, float* %t
  ret float %o
}

; CHECK-LABEL: CopyFloats
; CHECK: load float, float addrspace(3)*
; CHECK: store float {{.*}}, float addrspace(1)*
define void @CopyFloats(float addrspace(3)* %special_cur, float addrspace(3)* %special_end, float addrspace(1)* %special_out) {
bb:
  %cur = addrspacecast float addrspace(3)* %special_cur to float*
  %end = addrspacecast float addrspace(3)* %special_end to float*
  %out = addrspacecast float addrspace(1)* %special_out to float*
  %tmp = icmp eq float* %cur, %end
  br i1 %tmp, label %bb12, label %bb3

bb3:
  %tmp4 = phi float* [ %tmp9, %bb3 ], [ %out, %bb ]
  %tmp5 = phi float* [ %tmp6, %bb3 ], [ %cur, %bb ]
  %tmp6 = getelementptr inbounds float, float* %tmp5, i64 1
  %tmp8 = load float, float* %tmp5
  %tmp9 = getelementptr inbounds float, float* %tmp4, i64 1
  store float %tmp8, float* %tmp4
  %tmp11 = icmp eq float* %tmp6, %end
  br i1 %tmp11, label %bb12, label %bb3

bb12:
  ret void
}

; CHECK-LABEL: DoNotMergeDifferentAddrSpacePtrs
; CHECK: load float, float*
define float @DoNotMergeDifferentAddrSpacePtrs(float addrspace(3)* %special_in1, float addrspace(1)* %special_in2, i1 %selector) {
bb:
  br i1 %selector, label %bb2, label %bb3

bb2:
  %in1 = addrspacecast float addrspace(3)* %special_in1 to float*
  br label %bb4

bb3:
  %in2 = addrspacecast float addrspace(1)* %special_in2 to float*
  br label %bb4

bb4:
  %ptr = phi float* [ %in1, %bb2 ], [ %in2, %bb3 ]
  %ret = load float, float* %ptr
  ret float %ret
}

; CHECK-LABEL: MergeSameAddrSpacePtrs
; CHECK: load float, float addrspace(3)*
define float @MergeSameAddrSpacePtrs(float addrspace(3)* %special_in1, float addrspace(3)* %special_in2, i1 %selector) {
bb:
  br i1 %selector, label %bb2, label %bb3

bb2:
  %in1 = addrspacecast float addrspace(3)* %special_in1 to float*
  br label %bb4

bb3:
  %in2 = addrspacecast float addrspace(3)* %special_in2 to float*
  br label %bb4

bb4:
  %ptr = phi float* [ %in1, %bb2 ], [ %in2, %bb3 ]
  %ret = load float, float* %ptr
  ret float %ret
}
