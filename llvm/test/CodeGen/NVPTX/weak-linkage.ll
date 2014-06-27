; RUN: llc < %s -march=nvptx -mcpu=sm_20 | FileCheck %s


; CHECK: .weak .func foo
define weak void @foo() {
  ret void
}

; CHECK: .visible .func bar
define void @bar() {
  ret void
}
