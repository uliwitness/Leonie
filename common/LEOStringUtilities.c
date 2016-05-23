//
//  LEOStringUtilities.c
//  Leonie
//
//  Created by Uli Kusterer on 28/04/16.
//
//

#include "LEOStringUtilities.h"
#include <stdlib.h>
#include <string.h>


const char*	LEOStringEscapedForPrintingInQuotes( const char* inStr )
{
	static char*	sTempBuf = NULL;
	static size_t	sTempBufLen = 0;
	size_t			y = 0, len = strlen(inStr);
	
	if( sTempBuf == NULL )
	{
		sTempBuf = calloc(len +1,1);
		if( !sTempBuf )
			return NULL;
		sTempBufLen = len +1;
	}
	else if( inStr[0] == 0 )
		sTempBuf[0] = 0;
	
	for( size_t x = 0; x < len; x++ )
	{
		// Make sure we have enough room to hold the needed characters
		//	(at worst +2, which is what we'd need to escape this character),
		//	but only reallocate in chunks of kBlockSize bytes for efficiency.
		if( (y +3) >= sTempBufLen )
		{
			size_t			neededBytes = y +3;
			const size_t	kBlockSize = 16;
			neededBytes = neededBytes + ((kBlockSize -(neededBytes % kBlockSize)) % kBlockSize);
			char*	largerPtr = realloc( sTempBuf, neededBytes );
			if( largerPtr )
			{
				sTempBuf = largerPtr;
				sTempBufLen = neededBytes;
			}
			else
				return NULL;	// Not enough room to escape this string, indicate failure.
		}
		
		if( inStr[x] == '\r' )
		{
			sTempBuf[y++] = '\\';
			sTempBuf[y++] = 'r';
			sTempBuf[y] = 0;
		}
		else if( inStr[x] == '\n' )
		{
			sTempBuf[y++] = '\\';
			sTempBuf[y++] = 'n';
			sTempBuf[y] = 0;
		}
		else if( inStr[x] == '"' )
		{
			sTempBuf[y++] = '\\';
			sTempBuf[y++] = '"';
			sTempBuf[y] = 0;
		}
		else if( inStr[x] == '\\' )
		{
			sTempBuf[y++] = '\\';
			sTempBuf[y++] = '\\';
			sTempBuf[y] = 0;
		}
		else
		{
			sTempBuf[y++] = inStr[x];
			sTempBuf[y] = 0;
		}
	}
	
	return sTempBuf;
}