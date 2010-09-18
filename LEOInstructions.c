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
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>


#pragma mark -
#pragma mark Instruction Functions

void	LEOInvalidInstruction( LEOContext* inContext )
{
	snprintf( inContext->errMsg, sizeof(inContext->errMsg), "\n*** TERMINATING DUE TO UNKNOWN INSTRUCTION %u ***\n\n", inContext->currentInstruction->instructionID );
	
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
	// +++ Look up the string at index inContext->currentInstruction->param2 in some table somewhere.
	
	LEOInitStringConstantValue( (LEOValuePtr) inContext->stackEndPtr, "Hi world.\n" );
	inContext->stackEndPtr++;
	
	inContext->currentInstruction++;
}


void	LEOPrintInstruction( LEOContext* inContext )
{
	char		buf[1024] = { 0 };
	
	union LEOValue*	theValue = inContext->stackEndPtr -1;
	LEOGetValueAsString( theValue, buf, sizeof(buf), inContext );
	printf( "%s", buf );
	
	inContext->currentInstruction++;
}


void	LEOPopInstruction( LEOContext* inContext )
{
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );
	
	inContext->currentInstruction++;
}


void	LEOPushBooleanInstruction( LEOContext* inContext )
{
	LEOInitBooleanValue( (LEOValuePtr) inContext->stackEndPtr, inContext->currentInstruction->param2 == 1 );
	inContext->stackEndPtr++;

	inContext->currentInstruction++;
}


void	LEOAssignStringFromTableInstruction( LEOContext* inContext )
{
	union LEOValue*	theValue = inContext->stackEndPtr -1;
	const char*		theStr = "Top 'o the mornin' to ya!";
	
	LEOSetValueAsString( theValue, theStr, inContext );
	
	inContext->currentInstruction++;
}


void	LEOJumpRelativeInstruction( LEOContext* inContext )
{
	inContext->currentInstruction += inContext->currentInstruction->param2;
}


void	LEOJumpRelativeIfTrueInstruction( LEOContext* inContext )
{
	union LEOValue*	theValue = inContext->stackEndPtr -1;
	if( LEOGetValueAsBoolean( theValue, inContext ) )
		inContext->currentInstruction += inContext->currentInstruction->param2;
	else
		inContext->currentInstruction++;
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );
}


void	LEOJumpRelativeIfFalseInstruction( LEOContext* inContext )
{
	union LEOValue*	theValue = inContext->stackEndPtr -1;
	if( !LEOGetValueAsBoolean( theValue, inContext ) )
		inContext->currentInstruction += inContext->currentInstruction->param2;
	else
		inContext->currentInstruction++;
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );
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
	LEOJumpRelativeIfFalseInstruction
};

size_t		gNumInstructions = sizeof(gInstructions) / sizeof(LEOInstructionFuncPtr);
