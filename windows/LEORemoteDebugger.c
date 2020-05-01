/*
 *  LEORemoteDebugger.c
 *  Leonie
 *
 *  Created by Uli Kusterer on 26.02.19.
 *  Copyright 2019 Uli Kusterer. All rights reserved.
 *
 */

// -----------------------------------------------------------------------------
//	Headers:
// -----------------------------------------------------------------------------

#include "LEORemoteDebugger.h"

bool LEOInitRemoteDebugger(const char* inHostName) { return false; }

void LEORemoteDebuggerPreInstructionProc(struct LEOContext* inContext) {}

void LEORemoteDebuggerPrompt(struct LEOContext* inContext) {}

void	LEORemoteDebuggerAddFile(const char* filecontents, uint16_t inFileID, struct LEOScript* inScript) {}

void LEORemoteDebuggerAddBreakpoint(LEOInstruction* targetInstruction) {}

/*! Remove a breakpoint set using LEODebuggerAddBreakpoint().
	@seealso //leo_ref/c/func/LEODebuggerAddBreakpoint LEODebuggerAddBreakpoint */
void LEORemoteDebuggerRemoveBreakpoint(LEOInstruction* targetInstruction) {}
