; RUN: llc < %s -march=nvptx -mcpu=sm_53 | FileCheck %s
; RUN: llc < %s -march=nvptx64 -mcpu=sm_53 | FileCheck %s


; CHECK: .target sm_53

