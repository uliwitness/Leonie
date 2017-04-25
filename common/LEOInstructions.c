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
#include "UTF8UTF32Utilities.h"
#include "LEOStringUtilities.h"
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>


void	LEOExitToTopInstruction( LEOContext* inContext );
void	LEONoOpInstruction( LEOContext* inContext );
void	LEOPushStringFromTableInstruction( LEOContext* inContext );
void	LEOPushUnsetValueInstruction( LEOContext* inContext );
void	LEOPushStringVariantFromTableInstruction( LEOContext* inContext );
void	LEOPopValueInstruction( LEOContext* inContext );
void	LEOPopSimpleValueInstruction( LEOContext* inContext );
void	LEOPushBooleanInstruction( LEOContext* inContext );
void	LEOAssignStringFromTableInstruction( LEOContext* inContext );
void	LEOJumpRelativeInstruction( LEOContext* inContext );
void	LEOJumpRelativeIfTrueInstruction( LEOContext* inContext );
void	LEOJumpRelativeIfFalseInstruction( LEOContext* inContext );
void	LEOJumpRelativeIfGreaterThanZeroInstruction( LEOContext* inContext );
void	LEOJumpRelativeIfLessThanZeroInstruction( LEOContext* inContext );
void	LEOJumpRelativeIfGreaterSameThanZeroInstruction( LEOContext* inContext );
void	LEOJumpRelativeIfLessSameThanZeroInstruction( LEOContext* inContext );
void	LEOPushNumberInstruction( LEOContext* inContext );
void	LEOPushIntegerInstruction( LEOContext* inContext );
void	LEOPushIntegerStartInstruction( LEOContext* inContext );
void	LEOAssignIntegerEndInstruction( LEOContext* inContext );
void	LEOAddNumberInstruction( LEOContext* inContext );
void	LEOAddIntegerInstruction( LEOContext* inContext );
void	LEOCallHandlerInstruction( LEOContext* inContext );
void	LEOCleanUpHandlerStackInstruction( LEOContext* inContext );
void	LEOReturnFromHandlerInstruction( LEOContext* inContext );
void	LEOSetReturnValueInstruction( LEOContext* inContext );
void	LEOPushReferenceInstruction( LEOContext* inContext );

void	LEOPushChunkReferenceInstruction( LEOContext* inContext );
void	LEOPushChunkInstruction( LEOContext* inContext );
void	LEOSetChunkPropertyInstruction( LEOContext* inContext );
void	LEOPushChunkPropertyInstruction( LEOContext* inContext );
void	LEOParameterInstruction( LEOContext* inContext );
void	LEOParameterKeepRefsInstruction( LEOContext* inContext );
void	LEOPushParametersInstruction( LEOContext* inContext );
void	LEOParameterCountInstruction( LEOContext* inContext );
void	LEOConcatenateValuesInstruction( LEOContext* inContext );
void	LEOConcatenateValuesWithSpaceInstruction( LEOContext* inContext );
void	LEOAndOperatorInstruction( LEOContext* inContext );
void	LEOOrOperatorInstruction( LEOContext* inContext );
void	LEONegateBooleanInstruction( LEOContext* inContext );
void	LEOSubtractCommandInstruction( LEOContext* inContext );
void	LEOAddCommandInstruction( LEOContext* inContext );
void	LEOMultiplyCommandInstruction( LEOContext* inContext );
void	LEODivideCommandInstruction( LEOContext* inContext );
void	LEOSubtractOperatorInstruction( LEOContext* inContext );
void	LEOAddOperatorInstruction( LEOContext* inContext );
void	LEOMultiplyOperatorInstruction( LEOContext* inContext );
void	LEODivideOperatorInstruction( LEOContext* inContext );
void	LEOGreaterThanOperatorInstruction( LEOContext* inContext );
void	LEOLessThanOperatorInstruction( LEOContext* inContext );
void	LEOGreaterThanEqualOperatorInstruction( LEOContext* inContext );
void	LEOLessThanEqualOperatorInstruction( LEOContext* inContext );
void	LEONegateNumberInstruction( LEOContext* inContext );
void	LEOModuloOperatorInstruction( LEOContext* inContext );
void	LEOPowerOperatorInstruction( LEOContext* inContext );
void	LEOEqualOperatorInstruction( LEOContext* inContext );
void	LEONotEqualOperatorInstruction( LEOContext* inContext );
void	LEOLineMarkerInstruction( LEOContext* inContext );
void	LEOAssignChunkArrayInstruction( LEOContext* inContext );
void	LEOCountChunksInstruction( LEOContext* inContext );
void	LEOGetArrayItemInstruction( LEOContext* inContext );
void	LEOGetArrayItemCountInstruction( LEOContext* inContext );
void	LEOSetStringInstruction( LEOContext* inContext );
void	LEOPushItemDelimiterInstruction( LEOContext* inContext );
void	LEOSetItemDelimiterInstruction( LEOContext* inContext );
void	LEOPushGlobalReferenceInstruction( LEOContext* inContext );
void	LEOPutValueIntoValueInstruction( LEOContext* inContext );
void	LEONumToCharInstruction( LEOContext* inContext );
void	LEOCharToNumInstruction( LEOContext* inContext );
void	LEONumToHexInstruction( LEOContext* inContext );
void	LEOHexToNumInstruction( LEOContext* inContext );
void	LEONumToBinaryInstruction( LEOContext* inContext );
void	LEOBinaryToNumInstruction( LEOContext* inContext );
void	LEOIsWithinInstruction( LEOContext* inContext );
void	LEOIntersectsInstruction( LEOContext* inContext );
void	LEOIsUnsetInstruction( LEOContext* inContext );
void	LEOIsTypeInstruction( LEOContext* inContext );
void	LEOPushArrayConstantInstruction( LEOContext* inContext );
void	LEOParseErrorInstruction( LEOContext* inContext );


void	LEOInstructionsFindLineForInstruction( LEOInstruction* instr, size_t *lineNo, uint16_t *fileID )
{
	if( instr == NULL )
		return;
	
	// +++ ASSUMPTION: We know that we always generate a line marker at the start of a handler
	//	and line, so we should never walk off the start of the handler and off into invalid
	//	memory. If we ever offer removing debug info and saving raw bytecode, this assumption
	//	will break.
	
	while( instr->instructionID != LINE_MARKER_INSTR )
	{
		instr --;
	}
	
	*lineNo = instr->param2;
	*fileID = instr->param1;
}

#pragma mark Instruction Functions

/*!
	Whenever an invalid instruction opcode is encountered in bytecode, this
	instruction will be executed. It terminates execution and provides an error
	message indicating what instruction opcode was invalid.	(INVALID_INSTR)
*/

void	LEOInvalidInstruction( LEOContext* inContext )
{
	size_t		lineNo = SIZE_T_MAX;
	uint16_t	fileID = 0;
	LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
	LEOContextStopWithError( inContext, lineNo, SIZE_T_MAX, fileID, "Unknown instruction %u", inContext->currentInstruction->instructionID );	// Causes interpreter loop to exit.
}


/*!
	Abort execution of the current script without an error.	(EXIT_TO_TOP_INSTR)
*/

void	LEOExitToTopInstruction( LEOContext* inContext )
{
	inContext->flags &= ~kLEOContextKeepRunning;	// Causes interpreter loop to exit.
}


/*!
	Abort execution of the current script with an error message.	(PARSE_ERROR_INSTR)
*/

void	LEOParseErrorInstruction( LEOContext* inContext )
{
	size_t		errorIndex = inContext->currentInstruction->param2;
	LEOScript*	script = LEOContextPeekCurrentScript( inContext );
	LEOContextStopWithError( inContext, script->parseErrors[errorIndex].errorLine, script->parseErrors[errorIndex].errorOffset, script->parseErrors[errorIndex].fileID, "%s", script->parseErrors[errorIndex].errMsg );
}


/*!
	Push an array containing the given key/value pairs on the stack.	(PUSH_ARRAY_CONSTANT_INSTR)
*/

void	LEOPushArrayConstantInstruction( LEOContext* inContext )
{
	struct LEOArrayEntry *theArray = NULL;
	
	size_t	numPairs = inContext->currentInstruction->param1;
	for( size_t x = 0; x < numPairs; x++ )
	{
		size_t	currKeyIdx = x * 2;
		union LEOValue*	keyValue = inContext->stackEndPtr -(numPairs *2) +currKeyIdx;
		union LEOValue*	valueValue = inContext->stackEndPtr -(numPairs *2) +currKeyIdx +1;
		char	keyBuf[1024] = {0};
		const char*	keyStr = LEOGetValueAsString( keyValue, keyBuf, sizeof(keyBuf), inContext );
		LEOAddArrayEntryToRoot( &theArray, keyStr, valueValue, inContext );
	}
	
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -(numPairs * 2) );	// Remove array elements from stack.

	LEOInitArrayValue( (LEOValueArray*) inContext->stackEndPtr, theArray, kLEOInvalidateReferences, inContext );
	inContext->stackEndPtr++;
	
	inContext->currentInstruction++;
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
	
	LEOInitStringValue( (LEOValuePtr) inContext->stackEndPtr, theString, strlen(theString), kLEOInvalidateReferences, inContext );
	inContext->stackEndPtr++;
	
	inContext->currentInstruction++;
}


/*!
	Push the "unset" value on the stack as a LEOStringValue. (PUSH_UNSET_VALUE_INSTR)
*/

void	LEOPushUnsetValueInstruction( LEOContext* inContext )
{
	LEOInitUnsetValue( (LEOValuePtr) inContext->stackEndPtr, kLEOInvalidateReferences, inContext );
	inContext->stackEndPtr++;
	
	inContext->currentInstruction++;
}


/*!
	Take a string in the current script's string table and push it on the stack
	as a LEOStringValue. (PUSH_STR_VARIANT_FROM_TABLE_INSTR)
	
	param2	-	The index of the string table entry to retrieve.
*/

void	LEOPushStringVariantFromTableInstruction( LEOContext* inContext )
{
	const char*		theString = "";
	LEOScript*		script = LEOContextPeekCurrentScript( inContext );
	if( inContext->currentInstruction->param2 < script->numStrings )
		theString = script->strings[inContext->currentInstruction->param2];
	
	LEOInitStringVariantValue( (LEOValuePtr) inContext->stackEndPtr, theString, kLEOInvalidateReferences, inContext );
	inContext->stackEndPtr++;
	
	inContext->currentInstruction++;
}


/*!
	Pop the last value off the stack. (POP_VALUE_INSTR)
	
	param1	-	If this is not BACK_OF_STACK, we copy the value to that bp-relative
				stack location before we pop it.
*/

void	LEOPopValueInstruction( LEOContext* inContext )
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
	Pop the last value off the stack. (POP_SIMPLE_VALUE_INSTR)
	
	param1	-	If this is not BACK_OF_STACK, we copy the simple value of the
				last value on the stack to that bp-relative stack location
				before we pop it.
*/

void	LEOPopSimpleValueInstruction( LEOContext* inContext )
{
	bool			onStack = (inContext->currentInstruction->param1 == BACK_OF_STACK);
	union LEOValue*	destValue = onStack ? NULL : (inContext->stackBasePtr +(*(int16_t*)&inContext->currentInstruction->param1));
	
	if( destValue )
	{
		LEOCleanUpValue(destValue, kLEOKeepReferences, inContext);
		LEOInitSimpleCopy( inContext->stackEndPtr -1, destValue, kLEOKeepReferences, inContext );
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
	
	LEOSetValueAsString( theValue, theString, strlen(theString), inContext );	// TODO: Make NUL-safe.
	
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
	if( LEOGetValueAsNumber( theValue, NULL, inContext ) > 0 )
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
	if( LEOGetValueAsNumber( theValue, NULL, inContext ) < 0 )
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
	if( LEOGetValueAsNumber( theValue, NULL, inContext ) >= 0 )
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
	if( LEOGetValueAsNumber( theValue, NULL, inContext ) <= 0 )
		inContext->currentInstruction += LEOCastUInt32ToInt32( inContext->currentInstruction->param2 );
	else
		inContext->currentInstruction++;
	if( popOffStack )
		LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );
}


/*!
	Push the given LEONumber floating point quantity on the stack (PUSH_NUMBER_INSTR)
	
	param1	-	The LEOUnit for this number (kLEOUnitNone if it's really just a number).
	param2	-	The LEONumber (typecast to a uint32_t) to push.
*/

void	LEOPushNumberInstruction( LEOContext* inContext )
{
	LEOInitNumberValue( (LEOValuePtr) inContext->stackEndPtr, LEOCastUInt32ToLEONumber(inContext->currentInstruction->param2), inContext->currentInstruction->param1,
						kLEOInvalidateReferences, inContext );
	inContext->stackEndPtr++;

	inContext->currentInstruction++;
}


/*!
	Push the given 32-bit LEOInteger on the stack (PUSH_INTEGER_INSTR)
	
	param1	-	The LEOUnit for this integer (kLEOUnitNone if it's really just a number).
	param2	-	The LEOInteger (typecast to a uint32_t) to push.
*/

void	LEOPushIntegerInstruction( LEOContext* inContext )
{
	LEOInitIntegerValue( (LEOValuePtr) inContext->stackEndPtr, inContext->currentInstruction->param2, inContext->currentInstruction->param1,
							kLEOInvalidateReferences, inContext );
	inContext->stackEndPtr++;

	inContext->currentInstruction++;
}


/*!
	Push half of the given 64-bit LEOInteger on the stack (PUSH_INTEGER_START_INSTR)
	
	param1	-	The LEOUnit for this integer (kLEOUnitNone if it's really just a number).
	param2	-	The first 32 bits of the LEOInteger to push.
*/

void	LEOPushIntegerStartInstruction( LEOContext* inContext )
{
	uint64_t	topHalf = inContext->currentInstruction->param2;
	topHalf <<= 32;
	LEOInitIntegerValue( (LEOValuePtr) inContext->stackEndPtr, topHalf, inContext->currentInstruction->param1,
							kLEOInvalidateReferences, inContext );
	inContext->stackEndPtr++;

	inContext->currentInstruction++;
}


/*!
	Push half of the given 64-bit LEOInteger on the stack (ASSIGN_INTEGER_END_INSTR)
	
	param1	-	The LEOUnit for this integer (kLEOUnitNone if it's really just a number).
	param2	-	The second 32 bits of the LEOInteger to push.
*/

void	LEOAssignIntegerEndInstruction( LEOContext* inContext )
{
	uint64_t	bottomHalf = inContext->currentInstruction->param2;
	LEOValuePtr	theValue = inContext->stackEndPtr -1;
	
	bottomHalf |= (uint64_t)theValue->integer.integer;
	theValue->integer.integer = bottomHalf;
	
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
	LEONumber		theNum = LEOGetValueAsNumber( theValue, NULL, inContext );
	
	theNum += LEOCastUInt32ToLEONumber( inContext->currentInstruction->param2 );
	LEOSetValueAsNumber( theValue, theNum, kLEOUnitNone, inContext );
	
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
	LEOInteger		theNum = LEOGetValueAsInteger( theValue, NULL, inContext );
	
	theNum += LEOCastUInt32ToInt32( inContext->currentInstruction->param2 );
	LEOSetValueAsNumber( theValue, theNum, kLEOUnitNone, inContext );
	
	inContext->currentInstruction++;
}


/*!
	Call a given handler (CALL_HANDLER_INSTR)
	
	This saves off the current base pointer and the address of the next
	instruction so returning from the handler can restore the previous state,
	and retains the current script in case the script deletes its owner.
	
	This can be used to send a totally new message, or to pass a message
	up the message path.
	
	Push a value to hold the result of this call on the stack first, followed
	by the parameters in reverse order, and finally the parameter count.
	After this instruction returns, it is your responsibility to remove the
	pushed parameters, count and result from the stack again, e.g. by generating
	the requisite POP_VALUE_INSTR instructions.
	
	param1	-	Flags from eLEOCallHandlerFlags enum.
	param2	-	The LEOHandlerID of the handler to call.
	
	@seealso //leo_ref/c/func/LEOReturnFromHandlerInstruction LEOReturnFromHandlerInstruction
*/

void	LEOCallHandlerInstruction( LEOContext* inContext )
{
	bool			isMessagePassing = (inContext->currentInstruction->param1 & kLEOCallHandler_PassMessage) == kLEOCallHandler_PassMessage;
	LEOHandlerID	handlerName = inContext->currentInstruction->param2;
	if( inContext->group->messageSent && !isMessagePassing )
		inContext->group->messageSent( handlerName, inContext->group );

	LEOScript*		currScript = LEOContextPeekCurrentScript( inContext );
	if( isMessagePassing
		&& currScript && currScript->GetParentScript )
		currScript = currScript->GetParentScript( currScript, inContext, NULL );
	
	#if 0
	printf( "Calling handler '%s'\n", LEOContextGroupHandlerNameForHandlerID( inContext->group, handlerName ) );
	#endif
	
	LEOHandler*		foundHandler = NULL;
	if( currScript )
	{
		while( foundHandler == NULL )
		{
			if( (inContext->currentInstruction->param1 & kLEOCallHandler_IsFunctionFlag) == 0 )
				foundHandler = LEOScriptFindCommandHandlerWithID( currScript, handlerName );
			else
				foundHandler = LEOScriptFindFunctionHandlerWithID( currScript, handlerName );
			
			if( foundHandler )
			{
				LEOContextPushHandlerScriptReturnAddressAndBasePtr( inContext, foundHandler, currScript, inContext->currentInstruction +1, inContext->stackBasePtr );
				inContext->currentInstruction = foundHandler->instructions;
				inContext->stackBasePtr = inContext->stackEndPtr;
			}
			
			if( !foundHandler )
			{
				if( currScript->GetParentScript )
					currScript = currScript->GetParentScript( currScript, inContext, NULL );
				else
					currScript = NULL;
				if( !currScript )
					break;
			}
		}
	}
	
	if( !foundHandler )
	{
		if( inContext->callNonexistentHandlerProc )
			inContext->callNonexistentHandlerProc( inContext, handlerName, EMustBeHandled );
		else
		{
			size_t		lineNo = SIZE_T_MAX;
			uint16_t	fileID = 0;
			LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
			LEOContextStopWithError( inContext, lineNo, SIZE_T_MAX, fileID, "Couldn't find handler \"%s\".", LEOContextGroupHandlerNameForHandlerID( inContext->group, handlerName ) );
		}
		inContext->currentInstruction++;
	}
}


/*!
	Clean up the stack so that the parameters and local variables and temporaries
	allocated by LEOCallHandlerInstruction and the handler's actual code will be
	popped off the stack, leaving only the return value. (CLEAN_UP_HANDLER_STACK_INSTR)
	
	This method is intended for use by an actual handler that has been called and
	now wants to clean up behind itself without having to remember how many
	parameters it receives or what to do.
	
	<b>This instruction ignores currentInstruction, making it safe to call
	from other instructions without it looking at the wrong parameters,
	however it does advance the instruction pointer when it's done.</b>
	
	@seealso //leo_ref/c/func/LEOCleanUpHandlerParametersFromEndOfStack LEOCleanUpHandlerParametersFromEndOfStack
	@seealso //leo_ref/c/func/LEOCallHandlerInstruction LEOCallHandlerInstruction
	@seealso //leo_ref/c/func/LEOReturnFromHandlerInstruction LEOReturnFromHandlerInstruction
*/

void	LEOCleanUpHandlerStackInstruction( LEOContext* inContext )
{
	if( inContext->stackBasePtr != inContext->stackEndPtr )
	{
		union LEOValue*	paramCountValue = inContext->stackBasePtr -1;
		LEOInteger		paramCount = LEOGetValueAsNumber( paramCountValue, NULL, inContext );
		LEOCleanUpStackToPtr( inContext, inContext->stackBasePtr -1 -paramCount );
	}
	
	inContext->currentInstruction++;
}


/*!
	Clean up the stack so that the parameters allocated by LEOCallHandlerInstruction
	will be popped off the stack, leaving only the return value.
	
	This helper function is intended for the case where we attempted to call a handler
	but couldn't find it, and the handler has not set up its base pointer yet or
	allocated local variables, and we want to gracefully recover from this in some
	way.
	
	You could e.g. use this in your callNonexistentHandlerProc to clean up the stack
	after forwarding the parameters to and retrieving the return value from a native
	code plugin (in HyperCard parlance, an XCMD).
	
	@seealso //leo_ref/c/func/LEOCleanUpHandlerStackInstruction LEOCleanUpHandlerStackInstruction
	@seealso //leo_ref/c/func/LEOCallHandlerInstruction LEOCallHandlerInstruction
	@seealso //leo_ref/c/func/LEOReturnFromHandlerInstruction LEOReturnFromHandlerInstruction
*/

void	LEOCleanUpHandlerParametersFromEndOfStack( LEOContext* inContext )
{
	if( inContext->stackBasePtr != inContext->stackEndPtr )
	{
		union LEOValue*	paramCountValue = inContext->stackEndPtr -1;
		LEOInteger		paramCount = LEOGetValueAsNumber( paramCountValue, NULL, inContext );
		LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 -paramCount );
	}
}


/*!
	Grab the given parameter off the stack. May return NULL if there is no parameter
	at the given (zero-based) index.

	This helper function is intended for the case where we attempted to call a handler
	but couldn't find it, and the handler has not set up its base pointer yet or
	allocated local variables, and we want to gracefully recover from this in some
	way and need to look at the parameters to do that.
	
	You could e.g. use this in your callNonexistentHandlerProc to forward the parameters
	to and retrieving the return value from a native code plugin (in HyperCard parlance, an XCMD).
	
	@seealso //leo_ref/c/func/LEOCleanUpHandlerParametersFromEndOfStack LEOCleanUpHandlerParametersFromEndOfStack
*/

LEOValuePtr	LEOGetParameterAtIndexFromEndOfStack( LEOContext* inContext, LEOInteger paramIndex )
{
	if( inContext->stackBasePtr != inContext->stackEndPtr )
	{
		union LEOValue*	paramCountValue = inContext->stackEndPtr -1;
		LEOInteger		paramCount = LEOGetValueAsNumber( paramCountValue, NULL, inContext );
		if( paramCount < paramIndex )
			return NULL;
		return( inContext->stackEndPtr -1 -paramIndex );
	}
	return NULL;
}

/*!
	Return to the calling handler (RETURN_FROM_HANDLER_INSTR)
	
	This restores the previously-saved base pointer, jumps to the saved return
	address and releases its ownership of the current script (as established by
	CALL_HANDLER_INSTR).
	
	<b>This instruction ignores currentInstruction, making it safe to call
	from other instructions without it looking at the wrong parameters.</b>
	
	@seealso //leo_ref/c/func/LEOCallHandlerInstruction LEOCallHandlerInstruction
*/

void	LEOReturnFromHandlerInstruction( LEOContext* inContext )
{
	// No need to remove the parameters here, the caller is supposed to do this
	//	as they also know how many they pushed in the first place.
	
//    printf("About to return:\n");
//	LEOScript*	scr = LEOContextPeekCurrentScript( inContext );
//	LEODebugPrintScript( inContext->group, scr );
//	LEODebugPrintContext( inContext );
	
    inContext->currentInstruction = LEOContextPeekReturnAddress( inContext );
	inContext->stackBasePtr = LEOContextPeekBasePtr( inContext );
	LEOContextPopHandlerScriptReturnAddressAndBasePtr( inContext );
	
//	printf("Have just returned:\n");
//	if( inContext->currentInstruction != NULL )	// We didn't just exit the topmost handler?
//	{
//		scr = LEOContextPeekCurrentScript( inContext );
//		if( scr )
//			LEODebugPrintScript( inContext->group, scr );
//	}
//	LEODebugPrintContext( inContext );
}


/*!
	Pop a value off the back of the stack and copy it to the return value that
	our caller will look at when we return. (SET_RETURN_VALUE_INSTR)
*/

void	LEOSetReturnValueInstruction( LEOContext* inContext )
{
	union LEOValue*	paramCountValue = inContext->stackBasePtr -1;
	LEOInteger		paramCount = LEOGetValueAsNumber( paramCountValue, NULL, inContext );
	union LEOValue*	destValue = inContext->stackBasePtr -1 -paramCount -1;
	LEOCleanUpValue( destValue, kLEOKeepReferences, inContext );
	LEOInitSimpleCopy( inContext->stackEndPtr -1, destValue, kLEOKeepReferences, inContext );
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );
	
//    LEODebugPrintContext( inContext );
	
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
	union LEOValue	tmpRefValue = {};
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
	union LEOValue	tmpRefValue = {};
	LEOValuePtr		refValueOnStack = NULL;
	
	size_t	chunkStartOffs = LEOGetValueAsInteger(chunkStart,NULL,inContext) -1;
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;
	
	size_t	chunkEndOffs = LEOGetValueAsInteger(chunkEnd,NULL,inContext) -1;
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;
	
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -2 );
	
	LEOInitReferenceValue( &tmpRefValue, chunkTarget, kLEOInvalidateReferences, inContext->currentInstruction->param2, chunkStartOffs, chunkEndOffs, inContext );
	refValueOnStack = LEOPushValueOnStack( inContext, &tmpRefValue );
	
	inContext->currentInstruction++;
}


/*!
	Push a chunk out of a larger value onto the stack as a string value. (PUSH_CHUNK_INSTR)
	
	The chunk end and chunk start are popped off the back of the stack (in that order).
	
	param1	-	The basePtr-relative offset of the value to be referenced. If this is BACK_OF_STACK it will get the value from the stack, and expects it to have been pushed as the very first parameter.
	
	param2	-	The LEOChunkType of this chunk expression.
	
	@seealso //leo_ref/c/func/LEOGetChunkRanges LEOGetChunkRanges
*/

void	LEOPushChunkInstruction( LEOContext* inContext )
{
	LEOValuePtr		chunkTarget = NULL;
	bool			onStack = (inContext->currentInstruction->param1 == BACK_OF_STACK);
	if( onStack )
		chunkTarget = inContext->stackEndPtr -3;
	else
		chunkTarget = (inContext->stackBasePtr +(*(int16_t*)&inContext->currentInstruction->param1));
	LEOValuePtr		chunkEnd = inContext->stackEndPtr -1;
	LEOValuePtr		chunkStart = inContext->stackEndPtr -2;
	
	size_t	chunkStartOffs = LEOGetValueAsInteger(chunkStart,NULL,inContext) -1;
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;
	
	size_t	chunkEndOffs = LEOGetValueAsInteger(chunkEnd,NULL,inContext) -1;
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;
	
	char	str[1024] = { 0 };
	const char*	completeStr = LEOGetValueAsString( chunkTarget, str, sizeof(str), inContext );
	
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -2 );
	
	size_t	startDelOffs = 0, endDelOffs = 0;
	LEOGetChunkRanges( completeStr, inContext->currentInstruction->param2, chunkStartOffs, chunkEndOffs, &chunkStartOffs, &chunkEndOffs, &startDelOffs, &endDelOffs, inContext->itemDelimiter );
	LEOCleanUpValue( inContext->stackEndPtr -1, kLEOInvalidateReferences, inContext );
	LEOInitStringValue( inContext->stackEndPtr -1, completeStr +chunkStartOffs, chunkEndOffs -chunkStartOffs, kLEOInvalidateReferences, inContext );
	
	inContext->currentInstruction++;
}


/*!
	Change a property of a sub-range of an object. (SET_CHUNK_PROPERTY_INSTR)
	
	The property name, value, chunk end and chunk start are popped off the back of the stack (in that order).
	
	param1	-	The basePtr-relative offset of the value to be referenced. If this is BACK_OF_STACK it will get the value from the stack, and expects it to have been pushed as the very first parameter.
	
	param2	-	The LEOChunkType of this chunk expression.
	
	@seealso //leo_ref/c/func/LEOGetChunkRanges LEOGetChunkRanges
*/

void	LEOSetChunkPropertyInstruction( LEOContext* inContext )
{
	LEOValuePtr		chunkTarget = NULL;
	bool			onStack = (inContext->currentInstruction->param1 == BACK_OF_STACK);
	if( onStack )
		chunkTarget = inContext->stackEndPtr -5;
	else
		chunkTarget = (inContext->stackBasePtr +(*(int16_t*)&inContext->currentInstruction->param1));
	LEOValuePtr		propName = inContext->stackEndPtr -1;
	LEOValuePtr		propValue = inContext->stackEndPtr -2;
	LEOValuePtr		chunkEnd = inContext->stackEndPtr -3;
	LEOValuePtr		chunkStart = inContext->stackEndPtr -4;
	
	size_t	chunkStartOffs = LEOGetValueAsInteger(chunkStart,NULL,inContext) -1;
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;
	
	size_t	chunkEndOffs = LEOGetValueAsInteger(chunkEnd,NULL,inContext) -1;
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;

	char	propNameStr[1024] = { 0 };
	const char*	completePropNameStr = LEOGetValueAsString( propName, propNameStr, sizeof(propNameStr), inContext );
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;
	
	char	str[1024] = { 0 };
	const char*	completeStr = LEOGetValueAsString( chunkTarget, str, sizeof(str), inContext );
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;
	
	size_t	startDelOffs = 0, endDelOffs = 0;
	LEOGetChunkRanges( completeStr, inContext->currentInstruction->param2, chunkStartOffs, chunkEndOffs, &chunkStartOffs, &chunkEndOffs, &startDelOffs, &endDelOffs, inContext->itemDelimiter );
	LEOSetValueForKeyOfRange( chunkTarget, completePropNameStr, propValue, chunkStartOffs, chunkEndOffs, inContext );
	
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -4 -(onStack ? 1 : 0) );
	
	inContext->currentInstruction++;
}


/*!
	Push the value of a property of a chunk out of a larger value onto the stack as a string value. (PUSH_CHUNK_PROPERTY_INSTR)
	
	The name, chunk end and chunk start are popped off the back of the stack (in that order).
	
	param1	-	The basePtr-relative offset of the value to be referenced. If this is BACK_OF_STACK it will get the value from the stack, and expects it to have been pushed as the very first parameter.
	
	param2	-	The LEOChunkType of this chunk expression.
	
	@seealso //leo_ref/c/func/LEOGetChunkRanges LEOGetChunkRanges
*/

void	LEOPushChunkPropertyInstruction( LEOContext* inContext )
{
	/*
		We need to remove all but the topmost parameter from
		the stack, because the top param gets to hold the
		return value. However, to determine the return value,
		we need to still have the target value. Thus, we push
		the target value last, and replace the chunkStart (first
		on stack) with the result, and only *then* free all the
		others.
	*/
	
	LEOValuePtr		chunkTarget = NULL;
	size_t			shiftForTarget = 0;
	bool			onStack = (inContext->currentInstruction->param1 == BACK_OF_STACK);
	if( onStack )
	{
		chunkTarget = inContext->stackEndPtr -1;
		shiftForTarget = 1;
	}
	else
		chunkTarget = (inContext->stackBasePtr +(*(int16_t*)&inContext->currentInstruction->param1));
	LEOValuePtr		propName = inContext->stackEndPtr -1 -shiftForTarget;
	LEOValuePtr		chunkEnd = inContext->stackEndPtr -2 -shiftForTarget;
	LEOValuePtr		chunkStart = inContext->stackEndPtr -3 -shiftForTarget;
	
	size_t	chunkStartOffs = LEOGetValueAsInteger(chunkStart,NULL,inContext) -1;
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;
	
	size_t	chunkEndOffs = LEOGetValueAsInteger(chunkEnd,NULL,inContext) -1;
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;
	
	char	propNameStr[1024] = { 0 };
	const char*	completePropNameStr = LEOGetValueAsString( propName, propNameStr, sizeof(propNameStr), inContext );
	
	char	str[1024] = { 0 };
	const char*	completeStr = LEOGetValueAsString( chunkTarget, str, sizeof(str), inContext );
	
	size_t	startDelOffs = 0, endDelOffs = 0;
	LEOGetChunkRanges( completeStr, inContext->currentInstruction->param2, chunkStartOffs, chunkEndOffs, &chunkStartOffs, &chunkEndOffs, &startDelOffs, &endDelOffs, inContext->itemDelimiter );
	LEOCleanUpValue( chunkStart, kLEOInvalidateReferences, inContext );
	
	LEOGetValueForKeyOfRange( chunkTarget, completePropNameStr, chunkStartOffs, chunkEndOffs, chunkStart, inContext );
	
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -(onStack ? 3 : 2) );
	
	inContext->currentInstruction++;
}


/*!
	Copy the value of the parameter at given index into the given value on the
	stack. If no parameter of that index has been passed, this returns an empty
	string. (PARAMETER_INSTR)
	
	param1	-	The basePtr-relative offset of the value to be overwritten, or
				BACK_OF_STACK if you want the value to be pushed on the stack.
	
	param2	-	The number of the parameter to retrieve, as a 1-based index. If
				this is 0, grab the index from the back of the stack.
	
	@seealso //leo_ref/c/func/LEOParameterCountInstruction LEOParameterCountInstruction
*/

void	LEOParameterInstruction( LEOContext* inContext )
{
	bool		onStack = (inContext->currentInstruction->param1 == BACK_OF_STACK);
	int16_t		offset = (*(int16_t*)&inContext->currentInstruction->param1);
	LEOValuePtr	paramCountValue = inContext->stackBasePtr -1;
	LEOInteger	paramCount = LEOGetValueAsInteger( paramCountValue, NULL, inContext );
	int16_t		paramIndex = inContext->currentInstruction->param2;
	LEOValuePtr	valueTarget = NULL;
	
	if( paramIndex == 0 )
	{
		LEOValuePtr	paramIdxValue = inContext->stackEndPtr -1;	// Ignored if param2 isn't 0.
		paramIndex = LEOGetValueAsInteger( paramIdxValue, NULL, inContext );
		if( onStack )
		{
			valueTarget = inContext->stackEndPtr -1;
			LEOCleanUpValue( valueTarget, kLEOInvalidateReferences, inContext );
		}
		else
		{
			valueTarget = (inContext->stackBasePtr +offset);
			LEOCleanUpValue( valueTarget, kLEOKeepReferences, inContext );
		}
	}
	else
	{
		valueTarget = onStack ? (inContext->stackEndPtr++) : (inContext->stackBasePtr +offset);
		if( !onStack )
			LEOCleanUpValue( valueTarget, kLEOKeepReferences, inContext );
	}
	
	if( paramIndex <= paramCount && paramIndex > 0 )
	{
		LEOInitSimpleCopy( inContext->stackBasePtr -paramIndex -1, valueTarget,
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
	
	param2	-	The number of the parameter to retrieve, as a 1-based index. If
				this is 0, grab the index from the back of the stack.
	
	@seealso //leo_ref/c/func/LEOParameterCountInstruction LEOParameterCountInstruction
*/

void	LEOParameterKeepRefsInstruction( LEOContext* inContext )
{
	bool		onStack = (inContext->currentInstruction->param1 == BACK_OF_STACK);
	int16_t		offset = (*(int16_t*)&inContext->currentInstruction->param1);
	LEOValuePtr	paramCountValue = inContext->stackBasePtr -1;
	LEOInteger	paramCount = LEOGetValueAsInteger( paramCountValue, NULL, inContext );
	int16_t		paramIndex = inContext->currentInstruction->param2;
	LEOValuePtr	valueTarget = NULL;
	
	if( paramIndex == 0 )
	{
		LEOValuePtr	paramIdxValue = inContext->stackEndPtr -1;	// Ignored if param2 isn't 0.
		paramIndex = LEOGetValueAsInteger( paramIdxValue, NULL, inContext );
		if( onStack )
		{
			valueTarget = inContext->stackEndPtr -1;
			LEOCleanUpValue( valueTarget, kLEOInvalidateReferences, inContext );
		}
		else
		{
			valueTarget = (inContext->stackBasePtr +offset);
			LEOCleanUpValue( valueTarget, kLEOKeepReferences, inContext );
		}
	}
	else
	{
		valueTarget = onStack ? (inContext->stackEndPtr++) : (inContext->stackBasePtr +offset);
		if( !onStack )
			LEOCleanUpValue( valueTarget, kLEOKeepReferences, inContext );
	}
	
	if( paramIndex <= paramCount && paramIndex > 0 )
	{
		LEOInitCopy( inContext->stackBasePtr -paramIndex -1, valueTarget,
						(onStack ? kLEOInvalidateReferences : kLEOKeepReferences), inContext );
	}
	else
		LEOInitStringConstantValue( valueTarget, "", kLEOKeepReferences, inContext );
	
	inContext->currentInstruction++;
}


/*!
	Copy all parameters as an array and push it on the stack. (PUSH_PARAMETERS_INSTR)
	
	@seealso //leo_ref/c/func/LEOParameterCountInstruction LEOParameterCountInstruction
*/

void	LEOPushParametersInstruction( LEOContext* inContext )
{
	LEOValuePtr	valueTarget = inContext->stackEndPtr++;
	LEOInitStringConstantValue( valueTarget, "", kLEOInvalidateReferences, inContext );
	LEOValuePtr	paramCountValue = inContext->stackBasePtr -1;
	LEOInteger	paramCount = LEOGetValueAsNumber( paramCountValue, NULL, inContext );
	struct LEOArrayEntry *inArray = NULL;
	char	currKey[100] = {0};
	for( int x = 1; x <= paramCount; x++ )
	{
		snprintf( currKey, sizeof(currKey)-1, "%d", x );
		//LEODebugPrintContext( inContext );
		LEOAddArrayEntryToRoot( &inArray, currKey, inContext->stackBasePtr -x -1, inContext );	// 
	}
	if( inArray != NULL )
	{
		LEOCleanUpValue( valueTarget, kLEOInvalidateReferences, inContext );
		LEOInitArrayValue( &valueTarget->array, inArray, kLEOInvalidateReferences, inContext );
	}
	
	inContext->currentInstruction++;
}


/*!
	Determine the number of parameters that have been passed to this function
	(PARAMETER_COUNT_INSTR)
	
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
	LEOInteger	paramCount = LEOGetValueAsNumber( inContext->stackBasePtr -1, NULL, inContext );
	LEOInitIntegerValue( valueTarget, paramCount, kLEOUnitNone, (onStack ? kLEOInvalidateReferences : kLEOKeepReferences), inContext );
	
	inContext->currentInstruction++;
}


/*!
	(CONCATENATE_VALUES_INSTR)
*/

void	LEOConcatenateValuesInstruction( LEOContext* inContext )
{
	union LEOValue*	secondArgumentValue = inContext->stackEndPtr -1;
	union LEOValue*	firstArgumentValue = inContext->stackEndPtr -2;
	size_t			startOffs = 0, endOffs = SIZE_MAX,
					startDelOffs, endDelOffs;
	uint32_t		delimChar = inContext->currentInstruction->param2;
	char			tempStr[1024] = { 0 };	// TODO: Make this work with any length of string.
	char			tempStr2[1024] = { 0 };
	size_t			offs = 0;
	union LEOValue	resultValue;
	
	if( delimChar != 0 )
	{
		tempStr[0] = delimChar;		// TODO: Make this work with any Unicode character.
		offs = 1;
	}
	
	LEOGetValueAsString( secondArgumentValue, tempStr +offs, sizeof(tempStr) -offs, inContext );
	const char*		firstArgumentString = LEOGetValueAsString( firstArgumentValue, NULL, 0, inContext );
	if( !firstArgumentString )
		firstArgumentString = LEOGetValueAsString( firstArgumentValue, tempStr2, sizeof(tempStr2), inContext );
	LEOInitStringValue( &resultValue, firstArgumentString, strlen(firstArgumentString), kLEOInvalidateReferences, inContext );
	
	LEODetermineChunkRangeOfSubstring(	&resultValue, &startOffs, &endOffs,
										&startDelOffs, &endDelOffs,
										kLEOChunkTypeCharacter,
										SIZE_MAX, SIZE_MAX, inContext );
	LEOSetValuePredeterminedRangeAsString( &resultValue, endOffs, endOffs, tempStr, inContext );
	
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -2 );
	
	LEOPushValueOnStack( inContext, &resultValue );
	LEOCleanUpValue( &resultValue, kLEOInvalidateReferences, inContext );
	
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
	char			tempStr2[1024] = { 0 };
	size_t			offs = 0;
	union LEOValue	resultValue;
	
	if( delimChar == 0 )
		delimChar = ' ';

	tempStr[0] = delimChar;		// TODO: Make this work with any Unicode character.
	offs = 1;
	
	LEOGetValueAsString( secondArgumentValue, tempStr +offs, sizeof(tempStr) -offs, inContext );
	const char*		firstArgumentString = LEOGetValueAsString( firstArgumentValue, NULL, 0, inContext );
	if( !firstArgumentString )
		firstArgumentString = LEOGetValueAsString( firstArgumentValue, tempStr2, sizeof(tempStr2), inContext );
	LEOInitStringValue( &resultValue, firstArgumentString, strlen(firstArgumentString), kLEOInvalidateReferences, inContext );
		
	LEODetermineChunkRangeOfSubstring(	&resultValue, &startOffs, &endOffs,
										&startDelOffs, &endDelOffs,
										kLEOChunkTypeCharacter,
										SIZE_MAX, SIZE_MAX, inContext );
	LEOSetValuePredeterminedRangeAsString( &resultValue, endOffs, endOffs, tempStr, inContext );
	
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -2 );
	
	LEOPushValueOnStack( inContext, &resultValue );
	LEOCleanUpValue( &resultValue, kLEOInvalidateReferences, inContext );
	
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


void	LEONegateBooleanInstruction( LEOContext* inContext )
{
	union LEOValue*	firstArgumentValue = inContext->stackEndPtr -1;
	
	bool			firstArgument = LEOGetValueAsBoolean(firstArgumentValue,inContext);

	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );
	
	LEOPushBooleanOnStack( inContext, !firstArgument );
	
	inContext->currentInstruction++;
}


void	LEOSubtractCommandInstruction( LEOContext* inContext )
{
	union LEOValue*	secondArgumentValue = inContext->stackEndPtr -1;
	union LEOValue*	firstArgumentValue = inContext->stackEndPtr -2;
	
	LEOUnit			firstUnit = kLEOUnitNone;
	LEONumber		firstArgument = LEOGetValueAsNumber(firstArgumentValue,&firstUnit,inContext);
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;
	LEOUnit			secondUnit = kLEOUnitNone;
	LEONumber		secondArgument = LEOGetValueAsNumber(secondArgumentValue,&secondUnit,inContext);
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;
	
	LEOUnit	commonUnit = LEOConvertNumbersToCommonUnit( &firstArgument, firstUnit, &secondArgument, secondUnit );
	if( commonUnit == kLEOUnit_Last )
	{
		size_t		lineNo = SIZE_T_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_T_MAX, fileID, "Can't subtract apples from oranges, that'd give fruit punch." );
		return;
	}
	
	LEOSetValueAsNumber( secondArgumentValue, secondArgument -firstArgument, commonUnit, inContext );
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -2 );
	
	inContext->currentInstruction++;
}


void	LEOAddCommandInstruction( LEOContext* inContext )
{
	union LEOValue*	secondArgumentValue = inContext->stackEndPtr -1;
	union LEOValue*	firstArgumentValue = inContext->stackEndPtr -2;
	
	LEOUnit			firstUnit = kLEOUnitNone;
	LEONumber		firstArgument = LEOGetValueAsNumber(firstArgumentValue,&firstUnit,inContext);
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;
	LEOUnit			secondUnit = kLEOUnitNone;
	LEONumber		secondArgument = LEOGetValueAsNumber(secondArgumentValue,&secondUnit,inContext);
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;
	
	LEOUnit	commonUnit = LEOConvertNumbersToCommonUnit( &firstArgument, firstUnit, &secondArgument, secondUnit );
	if( commonUnit == kLEOUnit_Last )
	{
		size_t		lineNo = SIZE_T_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_T_MAX, fileID, "Can't subtract apples from oranges, that'd give fruit punch." );
		return;
	}

	LEOSetValueAsNumber( secondArgumentValue, firstArgument +secondArgument, commonUnit, inContext );
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -2 );
	
	inContext->currentInstruction++;
}


void	LEOMultiplyCommandInstruction( LEOContext* inContext )
{
	union LEOValue*	secondArgumentValue = inContext->stackEndPtr -1;
	union LEOValue*	firstArgumentValue = inContext->stackEndPtr -2;
	
	LEOUnit			firstUnit = kLEOUnitNone;
	LEONumber		firstArgument = LEOGetValueAsNumber(firstArgumentValue,&firstUnit,inContext);
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;
	LEOUnit			secondUnit = kLEOUnitNone;
	LEONumber		secondArgument = LEOGetValueAsNumber(secondArgumentValue,&secondUnit,inContext);
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;
	
	LEOUnit	commonUnit = LEOConvertNumbersToCommonUnit( &firstArgument, firstUnit, &secondArgument, secondUnit );
	if( commonUnit == kLEOUnit_Last )
	{
		size_t		lineNo = SIZE_T_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_T_MAX, fileID, "Can't subtract apples from oranges, that'd give fruit punch." );
		return;
	}

	LEOSetValueAsNumber( firstArgumentValue, firstArgument * secondArgument, commonUnit, inContext );
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -2 );
	
	inContext->currentInstruction++;
}


void	LEODivideCommandInstruction( LEOContext* inContext )
{
	union LEOValue*	secondArgumentValue = inContext->stackEndPtr -2;
	union LEOValue*	firstArgumentValue = inContext->stackEndPtr -1;
	
	LEOUnit			firstUnit = kLEOUnitNone;
	LEONumber		firstArgument = LEOGetValueAsNumber(firstArgumentValue,&firstUnit,inContext);
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;
	LEOUnit			secondUnit = kLEOUnitNone;
	LEONumber		secondArgument = LEOGetValueAsNumber(secondArgumentValue,&secondUnit,inContext);
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;
	
	if( secondArgument == 0.0 )
	{
		size_t		lineNo = SIZE_T_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_T_MAX, fileID, "Can't divide %g by 0.", firstArgument );
		return;
	}
	
	LEOUnit	commonUnit = LEOConvertNumbersToCommonUnit( &firstArgument, firstUnit, &secondArgument, secondUnit );
	if( commonUnit == kLEOUnit_Last )
	{
		size_t		lineNo = SIZE_T_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_T_MAX, fileID, "Can't subtract apples from oranges, that'd give fruit punch." );
		return;
	}
	
	LEOSetValueAsNumber( firstArgumentValue, firstArgument / secondArgument, commonUnit, inContext );
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -2 );
		
	inContext->currentInstruction++;
}


void	LEOSubtractOperatorInstruction( LEOContext* inContext )
{
	union LEOValue*	secondArgumentValue = inContext->stackEndPtr -1;
	union LEOValue*	firstArgumentValue = inContext->stackEndPtr -2;
	
	LEOUnit			firstUnit = kLEOUnitNone;
	LEONumber		firstArgument = LEOGetValueAsNumber(firstArgumentValue,&firstUnit,inContext);
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;
	LEOUnit			secondUnit = kLEOUnitNone;
	LEONumber		secondArgument = LEOGetValueAsNumber(secondArgumentValue,&secondUnit,inContext);
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;

	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -2 );
	
	LEOUnit	commonUnit = LEOConvertNumbersToCommonUnit( &firstArgument, firstUnit, &secondArgument, secondUnit );
	if( commonUnit == kLEOUnit_Last )
	{
		size_t		lineNo = SIZE_T_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_T_MAX, fileID, "Can't subtract apples from oranges, that'd give fruit punch." );
		return;
	}
	
	LEOPushNumberOnStack( inContext, firstArgument -secondArgument, commonUnit );
	
	inContext->currentInstruction++;
}


void	LEOAddOperatorInstruction( LEOContext* inContext )
{
	union LEOValue*	secondArgumentValue = inContext->stackEndPtr -1;
	union LEOValue*	firstArgumentValue = inContext->stackEndPtr -2;
	
	LEOUnit			firstUnit = kLEOUnitNone;
	LEONumber		firstArgument = LEOGetValueAsNumber(firstArgumentValue,&firstUnit,inContext);
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;
	LEOUnit			secondUnit = kLEOUnitNone;
	LEONumber		secondArgument = LEOGetValueAsNumber(secondArgumentValue,&secondUnit,inContext);
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;

	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -2 );
	
	LEOUnit	commonUnit = LEOConvertNumbersToCommonUnit( &firstArgument, firstUnit, &secondArgument, secondUnit );
	if( commonUnit == kLEOUnit_Last )
	{
		size_t		lineNo = SIZE_T_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_T_MAX, fileID, "Can't subtract apples from oranges, that'd give fruit punch." );
		return;
	}
	
	LEOPushNumberOnStack( inContext, firstArgument +secondArgument, commonUnit );
	
	inContext->currentInstruction++;
}


void	LEOMultiplyOperatorInstruction( LEOContext* inContext )
{
	union LEOValue*	secondArgumentValue = inContext->stackEndPtr -1;
	union LEOValue*	firstArgumentValue = inContext->stackEndPtr -2;
	
	LEOUnit			firstUnit = kLEOUnitNone;
	LEONumber		firstArgument = LEOGetValueAsNumber(firstArgumentValue,&firstUnit,inContext);
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;
	LEOUnit			secondUnit = kLEOUnitNone;
	LEONumber		secondArgument = LEOGetValueAsNumber(secondArgumentValue,&secondUnit,inContext);
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;

	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -2 );
	
	LEOUnit	commonUnit = LEOConvertNumbersToCommonUnit( &firstArgument, firstUnit, &secondArgument, secondUnit );
	if( commonUnit == kLEOUnit_Last )
	{
		size_t		lineNo = SIZE_T_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_T_MAX, fileID, "Can't subtract apples from oranges, that'd give fruit punch." );
		return;
	}
	
	LEOPushNumberOnStack( inContext, firstArgument * secondArgument, commonUnit );
	
	inContext->currentInstruction++;
}


void	LEODivideOperatorInstruction( LEOContext* inContext )
{
	union LEOValue*	secondArgumentValue = inContext->stackEndPtr -1;
	union LEOValue*	firstArgumentValue = inContext->stackEndPtr -2;
	
	LEOUnit			firstUnit = kLEOUnitNone;
	LEONumber		firstArgument = LEOGetValueAsNumber(firstArgumentValue,&firstUnit,inContext);
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;
	LEOUnit			secondUnit = kLEOUnitNone;
	LEONumber		secondArgument = LEOGetValueAsNumber(secondArgumentValue,&secondUnit,inContext);
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;

	if( secondArgument == 0.0 )
	{
		size_t		lineNo = SIZE_T_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_T_MAX, fileID, "Can't divide %g by 0.", firstArgument );	// Causes interpreter loop to exit.
		return;
	}

	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -2 );
	
	LEOUnit	commonUnit = LEOConvertNumbersToCommonUnit( &firstArgument, firstUnit, &secondArgument, secondUnit );
	if( commonUnit == kLEOUnit_Last )
	{
		size_t		lineNo = SIZE_T_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_T_MAX, fileID, "Can't subtract apples from oranges, that'd give fruit punch." );
		return;
	}
	
	LEOPushNumberOnStack( inContext, firstArgument / secondArgument, commonUnit );
	
	inContext->currentInstruction++;
}


void	LEOGreaterThanOperatorInstruction( LEOContext* inContext )
{
	union LEOValue*	secondArgumentValue = inContext->stackEndPtr -1;
	union LEOValue*	firstArgumentValue = inContext->stackEndPtr -2;
	
	bool			isEqual = false;
	
	if( LEOCanGetAsNumber(firstArgumentValue, inContext) && LEOCanGetAsNumber(secondArgumentValue, inContext) )
	{
		LEOUnit			firstUnit = kLEOUnitNone;
		LEONumber		firstArgument = LEOGetValueAsNumber(firstArgumentValue,&firstUnit,inContext);
		LEOUnit			secondUnit = kLEOUnitNone;
		LEONumber		secondArgument = LEOGetValueAsNumber(secondArgumentValue,&secondUnit,inContext);
	
		LEOUnit	commonUnit = LEOConvertNumbersToCommonUnit( &firstArgument, firstUnit, &secondArgument, secondUnit );
		if( commonUnit == kLEOUnit_Last )
		{
			size_t		lineNo = SIZE_T_MAX;
			uint16_t	fileID = 0;
			LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
			LEOContextStopWithError( inContext, lineNo, SIZE_T_MAX, fileID, "Can't subtract apples from oranges, that'd give fruit punch." );
			return;
		}
		
		isEqual = (firstArgument > secondArgument);
	}
	else
	{
		char		firstArgumentBuf[1024] = { 0 };
		char		secondArgumentBuf[1024] = { 0 };
		const char* firstArgumentStr = LEOGetValueAsString(firstArgumentValue, firstArgumentBuf, sizeof(firstArgumentBuf), inContext);
		const char* secondArgumentStr = LEOGetValueAsString(secondArgumentValue, secondArgumentBuf, sizeof(secondArgumentBuf), inContext);
		
//		printf("COMPARING \"%s", LEOStringEscapedForPrintingInQuotes(firstArgumentStr));
//		printf("\" > \"%s\"\n", LEOStringEscapedForPrintingInQuotes(secondArgumentStr));
		
		isEqual = (strcasecmp(firstArgumentStr, secondArgumentStr) > 0);
	}

	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -2 );
	
	LEOPushBooleanOnStack( inContext, isEqual );
	
	inContext->currentInstruction++;
}


void	LEOLessThanOperatorInstruction( LEOContext* inContext )
{
	union LEOValue*	secondArgumentValue = inContext->stackEndPtr -1;
	union LEOValue*	firstArgumentValue = inContext->stackEndPtr -2;
	
	bool			isEqual = false;
	
	if( LEOCanGetAsNumber(firstArgumentValue, inContext) && LEOCanGetAsNumber(secondArgumentValue, inContext) )
	{
		LEOUnit			firstUnit = kLEOUnitNone;
		LEONumber		firstArgument = LEOGetValueAsNumber(firstArgumentValue,&firstUnit,inContext);
		LEOUnit			secondUnit = kLEOUnitNone;
		LEONumber		secondArgument = LEOGetValueAsNumber(secondArgumentValue,&secondUnit,inContext);
	
		LEOUnit	commonUnit = LEOConvertNumbersToCommonUnit( &firstArgument, firstUnit, &secondArgument, secondUnit );
		if( commonUnit == kLEOUnit_Last )
		{
			size_t		lineNo = SIZE_T_MAX;
			uint16_t	fileID = 0;
			LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
			LEOContextStopWithError( inContext, lineNo, SIZE_T_MAX, fileID, "Can't subtract apples from oranges, that'd give fruit punch." );
			return;
		}
		
		isEqual = (firstArgument < secondArgument);
	}
	else
	{
		char		firstArgumentBuf[1024] = { 0 };
		char		secondArgumentBuf[1024] = { 0 };
		const char* firstArgumentStr = LEOGetValueAsString(firstArgumentValue, firstArgumentBuf, sizeof(firstArgumentBuf), inContext);
		const char* secondArgumentStr = LEOGetValueAsString(secondArgumentValue, secondArgumentBuf, sizeof(secondArgumentBuf), inContext);
		
//		printf("COMPARING \"%s", LEOStringEscapedForPrintingInQuotes(firstArgumentStr));
//		printf("\" < \"%s\"\n", LEOStringEscapedForPrintingInQuotes(secondArgumentStr));
		
		isEqual = (strcasecmp(firstArgumentStr, secondArgumentStr) < 0);
	}

	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -2 );
	
	LEOPushBooleanOnStack( inContext, isEqual );
	
	inContext->currentInstruction++;
}


void	LEOGreaterThanEqualOperatorInstruction( LEOContext* inContext )
{
	union LEOValue*	secondArgumentValue = inContext->stackEndPtr -1;
	union LEOValue*	firstArgumentValue = inContext->stackEndPtr -2;
	
	bool			isEqual = false;
	
	if( LEOCanGetAsNumber(firstArgumentValue, inContext) && LEOCanGetAsNumber(secondArgumentValue, inContext) )
	{
		LEOUnit			firstUnit = kLEOUnitNone;
		LEONumber		firstArgument = LEOGetValueAsNumber(firstArgumentValue,&firstUnit,inContext);
		LEOUnit			secondUnit = kLEOUnitNone;
		LEONumber		secondArgument = LEOGetValueAsNumber(secondArgumentValue,&secondUnit,inContext);
	
		LEOUnit	commonUnit = LEOConvertNumbersToCommonUnit( &firstArgument, firstUnit, &secondArgument, secondUnit );
		if( commonUnit == kLEOUnit_Last )
		{
			size_t		lineNo = SIZE_T_MAX;
			uint16_t	fileID = 0;
			LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
			LEOContextStopWithError( inContext, lineNo, SIZE_T_MAX, fileID, "Can't subtract apples from oranges, that'd give fruit punch." );
			return;
		}
		
		isEqual = (firstArgument >= secondArgument);
	}
	else
	{
		char		firstArgumentBuf[1024] = { 0 };
		char		secondArgumentBuf[1024] = { 0 };
		const char* firstArgumentStr = LEOGetValueAsString(firstArgumentValue, firstArgumentBuf, sizeof(firstArgumentBuf), inContext);
		const char* secondArgumentStr = LEOGetValueAsString(secondArgumentValue, secondArgumentBuf, sizeof(secondArgumentBuf), inContext);
		
//		printf("COMPARING \"%s", LEOStringEscapedForPrintingInQuotes(firstArgumentStr));
//		printf("\" >= \"%s\"\n", LEOStringEscapedForPrintingInQuotes(secondArgumentStr));
		
		isEqual = (strcasecmp(firstArgumentStr, secondArgumentStr) >= 0);
	}

	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -2 );
	
	LEOPushBooleanOnStack( inContext, isEqual );
	
	inContext->currentInstruction++;
}


void	LEOLessThanEqualOperatorInstruction( LEOContext* inContext )
{
	union LEOValue*	secondArgumentValue = inContext->stackEndPtr -1;
	union LEOValue*	firstArgumentValue = inContext->stackEndPtr -2;
	
	bool			isEqual = false;
	
	if( LEOCanGetAsNumber(firstArgumentValue, inContext) && LEOCanGetAsNumber(secondArgumentValue, inContext) )
	{
		LEOUnit			firstUnit = kLEOUnitNone;
		LEONumber		firstArgument = LEOGetValueAsNumber(firstArgumentValue,&firstUnit,inContext);
		LEOUnit			secondUnit = kLEOUnitNone;
		LEONumber		secondArgument = LEOGetValueAsNumber(secondArgumentValue,&secondUnit,inContext);
	
		LEOUnit	commonUnit = LEOConvertNumbersToCommonUnit( &firstArgument, firstUnit, &secondArgument, secondUnit );
		if( commonUnit == kLEOUnit_Last )
		{
			size_t		lineNo = SIZE_T_MAX;
			uint16_t	fileID = 0;
			LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
			LEOContextStopWithError( inContext, lineNo, SIZE_T_MAX, fileID, "Can't subtract apples from oranges, that'd give fruit punch." );
			return;
		}
		
		isEqual = (firstArgument <= secondArgument);
	}
	else
	{
		char		firstArgumentBuf[1024] = { 0 };
		char		secondArgumentBuf[1024] = { 0 };
		const char* firstArgumentStr = LEOGetValueAsString(firstArgumentValue, firstArgumentBuf, sizeof(firstArgumentBuf), inContext);
		const char* secondArgumentStr = LEOGetValueAsString(secondArgumentValue, secondArgumentBuf, sizeof(secondArgumentBuf), inContext);
		
//		printf("COMPARING \"%s", LEOStringEscapedForPrintingInQuotes(firstArgumentStr));
//		printf("\" <= \"%s\"\n", LEOStringEscapedForPrintingInQuotes(secondArgumentStr));
		
		isEqual = (strcasecmp(firstArgumentStr, secondArgumentStr) <= 0);
	}

	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -2 );
	
	LEOPushBooleanOnStack( inContext, isEqual );
	
	inContext->currentInstruction++;
}


void	LEONegateNumberInstruction( LEOContext* inContext )
{
	union LEOValue*	firstArgumentValue = inContext->stackEndPtr -1;
	
	LEOUnit				firstUnit = kLEOUnitNone;
	LEONumber			firstArgument = LEOGetValueAsNumber(firstArgumentValue,&firstUnit,inContext);
	
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );
	
	LEOPushNumberOnStack( inContext, -firstArgument, firstUnit );
	
	inContext->currentInstruction++;
}


void	LEOModuloOperatorInstruction( LEOContext* inContext )
{
	union LEOValue*	secondArgumentValue = inContext->stackEndPtr -1;
	union LEOValue*	firstArgumentValue = inContext->stackEndPtr -2;
	
	LEOUnit			firstUnit = kLEOUnitNone;
	LEONumber		firstArgument = LEOGetValueAsNumber(firstArgumentValue,&firstUnit,inContext);
	LEOUnit			secondUnit = kLEOUnitNone;
	LEONumber		secondArgument = LEOGetValueAsNumber(secondArgumentValue,&secondUnit,inContext);

	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -2 );
	
	LEOUnit	commonUnit = LEOConvertNumbersToCommonUnit( &firstArgument, firstUnit, &secondArgument, secondUnit );
	if( commonUnit == kLEOUnit_Last )
	{
		size_t		lineNo = SIZE_T_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_T_MAX, fileID, "Can't subtract apples from oranges, that'd give fruit punch." );
		return;
	}
	
	LEOPushNumberOnStack( inContext, fmod(firstArgument, secondArgument), commonUnit );
	
	inContext->currentInstruction++;
}


void	LEOPowerOperatorInstruction( LEOContext* inContext )
{
	union LEOValue*	secondArgumentValue = inContext->stackEndPtr -1;
	union LEOValue*	firstArgumentValue = inContext->stackEndPtr -2;
	
	LEOUnit			firstUnit = kLEOUnitNone;
	LEONumber		firstArgument = LEOGetValueAsNumber(firstArgumentValue,&firstUnit,inContext);
	LEOUnit			secondUnit = kLEOUnitNone;
	LEONumber		secondArgument = LEOGetValueAsNumber(secondArgumentValue,&secondUnit,inContext);

	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -2 );
	
	LEOUnit	commonUnit = LEOConvertNumbersToCommonUnit( &firstArgument, firstUnit, &secondArgument, secondUnit );
	if( commonUnit == kLEOUnit_Last )
	{
		size_t		lineNo = SIZE_T_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_T_MAX, fileID, "Can't subtract apples from oranges, that'd give fruit punch." );
		return;
	}
	
	LEOPushNumberOnStack( inContext, pow(firstArgument, secondArgument), commonUnit );
	
	inContext->currentInstruction++;
}


void	LEOEqualOperatorInstruction( LEOContext* inContext )
{
	union LEOValue*	secondArgumentValue = inContext->stackEndPtr -1;
	union LEOValue*	firstArgumentValue = inContext->stackEndPtr -2;
	
	bool			isEqual = false;
	
	if( LEOCanGetAsNumber(firstArgumentValue, inContext) && LEOCanGetAsNumber(secondArgumentValue, inContext) )
	{
		LEOUnit			firstUnit = kLEOUnitNone;
		LEONumber		firstArgument = LEOGetValueAsNumber(firstArgumentValue,&firstUnit,inContext);
		LEOUnit			secondUnit = kLEOUnitNone;
		LEONumber		secondArgument = LEOGetValueAsNumber(secondArgumentValue,&secondUnit,inContext);
	
		LEOUnit	commonUnit = LEOConvertNumbersToCommonUnit( &firstArgument, firstUnit, &secondArgument, secondUnit );
		if( commonUnit == kLEOUnit_Last )
		{
			size_t		lineNo = SIZE_T_MAX;
			uint16_t	fileID = 0;
			LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
			LEOContextStopWithError( inContext, lineNo, SIZE_T_MAX, fileID, "Can't subtract apples from oranges, that'd give fruit punch." );
			return;
		}
		
		isEqual = (firstArgument == secondArgument);
	}
	else
	{
		char		firstArgumentBuf[1024] = { 0 };
		char		secondArgumentBuf[1024] = { 0 };
		const char* firstArgumentStr = LEOGetValueAsString(firstArgumentValue, firstArgumentBuf, sizeof(firstArgumentBuf), inContext);
		const char* secondArgumentStr = LEOGetValueAsString(secondArgumentValue, secondArgumentBuf, sizeof(secondArgumentBuf), inContext);
		
//		printf("COMPARING \"%s", LEOStringEscapedForPrintingInQuotes(firstArgumentStr));
//		printf("\" == \"%s\"\n", LEOStringEscapedForPrintingInQuotes(secondArgumentStr));
		
		isEqual = (strcasecmp(firstArgumentStr, secondArgumentStr) == 0);
	}
	
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -2 );
	
	LEOPushBooleanOnStack( inContext, isEqual );
	
	inContext->currentInstruction++;
}


void	LEONotEqualOperatorInstruction( LEOContext* inContext )
{
	union LEOValue*	secondArgumentValue = inContext->stackEndPtr -1;
	union LEOValue*	firstArgumentValue = inContext->stackEndPtr -2;
	
	bool			isEqual = false;
	
	if( LEOCanGetAsNumber(firstArgumentValue, inContext) && LEOCanGetAsNumber(secondArgumentValue, inContext) )
	{
		LEOUnit			firstUnit = kLEOUnitNone;
		LEONumber		firstArgument = LEOGetValueAsNumber(firstArgumentValue,&firstUnit,inContext);
		LEOUnit			secondUnit = kLEOUnitNone;
		LEONumber		secondArgument = LEOGetValueAsNumber(secondArgumentValue,&secondUnit,inContext);
	
		LEOUnit	commonUnit = LEOConvertNumbersToCommonUnit( &firstArgument, firstUnit, &secondArgument, secondUnit );
		if( commonUnit == kLEOUnit_Last )
		{
			size_t		lineNo = SIZE_T_MAX;
			uint16_t	fileID = 0;
			LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
			LEOContextStopWithError( inContext, lineNo, SIZE_T_MAX, fileID, "Can't subtract apples from oranges, that'd give fruit punch." );
			return;
		}
		
		isEqual = (firstArgument != secondArgument);
	}
	else
	{
		char		firstArgumentBuf[1024] = { 0 };
		char		secondArgumentBuf[1024] = { 0 };
		const char* firstArgumentStr = LEOGetValueAsString(firstArgumentValue, firstArgumentBuf, sizeof(firstArgumentBuf), inContext);
		const char* secondArgumentStr = LEOGetValueAsString(secondArgumentValue, secondArgumentBuf, sizeof(secondArgumentBuf), inContext);
		
//		printf("COMPARING \"%s", LEOStringEscapedForPrintingInQuotes(firstArgumentStr));
//		printf("\" != \"%s\"\n", LEOStringEscapedForPrintingInQuotes(secondArgumentStr));
		
		isEqual = (strcasecmp(firstArgumentStr, secondArgumentStr) != 0);
	}
	
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -2 );
	
	LEOPushBooleanOnStack( inContext, isEqual );
	
	inContext->currentInstruction++;
}


/*!
	This instruction does nothing. It just advances to the next instruction.
	(LINE_MARKER_INSTR)
	
	param2		-	The line number.
*/

void	LEOLineMarkerInstruction( LEOContext* inContext )
{
	// Do nothing.

	inContext->currentInstruction++;
}


struct LEOAssignChunkArrayUserData
{
	struct LEOArrayEntry *	array;
	size_t					numItems;
	struct LEOContext	*	context;
};


static bool LEOAssignChunkArrayChunkCallback( const char *currStr, size_t currLen, size_t currStart, size_t currEnd, void *userData )
{
	struct LEOAssignChunkArrayUserData	*	ud = (struct LEOAssignChunkArrayUserData *) userData;
	char									keyString[20] = { 0 };
	snprintf( keyString, sizeof(keyString) -1, "%lu", ++ud->numItems );
	
	union LEOValue		tempStringValue = {};
	LEOInitStringValue( &tempStringValue, currStr, currLen, kLEOInvalidateReferences, ud->context );
	LEOAddArrayEntryToRoot( &ud->array, keyString, &tempStringValue, ud->context );
	LEOCleanUpValue( &tempStringValue, kLEOInvalidateReferences, ud->context );
	
	return true;
}


/*!
	@function LEOAssignChunkArrayInstruction
	Build an array containing each chunk item (i.e. item, line or word) in a
	given value's string representation.
	You must push the value whose chunks you want to get (or a reference to it)
	on the stack before calling this.
	
	param1		-	BP-relative address of at which you want the array to be
					created, or BACK_OF_STACK to push it on the back of the stack.
	param2		-	The chunk type to use.
*/

void	LEOAssignChunkArrayInstruction( LEOContext* inContext )
{
	union LEOValue	*		srcValue = inContext->stackEndPtr -1;
	struct LEOAssignChunkArrayUserData	userData = { 0 };
	userData.context = inContext;
	char					tempStr[1024] = { 0 };	// TODO: Make this work with any length of string.
	
	LEOGetValueAsString( srcValue, tempStr, sizeof(tempStr), inContext );
	LEOCleanUpStackToPtr( inContext, srcValue );	// Pop srcValue off the stack.
	
	bool			onStack = (inContext->currentInstruction->param1 == BACK_OF_STACK);
	LEOValuePtr		dstValue = onStack ? (inContext->stackEndPtr++) : (inContext->stackBasePtr +(*(int16_t*)&inContext->currentInstruction->param1));
	if( !onStack )
		LEOCleanUpValue( dstValue, kLEOKeepReferences, inContext );
	
	LEODoForEachChunk( tempStr, strlen(tempStr), inContext->currentInstruction->param2, LEOAssignChunkArrayChunkCallback, inContext->itemDelimiter, &userData );
	LEOInitArrayValue( &dstValue->array, userData.array, kLEOKeepReferences, inContext );

	inContext->currentInstruction++;
}


struct LEOCountChunksUserData
{
	size_t					numItems;
	struct LEOContext	*	context;
};


static bool LEOCountChunksChunkCallback( const char *currStr, size_t currLen, size_t currStart, size_t currEnd, void *userData )
{
	size_t	*	numItems = (size_t *) userData;
	++(*numItems);
		
	return true;
}


/*!
	@function LEOCountChunksInstruction
	Determine the number of chunks of the given type in a value's string
	representation.
	
	param2		-	The chunk type to use.
*/

void	LEOCountChunksInstruction( LEOContext* inContext )
{
	union LEOValue	*		srcValue = inContext->stackEndPtr -1;
	int						numItems = 0;
	char					tempStr[1024] = { 0 };	// TODO: Make this work with any length of string.
	
	LEOGetValueAsString( srcValue, tempStr, sizeof(tempStr), inContext );
	
	LEODoForEachChunk( tempStr, strlen(tempStr), inContext->currentInstruction->param2, LEOCountChunksChunkCallback, inContext->itemDelimiter, &numItems );

	inContext->currentInstruction++;
}


/*!
	@function LEOGetArrayItemInstruction
	Fetch an item out of an array, or an empty string if there is no such item.
	The key for the array item to be fetched, and the array value, must have been
	pushed on the stack before. (GET_ARRAY_ITEM_INSTR)
	
	param1		-	Destination BP-relative address, or BACK_OF_STACK.
*/


void	LEOGetArrayItemInstruction( LEOContext* inContext )
{
	bool					onStack = (inContext->currentInstruction->param1 == BACK_OF_STACK);
	LEOValuePtr				dstValue = onStack ? (inContext->stackEndPtr++) : (inContext->stackBasePtr +(*(int16_t*)&inContext->currentInstruction->param1));
	if( !onStack )
		LEOCleanUpValue( dstValue, kLEOKeepReferences, inContext );
	union LEOValue	*		keyValue = inContext->stackEndPtr -2;
	union LEOValue	*		srcValue = inContext->stackEndPtr -1;
	
	char					keyStr[1024] = { 0 };	// TODO: Make this work with any length of string.
	
	
	LEOGetValueAsString( keyValue, keyStr, sizeof(keyStr), inContext );	
	LEOValuePtr		foundItem = LEOGetValueForKey( srcValue, keyStr, dstValue, (onStack ? kLEOInvalidateReferences : kLEOKeepReferences), inContext );
	if( foundItem == NULL )
		LEOInitUnsetValue( dstValue, (onStack ? kLEOInvalidateReferences : kLEOKeepReferences), inContext );
	else if( foundItem != dstValue )
		LEOInitSimpleCopy( foundItem, dstValue, (onStack ? kLEOInvalidateReferences : kLEOKeepReferences), inContext );
	
	inContext->currentInstruction++;
}


/*!
	@function LEOGetArrayItemCountInstruction
	Write the number of items in an array to a value. (GET_ARRAY_ITEM_COUNT_INSTR)
	The the array value must have been pushed on the stack before.
	
	param1		-	Destination BP-relative address, or BACK_OF_STACK.
*/


void	LEOGetArrayItemCountInstruction( LEOContext* inContext )
{
	bool					onStack = (inContext->currentInstruction->param1 == BACK_OF_STACK);
	LEOValuePtr				dstValue = onStack ? (inContext->stackEndPtr++) : (inContext->stackBasePtr +(*(int16_t*)&inContext->currentInstruction->param1));
	if( !onStack )
		LEOCleanUpValue( dstValue, kLEOKeepReferences, inContext );
	union LEOValue	*		srcValue = inContext->stackEndPtr -1;
	
	size_t	numKeys = LEOGetKeyCount( srcValue, inContext );
	LEOInitIntegerValue( dstValue, numKeys, kLEOUnitNone, (onStack ? kLEOInvalidateReferences : kLEOKeepReferences), inContext );
	
	inContext->currentInstruction++;
}


/*!
	Pop the last value off the stack, evaluate it as a string, and then assign it to the value at the given bp-relative address. If param1 is BACK_OF_STACK, the penultimate item on the stack will be used, and popped off as well.
	(SET_STRING_INSTRUCTION)
*/

void	LEOSetStringInstruction( LEOContext* inContext )
{
	bool			onStack = (inContext->currentInstruction->param1 == BACK_OF_STACK);
	union LEOValue*	destValue = onStack ? (inContext->stackEndPtr -2) : inContext->stackBasePtr +inContext->currentInstruction->param1;
	char			strBuf[1024] = { 0 };
	const char*		str = LEOGetValueAsString( inContext->stackEndPtr -1, strBuf, sizeof(strBuf), inContext );
	LEOSetValueAsString( destValue, str, strlen(str), inContext );	// TODO: Make NUL-safe.
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr +(onStack ? -2 : -1) );
	
	inContext->currentInstruction++;
}


void	LEOPushItemDelimiterInstruction( LEOContext* inContext )
{
	char		delimStr[8] = { 0 };
	delimStr[0] = inContext->itemDelimiter;	// TODO: Make this work with more than 1-byte characters.
	LEOPushStringValueOnStack( inContext, delimStr, strlen(delimStr) );
	
	inContext->currentInstruction++;
}


void	LEOSetItemDelimiterInstruction( LEOContext* inContext )
{
	char		delimStr[8] = { 0 };
	LEOGetValueAsString( inContext->stackEndPtr -1, delimStr, sizeof(delimStr), inContext );
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );

	inContext->itemDelimiter = delimStr[0];	// TODO: Make this work with more than 1-byte characters.
	
	inContext->currentInstruction++;
}


void	LEOPushGlobalReferenceInstruction( LEOContext* inContext )
{
	char		globalNameBuf[1024] = { 0 };
	const char*	globalName = LEOGetValueAsString( inContext->stackEndPtr -1, globalNameBuf, sizeof(globalNameBuf), inContext );
	
	LEOValuePtr	theGlobal = LEOGetArrayValueForKey( inContext->group->globals, globalName );
	if( !theGlobal )
	{
		union LEOValue		emptyString = {};
		LEOInitStringVariantValue( &emptyString, "", kLEOInvalidateReferences, inContext );
		theGlobal = LEOAddArrayEntryToRoot( &inContext->group->globals, globalName, &emptyString, inContext );
		LEOCleanUpValue( &emptyString, kLEOInvalidateReferences, inContext );
	}
	
	union LEOValue	tmpRefValue = {};
	
	LEOInitReferenceValue( &tmpRefValue, theGlobal, kLEOInvalidateReferences, kLEOChunkTypeINVALID, 0, 0, inContext );
	LEOCleanUpValue( inContext->stackEndPtr -1, kLEOInvalidateReferences, inContext );
	LEOInitCopy( &tmpRefValue, inContext->stackEndPtr -1, kLEOInvalidateReferences, inContext );
	LEOCleanUpValue( &tmpRefValue, kLEOInvalidateReferences, inContext );
	
	inContext->currentInstruction++;
}


void	LEOPutValueIntoValueInstruction( LEOContext* inContext )
{
	LEOPutValueIntoValue( inContext->stackEndPtr -1, inContext->stackEndPtr -2, inContext );
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -2 );
	
	inContext->currentInstruction++;
}


void	LEONumToCharInstruction( LEOContext* inContext )
{
	uint32_t utf32Char = (uint32_t) LEOGetValueAsInteger( inContext->stackEndPtr -1, NULL, inContext );
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;
	
	LEOCleanUpValue( inContext->stackEndPtr -1, kLEOInvalidateReferences, inContext );
	
	char	utf8CharStr[9] = { 0 };
	size_t	theLength = sizeof(utf8CharStr);
	UTF8BytesForUTF32Character( utf32Char, utf8CharStr, &theLength );
	LEOInitStringValue( inContext->stackEndPtr -1, utf8CharStr, theLength,
						kLEOInvalidateReferences, inContext );
	
	inContext->currentInstruction++;
}


void	LEOCharToNumInstruction( LEOContext* inContext )
{
	char	utf8CharStr[9] = { 0 };
	size_t	ioOffset = 0;
	
	const char* theStr = LEOGetValueAsString( inContext->stackEndPtr -1, utf8CharStr, sizeof(utf8CharStr), inContext );
	
	uint32_t utf32Char = UTF8StringParseUTF32CharacterAtOffset( theStr, strlen(theStr), &ioOffset );
	
	LEOCleanUpValue( inContext->stackEndPtr -1, kLEOInvalidateReferences, inContext );
	LEOInitIntegerValue( inContext->stackEndPtr -1, utf32Char, kLEOUnitNone, kLEOInvalidateReferences, inContext );
	
	inContext->currentInstruction++;
}


void	LEONumToHexInstruction( LEOContext* inContext )
{
	LEOInteger theNumber = LEOGetValueAsInteger( inContext->stackEndPtr -1, NULL, inContext );
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;
	
	LEOCleanUpValue( inContext->stackEndPtr -1, kLEOInvalidateReferences, inContext );
	
	char	hexStr[16] = { 0 };
	snprintf( hexStr, sizeof(hexStr), "%llx", theNumber );
	LEOInitStringValue( inContext->stackEndPtr -1, hexStr, strlen(hexStr),
						kLEOInvalidateReferences, inContext );
	
	inContext->currentInstruction++;
}


void	LEOHexToNumInstruction( LEOContext* inContext )
{
	char	hexStr[16] = { 0 };
	char*	endPtr = NULL;
	
	const char* theStr = LEOGetValueAsString( inContext->stackEndPtr -1, hexStr, sizeof(hexStr), inContext );
	
	LEOInteger theNumber = strtol( theStr, &endPtr, 16 );
	
	LEOCleanUpValue( inContext->stackEndPtr -1, kLEOInvalidateReferences, inContext );
	LEOInitIntegerValue( inContext->stackEndPtr -1, theNumber, kLEOUnitNone, kLEOInvalidateReferences, inContext );
	
	inContext->currentInstruction++;
}


void	LEONumToBinaryInstruction( LEOContext* inContext )
{
	LEOInteger theNumber = LEOGetValueAsInteger( inContext->stackEndPtr -1, NULL, inContext );
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;
	
	LEOCleanUpValue( inContext->stackEndPtr -1, kLEOInvalidateReferences, inContext );
	
	char	binStr[65] = { 0 };
	if( theNumber == 0 )
		binStr[0] = '0';
	else
	{
		int		numDigits = 0;
		for( int x = 63; x >= 0; x-- )
		{
			if( theNumber & (1ULL << x) )
			{
				numDigits = x;
				break;
			}
		}
		for( int x = numDigits; x >= 0; x-- )
		{
			if( theNumber & (1ULL << x) )
				binStr[numDigits -x] = '1';
			else
				binStr[numDigits -x] = '0';
		}
	}
	LEOInitStringValue( inContext->stackEndPtr -1, binStr, strlen(binStr),
						kLEOInvalidateReferences, inContext );
	
	inContext->currentInstruction++;
}


void	LEOBinaryToNumInstruction( LEOContext* inContext )
{
	char	hexStr[16] = { 0 };
	char*	endPtr = NULL;
	
	const char* theStr = LEOGetValueAsString( inContext->stackEndPtr -1, hexStr, sizeof(hexStr), inContext );
	
	LEOInteger theNumber = strtol( theStr, &endPtr, 2 );
	
	LEOCleanUpValue( inContext->stackEndPtr -1, kLEOInvalidateReferences, inContext );
	LEOInitIntegerValue( inContext->stackEndPtr -1, theNumber, kLEOUnitNone, kLEOInvalidateReferences, inContext );
	
	inContext->currentInstruction++;
}


/*
	(IS_WITHIN_INSTR)
*/


void	LEOIsWithinInstruction( LEOContext* inContext )
{
	LEOInteger		l = 0, t = 0, r = 0, b = 0, x = 0, y = 0;
	LEOGetValueAsRect( inContext->stackEndPtr -1, &l, &t, &r, &b, inContext );
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;
	LEOGetValueAsPoint( inContext->stackEndPtr -2, &x, &y, inContext );
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;
	
	if( l > r ) { LEOInteger n = r; r = l; l = n; }
	if( t > b ) { LEOInteger	n = b; b = t; t = n; }
	
	bool			isWithin = x <= r && x >= l && y <= b && y >= t;
	
	LEOCleanUpValue( inContext->stackEndPtr -1, kLEOInvalidateReferences, inContext );
	LEOInitBooleanValue( inContext->stackEndPtr -1, isWithin, kLEOInvalidateReferences, inContext );
	
	inContext->currentInstruction++;
}


/*
	INTERSECTS_INSTR
*/

void	LEOIntersectsInstruction( LEOContext* inContext )
{
	LEOInteger		l = 0, t = 0, r = 0, b = 0, l2 = 0, t2 = 0, r2 = 0, b2 = 0;
	LEOGetValueAsRect( inContext->stackEndPtr -2, &l, &t, &r, &b, inContext );
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;
	LEOGetValueAsRect( inContext->stackEndPtr -1, &l2, &t2, &r2, &b2, inContext );
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;
	
	if( l > r ) { LEOInteger n = r; r = l; l = n; }
	if( t > b ) { LEOInteger	n = b; b = t; t = n; }
	if( l2 > r2 ) { LEOInteger	n = r2; r2 = l2; l2 = n; }
	if( t2 > b2 ) { LEOInteger	n = b2; b2 = t2; t2 = n; }
	
	bool			isWithin = (l2 <= r) && (r2 >= l) && (t2 <= b) && (b2 >= t);
	
	LEOCleanUpValue( inContext->stackEndPtr -1, kLEOInvalidateReferences, inContext );
	LEOInitBooleanValue( inContext->stackEndPtr -1, isWithin, kLEOInvalidateReferences, inContext );
	
	inContext->currentInstruction++;
}


/*
	Set param1 to 1 to have this behave as if there was an IS_NOT_UNSET instruction.
	
	(IS_UNSET_INSTR)
*/

void	LEOIsUnsetInstruction( LEOContext* inContext )
{
	bool	isUnset = LEOGetValueIsUnset( inContext->stackEndPtr -1, inContext );
	if( inContext->currentInstruction->param1 == 1 )
		isUnset = !isUnset;
	LEOCleanUpValue( inContext->stackEndPtr -1, kLEOInvalidateReferences, inContext);
	LEOInitBooleanValue( inContext->stackEndPtr -1, isUnset, kLEOInvalidateReferences, inContext );
	
	inContext->currentInstruction++;
}


/*
	Check what type the value on the back of the stack is, then pop it and push the result.
	param1 specifies what type you want it checked against:
	
	1	-	This is "is not a <type>", not "is a <type>", i.e. this will return the reverse of the type check. (add this to the other number)
	2	-	number
	4	-	integer
	
	(IS_TYPE_INSTR)
*/

void	LEOIsTypeInstruction( LEOContext* inContext )
{
	bool	isThisType = false;
	if( (inContext->currentInstruction->param1 & ~1) == 2 )
		isThisType = LEOCanGetAsNumber(inContext->stackEndPtr -1, inContext );
	else if( (inContext->currentInstruction->param1 & ~1) == 4 )
		isThisType = LEOCanGetAsInteger(inContext->stackEndPtr -1, inContext );
	if( inContext->currentInstruction->param1 & 1 )
		isThisType = !isThisType;
	LEOCleanUpValue( inContext->stackEndPtr -1, kLEOInvalidateReferences, inContext);
	LEOInitBooleanValue( inContext->stackEndPtr -1, isThisType, kLEOInvalidateReferences, inContext );
	
	inContext->currentInstruction++;
}


#pragma mark -
#pragma mark Instruction table

struct LEOInstructionEntry*	gInstructions = NULL;
size_t						gNumInstructions = 0;


LEOINSTR_START(Default,LEO_NUMBER_OF_INSTRUCTIONS)
LEOINSTR(LEOInvalidInstruction)
LEOINSTR(LEOExitToTopInstruction)
LEOINSTR(LEONoOpInstruction)
LEOINSTR(LEOPushStringFromTableInstruction)
LEOINSTR(LEOPushUnsetValueInstruction)
LEOINSTR(LEOPopValueInstruction)
LEOINSTR(LEOPushBooleanInstruction)
LEOINSTR(LEOAssignStringFromTableInstruction)
LEOINSTR(LEOJumpRelativeInstruction)
LEOINSTR(LEOJumpRelativeIfTrueInstruction)
LEOINSTR(LEOJumpRelativeIfFalseInstruction)
LEOINSTR(LEOJumpRelativeIfGreaterThanZeroInstruction)
LEOINSTR(LEOJumpRelativeIfLessThanZeroInstruction)
LEOINSTR(LEOJumpRelativeIfGreaterSameThanZeroInstruction)
LEOINSTR(LEOJumpRelativeIfLessSameThanZeroInstruction)
LEOINSTR(LEOPushNumberInstruction)
LEOINSTR(LEOPushIntegerInstruction)
LEOINSTR(LEOPushIntegerStartInstruction)
LEOINSTR(LEOAssignIntegerEndInstruction)
LEOINSTR(LEOAddNumberInstruction)
LEOINSTR(LEOAddIntegerInstruction)
LEOINSTR(LEOCallHandlerInstruction)
LEOINSTR(LEOCleanUpHandlerStackInstruction)
LEOINSTR(LEOReturnFromHandlerInstruction)
LEOINSTR(LEOPushReferenceInstruction)
LEOINSTR(LEOPushChunkReferenceInstruction)
LEOINSTR(LEOParameterInstruction)
LEOINSTR(LEOParameterCountInstruction)
LEOINSTR(LEOSetReturnValueInstruction)
LEOINSTR(LEOParameterKeepRefsInstruction)
LEOINSTR(LEOPushParametersInstruction)
LEOINSTR(LEOConcatenateValuesInstruction)
LEOINSTR(LEOAndOperatorInstruction)
LEOINSTR(LEOOrOperatorInstruction)
LEOINSTR(LEOConcatenateValuesWithSpaceInstruction)
LEOINSTR(LEONegateBooleanInstruction)
LEOINSTR(LEOSubtractCommandInstruction)
LEOINSTR(LEOAddCommandInstruction)
LEOINSTR(LEOMultiplyCommandInstruction)
LEOINSTR(LEODivideCommandInstruction)
LEOINSTR(LEOSubtractOperatorInstruction)
LEOINSTR(LEOAddOperatorInstruction)
LEOINSTR(LEOMultiplyOperatorInstruction)
LEOINSTR(LEODivideOperatorInstruction)
LEOINSTR(LEOGreaterThanOperatorInstruction)
LEOINSTR(LEOLessThanOperatorInstruction)
LEOINSTR(LEOGreaterThanEqualOperatorInstruction)
LEOINSTR(LEOLessThanEqualOperatorInstruction)
LEOINSTR(LEONegateNumberInstruction)
LEOINSTR(LEOModuloOperatorInstruction)
LEOINSTR(LEOPowerOperatorInstruction)
LEOINSTR(LEOEqualOperatorInstruction)
LEOINSTR(LEONotEqualOperatorInstruction)
LEOINSTR(LEOLineMarkerInstruction)
LEOINSTR(LEOAssignChunkArrayInstruction)
LEOINSTR(LEOGetArrayItemInstruction)
LEOINSTR(LEOCountChunksInstruction)
LEOINSTR(LEOGetArrayItemCountInstruction)
LEOINSTR(LEOPopSimpleValueInstruction)
LEOINSTR(LEOSetStringInstruction)
LEOINSTR(LEOPushChunkInstruction)
LEOINSTR(LEOPushItemDelimiterInstruction)
LEOINSTR(LEOSetItemDelimiterInstruction)
LEOINSTR(LEOPushGlobalReferenceInstruction)
LEOINSTR(LEOPutValueIntoValueInstruction)
LEOINSTR(LEOPushStringVariantFromTableInstruction)
LEOINSTR(LEONumToCharInstruction)
LEOINSTR(LEOCharToNumInstruction)
LEOINSTR(LEONumToHexInstruction)
LEOINSTR(LEOHexToNumInstruction)
LEOINSTR(LEONumToBinaryInstruction)
LEOINSTR(LEOBinaryToNumInstruction)
LEOINSTR(LEOSetChunkPropertyInstruction)
LEOINSTR(LEOPushChunkPropertyInstruction)
LEOINSTR(LEOPushArrayConstantInstruction)
LEOINSTR(LEOParseErrorInstruction)
LEOINSTR(LEOIsWithinInstruction)
LEOINSTR(LEOIntersectsInstruction)
LEOINSTR(LEOIsUnsetInstruction)
LEOINSTR_LAST(LEOIsTypeInstruction)



void	LEOInitInstructionArray()
{
	if( gInstructions == NULL )
	{
		gInstructions = gDefaultInstructions;
		gNumInstructions = LEO_NUMBER_OF_INSTRUCTIONS;
	}
}


void	LEOAddInstructionsToInstructionArray( struct LEOInstructionEntry *inInstructionArray, size_t inNumInstructions, size_t *outFirstNewInstruction )
{
	LEOInitInstructionArray();
	
	struct LEOInstructionEntry*	instrArray = NULL;
	
	if( gNumInstructions == LEO_NUMBER_OF_INSTRUCTIONS )	// Static buffer:
	{
		instrArray = calloc( gNumInstructions +inNumInstructions, sizeof(struct LEOInstructionEntry) );
		memmove( instrArray, gInstructions, gNumInstructions *sizeof(struct LEOInstructionEntry) );
	}
	else	// Dynamic buffer, already added something before:
	{
		instrArray = realloc( gInstructions, (gNumInstructions +inNumInstructions) * sizeof(struct LEOInstructionEntry) );
	}
	
	if( instrArray )
	{
		gInstructions = instrArray;
		
		memmove( gInstructions +gNumInstructions, inInstructionArray, inNumInstructions * sizeof(struct LEOInstructionEntry) );
		
		*outFirstNewInstruction = gNumInstructions;
		gNumInstructions += inNumInstructions;
	}
}
