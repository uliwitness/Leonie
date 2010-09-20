/*
 *  LEODebugger.c
 *  Leonie
 *
 *  Created by Uli Kusterer on 20.09.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include "LEODebugger.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>


LEOInstruction**	gLEODebuggerBreakpoints = NULL;
size_t				gLEONumDebuggerBreakpoints = 0;


void LEODebuggerPrompt( struct LEOContext* inContext )
{
	bool	stayInDebuggerPrompt = true;
	while( stayInDebuggerPrompt )
	{
		char		currCmd[128] = { 0 };
		printf("LDB> ");
		if( fgets(currCmd,sizeof(currCmd),stdin) != NULL )
		{
			size_t		lastCharIdx = strlen(currCmd);
			if( lastCharIdx > 0 )
				lastCharIdx -= 1;
			currCmd[lastCharIdx] = 0;	// Remove the return character.
			if( strcasecmp(currCmd,"continue") == 0 )
			{
				stayInDebuggerPrompt = false;
				printf( "\n" );
			}
			else if( strcasecmp(currCmd,"si") == 0 )
			{
				stayInDebuggerPrompt = false;
				inContext->numSteps += 1;
			}
			else if( strcasecmp(currCmd,"bt") == 0 || strcasecmp(currCmd,"backtrace") == 0  )
			{
				LEODebugPrintContext( inContext );
				printf("\n");
			}
			else if( strcasecmp(currCmd,"help") == 0 )
			{
				printf( "LDB, the Leonie Debugger, Version 1.0.\nList of commands:\n" );
				printf( "    continue  - continue running the program until the next breakpoint.\n" );
				printf( "    si        - Execute the current byte-code instruction, then break into the debugger again.\n" );
				printf( "    backtrace - Display information about the current context, including a stack backtrace.\n" );
				printf( "    bt        - Short form of 'backtrace'.\n" );
				printf( "    help      - Display this help text.\n\n" );
			}
			else
				printf( "Unknown command '%s'. Type 'help' for help.\n\n", currCmd );
		}
	}
}


void LEODebuggerPreInstructionProc( struct LEOContext* inContext )
{
	if( inContext->numSteps > 0 )
	{
		inContext->numSteps--;
		printf("  %p: ", inContext->currentInstruction); LEODebugPrintInstr( inContext->currentInstruction );
		LEODebuggerPrompt( inContext );
	}
	else if( gLEODebuggerBreakpoints )
	{
		for( size_t x = 0; x < gLEONumDebuggerBreakpoints; x++ )
		{
			if( inContext->currentInstruction == gLEODebuggerBreakpoints[x] )
			{
				printf("* %p: ", inContext->currentInstruction); LEODebugPrintInstr( inContext->currentInstruction );
				LEODebuggerPrompt( inContext );
				break;
			}
		}
	}
}

void LEODebuggerAddBreakpoint( LEOInstruction* targetInstruction )
{
	printf("Set Breakpoint on instruction %p:",targetInstruction); LEODebugPrintInstr( targetInstruction );
	
	gLEONumDebuggerBreakpoints += 1;
	
	if( !gLEODebuggerBreakpoints )
		gLEODebuggerBreakpoints = malloc( sizeof(LEOInstruction*) );
	else
		gLEODebuggerBreakpoints = realloc( gLEODebuggerBreakpoints, sizeof(LEOInstruction*) * gLEONumDebuggerBreakpoints );
	
	gLEODebuggerBreakpoints[gLEONumDebuggerBreakpoints-1] = targetInstruction;
}


void LEODebuggerRemoveBreakpoint( LEOInstruction* targetInstruction )
{
	for( size_t x = 0; x < gLEONumDebuggerBreakpoints; x++ )
	{
		if( targetInstruction == gLEODebuggerBreakpoints[x] )
		{
			gLEODebuggerBreakpoints[x] = NULL;
		}
	}
}