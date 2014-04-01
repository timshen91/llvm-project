// RUN: llvm-mc -g -triple  i686-pc-linux-gnu %s -filetype=obj -o - | llvm-readobj -r | FileCheck %s
// RUN: llvm-mc -g -triple  i686-pc-linux-gnu %s -filetype=asm -o - | FileCheck --check-prefix=ASM %s


// Test that on ELF:
// 1. the debug info has a relocation to debug_abbrev and one to to debug_line.
// 2. the debug_aranges has relocations to text and debug_line.


    .text
    .globl foo
    .type foo, @function
    .align 4
foo:
    ret
    .size foo, .-foo

// CHECK:      Relocations [
// CHECK:        Section ({{[^ ]+}}) .rel.debug_info {
// CHECK-NEXT:     0x6 R_386_32 .debug_abbrev 0x0
// CHECK-NEXT:     0xC R_386_32 .debug_line 0x0
// CHECK:        }
// CHECK-NEXT:   Section ({{[^ ]+}}) .rel.debug_aranges {
// CHECK-NEXT:     0x6 R_386_32 .debug_info 0x0
// CHECK-NEXT:     0x10 R_386_32 .text 0x0
// CHECK-NEXT:   }
// CHECK:      ]

// First instance of the section is just to give it a label for debug_aranges to refer to
// ASM: .section .debug_info

// ASM: .section .debug_abbrev
// ASM-NEXT: [[ABBREV_LABEL:.Ltmp[0-9]+]]

// Second instance of the section has the CU
// ASM: .section .debug_info
// Dwarf version
// ASM: .short 2
// ASM-NEXT: .long [[ABBREV_LABEL]]
// First .byte 1 is the abbreviation number for the compile_unit abbrev
// ASM: .byte 1
// ASM-NEXT: .long [[LINE_LABEL:.L[a-z0-9]+]]

// ASM: .section .debug_line
// ASM-NEXT: [[LINE_LABEL]]

