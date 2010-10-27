/*
 *  LEOInterpreter.c
 *  Leonie
 *
 *  Created by Uli Kusterer on 17.09.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include "LEOInterpreter.h"
#include "LEOInstructions.h"
#include "LEOContextGroup.h"
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


void	LEODoNothingPreInstructionProc( LEOContext* inContext )
{
	
}


void	LEOInitContext( LEOContext* theContext, struct LEOContextGroup* inGroup )
{
	memset( theContext, 0, sizeof(LEOContext) );
	theContext->preInstructionProc = LEODoNothingPreInstructionProc;
	theContext->itemDelimiter = ',';
	theContext->group = LEOContextGroupRetain( inGroup );
}


void	LEOCleanUpContext( LEOContext* theContext )
{
	LEOCleanUpStackToPtr( theContext, theContext->stack );
	LEOContextGroupRelease( theContext->group );
	theContext->group = NULL;
}


void	LEOCleanUpStackToPtr( LEOContext* theContext, union LEOValue* lastItemToDelete )
{
	while( theContext->stackEndPtr > lastItemToDelete )
	{
		theContext->stackEndPtr--;
		LEOCleanUpValue( theContext->stackEndPtr, kLEOInvalidateReferences, theContext );
	}
}


void	LEORunInContext( LEOInstruction instructions[], LEOContext *inContext )
{
	LEOPrepareContextForRunning( instructions, inContext );
	
	while( LEOContinueRunningContext( inContext ) )
		;
}


void	LEOPrepareContextForRunning( LEOInstruction instructions[], LEOContext *inContext )
{
	inContext->keepRunning = true;
	inContext->currentInstruction = instructions;
	inContext->stackBasePtr = inContext->stack;
	inContext->stackEndPtr = inContext->stack;
	inContext->errMsg[0] = 0;
	
	// +++ Should we call LEOCleanUpStackToPtr here? Would be necessary for reusing a context.
}


bool	LEOContinueRunningContext( LEOContext *inContext )
{
	inContext->errMsg[0] = 0;
	
	inContext->preInstructionProc(inContext);
	if( inContext->currentInstruction == NULL || !inContext->keepRunning )	// Did pre-instruction-proc request abort?
		return false;
	
	LEOInstructionID	currID = inContext->currentInstruction->instructionID;
	if( currID >= gNumInstructions )
		currID = 0;	// First instruction is the special "unimplemented" instruction.
	gInstructions[currID](inContext);
	
	return( inContext->currentInstruction != NULL && inContext->keepRunning );
}


void	LEODebugPrintInstr( LEOInstruction* instruction )
{
	if( !instruction )
	{
		printf("(null)\n");
		return;
	}
	
	LEOInstructionID	currID = instruction->instructionID;
	if( currID >= gNumInstructions )
		printf("UNKNOWN_%d",currID);
	else
		printf("%s",gInstructionNames[currID]);
	printf("( %u, %d );\n", instruction->param1, instruction->param2 );
}


void	LEODebugPrintInstructions( LEOInstruction instructions[], size_t numInstructions )
{
	printf( "%u INSTRUCTIONS:\n", (unsigned int)numInstructions );
	for( int x = 0; x < numInstructions; x++ )
	{
		printf( "    " );
		LEODebugPrintInstr( instructions +x );
	}
}


void	LEODebugPrintContext( LEOContext* ctx )
{
	printf( "CONTEXT:\n" );
	printf( "    keepRunning: %s\n", ctx->keepRunning ? "true" : "FALSE" );
	printf( "    errMsg: %s\n", ctx->errMsg );
	printf( "    currentInstruction: " ); LEODebugPrintInstr( ctx->currentInstruction );
	printf( "    -----------------------------\n" );
	
	if( ctx->stackEndPtr != NULL )
	{
		union LEOValue*		currValue = ctx->stack;
		while( currValue != ctx->stackEndPtr )
		{
			if( currValue == ctx->stackBasePtr )
				printf("--> ");
			else
				printf("    ");
			
			char		str[1024] = { 0 };
			LEOGetValueAsString( currValue, str, sizeof(str), ctx );
			printf( "\"%s\"\n", str );
			
			currValue ++;
		}
	}
}




