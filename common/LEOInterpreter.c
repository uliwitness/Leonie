/*
 *  LEOInterpreter.c
 *  Leonie
 *
 *  Created by Uli Kusterer on 17.09.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

// -----------------------------------------------------------------------------
//	Headers:
// -----------------------------------------------------------------------------

#include "LEOInterpreter.h"
#include "LEOInstructions.h"
#include "LEOContextGroup.h"
#include "LEOScript.h"
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>



// -----------------------------------------------------------------------------
//	Constants:
// -----------------------------------------------------------------------------

#define LEOCallStackEntriesChunkSize			16


void	LEODoNothingPreInstructionProc( LEOContext* inContext );




char*	*			gFileNamesTable = NULL;
size_t				gFileNamesTableSize = 0;
LEOInstructionID	gInstructionIDToDebugPrintBefore = INVALID_INSTR;
LEOInstructionID	gInstructionIDToDebugPrintAfter = INVALID_INSTR;
void				(*gCheckForResumeProc)() = NULL;


void	LEOSetCheckForResumeProc( void (*checkForResumeProc)() )
{
	gCheckForResumeProc = checkForResumeProc;
}


void	LEOSetInstructionIDToDebugPrintBefore( LEOInstructionID inID )
{
	gInstructionIDToDebugPrintBefore = inID;
}


void	LEOSetInstructionIDToDebugPrintAfter( LEOInstructionID inID )
{
	gInstructionIDToDebugPrintAfter = inID;
}


uint16_t		LEOFileIDForFileName( const char* inFileName )
{
	for( size_t x = 0; x < gFileNamesTableSize; x++ )
	{
		if( strcmp(inFileName, gFileNamesTable[x]) == 0 )
			return x;
	}
	
	// No match found? Add a new entry and return its index:
	size_t strBufferSize = strlen(inFileName) +1;
	
	if( gFileNamesTable == NULL )
		gFileNamesTable = (char**) malloc( sizeof(char*) );
	else
		gFileNamesTable = (char**) realloc( gFileNamesTable, sizeof(char*) * (gFileNamesTableSize +1) );
		
	gFileNamesTable[gFileNamesTableSize] = (char*) malloc( strBufferSize );
	memmove( gFileNamesTable[gFileNamesTableSize], inFileName, strBufferSize );
	
	return gFileNamesTableSize ++;
}


const char*	LEOFileNameForFileID( uint16_t inFileID )
{
	return gFileNamesTable[inFileID];
}



void	LEODoNothingPreInstructionProc( LEOContext* inContext )
{
#pragma unused(inContext)
}


LEOContext*	LEOContextCreate( struct LEOContextGroup* inGroup, void* inUserData, LEOUserDataCleanUpFuncPtr inCleanUpFunc )
{
	LEOContext*	theContext = calloc( 1, sizeof(LEOContext) );
	theContext->referenceCount = 1;
	theContext->preInstructionProc = LEODoNothingPreInstructionProc;
	theContext->promptProc = LEODoNothingPreInstructionProc;
	theContext->callNonexistentHandlerProc = NULL;
	theContext->itemDelimiter = ',';
	theContext->group = LEOContextGroupRetain( inGroup );
	theContext->flags = kLEOContextKeepRunning;
	theContext->userData = inUserData;
	theContext->cleanUpUserData = inCleanUpFunc;
	return theContext;
}


LEOContext*	LEOContextRetain( LEOContext* inContext )
{
	inContext->referenceCount++;
	
	return inContext;
}


void	LEOContextRelease( LEOContext* theContext )
{
	if( --theContext->referenceCount == 0 )
	{
		LEOCleanUpStackToPtr( theContext, theContext->stack );
		LEOContextGroupRelease( theContext->group );
		theContext->group = NULL;
		if( theContext->callStackEntries )
		{
			for( size_t x = 0; x < theContext->numCallStackEntries; x++ )
			{
				LEOScriptRelease( theContext->callStackEntries[x].script );
				theContext->callStackEntries[x].script = NULL;
				theContext->callStackEntries[x].handler = NULL;	// Script owns handlers, so this is invalid now, too.
			}
			
			free( theContext->callStackEntries );
			theContext->callStackEntries = NULL;
			theContext->numCallStackEntries = 0;
		}
		
		if( theContext->cleanUpUserData )
		{
			theContext->cleanUpUserData( theContext->userData );
			theContext->cleanUpUserData = NULL;
			theContext->userData = NULL;
		}
		free(theContext);
	}
}


void	LEOContextPushHandlerScriptReturnAddressAndBasePtr( LEOContext* inContext, LEOHandler* inHandler, LEOScript* inScript, LEOInstruction* returnAddress, LEOValuePtr oldBP )
{
	size_t		newEntryIndex = 0;
	if( inContext->callStackEntries == NULL )
	{
		inContext->numCallStackEntries = 1;
		inContext->callStackEntries = calloc( LEOCallStackEntriesChunkSize, sizeof(struct LEOCallStackEntry) );
		
		newEntryIndex = 0;	// Can start with first item right away.
	}
	else
	{
		newEntryIndex = inContext->numCallStackEntries;
		inContext->numCallStackEntries ++;
		if( (inContext->numCallStackEntries % LEOCallStackEntriesChunkSize) == 1 )	// Just exceeded previous block?
		{
			size_t	numSlots = inContext->numCallStackEntries +LEOCallStackEntriesChunkSize -1;
			inContext->callStackEntries = realloc( inContext->callStackEntries, sizeof(struct LEOCallStackEntry) * numSlots );
		}
	}
	
	inContext->callStackEntries[newEntryIndex].handler = inHandler;
	inContext->callStackEntries[newEntryIndex].script = LEOScriptRetain( inScript );
	inContext->callStackEntries[newEntryIndex].returnAddress = returnAddress;
	inContext->callStackEntries[newEntryIndex].oldBasePtr = oldBP;
}


LEOHandler*	LEOContextPeekCurrentHandler( LEOContext* inContext )
{
	if( inContext->numCallStackEntries < 1 )
	{
		LEOContextStopWithError( inContext, SIZE_T_MAX, SIZE_T_MAX, 0, "Error: No current handler found." );
		return NULL;
	}
	else
		return inContext->callStackEntries[inContext->numCallStackEntries -1].handler;
}


LEOScript*	LEOContextPeekCurrentScript( LEOContext* inContext )
{
	if( inContext->numCallStackEntries < 1 )
	{
		LEOContextStopWithError( inContext, SIZE_T_MAX, SIZE_T_MAX, 0, "Error: No current script found." );
		return NULL;
	}
	else
		return inContext->callStackEntries[inContext->numCallStackEntries -1].script;
}


LEOInstruction*	LEOContextPeekReturnAddress( LEOContext* inContext )
{
	if( inContext->numCallStackEntries < 1 )
	{
		LEOContextStopWithError( inContext, SIZE_T_MAX, SIZE_T_MAX, 0, "Error: No return address found." );
		return NULL;
	}
	else
		return inContext->callStackEntries[inContext->numCallStackEntries -1].returnAddress;
}


LEOValuePtr	LEOContextPeekBasePtr( LEOContext* inContext )
{
	if( inContext->numCallStackEntries < 1 )
	{
		LEOContextStopWithError( inContext, SIZE_T_MAX, SIZE_T_MAX, 0, "Error: No base pointer found." );
		return NULL;
	}
	else
		return inContext->callStackEntries[inContext->numCallStackEntries -1].oldBasePtr;
}


void	LEOContextPopHandlerScriptReturnAddressAndBasePtr( LEOContext* inContext )
{
	if( inContext->numCallStackEntries < 1 )
	{
		LEOContextStopWithError( inContext, SIZE_T_MAX, SIZE_T_MAX, 0, "Error: Script attempted to return from handler that has never been called." );
		return;
	}
	
	inContext->numCallStackEntries--;
	LEOScriptRelease( inContext->callStackEntries[inContext->numCallStackEntries].script );
	
	if( (inContext->numCallStackEntries % LEOCallStackEntriesChunkSize) == 0 && (inContext->numCallStackEntries > 0) )
	{
		inContext->callStackEntries = realloc( inContext->callStackEntries, sizeof(struct LEOCallStackEntry) * inContext->numCallStackEntries );
	}
}


LEOValuePtr	LEOPushValueOnStack( LEOContext* theContext, LEOValuePtr inValueToCopy )
{
	if( !theContext->stackEndPtr )
		theContext->stackEndPtr = theContext->stack;

	LEOValuePtr		theValue = theContext->stackEndPtr;
	
	theContext->stackEndPtr++;
	
	if( inValueToCopy )
		LEOInitCopy( inValueToCopy, theValue, kLEOInvalidateReferences, theContext );
	
	return theValue;
}


LEOValuePtr	LEOPushUnsetValueOnStack( LEOContext* theContext )
{
	if( !theContext->stackEndPtr )
		theContext->stackEndPtr = theContext->stack;

	LEOValuePtr		theValue = theContext->stackEndPtr;
	
	theContext->stackEndPtr++;
	
	LEOInitUnsetValue( theValue, kLEOInvalidateReferences, theContext );
	
	return theValue;
}


LEOValuePtr	LEOPushEmptyValueOnStack( LEOContext* theContext )
{
	if( !theContext->stackEndPtr )
		theContext->stackEndPtr = theContext->stack;

	LEOValuePtr		theValue = theContext->stackEndPtr;
	
	theContext->stackEndPtr++;
	
	LEOInitStringConstantValue( theValue, "", kLEOInvalidateReferences, theContext );
	
	return theValue;
}


LEOValuePtr	LEOPushStringValueOnStack( LEOContext* theContext, const char* inString, size_t strLen )
{
	if( !theContext->stackEndPtr )
		theContext->stackEndPtr = theContext->stack;
	
	LEOValuePtr		theValue = theContext->stackEndPtr;
	
	theContext->stackEndPtr++;
	
	LEOInitStringValue( theValue, inString, strLen, kLEOInvalidateReferences, theContext );
	
	return theValue;
}


LEOValuePtr	LEOPushStringConstantValueOnStack( LEOContext* theContext, const char* inString )
{
	if( !theContext->stackEndPtr )
		theContext->stackEndPtr = theContext->stack;
	
	LEOValuePtr		theValue = theContext->stackEndPtr;
	
	theContext->stackEndPtr++;
	
	LEOInitStringConstantValue( theValue, inString, kLEOInvalidateReferences, theContext );
	
	return theValue;
}


LEOValuePtr	LEOPushIntegerOnStack( LEOContext* theContext, LEOInteger inInteger, LEOUnit inUnit )
{
	if( !theContext->stackEndPtr )
		theContext->stackEndPtr = theContext->stack;

	LEOValuePtr		theValue = theContext->stackEndPtr;
	
	theContext->stackEndPtr++;
	
	LEOInitIntegerValue( theValue, inInteger, inUnit, kLEOInvalidateReferences, theContext );
	
	return theValue;
}


LEOValuePtr	LEOPushNumberOnStack( LEOContext* theContext, LEONumber inNumber, LEOUnit inUnit )
{
	if( !theContext->stackEndPtr )
		theContext->stackEndPtr = theContext->stack;

	LEOValuePtr		theValue = theContext->stackEndPtr;
	
	theContext->stackEndPtr++;
	
	LEOInitNumberValue( theValue, inNumber, inUnit, kLEOInvalidateReferences, theContext );
	
	return theValue;
}


LEOValuePtr	LEOPushBooleanOnStack( LEOContext* theContext, bool inBoolean )
{
	if( !theContext->stackEndPtr )
		theContext->stackEndPtr = theContext->stack;

	LEOValuePtr		theValue = theContext->stackEndPtr;
	
	theContext->stackEndPtr++;
	
	LEOInitBooleanValue( theValue, inBoolean, kLEOInvalidateReferences, theContext );
	
	return theValue;
}


void	LEOCleanUpStackToPtr( LEOContext* theContext, union LEOValue* lastItemToDelete )
{
	if( theContext->stack > lastItemToDelete )
	{
		printf("Error: Attempt to pop nonexistent elements off the stack.\n");
		return;
	}
	
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
	
	if( (inContext->flags & kLEOContextPause) == 0 && inContext->contextCompleted )
		inContext->contextCompleted( inContext );
}


static LEOContext*	sContextToResume = NULL;


void	LEOResumeContext( LEOContext *inContext )
{
	sContextToResume = LEOContextRetain( inContext );
	if( gCheckForResumeProc )
		gCheckForResumeProc();
}


void	LEOContextResumeIfAvailable( void )
{
	LEOContext*	contextToResume = sContextToResume;
	contextToResume->flags |= kLEOContextResuming | kLEOContextKeepRunning;
	contextToResume->flags &= ~kLEOContextPause;
	
	bool	goOn = LEOContinueRunningContext( contextToResume );
	contextToResume->flags &= ~kLEOContextResuming;
	if( goOn )
	{
		while( LEOContinueRunningContext( contextToResume ) )
			;
	}
	
	if( (contextToResume->flags & kLEOContextPause) == 0 && contextToResume->contextCompleted )
		contextToResume->contextCompleted( contextToResume );
	
	contextToResume = NULL;	// Either we're done, or we're paused.
}


void	LEOPauseContext( LEOContext *inContext )
{
	inContext->flags |= kLEOContextPause;
}


void	LEOPrepareContextForRunning( LEOInstruction instructions[], LEOContext *inContext )
{
	inContext->flags = kLEOContextKeepRunning;
	inContext->flags &= ~(kLEOContextPause | kLEOContextResuming);
	inContext->currentInstruction = instructions;
	if( !inContext->stackEndPtr )
		inContext->stackEndPtr = inContext->stack;
	inContext->stackBasePtr = inContext->stackEndPtr;
	inContext->errMsg[0] = 0;
	
	// +++ Should we call LEOCleanUpStackToPtr here? Would be necessary for reusing a context.
}


bool	LEOContinueRunningContext( LEOContext *inContext )
{
	inContext->errMsg[0] = 0;
	
	inContext->preInstructionProc(inContext);
	if( inContext->currentInstruction == NULL || (inContext->flags & kLEOContextKeepRunning) == 0 )	// Did pre-instruction-proc request abort?
		return false;
	
	LEOInstructionID	currID = inContext->currentInstruction->instructionID;
	if( currID >= gNumInstructions )
		currID = 0;	// First instruction is the special "unimplemented" instruction.
		
	if( gInstructionIDToDebugPrintBefore == currID )
		LEODebugPrintContext(inContext);
		
	gInstructions[currID].proc(inContext);
	
	if( gInstructionIDToDebugPrintAfter == currID )
		LEODebugPrintContext(inContext);
	
	return( inContext->currentInstruction != NULL && (inContext->flags & kLEOContextKeepRunning) && (inContext->flags & kLEOContextPause) == 0 );
}


void	LEOContextStopWithError( LEOContext* inContext, size_t errLine, size_t errOffset, uint16_t fileID, const char* inErrorFmt, ... )
{
	va_list		varargs;
	va_start( varargs, inErrorFmt );
	inContext->errMsg[sizeof(inContext->errMsg) -1] = 0;
	vsnprintf( inContext->errMsg, sizeof(inContext->errMsg) -1, inErrorFmt, varargs );
	va_end( varargs );
	inContext->errLine = errLine;
	inContext->errOffset = errOffset;
	inContext->errFileID = fileID;
	inContext->flags &= ~kLEOContextKeepRunning;
	
	inContext->promptProc( inContext );
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
		printf("%s",gInstructions[currID].name);
	printf("( %u, %d );\n", instruction->param1, instruction->param2 );
}


void	LEODebugPrintInstructions( LEOInstruction instructions[], size_t numInstructions )
{
	//printf( "%u INSTRUCTIONS:\n", (unsigned int)numInstructions );
	for( size_t x = 0; x < numInstructions; x++ )
	{
		printf( "    " );
		LEODebugPrintInstr( instructions +x );
	}
}


void	LEOContextDebugPrintCallStack( LEOContext* inContext )
{
	if( inContext->numCallStackEntries == 0 )
		return;
	
	size_t x = inContext->numCallStackEntries;
	
	do
	{
		x--;
		
		LEOHandlerID theID = inContext->callStackEntries[x].handler->handlerName;
		printf( "%s\n", LEOContextGroupHandlerNameForHandlerID( inContext->group, theID ) );
	} while( x > 0 );
}


void	LEODebugPrintContext( LEOContext* ctx )
{
	printf( "CONTEXT:\n" );
	if( (ctx->flags & kLEOContextKeepRunning) == 0 )
		printf( "    keepRunning: FALSE\n" );
	if( ctx->errMsg[0] != 0 )
		printf( "    errMsg: %s\n", ctx->errMsg );
	printf( "    currentInstruction: " ); LEODebugPrintInstr( ctx->currentInstruction );
	
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
			if( (ctx->flags & kLEOContextKeepRunning) == 0 && ctx->errMsg[0] != 0 )
			{
				ctx->flags |= kLEOContextKeepRunning;
				strcpy( str, ctx->errMsg );
				ctx->errMsg[0] = 0;
				
			}
			printf( "\"%s\" (%s)", str, currValue->base.isa->displayTypeName );
			
			long		bpRelativeAddress = currValue -ctx->stackBasePtr;
			if( bpRelativeAddress >= 0 && ctx->numCallStackEntries > 0 )
			{
				char*		theName = NULL;
				char*		theRealName = NULL;
				LEOHandlerFindVariableByAddress( ctx->callStackEntries[ ctx->numCallStackEntries -1 ].handler,
										bpRelativeAddress, &theName, &theRealName, ctx );
				if( strlen(theRealName) > 0 )
					printf("\t%s",theRealName);
				if( strlen(theName) > 0 )
					printf(" [%s]",theName);
			}
			printf( "\n" );
			
			currValue ++;
		}
		
		printf( "    -----------------------------\n" );
		LEOContextDebugPrintCallStack( ctx );
	}
}




