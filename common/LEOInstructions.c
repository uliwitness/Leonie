/*
 *  LEOInstructions.c
 *  Leonie
 *
 *  Created by Uli Kusterer on 17.09.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

/*!
	@header LEOInstructions
	These functions implement the actual instructions the Leonie bytecode
	interpreter actually understands. Or at least those that are portable between
	platforms.
*/

#include "LEOInstructions.h"
#include "LEOValue.h"
#include "LEOInterpreter.h"
#include "LEOScript.h"
#include "LEOContextGroup.h"
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>


#pragma mark Instruction Functions

/*!
	Whenever an invalid instruction opcode is encountered in bytecode, this
	instruction will be executed. It terminates execution and provides an error
	message indicating what instruction opcode was invalid.	(INVALID_INSTR)
*/

void	LEOInvalidInstruction( LEOContext* inContext )
{
	snprintf( inContext->errMsg, sizeof(inContext->errMsg), "Unknown instruction %u", inContext->currentInstruction->instructionID );
	
	inContext->keepRunning = false;	// Causes interpreter loop to exit.
}


/*!
	Abort execution of the current script without an error.	(EXIT_TO_TOP_INSTR)
*/

void	LEOExitToTopInstruction( LEOContext* inContext )
{
	inContext->keepRunning = false;	// Causes interpreter loop to exit.
}


/*!
	This instruction does nothing. It just advances to the next instruction.
	(NO_OP_INSTR)
*/

void	LEONoOpInstruction( LEOContext* inContext )
{
	// Do nothing.

	inContext->currentInstruction++;
}


/*!
	Take a string in the current script's string table and push it on the stack
	as a LEOStringValue. (PUSH_STR_FROM_TABLE_INSTR)
	
	param2	-	The index of the string table entry to retrieve.
*/

void	LEOPushStringFromTableInstruction( LEOContext* inContext )
{
	const char*		theString = "";
	LEOScript*		script = LEOContextPeekCurrentScript( inContext );
	if( inContext->currentInstruction->param2 < script->numStrings )
		theString = script->strings[inContext->currentInstruction->param2];
	
	LEOInitStringValue( (LEOValuePtr) inContext->stackEndPtr, theString, kLEOInvalidateReferences, inContext );
	inContext->stackEndPtr++;
	
	inContext->currentInstruction++;
}


void	LEOPrintInstruction( LEOContext* inContext );


/*!
	Pop the last value off the stack. (POP_VALUE_INSTR)
	
	param1	-	If this is not BACK_OF_STACK, we copy the value to that bp-relative
				stack location before we pop it.
*/

void	LEOPopInstruction( LEOContext* inContext )
{
	bool			onStack = (inContext->currentInstruction->param1 == BACK_OF_STACK);
	union LEOValue*	destValue = onStack ? NULL : (inContext->stackBasePtr +(*(int16_t*)&inContext->currentInstruction->param1));
	
	if( destValue )
	{
		LEOCleanUpValue(destValue, kLEOKeepReferences, inContext);
		LEOInitCopy( inContext->stackEndPtr -1, destValue, kLEOKeepReferences, inContext );
	}
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );
	
	inContext->currentInstruction++;
}


/*!
	Push a boolean on the stack. (PUSH_BOOLEAN_INSTR)
	
	param2	-	The boolean to push on the stack.
*/

void	LEOPushBooleanInstruction( LEOContext* inContext )
{
	LEOInitBooleanValue( (LEOValuePtr) inContext->stackEndPtr, inContext->currentInstruction->param2 == 1,
							kLEOInvalidateReferences, inContext );
	inContext->stackEndPtr++;

	inContext->currentInstruction++;
}


/*!
	Take a string from a strings table and assign it to the value in the given
	slot on the stack, or on the back of the stack. (ASSIGN_STRING_FROM_TABLE_INSTR)
	
	param1	-	The basePtr-relative offset of the instruction, or BACK_OF_STACK.
	
	param2	-	The index of the given string in the current context group's
				strings table.
*/

void	LEOAssignStringFromTableInstruction( LEOContext* inContext )
{
	bool			onStack = (inContext->currentInstruction->param1 == BACK_OF_STACK);
	union LEOValue*	theValue = onStack ? (inContext->stackEndPtr -1) : (inContext->stackBasePtr +(*(int16_t*)&inContext->currentInstruction->param1));
	const char*		theString = "";
	LEOScript*		script = LEOContextPeekCurrentScript( inContext );
	if( inContext->currentInstruction->param2 < script->numStrings )
		theString = script->strings[inContext->currentInstruction->param2];
	
	LEOSetValueAsString( theValue, theString, inContext );
	
	inContext->currentInstruction++;
}


/*!
	Jump to another instruction relative to this one (JUMP_RELATIVE_INSTR)
	
	param2	-	The number of instructions to jump by. A value of 1 would make
				this identical to NO_OP_INSTR.
*/

void	LEOJumpRelativeInstruction( LEOContext* inContext )
{
	inContext->currentInstruction += LEOCastUInt32ToInt32( inContext->currentInstruction->param2 );
}


/*!
	Jump to another instruction relative to this one if the given value is
	TRUE (JUMP_RELATIVE_IF_TRUE_INSTR)
	
	param1	-	The basePtr-relative offset of the value to examine, or BACK_OF_STACK.
	
	param2	-	The number of instructions to jump by.
*/

void	LEOJumpRelativeIfTrueInstruction( LEOContext* inContext )
{
	bool			popOffStack = (inContext->currentInstruction->param1 == BACK_OF_STACK);
	union LEOValue*	theValue = popOffStack ? (inContext->stackEndPtr -1) : (inContext->stackBasePtr +(*(int16_t*)&inContext->currentInstruction->param1));
	if( LEOGetValueAsBoolean( theValue, inContext ) )
		inContext->currentInstruction += LEOCastUInt32ToInt32( inContext->currentInstruction->param2 );
	else
		inContext->currentInstruction++;
	if( popOffStack )
		LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );
}


/*!
	Jump to another instruction relative to this one if the given value is
	FALSE (JUMP_RELATIVE_IF_FALSE_INSTR)
	
	param1	-	The basePtr-relative offset of the value to examine, or BACK_OF_STACK.
	
	param2	-	The number of instructions to jump by.
*/

void	LEOJumpRelativeIfFalseInstruction( LEOContext* inContext )
{
	bool			popOffStack = (inContext->currentInstruction->param1 == BACK_OF_STACK);
	union LEOValue*	theValue = popOffStack ? (inContext->stackEndPtr -1) : (inContext->stackBasePtr +(*(int16_t*)&inContext->currentInstruction->param1));
	if( !LEOGetValueAsBoolean( theValue, inContext ) )
		inContext->currentInstruction += LEOCastUInt32ToInt32( inContext->currentInstruction->param2 );
	else
		inContext->currentInstruction++;
	if( popOffStack )
		LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );
}


/*!
	Jump to another instruction relative to this one if the given value is
	&gt; 0 (JUMP_RELATIVE_IF_GT_ZERO_INSTR)
	
	param1	-	The basePtr-relative offset of the value to examine, or BACK_OF_STACK.
	
	param2	-	The number of instructions to jump by.
*/

void	LEOJumpRelativeIfGreaterThanZeroInstruction( LEOContext* inContext )
{
	bool			popOffStack = (inContext->currentInstruction->param1 == BACK_OF_STACK);
	union LEOValue*	theValue = popOffStack ? (inContext->stackEndPtr -1) : (inContext->stackBasePtr +(*(int16_t*)&inContext->currentInstruction->param1));
	if( LEOGetValueAsNumber( theValue, inContext ) > 0 )
		inContext->currentInstruction += LEOCastUInt32ToInt32( inContext->currentInstruction->param2 );
	else
		inContext->currentInstruction++;
	if( popOffStack )
		LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );
}


/*!
	Jump to another instruction relative to this one if the given value is
	&lt; 0 (JUMP_RELATIVE_IF_LT_ZERO_INSTR)
	
	param1	-	The basePtr-relative offset of the value to examine, or BACK_OF_STACK.
	
	param2	-	The number of instructions to jump by.
*/

void	LEOJumpRelativeIfLessThanZeroInstruction( LEOContext* inContext )
{
	bool			popOffStack = (inContext->currentInstruction->param1 == BACK_OF_STACK);
	union LEOValue*	theValue = popOffStack ? (inContext->stackEndPtr -1) : (inContext->stackBasePtr +(*(int16_t*)&inContext->currentInstruction->param1));
	if( LEOGetValueAsNumber( theValue, inContext ) < 0 )
		inContext->currentInstruction += LEOCastUInt32ToInt32( inContext->currentInstruction->param2 );
	else
		inContext->currentInstruction++;
	if( popOffStack )
		LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );
}


/*!
	Jump to another instruction relative to this one if the given value is
	&gt;= 0 (JUMP_RELATIVE_IF_GT_SAME_ZERO_INSTR)
	
	param1	-	The basePtr-relative offset of the value to examine, or BACK_OF_STACK.
	
	param2	-	The number of instructions to jump by.
*/

void	LEOJumpRelativeIfGreaterSameThanZeroInstruction( LEOContext* inContext )
{
	bool			popOffStack = (inContext->currentInstruction->param1 == BACK_OF_STACK);
	union LEOValue*	theValue = popOffStack ? (inContext->stackEndPtr -1) : (inContext->stackBasePtr +(*(int16_t*)&inContext->currentInstruction->param1));
	if( LEOGetValueAsNumber( theValue, inContext ) >= 0 )
		inContext->currentInstruction += LEOCastUInt32ToInt32( inContext->currentInstruction->param2 );
	else
		inContext->currentInstruction++;
	if( popOffStack )
		LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );
}


/*!
	Jump to another instruction relative to this one if the given value is
	&lt;= 0 (JUMP_RELATIVE_IF_LT_SAME_ZERO_INSTR)
	
	param1	-	The basePtr-relative offset of the value to examine, or BACK_OF_STACK.
	
	param2	-	The number of instructions to jump by.
*/

void	LEOJumpRelativeIfLessSameThanZeroInstruction( LEOContext* inContext )
{
	bool			popOffStack = (inContext->currentInstruction->param1 == BACK_OF_STACK);
	union LEOValue*	theValue = popOffStack ? (inContext->stackEndPtr -1) : (inContext->stackBasePtr +(*(int16_t*)&inContext->currentInstruction->param1));
	if( LEOGetValueAsNumber( theValue, inContext ) <= 0 )
		inContext->currentInstruction += LEOCastUInt32ToInt32( inContext->currentInstruction->param2 );
	else
		inContext->currentInstruction++;
	if( popOffStack )
		LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );
}


/*!
	Push the given LEONumber floating point quantity on the stack (PUSH_NUMBER_INSTR)
	
	param2	-	The LEONumber (typecast to a uint32_t) to push.
*/

void	LEOPushNumberInstruction( LEOContext* inContext )
{
	LEOInitNumberValue( (LEOValuePtr) inContext->stackEndPtr, LEOCastUInt32ToLEONumber(inContext->currentInstruction->param2),
						kLEOInvalidateReferences, inContext );
	inContext->stackEndPtr++;

	inContext->currentInstruction++;
}


/*!
	Push the given LEOInteger on the stack (PUSH_INTEGER_INSTR)
	
	param2	-	The LEOInteger (typecast to a uint32_t) to push.
*/

void	LEOPushIntegerInstruction( LEOContext* inContext )
{
	LEOInitIntegerValue( (LEOValuePtr) inContext->stackEndPtr, inContext->currentInstruction->param2,
							kLEOInvalidateReferences, inContext );
	inContext->stackEndPtr++;

	inContext->currentInstruction++;
}


/*!
	Add a LEONumber to a value (ADD_NUMBER_INSTR)
	
	param1	-	The basePtr-relative offset of the value to add to, or BACK_OF_STACK.
	
	param2	-	The LEONumber to add to the value (typecast to a uint32_t).
*/

void	LEOAddNumberInstruction( LEOContext* inContext )
{
	bool			onStack = (inContext->currentInstruction->param1 == BACK_OF_STACK);
	union LEOValue*	theValue = onStack ? (inContext->stackEndPtr -1) : (inContext->stackBasePtr +(*(int16_t*)&inContext->currentInstruction->param1));
	LEONumber		theNum = LEOGetValueAsNumber( theValue, inContext );
	
	theNum += LEOCastUInt32ToLEONumber( inContext->currentInstruction->param2 );
	LEOSetValueAsNumber( theValue, theNum, inContext );
	
	inContext->currentInstruction++;
}


/*!
	Add a LEOInteger to a value (ADD_INTEGER_INSTR)
	
	param1	-	The basePtr-relative offset of the value to add to, or BACK_OF_STACK.
	
	param2	-	The int32_t to add to the value (typecast to a uint32_t).
*/

void	LEOAddIntegerInstruction( LEOContext* inContext )
{
	bool			onStack = (inContext->currentInstruction->param1 == BACK_OF_STACK);
	union LEOValue*	theValue = onStack ? (inContext->stackEndPtr -1) : (inContext->stackBasePtr +(*(int16_t*)&inContext->currentInstruction->param1));
	LEOInteger		theNum = LEOGetValueAsInteger( theValue, inContext );
	
	theNum += LEOCastUInt32ToInt32( inContext->currentInstruction->param2 );
	LEOSetValueAsNumber( theValue, theNum, inContext );
	
	inContext->currentInstruction++;
}


/*!
	Call a given handler (CALL_HANDLER_INSTR)
	
	This saves off the current base pointer and the address of the next
	instruction so returning from the handler can restore the previous state,
	and retains the current script in case the script deletes its owner.
	
	param1	-	0 to call a command, 1 to call a function.
	param2	-	The LEOHandlerID of the handler to call.
	
	@seealso //leo_ref/c/func/LEOReturnFromHandlerInstruction LEOReturnFromHandlerInstruction
*/

void	LEOCallHandlerInstruction( LEOContext* inContext )
{
	//LEODebugPrintContext( inContext );
	
	LEOHandlerID	handlerName = inContext->currentInstruction->param2;
	LEOScript*		currScript = LEOContextPeekCurrentScript( inContext );
	LEOHandler*		foundHandler = NULL;
	if( currScript )
	{
		if( inContext->currentInstruction->param1 == 0 )
			foundHandler = LEOScriptFindCommandHandlerWithID( currScript, handlerName );
		else
			foundHandler = LEOScriptFindFunctionHandlerWithID( currScript, handlerName );
		if( foundHandler )
		{
			LEOContextPushHandlerScriptReturnAddressAndBasePtr( inContext, foundHandler, currScript, inContext->currentInstruction +1, inContext->stackBasePtr );
			inContext->currentInstruction = foundHandler->instructions;
			inContext->stackBasePtr = inContext->stackEndPtr;
		}
	}
	
	if( !foundHandler )
	{
		snprintf( inContext->errMsg, sizeof(inContext->errMsg), "Couldn't find handler \"%s\".", LEOContextGroupHandlerNameForHandlerID( inContext->group, handlerName ) );
		inContext->keepRunning = false;
		inContext->currentInstruction++;
	}
	
	//LEODebugPrintContext( inContext );
}


/*!
	Return to the calling handler (RETURN_FROM_HANDLER_INSTR)
	
	This restores the previously-saved base pointer, jumps to the saved return
	address and releases its ownership of the current script (as established by
	CALL_HANDLER_INSTR).
	
	param1		-		If this is BACK_OF_STACK, a return value will be popped
						off the back of the stack and stored in the return value
						area (right preceding the parameters on the stack).
	
	@seealso //leo_ref/c/func/LEOCallHandlerInstruction LEOCallHandlerInstruction
*/

void	LEOReturnFromHandlerInstruction( LEOContext* inContext )
{
	//LEODebugPrintContext( inContext );
	
	inContext->currentInstruction = LEOContextPeekReturnAddress( inContext );
	inContext->stackBasePtr = LEOContextPeekBasePtr( inContext );
	LEOContextPopHandlerScriptReturnAddressAndBasePtr( inContext );
	
	//LEODebugPrintContext( inContext );
}


/*!
	Push a reference to the given value onto the stack (SET_RETURN_VALUE_INSTR)
*/

void	LEOSetReturnValueInstruction( LEOContext* inContext )
{
	union LEOValue*	paramCountValue = inContext->stackBasePtr -1;
	LEOInteger		paramCount = LEOGetValueAsNumber( paramCountValue, inContext );
	union LEOValue*	destValue = inContext->stackBasePtr -1 -paramCount -1;
	LEOCleanUpValue( destValue, kLEOKeepReferences, inContext );
	LEOInitSimpleCopy( inContext->stackEndPtr -1, destValue, kLEOKeepReferences, inContext );
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );
	
	inContext->currentInstruction++;
}


/*!
	Push a reference to the given value onto the stack (PUSH_REFERENCE_INSTR)
	
	param1	-	The basePtr-relative offset of the value to be referenced, or BACK_OF_STACK.
*/

void	LEOPushReferenceInstruction( LEOContext* inContext )
{
	bool			onStack = (inContext->currentInstruction->param1 == BACK_OF_STACK);
	union LEOValue*	theValue = onStack ? (inContext->stackEndPtr -1) : (inContext->stackBasePtr +(*(int16_t*)&inContext->currentInstruction->param1));
	union LEOValue	tmpRefValue = { 0 };
	LEOValuePtr		refValueOnStack = NULL;
	
	LEOInitReferenceValue( &tmpRefValue, theValue, kLEOInvalidateReferences, kLEOChunkTypeINVALID, 0, 0, inContext );
	refValueOnStack = LEOPushValueOnStack( inContext, &tmpRefValue );
	
	inContext->currentInstruction++;
}


/*!
	Push a reference to a chunk out of a larger value onto the stack. (PUSH_CHUNK_REFERENCE_INSTR)
	
	The chunk end and chunk start are popped off the back of the stack (in that order).
	
	param1	-	The basePtr-relative offset of the value to be referenced.
	
	param2	-	The LEOChunkType of this chunk expression.
	
	@seealso //leo_ref/c/func/LEOGetChunkRanges LEOGetChunkRanges
*/

void	LEOPushChunkReferenceInstruction( LEOContext* inContext )
{
	LEOValuePtr		chunkTarget = (inContext->stackBasePtr +(*(int16_t*)&inContext->currentInstruction->param1));
	LEOValuePtr		chunkEnd = inContext->stackEndPtr -1;
	LEOValuePtr		chunkStart = inContext->stackEndPtr -2;
	union LEOValue	tmpRefValue = { 0 };
	LEOValuePtr		refValueOnStack = NULL;
	
	size_t	chunkStartOffs = LEOGetValueAsInteger(chunkStart,inContext);
	if( !inContext->keepRunning )
		return;
	
	size_t	chunkEndOffs = LEOGetValueAsInteger(chunkEnd,inContext);
	if( !inContext->keepRunning )
		return;
	
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -2 );
	
	LEOInitReferenceValue( &tmpRefValue, chunkTarget, kLEOInvalidateReferences, inContext->currentInstruction->param2, chunkStartOffs, chunkEndOffs, inContext );
	refValueOnStack = LEOPushValueOnStack( inContext, &tmpRefValue );
	
	inContext->currentInstruction++;
}


/*!
	Copy the value of the parameter at given index into the given value on the
	stack. If no parameter of that index has been passed, this returns an empty
	string. (PARAMETER_INSTR)
	
	param1	-	The basePtr-relative offset of the value to be overwritten, or
				BACK_OF_STACK if you want the value to be pushed on the stack.
	
	param2	-	The number of the parameter to retrieve, as a 1-based index.
	
	@seealso //leo_ref/c/func/LEOParameterCountInstruction LEOParameterCountInstruction
*/

void	LEOParameterInstruction( LEOContext* inContext )
{
	bool		onStack = (inContext->currentInstruction->param1 == BACK_OF_STACK);
	int16_t		offset = (*(int16_t*)&inContext->currentInstruction->param1);
	LEOValuePtr	valueTarget = onStack ? (inContext->stackEndPtr++) : (inContext->stackBasePtr +offset);
	if( !onStack )
		LEOCleanUpValue( valueTarget, kLEOKeepReferences, inContext );
	LEOValuePtr	paramCountValue = inContext->stackBasePtr -1;
	LEOInteger	paramCount = LEOGetValueAsNumber( paramCountValue, inContext );
	if( inContext->currentInstruction->param2 <= paramCount )
	{
		LEOInitSimpleCopy( inContext->stackBasePtr -inContext->currentInstruction->param2 -1, valueTarget,
							(onStack ? kLEOInvalidateReferences : kLEOKeepReferences), inContext );
	}
	else
		LEOInitStringConstantValue( valueTarget, "", kLEOKeepReferences, inContext );
	
	inContext->currentInstruction++;
}


/*!
	Copy the value of the parameter at given index into the given value on the
	stack. If no parameter of that index has been passed, this returns an empty
	string. (PARAMETER_KEEPREFS_INSTR)
	
	param1	-	The basePtr-relative offset of the value to be overwritten, or
				BACK_OF_STACK if you want the value to be pushed on the stack.
	
	param2	-	The number of the parameter to retrieve, as a 1-based index.
	
	@seealso //leo_ref/c/func/LEOParameterCountInstruction LEOParameterCountInstruction
*/

void	LEOParameterKeepRefsInstruction( LEOContext* inContext )
{
	bool		onStack = (inContext->currentInstruction->param1 == BACK_OF_STACK);
	int16_t		offset = (*(int16_t*)&inContext->currentInstruction->param1);
	LEOValuePtr	valueTarget = onStack ? (inContext->stackEndPtr++) : (inContext->stackBasePtr +offset);
	if( !onStack )
		LEOCleanUpValue( valueTarget, kLEOKeepReferences, inContext );
	LEOValuePtr	paramCountValue = inContext->stackBasePtr -1;
	LEOInteger	paramCount = LEOGetValueAsNumber( paramCountValue, inContext );
	if( inContext->currentInstruction->param2 <= paramCount )
	{
		LEOInitCopy( inContext->stackBasePtr -inContext->currentInstruction->param2 -1, valueTarget,
						(onStack ? kLEOInvalidateReferences : kLEOKeepReferences), inContext );
	}
	else
		LEOInitStringConstantValue( valueTarget, "", kLEOKeepReferences, inContext );
	
	inContext->currentInstruction++;
}


/*!
	Determine the number of parameters that have been passed to this function
	and (PARAMETER_COUNT_INSTR)
	
	param1	-	The basePtr-relative offset of the value to be overwritten, or
				BACK_OF_STACK if you want the value to be pushed on the stack.
	
	@seealso //leo_ref/c/func/LEOParameterInstruction LEOParameterInstruction
*/

void	LEOParameterCountInstruction( LEOContext* inContext )
{
	bool		onStack = (inContext->currentInstruction->param1 == BACK_OF_STACK);
	LEOValuePtr	valueTarget = onStack ? (inContext->stackEndPtr++) : (inContext->stackBasePtr +(*(int16_t*)&inContext->currentInstruction->param1));
	if( !onStack )
		LEOCleanUpValue( valueTarget, kLEOKeepReferences, inContext );
	LEOInteger	paramCount = LEOGetValueAsNumber( inContext->stackBasePtr -1, inContext );
	LEOInitIntegerValue( valueTarget, paramCount, (onStack ? kLEOInvalidateReferences : kLEOKeepReferences), inContext );
	
	inContext->currentInstruction++;
}


void	LEOConcatenateValuesInstruction( LEOContext* inContext )
{
	union LEOValue*	secondArgumentValue = inContext->stackEndPtr -1;
	union LEOValue*	firstArgumentValue = inContext->stackEndPtr -2;
	size_t			startOffs = 0, endOffs = SIZE_MAX,
					startDelOffs, endDelOffs;
	uint32_t		delimChar = inContext->currentInstruction->param2;
	char			tempStr[1024] = { 0 };	// TODO: Make this work with any length of string.
	size_t			offs = 0;
	
	if( delimChar != 0 )
	{
		tempStr[0] = delimChar;		// TODO: Make this work with any Unicode character.
		offs = 1;
	}
	
	LEOGetValueAsString( secondArgumentValue, tempStr +offs, sizeof(tempStr) -offs, inContext );
	
	LEODetermineChunkRangeOfSubstring(	firstArgumentValue, &startOffs, &endOffs,
										&startDelOffs, &endDelOffs,
										kLEOChunkTypeCharacter,
										SIZE_MAX, SIZE_MAX, inContext );
	LEOSetValuePredeterminedRangeAsString( firstArgumentValue, endOffs, endOffs, tempStr, inContext );
	
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );
	
	inContext->currentInstruction++;
}


void	LEOConcatenateValuesWithSpaceInstruction( LEOContext* inContext )
{
	union LEOValue*	secondArgumentValue = inContext->stackEndPtr -1;
	union LEOValue*	firstArgumentValue = inContext->stackEndPtr -2;
	size_t			startOffs = 0, endOffs = SIZE_MAX,
					startDelOffs, endDelOffs;
	uint32_t		delimChar = inContext->currentInstruction->param2;
	char			tempStr[1024] = { 0 };	// TODO: Make this work with any length of string.
	size_t			offs = 0;
	
	if( delimChar == 0 )
		delimChar = '0';
	
	tempStr[0] = delimChar;		// TODO: Make this work with any Unicode character.
	offs = 1;
		
	LEOGetValueAsString( secondArgumentValue, tempStr +offs, sizeof(tempStr) -offs, inContext );
	
	LEODetermineChunkRangeOfSubstring(	firstArgumentValue, &startOffs, &endOffs,
										&startDelOffs, &endDelOffs,
										kLEOChunkTypeCharacter,
										SIZE_MAX, SIZE_MAX, inContext );
	LEOSetValuePredeterminedRangeAsString( firstArgumentValue, endOffs, endOffs, tempStr, inContext );
	
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );
	
	inContext->currentInstruction++;
}


void	LEOAndOperatorInstruction( LEOContext* inContext )
{
	union LEOValue*	secondArgumentValue = inContext->stackEndPtr -1;
	union LEOValue*	firstArgumentValue = inContext->stackEndPtr -2;
	
	bool			firstArgument = LEOGetValueAsBoolean(firstArgumentValue,inContext);
	bool			secondArgument = LEOGetValueAsBoolean(secondArgumentValue,inContext);

	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -2 );
	
	LEOPushBooleanOnStack( inContext, firstArgument && secondArgument );
	
	inContext->currentInstruction++;
}

void	LEOOrOperatorInstruction( LEOContext* inContext )
{
	union LEOValue*	secondArgumentValue = inContext->stackEndPtr -1;
	union LEOValue*	firstArgumentValue = inContext->stackEndPtr -2;
	
	bool			firstArgument = LEOGetValueAsBoolean(firstArgumentValue,inContext);
	bool			secondArgument = LEOGetValueAsBoolean(secondArgumentValue,inContext);

	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -2 );
	
	LEOPushBooleanOnStack( inContext, firstArgument || secondArgument );
	
	inContext->currentInstruction++;
}



#pragma mark -
#pragma mark Instruction table

LEOInstructionFuncPtr	gInstructions[] =
{
	LEOInvalidInstruction,
	LEOExitToTopInstruction,
	LEONoOpInstruction,
	LEOPushStringFromTableInstruction,
	LEOPrintInstruction,
	LEOPopInstruction,
	LEOPushBooleanInstruction,
	LEOAssignStringFromTableInstruction,
	LEOJumpRelativeInstruction,
	LEOJumpRelativeIfTrueInstruction,
	LEOJumpRelativeIfFalseInstruction,
	LEOJumpRelativeIfGreaterThanZeroInstruction,
	LEOJumpRelativeIfLessThanZeroInstruction,
	LEOJumpRelativeIfGreaterSameThanZeroInstruction,
	LEOJumpRelativeIfLessSameThanZeroInstruction,
	LEOPushNumberInstruction,
	LEOPushIntegerInstruction,
	LEOAddNumberInstruction,
	LEOAddIntegerInstruction,
	LEOCallHandlerInstruction,
	LEOReturnFromHandlerInstruction,
	LEOPushReferenceInstruction,
	LEOPushChunkReferenceInstruction,
	LEOParameterInstruction,
	LEOParameterCountInstruction,
	LEOSetReturnValueInstruction,
	LEOParameterKeepRefsInstruction,
	LEOConcatenateValuesInstruction,
	LEOAndOperatorInstruction,
	LEOOrOperatorInstruction,
	LEOConcatenateValuesWithSpaceInstruction
};

const char*	gInstructionNames[] =
{
	"Invalid",
	"ExitToTop",
	"NoOp",
	"PushStringFromTable",
	"Print",
	"Pop",
	"PushBoolean",
	"AssignStringFromTable",
	"JumpRelative",
	"JumpRelativeIfTrue",
	"JumpRelativeIfFalse",
	"JumpRelativeIfGreaterThanZero",
	"JumpRelativeIfLessThanZero",
	"JumpRelativeIfGreaterSameThanZero",
	"JumpRelativeIfLessSameThanZero",
	"PushNumber",
	"PushInteger",
	"AddNumber",
	"AddInteger",
	"CallHandler",
	"ReturnFromHandler",
	"PushReference",
	"PushChunkReference",
	"Parameter",
	"ParameterCount",
	"SetReturnValue",
	"ParameterKeepRefs",
	"ConcatenateValues",
	"And",
	"Or",
	"ConcatenateValuesWithSpace"
};

size_t		gNumInstructions = sizeof(gInstructions) / sizeof(LEOInstructionFuncPtr);
