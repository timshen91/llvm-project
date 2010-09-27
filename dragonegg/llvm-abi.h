/* Processor ABI customization hooks
Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
Contributed by Chris Lattner (sabre@nondot.org)

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.  */

//===----------------------------------------------------------------------===//
// This is a C++ header file that specifies how argument values are passed and
// returned from function calls.  This allows the target to specialize handling
// of things like how structures are passed by-value.
//===----------------------------------------------------------------------===//

#ifndef LLVM_ABI_H
#define LLVM_ABI_H

// Plugin headers
#include "llvm-internal.h"
#include "llvm-target.h"

// LLVM headers
#include "llvm/Attributes.h"
#include "llvm/CallingConv.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/LLVMContext.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Target/TargetData.h"

namespace llvm {
  class BasicBlock;
}

/// DefaultABIClient - This is a simple implementation of the ABI client
/// interface that can be subclassed.
struct DefaultABIClient {
  virtual ~DefaultABIClient() {}
  virtual CallingConv::ID& getCallingConv(void) = 0;
  virtual bool isShadowReturn() const { return false; }

  /// HandleScalarResult - This callback is invoked if the function returns a
  /// simple scalar result value, which is of type RetTy.
  virtual void HandleScalarResult(const Type *RetTy ATTRIBUTE_UNUSED) {}

  /// HandleAggregateResultAsScalar - This callback is invoked if the function
  /// returns an aggregate value by bit converting it to the specified scalar
  /// type and returning that.  The bit conversion should start at byte Offset
  /// within the struct, and ScalarTy is not necessarily big enough to cover
  /// the entire struct.
  virtual void HandleAggregateResultAsScalar(
    const Type *ScalarTy ATTRIBUTE_UNUSED,
    unsigned Offset ATTRIBUTE_UNUSED = 0) {}

  /// HandleAggregateResultAsAggregate - This callback is invoked if the function
  /// returns an aggregate value using multiple return values.
  virtual void HandleAggregateResultAsAggregate(
    const Type *AggrTy ATTRIBUTE_UNUSED) {}

  /// HandleAggregateShadowResult - This callback is invoked if the function
  /// returns an aggregate value by using a "shadow" first parameter, which is
  /// a pointer to the aggregate, of type PtrArgTy.  If RetPtr is set to true,
  /// the pointer argument itself is returned from the function.
  virtual void HandleAggregateShadowResult(
    const PointerType *PtrArgTy ATTRIBUTE_UNUSED,
    bool RetPtr ATTRIBUTE_UNUSED) {}

  /// HandleScalarShadowResult - This callback is invoked if the function
  /// returns a scalar value by using a "shadow" first parameter, which is a
  /// pointer to the scalar, of type PtrArgTy.  If RetPtr is set to true,
  /// the pointer argument itself is returned from the function.
  virtual void HandleScalarShadowResult(
    const PointerType *PtrArgTy ATTRIBUTE_UNUSED,
    bool RetPtr ATTRIBUTE_UNUSED) {}


  /// HandleScalarArgument - This is the primary callback that specifies an
  /// LLVM argument to pass.  It is only used for first class types.
  /// If RealSize is non Zero then it specifies number of bytes to access
  /// from LLVMTy.
  virtual void HandleScalarArgument(const llvm::Type *LLVMTy ATTRIBUTE_UNUSED,
                                    tree_node *type ATTRIBUTE_UNUSED,
                                    unsigned RealSize ATTRIBUTE_UNUSED = 0) {}

  /// HandleByInvisibleReferenceArgument - This callback is invoked if a pointer
  /// (of type PtrTy) to the argument is passed rather than the argument itself.
  virtual void HandleByInvisibleReferenceArgument(
    const llvm::Type *PtrTy ATTRIBUTE_UNUSED,
    tree_node *type ATTRIBUTE_UNUSED) {}

  /// HandleByValArgument - This callback is invoked if the aggregate function
  /// argument is passed by value.
  virtual void HandleByValArgument(const llvm::Type *LLVMTy ATTRIBUTE_UNUSED,
                                   tree_node *type ATTRIBUTE_UNUSED) {}

  /// HandleFCAArgument - This callback is invoked if the aggregate function
  /// argument is passed by value as a first class aggregate.
  virtual void HandleFCAArgument(const llvm::Type *LLVMTy ATTRIBUTE_UNUSED,
                                 tree_node *type ATTRIBUTE_UNUSED) {}

  /// EnterField - Called when we're about the enter the field of a struct
  /// or union.  FieldNo is the number of the element we are entering in the
  /// LLVM Struct, StructTy is the LLVM type of the struct we are entering.
  virtual void EnterField(unsigned FieldNo ATTRIBUTE_UNUSED,
                          const llvm::Type *StructTy ATTRIBUTE_UNUSED) {}
  virtual void ExitField() {}
  virtual void HandlePad(const llvm::Type *LLVMTy ATTRIBUTE_UNUSED) {}
};

// LLVM_SHOULD_NOT_RETURN_COMPLEX_IN_MEMORY - A hook to allow
// special _Complex handling. Return true if X should be returned using
// multiple value return instruction.
#ifndef LLVM_SHOULD_NOT_RETURN_COMPLEX_IN_MEMORY
#define LLVM_SHOULD_NOT_RETURN_COMPLEX_IN_MEMORY(X) \
 false
#endif

// LLVM_SHOULD_NOT_USE_SHADOW_RETURN - A hook to allow aggregates to be
// returned in registers.
#ifndef LLVM_SHOULD_NOT_USE_SHADOW_RETURN
#define LLVM_SHOULD_NOT_USE_SHADOW_RETURN(X, CC) \
 false
#endif

// doNotUseShadowReturn - Return true if the specified GCC type
// should not be returned using a pointer to struct parameter.
extern bool doNotUseShadowReturn(tree_node *type, tree_node *fndecl,
                                 CallingConv::ID CC);

/// isSingleElementStructOrArray - If this is (recursively) a structure with one
/// field or an array with one element, return the field type, otherwise return
/// null.  If ignoreZeroLength, the struct (recursively) may include zero-length
/// fields in addition to the single element that has data.  If
/// rejectFatBitField, and the single element is a bitfield of a type that's
/// bigger than the struct, return null anyway.
extern tree_node *isSingleElementStructOrArray(tree_node *type,
                                               bool ignoreZeroLength,
                                               bool rejectFatBitfield);

/// isZeroSizedStructOrUnion - Returns true if this is a struct or union
/// which is zero bits wide.
extern bool isZeroSizedStructOrUnion(tree_node *type);

// getLLVMScalarTypeForStructReturn - Return LLVM Type if TY can be
// returned as a scalar, otherwise return NULL. This is the default
// target independent implementation.
static inline
const Type* getLLVMScalarTypeForStructReturn(tree_node *type, unsigned *Offset) {
  const Type *Ty = ConvertType(type);
  unsigned Size = getTargetData().getTypeAllocSize(Ty);
  *Offset = 0;
  if (Size == 0)
    return Type::getVoidTy(getGlobalContext());
  else if (Size == 1)
    return Type::getInt8Ty(getGlobalContext());
  else if (Size == 2)
    return Type::getInt16Ty(getGlobalContext());
  else if (Size <= 4)
    return Type::getInt32Ty(getGlobalContext());
  else if (Size <= 8)
    return Type::getInt64Ty(getGlobalContext());
  else if (Size <= 16)
    return IntegerType::get(getGlobalContext(), 128);
  else if (Size <= 32)
    return IntegerType::get(getGlobalContext(), 256);

  return NULL;
}

// getLLVMAggregateTypeForStructReturn - Return LLVM type if TY can be
// returns as multiple values, otherwise return NULL. This is the default
// target independent implementation.
static inline const Type* getLLVMAggregateTypeForStructReturn(
  tree_node *type ATTRIBUTE_UNUSED) {
  return NULL;
}

#ifndef LLVM_TRY_PASS_AGGREGATE_CUSTOM
#define LLVM_TRY_PASS_AGGREGATE_CUSTOM(T, E, CC, C) \
  false
#endif

// LLVM_SHOULD_PASS_VECTOR_IN_INTEGER_REGS - Return true if this vector
// type should be passed as integer registers.  Generally vectors which are
// not part of the target architecture should do this.
#ifndef LLVM_SHOULD_PASS_VECTOR_IN_INTEGER_REGS
#define LLVM_SHOULD_PASS_VECTOR_IN_INTEGER_REGS(TY) \
  false
#endif

// LLVM_SHOULD_PASS_VECTOR_USING_BYVAL_ATTR - Return true if this vector
// type should be passed byval.  Used for generic vectors on x86-64.
#ifndef LLVM_SHOULD_PASS_VECTOR_USING_BYVAL_ATTR
#define LLVM_SHOULD_PASS_VECTOR_USING_BYVAL_ATTR(X) \
  false
#endif

// LLVM_SHOULD_PASS_AGGREGATE_USING_BYVAL_ATTR - Return true if this aggregate
// value should be passed by value, i.e. passing its address with the byval
// attribute bit set. The default is false.
#ifndef LLVM_SHOULD_PASS_AGGREGATE_USING_BYVAL_ATTR
#define LLVM_SHOULD_PASS_AGGREGATE_USING_BYVAL_ATTR(X, TY) \
    false
#endif

// LLVM_SHOULD_PASS_AGGREGATE_AS_FCA - Return true if this aggregate value
// should be passed by value as a first class aggregate. The default is false.
#ifndef LLVM_SHOULD_PASS_AGGREGATE_AS_FCA
#define LLVM_SHOULD_PASS_AGGREGATE_AS_FCA(X, TY) \
    false
#endif

// LLVM_SHOULD_PASS_AGGREGATE_IN_MIXED_REGS - Return true if this aggregate
// value should be passed in a mixture of integer, floating point, and vector
// registers. The routine should also return by reference a vector of the
// types of the registers being used. The default is false.
#ifndef LLVM_SHOULD_PASS_AGGREGATE_IN_MIXED_REGS
#define LLVM_SHOULD_PASS_AGGREGATE_IN_MIXED_REGS(T, TY, CC, E) \
    false
#endif

// LLVM_AGGREGATE_PARTIALLY_PASSED_IN_REGS - Only called if
// LLVM_SHOULD_PASS_AGGREGATE_IN_MIXED_REGS returns true. This returns true if
// there are only enough unused argument passing registers to pass a part of
// the aggregate. Note, this routine should return false if none of the needed
// registers are available.
#ifndef LLVM_AGGREGATE_PARTIALLY_PASSED_IN_REGS
#define LLVM_AGGREGATE_PARTIALLY_PASSED_IN_REGS(E, SE, ISR, CC) \
    false
#endif

// LLVM_BYVAL_ALIGNMENT - Returns the alignment of the type in bytes, if known,
// in the getGlobalContext() of its use as a function parameter.
// Note that the alignment in the TYPE node is usually the alignment appropriate
// when the type is used within a struct, which may or may not be appropriate
// here.
#ifndef LLVM_BYVAL_ALIGNMENT
#define LLVM_BYVAL_ALIGNMENT(T)  0
#endif

// LLVM_SHOULD_PASS_AGGREGATE_IN_INTEGER_REGS - Return true if this aggregate
// value should be passed in integer registers.  By default, we do this for all
// values that are not single-element structs.  This ensures that things like
// {short,short} are passed in one 32-bit chunk, not as two arguments (which
// would often be 64-bits).  We also do it for single-element structs when the
// single element is a bitfield of a type bigger than the struct; the code
// for field-by-field struct passing does not handle this one right.
#ifndef LLVM_SHOULD_PASS_AGGREGATE_IN_INTEGER_REGS
#define LLVM_SHOULD_PASS_AGGREGATE_IN_INTEGER_REGS(X, Y, Z) \
   !isSingleElementStructOrArray((X), false, true)
#endif

// LLVM_SHOULD_RETURN_SELT_STRUCT_AS_SCALAR - Return a TYPE tree if this single
// element struct should be returned using the convention for that scalar TYPE,
// 0 otherwise.
// The returned TYPE must be the same size as X for this to work; that is
// checked elsewhere.  (Structs where this is not the case can be constructed
// by abusing the __aligned__ attribute.)
#ifndef LLVM_SHOULD_RETURN_SELT_STRUCT_AS_SCALAR
#define LLVM_SHOULD_RETURN_SELT_STRUCT_AS_SCALAR(X) \
  isSingleElementStructOrArray(X, false, false)
#endif

// LLVM_SHOULD_RETURN_VECTOR_AS_SCALAR - Return a TYPE tree if this vector type
// should be returned using the convention for that scalar TYPE, 0 otherwise.
// X may be evaluated more than once.
#ifndef LLVM_SHOULD_RETURN_VECTOR_AS_SCALAR
#define LLVM_SHOULD_RETURN_VECTOR_AS_SCALAR(X,Y) 0
#endif

// LLVM_SHOULD_RETURN_VECTOR_AS_SHADOW - Return true if this vector type
// should be returned using the aggregate shadow (sret) convention, 0 otherwise.
// X may be evaluated more than once.
#ifndef LLVM_SHOULD_RETURN_VECTOR_AS_SHADOW
#define LLVM_SHOULD_RETURN_VECTOR_AS_SHADOW(X,Y) 0
#endif

// LLVM_SCALAR_TYPE_FOR_STRUCT_RETURN - Return LLVM Type if X can be
// returned as a scalar, otherwise return NULL.
#ifndef LLVM_SCALAR_TYPE_FOR_STRUCT_RETURN
#define LLVM_SCALAR_TYPE_FOR_STRUCT_RETURN(X, Y) \
  getLLVMScalarTypeForStructReturn((X), (Y))
#endif

// LLVM_AGGR_TYPE_FOR_STRUCT_RETURN - Return LLVM Type if X can be
// returned as an aggregate, otherwise return NULL.
#ifndef LLVM_AGGR_TYPE_FOR_STRUCT_RETURN
#define LLVM_AGGR_TYPE_FOR_STRUCT_RETURN(X, CC)    \
  getLLVMAggregateTypeForStructReturn(X)
#endif

// LLVM_EXTRACT_MULTIPLE_RETURN_VALUE - Extract multiple return value from
// SRC and assign it to DEST. Each target that supports multiple return
// value must implement this hook.
#ifndef LLVM_EXTRACT_MULTIPLE_RETURN_VALUE
#define LLVM_EXTRACT_MULTIPLE_RETURN_VALUE(Src,Dest,V,B)     \
  llvm_default_extract_multiple_return_value((Src),(Dest),(V),(B))
#endif
static inline
void llvm_default_extract_multiple_return_value(
  Value *Src ATTRIBUTE_UNUSED, Value *Dest ATTRIBUTE_UNUSED,
  bool isVolatile ATTRIBUTE_UNUSED, LLVMBuilder &Builder ATTRIBUTE_UNUSED) {
  assert (0 && "LLVM_EXTRACT_MULTIPLE_RETURN_VALUE is not implemented!");
}

/// DefaultABI - This class implements the default LLVM ABI where structures are
/// passed by decimating them into individual components and unions are passed
/// by passing the largest member of the union.
///
class DefaultABI {
protected:
  DefaultABIClient &C;
public:
  DefaultABI(DefaultABIClient &c);

  bool isShadowReturn() const;

  /// HandleReturnType - This is invoked by the target-independent code for the
  /// return type. It potentially breaks down the argument and invokes methods
  /// on the client that indicate how its pieces should be handled.  This
  /// handles things like returning structures via hidden parameters.
  void HandleReturnType(tree_node *type, tree_node *fn, bool isBuiltin);

  /// HandleArgument - This is invoked by the target-independent code for each
  /// argument type passed into the function.  It potentially breaks down the
  /// argument and invokes methods on the client that indicate how its pieces
  /// should be handled.  This handles things like decimating structures into
  /// their fields.
  void HandleArgument(tree_node *type, std::vector<const Type*> &ScalarElts,
                      Attributes *Attributes = NULL);

  /// HandleUnion - Handle a UNION_TYPE or QUAL_UNION_TYPE tree.
  ///
  void HandleUnion(tree_node *type, std::vector<const Type*> &ScalarElts);

  /// PassInIntegerRegisters - Given an aggregate value that should be passed in
  /// integer registers, convert it to a structure containing ints and pass all
  /// of the struct elements in.  If Size is set we pass only that many bytes.
  void PassInIntegerRegisters(tree_node *type,
                              std::vector<const Type*> &ScalarElts,
                              unsigned origSize, bool DontCheckAlignment);

  /// PassInMixedRegisters - Given an aggregate value that should be passed in
  /// mixed integer, floating point, and vector registers, convert it to a
  /// structure containing the specified struct elements in.
  void PassInMixedRegisters(const Type *Ty, std::vector<const Type*> &OrigElts,
                            std::vector<const Type*> &ScalarElts);
};

#endif /* LLVM_ABI_H */
