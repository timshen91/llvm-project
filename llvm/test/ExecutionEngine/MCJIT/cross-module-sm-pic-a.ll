; RUN: %lli -extra-module=%p/Inputs/cross-module-b.ll -relocation-model=pic -code-model=small %s > /dev/null
; RUN: %lli -mtriple=x86_64-pc-linux -extra-module=%p/Inputs/cross-module-b.ll -relocation-model=pic -code-model=small %s > /dev/null
; XFAIL: mips, i686, i386

declare i32 @FB()

define i32 @FA() {
  ret i32 0
}

define i32 @main() {
  %r = call i32 @FB( )   ; <i32> [#uses=1]
  ret i32 %r
}

