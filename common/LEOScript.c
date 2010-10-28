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


#define		NUM_INSTRUCTIONS_PER_CHUNK		16
#define		NUM_STRINGS_PER_CHUNK			16




void	LEOInitHandlerWithID( LEOHandler* inStorage, LEOHandlerID inHandlerName )
{
	inStorage->handlerName = inHandlerName;
	inStorage->numInstructions = 0;
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
	
	inStorage->handlerName = LEOHandlerIDINVALID;
}


void	LEOHandlerAddInstruction( LEOHandler* inHandler, LEOInstructionID instructionID, uint16_t param1, uint32_t param2 )
{
	inHandler->numInstructions ++;
	if( (inHandler->numInstructions % NUM_INSTRUCTIONS_PER_CHUNK) == 1 && inHandler->numInstructions != 1 )
	{
		LEOInstruction*	instructionArray = realloc( inHandler->instructions, inHandler->numInstructions -1 +NUM_INSTRUCTIONS_PER_CHUNK );
		if( instructionArray )
			inHandler->instructions = instructionArray;
		else
			return;
	}
	
	inHandler->instructions[inHandler->numInstructions -1].instructionID = instructionID;
	inHandler->instructions[inHandler->numInstructions -1].param1 = param1;
	inHandler->instructions[inHandler->numInstructions -1].param2 = param2;
}


LEOScript*	LEOScriptCreateForOwner( LEOObjectID ownerObject, LEOObjectSeed ownerSeed )
{
	LEOScript	*	theStorage = malloc( sizeof(LEOScript) );
	
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
			free( inScript->strings +x );
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
		commandsArray = malloc( sizeof(LEOHandler) * inScript->numCommands );
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
		commandsArray = malloc( sizeof(LEOHandler) * inScript->numFunctions );
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
	inScript->numStrings ++;

	if( inScript->numStrings == 1 && inScript->strings == NULL )
		inScript->strings = malloc( NUM_STRINGS_PER_CHUNK *sizeof(char*) );
	else if( (inScript->numStrings % NUM_STRINGS_PER_CHUNK) == 1 && inScript->numStrings != 1 )
	{
		char**	stringsArray = realloc( inScript->strings, (inScript->numStrings -1 +NUM_STRINGS_PER_CHUNK) *sizeof(char*) );
		if( stringsArray )
			inScript->strings = stringsArray;
		else
			return SIZE_MAX;
	}
	
	size_t		inStringLen = strlen(inString) +1;
	inScript->strings[inScript->numStrings -1] = malloc( inStringLen );
	memmove( inScript->strings[inScript->numStrings -1], inString, inStringLen );
	
	return inScript->numStrings -1;
}



