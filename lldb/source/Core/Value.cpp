//===-- Value.cpp -----------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "lldb/Core/Value.h"

// C Includes
// C++ Includes
// Other libraries and framework includes
// Project includes
#include "lldb/Core/DataExtractor.h"
#include "lldb/Core/DataBufferHeap.h"
#include "lldb/Core/Module.h"
#include "lldb/Core/Stream.h"
#include "lldb/Symbol/ClangASTType.h"
#include "lldb/Symbol/ClangASTContext.h"
#include "lldb/Symbol/ObjectFile.h"
#include "lldb/Symbol/SymbolContext.h"
#include "lldb/Symbol/Type.h"
#include "lldb/Symbol/Variable.h"
#include "lldb/Target/ExecutionContext.h"
#include "lldb/Target/Process.h"
#include "lldb/Target/Target.h"

using namespace lldb;
using namespace lldb_private;

Value::Value() :
    m_value (),
    m_value_type (eValueTypeScalar),
    m_context (NULL),
    m_context_type (eContextTypeInvalid),
    m_data_buffer ()
{
}

Value::Value(const Scalar& scalar) :
    m_value (scalar),
    m_value_type (eValueTypeScalar),
    m_context (NULL),
    m_context_type (eContextTypeInvalid),
    m_data_buffer ()
{
}


Value::Value(const uint8_t *bytes, int len) :
    m_value (),
    m_value_type (eValueTypeHostAddress),
    m_context (NULL),
    m_context_type (eContextTypeInvalid),
    m_data_buffer ()
{
    m_data_buffer.CopyData(bytes, len);
    m_value = (uintptr_t)m_data_buffer.GetBytes();
}

Value::Value(const Value &v) :
    m_value(v.m_value),
    m_value_type(v.m_value_type),
    m_context(v.m_context),
    m_context_type(v.m_context_type)
{
    if ((uintptr_t)v.m_value.ULongLong(LLDB_INVALID_ADDRESS) == (uintptr_t)v.m_data_buffer.GetBytes())
    {
        m_data_buffer.CopyData(v.m_data_buffer.GetBytes(),
                               v.m_data_buffer.GetByteSize());
    
        m_value = (uintptr_t)m_data_buffer.GetBytes();
    }
}

Value &
Value::operator=(const Value &rhs)
{
    if (this != &rhs)
    {
        m_value = rhs.m_value;
        m_value_type = rhs.m_value_type;
        m_context = rhs.m_context;
        m_context_type = rhs.m_context_type;
        if ((uintptr_t)rhs.m_value.ULongLong(LLDB_INVALID_ADDRESS) == (uintptr_t)rhs.m_data_buffer.GetBytes())
        {
            m_data_buffer.CopyData(rhs.m_data_buffer.GetBytes(),
                                   rhs.m_data_buffer.GetByteSize());
        
            m_value = (uintptr_t)m_data_buffer.GetBytes();
        }
    }
    return *this;
}

void
Value::Dump (Stream* strm)
{
    m_value.GetValue (strm, true);
    strm->Printf(", value_type = %s, context = %p, context_type = %s",
                Value::GetValueTypeAsCString(m_value_type),
                m_context,
                Value::GetContextTypeAsCString(m_context_type));
}

Value::ValueType
Value::GetValueType() const
{
    return m_value_type;
}

AddressType
Value::GetValueAddressType () const
{
    switch (m_value_type)
    {
    default:
    case eValueTypeScalar:
        break;
    case eValueTypeLoadAddress: return eAddressTypeLoad;
    case eValueTypeFileAddress: return eAddressTypeFile;
    case eValueTypeHostAddress: return eAddressTypeHost;
    }
    return eAddressTypeInvalid;
}

RegisterInfo *
Value::GetRegisterInfo()
{
    if (m_context_type == eContextTypeRegisterInfo)
        return static_cast<RegisterInfo *> (m_context);
    return NULL;
}

Type *
Value::GetType()
{
    if (m_context_type == eContextTypeLLDBType)
        return static_cast<Type *> (m_context);
    return NULL;
}

void
Value::ResizeData(int len)
{
    m_value_type = eValueTypeHostAddress;
    m_data_buffer.SetByteSize(len);
    m_value = (uintptr_t)m_data_buffer.GetBytes();
}

bool
Value::ValueOf(ExecutionContext *exe_ctx, clang::ASTContext *ast_context)
{
    switch (m_context_type)
    {
    default:
    case eContextTypeInvalid:
    case eContextTypeClangType:         // clang::Type *
    case eContextTypeRegisterInfo:      // RegisterInfo *
    case eContextTypeLLDBType:          // Type *
        break;

    case eContextTypeVariable:          // Variable *
        ResolveValue(exe_ctx, ast_context);
        return true;
    }
    return false;
}

size_t
Value::GetValueByteSize (clang::ASTContext *ast_context, Error *error_ptr)
{
    size_t byte_size = 0;

    switch (m_context_type)
    {
    default:
    case eContextTypeInvalid:
        // If we have no context, there is no way to know how much memory to read
        if (error_ptr)
            error_ptr->SetErrorString ("Invalid context type, there is no way to know how much memory to read.");
        break;

    case eContextTypeClangType:
        if (ast_context == NULL)
        {
            if (error_ptr)
                error_ptr->SetErrorString ("Can't determine size of opaque clang type with NULL ASTContext *.");
        }
        else
        {
            uint64_t bit_width = ClangASTType::GetClangTypeBitWidth (ast_context, m_context);
            byte_size = (bit_width + 7 ) / 8;
        }
        break;

    case eContextTypeRegisterInfo:     // RegisterInfo *
        if (GetRegisterInfo())
            byte_size = GetRegisterInfo()->byte_size;
        else if (error_ptr)
                error_ptr->SetErrorString ("Can't determine byte size with NULL RegisterInfo *.");

        break;

    case eContextTypeLLDBType:             // Type *
        if (GetType())
            byte_size = GetType()->GetByteSize();
        else if (error_ptr)
            error_ptr->SetErrorString ("Can't determine byte size with NULL Type *.");
        break;

    case eContextTypeVariable:         // Variable *
        if (GetVariable())
            byte_size = GetVariable()->GetType()->GetByteSize();
        else if (error_ptr)
            error_ptr->SetErrorString ("Can't determine byte size with NULL Variable *.");
        break;
    }

    if (error_ptr)
    {
        if (byte_size == 0)
        {
            if (error_ptr->Success())
                error_ptr->SetErrorString("Unable to determine byte size.");
        }
        else
        {
            error_ptr->Clear();
        }
    }
    return byte_size;
}

clang_type_t
Value::GetClangType ()
{
    switch (m_context_type)
    {
    default:
    case eContextTypeInvalid:
        break;

    case eContextTypeClangType:
        return m_context;

    case eContextTypeRegisterInfo:
        break;    // TODO: Eventually convert into a clang type?

    case eContextTypeLLDBType:
        if (GetType())
            return GetType()->GetClangForwardType();
        break;

    case eContextTypeVariable:
        if (GetVariable())
            return GetVariable()->GetType()->GetClangForwardType();
        break;
    }

    return NULL;
}

lldb::Format
Value::GetValueDefaultFormat ()
{
    switch (m_context_type)
    {
    default:
    case eContextTypeInvalid:
        break;

    case eContextTypeClangType:
        return ClangASTType::GetFormat (m_context);

    case eContextTypeRegisterInfo:
        if (GetRegisterInfo())
            return GetRegisterInfo()->format;
        break;

    case eContextTypeLLDBType:
        if (GetType())
            return GetType()->GetFormat();
        break;

    case eContextTypeVariable:
        if (GetVariable())
            return GetVariable()->GetType()->GetFormat();
        break;

    }

    // Return a good default in case we can't figure anything out
    return eFormatHex;
}

bool
Value::GetData (DataExtractor &data)
{
    switch (m_value_type)
    {
    default:
        break;

    case eValueTypeScalar:
        if (m_value.GetData (data))
            return true;
        break;

    case eValueTypeLoadAddress:
    case eValueTypeFileAddress:
    case eValueTypeHostAddress:
        if (m_data_buffer.GetByteSize())
        {
            data.SetData(m_data_buffer.GetBytes(), m_data_buffer.GetByteSize(), data.GetByteOrder());
            return true;
        }
        break;
    }

    return false;

}

Error
Value::GetValueAsData (ExecutionContext *exe_ctx, 
                       clang::ASTContext *ast_context, 
                       DataExtractor &data, 
                       uint32_t data_offset,
                       Module *module)
{
    data.Clear();

    Error error;
    lldb::addr_t address = LLDB_INVALID_ADDRESS;
    AddressType address_type = eAddressTypeFile;
    Address file_so_addr;
    switch (m_value_type)
    {
    default:
        error.SetErrorStringWithFormat("invalid value type %i", m_value_type);
        break;

    case eValueTypeScalar:
        data.SetByteOrder (lldb::endian::InlHostByteOrder());
        if (m_context_type == eContextTypeClangType && ast_context)
        {
            uint32_t ptr_bit_width = ClangASTType::GetClangTypeBitWidth (ast_context, 
                                                                     ClangASTContext::GetVoidPtrType(ast_context, false));
            uint32_t ptr_byte_size = (ptr_bit_width + 7) / 8;
            data.SetAddressByteSize (ptr_byte_size);
        }
        else
            data.SetAddressByteSize(sizeof(void *));
        if (m_value.GetData (data))
            return error;   // Success;
        error.SetErrorStringWithFormat("extracting data from value failed");
        break;

    case eValueTypeLoadAddress:
        if (exe_ctx == NULL)
        {
            error.SetErrorString ("can't read load address (no execution context)");
        }
        else 
        {
            Process *process = exe_ctx->GetProcess();
            if (process == NULL)
            {
                error.SetErrorString ("can't read load address (invalid process)");
            }
            else
            {
                address = m_value.ULongLong(LLDB_INVALID_ADDRESS);
                address_type = eAddressTypeLoad;
                data.SetByteOrder(process->GetTarget().GetArchitecture().GetByteOrder());
                data.SetAddressByteSize(process->GetTarget().GetArchitecture().GetAddressByteSize());
            }
        }
        break;

    case eValueTypeFileAddress:
        if (exe_ctx == NULL)
        {
            error.SetErrorString ("can't read file address (no execution context)");
        }
        else if (exe_ctx->target == NULL)
        {
            error.SetErrorString ("can't read file address (invalid target)");
        }
        else
        {
            address = m_value.ULongLong(LLDB_INVALID_ADDRESS);
            if (address == LLDB_INVALID_ADDRESS)
            {
                error.SetErrorString ("invalid file address");
            }
            else
            {
                if (module == NULL)
                {
                    // The only thing we can currently lock down to a module so that
                    // we can resolve a file address, is a variable.
                    Variable *variable = GetVariable();
                    if (variable)
                    {
                        SymbolContext var_sc;
                        variable->CalculateSymbolContext(&var_sc);
                        module = var_sc.module_sp.get();
                    }
                }
                
                if (module)
                {
                    bool resolved = false;
                    ObjectFile *objfile = module->GetObjectFile();
                    if (objfile)
                    {
                        Address so_addr(address, objfile->GetSectionList());
                        addr_t load_address = so_addr.GetLoadAddress (exe_ctx->target);
                        if (load_address != LLDB_INVALID_ADDRESS)
                        {
                            resolved = true;
                            address = load_address;
                            address_type = eAddressTypeLoad;
                            data.SetByteOrder(exe_ctx->target->GetArchitecture().GetByteOrder());
                            data.SetAddressByteSize(exe_ctx->target->GetArchitecture().GetAddressByteSize());
                        }
                        else
                        {
                            if (so_addr.IsSectionOffset())
                            {
                                resolved = true;
                                file_so_addr = so_addr;
                                data.SetByteOrder(objfile->GetByteOrder());
                                data.SetAddressByteSize(objfile->GetAddressByteSize());
                            }
                        }
                    }
                    if (!resolved)
                    {
                        Variable *variable = GetVariable();
                        
                        if (module)
                        {
                            if (variable)
                                error.SetErrorStringWithFormat ("unable to resolve the module for file address 0x%llx for variable '%s' in %s%s%s", 
                                                                address, 
                                                                variable->GetName().AsCString(""),
                                                                module->GetFileSpec().GetDirectory().GetCString(),
                                                                module->GetFileSpec().GetDirectory() ? "/" : "",
                                                                module->GetFileSpec().GetFilename().GetCString());
                            else
                                error.SetErrorStringWithFormat ("unable to resolve the module for file address 0x%llx in %s%s%s", 
                                                                address, 
                                                                module->GetFileSpec().GetDirectory().GetCString(),
                                                                module->GetFileSpec().GetDirectory() ? "/" : "",
                                                                module->GetFileSpec().GetFilename().GetCString());
                        }
                        else
                        {
                            if (variable)
                                error.SetErrorStringWithFormat ("unable to resolve the module for file address 0x%llx for variable '%s'", 
                                                                address, 
                                                                variable->GetName().AsCString(""));
                            else
                                error.SetErrorStringWithFormat ("unable to resolve the module for file address 0x%llx", address);
                        }
                    }
                }
                else
                {
                    // Can't convert a file address to anything valid without more
                    // context (which Module it came from)
                    error.SetErrorString ("can't read memory from file address without more context");
                }
            }
        }
        break;

    case eValueTypeHostAddress:
        address = m_value.ULongLong(LLDB_INVALID_ADDRESS);
        data.SetByteOrder(lldb::endian::InlHostByteOrder());
        data.SetAddressByteSize(sizeof(void *));
        address_type = eAddressTypeHost;
        break;
    }

    // Bail if we encountered any errors
    if (error.Fail())
        return error;

    if (address == LLDB_INVALID_ADDRESS)
    {
        error.SetErrorStringWithFormat ("invalid %s address", address_type == eAddressTypeHost ? "host" : "load");
        return error;
    }

    // If we got here, we need to read the value from memory
    uint32_t byte_size = GetValueByteSize (ast_context, &error);

    // Bail if we encountered any errors getting the byte size
    if (error.Fail())
        return error;

    // Make sure we have enough room within "data", and if we don't make
    // something large enough that does
    if (!data.ValidOffsetForDataOfSize (data_offset, byte_size))
    {
        DataBufferSP data_sp(new DataBufferHeap (data_offset + byte_size, '\0'));
        data.SetData(data_sp);
    }

    uint8_t* dst = const_cast<uint8_t*>(data.PeekData (data_offset, byte_size));
    if (dst != NULL)
    {
        if (address_type == eAddressTypeHost)
        {
            // The address is an address in this process, so just copy it
            memcpy (dst, (uint8_t*)NULL + address, byte_size);
        }
        else if ((address_type == eAddressTypeLoad) || (address_type == eAddressTypeFile))
        {
            if (file_so_addr.IsValid())
            {
                // We have a file address that we were able to translate into a
                // section offset address so we might be able to read this from
                // the object files if we don't have a live process. Lets always
                // try and read from the process if we have one though since we
                // want to read the actual value by setting "prefer_file_cache"
                // to false. 
                const bool prefer_file_cache = false;
                if (exe_ctx->target->ReadMemory(file_so_addr, prefer_file_cache, dst, byte_size, error) != byte_size)
                {
                    error.SetErrorStringWithFormat("read memory from 0x%llx failed", (uint64_t)address);
                }
            }
            else
            {
                // The execution context might have a NULL process, but it
                // might have a valid process in the exe_ctx->target, so use
                // the ExecutionContext::GetProcess accessor to ensure we
                // get the process if there is one.
                Process *process = exe_ctx->GetProcess();

                if (process)
                {
                    const size_t bytes_read = process->ReadMemory(address, dst, byte_size, error);
                    if (bytes_read != byte_size)
                        error.SetErrorStringWithFormat("read memory from 0x%llx failed (%u of %u bytes read)", 
                                                       (uint64_t)address, 
                                                       (uint32_t)bytes_read, 
                                                       (uint32_t)byte_size);
                }
                else
                {
                    error.SetErrorStringWithFormat("read memory from 0x%llx failed (invalid process)", (uint64_t)address);                    
                }
            }
        }
        else
        {
            error.SetErrorStringWithFormat ("unsupported AddressType value (%i)", address_type);
        }
    }
    else
    {
        error.SetErrorStringWithFormat ("out of memory");
    }

    return error;
}

Scalar &
Value::ResolveValue(ExecutionContext *exe_ctx, clang::ASTContext *ast_context)
{    
    void *opaque_clang_qual_type = GetClangType();
    if (opaque_clang_qual_type)
    {
        switch (m_value_type)
        {
        case eValueTypeScalar:               // raw scalar value
            break;

        default:
        case eValueTypeFileAddress:
            m_value.Clear();
            break;

        case eValueTypeLoadAddress:          // load address value
        case eValueTypeHostAddress:          // host address value (for memory in the process that is using liblldb)
            {
                AddressType address_type = m_value_type == eValueTypeLoadAddress ? eAddressTypeLoad : eAddressTypeHost;
                lldb::addr_t addr = m_value.ULongLong(LLDB_INVALID_ADDRESS);
                DataExtractor data;
                if (ClangASTType::ReadFromMemory (ast_context, opaque_clang_qual_type, exe_ctx, addr, address_type, data))
                {
                    Scalar scalar;
                    if (ClangASTType::GetValueAsScalar (ast_context, opaque_clang_qual_type, data, 0, data.GetByteSize(), scalar))
                    {
                        m_value = scalar;
                        m_value_type = eValueTypeScalar;
                    }
                    else
                    {
                        if ((uintptr_t)addr != (uintptr_t)m_data_buffer.GetBytes())
                        {
                            m_value.Clear();
                            m_value_type = eValueTypeScalar;
                        }
                    }
                }
                else
                {
                    if ((uintptr_t)addr != (uintptr_t)m_data_buffer.GetBytes())
                    {
                        m_value.Clear();
                        m_value_type = eValueTypeScalar;
                    }
                }
            }
            break;
        }
    }
    return m_value;
}

Variable *
Value::GetVariable()
{
    if (m_context_type == eContextTypeVariable)
        return static_cast<Variable *> (m_context);
    return NULL;
}

const char *
Value::GetValueTypeAsCString (ValueType value_type)
{    
    switch (value_type)
    {
    case eValueTypeScalar:      return "scalar";
    case eValueTypeFileAddress: return "file address";
    case eValueTypeLoadAddress: return "load address";
    case eValueTypeHostAddress: return "host address";
    };
    return "???";
}

const char *
Value::GetContextTypeAsCString (ContextType context_type)
{
    switch (context_type)
    {
    case eContextTypeInvalid:       return "invalid";
    case eContextTypeClangType:     return "clang::Type *";
    case eContextTypeRegisterInfo:  return "RegisterInfo *";
    case eContextTypeLLDBType:      return "Type *";
    case eContextTypeVariable:      return "Variable *";
    };
    return "???";
}

ValueList::ValueList (const ValueList &rhs)
{
    m_values = rhs.m_values;
}

const ValueList &
ValueList::operator= (const ValueList &rhs)
{
    m_values = rhs.m_values;
    return *this;
}

void
ValueList::PushValue (const Value &value)
{
    m_values.push_back (value);
}

size_t
ValueList::GetSize()
{
    return m_values.size();
}

Value *
ValueList::GetValueAtIndex (size_t idx)
{
    if (idx < GetSize())
    {
        return &(m_values[idx]);
    }
    else
        return NULL;
}

void
ValueList::Clear ()
{
    m_values.clear();
}
