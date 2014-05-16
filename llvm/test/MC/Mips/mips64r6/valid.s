# Instructions that are valid
#
# RUN: llvm-mc %s -triple=mips-unknown-linux -show-encoding -mcpu=mips64r6 | FileCheck %s

        .set noat
        # FIXME: Add the instructions carried forward from older ISA's
        addiupc $4, 100          # CHECK: addiupc $4, 100     # encoding: [0xec,0x80,0x00,0x19]
        align   $4, $2, $3, 2    # CHECK: align $4, $2, $3, 2 # encoding: [0x7c,0x43,0x22,0xa0]
        aluipc  $3, 56           # CHECK: aluipc $3, 56       # encoding: [0xec,0x7f,0x00,0x38]
        aui     $3,$2,-23        # CHECK: aui $3, $2, -23     # encoding: [0x3c,0x62,0xff,0xe9]
        auipc   $3, -1           # CHECK: auipc $3, -1        # encoding: [0xec,0x7e,0xff,0xff]
        bitswap $4, $2           # CHECK: bitswap $4, $2      # encoding: [0x7c,0x02,0x20,0x20]
        cmp.f.s    $f2,$f3,$f4      # CHECK: cmp.f.s $f2, $f3, $f4  # encoding: [0x46,0x84,0x18,0x80]
        cmp.f.d    $f2,$f3,$f4      # CHECK: cmp.f.d $f2, $f3, $f4  # encoding: [0x46,0xa4,0x18,0x80]
        cmp.un.s   $f2,$f3,$f4      # CHECK: cmp.un.s $f2, $f3, $f4  # encoding: [0x46,0x84,0x18,0x81]
        cmp.un.d   $f2,$f3,$f4      # CHECK: cmp.un.d $f2, $f3, $f4  # encoding: [0x46,0xa4,0x18,0x81]
        cmp.eq.s   $f2,$f3,$f4      # CHECK: cmp.eq.s $f2, $f3, $f4  # encoding: [0x46,0x84,0x18,0x82]
        cmp.eq.d   $f2,$f3,$f4      # CHECK: cmp.eq.d $f2, $f3, $f4  # encoding: [0x46,0xa4,0x18,0x82]
        cmp.ueq.s  $f2,$f3,$f4      # CHECK: cmp.ueq.s $f2, $f3, $f4  # encoding: [0x46,0x84,0x18,0x83]
        cmp.ueq.d  $f2,$f3,$f4      # CHECK: cmp.ueq.d $f2, $f3, $f4  # encoding: [0x46,0xa4,0x18,0x83]
        cmp.olt.s  $f2,$f3,$f4      # CHECK: cmp.olt.s $f2, $f3, $f4  # encoding: [0x46,0x84,0x18,0x84]
        cmp.olt.d  $f2,$f3,$f4      # CHECK: cmp.olt.d $f2, $f3, $f4  # encoding: [0x46,0xa4,0x18,0x84]
        cmp.ult.s  $f2,$f3,$f4      # CHECK: cmp.ult.s $f2, $f3, $f4  # encoding: [0x46,0x84,0x18,0x85]
        cmp.ult.d  $f2,$f3,$f4      # CHECK: cmp.ult.d $f2, $f3, $f4  # encoding: [0x46,0xa4,0x18,0x85]
        cmp.ole.s  $f2,$f3,$f4      # CHECK: cmp.ole.s $f2, $f3, $f4  # encoding: [0x46,0x84,0x18,0x86]
        cmp.ole.d  $f2,$f3,$f4      # CHECK: cmp.ole.d $f2, $f3, $f4  # encoding: [0x46,0xa4,0x18,0x86]
        cmp.ule.s  $f2,$f3,$f4      # CHECK: cmp.ule.s $f2, $f3, $f4  # encoding: [0x46,0x84,0x18,0x87]
        cmp.ule.d  $f2,$f3,$f4      # CHECK: cmp.ule.d $f2, $f3, $f4  # encoding: [0x46,0xa4,0x18,0x87]
        cmp.sf.s   $f2,$f3,$f4      # CHECK: cmp.sf.s $f2, $f3, $f4  # encoding: [0x46,0x84,0x18,0x88]
        cmp.sf.d   $f2,$f3,$f4      # CHECK: cmp.sf.d $f2, $f3, $f4  # encoding: [0x46,0xa4,0x18,0x88]
        cmp.ngle.s $f2,$f3,$f4      # CHECK: cmp.ngle.s $f2, $f3, $f4  # encoding: [0x46,0x84,0x18,0x89]
        cmp.ngle.d $f2,$f3,$f4      # CHECK: cmp.ngle.d $f2, $f3, $f4  # encoding: [0x46,0xa4,0x18,0x89]
        cmp.seq.s  $f2,$f3,$f4      # CHECK: cmp.seq.s $f2, $f3, $f4  # encoding: [0x46,0x84,0x18,0x8a]
        cmp.seq.d  $f2,$f3,$f4      # CHECK: cmp.seq.d $f2, $f3, $f4  # encoding: [0x46,0xa4,0x18,0x8a]
        cmp.ngl.s  $f2,$f3,$f4      # CHECK: cmp.ngl.s $f2, $f3, $f4  # encoding: [0x46,0x84,0x18,0x8b]
        cmp.ngl.d  $f2,$f3,$f4      # CHECK: cmp.ngl.d $f2, $f3, $f4  # encoding: [0x46,0xa4,0x18,0x8b]
        cmp.lt.s   $f2,$f3,$f4      # CHECK: cmp.lt.s $f2, $f3, $f4  # encoding: [0x46,0x84,0x18,0x8c]
        cmp.lt.d   $f2,$f3,$f4      # CHECK: cmp.lt.d $f2, $f3, $f4  # encoding: [0x46,0xa4,0x18,0x8c]
        cmp.nge.s  $f2,$f3,$f4      # CHECK: cmp.nge.s $f2, $f3, $f4  # encoding: [0x46,0x84,0x18,0x8d]
        cmp.nge.d  $f2,$f3,$f4      # CHECK: cmp.nge.d $f2, $f3, $f4  # encoding: [0x46,0xa4,0x18,0x8d]
        cmp.le.s   $f2,$f3,$f4      # CHECK: cmp.le.s $f2, $f3, $f4  # encoding: [0x46,0x84,0x18,0x8e]
        cmp.le.d   $f2,$f3,$f4      # CHECK: cmp.le.d $f2, $f3, $f4  # encoding: [0x46,0xa4,0x18,0x8e]
        cmp.ngt.s  $f2,$f3,$f4      # CHECK: cmp.ngt.s $f2, $f3, $f4  # encoding: [0x46,0x84,0x18,0x8f]
        cmp.ngt.d  $f2,$f3,$f4      # CHECK: cmp.ngt.d $f2, $f3, $f4  # encoding: [0x46,0xa4,0x18,0x8f]
        dalign  $4,$2,$3,5       # CHECK: dalign $4, $2, $3, 5 # encoding: [0x7c,0x43,0x23,0x64]
        daui    $3,$2,0x1234     # CHECK: daui $3, $2, 4660  # encoding: [0x74,0x62,0x12,0x34]
        dahi    $3,$3,0x5678     # CHECK: dahi $3, $3, 22136 # encoding: [0x04,0x66,0x56,0x78]
        dati    $3,$3,0xabcd     # CHECK: dati $3, $3, 43981 # encoding: [0x04,0x7e,0xab,0xcd]
        dbitswap $4, $2           # CHECK: bitswap $4, $2      # encoding: [0x7c,0x02,0x20,0x24]
        div     $2,$3,$4         # CHECK: div $2, $3, $4   # encoding: [0x00,0x64,0x10,0x9a]
        divu    $2,$3,$4         # CHECK: divu $2, $3, $4  # encoding: [0x00,0x64,0x10,0x9b]
        mod     $2,$3,$4         # CHECK: mod $2, $3, $4   # encoding: [0x00,0x64,0x10,0xda]
        modu    $2,$3,$4         # CHECK: modu $2, $3, $4  # encoding: [0x00,0x64,0x10,0xdb]
        ddiv    $2,$3,$4         # CHECK: ddiv $2, $3, $4  # encoding: [0x00,0x64,0x10,0x9e]
        ddivu   $2,$3,$4         # CHECK: ddivu $2, $3, $4 # encoding: [0x00,0x64,0x10,0x9f]
        dmod    $2,$3,$4         # CHECK: dmod $2, $3, $4  # encoding: [0x00,0x64,0x10,0xde]
        dmodu   $2,$3,$4         # CHECK: dmodu $2, $3, $4 # encoding: [0x00,0x64,0x10,0xdf]
#        mul     $2,$3,$4         # CHECK-TODO: mul $2, $3, $4   # encoding: [0x00,0x64,0x10,0x98]
        muh     $2,$3,$4         # CHECK: muh $2, $3, $4   # encoding: [0x00,0x64,0x10,0xd8]
        mulu    $2,$3,$4         # CHECK: mulu $2, $3, $4  # encoding: [0x00,0x64,0x10,0x99]
        muhu    $2,$3,$4         # CHECK: muhu $2, $3, $4  # encoding: [0x00,0x64,0x10,0xd9]
        dmul    $2,$3,$4         # CHECK: dmul $2, $3, $4  # encoding: [0x00,0x64,0x10,0xb8]
        dmuh    $2,$3,$4         # CHECK: dmuh $2, $3, $4  # encoding: [0x00,0x64,0x10,0xf8]
        dmulu   $2,$3,$4         # CHECK: dmulu $2, $3, $4 # encoding: [0x00,0x64,0x10,0xb9]
        dmuhu   $2,$3,$4         # CHECK: dmuhu $2, $3, $4 # encoding: [0x00,0x64,0x10,0xf9]
        maddf.s $f2,$f3,$f4      # CHECK: maddf.s $f2, $f3, $f4  # encoding: [0x46,0x04,0x18,0x98]
        maddf.d $f2,$f3,$f4      # CHECK: maddf.d $f2, $f3, $f4  # encoding: [0x46,0x24,0x18,0x98]
        msubf.s $f2,$f3,$f4      # CHECK: msubf.s $f2, $f3, $f4  # encoding: [0x46,0x04,0x18,0x99]
        msubf.d $f2,$f3,$f4      # CHECK: msubf.d $f2, $f3, $f4  # encoding: [0x46,0x24,0x18,0x99]
        sel.d   $f0,$f1,$f2      # CHECK: sel.d $f0, $f1, $f2 # encoding: [0x46,0x22,0x08,0x10]
        sel.s   $f0,$f1,$f2      # CHECK: sel.s $f0, $f1, $f2 # encoding: [0x46,0x02,0x08,0x10]
        max.s   $f0, $f2, $f4    # CHECK: max.s $f0, $f2, $f4 # encoding: [0x46,0x04,0x10,0x1d]
        max.d   $f0, $f2, $f4    # CHECK: max.d $f0, $f2, $f4 # encoding: [0x46,0x24,0x10,0x1d]
        min.s   $f0, $f2, $f4    # CHECK: min.s $f0, $f2, $f4 # encoding: [0x46,0x04,0x10,0x1c]
        min.d   $f0, $f2, $f4    # CHECK: min.d $f0, $f2, $f4 # encoding: [0x46,0x24,0x10,0x1c]
        maxa.s  $f0, $f2, $f4    # CHECK: maxa.s $f0, $f2, $f4 # encoding: [0x46,0x04,0x10,0x1f]
        maxa.d  $f0, $f2, $f4    # CHECK: maxa.d $f0, $f2, $f4 # encoding: [0x46,0x24,0x10,0x1f]
        mina.s  $f0, $f2, $f4    # CHECK: mina.s $f0, $f2, $f4 # encoding: [0x46,0x04,0x10,0x1e]
        mina.d  $f0, $f2, $f4    # CHECK: mina.d $f0, $f2, $f4 # encoding: [0x46,0x24,0x10,0x1e]
        seleqz.s $f0, $f2, $f4   # CHECK: seleqz.s $f0, $f2, $f4 # encoding: [0x46,0x04,0x10,0x14]
        seleqz.d $f0, $f2, $f4   # CHECK: seleqz.d $f0, $f2, $f4 # encoding: [0x46,0x24,0x10,0x14]
        selnez.s $f0, $f2, $f4   # CHECK: selnez.s $f0, $f2, $f4 # encoding: [0x46,0x04,0x10,0x17]
        selnez.d $f0, $f2, $f4   # CHECK: selnez.d $f0, $f2, $f4 # encoding: [0x46,0x24,0x10,0x17]
        rint.s $f2, $f4          # CHECK: rint.s $f2, $f4        # encoding: [0x46,0x00,0x20,0x9a]
        rint.d $f2, $f4          # CHECK: rint.d $f2, $f4        # encoding: [0x46,0x20,0x20,0x9a]
        class.s $f2, $f4         # CHECK: class.s $f2, $f4       # encoding: [0x46,0x00,0x20,0x9b]
        class.d $f2, $f4         # CHECK: class.d $f2, $f4       # encoding: [0x46,0x20,0x20,0x9b]
