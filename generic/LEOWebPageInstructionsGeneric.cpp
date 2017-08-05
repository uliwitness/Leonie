/*
 *  LEOWebPageInstructionsGeneric.cpp
 *  Leonie
 *
 *  Created by Uli Kusterer on 28.05.17.
 *  Copyright 2017 Uli Kusterer. All rights reserved.
 *
 */

/*!
	@header LEOWebPageInstructionsGeneric
	Generic implementation of web page generator mode instructions. Should work on all ANSI C++
	systems.
*/

#include <stdio.h>
#include "LEOWebPageInstructionsGeneric.h"
#include "LEOInterpreter.h"
#include "LEOScript.h"
#include "UTF8UTF32Utilities.h"
#include <string.h>
#include <stdlib.h>
#include <string>


void	LEOHTMLEncodedInstruction( LEOContext* inContext );



size_t					kFirstWebPageInstruction = 0;


struct TBuiltInFunctionEntry	gWebPageBuiltInFunctions[] =
{
	{ EHTMLEncodedIdentifier, LEO_HTML_ENCODED_INSTR, 0, 0, 1 },
	{ EOffsetIdentifier, LEO_OFFSET_FUNC_INSTR, 0, 0, 2 },
	{ ELastIdentifier_Sentinel, INVALID_INSTR2, 0, 0, 0 }
};


/*!
	Pop a string off the stack and return a version with all HTML-unsafe characters
	escaped as entities. I.e. replace < and > and & with &lt; and &gt; and &amp;.
	(LEO_HTML_ENCODED_INSTR)
*/

void	LEOHTMLEncodedInstruction( LEOContext* inContext )
{
	char			dataBuf[1024] = { 0 };
	union LEOValue*	theDataValue = inContext->stackEndPtr -1;
	const char* strToEncode = LEOGetValueAsString( theDataValue, dataBuf, sizeof(dataBuf), inContext );
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;
	
	size_t strToEncodeLen = strlen(strToEncode);
	std::string	escapedStr;
	escapedStr.reserve(strToEncodeLen);
	for( size_t x = 0; x < strToEncodeLen; ++x )
	{
		switch( strToEncode[x] )
		{
			case '<': escapedStr.append("&lt;"); break;
			case '>': escapedStr.append("&gt;"); break;
			case '&': escapedStr.append("&amp;"); break;
			case '"': escapedStr.append("&quot;"); break;
			case '\r': case '\n': escapedStr.append("<br />"); break;
			default: escapedStr.append( 1, strToEncode[x] );
		}
	}
	
	LEOCleanUpValue( inContext->stackEndPtr -1, kLEOInvalidateReferences, inContext );
	LEOInitStringValue( inContext->stackEndPtr -1, escapedStr.c_str(), escapedStr.length(), kLEOInvalidateReferences, inContext );
	
	inContext->currentInstruction++;
}


void	LEOOffsetFunctionInstruction( LEOContext* inContext )
{
	/*
		TODO: Naïve implementation, doesn't account for o¨ being equal to ö
				and similar Unicode gotchas. Only works correctly on
				normalized text.
	 */
	
	char			needleBuf[1024] = { 0 };
	union LEOValue*	theNeedleValue = inContext->stackEndPtr -2;
	const char	*	needleStr = LEOGetValueAsString( theNeedleValue, needleBuf, sizeof(needleBuf), inContext );
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;

	char			haystackBuf[1024] = { 0 };
	union LEOValue*	theHaystackValue = inContext->stackEndPtr -1;
	const char	*	haystackStr = LEOGetValueAsString( theHaystackValue, haystackBuf, sizeof(haystackBuf), inContext );
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;
	
	LEOInteger	offset = -1;
	
	size_t haystackLen = strlen(haystackStr);
	size_t needleLen = strlen(needleStr);
	
	if( haystackLen > needleLen && haystackLen > 0 && needleLen > 0 )
	{
		size_t currNeedleOffset = 0;
		uint32_t firstNeedleChar = UTF8StringParseUTF32CharacterAtOffset( needleStr, needleLen, &currNeedleOffset );
		
		size_t currHaystackOffset = 0;
		while( currHaystackOffset < haystackLen )
		{
			offset = currHaystackOffset;
			uint32_t currHaystackChar = UTF8StringParseUTF32CharacterAtOffset( haystackStr, haystackLen, &currHaystackOffset );
			if( firstNeedleChar != currHaystackChar )
			{
				offset = -1;
				continue;
			}
			size_t potentialNeedleOffset = currNeedleOffset;
			size_t potentialHaystackOffset = currHaystackOffset;
			
			while( potentialHaystackOffset < haystackLen && potentialNeedleOffset < needleLen )
			{
				uint32_t potentialNeedleChar = UTF8StringParseUTF32CharacterAtOffset( needleStr, needleLen, &potentialNeedleOffset );
				uint32_t potentialHaystackChar = UTF8StringParseUTF32CharacterAtOffset( haystackStr, haystackLen, &potentialHaystackOffset );
				if( potentialNeedleChar != potentialHaystackChar )
				{
					offset = -1;
					break;
				}
			}
			
			if( potentialNeedleOffset == needleLen )	// A full match?
			{
				break;
			}
			else
			{
				offset = -1;
			}
		}
	}
	
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );
	LEOCleanUpValue( inContext->stackEndPtr -1, kLEOInvalidateReferences, inContext );
	LEOInitIntegerValue( inContext->stackEndPtr -1, offset, kLEOUnitNone, kLEOInvalidateReferences, inContext );
	
	inContext->currentInstruction++;
}

LEOINSTR_START(WebPage,LEO_NUMBER_OF_WEB_PAGE_INSTRUCTIONS)
LEOINSTR(LEOHTMLEncodedInstruction)
LEOINSTR_LAST(LEOOffsetFunctionInstruction)

