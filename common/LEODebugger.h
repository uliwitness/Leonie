/*
 *  LEODebugger.h
 *  Leonie
 *
 *  Created by Uli Kusterer on 20.09.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

/*!
	@header LEODebugger
	These are hooks and utility functions for a little command-line debugger
	that can be used to analyze your bytecode while it is running.
	
	To activate it, set your LEOContext's PreInstructionProc to
	LEODebuggerPreInstructionProc.
*/

#include "LEOInterpreter.h"

/*! Use this function as the PreInstructionProc of your LEOContext to activate
	the debugger. */
void LEODebuggerPreInstructionProc( struct LEOContext* inContext );

/*! Set a breakpoint on the given instruction. This will cause execution to be
	interrupted and a debugger console to be shown that allows examining the
	current stack.
	@seealso //leo_ref/c/func/LEODebuggerRemoveBreakpoint LEODebuggerRemoveBreakpoint */
void LEODebuggerAddBreakpoint( LEOInstruction* targetInstruction );

/*! Remove a breakpoint set using LEODebuggerAddBreakpoint().
	@seealso //leo_ref/c/func/LEODebuggerAddBreakpoint LEODebuggerAddBreakpoint */
void LEODebuggerRemoveBreakpoint( LEOInstruction* targetInstruction );