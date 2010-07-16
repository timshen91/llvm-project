//===-- ThreadPlanStepInstruction.cpp ---------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//


#include "lldb/Target/ThreadPlanStepInstruction.h"

// C Includes
// C++ Includes
// Other libraries and framework includes
// Project includes
#include "lldb/lldb-private-log.h"
#include "lldb/Core/Log.h"
#include "lldb/Core/Stream.h"
#include "lldb/Target/RegisterContext.h"
#include "lldb/Target/Target.h"
#include "lldb/Target/RegisterContext.h"
#include "lldb/Target/Process.h"

using namespace lldb;
using namespace lldb_private;

//----------------------------------------------------------------------
// ThreadPlanStepInstruction: Step over the current instruction
//----------------------------------------------------------------------

ThreadPlanStepInstruction::ThreadPlanStepInstruction
(
    Thread &thread,
    bool step_over,
    bool stop_other_threads,
    Vote stop_vote,
    Vote run_vote
) :
    ThreadPlan (ThreadPlan::eKindStepInstruction, "Step over single instruction", thread, stop_vote, run_vote),
    m_instruction_addr (0),
    m_stop_other_threads (stop_other_threads),
    m_step_over (step_over),
    m_stack_depth (0)
{
    m_instruction_addr = m_thread.GetRegisterContext()->GetPC(0);
    m_stack_depth = m_thread.GetStackFrameCount();
}

ThreadPlanStepInstruction::~ThreadPlanStepInstruction ()
{
}

void
ThreadPlanStepInstruction::GetDescription (Stream *s, lldb::DescriptionLevel level)
{
    if (level == lldb::eDescriptionLevelBrief)
    {
        if (m_step_over)
            s->Printf ("instruction step over");
        else
            s->Printf ("instruction step into");
    }
    else
    {
        s->Printf ("Stepping one instruction past ");
        s->Address(m_instruction_addr, sizeof (addr_t));
        if (m_step_over)
            s->Printf(" stepping over calls");
        else
            s->Printf(" stepping into calls");
    }
}

bool
ThreadPlanStepInstruction::ValidatePlan (Stream *error)
{
    // Since we read the instruction we're stepping over from the thread,
    // this plan will always work.
    return true;
}

bool
ThreadPlanStepInstruction::PlanExplainsStop ()
{
    Thread::StopInfo info;
    if (m_thread.GetStopInfo (&info))
    {
        StopReason reason = info.GetStopReason();
        if (reason == eStopReasonTrace || reason ==eStopReasonNone)
            return true;
        else
            return false;
    }
    return false;
}

bool
ThreadPlanStepInstruction::ShouldStop (Event *event_ptr)
{
    if (m_step_over)
    {
        Log *log = lldb_private::GetLogIfAllCategoriesSet (LIBLLDB_LOG_STEP);
        if (m_thread.GetStackFrameCount() <= m_stack_depth)
        {
            if (m_thread.GetRegisterContext()->GetPC(0) != m_instruction_addr)
            {
                SetPlanComplete();
                return true;
            }
            else
                return false;
        }
        else
        {
            // We've stepped in, step back out again:
            StackFrame *return_frame = m_thread.GetStackFrameAtIndex(1).get();
            if (return_frame)
            {
                if (log)
                {
                    StreamString s;
                    s.PutCString ("Stepped in to: ");
                    addr_t stop_addr = m_thread.GetStackFrameAtIndex(0)->GetPC().GetLoadAddress(&m_thread.GetProcess());
                    s.Address (stop_addr, m_thread.GetProcess().GetAddressByteSize());
                    s.PutCString (" stepping out to: ");
                    addr_t return_addr = return_frame->GetPC().GetLoadAddress(&m_thread.GetProcess());
                    s.Address (return_addr, m_thread.GetProcess().GetAddressByteSize());
                    log->Printf("%s.", s.GetData());
                }
                m_thread.QueueThreadPlanForStepOut(false, NULL, true, m_stop_other_threads, eVoteNo, eVoteNoOpinion);
                return false;
            }
            else
            {
                if (log)
                    log->Printf("Could not find previous frame, stopping.");
                SetPlanComplete();
                return true;
            }

        }

    }
    else
    {
        if (m_thread.GetRegisterContext()->GetPC(0) != m_instruction_addr)
        {
            SetPlanComplete();
            return true;
        }
        else
            return false;
    }
}

bool
ThreadPlanStepInstruction::StopOthers ()
{
    return m_stop_other_threads;
}

StateType
ThreadPlanStepInstruction::RunState ()
{
    return eStateStepping;
}

bool
ThreadPlanStepInstruction::WillStop ()
{
    return true;
}

bool
ThreadPlanStepInstruction::MischiefManaged ()
{
    if (IsPlanComplete())
    {
        Log *log = lldb_private::GetLogIfAllCategoriesSet (LIBLLDB_LOG_STEP);
        if (log)
            log->Printf("Completed single instruction step plan.");
        ThreadPlan::MischiefManaged ();
        return true;
    }
    else
    {
        return false;
    }
}

