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
	@unsorted
	This is the core interpreter logic for the Leonie bytecode interpreter.
	
	To compile Leonie bytecode, create a LEOScript, add LEOHandlers to it and
	add LEOInstructions to those using the constants in LEOInstructions.h
	for the instruction IDs.
	
	To run Leonie bytecode, create a LEOContextGroup, then create a LEOContext
	using LEOContextCreate() that references this group. You can now release the
	context group, unless you want to keep using it, the context will have
	retained it. To execute the bytecode, call LEORunInContext(). When the call
	returns, call LEOCleanUpContext() to free all associated data again.
	
	@seealso //leo_ref/c/tdef/LEOInstruction LEOInstruction
	@seealso //leo_ref/c/tdef/LEOContext LEOContext
	@seealso //leo_ref/c/func/LEOContextCreate	LEOContextCreate
	@seealso //leo_ref/c/func/LEORunInContext	LEORunInContext
	@seealso //leo_ref/c/func/LEOContextRelease	LEOContextRelease
	@seealso //leo_ref/c/func/LEOContextGroupCreate	LEOContextGroupCreate
	@seealso //leo_ref/c/func/LEOContextGroupRelease	LEOContextGroupRelease
*/

#ifndef LEO_INTERPRETER_H
#define LEO_INTERPRETER_H		1

#if __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//	Headers:
// -----------------------------------------------------------------------------

#include "LEOValue.h"
#include "LEOHandlerID.h"
#include "LEOUserData.h"
#include <stdint.h>
#include <assert.h>


// -----------------------------------------------------------------------------
//	Constants:
// -----------------------------------------------------------------------------

/*! How many LEOValues can be on the stack before we run out of stack space. */
#define LEO_STACK_SIZE			1024

/*!
	Pass this as param1 to some instructions that take a
	basePtr-relative address to make it pop the last
	value off the stack instead of reading a value from that
	address. Since they reinterpret the number as an int16_t, it needs to be
	a value in the middle of the unsigned range.
*/
#define BACK_OF_STACK			((uint16_t) INT16_MIN)


typedef enum
{
	EMayGoUnhandled,	//!< If there is no handler for this message, just quietly ignore it.
	EMustBeHandled		//!< Present an error if this message is not handled.
} TMayGoUnhandledFlag;


// -----------------------------------------------------------------------------
//	Types:
// -----------------------------------------------------------------------------

/*! Index into our instruction function array. Index 0 contains the
	"unimplemented" error exit function.
	
	Note that instruction IDs are sequential and not intended to be saved to
	disk. If you want to save bytecode to disk, you should save a look-up-table
	from some persistent instruction opcode along to the file and then when
	loading fix up all the instruction IDs based on that name. */
typedef uint16_t	LEOInstructionID;


/*! All instructions are implemented as functions with the following signature:
	Apart from branching instructions, every instruction has to increment the
	inContext->currentInstruction so it'll advance to the next instruction. The
	instruction functions are looked up in the gInstructions array. */
typedef void (*LEOInstructionFuncPtr)( struct LEOContext* inContext );


/*! If a handler call can't find the handler in the current script and no parent
	for that script exists that handles the call, this function is called to
	allow the host application to provide default behaviour for unhandled messages. */
typedef void (*LEONonexistentHandlerFuncPtr)( struct LEOContext* inContext, LEOHandlerID inHandler, TMayGoUnhandledFlag mayGoUnhandled );


/*! This function is called when a context finishes execution, either
	because of an error, or because the outermost function finished
	executing. You can now look at the return value or pass the message
	on to the next object in the chain. */
typedef void (*LEOContextCompletionFuncPtr)( struct LEOContext* inContext );


/*! A LEOInstruction is how an instruction looks in bytecode. It has the
	instruction's ID as the first item, and then one 16-bit and one 32-bit
	argument that depends on the particular instruction. */
typedef struct LEOInstruction
{
	LEOInstructionID		instructionID;
	uint16_t				param1;
	uint32_t				param2;
} LEOInstruction;


/*!	A LEOInstructionEntry is how we look up the function associated with an
	instruction ID. We keep a master table of all instructions we know, and you
	can add your own instructions by simply adding to that list. You
	generally do not use this directly. Instead, you use the LEOINSTR_START,
	LEOINSTR, and LEOINSTR_LAST macros below to declare a global array:
	
		LEOINSTR_START(FrobnitzWebService,NUM_WEBSERVICE_INSTRUCTIONS)
		LEOINSTR(MyWebServiceStartInstruction)
		LEOINSTR_LAST(MyWebServiceStopInstruction)
	
	and pass that to LEOAddInstructionsToInstructionArray:
	
		LEOInstructionID outFirstNewInstruction = 0;
		LEOAddInstructionsToInstructionArray( gFrobnitzWebServiceInstructions,
						NUM_WEBSERVICE_INSTRUCTIONS, &outFirstNewInstruction );
	
	Assuming you've declared your instruction IDs in an enum like this:
	
		enum
		{
			MY_WEBSERVICE_START_INSTR = 0,
			MY_WEBSERVICE_STOP_INSTR,
			NUM_WEBSERVICE_INSTRUCTIONS
		};
	
	The instruction IDs to generate can now be calculated like:
	
		LEOHandlerAddInstruction( theHandler, outFirstNewInstruction +MY_WEB_SERVICE_START_INSTR, 0, 0 );
	
	or the likes. */
struct LEOInstructionEntry
{
	LEOInstructionFuncPtr		proc;	//! The function to call for this instruction
	const char*					name;	//! String representation of the function's name. *must be* a constant or otherwise guaranteed to stay around as long as Leonie needs it.
};


#define LEOINSTR_DECL(n,c)	extern struct LEOInstructionEntry	g ## n ##Instructions[c];
#define LEOINSTR_START(n,c)	struct LEOInstructionEntry	g ## n ##Instructions[c] = {
#define LEOINSTR(n)				{ n, #n }, 
#define LEOINSTR_LAST(n)		{ n, #n } };
#define LEOINSTR_DUMMY(n)		{ LEOInvalidInstruction, #n },
#define LEOINSTR_DUMMY_LAST(n)	{ LEOInvalidInstruction, #n } };

/*! @functiongroup Static typecasting functions */
/*! Reinterpret the given unsigned uint32_t as a signed int32_t. E.g. useful for an instruction's param2 field. */
static inline int32_t		LEOCastUInt32ToInt32( uint32_t inNum )		{ return *(int32_t*)&inNum; }

/*! Reinterpret the given unsigned uint16_t as a signed int16_t. E.g. useful for an instruction's param1 field. */
static inline int16_t		LEOCastUInt16ToInt16( uint16_t inNum )		{ return *(int16_t*)&inNum; }

/*! Reinterpret the given unsigned uint32_t as a LEONumber floating point quantity. E.g. useful for an instruction's param2 field. */
static inline LEONumber	LEOCastUInt32ToLEONumber( uint32_t inNum )	{ assert(sizeof(LEONumber) <= sizeof(uint32_t));  return *(LEONumber*)&inNum; }


/*! Call this method once before calling any other interpreter functions. This initializes
	the instruction look-up table with the built-in instructions. */
void	LEOInitInstructionArray( void );

/*! Use this method to register new instructions for your host application. The new
	entries will be appended to the built-in instruction array (and your array copied),
	and the index of your first instruction will be returned in outFirstNewInstruction.
	You can now add this value to your instruction indices to calculate the actual
	LEOInstructionID you need to generate Leonie bytecode.
	
	See the <tt>LEOInstructionEntry</tt> documentation on information on how to most easily
	declare your instruction array. */
void	LEOAddInstructionsToInstructionArray( struct LEOInstructionEntry *inInstructionArray, LEOInstructionID inNumInstructions, LEOInstructionID *outFirstNewInstruction );



/*! @functiongroup LEOContext methods */

// Data type used internally to be able to show the call stack to the user and
//	to look up handlers in the current script, even if the owning object is now
//	gone.
typedef struct LEOCallStackEntry
{
	struct LEOScript*	script;			// The script that owns 'handler'. The script in here should be retained/released to make sure it doesn't go away.
	struct LEOHandler*	handler;		// The current handler, so we can show a nice call stack.
	LEOInstruction*		returnAddress;	// Instruction at which we are to continue when this handler returns.
	LEOValuePtr			oldBasePtr;		// The base pointer relative to which we calculate our parameters' and local variables' addresses.
} LEOCallStackEntry;


/*! @enum Possible values for the 'flags' field in the LEOContext. */
enum
{
	kLEOContextKeepRunning	= (1 << 0),	//! Clear this bit to stop script execution. Used on errors and for ExitToTop.
	kLEOContextPause		= (1 << 1),	//! Set by the current instruction when it wants to pause the current context (e.g. to perform some async tasks which should appear synchronous to scripts). The instruction should not advance the PC until the context is resumed, which it can detect by looking at the kLEOContextResuming flag.
	kLEOContextResuming		= (1 << 2)	//! Context was just resumed from being paused. The current instruction can now finish its work, advance the PC and return.
};
typedef uint32_t	LEOContextFlags;


/*! A LEOContext encapsulates all execution state needed to run bytecode. Speaking
	in CPU terms, it encapsulates the registers, the call stack, and a few
	thread-globals. Hence, each thread in which you want to run bytecode needs
	its own LEOContext.
	@field	group				The LEOContextGroup that collects the global state
								shared between this context and any others in
								its group.
	@field	flags				flags that let you pause or exit scripts.
	@field	errMsg				Error message to display when the kLEOContextKeepRunning flag has
								been set to FALSE. If this is an empty string, it was a benign exit.
	@field	itemDelimiter		The delimiter to use for the "item" chunk expression. Defaults to comma (',').
	@field	preInstructionProc	A function to call on each instruction before it
								is executed. Useful as a hook-up-point for a debugger,
								or to process events while a script is running (e.g. user cancellation).
	@field	callNonexistentHandlerProc	When a handler is called that doesn't exist, and a script has no parent, this function is called (e.g. to display an error message or call an XCMD or equivalent third-party plugin).
	@field	numSteps			Used by LEODebugger's PreInstructionProc to implement single-stepping.
	@field	currentInstruction	The instruction currently being executed. Essentially the Program Counter of our virtual CPU.
	@field	stackBasePtr		Base pointer into stack, used during function calls to find parameters & start of local variable section.
	@field	stackEndPtr			Stack pointer indicating used size of our stack. Always points at element after last element.
	@field	stack				The stack containing all our local variables, parameters etc.
	@field	userData			A pointer for use by the client, for storing additional information in.
	@field	cleanUpUserData		A function that will be passed the userData pointer and called right before we free the context's memory, which you can provide to free any memory used by the userData.
	@field	contextCompleted	A function that is called once execution aborts due to an error or the outermost function completes. It can retrieve result values from the context and free its memory.
	
	@seealso //leo_ref/c/tag/LEOValueReference LEOValueReference
	@seealso //leo_ref/c/tdef/LEOValuePtr LEOValuePtr
	@seealso //leo_ref/c/tdef/LEOContextGroup LEOContextGroup
	@seealso //leo_ref/c/tdef/LEOInstruction LEOInstruction */

typedef struct LEOContext
{
	size_t							referenceCount;
	struct LEOContextGroup	*		group;					// The group this context belongs to, containing its global state, references etc.
	LEOContextFlags					flags;					// But flags for flow control etc.
	char							errMsg[1024];			// Error message to display when kLEOContextKeepRunning flag has been set to FALSE.
	size_t							errLine;				// Line on which the error occurred. Or SIZE_MAX if we don't know.
	size_t							errOffset;				// Byte offset in script at which the error occurred, or SIZE_MAX if we don't know. If this isn't present, errLine might still be set.
	uint16_t						errFileID;				// ID of the file in which the error occurred.
	char							itemDelimiter;			// item delimiter to use for chunk expressions in values.
	size_t							numCallStackEntries;	// Number of items in callStackEntries.
	LEOCallStackEntry		*		callStackEntries;		// Array of call stack entries to allow showing a simple backtrace and picking handlers from the current script.
	LEOInstructionFuncPtr			preInstructionProc;		// For each instruction, this function gets called, to let you do idle processing, hook in a debugger etc. This should NOT be an instruction, as that would advance the PC and screw up the call of the actual instruction.
	LEOInstructionFuncPtr			promptProc;				// On certain errors, this function is called to enter into the debugger prompt.
	LEONonexistentHandlerFuncPtr	callNonexistentHandlerProc;	// When a handler is called that doesn't exist, and a script has no parent, this function is called (e.g. to display an error message or call an XCMD).
	size_t							numSteps;				// Used by LEODebugger's PreInstructionProc to implement single-stepping.
	LEOInstruction			*		currentInstruction;		// PC
	union LEOValue			*		stackBasePtr;			// BP
	union LEOValue			*		stackEndPtr;			// SP (always points at element after last element)
	union LEOValue					stack[LEO_STACK_SIZE];	// The stack.
	void*							userData;
	LEOUserDataCleanUpFuncPtr		cleanUpUserData;
	LEOContextCompletionFuncPtr		contextCompleted;
} LEOContext;


// -----------------------------------------------------------------------------
//	Prototypes:
// -----------------------------------------------------------------------------

/*! Create a new LEOContext.
	The context is added to the provided group, and will retain the context
	group until the context is released by its last client. So once you have
	added the context to the context group, you can release it if you don't
	intend to create any other contexts in that group directly.
	The context starts out with a reference count of 1. When you are done with
	it, release it using LEOContextRelease().
	@seealso //leo_ref/c/func/LEOContextRelease LEOContextRelease
	@seealso //leo_ref/c/func/LEOContextRetain LEOContextRetain
	@seealso //leo_ref/c/func/LEOContextGroupCreate	LEOContextGroupCreate
	@seealso //leo_ref/c/func/LEOContextGroupRelease LEOContextGroupRelease
*/
LEOContext*	LEOContextCreate( struct LEOContextGroup* inGroup, void* inUserData,
								LEOUserDataCleanUpFuncPtr inCleanUpFunc );

/*! Release your reference to this context. If you were the last owner of this
	context, disposes of the given context's associated data structures
	and releases the reference to its context group that the context
	holds.
	@seealso //leo_ref/c/func/LEOContextCreate LEOContextCreate
	@seealso //leo_ref/c/func/LEOContextCreate LEOContextRetain
*/
void		LEOContextRelease( LEOContext* inContext );

/*! Acquire a reference to a context. Use this if, for example, you are
	passing LEOContext objects between functions, or otherwise sharing
	ownership of a context between several objects. Once you are done with
	the context, call LEOContextRelease on it to give up your reference.
	@seealso //leo_ref/c/func/LEOContextCreate LEOContextCreate
	@seealso //leo_ref/c/func/LEOContextRelease LEOContextRelease
*/
LEOContext*	LEOContextRetain( LEOContext* inContext );	// returns inContext.

/*! Shorthand for LEOPrepareContextForRunning and a loop of LEOContinueRunningContext.
	@seealso //leo_ref/c/func/LEOPrepareContextForRunning LEOPrepareContextForRunning
	@seealso //leo_ref/c/func/LEOContinueRunningContext LEOContinueRunningContext
*/
void	LEORunInContext( LEOInstruction instructions[], LEOContext *inContext );


/*! Queues up a paused context for resumption of execution the next time
	LEOContextResumeIfAvailable is called.
	@seealso //leo_ref/c/func/LEORunInContext LEORunInContext
	@seealso //leo_ref/c/func/LEOPauseContext LEOPauseContext
	@seealso //leo_ref/c/func/LEOContextResumeIfAvailable LEOContextResumeIfAvailable
*/
void	LEOResumeContext( LEOContext *inContext );


/*! An instruction can call this to cause the given context so you can do some
	async work, then later come back to it with LEOResumeContext.
	@seealso //leo_ref/c/func/LEOResumeContext LEOResumeContext
*/
void	LEOPauseContext( LEOContext *inContext );


/*!
	@seealso //leo_ref/c/func/LEOResumeContext LEOResumeContext
	@seealso //leo_ref/c/func/LEOPauseContext LEOPauseContext
*/
void	LEOContextResumeIfAvailable( void );


/*! Set the currentInstruction of the given LEOContext to the given instruction 
	array's first instruction, and initialize the Base pointer and stack end pointer
	and flags etc.
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

/*! Stop execution in the given context with an error message.
	Currently sets the errMsg field of the context to the given string and set
	keepRunning to FALSE. inErrorFmt is a format string like you pass it to
    printf().
 */
void	LEOContextStopWithError( LEOContext* inContext, size_t errLine, size_t errOffset, uint16_t fileID, const char* inErrorFmt, ... );

    
/*! Look up the local variable with name varName in the current script, if present, and set
    its value to the given string. inMessageFmt is a format string like you pass it to
    printf().
*/
void	LEOContextSetLocalVariable( LEOContext* inContext, const char* varName, const char* inMessageFmt, ... );

/*! Push a copy of the given value onto the stack, returning a pointer to it.
 @seealso //leo_ref/c/func/LEOCleanUpStackToPtr LEOCleanUpStackToPtr
 @seealso //leo_ref/c/func/LEOPushIntegerOnStack LEOPushIntegerOnStack
 @seealso //leo_ref/c/func/LEOPushNumberOnStack LEOPushNumberOnStack
 @seealso //leo_ref/c/func/LEOPushBooleanOnStack LEOPushBooleanOnStack
 @seealso //leo_ref/c/func/LEOPushEmptyValueOnStack LEOPushEmptyValueOnStack
 @seealso //leo_ref/c/func/LEOPushStringValueStack LEOPushStringValueStack
 @seealso //leo_ref/c/func/LEOPushStringConstantValueOnStack LEOPushStringConstantValueOnStack
 */
LEOValuePtr	LEOPushValueOnStack( LEOContext* theContext, LEOValuePtr inValueToCopy );

/*! Push a copy of the given integer onto the stack, returning a pointer to it.
	This is a shorthand for LEOPushValueOnStack and the corresponding LEOInitXXValue call.
 @seealso //leo_ref/c/func/LEOCleanUpStackToPtr LEOCleanUpStackToPtr
 @seealso //leo_ref/c/func/LEOPushValueOnStack LEOPushValueOnStack
 */
LEOValuePtr	LEOPushIntegerOnStack( LEOContext* theContext, LEOInteger inInteger, LEOUnit inUnit );


/*! Push a copy of the given number onto the stack, returning a pointer to it.
	This is a shorthand for LEOPushValueOnStack and the corresponding LEOInitXXValue call.
 @seealso //leo_ref/c/func/LEOCleanUpStackToPtr LEOCleanUpStackToPtr
 @seealso //leo_ref/c/func/LEOPushValueOnStack LEOPushValueOnStack
 */
LEOValuePtr	LEOPushNumberOnStack( LEOContext* theContext, LEONumber inNumber, LEOUnit inUnit );


/*! Push a copy of the given boolean onto the stack, returning a pointer to it.
	This is a shorthand for LEOPushValueOnStack and the corresponding LEOInitXXValue call.
 @seealso //leo_ref/c/func/LEOCleanUpStackToPtr LEOCleanUpStackToPtr
 @seealso //leo_ref/c/func/LEOPushValueOnStack LEOPushValueOnStack
 */
LEOValuePtr	LEOPushBooleanOnStack( LEOContext* theContext, bool inBoolean );


/*! Push a copy of the given point onto the stack, returning a pointer to it.
	This is a shorthand for LEOPushValueOnStack and the corresponding LEOInitXXValue call.
 @seealso //leo_ref/c/func/LEOCleanUpStackToPtr LEOCleanUpStackToPtr
 @seealso //leo_ref/c/func/LEOPushValueOnStack LEOPushValueOnStack
 */
LEOValuePtr	LEOPushPointOnStack( LEOContext* theContext, LEOInteger l, LEOInteger t );


/*! Push a copy of the given rectangle onto the stack, returning a pointer to it.
	This is a shorthand for LEOPushValueOnStack and the corresponding LEOInitXXValue call.
 @seealso //leo_ref/c/func/LEOCleanUpStackToPtr LEOCleanUpStackToPtr
 @seealso //leo_ref/c/func/LEOPushValueOnStack LEOPushValueOnStack
 */
LEOValuePtr	LEOPushRectOnStack( LEOContext* theContext, LEOInteger l, LEOInteger t, LEOInteger r, LEOInteger b );

	
/*! Push the given array onto the stack, returning a pointer to the value.
 This is a shorthand for LEOPushValueOnStack and the corresponding LEOInitXXValue call.
 You may pass NULL for inArray to push an empty array.
 *** This takes over ownership of the given array. ***
 @seealso //leo_ref/c/func/LEOCleanUpStackToPtr LEOCleanUpStackToPtr
 @seealso //leo_ref/c/func/LEOPushValueOnStack LEOPushValueOnStack
 */
LEOValuePtr	LEOPushArrayValueOnStack( LEOContext* theContext, struct LEOArrayEntry* inArray );


/*! Pushes a special empty string on the stack that a user can't generate,
	and which returns TRUE from LEOGetValueIsUnset(). To the user it looks
	just like any empty string, but if the user replaces it with any other
	value (even an empty string), you can detect that.
	Used e.g. by the message box to detect whether the code the user entered
	returned a result (i.e. was an expression) or not (i.e. was a command).
	This is a shorthand for LEOPushValueOnStack and LEOInitUnsetValue.
 @seealso //leo_ref/c/func/LEOCleanUpStackToPtr LEOCleanUpStackToPtr
 @seealso //leo_ref/c/func/LEOPushValueOnStack LEOPushValueOnStack
 @seealso //leo_ref/c/func/LEOPushEmptyValueOnStack LEOPushEmptyValueOnStack
 @seealso //leo_ref/c/func/LEOInitUnsetValue LEOInitUnsetValue
 */
LEOValuePtr	LEOPushUnsetValueOnStack( LEOContext* theContext );


/*! Push an empty string onto the stack, returning a pointer to it.
	This is a shorthand for LEOPushValueOnStack and the corresponding LEInitStringConstantValue("") call.
 @seealso //leo_ref/c/func/LEOCleanUpStackToPtr LEOCleanUpStackToPtr
 @seealso //leo_ref/c/func/LEOPushValueOnStack LEOPushValueOnStack
 @seealso //leo_ref/c/func/LEOInitStringConstantValue LEInitStringConstantValue
 @seealso //leo_ref/c/func/LEOPushUnsetValueOnStack LEOPushUnsetValueOnStack
 */
LEOValuePtr	LEOPushEmptyValueOnStack( LEOContext* theContext );


/*! Push a copy of the given string onto the stack, returning a pointer to it.
	This is a shorthand for LEOPushValueOnStack and the corresponding LEOInitXXValue call.
 @seealso //leo_ref/c/func/LEOCleanUpStackToPtr LEOCleanUpStackToPtr
 @seealso //leo_ref/c/func/LEOPushValueOnStack LEOPushValueOnStack
 @seealso //leo_ref/c/func/LEOInitStringValue LEInitStringValue
 */
LEOValuePtr	LEOPushStringValueOnStack( LEOContext* theContext, const char* inString, size_t strLen );


/*! Push a copy of the given string constant onto the stack, returning a pointer to it.
	This is a shorthand for LEOPushValueOnStack and the corresponding LEOInitXXValue call.
 @seealso //leo_ref/c/func/LEOCleanUpStackToPtr LEOCleanUpStackToPtr
 @seealso //leo_ref/c/func/LEOPushValueOnStack LEOPushValueOnStack
 @seealso //leo_ref/c/func/LEOInitStringConstantValue LEInitStringConstantValue
 */
LEOValuePtr	LEOPushStringConstantValueOnStack( LEOContext* theContext, const char* inString );


/*! Used by instructions to unwind the stack (e.g. when removing their parameters) and ensure
	values get destructed correctly.
 @seealso //leo_ref/c/func/LEOPushValueOnStack LEOPushValueOnStack
*/
void	LEOCleanUpStackToPtr( LEOContext* theContext, union LEOValue* lastItemToDelete );


/*! Print the given instruction to the console for debugging purposes.
	@param inContext may be NULL. If not NULL, this context will be used to display
						additional information about certain instructions.
	@param inHandler may be NULL. If not NULL, this handler will be used to display
						additional information about certain instructions.
	@seealso //leo_ref/c/func/LEODebugPrintInstructions	LEODebugPrintInstructions
*/
void	LEODebugPrintInstr( LEOInstruction* instruction, struct LEOScript* inScript, struct LEOHandler * inHandler, LEOContext * inContext );

/*! Print the given array of instructions to the console for debugging purposes using LEODebugPrintInstr.
	@param inContext may be NULL. If not NULL, this context will be used to display
					additional information about certain instructions.
	@param inHandler may be NULL. If not NULL, this handler will be used to display
					additional information about certain instructions.
	@seealso //leo_ref/c/func/LEODebugPrintInstr	LEODebugPrintInstr
*/
void	LEODebugPrintInstructions( LEOInstruction instructions[], size_t numInstructions, struct LEOScript* inScript, struct LEOHandler * inHandler, LEOContext * inContext );

/*! Print the given context to the console for debugging purposes. This includes
	the stack, and is very useful for debugging new instructions.
	@seealso //leo_ref/c/func/LEODebugPrintInstr	LEODebugPrintInstr
	@seealso //leo_ref/c/func/LEOContextDebugPrintCallStack	LEOContextDebugPrintCallStack
*/
void	LEODebugPrintContext( LEOContext* ctx );

/*!
	Print the current call stack of the given context to the console for
	debugging purposes.
*/
void	LEOContextDebugPrintCallStack( LEOContext* inContext );


/*!
	@functiongroup LEOContext calls for call stack management
	These are used by the CALL_HANDLER_INSTR and RETURN_FROM_HANDLER_INSTR
	instructions.
*/

/*!
	Push a reference to the current script, the handler in it that is current,
	the return address and the previous base pointer to restore onto a special
	stack so the call instruction can retain the script/handler while it is
	running even if the owning object goes away, and so the return instruction
	can restore the base pointer and knows what instruction to return to.
	
	@seealso //leo_ref/c/func/LEOContextPeekCurrentHandler	LEOContextPeekCurrentHandler
	@seealso //leo_ref/c/func/LEOContextPeekCurrentScript	LEOContextPeekCurrentScript
	@seealso //leo_ref/c/func/LEOContextPeekReturnAddress	LEOContextPeekReturnAddress
	@seealso //leo_ref/c/func/LEOContextPeekBasePtr	LEOContextPeekBasePtr
	@seealso //leo_ref/c/func/LEOContextPopHandlerScriptReturnAddressAndBasePtr	LEOContextPopHandlerScriptReturnAddressAndBasePtr
*/
void				LEOContextPushHandlerScriptReturnAddressAndBasePtr( LEOContext* inContext, struct LEOHandler* inHandler, struct LEOScript* inScript, LEOInstruction* returnAddress, LEOValuePtr oldBP );

/*!
	Retrieve the object representing the handler currently in progress, which
	was pushed last onto a special stack.
	@seealso //leo_ref/c/func/LEOContextPushHandlerScriptReturnAddressAndBasePtr	LEOContextPushHandlerScriptReturnAddressAndBasePtr
*/
struct LEOHandler*	LEOContextPeekCurrentHandler( LEOContext* inContext );

/*!
	Retrieve the object representing the script owning the handler currently in
	progress, which was pushed last onto a special stack.
	@seealso //leo_ref/c/func/LEOContextPushHandlerScriptReturnAddressAndBasePtr	LEOContextPushHandlerScriptReturnAddressAndBasePtr
*/
struct LEOScript*	LEOContextPeekCurrentScript( LEOContext* inContext );

/*!
	Retrieve the address of the instruction that follows the CALL_HANDLER_INSTR
	instruction that brought you to this handler, and to which we are expected
	to return after this handler call. Note that this may be NULL to indicate
	you are the first handler pushed on the call stack (in C that would e.g. be
	main()).
	@seealso //leo_ref/c/func/LEOContextPushHandlerScriptReturnAddressAndBasePtr	LEOContextPushHandlerScriptReturnAddressAndBasePtr
*/
LEOInstruction*		LEOContextPeekReturnAddress( LEOContext* inContext );

/*!
	Retrieve the address of the base pointer as it was before this handler was
	called. Used to restore the calling handler's base pointer when this handler
	returns.
	@seealso //leo_ref/c/func/LEOContextPushHandlerScriptReturnAddressAndBasePtr	LEOContextPushHandlerScriptReturnAddressAndBasePtr
*/
LEOValuePtr			LEOContextPeekBasePtr( LEOContext* inContext );

/*!
	Remove the reference to the current handler, current script, return address
	and base pointer before the current handler was called from the special
	stack. This is called when returning from the current handler.
	@seealso //leo_ref/c/func/LEOContextPushHandlerScriptReturnAddressAndBasePtr	LEOContextPushHandlerScriptReturnAddressAndBasePtr
*/
void				LEOContextPopHandlerScriptReturnAddressAndBasePtr( LEOContext* inContext );


/*!
	Generate a file ID for the given file name or file path (or could even be an
	object 'path' of some sort). This is used e.g. by the debugger to associate
	a source file with certain instructions.
	@seealso //leo_ref/c/func/LEOFileIDForFileName	LEOFileIDForFileName
*/
uint16_t		LEOFileIDForFileName( const char* inFileName );

/*!
	Return the file name that corresponds to the given file ID.
	@seealso //leo_ref/c/func/LEOFileIDForFileName	LEOFileIDForFileName
*/
const char*		LEOFileNameForFileID( uint16_t inFileID );


/*!
	Tell the interpreter to log the current context (current instruction and stack) to the console before executing the specified instruction.
	@param inID	The instruction ID constant of the instruction you're interested in.
	@seealso //leo_ref/c/func/LEOSetInstructionIDToDebugPrintAfter	LEOSetInstructionIDToDebugPrintAfter
*/
void	LEOSetInstructionIDToDebugPrintBefore( LEOInstructionID inID );


/*!
	Tell the interpreter to log the current context (current instruction and stack) to the console after executing the specified instruction.
	@param inID	The instruction ID constant of the instruction you're interested in.
	@seealso //leo_ref/c/func/LEOSetInstructionIDToDebugPrintBefore	LEOSetInstructionIDToDebugPrintBefore
*/
void	LEOSetInstructionIDToDebugPrintAfter( LEOInstructionID inID );


/*!
	A script can be paused mid-instruction and later resumed. This is handy e.g.
	when an instruction actually involves an asynchronous operation. Once the
	async operation has finished, it can call to request for the script to be
	resumed. Since we don't want scripts piling up in async callbacks, scripts
	are resumed by calling LEOContextResumeIfAvailable() from somewhere farther
	up the main thread (e.g. where regular event handlers would be triggered).
	When a resume request is made, this callback is invoked to give you the
	opportunity to ensure a call to LEOContextResumeIfAvailable() will be triggered
	by whatever mechanism you choose (queue up an event, signal a semaphore, whatever).
*/
void	LEOSetCheckForResumeProc( void (*checkForResumeProc)(void) );


#if __cplusplus
}
#endif


#endif // LEO_INTERPRETER_H
