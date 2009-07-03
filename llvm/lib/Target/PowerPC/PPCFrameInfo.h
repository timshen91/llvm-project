//===-- PPCFrameInfo.h - Define TargetFrameInfo for PowerPC -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//

#ifndef POWERPC_FRAMEINFO_H
#define POWERPC_FRAMEINFO_H

#include "PPC.h"
#include "PPCSubtarget.h"
#include "llvm/Target/TargetFrameInfo.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/ADT/STLExtras.h"

namespace llvm {

class PPCFrameInfo: public TargetFrameInfo {
  const TargetMachine &TM;

public:
  PPCFrameInfo(const TargetMachine &tm, bool LP64)
    : TargetFrameInfo(TargetFrameInfo::StackGrowsDown, 16, 0), TM(tm) {
  }

  /// getReturnSaveOffset - Return the previous frame offset to save the
  /// return address.
  static unsigned getReturnSaveOffset(bool LP64, bool isDarwinABI) {
    if (isDarwinABI)
      return LP64 ? 16 : 8;
    // SVR4 ABI:
    return 4;
  }

  /// getFramePointerSaveOffset - Return the previous frame offset to save the
  /// frame pointer.
  static unsigned getFramePointerSaveOffset(bool LP64, bool isDarwinABI) {
    // For the Darwin ABI:
    // Use the TOC save slot in the PowerPC linkage area for saving the frame
    // pointer (if needed.)  LLVM does not generate code that uses the TOC (R2
    // is treated as a caller saved register.)
    if (isDarwinABI)
      return LP64 ? 40 : 20;
    
    // SVR4 ABI:
    // Save it right before the link register
    return -4U;
  }
  
  /// getLinkageSize - Return the size of the PowerPC ABI linkage area.
  ///
  static unsigned getLinkageSize(bool LP64, bool isDarwinABI) {
    if (isDarwinABI)
      return 6 * (LP64 ? 8 : 4);
    
    // SVR4 ABI:
    return 8;
  }

  /// getMinCallArgumentsSize - Return the size of the minium PowerPC ABI
  /// argument area.
  static unsigned getMinCallArgumentsSize(bool LP64, bool isDarwinABI) {
    // For the Darwin ABI:
    // The prolog code of the callee may store up to 8 GPR argument registers to
    // the stack, allowing va_start to index over them in memory if its varargs.
    // Because we cannot tell if this is needed on the caller side, we have to
    // conservatively assume that it is needed.  As such, make sure we have at
    // least enough stack space for the caller to store the 8 GPRs.
    if (isDarwinABI)
      return 8 * (LP64 ? 8 : 4);
    
    // SVR4 ABI:
    // There is no default stack allocated for the 8 first GPR arguments.
    return 0;
  }

  /// getMinCallFrameSize - Return the minimum size a call frame can be using
  /// the PowerPC ABI.
  static unsigned getMinCallFrameSize(bool LP64, bool isDarwinABI) {
    // The call frame needs to be at least big enough for linkage and 8 args.
    return getLinkageSize(LP64, isDarwinABI) +
           getMinCallArgumentsSize(LP64, isDarwinABI);
  }

  // With the SVR4 ABI, callee-saved registers have fixed offsets on the stack.
  const std::pair<unsigned, int> *
  getCalleeSavedSpillSlots(unsigned &NumEntries) const {
    // Early exit if not using the SVR4 ABI.
    if (!TM.getSubtarget<PPCSubtarget>().isSVR4ABI()) {
      NumEntries = 0;
      return 0;
    }
    
    static const std::pair<unsigned, int> Offsets[] = {
      // Floating-point register save area offsets.
      std::pair<unsigned, int>(PPC::F31, -8),
      std::pair<unsigned, int>(PPC::F30, -16),
      std::pair<unsigned, int>(PPC::F29, -24),
      std::pair<unsigned, int>(PPC::F28, -32),
      std::pair<unsigned, int>(PPC::F27, -40),
      std::pair<unsigned, int>(PPC::F26, -48),
      std::pair<unsigned, int>(PPC::F25, -56),
      std::pair<unsigned, int>(PPC::F24, -64),
      std::pair<unsigned, int>(PPC::F23, -72),
      std::pair<unsigned, int>(PPC::F22, -80),
      std::pair<unsigned, int>(PPC::F21, -88),
      std::pair<unsigned, int>(PPC::F20, -96),
      std::pair<unsigned, int>(PPC::F19, -104),
      std::pair<unsigned, int>(PPC::F18, -112),
      std::pair<unsigned, int>(PPC::F17, -120),
      std::pair<unsigned, int>(PPC::F16, -128),
      std::pair<unsigned, int>(PPC::F15, -136),
      std::pair<unsigned, int>(PPC::F14, -144),
        
      // General register save area offsets.
      std::pair<unsigned, int>(PPC::R31, -4),
      std::pair<unsigned, int>(PPC::R30, -8),
      std::pair<unsigned, int>(PPC::R29, -12),
      std::pair<unsigned, int>(PPC::R28, -16),
      std::pair<unsigned, int>(PPC::R27, -20),
      std::pair<unsigned, int>(PPC::R26, -24),
      std::pair<unsigned, int>(PPC::R25, -28),
      std::pair<unsigned, int>(PPC::R24, -32),
      std::pair<unsigned, int>(PPC::R23, -36),
      std::pair<unsigned, int>(PPC::R22, -40),
      std::pair<unsigned, int>(PPC::R21, -44),
      std::pair<unsigned, int>(PPC::R20, -48),
      std::pair<unsigned, int>(PPC::R19, -52),
      std::pair<unsigned, int>(PPC::R18, -56),
      std::pair<unsigned, int>(PPC::R17, -60),
      std::pair<unsigned, int>(PPC::R16, -64),
      std::pair<unsigned, int>(PPC::R15, -68),
      std::pair<unsigned, int>(PPC::R14, -72),

      // CR save area offset.
      std::pair<unsigned, int>(PPC::CR2, -4),
      std::pair<unsigned, int>(PPC::CR3, -4),
      std::pair<unsigned, int>(PPC::CR4, -4),
      std::pair<unsigned, int>(PPC::CR2LT, -4),
      std::pair<unsigned, int>(PPC::CR2GT, -4),
      std::pair<unsigned, int>(PPC::CR2EQ, -4),
      std::pair<unsigned, int>(PPC::CR2UN, -4),
      std::pair<unsigned, int>(PPC::CR3LT, -4),
      std::pair<unsigned, int>(PPC::CR3GT, -4),
      std::pair<unsigned, int>(PPC::CR3EQ, -4),
      std::pair<unsigned, int>(PPC::CR3UN, -4),
      std::pair<unsigned, int>(PPC::CR4LT, -4),
      std::pair<unsigned, int>(PPC::CR4GT, -4),
      std::pair<unsigned, int>(PPC::CR4EQ, -4),
      std::pair<unsigned, int>(PPC::CR4UN, -4),

      // VRSAVE save area offset.
      std::pair<unsigned, int>(PPC::VRSAVE, -4),
      
      // Vector register save area
      std::pair<unsigned, int>(PPC::V31, -16),
      std::pair<unsigned, int>(PPC::V30, -32),
      std::pair<unsigned, int>(PPC::V29, -48),
      std::pair<unsigned, int>(PPC::V28, -64),
      std::pair<unsigned, int>(PPC::V27, -80),
      std::pair<unsigned, int>(PPC::V26, -96),
      std::pair<unsigned, int>(PPC::V25, -112),
      std::pair<unsigned, int>(PPC::V24, -128),
      std::pair<unsigned, int>(PPC::V23, -144),
      std::pair<unsigned, int>(PPC::V22, -160),
      std::pair<unsigned, int>(PPC::V21, -176),
      std::pair<unsigned, int>(PPC::V20, -192)
    };
    
    NumEntries = array_lengthof(Offsets);
    
    return Offsets;
  }
};

} // End llvm namespace

#endif
