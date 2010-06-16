//===-- BreakpointLocation.h ------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef liblldb_BreakpointLocation_h_
#define liblldb_BreakpointLocation_h_

// C Includes

// C++ Includes
#include <list>
#include <memory>

// Other libraries and framework includes

// Project includes
#include "lldb/lldb-private.h"
#include "lldb/Core/UserID.h"
#include "lldb/Breakpoint/StoppointLocation.h"
#include "lldb/Core/Address.h"
#include "lldb/Breakpoint/Breakpoint.h"
#include "lldb/Breakpoint/BreakpointOptions.h"
#include "lldb/Target/Process.h"
#include "lldb/Core/StringList.h"

namespace lldb_private {

//----------------------------------------------------------------------
/// @class BreakpointLocation BreakpointLocation.h "lldb/Breakpoint/BreakpointLocation.h"
/// @brief Class that manages one unique (by address) instance of a logical breakpoint.
//----------------------------------------------------------------------

//----------------------------------------------------------------------
/// General Outline:
/// A breakpoint location is defined by the breakpoint that produces it,
/// and the address that resulted in this particular instantiation.
/// Each breakpoint location also may have a breakpoint site if its
/// address has been loaded into the program.
/// Finally it has a settable options object.
///
/// FIXME: Should we also store some fingerprint for the location, so
/// we can map one location to the "equivalent location" on rerun?  This
/// would be useful if you've set options on the locations.
//----------------------------------------------------------------------

class BreakpointLocation : public StoppointLocation
{
public:

    ~BreakpointLocation ();

    //------------------------------------------------------------------
    /// Gets the load address for this breakpoint location
    /// @return
    ///     Returns breakpoint location load address, \b
    ///     LLDB_INVALID_ADDRESS if not yet set.
    //------------------------------------------------------------------
    lldb::addr_t
    GetLoadAddress () const;

    //------------------------------------------------------------------
    /// Gets the Address for this breakpoint location
    /// @return
    ///     Returns breakpoint location Address.
    //------------------------------------------------------------------
    Address &
    GetAddress ();
    //------------------------------------------------------------------
    /// Gets the Breakpoint that created this breakpoint location
    /// @return
    ///     Returns the owning breakpoint.
    //------------------------------------------------------------------
    Breakpoint &
    GetBreakpoint ();

    //------------------------------------------------------------------
    /// Determines whether we should stop due to a hit at this
    /// breakpoint location.
    ///
    /// Side Effects: This may evaluate the breakpoint condition, and
    /// run the callback.  So this command may do a considerable amount
    /// of work.
    ///
    /// @return
    ///     \b true if this breakpoint location thinks we should stop,
    ///     \b false otherwise.
    //------------------------------------------------------------------
    bool
    ShouldStop (StoppointCallbackContext *context);

    //------------------------------------------------------------------
    // The next section deals with various breakpoint options.
    //------------------------------------------------------------------

    //------------------------------------------------------------------
    /// If \a enable is \b true, enable the breakpoint, if \b false
    /// disable it.
    //------------------------------------------------------------------
    void
    SetEnabled(bool enabled);

    //------------------------------------------------------------------
    /// Check the Enable/Disable state.
    ///
    /// @return
    ///     \b true if the breakpoint is enabled, \b false if disabled.
    //------------------------------------------------------------------
    bool
    IsEnabled ();

    //------------------------------------------------------------------
    /// Return the current Ignore Count.
    ///
    /// @return
    ///     The number of breakpoint hits to be ignored.
    //------------------------------------------------------------------
    int32_t
    GetIgnoreCount ();

    //------------------------------------------------------------------
    /// Set the breakpoint to ignore the next \a count breakpoint hits.
    ///
    /// @param[in] count
    ///    The number of breakpoint hits to ignore.
    //------------------------------------------------------------------
    void
    SetIgnoreCount (int32_t n);

    //------------------------------------------------------------------
    /// Set the callback action invoked when the breakpoint is hit.
    ///
    /// The callback will return a bool indicating whether the target
    /// should stop at this breakpoint or not.
    ///
    /// @param[in] callback
    ///     The method that will get called when the breakpoint is hit.
    ///
    /// @param[in] callback_baton_sp
    ///     A shared pointer to a Baton that provides the void * needed
    ///     for the callback.
    ///
    /// @see lldb_private::Baton
    //------------------------------------------------------------------
    void
    SetCallback (BreakpointHitCallback callback, 
                 const lldb::BatonSP &callback_baton_sp,
                 bool is_synchronous);

    void
    SetCallback (BreakpointHitCallback callback, 
                 void *baton,
                 bool is_synchronous);
    
    void
    ClearCallback ();

    //------------------------------------------------------------------
    /// Set the condition expression to be checked when the breakpoint is hit.
    ///
    /// @param[in] expression
    ///    The method that will get called when the breakpoint is hit.
    //------------------------------------------------------------------
    void
    SetCondition (void *condition);


    //------------------------------------------------------------------
    /// Set the valid thread to be checked when the breakpoint is hit.
    ///
    /// @param[in] thread_id
    ///    If this thread hits the breakpoint, we stop, otherwise not.
    //------------------------------------------------------------------
    void
    SetThreadID (lldb::tid_t thread_id);

    //------------------------------------------------------------------
    // The next section deals with this location's breakpoint sites.
    //------------------------------------------------------------------

    //------------------------------------------------------------------
    /// Try to resolve the breakpoint site for this location.
    ///
    /// @return
    ///     \b true if we were successful at setting a breakpoint site,
    ///     \b false otherwise.
    //------------------------------------------------------------------
    bool
    ResolveBreakpointSite ();

    //------------------------------------------------------------------
    /// Clear this breakpoint location's breakpoint site - for instance
    /// when disabling the breakpoint.
    ///
    /// @return
    ///     \b true if there was a breakpoint site to be cleared, \b false
    ///     otherwise.
    //------------------------------------------------------------------
    bool
    ClearBreakpointSite ();

    //------------------------------------------------------------------
    /// Return whether this breakpoint location has a breakpoint site.
    /// @return
    ///     \b true if there was a breakpoint site for this breakpoint
    ///     location, \b false otherwise.
    //------------------------------------------------------------------
    bool
    IsResolved () const;

    //------------------------------------------------------------------
    // The next section are generic report functions.
    //------------------------------------------------------------------

    //------------------------------------------------------------------
    /// Print a description of this breakpoint location to the stream
    /// \a s.
    ///
    /// @param[in] s
    ///     The stream to which to print the description.
    ///
    /// @param[in] level
    ///     The description level that indicates the detail level to
    ///     provide.
    ///
    /// @see lldb::DescriptionLevel
    //------------------------------------------------------------------
    void
    GetDescription (Stream *s, lldb::DescriptionLevel level);

    //------------------------------------------------------------------
    /// Standard "Dump" method.  At present it does nothing.
    //------------------------------------------------------------------
    void
    Dump (Stream *s) const;

    //------------------------------------------------------------------
    /// Use this to set location specific breakpoint options.
    ///
    /// It will create a copy of the containing breakpoint's options if
    /// that hasn't been done already
    ///
    /// @return
    ///    A pointer to the breakpoint options.
    //------------------------------------------------------------------
    BreakpointOptions *
    GetLocationOptions ();

    //------------------------------------------------------------------
    /// Use this to access location specific breakpoint options.
    ///
    /// @return
    ///     A pointer to the containing breakpoint's options if this
    ///     location doesn't have its own copy.
    //------------------------------------------------------------------
    const BreakpointOptions *
    GetOptionsNoCopy () const;
    
    bool
    ValidForThisThread (Thread *thread);

protected:
    friend class Breakpoint;
    friend class CommandObjectBreakpointCommandAdd;
    friend class Process;
    
    //------------------------------------------------------------------
    /// Invoke the callback action when the breakpoint is hit.
    ///
    /// Meant to be used by the BreakpointLocation class.
    ///
    /// @param[in] context
    ///    Described the breakpoint event.
    ///
    /// @param[in] bp_loc_id
    ///    Which breakpoint location hit this breakpoint.
    ///
    /// @return
    ///     \b true if the target should stop at this breakpoint and \b
    ///     false not.
    //------------------------------------------------------------------
    bool
    InvokeCallback (StoppointCallbackContext *context);

    //------------------------------------------------------------------
    /// Set the breakpoint site for this location to \a bp_site_sp.
    ///
    /// @param[in] bp_site_sp
    ///      The breakpoint site we are setting for this location.
    ///
    /// @return
    ///     \b true if we were successful at setting the breakpoint site,
    ///     \b false otherwise.
    //------------------------------------------------------------------
    bool
    SetBreakpointSite (lldb::BreakpointSiteSP& bp_site_sp);

private:

    //------------------------------------------------------------------
    // Constructors and Destructors
    //
    // Only the Breakpoint can make breakpoint locations, and it owns
    // them.
    //------------------------------------------------------------------

    //------------------------------------------------------------------
    /// Constructor.
    ///
    /// @param[in] owner
    ///     A back pointer to the breakpoint that owns this location.
    ///
    /// @param[in] addr
    ///     The Address defining this location.
    ///
    /// @param[in] tid
    ///     The thread for which this breakpoint location is valid, or
    ///     LLDB_INVALID_THREAD_ID if it is valid for all threads.
    ///
    /// @param[in] hardware
    ///     \b true if a hardware breakpoint is requested.
    //------------------------------------------------------------------

    BreakpointLocation (lldb::break_id_t bid,
                        Breakpoint &owner,
                        Address &addr,
                        lldb::tid_t tid = LLDB_INVALID_THREAD_ID,
                        bool hardware = false);

    //------------------------------------------------------------------
    // Data members:
    //------------------------------------------------------------------
    Address m_address; ///< The address defining this location.
    Breakpoint &m_owner; ///< The breakpoint that produced this object.
    std::auto_ptr<BreakpointOptions> m_options_ap; ///< Breakpoint options pointer, NULL if we're using our breakpoint's options.
    lldb::BreakpointSiteSP m_bp_site_sp; ///< Our breakpoint site (it may be shared by more than one location.)

    DISALLOW_COPY_AND_ASSIGN (BreakpointLocation);
};

} // namespace lldb_private

#endif  // liblldb_BreakpointLocation_h_
