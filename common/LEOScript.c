/*
 *  LEOScript.c
 *  Leonie
 *
 *  Created by Uli Kusterer on 16.10.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include "LEOScript.h"
#include <stddef.h>
#include <string.h>
#include <stdlib.h>


#define		NUM_INSTRUCTIONS_PER_CHUNK		16




void	LEOInitHandlerNamed( LEOHandler* inStorage, const char* inHandlerName )
{
	strncpy( inStorage->handlerName, inHandlerName, sizeof(inStorage->handlerName) -1 );
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
	
	inStorage->handlerName[0] = 0;
}


void	LEOHandlerAddInstruction( LEOHandler* inHandler, LEOInstruction instr )
{
	inHandler->numInstructions ++;
	if( (inHandler->numInstructions % NUM_INSTRUCTIONS_PER_CHUNK) == 1 && inHandler->numInstructions != 1 )
	{
		LEOInstruction*	instructionArray = realloc( inHandler->instructions, inHandler->numInstructions -1 +NUM_INSTRUCTIONS_PER_CHUNK );
		if( instructionArray )
		{
			memcpy( instructionArray +inHandler->numInstructions -1, &instr, sizeof(LEOInstruction) );
			inHandler->instructions = instructionArray;
		}
	}
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
		
		free( inScript );
	}
}


LEOHandler*	LEOScriptAddCommandHandlerNamed( LEOScript* inScript, const char* inName )
{
	inScript->numCommands++;
	LEOHandler*		commandsArray = realloc( inScript->commands, sizeof(LEOHandler) * inScript->numCommands );
	if( commandsArray )
	{
		LEOInitHandlerNamed( commandsArray +inScript->numCommands -1, inName );
		inScript->commands = commandsArray;
		
		return commandsArray +inScript->numCommands -1;
	}
	else
		return NULL;
}


LEOHandler*	LEOScriptAddFunctionHandlerNamed( LEOScript* inScript, const char* inName )
{
	inScript->numFunctions++;
	LEOHandler*		commandsArray = realloc( inScript->functions, sizeof(LEOHandler) * inScript->numFunctions );
	if( commandsArray )
	{
		LEOInitHandlerNamed( commandsArray +inScript->numFunctions -1, inName );
		inScript->functions = commandsArray;
		
		return commandsArray +inScript->numFunctions -1;
	}
	else
		return NULL;
}


LEOHandler*	LEOScriptFindCommandHandlerNamed( LEOScript* inScript, const char* inName )
{
	for( size_t x = 0; x < inScript->numCommands; x++ )
	{
		if( strcasecmp( inScript->commands[x].handlerName, inName ) == 0 )
			return inScript->commands + x;
	}
	
	return NULL;
}


LEOHandler*	LEOScriptFindFunctionHandlerNamed( LEOScript* inScript, const char* inName )
{
	for( size_t x = 0; x < inScript->numCommands; x++ )
	{
		if( strcasecmp( inScript->commands[x].handlerName, inName ) == 0 )
			return inScript->commands + x;
	}
	
	return NULL;
}


