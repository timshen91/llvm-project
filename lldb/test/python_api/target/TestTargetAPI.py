"""
Test SBTarget APIs.
"""

import os, time
import re
import unittest2
import lldb, lldbutil
from lldbtest import *

class TargetAPITestCase(TestBase):

    mydir = os.path.join("python_api", "target")

    @unittest2.skipUnless(sys.platform.startswith("darwin"), "requires Darwin")
    @python_api_test
    def test_get_description_with_dsym(self):
        """Exercise SBTaget.GetDescription() API."""
        self.buildDsym()
        self.get_description()

    @python_api_test
    def test_get_description_with_dwarf(self):
        """Exercise SBTarget.GetDescription() API."""
        self.buildDwarf()
        self.get_description()

    @unittest2.skipUnless(sys.platform.startswith("darwin"), "requires Darwin")
    @python_api_test
    def test_launch_new_process_and_redirect_stdout_with_dsym(self):
        """Exercise SBTaget.Launch() API."""
        self.buildDsym()
        self.launch_new_process_and_redirect_stdout()

    @python_api_test
    def test_launch_new_process_and_redirect_stdout_with_dwarf(self):
        """Exercise SBTarget.Launch() API."""
        self.buildDwarf()
        self.launch_new_process_and_redirect_stdout()

    @unittest2.skipUnless(sys.platform.startswith("darwin"), "requires Darwin")
    @python_api_test
    def test_resolve_symbol_context_with_address_with_dsym(self):
        """Exercise SBTaget.ResolveSymbolContextForAddress() API."""
        self.buildDsym()
        self.resolve_symbol_context_with_address()

    @python_api_test
    def test_resolve_symbol_context_with_address_with_dwarf(self):
        """Exercise SBTarget.ResolveSymbolContextForAddress() API."""
        self.buildDwarf()
        self.resolve_symbol_context_with_address()

    def setUp(self):
        # Call super's setUp().
        TestBase.setUp(self)
        # Find the line number to of function 'c'.
        self.line1 = line_number('main.c', '// Find the line number for breakpoint 1 here.')
        self.line2 = line_number('main.c', '// Find the line number for breakpoint 2 here.')

    def get_description(self):
        """Exercise SBTaget.GetDescription() API."""
        exe = os.path.join(os.getcwd(), "a.out")

        # Create a target by the debugger.
        target = self.dbg.CreateTarget(exe)
        self.assertTrue(target.IsValid(), VALID_TARGET)

        from lldbutil import get_description
        desc = get_description(target, option=lldb.eDescriptionLevelBrief)
        if not desc:
            self.fail("SBTarget.GetDescription() failed")
        self.expect(desc, exe=False,
            substrs = ['a.out'])
        self.expect(desc, exe=False, matching=False,
            substrs = ['Target', 'Module', 'Breakpoint'])

        desc = get_description(target, option=lldb.eDescriptionLevelFull)
        if not desc:
            self.fail("SBTarget.GetDescription() failed")
        self.expect(desc, exe=False,
            substrs = ['a.out', 'Target', 'Module', 'Breakpoint'])


    def launch_new_process_and_redirect_stdout(self):
        """Exercise SBTaget.Launch() API with redirected stdout."""
        exe = os.path.join(os.getcwd(), "a.out")

        # Create a target by the debugger.
        target = self.dbg.CreateTarget(exe)
        self.assertTrue(target.IsValid(), VALID_TARGET)

        # Add an extra twist of stopping the inferior in a breakpoint, and then continue till it's done.
        # We should still see the entire stdout redirected once the process is finished.
        line = line_number('main.c', '// a(3) -> c(3)')
        breakpoint = target.BreakpointCreateByLocation('main.c', line)

        # Now launch the process, do not stop at entry point, and redirect stdout to "stdout.txt" file.
        # The inferior should run to completion after "process.Continue()" call, so there's no need
        # to assign to self.process to have the inferior kiiled during test teardown.
        error = lldb.SBError()
        process = target.Launch (self.dbg.GetListener(), None, None, None, "stdout.txt", None, None, 0, False, error)
        process.Continue()
        #self.runCmd("process status")

        # The 'stdout.txt' file should now exist.
        self.assertTrue(os.path.isfile("stdout.txt"),
                        "'stdout.txt' exists due to redirected stdout via SBTarget.Launch() API.")

        # Read the output file produced by running the program.
        with open('stdout.txt', 'r') as f:
            output = f.read()

        # Let's delete the 'stdout.txt' file as a cleanup step.
        try:
            os.remove("stdout.txt")
            pass
        except OSError:
            pass

        self.expect(output, exe=False,
            substrs = ["a(1)", "b(2)", "a(3)"])


    def resolve_symbol_context_with_address(self):
        """Exercise SBTaget.ResolveSymbolContextForAddress() API."""
        exe = os.path.join(os.getcwd(), "a.out")

        # Create a target by the debugger.
        target = self.dbg.CreateTarget(exe)
        self.assertTrue(target.IsValid(), VALID_TARGET)

        # Now create the two breakpoints inside function 'a'.
        breakpoint1 = target.BreakpointCreateByLocation('main.c', self.line1)
        breakpoint2 = target.BreakpointCreateByLocation('main.c', self.line2)
        #print "breakpoint1:", breakpoint1
        #print "breakpoint2:", breakpoint2
        self.assertTrue(breakpoint1.IsValid() and
                        breakpoint1.GetNumLocations() == 1,
                        VALID_BREAKPOINT)
        self.assertTrue(breakpoint2.IsValid() and
                        breakpoint2.GetNumLocations() == 1,
                        VALID_BREAKPOINT)

        # Now launch the process, and do not stop at entry point.
        self.process = target.LaunchSimple(None, None, os.getcwd())

        self.process = target.GetProcess()
        self.assertTrue(self.process.IsValid(), PROCESS_IS_VALID)

        # Frame #0 should be on self.line1.
        self.assertTrue(self.process.GetState() == lldb.eStateStopped)
        thread = lldbutil.get_stopped_thread(self.process, lldb.eStopReasonBreakpoint)
        self.assertTrue(thread != None, "There should be a thread stopped due to breakpoint condition")
        #self.runCmd("process status")
        frame0 = thread.GetFrameAtIndex(0)
        lineEntry = frame0.GetLineEntry()
        self.assertTrue(lineEntry.GetLine() == self.line1)

        address1 = lineEntry.GetStartAddress()

        # Continue the inferior, the breakpoint 2 should be hit.
        self.process.Continue()
        self.assertTrue(self.process.GetState() == lldb.eStateStopped)
        thread = lldbutil.get_stopped_thread(self.process, lldb.eStopReasonBreakpoint)
        self.assertTrue(thread != None, "There should be a thread stopped due to breakpoint condition")
        #self.runCmd("process status")
        frame0 = thread.GetFrameAtIndex(0)
        lineEntry = frame0.GetLineEntry()
        self.assertTrue(lineEntry.GetLine() == self.line2)

        address2 = lineEntry.GetStartAddress()

        #print "address1:", address1
        #print "address2:", address2

        # Now call SBTarget.ResolveSymbolContextForAddress() with the addresses from our line entry.
        context1 = target.ResolveSymbolContextForAddress(address1, lldb.eSymbolContextEverything)
        context2 = target.ResolveSymbolContextForAddress(address2, lldb.eSymbolContextEverything)

        self.assertTrue(context1.IsValid() and context2.IsValid())
        #print "context1:", context1
        #print "context2:", context2

        # Verify that the context point to the same function 'a'.
        symbol1 = context1.GetSymbol()
        symbol2 = context2.GetSymbol()
        self.assertTrue(symbol1.IsValid() and symbol2.IsValid())
        #print "symbol1:", symbol1
        #print "symbol2:", symbol2

        from lldbutil import get_description
        desc1 = get_description(symbol1)
        desc2 = get_description(symbol2)
        self.assertTrue(desc1 and desc2 and desc1 == desc2,
                        "The two addresses should resolve to the same symbol")

        
if __name__ == '__main__':
    import atexit
    lldb.SBDebugger.Initialize()
    atexit.register(lambda: lldb.SBDebugger.Terminate())
    unittest2.main()
