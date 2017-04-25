/*
 *  LEOFileInstructionsGeneric.c
 *  Leonie
 *
 *  Created by Uli Kusterer on 09.10.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

/*!
	@header LEOFileInstructionsGeneric
	Generic implementation of file I/O instructions. Should work on all ANSI C
	systems.
*/

#include <stdio.h>
#include "LEOFileInstructionsGeneric.h"
#include "LEOInterpreter.h"
#include "LEOScript.h"
#include <string.h>
#include <stdlib.h>

void	LEOWriteToFileInstruction( LEOContext* inContext );
void	LEOReadFromFileInstruction( LEOContext* inContext );



size_t					kFirstFileInstruction = 0;


struct THostCommandEntry	gFileCommands[] =
{
	{
		EWriteIdentifier, WRITE_TO_FILE_INSTR, BACK_OF_STACK, 0, '\0', 'X',
		{
			{ EHostParamExpression, ELastIdentifier_Sentinel, EHostParameterRequired, INVALID_INSTR2, 0, 0, '\0', '0' },
			{ EHostParamInvisibleIdentifier, EToIdentifier, EHostParameterRequired, INVALID_INSTR2, 0, 0, '\0', '\0' },
			{ EHostParamInvisibleIdentifier, EFileIdentifier, EHostParameterRequired, INVALID_INSTR2, 0, 0, '\0', '\0' },
			{ EHostParamExpression, ELastIdentifier_Sentinel, EHostParameterRequired, INVALID_INSTR2, 0, 0, '\0', 'X' },
			{ EHostParam_Sentinel, ELastIdentifier_Sentinel, EHostParameterOptional, INVALID_INSTR2, 0, 0, '\0', '\0' }
		}
	},
	{
		EReadIdentifier, READ_FROM_FILE_INSTR, BACK_OF_STACK, 0, '\0', 'X',
		{
			{ EHostParamInvisibleIdentifier, EFromIdentifier, EHostParameterRequired, INVALID_INSTR2, 0, 0, '\0', '\0' },
			{ EHostParamInvisibleIdentifier, EFileIdentifier, EHostParameterRequired, INVALID_INSTR2, 0, 0, '\0', '\0' },
			{ EHostParamExpression, ELastIdentifier_Sentinel, EHostParameterRequired, INVALID_INSTR2, 0, 0, '\0', 'X' },
			{ EHostParam_Sentinel, ELastIdentifier_Sentinel, EHostParameterOptional, INVALID_INSTR2, 0, 0, '\0', '\0' }
		}
	},
	{
		ELastIdentifier_Sentinel, INVALID_INSTR2, 0, 0, '\0', '\0',
		{
			{ EHostParam_Sentinel, ELastIdentifier_Sentinel, EHostParameterOptional, INVALID_INSTR2, 0, 0, '\0', '\0' }
		}
	}
};


/*!
	Pop a file path and string off the stack and write the string to the
	file at the given path.
	(WRITE_TO_FILE_INSTR)
*/

void	LEOWriteToFileInstruction( LEOContext* inContext )
{
	char			dataBuf[1024] = { 0 };
	char			filePathBuf[1024] = { 0 };
	union LEOValue*	theDataValue = inContext->stackEndPtr -2;
	union LEOValue*	theFileValue = inContext->stackEndPtr -1;
	const char* strToWrite = LEOGetValueAsString( theDataValue, dataBuf, sizeof(dataBuf), inContext );
	const char* filePath = LEOGetValueAsString( theFileValue, filePathBuf, sizeof(filePathBuf), inContext );
	FILE * theFile = fopen( filePath, "w" );
	if( theFile )
	{
		size_t itemsToWrite = strlen(strToWrite);
		size_t itemsWritten = fwrite( strToWrite, 1, itemsToWrite, theFile );
		fclose( theFile );
		
		if( itemsWritten != itemsToWrite )
		{
			LEOContextSetLocalVariable( inContext, "result", "%zu bytes could not be written.", (itemsToWrite -itemsWritten) );
		}
	}
	else
	{
		LEOContextSetLocalVariable( inContext, "result", "Couldn't open file for writing." );
	}
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -2 );
	
	inContext->currentInstruction++;
}


void	LEOReadFromFileInstruction( LEOContext* inContext )
{
	char			filePathBuf[1024] = { 0 };
	union LEOValue*	theFileValue = inContext->stackEndPtr -1;
	const char* filePath = LEOGetValueAsString( theFileValue, filePathBuf, sizeof(filePathBuf), inContext );
	FILE * theFile = fopen( filePath, "r" );
	if( theFile )
	{
		fseek( theFile, 0, SEEK_END);
		size_t fileLength = ftell( theFile );
		fseek( theFile, 0, SEEK_SET);
		
		char*	fileBuf = malloc(fileLength);
		
		size_t itemsRead = fread( fileBuf, 1, fileLength, theFile );
		fclose( theFile );
		
		if( itemsRead != fileLength )
		{
			LEOContextSetLocalVariable( inContext, "result", "%zu bytes could not be read.", (fileLength -itemsRead) );
		}

		struct LEOHandler	*	theHandler = LEOContextPeekCurrentHandler( inContext );
		long					bpRelativeOffset = LEOHandlerFindVariableByName( theHandler, "it" );
		if( bpRelativeOffset >= 0 )
		{
			LEOSetValueAsString( inContext->stackBasePtr +bpRelativeOffset, fileBuf, fileLength, inContext );
		}
		
		free(fileBuf);
	}
	else
	{
		LEOContextSetLocalVariable( inContext, "result", "Couldn't open file for writing." );
	}
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );
	
	inContext->currentInstruction++;
}


LEOINSTR_START(File,LEO_NUMBER_OF_FILE_INSTRUCTIONS)
LEOINSTR(LEOWriteToFileInstruction)
LEOINSTR_LAST(LEOReadFromFileInstruction)

