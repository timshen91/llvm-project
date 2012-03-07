//===------ polly/Dependences.h - Polyhedral dependency analysis *- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Calculate the data dependency relations for a Scop using ISL.
//
// The integer set library (ISL) from Sven, has a integrated dependency analysis
// to calculate data dependences. This pass takes advantage of this and
// calculate those dependences a Scop.
//
// The dependences in this pass are exact in terms that for a specific read
// statement instance only the last write statement instance is returned. In
// case of may writes a set of possible write instances is returned. This
// analysis will never produce redundant dependences.
//
//===----------------------------------------------------------------------===//

#ifndef POLLY_DEPENDENCES_H
#define POLLY_DEPENDENCES_H

#include "polly/ScopPass.h"

#include <map>

struct isl_union_map;
struct isl_union_set;
struct isl_map;
struct isl_set;
struct clast_for;

using namespace llvm;

namespace polly {

  class Scop;
  class ScopStmt;

  class Dependences : public ScopPass {
  public:
    static char ID;

    /// @brief The type of the dependences.
    enum Type {
      // Write after read
      TYPE_WAR = 0x1,

      // Read after write
      TYPE_RAW = 0x2,

      // Write after write
      TYPE_WAW = 0x4,

      // All dependences
      TYPE_ALL = (TYPE_WAR | TYPE_RAW | TYPE_WAW)
    };

    typedef std::map<ScopStmt*, isl_map*> StatementToIslMapTy;

    Dependences();

    // @brief Check if a new scattering is valid.
    //
    // @param NewScattering The new scatterings
    //
    // @return bool True if the new scattering is valid, false it it reverses
    //              dependences.
    bool isValidScattering(StatementToIslMapTy *NewScatterings);

    /// @brief Check if a dimension of the Scop can be executed in parallel.
    ///
    /// @param LoopDomain The subset of the scattering space that is executed in
    ///                   parallel.
    /// @param ParallelDimension The scattering dimension that is being executed
    ///                          in parallel.
    ///
    /// @return bool Returns true, if executing parallelDimension in parallel is
    ///              valid for the scattering domain subset given.
    bool isParallelDimension(isl_set *LoopDomain, unsigned ParallelDimension);

    /// @brief Check if a loop is parallel
    ///
    /// Detect if a clast_for loop can be executed in parallel.
    ///
    /// @param f The clast for loop to check.
    ///
    /// @return bool Returns true if the incoming clast_for statement can
    ///              execute in parallel.
    bool isParallelFor(const clast_for *For);

    /// @brief Get the dependences in this Scop.
    ///
    /// @param Kinds This integer defines the different kinds of dependences
    ///              that will be returned. To return more than one kind, the
    ///              different kinds are 'ored' together.
    isl_union_map *getDependences(int Kinds);

    bool runOnScop(Scop &S);
    void printScop(raw_ostream &OS) const;
    virtual void releaseMemory();
    virtual void getAnalysisUsage(AnalysisUsage &AU) const;

private:
    isl_union_map *must_dep, *may_dep;

    isl_union_map *war_dep;
    isl_union_map *waw_dep;


    isl_union_map *sink;
    isl_union_map *must_source;
    isl_union_map *may_source;
  };


} // End polly namespace.

namespace llvm {
  class PassRegistry;
  void initializeDependencesPass(llvm::PassRegistry&);
}

#endif
