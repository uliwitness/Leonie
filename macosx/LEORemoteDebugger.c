/*
 *  LEORemoteDebugger.c
 *  Leonie
 *
 *  Created by Uli Kusterer on 20.09.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include "LEORemoteDebugger.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/tcp.h>
#include "LEOHandlerID.h"
#include "LEOInterpreter.h"
#include "LEOScript.h"
#include "LEOContextGroup.h"
#include "LEOInstructions.h"


void	LEORemoteDebuggerUpdateState( struct LEOContext* inContext );
void	LEORemoteDebuggerAddHandler( struct LEOHandler* inHandler );
void	LEORemoteDebuggerDoNothingPreInstructionProc( LEOContext* inContext );



LEOInstruction**	gLEORemoteDebuggerBreakpoints = NULL;
size_t				gLEONumRemoteDebuggerBreakpoints = 0;

static int			gLEORemoteDebuggerSocketFD = -1;
static bool			gLEORemoteDebuggerInitialized = false;
static char			gLEORemoteDebuggerHostName[1024] = { 0 };


#define LEO_DEBUGGER_PORT		13762


bool	LEOInitRemoteDebugger( const char* inHostName )
{
	if( inHostName )
		strncpy( gLEORemoteDebuggerHostName, inHostName, sizeof(gLEORemoteDebuggerHostName) -1 );
	
	if( !gLEORemoteDebuggerInitialized )
	{
		struct sockaddr_in		serv_addr = { 0 };
		struct hostent		*	server = NULL;

		gLEORemoteDebuggerSocketFD = socket( AF_INET, SOCK_STREAM, 0 );
		int v = 1; 
		if( setsockopt( gLEORemoteDebuggerSocketFD, IPPROTO_TCP, TCP_NODELAY, &v, sizeof(v)) )
		{
			fprintf( stderr, "Couldn't set options when connecting to %s: %d\n", gLEORemoteDebuggerHostName, errno );
			return false;
		}
		server = gethostbyname( gLEORemoteDebuggerHostName );
		if( server == NULL )
		{
			fprintf( stderr, "Couldn't resolve %s\n", gLEORemoteDebuggerHostName );
			return false;
		}
		
		serv_addr.sin_family = AF_INET;
		bcopy( (char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length );
		serv_addr.sin_port = htons(LEO_DEBUGGER_PORT);
		if( connect( gLEORemoteDebuggerSocketFD, (struct sockaddr*)&serv_addr, sizeof(serv_addr) ) < 0 ) 
		{
			//fprintf( stderr, "Couldn't connect to remote debugger %s:%d error = %d\n", gLEORemoteDebuggerHostName, LEO_DEBUGGER_PORT, errno );
			return false;
		}
		gLEORemoteDebuggerInitialized = true;
	}
	
	return true;
}


void LEORemoteDebuggerUpdateState( struct LEOContext* inContext )
{
	if( !gLEORemoteDebuggerInitialized )
		return;
	
	size_t	actuallyWritten = write( gLEORemoteDebuggerSocketFD, "EMTY\0\0\0\0", 8 );	// Clear any existing variable display.
	printf( "Remote debugger: Emptying (%lu bytes written)\n", actuallyWritten );
	
	// Print all local variables:
	if( inContext->stackEndPtr != NULL )
	{
		uint32_t			dataLen = 0;
		char				str[1024] = { 0 };
		union LEOValue*		currValue = inContext->stack;
		while( currValue != inContext->stackEndPtr )
		{
			long		bpRelativeAddress = currValue -inContext->stackBasePtr;
			//if( bpRelativeAddress >= -1 )
			{
				char*				theRealName = NULL;
				char*				theName = NULL;
				unsigned long long	objectID = 0;
				LEOHandlerFindVariableByAddress( inContext->callStackEntries[ inContext->numCallStackEntries -1 ].handler,
										bpRelativeAddress, &theName, &theRealName, inContext );
				if( strlen(theRealName) == 0 && strlen(theName) > 0 )
					theRealName = theName;
				
				objectID = currValue->base.refObjectID;
				
				str[0] = 0;
				if( currValue == NULL || currValue->base.isa == NULL )
					strncpy( str, "*** INVALID ***", sizeof(str)-1 );
				else
				{
					LEOGetValueAsString( currValue, str, sizeof(str), inContext );
					if( (inContext->flags & kLEOContextKeepRunning) == 0 )
					{
						inContext->flags |= kLEOContextKeepRunning;
						strncpy(str, inContext->errMsg, sizeof(str) -1 );
					}
				}
				unsigned long long	referenceObjectID = 0;
				unsigned long long	referenceObjectSeed = 0;
				if( currValue->base.isa == &kLeoValueTypeReference )
				{
					referenceObjectID = currValue->reference.objectID;
					referenceObjectSeed = currValue->reference.objectSeed;
				}
			
				const char*	theTypeName = (currValue && currValue->base.isa) ? currValue->base.isa->displayTypeName : "*** INVALID ***";
				dataLen = (uint32_t) (strlen(str) +1 +strlen(theTypeName) +1 +strlen(theRealName) +1 +sizeof(objectID) +sizeof(referenceObjectID) +sizeof(referenceObjectSeed));
				
				actuallyWritten = write( gLEORemoteDebuggerSocketFD, "VARI", 4 );
				actuallyWritten = write( gLEORemoteDebuggerSocketFD, &dataLen, 4 );
				printf("Remote debugger: Sending 'VARI' (%u bytes) for %s\n",dataLen,theName);
				actuallyWritten = write( gLEORemoteDebuggerSocketFD, theRealName, strlen(theRealName) +1 );
				actuallyWritten = write( gLEORemoteDebuggerSocketFD, theTypeName, strlen(theTypeName) +1 );
				actuallyWritten = write( gLEORemoteDebuggerSocketFD, str, strlen(str) +1 );
				actuallyWritten = write( gLEORemoteDebuggerSocketFD, &objectID, sizeof(objectID) );
				actuallyWritten = write( gLEORemoteDebuggerSocketFD, &referenceObjectID, sizeof(referenceObjectID) );
				actuallyWritten = write( gLEORemoteDebuggerSocketFD, &referenceObjectSeed, sizeof(referenceObjectSeed) );
			}
			currValue ++;
		}
	}
	
	// Print the call stack:
	if( inContext->numCallStackEntries > 0 )
	{
		size_t x = inContext->numCallStackEntries;
		
		do
		{
			x--;
			
			LEOHandlerID	theID = inContext->callStackEntries[x].handler->handlerName;
			const char*		hdlNameStr = LEOContextGroupHandlerNameForHandlerID( inContext->group, theID );
			uint32_t		dataLen = (uint32_t) (strlen(hdlNameStr) +1);
			actuallyWritten = write( gLEORemoteDebuggerSocketFD, "CALL", 4 );
			actuallyWritten = write( gLEORemoteDebuggerSocketFD, &dataLen, 4 );
			printf("Remote debugger: Sending 'CALL' (%u bytes) for %s\n",dataLen,hdlNameStr);
			actuallyWritten = write( gLEORemoteDebuggerSocketFD, hdlNameStr, strlen(hdlNameStr) +1 );
		} while( x > 0 );
	}
	
	// Tell the debugger what instruction we're currently stopped at:
	unsigned long long	instructionPointer = (intptr_t) inContext->currentInstruction;	// Address so we can tell repeated calls apart.
	assert( sizeof(instructionPointer) >= sizeof(LEOInstruction*) );
	uint32_t		dataLen = sizeof(instructionPointer);
	actuallyWritten = write( gLEORemoteDebuggerSocketFD, "CURR", 4 );
	actuallyWritten = write( gLEORemoteDebuggerSocketFD, &dataLen, 4 );
			printf("Remote debugger: Sending 'CURR' (%u bytes) for %llx\n",dataLen,instructionPointer);
	actuallyWritten = write( gLEORemoteDebuggerSocketFD, &instructionPointer, sizeof(instructionPointer) );

	// Tell the debugger what source file we're dealing with:
	if( inContext->currentInstruction->instructionID == LINE_MARKER_INSTR )
	{
		actuallyWritten = write( gLEORemoteDebuggerSocketFD, "LINE", 4 );
		uint16_t	fileID = inContext->currentInstruction->param1;
		uint32_t	lineNumber = inContext->currentInstruction->param2;
		dataLen = sizeof(fileID) + sizeof(lineNumber);
		actuallyWritten = write( gLEORemoteDebuggerSocketFD, &dataLen, sizeof(dataLen) );
		printf("Remote debugger: Sending 'LINE' (%u bytes) for %u\n",dataLen,lineNumber);
		actuallyWritten = write( gLEORemoteDebuggerSocketFD, &fileID, sizeof(fileID) );
		actuallyWritten = write( gLEORemoteDebuggerSocketFD, &lineNumber, sizeof(lineNumber) );
	}
	else if( inContext->currentInstruction->instructionID == PARSE_ERROR_INSTR )
	{
		LEOScript*	script = LEOContextPeekCurrentScript(inContext);
		uint16_t	fileID = script->parseErrors[inContext->currentInstruction->param2].fileID;
		uint32_t	lineNumber = (uint32_t)script->parseErrors[inContext->currentInstruction->param2].errorLine;
		
		actuallyWritten = write( gLEORemoteDebuggerSocketFD, "LINE", 4 );
		dataLen = sizeof(fileID) + sizeof(lineNumber);
		actuallyWritten = write( gLEORemoteDebuggerSocketFD, &dataLen, sizeof(dataLen) );
		printf("Remote debugger: Sending 'LINE' (%u bytes) for %u\n",dataLen,lineNumber);
		actuallyWritten = write( gLEORemoteDebuggerSocketFD, &fileID, sizeof(fileID) );
		actuallyWritten = write( gLEORemoteDebuggerSocketFD, &lineNumber, sizeof(lineNumber) );
	}
}


void	LEORemoteDebuggerAddHandler( struct LEOHandler* inHandler )
{
	if( !gLEORemoteDebuggerInitialized )
		return;
	
	for( size_t x = 0; x < inHandler->numInstructions; x++ )
	{
		char				instructionStr[256] = { 0 };
		unsigned long long	instructionPointer = (intptr_t) (inHandler->instructions +x);	// Address so we can find the right string for the right instruction to show.
		assert( sizeof(instructionPointer) >= sizeof(LEOInstruction*) );
		snprintf( instructionStr, 255, "%s( %d, %d )", gInstructions[inHandler->instructions[x].instructionID].name,
						inHandler->instructions[x].param1, inHandler->instructions[x].param2 );
		size_t	dataLen = strlen(instructionStr) +1 +sizeof(instructionPointer);
		size_t	actuallyWritten = write( gLEORemoteDebuggerSocketFD, "INST", 4 );
		actuallyWritten = write( gLEORemoteDebuggerSocketFD, &dataLen, 4 );
		printf("Remote debugger: Sending 'INST' (%zu bytes) for %s\n",dataLen,instructionStr);
		actuallyWritten = write( gLEORemoteDebuggerSocketFD, instructionStr, strlen(instructionStr) +1 );
		actuallyWritten = write( gLEORemoteDebuggerSocketFD, &instructionPointer, sizeof(instructionPointer) );
	}
}


void	LEORemoteDebuggerAddFile( const char* filecontents, uint16_t inFileID, struct LEOScript* inScript )
{
	if( !gLEORemoteDebuggerInitialized )
		return;
	
	// Tell the debugger what source file we're dealing with:
	const char*	filename = LEOFileNameForFileID( inFileID );
	size_t		filenameLen = strlen(filename) +1;
	size_t		filecontentsLen = strlen(filecontents) +1;
	size_t		actuallyWritten = write( gLEORemoteDebuggerSocketFD, "SOUR", 4 );
	uint32_t	dataLen = (uint32_t) (sizeof(uint16_t) + filenameLen + filecontentsLen);
	actuallyWritten = write( gLEORemoteDebuggerSocketFD, &dataLen, 4 );
	printf("Remote debugger: Sending 'SOUR' (%u bytes) for %s (%d)\n",dataLen,filename,inFileID);
	actuallyWritten = write( gLEORemoteDebuggerSocketFD, &inFileID, sizeof(uint16_t) );
	actuallyWritten = write( gLEORemoteDebuggerSocketFD, filename, filenameLen );
	actuallyWritten = write( gLEORemoteDebuggerSocketFD, filecontents, filecontentsLen );
	
	// Transmit all instructions for this file to remote debugger:
	for( size_t x = 0; x < inScript->numFunctions; x++ )
	{
		LEORemoteDebuggerAddHandler( inScript->functions +x );
	}

	for( size_t x = 0; x < inScript->numCommands; x++ )
	{
		LEORemoteDebuggerAddHandler( inScript->commands +x );
	}
}


void	LEORemoteDebuggerDoNothingPreInstructionProc( LEOContext* inContext )
{
#pragma unused(inContext)
}


void LEORemoteDebuggerPrompt( struct LEOContext* inContext )
{
	if( !gLEORemoteDebuggerInitialized )
		return;
	
	bool	stayInDebuggerPrompt = true;
	while( stayInDebuggerPrompt )
	{
		LEOInstructionFuncPtr	savedPrompt = inContext->promptProc;
		inContext->promptProc = LEORemoteDebuggerDoNothingPreInstructionProc;	// Prevent recursion when something we do here calls LEOContextStopWithError (which would call us again).
		
		LEORemoteDebuggerUpdateState( inContext );
		printf( "Remote debugger: About to WAIT\n" );
		size_t	actuallyWritten = write( gLEORemoteDebuggerSocketFD, "WAIT\0\0\0\0", 8 );	// Tell remote debugger to show its prompt now and call us back when the user has made a decision.
		printf( "Remote debugger: Returned from WAIT having written %lu bytes\n", actuallyWritten );
		
		printf( "Remote debugger: Now waiting for reply.\n" );
		uint32_t		currCmd = 0;
		size_t			amountLeft = sizeof(currCmd);
		char*			currBytesPtr = (char*) &currCmd;
		
		while( amountLeft > 0 )
		{
			//printf( "Remote debugger: About to read.\n" );
			size_t	numRead = read( gLEORemoteDebuggerSocketFD, currBytesPtr, amountLeft );
			//printf( "Remote debugger: Read %lu bytes.\n", numRead );
			amountLeft -= numRead;
			currBytesPtr += numRead;
		}
		currBytesPtr = (char*) &currCmd;
		printf( "Remote debugger: Read one command.\n" );
		
		switch( ntohl(currCmd) )
		{
			case 'CONT':
				printf( "Remote debugger: Continuing.\n" );
				stayInDebuggerPrompt = false;
				break;

			case 'step':
				stayInDebuggerPrompt = false;
				inContext->numSteps += 1;
				printf( "Remote debugger: Stepping.\n" );
				break;

			case '+CHK':
				LEORemoteDebuggerAddBreakpoint( inContext->currentInstruction );
				break;

			case '-CHK':
				LEORemoteDebuggerRemoveBreakpoint( inContext->currentInstruction );
				break;

			case 'EXIT':
				inContext->flags &= ~kLEOContextKeepRunning;
				stayInDebuggerPrompt = false;
				break;
			
			default:
				printf( "Remote Debugger: Unknown command '%c%c%c%c' (%d %d %d %d)\n", currBytesPtr[0], currBytesPtr[1], currBytesPtr[2], currBytesPtr[3], currBytesPtr[0], currBytesPtr[1], currBytesPtr[2], currBytesPtr[3] );
				break;
		}
		
		inContext->promptProc = savedPrompt;
	}
	printf( "Remote Debugger: Exiting debugger prompt.\n" );
}


void LEORemoteDebuggerPreInstructionProc( struct LEOContext* inContext )
{
	if( !gLEORemoteDebuggerInitialized )
		return;
	
	if( inContext->numSteps > 0 )
	{
		inContext->numSteps--;
		LEORemoteDebuggerPrompt( inContext );
	}
	else if( gLEORemoteDebuggerBreakpoints )
	{
		for( size_t x = 0; x < gLEONumRemoteDebuggerBreakpoints; x++ )
		{
			if( inContext->currentInstruction == gLEORemoteDebuggerBreakpoints[x] )
			{
				LEORemoteDebuggerPrompt( inContext );
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
				LEORemoteDebuggerPrompt( inContext );
		}
	}
}


void LEORemoteDebuggerAddBreakpoint( LEOInstruction* targetInstruction )
{
	gLEONumRemoteDebuggerBreakpoints += 1;
	
	if( !gLEORemoteDebuggerBreakpoints )
		gLEORemoteDebuggerBreakpoints = calloc( 1, sizeof(LEOInstruction*) );
	else
		gLEORemoteDebuggerBreakpoints = realloc( gLEORemoteDebuggerBreakpoints, sizeof(LEOInstruction*) * gLEONumRemoteDebuggerBreakpoints );
	
	gLEORemoteDebuggerBreakpoints[gLEONumRemoteDebuggerBreakpoints-1] = targetInstruction;
}


void LEORemoteDebuggerRemoveBreakpoint( LEOInstruction* targetInstruction )
{
	for( size_t x = 0; x < gLEONumRemoteDebuggerBreakpoints; x++ )
	{
		if( targetInstruction == gLEORemoteDebuggerBreakpoints[x] )
		{
			gLEORemoteDebuggerBreakpoints[x] = NULL;
		}
	}
}
