//===-- SBAddress.h ---------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLDB_SBAddress_h_
#define LLDB_SBAddress_h_

#include "lldb/API/SBDefines.h"
#include "lldb/API/SBModule.h"

namespace lldb {

class SBAddress
{
public:

    SBAddress ();

    SBAddress (const lldb::SBAddress &rhs);

    // Create an address by resolving a load address using the supplied target
    SBAddress (lldb::addr_t load_addr, lldb::SBTarget &target);

    ~SBAddress ();

#ifndef SWIG
    const lldb::SBAddress &
    operator = (const lldb::SBAddress &rhs);
#endif

    bool
    IsValid () const;

    void
    Clear ();

    addr_t
    GetFileAddress () const;

    addr_t
    GetLoadAddress (const lldb::SBTarget &target) const;

    void
    SetLoadAddress (lldb::addr_t load_addr, 
                    lldb::SBTarget &target);
    bool
    OffsetAddress (addr_t offset);

    bool
    GetDescription (lldb::SBStream &description);

    SectionType
    GetSectionType ();

    lldb::SBModule
    GetModule ();

protected:

    friend class SBFrame;
    friend class SBFunction;
    friend class SBLineEntry;
    friend class SBInstruction;
    friend class SBModule;
    friend class SBSymbol;
    friend class SBSymbolContext;
    friend class SBTarget;
    friend class SBThread;

#ifndef SWIG

    lldb_private::Address *
    operator->();

    const lldb_private::Address *
    operator->() const;

    lldb_private::Address *
    get ();

    lldb_private::Address &
    ref();

    const lldb_private::Address &
    ref() const;

#endif


    SBAddress (const lldb_private::Address *lldb_object_ptr);

    void
    SetAddress (const lldb_private::Address *lldb_object_ptr);

private:

    std::auto_ptr<lldb_private::Address> m_opaque_ap;
};


} // namespace lldb

#endif // LLDB_SBAddress_h_
