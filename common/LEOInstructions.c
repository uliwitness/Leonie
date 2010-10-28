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
	inContext->currentInstruction += *(int32_t*)&inContext->currentInstruction->param2;
}


void	LEOJumpRelativeIfTrueInstruction( LEOContext* inContext )
{
	bool			popOffStack = (inContext->currentInstruction->param1 == 0xffff);
	union LEOValue*	theValue = popOffStack ? (inContext->stackEndPtr -1) : (inContext->stackBasePtr +inContext->currentInstruction->param1);
	if( LEOGetValueAsBoolean( theValue, inContext ) )
		inContext->currentInstruction += *(int32_t*)&inContext->currentInstruction->param2;
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
		inContext->currentInstruction += *(int32_t*)&inContext->currentInstruction->param2;
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
		inContext->currentInstruction += *(int32_t*)&inContext->currentInstruction->param2;
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
		inContext->currentInstruction += *(int32_t*)&inContext->currentInstruction->param2;
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
		inContext->currentInstruction += *(int32_t*)&inContext->currentInstruction->param2;
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
		inContext->currentInstruction += *(int32_t*)&inContext->currentInstruction->param2;
	else
		inContext->currentInstruction++;
	if( popOffStack )
		LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );
}


void	LEOPushNumberInstruction( LEOContext* inContext )
{
	LEOInitNumberValue( (LEOValuePtr) inContext->stackEndPtr, inContext->currentInstruction->param2,
						kLEOInvalidateReferences, inContext );
	inContext->stackEndPtr++;

	inContext->currentInstruction++;
}


void	LEOAddNumberInstruction( LEOContext* inContext )
{
	bool			onStack = (inContext->currentInstruction->param1 == 0xffff);
	union LEOValue*	theValue = onStack ? (inContext->stackEndPtr -1) : (inContext->stackBasePtr +inContext->currentInstruction->param1);
	double	theNum = LEOGetValueAsNumber( theValue, inContext );
	
	theNum += *(int32_t*)&inContext->currentInstruction->param2;
	LEOSetValueAsNumber( theValue, theNum, inContext );
	inContext->currentInstruction++;
}


void	LEOCallHandlerInstruction( LEOContext* inContext )
{
	LEOHandlerID	handlerName = inContext->currentInstruction->param2;
	LEOScript*		currScript = LEOContextPeekCurrentScript( inContext );
	LEOHandler*		foundHandler = NULL;
	if( currScript )
	{
		foundHandler = LEOScriptFindCommandHandlerWithID( currScript, handlerName );
		if( foundHandler )
		{
			LEOContextPushHandlerScriptAndReturnAddress( inContext, foundHandler, currScript, inContext->currentInstruction +1 );
			inContext->currentInstruction = foundHandler->instructions;
		}
	}
	
	if( !foundHandler )
	{
		snprintf( inContext->errMsg, sizeof(inContext->errMsg), "Couldn't find handler \"%s\".", LEOContextGroupHandlerNameForHandlerID( inContext->group, handlerName ) );
		inContext->keepRunning = false;
		inContext->currentInstruction++;
	}
}


void	LEOReturnFromHandlerInstruction( LEOContext* inContext )
{
	inContext->currentInstruction = LEOContextPeekReturnAddress( inContext );
	LEOContextPopHandlerScriptAndReturnAddress( inContext );
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
	LEOAddNumberInstruction,
	LEOCallHandlerInstruction,
	LEOReturnFromHandlerInstruction
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
	"AddNumber",
	"CallHandler",
	"ReturnFromHandler"
};

size_t		gNumInstructions = sizeof(gInstructions) / sizeof(LEOInstructionFuncPtr);
