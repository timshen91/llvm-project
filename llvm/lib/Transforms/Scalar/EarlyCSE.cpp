//===- EarlyCSE.cpp - Simple and fast CSE pass ----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This pass performs a simple dominator tree walk that eliminates trivially
// redundant instructions.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "early-cse"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/Dominators.h"
#include "llvm/Analysis/InstructionSimplify.h"
#include "llvm/Analysis/InstructionSimplify.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/ADT/ScopedHashTable.h"
using namespace llvm;

namespace {
  /// InstValue - Instances of this struct represent available values in the
  /// scoped hash table.
  struct InstValue {
    Instruction *Inst;
    
    bool isSentinel() const {
      return Inst == DenseMapInfo<Instruction*>::getEmptyKey() ||
             Inst == DenseMapInfo<Instruction*>::getTombstoneKey();
    }
    
    static bool canHandle(Instruction *Inst) {
      return isa<CastInst>(Inst);
    }
    
    static InstValue get(Instruction *I) {
      InstValue X; X.Inst = I;
      assert((X.isSentinel() || canHandle(I)) && "Inst can't be handled!");
      return X;
    }
  };
}

namespace llvm {
// InstValue is POD.
template<> struct isPodLike<InstValue> {
  static const bool value = true;
};

template<> struct DenseMapInfo<InstValue> {
  static inline InstValue getEmptyKey() {
    return InstValue::get(DenseMapInfo<Instruction*>::getEmptyKey());
  }
  static inline InstValue getTombstoneKey() {
    return InstValue::get(DenseMapInfo<Instruction*>::getTombstoneKey());
  }
  static unsigned getHashValue(InstValue Val);
  static bool isEqual(InstValue LHS, InstValue RHS);
};
}

unsigned getHash(const void *V) {
  return DenseMapInfo<const void*>::getHashValue(V);
}

unsigned DenseMapInfo<InstValue>::getHashValue(InstValue Val) {
  Instruction *Inst = Val.Inst;
  unsigned Res = 0;
  if (CastInst *CI = dyn_cast<CastInst>(Inst))
    Res = getHash(CI->getOperand(0)) ^ getHash(CI->getType());
  else
    assert(0 && "Unhandled instruction kind");
  
  return (Res << 1) ^ Inst->getOpcode();
}

bool DenseMapInfo<InstValue>::isEqual(InstValue LHS, InstValue RHS) {
  Instruction *LHSI = LHS.Inst, *RHSI = RHS.Inst;

  if (LHS.isSentinel() || RHS.isSentinel())
    return LHSI == RHSI;
  
  if (LHSI->getOpcode() != RHSI->getOpcode()) return false;
  return LHSI->isIdenticalTo(RHSI);
}


namespace {
  
/// EarlyCSE - This pass does a simple depth-first walk over the dominator
/// tree, eliminating trivially redundant instructions and using instsimplify
/// to canonicalize things as it goes.  It is intended to be fast and catch
/// obvious cases so that instcombine and other passes are more effective.  It
/// is expected that a later pass of GVN will catch the interesting/hard
/// cases.
class EarlyCSE : public FunctionPass {
public:
  const TargetData *TD;
  DominatorTree *DT;
  ScopedHashTable<InstValue, Instruction*> *AvailableValues;
  
  static char ID;
  explicit EarlyCSE()
      : FunctionPass(ID) {
    initializeEarlyCSEPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F);

private:
  
  bool processNode(DomTreeNode *Node);
  
  // This transformation requires dominator postdominator info
  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<DominatorTree>();
    AU.setPreservesCFG();
  }
};
}

char EarlyCSE::ID = 0;

// createEarlyCSEPass - The public interface to this file.
FunctionPass *llvm::createEarlyCSEPass() {
  return new EarlyCSE();
}

INITIALIZE_PASS_BEGIN(EarlyCSE, "early-cse", "Early CSE", false, false)
INITIALIZE_PASS_DEPENDENCY(DominatorTree)
INITIALIZE_PASS_END(EarlyCSE, "early-cse", "Early CSE", false, false)

// FIXME: Should bump pointer allocate entries in scoped hash table.

bool EarlyCSE::processNode(DomTreeNode *Node) {
  // Define a scope in the scoped hash table.
  ScopedHashTableScope<InstValue, Instruction*> Scope(*AvailableValues);
  
  BasicBlock *BB = Node->getBlock();
  
  bool Changed = false;

  // See if any instructions in the block can be eliminated.  If so, do it.  If
  // not, add them to AvailableValues.
  for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E; ) {
    Instruction *Inst = I++;
    
    // Dead instructions should just be removed.
    if (isInstructionTriviallyDead(Inst)) {
      Inst->eraseFromParent();
      Changed = true;
      continue;
    }
    
    // If the instruction can be simplified (e.g. X+0 = X) then replace it with
    // its simpler value.
    if (Value *V = SimplifyInstruction(Inst, TD, DT)) {
      Inst->replaceAllUsesWith(V);
      Inst->eraseFromParent();
      Changed = true;
      continue;
    }
    
    // If this instruction is something that we can't value number, ignore it.
    if (!InstValue::canHandle(Inst))
      continue;
    
    // See if the instruction has an available value.  If so, use it.
    if (Instruction *V = AvailableValues->lookup(InstValue::get(Inst))) {
      Inst->replaceAllUsesWith(V);
      Inst->eraseFromParent();
      Changed = true;
      continue;
    }
    
    // Otherwise, just remember that this value is available.
    AvailableValues->insert(InstValue::get(Inst), Inst);
  }
  
  
  for (DomTreeNode::iterator I = Node->begin(), E = Node->end(); I != E; ++I)
    Changed |= processNode(*I);
  return Changed;
}


bool EarlyCSE::runOnFunction(Function &F) {
  TD = getAnalysisIfAvailable<TargetData>();
  DT = &getAnalysis<DominatorTree>();
  ScopedHashTable<InstValue, Instruction*> AVTable;
  AvailableValues = &AVTable;
  return processNode(DT->getRootNode());
}

