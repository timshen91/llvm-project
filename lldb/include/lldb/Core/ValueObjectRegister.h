//===-- ValueObjectRegister.h -----------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef liblldb_ValueObjectRegister_h_
#define liblldb_ValueObjectRegister_h_

// C Includes
// C++ Includes
// Other libraries and framework includes
// Project includes
#include "lldb/lldb-private.h"
#include "lldb/Core/ValueObject.h"

namespace lldb_private {

//----------------------------------------------------------------------
// A ValueObject that contains a root variable that may or may not
// have children.
//----------------------------------------------------------------------
class ValueObjectRegisterContext : public ValueObject
{
public:
    ValueObjectRegisterContext (ValueObject &parent, lldb::RegisterContextSP &reg_ctx_sp);

    virtual
    ~ValueObjectRegisterContext();

    virtual size_t
    GetByteSize();

    virtual clang::ASTContext *
    GetClangAST ();

    virtual lldb::clang_type_t
    GetClangType ();

    virtual lldb::ValueType
    GetValueType () const
    {
        return lldb::eValueTypeRegisterSet;
    }

    virtual ConstString
    GetTypeName();

    virtual uint32_t
    CalculateNumChildren();

    virtual lldb::ValueObjectSP
    CreateChildAtIndex (uint32_t idx, bool synthetic_array_member, int32_t synthetic_index);

protected:
    virtual bool
    UpdateValue ();

    lldb::RegisterContextSP m_reg_ctx_sp;

private:
    //------------------------------------------------------------------
    // For ValueObject only
    //------------------------------------------------------------------
    DISALLOW_COPY_AND_ASSIGN (ValueObjectRegisterContext);
};

class ValueObjectRegisterSet : public ValueObject
{
public:
    ValueObjectRegisterSet (ExecutionContextScope *exe_scope, lldb::RegisterContextSP &reg_ctx_sp, uint32_t set_idx);

    virtual
    ~ValueObjectRegisterSet();

    virtual size_t
    GetByteSize();

    virtual clang::ASTContext *
    GetClangAST ();

    virtual lldb::clang_type_t
    GetClangType ();

    virtual lldb::ValueType
    GetValueType () const
    {
        return lldb::eValueTypeRegisterSet;
    }

    virtual ConstString
    GetTypeName();

    virtual uint32_t
    CalculateNumChildren();

    virtual lldb::ValueObjectSP
    CreateChildAtIndex (uint32_t idx, bool synthetic_array_member, int32_t synthetic_index);
    
    virtual lldb::ValueObjectSP
    GetChildMemberWithName (const ConstString &name, bool can_create);

    virtual uint32_t
    GetIndexOfChildWithName (const ConstString &name);


protected:
    virtual bool
    UpdateValue ();

    lldb::RegisterContextSP m_reg_ctx_sp;
    const RegisterSet *m_reg_set;
    uint32_t m_reg_set_idx;

private:
    //------------------------------------------------------------------
    // For ValueObject only
    //------------------------------------------------------------------
    DISALLOW_COPY_AND_ASSIGN (ValueObjectRegisterSet);
};

class ValueObjectRegister : public ValueObject
{
public:
    ValueObjectRegister (ValueObject &parent, lldb::RegisterContextSP &reg_ctx_sp, uint32_t reg_num);
    ValueObjectRegister (ExecutionContextScope *exe_scope, lldb::RegisterContextSP &reg_ctx_sp, uint32_t reg_num);

    virtual
    ~ValueObjectRegister();

    virtual size_t
    GetByteSize();

    virtual clang::ASTContext *
    GetClangAST ();

    virtual lldb::clang_type_t
    GetClangType ();

    virtual lldb::ValueType
    GetValueType () const
    {
        return lldb::eValueTypeRegister;
    }

    virtual ConstString
    GetTypeName();

    virtual uint32_t
    CalculateNumChildren();

protected:
    virtual bool
    UpdateValue ();

    lldb::RegisterContextSP m_reg_ctx_sp;
    const RegisterInfo *m_reg_info;
    uint32_t m_reg_num;
    ConstString m_type_name;
    void *m_clang_type;

private:
    void
    ConstructObject ();
    //------------------------------------------------------------------
    // For ValueObject only
    //------------------------------------------------------------------
    DISALLOW_COPY_AND_ASSIGN (ValueObjectRegister);
};

} // namespace lldb_private

#endif  // liblldb_ValueObjectRegister_h_
