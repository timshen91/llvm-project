//===---- ScheduleDAGSDNodes.h - SDNode Scheduling --------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the ScheduleDAGSDNodes class, which implements
// scheduling for an SDNode-based dependency graph.
//
//===----------------------------------------------------------------------===//

#ifndef SCHEDULEDAGSDNODES_H
#define SCHEDULEDAGSDNODES_H

#include "llvm/CodeGen/ScheduleDAG.h"
#include "llvm/CodeGen/SelectionDAG.h"

namespace llvm {
  /// ScheduleDAGSDNodes - A ScheduleDAG for scheduling SDNode-based DAGs.
  ///
  /// Edges between SUnits are initially based on edges in the SelectionDAG,
  /// and additional edges can be added by the schedulers as heuristics.
  /// SDNodes such as Constants, Registers, and a few others that are not
  /// interesting to schedulers are not allocated SUnits.
  ///
  /// SDNodes with MVT::Glue operands are grouped along with the flagged
  /// nodes into a single SUnit so that they are scheduled together.
  ///
  /// SDNode-based scheduling graphs do not use SDep::Anti or SDep::Output
  /// edges.  Physical register dependence information is not carried in
  /// the DAG and must be handled explicitly by schedulers.
  ///
  class ScheduleDAGSDNodes : public ScheduleDAG {
  public:
    SelectionDAG *DAG;                    // DAG of the current basic block
    const InstrItineraryData *InstrItins;

    explicit ScheduleDAGSDNodes(MachineFunction &mf);

    virtual ~ScheduleDAGSDNodes() {}

    /// Run - perform scheduling.
    ///
    void Run(SelectionDAG *dag, MachineBasicBlock *bb,
             MachineBasicBlock::iterator insertPos);

    /// isPassiveNode - Return true if the node is a non-scheduled leaf.
    ///
    static bool isPassiveNode(SDNode *Node) {
      if (isa<ConstantSDNode>(Node))       return true;
      if (isa<ConstantFPSDNode>(Node))     return true;
      if (isa<RegisterSDNode>(Node))       return true;
      if (isa<RegisterMaskSDNode>(Node))   return true;
      if (isa<GlobalAddressSDNode>(Node))  return true;
      if (isa<BasicBlockSDNode>(Node))     return true;
      if (isa<FrameIndexSDNode>(Node))     return true;
      if (isa<ConstantPoolSDNode>(Node))   return true;
      if (isa<JumpTableSDNode>(Node))      return true;
      if (isa<ExternalSymbolSDNode>(Node)) return true;
      if (isa<BlockAddressSDNode>(Node))   return true;
      if (Node->getOpcode() == ISD::EntryToken ||
          isa<MDNodeSDNode>(Node)) return true;
      return false;
    }

    /// NewSUnit - Creates a new SUnit and return a ptr to it.
    ///
    SUnit *NewSUnit(SDNode *N);

    /// Clone - Creates a clone of the specified SUnit. It does not copy the
    /// predecessors / successors info nor the temporary scheduling states.
    ///
    SUnit *Clone(SUnit *N);

    /// BuildSchedGraph - Build the SUnit graph from the selection dag that we
    /// are input.  This SUnit graph is similar to the SelectionDAG, but
    /// excludes nodes that aren't interesting to scheduling, and represents
    /// flagged together nodes with a single SUnit.
    void BuildSchedGraph(AliasAnalysis *AA);

    /// InitVRegCycleFlag - Set isVRegCycle if this node's single use is
    /// CopyToReg and its only active data operands are CopyFromReg within a
    /// single block loop.
    ///
    void InitVRegCycleFlag(SUnit *SU);

    /// InitNumRegDefsLeft - Determine the # of regs defined by this node.
    ///
    void InitNumRegDefsLeft(SUnit *SU);

    /// ComputeLatency - Compute node latency.
    ///
    virtual void ComputeLatency(SUnit *SU);

    /// ComputeOperandLatency - Override dependence edge latency using
    /// operand use/def information
    ///
    virtual void ComputeOperandLatency(SUnit *Def, SUnit *Use,
                                       SDep& dep) const { }

    virtual void ComputeOperandLatency(SDNode *Def, SDNode *Use,
                                       unsigned OpIdx, SDep& dep) const;

    virtual MachineBasicBlock *EmitSchedule();

    /// Schedule - Order nodes according to selected style, filling
    /// in the Sequence member.
    ///
    virtual void Schedule() = 0;

    /// VerifyScheduledSequence - Verify that all SUnits are scheduled and
    /// consistent with the Sequence of scheduled instructions.
    void VerifyScheduledSequence(bool isBottomUp);

    virtual void dumpNode(const SUnit *SU) const;

    void dumpSchedule() const;

    virtual std::string getGraphNodeLabel(const SUnit *SU) const;

    virtual std::string getDAGName() const;

    virtual void getCustomGraphFeatures(GraphWriter<ScheduleDAG*> &GW) const;

    /// RegDefIter - In place iteration over the values defined by an
    /// SUnit. This does not need copies of the iterator or any other STLisms.
    /// The iterator creates itself, rather than being provided by the SchedDAG.
    class RegDefIter {
      const ScheduleDAGSDNodes *SchedDAG;
      const SDNode *Node;
      unsigned DefIdx;
      unsigned NodeNumDefs;
      EVT ValueType;
    public:
      RegDefIter(const SUnit *SU, const ScheduleDAGSDNodes *SD);

      bool IsValid() const { return Node != NULL; }

      EVT GetValue() const {
        assert(IsValid() && "bad iterator");
        return ValueType;
      }

      const SDNode *GetNode() const {
        return Node;
      }

      unsigned GetIdx() const {
        return DefIdx-1;
      }

      void Advance();
    private:
      void InitNodeNumDefs();
    };

  private:
    /// ClusterNeighboringLoads - Cluster loads from "near" addresses into
    /// combined SUnits.
    void ClusterNeighboringLoads(SDNode *Node);
    /// ClusterNodes - Cluster certain nodes which should be scheduled together.
    ///
    void ClusterNodes();

    /// BuildSchedUnits, AddSchedEdges - Helper functions for BuildSchedGraph.
    void BuildSchedUnits();
    void AddSchedEdges();
  };
}

#endif
