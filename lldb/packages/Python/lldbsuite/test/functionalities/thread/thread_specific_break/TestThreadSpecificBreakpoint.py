"""
Test that we obey thread conditioned breakpoints.
"""

from __future__ import print_function



import os, time
import re
import lldb
import lldbsuite.test.lldbutil as lldbutil
from lldbsuite.test.lldbtest import *

class ThreadSpecificBreakTestCase(TestBase):

    mydir = TestBase.compute_mydir(__file__)

    @skipIfFreeBSD # test frequently times out or hangs
    @expectedFailureFreeBSD('llvm.org/pr18522') # hits break in another thread in testrun
    @add_test_categories(['pyapi'])
    @expectedFlakeyLinux # this test fails 6/100 dosep runs
    def test_python(self):
        """Test that we obey thread conditioned breakpoints."""
        self.build()
        exe = os.path.join(os.getcwd(), "a.out")

        self.dbg.HandleCommand ("log enable -f /tmp/lldb-testsuite-log.txt lldb step breakpoint process") 
        target = self.dbg.CreateTarget(exe)
        self.assertTrue(target, VALID_TARGET)

        main_source_spec = lldb.SBFileSpec ("main.cpp")

        # Set a breakpoint in the thread body, and make it active for only the first thread.
        break_thread_body = target.BreakpointCreateBySourceRegex ("Break here in thread body.", main_source_spec)
        self.assertTrue (break_thread_body.IsValid() and break_thread_body.GetNumLocations() > 0, "Failed to set thread body breakpoint.")

        process = target.LaunchSimple (None, None, self.get_process_working_directory())

        self.assertTrue(process, PROCESS_IS_VALID)

        threads = lldbutil.get_threads_stopped_at_breakpoint (process, break_thread_body)
        
        victim_thread = threads[0]

        # Pick one of the threads, and change the breakpoint so it ONLY stops for this thread,
        # but add a condition that it won't stop for this thread's my_value.  The other threads
        # pass the condition, so they should stop, but if the thread-specification is working
        # they should not stop.  So nobody should hit the breakpoint anymore, and we should
        # just exit cleanly.

        frame = victim_thread.GetFrameAtIndex(0)
        value = frame.FindVariable("my_value").GetValueAsSigned(0)
        self.assertTrue (value > 0 and value < 11, "Got a reasonable value for my_value.")

        cond_string = "my_value != %d"%(value)

        break_thread_body.SetThreadID(victim_thread.GetThreadID())
        break_thread_body.SetCondition (cond_string)

        process.Continue()

        next_stop_state = process.GetState()
        self.assertTrue (next_stop_state == lldb.eStateExited, "We should have not hit the breakpoint again.")
