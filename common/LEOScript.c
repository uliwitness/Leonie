/*
 *  LEOScript.c
 *  Leonie
 *
 *  Created by Uli Kusterer on 16.10.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include "LEOScript.h"
#include "LEOHandlerID.h"
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "LEOContextGroup.h"


#define		NUM_INSTRUCTIONS_PER_CHUNK		16
#define		NUM_STRINGS_PER_CHUNK			16


void	LEOInitHandlerWithID( LEOHandler* inStorage, LEOHandlerID inHandlerName )
{
	inStorage->handlerName = inHandlerName;
	inStorage->numInstructions = 0;
	inStorage->numVariables = 0;
	inStorage->varNames = NULL;
	inStorage->instructions = calloc(NUM_INSTRUCTIONS_PER_CHUNK, sizeof(LEOInstruction));
}


void	LEOCleanUpHandler( LEOHandler* inStorage )
{
	if( inStorage->instructions )
	{
		free( inStorage->instructions );
		inStorage->numInstructions = 0;
		inStorage->instructions = NULL;
	}
	
	if( inStorage->varNames )
	{
		free( inStorage->varNames );
		inStorage->numVariables = 0;
		inStorage->varNames = NULL;
	}
	
	inStorage->handlerName = kLEOHandlerIDINVALID;
}


void	LEOHandlerAddInstruction( LEOHandler* inHandler, LEOInstructionID instructionID, uint16_t param1, uint32_t param2 )
{
	inHandler->numInstructions ++;
	if( (inHandler->numInstructions % NUM_INSTRUCTIONS_PER_CHUNK) == 1 && inHandler->numInstructions != 1 )
	{
		size_t			numSlots = inHandler->numInstructions -1 +NUM_INSTRUCTIONS_PER_CHUNK;
		LEOInstruction*	instructionArray = realloc( inHandler->instructions, numSlots *sizeof(LEOInstruction) );
		if( instructionArray )
			inHandler->instructions = instructionArray;
		else
		{
			printf( "*** Failed to allocate instruction! ***\n" );
			return;
		}
	}
		
	inHandler->instructions[inHandler->numInstructions -1].instructionID = instructionID;
	inHandler->instructions[inHandler->numInstructions -1].param1 = param1;
	inHandler->instructions[inHandler->numInstructions -1].param2 = param2;
}


void	LEOHandlerAddVariableNameMapping( LEOHandler* inHandler, const char* inName, const char *inRealName, size_t inBPRelativeAddress )
{
	if( !inHandler->varNames )
	{
		inHandler->numVariables = 1;
		inHandler->varNames = calloc( 1, sizeof(struct LEOVariableNameMapping) );
		if( !inHandler->varNames )
		{
			printf( "*** Failed to allocate var name entry! ***\n" );
			return;
		}
	}
	else
	{
		struct LEOVariableNameMapping	*	vars = realloc( inHandler->varNames, sizeof(struct LEOVariableNameMapping) * (inHandler->numVariables +1) );
		if( vars )
		{
			inHandler->varNames = vars;
			inHandler->numVariables++;
		}
		else
		{
			printf( "*** Failed to allocate var name entry! ***\n" );
			return;
		}
	}
	
	strncpy( inHandler->varNames[ inHandler->numVariables -1 ].variableName, inName, DBG_VAR_NAME_SIZE );
	strncpy( inHandler->varNames[ inHandler->numVariables -1 ].realVariableName, inRealName, DBG_VAR_NAME_SIZE );
	inHandler->varNames[ inHandler->numVariables -1 ].bpRelativeAddress = inBPRelativeAddress;
}


void	LEOHandlerFindVariable( LEOHandler* inHandler, long bpRelativeAddress, char** outName, char**outRealName )
{
	for( size_t x = 0; x < inHandler->numVariables; x++ )
	{
		if( inHandler->varNames[x].bpRelativeAddress == bpRelativeAddress )
		{
			*outName = inHandler->varNames[x].variableName;
			*outRealName = inHandler->varNames[x].realVariableName;
			return;
		}
	}
	
	*outName = "";
	*outRealName = "";
}


LEOScript*	LEOScriptCreateForOwner( LEOObjectID ownerObject, LEOObjectSeed ownerSeed )
{
	LEOScript	*	theStorage = calloc( 1, sizeof(LEOScript) );
	
	if( theStorage )
	{
		theStorage->ownerObject = ownerObject;
		theStorage->ownerObjectSeed = ownerSeed;
		theStorage->referenceCount = 1;
		theStorage->numFunctions = 0;
		theStorage->functions = NULL;
		theStorage->numCommands = 0;
		theStorage->commands = NULL;
		theStorage->numStrings = 0;
		theStorage->strings = NULL;
	}
	
	return theStorage;
}


LEOScript*	LEOScriptRetain( LEOScript* inScript )
{
	inScript->referenceCount ++;
	return inScript;
}


void	LEOScriptRelease( LEOScript* inScript )
{
	inScript->referenceCount --;
	
	if( inScript->referenceCount == 0 )
	{
		for( size_t x = 0; x < inScript->numFunctions; x++ )
		{
			LEOCleanUpHandler( inScript->functions +x );
		}
		for( size_t x = 0; x < inScript->numCommands; x++ )
		{
			LEOCleanUpHandler( inScript->commands +x );
		}
		for( size_t x = 0; x < inScript->numStrings; x++ )
		{
			free( inScript->strings[x] );
			inScript->strings[x] = NULL;
		}
		
		free( inScript );
	}
}


LEOHandler*	LEOScriptAddCommandHandlerWithID( LEOScript* inScript, LEOHandlerID inHandlerName )
{
	inScript->numCommands++;
	LEOHandler*		commandsArray = NULL;
	if( inScript->commands )
		commandsArray = realloc( inScript->commands, sizeof(LEOHandler) * inScript->numCommands );
	else
		commandsArray = calloc( sizeof(LEOHandler) * inScript->numCommands, 1 );
	if( commandsArray )
	{
		LEOInitHandlerWithID( commandsArray +inScript->numCommands -1, inHandlerName );
		inScript->commands = commandsArray;
		
		return commandsArray +inScript->numCommands -1;
	}
	else
		return NULL;
}


LEOHandler*	LEOScriptAddFunctionHandlerWithID( LEOScript* inScript, LEOHandlerID inHandlerName )
{
	inScript->numFunctions++;
	LEOHandler*		commandsArray = NULL;
	if( inScript->commands )
		commandsArray = realloc( inScript->functions, sizeof(LEOHandler) * inScript->numFunctions );
	else
		commandsArray = calloc( sizeof(LEOHandler) * inScript->numFunctions, 1 );
	if( commandsArray )
	{
		LEOInitHandlerWithID( commandsArray +inScript->numFunctions -1, inHandlerName );
		inScript->functions = commandsArray;
		
		return commandsArray +inScript->numFunctions -1;
	}
	else
		return NULL;
}


LEOHandler*	LEOScriptFindCommandHandlerWithID( LEOScript* inScript, LEOHandlerID inHandlerName )
{
	for( size_t x = 0; x < inScript->numCommands; x++ )
	{
		if( inScript->commands[x].handlerName == inHandlerName )
			return inScript->commands + x;
	}
	
	return NULL;
}


LEOHandler*	LEOScriptFindFunctionHandlerWithID( LEOScript* inScript, LEOHandlerID inHandlerName )
{
	for( size_t x = 0; x < inScript->numFunctions; x++ )
	{
		if( inScript->functions[x].handlerName == inHandlerName )
			return inScript->functions + x;
	}
	
	return NULL;
}


size_t	LEOScriptAddString( LEOScript* inScript, const char* inString )
{
	// First, try to re-use an existing string:
	if( inScript->strings )
	{
		for( size_t x = 0; x < inScript->numStrings; x++ )
		{
			const char*	possibleMatch = inScript->strings[x];
			if( strcmp( possibleMatch, inString ) == 0 )	// Absolutely equal, doesn't even differ in case? (wouldn't want to change the case of user's text!)
				return x;
		}
	}
	
	// Otherwise, add new entry for this string:
	inScript->numStrings ++;

	if( inScript->numStrings == 1 && inScript->strings == NULL )
	{
		inScript->strings = calloc( NUM_STRINGS_PER_CHUNK, sizeof(char*) );
	}
	else if( (inScript->numStrings % NUM_STRINGS_PER_CHUNK) == 1 && inScript->numStrings != 1 )
	{
		size_t		numSlots = (inScript->numStrings -1 +NUM_STRINGS_PER_CHUNK);
		char**	stringsArray = realloc( inScript->strings, numSlots * sizeof(char*) );
		if( stringsArray )
			inScript->strings = stringsArray;
		else
		{
			printf( "*** Failed to allocate string! ***\n" );
			return SIZE_MAX;
		}
	}
	
	size_t		inStringLen = strlen(inString) +1;
	char*		newStr = calloc( inStringLen, sizeof(char) );
	inScript->strings[inScript->numStrings -1] = newStr;
	memmove( newStr, inString, inStringLen );
	
	return inScript->numStrings -1;
}


void	LEODebugPrintHandler( struct LEOContextGroup* inGroup, LEOHandler* inHandler )
{
	printf( "%s:\n", LEOContextGroupHandlerNameForHandlerID( inGroup, inHandler->handlerName ) );
	LEODebugPrintInstructions( inHandler->instructions, inHandler->numInstructions );
	for( size_t x = 0; x < inHandler->numVariables; x++ )
	{
		printf( "%s @%ld (%s)\n", inHandler->varNames[x].realVariableName,
				inHandler->varNames[x].bpRelativeAddress, inHandler->varNames[x].variableName );
	}
}


void	LEODebugPrintScript( struct LEOContextGroup* inGroup, LEOScript* inScript )
{
	printf( "----------\n" );
	printf("FUNCTIONS:\n");
	for( size_t x = 0; x < inScript->numFunctions; x++ )
		LEODebugPrintHandler( inGroup, inScript->functions +x );
	printf("COMMANDS:\n");
	for( size_t x = 0; x < inScript->numCommands; x++ )
		LEODebugPrintHandler( inGroup, inScript->commands +x );
	printf("STRINGS:\n");
	for( size_t x = 0; x < inScript->numStrings; x++ )
		printf( "\t\"%s\"\n", inScript->strings[x] );
	printf( "----------\n" );
}
