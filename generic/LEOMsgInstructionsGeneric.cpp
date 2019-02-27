/*
 *  LEOMsgInstructionsGeneric.c
 *  Leonie
 *
 *  Created by Uli Kusterer on 09.10.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

/*!
	@header LEOMsgInstructionsGeneric
	Generic implementation of console I/O instructions. Should work on all ANSI C
	systems.
*/

#include "LEOMsgInstructionsGeneric.h"
#include "LEOInterpreter.h"
#include <iostream>
#include <sstream>
#include <vector>
#include "AnsiStrings.h"


LEOInstructionID		kFirstMsgInstruction = 0;
std::ostream*			gLEOMsgOutputStream = &std::cout;



/*!
	Pop a value off the back of the stack (or just read it from the given
	BasePointer-relative address) and present it to the user in string form.
	(PRINT_VALUE_INSTR)
	
	param1	-	If this is BACK_OF_STACK, we're supposed to pop the last item
				off the stack. Otherwise, this is a basePtr-relative address
				where a value will just be read.
*/

void	LEOPrintInstruction( LEOContext* inContext )
{
	char			buf[1024] = { 0 };
	
	bool			popOffStack = (inContext->currentInstruction->param1 == BACK_OF_STACK);
	union LEOValue*	theValue = popOffStack ? (inContext->stackEndPtr -1) : (inContext->stackBasePtr +inContext->currentInstruction->param1);
	const char* theString = LEOGetValueAsString( theValue, buf, sizeof(buf), inContext );
	(*gLEOMsgOutputStream) << theString;
	if( popOffStack )
		LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );
	
	inContext->currentInstruction++;
}


void	LEODeleteInstruction( LEOContext* inContext )
{
	union LEOValue*	theValue = inContext->stackEndPtr -1;
	LEOSetValueAsString( theValue, NULL, 0, inContext );
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );
	
	inContext->currentInstruction++;
}


struct LEOOutputRecordingEntry
{
	explicit LEOOutputRecordingEntry( std::string inVariableName ) : mOutputVariableName(inVariableName) {}
	
	std::string			mOutputVariableName;
	std::stringstream	mOutputDestination;
};


static std::vector<LEOOutputRecordingEntry> sOutputRecordingStack;
static std::ostream*						sOriginalLEOMsgOutputStream = nullptr;


void	LEOStartRecordingOutputInstruction( LEOContext* inContext )
{
	char			buf[1024] = { 0 };
	
	union LEOValue*	theValue = inContext->stackEndPtr -1;
	const char* theString = LEOGetValueAsString( theValue, buf, sizeof(buf), inContext );
	
	if( !sOriginalLEOMsgOutputStream )
		sOriginalLEOMsgOutputStream = gLEOMsgOutputStream;
	sOutputRecordingStack.push_back( LEOOutputRecordingEntry(theString) );
	gLEOMsgOutputStream = &sOutputRecordingStack.back().mOutputDestination;
	
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );
	
	inContext->currentInstruction++;
}


void	LEOStopRecordingOutputInstruction( LEOContext* inContext )
{
	char			buf[1024] = { 0 };
	
	union LEOValue*	theValue = inContext->stackEndPtr -1;
	const char* theString = LEOGetValueAsString( theValue, buf, sizeof(buf), inContext );
	
	if( sOutputRecordingStack.size() > 0 )
	{
		if( strcasecmp( theString, sOutputRecordingStack.back().mOutputVariableName.c_str()) == 0 )
		{
			LEOContextSetLocalVariable( inContext, sOutputRecordingStack.back().mOutputVariableName.c_str(), "%s", sOutputRecordingStack.back().mOutputDestination.str().c_str() );
			sOutputRecordingStack.pop_back();	// Remove our override.
			
			if( sOutputRecordingStack.size() > 0 )	// Restore previous output destination, if one.
			{
				gLEOMsgOutputStream = &sOutputRecordingStack.back().mOutputDestination;
			}
			else	// No more destinations? Restore default.
			{
				gLEOMsgOutputStream = sOriginalLEOMsgOutputStream;
			}
		}
		else
		{
			size_t		lineNo = SIZE_MAX;
			uint16_t	fileID = 0;
			LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
			LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Unbalanced 'stop recording output' instruction. Original variable name given was '%s', name given for stop was '%s'.", sOutputRecordingStack.back().mOutputVariableName.c_str(), theString );
		}
	}
	else
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Found 'stop recording output' instruction for variable '%s', but 'start recording output' was never called.", theString );
	}
	
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );
	
	inContext->currentInstruction++;
}


LEOINSTR_START(Msg,LEO_NUMBER_OF_MSG_INSTRUCTIONS)
LEOINSTR(LEOPrintInstruction)
LEOINSTR(LEODeleteInstruction)
LEOINSTR(LEOStartRecordingOutputInstruction)
LEOINSTR_LAST(LEOStopRecordingOutputInstruction)

