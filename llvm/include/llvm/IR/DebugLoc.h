//===- DebugLoc.h - Debug Location Information ------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines a number of light weight data structures used
// to describe and track debug location information.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_DEBUGLOC_H
#define LLVM_IR_DEBUGLOC_H

#include "llvm/IR/TrackingMDRef.h"
#include "llvm/Support/DataTypes.h"

namespace llvm {

  class LLVMContext;
  class raw_ostream;
  class MDLocation;

  /// \brief A debug info location.
  ///
  /// This class is a wrapper around a tracking reference to an \a MDLocation
  /// pointer.
  ///
  /// To avoid extra includes, \a DebugLoc doubles the \a MDLocation API with a
  /// one based on relatively opaque \a MDNode pointers.
  class DebugLoc {
    TrackingMDNodeRef Loc;

  public:
    DebugLoc() {}
    DebugLoc(DebugLoc &&X) : Loc(std::move(X.Loc)) {}
    DebugLoc(const DebugLoc &X) : Loc(X.Loc) {}
    DebugLoc &operator=(DebugLoc &&X) {
      Loc = std::move(X.Loc);
      return *this;
    }
    DebugLoc &operator=(const DebugLoc &X) {
      Loc = X.Loc;
      return *this;
    }

    /// \brief Construct from an \a MDLocation.
    DebugLoc(MDLocation *L);

    /// \brief Construct from an \a MDNode.
    ///
    /// Note: if \c N is not an \a MDLocation, a verifier check will fail, and
    /// accessors will crash.  However, construction from other nodes is
    /// supported in order to handle forward references when reading textual
    /// IR.
    explicit DebugLoc(MDNode *N);

    MDLocation *get() const;
    operator MDLocation *() const { return get(); }
    MDLocation *operator->() const { return get(); }
    MDLocation &operator*() const { return *get(); }

    /// \brief Check whether this has a trivial destructor.
    bool hasTrivialDestructor() const { return Loc.hasTrivialDestructor(); }

    /// \brief Create a new DebugLoc.
    ///
    /// Create a new DebugLoc at the specified line/col and scope/inline.  This
    /// forwards to \a MDLocation::get().
    ///
    /// If \c !Scope, returns a default-constructed \a DebugLoc.
    ///
    /// FIXME: Remove this.  Users should use MDLocation::get().
    static DebugLoc get(unsigned Line, unsigned Col, MDNode *Scope,
                        MDNode *InlinedAt = nullptr);

    /// \brief Translate the DILexicalBlock into a DebugLoc.
    ///
    /// FIXME: Remove this.  It has only one user, and the user just wants to
    /// pass an \a MDScope around.
    static DebugLoc getFromDILexicalBlock(MDNode *N);

    unsigned getLine() const;
    unsigned getCol() const;
    MDNode *getScope() const;
    MDLocation *getInlinedAt() const;

    /// \brief Get the fully inlined-at scope for a DebugLoc.
    ///
    /// Gets the inlined-at scope for a DebugLoc.
    MDNode *getInlinedAtScope() const;

    /// \brief Find the debug info location for the start of the function.
    ///
    /// Walk up the scope chain of given debug loc and find line number info
    /// for the function.
    ///
    /// FIXME: Remove this.  Users should use MDLocation/MDLocalScope API to
    /// find the subprogram, and then MDLocation::get().
    DebugLoc getFnDebugLoc() const;

    /// \brief Return \c this as a bar \a MDNode.
    MDNode *getAsMDNode() const { return Loc; }

    bool operator==(const DebugLoc &DL) const { return Loc == DL.Loc; }
    bool operator!=(const DebugLoc &DL) const { return Loc != DL.Loc; }

    void dump() const;

    /// \brief prints source location /path/to/file.exe:line:col @[inlined at]
    void print(raw_ostream &OS) const;

    // FIXME: Remove this old API once callers have been updated.
    static DebugLoc getFromDILocation(MDNode *N) { return DebugLoc(N); }
    bool isUnknown() const { return !Loc; }
    MDNode *getScope(const LLVMContext &) const { return getScope(); }
    MDNode *getInlinedAt(const LLVMContext &) const;
    void getScopeAndInlinedAt(MDNode *&Scope, MDNode *&IA) const;
    void getScopeAndInlinedAt(MDNode *&Scope, MDNode *&IA,
                              const LLVMContext &) const {
      return getScopeAndInlinedAt(Scope, IA);
    }
    MDNode *getScopeNode() const { return getInlinedAtScope(); }
    MDNode *getScopeNode(const LLVMContext &) const { return getScopeNode(); }
    DebugLoc getFnDebugLoc(const LLVMContext &) const {
      return getFnDebugLoc();
    }
    MDNode *getAsMDNode(LLVMContext &) const { return getAsMDNode(); }
    void dump(const LLVMContext &) const { dump(); }
  };

} // end namespace llvm

#endif /* LLVM_SUPPORT_DEBUGLOC_H */
