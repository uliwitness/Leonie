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


#include "fake_filesystem.hpp"	// Until std::filesystem arrives on Xcode's clang.
using namespace fake;


void	LEOWriteToFileInstruction( LEOContext* inContext );
void	LEOReadFromFileInstruction( LEOContext* inContext );
void	LEOCopyFileInstruction( LEOContext* inContext );
void	LEOListFilesInstruction( LEOContext* inContext );



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
		ECopyIdentifier, LEO_COPY_FILE_INSTR, BACK_OF_STACK, 0, '\0', 'X',
		{
			{ EHostParamInvisibleIdentifier, EFileIdentifier, EHostParameterRequired, INVALID_INSTR2, 0, 0, '\0', '\0' },
			{ EHostParamExpression, ELastIdentifier_Sentinel, EHostParameterRequired, INVALID_INSTR2, 0, 0, '\0', 'X' },
			{ EHostParamInvisibleIdentifier, EToIdentifier, EHostParameterRequired, INVALID_INSTR2, 0, 0, '\0', '\0' },
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


struct THostCommandEntry		gFileHostFunctions[] =
{
	{
		EFilesIdentifier, LEO_LIST_FILES_INSTR, BACK_OF_STACK, 0, '\0', 'X',
		{
			{ EHostParamInvisibleIdentifier, EInIdentifier, EHostParameterRequired, INVALID_INSTR2, 0, 0, '\0', '\0' },
			{ EHostParamInvisibleIdentifier, EFolderIdentifier, EHostParameterRequired, INVALID_INSTR2, 0, 0, '\0', '\0' },
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
		
		char*	fileBuf = (char*) malloc(fileLength);
		
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


/*!
	Pop two strings off the stack, the source path and destination path,
	and copy the given file to the given destination.
	(LEO_COPY_FILE_INSTR)
 */

void	LEOCopyFileInstruction( LEOContext* inContext )
{
	char			dataBuf[1024] = { 0 };
	union LEOValue*	theSrcPathValue = inContext->stackEndPtr -2;
	const char* srcPath = LEOGetValueAsString( theSrcPathValue, dataBuf, sizeof(dataBuf), inContext );
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;
	
	char			dataBuf2[1024] = { 0 };
	union LEOValue*	theDstPathValue = inContext->stackEndPtr -1;
	const char* dstPath = LEOGetValueAsString( theDstPathValue, dataBuf2, sizeof(dataBuf2), inContext );
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;
	
	FILE * srcFile = fopen( srcPath, "r" );
	if( !srcFile )
	{
		size_t		lineNo = SIZE_T_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_T_MAX, fileID, "Can't open source file \"%s\" for reading.", srcPath );
		return;
	}
	
	filesystem::create_directories( filesystem::path(dstPath).parent_path() );
	
	FILE * dstFile = fopen( dstPath, "w" );
	if( !dstFile )
	{
		size_t		lineNo = SIZE_T_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_T_MAX, fileID, "Couldn't create file at \"%s\".", dstPath );
		return;
	}
	
	char fileBuf[1024] = {};
	size_t numBytesRead = 0, numBytesToRead = 0;
	fseek( srcFile, 0, SEEK_END );
	numBytesToRead = ftell( srcFile );
	fseek( srcFile, 0, SEEK_SET );
	while( numBytesRead < numBytesToRead )
	{
		size_t currBytesToRead = sizeof(fileBuf);
		size_t bytesLeft = numBytesToRead -numBytesRead;
		if( currBytesToRead > bytesLeft )
			currBytesToRead = bytesLeft;
		size_t currBytesRead = fread( fileBuf, 1, currBytesToRead, srcFile );
		if( currBytesRead != currBytesToRead )
		{
			size_t		lineNo = SIZE_T_MAX;
			uint16_t	fileID = 0;
			LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
			LEOContextStopWithError( inContext, lineNo, SIZE_T_MAX, fileID, "Unable to read the remaining %zu bytes from file \"%s\".", bytesLeft, srcPath );
			return;
		}
		size_t currBytesWritten = fwrite( fileBuf, 1, currBytesRead, dstFile );
		if( currBytesWritten != currBytesRead )
		{
			size_t		lineNo = SIZE_T_MAX;
			uint16_t	fileID = 0;
			LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
			LEOContextStopWithError( inContext, lineNo, SIZE_T_MAX, fileID, "Unable to write the remaining %zu bytes to file \"%s\".", bytesLeft, dstPath );
			return;
		}
		numBytesRead += currBytesRead;
	}
	
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -2 );

	inContext->currentInstruction++;
}


/*
	(LEO_LIST_FILES_INSTR)
 */


void	LEOListFilesInstruction( LEOContext* inContext )
{
	char			filePathBuf[1024] = { 0 };
	union LEOValue*	theFileValue = inContext->stackEndPtr -1;
	const char*		filePath = LEOGetValueAsString( theFileValue, filePathBuf, sizeof(filePathBuf), inContext );
	
	filesystem::directory_iterator	currFile(filePath);
	
	LEOCleanUpValue(theFileValue, kLEOInvalidateReferences, inContext);
	LEOValueArray * theArrayValue = (LEOValueArray*)theFileValue;
	LEOInitArrayValue( theArrayValue, NULL, kLEOInvalidateReferences, inContext );
	
	char	keyStr[100] = {};
	size_t	x = 0;
	for( ; currFile != filesystem::directory_iterator(); ++currFile )
	{
		filesystem::path	fpath( (*currFile).path() );
		std::string			fname( fpath.filename().string() );
		if( fname == "." || fname == ".." )
			continue;
		
		snprintf( keyStr, sizeof(keyStr) -1, "%zu", ++x );
		
		LEOAddStringArrayEntryToRoot( &theArrayValue->array, keyStr, fname.data(), fname.size(), inContext );
	}
	
	inContext->currentInstruction++;
}


LEOINSTR_START(File,LEO_NUMBER_OF_FILE_INSTRUCTIONS)
LEOINSTR(LEOWriteToFileInstruction)
LEOINSTR(LEOReadFromFileInstruction)
LEOINSTR(LEOCopyFileInstruction)
LEOINSTR_LAST(LEOListFilesInstruction)

