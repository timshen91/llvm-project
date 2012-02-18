//===-- HexagonAsmPrinter.cpp - Print machine instrs to Hexagon assembly --===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains a printer that converts from our internal representation
// of machine-dependent LLVM code to Hexagon assembly language. This printer is
// the output mechanism used by `llc'.
//
// Documentation at http://developer.apple.com/documentation/DeveloperTools/
// Reference/Assembler/ASMIntroduction/chapter_1_section_1.html
//
//===----------------------------------------------------------------------===//


#define DEBUG_TYPE "asm-printer"
#include "Hexagon.h"
#include "HexagonTargetMachine.h"
#include "HexagonSubtarget.h"
#include "HexagonMachineFunctionInfo.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Module.h"
#include "llvm/Assembly/Writer.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/Mangler.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
#include "llvm/Target/TargetRegisterInfo.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

static cl::opt<bool> AlignCalls(
         "hexagon-align-calls", cl::Hidden, cl::init(true),
          cl::desc("Insert falign after call instruction for Hexagon target"));


namespace {
  class HexagonAsmPrinter : public AsmPrinter {
    const HexagonSubtarget *Subtarget;

  public:
    explicit HexagonAsmPrinter(TargetMachine &TM, MCStreamer &Streamer)
      : AsmPrinter(TM, Streamer) {
      Subtarget = &TM.getSubtarget<HexagonSubtarget>();
    }

    virtual const char *getPassName() const {
      return "Hexagon Assembly Printer";
    }

    /// printInstruction - This method is automatically generated by tablegen
    /// from the instruction set description.  This method returns true if the
    /// machine instruction was sufficiently described to print it, otherwise it
    void printInstruction(const MachineInstr *MI, raw_ostream &O);
    virtual void EmitInstruction(const MachineInstr *MI);

    void printOp(const MachineOperand &MO, raw_ostream &O);

    /// printRegister - Print register according to target requirements.
    ///
    void printRegister(const MachineOperand &MO, bool R0AsZero,
                       raw_ostream &O) {
      unsigned RegNo = MO.getReg();
      assert(TargetRegisterInfo::isPhysicalRegister(RegNo) && "Not physreg??");
      O << getRegisterName(RegNo);
    }

    void printOperand(const MachineInstr *MI, unsigned OpNo, raw_ostream &OS) {
      const MachineOperand &MO = MI->getOperand(OpNo);
      if (MO.isReg()) {
        printRegister(MO, false, OS);
      } else if (MO.isImm()) {
        OS << MO.getImm();
      } else {
        printOp(MO, OS);
      }
    }


    bool isBlockOnlyReachableByFallthrough(const MachineBasicBlock *MBB) const;

    bool PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                         unsigned AsmVariant, const char *ExtraCode,
                         raw_ostream &OS);
    bool PrintAsmMemoryOperand(const MachineInstr *MI, unsigned OpNo,
                               unsigned AsmVariant, const char *ExtraCode,
                               raw_ostream &OS);


    void printHexagonImmOperand(const MachineInstr *MI, unsigned OpNo,
                                raw_ostream &O) {
      int value = MI->getOperand(OpNo).getImm();
      O << value;
    }


    void printHexagonNegImmOperand(const MachineInstr *MI, unsigned OpNo,
                                   raw_ostream &O) {
      int value = MI->getOperand(OpNo).getImm();
      O << -value;
    }

    void printHexagonNOneImmOperand(const MachineInstr *MI, unsigned OpNo,
                                    raw_ostream &O) const {
      O << -1;
    }

    void printHexagonMEMriOperand(const MachineInstr *MI, unsigned OpNo,
                                  raw_ostream &O) {
      const MachineOperand &MO1 = MI->getOperand(OpNo);
      const MachineOperand &MO2 = MI->getOperand(OpNo+1);

      O << getRegisterName(MO1.getReg())
        << " + #"
        << (int) MO2.getImm();
    }


    void printHexagonFrameIndexOperand(const MachineInstr *MI, unsigned OpNo,
                                       raw_ostream &O) {
      const MachineOperand &MO1 = MI->getOperand(OpNo);
      const MachineOperand &MO2 = MI->getOperand(OpNo+1);

      O << getRegisterName(MO1.getReg())
        << ", #"
        << MO2.getImm();
    }

    void printBranchOperand(const MachineInstr *MI, unsigned OpNo,
                            raw_ostream &O) {
      // Branches can take an immediate operand.  This is used by the branch
      // selection pass to print $+8, an eight byte displacement from the PC.
      if (MI->getOperand(OpNo).isImm()) {
        O << "$+" << MI->getOperand(OpNo).getImm()*4;
      } else {
        printOp(MI->getOperand(OpNo), O);
      }
    }

    void printCallOperand(const MachineInstr *MI, unsigned OpNo,
                          raw_ostream &O) {
    }

    void printAbsAddrOperand(const MachineInstr *MI, unsigned OpNo,
                            raw_ostream &O) {
    }


    void printSymbolHi(const MachineInstr *MI, unsigned OpNo, raw_ostream &O) {
      O << "#HI(";
      if (MI->getOperand(OpNo).isImm()) {
        printHexagonImmOperand(MI, OpNo, O);
      } else {
        printOp(MI->getOperand(OpNo), O);
      }
      O << ")";
    }

    void printSymbolLo(const MachineInstr *MI, unsigned OpNo, raw_ostream &O) {
      O << "#HI(";
      if (MI->getOperand(OpNo).isImm()) {
        printHexagonImmOperand(MI, OpNo, O);
      } else {
        printOp(MI->getOperand(OpNo), O);
      }
      O << ")";
    }

    void printPredicateOperand(const MachineInstr *MI, unsigned OpNo,
                               raw_ostream &O);

    void printAddrModeBasePlusOffset(const MachineInstr *MI, int OpNo,
                                     raw_ostream &O);

    void printGlobalOperand(const MachineInstr *MI, int OpNo, raw_ostream &O);
    void printJumpTable(const MachineInstr *MI, int OpNo, raw_ostream &O);

    void EmitAlignment(unsigned NumBits, const GlobalValue *GV = 0) const;

    static const char *getRegisterName(unsigned RegNo);
  };

} // end of anonymous namespace

// Include the auto-generated portion of the assembly writer.
#include "HexagonGenAsmWriter.inc"


void HexagonAsmPrinter::EmitAlignment(unsigned NumBits,
                                      const GlobalValue *GV) const {

  // For basic block level alignment, use falign.
  if (!GV) {
    OutStreamer.EmitRawText(StringRef("\t.falign"));
    return;
  }

  AsmPrinter::EmitAlignment(NumBits, GV);
}

void HexagonAsmPrinter::printOp(const MachineOperand &MO, raw_ostream &O) {
  switch (MO.getType()) {
  case MachineOperand::MO_Immediate:
    dbgs() << "printOp() does not handle immediate values\n";
    abort();

  case MachineOperand::MO_MachineBasicBlock:
    O << *MO.getMBB()->getSymbol();
    return;
  case MachineOperand::MO_JumpTableIndex:
    O << *GetJTISymbol(MO.getIndex());
    // FIXME: PIC relocation model.
    return;
  case MachineOperand::MO_ConstantPoolIndex:
    O << *GetCPISymbol(MO.getIndex());
    return;
  case MachineOperand::MO_ExternalSymbol:
    O << *GetExternalSymbolSymbol(MO.getSymbolName());
    return;
  case MachineOperand::MO_GlobalAddress: {
    // Computing the address of a global symbol, not calling it.
    O << *Mang->getSymbol(MO.getGlobal());
    printOffset(MO.getOffset(), O);
    return;
  }

  default:
    O << "<unknown operand type: " << MO.getType() << ">";
    return;
  }
}


//
// isBlockOnlyReachableByFallthrough - We need to override this since the
// default AsmPrinter does not print labels for any basic block that
// is only reachable by a fall through. That works for all cases except
// for the case in which the basic block is reachable by a fall through but
// through an indirect from a jump table. In this case, the jump table
// will contain a label not defined by AsmPrinter.
//
bool HexagonAsmPrinter::
isBlockOnlyReachableByFallthrough(const MachineBasicBlock *MBB) const {
  if (MBB->hasAddressTaken()) {
    return false;
  }
  return AsmPrinter::isBlockOnlyReachableByFallthrough(MBB);
}


/// PrintAsmOperand - Print out an operand for an inline asm expression.
///
bool HexagonAsmPrinter::PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                                        unsigned AsmVariant,
                                        const char *ExtraCode,
                                      raw_ostream &OS) {
  // Does this asm operand have a single letter operand modifier?
  if (ExtraCode && ExtraCode[0]) {
    if (ExtraCode[1] != 0) return true; // Unknown modifier.

    switch (ExtraCode[0]) {
    default: return true;  // Unknown modifier.
    case 'c': // Don't print "$" before a global var name or constant.
      // Hexagon never has a prefix.
      printOperand(MI, OpNo, OS);
      return false;
    case 'L': // Write second word of DImode reference.
      // Verify that this operand has two consecutive registers.
      if (!MI->getOperand(OpNo).isReg() ||
          OpNo+1 == MI->getNumOperands() ||
          !MI->getOperand(OpNo+1).isReg())
        return true;
      ++OpNo;   // Return the high-part.
      break;
    case 'I':
      // Write 'i' if an integer constant, otherwise nothing.  Used to print
      // addi vs add, etc.
      if (MI->getOperand(OpNo).isImm())
        OS << "i";
      return false;
    }
  }

  printOperand(MI, OpNo, OS);
  return false;
}

bool HexagonAsmPrinter::PrintAsmMemoryOperand(const MachineInstr *MI,
                                            unsigned OpNo, unsigned AsmVariant,
                                            const char *ExtraCode,
                                            raw_ostream &O) {
  if (ExtraCode && ExtraCode[0])
    return true; // Unknown modifier.

  const MachineOperand &Base  = MI->getOperand(OpNo);
  const MachineOperand &Offset = MI->getOperand(OpNo+1);

  if (Base.isReg())
    printOperand(MI, OpNo, O);
  else
    llvm_unreachable("Unimplemented");

  if (Offset.isImm()) {
    if (Offset.getImm())
      O << " + #" << Offset.getImm();
  }
  else
    llvm_unreachable("Unimplemented");

  return false;
}

void HexagonAsmPrinter::printPredicateOperand(const MachineInstr *MI,
                                              unsigned OpNo,
                                              raw_ostream &O) {
  llvm_unreachable("Unimplemented");
}


/// printMachineInstruction -- Print out a single Hexagon MI in Darwin syntax to
/// the current output stream.
///
void HexagonAsmPrinter::EmitInstruction(const MachineInstr *MI) {
  SmallString<128> Str;
  raw_svector_ostream O(Str);

  const MachineFunction* MF = MI->getParent()->getParent();
  const HexagonMachineFunctionInfo* MFI =
    (const HexagonMachineFunctionInfo*)
    MF->getInfo<HexagonMachineFunctionInfo>();



  // Print a brace for the beginning of the packet.
  if (MFI->isStartPacket(MI)) {
    O << "\t{" << '\n';
  }

  DEBUG( O << "// MI = " << *MI << '\n';);

  // Indent
  O << "\t";


  if (MI->getOpcode() == Hexagon::ENDLOOP0) {
    if (MFI->isEndPacket(MI) && MFI->isStartPacket(MI)) {
      O << "\t{ nop }";
    } else {
    O << "}";
    }
    printInstruction(MI, O);
  } else if (MI->getOpcode() == Hexagon::STriwt) {
    //
    // Handle truncated store on Hexagon.
    //
    O << "\tmemw(";
    printHexagonMEMriOperand(MI, 0, O);

    O << ") = ";
    unsigned SubRegNum =
      TM.getRegisterInfo()->getSubReg(MI->getOperand(2)
                                      .getReg(), Hexagon::subreg_loreg);
    const char *SubRegName = getRegisterName(SubRegNum);
    O << SubRegName << '\n';
  } else if (MI->getOpcode() == Hexagon::MPYI_rin) {
    // Handle multipy with -ve constant on Hexagon:
    // "$dst =- mpyi($src1, #$src2)"
      printOperand(MI, 0, O);
    O << " =- mpyi(";
    printOperand(MI, 1, O);
    O << ", #";
    printHexagonNegImmOperand(MI, 2, O);
    O << ")";
  } else if (MI->getOpcode() == Hexagon::MEMw_ADDSUBi_indexed_MEM_V4) {
    //
    // Handle memw(Rs+u6:2) [+-]= #U5
    //
    O << "\tmemw("; printHexagonMEMriOperand(MI, 0, O); O << ") ";
    int addend = MI->getOperand(2).getImm();
    if (addend < 0)
      O << "-= " << "#" << -addend << '\n';
    else
      O << "+= " << "#" << addend << '\n';
  } else if (MI->getOpcode() == Hexagon::MEMw_ADDSUBi_MEM_V4) {
    //
    // Handle memw(Rs+u6:2) [+-]= #U5
    //
    O << "\tmemw("; printHexagonMEMriOperand(MI, 0, O); O << ") ";
    int addend = MI->getOperand(2).getImm();
    if (addend < 0)
      O << "-= " << "#" << -addend << '\n';
    else
      O << "+= " << "#" << addend << '\n';
  } else if (MI->getOpcode() == Hexagon::MEMh_ADDSUBi_indexed_MEM_V4) {
    //
    // Handle memh(Rs+u6:1) [+-]= #U5
    //
    O << "\tmemh("; printHexagonMEMriOperand(MI, 0, O); O << ") ";
    int addend = MI->getOperand(2).getImm();
    if (addend < 0)
      O << "-= " << "#" << -addend << '\n';
    else
      O << "+= " << "#" << addend << '\n';
  } else if (MI->getOpcode() == Hexagon::MEMh_ADDSUBi_MEM_V4) {
    //
    // Handle memh(Rs+u6:1) [+-]= #U5
    //
    O << "\tmemh("; printHexagonMEMriOperand(MI, 0, O); O << ") ";
    int addend = MI->getOperand(2).getImm();
    if (addend < 0)
      O << "-= " << "#" << -addend << '\n';
    else
      O << "+= " << "#" << addend << '\n';
  } else if (MI->getOpcode() == Hexagon::MEMb_ADDSUBi_indexed_MEM_V4) {
    //
    // Handle memb(Rs+u6:1) [+-]= #U5
    //
    O << "\tmemb("; printHexagonMEMriOperand(MI, 0, O); O << ") ";
    int addend = MI->getOperand(2).getImm();
    if (addend < 0)
      O << "-= " << "#" << -addend << '\n';
    else
      O << "+= " << "#" << addend << '\n';
  } else if (MI->getOpcode() == Hexagon::MEMb_ADDSUBi_MEM_V4) {
    //
    // Handle memb(Rs+u6:1) [+-]= #U5
    //
    O << "\tmemb("; printHexagonMEMriOperand(MI, 0, O); O << ") ";
    int addend = MI->getOperand(2).getImm();
    if (addend < 0)
      O << "-= " << "#" << -addend << '\n';
    else
      O << "+= " << "#" << addend << '\n';
  } else if (MI->getOpcode() == Hexagon::CMPbGTri_V4) {
    //
    // Handle Pd=cmpb.gt(Rs,#s8)
    //
    O << "\t";
    printRegister(MI->getOperand(0), false, O);
    O << " = cmpb.gt(";
    printRegister(MI->getOperand(1), false, O);
    O << ", ";
    int val = MI->getOperand(2).getImm() >> 24;
    O << "#" << val << ")" << '\n';
  } else if (MI->getOpcode() == Hexagon::CMPhEQri_V4) {
    //
    // Handle Pd=cmph.eq(Rs,#8)
    //
    O << "\t";
    printRegister(MI->getOperand(0), false, O);
    O << " = cmph.eq(";
    printRegister(MI->getOperand(1), false, O);
    O << ", ";
    int val = MI->getOperand(2).getImm();
    assert((((0 <= val) && (val <= 127)) ||
            ((65408 <= val) && (val <= 65535))) &&
           "Not in correct range!");
    if (val >= 65408) val -= 65536;
    O << "#" << val << ")" << '\n';
  } else if (MI->getOpcode() == Hexagon::CMPhGTri_V4) {
    //
    // Handle Pd=cmph.gt(Rs,#8)
    //
    O << "\t";
    printRegister(MI->getOperand(0), false, O);
    O << " = cmph.gt(";
    printRegister(MI->getOperand(1), false, O);
    O << ", ";
    int val = MI->getOperand(2).getImm() >> 16;
    O << "#" << val << ")" << '\n';
  } else {
    printInstruction(MI, O);
  }

  // Print a brace for the end of the packet.
  if (MFI->isEndPacket(MI) && MI->getOpcode() != Hexagon::ENDLOOP0) {
    O << "\n\t}" << '\n';
  }

  if (AlignCalls && MI->getDesc().isCall()) {
    O << "\n\t.falign" << "\n";
  }

  OutStreamer.EmitRawText(O.str());
  return;
}

/// PrintUnmangledNameSafely - Print out the printable characters in the name.
/// Don't print things like \n or \0.
// static void PrintUnmangledNameSafely(const Value *V, raw_ostream &OS) {
//   for (const char *Name = V->getNameStart(), *E = Name+V->getNameLen();
//        Name != E; ++Name)
//     if (isprint(*Name))
//       OS << *Name;
// }


void HexagonAsmPrinter::printAddrModeBasePlusOffset(const MachineInstr *MI,
                                                    int OpNo, raw_ostream &O) {
  const MachineOperand &MO1 = MI->getOperand(OpNo);
  const MachineOperand &MO2 = MI->getOperand(OpNo+1);

  O << getRegisterName(MO1.getReg())
    << " + #"
    << MO2.getImm();
}


void HexagonAsmPrinter::printGlobalOperand(const MachineInstr *MI, int OpNo,
                                           raw_ostream &O) {
  const MachineOperand &MO = MI->getOperand(OpNo);
  assert( (MO.getType() == MachineOperand::MO_GlobalAddress) &&
         "Expecting global address");

  O << *Mang->getSymbol(MO.getGlobal());
  if (MO.getOffset() != 0) {
    O << " + ";
    O << MO.getOffset();
  }
}

void HexagonAsmPrinter::printJumpTable(const MachineInstr *MI, int OpNo,
                                       raw_ostream &O) {
  const MachineOperand &MO = MI->getOperand(OpNo);
  assert( (MO.getType() == MachineOperand::MO_JumpTableIndex) &&
	  "Expecting jump table index");

  // Hexagon_TODO: Do we need name mangling?
  O << *GetJTISymbol(MO.getIndex());
}

extern "C" void LLVMInitializeHexagonAsmPrinter() {
  RegisterAsmPrinter<HexagonAsmPrinter> X(TheHexagonTarget);
}
