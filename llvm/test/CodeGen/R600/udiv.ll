;RUN: llc < %s -march=r600 -mcpu=redwood | FileCheck --check-prefix=EG-CHECK %s
;RUN: llc < %s -march=amdgcn -mcpu=verde -verify-machineinstrs | FileCheck --check-prefix=SI-CHECK %s

;EG-CHECK-LABEL: {{^}}test:
;EG-CHECK-NOT: SETGE_INT
;EG-CHECK: CF_END

define void @test(i32 addrspace(1)* %out, i32 addrspace(1)* %in) {
  %b_ptr = getelementptr i32 addrspace(1)* %in, i32 1
  %a = load i32 addrspace(1) * %in
  %b = load i32 addrspace(1) * %b_ptr
  %result = udiv i32 %a, %b
  store i32 %result, i32 addrspace(1)* %out
  ret void
}

;The code generated by udiv is long and complex and may frequently change.
;The goal of this test is to make sure the ISel doesn't fail when it gets
;a v4i32 udiv

;EG-CHECK-LABEL: {{^}}test2:
;EG-CHECK: CF_END
;SI-CHECK-LABEL: {{^}}test2:
;SI-CHECK: s_endpgm

define void @test2(<2 x i32> addrspace(1)* %out, <2 x i32> addrspace(1)* %in) {
  %b_ptr = getelementptr <2 x i32> addrspace(1)* %in, i32 1
  %a = load <2 x i32> addrspace(1) * %in
  %b = load <2 x i32> addrspace(1) * %b_ptr
  %result = udiv <2 x i32> %a, %b
  store <2 x i32> %result, <2 x i32> addrspace(1)* %out
  ret void
}

;EG-CHECK-LABEL: {{^}}test4:
;EG-CHECK: CF_END
;SI-CHECK-LABEL: {{^}}test4:
;SI-CHECK: s_endpgm

define void @test4(<4 x i32> addrspace(1)* %out, <4 x i32> addrspace(1)* %in) {
  %b_ptr = getelementptr <4 x i32> addrspace(1)* %in, i32 1
  %a = load <4 x i32> addrspace(1) * %in
  %b = load <4 x i32> addrspace(1) * %b_ptr
  %result = udiv <4 x i32> %a, %b
  store <4 x i32> %result, <4 x i32> addrspace(1)* %out
  ret void
}
