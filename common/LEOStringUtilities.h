//
//  LEOStringUtilities.h
//  Leonie
//
//  Created by Uli Kusterer on 28/04/16.
//
//

#ifndef LEOStringUtilities_h
#define LEOStringUtilities_h

#include <stdio.h>

extern const char*	LEOStringEscapedForPrintingInQuotes( const char* inStr );	// Returns an internal buffer, not suitable for concurrent use, and you are NOT supposed to free it.

#endif /* LEOStringUtilities_h */
