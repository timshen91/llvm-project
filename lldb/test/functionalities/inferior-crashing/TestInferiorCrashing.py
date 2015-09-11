"""Test that lldb functions correctly after the inferior has crashed."""

import os, time
import unittest2
import lldb, lldbutil, lldbplatformutil
from lldbtest import *

class CrashingInferiorTestCase(TestBase):

    mydir = TestBase.compute_mydir(__file__)

    @skipUnlessDarwin
    def test_inferior_crashing_dsym(self):
        """Test that lldb reliably catches the inferior crashing (command)."""
        self.buildDsym()
        self.inferior_crashing()

    @expectedFailureFreeBSD("llvm.org/pr23699 SIGSEGV is reported as exception, not signal")
    @expectedFailureWindows("llvm.org/pr24778") # This actually works, but the test relies on the output format instead of the API
    def test_inferior_crashing_dwarf(self):
        """Test that lldb reliably catches the inferior crashing (command)."""
        self.buildDwarf()
        self.inferior_crashing()

    @skipUnlessDarwin
    def test_inferior_crashing_registers_dsym(self):
        """Test that lldb reliably reads registers from the inferior after crashing (command)."""
        self.buildDsym()
        self.inferior_crashing_registers()

    @expectedFailureWindows("llvm.org/pr24778")
    def test_inferior_crashing_register_dwarf(self):
        """Test that lldb reliably reads registers from the inferior after crashing (command)."""
        self.buildDwarf()
        self.inferior_crashing_registers()

    @python_api_test
    @expectedFailureWindows("llvm.org/pr24778")
    def test_inferior_crashing_python(self):
        """Test that lldb reliably catches the inferior crashing (Python API)."""
        self.buildDefault()
        self.inferior_crashing_python()

    @skipUnlessDarwin
    def test_inferior_crashing_expr_dsym(self):
        """Test that the lldb expression interpreter can read from the inferior after crashing (command)."""
        self.buildDsym()
        self.inferior_crashing_expr()

    @expectedFailureWindows("llvm.org/pr24778")
    def test_inferior_crashing_expr_dwarf(self):
        """Test that the lldb expression interpreter can read from the inferior after crashing (command)."""
        self.buildDwarf()
        self.inferior_crashing_expr()

    @skipUnlessDarwin
    def test_inferior_crashing_step_dsym(self):
        """Test that lldb functions correctly after stepping through a crash."""
        self.buildDsym()
        self.inferior_crashing_step()

    @expectedFailureAll("llvm.org/pr23139", oslist=["linux"], compiler="gcc", compiler_version=[">=","4.9"], archs=["i386"])
    @expectedFailureWindows("llvm.org/pr24778")
    def test_inferior_crashing_step_dwarf(self):
        """Test that stepping after a crash behaves correctly."""
        self.buildDwarf()
        self.inferior_crashing_step()

    @skipUnlessDarwin
    def test_inferior_crashing_step_after_break_dsym(self):
        """Test that stepping after a crash behaves correctly."""
        self.buildDsym()
        self.inferior_crashing_step_after_break()

    @skipIfFreeBSD # llvm.org/pr16684
    @expectedFailureWindows("llvm.org/pr24778")
    def test_inferior_crashing_step_after_break_dwarf(self):
        """Test that lldb functions correctly after stepping through a crash."""
        self.buildDwarf()
        self.inferior_crashing_step_after_break()

    @skipUnlessDarwin
    def test_inferior_crashing_expr_step_and_expr_dsym(self):
        """Test that lldb expressions work before and after stepping after a crash."""
        self.buildDsym()
        self.inferior_crashing_expr_step_expr()

    @expectedFailureFreeBSD('llvm.org/pr15989') # Couldn't allocate space for the stack frame
    @expectedFailureWindows("llvm.org/pr24778")
    @skipIfLinux # Inferior exits after stepping after a segfault. This is working as intended IMHO.
    def test_inferior_crashing_expr_step_and_expr_dwarf(self):
        """Test that lldb expressions work before and after stepping after a crash."""
        self.buildDwarf()
        self.inferior_crashing_expr_step_expr()

    def set_breakpoint(self, line):
        lldbutil.run_break_set_by_file_and_line (self, "main.c", line, num_expected_locations=1, loc_exact=True)

    def check_stop_reason(self):
        # We should have one crashing thread
        self.assertEquals(
                len(lldbutil.get_crashed_threads(self, self.dbg.GetSelectedTarget().GetProcess())),
                1,
                STOPPED_DUE_TO_EXC_BAD_ACCESS)

    def get_api_stop_reason(self):
        return lldb.eStopReasonException

    def setUp(self):
        # Call super's setUp().
        TestBase.setUp(self)
        # Find the line number of the crash.
        self.line = line_number('main.c', '// Crash here.')

    def inferior_crashing(self):
        """Inferior crashes upon launching; lldb should catch the event and stop."""
        exe = os.path.join(os.getcwd(), "a.out")
        self.runCmd("file " + exe, CURRENT_EXECUTABLE_SET)

        self.runCmd("run", RUN_SUCCEEDED)
        # The exact stop reason depends on the platform
        if self.platformIsDarwin():
            stop_reason = 'stop reason = EXC_BAD_ACCESS'
        elif self.getPlatform() == "linux":
            stop_reason = 'stop reason = signal SIGSEGV'
        else:
            stop_reason = 'stop reason = invalid address'
        self.expect("thread list", STOPPED_DUE_TO_EXC_BAD_ACCESS,
            substrs = ['stopped',
                       stop_reason])

        # And it should report the correct line number.
        self.expect("thread backtrace all",
            substrs = [stop_reason,
                       'main.c:%d' % self.line])

    def inferior_crashing_python(self):
        """Inferior crashes upon launching; lldb should catch the event and stop."""
        exe = os.path.join(os.getcwd(), "a.out")

        target = self.dbg.CreateTarget(exe)
        self.assertTrue(target, VALID_TARGET)

        # Now launch the process, and do not stop at entry point.
        # Both argv and envp are null.
        process = target.LaunchSimple (None, None, self.get_process_working_directory())

        if process.GetState() != lldb.eStateStopped:
            self.fail("Process should be in the 'stopped' state, "
                      "instead the actual state is: '%s'" %
                      lldbutil.state_type_to_str(process.GetState()))

        threads = lldbutil.get_crashed_threads(self, process)
        self.assertEqual(len(threads), 1, "Failed to stop the thread upon bad access exception")

        if self.TraceOn():
            lldbutil.print_stacktrace(threads[0])

    def inferior_crashing_registers(self):
        """Test that lldb can read registers after crashing."""
        exe = os.path.join(os.getcwd(), "a.out")
        self.runCmd("file " + exe, CURRENT_EXECUTABLE_SET)

        self.runCmd("run", RUN_SUCCEEDED)
        self.check_stop_reason()

        # lldb should be able to read from registers from the inferior after crashing.
        lldbplatformutil.check_first_register_readable(self)

    def inferior_crashing_expr(self):
        """Test that the lldb expression interpreter can read symbols after crashing."""
        exe = os.path.join(os.getcwd(), "a.out")
        self.runCmd("file " + exe, CURRENT_EXECUTABLE_SET)

        self.runCmd("run", RUN_SUCCEEDED)
        self.check_stop_reason()

        # The lldb expression interpreter should be able to read from addresses of the inferior after a crash.
        self.expect("p argc",
            startstr = '(int) $0 = 1')

        self.expect("p hello_world",
            substrs = ['Hello'])

    def inferior_crashing_step(self):
        """Test that lldb functions correctly after stepping through a crash."""
        exe = os.path.join(os.getcwd(), "a.out")
        self.runCmd("file " + exe, CURRENT_EXECUTABLE_SET)

        self.set_breakpoint(self.line)
        self.runCmd("run", RUN_SUCCEEDED)

        self.expect("thread list", STOPPED_DUE_TO_BREAKPOINT,
            substrs = ['main.c:%d' % self.line,
                       'stop reason = breakpoint'])

        self.runCmd("next")
        self.check_stop_reason()

        # The lldb expression interpreter should be able to read from addresses of the inferior after a crash.
        self.expect("p argv[0]",
            substrs = ['a.out'])
        self.expect("p null_ptr",
            substrs = ['= 0x0'])

        # lldb should be able to read from registers from the inferior after crashing.
        lldbplatformutil.check_first_register_readable(self)

        # And it should report the correct line number.
        self.expect("thread backtrace all",
            substrs = ['main.c:%d' % self.line])

    def inferior_crashing_step_after_break(self):
        """Test that lldb behaves correctly when stepping after a crash."""
        exe = os.path.join(os.getcwd(), "a.out")
        self.runCmd("file " + exe, CURRENT_EXECUTABLE_SET)

        self.runCmd("run", RUN_SUCCEEDED)
        self.check_stop_reason()

        expected_state = 'exited' # Provide the exit code.
        if self.platformIsDarwin():
            expected_state = 'stopped' # TODO: Determine why 'next' and 'continue' have no effect after a crash.
        elif re.match(".*-.*-.*-android", self.dbg.GetSelectedPlatform().GetTriple()):
            expected_state = 'stopped' # android has a default SEGV handler, which will re-raise the signal, so we come up stopped again

        self.expect("next",
            substrs = ['Process', expected_state])

        if expected_state == 'exited':
            self.expect("thread list", error=True,substrs = ['Process must be launched'])
        else:
            self.check_stop_reason()

    def inferior_crashing_expr_step_expr(self):
        """Test that lldb expressions work before and after stepping after a crash."""
        exe = os.path.join(os.getcwd(), "a.out")
        self.runCmd("file " + exe, CURRENT_EXECUTABLE_SET)

        self.runCmd("run", RUN_SUCCEEDED)
        self.check_stop_reason()

        # The lldb expression interpreter should be able to read from addresses of the inferior after a crash.
        self.expect("p argv[0]",
            substrs = ['a.out'])

        self.runCmd("next")
        self.check_stop_reason()

        # The lldb expression interpreter should be able to read from addresses of the inferior after a crash.
        self.expect("p argv[0]",
            substrs = ['a.out'])


if __name__ == '__main__':
    import atexit
    lldb.SBDebugger.Initialize()
    atexit.register(lambda: lldb.SBDebugger.Terminate())
    unittest2.main()
