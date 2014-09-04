"""
Test that ASan memory history provider returns correct stack traces
"""

import os, time
import unittest2
import lldb
from lldbtest import *
import lldbutil

class AsanTestCase(TestBase):

    mydir = TestBase.compute_mydir(__file__)

    # The default compiler ("clang") may not support Address Sanitizer or it
    # may not have the debugging API which was recently added, so we're calling
    # self.useBuiltClang() to use clang from the llvm-build directory instead

    @unittest2.skipUnless(sys.platform.startswith("darwin"), "requires Darwin")
    @dsym_test
    def test_with_dsym (self):
        compiler = self.findBuiltClang ()
        self.buildDsym (None, compiler)
        self.asan_tests ()

    @dwarf_test
    def test_with_dwarf (self):
        compiler = self.findBuiltClang ()
        self.buildDwarf (None, compiler)
        self.asan_tests ()

    def setUp(self):
        # Call super's setUp().
        TestBase.setUp(self)
        self.line_malloc = line_number('main.c', '// malloc line')
        self.line_malloc2 = line_number('main.c', '// malloc2 line')
        self.line_free = line_number('main.c', '// free line')
        self.line_breakpoint = line_number('main.c', '// break line')

    def asan_tests (self):
        exe = os.path.join (os.getcwd(), "a.out")
        self.expect("file " + exe, patterns = [ "Current executable set to .*a.out" ])

        self.runCmd("breakpoint set -f main.c -l %d" % self.line_breakpoint)

        # "memory history" command should not work without a process
        self.expect("memory history 0",
            error = True,
            substrs = ["invalid process"])

        self.runCmd("run")

        # ASan will relaunch the process to insert its library.
        self.expect("thread list", "Process should be stopped due to exec.",
            substrs = ['stopped', 'stop reason = exec'])

        self.runCmd("continue")

        # the stop reason of the thread should be breakpoint.
        self.expect("thread list", STOPPED_DUE_TO_BREAKPOINT,
            substrs = ['stopped', 'stop reason = breakpoint'])

        # test that the ASan dylib is present
        self.expect("image lookup -n __asan_describe_address", "__asan_describe_address should be present",
            substrs = ['1 match found'])

        # test the 'memory history' command
        self.expect("memory history 'pointer'",
            substrs = [
                'Memory allocated at', 'a.out`f1', 'main.c:%d' % self.line_malloc,
                'Memory deallocated at', 'a.out`f2', 'main.c:%d' % self.line_free])

        self.runCmd("breakpoint set -n __asan_report_error")

        self.runCmd("continue")

        # the stop reason of the thread should be breakpoint.
        self.expect("thread list", STOPPED_DUE_TO_BREAKPOINT,
            substrs = ['stopped', 'stop reason = breakpoint'])

        # make sure the 'memory history' command still works even when we're generating a report now
        self.expect("memory history 'another_pointer'",
            substrs = [
                'Memory allocated at', 'a.out`f1', 'main.c:%d' % self.line_malloc2])

if __name__ == '__main__':
    import atexit
    lldb.SBDebugger.Initialize()
    atexit.register(lambda: lldb.SBDebugger.Terminate())
    unittest2.main()
