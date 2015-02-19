//===- ARCInstKind.cpp - ObjC ARC Optimization ----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
/// \file
/// This file defines several utility functions used by various ARC
/// optimizations which are IMHO too big to be in a header file.
///
/// WARNING: This file knows about certain library functions. It recognizes them
/// by name, and hardwires knowledge of their semantics.
///
/// WARNING: This file knows about how certain Objective-C library functions are
/// used. Naive LLVM IR transformations which would otherwise be
/// behavior-preserving may break these assumptions.
///
//===----------------------------------------------------------------------===//

#include "ObjCARC.h"
#include "llvm/IR/Intrinsics.h"

using namespace llvm;
using namespace llvm::objcarc;

raw_ostream &llvm::objcarc::operator<<(raw_ostream &OS,
                                       const ARCInstKind Class) {
  switch (Class) {
  case ARCInstKind::Retain:
    return OS << "ARCInstKind::Retain";
  case ARCInstKind::RetainRV:
    return OS << "ARCInstKind::RetainRV";
  case ARCInstKind::RetainBlock:
    return OS << "ARCInstKind::RetainBlock";
  case ARCInstKind::Release:
    return OS << "ARCInstKind::Release";
  case ARCInstKind::Autorelease:
    return OS << "ARCInstKind::Autorelease";
  case ARCInstKind::AutoreleaseRV:
    return OS << "ARCInstKind::AutoreleaseRV";
  case ARCInstKind::AutoreleasepoolPush:
    return OS << "ARCInstKind::AutoreleasepoolPush";
  case ARCInstKind::AutoreleasepoolPop:
    return OS << "ARCInstKind::AutoreleasepoolPop";
  case ARCInstKind::NoopCast:
    return OS << "ARCInstKind::NoopCast";
  case ARCInstKind::FusedRetainAutorelease:
    return OS << "ARCInstKind::FusedRetainAutorelease";
  case ARCInstKind::FusedRetainAutoreleaseRV:
    return OS << "ARCInstKind::FusedRetainAutoreleaseRV";
  case ARCInstKind::LoadWeakRetained:
    return OS << "ARCInstKind::LoadWeakRetained";
  case ARCInstKind::StoreWeak:
    return OS << "ARCInstKind::StoreWeak";
  case ARCInstKind::InitWeak:
    return OS << "ARCInstKind::InitWeak";
  case ARCInstKind::LoadWeak:
    return OS << "ARCInstKind::LoadWeak";
  case ARCInstKind::MoveWeak:
    return OS << "ARCInstKind::MoveWeak";
  case ARCInstKind::CopyWeak:
    return OS << "ARCInstKind::CopyWeak";
  case ARCInstKind::DestroyWeak:
    return OS << "ARCInstKind::DestroyWeak";
  case ARCInstKind::StoreStrong:
    return OS << "ARCInstKind::StoreStrong";
  case ARCInstKind::CallOrUser:
    return OS << "ARCInstKind::CallOrUser";
  case ARCInstKind::Call:
    return OS << "ARCInstKind::Call";
  case ARCInstKind::User:
    return OS << "ARCInstKind::User";
  case ARCInstKind::IntrinsicUser:
    return OS << "ARCInstKind::IntrinsicUser";
  case ARCInstKind::None:
    return OS << "ARCInstKind::None";
  }
  llvm_unreachable("Unknown instruction class!");
}

ARCInstKind llvm::objcarc::GetFunctionClass(const Function *F) {
  Function::const_arg_iterator AI = F->arg_begin(), AE = F->arg_end();

  // No (mandatory) arguments.
  if (AI == AE)
    return StringSwitch<ARCInstKind>(F->getName())
        .Case("objc_autoreleasePoolPush", ARCInstKind::AutoreleasepoolPush)
        .Case("clang.arc.use", ARCInstKind::IntrinsicUser)
        .Default(ARCInstKind::CallOrUser);

  // One argument.
  const Argument *A0 = AI++;
  if (AI == AE)
    // Argument is a pointer.
    if (PointerType *PTy = dyn_cast<PointerType>(A0->getType())) {
      Type *ETy = PTy->getElementType();
      // Argument is i8*.
      if (ETy->isIntegerTy(8))
        return StringSwitch<ARCInstKind>(F->getName())
            .Case("objc_retain", ARCInstKind::Retain)
            .Case("objc_retainAutoreleasedReturnValue", ARCInstKind::RetainRV)
            .Case("objc_retainBlock", ARCInstKind::RetainBlock)
            .Case("objc_release", ARCInstKind::Release)
            .Case("objc_autorelease", ARCInstKind::Autorelease)
            .Case("objc_autoreleaseReturnValue", ARCInstKind::AutoreleaseRV)
            .Case("objc_autoreleasePoolPop", ARCInstKind::AutoreleasepoolPop)
            .Case("objc_retainedObject", ARCInstKind::NoopCast)
            .Case("objc_unretainedObject", ARCInstKind::NoopCast)
            .Case("objc_unretainedPointer", ARCInstKind::NoopCast)
            .Case("objc_retain_autorelease",
                  ARCInstKind::FusedRetainAutorelease)
            .Case("objc_retainAutorelease", ARCInstKind::FusedRetainAutorelease)
            .Case("objc_retainAutoreleaseReturnValue",
                  ARCInstKind::FusedRetainAutoreleaseRV)
            .Case("objc_sync_enter", ARCInstKind::User)
            .Case("objc_sync_exit", ARCInstKind::User)
            .Default(ARCInstKind::CallOrUser);

      // Argument is i8**
      if (PointerType *Pte = dyn_cast<PointerType>(ETy))
        if (Pte->getElementType()->isIntegerTy(8))
          return StringSwitch<ARCInstKind>(F->getName())
              .Case("objc_loadWeakRetained", ARCInstKind::LoadWeakRetained)
              .Case("objc_loadWeak", ARCInstKind::LoadWeak)
              .Case("objc_destroyWeak", ARCInstKind::DestroyWeak)
              .Default(ARCInstKind::CallOrUser);
    }

  // Two arguments, first is i8**.
  const Argument *A1 = AI++;
  if (AI == AE)
    if (PointerType *PTy = dyn_cast<PointerType>(A0->getType()))
      if (PointerType *Pte = dyn_cast<PointerType>(PTy->getElementType()))
        if (Pte->getElementType()->isIntegerTy(8))
          if (PointerType *PTy1 = dyn_cast<PointerType>(A1->getType())) {
            Type *ETy1 = PTy1->getElementType();
            // Second argument is i8*
            if (ETy1->isIntegerTy(8))
              return StringSwitch<ARCInstKind>(F->getName())
                  .Case("objc_storeWeak", ARCInstKind::StoreWeak)
                  .Case("objc_initWeak", ARCInstKind::InitWeak)
                  .Case("objc_storeStrong", ARCInstKind::StoreStrong)
                  .Default(ARCInstKind::CallOrUser);
            // Second argument is i8**.
            if (PointerType *Pte1 = dyn_cast<PointerType>(ETy1))
              if (Pte1->getElementType()->isIntegerTy(8))
                return StringSwitch<ARCInstKind>(F->getName())
                    .Case("objc_moveWeak", ARCInstKind::MoveWeak)
                    .Case("objc_copyWeak", ARCInstKind::CopyWeak)
                    // Ignore annotation calls. This is important to stop the
                    // optimizer from treating annotations as uses which would
                    // make the state of the pointers they are attempting to
                    // elucidate to be incorrect.
                    .Case("llvm.arc.annotation.topdown.bbstart",
                          ARCInstKind::None)
                    .Case("llvm.arc.annotation.topdown.bbend",
                          ARCInstKind::None)
                    .Case("llvm.arc.annotation.bottomup.bbstart",
                          ARCInstKind::None)
                    .Case("llvm.arc.annotation.bottomup.bbend",
                          ARCInstKind::None)
                    .Default(ARCInstKind::CallOrUser);
          }

  // Anything else.
  return ARCInstKind::CallOrUser;
}

/// \brief Determine what kind of construct V is.
ARCInstKind llvm::objcarc::GetARCInstKind(const Value *V) {
  if (const Instruction *I = dyn_cast<Instruction>(V)) {
    // Any instruction other than bitcast and gep with a pointer operand have a
    // use of an objc pointer. Bitcasts, GEPs, Selects, PHIs transfer a pointer
    // to a subsequent use, rather than using it themselves, in this sense.
    // As a short cut, several other opcodes are known to have no pointer
    // operands of interest. And ret is never followed by a release, so it's
    // not interesting to examine.
    switch (I->getOpcode()) {
    case Instruction::Call: {
      const CallInst *CI = cast<CallInst>(I);
      // Check for calls to special functions.
      if (const Function *F = CI->getCalledFunction()) {
        ARCInstKind Class = GetFunctionClass(F);
        if (Class != ARCInstKind::CallOrUser)
          return Class;

        // None of the intrinsic functions do objc_release. For intrinsics, the
        // only question is whether or not they may be users.
        switch (F->getIntrinsicID()) {
        case Intrinsic::returnaddress:
        case Intrinsic::frameaddress:
        case Intrinsic::stacksave:
        case Intrinsic::stackrestore:
        case Intrinsic::vastart:
        case Intrinsic::vacopy:
        case Intrinsic::vaend:
        case Intrinsic::objectsize:
        case Intrinsic::prefetch:
        case Intrinsic::stackprotector:
        case Intrinsic::eh_return_i32:
        case Intrinsic::eh_return_i64:
        case Intrinsic::eh_typeid_for:
        case Intrinsic::eh_dwarf_cfa:
        case Intrinsic::eh_sjlj_lsda:
        case Intrinsic::eh_sjlj_functioncontext:
        case Intrinsic::init_trampoline:
        case Intrinsic::adjust_trampoline:
        case Intrinsic::lifetime_start:
        case Intrinsic::lifetime_end:
        case Intrinsic::invariant_start:
        case Intrinsic::invariant_end:
        // Don't let dbg info affect our results.
        case Intrinsic::dbg_declare:
        case Intrinsic::dbg_value:
          // Short cut: Some intrinsics obviously don't use ObjC pointers.
          return ARCInstKind::None;
        default:
          break;
        }
      }
      return GetCallSiteClass(CI);
    }
    case Instruction::Invoke:
      return GetCallSiteClass(cast<InvokeInst>(I));
    case Instruction::BitCast:
    case Instruction::GetElementPtr:
    case Instruction::Select:
    case Instruction::PHI:
    case Instruction::Ret:
    case Instruction::Br:
    case Instruction::Switch:
    case Instruction::IndirectBr:
    case Instruction::Alloca:
    case Instruction::VAArg:
    case Instruction::Add:
    case Instruction::FAdd:
    case Instruction::Sub:
    case Instruction::FSub:
    case Instruction::Mul:
    case Instruction::FMul:
    case Instruction::SDiv:
    case Instruction::UDiv:
    case Instruction::FDiv:
    case Instruction::SRem:
    case Instruction::URem:
    case Instruction::FRem:
    case Instruction::Shl:
    case Instruction::LShr:
    case Instruction::AShr:
    case Instruction::And:
    case Instruction::Or:
    case Instruction::Xor:
    case Instruction::SExt:
    case Instruction::ZExt:
    case Instruction::Trunc:
    case Instruction::IntToPtr:
    case Instruction::FCmp:
    case Instruction::FPTrunc:
    case Instruction::FPExt:
    case Instruction::FPToUI:
    case Instruction::FPToSI:
    case Instruction::UIToFP:
    case Instruction::SIToFP:
    case Instruction::InsertElement:
    case Instruction::ExtractElement:
    case Instruction::ShuffleVector:
    case Instruction::ExtractValue:
      break;
    case Instruction::ICmp:
      // Comparing a pointer with null, or any other constant, isn't an
      // interesting use, because we don't care what the pointer points to, or
      // about the values of any other dynamic reference-counted pointers.
      if (IsPotentialRetainableObjPtr(I->getOperand(1)))
        return ARCInstKind::User;
      break;
    default:
      // For anything else, check all the operands.
      // Note that this includes both operands of a Store: while the first
      // operand isn't actually being dereferenced, it is being stored to
      // memory where we can no longer track who might read it and dereference
      // it, so we have to consider it potentially used.
      for (User::const_op_iterator OI = I->op_begin(), OE = I->op_end();
           OI != OE; ++OI)
        if (IsPotentialRetainableObjPtr(*OI))
          return ARCInstKind::User;
    }
  }

  // Otherwise, it's totally inert for ARC purposes.
  return ARCInstKind::None;
}
