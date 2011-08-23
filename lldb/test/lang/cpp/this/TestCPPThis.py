"""
Tests that C++ member and static variables are available where they should be.
"""

from lldbtest import *

class CPPThisTestCase(TestBase):
    
    mydir = os.path.join("lang", "cpp", "this")
    
    @unittest2.skip("segmentation fault -- skipping")
    @unittest2.skipUnless(sys.platform.startswith("darwin"), "requires Darwin")
    #rdar://problem/9962849
    @expectedFailureClang
    def test_with_dsym_and_run_command(self):
        """Test that the appropriate member variables are available when stopped in C++ static, inline, and const methods"""
        self.buildDsym()
        self.static_method_commands()

    @unittest2.skip("segmentation fault -- skipping")
    #rdar://problem/9962849
    @expectedFailureClang
    def test_with_dwarf_and_run_command(self):
        """Test that the appropriate member variables are available when stopped in C++ static, inline, and const methods"""
        self.buildDwarf()
        self.static_method_commands()

    def setUp(self):
        TestBase.setUp(self)
    
    def set_breakpoint(self, line):
        self.expect("breakpoint set -f main.cpp -l %d" % line,
                    BREAKPOINT_CREATED,
                    startstr = "Breakpoint created")
    
    def static_method_commands(self):
        """Test that the appropriate member variables are available when stopped in C++ static, inline, and const methods"""
        self.runCmd("file a.out", CURRENT_EXECUTABLE_SET)

        self.set_breakpoint(line_number('main.cpp', '// breakpoint 1'))
        self.set_breakpoint(line_number('main.cpp', '// breakpoint 2'))
        self.set_breakpoint(line_number('main.cpp', '// breakpoint 3'))
        self.set_breakpoint(line_number('main.cpp', '// breakpoint 4'))

        self.runCmd("process launch", RUN_SUCCEEDED)

        self.expect("expression -- m_a = 2",
                    startstr = "(int) $0 = 2")
        
        self.runCmd("process continue")
        
        # This would be disallowed if we enforced const.  But we don't.
        self.expect("expression -- m_a = 2",
                    startstr = "(int) $1 = 2")
        
        self.expect("expression -- m_a", 
                    startstr = "(int) $2 = 2")

        self.runCmd("process continue")

        self.expect("expression -- s_a",
                    startstr = "(int) $3 = 5")

        self.runCmd("process continue")

        self.expect("expression -- m_a",
                    startstr = "(int) $4 = 2")

if __name__ == '__main__':
    import atexit
    lldb.SBDebugger.Initialize()
    atexit.register(lambda: lldb.SBDebugger.Terminate())
    unittest2.main()
