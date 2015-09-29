// RUN: llvm-mc -filetype=obj -triple=x86_64-unknown-linux %s -o %t
// RUN: lld -flavor gnu2 %t -o %t2
// RUN: llvm-readobj -sections %t2 | FileCheck %s
// REQUIRES: x86

// Test that bss takes no space on disk.

// CHECK:        Name: .bss
// CHECK-NEXT:   Type: SHT_NOBITS
// CHECK-NEXT:   Flags [
// CHECK-NEXT:     SHF_ALLOC
// CHECK-NEXT:     SHF_WRITE
// CHECK-NEXT:   ]
// CHECK-NEXT:   Address: 0x11000
// CHECK-NEXT:   Offset: 0x1000
// CHECK-NEXT:   Size: 4
// CHECK-NEXT:   Link: 0
// CHECK-NEXT:   Info: 0
// CHECK-NEXT:   AddressAlignment: 4
// CHECK-NEXT:   EntrySize: 0
// CHECK-NEXT: }
// CHECK-NEXT: Section {
// CHECK-NEXT:   Index:
// CHECK-NEXT:   Name:
// CHECK-NEXT:   Type:
// CHECK-NEXT:   Flags [
// CHECK-NEXT:     SHF_ALLOC
// CHECK-NEXT:     SHF_EXECINSTR
// CHECK-NEXT:   ]
// CHECK-NEXT:   Address:
// CHECK-NEXT:   Offset: 0x1000


        .global _start
_start:

        .bss
        .zero 4
