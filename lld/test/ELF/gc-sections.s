# REQUIRES: x86

# RUN: llvm-mc -filetype=obj -triple=x86_64-unknown-linux %s -o %t
# RUN: ld.lld %t -o %t2
# RUN: llvm-readobj -sections -symbols %t2 | FileCheck -check-prefix=NOGC %s
# RUN: ld.lld --gc-sections %t -o %t2
# RUN: llvm-readobj -sections -symbols %t2 | FileCheck -check-prefix=GC1 %s
# RUN: ld.lld --export-dynamic --gc-sections %t -o %t2
# RUN: llvm-readobj -sections -symbols %t2 | FileCheck -check-prefix=GC2 %s

# NOGC: Name: .eh_frame
# NOGC: Name: .text
# NOGC: Name: .init
# NOGC: Name: .fini
# NOGC: Name: a
# NOGC: Name: b
# NOGC: Name: c
# NOGC: Name: x
# NOGC: Name: y
# NOGC: Name: __preinit_array_start
# NOGC: Name: __preinit_array_end
# NOGC: Name: d

# GC1:     Name: .eh_frame
# GC1:     Name: .text
# GC1:     Name: .init
# GC1:     Name: .fini
# GC1:     Name: a
# GC1:     Name: b
# GC1:     Name: c
# GC1-NOT: Name: x
# GC1-NOT: Name: y
# GC1:     Name: __preinit_array_start
# GC1:     Name: __preinit_array_end
# GC1-NOT: Name: d

# GC2:     Name: .eh_frame
# GC2:     Name: .text
# GC2:     Name: .init
# GC2:     Name: .fini
# GC2:     Name: a
# GC2:     Name: b
# GC2:     Name: c
# GC2-NOT: Name: x
# GC2-NOT: Name: y
# GC2:     Name: __preinit_array_start
# GC2:     Name: __preinit_array_end
# GC2:     Name: d

.globl _start, d
.protected a, b, c, x, y
_start:
  call a

.section .text.a,"ax",@progbits
a:
  call _start
  call b

.section .text.b,"ax",@progbits
b:
  call c

.section .text.c,"ax",@progbits
c:
  nop

.section .text.d,"ax",@progbits
d:
  nop

.section .text.x,"ax",@progbits
x:
  call y

.section .text.y,"ax",@progbits
y:
  call x

.section .init,"aw",@init_array
  .quad 0

.section .fini,"aw",@fini_array
  .quad 0

.section .preinit_array,"aw",@preinit_array
  .quad 0

.section .eh_frame,"a",@unwind
  .quad 0
