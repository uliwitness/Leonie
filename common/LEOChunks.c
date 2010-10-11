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


// Gives us both the actual range of a chunk, and the range that should be deleted
//	when deleting a chunk, since for items or lines, there may be an extra delimiter
//	that needs to be deleted to completely get rid of a line, and not just set it
//	to empty.

void	LEOGetChunkRanges( const char* inStr, LEOChunkType inType,
							size_t inRangeStart, size_t inRangeEnd,
							size_t *outChunkStart, size_t *outChunkEnd,
							size_t *outDelChunkStart, size_t *outDelChunkEnd,
							char itemDelimiter )
{
	size_t		theLen = strlen(inStr);
	
	if( inType == kLEOChunkTypeCharacter )
	{
		if( theLen < inRangeEnd )
			inRangeEnd = theLen;
		if( inRangeStart < 0 )
			inRangeStart = 0;
		if( inRangeEnd < inRangeStart )
			inRangeEnd = inRangeStart;
		
		*outChunkStart = inRangeStart;
		*outChunkEnd = inRangeEnd;
		*outDelChunkStart = inRangeStart;
		*outDelChunkEnd = inRangeEnd;
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
		
		for( size_t x = 0; x < theLen; x++ )
		{
			bool	foundDelimiter = false;
			if( inType == kLEOChunkTypeItem )
				foundDelimiter = (inStr[x] == itemDelimiter);
			else if( inType == kLEOChunkTypeLine )
				foundDelimiter = (inStr[x] == '\n' || inStr[x] == '\r');
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
		bool		isInWord = !(inStr[0] == ' ' || inStr[0] == '\t' || inStr[0] == '\r' || inStr[0] == '\n');
		
		*outChunkStart = 0;
		*outDelChunkStart = 0;
		
		for( size_t x = 0; x < theLen; x++ )
		{
			bool		isWhitespace = (inStr[x] == ' ' || inStr[x] == '\t' || inStr[x] == '\r' || inStr[x] == '\n');
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
		}
		
		if( isInWord && wordNum == inRangeEnd )
		{
			*outChunkEnd = theLen;
			*outDelChunkEnd = theLen;
		}
	}
}
