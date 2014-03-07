//===-- RegAllocPBQP.h ------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the PBQPBuilder interface, for classes which build PBQP
// instances to represent register allocation problems, and the RegAllocPBQP
// interface.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CODEGEN_REGALLOCPBQP_H
#define LLVM_CODEGEN_REGALLOCPBQP_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/PBQP/RegAllocSolver.h"
#include <map>
#include <set>

namespace llvm {

  class LiveIntervals;
  class MachineBlockFrequencyInfo;
  class MachineFunction;
  class TargetRegisterInfo;

  typedef PBQP::RegAlloc::Graph PBQPRAGraph;

  /// This class wraps up a PBQP instance representing a register allocation
  /// problem, plus the structures necessary to map back from the PBQP solution
  /// to a register allocation solution. (i.e. The PBQP-node <--> vreg map,
  /// and the PBQP option <--> storage location map).
  class PBQPRAProblem {
  public:

    typedef SmallVector<unsigned, 16> AllowedSet;

    PBQPRAGraph& getGraph() { return graph; }

    const PBQPRAGraph& getGraph() const { return graph; }

    /// Record the mapping between the given virtual register and PBQP node,
    /// and the set of allowed pregs for the vreg.
    ///
    /// If you are extending
    /// PBQPBuilder you are unlikely to need this: Nodes and options for all
    /// vregs will already have been set up for you by the base class.
    template <typename AllowedRegsItr>
    void recordVReg(unsigned vreg, PBQPRAGraph::NodeId nodeId,
                    AllowedRegsItr arBegin, AllowedRegsItr arEnd) {
      assert(node2VReg.find(nodeId) == node2VReg.end() && "Re-mapping node.");
      assert(vreg2Node.find(vreg) == vreg2Node.end() && "Re-mapping vreg.");
      assert(allowedSets[vreg].empty() && "vreg already has pregs.");

      node2VReg[nodeId] = vreg;
      vreg2Node[vreg] = nodeId;
      std::copy(arBegin, arEnd, std::back_inserter(allowedSets[vreg]));
    }

    /// Get the virtual register corresponding to the given PBQP node.
    unsigned getVRegForNode(PBQPRAGraph::NodeId nodeId) const;

    /// Get the PBQP node corresponding to the given virtual register.
    PBQPRAGraph::NodeId getNodeForVReg(unsigned vreg) const;

    /// Returns true if the given PBQP option represents a physical register,
    /// false otherwise.
    bool isPRegOption(unsigned vreg, unsigned option) const {
      // At present we only have spills or pregs, so anything that's not a
      // spill is a preg. (This might be extended one day to support remat).
      return !isSpillOption(vreg, option);
    }

    /// Returns true if the given PBQP option represents spilling, false
    /// otherwise.
    bool isSpillOption(unsigned vreg, unsigned option) const {
      // We hardcode option zero as the spill option.
      return option == 0;
    }

    /// Returns the allowed set for the given virtual register.
    const AllowedSet& getAllowedSet(unsigned vreg) const;

    /// Get PReg for option.
    unsigned getPRegForOption(unsigned vreg, unsigned option) const;

  private:

    typedef std::map<PBQPRAGraph::NodeId, unsigned>  Node2VReg;
    typedef DenseMap<unsigned, PBQPRAGraph::NodeId> VReg2Node;
    typedef DenseMap<unsigned, AllowedSet> AllowedSetMap;

    PBQPRAGraph graph;
    Node2VReg node2VReg;
    VReg2Node vreg2Node;

    AllowedSetMap allowedSets;

  };

  /// Builds PBQP instances to represent register allocation problems. Includes
  /// spill, interference and coalescing costs by default. You can extend this
  /// class to support additional constraints for your architecture.
  class PBQPBuilder {
  private:
    PBQPBuilder(const PBQPBuilder&) LLVM_DELETED_FUNCTION;
    void operator=(const PBQPBuilder&) LLVM_DELETED_FUNCTION;
  public:

    typedef std::set<unsigned> RegSet;

    /// Default constructor.
    PBQPBuilder() {}

    /// Clean up a PBQPBuilder.
    virtual ~PBQPBuilder() {}

    /// Build a PBQP instance to represent the register allocation problem for
    /// the given MachineFunction.
    virtual PBQPRAProblem *build(MachineFunction *mf, const LiveIntervals *lis,
                                 const MachineBlockFrequencyInfo *mbfi,
                                 const RegSet &vregs);
  private:

    void addSpillCosts(PBQP::Vector &costVec, PBQP::PBQPNum spillCost);

    void addInterferenceCosts(PBQP::Matrix &costMat,
                              const PBQPRAProblem::AllowedSet &vr1Allowed,
                              const PBQPRAProblem::AllowedSet &vr2Allowed,
                              const TargetRegisterInfo *tri);
  };

  /// Extended builder which adds coalescing constraints to a problem.
  class PBQPBuilderWithCoalescing : public PBQPBuilder {
  public:

    /// Build a PBQP instance to represent the register allocation problem for
    /// the given MachineFunction.
    PBQPRAProblem *build(MachineFunction *mf, const LiveIntervals *lis,
                         const MachineBlockFrequencyInfo *mbfi,
                         const RegSet &vregs) override;

  private:

    void addPhysRegCoalesce(PBQP::Vector &costVec, unsigned pregOption,
                            PBQP::PBQPNum benefit);

    void addVirtRegCoalesce(PBQP::Matrix &costMat,
                            const PBQPRAProblem::AllowedSet &vr1Allowed,
                            const PBQPRAProblem::AllowedSet &vr2Allowed,
                            PBQP::PBQPNum benefit);
  };

  FunctionPass *
  createPBQPRegisterAllocator(std::unique_ptr<PBQPBuilder> &builder,
                              char *customPassID = 0);
}

#endif /* LLVM_CODEGEN_REGALLOCPBQP_H */
