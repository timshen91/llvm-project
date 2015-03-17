@ RUN: not llvm-mc -triple=armv6 -show-encoding < %s > %t1 2> %t2
@ RUN: FileCheck --check-prefix=CHECK-V6 %s < %t1
@ RUN: FileCheck --check-prefix=CHECK-ERROR-V6 %s < %t2
@ RUN: llvm-mc -triple=armv6k -show-encoding < %s \
@ RUN:     | FileCheck --check-prefix=CHECK-ARM %s
@ RUN: llvm-mc -triple=armv6t2 -show-encoding < %s \
@ RUN:     | FileCheck --check-prefix=CHECK-ARM %s
@ RUN: llvm-mc -triple=thumb -mcpu=arm1156t2-s -show-encoding < %s \
@ RUN:     | FileCheck --check-prefix=CHECK-THUMB %s
@ RUN: llvm-mc -triple=armv6m -show-encoding < %s \
@ RUN:     | FileCheck --check-prefix=CHECK-V6M %s

  .syntax unified

@------------------------------------------------------------------------------
@ YIELD/WFE/WFI/SEV - are not supported pre v6K
@------------------------------------------------------------------------------
        nop
        yield
        wfe
        wfi
        sev


@------------------------------------------------------------------------------
@ v6 using ARM encoding
@------------------------------------------------------------------------------
@ CHECK-V6: mov	r0, r0                  @ encoding: [0x00,0x00,0xa0,0xe1]
@ CHECK-ERROR-V6: error: instruction requires: armv6k
@ CHECK-ERROR-V6: yield
@ CHECK-ERROR-V6: ^
@ CHECK-ERROR-V6: error: instruction requires: armv6k
@ CHECK-ERROR-V6: wfe
@ CHECK-ERROR-V6: ^
@ CHECK-ERROR-V6: error: instruction requires: armv6k
@ CHECK-ERROR-V6: wfi
@ CHECK-ERROR-V6: error: instruction requires: armv6k
@ CHECK-ERROR-V6: sev
@ CHECK-ERROR-V6: ^

@------------------------------------------------------------------------------
@ v6K using ARM encoding
@------------------------------------------------------------------------------
@------------------------------------------------------------------------------
@ v6T2 using ARM encoding (arm triple)
@------------------------------------------------------------------------------
@ CHECK-ARM: nop                             @ encoding: [0x00,0xf0,0x20,0xe3]
@ CHECK-ARM: yield                           @ encoding: [0x01,0xf0,0x20,0xe3]
@ CHECK-ARM: wfe                             @ encoding: [0x02,0xf0,0x20,0xe3]
@ CHECK-ARM: wfi                             @ encoding: [0x03,0xf0,0x20,0xe3]
@ CHECK-ARM: sev                             @ encoding: [0x04,0xf0,0x20,0xe3]

@------------------------------------------------------------------------------
@ v6T2 using THUMB encoding (thumb triple)
@------------------------------------------------------------------------------
@ CHECK-THUMB: nop                             @ encoding: [0x00,0xbf]
@ CHECK-THUMB: yield                           @ encoding: [0x10,0xbf]
@ CHECK-THUMB: wfe                             @ encoding: [0x20,0xbf]
@ CHECK-THUMB: wfi                             @ encoding: [0x30,0xbf]
@ CHECK-THUMB: sev                             @ encoding: [0x40,0xbf]

@------------------------------------------------------------------------------
@ v6M using THUMB encoding
@------------------------------------------------------------------------------
@ CHECK-V6M: nop                             @ encoding: [0x00,0xbf]
@ CHECK-V6M: yield                           @ encoding: [0x10,0xbf]
@ CHECK-V6M: wfe                             @ encoding: [0x20,0xbf]
@ CHECK-V6M: wfi                             @ encoding: [0x30,0xbf]
@ CHECK-V6M: sev                             @ encoding: [0x40,0xbf]
