/*
 *  LEOInterpreter.h
 *  Leonie
 *
 *  Created by Uli Kusterer on 17.09.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#ifndef LEO_INTERPRETER_H
#define LEO_INTERPRETER_H		1

// -----------------------------------------------------------------------------
//	Headers:
// -----------------------------------------------------------------------------

#include "LEOValue.h"
#include <stdint.h>


// -----------------------------------------------------------------------------
//	Constants:
// -----------------------------------------------------------------------------

#define LEO_STACK_SIZE			1024


// -----------------------------------------------------------------------------
//	Types:
// -----------------------------------------------------------------------------

typedef uint16_t	LEOInstructionID;	// Index into our instruction function array. Index 0 contains the "unimplemented" error exit function.


// All instructions are implemented as functions with the following signature:
//	Apart from branching instructions, every instruction has to increment the
//	inContext->currentInstruction so it'll advance to the next instruction. The
//	instruction functions are looked up in the gInstructions array.
typedef void (*LEOInstructionFuncPtr)( struct LEOContext* inContext );


typedef struct LEOInstruction
{
	LEOInstructionID		instructionID;
	uint16_t				param1;
	uint32_t				param2;
} LEOInstruction;


typedef struct LEOContext
{
	bool					keepRunning;			// ExitToShell and errors set this to TRUE to stop interpreting of code.
	char					errMsg[1024];			// Error message to display when keepRunning has been set to FALSE.
	const char**			stringsTable;			// List of string constants in this script, which we can load.
	size_t					stringsTableSize;		// Number of items in stringsTable.
	LEOInstructionFuncPtr	preInstructionProc;		// For each instruction, this function gets called, to let you do idle processing, hook in a debugger etc. This should NOT be an instruction, as that would advance the PC and screw up the call of the actual instruction.
	size_t					numSteps;				// Used by LEODebugger's PreInstructionProc to implement single-stepping.
	LEOInstruction			*currentInstruction;	// PC
	union LEOValue			*stackBasePtr;			// BP
	union LEOValue			*stackEndPtr;			// SP (always points at element after last element)
	union LEOValue			stack[LEO_STACK_SIZE];	// The stack.
} LEOContext;


// -----------------------------------------------------------------------------
//	Prototypes:
// -----------------------------------------------------------------------------

void	LEOInitContext( LEOContext* theContext );
void	LEORunInContext( LEOInstruction instructions[], LEOContext *inContext );	// Shorthand for LEOPrepareContextForRunning and a loop of LEOContinueRunningContext
void	LEOPrepareContextForRunning( LEOInstruction instructions[], LEOContext *inContext );
bool	LEOContinueRunningContext( LEOInstruction instructions[], LEOContext *inContext );	// returns false if the code has finished executing or exited with an error.
void	LEOCleanUpStackToPtr( LEOContext* theContext, union LEOValue* lastItemToDelete );
void	LEOCleanUpContext( LEOContext* theContext );

void	LEODebugPrintInstr( LEOInstruction* instruction );
void	LEODebugPrintInstructions( LEOInstruction instructions[], size_t numInstructions );
void	LEODebugPrintContext( LEOContext* ctx );

#endif // LEO_INTERPRETER_H
