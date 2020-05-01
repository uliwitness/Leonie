/*
 *  LEODebugger.h
 *  Leonie
 *
 *  Created by Uli Kusterer on 20.09.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#ifndef LEO_DEBUGGER_H
#define LEO_DEBUGGER_H		1

/*!
	@header LEODebugger
	These are hooks and utility functions for a little command-line debugger
	that can be used to analyze your bytecode while it is running.
	
	To activate it, set your LEOContext's PreInstructionProc to
	LEODebuggerPreInstructionProc.
*/

// -----------------------------------------------------------------------------
//	Headers:
// -----------------------------------------------------------------------------

#include "LEOInterpreter.h"


#if __cplusplus
extern "C" {
#endif


/*! Use this function as the PreInstructionProc of your LEOContext to activate
	the debugger. */
void LEODebuggerPreInstructionProc( struct LEOContext* inContext );

/*! Set a breakpoint on the given instruction. This will cause execution to be
	interrupted and a debugger console to be shown that allows examining the
	current stack.
	@seealso //leo_ref/c/func/LEODebuggerRemoveBreakpoint LEODebuggerRemoveBreakpoint */
void LEODebuggerAddBreakpoint( LEOInstruction* targetInstruction, struct LEOScript * inScript );

/*! Remove a breakpoint set using LEODebuggerAddBreakpoint().
	@seealso //leo_ref/c/func/LEODebuggerAddBreakpoint LEODebuggerAddBreakpoint */
void LEODebuggerRemoveBreakpoint( LEOInstruction* targetInstruction );


void LEODebuggerPrompt( struct LEOContext* inContext );


#if __cplusplus
}
#endif

#endif // LEO_DEBUGGER_H

