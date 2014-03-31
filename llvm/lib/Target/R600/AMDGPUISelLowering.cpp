//===-- AMDGPUISelLowering.cpp - AMDGPU Common DAG lowering functions -----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
/// \file
/// \brief This is the parent TargetLowering class for hardware code gen
/// targets.
//
//===----------------------------------------------------------------------===//

#include "AMDGPUISelLowering.h"
#include "AMDGPU.h"
#include "AMDGPUFrameLowering.h"
#include "AMDGPURegisterInfo.h"
#include "AMDGPUSubtarget.h"
#include "AMDILIntrinsicInfo.h"
#include "R600MachineFunctionInfo.h"
#include "SIMachineFunctionInfo.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/IR/DataLayout.h"

using namespace llvm;
static bool allocateStack(unsigned ValNo, MVT ValVT, MVT LocVT,
                      CCValAssign::LocInfo LocInfo,
                      ISD::ArgFlagsTy ArgFlags, CCState &State) {
  unsigned Offset = State.AllocateStack(ValVT.getStoreSize(),
                                        ArgFlags.getOrigAlign());
  State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset, LocVT, LocInfo));

  return true;
}

#include "AMDGPUGenCallingConv.inc"

AMDGPUTargetLowering::AMDGPUTargetLowering(TargetMachine &TM) :
  TargetLowering(TM, new TargetLoweringObjectFileELF()) {

  Subtarget = &TM.getSubtarget<AMDGPUSubtarget>();

  // Initialize target lowering borrowed from AMDIL
  InitAMDILLowering();

  // We need to custom lower some of the intrinsics
  setOperationAction(ISD::INTRINSIC_WO_CHAIN, MVT::Other, Custom);

  // Library functions.  These default to Expand, but we have instructions
  // for them.
  setOperationAction(ISD::FCEIL,  MVT::f32, Legal);
  setOperationAction(ISD::FEXP2,  MVT::f32, Legal);
  setOperationAction(ISD::FPOW,   MVT::f32, Legal);
  setOperationAction(ISD::FLOG2,  MVT::f32, Legal);
  setOperationAction(ISD::FABS,   MVT::f32, Legal);
  setOperationAction(ISD::FFLOOR, MVT::f32, Legal);
  setOperationAction(ISD::FRINT,  MVT::f32, Legal);
  setOperationAction(ISD::FROUND, MVT::f32, Legal);
  setOperationAction(ISD::FTRUNC, MVT::f32, Legal);

  // The hardware supports ROTR, but not ROTL
  setOperationAction(ISD::ROTL, MVT::i32, Expand);

  // Lower floating point store/load to integer store/load to reduce the number
  // of patterns in tablegen.
  setOperationAction(ISD::STORE, MVT::f32, Promote);
  AddPromotedToType(ISD::STORE, MVT::f32, MVT::i32);

  setOperationAction(ISD::STORE, MVT::v2f32, Promote);
  AddPromotedToType(ISD::STORE, MVT::v2f32, MVT::v2i32);

  setOperationAction(ISD::STORE, MVT::v4f32, Promote);
  AddPromotedToType(ISD::STORE, MVT::v4f32, MVT::v4i32);

  setOperationAction(ISD::STORE, MVT::v8f32, Promote);
  AddPromotedToType(ISD::STORE, MVT::v8f32, MVT::v8i32);

  setOperationAction(ISD::STORE, MVT::v16f32, Promote);
  AddPromotedToType(ISD::STORE, MVT::v16f32, MVT::v16i32);

  setOperationAction(ISD::STORE, MVT::f64, Promote);
  AddPromotedToType(ISD::STORE, MVT::f64, MVT::i64);

  // Custom lowering of vector stores is required for local address space
  // stores.
  setOperationAction(ISD::STORE, MVT::v4i32, Custom);
  // XXX: Native v2i32 local address space stores are possible, but not
  // currently implemented.
  setOperationAction(ISD::STORE, MVT::v2i32, Custom);

  setTruncStoreAction(MVT::v2i32, MVT::v2i16, Custom);
  setTruncStoreAction(MVT::v2i32, MVT::v2i8, Custom);
  setTruncStoreAction(MVT::v4i32, MVT::v4i8, Custom);

  // XXX: This can be change to Custom, once ExpandVectorStores can
  // handle 64-bit stores.
  setTruncStoreAction(MVT::v4i32, MVT::v4i16, Expand);

  setTruncStoreAction(MVT::i64, MVT::i1, Expand);
  setTruncStoreAction(MVT::v2i64, MVT::v2i1, Expand);
  setTruncStoreAction(MVT::v4i64, MVT::v4i1, Expand);


  setOperationAction(ISD::LOAD, MVT::f32, Promote);
  AddPromotedToType(ISD::LOAD, MVT::f32, MVT::i32);

  setOperationAction(ISD::LOAD, MVT::v2f32, Promote);
  AddPromotedToType(ISD::LOAD, MVT::v2f32, MVT::v2i32);

  setOperationAction(ISD::LOAD, MVT::v4f32, Promote);
  AddPromotedToType(ISD::LOAD, MVT::v4f32, MVT::v4i32);

  setOperationAction(ISD::LOAD, MVT::v8f32, Promote);
  AddPromotedToType(ISD::LOAD, MVT::v8f32, MVT::v8i32);

  setOperationAction(ISD::LOAD, MVT::v16f32, Promote);
  AddPromotedToType(ISD::LOAD, MVT::v16f32, MVT::v16i32);

  setOperationAction(ISD::LOAD, MVT::f64, Promote);
  AddPromotedToType(ISD::LOAD, MVT::f64, MVT::i64);

  setOperationAction(ISD::CONCAT_VECTORS, MVT::v4i32, Custom);
  setOperationAction(ISD::CONCAT_VECTORS, MVT::v4f32, Custom);
  setOperationAction(ISD::CONCAT_VECTORS, MVT::v8i32, Custom);
  setOperationAction(ISD::CONCAT_VECTORS, MVT::v8f32, Custom);
  setOperationAction(ISD::EXTRACT_SUBVECTOR, MVT::v2f32, Custom);
  setOperationAction(ISD::EXTRACT_SUBVECTOR, MVT::v2i32, Custom);
  setOperationAction(ISD::EXTRACT_SUBVECTOR, MVT::v4f32, Custom);
  setOperationAction(ISD::EXTRACT_SUBVECTOR, MVT::v4i32, Custom);
  setOperationAction(ISD::EXTRACT_SUBVECTOR, MVT::v8f32, Custom);
  setOperationAction(ISD::EXTRACT_SUBVECTOR, MVT::v8i32, Custom);

  setLoadExtAction(ISD::EXTLOAD, MVT::v2i8, Expand);
  setLoadExtAction(ISD::SEXTLOAD, MVT::v2i8, Expand);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::v2i8, Expand);
  setLoadExtAction(ISD::EXTLOAD, MVT::v4i8, Expand);
  setLoadExtAction(ISD::SEXTLOAD, MVT::v4i8, Expand);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::v4i8, Expand);
  setLoadExtAction(ISD::EXTLOAD, MVT::v2i16, Expand);
  setLoadExtAction(ISD::SEXTLOAD, MVT::v2i16, Expand);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::v2i16, Expand);
  setLoadExtAction(ISD::EXTLOAD, MVT::v4i16, Expand);
  setLoadExtAction(ISD::SEXTLOAD, MVT::v4i16, Expand);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::v4i16, Expand);

  setOperationAction(ISD::BR_CC, MVT::i1, Expand);

  setOperationAction(ISD::FNEG, MVT::v2f32, Expand);
  setOperationAction(ISD::FNEG, MVT::v4f32, Expand);

  setOperationAction(ISD::UINT_TO_FP, MVT::i64, Custom);

  setOperationAction(ISD::MUL, MVT::i64, Expand);

  setOperationAction(ISD::UDIV, MVT::i32, Expand);
  setOperationAction(ISD::UDIVREM, MVT::i32, Custom);
  setOperationAction(ISD::UREM, MVT::i32, Expand);
  setOperationAction(ISD::VSELECT, MVT::v2f32, Expand);
  setOperationAction(ISD::VSELECT, MVT::v4f32, Expand);

  static const MVT::SimpleValueType IntTypes[] = {
    MVT::v2i32, MVT::v4i32
  };
  const size_t NumIntTypes = array_lengthof(IntTypes);

  for (unsigned int x  = 0; x < NumIntTypes; ++x) {
    MVT::SimpleValueType VT = IntTypes[x];
    //Expand the following operations for the current type by default
    setOperationAction(ISD::ADD,  VT, Expand);
    setOperationAction(ISD::AND,  VT, Expand);
    setOperationAction(ISD::FP_TO_SINT, VT, Expand);
    setOperationAction(ISD::FP_TO_UINT, VT, Expand);
    setOperationAction(ISD::MUL,  VT, Expand);
    setOperationAction(ISD::OR,   VT, Expand);
    setOperationAction(ISD::SHL,  VT, Expand);
    setOperationAction(ISD::SINT_TO_FP, VT, Expand);
    setOperationAction(ISD::SRL,  VT, Expand);
    setOperationAction(ISD::SRA,  VT, Expand);
    setOperationAction(ISD::SUB,  VT, Expand);
    setOperationAction(ISD::UDIV, VT, Expand);
    setOperationAction(ISD::UINT_TO_FP, VT, Expand);
    setOperationAction(ISD::UREM, VT, Expand);
    setOperationAction(ISD::SELECT, VT, Expand);
    setOperationAction(ISD::VSELECT, VT, Expand);
    setOperationAction(ISD::XOR,  VT, Expand);
  }

  static const MVT::SimpleValueType FloatTypes[] = {
    MVT::v2f32, MVT::v4f32
  };
  const size_t NumFloatTypes = array_lengthof(FloatTypes);

  for (unsigned int x = 0; x < NumFloatTypes; ++x) {
    MVT::SimpleValueType VT = FloatTypes[x];
    setOperationAction(ISD::FABS, VT, Expand);
    setOperationAction(ISD::FADD, VT, Expand);
    setOperationAction(ISD::FDIV, VT, Expand);
    setOperationAction(ISD::FPOW, VT, Expand);
    setOperationAction(ISD::FFLOOR, VT, Expand);
    setOperationAction(ISD::FTRUNC, VT, Expand);
    setOperationAction(ISD::FMUL, VT, Expand);
    setOperationAction(ISD::FRINT, VT, Expand);
    setOperationAction(ISD::FSQRT, VT, Expand);
    setOperationAction(ISD::FSUB, VT, Expand);
    setOperationAction(ISD::SELECT, VT, Expand);
  }

  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i1, Custom);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::v2i1, Custom);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::v4i1, Custom);

  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i8, Custom);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::v2i8, Custom);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::v4i8, Custom);

  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i16, Custom);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::v2i16, Custom);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::v4i16, Custom);

  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i32, Custom);

  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::Other, Custom);
}

//===----------------------------------------------------------------------===//
// Target Information
//===----------------------------------------------------------------------===//

MVT AMDGPUTargetLowering::getVectorIdxTy() const {
  return MVT::i32;
}

bool AMDGPUTargetLowering::isLoadBitCastBeneficial(EVT LoadTy,
                                                   EVT CastTy) const {
  if (LoadTy.getSizeInBits() != CastTy.getSizeInBits())
    return true;

  unsigned LScalarSize = LoadTy.getScalarType().getSizeInBits();
  unsigned CastScalarSize = CastTy.getScalarType().getSizeInBits();

  return ((LScalarSize <= CastScalarSize) ||
          (CastScalarSize >= 32) ||
          (LScalarSize < 32));
}

//===---------------------------------------------------------------------===//
// Target Properties
//===---------------------------------------------------------------------===//

bool AMDGPUTargetLowering::isFAbsFree(EVT VT) const {
  assert(VT.isFloatingPoint());
  return VT == MVT::f32;
}

bool AMDGPUTargetLowering::isFNegFree(EVT VT) const {
  assert(VT.isFloatingPoint());
  return VT == MVT::f32;
}

bool AMDGPUTargetLowering::isTruncateFree(EVT Source, EVT Dest) const {
  // Truncate is just accessing a subregister.
  return Dest.bitsLT(Source) && (Dest.getSizeInBits() % 32 == 0);
}

bool AMDGPUTargetLowering::isTruncateFree(Type *Source, Type *Dest) const {
  // Truncate is just accessing a subregister.
  return Dest->getPrimitiveSizeInBits() < Source->getPrimitiveSizeInBits() &&
         (Dest->getPrimitiveSizeInBits() % 32 == 0);
}

bool AMDGPUTargetLowering::isZExtFree(Type *Src, Type *Dest) const {
  const DataLayout *DL = getDataLayout();
  unsigned SrcSize = DL->getTypeSizeInBits(Src->getScalarType());
  unsigned DestSize = DL->getTypeSizeInBits(Dest->getScalarType());

  return SrcSize == 32 && DestSize == 64;
}

bool AMDGPUTargetLowering::isZExtFree(EVT Src, EVT Dest) const {
  // Any register load of a 64-bit value really requires 2 32-bit moves. For all
  // practical purposes, the extra mov 0 to load a 64-bit is free.  As used,
  // this will enable reducing 64-bit operations the 32-bit, which is always
  // good.
  return Src == MVT::i32 && Dest == MVT::i64;
}

bool AMDGPUTargetLowering::isNarrowingProfitable(EVT SrcVT, EVT DestVT) const {
  // There aren't really 64-bit registers, but pairs of 32-bit ones and only a
  // limited number of native 64-bit operations. Shrinking an operation to fit
  // in a single 32-bit register should always be helpful. As currently used,
  // this is much less general than the name suggests, and is only used in
  // places trying to reduce the sizes of loads. Shrinking loads to < 32-bits is
  // not profitable, and may actually be harmful.
  return SrcVT.getSizeInBits() > 32 && DestVT.getSizeInBits() == 32;
}

//===---------------------------------------------------------------------===//
// TargetLowering Callbacks
//===---------------------------------------------------------------------===//

void AMDGPUTargetLowering::AnalyzeFormalArguments(CCState &State,
                             const SmallVectorImpl<ISD::InputArg> &Ins) const {

  State.AnalyzeFormalArguments(Ins, CC_AMDGPU);
}

SDValue AMDGPUTargetLowering::LowerReturn(
                                     SDValue Chain,
                                     CallingConv::ID CallConv,
                                     bool isVarArg,
                                     const SmallVectorImpl<ISD::OutputArg> &Outs,
                                     const SmallVectorImpl<SDValue> &OutVals,
                                     SDLoc DL, SelectionDAG &DAG) const {
  return DAG.getNode(AMDGPUISD::RET_FLAG, DL, MVT::Other, Chain);
}

//===---------------------------------------------------------------------===//
// Target specific lowering
//===---------------------------------------------------------------------===//

SDValue AMDGPUTargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG)
    const {
  switch (Op.getOpcode()) {
  default:
    Op.getNode()->dump();
    llvm_unreachable("Custom lowering code for this"
                     "instruction is not implemented yet!");
    break;
  // AMDIL DAG lowering
  case ISD::SDIV: return LowerSDIV(Op, DAG);
  case ISD::SREM: return LowerSREM(Op, DAG);
  case ISD::SIGN_EXTEND_INREG: return LowerSIGN_EXTEND_INREG(Op, DAG);
  case ISD::BRCOND: return LowerBRCOND(Op, DAG);
  // AMDGPU DAG lowering
  case ISD::CONCAT_VECTORS: return LowerCONCAT_VECTORS(Op, DAG);
  case ISD::EXTRACT_SUBVECTOR: return LowerEXTRACT_SUBVECTOR(Op, DAG);
  case ISD::FrameIndex: return LowerFrameIndex(Op, DAG);
  case ISD::INTRINSIC_WO_CHAIN: return LowerINTRINSIC_WO_CHAIN(Op, DAG);
  case ISD::UDIVREM: return LowerUDIVREM(Op, DAG);
  case ISD::UINT_TO_FP: return LowerUINT_TO_FP(Op, DAG);
  }
  return Op;
}

void AMDGPUTargetLowering::ReplaceNodeResults(SDNode *N,
                                              SmallVectorImpl<SDValue> &Results,
                                              SelectionDAG &DAG) const {
  switch (N->getOpcode()) {
  case ISD::SIGN_EXTEND_INREG:
    // Different parts of legalization seem to interpret which type of
    // sign_extend_inreg is the one to check for custom lowering. The extended
    // from type is what really matters, but some places check for custom
    // lowering of the result type. This results in trying to use
    // ReplaceNodeResults to sext_in_reg to an illegal type, so we'll just do
    // nothing here and let the illegal result integer be handled normally.
    return;

  default:
    return;
  }
}

SDValue AMDGPUTargetLowering::LowerConstantInitializer(const Constant* Init,
                                                       const GlobalValue *GV,
                                                       const SDValue &InitPtr,
                                                       SDValue Chain,
                                                       SelectionDAG &DAG) const {
  const DataLayout *TD = getTargetMachine().getDataLayout();
  SDLoc DL(InitPtr);
  if (const ConstantInt *CI = dyn_cast<ConstantInt>(Init)) {
    EVT VT = EVT::getEVT(CI->getType());
    PointerType *PtrTy = PointerType::get(CI->getType(), 0);
    return DAG.getStore(Chain, DL,  DAG.getConstant(*CI, VT), InitPtr,
                 MachinePointerInfo(UndefValue::get(PtrTy)), false, false,
                 TD->getPrefTypeAlignment(CI->getType()));
  } else if (const ConstantFP *CFP = dyn_cast<ConstantFP>(Init)) {
    EVT VT = EVT::getEVT(CFP->getType());
    PointerType *PtrTy = PointerType::get(CFP->getType(), 0);
    return DAG.getStore(Chain, DL, DAG.getConstantFP(*CFP, VT), InitPtr,
                 MachinePointerInfo(UndefValue::get(PtrTy)), false, false,
                 TD->getPrefTypeAlignment(CFP->getType()));
  } else if (Init->getType()->isAggregateType()) {
    EVT PtrVT = InitPtr.getValueType();
    unsigned NumElements = Init->getType()->getArrayNumElements();
    SmallVector<SDValue, 8> Chains;
    for (unsigned i = 0; i < NumElements; ++i) {
      SDValue Offset = DAG.getConstant(i * TD->getTypeAllocSize(
          Init->getType()->getArrayElementType()), PtrVT);
      SDValue Ptr = DAG.getNode(ISD::ADD, DL, PtrVT, InitPtr, Offset);
      Chains.push_back(LowerConstantInitializer(Init->getAggregateElement(i),
                       GV, Ptr, Chain, DAG));
    }
    return DAG.getNode(ISD::TokenFactor, DL, MVT::Other, &Chains[0],
                       Chains.size());
  } else {
    Init->dump();
    llvm_unreachable("Unhandled constant initializer");
  }
}

SDValue AMDGPUTargetLowering::LowerGlobalAddress(AMDGPUMachineFunction* MFI,
                                                 SDValue Op,
                                                 SelectionDAG &DAG) const {

  const DataLayout *TD = getTargetMachine().getDataLayout();
  GlobalAddressSDNode *G = cast<GlobalAddressSDNode>(Op);
  const GlobalValue *GV = G->getGlobal();

  switch (G->getAddressSpace()) {
  default: llvm_unreachable("Global Address lowering not implemented for this "
                            "address space");
  case AMDGPUAS::LOCAL_ADDRESS: {
    // XXX: What does the value of G->getOffset() mean?
    assert(G->getOffset() == 0 &&
         "Do not know what to do with an non-zero offset");

    unsigned Offset;
    if (MFI->LocalMemoryObjects.count(GV) == 0) {
      uint64_t Size = TD->getTypeAllocSize(GV->getType()->getElementType());
      Offset = MFI->LDSSize;
      MFI->LocalMemoryObjects[GV] = Offset;
      // XXX: Account for alignment?
      MFI->LDSSize += Size;
    } else {
      Offset = MFI->LocalMemoryObjects[GV];
    }

    return DAG.getConstant(Offset, getPointerTy(G->getAddressSpace()));
  }
  case AMDGPUAS::CONSTANT_ADDRESS: {
    MachineFrameInfo *FrameInfo = DAG.getMachineFunction().getFrameInfo();
    Type *EltType = GV->getType()->getElementType();
    unsigned Size = TD->getTypeAllocSize(EltType);
    unsigned Alignment = TD->getPrefTypeAlignment(EltType);

    const GlobalVariable *Var = dyn_cast<GlobalVariable>(GV);
    const Constant *Init = Var->getInitializer();
    int FI = FrameInfo->CreateStackObject(Size, Alignment, false);
    SDValue InitPtr = DAG.getFrameIndex(FI,
        getPointerTy(AMDGPUAS::PRIVATE_ADDRESS));
    SmallVector<SDNode*, 8> WorkList;

    for (SDNode::use_iterator I = DAG.getEntryNode()->use_begin(),
                              E = DAG.getEntryNode()->use_end(); I != E; ++I) {
      if (I->getOpcode() != AMDGPUISD::REGISTER_LOAD && I->getOpcode() != ISD::LOAD)
        continue;
      WorkList.push_back(*I);
    }
    SDValue Chain = LowerConstantInitializer(Init, GV, InitPtr, DAG.getEntryNode(), DAG);
    for (SmallVector<SDNode*, 8>::iterator I = WorkList.begin(),
                                           E = WorkList.end(); I != E; ++I) {
      SmallVector<SDValue, 8> Ops;
      Ops.push_back(Chain);
      for (unsigned i = 1; i < (*I)->getNumOperands(); ++i) {
        Ops.push_back((*I)->getOperand(i));
      }
      DAG.UpdateNodeOperands(*I, &Ops[0], Ops.size());
    }
    return DAG.getZExtOrTrunc(InitPtr, SDLoc(Op),
        getPointerTy(AMDGPUAS::CONSTANT_ADDRESS));
  }
  }
}

void AMDGPUTargetLowering::ExtractVectorElements(SDValue Op, SelectionDAG &DAG,
                                         SmallVectorImpl<SDValue> &Args,
                                         unsigned Start,
                                         unsigned Count) const {
  EVT VT = Op.getValueType();
  for (unsigned i = Start, e = Start + Count; i != e; ++i) {
    Args.push_back(DAG.getNode(ISD::EXTRACT_VECTOR_ELT, SDLoc(Op),
                               VT.getVectorElementType(),
                               Op, DAG.getConstant(i, MVT::i32)));
  }
}

SDValue AMDGPUTargetLowering::LowerCONCAT_VECTORS(SDValue Op,
                                                  SelectionDAG &DAG) const {
  SmallVector<SDValue, 8> Args;
  SDValue A = Op.getOperand(0);
  SDValue B = Op.getOperand(1);

  ExtractVectorElements(A, DAG, Args, 0,
                        A.getValueType().getVectorNumElements());
  ExtractVectorElements(B, DAG, Args, 0,
                        B.getValueType().getVectorNumElements());

  return DAG.getNode(ISD::BUILD_VECTOR, SDLoc(Op), Op.getValueType(),
                     &Args[0], Args.size());
}

SDValue AMDGPUTargetLowering::LowerEXTRACT_SUBVECTOR(SDValue Op,
                                                     SelectionDAG &DAG) const {

  SmallVector<SDValue, 8> Args;
  EVT VT = Op.getValueType();
  unsigned Start = cast<ConstantSDNode>(Op.getOperand(1))->getZExtValue();
  ExtractVectorElements(Op.getOperand(0), DAG, Args, Start,
                        VT.getVectorNumElements());

  return DAG.getNode(ISD::BUILD_VECTOR, SDLoc(Op), Op.getValueType(),
                     &Args[0], Args.size());
}

SDValue AMDGPUTargetLowering::LowerFrameIndex(SDValue Op,
                                              SelectionDAG &DAG) const {

  MachineFunction &MF = DAG.getMachineFunction();
  const AMDGPUFrameLowering *TFL =
   static_cast<const AMDGPUFrameLowering*>(getTargetMachine().getFrameLowering());

  FrameIndexSDNode *FIN = dyn_cast<FrameIndexSDNode>(Op);
  assert(FIN);

  unsigned FrameIndex = FIN->getIndex();
  unsigned Offset = TFL->getFrameIndexOffset(MF, FrameIndex);
  return DAG.getConstant(Offset * 4 * TFL->getStackWidth(MF),
                         Op.getValueType());
}

SDValue AMDGPUTargetLowering::LowerINTRINSIC_WO_CHAIN(SDValue Op,
    SelectionDAG &DAG) const {
  unsigned IntrinsicID = cast<ConstantSDNode>(Op.getOperand(0))->getZExtValue();
  SDLoc DL(Op);
  EVT VT = Op.getValueType();

  switch (IntrinsicID) {
    default: return Op;
    case AMDGPUIntrinsic::AMDIL_abs:
      return LowerIntrinsicIABS(Op, DAG);
    case AMDGPUIntrinsic::AMDIL_exp:
      return DAG.getNode(ISD::FEXP2, DL, VT, Op.getOperand(1));
    case AMDGPUIntrinsic::AMDGPU_lrp:
      return LowerIntrinsicLRP(Op, DAG);
    case AMDGPUIntrinsic::AMDIL_fraction:
      return DAG.getNode(AMDGPUISD::FRACT, DL, VT, Op.getOperand(1));
    case AMDGPUIntrinsic::AMDIL_max:
      return DAG.getNode(AMDGPUISD::FMAX, DL, VT, Op.getOperand(1),
                                                  Op.getOperand(2));
    case AMDGPUIntrinsic::AMDGPU_imax:
      return DAG.getNode(AMDGPUISD::SMAX, DL, VT, Op.getOperand(1),
                                                  Op.getOperand(2));
    case AMDGPUIntrinsic::AMDGPU_umax:
      return DAG.getNode(AMDGPUISD::UMAX, DL, VT, Op.getOperand(1),
                                                  Op.getOperand(2));
    case AMDGPUIntrinsic::AMDIL_min:
      return DAG.getNode(AMDGPUISD::FMIN, DL, VT, Op.getOperand(1),
                                                  Op.getOperand(2));
    case AMDGPUIntrinsic::AMDGPU_imin:
      return DAG.getNode(AMDGPUISD::SMIN, DL, VT, Op.getOperand(1),
                                                  Op.getOperand(2));
    case AMDGPUIntrinsic::AMDGPU_umin:
      return DAG.getNode(AMDGPUISD::UMIN, DL, VT, Op.getOperand(1),
                                                  Op.getOperand(2));

    case AMDGPUIntrinsic::AMDGPU_bfe_i32:
      return DAG.getNode(AMDGPUISD::BFE_I32, DL, VT,
                         Op.getOperand(1),
                         Op.getOperand(2),
                         Op.getOperand(3));

    case AMDGPUIntrinsic::AMDGPU_bfe_u32:
      return DAG.getNode(AMDGPUISD::BFE_U32, DL, VT,
                         Op.getOperand(1),
                         Op.getOperand(2),
                         Op.getOperand(3));

    case AMDGPUIntrinsic::AMDGPU_bfi:
      return DAG.getNode(AMDGPUISD::BFI, DL, VT,
                         Op.getOperand(1),
                         Op.getOperand(2),
                         Op.getOperand(3));

    case AMDGPUIntrinsic::AMDGPU_bfm:
      return DAG.getNode(AMDGPUISD::BFM, DL, VT,
                         Op.getOperand(1),
                         Op.getOperand(2));

    case AMDGPUIntrinsic::AMDIL_round_nearest:
      return DAG.getNode(ISD::FRINT, DL, VT, Op.getOperand(1));
  }
}

///IABS(a) = SMAX(sub(0, a), a)
SDValue AMDGPUTargetLowering::LowerIntrinsicIABS(SDValue Op,
    SelectionDAG &DAG) const {

  SDLoc DL(Op);
  EVT VT = Op.getValueType();
  SDValue Neg = DAG.getNode(ISD::SUB, DL, VT, DAG.getConstant(0, VT),
                                              Op.getOperand(1));

  return DAG.getNode(AMDGPUISD::SMAX, DL, VT, Neg, Op.getOperand(1));
}

/// Linear Interpolation
/// LRP(a, b, c) = muladd(a,  b, (1 - a) * c)
SDValue AMDGPUTargetLowering::LowerIntrinsicLRP(SDValue Op,
    SelectionDAG &DAG) const {
  SDLoc DL(Op);
  EVT VT = Op.getValueType();
  SDValue OneSubA = DAG.getNode(ISD::FSUB, DL, VT,
                                DAG.getConstantFP(1.0f, MVT::f32),
                                Op.getOperand(1));
  SDValue OneSubAC = DAG.getNode(ISD::FMUL, DL, VT, OneSubA,
                                                    Op.getOperand(3));
  return DAG.getNode(ISD::FADD, DL, VT,
      DAG.getNode(ISD::FMUL, DL, VT, Op.getOperand(1), Op.getOperand(2)),
      OneSubAC);
}

/// \brief Generate Min/Max node
SDValue AMDGPUTargetLowering::LowerMinMax(SDValue Op,
    SelectionDAG &DAG) const {
  SDLoc DL(Op);
  EVT VT = Op.getValueType();

  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  SDValue True = Op.getOperand(2);
  SDValue False = Op.getOperand(3);
  SDValue CC = Op.getOperand(4);

  if (VT != MVT::f32 ||
      !((LHS == True && RHS == False) || (LHS == False && RHS == True))) {
    return SDValue();
  }

  ISD::CondCode CCOpcode = cast<CondCodeSDNode>(CC)->get();
  switch (CCOpcode) {
  case ISD::SETOEQ:
  case ISD::SETONE:
  case ISD::SETUNE:
  case ISD::SETNE:
  case ISD::SETUEQ:
  case ISD::SETEQ:
  case ISD::SETFALSE:
  case ISD::SETFALSE2:
  case ISD::SETTRUE:
  case ISD::SETTRUE2:
  case ISD::SETUO:
  case ISD::SETO:
    llvm_unreachable("Operation should already be optimised!");
  case ISD::SETULE:
  case ISD::SETULT:
  case ISD::SETOLE:
  case ISD::SETOLT:
  case ISD::SETLE:
  case ISD::SETLT: {
    if (LHS == True)
      return DAG.getNode(AMDGPUISD::FMIN, DL, VT, LHS, RHS);
    else
      return DAG.getNode(AMDGPUISD::FMAX, DL, VT, LHS, RHS);
  }
  case ISD::SETGT:
  case ISD::SETGE:
  case ISD::SETUGE:
  case ISD::SETOGE:
  case ISD::SETUGT:
  case ISD::SETOGT: {
    if (LHS == True)
      return DAG.getNode(AMDGPUISD::FMAX, DL, VT, LHS, RHS);
    else
      return DAG.getNode(AMDGPUISD::FMIN, DL, VT, LHS, RHS);
  }
  case ISD::SETCC_INVALID:
    llvm_unreachable("Invalid setcc condcode!");
  }
  return Op;
}

SDValue AMDGPUTargetLowering::SplitVectorLoad(const SDValue &Op,
                                              SelectionDAG &DAG) const {
  LoadSDNode *Load = dyn_cast<LoadSDNode>(Op);
  EVT MemEltVT = Load->getMemoryVT().getVectorElementType();
  EVT EltVT = Op.getValueType().getVectorElementType();
  EVT PtrVT = Load->getBasePtr().getValueType();
  unsigned NumElts = Load->getMemoryVT().getVectorNumElements();
  SmallVector<SDValue, 8> Loads;
  SDLoc SL(Op);

  for (unsigned i = 0, e = NumElts; i != e; ++i) {
    SDValue Ptr = DAG.getNode(ISD::ADD, SL, PtrVT, Load->getBasePtr(),
                    DAG.getConstant(i * (MemEltVT.getSizeInBits() / 8), PtrVT));
    Loads.push_back(DAG.getExtLoad(Load->getExtensionType(), SL, EltVT,
                        Load->getChain(), Ptr,
                        MachinePointerInfo(Load->getMemOperand()->getValue()),
                        MemEltVT, Load->isVolatile(), Load->isNonTemporal(),
                        Load->getAlignment()));
  }
  return DAG.getNode(ISD::BUILD_VECTOR, SL, Op.getValueType(),
                     Loads.data(), Loads.size());
}

SDValue AMDGPUTargetLowering::MergeVectorStore(const SDValue &Op,
                                               SelectionDAG &DAG) const {
  StoreSDNode *Store = dyn_cast<StoreSDNode>(Op);
  EVT MemVT = Store->getMemoryVT();
  unsigned MemBits = MemVT.getSizeInBits();

  // Byte stores are really expensive, so if possible, try to pack 32-bit vector
  // truncating store into an i32 store.
  // XXX: We could also handle optimize other vector bitwidths.
  if (!MemVT.isVector() || MemBits > 32) {
    return SDValue();
  }

  SDLoc DL(Op);
  const SDValue &Value = Store->getValue();
  EVT VT = Value.getValueType();
  const SDValue &Ptr = Store->getBasePtr();
  EVT MemEltVT = MemVT.getVectorElementType();
  unsigned MemEltBits = MemEltVT.getSizeInBits();
  unsigned MemNumElements = MemVT.getVectorNumElements();
  EVT PackedVT = EVT::getIntegerVT(*DAG.getContext(), MemVT.getSizeInBits());
  SDValue Mask = DAG.getConstant((1 << MemEltBits) - 1, PackedVT);

  SDValue PackedValue;
  for (unsigned i = 0; i < MemNumElements; ++i) {
    EVT ElemVT = VT.getVectorElementType();
    SDValue Elt = DAG.getNode(ISD::EXTRACT_VECTOR_ELT, DL, ElemVT, Value,
                              DAG.getConstant(i, MVT::i32));
    Elt = DAG.getZExtOrTrunc(Elt, DL, PackedVT);
    Elt = DAG.getNode(ISD::AND, DL, PackedVT, Elt, Mask);
    SDValue Shift = DAG.getConstant(MemEltBits * i, PackedVT);
    Elt = DAG.getNode(ISD::SHL, DL, PackedVT, Elt, Shift);
    if (i == 0) {
      PackedValue = Elt;
    } else {
      PackedValue = DAG.getNode(ISD::OR, DL, PackedVT, PackedValue, Elt);
    }
  }
  return DAG.getStore(Store->getChain(), DL, PackedValue, Ptr,
                      MachinePointerInfo(Store->getMemOperand()->getValue()),
                      Store->isVolatile(),  Store->isNonTemporal(),
                      Store->getAlignment());
}

SDValue AMDGPUTargetLowering::SplitVectorStore(SDValue Op,
                                            SelectionDAG &DAG) const {
  StoreSDNode *Store = cast<StoreSDNode>(Op);
  EVT MemEltVT = Store->getMemoryVT().getVectorElementType();
  EVT EltVT = Store->getValue().getValueType().getVectorElementType();
  EVT PtrVT = Store->getBasePtr().getValueType();
  unsigned NumElts = Store->getMemoryVT().getVectorNumElements();
  SDLoc SL(Op);

  SmallVector<SDValue, 8> Chains;

  for (unsigned i = 0, e = NumElts; i != e; ++i) {
    SDValue Val = DAG.getNode(ISD::EXTRACT_VECTOR_ELT, SL, EltVT,
                              Store->getValue(), DAG.getConstant(i, MVT::i32));
    SDValue Ptr = DAG.getNode(ISD::ADD, SL, PtrVT,
                              Store->getBasePtr(),
                            DAG.getConstant(i * (MemEltVT.getSizeInBits() / 8),
                                            PtrVT));
    Chains.push_back(DAG.getTruncStore(Store->getChain(), SL, Val, Ptr,
                         MachinePointerInfo(Store->getMemOperand()->getValue()),
                         MemEltVT, Store->isVolatile(), Store->isNonTemporal(),
                         Store->getAlignment()));
  }
  return DAG.getNode(ISD::TokenFactor, SL, MVT::Other, &Chains[0], NumElts);
}

SDValue AMDGPUTargetLowering::LowerLOAD(SDValue Op, SelectionDAG &DAG) const {
  SDLoc DL(Op);
  LoadSDNode *Load = cast<LoadSDNode>(Op);
  ISD::LoadExtType ExtType = Load->getExtensionType();
  EVT VT = Op.getValueType();
  EVT MemVT = Load->getMemoryVT();

  if (ExtType != ISD::NON_EXTLOAD && !VT.isVector() && VT.getSizeInBits() > 32) {
    // We can do the extload to 32-bits, and then need to separately extend to
    // 64-bits.

    SDValue ExtLoad32 = DAG.getExtLoad(ExtType, DL, MVT::i32,
                                       Load->getChain(),
                                       Load->getBasePtr(),
                                       MemVT,
                                       Load->getMemOperand());
    return DAG.getNode(ISD::getExtForLoadExtType(ExtType), DL, VT, ExtLoad32);
  }

  // Lower loads constant address space global variable loads
  if (Load->getAddressSpace() == AMDGPUAS::CONSTANT_ADDRESS &&
      isa<GlobalVariable>(GetUnderlyingObject(Load->getPointerInfo().V))) {

    SDValue Ptr = DAG.getZExtOrTrunc(Load->getBasePtr(), DL,
        getPointerTy(AMDGPUAS::PRIVATE_ADDRESS));
    Ptr = DAG.getNode(ISD::SRL, DL, MVT::i32, Ptr,
        DAG.getConstant(2, MVT::i32));
    return DAG.getNode(AMDGPUISD::REGISTER_LOAD, DL, Op.getValueType(),
                       Load->getChain(), Ptr,
                       DAG.getTargetConstant(0, MVT::i32), Op.getOperand(2));
  }

  if (Load->getAddressSpace() != AMDGPUAS::PRIVATE_ADDRESS ||
      ExtType == ISD::NON_EXTLOAD || Load->getMemoryVT().bitsGE(MVT::i32))
    return SDValue();


  SDValue Ptr = DAG.getNode(ISD::SRL, DL, MVT::i32, Load->getBasePtr(),
                            DAG.getConstant(2, MVT::i32));
  SDValue Ret = DAG.getNode(AMDGPUISD::REGISTER_LOAD, DL, Op.getValueType(),
                            Load->getChain(), Ptr,
                            DAG.getTargetConstant(0, MVT::i32),
                            Op.getOperand(2));
  SDValue ByteIdx = DAG.getNode(ISD::AND, DL, MVT::i32,
                                Load->getBasePtr(),
                                DAG.getConstant(0x3, MVT::i32));
  SDValue ShiftAmt = DAG.getNode(ISD::SHL, DL, MVT::i32, ByteIdx,
                                 DAG.getConstant(3, MVT::i32));

  Ret = DAG.getNode(ISD::SRL, DL, MVT::i32, Ret, ShiftAmt);

  EVT MemEltVT = MemVT.getScalarType();
  if (ExtType == ISD::SEXTLOAD) {
    SDValue MemEltVTNode = DAG.getValueType(MemEltVT);
    return DAG.getNode(ISD::SIGN_EXTEND_INREG, DL, MVT::i32, Ret, MemEltVTNode);
  }

  return DAG.getZeroExtendInReg(Ret, DL, MemEltVT);
}

SDValue AMDGPUTargetLowering::LowerSTORE(SDValue Op, SelectionDAG &DAG) const {
  SDLoc DL(Op);
  SDValue Result = AMDGPUTargetLowering::MergeVectorStore(Op, DAG);
  if (Result.getNode()) {
    return Result;
  }

  StoreSDNode *Store = cast<StoreSDNode>(Op);
  SDValue Chain = Store->getChain();
  if ((Store->getAddressSpace() == AMDGPUAS::LOCAL_ADDRESS ||
       Store->getAddressSpace() == AMDGPUAS::PRIVATE_ADDRESS) &&
      Store->getValue().getValueType().isVector()) {
    return SplitVectorStore(Op, DAG);
  }

  EVT MemVT = Store->getMemoryVT();
  if (Store->getAddressSpace() == AMDGPUAS::PRIVATE_ADDRESS &&
      MemVT.bitsLT(MVT::i32)) {
    unsigned Mask = 0;
    if (Store->getMemoryVT() == MVT::i8) {
      Mask = 0xff;
    } else if (Store->getMemoryVT() == MVT::i16) {
      Mask = 0xffff;
    }
    SDValue BasePtr = Store->getBasePtr();
    SDValue Ptr = DAG.getNode(ISD::SRL, DL, MVT::i32, BasePtr,
                              DAG.getConstant(2, MVT::i32));
    SDValue Dst = DAG.getNode(AMDGPUISD::REGISTER_LOAD, DL, MVT::i32,
                              Chain, Ptr, DAG.getTargetConstant(0, MVT::i32));

    SDValue ByteIdx = DAG.getNode(ISD::AND, DL, MVT::i32, BasePtr,
                                  DAG.getConstant(0x3, MVT::i32));

    SDValue ShiftAmt = DAG.getNode(ISD::SHL, DL, MVT::i32, ByteIdx,
                                   DAG.getConstant(3, MVT::i32));

    SDValue SExtValue = DAG.getNode(ISD::SIGN_EXTEND, DL, MVT::i32,
                                    Store->getValue());

    SDValue MaskedValue = DAG.getZeroExtendInReg(SExtValue, DL, MemVT);

    SDValue ShiftedValue = DAG.getNode(ISD::SHL, DL, MVT::i32,
                                       MaskedValue, ShiftAmt);

    SDValue DstMask = DAG.getNode(ISD::SHL, DL, MVT::i32, DAG.getConstant(Mask, MVT::i32),
                                  ShiftAmt);
    DstMask = DAG.getNode(ISD::XOR, DL, MVT::i32, DstMask,
                          DAG.getConstant(0xffffffff, MVT::i32));
    Dst = DAG.getNode(ISD::AND, DL, MVT::i32, Dst, DstMask);

    SDValue Value = DAG.getNode(ISD::OR, DL, MVT::i32, Dst, ShiftedValue);
    return DAG.getNode(AMDGPUISD::REGISTER_STORE, DL, MVT::Other,
                       Chain, Value, Ptr, DAG.getTargetConstant(0, MVT::i32));
  }
  return SDValue();
}

SDValue AMDGPUTargetLowering::LowerUDIVREM(SDValue Op,
    SelectionDAG &DAG) const {
  SDLoc DL(Op);
  EVT VT = Op.getValueType();

  SDValue Num = Op.getOperand(0);
  SDValue Den = Op.getOperand(1);

  SmallVector<SDValue, 8> Results;

  // RCP =  URECIP(Den) = 2^32 / Den + e
  // e is rounding error.
  SDValue RCP = DAG.getNode(AMDGPUISD::URECIP, DL, VT, Den);

  // RCP_LO = umulo(RCP, Den) */
  SDValue RCP_LO = DAG.getNode(ISD::UMULO, DL, VT, RCP, Den);

  // RCP_HI = mulhu (RCP, Den) */
  SDValue RCP_HI = DAG.getNode(ISD::MULHU, DL, VT, RCP, Den);

  // NEG_RCP_LO = -RCP_LO
  SDValue NEG_RCP_LO = DAG.getNode(ISD::SUB, DL, VT, DAG.getConstant(0, VT),
                                                     RCP_LO);

  // ABS_RCP_LO = (RCP_HI == 0 ? NEG_RCP_LO : RCP_LO)
  SDValue ABS_RCP_LO = DAG.getSelectCC(DL, RCP_HI, DAG.getConstant(0, VT),
                                           NEG_RCP_LO, RCP_LO,
                                           ISD::SETEQ);
  // Calculate the rounding error from the URECIP instruction
  // E = mulhu(ABS_RCP_LO, RCP)
  SDValue E = DAG.getNode(ISD::MULHU, DL, VT, ABS_RCP_LO, RCP);

  // RCP_A_E = RCP + E
  SDValue RCP_A_E = DAG.getNode(ISD::ADD, DL, VT, RCP, E);

  // RCP_S_E = RCP - E
  SDValue RCP_S_E = DAG.getNode(ISD::SUB, DL, VT, RCP, E);

  // Tmp0 = (RCP_HI == 0 ? RCP_A_E : RCP_SUB_E)
  SDValue Tmp0 = DAG.getSelectCC(DL, RCP_HI, DAG.getConstant(0, VT),
                                     RCP_A_E, RCP_S_E,
                                     ISD::SETEQ);
  // Quotient = mulhu(Tmp0, Num)
  SDValue Quotient = DAG.getNode(ISD::MULHU, DL, VT, Tmp0, Num);

  // Num_S_Remainder = Quotient * Den
  SDValue Num_S_Remainder = DAG.getNode(ISD::UMULO, DL, VT, Quotient, Den);

  // Remainder = Num - Num_S_Remainder
  SDValue Remainder = DAG.getNode(ISD::SUB, DL, VT, Num, Num_S_Remainder);

  // Remainder_GE_Den = (Remainder >= Den ? -1 : 0)
  SDValue Remainder_GE_Den = DAG.getSelectCC(DL, Remainder, Den,
                                                 DAG.getConstant(-1, VT),
                                                 DAG.getConstant(0, VT),
                                                 ISD::SETUGE);
  // Remainder_GE_Zero = (Num >= Num_S_Remainder ? -1 : 0)
  SDValue Remainder_GE_Zero = DAG.getSelectCC(DL, Num,
                                                  Num_S_Remainder,
                                                  DAG.getConstant(-1, VT),
                                                  DAG.getConstant(0, VT),
                                                  ISD::SETUGE);
  // Tmp1 = Remainder_GE_Den & Remainder_GE_Zero
  SDValue Tmp1 = DAG.getNode(ISD::AND, DL, VT, Remainder_GE_Den,
                                               Remainder_GE_Zero);

  // Calculate Division result:

  // Quotient_A_One = Quotient + 1
  SDValue Quotient_A_One = DAG.getNode(ISD::ADD, DL, VT, Quotient,
                                                         DAG.getConstant(1, VT));

  // Quotient_S_One = Quotient - 1
  SDValue Quotient_S_One = DAG.getNode(ISD::SUB, DL, VT, Quotient,
                                                         DAG.getConstant(1, VT));

  // Div = (Tmp1 == 0 ? Quotient : Quotient_A_One)
  SDValue Div = DAG.getSelectCC(DL, Tmp1, DAG.getConstant(0, VT),
                                     Quotient, Quotient_A_One, ISD::SETEQ);

  // Div = (Remainder_GE_Zero == 0 ? Quotient_S_One : Div)
  Div = DAG.getSelectCC(DL, Remainder_GE_Zero, DAG.getConstant(0, VT),
                            Quotient_S_One, Div, ISD::SETEQ);

  // Calculate Rem result:

  // Remainder_S_Den = Remainder - Den
  SDValue Remainder_S_Den = DAG.getNode(ISD::SUB, DL, VT, Remainder, Den);

  // Remainder_A_Den = Remainder + Den
  SDValue Remainder_A_Den = DAG.getNode(ISD::ADD, DL, VT, Remainder, Den);

  // Rem = (Tmp1 == 0 ? Remainder : Remainder_S_Den)
  SDValue Rem = DAG.getSelectCC(DL, Tmp1, DAG.getConstant(0, VT),
                                    Remainder, Remainder_S_Den, ISD::SETEQ);

  // Rem = (Remainder_GE_Zero == 0 ? Remainder_A_Den : Rem)
  Rem = DAG.getSelectCC(DL, Remainder_GE_Zero, DAG.getConstant(0, VT),
                            Remainder_A_Den, Rem, ISD::SETEQ);
  SDValue Ops[2];
  Ops[0] = Div;
  Ops[1] = Rem;
  return DAG.getMergeValues(Ops, 2, DL);
}

SDValue AMDGPUTargetLowering::LowerUINT_TO_FP(SDValue Op,
                                               SelectionDAG &DAG) const {
  SDValue S0 = Op.getOperand(0);
  SDLoc DL(Op);
  if (Op.getValueType() != MVT::f32 || S0.getValueType() != MVT::i64)
    return SDValue();

  // f32 uint_to_fp i64
  SDValue Lo = DAG.getNode(ISD::EXTRACT_ELEMENT, DL, MVT::i32, S0,
                           DAG.getConstant(0, MVT::i32));
  SDValue FloatLo = DAG.getNode(ISD::UINT_TO_FP, DL, MVT::f32, Lo);
  SDValue Hi = DAG.getNode(ISD::EXTRACT_ELEMENT, DL, MVT::i32, S0,
                           DAG.getConstant(1, MVT::i32));
  SDValue FloatHi = DAG.getNode(ISD::UINT_TO_FP, DL, MVT::f32, Hi);
  FloatHi = DAG.getNode(ISD::FMUL, DL, MVT::f32, FloatHi,
                        DAG.getConstantFP(4294967296.0f, MVT::f32)); // 2^32
  return DAG.getNode(ISD::FADD, DL, MVT::f32, FloatLo, FloatHi);

}

SDValue AMDGPUTargetLowering::ExpandSIGN_EXTEND_INREG(SDValue Op,
                                                      unsigned BitsDiff,
                                                      SelectionDAG &DAG) const {
  MVT VT = Op.getSimpleValueType();
  SDLoc DL(Op);
  SDValue Shift = DAG.getConstant(BitsDiff, VT);
  // Shift left by 'Shift' bits.
  SDValue Shl = DAG.getNode(ISD::SHL, DL, VT, Op.getOperand(0), Shift);
  // Signed shift Right by 'Shift' bits.
  return DAG.getNode(ISD::SRA, DL, VT, Shl, Shift);
}

SDValue AMDGPUTargetLowering::LowerSIGN_EXTEND_INREG(SDValue Op,
                                                     SelectionDAG &DAG) const {
  EVT ExtraVT = cast<VTSDNode>(Op.getOperand(1))->getVT();
  MVT VT = Op.getSimpleValueType();
  MVT ScalarVT = VT.getScalarType();

  unsigned SrcBits = ExtraVT.getScalarType().getSizeInBits();
  unsigned DestBits = ScalarVT.getSizeInBits();
  unsigned BitsDiff = DestBits - SrcBits;

  if (!Subtarget->hasBFE())
    return ExpandSIGN_EXTEND_INREG(Op, BitsDiff, DAG);

  SDValue Src = Op.getOperand(0);
  if (VT.isVector()) {
    SDLoc DL(Op);
    // Need to scalarize this, and revisit each of the scalars later.
    // TODO: Don't scalarize on Evergreen?
    unsigned NElts = VT.getVectorNumElements();
    SmallVector<SDValue, 8> Args;
    ExtractVectorElements(Src, DAG, Args, 0, NElts);

    SDValue VTOp = DAG.getValueType(ExtraVT.getScalarType());
    for (unsigned I = 0; I < NElts; ++I)
      Args[I] = DAG.getNode(ISD::SIGN_EXTEND_INREG, DL, ScalarVT, Args[I], VTOp);

    return DAG.getNode(ISD::BUILD_VECTOR, DL, VT, Args.data(), Args.size());
  }

  if (SrcBits == 32) {
    SDLoc DL(Op);

    // If the source is 32-bits, this is really half of a 2-register pair, and
    // we need to discard the unused half of the pair.
    SDValue TruncSrc = DAG.getNode(ISD::TRUNCATE, DL, MVT::i32, Src);
    return DAG.getNode(ISD::SIGN_EXTEND, DL, VT, TruncSrc);
  }

  unsigned NElts = VT.isVector() ? VT.getVectorNumElements() : 1;

  // TODO: Match 64-bit BFE. SI has a 64-bit BFE, but it's scalar only so it
  // might not be worth the effort, and will need to expand to shifts when
  // fixing SGPR copies.
  if (SrcBits < 32 && DestBits <= 32) {
    SDLoc DL(Op);
    MVT ExtVT = (NElts == 1) ? MVT::i32 : MVT::getVectorVT(MVT::i32, NElts);

    if (DestBits != 32)
      Src = DAG.getNode(ISD::ZERO_EXTEND, DL, ExtVT, Src);

    // FIXME: This should use TargetConstant, but that hits assertions for
    // Evergreen.
    SDValue Ext = DAG.getNode(AMDGPUISD::BFE_I32, DL, ExtVT,
                              Op.getOperand(0), // Operand
                              DAG.getConstant(0, ExtVT), // Offset
                              DAG.getConstant(SrcBits, ExtVT)); // Width

    // Truncate to the original type if necessary.
    if (ScalarVT == MVT::i32)
      return Ext;
    return DAG.getNode(ISD::TRUNCATE, DL, VT, Ext);
  }

  // For small types, extend to 32-bits first.
  if (SrcBits < 32) {
    SDLoc DL(Op);
    MVT ExtVT = (NElts == 1) ? MVT::i32 : MVT::getVectorVT(MVT::i32, NElts);

    SDValue TruncSrc = DAG.getNode(ISD::TRUNCATE, DL, ExtVT, Src);
    SDValue Ext32 = DAG.getNode(AMDGPUISD::BFE_I32,
                                DL,
                                ExtVT,
                                TruncSrc, // Operand
                                DAG.getConstant(0, ExtVT), // Offset
                                DAG.getConstant(SrcBits, ExtVT)); // Width

    return DAG.getNode(ISD::SIGN_EXTEND, DL, VT, Ext32);
  }

  // For everything else, use the standard bitshift expansion.
  return ExpandSIGN_EXTEND_INREG(Op, BitsDiff, DAG);
}

//===----------------------------------------------------------------------===//
// Helper functions
//===----------------------------------------------------------------------===//

void AMDGPUTargetLowering::getOriginalFunctionArgs(
                               SelectionDAG &DAG,
                               const Function *F,
                               const SmallVectorImpl<ISD::InputArg> &Ins,
                               SmallVectorImpl<ISD::InputArg> &OrigIns) const {

  for (unsigned i = 0, e = Ins.size(); i < e; ++i) {
    if (Ins[i].ArgVT == Ins[i].VT) {
      OrigIns.push_back(Ins[i]);
      continue;
    }

    EVT VT;
    if (Ins[i].ArgVT.isVector() && !Ins[i].VT.isVector()) {
      // Vector has been split into scalars.
      VT = Ins[i].ArgVT.getVectorElementType();
    } else if (Ins[i].VT.isVector() && Ins[i].ArgVT.isVector() &&
               Ins[i].ArgVT.getVectorElementType() !=
               Ins[i].VT.getVectorElementType()) {
      // Vector elements have been promoted
      VT = Ins[i].ArgVT;
    } else {
      // Vector has been spilt into smaller vectors.
      VT = Ins[i].VT;
    }

    ISD::InputArg Arg(Ins[i].Flags, VT, VT, Ins[i].Used,
                      Ins[i].OrigArgIndex, Ins[i].PartOffset);
    OrigIns.push_back(Arg);
  }
}

bool AMDGPUTargetLowering::isHWTrueValue(SDValue Op) const {
  if (ConstantFPSDNode * CFP = dyn_cast<ConstantFPSDNode>(Op)) {
    return CFP->isExactlyValue(1.0);
  }
  if (ConstantSDNode *C = dyn_cast<ConstantSDNode>(Op)) {
    return C->isAllOnesValue();
  }
  return false;
}

bool AMDGPUTargetLowering::isHWFalseValue(SDValue Op) const {
  if (ConstantFPSDNode * CFP = dyn_cast<ConstantFPSDNode>(Op)) {
    return CFP->getValueAPF().isZero();
  }
  if (ConstantSDNode *C = dyn_cast<ConstantSDNode>(Op)) {
    return C->isNullValue();
  }
  return false;
}

SDValue AMDGPUTargetLowering::CreateLiveInRegister(SelectionDAG &DAG,
                                                  const TargetRegisterClass *RC,
                                                   unsigned Reg, EVT VT) const {
  MachineFunction &MF = DAG.getMachineFunction();
  MachineRegisterInfo &MRI = MF.getRegInfo();
  unsigned VirtualRegister;
  if (!MRI.isLiveIn(Reg)) {
    VirtualRegister = MRI.createVirtualRegister(RC);
    MRI.addLiveIn(Reg, VirtualRegister);
  } else {
    VirtualRegister = MRI.getLiveInVirtReg(Reg);
  }
  return DAG.getRegister(VirtualRegister, VT);
}

#define NODE_NAME_CASE(node) case AMDGPUISD::node: return #node;

const char* AMDGPUTargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch (Opcode) {
  default: return 0;
  // AMDIL DAG nodes
  NODE_NAME_CASE(CALL);
  NODE_NAME_CASE(UMUL);
  NODE_NAME_CASE(DIV_INF);
  NODE_NAME_CASE(RET_FLAG);
  NODE_NAME_CASE(BRANCH_COND);

  // AMDGPU DAG nodes
  NODE_NAME_CASE(DWORDADDR)
  NODE_NAME_CASE(FRACT)
  NODE_NAME_CASE(FMAX)
  NODE_NAME_CASE(SMAX)
  NODE_NAME_CASE(UMAX)
  NODE_NAME_CASE(FMIN)
  NODE_NAME_CASE(SMIN)
  NODE_NAME_CASE(UMIN)
  NODE_NAME_CASE(BFE_U32)
  NODE_NAME_CASE(BFE_I32)
  NODE_NAME_CASE(BFI)
  NODE_NAME_CASE(BFM)
  NODE_NAME_CASE(URECIP)
  NODE_NAME_CASE(DOT4)
  NODE_NAME_CASE(EXPORT)
  NODE_NAME_CASE(CONST_ADDRESS)
  NODE_NAME_CASE(REGISTER_LOAD)
  NODE_NAME_CASE(REGISTER_STORE)
  NODE_NAME_CASE(LOAD_CONSTANT)
  NODE_NAME_CASE(LOAD_INPUT)
  NODE_NAME_CASE(SAMPLE)
  NODE_NAME_CASE(SAMPLEB)
  NODE_NAME_CASE(SAMPLED)
  NODE_NAME_CASE(SAMPLEL)
  NODE_NAME_CASE(STORE_MSKOR)
  NODE_NAME_CASE(TBUFFER_STORE_FORMAT)
  }
}

void AMDGPUTargetLowering::computeMaskedBitsForTargetNode(
  const SDValue Op,
  APInt &KnownZero,
  APInt &KnownOne,
  const SelectionDAG &DAG,
  unsigned Depth) const {
  KnownZero = KnownOne = APInt(KnownOne.getBitWidth(), 0); // Don't know anything.
}
