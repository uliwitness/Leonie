/*
 *  LEOChunks.c
 *  Leonie
 *
 *  Created by Uli Kusterer on 20.09.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include "LEOChunks.h"


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
	if( inType == kLEOChunkTypeCharacter )
	{
		*outChunkStart = inRangeStart;
		*outChunkEnd = inRangeEnd;
		*outDelChunkStart = inRangeStart;
		*outDelChunkEnd = inRangeEnd;
	}
	else if( inType == kLEOChunkTypeItem )
	{
		size_t		itemNum = 0;
		size_t		theLen = strlen(inStr);
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
			if( inStr[x] == itemDelimiter )
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
}