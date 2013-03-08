//
//  Xcode.h
//  PerfTestDriver
//
//  Created by Enrico Granata on 3/6/13.
//  Copyright (c) 2013 Apple Inc. All rights reserved.
//

#ifndef __PerfTestDriver__Xcode__
#define __PerfTestDriver__Xcode__

#include "lldb/API/SBDefines.h"
#include "lldb/API/SBValue.h"
#include "lldb/API/SBTarget.h"
#include "lldb/API/SBModule.h"
#include "lldb/API/SBProcess.h"
#include "lldb/API/SBLineEntry.h"
#include "lldb/API/SBThread.h"
#include "lldb/API/SBDebugger.h"
#include "lldb/API/SBCommandInterpreter.h"
#include "lldb/API/SBCommandReturnObject.h"
#include "lldb/API/SBBreakpoint.h"

using namespace lldb;

namespace lldb { namespace perf
{
class Xcode
{
public:
	static void
	FetchVariable (SBValue value, uint32_t expand = 0, bool verbose = false);
    
	static void
	FetchModules (SBTarget target, bool verbose = false);
    
	static void
	FetchVariables (SBFrame frame, uint32_t expand = 0, bool verbose = false);
    
	static void
	FetchFrames (SBProcess process, bool variables = false, bool verbose = false);
	
	static void
	RunExpression (SBFrame frame, const char* expression, bool po = false, bool verbose = false);
	
	static void
	Next (SBThread thread);
	
	static void
	Continue (SBProcess process);
	
	static void
	RunCommand (SBDebugger debugger, const char* cmd, bool verbose = false);
	
	static SBThread
	GetThreadWithStopReason (SBProcess process, StopReason reason);
    
    static SBBreakpoint
    CreateFileLineBreakpoint (SBTarget target, const char* file, uint32_t line);
};
} }

#endif /* defined(__PerfTestDriver__Xcode__) */
