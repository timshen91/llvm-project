# RUN: llvm-mc %s -triple=mips-unknown-linux -show-encoding -mcpu=mips32r6 -mattr=micromips | FileCheck %s

  .set noat
  add $3, $4, $5           # CHECK: add $3, $4, $5      # encoding: [0x00,0xa4,0x19,0x10]
  addiu $3, $4, 1234       # CHECK: addiu $3, $4, 1234  # encoding: [0x30,0x64,0x04,0xd2]
  addu $3, $4, $5          # CHECK: addu $3, $4, $5     # encoding: [0x00,0xa4,0x19,0x50]
  addiupc $4, 100          # CHECK: addiupc $4, 100     # encoding: [0x78,0x80,0x00,0x19]
  addiur1sp $7, 4          # CHECK: addiur1sp $7, 4     # encoding: [0x6f,0x83]
  addiur2 $6, $7, -1       # CHECK: addiur2 $6, $7, -1  # encoding: [0x6f,0x7e]
  addiur2 $6, $7, 12       # CHECK: addiur2 $6, $7, 12  # encoding: [0x6f,0x76]
  addius5 $7, -2           # CHECK: addius5 $7, -2      # encoding: [0x4c,0xfc]
  addiusp -1028            # CHECK: addiusp -1028       # encoding: [0x4f,0xff]
  addiusp -1032            # CHECK: addiusp -1032       # encoding: [0x4f,0xfd]
  addiusp 1024             # CHECK: addiusp 1024        # encoding: [0x4c,0x01]
  addiusp 1028             # CHECK: addiusp 1028        # encoding: [0x4c,0x03]
  addiusp -16              # CHECK: addiusp -16         # encoding: [0x4f,0xf9]
  aluipc $3, 56            # CHECK: aluipc $3, 56       # encoding: [0x78,0x7f,0x00,0x38]
  and $3, $4, $5           # CHECK: and $3, $4, $5      # encoding: [0x00,0xa4,0x1a,0x50]
  andi $3, $4, 1234        # CHECK: andi $3, $4, 1234   # encoding: [0xd0,0x64,0x04,0xd2]
  auipc $3, -1             # CHECK: auipc $3, -1        # encoding: [0x78,0x7e,0xff,0xff]
  align $4, $2, $3, 2      # CHECK: align $4, $2, $3, 2 # encoding: [0x00,0x43,0x24,0x1f]
  aui $3,$2,-23            # CHECK: aui $3, $2, -23     # encoding: [0x10,0x62,0xff,0xe9]
  beqzalc $2, 1332         # CHECK: beqzalc $2, 1332    # encoding: [0x74,0x40,0x02,0x9a]
  bnezalc $2, 1332         # CHECK: bnezalc $2, 1332    # encoding: [0x7c,0x40,0x02,0x9a]
  bgezalc $2, 1332         # CHECK: bgezalc $2, 1332    # encoding: [0xc0,0x42,0x02,0x9a]
  bgtzalc $2, 1332         # CHECK: bgtzalc $2, 1332    # encoding: [0xe0,0x40,0x02,0x9a]
  bltzalc $2, 1332         # CHECK: bltzalc $2, 1332    # encoding: [0xe0,0x42,0x02,0x9a]
  blezalc $2, 1332         # CHECK: blezalc $2, 1332    # encoding: [0xc0,0x40,0x02,0x9a]
  balc 14572256            # CHECK: balc 14572256       # encoding: [0xb4,0x37,0x96,0xb8]
  b 132                    # CHECK: bc16 132            # encoding: [0xcc,0x42]
  bc 14572256              # CHECK: bc 14572256         # encoding: [0x94,0x37,0x96,0xb8]
  bc16 132                 # CHECK: bc16 132            # encoding: [0xcc,0x42]
  beqzc16 $6, 20           # CHECK: beqzc16 $6, 20      # encoding: [0x8f,0x0a]
  bnezc16 $6, 20           # CHECK: bnezc16 $6, 20      # encoding: [0xaf,0x0a]
  bitswap $4, $2           # CHECK: bitswap $4, $2      # encoding: [0x00,0x44,0x0b,0x3c]
  break                    # CHECK: break               # encoding: [0x00,0x00,0x00,0x07]
  break 7                  # CHECK: break 7             # encoding: [0x00,0x07,0x00,0x07]
  break 7, 5               # CHECK: break 7, 5          # encoding: [0x00,0x07,0x01,0x47]
  cache 1, 8($5)           # CHECK: cache 1, 8($5)      # encoding: [0x20,0x25,0x60,0x08]
  clo $11, $a1             # CHECK: clo $11, $5         # encoding: [0x01,0x65,0x4b,0x3c]
  clz $sp, $gp             # CHECK: clz $sp, $gp        # encoding: [0x03,0x80,0xe8,0x50]
  div $3, $4, $5           # CHECK: div $3, $4, $5      # encoding: [0x00,0xa4,0x19,0x18]
  divu $3, $4, $5          # CHECK: divu $3, $4, $5     # encoding: [0x00,0xa4,0x19,0x98]
  ehb                      # CHECK: ehb                 # encoding: [0x00,0x00,0x18,0x00]
  ei                       # CHECK: ei                  # encoding: [0x00,0x00,0x57,0x7c]
  ei $0                    # CHECK: ei                  # encoding: [0x00,0x00,0x57,0x7c]
  ei $10                   # CHECK: ei $10              # encoding: [0x00,0x0a,0x57,0x7c]
  eret                     # CHECK: eret                # encoding: [0x00,0x00,0xf3,0x7c]
  eretnc                   # CHECK: eretnc              # encoding: [0x00,0x01,0xf3,0x7c]
  jialc $5, 256            # CHECK: jialc $5, 256       # encoding: [0x80,0x05,0x01,0x00]
  jic   $5, 256            # CHECK: jic $5, 256         # encoding: [0xa0,0x05,0x01,0x00]
  lsa $2, $3, $4, 3        # CHECK: lsa  $2, $3, $4, 3  # encoding: [0x00,0x43,0x26,0x0f]
  lwpc    $2,268           # CHECK: lwpc $2, 268        # encoding: [0x78,0x48,0x00,0x43]
  mod $3, $4, $5           # CHECK: mod $3, $4, $5      # encoding: [0x00,0xa4,0x19,0x58]
  modu $3, $4, $5          # CHECK: modu $3, $4, $5     # encoding: [0x00,0xa4,0x19,0xd8]
  mul $3, $4, $5           # CHECK mul $3, $4, $5       # encoding: [0x00,0xa4,0x18,0x18]
  muh $3, $4, $5           # CHECK muh $3, $4, $5       # encoding: [0x00,0xa4,0x18,0x58]
  mulu $3, $4, $5          # CHECK mulu $3, $4, $5      # encoding: [0x00,0xa4,0x18,0x98]
  muhu $3, $4, $5          # CHECK muhu $3, $4, $5      # encoding: [0x00,0xa4,0x18,0xd8]
  nop                      # CHECK: nop                 # encoding: [0x00,0x00,0x00,0x00]
  nor $3, $4, $5           # CHECK: nor $3, $4, $5      # encoding: [0x00,0xa4,0x1a,0xd0]
  or $3, $4, $5            # CHECK: or $3, $4, $5       # encoding: [0x00,0xa4,0x1a,0x90]
  ori $3, $4, 1234         # CHECK: ori $3, $4, 1234    # encoding: [0x50,0x64,0x04,0xd2]
  pref 1, 8($5)            # CHECK: pref 1, 8($5)       # encoding: [0x60,0x25,0x20,0x08]
  seb $3, $4               # CHECK: seb $3, $4          # encoding: [0x00,0x64,0x2b,0x3c]
  seh $3, $4               # CHECK: seh $3, $4          # encoding: [0x00,0x64,0x3b,0x3c]
  seleqz $2,$3,$4          # CHECK: seleqz $2, $3, $4   # encoding: [0x00,0x83,0x11,0x40]
  selnez $2,$3,$4          # CHECK: selnez $2, $3, $4   # encoding: [0x00,0x83,0x11,0x80]
  sll $4, $3, 7            # CHECK: sll $4, $3, 7       # encoding: [0x00,0x83,0x38,0x00]
  sub $3, $4, $5           # CHECK: sub $3, $4, $5      # encoding: [0x00,0xa4,0x19,0x90]
  subu $3, $4, $5          # CHECK: subu $3, $4, $5     # encoding: [0x00,0xa4,0x19,0xd0]
  xor $3, $4, $5           # CHECK: xor $3, $4, $5      # encoding: [0x00,0xa4,0x1b,0x10]
  xori $3, $4, 1234        # CHECK: xori $3, $4, 1234   # encoding: [0x70,0x64,0x04,0xd2]
  sw $5, 4($6)             # CHECK: sw $5, 4($6)        # encoding: [0xf8,0xa6,0x00,0x04]
  swe $5, 8($4)            # CHECK: swe $5, 8($4)       # encoding: [0x60,0xa4,0xae,0x08]
  add.s $f3, $f4, $f5      # CHECK: add.s $f3, $f4, $f5 # encoding: [0x54,0xa4,0x18,0x30]
  add.d $f2, $f4, $f6      # CHECK: add.d $f2, $f4, $f6 # encoding: [0x54,0xc4,0x11,0x30]
  sub.s $f3, $f4, $f5      # CHECK: sub.s $f3, $f4, $f5 # encoding: [0x54,0xa4,0x18,0x70]
  sub.d $f2, $f4, $f6      # CHECK: sub.d $f2, $f4, $f6 # encoding: [0x54,0xc4,0x11,0x70]
  mul.s $f3, $f4, $f5      # CHECK: mul.s $f3, $f4, $f5 # encoding: [0x54,0xa4,0x18,0xb0]
  mul.d $f2, $f4, $f6      # CHECK: mul.d $f2, $f4, $f6 # encoding: [0x54,0xc4,0x11,0xb0]
  div.s $f3, $f4, $f5      # CHECK: div.s $f3, $f4, $f5 # encoding: [0x54,0xa4,0x18,0xf0]
  div.d $f2, $f4, $f6      # CHECK: div.d $f2, $f4, $f6 # encoding: [0x54,0xc4,0x11,0xf0]
  maddf.s $f3, $f4, $f5    # CHECK: maddf.s $f3, $f4, $f5 # encoding: [0x54,0xa4,0x19,0xb8]
  maddf.d $f3, $f4, $f5    # CHECK: maddf.d $f3, $f4, $f5 # encoding: [0x54,0xa4,0x1b,0xb8]
  msubf.s $f3, $f4, $f5    # CHECK: msubf.s $f3, $f4, $f5 # encoding: [0x54,0xa4,0x19,0xf8]
  msubf.d $f3, $f4, $f5    # CHECK: msubf.d $f3, $f4, $f5 # encoding: [0x54,0xa4,0x1b,0xf8]
  mov.s $f6, $f7           # CHECK: mov.s $f6, $f7      # encoding: [0x54,0xc7,0x00,0x7b]
  mov.d $f4, $f6           # CHECK: mov.d $f4, $f6      # encoding: [0x54,0x86,0x20,0x7b]
  neg.s $f6, $f7           # CHECK: neg.s $f6, $f7      # encoding: [0x54,0xc7,0x0b,0x7b]
  neg.d $f4, $f6           # CHECK: neg.d $f4, $f6      # encoding: [0x54,0x86,0x2b,0x7b]
  max.s $f5, $f4, $f3      # CHECK: max.s $f5, $f4, $f3      # encoding: [0x54,0x64,0x28,0x0b]
  max.d $f5, $f4, $f3      # CHECK: max.d $f5, $f4, $f3      # encoding: [0x54,0x64,0x2a,0x0b]
  maxa.s $f5, $f4, $f3     # CHECK: maxa.s $f5, $f4, $f3     # encoding: [0x54,0x64,0x28,0x2b]
  maxa.d $f5, $f4, $f3     # CHECK: maxa.d $f5, $f4, $f3     # encoding: [0x54,0x64,0x2a,0x2b]
  min.s $f5, $f4, $f3      # CHECK: min.s $f5, $f4, $f3      # encoding: [0x54,0x64,0x28,0x03]
  min.d $f5, $f4, $f3      # CHECK: min.d $f5, $f4, $f3      # encoding: [0x54,0x64,0x2a,0x03]
  mina.s $f5, $f4, $f3     # CHECK: mina.s $f5, $f4, $f3     # encoding: [0x54,0x64,0x28,0x23]
  mina.d $f5, $f4, $f3     # CHECK: mina.d $f5, $f4, $f3     # encoding: [0x54,0x64,0x2a,0x23]
  cmp.af.s $f2, $f3, $f4   # CHECK: cmp.af.s $f2, $f3, $f4   # encoding: [0x54,0x83,0x10,0x05]
  cmp.af.d $f2, $f3, $f4   # CHECK: cmp.af.d $f2, $f3, $f4   # encoding: [0x54,0x83,0x10,0x15]
  cmp.un.s $f2, $f3, $f4   # CHECK: cmp.un.s $f2, $f3, $f4   # encoding: [0x54,0x83,0x10,0x45]
  cmp.un.d $f2, $f3, $f4   # CHECK: cmp.un.d $f2, $f3, $f4   # encoding: [0x54,0x83,0x10,0x55]
  cmp.eq.s $f2, $f3, $f4   # CHECK: cmp.eq.s $f2, $f3, $f4   # encoding: [0x54,0x83,0x10,0x85]
  cmp.eq.d $f2, $f3, $f4   # CHECK: cmp.eq.d $f2, $f3, $f4   # encoding: [0x54,0x83,0x10,0x95]
  cmp.ueq.s $f2, $f3, $f4  # CHECK: cmp.ueq.s $f2, $f3, $f4  # encoding: [0x54,0x83,0x10,0xc5]
  cmp.ueq.d $f2, $f3, $f4  # CHECK: cmp.ueq.d $f2, $f3, $f4  # encoding: [0x54,0x83,0x10,0xd5]
  cmp.lt.s $f2, $f3, $f4   # CHECK: cmp.lt.s  $f2, $f3, $f4  # encoding: [0x54,0x83,0x11,0x05]
  cmp.lt.d $f2, $f3, $f4   # CHECK: cmp.lt.d  $f2, $f3, $f4  # encoding: [0x54,0x83,0x11,0x15]
  cmp.ult.s $f2, $f3, $f4  # CHECK: cmp.ult.s $f2, $f3, $f4  # encoding: [0x54,0x83,0x11,0x45]
  cmp.ult.d $f2, $f3, $f4  # CHECK: cmp.ult.d $f2, $f3, $f4  # encoding: [0x54,0x83,0x11,0x55]
  cmp.le.s $f2, $f3, $f4   # CHECK: cmp.le.s  $f2, $f3, $f4  # encoding: [0x54,0x83,0x11,0x85]
  cmp.le.d $f2, $f3, $f4   # CHECK: cmp.le.d  $f2, $f3, $f4  # encoding: [0x54,0x83,0x11,0x95]
  cmp.ule.s $f2, $f3, $f4  # CHECK: cmp.ule.s $f2, $f3, $f4  # encoding: [0x54,0x83,0x11,0xc5]
  cmp.ule.d $f2, $f3, $f4  # CHECK: cmp.ule.d $f2, $f3, $f4  # encoding: [0x54,0x83,0x11,0xd5]
  cmp.saf.s $f2, $f3, $f4  # CHECK: cmp.saf.s $f2, $f3, $f4  # encoding: [0x54,0x83,0x12,0x05]
  cmp.saf.d $f2, $f3, $f4  # CHECK: cmp.saf.d $f2, $f3, $f4  # encoding: [0x54,0x83,0x12,0x15]
  cmp.sun.s $f2, $f3, $f4  # CHECK: cmp.sun.s $f2, $f3, $f4  # encoding: [0x54,0x83,0x12,0x45]
  cmp.sun.d $f2, $f3, $f4  # CHECK: cmp.sun.d $f2, $f3, $f4  # encoding: [0x54,0x83,0x12,0x55]
  cmp.seq.s $f2, $f3, $f4  # CHECK: cmp.seq.s $f2, $f3, $f4  # encoding: [0x54,0x83,0x12,0x85]
  cmp.seq.d $f2, $f3, $f4  # CHECK: cmp.seq.d $f2, $f3, $f4  # encoding: [0x54,0x83,0x12,0x95]
  cmp.sueq.s $f2, $f3, $f4 # CHECK: cmp.sueq.s $f2, $f3, $f4 # encoding: [0x54,0x83,0x12,0xc5]
  cmp.sueq.d $f2, $f3, $f4 # CHECK: cmp.sueq.d $f2, $f3, $f4 # encoding: [0x54,0x83,0x12,0xd5]
  cmp.slt.s $f2, $f3, $f4  # CHECK: cmp.slt.s $f2, $f3, $f4  # encoding: [0x54,0x83,0x13,0x05]
  cmp.slt.d $f2, $f3, $f4  # CHECK: cmp.slt.d $f2, $f3, $f4  # encoding: [0x54,0x83,0x13,0x15]
  cmp.sult.s $f2, $f3, $f4 # CHECK: cmp.sult.s $f2, $f3, $f4 # encoding: [0x54,0x83,0x13,0x45]
  cmp.sult.d $f2, $f3, $f4 # CHECK: cmp.sult.d $f2, $f3, $f4 # encoding: [0x54,0x83,0x13,0x55]
  cmp.sle.s $f2, $f3, $f4  # CHECK: cmp.sle.s $f2, $f3, $f4  # encoding: [0x54,0x83,0x13,0x85]
  cmp.sle.d $f2, $f3, $f4  # CHECK: cmp.sle.d $f2, $f3, $f4  # encoding: [0x54,0x83,0x13,0x95]
  cmp.sule.s $f2, $f3, $f4 # CHECK: cmp.sule.s $f2, $f3, $f4 # encoding: [0x54,0x83,0x13,0xc5]
  cmp.sule.d $f2, $f3, $f4 # CHECK: cmp.sule.d $f2, $f3, $f4 # encoding: [0x54,0x83,0x13,0xd5]
  cvt.l.s $f3, $f4         # CHECK: cvt.l.s $f3, $f4         # encoding: [0x54,0x64,0x01,0x3b]
  cvt.l.d $f3, $f4         # CHECK: cvt.l.d $f3, $f4         # encoding: [0x54,0x64,0x41,0x3b]
  cvt.w.s $f3, $f4         # CHECK: cvt.w.s $f3, $f4         # encoding: [0x54,0x64,0x09,0x3b]
  cvt.w.d $f3, $f4         # CHECK: cvt.w.d $f3, $f4         # encoding: [0x54,0x64,0x49,0x3b]
  cvt.d.s $f2, $f4         # CHECK: cvt.d.s $f2, $f4         # encoding: [0x54,0x44,0x13,0x7b]
  cvt.d.w $f2, $f4         # CHECK: cvt.d.w $f2, $f4         # encoding: [0x54,0x44,0x33,0x7b]
  cvt.d.l $f2, $f4         # CHECK: cvt.d.l $f2, $f4         # encoding: [0x54,0x44,0x53,0x7b]
  cvt.s.d $f2, $f4         # CHECK: cvt.s.d $f2, $f4         # encoding: [0x54,0x44,0x1b,0x7b]
  cvt.s.w $f3, $f4         # CHECK: cvt.s.w $f3, $f4         # encoding: [0x54,0x64,0x3b,0x7b]
  cvt.s.l $f3, $f4         # CHECK: cvt.s.l $f3, $f4         # encoding: [0x54,0x64,0x5b,0x7b]
  abs.s $f3, $f5           # CHECK: abs.s $f3, $f5      # encoding: [0x54,0x65,0x03,0x7b]
  abs.d $f2, $f4           # CHECK: abs.d $f2, $f4      # encoding: [0x54,0x44,0x23,0x7b]
  floor.l.s $f3, $f5       # CHECK: floor.l.s $f3, $f5  # encoding: [0x54,0x65,0x03,0x3b]
  floor.l.d $f2, $f4       # CHECK: floor.l.d $f2, $f4  # encoding: [0x54,0x44,0x43,0x3b]
  floor.w.s $f3, $f5       # CHECK: floor.w.s $f3, $f5  # encoding: [0x54,0x65,0x0b,0x3b]
  floor.w.d $f2, $f4       # CHECK: floor.w.d $f2, $f4  # encoding: [0x54,0x44,0x4b,0x3b]
  ceil.l.s $f3, $f5        # CHECK: ceil.l.s $f3, $f5   # encoding: [0x54,0x65,0x13,0x3b]
  ceil.l.d $f2, $f4        # CHECK: ceil.l.d $f2, $f4   # encoding: [0x54,0x44,0x53,0x3b]
  ceil.w.s $f3, $f5        # CHECK: ceil.w.s $f3, $f5   # encoding: [0x54,0x65,0x1b,0x3b]
  ceil.w.d $f2, $f4        # CHECK: ceil.w.d $f2, $f4   # encoding: [0x54,0x44,0x5b,0x3b]
  trunc.l.s $f3, $f5       # CHECK: trunc.l.s $f3, $f5  # encoding: [0x54,0x65,0x23,0x3b]
  trunc.l.d $f2, $f4       # CHECK: trunc.l.d $f2, $f4  # encoding: [0x54,0x44,0x63,0x3b]
  trunc.w.s $f3, $f5       # CHECK: trunc.w.s $f3, $f5  # encoding: [0x54,0x65,0x2b,0x3b]
  trunc.w.d $f2, $f4       # CHECK: trunc.w.d $f2, $f4  # encoding: [0x54,0x44,0x6b,0x3b]
  sqrt.s $f3, $f5          # CHECK: sqrt.s $f3, $f5     # encoding: [0x54,0x65,0x0a,0x3b]
  sqrt.d $f2, $f4          # CHECK: sqrt.d $f2, $f4     # encoding: [0x54,0x44,0x4a,0x3b]
  rsqrt.s $f3, $f5         # CHECK: rsqrt.s $f3, $f5    # encoding: [0x54,0x65,0x02,0x3b]
  rsqrt.d $f2, $f4         # CHECK: rsqrt.d $f2, $f4    # encoding: [0x54,0x44,0x42,0x3b]
