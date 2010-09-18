/*
 *  LEOInterpreter.c
 *  Leonie
 *
 *  Created by Uli Kusterer on 17.09.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include "LEOInterpreter.h"
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include "LEOInstructions.h"


void	LEODoNothingPreInstructionProc( LEOContext* inContext )
{
	
}


void	LEOInitContext( LEOContext* theContext )
{
	memset( theContext, 0, sizeof(LEOContext) );
	theContext->preInstructionProc = LEODoNothingPreInstructionProc;
}


void	LEOCleanUpContext( LEOContext* theContext )
{
	LEOCleanUpStackToPtr( theContext, theContext->stack );
}


void	LEOCleanUpStackToPtr( LEOContext* theContext, union LEOValue* lastItemToDelete )
{
	while( theContext->stackEndPtr > lastItemToDelete )
	{
		theContext->stackEndPtr--;
		LEOCleanUpValue( theContext->stackEndPtr );
	}
}


void	LEORunInContext( LEOInstruction instructions[], LEOContext *inContext )
{
	inContext->keepRunning = true;
	inContext->currentInstruction = instructions;
	inContext->stackBasePtr = inContext->stack;
	inContext->stackEndPtr = inContext->stack;
	inContext->errMsg[0] = 0;
	
	while( inContext->currentInstruction != NULL && inContext->keepRunning )	// Set keepRunning to FALSE to do the equivalent of exit().
	{
		inContext->errMsg[0] = 0;
		
		inContext->preInstructionProc(inContext);
		if( inContext->currentInstruction == NULL || !inContext->keepRunning )	// Did pre-instruction-proc request abort?
			break;
		
		LEOInstructionID	currID = inContext->currentInstruction->instructionID;
		if( currID >= gNumInstructions )
			currID = 0;	// First instruction is the special "unimplemented" instruction.
		gInstructions[currID](inContext);
	}
}
