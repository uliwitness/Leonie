/*
 *  LEODebugger.h
 *  Leonie
 *
 *  Created by Uli Kusterer on 20.09.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include "LEOInterpreter.h"

// Use this function as the PreInstructionProc of your context to activate the debugger:
void LEODebuggerPreInstructionProc( struct LEOContext* inContext );

// Yes, we can do simple breakpoints:
void LEODebuggerAddBreakpoint( LEOInstruction* targetInstruction );
void LEODebuggerRemoveBreakpoint( LEOInstruction* targetInstruction );