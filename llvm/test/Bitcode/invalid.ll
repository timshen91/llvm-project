; RUN:  not llvm-dis < %s.bc 2>&1 | FileCheck %s

; CHECK: llvm-dis{{(\.EXE|\.exe)?}}: Unknown attribute kind

; invalid.ll.bc has an invalid attribute number.
; The test checks that LLVM reports the error and doesn't access freed memory
; in doing so.
