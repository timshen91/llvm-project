//===-- NVPTXInferAddressSpace.cpp - ---------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// CUDA C/C++ includes memory space designation as variable type qualifers (such
// as __global__ and __shared__). Knowing the space of a memory access allows
// CUDA compilers to emit faster PTX loads and stores. For example, a load from
// shared memory can be translated to `ld.shared` which is roughly 10% faster
// than a generic `ld` on an NVIDIA Tesla K40c.
//
// Unfortunately, type qualifiers only apply to variable declarations, so CUDA
// compilers must infer the memory space of an address expression from
// type-qualified variables.
//
// LLVM IR uses non-zero (so-called) specific address spaces to represent memory
// spaces (e.g. addrspace(3) means shared memory). The Clang frontend
// places only type-qualified variables in specific address spaces, and then
// conservatively `addrspacecast`s each type-qualified variable to addrspace(0)
// (so-called the generic address space) for other instructions to use.
//
// For example, the Clang translates the following CUDA code
//   __shared__ float a[10];
//   float v = a[i];
// to
//   %0 = addrspacecast [10 x float] addrspace(3)* @a to [10 x float]*
//   %1 = gep [10 x float], [10 x float]* %0, i64 0, i64 %i
//   %v = load float, float* %1 ; emits ld.f32
// @a is in addrspace(3) since it's type-qualified, but its use from %1 is
// redirected to %0 (the generic version of @a).
//
// The optimization implemented in this file propagates specific address spaces
// from type-qualified variable declarations to its users. For example, it
// optimizes the above IR to
//   %1 = gep [10 x float] addrspace(3)* @a, i64 0, i64 %i
//   %v = load float addrspace(3)* %1 ; emits ld.shared.f32
// propagating the addrspace(3) from @a to %1. As the result, the NVPTX
// codegen is able to emit ld.shared.f32 for %v.
//
// Address space inference works in two steps. First, it uses a data-flow
// analysis to infer as many generic pointers as possible to point to only one
// specific address space. In the above example, it can prove that %1 only
// points to addrspace(3). This algorithm was published in
//   CUDA: Compiling and optimizing for a GPU platform
//   Chakrabarti, Grover, Aarts, Kong, Kudlur, Lin, Marathe, Murphy, Wang
//   ICCS 2012
//
// Then, address space inference replaces all refinable generic pointers with
// equivalent specific pointers.
//
// The major challenge of implementing this optimization is handling PHINodes,
// which may create loops in the data flow graph. This brings two complications.
//
// First, the data flow analysis in Step 1 needs to be circular. For example,
//     %generic.input = addrspacecast float addrspace(3)* %input to float*
//   loop:
//     %y = phi [ %generic.input, %y2 ]
//     %y2 = getelementptr %y, 1
//     %v = load %y2
//     br ..., label %loop, ...
// proving %y specific requires proving both %generic.input and %y2 specific,
// but proving %y2 specific circles back to %y. To address this complication,
// the data flow analysis operates on a lattice:
//   uninitialized > specific address spaces > generic.
// All address expressions (our implementation only considers phi, bitcast,
// addrspacecast, and getelementptr) start with the uninitialized address space.
// The monotone transfer function moves the address space of a pointer down a
// lattice path from uninitialized to specific and then to generic. A join
// operation of two different specific address spaces pushes the expression down
// to the generic address space. The analysis completes once it reaches a fixed
// point.
//
// Second, IR rewriting in Step 2 also needs to be circular. For example,
// converting %y to addrspace(3) requires the compiler to know the converted
// %y2, but converting %y2 needs the converted %y. To address this complication,
// we break these cycles using "undef" placeholders. When converting an
// instruction `I` to a new address space, if its operand `Op` is not converted
// yet, we let `I` temporarily use `undef` and fix all the uses of undef later.
// For instance, our algorithm first converts %y to
//   %y' = phi float addrspace(3)* [ %input, undef ]
// Then, it converts %y2 to
//   %y2' = getelementptr %y', 1
// Finally, it fixes the undef in %y' so that
//   %y' = phi float addrspace(3)* [ %input, %y2' ]
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "nvptx-infer-addrspace"

#include "NVPTX.h"
#include "MCTargetDesc/NVPTXBaseInfo.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Operator.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/ValueMapper.h"

#include <queue>

using namespace llvm;

namespace {
const unsigned ADDRESS_SPACE_UNINITIALIZED = (unsigned)-1;

using ValueToAddrSpaceMapTy = DenseMap<const Value *, unsigned>;

/// \brief NVPTXInferAddressSpaces
class NVPTXInferAddressSpaces : public ModulePass {
public:
  static char ID;

  NVPTXInferAddressSpaces() : ModulePass(ID) {}

  bool runOnModule(Module &M) override;

private:
  // Returns the new address space of V if updated; otherwise, returns None.
  Optional<unsigned>
  updateAddressSpace(const Value &V,
                     const ValueToAddrSpaceMapTy &InferredAddrSpace);

  // Tries to infer the specific address space of each address expression in
  // Postorder.
  void inferAddressSpaces(const std::vector<Value *> &Postorder,
                          ValueToAddrSpaceMapTy *InferredAddrSpace);

  // Changes the generic address expressions in function F to point to specific
  // address spaces if InferredAddrSpace says so. Postorder is the postorder of
  // all generic address expressions in the use-def graph of function F.
  bool
  rewriteWithNewAddressSpaces(const std::vector<Value *> &Postorder,
                              const ValueToAddrSpaceMapTy &InferredAddrSpace,
                              Function *F);
};
} // end anonymous namespace

char NVPTXInferAddressSpaces::ID = 0;

namespace llvm {
void initializeNVPTXInferAddressSpacesPass(PassRegistry &);
}
INITIALIZE_PASS(NVPTXInferAddressSpaces, "nvptx-infer-addrspace",
                "Infer address spaces",
                false, false)

// Returns true if V is an address expression.
// TODO: Currently, we consider only phi, bitcast, addrspacecast, and
// getelementptr operators.
static bool isAddressExpression(const Value &V) {
  if (!isa<Operator>(V))
    return false;

  switch (cast<Operator>(V).getOpcode()) {
  case Instruction::PHI:
  case Instruction::BitCast:
  case Instruction::AddrSpaceCast:
  case Instruction::GetElementPtr:
    return true;
  default:
    return false;
  }
}

// Returns the pointer operands of V.
//
// Precondition: V is an address expression.
static SmallVector<Value *, 2> getPointerOperands(const Value &V) {
  assert(isAddressExpression(V));
  const Operator& Op = cast<Operator>(V);
  switch (Op.getOpcode()) {
  case Instruction::PHI: {
    auto IncomingValues = cast<PHINode>(Op).incoming_values();
    return SmallVector<Value *, 2>(IncomingValues.begin(),
                                   IncomingValues.end());
  }
  case Instruction::BitCast:
  case Instruction::AddrSpaceCast:
  case Instruction::GetElementPtr:
    return {Op.getOperand(0)};
  default:
    llvm_unreachable("Unexpected instruction type.");
  }
}

// If V is an unvisited generic address expression, appends V to PostorderStack
// and marks it as visited.
static void appendsGenericAddressExpressionToPostorderStack(
    Value *V, std::vector<std::pair<Value *, bool>> *PostorderStack,
    DenseSet<Value *> *Visited) {
  assert(V->getType()->isPointerTy());
  if (isAddressExpression(*V) &&
      V->getType()->getPointerAddressSpace() ==
          AddressSpace::ADDRESS_SPACE_GENERIC) {
    if (Visited->insert(V).second)
      PostorderStack->push_back(std::make_pair(V, false));
  }
}

// Returns all generic address expressions in function F. The elements are
// ordered in postorder.
static std::vector<Value *> collectGenericAddressExpressions(Function &F) {
  // This function implements a non-recursive postorder traversal of a partial
  // use-def graph of function F.
  std::vector<std::pair<Value*, bool>> PostorderStack;
  // The set of visited expressions.
  DenseSet<Value*> Visited;
  // We only explore address expressions that are reachable from loads and
  // stores for now because we aim at generating faster loads and stores.
  for (Instruction &I : instructions(F)) {
    if (isa<LoadInst>(I)) {
      appendsGenericAddressExpressionToPostorderStack(
          I.getOperand(0), &PostorderStack, &Visited);
    } else if (isa<StoreInst>(I)) {
      appendsGenericAddressExpressionToPostorderStack(
          I.getOperand(1), &PostorderStack, &Visited);
    }
  }

  std::vector<Value *> Postorder; // The resultant postorder.
  while (!PostorderStack.empty()) {
    // If the operands of the expression on the top are already explored,
    // adds that expression to the resultant postorder.
    if (PostorderStack.back().second) {
      Postorder.push_back(PostorderStack.back().first);
      PostorderStack.pop_back();
      continue;
    }
    // Otherwise, adds its operands to the stack and explores them.
    PostorderStack.back().second = true;
    for (Value *PtrOperand : getPointerOperands(*PostorderStack.back().first)) {
      appendsGenericAddressExpressionToPostorderStack(
          PtrOperand, &PostorderStack, &Visited);
    }
  }
  return Postorder;
}

// A helper function for cloneInstructionWithNewAddressSpace. Returns the clone
// of OperandUse.get() in the new address space. If the clone is not ready yet,
// returns an undef in the new address space as a placeholder.
static Value *operandWithNewAddressSpaceOrCreateUndef(
    const Use &OperandUse, unsigned NewAddrSpace,
    const ValueToValueMapTy &ValueWithNewAddrSpace,
    SmallVectorImpl<const Use *> *UndefUsesToFix) {
  Value *Operand = OperandUse.get();
  if (Value *NewOperand = ValueWithNewAddrSpace.lookup(Operand))
    return NewOperand;

  UndefUsesToFix->push_back(&OperandUse);
  return UndefValue::get(
      Operand->getType()->getPointerElementType()->getPointerTo(NewAddrSpace));
}

// Returns a clone of `I` with its operands converted to those specified in
// ValueWithNewAddrSpace. Due to potential cycles in the data flow graph, an
// operand whose address space needs to be modified might not exist in
// ValueWithNewAddrSpace. In that case, uses undef as a placeholder operand and
// adds that operand use to UndefUsesToFix so that caller can fix them later.
//
// Note that we do not necessarily clone `I`, e.g., if it is an addrspacecast
// from a pointer whose type already matches. Therefore, this function returns a
// Value* instead of an Instruction*.
static Value *cloneInstructionWithNewAddressSpace(
    Instruction *I, unsigned NewAddrSpace,
    const ValueToValueMapTy &ValueWithNewAddrSpace,
    SmallVectorImpl<const Use *> *UndefUsesToFix) {
  Type *NewPtrType =
      I->getType()->getPointerElementType()->getPointerTo(NewAddrSpace);

  if (I->getOpcode() == Instruction::AddrSpaceCast) {
    Value *Src = I->getOperand(0);
    // Because `I` is generic, the source address space must be specific.
    // Therefore, the inferred address space must be the source space, according
    // to our algorithm.
    assert(Src->getType()->getPointerAddressSpace() == NewAddrSpace);
    if (Src->getType() != NewPtrType)
      return new BitCastInst(Src, NewPtrType);
    return Src;
  }

  // Computes the converted pointer operands.
  SmallVector<Value *, 4> NewPointerOperands;
  for (const Use &OperandUse : I->operands()) {
    if (!OperandUse.get()->getType()->isPointerTy())
      NewPointerOperands.push_back(nullptr);
    else
      NewPointerOperands.push_back(operandWithNewAddressSpaceOrCreateUndef(
          OperandUse, NewAddrSpace, ValueWithNewAddrSpace, UndefUsesToFix));
  }

  switch (I->getOpcode()) {
  case Instruction::BitCast:
    return new BitCastInst(NewPointerOperands[0], NewPtrType);
  case Instruction::PHI: {
    assert(I->getType()->isPointerTy());
    PHINode *PHI = cast<PHINode>(I);
    PHINode *NewPHI = PHINode::Create(NewPtrType, PHI->getNumIncomingValues());
    for (unsigned Index = 0; Index < PHI->getNumIncomingValues(); ++Index) {
      unsigned OperandNo = PHINode::getOperandNumForIncomingValue(Index);
      NewPHI->addIncoming(NewPointerOperands[OperandNo],
                          PHI->getIncomingBlock(Index));
    }
    return NewPHI;
  }
  case Instruction::GetElementPtr: {
    GetElementPtrInst *GEP = cast<GetElementPtrInst>(I);
    GetElementPtrInst *NewGEP = GetElementPtrInst::Create(
        GEP->getSourceElementType(), NewPointerOperands[0],
        SmallVector<Value *, 4>(GEP->idx_begin(), GEP->idx_end()));
    NewGEP->setIsInBounds(GEP->isInBounds());
    return NewGEP;
  }
  default:
    llvm_unreachable("Unexpected opcode");
  }
}

// Similar to cloneInstructionWithNewAddressSpace, returns a clone of the
// constant expression `CE` with its operands replaced as specified in
// ValueWithNewAddrSpace.
static Value *cloneConstantExprWithNewAddressSpace(
    ConstantExpr *CE, unsigned NewAddrSpace,
    const ValueToValueMapTy &ValueWithNewAddrSpace) {
  Type *TargetType =
      CE->getType()->getPointerElementType()->getPointerTo(NewAddrSpace);

  if (CE->getOpcode() == Instruction::AddrSpaceCast) {
    // Because CE is generic, the source address space must be specific.
    // Therefore, the inferred address space must be the source space according
    // to our algorithm.
    assert(CE->getOperand(0)->getType()->getPointerAddressSpace() ==
           NewAddrSpace);
    return ConstantExpr::getBitCast(CE->getOperand(0), TargetType);
  }

  // Computes the operands of the new constant expression.
  SmallVector<Constant *, 4> NewOperands;
  for (unsigned Index = 0; Index < CE->getNumOperands(); ++Index) {
    Constant *Operand = CE->getOperand(Index);
    // If the address space of `Operand` needs to be modified, the new operand
    // with the new address space should already be in ValueWithNewAddrSpace
    // because (1) the constant expressions we consider (i.e. addrspacecast,
    // bitcast, and getelementptr) do not incur cycles in the data flow graph
    // and (2) this function is called on constant expressions in postorder.
    if (Value *NewOperand = ValueWithNewAddrSpace.lookup(Operand)) {
      NewOperands.push_back(cast<Constant>(NewOperand));
    } else {
      // Otherwise, reuses the old operand.
      NewOperands.push_back(Operand);
    }
  }

  if (CE->getOpcode() == Instruction::GetElementPtr) {
    // Needs to specify the source type while constructing a getelementptr
    // constant expression.
    return CE->getWithOperands(
        NewOperands, TargetType, /*OnlyIfReduced=*/false,
        NewOperands[0]->getType()->getPointerElementType());
  }

  return CE->getWithOperands(NewOperands, TargetType);
}

// Returns a clone of the value `V`, with its operands replaced as specified in
// ValueWithNewAddrSpace. This function is called on every generic address
// expression whose address space needs to be modified, in postorder.
//
// See cloneInstructionWithNewAddressSpace for the meaning of UndefUsesToFix.
static Value *
cloneValueWithNewAddressSpace(Value *V, unsigned NewAddrSpace,
                              const ValueToValueMapTy &ValueWithNewAddrSpace,
                              SmallVectorImpl<const Use *> *UndefUsesToFix) {
  // All values in Postorder are generic address expressions.
  assert(isAddressExpression(*V) &&
         V->getType()->getPointerAddressSpace() ==
             AddressSpace::ADDRESS_SPACE_GENERIC);

  if (Instruction *I = dyn_cast<Instruction>(V)) {
    Value *NewV = cloneInstructionWithNewAddressSpace(
        I, NewAddrSpace, ValueWithNewAddrSpace, UndefUsesToFix);
    if (Instruction *NewI = dyn_cast<Instruction>(NewV)) {
      if (NewI->getParent() == nullptr) {
        NewI->insertBefore(I);
        NewI->takeName(I);
      }
    }
    return NewV;
  }

  return cloneConstantExprWithNewAddressSpace(
      cast<ConstantExpr>(V), NewAddrSpace, ValueWithNewAddrSpace);
}

// Defines the join operation on the address space lattice (see the file header
// comments).
static unsigned joinAddressSpaces(unsigned AS1, unsigned AS2) {
  if (AS1 == AddressSpace::ADDRESS_SPACE_GENERIC ||
      AS2 == AddressSpace::ADDRESS_SPACE_GENERIC)
    return AddressSpace::ADDRESS_SPACE_GENERIC;

  if (AS1 == ADDRESS_SPACE_UNINITIALIZED)
    return AS2;
  if (AS2 == ADDRESS_SPACE_UNINITIALIZED)
    return AS1;

  // The join of two different specific address spaces is generic.
  return AS1 == AS2 ? AS1 : (unsigned)AddressSpace::ADDRESS_SPACE_GENERIC;
}

enum class Cost : unsigned {
  GLOBAL = 1 << 0,
  SHARED = 1 << 1,
  CONST = 1 << 2,
  LOCAL = 1 << 3,
  PARAM = 1 << 4,

  ZERO = (1 << 5) - 1,
  INF = 0,
};

Cost Propagate(Cost Original, Cost Delta) {
  return (Cost)((unsigned)Original & (unsigned)Delta);
}

bool Merge(Cost &Original, Cost Delta) {
  Cost New = (Cost)((unsigned)Original | (unsigned)Delta);
  bool Ret = Original != New;
  Original = New;
  return Ret;
}

static Cost AddrSpaceToCost(AddressSpace AddrSpace) {
  switch (AddrSpace) {
  case ADDRESS_SPACE_GENERIC:
    return Cost::ZERO;
  case ADDRESS_SPACE_GLOBAL:
    return Cost::GLOBAL;
  case ADDRESS_SPACE_SHARED:
    return Cost::SHARED;
  case ADDRESS_SPACE_CONST:
    return Cost::CONST;
  case ADDRESS_SPACE_LOCAL:
    return Cost::LOCAL;
  case ADDRESS_SPACE_PARAM:
    return Cost::PARAM;
  }
}

static Optional<AddressSpace> CostToSingleAddrSpace(Cost C) {
  switch (C) {
  default:
    return Optional<AddressSpace>{};
  case Cost::GLOBAL:
    return ADDRESS_SPACE_GLOBAL;
  case Cost::SHARED:
    return ADDRESS_SPACE_SHARED;
  case Cost::CONST:
    return ADDRESS_SPACE_CONST;
  case Cost::LOCAL:
    return ADDRESS_SPACE_LOCAL;
  case Cost::PARAM:
    return ADDRESS_SPACE_PARAM;
  case Cost::ZERO:
    return ADDRESS_SPACE_GENERIC;
  }
}

class Graph {
  std::vector<std::pair<const Value *, std::vector<std::pair<int, Cost>>>>
      Nodes;
  std::map<const Value *, int> ValueMap;

public:
  int addNode(const Value *Node) {
    if (ValueMap.count(Node)) {
      return ValueMap[Node];
    }
    Nodes.emplace_back(Node, std::vector<std::pair<int, Cost>>{});
    return ValueMap[Node] = Nodes.size() - 1;
  }
  // TODO: Duplicated edges
  void addEdge(int Left, int Right, Cost Cost) {
    Nodes[Left].second.emplace_back(Right, Cost);
  }
  int getNumNodes() const { return Nodes.size(); }
  const Value *getNodeValue(int Index) const { return Nodes[Index].first; }
  Optional<int> getNodeByValue(const Value *V) const {
    if (ValueMap.count(V)) {
      return ValueMap.at(V);
    }
    return Optional<int>{};
  }
  iterator_range<std::vector<std::pair<int, Cost>>::const_iterator>
  getSuccessors(int Index) const {
    return Nodes[Index].second;
  }
  void dump() const {
    dbgs() << "Graph: \n";
    dbgs() << "  Nodes:\n";
    for (size_t i = 0; i < Nodes.size(); i++) {
      dbgs() << "    " << i << ": ";
      if (Nodes[i].first) {
        Nodes[i].first->dump();
      } else {
        dbgs() << "Phony source\n";
      }
    }
    dbgs() << "  Edges:\n";
    for (size_t i = 0; i < Nodes.size(); i++) {
      for (const auto &Tp : Nodes[i].second) {
        int Right;
        Cost C;
        std::tie(Right, C) = Tp;
        fprintf(stderr, "    %zu -> %d: 0x%x\n", i, Right, (unsigned)C);
      }
    }
  }
};

static std::pair<Graph, int> buildGraph(const Module &M) {
  Graph G;
  int Source = G.addNode(nullptr);
  for (const GlobalVariable &GV : M.getGlobalList()) {
    const auto *Pointer = cast<PointerType>(GV.getType());
    G.addEdge(Source, G.addNode(&GV),
              AddrSpaceToCost((AddressSpace)Pointer->getAddressSpace()));
  }
  for (const Function &F : M) {
    for (const BasicBlock &BB : F) {
      for (const Instruction &Inst : BB) {
        // Nodes that create pointers
        if (isa<AddrSpaceCastInst>(Inst) || isa<AllocaInst>(Inst)) {
          if (const auto *Pointer = dyn_cast<PointerType>(Inst.getType())) {
            int Dest = G.addNode(&Inst);
            for (const Use &U : Inst.operands()) {
              G.addEdge(
                  G.addNode(U.get()), Dest,
                  AddrSpaceToCost((AddressSpace)Pointer->getAddressSpace()));
            }
          }
          // Nodes that propagate other pointers
        } else if (isa<PHINode>(Inst) || isa<GetElementPtrInst>(Inst)) {
          int Dest = G.addNode(&Inst);
          for (const Use &U : Inst.operands()) {
            G.addEdge(G.addNode(U.get()), Dest, Cost::ZERO);
          }
          // IPO.
          // For `call %ret = @f(%arg_a, %arg_b)`, where `define ret_type
          // @f(%param_a, %param_b)`,
          // Create edges:
          //   %arg_a -> %param_a
          //   %arg_b -> %param_b
          //   @f -> %ret
          // Thus @f is used as the `returned param`. It's hacky, should create
          // a phony node for returned param.
        } else if (const auto *Call = dyn_cast<CallInst>(&Inst)) {
          const auto *Callee = Call->getCalledFunction();

          auto ArgIt = Inst.op_begin();
          auto ParamIt = Callee->arg_begin();
          while (ArgIt != Inst.op_end() && ParamIt != Callee->arg_end()) {
            G.addEdge(G.addNode(ArgIt->get()), G.addNode(&*ParamIt),
                      Cost::ZERO);
            ++ArgIt;
            ++ParamIt;
          }
          G.addEdge(G.addNode(Callee), G.addNode(&Inst), Cost::ZERO);
          // `ret %e` creates
          //   %e -> @f
        } else if (const auto *Return = dyn_cast<ReturnInst>(&Inst)) {
          if (const auto *RV = Return->getReturnValue()) {
            G.addEdge(G.addNode(RV), G.addNode(&F), Cost::ZERO);
          }
        }
      }
    }
    if (F.hasLocalLinkage()) {
      // internal functions' parameters propagate other pointers.
      for (const Argument &Arg : F.getArgumentList()) {
        int ArgNode = G.addNode(&Arg);
        for (const Value *V : Arg.users()) {
          G.addEdge(ArgNode, G.addNode(V), Cost::ZERO);
        }
      }
    } else {
      // extern function parameters create pointers.
      for (const Argument &Arg : F.getArgumentList()) {
        if (const auto *Pointer = dyn_cast<PointerType>(Arg.getType())) {
          G.addEdge(Source, G.addNode(&Arg),
                    AddrSpaceToCost((AddressSpace)Pointer->getAddressSpace()));
        }
      }
    }
  }
  // G.dump();
  return std::make_pair(G, Source);
}

struct ShortestPaths {
  std::vector<Cost> Distances;

  void dump() {
    dbgs() << "Distances:\n";
    for (size_t i = 0; i < Distances.size(); i++) {
      fprintf(stderr, "  %zu: %x\n", i, (unsigned)Distances[i]);
    }
  }
};

static ShortestPaths runShortestPath(const Graph &G, int Source) {
  ShortestPaths Paths;
  Paths.Distances.assign(G.getNumNodes(), Cost::INF);
  Paths.Distances[Source] = Cost::ZERO;
  std::queue<int> Q;
  std::vector<char> InQueue(G.getNumNodes());
  const auto Push = [&](int Node) {
    assert(!InQueue[Node]);
    Q.push(Node);
    InQueue[Node] = true;
  };
  const auto Pop = [&]() -> int {
    assert(!Q.empty());
    int Ret = Q.front();
    InQueue[Ret] = false;
    Q.pop();
    return Ret;
  };
  Push(Source);
  while (!Q.empty()) {
    int Current = Pop();
    for (const auto &Tp : G.getSuccessors(Current)) {
      int To;
      Cost C;
      std::tie(To, C) = Tp;
      if (Merge(Paths.Distances[To], Propagate(Paths.Distances[Current], C))) {
        if (!InQueue[To]) {
          Push(To);
        }
      }
    }
  }
  // Paths.dump();
  return Paths;
}

static bool rewrite(const Graph &G, const ShortestPaths &P, Module &M) {
  bool Changed = false;

  // TODO: add more instructions to rewrite.
  for (size_t i = 0; i < P.Distances.size(); i++) {
    if (auto AddrSpace = CostToSingleAddrSpace(P.Distances[i])) {
      if (auto V = G.getNodeValue(i)) {
        std::vector<Use *> Uses;
        for (const Use &U : V->uses()) {
          Uses.push_back(const_cast<Use *>(&U));
        }
        for (Use *U : Uses) {
          if (auto *Load = dyn_cast<LoadInst>(U->getUser())) {
            auto *Operand = Load->getPointerOperand();
            U->set(IRBuilder<>(Load).CreateAddrSpaceCast(
                Operand,
                Operand->getType()->getPointerElementType()->getPointerTo(
                    *AddrSpace)));
            Changed = true;
          } else if (auto *Store = dyn_cast<StoreInst>(U->getUser())) {
            auto *Operand = Store->getPointerOperand();
            U->set(IRBuilder<>(Store).CreateAddrSpaceCast(
                Operand,
                Operand->getType()->getPointerElementType()->getPointerTo(
                    *AddrSpace)));
            Changed = true;
          }
        }
      }
    }
  }

  return Changed;
}

bool NVPTXInferAddressSpaces::runOnModule(Module &M) {
  if (skipModule(M))
    return false;

  auto Pr = buildGraph(M);
  return rewrite(Pr.first, runShortestPath(Pr.first, Pr.second), M);
//
//  // Collects all generic address expressions in postorder.
//  std::vector<Value *> Postorder = collectGenericAddressExpressions(F);
//
//  // Runs a data-flow analysis to refine the address spaces of every expression
//  // in Postorder.
//  ValueToAddrSpaceMapTy InferredAddrSpace;
//  inferAddressSpaces(Postorder, &InferredAddrSpace);
//
//  // Changes the address spaces of the generic address expressions who are
//  // inferred to point to a specific address space.
//  return rewriteWithNewAddressSpaces(Postorder, InferredAddrSpace, &F);
}

void NVPTXInferAddressSpaces::inferAddressSpaces(
    const std::vector<Value *> &Postorder,
    ValueToAddrSpaceMapTy *InferredAddrSpace) {
  SetVector<Value *> Worklist(Postorder.begin(), Postorder.end());
  // Initially, all expressions are in the uninitialized address space.
  for (Value *V : Postorder)
    (*InferredAddrSpace)[V] = ADDRESS_SPACE_UNINITIALIZED;

  while (!Worklist.empty()) {
    Value* V = Worklist.pop_back_val();

    // Tries to update the address space of the stack top according to the
    // address spaces of its operands.
    DEBUG(dbgs() << "Updating the address space of\n"
                 << "  " << *V << "\n");
    Optional<unsigned> NewAS = updateAddressSpace(*V, *InferredAddrSpace);
    if (!NewAS.hasValue())
      continue;
    // If any updates are made, grabs its users to the worklist because
    // their address spaces can also be possibly updated.
    DEBUG(dbgs() << "  to " << NewAS.getValue() << "\n");
    (*InferredAddrSpace)[V] = NewAS.getValue();

    for (Value *User : V->users()) {
      // Skip if User is already in the worklist.
      if (Worklist.count(User))
        continue;

      auto Pos = InferredAddrSpace->find(User);
      // Our algorithm only updates the address spaces of generic address
      // expressions, which are those in InferredAddrSpace.
      if (Pos == InferredAddrSpace->end())
        continue;

      // Function updateAddressSpace moves the address space down a lattice
      // path. Therefore, nothing to do if User is already inferred as
      // generic (the bottom element in the lattice).
      if (Pos->second == AddressSpace::ADDRESS_SPACE_GENERIC)
        continue;

      Worklist.insert(User);
    }
  }
}

Optional<unsigned> NVPTXInferAddressSpaces::updateAddressSpace(
    const Value &V, const ValueToAddrSpaceMapTy &InferredAddrSpace) {
  assert(InferredAddrSpace.count(&V));

  // The new inferred address space equals the join of the address spaces
  // of all its pointer operands.
  unsigned NewAS = ADDRESS_SPACE_UNINITIALIZED;
  for (Value *PtrOperand : getPointerOperands(V)) {
    unsigned OperandAS;
    if (InferredAddrSpace.count(PtrOperand))
      OperandAS = InferredAddrSpace.lookup(PtrOperand);
    else
      OperandAS = PtrOperand->getType()->getPointerAddressSpace();
    NewAS = joinAddressSpaces(NewAS, OperandAS);
    // join(generic, *) = generic. So we can break if NewAS is already generic.
    if (NewAS == AddressSpace::ADDRESS_SPACE_GENERIC)
      break;
  }

  unsigned OldAS = InferredAddrSpace.lookup(&V);
  assert(OldAS != AddressSpace::ADDRESS_SPACE_GENERIC);
  if (OldAS == NewAS)
    return None;
  return NewAS;
}

bool NVPTXInferAddressSpaces::rewriteWithNewAddressSpaces(
    const std::vector<Value *> &Postorder,
    const ValueToAddrSpaceMapTy &InferredAddrSpace, Function *F) {
  // For each address expression to be modified, creates a clone of it with its
  // pointer operands converted to the new address space. Since the pointer
  // operands are converted, the clone is naturally in the new address space by
  // construction.
  ValueToValueMapTy ValueWithNewAddrSpace;
  SmallVector<const Use *, 32> UndefUsesToFix;
  for (Value* V : Postorder) {
    unsigned NewAddrSpace = InferredAddrSpace.lookup(V);
    if (V->getType()->getPointerAddressSpace() != NewAddrSpace) {
      ValueWithNewAddrSpace[V] = cloneValueWithNewAddressSpace(
          V, NewAddrSpace, ValueWithNewAddrSpace, &UndefUsesToFix);
    }
  }

  if (ValueWithNewAddrSpace.empty())
    return false;

  // Fixes all the undef uses generated by cloneInstructionWithNewAddressSpace.
  for (const Use* UndefUse : UndefUsesToFix) {
    User *V = UndefUse->getUser();
    User *NewV = cast<User>(ValueWithNewAddrSpace.lookup(V));
    unsigned OperandNo = UndefUse->getOperandNo();
    assert(isa<UndefValue>(NewV->getOperand(OperandNo)));
    NewV->setOperand(OperandNo, ValueWithNewAddrSpace.lookup(UndefUse->get()));
  }

  // Replaces the uses of the old address expressions with the new ones.
  for (Value *V : Postorder) {
    Value *NewV = ValueWithNewAddrSpace.lookup(V);
    if (NewV == nullptr)
      continue;

    SmallVector<Use *, 4> Uses;
    for (Use &U : V->uses())
      Uses.push_back(&U);
    DEBUG(dbgs() << "Replacing the uses of " << *V << "\n  to\n  " << *NewV
                 << "\n");
    for (Use *U : Uses) {
      if (isa<LoadInst>(U->getUser()) ||
          (isa<StoreInst>(U->getUser()) && U->getOperandNo() == 1)) {
        // If V is used as the pointer operand of a load/store, sets the pointer
        // operand to NewV. This replacement does not change the element type,
        // so the resultant load/store is still valid.
        U->set(NewV);
      } else if (isa<Instruction>(U->getUser())) {
        // Otherwise, replaces the use with generic(NewV).
        // TODO: Some optimization opportunities are missed. For example, in
        //   %0 = icmp eq float* %p, %q
        // if both p and q are inferred to be shared, we can rewrite %0 as
        //   %0 = icmp eq float addrspace(3)* %new_p, %new_q
        // instead of currently
        //   %generic_p = addrspacecast float addrspace(3)* %new_p to float*
        //   %generic_q = addrspacecast float addrspace(3)* %new_q to float*
        //   %0 = icmp eq float* %generic_p, %generic_q
        if (Instruction *I = dyn_cast<Instruction>(V)) {
          BasicBlock::iterator InsertPos = std::next(I->getIterator());
          while (isa<PHINode>(InsertPos))
            ++InsertPos;
          U->set(new AddrSpaceCastInst(NewV, V->getType(), "", &*InsertPos));
        } else {
          U->set(ConstantExpr::getAddrSpaceCast(cast<Constant>(NewV),
                                                V->getType()));
        }
      }
    }
    if (V->use_empty())
      RecursivelyDeleteTriviallyDeadInstructions(V);
  }

  return true;
}

ModulePass *llvm::createNVPTXInferAddressSpacesPass() {
  return new NVPTXInferAddressSpaces();
}
