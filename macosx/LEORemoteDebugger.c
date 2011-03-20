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


LEOInstruction**	gLEORemoteDebuggerBreakpoints = NULL;
size_t				gLEONumRemoteDebuggerBreakpoints = 0;

static int			gLEORemoteDebuggerSocketFD = -1;
static bool			gLEORemoteDebuggerInitialized = false;

bool	LEOInitRemoteDebugger( const char* inHostName )
{
	if( !gLEORemoteDebuggerInitialized )
	{
		struct sockaddr_in		serv_addr = { 0 };
		struct hostent		*	server = NULL;

		gLEORemoteDebuggerSocketFD = socket( AF_INET, SOCK_STREAM, 0 );
		int v = 1; 
		if( setsockopt( gLEORemoteDebuggerSocketFD, IPPROTO_TCP, TCP_NODELAY, &v, sizeof(v)) )
		{
			printf( "Couldn't set options when connecting to remote debugger: %d\n", errno );
			return false;
		}
		server = gethostbyname( inHostName );
		if( server == NULL )
			return false;
		
		serv_addr.sin_family = AF_INET;
		bcopy( (char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length );
		serv_addr.sin_port = htons(13762);
		if( connect( gLEORemoteDebuggerSocketFD, (struct sockaddr*)&serv_addr, sizeof(serv_addr) ) < 0 ) 
		{
			printf( "Couldn't connect to remote debugger: %d\n", errno );
			return false;
		}
		gLEORemoteDebuggerInitialized = true;
	}
	
	return true;
}


void LEORemoteDebuggerUpdateState( struct LEOContext* inContext )
{
	size_t	actuallyWritten = write( gLEORemoteDebuggerSocketFD, "EMTY\0\0\0\0", 8 );	// Clear any existing variable display.
	printf( "Remote debugger: Emptying (%lu bytes written)\n", actuallyWritten );
	
	// Print all local variables:
	if( inContext->stackEndPtr != NULL )
	{
		union LEOValue*		currValue = inContext->stack;
		while( currValue != inContext->stackEndPtr )
		{
			long		bpRelativeAddress = currValue -inContext->stackBasePtr;
			if( bpRelativeAddress >= 0 )
			{
				char*		theRealName = NULL;
				char*		theName = NULL;
				LEOHandlerFindVariable( inContext->callStackEntries[ inContext->numCallStackEntries -1 ].handler,
										bpRelativeAddress, &theName, &theRealName );
				if( strlen(theRealName) == 0 && strlen(theName) > 0 )
					theRealName = theName;
				
				char		str[1024] = { 0 };
				LEOGetValueAsString( currValue, str, sizeof(str), inContext );
			
				uint32_t	dataLen = strlen(str) +1 +strlen(currValue->base.isa->displayTypeName) +1 +strlen(theRealName) +1;
				
				actuallyWritten = write( gLEORemoteDebuggerSocketFD, "VARI", 4 );
				actuallyWritten = write( gLEORemoteDebuggerSocketFD, &dataLen, 4 );
				actuallyWritten = write( gLEORemoteDebuggerSocketFD, theRealName, strlen(theRealName) +1 );
				actuallyWritten = write( gLEORemoteDebuggerSocketFD, currValue->base.isa->displayTypeName, strlen(currValue->base.isa->displayTypeName) +1 );
				actuallyWritten = write( gLEORemoteDebuggerSocketFD, str, strlen(str) +1 );
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
			uint32_t		dataLen = strlen(hdlNameStr) +1;
			actuallyWritten = write( gLEORemoteDebuggerSocketFD, "CALL", 4 );
			actuallyWritten = write( gLEORemoteDebuggerSocketFD, &dataLen, 4 );
			actuallyWritten = write( gLEORemoteDebuggerSocketFD, hdlNameStr, strlen(hdlNameStr) +1 );
		} while( x > 0 );
	}
	
	// Tell the debugger what instruction we're currently stopped at:
	char				instructionStr[256] = { 0 };
	unsigned long long	instructionPointer = (unsigned long long) inContext->currentInstruction;	// Address so we can tell repeated calls apart.
	assert( sizeof(instructionPointer) >= sizeof(LEOInstruction*) );
	snprintf( instructionStr, 255, "%s( %d, %d )", gInstructionNames[inContext->currentInstruction->instructionID],
					inContext->currentInstruction->param1, inContext->currentInstruction->param2 );
	size_t	dataLen = strlen(instructionStr) +1 +sizeof(instructionPointer);
	actuallyWritten = write( gLEORemoteDebuggerSocketFD, "INST", 4 );
	actuallyWritten = write( gLEORemoteDebuggerSocketFD, &dataLen, 4 );
	actuallyWritten = write( gLEORemoteDebuggerSocketFD, instructionStr, strlen(instructionStr) +1 );
	actuallyWritten = write( gLEORemoteDebuggerSocketFD, &instructionPointer, sizeof(instructionPointer) );
}


void LEORemoteDebuggerPrompt( struct LEOContext* inContext )
{
	bool	stayInDebuggerPrompt = true;
	while( stayInDebuggerPrompt )
	{
		LEORemoteDebuggerUpdateState( inContext );
		printf( "Remote debugger: About to WAIT\n" );
		size_t	actuallyWritten = write( gLEORemoteDebuggerSocketFD, "WAIT\0\0\0\0", 8 );	// Tell remote debugger to show its prompt now and call us back when the user has made a decision.
		printf( "Remote debugger: Returned from WAIT having written %lu bytes\n", actuallyWritten );
		
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
				inContext->keepRunning = false;
				stayInDebuggerPrompt = false;
				break;
			
			default:
				printf( "Remote Debugger: Unknown command '%c%c%c%c'\n", currBytesPtr[0], currBytesPtr[1], currBytesPtr[2], currBytesPtr[3] );
				break;
		}
	}
}


void LEORemoteDebuggerPreInstructionProc( struct LEOContext* inContext )
{
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