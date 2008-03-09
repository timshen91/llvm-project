//===- SimplifyCFG.cpp - Code to perform CFG simplification ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Peephole optimize the CFG.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "simplifycfg"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Constants.h"
#include "llvm/Instructions.h"
#include "llvm/Type.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Support/CFG.h"
#include "llvm/Support/Debug.h"
#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/SmallPtrSet.h"
#include <algorithm>
#include <functional>
#include <set>
#include <map>
using namespace llvm;

/// SafeToMergeTerminators - Return true if it is safe to merge these two
/// terminator instructions together.
///
static bool SafeToMergeTerminators(TerminatorInst *SI1, TerminatorInst *SI2) {
  if (SI1 == SI2) return false;  // Can't merge with self!
  
  // It is not safe to merge these two switch instructions if they have a common
  // successor, and if that successor has a PHI node, and if *that* PHI node has
  // conflicting incoming values from the two switch blocks.
  BasicBlock *SI1BB = SI1->getParent();
  BasicBlock *SI2BB = SI2->getParent();
  SmallPtrSet<BasicBlock*, 16> SI1Succs(succ_begin(SI1BB), succ_end(SI1BB));
  
  for (succ_iterator I = succ_begin(SI2BB), E = succ_end(SI2BB); I != E; ++I)
    if (SI1Succs.count(*I))
      for (BasicBlock::iterator BBI = (*I)->begin();
           isa<PHINode>(BBI); ++BBI) {
        PHINode *PN = cast<PHINode>(BBI);
        if (PN->getIncomingValueForBlock(SI1BB) !=
            PN->getIncomingValueForBlock(SI2BB))
          return false;
      }
        
  return true;
}

/// AddPredecessorToBlock - Update PHI nodes in Succ to indicate that there will
/// now be entries in it from the 'NewPred' block.  The values that will be
/// flowing into the PHI nodes will be the same as those coming in from
/// ExistPred, an existing predecessor of Succ.
static void AddPredecessorToBlock(BasicBlock *Succ, BasicBlock *NewPred,
                                  BasicBlock *ExistPred) {
  assert(std::find(succ_begin(ExistPred), succ_end(ExistPred), Succ) !=
         succ_end(ExistPred) && "ExistPred is not a predecessor of Succ!");
  if (!isa<PHINode>(Succ->begin())) return; // Quick exit if nothing to do
  
  for (BasicBlock::iterator I = Succ->begin(); isa<PHINode>(I); ++I) {
    PHINode *PN = cast<PHINode>(I);
    Value *V = PN->getIncomingValueForBlock(ExistPred);
    PN->addIncoming(V, NewPred);
  }
}

// CanPropagatePredecessorsForPHIs - Return true if we can fold BB, an
// almost-empty BB ending in an unconditional branch to Succ, into succ.
//
// Assumption: Succ is the single successor for BB.
//
static bool CanPropagatePredecessorsForPHIs(BasicBlock *BB, BasicBlock *Succ) {
  assert(*succ_begin(BB) == Succ && "Succ is not successor of BB!");

  // Check to see if one of the predecessors of BB is already a predecessor of
  // Succ.  If so, we cannot do the transformation if there are any PHI nodes
  // with incompatible values coming in from the two edges!
  //
  if (isa<PHINode>(Succ->front())) {
    SmallPtrSet<BasicBlock*, 16> BBPreds(pred_begin(BB), pred_end(BB));
    for (pred_iterator PI = pred_begin(Succ), PE = pred_end(Succ);
         PI != PE; ++PI)
      if (BBPreds.count(*PI)) {
        // Loop over all of the PHI nodes checking to see if there are
        // incompatible values coming in.
        for (BasicBlock::iterator I = Succ->begin(); isa<PHINode>(I); ++I) {
          PHINode *PN = cast<PHINode>(I);
          // Loop up the entries in the PHI node for BB and for *PI if the
          // values coming in are non-equal, we cannot merge these two blocks
          // (instead we should insert a conditional move or something, then
          // merge the blocks).
          if (PN->getIncomingValueForBlock(BB) !=
              PN->getIncomingValueForBlock(*PI))
            return false;  // Values are not equal...
        }
      }
  }
    
  // Finally, if BB has PHI nodes that are used by things other than the PHIs in
  // Succ and Succ has predecessors that are not Succ and not Pred, we cannot
  // fold these blocks, as we don't know whether BB dominates Succ or not to
  // update the PHI nodes correctly.
  if (!isa<PHINode>(BB->begin()) || Succ->getSinglePredecessor()) return true;

  // If the predecessors of Succ are only BB, handle it.
  bool IsSafe = true;
  for (pred_iterator PI = pred_begin(Succ), E = pred_end(Succ); PI != E; ++PI)
    if (*PI != BB) {
      IsSafe = false;
      break;
    }
  if (IsSafe) return true;
  
  // If the PHI nodes in BB are only used by instructions in Succ, we are ok if
  // BB and Succ have no common predecessors.
  for (BasicBlock::iterator I = BB->begin(); isa<PHINode>(I); ++I) {
    PHINode *PN = cast<PHINode>(I);
    for (Value::use_iterator UI = PN->use_begin(), E = PN->use_end(); UI != E;
         ++UI)
      if (cast<Instruction>(*UI)->getParent() != Succ)
        return false;
  }
  
  // Scan the predecessor sets of BB and Succ, making sure there are no common
  // predecessors.  Common predecessors would cause us to build a phi node with
  // differing incoming values, which is not legal.
  SmallPtrSet<BasicBlock*, 16> BBPreds(pred_begin(BB), pred_end(BB));
  for (pred_iterator PI = pred_begin(Succ), E = pred_end(Succ); PI != E; ++PI)
    if (BBPreds.count(*PI))
      return false;
    
  return true;
}

/// TryToSimplifyUncondBranchFromEmptyBlock - BB contains an unconditional
/// branch to Succ, and contains no instructions other than PHI nodes and the
/// branch.  If possible, eliminate BB.
static bool TryToSimplifyUncondBranchFromEmptyBlock(BasicBlock *BB,
                                                    BasicBlock *Succ) {
  // If our successor has PHI nodes, then we need to update them to include
  // entries for BB's predecessors, not for BB itself.  Be careful though,
  // if this transformation fails (returns true) then we cannot do this
  // transformation!
  //
  if (!CanPropagatePredecessorsForPHIs(BB, Succ)) return false;
  
  DOUT << "Killing Trivial BB: \n" << *BB;
  
  if (isa<PHINode>(Succ->begin())) {
    // If there is more than one pred of succ, and there are PHI nodes in
    // the successor, then we need to add incoming edges for the PHI nodes
    //
    const SmallVector<BasicBlock*, 16> BBPreds(pred_begin(BB), pred_end(BB));
    
    // Loop over all of the PHI nodes in the successor of BB.
    for (BasicBlock::iterator I = Succ->begin(); isa<PHINode>(I); ++I) {
      PHINode *PN = cast<PHINode>(I);
      Value *OldVal = PN->removeIncomingValue(BB, false);
      assert(OldVal && "No entry in PHI for Pred BB!");
      
      // If this incoming value is one of the PHI nodes in BB, the new entries
      // in the PHI node are the entries from the old PHI.
      if (isa<PHINode>(OldVal) && cast<PHINode>(OldVal)->getParent() == BB) {
        PHINode *OldValPN = cast<PHINode>(OldVal);
        for (unsigned i = 0, e = OldValPN->getNumIncomingValues(); i != e; ++i)
          PN->addIncoming(OldValPN->getIncomingValue(i),
                          OldValPN->getIncomingBlock(i));
      } else {
        // Add an incoming value for each of the new incoming values.
        for (unsigned i = 0, e = BBPreds.size(); i != e; ++i)
          PN->addIncoming(OldVal, BBPreds[i]);
      }
    }
  }
  
  if (isa<PHINode>(&BB->front())) {
    SmallVector<BasicBlock*, 16>
    OldSuccPreds(pred_begin(Succ), pred_end(Succ));
    
    // Move all PHI nodes in BB to Succ if they are alive, otherwise
    // delete them.
    while (PHINode *PN = dyn_cast<PHINode>(&BB->front()))
      if (PN->use_empty()) {
        // Just remove the dead phi.  This happens if Succ's PHIs were the only
        // users of the PHI nodes.
        PN->eraseFromParent();
      } else {
        // The instruction is alive, so this means that Succ must have
        // *ONLY* had BB as a predecessor, and the PHI node is still valid
        // now.  Simply move it into Succ, because we know that BB
        // strictly dominated Succ.
        Succ->getInstList().splice(Succ->begin(),
                                   BB->getInstList(), BB->begin());
        
        // We need to add new entries for the PHI node to account for
        // predecessors of Succ that the PHI node does not take into
        // account.  At this point, since we know that BB dominated succ,
        // this means that we should any newly added incoming edges should
        // use the PHI node as the value for these edges, because they are
        // loop back edges.
        for (unsigned i = 0, e = OldSuccPreds.size(); i != e; ++i)
          if (OldSuccPreds[i] != BB)
            PN->addIncoming(PN, OldSuccPreds[i]);
      }
  }
    
  // Everything that jumped to BB now goes to Succ.
  BB->replaceAllUsesWith(Succ);
  if (!Succ->hasName()) Succ->takeName(BB);
  BB->eraseFromParent();              // Delete the old basic block.
  return true;
}

/// GetIfCondition - Given a basic block (BB) with two predecessors (and
/// presumably PHI nodes in it), check to see if the merge at this block is due
/// to an "if condition".  If so, return the boolean condition that determines
/// which entry into BB will be taken.  Also, return by references the block
/// that will be entered from if the condition is true, and the block that will
/// be entered if the condition is false.
///
///
static Value *GetIfCondition(BasicBlock *BB,
                             BasicBlock *&IfTrue, BasicBlock *&IfFalse) {
  assert(std::distance(pred_begin(BB), pred_end(BB)) == 2 &&
         "Function can only handle blocks with 2 predecessors!");
  BasicBlock *Pred1 = *pred_begin(BB);
  BasicBlock *Pred2 = *++pred_begin(BB);

  // We can only handle branches.  Other control flow will be lowered to
  // branches if possible anyway.
  if (!isa<BranchInst>(Pred1->getTerminator()) ||
      !isa<BranchInst>(Pred2->getTerminator()))
    return 0;
  BranchInst *Pred1Br = cast<BranchInst>(Pred1->getTerminator());
  BranchInst *Pred2Br = cast<BranchInst>(Pred2->getTerminator());

  // Eliminate code duplication by ensuring that Pred1Br is conditional if
  // either are.
  if (Pred2Br->isConditional()) {
    // If both branches are conditional, we don't have an "if statement".  In
    // reality, we could transform this case, but since the condition will be
    // required anyway, we stand no chance of eliminating it, so the xform is
    // probably not profitable.
    if (Pred1Br->isConditional())
      return 0;

    std::swap(Pred1, Pred2);
    std::swap(Pred1Br, Pred2Br);
  }

  if (Pred1Br->isConditional()) {
    // If we found a conditional branch predecessor, make sure that it branches
    // to BB and Pred2Br.  If it doesn't, this isn't an "if statement".
    if (Pred1Br->getSuccessor(0) == BB &&
        Pred1Br->getSuccessor(1) == Pred2) {
      IfTrue = Pred1;
      IfFalse = Pred2;
    } else if (Pred1Br->getSuccessor(0) == Pred2 &&
               Pred1Br->getSuccessor(1) == BB) {
      IfTrue = Pred2;
      IfFalse = Pred1;
    } else {
      // We know that one arm of the conditional goes to BB, so the other must
      // go somewhere unrelated, and this must not be an "if statement".
      return 0;
    }

    // The only thing we have to watch out for here is to make sure that Pred2
    // doesn't have incoming edges from other blocks.  If it does, the condition
    // doesn't dominate BB.
    if (++pred_begin(Pred2) != pred_end(Pred2))
      return 0;

    return Pred1Br->getCondition();
  }

  // Ok, if we got here, both predecessors end with an unconditional branch to
  // BB.  Don't panic!  If both blocks only have a single (identical)
  // predecessor, and THAT is a conditional branch, then we're all ok!
  if (pred_begin(Pred1) == pred_end(Pred1) ||
      ++pred_begin(Pred1) != pred_end(Pred1) ||
      pred_begin(Pred2) == pred_end(Pred2) ||
      ++pred_begin(Pred2) != pred_end(Pred2) ||
      *pred_begin(Pred1) != *pred_begin(Pred2))
    return 0;

  // Otherwise, if this is a conditional branch, then we can use it!
  BasicBlock *CommonPred = *pred_begin(Pred1);
  if (BranchInst *BI = dyn_cast<BranchInst>(CommonPred->getTerminator())) {
    assert(BI->isConditional() && "Two successors but not conditional?");
    if (BI->getSuccessor(0) == Pred1) {
      IfTrue = Pred1;
      IfFalse = Pred2;
    } else {
      IfTrue = Pred2;
      IfFalse = Pred1;
    }
    return BI->getCondition();
  }
  return 0;
}


// If we have a merge point of an "if condition" as accepted above, return true
// if the specified value dominates the block.  We don't handle the true
// generality of domination here, just a special case which works well enough
// for us.
//
// If AggressiveInsts is non-null, and if V does not dominate BB, we check to
// see if V (which must be an instruction) is cheap to compute and is
// non-trapping.  If both are true, the instruction is inserted into the set and
// true is returned.
static bool DominatesMergePoint(Value *V, BasicBlock *BB,
                                std::set<Instruction*> *AggressiveInsts) {
  Instruction *I = dyn_cast<Instruction>(V);
  if (!I) {
    // Non-instructions all dominate instructions, but not all constantexprs
    // can be executed unconditionally.
    if (ConstantExpr *C = dyn_cast<ConstantExpr>(V))
      if (C->canTrap())
        return false;
    return true;
  }
  BasicBlock *PBB = I->getParent();

  // We don't want to allow weird loops that might have the "if condition" in
  // the bottom of this block.
  if (PBB == BB) return false;

  // If this instruction is defined in a block that contains an unconditional
  // branch to BB, then it must be in the 'conditional' part of the "if
  // statement".
  if (BranchInst *BI = dyn_cast<BranchInst>(PBB->getTerminator()))
    if (BI->isUnconditional() && BI->getSuccessor(0) == BB) {
      if (!AggressiveInsts) return false;
      // Okay, it looks like the instruction IS in the "condition".  Check to
      // see if its a cheap instruction to unconditionally compute, and if it
      // only uses stuff defined outside of the condition.  If so, hoist it out.
      switch (I->getOpcode()) {
      default: return false;  // Cannot hoist this out safely.
      case Instruction::Load:
        // We can hoist loads that are non-volatile and obviously cannot trap.
        if (cast<LoadInst>(I)->isVolatile())
          return false;
        if (!isa<AllocaInst>(I->getOperand(0)) &&
            !isa<Constant>(I->getOperand(0)))
          return false;

        // Finally, we have to check to make sure there are no instructions
        // before the load in its basic block, as we are going to hoist the loop
        // out to its predecessor.
        if (PBB->begin() != BasicBlock::iterator(I))
          return false;
        break;
      case Instruction::Add:
      case Instruction::Sub:
      case Instruction::And:
      case Instruction::Or:
      case Instruction::Xor:
      case Instruction::Shl:
      case Instruction::LShr:
      case Instruction::AShr:
      case Instruction::ICmp:
      case Instruction::FCmp:
        if (I->getOperand(0)->getType()->isFPOrFPVector())
          return false;  // FP arithmetic might trap.
        break;   // These are all cheap and non-trapping instructions.
      }

      // Okay, we can only really hoist these out if their operands are not
      // defined in the conditional region.
      for (unsigned i = 0, e = I->getNumOperands(); i != e; ++i)
        if (!DominatesMergePoint(I->getOperand(i), BB, 0))
          return false;
      // Okay, it's safe to do this!  Remember this instruction.
      AggressiveInsts->insert(I);
    }

  return true;
}

// GatherConstantSetEQs - Given a potentially 'or'd together collection of 
// icmp_eq instructions that compare a value against a constant, return the 
// value being compared, and stick the constant into the Values vector.
static Value *GatherConstantSetEQs(Value *V, std::vector<ConstantInt*> &Values){
  if (Instruction *Inst = dyn_cast<Instruction>(V)) {
    if (Inst->getOpcode() == Instruction::ICmp &&
        cast<ICmpInst>(Inst)->getPredicate() == ICmpInst::ICMP_EQ) {
      if (ConstantInt *C = dyn_cast<ConstantInt>(Inst->getOperand(1))) {
        Values.push_back(C);
        return Inst->getOperand(0);
      } else if (ConstantInt *C = dyn_cast<ConstantInt>(Inst->getOperand(0))) {
        Values.push_back(C);
        return Inst->getOperand(1);
      }
    } else if (Inst->getOpcode() == Instruction::Or) {
      if (Value *LHS = GatherConstantSetEQs(Inst->getOperand(0), Values))
        if (Value *RHS = GatherConstantSetEQs(Inst->getOperand(1), Values))
          if (LHS == RHS)
            return LHS;
    }
  }
  return 0;
}

// GatherConstantSetNEs - Given a potentially 'and'd together collection of
// setne instructions that compare a value against a constant, return the value
// being compared, and stick the constant into the Values vector.
static Value *GatherConstantSetNEs(Value *V, std::vector<ConstantInt*> &Values){
  if (Instruction *Inst = dyn_cast<Instruction>(V)) {
    if (Inst->getOpcode() == Instruction::ICmp &&
               cast<ICmpInst>(Inst)->getPredicate() == ICmpInst::ICMP_NE) {
      if (ConstantInt *C = dyn_cast<ConstantInt>(Inst->getOperand(1))) {
        Values.push_back(C);
        return Inst->getOperand(0);
      } else if (ConstantInt *C = dyn_cast<ConstantInt>(Inst->getOperand(0))) {
        Values.push_back(C);
        return Inst->getOperand(1);
      }
    } else if (Inst->getOpcode() == Instruction::And) {
      if (Value *LHS = GatherConstantSetNEs(Inst->getOperand(0), Values))
        if (Value *RHS = GatherConstantSetNEs(Inst->getOperand(1), Values))
          if (LHS == RHS)
            return LHS;
    }
  }
  return 0;
}



/// GatherValueComparisons - If the specified Cond is an 'and' or 'or' of a
/// bunch of comparisons of one value against constants, return the value and
/// the constants being compared.
static bool GatherValueComparisons(Instruction *Cond, Value *&CompVal,
                                   std::vector<ConstantInt*> &Values) {
  if (Cond->getOpcode() == Instruction::Or) {
    CompVal = GatherConstantSetEQs(Cond, Values);

    // Return true to indicate that the condition is true if the CompVal is
    // equal to one of the constants.
    return true;
  } else if (Cond->getOpcode() == Instruction::And) {
    CompVal = GatherConstantSetNEs(Cond, Values);

    // Return false to indicate that the condition is false if the CompVal is
    // equal to one of the constants.
    return false;
  }
  return false;
}

/// ErasePossiblyDeadInstructionTree - If the specified instruction is dead and
/// has no side effects, nuke it.  If it uses any instructions that become dead
/// because the instruction is now gone, nuke them too.
static void ErasePossiblyDeadInstructionTree(Instruction *I) {
  if (!isInstructionTriviallyDead(I)) return;
  
  SmallVector<Instruction*, 16> InstrsToInspect;
  InstrsToInspect.push_back(I);

  while (!InstrsToInspect.empty()) {
    I = InstrsToInspect.back();
    InstrsToInspect.pop_back();

    if (!isInstructionTriviallyDead(I)) continue;

    // If I is in the work list multiple times, remove previous instances.
    for (unsigned i = 0, e = InstrsToInspect.size(); i != e; ++i)
      if (InstrsToInspect[i] == I) {
        InstrsToInspect.erase(InstrsToInspect.begin()+i);
        --i, --e;
      }

    // Add operands of dead instruction to worklist.
    for (unsigned i = 0, e = I->getNumOperands(); i != e; ++i)
      if (Instruction *OpI = dyn_cast<Instruction>(I->getOperand(i)))
        InstrsToInspect.push_back(OpI);

    // Remove dead instruction.
    I->eraseFromParent();
  }
}

// isValueEqualityComparison - Return true if the specified terminator checks to
// see if a value is equal to constant integer value.
static Value *isValueEqualityComparison(TerminatorInst *TI) {
  if (SwitchInst *SI = dyn_cast<SwitchInst>(TI)) {
    // Do not permit merging of large switch instructions into their
    // predecessors unless there is only one predecessor.
    if (SI->getNumSuccessors() * std::distance(pred_begin(SI->getParent()),
                                               pred_end(SI->getParent())) > 128)
      return 0;

    return SI->getCondition();
  }
  if (BranchInst *BI = dyn_cast<BranchInst>(TI))
    if (BI->isConditional() && BI->getCondition()->hasOneUse())
      if (ICmpInst *ICI = dyn_cast<ICmpInst>(BI->getCondition()))
        if ((ICI->getPredicate() == ICmpInst::ICMP_EQ ||
             ICI->getPredicate() == ICmpInst::ICMP_NE) &&
            isa<ConstantInt>(ICI->getOperand(1)))
          return ICI->getOperand(0);
  return 0;
}

// Given a value comparison instruction, decode all of the 'cases' that it
// represents and return the 'default' block.
static BasicBlock *
GetValueEqualityComparisonCases(TerminatorInst *TI,
                                std::vector<std::pair<ConstantInt*,
                                                      BasicBlock*> > &Cases) {
  if (SwitchInst *SI = dyn_cast<SwitchInst>(TI)) {
    Cases.reserve(SI->getNumCases());
    for (unsigned i = 1, e = SI->getNumCases(); i != e; ++i)
      Cases.push_back(std::make_pair(SI->getCaseValue(i), SI->getSuccessor(i)));
    return SI->getDefaultDest();
  }

  BranchInst *BI = cast<BranchInst>(TI);
  ICmpInst *ICI = cast<ICmpInst>(BI->getCondition());
  Cases.push_back(std::make_pair(cast<ConstantInt>(ICI->getOperand(1)),
                                 BI->getSuccessor(ICI->getPredicate() ==
                                                  ICmpInst::ICMP_NE)));
  return BI->getSuccessor(ICI->getPredicate() == ICmpInst::ICMP_EQ);
}


// EliminateBlockCases - Given a vector of bb/value pairs, remove any entries
// in the list that match the specified block.
static void EliminateBlockCases(BasicBlock *BB,
               std::vector<std::pair<ConstantInt*, BasicBlock*> > &Cases) {
  for (unsigned i = 0, e = Cases.size(); i != e; ++i)
    if (Cases[i].second == BB) {
      Cases.erase(Cases.begin()+i);
      --i; --e;
    }
}

// ValuesOverlap - Return true if there are any keys in C1 that exist in C2 as
// well.
static bool
ValuesOverlap(std::vector<std::pair<ConstantInt*, BasicBlock*> > &C1,
              std::vector<std::pair<ConstantInt*, BasicBlock*> > &C2) {
  std::vector<std::pair<ConstantInt*, BasicBlock*> > *V1 = &C1, *V2 = &C2;

  // Make V1 be smaller than V2.
  if (V1->size() > V2->size())
    std::swap(V1, V2);

  if (V1->size() == 0) return false;
  if (V1->size() == 1) {
    // Just scan V2.
    ConstantInt *TheVal = (*V1)[0].first;
    for (unsigned i = 0, e = V2->size(); i != e; ++i)
      if (TheVal == (*V2)[i].first)
        return true;
  }

  // Otherwise, just sort both lists and compare element by element.
  std::sort(V1->begin(), V1->end());
  std::sort(V2->begin(), V2->end());
  unsigned i1 = 0, i2 = 0, e1 = V1->size(), e2 = V2->size();
  while (i1 != e1 && i2 != e2) {
    if ((*V1)[i1].first == (*V2)[i2].first)
      return true;
    if ((*V1)[i1].first < (*V2)[i2].first)
      ++i1;
    else
      ++i2;
  }
  return false;
}

// SimplifyEqualityComparisonWithOnlyPredecessor - If TI is known to be a
// terminator instruction and its block is known to only have a single
// predecessor block, check to see if that predecessor is also a value
// comparison with the same value, and if that comparison determines the outcome
// of this comparison.  If so, simplify TI.  This does a very limited form of
// jump threading.
static bool SimplifyEqualityComparisonWithOnlyPredecessor(TerminatorInst *TI,
                                                          BasicBlock *Pred) {
  Value *PredVal = isValueEqualityComparison(Pred->getTerminator());
  if (!PredVal) return false;  // Not a value comparison in predecessor.

  Value *ThisVal = isValueEqualityComparison(TI);
  assert(ThisVal && "This isn't a value comparison!!");
  if (ThisVal != PredVal) return false;  // Different predicates.

  // Find out information about when control will move from Pred to TI's block.
  std::vector<std::pair<ConstantInt*, BasicBlock*> > PredCases;
  BasicBlock *PredDef = GetValueEqualityComparisonCases(Pred->getTerminator(),
                                                        PredCases);
  EliminateBlockCases(PredDef, PredCases);  // Remove default from cases.

  // Find information about how control leaves this block.
  std::vector<std::pair<ConstantInt*, BasicBlock*> > ThisCases;
  BasicBlock *ThisDef = GetValueEqualityComparisonCases(TI, ThisCases);
  EliminateBlockCases(ThisDef, ThisCases);  // Remove default from cases.

  // If TI's block is the default block from Pred's comparison, potentially
  // simplify TI based on this knowledge.
  if (PredDef == TI->getParent()) {
    // If we are here, we know that the value is none of those cases listed in
    // PredCases.  If there are any cases in ThisCases that are in PredCases, we
    // can simplify TI.
    if (ValuesOverlap(PredCases, ThisCases)) {
      if (BranchInst *BTI = dyn_cast<BranchInst>(TI)) {
        // Okay, one of the successors of this condbr is dead.  Convert it to a
        // uncond br.
        assert(ThisCases.size() == 1 && "Branch can only have one case!");
        Value *Cond = BTI->getCondition();
        // Insert the new branch.
        Instruction *NI = new BranchInst(ThisDef, TI);

        // Remove PHI node entries for the dead edge.
        ThisCases[0].second->removePredecessor(TI->getParent());

        DOUT << "Threading pred instr: " << *Pred->getTerminator()
             << "Through successor TI: " << *TI << "Leaving: " << *NI << "\n";

        TI->eraseFromParent();   // Nuke the old one.
        // If condition is now dead, nuke it.
        if (Instruction *CondI = dyn_cast<Instruction>(Cond))
          ErasePossiblyDeadInstructionTree(CondI);
        return true;

      } else {
        SwitchInst *SI = cast<SwitchInst>(TI);
        // Okay, TI has cases that are statically dead, prune them away.
        SmallPtrSet<Constant*, 16> DeadCases;
        for (unsigned i = 0, e = PredCases.size(); i != e; ++i)
          DeadCases.insert(PredCases[i].first);

        DOUT << "Threading pred instr: " << *Pred->getTerminator()
             << "Through successor TI: " << *TI;

        for (unsigned i = SI->getNumCases()-1; i != 0; --i)
          if (DeadCases.count(SI->getCaseValue(i))) {
            SI->getSuccessor(i)->removePredecessor(TI->getParent());
            SI->removeCase(i);
          }

        DOUT << "Leaving: " << *TI << "\n";
        return true;
      }
    }

  } else {
    // Otherwise, TI's block must correspond to some matched value.  Find out
    // which value (or set of values) this is.
    ConstantInt *TIV = 0;
    BasicBlock *TIBB = TI->getParent();
    for (unsigned i = 0, e = PredCases.size(); i != e; ++i)
      if (PredCases[i].second == TIBB) {
        if (TIV == 0)
          TIV = PredCases[i].first;
        else
          return false;  // Cannot handle multiple values coming to this block.
      }
    assert(TIV && "No edge from pred to succ?");

    // Okay, we found the one constant that our value can be if we get into TI's
    // BB.  Find out which successor will unconditionally be branched to.
    BasicBlock *TheRealDest = 0;
    for (unsigned i = 0, e = ThisCases.size(); i != e; ++i)
      if (ThisCases[i].first == TIV) {
        TheRealDest = ThisCases[i].second;
        break;
      }

    // If not handled by any explicit cases, it is handled by the default case.
    if (TheRealDest == 0) TheRealDest = ThisDef;

    // Remove PHI node entries for dead edges.
    BasicBlock *CheckEdge = TheRealDest;
    for (succ_iterator SI = succ_begin(TIBB), e = succ_end(TIBB); SI != e; ++SI)
      if (*SI != CheckEdge)
        (*SI)->removePredecessor(TIBB);
      else
        CheckEdge = 0;

    // Insert the new branch.
    Instruction *NI = new BranchInst(TheRealDest, TI);

    DOUT << "Threading pred instr: " << *Pred->getTerminator()
         << "Through successor TI: " << *TI << "Leaving: " << *NI << "\n";
    Instruction *Cond = 0;
    if (BranchInst *BI = dyn_cast<BranchInst>(TI))
      Cond = dyn_cast<Instruction>(BI->getCondition());
    TI->eraseFromParent();   // Nuke the old one.

    if (Cond) ErasePossiblyDeadInstructionTree(Cond);
    return true;
  }
  return false;
}

// FoldValueComparisonIntoPredecessors - The specified terminator is a value
// equality comparison instruction (either a switch or a branch on "X == c").
// See if any of the predecessors of the terminator block are value comparisons
// on the same value.  If so, and if safe to do so, fold them together.
static bool FoldValueComparisonIntoPredecessors(TerminatorInst *TI) {
  BasicBlock *BB = TI->getParent();
  Value *CV = isValueEqualityComparison(TI);  // CondVal
  assert(CV && "Not a comparison?");
  bool Changed = false;

  SmallVector<BasicBlock*, 16> Preds(pred_begin(BB), pred_end(BB));
  while (!Preds.empty()) {
    BasicBlock *Pred = Preds.back();
    Preds.pop_back();

    // See if the predecessor is a comparison with the same value.
    TerminatorInst *PTI = Pred->getTerminator();
    Value *PCV = isValueEqualityComparison(PTI);  // PredCondVal

    if (PCV == CV && SafeToMergeTerminators(TI, PTI)) {
      // Figure out which 'cases' to copy from SI to PSI.
      std::vector<std::pair<ConstantInt*, BasicBlock*> > BBCases;
      BasicBlock *BBDefault = GetValueEqualityComparisonCases(TI, BBCases);

      std::vector<std::pair<ConstantInt*, BasicBlock*> > PredCases;
      BasicBlock *PredDefault = GetValueEqualityComparisonCases(PTI, PredCases);

      // Based on whether the default edge from PTI goes to BB or not, fill in
      // PredCases and PredDefault with the new switch cases we would like to
      // build.
      SmallVector<BasicBlock*, 8> NewSuccessors;

      if (PredDefault == BB) {
        // If this is the default destination from PTI, only the edges in TI
        // that don't occur in PTI, or that branch to BB will be activated.
        std::set<ConstantInt*> PTIHandled;
        for (unsigned i = 0, e = PredCases.size(); i != e; ++i)
          if (PredCases[i].second != BB)
            PTIHandled.insert(PredCases[i].first);
          else {
            // The default destination is BB, we don't need explicit targets.
            std::swap(PredCases[i], PredCases.back());
            PredCases.pop_back();
            --i; --e;
          }

        // Reconstruct the new switch statement we will be building.
        if (PredDefault != BBDefault) {
          PredDefault->removePredecessor(Pred);
          PredDefault = BBDefault;
          NewSuccessors.push_back(BBDefault);
        }
        for (unsigned i = 0, e = BBCases.size(); i != e; ++i)
          if (!PTIHandled.count(BBCases[i].first) &&
              BBCases[i].second != BBDefault) {
            PredCases.push_back(BBCases[i]);
            NewSuccessors.push_back(BBCases[i].second);
          }

      } else {
        // If this is not the default destination from PSI, only the edges
        // in SI that occur in PSI with a destination of BB will be
        // activated.
        std::set<ConstantInt*> PTIHandled;
        for (unsigned i = 0, e = PredCases.size(); i != e; ++i)
          if (PredCases[i].second == BB) {
            PTIHandled.insert(PredCases[i].first);
            std::swap(PredCases[i], PredCases.back());
            PredCases.pop_back();
            --i; --e;
          }

        // Okay, now we know which constants were sent to BB from the
        // predecessor.  Figure out where they will all go now.
        for (unsigned i = 0, e = BBCases.size(); i != e; ++i)
          if (PTIHandled.count(BBCases[i].first)) {
            // If this is one we are capable of getting...
            PredCases.push_back(BBCases[i]);
            NewSuccessors.push_back(BBCases[i].second);
            PTIHandled.erase(BBCases[i].first);// This constant is taken care of
          }

        // If there are any constants vectored to BB that TI doesn't handle,
        // they must go to the default destination of TI.
        for (std::set<ConstantInt*>::iterator I = PTIHandled.begin(),
               E = PTIHandled.end(); I != E; ++I) {
          PredCases.push_back(std::make_pair(*I, BBDefault));
          NewSuccessors.push_back(BBDefault);
        }
      }

      // Okay, at this point, we know which new successor Pred will get.  Make
      // sure we update the number of entries in the PHI nodes for these
      // successors.
      for (unsigned i = 0, e = NewSuccessors.size(); i != e; ++i)
        AddPredecessorToBlock(NewSuccessors[i], Pred, BB);

      // Now that the successors are updated, create the new Switch instruction.
      SwitchInst *NewSI = new SwitchInst(CV, PredDefault, PredCases.size(),PTI);
      for (unsigned i = 0, e = PredCases.size(); i != e; ++i)
        NewSI->addCase(PredCases[i].first, PredCases[i].second);

      Instruction *DeadCond = 0;
      if (BranchInst *BI = dyn_cast<BranchInst>(PTI))
        // If PTI is a branch, remember the condition.
        DeadCond = dyn_cast<Instruction>(BI->getCondition());
      Pred->getInstList().erase(PTI);

      // If the condition is dead now, remove the instruction tree.
      if (DeadCond) ErasePossiblyDeadInstructionTree(DeadCond);

      // Okay, last check.  If BB is still a successor of PSI, then we must
      // have an infinite loop case.  If so, add an infinitely looping block
      // to handle the case to preserve the behavior of the code.
      BasicBlock *InfLoopBlock = 0;
      for (unsigned i = 0, e = NewSI->getNumSuccessors(); i != e; ++i)
        if (NewSI->getSuccessor(i) == BB) {
          if (InfLoopBlock == 0) {
            // Insert it at the end of the loop, because it's either code,
            // or it won't matter if it's hot. :)
            InfLoopBlock = new BasicBlock("infloop", BB->getParent());
            new BranchInst(InfLoopBlock, InfLoopBlock);
          }
          NewSI->setSuccessor(i, InfLoopBlock);
        }

      Changed = true;
    }
  }
  return Changed;
}

/// HoistThenElseCodeToIf - Given a conditional branch that goes to BB1 and
/// BB2, hoist any common code in the two blocks up into the branch block.  The
/// caller of this function guarantees that BI's block dominates BB1 and BB2.
static bool HoistThenElseCodeToIf(BranchInst *BI) {
  // This does very trivial matching, with limited scanning, to find identical
  // instructions in the two blocks.  In particular, we don't want to get into
  // O(M*N) situations here where M and N are the sizes of BB1 and BB2.  As
  // such, we currently just scan for obviously identical instructions in an
  // identical order.
  BasicBlock *BB1 = BI->getSuccessor(0);  // The true destination.
  BasicBlock *BB2 = BI->getSuccessor(1);  // The false destination

  Instruction *I1 = BB1->begin(), *I2 = BB2->begin();
  if (I1->getOpcode() != I2->getOpcode() || isa<PHINode>(I1) || 
      isa<InvokeInst>(I1) || !I1->isIdenticalTo(I2))
    return false;

  // If we get here, we can hoist at least one instruction.
  BasicBlock *BIParent = BI->getParent();

  do {
    // If we are hoisting the terminator instruction, don't move one (making a
    // broken BB), instead clone it, and remove BI.
    if (isa<TerminatorInst>(I1))
      goto HoistTerminator;

    // For a normal instruction, we just move one to right before the branch,
    // then replace all uses of the other with the first.  Finally, we remove
    // the now redundant second instruction.
    BIParent->getInstList().splice(BI, BB1->getInstList(), I1);
    if (!I2->use_empty())
      I2->replaceAllUsesWith(I1);
    BB2->getInstList().erase(I2);

    I1 = BB1->begin();
    I2 = BB2->begin();
  } while (I1->getOpcode() == I2->getOpcode() && I1->isIdenticalTo(I2));

  return true;

HoistTerminator:
  // Okay, it is safe to hoist the terminator.
  Instruction *NT = I1->clone();
  BIParent->getInstList().insert(BI, NT);
  if (NT->getType() != Type::VoidTy) {
    I1->replaceAllUsesWith(NT);
    I2->replaceAllUsesWith(NT);
    NT->takeName(I1);
  }

  // Hoisting one of the terminators from our successor is a great thing.
  // Unfortunately, the successors of the if/else blocks may have PHI nodes in
  // them.  If they do, all PHI entries for BB1/BB2 must agree for all PHI
  // nodes, so we insert select instruction to compute the final result.
  std::map<std::pair<Value*,Value*>, SelectInst*> InsertedSelects;
  for (succ_iterator SI = succ_begin(BB1), E = succ_end(BB1); SI != E; ++SI) {
    PHINode *PN;
    for (BasicBlock::iterator BBI = SI->begin();
         (PN = dyn_cast<PHINode>(BBI)); ++BBI) {
      Value *BB1V = PN->getIncomingValueForBlock(BB1);
      Value *BB2V = PN->getIncomingValueForBlock(BB2);
      if (BB1V != BB2V) {
        // These values do not agree.  Insert a select instruction before NT
        // that determines the right value.
        SelectInst *&SI = InsertedSelects[std::make_pair(BB1V, BB2V)];
        if (SI == 0)
          SI = new SelectInst(BI->getCondition(), BB1V, BB2V,
                              BB1V->getName()+"."+BB2V->getName(), NT);
        // Make the PHI node use the select for all incoming values for BB1/BB2
        for (unsigned i = 0, e = PN->getNumIncomingValues(); i != e; ++i)
          if (PN->getIncomingBlock(i) == BB1 || PN->getIncomingBlock(i) == BB2)
            PN->setIncomingValue(i, SI);
      }
    }
  }

  // Update any PHI nodes in our new successors.
  for (succ_iterator SI = succ_begin(BB1), E = succ_end(BB1); SI != E; ++SI)
    AddPredecessorToBlock(*SI, BIParent, BB1);

  BI->eraseFromParent();
  return true;
}

/// BlockIsSimpleEnoughToThreadThrough - Return true if we can thread a branch
/// across this block.
static bool BlockIsSimpleEnoughToThreadThrough(BasicBlock *BB) {
  BranchInst *BI = cast<BranchInst>(BB->getTerminator());
  unsigned Size = 0;
  
  // If this basic block contains anything other than a PHI (which controls the
  // branch) and branch itself, bail out.  FIXME: improve this in the future.
  for (BasicBlock::iterator BBI = BB->begin(); &*BBI != BI; ++BBI, ++Size) {
    if (Size > 10) return false;  // Don't clone large BB's.
    
    // We can only support instructions that are do not define values that are
    // live outside of the current basic block.
    for (Value::use_iterator UI = BBI->use_begin(), E = BBI->use_end();
         UI != E; ++UI) {
      Instruction *U = cast<Instruction>(*UI);
      if (U->getParent() != BB || isa<PHINode>(U)) return false;
    }
    
    // Looks ok, continue checking.
  }

  return true;
}

/// FoldCondBranchOnPHI - If we have a conditional branch on a PHI node value
/// that is defined in the same block as the branch and if any PHI entries are
/// constants, thread edges corresponding to that entry to be branches to their
/// ultimate destination.
static bool FoldCondBranchOnPHI(BranchInst *BI) {
  BasicBlock *BB = BI->getParent();
  PHINode *PN = dyn_cast<PHINode>(BI->getCondition());
  // NOTE: we currently cannot transform this case if the PHI node is used
  // outside of the block.
  if (!PN || PN->getParent() != BB || !PN->hasOneUse())
    return false;
  
  // Degenerate case of a single entry PHI.
  if (PN->getNumIncomingValues() == 1) {
    if (PN->getIncomingValue(0) != PN)
      PN->replaceAllUsesWith(PN->getIncomingValue(0));
    else
      PN->replaceAllUsesWith(UndefValue::get(PN->getType()));
    PN->eraseFromParent();
    return true;    
  }

  // Now we know that this block has multiple preds and two succs.
  if (!BlockIsSimpleEnoughToThreadThrough(BB)) return false;
  
  // Okay, this is a simple enough basic block.  See if any phi values are
  // constants.
  for (unsigned i = 0, e = PN->getNumIncomingValues(); i != e; ++i) {
    ConstantInt *CB;
    if ((CB = dyn_cast<ConstantInt>(PN->getIncomingValue(i))) &&
        CB->getType() == Type::Int1Ty) {
      // Okay, we now know that all edges from PredBB should be revectored to
      // branch to RealDest.
      BasicBlock *PredBB = PN->getIncomingBlock(i);
      BasicBlock *RealDest = BI->getSuccessor(!CB->getZExtValue());
      
      if (RealDest == BB) continue;  // Skip self loops.
      
      // The dest block might have PHI nodes, other predecessors and other
      // difficult cases.  Instead of being smart about this, just insert a new
      // block that jumps to the destination block, effectively splitting
      // the edge we are about to create.
      BasicBlock *EdgeBB = new BasicBlock(RealDest->getName()+".critedge",
                                          RealDest->getParent(), RealDest);
      new BranchInst(RealDest, EdgeBB);
      PHINode *PN;
      for (BasicBlock::iterator BBI = RealDest->begin();
           (PN = dyn_cast<PHINode>(BBI)); ++BBI) {
        Value *V = PN->getIncomingValueForBlock(BB);
        PN->addIncoming(V, EdgeBB);
      }

      // BB may have instructions that are being threaded over.  Clone these
      // instructions into EdgeBB.  We know that there will be no uses of the
      // cloned instructions outside of EdgeBB.
      BasicBlock::iterator InsertPt = EdgeBB->begin();
      std::map<Value*, Value*> TranslateMap;  // Track translated values.
      for (BasicBlock::iterator BBI = BB->begin(); &*BBI != BI; ++BBI) {
        if (PHINode *PN = dyn_cast<PHINode>(BBI)) {
          TranslateMap[PN] = PN->getIncomingValueForBlock(PredBB);
        } else {
          // Clone the instruction.
          Instruction *N = BBI->clone();
          if (BBI->hasName()) N->setName(BBI->getName()+".c");
          
          // Update operands due to translation.
          for (unsigned i = 0, e = N->getNumOperands(); i != e; ++i) {
            std::map<Value*, Value*>::iterator PI =
              TranslateMap.find(N->getOperand(i));
            if (PI != TranslateMap.end())
              N->setOperand(i, PI->second);
          }
          
          // Check for trivial simplification.
          if (Constant *C = ConstantFoldInstruction(N)) {
            TranslateMap[BBI] = C;
            delete N;   // Constant folded away, don't need actual inst
          } else {
            // Insert the new instruction into its new home.
            EdgeBB->getInstList().insert(InsertPt, N);
            if (!BBI->use_empty())
              TranslateMap[BBI] = N;
          }
        }
      }

      // Loop over all of the edges from PredBB to BB, changing them to branch
      // to EdgeBB instead.
      TerminatorInst *PredBBTI = PredBB->getTerminator();
      for (unsigned i = 0, e = PredBBTI->getNumSuccessors(); i != e; ++i)
        if (PredBBTI->getSuccessor(i) == BB) {
          BB->removePredecessor(PredBB);
          PredBBTI->setSuccessor(i, EdgeBB);
        }
      
      // Recurse, simplifying any other constants.
      return FoldCondBranchOnPHI(BI) | true;
    }
  }

  return false;
}

/// FoldTwoEntryPHINode - Given a BB that starts with the specified two-entry
/// PHI node, see if we can eliminate it.
static bool FoldTwoEntryPHINode(PHINode *PN) {
  // Ok, this is a two entry PHI node.  Check to see if this is a simple "if
  // statement", which has a very simple dominance structure.  Basically, we
  // are trying to find the condition that is being branched on, which
  // subsequently causes this merge to happen.  We really want control
  // dependence information for this check, but simplifycfg can't keep it up
  // to date, and this catches most of the cases we care about anyway.
  //
  BasicBlock *BB = PN->getParent();
  BasicBlock *IfTrue, *IfFalse;
  Value *IfCond = GetIfCondition(BB, IfTrue, IfFalse);
  if (!IfCond) return false;
  
  // Okay, we found that we can merge this two-entry phi node into a select.
  // Doing so would require us to fold *all* two entry phi nodes in this block.
  // At some point this becomes non-profitable (particularly if the target
  // doesn't support cmov's).  Only do this transformation if there are two or
  // fewer PHI nodes in this block.
  unsigned NumPhis = 0;
  for (BasicBlock::iterator I = BB->begin(); isa<PHINode>(I); ++NumPhis, ++I)
    if (NumPhis > 2)
      return false;
  
  DOUT << "FOUND IF CONDITION!  " << *IfCond << "  T: "
       << IfTrue->getName() << "  F: " << IfFalse->getName() << "\n";
  
  // Loop over the PHI's seeing if we can promote them all to select
  // instructions.  While we are at it, keep track of the instructions
  // that need to be moved to the dominating block.
  std::set<Instruction*> AggressiveInsts;
  
  BasicBlock::iterator AfterPHIIt = BB->begin();
  while (isa<PHINode>(AfterPHIIt)) {
    PHINode *PN = cast<PHINode>(AfterPHIIt++);
    if (PN->getIncomingValue(0) == PN->getIncomingValue(1)) {
      if (PN->getIncomingValue(0) != PN)
        PN->replaceAllUsesWith(PN->getIncomingValue(0));
      else
        PN->replaceAllUsesWith(UndefValue::get(PN->getType()));
    } else if (!DominatesMergePoint(PN->getIncomingValue(0), BB,
                                    &AggressiveInsts) ||
               !DominatesMergePoint(PN->getIncomingValue(1), BB,
                                    &AggressiveInsts)) {
      return false;
    }
  }
  
  // If we all PHI nodes are promotable, check to make sure that all
  // instructions in the predecessor blocks can be promoted as well.  If
  // not, we won't be able to get rid of the control flow, so it's not
  // worth promoting to select instructions.
  BasicBlock *DomBlock = 0, *IfBlock1 = 0, *IfBlock2 = 0;
  PN = cast<PHINode>(BB->begin());
  BasicBlock *Pred = PN->getIncomingBlock(0);
  if (cast<BranchInst>(Pred->getTerminator())->isUnconditional()) {
    IfBlock1 = Pred;
    DomBlock = *pred_begin(Pred);
    for (BasicBlock::iterator I = Pred->begin();
         !isa<TerminatorInst>(I); ++I)
      if (!AggressiveInsts.count(I)) {
        // This is not an aggressive instruction that we can promote.
        // Because of this, we won't be able to get rid of the control
        // flow, so the xform is not worth it.
        return false;
      }
  }
    
  Pred = PN->getIncomingBlock(1);
  if (cast<BranchInst>(Pred->getTerminator())->isUnconditional()) {
    IfBlock2 = Pred;
    DomBlock = *pred_begin(Pred);
    for (BasicBlock::iterator I = Pred->begin();
         !isa<TerminatorInst>(I); ++I)
      if (!AggressiveInsts.count(I)) {
        // This is not an aggressive instruction that we can promote.
        // Because of this, we won't be able to get rid of the control
        // flow, so the xform is not worth it.
        return false;
      }
  }
      
  // If we can still promote the PHI nodes after this gauntlet of tests,
  // do all of the PHI's now.

  // Move all 'aggressive' instructions, which are defined in the
  // conditional parts of the if's up to the dominating block.
  if (IfBlock1) {
    DomBlock->getInstList().splice(DomBlock->getTerminator(),
                                   IfBlock1->getInstList(),
                                   IfBlock1->begin(),
                                   IfBlock1->getTerminator());
  }
  if (IfBlock2) {
    DomBlock->getInstList().splice(DomBlock->getTerminator(),
                                   IfBlock2->getInstList(),
                                   IfBlock2->begin(),
                                   IfBlock2->getTerminator());
  }
  
  while (PHINode *PN = dyn_cast<PHINode>(BB->begin())) {
    // Change the PHI node into a select instruction.
    Value *TrueVal =
      PN->getIncomingValue(PN->getIncomingBlock(0) == IfFalse);
    Value *FalseVal =
      PN->getIncomingValue(PN->getIncomingBlock(0) == IfTrue);
    
    Value *NV = new SelectInst(IfCond, TrueVal, FalseVal, "", AfterPHIIt);
    PN->replaceAllUsesWith(NV);
    NV->takeName(PN);
    
    BB->getInstList().erase(PN);
  }
  return true;
}

namespace {
  /// ConstantIntOrdering - This class implements a stable ordering of constant
  /// integers that does not depend on their address.  This is important for
  /// applications that sort ConstantInt's to ensure uniqueness.
  struct ConstantIntOrdering {
    bool operator()(const ConstantInt *LHS, const ConstantInt *RHS) const {
      return LHS->getValue().ult(RHS->getValue());
    }
  };
}

// SimplifyCFG - This function is used to do simplification of a CFG.  For
// example, it adjusts branches to branches to eliminate the extra hop, it
// eliminates unreachable basic blocks, and does other "peephole" optimization
// of the CFG.  It returns true if a modification was made.
//
// WARNING:  The entry node of a function may not be simplified.
//
bool llvm::SimplifyCFG(BasicBlock *BB) {
  bool Changed = false;
  Function *M = BB->getParent();

  assert(BB && BB->getParent() && "Block not embedded in function!");
  assert(BB->getTerminator() && "Degenerate basic block encountered!");
  assert(&BB->getParent()->getEntryBlock() != BB &&
         "Can't Simplify entry block!");

  // Remove basic blocks that have no predecessors... which are unreachable.
  if ((pred_begin(BB) == pred_end(BB)) ||
      (*pred_begin(BB) == BB && ++pred_begin(BB) == pred_end(BB))) {
    DOUT << "Removing BB: \n" << *BB;

    // Loop through all of our successors and make sure they know that one
    // of their predecessors is going away.
    for (succ_iterator SI = succ_begin(BB), E = succ_end(BB); SI != E; ++SI)
      SI->removePredecessor(BB);

    while (!BB->empty()) {
      Instruction &I = BB->back();
      // If this instruction is used, replace uses with an arbitrary
      // value.  Because control flow can't get here, we don't care
      // what we replace the value with.  Note that since this block is
      // unreachable, and all values contained within it must dominate their
      // uses, that all uses will eventually be removed.
      if (!I.use_empty())
        // Make all users of this instruction use undef instead
        I.replaceAllUsesWith(UndefValue::get(I.getType()));

      // Remove the instruction from the basic block
      BB->getInstList().pop_back();
    }
    M->getBasicBlockList().erase(BB);
    return true;
  }

  // Check to see if we can constant propagate this terminator instruction
  // away...
  Changed |= ConstantFoldTerminator(BB);

  // If this is a returning block with only PHI nodes in it, fold the return
  // instruction into any unconditional branch predecessors.
  //
  // If any predecessor is a conditional branch that just selects among
  // different return values, fold the replace the branch/return with a select
  // and return.
  if (ReturnInst *RI = dyn_cast<ReturnInst>(BB->getTerminator())) {
    BasicBlock::iterator BBI = BB->getTerminator();
    if (BBI == BB->begin() || isa<PHINode>(--BBI)) {
      // Find predecessors that end with branches.
      SmallVector<BasicBlock*, 8> UncondBranchPreds;
      SmallVector<BranchInst*, 8> CondBranchPreds;
      for (pred_iterator PI = pred_begin(BB), E = pred_end(BB); PI != E; ++PI) {
        if ((*PI)->getUnwindDest() == BB) continue;

        TerminatorInst *PTI = (*PI)->getTerminator();
        if (BranchInst *BI = dyn_cast<BranchInst>(PTI)) {
          if (BI->isUnconditional())
            UncondBranchPreds.push_back(*PI);
          else
            CondBranchPreds.push_back(BI);
        }
      }

      // If we found some, do the transformation!
      if (!UncondBranchPreds.empty()) {
        while (!UncondBranchPreds.empty()) {
          BasicBlock *Pred = UncondBranchPreds.back();
          DOUT << "FOLDING: " << *BB
               << "INTO UNCOND BRANCH PRED: " << *Pred;
          UncondBranchPreds.pop_back();
          Instruction *UncondBranch = Pred->getTerminator();
          // Clone the return and add it to the end of the predecessor.
          Instruction *NewRet = RI->clone();
          Pred->getInstList().push_back(NewRet);

          // If the return instruction returns a value, and if the value was a
          // PHI node in "BB", propagate the right value into the return.
          if (NewRet->getNumOperands() == 1)
            if (PHINode *PN = dyn_cast<PHINode>(NewRet->getOperand(0)))
              if (PN->getParent() == BB)
                NewRet->setOperand(0, PN->getIncomingValueForBlock(Pred));
          // Update any PHI nodes in the returning block to realize that we no
          // longer branch to them.
          BB->removePredecessor(Pred);
          Pred->getInstList().erase(UncondBranch);
        }

        // If we eliminated all predecessors of the block, delete the block now.
        if (pred_begin(BB) == pred_end(BB))
          // We know there are no successors, so just nuke the block.
          M->getBasicBlockList().erase(BB);

        return true;
      }

      // Check out all of the conditional branches going to this return
      // instruction.  If any of them just select between returns, change the
      // branch itself into a select/return pair.
      while (!CondBranchPreds.empty()) {
        BranchInst *BI = CondBranchPreds.back();
        CondBranchPreds.pop_back();
        BasicBlock *TrueSucc = BI->getSuccessor(0);
        BasicBlock *FalseSucc = BI->getSuccessor(1);
        BasicBlock *OtherSucc = TrueSucc == BB ? FalseSucc : TrueSucc;

        // Check to see if the non-BB successor is also a return block.
        if (isa<ReturnInst>(OtherSucc->getTerminator())) {
          // Check to see if there are only PHI instructions in this block.
          BasicBlock::iterator OSI = OtherSucc->getTerminator();
          if (OSI == OtherSucc->begin() || isa<PHINode>(--OSI)) {
            // Okay, we found a branch that is going to two return nodes.  If
            // there is no return value for this function, just change the
            // branch into a return.
            if (RI->getNumOperands() == 0) {
              TrueSucc->removePredecessor(BI->getParent());
              FalseSucc->removePredecessor(BI->getParent());
              new ReturnInst(0, BI);
              BI->getParent()->getInstList().erase(BI);
              return true;
            }

            // Otherwise, figure out what the true and false return values are
            // so we can insert a new select instruction.
            Value *TrueValue = TrueSucc->getTerminator()->getOperand(0);
            Value *FalseValue = FalseSucc->getTerminator()->getOperand(0);

            // Unwrap any PHI nodes in the return blocks.
            if (PHINode *TVPN = dyn_cast<PHINode>(TrueValue))
              if (TVPN->getParent() == TrueSucc)
                TrueValue = TVPN->getIncomingValueForBlock(BI->getParent());
            if (PHINode *FVPN = dyn_cast<PHINode>(FalseValue))
              if (FVPN->getParent() == FalseSucc)
                FalseValue = FVPN->getIncomingValueForBlock(BI->getParent());

            // In order for this transformation to be safe, we must be able to
            // unconditionally execute both operands to the return.  This is
            // normally the case, but we could have a potentially-trapping
            // constant expression that prevents this transformation from being
            // safe.
            if ((!isa<ConstantExpr>(TrueValue) ||
                 !cast<ConstantExpr>(TrueValue)->canTrap()) &&
                (!isa<ConstantExpr>(TrueValue) ||
                 !cast<ConstantExpr>(TrueValue)->canTrap())) {
              TrueSucc->removePredecessor(BI->getParent());
              FalseSucc->removePredecessor(BI->getParent());

              // Insert a new select instruction.
              Value *NewRetVal;
              Value *BrCond = BI->getCondition();
              if (TrueValue != FalseValue)
                NewRetVal = new SelectInst(BrCond, TrueValue,
                                           FalseValue, "retval", BI);
              else
                NewRetVal = TrueValue;
              
              DOUT << "\nCHANGING BRANCH TO TWO RETURNS INTO SELECT:"
                   << "\n  " << *BI << "Select = " << *NewRetVal
                   << "TRUEBLOCK: " << *TrueSucc << "FALSEBLOCK: "<< *FalseSucc;

              new ReturnInst(NewRetVal, BI);
              BI->eraseFromParent();
              if (Instruction *BrCondI = dyn_cast<Instruction>(BrCond))
                if (isInstructionTriviallyDead(BrCondI))
                  BrCondI->eraseFromParent();
              return true;
            }
          }
        }
      }
    }
  } else if (isa<UnwindInst>(BB->begin())) {
    // Check to see if the first instruction in this block is just an unwind.
    // If so, replace any invoke instructions which use this as an exception
    // destination with call instructions, and any unconditional branch
    // predecessor with an unwind.
    //
    SmallVector<BasicBlock*, 8> Preds(pred_begin(BB), pred_end(BB));
    while (!Preds.empty()) {
      BasicBlock *Pred = Preds.back();

      if (Pred->getUnwindDest() == BB) {
        Pred->setUnwindDest(NULL);
        Changed = true;
      }

      if (BranchInst *BI = dyn_cast<BranchInst>(Pred->getTerminator())) {
        if (BI->isUnconditional() && BI->getSuccessor(0) == BB) {
          Pred->getInstList().pop_back();  // nuke uncond branch
          new UnwindInst(Pred);            // Use unwind.
          Changed = true;
        }
      } else if (InvokeInst *II = dyn_cast<InvokeInst>(Pred->getTerminator()))
        if (II->getUnwindDest() == BB) {
          // Insert a new branch instruction before the invoke, because this
          // is now a fall through...
          BranchInst *BI = new BranchInst(II->getNormalDest(), II);
          Pred->getInstList().remove(II);   // Take out of symbol table

          // Insert the call now...
          SmallVector<Value*,8> Args(II->op_begin()+3, II->op_end());
          CallInst *CI = new CallInst(II->getCalledValue(),
                                      Args.begin(), Args.end(), II->getName(), BI);
          CI->setCallingConv(II->getCallingConv());
          CI->setParamAttrs(II->getParamAttrs());
          // If the invoke produced a value, the Call now does instead
          II->replaceAllUsesWith(CI);
          delete II;
          Changed = true;
        }

      Preds.pop_back();
    }

    // If this block is now dead, remove it.
    if (pred_begin(BB) == pred_end(BB)) {
      // We know there are no successors, so just nuke the block.
      M->getBasicBlockList().erase(BB);
      return true;
    }

  } else if (SwitchInst *SI = dyn_cast<SwitchInst>(BB->getTerminator())) {
    if (isValueEqualityComparison(SI)) {
      // If we only have one predecessor, and if it is a branch on this value,
      // see if that predecessor totally determines the outcome of this switch.
      if (BasicBlock *OnlyPred = BB->getSinglePredecessor())
        if (SimplifyEqualityComparisonWithOnlyPredecessor(SI, OnlyPred))
          return SimplifyCFG(BB) || 1;

      // If the block only contains the switch, see if we can fold the block
      // away into any preds.
      if (SI == &BB->front())
        if (FoldValueComparisonIntoPredecessors(SI))
          return SimplifyCFG(BB) || 1;
    }
  } else if (BranchInst *BI = dyn_cast<BranchInst>(BB->getTerminator())) {
    if (BI->isUnconditional()) {
      BasicBlock::iterator BBI = BB->begin();  // Skip over phi nodes...
      while (isa<PHINode>(*BBI)) ++BBI;

      BasicBlock *Succ = BI->getSuccessor(0);
      if (BBI->isTerminator() &&  // Terminator is the only non-phi instruction!
          Succ != BB)             // Don't hurt infinite loops!
        if (TryToSimplifyUncondBranchFromEmptyBlock(BB, Succ))
          return 1;
      
    } else {  // Conditional branch
      if (isValueEqualityComparison(BI)) {
        // If we only have one predecessor, and if it is a branch on this value,
        // see if that predecessor totally determines the outcome of this
        // switch.
        if (BasicBlock *OnlyPred = BB->getSinglePredecessor())
          if (SimplifyEqualityComparisonWithOnlyPredecessor(BI, OnlyPred))
            return SimplifyCFG(BB) || 1;

        // This block must be empty, except for the setcond inst, if it exists.
        BasicBlock::iterator I = BB->begin();
        if (&*I == BI ||
            (&*I == cast<Instruction>(BI->getCondition()) &&
             &*++I == BI))
          if (FoldValueComparisonIntoPredecessors(BI))
            return SimplifyCFG(BB) | true;
      }
      
      // If this is a branch on a phi node in the current block, thread control
      // through this block if any PHI node entries are constants.
      if (PHINode *PN = dyn_cast<PHINode>(BI->getCondition()))
        if (PN->getParent() == BI->getParent())
          if (FoldCondBranchOnPHI(BI))
            return SimplifyCFG(BB) | true;

      // If this basic block is ONLY a setcc and a branch, and if a predecessor
      // branches to us and one of our successors, fold the setcc into the
      // predecessor and use logical operations to pick the right destination.
      BasicBlock *TrueDest  = BI->getSuccessor(0);
      BasicBlock *FalseDest = BI->getSuccessor(1);
      if (Instruction *Cond = dyn_cast<Instruction>(BI->getCondition())) {
        BasicBlock::iterator CondIt = Cond;
        if ((isa<CmpInst>(Cond) || isa<BinaryOperator>(Cond)) &&
            Cond->getParent() == BB && &BB->front() == Cond &&
            &*++CondIt == BI && Cond->hasOneUse() &&
            TrueDest != BB && FalseDest != BB)
          for (pred_iterator PI = pred_begin(BB), E = pred_end(BB); PI!=E; ++PI)
            if (BranchInst *PBI = dyn_cast<BranchInst>((*PI)->getTerminator()))
              if (PBI->isConditional() && SafeToMergeTerminators(BI, PBI)) {
                BasicBlock *PredBlock = *PI;
                if (PBI->getSuccessor(0) == FalseDest ||
                    PBI->getSuccessor(1) == TrueDest) {
                  // Invert the predecessors condition test (xor it with true),
                  // which allows us to write this code once.
                  Value *NewCond =
                    BinaryOperator::createNot(PBI->getCondition(),
                                    PBI->getCondition()->getName()+".not", PBI);
                  PBI->setCondition(NewCond);
                  BasicBlock *OldTrue = PBI->getSuccessor(0);
                  BasicBlock *OldFalse = PBI->getSuccessor(1);
                  PBI->setSuccessor(0, OldFalse);
                  PBI->setSuccessor(1, OldTrue);
                }

                if ((PBI->getSuccessor(0) == TrueDest && FalseDest != BB) ||
                    (PBI->getSuccessor(1) == FalseDest && TrueDest != BB)) {
                  // Clone Cond into the predecessor basic block, and or/and the
                  // two conditions together.
                  Instruction *New = Cond->clone();
                  PredBlock->getInstList().insert(PBI, New);
                  New->takeName(Cond);
                  Cond->setName(New->getName()+".old");
                  Instruction::BinaryOps Opcode =
                    PBI->getSuccessor(0) == TrueDest ?
                    Instruction::Or : Instruction::And;
                  Value *NewCond =
                    BinaryOperator::create(Opcode, PBI->getCondition(),
                                           New, "bothcond", PBI);
                  PBI->setCondition(NewCond);
                  if (PBI->getSuccessor(0) == BB) {
                    AddPredecessorToBlock(TrueDest, PredBlock, BB);
                    PBI->setSuccessor(0, TrueDest);
                  }
                  if (PBI->getSuccessor(1) == BB) {
                    AddPredecessorToBlock(FalseDest, PredBlock, BB);
                    PBI->setSuccessor(1, FalseDest);
                  }
                  return SimplifyCFG(BB) | 1;
                }
              }
      }

      // Scan predessor blocks for conditional branches.
      for (pred_iterator PI = pred_begin(BB), E = pred_end(BB); PI != E; ++PI)
        if (BranchInst *PBI = dyn_cast<BranchInst>((*PI)->getTerminator()))
          if (PBI != BI && PBI->isConditional()) {
              
            // If this block ends with a branch instruction, and if there is a
            // predecessor that ends on a branch of the same condition, make 
            // this conditional branch redundant.
            if (PBI->getCondition() == BI->getCondition() &&
                PBI->getSuccessor(0) != PBI->getSuccessor(1)) {
              // Okay, the outcome of this conditional branch is statically
              // knowable.  If this block had a single pred, handle specially.
              if (BB->getSinglePredecessor()) {
                // Turn this into a branch on constant.
                bool CondIsTrue = PBI->getSuccessor(0) == BB;
                BI->setCondition(ConstantInt::get(Type::Int1Ty, CondIsTrue));
                return SimplifyCFG(BB);  // Nuke the branch on constant.
              }
              
              // Otherwise, if there are multiple predecessors, insert a PHI 
              // that merges in the constant and simplify the block result.
              if (BlockIsSimpleEnoughToThreadThrough(BB)) {
                PHINode *NewPN = new PHINode(Type::Int1Ty,
                                            BI->getCondition()->getName()+".pr",
                                            BB->begin());
                for (PI = pred_begin(BB), E = pred_end(BB); PI != E; ++PI)
                  if ((PBI = dyn_cast<BranchInst>((*PI)->getTerminator())) &&
                      PBI != BI && PBI->isConditional() &&
                      PBI->getCondition() == BI->getCondition() &&
                      PBI->getSuccessor(0) != PBI->getSuccessor(1)) {
                    bool CondIsTrue = PBI->getSuccessor(0) == BB;
                    NewPN->addIncoming(ConstantInt::get(Type::Int1Ty, 
                                                        CondIsTrue), *PI);
                  } else {
                    NewPN->addIncoming(BI->getCondition(), *PI);
                  }
                
                BI->setCondition(NewPN);
                // This will thread the branch.
                return SimplifyCFG(BB) | true;
              }
            }
            
            // If this is a conditional branch in an empty block, and if any
            // predecessors is a conditional branch to one of our destinations,
            // fold the conditions into logical ops and one cond br.
            if (&BB->front() == BI) {
              int PBIOp, BIOp;
              if (PBI->getSuccessor(0) == BI->getSuccessor(0)) {
                PBIOp = BIOp = 0;
              } else if (PBI->getSuccessor(0) == BI->getSuccessor(1)) {
                PBIOp = 0; BIOp = 1;
              } else if (PBI->getSuccessor(1) == BI->getSuccessor(0)) {
                PBIOp = 1; BIOp = 0;
              } else if (PBI->getSuccessor(1) == BI->getSuccessor(1)) {
                PBIOp = BIOp = 1;
              } else {
                PBIOp = BIOp = -1;
              }
              
              // Check to make sure that the other destination of this branch
              // isn't BB itself.  If so, this is an infinite loop that will
              // keep getting unwound.
              if (PBIOp != -1 && PBI->getSuccessor(PBIOp) == BB)
                PBIOp = BIOp = -1;
              
              // Do not perform this transformation if it would require 
              // insertion of a large number of select instructions. For targets
              // without predication/cmovs, this is a big pessimization.
              if (PBIOp != -1) {
                BasicBlock *CommonDest = PBI->getSuccessor(PBIOp);
           
                unsigned NumPhis = 0;
                for (BasicBlock::iterator II = CommonDest->begin();
                     isa<PHINode>(II); ++II, ++NumPhis) {
                  if (NumPhis > 2) {
                    // Disable this xform.
                    PBIOp = -1;
                    break;
                  }
                }
              }

              // Finally, if everything is ok, fold the branches to logical ops.
              if (PBIOp != -1) {
                BasicBlock *CommonDest = PBI->getSuccessor(PBIOp);
                BasicBlock *OtherDest  = BI->getSuccessor(BIOp ^ 1);

                // If OtherDest *is* BB, then this is a basic block with just
                // a conditional branch in it, where one edge (OtherDesg) goes
                // back to the block.  We know that the program doesn't get
                // stuck in the infinite loop, so the condition must be such
                // that OtherDest isn't branched through. Forward to CommonDest,
                // and avoid an infinite loop at optimizer time.
                if (OtherDest == BB)
                  OtherDest = CommonDest;
                
                DOUT << "FOLDING BRs:" << *PBI->getParent()
                     << "AND: " << *BI->getParent();
                                
                // BI may have other predecessors.  Because of this, we leave
                // it alone, but modify PBI.
                
                // Make sure we get to CommonDest on True&True directions.
                Value *PBICond = PBI->getCondition();
                if (PBIOp)
                  PBICond = BinaryOperator::createNot(PBICond,
                                                      PBICond->getName()+".not",
                                                      PBI);
                Value *BICond = BI->getCondition();
                if (BIOp)
                  BICond = BinaryOperator::createNot(BICond,
                                                     BICond->getName()+".not",
                                                     PBI);
                // Merge the conditions.
                Value *Cond =
                  BinaryOperator::createOr(PBICond, BICond, "brmerge", PBI);
                
                // Modify PBI to branch on the new condition to the new dests.
                PBI->setCondition(Cond);
                PBI->setSuccessor(0, CommonDest);
                PBI->setSuccessor(1, OtherDest);

                // OtherDest may have phi nodes.  If so, add an entry from PBI's
                // block that are identical to the entries for BI's block.
                PHINode *PN;
                for (BasicBlock::iterator II = OtherDest->begin();
                     (PN = dyn_cast<PHINode>(II)); ++II) {
                  Value *V = PN->getIncomingValueForBlock(BB);
                  PN->addIncoming(V, PBI->getParent());
                }
                
                // We know that the CommonDest already had an edge from PBI to
                // it.  If it has PHIs though, the PHIs may have different
                // entries for BB and PBI's BB.  If so, insert a select to make
                // them agree.
                for (BasicBlock::iterator II = CommonDest->begin();
                     (PN = dyn_cast<PHINode>(II)); ++II) {
                  Value * BIV = PN->getIncomingValueForBlock(BB);
                  unsigned PBBIdx = PN->getBasicBlockIndex(PBI->getParent());
                  Value *PBIV = PN->getIncomingValue(PBBIdx);
                  if (BIV != PBIV) {
                    // Insert a select in PBI to pick the right value.
                    Value *NV = new SelectInst(PBICond, PBIV, BIV,
                                               PBIV->getName()+".mux", PBI);
                    PN->setIncomingValue(PBBIdx, NV);
                  }
                }

                DOUT << "INTO: " << *PBI->getParent();

                // This basic block is probably dead.  We know it has at least
                // one fewer predecessor.
                return SimplifyCFG(BB) | true;
              }
            }
          }
    }
  } else if (isa<UnreachableInst>(BB->getTerminator())) {
    // If there are any instructions immediately before the unreachable that can
    // be removed, do so.
    Instruction *Unreachable = BB->getTerminator();
    while (Unreachable != BB->begin()) {
      BasicBlock::iterator BBI = Unreachable;
      --BBI;
      if (isa<CallInst>(BBI)) break;
      // Delete this instruction
      BB->getInstList().erase(BBI);
      Changed = true;
    }

    // If the unreachable instruction is the first in the block, take a gander
    // at all of the predecessors of this instruction, and simplify them.
    if (&BB->front() == Unreachable) {
      SmallVector<BasicBlock*, 8> Preds(pred_begin(BB), pred_end(BB));
      for (unsigned i = 0, e = Preds.size(); i != e; ++i) {
        TerminatorInst *TI = Preds[i]->getTerminator();

        if (BranchInst *BI = dyn_cast<BranchInst>(TI)) {
          if (BI->isUnconditional()) {
            if (BI->getSuccessor(0) == BB) {
              new UnreachableInst(TI);
              TI->eraseFromParent();
              Changed = true;
            }
          } else {
            if (BI->getSuccessor(0) == BB) {
              new BranchInst(BI->getSuccessor(1), BI);
              BI->eraseFromParent();
            } else if (BI->getSuccessor(1) == BB) {
              new BranchInst(BI->getSuccessor(0), BI);
              BI->eraseFromParent();
              Changed = true;
            }
          }
        } else if (SwitchInst *SI = dyn_cast<SwitchInst>(TI)) {
          for (unsigned i = 1, e = SI->getNumCases(); i != e; ++i)
            if (SI->getSuccessor(i) == BB) {
              BB->removePredecessor(SI->getParent());
              SI->removeCase(i);
              --i; --e;
              Changed = true;
            }
          // If the default value is unreachable, figure out the most popular
          // destination and make it the default.
          if (SI->getSuccessor(0) == BB) {
            std::map<BasicBlock*, unsigned> Popularity;
            for (unsigned i = 1, e = SI->getNumCases(); i != e; ++i)
              Popularity[SI->getSuccessor(i)]++;

            // Find the most popular block.
            unsigned MaxPop = 0;
            BasicBlock *MaxBlock = 0;
            for (std::map<BasicBlock*, unsigned>::iterator
                   I = Popularity.begin(), E = Popularity.end(); I != E; ++I) {
              if (I->second > MaxPop) {
                MaxPop = I->second;
                MaxBlock = I->first;
              }
            }
            if (MaxBlock) {
              // Make this the new default, allowing us to delete any explicit
              // edges to it.
              SI->setSuccessor(0, MaxBlock);
              Changed = true;

              // If MaxBlock has phinodes in it, remove MaxPop-1 entries from
              // it.
              if (isa<PHINode>(MaxBlock->begin()))
                for (unsigned i = 0; i != MaxPop-1; ++i)
                  MaxBlock->removePredecessor(SI->getParent());

              for (unsigned i = 1, e = SI->getNumCases(); i != e; ++i)
                if (SI->getSuccessor(i) == MaxBlock) {
                  SI->removeCase(i);
                  --i; --e;
                }
            }
          }
        } else if (InvokeInst *II = dyn_cast<InvokeInst>(TI)) {
          if (II->getUnwindDest() == BB) {
            // Convert the invoke to a call instruction.  This would be a good
            // place to note that the call does not throw though.
            BranchInst *BI = new BranchInst(II->getNormalDest(), II);
            II->removeFromParent();   // Take out of symbol table

            // Insert the call now...
            SmallVector<Value*, 8> Args(II->op_begin()+3, II->op_end());
            CallInst *CI = new CallInst(II->getCalledValue(),
                                        Args.begin(), Args.end(),
                                        II->getName(), BI);
            CI->setCallingConv(II->getCallingConv());
            CI->setParamAttrs(II->getParamAttrs());
            // If the invoke produced a value, the Call does now instead.
            II->replaceAllUsesWith(CI);
            delete II;
            Changed = true;
          }
        }
      }

      // If this block is now dead, remove it.
      if (pred_begin(BB) == pred_end(BB)) {
        // We know there are no successors, so just nuke the block.
        M->getBasicBlockList().erase(BB);
        return true;
      }
    }
  }

  // Merge basic blocks into their predecessor if there is only one distinct
  // pred, and if there is only one distinct successor of the predecessor, and
  // if there are no PHI nodes.
  //
  pred_iterator PI(pred_begin(BB)), PE(pred_end(BB));
  BasicBlock *OnlyPred = *PI++;
  for (; PI != PE; ++PI)  // Search all predecessors, see if they are all same
    if (*PI != OnlyPred) {
      OnlyPred = 0;       // There are multiple different predecessors...
      break;
    }

  BasicBlock *OnlySucc = 0;
  if (OnlyPred && OnlyPred != BB &&    // Don't break self loops
      OnlyPred->getUnwindDest() != BB &&
      OnlyPred->getTerminator()->getOpcode() != Instruction::Invoke) {
    // Check to see if there is only one distinct successor...
    succ_iterator SI(succ_begin(OnlyPred)), SE(succ_end(OnlyPred));
    OnlySucc = BB;
    for (; SI != SE; ++SI)
      if (*SI != OnlySucc) {
        OnlySucc = 0;     // There are multiple distinct successors!
        break;
      }
  }

  if (OnlySucc && (BB->getUnwindDest() == OnlyPred->getUnwindDest() ||
                   !BB->getUnwindDest() || !OnlyPred->getUnwindDest())) {
    DOUT << "Merging: " << *BB << "into: " << *OnlyPred;

    // Resolve any PHI nodes at the start of the block.  They are all
    // guaranteed to have exactly one entry if they exist, unless there are
    // multiple duplicate (but guaranteed to be equal) entries for the
    // incoming edges.  This occurs when there are multiple edges from
    // OnlyPred to OnlySucc.
    //
    while (PHINode *PN = dyn_cast<PHINode>(&BB->front())) {
      PN->replaceAllUsesWith(PN->getIncomingValue(0));
      BB->getInstList().pop_front();  // Delete the phi node.
    }

    // Delete the unconditional branch from the predecessor.
    OnlyPred->getInstList().pop_back();

    // Move all definitions in the successor to the predecessor.
    OnlyPred->getInstList().splice(OnlyPred->end(), BB->getInstList());

    // Move the unwind_to block
    if (!OnlyPred->getUnwindDest() && BB->getUnwindDest())
      OnlyPred->setUnwindDest(BB->getUnwindDest());

    // Make all PHI nodes that referred to BB now refer to Pred as their
    // source.
    BB->replaceAllUsesWith(OnlyPred);

    // Inherit predecessors name if it exists.
    if (!OnlyPred->hasName())
      OnlyPred->takeName(BB);
    
    // Erase basic block from the function.
    M->getBasicBlockList().erase(BB);

    return true;
  }

  // Otherwise, if this block only has a single predecessor, and if that block
  // is a conditional branch, see if we can hoist any code from this block up
  // into our predecessor.
  if (OnlyPred)
    if (BranchInst *BI = dyn_cast<BranchInst>(OnlyPred->getTerminator()))
      if (BI->isConditional()) {
        // Get the other block.
        BasicBlock *OtherBB = BI->getSuccessor(BI->getSuccessor(0) == BB);
        PI = pred_begin(OtherBB);
        ++PI;
        if (PI == pred_end(OtherBB)) {
          // We have a conditional branch to two blocks that are only reachable
          // from the condbr.  We know that the condbr dominates the two blocks,
          // so see if there is any identical code in the "then" and "else"
          // blocks.  If so, we can hoist it up to the branching block.
          Changed |= HoistThenElseCodeToIf(BI);
        }
      }

  for (pred_iterator PI = pred_begin(BB), E = pred_end(BB); PI != E; ++PI)
    if (BranchInst *BI = dyn_cast<BranchInst>((*PI)->getTerminator()))
      // Change br (X == 0 | X == 1), T, F into a switch instruction.
      if (BI->isConditional() && isa<Instruction>(BI->getCondition())) {
        Instruction *Cond = cast<Instruction>(BI->getCondition());
        // If this is a bunch of seteq's or'd together, or if it's a bunch of
        // 'setne's and'ed together, collect them.
        Value *CompVal = 0;
        std::vector<ConstantInt*> Values;
        bool TrueWhenEqual = GatherValueComparisons(Cond, CompVal, Values);
        if (CompVal && CompVal->getType()->isInteger()) {
          // There might be duplicate constants in the list, which the switch
          // instruction can't handle, remove them now.
          std::sort(Values.begin(), Values.end(), ConstantIntOrdering());
          Values.erase(std::unique(Values.begin(), Values.end()), Values.end());

          // Figure out which block is which destination.
          BasicBlock *DefaultBB = BI->getSuccessor(1);
          BasicBlock *EdgeBB    = BI->getSuccessor(0);
          if (!TrueWhenEqual) std::swap(DefaultBB, EdgeBB);

          // Create the new switch instruction now.
          SwitchInst *New = new SwitchInst(CompVal, DefaultBB,Values.size(),BI);

          // Add all of the 'cases' to the switch instruction.
          for (unsigned i = 0, e = Values.size(); i != e; ++i)
            New->addCase(Values[i], EdgeBB);

          // We added edges from PI to the EdgeBB.  As such, if there were any
          // PHI nodes in EdgeBB, they need entries to be added corresponding to
          // the number of edges added.
          for (BasicBlock::iterator BBI = EdgeBB->begin();
               isa<PHINode>(BBI); ++BBI) {
            PHINode *PN = cast<PHINode>(BBI);
            Value *InVal = PN->getIncomingValueForBlock(*PI);
            for (unsigned i = 0, e = Values.size()-1; i != e; ++i)
              PN->addIncoming(InVal, *PI);
          }

          // Erase the old branch instruction.
          (*PI)->getInstList().erase(BI);

          // Erase the potentially condition tree that was used to computed the
          // branch condition.
          ErasePossiblyDeadInstructionTree(Cond);
          return true;
        }
      }

  // If there is a trivial two-entry PHI node in this basic block, and we can
  // eliminate it, do so now.
  if (PHINode *PN = dyn_cast<PHINode>(BB->begin()))
    if (PN->getNumIncomingValues() == 2)
      Changed |= FoldTwoEntryPHINode(PN); 

  return Changed;
}
