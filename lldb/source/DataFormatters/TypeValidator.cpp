//===-- TypeValidator.cpp ---------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// C Includes

// C++ Includes

// Other libraries and framework includes

// Project includes
#include "lldb/DataFormatters/TypeValidator.h"
#import "lldb/Core/StreamString.h"

using namespace lldb;
using namespace lldb_private;

TypeValidatorImpl::ValidationResult
TypeValidatorImpl::Success ()
{
    return ValidationResult { ValidationResult::ResultType::eSuccess, "" };
}

TypeValidatorImpl::ValidationResult
TypeValidatorImpl::Failure (std::string message)
{
    return ValidationResult { ValidationResult::ResultType::eFailure, message };
}

TypeValidatorImpl_CXX::TypeValidatorImpl_CXX (ValidatorFunction f, std::string d, const TypeValidatorImpl::Flags& flags) :
    TypeValidatorImpl(flags),
    m_description(d),
    m_validator_function(f)
{
}

TypeValidatorImpl::ValidationResult
TypeValidatorImpl_CXX::FormatObject (ValueObject *valobj) const
{
    if (!valobj)
        return Success(); // I guess there's nothing wrong with a null valueobject..
    
    return m_validator_function(valobj);
}

std::string
TypeValidatorImpl_CXX::GetDescription()
{
    StreamString sstr;
    sstr.Printf ("%s%s%s%s",
                 m_description.c_str(),
                 Cascades() ? "" : " (not cascading)",
                 SkipsPointers() ? " (skip pointers)" : "",
                 SkipsReferences() ? " (skip references)" : "");
    return sstr.GetString();
}
