//===---- llvm/Support/TypeBuilder.h - Builder for LLVM types ---*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the TypeBuilder class, which is used as a convenient way to
// create LLVM types with a consistent and simplified interface.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_SUPPORT_TYPEBUILDER_H
#define LLVM_SUPPORT_TYPEBUILDER_H

#include "llvm/DerivedTypes.h"
#include <limits.h>

namespace llvm {

/// TypeBuilder - This provides a uniform API for looking up types
/// known at compile time.  To support cross-compilation, we define a
/// series of tag types in the llvm::types namespace, like i<N>,
/// ieee_float, ppc_fp128, etc.  TypeBuilder<T, false> allows T to be
/// any of these, a native C type (whose size may depend on the host
/// compiler), or a pointer, function, or struct type built out of
/// these.  TypeBuilder<T, true> removes native C types from this set
/// to guarantee that its result is suitable for cross-compilation.
/// We define the primitive types, pointer types, and functions up to
/// 5 arguments here, but to use this class with your own types,
/// you'll need to specialize it.  For example, say you want to call a
/// function defined externally as:
///
///   struct MyType {
///     int32 a;
///     int32 *b;
///     void *array[1];  // Intended as a flexible array.
///   };
///   int8 AFunction(struct MyType *value);
///
/// You'll want to use
///   Function::Create(TypeBuilder<types::i<8>(MyType*), true>::get(), ...)
/// to declare the function, but when you first try this, your compiler will
/// complain that TypeBuilder<MyType, true>::get() doesn't exist. To fix this,
/// write:
///
///   namespace llvm {
///   template<bool xcompile> class TypeBuilder<MyType, xcompile> {
///   public:
///     static const StructType *get() {
///       // Using the static result variable ensures that the type is
///       // only looked up once.
///       static const StructType *const result = StructType::get(
///         TypeBuilder<types::i<32>, xcompile>::get(),
///         TypeBuilder<types::i<32>*, xcompile>::get(),
///         TypeBuilder<types::i<8>*[], xcompile>::get(),
///         NULL);
///       return result;
///     }
///
///     // You may find this a convenient place to put some constants
///     // to help with getelementptr.  They don't have any effect on
///     // the operation of TypeBuilder.
///     enum Fields {
///       FIELD_A,
///       FIELD_B,
///       FIELD_ARRAY
///     };
///   }
///   }  // namespace llvm
///
/// Using the static result variable ensures that the type is only looked up
/// once.
///
/// TypeBuilder cannot handle recursive types or types you only know at runtime.
/// If you try to give it a recursive type, it will deadlock, infinitely
/// recurse, or throw a recursive_init exception.
template<typename T, bool cross_compilable> class TypeBuilder {};

// Types for use with cross-compilable TypeBuilders.  These correspond
// exactly with an LLVM-native type.
namespace types {
/// i<N> corresponds to the LLVM IntegerType with N bits.
template<uint32_t num_bits> class i {};

// The following classes represent the LLVM floating types.
class ieee_float {};
class ieee_double {};
class x86_fp80 {};
class fp128 {};
class ppc_fp128 {};
}  // namespace types

// LLVM doesn't have const or volatile types.
template<typename T, bool cross> class TypeBuilder<const T, cross>
  : public TypeBuilder<T, cross> {};
template<typename T, bool cross> class TypeBuilder<volatile T, cross>
  : public TypeBuilder<T, cross> {};
template<typename T, bool cross> class TypeBuilder<const volatile T, cross>
  : public TypeBuilder<T, cross> {};

// Pointers
template<typename T, bool cross> class TypeBuilder<T*, cross> {
public:
  static const PointerType *get(LLVMContext &Context) {
    static const PointerType *const result =
      Context.getPointerTypeUnqual(TypeBuilder<T,cross>::get(Context));
    return result;
  }
};

/// There is no support for references
template<typename T, bool cross> class TypeBuilder<T&, cross> {};

// Arrays
template<typename T, size_t N, bool cross> class TypeBuilder<T[N], cross> {
public:
  static const ArrayType *get(LLVMContext &Context) {
    static const ArrayType *const result =
      Context.getArrayType(TypeBuilder<T, cross>::get(Context), N);
    return result;
  }
};
/// LLVM uses an array of length 0 to represent an unknown-length array.
template<typename T, bool cross> class TypeBuilder<T[], cross> {
public:
  static const ArrayType *get(LLVMContext &Context) {
    static const ArrayType *const result =
      Context.getArrayType(TypeBuilder<T, cross>::get(Context), 0);
    return result;
  }
};

// Define the C integral types only for TypeBuilder<T, false>.
//
// C integral types do not have a defined size. It would be nice to use the
// stdint.h-defined typedefs that do have defined sizes, but we'd run into the
// following problem:
//
// On an ILP32 machine, stdint.h might define:
//
//   typedef int int32_t;
//   typedef long long int64_t;
//   typedef long size_t;
//
// If we defined TypeBuilder<int32_t> and TypeBuilder<int64_t>, then any use of
// TypeBuilder<size_t> would fail.  We couldn't define TypeBuilder<size_t> in
// addition to the defined-size types because we'd get duplicate definitions on
// platforms where stdint.h instead defines:
//
//   typedef int int32_t;
//   typedef long long int64_t;
//   typedef int size_t;
//
// So we define all the primitive C types and nothing else.
#define DEFINE_INTEGRAL_TYPEBUILDER(T) \
template<> class TypeBuilder<T, false> { \
public: \
  static const IntegerType *get(LLVMContext &Context) { \
    static const IntegerType *const result = \
      Context.getIntegerType(sizeof(T) * CHAR_BIT); \
    return result; \
  } \
}; \
template<> class TypeBuilder<T, true> { \
  /* We provide a definition here so users don't accidentally */ \
  /* define these types to work. */ \
}
DEFINE_INTEGRAL_TYPEBUILDER(char);
DEFINE_INTEGRAL_TYPEBUILDER(signed char);
DEFINE_INTEGRAL_TYPEBUILDER(unsigned char);
DEFINE_INTEGRAL_TYPEBUILDER(short);
DEFINE_INTEGRAL_TYPEBUILDER(unsigned short);
DEFINE_INTEGRAL_TYPEBUILDER(int);
DEFINE_INTEGRAL_TYPEBUILDER(unsigned int);
DEFINE_INTEGRAL_TYPEBUILDER(long);
DEFINE_INTEGRAL_TYPEBUILDER(unsigned long);
#ifdef _MSC_VER
DEFINE_INTEGRAL_TYPEBUILDER(__int64);
DEFINE_INTEGRAL_TYPEBUILDER(unsigned __int64);
#else /* _MSC_VER */
DEFINE_INTEGRAL_TYPEBUILDER(long long);
DEFINE_INTEGRAL_TYPEBUILDER(unsigned long long);
#endif /* _MSC_VER */
#undef DEFINE_INTEGRAL_TYPEBUILDER

template<uint32_t num_bits, bool cross>
class TypeBuilder<types::i<num_bits>, cross> {
public:
  static const IntegerType *get(LLVMContext &Context) {
    static const IntegerType *const result = Context.getIntegerType(num_bits);
    return result;
  }
};

template<> class TypeBuilder<float, false> {
public:
  static const Type *get(LLVMContext&) {
    return Type::FloatTy;
  }
};
template<> class TypeBuilder<float, true> {};

template<> class TypeBuilder<double, false> {
public:
  static const Type *get(LLVMContext&) {
    return Type::DoubleTy;
  }
};
template<> class TypeBuilder<double, true> {};

template<bool cross> class TypeBuilder<types::ieee_float, cross> {
public:
  static const Type *get(LLVMContext&) { return Type::FloatTy; }
};
template<bool cross> class TypeBuilder<types::ieee_double, cross> {
public:
  static const Type *get(LLVMContext&) { return Type::DoubleTy; }
};
template<bool cross> class TypeBuilder<types::x86_fp80, cross> {
public:
  static const Type *get(LLVMContext&) { return Type::X86_FP80Ty; }
};
template<bool cross> class TypeBuilder<types::fp128, cross> {
public:
  static const Type *get(LLVMContext&) { return Type::FP128Ty; }
};
template<bool cross> class TypeBuilder<types::ppc_fp128, cross> {
public:
  static const Type *get(LLVMContext&) { return Type::PPC_FP128Ty; }
};

template<bool cross> class TypeBuilder<void, cross> {
public:
  static const Type *get(LLVMContxt&) {
    return Type::VoidTy;
  }
};

/// void* is disallowed in LLVM types, but it occurs often enough in C code that
/// we special case it.
template<> class TypeBuilder<void*, false>
  : public TypeBuilder<types::i<8>*, false> {};

template<typename R, bool cross> class TypeBuilder<R(), cross> {
public:
  static const FunctionType *get(LLVMContext &Context) {
    static const FunctionType *const result = create(Context);
    return result;
  }

private:
  static const FunctionType *create(LLVMContext &Context) {
    return Context.getFunctionType(TypeBuilder<R, cross>::get(Context), false);
  }
};
template<typename R, typename A1, bool cross> class TypeBuilder<R(A1), cross> {
public:
  static const FunctionType *get(LLVMContext &Context) {
    static const FunctionType *const result = create(Context);
    return result;
  }

private:
  static const FunctionType *create(LLVMContext &Context) {
    std::vector<const Type*> params;
    params.reserve(1);
    params.push_back(TypeBuilder<A1, cross>::get(Context));
    return Context.getFunctionType(TypeBuilder<R, cross>::get(Context),
                             params, false);
  }
};
template<typename R, typename A1, typename A2, bool cross>
class TypeBuilder<R(A1, A2), cross> {
public:
  static const FunctionType *get(LLVMContext &Context) {
    static const FunctionType *const result = create(Context);
    return result;
  }

private:
  static const FunctionType *create(LLVMContext &Context) {
    std::vector<const Type*> params;
    params.reserve(2);
    params.push_back(TypeBuilder<A1, cross>::get(Context));
    params.push_back(TypeBuilder<A2, cross>::get(Context));
    return Context.getFunctionType(TypeBuilder<R, cross>::get(Context),
                             params, false);
  }
};
template<typename R, typename A1, typename A2, typename A3, bool cross>
class TypeBuilder<R(A1, A2, A3), cross> {
public:
  static const FunctionType *get(LLVMContext &Context) {
    static const FunctionType *const result = create(Context);
    return result;
  }

private:
  static const FunctionType *create(LLVMContext &Context) {
    std::vector<const Type*> params;
    params.reserve(3);
    params.push_back(TypeBuilder<A1, cross>::get(Context));
    params.push_back(TypeBuilder<A2, cross>::get(Context));
    params.push_back(TypeBuilder<A3, cross>::get(Context));
    return Context.getFunctionType(TypeBuilder<R, cross>::get(Context),
                             params, false);
  }
};

template<typename R, typename A1, typename A2, typename A3, typename A4,
         bool cross>
class TypeBuilder<R(A1, A2, A3, A4), cross> {
public:
  static const FunctionType *get(LLVMContext &Context) {
    static const FunctionType *const result = create(Context);
    return result;
  }

private:
  static const FunctionType *create(LLVMContext &Context) {
    std::vector<const Type*> params;
    params.reserve(4);
    params.push_back(TypeBuilder<A1, cross>::get(Context));
    params.push_back(TypeBuilder<A2, cross>::get(Context));
    params.push_back(TypeBuilder<A3, cross>::get(Context));
    params.push_back(TypeBuilder<A4, cross>::get(Context));
    return Context.getFunctionType(TypeBuilder<R, cross>::get(Context),
                             params, false);
  }
};

template<typename R, typename A1, typename A2, typename A3, typename A4,
         typename A5, bool cross>
class TypeBuilder<R(A1, A2, A3, A4, A5), cross> {
public:
  static const FunctionType *get(LLVMContext &Context) {
    static const FunctionType *const result = create(Context);
    return result;
  }

private:
  static const FunctionType *create(LLVMContext &Context) {
    std::vector<const Type*> params;
    params.reserve(5);
    params.push_back(TypeBuilder<A1, cross>::get(Context));
    params.push_back(TypeBuilder<A2, cross>::get(Context));
    params.push_back(TypeBuilder<A3, cross>::get(Context));
    params.push_back(TypeBuilder<A4, cross>::get(Context));
    params.push_back(TypeBuilder<A5, cross>::get(Context));
    return Context.getFunctionType(TypeBuilder<R, cross>::get(Context),
                             params, false);
  }
};

template<typename R, bool cross> class TypeBuilder<R(...), cross> {
public:
  static const FunctionType *get(LLVMContext &Context) {
    static const FunctionType *const result = create(Context);
    return result;
  }

private:
  static const FunctionType *create(LLVMContext &Context) {
    return Context.getFunctionType(TypeBuilder<R, cross>::get(Context), true);
  }
};
template<typename R, typename A1, bool cross>
class TypeBuilder<R(A1, ...), cross> {
public:
  static const FunctionType *get(LLVMContext &Context) {
    static const FunctionType *const result = create(Context);
    return result;
  }

private:
  static const FunctionType *create(LLVMContext &Context) {
    std::vector<const Type*> params;
    params.reserve(1);
    params.push_back(TypeBuilder<A1, cross>::get(Context));
    return Context.getFunctionType(TypeBuilder<R, cross>::get(Context),
                                   params, true);
  }
};
template<typename R, typename A1, typename A2, bool cross>
class TypeBuilder<R(A1, A2, ...), cross> {
public:
  static const FunctionType *get(LLVMContext &Context) {
    static const FunctionType *const result = create(Context);
    return result;
  }

private:
  static const FunctionType *create(LLVMContext &Context) {
    std::vector<const Type*> params;
    params.reserve(2);
    params.push_back(TypeBuilder<A1, cross>::get(Context));
    params.push_back(TypeBuilder<A2, cross>::get(Context));
    return Context.getFunctionType(TypeBuilder<R, cross>::get(Context),
                                   params, true);
  }
};
template<typename R, typename A1, typename A2, typename A3, bool cross>
class TypeBuilder<R(A1, A2, A3, ...), cross> {
public:
  static const FunctionType *get(LLVMContext &Context) {
    static const FunctionType *const result = create(Context);
    return result;
  }

private:
  static const FunctionType *create(LLVMContext &Context) {
    std::vector<const Type*> params;
    params.reserve(3);
    params.push_back(TypeBuilder<A1, cross>::get(Context));
    params.push_back(TypeBuilder<A2, cross>::get(Context));
    params.push_back(TypeBuilder<A3, cross>::get(Context));
    return Context.getFunctionType(TypeBuilder<R, cross>::get(Context),
                                   params, true);
  }
};

template<typename R, typename A1, typename A2, typename A3, typename A4,
         bool cross>
class TypeBuilder<R(A1, A2, A3, A4, ...), cross> {
public:
  static const FunctionType *get(LLVMContext &Context) {
    static const FunctionType *const result = create(Context);
    return result;
  }

private:
  static const FunctionType *create(LLVMContext &Context) {
    std::vector<const Type*> params;
    params.reserve(4);
    params.push_back(TypeBuilder<A1, cross>::get(Context));
    params.push_back(TypeBuilder<A2, cross>::get(Context));
    params.push_back(TypeBuilder<A3, cross>::get(Context));
    params.push_back(TypeBuilder<A4, cross>::get(Context));
    return Context.getFunctionType(TypeBuilder<R, cross>::get(Context),
                             params, true);
  }
};

template<typename R, typename A1, typename A2, typename A3, typename A4,
         typename A5, bool cross>
class TypeBuilder<R(A1, A2, A3, A4, A5, ...), cross> {
public:
  static const FunctionType *get(LLVMContext &Context) {
    static const FunctionType *const result = create(Context);
    return result;
  }

private:
  static const FunctionType *create(LLVMContext &Context) {
    std::vector<const Type*> params;
    params.reserve(5);
    params.push_back(TypeBuilder<A1, cross>::get(Context));
    params.push_back(TypeBuilder<A2, cross>::get(Context));
    params.push_back(TypeBuilder<A3, cross>::get(Context));
    params.push_back(TypeBuilder<A4, cross>::get(Context));
    params.push_back(TypeBuilder<A5, cross>::get(Context));
    return Context.getFunctionType(TypeBuilder<R, cross>::get(Context),
                                   params, true);
  }
};

}  // namespace llvm

#endif
