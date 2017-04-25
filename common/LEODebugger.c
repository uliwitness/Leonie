/*
 *  LEODebugger.c
 *  Leonie
 *
 *  Created by Uli Kusterer on 20.09.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include "LEODebugger.h"
#include "LEOScript.h"
#include "LEOInstructions.h"
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
			if( strcasecmp(currCmd,"c") == 0 || strcasecmp(currCmd,"continue") == 0 )
			{
				stayInDebuggerPrompt = false;
				printf( "\n" );
			}
			else if( strcasecmp(currCmd,"si") == 0 || strcasecmp(currCmd,"stepi") == 0 )
			{
				stayInDebuggerPrompt = false;
				inContext->numSteps += 1;
			}
			else if( strcasecmp(currCmd,"bt") == 0 || strcasecmp(currCmd,"backtrace") == 0 )
			{
				LEODebugPrintContext( inContext );
				printf("\n");
			}
			else if( strcasecmp(currCmd,"b") == 0 || strcasecmp(currCmd,"breakpoint") == 0 )
			{
				LEODebuggerAddBreakpoint( inContext->currentInstruction, LEOContextPeekCurrentScript(inContext) );
				printf("\n");
			}
			else if( strcasecmp(currCmd,"delete") == 0 )
			{
				LEODebuggerRemoveBreakpoint( inContext->currentInstruction );
				printf("\n");
			}
			else if( strcasecmp(currCmd,"exit") == 0 || strcasecmp(currCmd,"quit") == 0
					|| strcasecmp(currCmd,"q") == 0 )
			{
				inContext->flags &= ~kLEOContextKeepRunning;
				stayInDebuggerPrompt = false;
			}
			else if( strcasecmp(currCmd,"help") == 0 )
			{
				printf( "LDB, the Leonie Debugger, Version 1.0.\nList of commands:\n" );
				printf( "    continue   - continue running the program until the next breakpoint.\n" );
				printf( "    c   		- Short form of 'continue'.\n" );
				printf( "    stepi      - Execute the current byte-code instruction, then break into the debugger again.\n" );
				printf( "    si         - Short form of 'stepi'.\n" );
				printf( "    backtrace  - Display information about the current context, including a stack backtrace.\n" );
				printf( "    bt         - Short form of 'backtrace'.\n" );
				printf( "    breakpoint - Set a breakpoint at the current instruction.\n" );
				printf( "    b          - Short form of 'breakpoint'.\n" );
				printf( "    delete     - Delete a breakpoint at the current instruction.\n" );
				printf( "    exit       - Stop execution (and exit the debugger).\n" );
				printf( "    quit       - Synonym of 'exit'.\n" );
				printf( "    q          - Short form of 'quit'.\n" );
				printf( "    help       - Display this help text.\n\n" );
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
		LEOScript * script = LEOContextPeekCurrentScript( inContext );
		printf("  %p: ", inContext->currentInstruction); LEODebugPrintInstr( inContext->currentInstruction, script );
		LEODebuggerPrompt( inContext );
	}
	else if( gLEODebuggerBreakpoints )
	{
		for( size_t x = 0; x < gLEONumDebuggerBreakpoints; x++ )
		{
			if( inContext->currentInstruction == gLEODebuggerBreakpoints[x] )
			{
				LEOScript * script = LEOContextPeekCurrentScript( inContext );
				printf("* %p: ", inContext->currentInstruction); LEODebugPrintInstr( inContext->currentInstruction, script );
				LEODebuggerPrompt( inContext );
				break;
			}
		}
	}
	
	if( inContext->currentInstruction && inContext->currentInstruction->instructionID == LINE_MARKER_INSTR )
	{
		LEOScript	*	theScript = LEOContextPeekCurrentScript( inContext );
		if( theScript )
		{
			if( LEOScriptHasBreakpointAtLine( theScript, inContext->currentInstruction->param2 ) )
				LEODebuggerPrompt( inContext );
		}
	}
}


void LEODebuggerAddBreakpoint( LEOInstruction* targetInstruction, LEOScript * inScript )
{
	printf("Set Breakpoint on instruction %p: ",targetInstruction); LEODebugPrintInstr( targetInstruction, inScript );
	
	gLEONumDebuggerBreakpoints += 1;
	
	if( !gLEODebuggerBreakpoints )
		gLEODebuggerBreakpoints = calloc( 1, sizeof(LEOInstruction*) );
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
