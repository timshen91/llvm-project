//===- AMDGPUAliasAnalysis ---------------------------------------*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
/// \file
/// This is the AMGPU address space based alias analysis pass.
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_AMDGPUALIASANALYSIS_H
#define LLVM_ANALYSIS_AMDGPUALIASANALYSIS_H

#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"

namespace llvm {

/// A simple AA result that uses TBAA metadata to answer queries.
class AMDGPUAAResult : public AAResultBase<AMDGPUAAResult> {
  friend AAResultBase<AMDGPUAAResult>;

  const DataLayout &DL;

public:
  explicit AMDGPUAAResult(const DataLayout &DL) : AAResultBase(), DL(DL) {}
  AMDGPUAAResult(AMDGPUAAResult &&Arg)
      : AAResultBase(std::move(Arg)), DL(Arg.DL){}

  /// Handle invalidation events from the new pass manager.
  ///
  /// By definition, this result is stateless and so remains valid.
  bool invalidate(Function &, const PreservedAnalyses &) { return false; }

  AliasResult alias(const MemoryLocation &LocA, const MemoryLocation &LocB);
  bool pointsToConstantMemory(const MemoryLocation &Loc, bool OrLocal);

private:
  bool Aliases(const MDNode *A, const MDNode *B) const;
  bool PathAliases(const MDNode *A, const MDNode *B) const;
};

/// Analysis pass providing a never-invalidated alias analysis result.
class AMDGPUAA : public AnalysisInfoMixin<AMDGPUAA> {
  friend AnalysisInfoMixin<AMDGPUAA>;
  static char PassID;

public:
  typedef AMDGPUAAResult Result;

  AMDGPUAAResult run(Function &F, AnalysisManager<Function> &AM) {
    return AMDGPUAAResult(F.getParent()->getDataLayout());
  }
};

/// Legacy wrapper pass to provide the AMDGPUAAResult object.
class AMDGPUAAWrapperPass : public ImmutablePass {
  std::unique_ptr<AMDGPUAAResult> Result;

public:
  static char ID;

  AMDGPUAAWrapperPass() : ImmutablePass(ID) {
    initializeAMDGPUAAWrapperPassPass(*PassRegistry::getPassRegistry());
  }

  AMDGPUAAResult &getResult() { return *Result; }
  const AMDGPUAAResult &getResult() const { return *Result; }

  bool doInitialization(Module &M) override {
    Result.reset(new AMDGPUAAResult(M.getDataLayout()));
    return false;
  }
  bool doFinalization(Module &M) override {
    Result.reset();
    return false;
  }
  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

}
#endif // LLVM_ANALYSIS_AMDGPUALIASANALYSIS_H
