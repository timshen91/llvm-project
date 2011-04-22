//===-- ValueObjectMemory.h -----------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef liblldb_ValueObjectMemory_h_
#define liblldb_ValueObjectMemory_h_

// C Includes
// C++ Includes
// Other libraries and framework includes
// Project includes
#include "lldb/Core/ValueObject.h"

namespace lldb_private {

//----------------------------------------------------------------------
// A ValueObject that represents memory at a given address, viewed as some 
// set lldb type.
//----------------------------------------------------------------------
class ValueObjectMemory : public ValueObject
{
public:
    static lldb::ValueObjectSP
    Create (ExecutionContextScope *exe_scope, 
            const char *name,
            const Address &address, 
            lldb::TypeSP &type_sp);

    virtual
    ~ValueObjectMemory();

    virtual size_t
    GetByteSize();

    virtual clang::ASTContext *
    GetClangAST ();

    virtual lldb::clang_type_t
    GetClangType ();

    virtual ConstString
    GetTypeName();

    virtual uint32_t
    CalculateNumChildren();

    virtual lldb::ValueType
    GetValueType() const;

    virtual bool
    IsInScope ();

protected:
    virtual bool
    UpdateValue ();

    Address  m_address;  ///< The variable that this value object is based upon
    lldb::TypeSP m_type_sp;

private:
    ValueObjectMemory (ExecutionContextScope *exe_scope, 
                       const char *name,
                       const Address &address, 
                       lldb::TypeSP &type_sp);

    //------------------------------------------------------------------
    // For ValueObject only
    //------------------------------------------------------------------
    DISALLOW_COPY_AND_ASSIGN (ValueObjectMemory);
};

} // namespace lldb_private

#endif  // liblldb_ValueObjectMemory_h_
