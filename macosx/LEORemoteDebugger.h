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


bool LEOInitRemoteDebugger( const char* inHostName );


/*! Use this function as the PreInstructionProc of your LEOContext to activate
	the debugger. */
void LEORemoteDebuggerPreInstructionProc( struct LEOContext* inContext );

/*! Register a file name and its contents with the debugger, so it can display
	its contents and highlight the current line.
*/
void	LEORemoteDebuggerAddFile( const char* filename, const char* filecontents, LEOInstruction* instrs, size_t numInstrs );


/*! Set a breakpoint on the given instruction. This will cause execution to be
	interrupted and a debugger console to be shown that allows examining the
	current stack.
	@seealso //leo_ref/c/func/LEODebuggerRemoveBreakpoint LEODebuggerRemoveBreakpoint */
void LEORemoteDebuggerAddBreakpoint( LEOInstruction* targetInstruction );

/*! Remove a breakpoint set using LEODebuggerAddBreakpoint().
	@seealso //leo_ref/c/func/LEODebuggerAddBreakpoint LEODebuggerAddBreakpoint */
void LEORemoteDebuggerRemoveBreakpoint( LEOInstruction* targetInstruction );


#endif // LEO_DEBUGGER_H

