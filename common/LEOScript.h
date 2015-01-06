/*
 *  LEOScript.h
 *  Leonie
 *
 *  Created by Uli Kusterer on 16.10.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#ifndef LEO_SCRIPT_H
#define	LEO_SCRIPT_H		1

/*!
	@header LEOScript
	A script is a reference-counted data type that collects all the methods
	(functions and commands) of an object. After you create a script and
	populate it with handlers, it is intended to be immutable. If you want to
	remove a handler or otherwise change a script afterwards, create a new
	script.
	
	This way, if a script is changed, any still executing instances of that
	script can finish on their own. This is especially needed for scripts that
	modify themselves.
	
	For similar reasons, we reference the owning object using a LEOObjectID and
	LEOObjectSeed, so a script can delete the object owning it and still finish
	executing.
*/

// -----------------------------------------------------------------------------
//	Headers:
// -----------------------------------------------------------------------------

#include "LEOInterpreter.h"
#include "LEOHandlerID.h"


#if __cplusplus
extern "C" {
#endif

#define	DBG_VAR_NAME_SIZE		40

// -----------------------------------------------------------------------------
/*!	This struct is used to keep information for the debugger and for error
	display about each local variable, and each global imported into local
	scope by adding a local variable containing a reference to the global.
	@field variableName			Name of the variable as Leonie sees it
								internally. User-defined variables get a prefix
								so they can't collide with temporaries the
								compiler generates.
	@field realVariableName		The name that the user chose for a variable when
								defining it. Identical to variableName if not a
								user-defined variable.
	@field bpRelativeAddress	The basepointer-relative address of this variable
								on the stack.
	@seealso //leo_ref/c/func/LEOHandlerAddVariableNameMapping LEOHandlerAddVariableNameMapping
	@seealso //leo_ref/c/tdef/LEOHandler LEOHandler */
// -----------------------------------------------------------------------------

typedef struct LEOVariableNameMapping
{
	char			variableName[DBG_VAR_NAME_SIZE];
	char			realVariableName[DBG_VAR_NAME_SIZE];	// Name as the user sees it.
	long			bpRelativeAddress;
} LEOVariableNameMapping;


// -----------------------------------------------------------------------------
/*!	Every method is represented by a struct like this:
	@field handlerName		The name of this handler. Case INsensitive.
	@field numInstructions	The number of instructions in the instructions
							array.
	@field instructions		An array that holds the instructions for this
							handler.
	@seealso //leo_ref/c/func/LEOScriptAddCommandHandlerWithID LEOScriptAddCommandHandlerWithID
	@seealso //leo_ref/c/func/LEOScriptAddFunctionHandlerWithID LEOScriptAddFunctionHandlerWithID
	@seealso //leo_ref/c/func/LEOScriptFindCommandHandlerWithID LEOScriptFindCommandHandlerWithID
	@seealso //leo_ref/c/func/LEOScriptFindFunctionHandlerWithID LEOScriptFindFunctionHandlerWithID
	@seealso //leo_ref/c/func/LEOHandlerAddInstruction LEOHandlerAddInstruction
	@seealso //leo_ref/c/func/LEOHandlerAddVariableNameMapping LEOHandlerAddVariableNameMapping
	@seealso //leo_ref/c/func/LEODebugPrintHandler LEODebugPrintHandler
	@seealso //leo_ref/c/tdef/LEOScript LEOScript */
// -----------------------------------------------------------------------------

typedef struct LEOHandler
{
	LEOHandlerID			handlerName;		// Unique ID of handlers with this name.
	size_t					numInstructions;
	LEOInstruction			*instructions;
	size_t					numVariables;
	LEOVariableNameMapping	*varNames;
} LEOHandler;


// -----------------------------------------------------------------------------
/*! Only the host application can know where in its hierarchy a script resides,
	and to which script unhandled messages should be forwarded. It can provide
	a function with this signature to let the interpreter look up the next
	script in the hierarchy.
	@field inScript		The script whose parent needs to be determined.
	@field inContext	The execution context in which the script is being
						run, with all its globals, references etc.
	@seealso //leo_ref/c/tdef/LEOScript LEOScript
	@seealso //leo_ref/c/tdef/LEOContext LEOContext */
// -----------------------------------------------------------------------------

typedef struct LEOScript*	(*LEOGetParentScriptFuncPtr)( struct LEOScript* inScript, struct LEOContext* inContext );


// -----------------------------------------------------------------------------
/*!	We postpone the reporting of parse errors until someone actually attempts to
	run a script and try to muddle through parsing it until then. These entries
	hold the error position and message so we can report it then.
	The advantage of this is that if e.g. a mouseUp handler has a typo, but the
	mouseWithin handler in the same script is fine, at least the user will not
	get an error message whenever they move the mouse. Also, if both have a typo
	but the mouseUp handler comes first, you won't get the error message for the
	mouseUp handler on mouseWithin. Errors are reported where they occur, as you
	would expect from an interpreter.
	
	@field	errMsg		The error message as the compiler would have reported it.
	@field	errorLine	The line number where the parse error occurred.
	@field	errorOffset	If this isn't SIZE_T_MAX, this is an offset to the actual
						character offset at which the parser choked.
	@field	fileID		The file ID for the file in which this error occurred.
	@seealso //leo_ref/c/func/LEOParseErrorInstruction	LEOParseErrorInstruction
	@seealso //leo_ref/c/func/LEOFileIDForFileName	LEOFileIDForFileName
*/
// -----------------------------------------------------------------------------

typedef struct LEOParseErrorEntry
{
	char*		errMsg;
	size_t		errorLine;
	size_t		errorOffset;
	uint16_t	fileID;
} LEOParseErrorEntry;


// -----------------------------------------------------------------------------
/*!	Every object has a script, which is a grouping of functions and commands:
	@field referenceCount		The number of owners this script currently has.
								Owners are e.g. either contexts running the
								script or the object the script belongs to.
	@field ownerObject			reference to the object that this script belongs
								to, i.e. the 'me' or 'self'.
	@field ownerObjectSeed		The 'seed' for the ownerObject slot, to detect
								the case where the owner has been released and
								the slot re-used.
	@field numFunctions			The number of function handler entries in the
								functions array.
	@field functions			An array of handlers implementing the functions
								this script implements.
	@field numCommands			The number of command handler entries in the
								commands array.
	@field commands				An array of handlers implementing the commands
								this script implements.
	@field numStrings			Number of items in <tt>strings</tt>.
	@field strings				List of string constants in this script, which we
								can load using <tt>ASSIGN_STRING_FROM_TABLE_INSTR</tt>.
	@field GetParentScript		A pointer to a function provided by the host that
								returns a script to which unhandled or passed
								messages will be forwarded, or NULL if there is
								no script behind this one to handle those messages.
	@field numParseErrors		Number of elements in <tt>parseErrors</tt> array.
	@field parseErrors			List of errors for the <tt>PARSE_ERROR_INSTR</tt>
								instruction to refer to.
	@field numBreakpointLines	Number of elements in <tt>breakpointLines</tt> array.
	@field breakpointLines		List of line numbers where the user set a breakpoint,
								for use by the debugger from a <tt>LINE_MARKER_INSTR</tt>
								instruction.
	
	@seealso //leo_ref/c/func/LEOScriptCreateForOwner LEOScriptCreateForOwner
	@seealso //leo_ref/c/func/LEOScriptAddCommandHandlerWithID LEOScriptAddCommandHandlerWithID
	@seealso //leo_ref/c/func/LEOScriptAddFunctionHandlerWithID LEOScriptAddFunctionHandlerWithID
	@seealso //leo_ref/c/func/LEOScriptFindCommandHandlerWithID LEOScriptFindCommandHandlerWithID
	@seealso //leo_ref/c/func/LEOScriptFindFunctionHandlerWithID LEOScriptFindFunctionHandlerWithID
	@seealso //leo_ref/c/func/LEOScriptAddString LEOScriptAddString
	@seealso //leo_ref/c/tdef/LEOGetParentScriptFuncPtr LEOGetParentScriptFuncPtr
	@seealso //leo_ref/c/tdef/LEOScriptAddSyntaxError LEOScriptAddSyntaxError
	@seealso //leo_ref/c/tdef/LEOScriptAddBreakpointAtLine LEOScriptAddBreakpointAtLine
	@seealso //leo_ref/c/tdef/LEOScriptRemoveBreakpointAtLine LEOScriptRemoveBreakpointAtLine
	@seealso //leo_ref/c/tdef/LEOScriptHasBreakpointAtLine LEOScriptHasBreakpointAtLine
	@seealso //leo_ref/c/tdef/LEODebugPrintScript LEODebugPrintScript */
// -----------------------------------------------------------------------------

typedef struct LEOScript
{
	size_t				referenceCount;
	LEOObjectID			ownerObject;		// Deletion-safe reference to owning object.
	LEOObjectSeed		ownerObjectSeed;
	size_t				numFunctions;
	LEOHandler			*functions;
	size_t				numCommands;
	LEOHandler			*commands;
	size_t				numStrings;			// Number of items in stringsTable.
	char**				strings;			// List of string constants in this script, which we can load.
	LEOGetParentScriptFuncPtr	GetParentScript;
	size_t						numParseErrors;		// Number of elements in parseErrors array.
	LEOParseErrorEntry*			parseErrors;		// List of errors for the PARSE_ERROR_INSTR instruction to refer to.
	size_t						numBreakpointLines;	// Number of elements in breakpointLines array.
	size_t					*	breakpointLines;	// List of line numbers where the user set a breakpoint.
} LEOScript;


/*!
	Creates a script referencing the given owner. The LEOScript* is reference-
	counted and its reference count is set to 1, so when you're done with it,
	you must release it. When a script is executing, it should usually be retained
	so that deleting its owner doesn't pull out the rug from under the interpreter.
	@seealso //leo_ref/c/func/LEOScriptRetain LEOScriptRetain
	@seealso //leo_ref/c/func/LEOScriptRelease LEOScriptRelease
*/
LEOScript*	LEOScriptCreateForOwner( LEOObjectID ownerObject, LEOObjectSeed ownerSeed, LEOGetParentScriptFuncPtr inGetParentScriptFunc );	// Gives referenceCount of 1.

/*!
	Acquire ownership of the given script, so that when the current owner releases
	it the object still stays around for your use. Increases the reference count
	by 1.
	@result Returns inScript so you can assign it to a variable in a struct,
			or a global, or whatever makes sense.
	@seealso //leo_ref/c/func/LEOScriptCreateForOwner LEOScriptCreateForOwner
	@seealso //leo_ref/c/func/LEOScriptRelease LEOScriptRelease
*/
LEOScript*	LEOScriptRetain( LEOScript* inScript );		// Adds 1 to referenceCount. Returns inScript.

/*!
	Give up ownership of the given script. You acquire ownership by either
	creating the script, or by retaining it. Giving up ownership decreases its
	reference count by 1. When the last owner releases the object (and the
	reference count reaches 0), the script is freed.
	@seealso //leo_ref/c/func/LEOScriptCreateForOwner LEOScriptCreateForOwner
	@seealso //leo_ref/c/func/LEOScriptRetain LEOScriptRetain
*/
void		LEOScriptRelease( LEOScript* inScript );	// Subtracts 1 from referenceCount. If it hits 0, disposes of inScript.

/*!
	Create a new command handler with the given handler ID and return a pointer to it.
	Use this only when initially setting up a script and parsing/compiling
	bytecode into it. Whenever you add a new handler to a script, all pointers
	to existing LEOHandlers in the script may be invalidated, including the ones
	previously returned by this call.
	
	@seealso //leo_ref/c/func/LEOScriptAddFunctionHandlerWithID LEOScriptAddFunctionHandlerWithID
	@seealso //leo_ref/c/func/LEOHandlerAddInstruction LEOHandlerAddInstruction
	@seealso //leo_ref/c/func/LEOScriptFindCommandHandlerWithID LEOScriptFindCommandHandlerWithID
	@seealso //leo_ref/c/func/LEOContextGroupHandlerIDForHandlerName LEOContextGroupHandlerIDForHandlerName
*/
LEOHandler*	LEOScriptAddCommandHandlerWithID( LEOScript* inScript, LEOHandlerID inHandlerName );	// Invalidates all LEOHandler pointers anyone may have into this script.

/*!
	Create a new function handler with the given handler ID and return a pointer to it.
	Use this only when initially setting up a script and parsing/compiling
	bytecode into it. Whenever you add a new handler to a script, all pointers
	to existing LEOHandlers in the script may be invalidated, including the ones
	previously returned by this call.
	
	@seealso //leo_ref/c/func/LEOScriptAddCommandHandlerWithID LEOScriptAddCommandHandlerWithID
	@seealso //leo_ref/c/func/LEOHandlerAddInstruction LEOHandlerAddInstruction
	@seealso //leo_ref/c/func/LEOScriptFindFunctionHandlerWithID LEOScriptFindFunctionHandlerWithID
	@seealso //leo_ref/c/func/LEOContextGroupHandlerIDForHandlerName LEOContextGroupHandlerIDForHandlerName
*/
LEOHandler*	LEOScriptAddFunctionHandlerWithID( LEOScript* inScript, LEOHandlerID inHandlerName );	// Invalidates all LEOHandler pointers anyone may have into this script.

/*!
	Return a pointer to a command handler with the given handler ID (i.e. handler name).
	
	Returns <tt>NULL</tt> if no such handler exists.
	
	@seealso //leo_ref/c/func/LEOScriptFindFunctionHandlerWithID LEOScriptFindFunctionHandlerWithID
	@seealso //leo_ref/c/func/LEOScriptAddCommandHandlerWithID LEOScriptAddCommandHandlerWithID
	@seealso //leo_ref/c/func/LEOContextGroupHandlerIDForHandlerName LEOContextGroupHandlerIDForHandlerName
*/
LEOHandler*	LEOScriptFindCommandHandlerWithID( LEOScript* inScript, LEOHandlerID inHandlerName );

/*!
	Return a pointer to a function handler with the given handler ID (i.e. handler name).
	
	Returns <tt>NULL</tt> if no such handler exists.
	
	@seealso //leo_ref/c/func/LEOScriptFindCommandHandlerWithID LEOScriptFindCommandHandlerWithID
	@seealso //leo_ref/c/func/LEOScriptAddFunctionHandlerWithID LEOScriptAddFunctionHandlerWithID
	@seealso //leo_ref/c/func/LEOContextGroupHandlerIDForHandlerName LEOContextGroupHandlerIDForHandlerName
*/
LEOHandler*	LEOScriptFindFunctionHandlerWithID( LEOScript* inScript, LEOHandlerID inHandlerName );

/*!
	Add an instruction with the given instruction ID and parameters to a handler.
	Use this only when initially setting up a script and parsing/compiling
	bytecode into it. Pointers to instructions inside a handler may not stay
	valid after a call to this.
	@seealso //leo_ref/c/func/LEOScriptAddCommandHandlerWithID LEOScriptAddCommandHandlerWithID
	@seealso //leo_ref/c/func/LEOScriptAddFunctionHandlerWithID LEOScriptAddFunctionHandlerWithID
*/
void	LEOHandlerAddInstruction( LEOHandler* inHandler, LEOInstructionID instructionID, uint16_t param1, uint32_t param2 );


/*!
	Add an entry to this handler so we can display a name for this variable.
	@param	inHandler	The handler to add this variable name mapping to.
	@param	inName		The unique name that is used internally to identify this variable
						(which e.g. usually includes a prefix for user-defined variables
						so they can't collide with temporaries the compiler generates).
	@param inRealName	The name the user specified and expects displayed back to it.
	@param inBPRelativeAddress	The BasePointer-relative address on the stack for the
								variable of this name.
	@seealso //leo_ref/c/func/LEOHandlerFindVariableByName LEOHandlerFindVariableByName
	@seealso //leo_ref/c/func/LEOHandlerFindVariableByAddress LEOHandlerFindVariableByAddress
	@seealso //leo_ref/c/tdef/LEOVariableNameMapping	LEOVariableNameMapping
*/
void	LEOHandlerAddVariableNameMapping( LEOHandler* inHandler, const char* inName, const char *inRealName, size_t inBPRelativeAddress );


/*!
	Find a variable's name in a handler based on its basePointer-relative offset.
	@seealso //leo_ref/c/func/LEOHandlerFindVariableByName LEOHandlerFindVariableByName
	@seealso //leo_ref/c/func/LEOHandlerAddVariableNameMapping LEOHandlerAddVariableNameMapping
*/
void	LEOHandlerFindVariableByAddress( LEOHandler* inHandler, long bpRelativeAddress, char** outName, char**outRealName, LEOContext* inContext );


/*!
	Find a variable's basePointer-relative offset based on its real name.
	@seealso //leo_ref/c/func/LEOHandlerFindVariableByAddress LEOHandlerFindVariableByAddress
	@seealso //leo_ref/c/func/LEOHandlerAddVariableNameMapping LEOHandlerAddVariableNameMapping
*/
long	LEOHandlerFindVariableByName( LEOHandler* inHandler, const char* inName );


/*!
	Add a string to our strings table, so you can push it on the stack using the
	PUSH_STR_FROM_TABLE_INSTR instruction and operate on it in the script.
	
	@param	inScript	The script to whose strings table you want to add a string.
	@param	inString	The string to be copied to the script's strings table.
	@result the index into the strings table at which the string can now be found.
	
	@seealso //leo_ref/c/func/LEOPushStringFromTableInstruction LEOPushStringFromTableInstruction
*/
size_t	LEOScriptAddString( LEOScript* inScript, const char* inString );


/*!
	Add an error message and position information to our strings table, so it can be
	reported using the PARSE_ERROR_INSTR instruction.
	
	@param	inScript		The script to whose strings table you want to add a string.
	@param	inErrMsg		The error message to be copied to the script's syntax error table.
	@param	inErrorLine		The line number at which the error occurred.
	@param	inErrorOffset	If this isn't SIZE_T_MAX, an actual character offset where the
							parser croaked.
	@field	inFileID		The file where the error occurred.
	@result the index into the syntax error table at which the string can now be found.
	
	@seealso //leo_ref/c/func/LEOParseErrorInstruction LEOParseErrorInstruction
	@seealso //leo_ref/c/func/LEOFileIDForFileName LEOFileIDForFileName
*/
size_t	LEOScriptAddSyntaxError( LEOScript* inScript, const char* inErrMsg, uint16_t inFileID, size_t inErrorLine, size_t inErrorOffset );

/*!
	Add a line number to our list of breakpoints. It is the responsibility of the debugger
	that you use (e.g. LEODebugger.h or LEORemoteDebugger.h) to look at each LINE_MARKER_INSTR
	instruction and verify whether it matches one of these lines, and to trigger accordingly.
	
	This can be used to e.g. implement clicking in the gutter of a script editor window to set
	a breakpoint.
	
	@seealso //leo_ref/c/func/LEOScriptRemoveBreakpointAtLine LEOScriptRemoveBreakpointAtLine
	@seealso //leo_ref/c/func/LEOScriptHasBreakpointAtLine LEOScriptHasBreakpointAtLine
	@seealso //leo_ref/c/func/LEOScriptRemoveAllBreakpoints LEOScriptRemoveAllBreakpoints
*/
size_t	LEOScriptAddBreakpointAtLine( LEOScript* inScript, size_t inLineNumber );

/*!
	Remove a breakpoint from a given line, to undo what LEOScriptAddBreakpointAtLine did.
	
	@seealso //leo_ref/c/func/LEOScriptAddBreakpointAtLine LEOScriptAddBreakpointAtLine
	@seealso //leo_ref/c/func/LEOScriptHasBreakpointAtLine LEOScriptHasBreakpointAtLine
	@seealso //leo_ref/c/func/LEOScriptRemoveAllBreakpoints LEOScriptRemoveAllBreakpoints
*/
void	LEOScriptRemoveBreakpointAtLine( LEOScript* inScript, size_t inLineNumber );


/*!
	Remove all breakpoint for lines in this script to undo what any calls to
	LEOScriptAddBreakpointAtLine did.
	
	@seealso //leo_ref/c/func/LEOScriptAddBreakpointAtLine LEOScriptAddBreakpointAtLine
	@seealso //leo_ref/c/func/LEOScriptRemoveBreakpointAtLine LEOScriptRemoveBreakpointAtLine
	@seealso //leo_ref/c/func/LEOScriptHasBreakpointAtLine LEOScriptHasBreakpointAtLine
*/
void	LEOScriptRemoveAllBreakpoints( LEOScript* inScript );

/*!
	Returns TRUE if a script has been told a breakpoint exists at the given 1-based line number,
	FALSE otherwise.
	
	@seealso //leo_ref/c/func/LEOScriptAddBreakpointAtLine LEOScriptAddBreakpointAtLine
	@seealso //leo_ref/c/func/LEOScriptRemoveBreakpointAtLine LEOScriptRemoveBreakpointAtLine
	@seealso //leo_ref/c/func/LEOScriptRemoveAllBreakpoints LEOScriptRemoveAllBreakpoints
*/
bool	LEOScriptHasBreakpointAtLine( LEOScript* inScript, size_t inLineNumber );


void	LEODebugPrintScript( struct LEOContextGroup* inGroup, LEOScript* inScript );

void	LEODebugPrintHandler( struct LEOContextGroup* inGroup, LEOHandler* inHandler );


#if __cplusplus
}
#endif

#endif // LEO_SCRIPT_H
