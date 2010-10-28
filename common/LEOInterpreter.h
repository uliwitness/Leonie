/*
 *  LEOInterpreter.h
 *  Leonie
 *
 *  Created by Uli Kusterer on 17.09.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

/*!
	@header LEOInterpreter
	This is the core interpreter logic for the Leonie bytecode interpreter.
	
	To compile Leonie bytecode, create a LEOScript, add LEOHandlers to it and
	add LEOInstructions to those using the constants in LEOInstructions.h
	for the instruction IDs.
	
	To run Leonie bytecode, create LEOContextGroup, then create a LEOContext
	using LEOInitContext() that references this group. You can now release the
	context group, unless you want to keep using it, the context will have
	retained it. To execute the bytecode, call LEORunInContext(). When the call
	returns, call LEOCleanUpContext() to free all associated data again.
	
	@seealso //leo_ref/c/tdef/LEOInstruction LEOInstruction
	@seealso //leo_ref/c/tdef/LEOContext LEOContext
	@seealso //leo_ref/c/func/LEOInitContext	LEOInitContext
	@seealso //leo_ref/c/func/LEORunInContext	LEORunInContext
	@seealso //leo_ref/c/func/LEOCleanUpContext	LEOCleanUpContext
	@seealso //leo_ref/c/func/LEOContextGroupCreate	LEOContextGroupCreate
	@seealso //leo_ref/c/func/LEOContextGroupRelease	LEOContextGroupRelease
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

/*! How many LEOValues can be on the stack before we run out of stack space. */
#define LEO_STACK_SIZE			1024


// -----------------------------------------------------------------------------
//	Types:
// -----------------------------------------------------------------------------

/*! Index into our instruction function array. Index 0 contains the
	"unimplemented" error exit function. */
typedef uint16_t	LEOInstructionID;


/*! All instructions are implemented as functions with the following signature:
	Apart from branching instructions, every instruction has to increment the
	inContext->currentInstruction so it'll advance to the next instruction. The
	instruction functions are looked up in the gInstructions array. */
typedef void (*LEOInstructionFuncPtr)( struct LEOContext* inContext );


/*! This is how an instruction looks in bytecode. It has the instruction's ID
	as the first item, and then one 16-bit and one 32-bit argument that depends
	on the particular instruction. */
typedef struct LEOInstruction
{
	LEOInstructionID		instructionID;
	uint16_t				param1;
	uint32_t				param2;
} LEOInstruction;


// Data type used internally to be able to show the call stack to the user and
//	to look up handlers in the current script, even if the owning object is now
//	gone.
typedef struct LEOCallStackEntry
{
	struct LEOScript*	script;		// The script that owns 'handler'. The script in here should be retained/released to make sure it doesn't go away.
	struct LEOHandler*	handler;	// The current handler, so we can show a nice call stack.
} LEOCallStackEntry;


/*! A LEOContext encapsulates all execution state needed to run bytecode. Speaking
	in CPU terms, it encapsulates the registers, the call stack, and a few
	thread-globals. Hence, each thread in which you want to run bytecode needs
	its own LEOContext.
	@field	group				The LEOContextGroup that collects the global state
								shared between this context and any others in
								its group.
	@field	keepRunning			boolean that instructions like ExitToTop can set
								to FALSE to stop execution of the script. Also used
								when script errors occur.
	@field	errMsg				Error message to display when keepRunning has
								been set to FALSE.
	@field	stringsTable		List of string constants in this script, which we can load.
	@field	stringsTableSize	Number of items in stringsTable.
	@field	itemDelimiter		The delimiter to use for the "item" chunk expression. Defaults to comma (',').
	@field	preInstructionProc	A function to call on each instruction before it
								is executed. Useful as a hook-up-point for a debugger,
								or to process events while a script is running.
	@field	numSteps			Used by LEODebugger's PreInstructionProc to implement single-stepping.
	@field	currentInstruction	The instruction currently being executed. Essentially the Program Counter of our virtual CPU.
	@field	stackBasePtr		Base pointer into stack, used during function calls to find parameters & start of local variable section.
	@field	stackEndPtr			Stack pointer indicating used size of our stack. Always points at element after last element.
	@field	stack				The stack containing all our local variables, parameters etc.
								
	@seealso //leo_ref/c/tag/LEOValueReference LEOValueReference
	@seealso //leo_ref/c/tdef/LEOValuePtr LEOValuePtr
	@seealso //leo_ref/c/tdef/LEOContextGroup LEOContextGroup
	@seealso //leo_ref/c/tdef/LEOInstruction LEOInstruction */

typedef struct LEOContext
{
	struct LEOContextGroup	*group;					// The group this context belongs to, containing its global state, references etc.
	bool					keepRunning;			// ExitToShell and errors set this to FALSE to stop interpreting of code.
	char					errMsg[1024];			// Error message to display when keepRunning has been set to FALSE.
	const char**			stringsTable;			// List of string constants in this script, which we can load.
	size_t					stringsTableSize;		// Number of items in stringsTable.
	char					itemDelimiter;			// item delimiter to use for chunk expressions in values.
	size_t					numCallStackEntries;	// Number of items in callStackEntries.
	LEOCallStackEntry*		callStackEntries;		// Array of call stack entries to allow showing a simple backtrace and picking handlers from the current script.
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

/*! Initialize the given LEOContext so its instance variables are valid.
	The context is added to the provided group, and will retain the context
	group until the context is cleaned up. So once you have added the context
	to the context group, you can release it if you don't intend to create any
	other contexts in that group directly.
	@seealso //leo_ref/c/func/LEOCleanUpContext LEOCleanUpContext
	@seealso //leo_ref/c/func/LEOContextGroupCreate	LEOContextGroupCreate
	@seealso //leo_ref/c/func/LEOContextGroupRelease LEOContextGroupRelease
*/
void	LEOInitContext( LEOContext* theContext, struct LEOContextGroup* inGroup );

/*! Shorthand for LEOPrepareContextForRunning and a loop of LEOContinueRunningContext.
	@seealso //leo_ref/c/func/LEOPrepareContextForRunning LEOPrepareContextForRunning
	@seealso //leo_ref/c/func/LEOContinueRunningContext LEOContinueRunningContext
*/
void	LEORunInContext( LEOInstruction instructions[], LEOContext *inContext );

/*! Set the currentInstruction of the given LEOContext to the given instruction 
	array's first instruction, and initialize the Base pointer and stack end pointer
	and keepRunning etc.
	@seealso //leo_ref/c/func/LEORunInContext LEORunInContext
	@seealso //leo_ref/c/func/LEOContinueRunningContext LEOContinueRunningContext
*/
void	LEOPrepareContextForRunning( LEOInstruction instructions[], LEOContext *inContext );

/*! Execute the next instruction in the context. Returns false if the code has
	finished executing or exited with an error.
	@seealso //leo_ref/c/func/LEORunInContext LEORunInContext
	@seealso //leo_ref/c/func/LEOPrepareContextForRunning LEOPrepareContextForRunning
*/
bool	LEOContinueRunningContext( LEOContext *inContext );

// Used internally to unwind the stack and ensure values get destructed correctly.
void	LEOCleanUpStackToPtr( LEOContext* theContext, union LEOValue* lastItemToDelete );

/*! Dispose of the given context's associated data structures once you're
	finished, and release the reference to its context group that the context
	holds.
	@seealso //leo_ref/c/func/LEOInitContext LEOInitContext
*/
void	LEOCleanUpContext( LEOContext* theContext );


/*! Print the given instruction to the console for debugging purposes.
	@seealso //leo_ref/c/func/LEODebugPrintInstructions	LEODebugPrintInstructions
*/
void	LEODebugPrintInstr( LEOInstruction* instruction );

/*! Print the given array of instructions to the console for debugging purposes using LEODebugPrintInstr.
	@seealso //leo_ref/c/func/LEODebugPrintInstr	LEODebugPrintInstr
*/
void	LEODebugPrintInstructions( LEOInstruction instructions[], size_t numInstructions );

/*! Print the given context to the console for debugging purposes.
	@seealso //leo_ref/c/func/LEODebugPrintInstr	LEODebugPrintInstr
	@seealso //leo_ref/c/func/LEOContextDebugPrintCallStack	LEOContextDebugPrintCallStack
*/
void	LEODebugPrintContext( LEOContext* ctx );

/*!
	Print the current call stack of the given context to the console for
	debugging purposes.
*/
void	LEOContextDebugPrintCallStack( LEOContext* inContext );


void				LEOContextPushHandlerAndScript( LEOContext* inContext, struct LEOHandler* inHandler, struct LEOScript* inScript );
struct LEOHandler*	LEOContextPeekCurrentHandler( LEOContext* inContext );
struct LEOScript*	LEOContextPeekCurrentScript( LEOContext* inContext );
void				LEOContextPopHandlerAndScript( LEOContext* inContext );



#endif // LEO_INTERPRETER_H
