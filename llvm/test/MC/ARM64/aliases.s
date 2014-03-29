; RUN: llvm-mc -triple arm64-apple-darwin -output-asm-variant=1 -show-encoding < %s | FileCheck %s

foo:
;-----------------------------------------------------------------------------
; ADD #0 to/from SP/WSP is a MOV
;-----------------------------------------------------------------------------
  add x1, sp, #0
; CHECK: mov x1, sp
  add sp, x2, #0
; CHECK: mov sp, x2
  add w3, wsp, #0
; CHECK: mov w3, wsp
  add wsp, w4, #0
; CHECK: mov wsp, w4
  mov x5, sp
; CHECK: mov x5, sp
  mov sp, x6
; CHECK: mov sp, x6
  mov w7, wsp
; CHECK: mov w7, wsp
  mov wsp, w8
; CHECK: mov wsp, w8

;-----------------------------------------------------------------------------
; ORR Rd, Rn, Rn is a MOV
;-----------------------------------------------------------------------------
  orr x2, xzr, x9
; CHECK: mov x2, x9
  orr w2, wzr, w9
; CHECK: mov w2, w9
  mov x3, x4
; CHECK: mov x3, x4
  mov w5, w6
; CHECK: mov w5, w6

;-----------------------------------------------------------------------------
; TST Xn, #<imm>
;-----------------------------------------------------------------------------
        tst w1, #3
        tst x1, #3
        tst w1, w2
        tst x1, x2
        ands wzr, w1, w2, lsl #2
        ands xzr, x1, x2, lsl #3
        tst w3, w7, lsl #31
        tst x2, x20, asr #0

; CHECK: tst	w1, #0x3                ; encoding: [0x3f,0x04,0x00,0x72]
; CHECK: tst	x1, #0x3                ; encoding: [0x3f,0x04,0x40,0xf2]
; CHECK: tst	w1, w2                  ; encoding: [0x3f,0x00,0x02,0x6a]
; CHECK: tst	x1, x2                  ; encoding: [0x3f,0x00,0x02,0xea]
; CHECK: tst	w1, w2, lsl #2          ; encoding: [0x3f,0x08,0x02,0x6a]
; CHECK: tst	x1, x2, lsl #3          ; encoding: [0x3f,0x0c,0x02,0xea]
; CHECK: tst	w3, w7, lsl #31         ; encoding: [0x7f,0x7c,0x07,0x6a]
; CHECK: tst	x2, x20, asr #0         ; encoding: [0x5f,0x00,0x94,0xea]

;-----------------------------------------------------------------------------
; ADDS to WZR/XZR is a CMN
;-----------------------------------------------------------------------------
  cmn w1, #3, lsl #0
  cmn x2, #4194304
  cmn w4, w5
  cmn x6, x7
  cmn w8, w9, asr #3
  cmn x2, x3, lsr #4
  cmn x2, w3, uxtb #1
  cmn x4, x5, uxtx #1

; CHECK: cmn	w1, #3                  ; encoding: [0x3f,0x0c,0x00,0x31]
; CHECK: cmn	x2, #4194304            ; encoding: [0x5f,0x00,0x50,0xb1]
; CHECK: cmn	w4, w5                  ; encoding: [0x9f,0x00,0x05,0x2b]
; CHECK: cmn	x6, x7                  ; encoding: [0xdf,0x00,0x07,0xab]
; CHECK: cmn	w8, w9, asr #3          ; encoding: [0x1f,0x0d,0x89,0x2b]
; CHECK: cmn	x2, x3, lsr #4          ; encoding: [0x5f,0x10,0x43,0xab]
; CHECK: cmn	x2, w3, uxtb #1         ; encoding: [0x5f,0x04,0x23,0xab]
; CHECK: cmn	x4, x5, uxtx #1         ; encoding: [0x9f,0x64,0x25,0xab]


;-----------------------------------------------------------------------------
; SUBS to WZR/XZR is a CMP
;-----------------------------------------------------------------------------
  cmp w1, #1024, lsl #12
  cmp x2, #1024
  cmp w4, w5
  cmp x6, x7
  cmp w8, w9, asr #3
  cmp x2, x3, lsr #4
  cmp x2, w3, uxth #2
  cmp x4, x5, uxtx
  cmp wzr, w1
  cmp x8, w8, uxtw
  cmp w9, w8, uxtw
  cmp wsp, w9, lsl #0

; CHECK: cmp	w1, #4194304            ; encoding: [0x3f,0x00,0x50,0x71]
; CHECK: cmp	x2, #1024               ; encoding: [0x5f,0x00,0x10,0xf1]
; CHECK: cmp	w4, w5                  ; encoding: [0x9f,0x00,0x05,0x6b]
; CHECK: cmp	x6, x7                  ; encoding: [0xdf,0x00,0x07,0xeb]
; CHECK: cmp	w8, w9, asr #3          ; encoding: [0x1f,0x0d,0x89,0x6b]
; CHECK: cmp	x2, x3, lsr #4          ; encoding: [0x5f,0x10,0x43,0xeb]
; CHECK: cmp	x2, w3, uxth #2         ; encoding: [0x5f,0x28,0x23,0xeb]
; CHECK: cmp	x4, x5, uxtx            ; encoding: [0x9f,0x60,0x25,0xeb]
; CHECK: cmp	wzr, w1                 ; encoding: [0xff,0x03,0x01,0x6b]
; CHECK: cmp	x8, w8, uxtw            ; encoding: [0x1f,0x41,0x28,0xeb]
; CHECK: cmp	w9, w8, uxtw            ; encoding: [0x3f,0x41,0x28,0x6b]
; CHECK: cmp	wsp, w9                 ; encoding: [0xff,0x63,0x29,0x6b]


;-----------------------------------------------------------------------------
; SUB/SUBS from WZR/XZR is a NEG
;-----------------------------------------------------------------------------

  neg w0, w1
; CHECK: neg w0, w1
  neg w0, w1, lsl #1
; CHECK: sub w0, wzr, w1, lsl #1
  neg x0, x1
; CHECK: neg x0, x1
  neg x0, x1, asr #1
; CHECK: sub x0, xzr, x1, asr #1
  negs w0, w1
; CHECK: negs w0, w1
  negs w0, w1, lsl #1
; CHECK: subs w0, wzr, w1, lsl #1
  negs x0, x1
; CHECK: negs x0, x1
  negs x0, x1, asr #1
; CHECK: subs x0, xzr, x1, asr #1

;-----------------------------------------------------------------------------
; MOV aliases
;-----------------------------------------------------------------------------

  mov x0, #281470681743360
  mov x0, #18446744073709486080

; CHECK: movz	x0, #65535, lsl #32
; CHECK: movn	x0, #65535

  mov w0, #0xffffffff
  mov w0, #0xffffff00

; CHECK: movn   w0, #0
; CHECK: movn   w0, #255

;-----------------------------------------------------------------------------
; MVN aliases
;-----------------------------------------------------------------------------

        mvn w4, w9
        mvn x2, x3
        orn w4, wzr, w9

; CHECK: mvn	w4, w9             ; encoding: [0xe4,0x03,0x29,0x2a]
; CHECK: mvn	x2, x3             ; encoding: [0xe2,0x03,0x23,0xaa]
; CHECK: mvn	w4, w9             ; encoding: [0xe4,0x03,0x29,0x2a]

;-----------------------------------------------------------------------------
; Bitfield aliases
;-----------------------------------------------------------------------------

  bfi   w0, w0, #1, #4
  bfi   x0, x0, #1, #4
  bfi   w0, w0, #0, #2
  bfi   x0, x0, #0, #2
  bfxil w0, w0, #2, #3
  bfxil x0, x0, #2, #3
  sbfiz w0, w0, #1, #4
  sbfiz x0, x0, #1, #4
  sbfx  w0, w0, #2, #3
  sbfx  x0, x0, #2, #3
  ubfiz w0, w0, #1, #4
  ubfiz x0, x0, #1, #4
  ubfx  w0, w0, #2, #3
  ubfx  x0, x0, #2, #3

; CHECK: bfm  w0, w0, #31, #3
; CHECK: bfm  x0, x0, #63, #3
; CHECK: bfm  w0, w0, #0, #1
; CHECK: bfm  x0, x0, #0, #1
; CHECK: bfm  w0, w0, #2, #4
; CHECK: bfm  x0, x0, #2, #4
; CHECK: sbfm w0, w0, #31, #3
; CHECK: sbfm x0, x0, #63, #3
; CHECK: sbfm w0, w0, #2, #4
; CHECK: sbfm x0, x0, #2, #4
; CHECK: ubfm w0, w0, #31, #3
; CHECK: ubfm x0, x0, #63, #3
; CHECK: ubfm w0, w0, #2, #4
; CHECK: ubfm x0, x0, #2, #4

;-----------------------------------------------------------------------------
; Shift (immediate) aliases
;-----------------------------------------------------------------------------

; CHECK: asr w1, w3, #13
; CHECK: asr x1, x3, #13
; CHECK: lsl w0, w0, #1
; CHECK: lsl x0, x0, #1
; CHECK: lsr w0, w0, #4
; CHECK: lsr x0, x0, #4

   sbfm w1, w3, #13, #31
   sbfm x1, x3, #13, #63
   ubfm w0, w0, #31, #30
   ubfm x0, x0, #63, #62
   ubfm w0, w0, #4, #31
   ubfm x0, x0, #4, #63
; CHECK: extr w1, w3, w3, #5
; CHECK: extr x1, x3, x3, #5
   ror w1, w3, #5
   ror x1, x3, #5
; CHECK: lsl w1, wzr, #3
   lsl w1, wzr, #3

;-----------------------------------------------------------------------------
; Sign/Zero extend aliases
;-----------------------------------------------------------------------------

  sxtb  w1, w2
  sxth  w1, w2
  uxtb  w1, w2
  uxth  w1, w2

; CHECK: sxtb w1, w2
; CHECK: sxth w1, w2
; CHECK: uxtb w1, w2
; CHECK: uxth w1, w2

  sxtb  x1, x2
  sxth  x1, x2
  sxtw  x1, x2
  uxtb  x1, x2
  uxth  x1, x2
  uxtw  x1, x2

; CHECK: sxtb x1, x2
; CHECK: sxth x1, x2
; CHECK: sxtw x1, x2
; CHECK: uxtb x1, x2
; CHECK: uxth x1, x2
; CHECK: uxtw x1, x2

;-----------------------------------------------------------------------------
; Negate with carry
;-----------------------------------------------------------------------------

  ngc   w1, w2
  ngc   x1, x2
  ngcs  w1, w2
  ngcs  x1, x2

; CHECK: ngc  w1, w2
; CHECK: ngc  x1, x2
; CHECK: ngcs w1, w2
; CHECK: ngcs x1, x2

;-----------------------------------------------------------------------------
; 6.6.1 Multiply aliases
;-----------------------------------------------------------------------------

  mneg   w1, w2, w3
  mneg   x1, x2, x3
  mul    w1, w2, w3
  mul    x1, x2, x3
  smnegl x1, w2, w3
  umnegl x1, w2, w3
  smull   x1, w2, w3
  umull   x1, w2, w3

; CHECK: mneg w1, w2, w3
; CHECK: mneg x1, x2, x3
; CHECK: mul w1, w2, w3
; CHECK: mul x1, x2, x3
; CHECK: smnegl x1, w2, w3
; CHECK: umnegl x1, w2, w3
; CHECK: smull x1, w2, w3
; CHECK: umull x1, w2, w3

;-----------------------------------------------------------------------------
; Conditional select aliases
;-----------------------------------------------------------------------------

  cset   w1, eq
  cset   x1, eq
  csetm  w1, ne
  csetm  x1, ne
  cinc   w1, w2, lt
  cinc   x1, x2, lt
  cinv   w1, w2, mi
  cinv   x1, x2, mi

; CHECK: csinc  w1, wzr, wzr, ne
; CHECK: csinc  x1, xzr, xzr, ne
; CHECK: csinv  w1, wzr, wzr, eq
; CHECK: csinv  x1, xzr, xzr, eq
; CHECK: csinc  w1, w2, w2, ge
; CHECK: csinc  x1, x2, x2, ge
; CHECK: csinv  w1, w2, w2, pl
; CHECK: csinv  x1, x2, x2, pl

;-----------------------------------------------------------------------------
; SYS aliases
;-----------------------------------------------------------------------------

  sys #0, c7, c1, #0
; CHECK: ic ialluis
  sys #0, c7, c5, #0
; CHECK: ic iallu
  sys #3, c7, c5, #1
; CHECK: ic ivau

  sys #3, c7, c4, #1
; CHECK: dc zva
  sys #0, c7, c6, #1
; CHECK: dc ivac
  sys #0, c7, c6, #2
; CHECK: dc isw
  sys #3, c7, c10, #1
; CHECK: dc cvac
  sys #0, c7, c10, #2
; CHECK: dc csw
  sys #3, c7, c11, #1
; CHECK: dc cvau
  sys #3, c7, c14, #1
; CHECK: dc civac
  sys #0, c7, c14, #2
; CHECK: dc cisw

  sys #0, c7, c8, #0
; CHECK: at s1e1r
  sys #4, c7, c8, #0
; CHECK: at s1e2r
  sys #6, c7, c8, #0
; CHECK: at s1e3r
  sys #0, c7, c8, #1
; CHECK: at s1e1w
  sys #4, c7, c8, #1
; CHECK: at s1e2w
  sys #6, c7, c8, #1
; CHECK: at s1e3w
  sys #0, c7, c8, #2
; CHECK: at s1e0r
  sys #0, c7, c8, #3
; CHECK: at s1e0w
  sys #4, c7, c8, #4
; CHECK: at s12e1r
  sys #4, c7, c8, #5
; CHECK: at s12e1w
  sys #4, c7, c8, #6
; CHECK: at s12e0r
  sys #4, c7, c8, #7
; CHECK: at s12e0w

  sys #0, c8, c3, #0
; CHECK: tlbi vmalle1is
  sys #4, c8, c3, #0
; CHECK: tlbi alle2is
  sys #6, c8, c3, #0
; CHECK: tlbi alle3is
  sys #0, c8, c3, #1
; CHECK: tlbi vae1is
  sys #4, c8, c3, #1
; CHECK: tlbi vae2is
  sys #6, c8, c3, #1
; CHECK: tlbi vae3is
  sys #0, c8, c3, #2
; CHECK: tlbi aside1is
  sys #0, c8, c3, #3
; CHECK: tlbi vaae1is
  sys #4, c8, c3, #4
; CHECK: tlbi alle1is
  sys #0, c8, c3, #5
; CHECK: tlbi vale1is
  sys #0, c8, c3, #7
; CHECK: tlbi vaale1is
  sys #0, c8, c7, #0
; CHECK: tlbi vmalle1
  sys #4, c8, c7, #0
; CHECK: tlbi alle2
  sys #4, c8, c3, #5
; CHECK: tlbi vale2is
  sys #6, c8, c3, #5
; CHECK: tlbi vale3is
  sys #6, c8, c7, #0
; CHECK: tlbi alle3
  sys #0, c8, c7, #1
; CHECK: tlbi vae1
  sys #4, c8, c7, #1
; CHECK: tlbi vae2
  sys #6, c8, c7, #1
; CHECK: tlbi vae3
  sys #0, c8, c7, #2
; CHECK: tlbi aside1
  sys #0, c8, c7, #3
; CHECK: tlbi vaae1
  sys #4, c8, c7, #4
; CHECK: tlbi alle1
  sys #0, c8, c7, #5
; CHECK: tlbi vale1
  sys #4, c8, c7, #5
; CHECK: tlbi vale2
  sys #6, c8, c7, #5
; CHECK: tlbi vale3
  sys #0, c8, c7, #7
; CHECK: tlbi vaale1
  sys #4, c8, c4, #1
; CHECK: tlbi ipas2e1
  sys #4, c8, c4, #5
; CHECK: tlbi ipas2le1
  sys #4, c8, c7, #6
; CHECK: tlbi vmalls12e1
  sys #4, c8, c3, #6
; CHECK: tlbi vmalls12e1is

  ic ialluis
; CHECK: ic ialluis
  ic iallu
; CHECK: ic iallu
  ic ivau
; CHECK: ic ivau

  dc zva
; CHECK: dc zva
  dc ivac
; CHECK: dc ivac
  dc isw
; CHECK: dc isw
  dc cvac
; CHECK: dc cvac
  dc csw
; CHECK: dc csw
  dc cvau
; CHECK: dc cvau
  dc civac
; CHECK: dc civac
  dc cisw
; CHECK: dc cisw

  at s1e1r
; CHECK: at s1e1r
  at s1e2r
; CHECK: at s1e2r
  at s1e3r
; CHECK: at s1e3r
  at s1e1w
; CHECK: at s1e1w
  at s1e2w
; CHECK: at s1e2w
  at s1e3w
; CHECK: at s1e3w
  at s1e0r
; CHECK: at s1e0r
  at s1e0w
; CHECK: at s1e0w
  at s12e1r
; CHECK: at s12e1r
  at s12e1w
; CHECK: at s12e1w
  at s12e0r
; CHECK: at s12e0r
  at s12e0w
; CHECK: at s12e0w

  tlbi vmalle1is
; CHECK: tlbi vmalle1is
  tlbi alle2is
; CHECK: tlbi alle2is
  tlbi alle3is
; CHECK: tlbi alle3is
  tlbi vae1is
; CHECK: tlbi vae1is
  tlbi vae2is
; CHECK: tlbi vae2is
  tlbi vae3is
; CHECK: tlbi vae3is
  tlbi aside1is
; CHECK: tlbi aside1is
  tlbi vaae1is
; CHECK: tlbi vaae1is
  tlbi alle1is
; CHECK: tlbi alle1is
  tlbi vale1is
; CHECK: tlbi vale1is
  tlbi vaale1is
; CHECK: tlbi vaale1is
  tlbi vmalle1
; CHECK: tlbi vmalle1
  tlbi alle2
; CHECK: tlbi alle2
  tlbi vale2is
; CHECK: tlbi vale2is
  tlbi vale3is
; CHECK: tlbi vale3is
  tlbi alle3
; CHECK: tlbi alle3
  tlbi vae1
; CHECK: tlbi vae1
  tlbi vae2
; CHECK: tlbi vae2
  tlbi vae3
; CHECK: tlbi vae3
  tlbi aside1
; CHECK: tlbi aside1
  tlbi vaae1
; CHECK: tlbi vaae1
  tlbi alle1
; CHECK: tlbi alle1
  tlbi vale1
; CHECK: tlbi vale1
  tlbi vale2
; CHECK: tlbi vale2
  tlbi vale3
; CHECK: tlbi vale3
  tlbi vaale1
; CHECK: tlbi vaale1
  tlbi ipas2e1, x10
; CHECK: tlbi ipas2e1, x10
  tlbi ipas2le1, x1
; CHECK: tlbi ipas2le1, x1
  tlbi vmalls12e1
; CHECK: tlbi vmalls12e1
  tlbi vmalls12e1is
; CHECK: tlbi vmalls12e1is

;-----------------------------------------------------------------------------
; 5.8.5 Vector Arithmetic aliases
;-----------------------------------------------------------------------------

  cmls.8b v0, v2, v1
  cmls.16b v0, v2, v1
  cmls.4h v0, v2, v1
  cmls.8h v0, v2, v1
  cmls.2s v0, v2, v1
  cmls.4s v0, v2, v1
  cmls.2d v0, v2, v1
; CHECK: cmhs.8b v0, v1, v2
; CHECK: cmhs.16b v0, v1, v2
; CHECK: cmhs.4h v0, v1, v2
; CHECK: cmhs.8h v0, v1, v2
; CHECK: cmhs.2s v0, v1, v2
; CHECK: cmhs.4s v0, v1, v2
; CHECK: cmhs.2d v0, v1, v2

  cmlo.8b v0, v2, v1
  cmlo.16b v0, v2, v1
  cmlo.4h v0, v2, v1
  cmlo.8h v0, v2, v1
  cmlo.2s v0, v2, v1
  cmlo.4s v0, v2, v1
  cmlo.2d v0, v2, v1
; CHECK: cmhi.8b v0, v1, v2
; CHECK: cmhi.16b v0, v1, v2
; CHECK: cmhi.4h v0, v1, v2
; CHECK: cmhi.8h v0, v1, v2
; CHECK: cmhi.2s v0, v1, v2
; CHECK: cmhi.4s v0, v1, v2
; CHECK: cmhi.2d v0, v1, v2

  cmle.8b v0, v2, v1
  cmle.16b v0, v2, v1
  cmle.4h v0, v2, v1
  cmle.8h  v0, v2, v1
  cmle.2s v0, v2, v1
  cmle.4s v0, v2, v1
  cmle.2d v0, v2, v1
; CHECK: cmge.8b v0, v1, v2
; CHECK: cmge.16b v0, v1, v2
; CHECK: cmge.4h v0, v1, v2
; CHECK: cmge.8h v0, v1, v2
; CHECK: cmge.2s v0, v1, v2
; CHECK: cmge.4s v0, v1, v2
; CHECK: cmge.2d v0, v1, v2

  cmlt.8b v0, v2, v1
  cmlt.16b v0, v2, v1
  cmlt.4h v0, v2, v1
  cmlt.8h  v0, v2, v1
  cmlt.2s v0, v2, v1
  cmlt.4s v0, v2, v1
  cmlt.2d v0, v2, v1
; CHECK: cmgt.8b v0, v1, v2
; CHECK: cmgt.16b v0, v1, v2
; CHECK: cmgt.4h v0, v1, v2
; CHECK: cmgt.8h v0, v1, v2
; CHECK: cmgt.2s v0, v1, v2
; CHECK: cmgt.4s v0, v1, v2
; CHECK: cmgt.2d v0, v1, v2

  fcmle.2s v0, v2, v1
  fcmle.4s v0, v2, v1
  fcmle.2d v0, v2, v1
; CHECK: fcmge.2s v0, v1, v2
; CHECK: fcmge.4s v0, v1, v2
; CHECK: fcmge.2d v0, v1, v2

  fcmlt.2s v0, v2, v1
  fcmlt.4s v0, v2, v1
  fcmlt.2d v0, v2, v1
; CHECK: fcmgt.2s v0, v1, v2
; CHECK: fcmgt.4s v0, v1, v2
; CHECK: fcmgt.2d v0, v1, v2

  facle.2s v0, v2, v1
  facle.4s v0, v2, v1
  facle.2d v0, v2, v1
; CHECK: facge.2s v0, v1, v2
; CHECK: facge.4s v0, v1, v2
; CHECK: facge.2d v0, v1, v2

  faclt.2s v0, v2, v1
  faclt.4s v0, v2, v1
  faclt.2d v0, v2, v1
; CHECK: facgt.2s v0, v1, v2
; CHECK: facgt.4s v0, v1, v2
; CHECK: facgt.2d v0, v1, v2

;-----------------------------------------------------------------------------
; 5.8.6 Scalar Arithmetic aliases
;-----------------------------------------------------------------------------

  cmls d0, d2, d1
; CHECK: cmhs d0, d1, d2

  cmle d0, d2, d1
; CHECK: cmge d0, d1, d2

  cmlo d0, d2, d1
; CHECK: cmhi d0, d1, d2

  cmlt d0, d2, d1
; CHECK: cmgt d0, d1, d2

  fcmle s0, s2, s1
  fcmle d0, d2, d1
; CHECK: fcmge s0, s1, s2
; CHECK: fcmge d0, d1, d2

  fcmlt s0, s2, s1
  fcmlt d0, d2, d1
; CHECK: fcmgt s0, s1, s2
; CHECK: fcmgt d0, d1, d2

  facle s0, s2, s1
  facle d0, d2, d1
; CHECK: facge s0, s1, s2
; CHECK: facge d0, d1, d2

  faclt s0, s2, s1
  faclt d0, d2, d1
; CHECK: facgt s0, s1, s2
; CHECK: facgt d0, d1, d2

;-----------------------------------------------------------------------------
; 5.8.14 Vector Shift (immediate)
;-----------------------------------------------------------------------------
  sxtl v1.8h, v2.8b
; CHECK: sshll.8h v1, v2, #0
  sxtl.8h v1, v2
; CHECK: sshll.8h v1, v2, #0

  sxtl v1.4s, v2.4h
; CHECK: sshll.4s v1, v2, #0
  sxtl.4s v1, v2
; CHECK: sshll.4s v1, v2, #0

  sxtl v1.2d, v2.2s
; CHECK: sshll.2d v1, v2, #0
  sxtl.2d v1, v2
; CHECK: sshll.2d v1, v2, #0

  sxtl2 v1.8h, v2.16b
; CHECK: sshll2.8h v1, v2, #0
  sxtl2.8h v1, v2
; CHECK: sshll2.8h v1, v2, #0

  sxtl2 v1.4s, v2.8h
; CHECK: sshll2.4s v1, v2, #0
  sxtl2.4s v1, v2
; CHECK: sshll2.4s v1, v2, #0

  sxtl2 v1.2d, v2.4s
; CHECK: sshll2.2d v1, v2, #0
  sxtl2.2d v1, v2
; CHECK: sshll2.2d v1, v2, #0

  uxtl v1.8h, v2.8b
; CHECK: ushll.8h v1, v2, #0
  uxtl.8h v1, v2
; CHECK: ushll.8h v1, v2, #0

  uxtl v1.4s, v2.4h
; CHECK: ushll.4s v1, v2, #0
  uxtl.4s v1, v2
; CHECK: ushll.4s v1, v2, #0

  uxtl v1.2d, v2.2s
; CHECK: ushll.2d v1, v2, #0
  uxtl.2d v1, v2
; CHECK: ushll.2d v1, v2, #0

  uxtl2 v1.8h, v2.16b
; CHECK: ushll2.8h v1, v2, #0
  uxtl2.8h v1, v2
; CHECK: ushll2.8h v1, v2, #0

  uxtl2 v1.4s, v2.8h
; CHECK: ushll2.4s v1, v2, #0
  uxtl2.4s v1, v2
; CHECK: ushll2.4s v1, v2, #0

  uxtl2 v1.2d, v2.4s
; CHECK: ushll2.2d v1, v2, #0
  uxtl2.2d v1, v2
; CHECK: ushll2.2d v1, v2, #0


;-----------------------------------------------------------------------------
; MOVI verbose syntax with shift operand omitted.
;-----------------------------------------------------------------------------
  movi v4.16b, #0x00
  movi v4.16B, #0x01
  movi v4.8b, #0x02
  movi v4.8B, #0x03
  movi v1.2d, #0x000000000000ff
  movi v2.2D, #0x000000000000ff

; CHECK: movi.16b	v4, #0              ; encoding: [0x04,0xe4,0x00,0x4f]
; CHECK: movi.16b	v4, #1              ; encoding: [0x24,0xe4,0x00,0x4f]
; CHECK: movi.8b	v4, #2               ; encoding: [0x44,0xe4,0x00,0x0f]
; CHECK: movi.8b	v4, #3               ; encoding: [0x64,0xe4,0x00,0x0f]
; CHECK: movi.2d	v1, #0x000000000000ff ; encoding: [0x21,0xe4,0x00,0x6f]
; CHECK: movi.2d	v2, #0x000000000000ff ; encoding: [0x22,0xe4,0x00,0x6f]
