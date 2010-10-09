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
#include <stdio.h>


void	LEODoNothingPreInstructionProc( LEOContext* inContext )
{
	
}


void	LEOInitContext( LEOContext* theContext )
{
	memset( theContext, 0, sizeof(LEOContext) );
	theContext->preInstructionProc = LEODoNothingPreInstructionProc;
	theContext->itemDelimiter = ',';
}


void	LEOCleanUpContext( LEOContext* theContext )
{
	LEOCleanUpStackToPtr( theContext, theContext->stack );
	free( theContext->references );
	theContext->references = NULL;
}


void	LEOCleanUpStackToPtr( LEOContext* theContext, union LEOValue* lastItemToDelete )
{
	while( theContext->stackEndPtr > lastItemToDelete )
	{
		theContext->stackEndPtr--;
		LEOCleanUpValue( theContext->stackEndPtr, theContext );
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


#define LEOReferencesTableChunkSize			16


LEOObjectID	LEOCreateNewObjectIDForValue( LEOValuePtr theValue, struct LEOContext* inContext )
{
	LEOObjectID		newObjectID = LEOObjectIDINVALID;
	if( inContext->references == NULL )
	{
		inContext->numReferences = LEOReferencesTableChunkSize;
		inContext->references = calloc( inContext->numReferences, sizeof(struct LEOObject) );
		
		newObjectID = 0;	// Can start with first item right away.
	}
	else
	{
		// +++ Optimize: remember the last one we cleared or returned or so and start scanning there.
		
		for( size_t x = 0; x < inContext->numReferences; x++ )
		{
			if( inContext->references[x].value == NULL )	// Unused slot!
				newObjectID = x;
		}
		
		if( newObjectID == LEOObjectIDINVALID )
		{
			// No free slots left?
			size_t		oldNumReferences = inContext->numReferences;
			inContext->numReferences += LEOReferencesTableChunkSize;
			inContext->references = realloc( inContext->references, sizeof(struct LEOObject) * inContext->numReferences );
			memset( inContext->references +(oldNumReferences * sizeof(struct LEOObject)), 0, LEOReferencesTableChunkSize * sizeof(struct LEOObject) );
			
			newObjectID = oldNumReferences;	// Same as index of first new item.
		}
	}
	
	theValue->refObjectID = newObjectID;
	inContext->references[newObjectID].value = theValue;
	
	return newObjectID;
}


void	LEORecycleObjectID( LEOObjectID inObjectID, struct LEOContext* inContext )
{
	inContext->references[inObjectID].value = NULL;
	inContext->references[inObjectID].seed += 1;	// Make sure that if this is reused, whoever still references it knows it's gone.
}


LEOValuePtr	LEOGetValueForObjectIDAndSeed( LEOObjectID inObjectID, LEOObjectSeed inObjectSeed, struct LEOContext* inContext )
{
	if( inContext->references[inObjectID].seed != inObjectSeed )
		return NULL;
	
	return inContext->references[inObjectID].value;
}






