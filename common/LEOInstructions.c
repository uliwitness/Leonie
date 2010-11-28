/*
 *  LEOInstructions.c
 *  Leonie
 *
 *  Created by Uli Kusterer on 17.09.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include "LEOInstructions.h"
#include "LEOValue.h"
#include "LEOInterpreter.h"
#include "LEOScript.h"
#include "LEOContextGroup.h"
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>


#pragma mark -
#pragma mark Instruction Functions

void	LEOInvalidInstruction( LEOContext* inContext )
{
	snprintf( inContext->errMsg, sizeof(inContext->errMsg), "Unknown instruction %u", inContext->currentInstruction->instructionID );
	
	inContext->keepRunning = false;	// Causes interpreter loop to exit.
}


void	LEOExitToTopInstruction( LEOContext* inContext )
{
	inContext->keepRunning = false;	// Causes interpreter loop to exit.
}


void	LEONoOpInstruction( LEOContext* inContext )
{
	// Do nothing.

	inContext->currentInstruction++;
}


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


void	LEOPopInstruction( LEOContext* inContext )
{
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );
	
	inContext->currentInstruction++;
}


void	LEOPushBooleanInstruction( LEOContext* inContext )
{
	LEOInitBooleanValue( (LEOValuePtr) inContext->stackEndPtr, inContext->currentInstruction->param2 == 1,
							kLEOInvalidateReferences, inContext );
	inContext->stackEndPtr++;

	inContext->currentInstruction++;
}


void	LEOAssignStringFromTableInstruction( LEOContext* inContext )
{
	bool			onStack = (inContext->currentInstruction->param1 == 0xffff);
	union LEOValue*	theValue = onStack ? (inContext->stackEndPtr -1) : (inContext->stackBasePtr +inContext->currentInstruction->param1);
	const char*		theString = "";
	LEOScript*		script = LEOContextPeekCurrentScript( inContext );
	if( inContext->currentInstruction->param2 < script->numStrings )
		theString = script->strings[inContext->currentInstruction->param2];
	
	LEOSetValueAsString( theValue, theString, inContext );
	
	inContext->currentInstruction++;
}


void	LEOJumpRelativeInstruction( LEOContext* inContext )
{
	inContext->currentInstruction += LEOCastUInt32ToInt32( inContext->currentInstruction->param2 );
}


void	LEOJumpRelativeIfTrueInstruction( LEOContext* inContext )
{
	bool			popOffStack = (inContext->currentInstruction->param1 == 0xffff);
	union LEOValue*	theValue = popOffStack ? (inContext->stackEndPtr -1) : (inContext->stackBasePtr +inContext->currentInstruction->param1);
	if( LEOGetValueAsBoolean( theValue, inContext ) )
		inContext->currentInstruction += LEOCastUInt32ToInt32( inContext->currentInstruction->param2 );
	else
		inContext->currentInstruction++;
	if( popOffStack )
		LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );
}


void	LEOJumpRelativeIfFalseInstruction( LEOContext* inContext )
{
	bool			popOffStack = (inContext->currentInstruction->param1 == 0xffff);
	union LEOValue*	theValue = popOffStack ? (inContext->stackEndPtr -1) : (inContext->stackBasePtr +inContext->currentInstruction->param1);
	if( !LEOGetValueAsBoolean( theValue, inContext ) )
		inContext->currentInstruction += LEOCastUInt32ToInt32( inContext->currentInstruction->param2 );
	else
		inContext->currentInstruction++;
	if( popOffStack )
		LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );
}


void	LEOJumpRelativeIfGreaterThanZeroInstruction( LEOContext* inContext )
{
	bool			popOffStack = (inContext->currentInstruction->param1 == 0xffff);
	union LEOValue*	theValue = popOffStack ? (inContext->stackEndPtr -1) : (inContext->stackBasePtr +inContext->currentInstruction->param1);
	if( LEOGetValueAsNumber( theValue, inContext ) > 0 )
		inContext->currentInstruction += LEOCastUInt32ToInt32( inContext->currentInstruction->param2 );
	else
		inContext->currentInstruction++;
	if( popOffStack )
		LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );
}


void	LEOJumpRelativeIfLessThanZeroInstruction( LEOContext* inContext )
{
	bool			popOffStack = (inContext->currentInstruction->param1 == 0xffff);
	union LEOValue*	theValue = popOffStack ? (inContext->stackEndPtr -1) : (inContext->stackBasePtr +inContext->currentInstruction->param1);
	if( LEOGetValueAsNumber( theValue, inContext ) < 0 )
		inContext->currentInstruction += LEOCastUInt32ToInt32( inContext->currentInstruction->param2 );
	else
		inContext->currentInstruction++;
	if( popOffStack )
		LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );
}


void	LEOJumpRelativeIfGreaterSameThanZeroInstruction( LEOContext* inContext )
{
	bool			popOffStack = (inContext->currentInstruction->param1 == 0xffff);
	union LEOValue*	theValue = popOffStack ? (inContext->stackEndPtr -1) : (inContext->stackBasePtr +inContext->currentInstruction->param1);
	if( LEOGetValueAsNumber( theValue, inContext ) >= 0 )
		inContext->currentInstruction += LEOCastUInt32ToInt32( inContext->currentInstruction->param2 );
	else
		inContext->currentInstruction++;
	if( popOffStack )
		LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );
}


void	LEOJumpRelativeIfLessSameThanZeroInstruction( LEOContext* inContext )
{
	bool			popOffStack = (inContext->currentInstruction->param1 == 0xffff);
	union LEOValue*	theValue = popOffStack ? (inContext->stackEndPtr -1) : (inContext->stackBasePtr +inContext->currentInstruction->param1);
	if( LEOGetValueAsNumber( theValue, inContext ) <= 0 )
		inContext->currentInstruction += LEOCastUInt32ToInt32( inContext->currentInstruction->param2 );
	else
		inContext->currentInstruction++;
	if( popOffStack )
		LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );
}


void	LEOPushNumberInstruction( LEOContext* inContext )
{
	LEOInitNumberValue( (LEOValuePtr) inContext->stackEndPtr, LEOCastUInt32ToLEONumber(inContext->currentInstruction->param2),
						kLEOInvalidateReferences, inContext );
	inContext->stackEndPtr++;

	inContext->currentInstruction++;
}


void	LEOPushIntegerInstruction( LEOContext* inContext )
{
	LEOInitIntegerValue( (LEOValuePtr) inContext->stackEndPtr, inContext->currentInstruction->param2,
							kLEOInvalidateReferences, inContext );
	inContext->stackEndPtr++;

	inContext->currentInstruction++;
}


void	LEOAddNumberInstruction( LEOContext* inContext )
{
	bool			onStack = (inContext->currentInstruction->param1 == 0xffff);
	union LEOValue*	theValue = onStack ? (inContext->stackEndPtr -1) : (inContext->stackBasePtr +inContext->currentInstruction->param1);
	LEONumber		theNum = LEOGetValueAsInteger( theValue, inContext );
	
	theNum += LEOCastUInt32ToLEONumber( inContext->currentInstruction->param2 );
	LEOSetValueAsNumber( theValue, theNum, inContext );
	inContext->currentInstruction++;
}


void	LEOAddIntegerInstruction( LEOContext* inContext )
{
	bool			onStack = (inContext->currentInstruction->param1 == 0xffff);
	union LEOValue*	theValue = onStack ? (inContext->stackEndPtr -1) : (inContext->stackBasePtr +inContext->currentInstruction->param1);
	LEOInteger		theNum = LEOGetValueAsNumber( theValue, inContext );
	
	theNum += LEOCastUInt32ToInt32( inContext->currentInstruction->param2 );
	LEOSetValueAsNumber( theValue, theNum, inContext );
	inContext->currentInstruction++;
}


void	LEOCallHandlerInstruction( LEOContext* inContext )
{
	//LEODebugPrintContext( inContext );
	
	LEOHandlerID	handlerName = inContext->currentInstruction->param2;
	LEOScript*		currScript = LEOContextPeekCurrentScript( inContext );
	LEOHandler*		foundHandler = NULL;
	if( currScript )
	{
		foundHandler = LEOScriptFindCommandHandlerWithID( currScript, handlerName );
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


void	LEOReturnFromHandlerInstruction( LEOContext* inContext )
{
	//LEODebugPrintContext( inContext );

	inContext->currentInstruction = LEOContextPeekReturnAddress( inContext );
	inContext->stackBasePtr = LEOContextPeekBasePtr( inContext );
	LEOContextPopHandlerScriptReturnAddressAndBasePtr( inContext );
	
	//LEODebugPrintContext( inContext );
}


void	LEOPushReferenceInstruction( LEOContext* inContext )
{
	bool			onStack = (inContext->currentInstruction->param1 == 0xffff);
	union LEOValue*	theValue = onStack ? (inContext->stackEndPtr -1) : (inContext->stackBasePtr +inContext->currentInstruction->param1);
	union LEOValue	tmpRefValue = { 0 };
	LEOValuePtr		refValueOnStack = NULL;
	
	LEOInitReferenceValue( theValue, &tmpRefValue, kLEOInvalidateReferences, kLEOChunkTypeINVALID, 0, 0, inContext );
	refValueOnStack = LEOPushValueOnStack( inContext, &tmpRefValue );
	
	inContext->currentInstruction++;
}


void	LEOPushChunkReferenceInstruction( LEOContext* inContext )
{
	LEOValuePtr		chunkTarget = (inContext->stackBasePtr +inContext->currentInstruction->param1);
	LEOValuePtr		chunkEnd = inContext->stackEndPtr;
	LEOValuePtr		chunkStart = inContext->stackEndPtr -1;
	union LEOValue	tmpRefValue = { 0 };
	LEOValuePtr		refValueOnStack = NULL;
	
	size_t	chunkStartOffs = LEOGetValueAsNumber(chunkStart,inContext);
	if( !inContext->keepRunning )
		return;
	
	size_t	chunkEndOffs = LEOGetValueAsNumber(chunkEnd,inContext);
	if( !inContext->keepRunning )
		return;
	
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -2 );
	
	LEOInitReferenceValue( chunkTarget, &tmpRefValue, kLEOInvalidateReferences, inContext->currentInstruction->param2, chunkStartOffs, chunkEndOffs, inContext );
	refValueOnStack = LEOPushValueOnStack( inContext, &tmpRefValue );
	
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
	LEOPushChunkReferenceInstruction
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
	"PushChunkReference"
};

size_t		gNumInstructions = sizeof(gInstructions) / sizeof(LEOInstructionFuncPtr);
