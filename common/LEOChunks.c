/*
 *  LEOChunks.c
 *  Leonie
 *
 *  Created by Uli Kusterer on 20.09.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include "LEOChunks.h"
#include <string.h>
#include <stdbool.h>


const char*	gLEOChunkTypeNames[kLEOChunkType_Last +1] =
{
	"*INVALID*",
	"byte",
	"character",
	"item",
	"line",
	"word",
	NULL
};


size_t		LEOGetLengthOfUTF8SequenceStartingWith( unsigned char inChar );
uint32_t	LEOUTF8StringParseUTF32CharacterAtOffset( const char *utf8, size_t len, size_t *ioOffset );


size_t	LEOGetLengthOfUTF8SequenceStartingWith( unsigned char inChar )
{
	size_t		outLength = 0;
	inChar >>= 3;
	
	if( inChar == 0x1e )
		outLength = 4;
	else
	{
		inChar >>= 1;
		if( inChar == 0x0e )
		{
			outLength = 3;
		}
		else
		{
			inChar >>= 1;
			if( inChar == 0x06 )
				outLength = 2;
			else
				outLength = 1;
		}
	}
	return outLength;
}



uint32_t	LEOUTF8StringParseUTF32CharacterAtOffset( const char *utf8, size_t len, size_t *ioOffset )
{
	const uint8_t		*currUTF8Byte = (const uint8_t*) utf8 + (*ioOffset);
	uint32_t			utf32Char = 0L;
	size_t				numBytesInSequence = 0;
	numBytesInSequence = LEOGetLengthOfUTF8SequenceStartingWith( *currUTF8Byte );

	switch( numBytesInSequence )
	{
		case 4:
			utf32Char = ((*currUTF8Byte) ^ 0xf0);
			break;
			
		case 3:
			utf32Char = ((*currUTF8Byte) ^ 0xe0);
			break;
			
		case 2:
			utf32Char = ((*currUTF8Byte) ^ 0xc0);
			break;
			
		case 1:
			utf32Char = (*currUTF8Byte);
			break;
	}
	
	currUTF8Byte ++;
	
	for( size_t y = numBytesInSequence; y > 1; y-- )
	{
		utf32Char <<= 6;
		utf32Char |= ((*currUTF8Byte) ^ 0x80);
		currUTF8Byte ++;;
	}
	
	(*ioOffset) += numBytesInSequence;
	
	return utf32Char;
}


// Gives us both the actual range of a chunk, and the range that should be deleted
//	when deleting a chunk, since for items or lines, there may be an extra delimiter
//	that needs to be deleted to completely get rid of a line, and not just set it
//	to empty.

void	LEOGetChunkRanges( const char* inStr, LEOChunkType inType,
							size_t inRangeStart, size_t inRangeEnd,
							size_t *outChunkStart, size_t *outChunkEnd,
							size_t *outDelChunkStart, size_t *outDelChunkEnd,
							uint32_t itemDelimiter )
{
	size_t		theLen = strlen(inStr);
	
	if( inType == kLEOChunkTypeByte )
	{
		if( inRangeEnd > theLen )
			inRangeEnd = theLen;
		
		*outChunkStart = inRangeStart;
		*outDelChunkStart = inRangeStart;
		*outChunkEnd = inRangeEnd;
		*outDelChunkEnd = inRangeEnd;
	}
	else if( inType == kLEOChunkTypeCharacter )
	{
		bool		didFindEnd = false,
					didFindStart = false;
		*outChunkStart = 0;
		*outChunkEnd = 0;
		*outDelChunkStart = theLen;
		*outDelChunkEnd = theLen;
		
		size_t	currOffset = 0;
		size_t	currChar = 0;
		while( currOffset < theLen )
		{
			if( currChar == inRangeStart )
			{
				*outChunkStart = currOffset;
				*outDelChunkStart = currOffset;
				didFindStart = true;
			}
			
			/*uint32_t*/ LEOUTF8StringParseUTF32CharacterAtOffset( inStr, theLen, &currOffset );

			if( currChar == inRangeEnd )
			{
				*outChunkEnd = currOffset;
				*outDelChunkEnd = currOffset;
				didFindEnd = true;
			}
			
			currChar ++;
		}
		
		if( currChar == inRangeStart || !didFindStart )
		{
			*outChunkStart = currOffset;
			*outDelChunkStart = currOffset;
		}
		
		if( currChar == inRangeEnd || !didFindEnd )
		{
			*outChunkEnd = currOffset;
			*outDelChunkEnd = currOffset;
		}
	}
	else if( inType == kLEOChunkTypeItem || inType == kLEOChunkTypeLine )
	{
		size_t		itemNum = 0;
		size_t		currChunkStart = 0,
					currChunkEnd = 0,
					currDelChunkStart = 0,
					currDelChunkEnd = 0;
		
		*outChunkStart = 0;
		*outDelChunkStart = 0;
		*outChunkEnd = 0;
		*outDelChunkEnd = 0;
		
		size_t x = 0;
		for( ; x < theLen; )
		{
			size_t		newX = x;
			uint32_t	currCh = LEOUTF8StringParseUTF32CharacterAtOffset( inStr, theLen, &newX );
			bool		foundDelimiter = false;
			if( inType == kLEOChunkTypeItem )
				foundDelimiter = (currCh == itemDelimiter);
			else if( inType == kLEOChunkTypeLine )
				foundDelimiter = (currCh == '\n' || currCh == '\r');
			if( foundDelimiter )
			{
				currChunkEnd = x;
				currDelChunkEnd = x+1;
				
				if( itemNum == inRangeStart )
				{
					*outChunkStart = currChunkStart;
					*outDelChunkStart = currDelChunkStart;
				}
				currDelChunkStart = currChunkEnd;
				currChunkStart = currDelChunkEnd;
				if( itemNum == inRangeEnd )
				{
					*outChunkEnd = currChunkEnd;
					*outDelChunkEnd = (inRangeStart == 0) ? currDelChunkEnd : currChunkEnd;
					return;
				}
				itemNum++;
			}
			else
			{
				currChunkEnd = x;
				currDelChunkEnd = x;
			}
			
			x = newX;
		}
		
		// Last item is start or end of chunk?
		if( itemNum == inRangeStart )
		{
			*outChunkStart = currChunkStart;
			*outDelChunkStart = currDelChunkStart;
		}
		if( itemNum == inRangeEnd )
		{
			*outChunkEnd = theLen;
			*outDelChunkEnd = theLen;
		}
	}
	else if( inType == kLEOChunkTypeWord )
	{
		size_t		wordNum = 0;
		bool		isInWord = true;	// Ignored, initialized when we know what 1st char is.
		
		*outChunkStart = 0;
		*outDelChunkStart = 0;
		
		size_t x = 0;
		for( ; x < theLen; )
		{
			size_t		newX = x;
			uint32_t	currCh = LEOUTF8StringParseUTF32CharacterAtOffset( inStr, theLen, &newX );
			bool		isWhitespace = (currCh == ' ' || currCh == '\t' || currCh == '\r' || currCh == '\n');
			if( x == 0 )
				isInWord = !isWhitespace;
			if( isWhitespace && isInWord )
			{
				isInWord = false;
				if( wordNum == inRangeEnd )
				{
					*outChunkEnd = x;
					*outDelChunkEnd = x;
					break;
				}
				wordNum++;
			}
			else if( !isWhitespace && !isInWord )
			{
				isInWord = true;
				if( wordNum == inRangeStart )
				{
					*outChunkStart = x;
					*outDelChunkStart = x;
				}
			}
			
			x = newX;
		}
		
		if( isInWord && wordNum == inRangeEnd )
		{
			*outChunkEnd = theLen;
			*outDelChunkEnd = theLen;
		}
	}
}


void	LEODoForEachChunk( const char* inStr, size_t inBufSize, LEOChunkType inType,
							bool (*inChunkCallback)( const char* currStr, size_t currLen, size_t currStart, size_t currEnd, void* userData ),
							uint32_t itemDelimiter, void* userData )
{
	size_t		theLen = inBufSize;
	size_t		foundChunkStart = 0;
	size_t		foundChunkEnd = 0;
	
	if( inType == kLEOChunkTypeByte )
	{
		size_t		currIdx = 0;
		
		while( currIdx < theLen )
		{
			if( !inChunkCallback( inStr +currIdx, 1, currIdx, currIdx, userData ) )
				return;
			++currIdx;
		}
	}
	else if( inType == kLEOChunkTypeCharacter )
	{
		size_t		currOffset = 0;
		size_t		currChar = 0;
		
		while( currOffset < theLen )
		{
			size_t		startOffset = currOffset;
			
			/*uint32_t*/ LEOUTF8StringParseUTF32CharacterAtOffset( inStr, theLen, &currOffset );
			
			++currChar;
			if( !inChunkCallback( inStr +startOffset, currOffset -startOffset, currChar, currChar, userData ) )
				return;
		}
	}
	else if( inType == kLEOChunkTypeItem || inType == kLEOChunkTypeLine )
	{
		size_t		currOffset = 0;
		size_t		startOffset = currOffset;
		uint32_t	currCh = 0;
		bool		foundDelimiter = false;
		
		while( currOffset < theLen )
		{
			size_t		prevOffset = currOffset;
			foundDelimiter = false;
			currCh = LEOUTF8StringParseUTF32CharacterAtOffset( inStr, theLen, &currOffset );
			
			if( inType == kLEOChunkTypeItem )
				foundDelimiter = (currCh == itemDelimiter);
			else if( inType == kLEOChunkTypeLine )
				foundDelimiter = (currCh == '\n' || currCh == '\r');
			
			if( foundDelimiter )
			{
				if( !inChunkCallback( inStr +startOffset, prevOffset -startOffset, startOffset, prevOffset, userData ) )
					return;
				
				startOffset = currOffset;
			}
		}
		
		// There's always a last item that we haven't reported yet, though it can be empty:
		inChunkCallback( inStr +startOffset, currOffset -startOffset, startOffset, currOffset, userData );
	}
	else if( inType == kLEOChunkTypeWord )
	{
		size_t		wordNum = 0;
		bool		isInWord = true;	// Ignored, initialized when we know what 1st char is.
		
		foundChunkStart = 0;
		
		size_t x = 0;
		for( ; x < theLen; )
		{
			size_t		newX = x;
			uint32_t	currCh = LEOUTF8StringParseUTF32CharacterAtOffset( inStr, theLen, &newX );
			bool		isWhitespace = (currCh == ' ' || currCh == '\t' || currCh == '\r' || currCh == '\n');
			if( x == 0 )
				isInWord = !isWhitespace;
			if( isWhitespace && isInWord )
			{
				isInWord = false;
				foundChunkEnd = x;
				if( !inChunkCallback( inStr +foundChunkStart, foundChunkEnd -foundChunkStart, foundChunkStart, foundChunkEnd, userData ) )
					return;
				wordNum++;
			}
			else if( !isWhitespace && !isInWord )
			{
				isInWord = true;
				foundChunkStart = x;
			}
			
			x = newX;
		}
		
		if( isInWord )
		{
			foundChunkEnd = theLen;
			inChunkCallback( inStr +foundChunkStart, foundChunkEnd -foundChunkStart, foundChunkStart, foundChunkEnd, userData );
		}
	}
}
