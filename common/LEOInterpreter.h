/*
 *  LEOInterpreter.h
 *  Leonie
 *
 *  Created by Uli Kusterer on 17.09.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

/*!
	@header LEOInterpreter (Public)
	This is the core interpreter logic for the Leonie bytecode interpreter.
	
	To compile Leonie bytecode, generate an array of LEOInstruction entries and
	use the constants in LEOInstructions.h for the instruction IDs.
	
	To run Leonie bytecode, create a LEOContext using LEOInitContext(), then
	call LEORunInContext() to execute the bytecode. When the call returns, call
	LEOCleanUpContext() to free associated data again.
	
	@seealso //leo_ref/c/tdef/LEOInstruction LEOInstruction
	@seealso //leo_ref/c/tdef/LEOContext LEOContext
	@seealso //leo_ref/c/func/LEOInitContext	LEOInitContext
	@seealso //leo_ref/c/func/LEORunInContext	LEORunInContext
	@seealso //leo_ref/c/func/LEOCleanUpContext	LEOCleanUpContext
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


/*! What a LEOObjectID refers to, used by reference values. These are kept in a
	big array of "master pointers" named "references" in the LEOContext.
	@field	value	The actual pointer to the referenced value. NULL for unused object entries.
	@field	seed	Whenever a referenced object entry is re-used, this seed is incremented, so people still referencing it know they're wrong.
	@seealso //leo_ref/c/tag/LEOValueReference LEOValueReference */
typedef struct LEOObject	// What a LEOObjectID refers to. These are kept in a big array of "master pointers" in the context.
{
	LEOValuePtr		value;	// NULL for unused object entries.
	LEOObjectSeed	seed;	// Whenever a referenced object entry is re-used, this seed is incremented, so people still referencing it know they're wrong.
} LEOObject;


/*! A LEOContext encapsulates all execution state needed to run bytecode. Speaking
	in CPU terms, it encapsulates the registers, the call stack, and a few
	thread-globals. Hence, each thread in which you want to run bytecode needs
	its own LEOContext.
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
	@field	references			An array of "master pointers" to values to which references have been created.
	@field	numReferences		Number of items in the <tt>references</tt>.
	@field	stackBasePtr		Base pointer into stack, used during function calls to find parameters & start of local variable section.
	@field	stackEndPtr			Stack pointer indicating used size of our stack. Always points at element after last element.
	@field	stack				The stack containing all our local variables, parameters etc.
								
	@seealso //leo_ref/c/tag/LEOValueReference LEOValueReference
	@seealso //leo_ref/c/tdef/LEOValuePtr LEOValuePtr */

typedef struct LEOContext
{
	bool					keepRunning;			// ExitToShell and errors set this to FALSE to stop interpreting of code.
	char					errMsg[1024];			// Error message to display when keepRunning has been set to FALSE.
	const char**			stringsTable;			// List of string constants in this script, which we can load.
	size_t					stringsTableSize;		// Number of items in stringsTable.
	char					itemDelimiter;
	LEOInstructionFuncPtr	preInstructionProc;		// For each instruction, this function gets called, to let you do idle processing, hook in a debugger etc. This should NOT be an instruction, as that would advance the PC and screw up the call of the actual instruction.
	size_t					numSteps;				// Used by LEODebugger's PreInstructionProc to implement single-stepping.
	LEOInstruction			*currentInstruction;	// PC
	LEOObject				*references;			// "Master pointer" table for references so we can detect when a reference goes away.
	size_t					numReferences;			// Available slots in "references" array.
	union LEOValue			*stackBasePtr;			// BP
	union LEOValue			*stackEndPtr;			// SP (always points at element after last element)
	union LEOValue			stack[LEO_STACK_SIZE];	// The stack.
} LEOContext;


// -----------------------------------------------------------------------------
//	Prototypes:
// -----------------------------------------------------------------------------

/*! Initialize the given LEOContext so its instance variables are valid. */
void	LEOInitContext( LEOContext* theContext );

/*! Shorthand for LEOPrepareContextForRunning and a loop of LEOContinueRunningContext. */
void	LEORunInContext( LEOInstruction instructions[], LEOContext *inContext );

/*! Set the currentInstruction of the given LEOContext to the given instruction 
	array's first instruction, and initialize the Base pointer and stack end pointer
	and keepRunning etc. */
void	LEOPrepareContextForRunning( LEOInstruction instructions[], LEOContext *inContext );

/*! Execute the next instruction in the context. Returns false if the code has
	finished executing or exited with an error. */
bool	LEOContinueRunningContext( LEOContext *inContext );

// Used internally to unwind the stack and ensure values get destructed correctly.
void	LEOCleanUpStackToPtr( LEOContext* theContext, union LEOValue* lastItemToDelete );

/*! Dispose of the given context's associated data structures once you're finished. */
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
*/
void	LEODebugPrintContext( LEOContext* ctx );


// Used to implement references to values that can disappear:
LEOObjectID	LEOCreateNewObjectIDForValue( LEOValuePtr theValue, struct LEOContext* inContext );
void		LEORecycleObjectID( LEOObjectID inObjectID, struct LEOContext* inContext );
LEOValuePtr	LEOGetValueForObjectIDAndSeed( LEOObjectID inObjectID, LEOObjectSeed inObjectSeed, struct LEOContext* inContext );


#endif // LEO_INTERPRETER_H
