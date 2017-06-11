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
#include <string.h>
#include <stdlib.h>
#include <string>


void	LEOHTMLEncodedInstruction( LEOContext* inContext );



size_t					kFirstWebPageInstruction = 0;


struct TBuiltInFunctionEntry	gWebPageBuiltInFunctions[] =
{
	{ EHTMLEncodedIdentifier, LEO_HTML_ENCODED_INSTR, 0, 0, 1 },
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


LEOINSTR_START(WebPage,LEO_NUMBER_OF_WEB_PAGE_INSTRUCTIONS)
LEOINSTR_LAST(LEOHTMLEncodedInstruction)

