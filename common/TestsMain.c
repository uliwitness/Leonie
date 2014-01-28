/*
 *  TestsMain.c
 *  Leonie
 *
 *  Created by Uli Kusterer on 09.10.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include "LEOInterpreter.h"
#include "LEOChunks.h"
#include "LEOContextGroup.h"
#include "LEOScript.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>


bool		gAnyTestFailed = false;


#define ASSERT(expr)	({ if( !(expr) ) { printf( "error: Test failed: %s\n", #expr ); gAnyTestFailed = true; } else printf( "note: Test passed: %s\n", #expr ); })
#define ASSERT_STRING_MATCH(a,b)	({ if( strcmp( (a), (b) ) != 0 ) { printf( "error: Test failed: \"%s\" != \"%s\"\n", (a), (b) ); gAnyTestFailed = true; } else printf( "note: Test passed: \"%s\" == \"%s\"\n", (a), (b) ); })


void	ASSERT_RANGE_MATCHES_STRING( const char* theStr, size_t chunkStart, size_t chunkEnd, const char* matchStr )
{
	char		substr[1024] = {0};
	size_t		currDest = 0;
	for( size_t x = chunkStart; x < chunkEnd; x++ )
	{
		substr[currDest++] = theStr[x];
	}
	substr[currDest] = 0;
	
	if( strcmp(substr, matchStr) == 0 )
		printf( "note: Range %lu to %lu of \"%s\" matches \"%s\"\n", chunkStart, chunkEnd, theStr, matchStr );
	else
		printf( "error: Test failed: Range %lu to %lu of \"%s\" doesn't match \"%s\"\n", chunkStart, chunkEnd, theStr, matchStr );
}



void	DoScriptTest( void )
{
	LEOContextGroup	*	group = LEOContextGroupCreate();
	LEOScript		*	theScript = LEOScriptCreateForOwner( 0, 0, NULL );
	LEOHandler		*	newHandler = NULL;
	LEOHandler		*	foundHandler = NULL;
	
	printf( "\nnote: Script generation tests\n" );
	
	// Add 1st handler:
	LEOHandlerID	mouseDownID = LEOContextGroupHandlerIDForHandlerName( group, "mouseDown" );
	newHandler = LEOScriptAddCommandHandlerWithID( theScript, mouseDownID );
	ASSERT( theScript->numCommands == 1 );
	ASSERT( newHandler != NULL );
	ASSERT( newHandler->handlerName == mouseDownID );
	
	foundHandler = LEOScriptFindCommandHandlerWithID( theScript, mouseDownID );
	ASSERT( foundHandler != NULL );
	ASSERT( foundHandler->handlerName == mouseDownID );

	// Add 2nd handler:
	LEOHandlerID	mouseUpID = LEOContextGroupHandlerIDForHandlerName( group, "mouseUp" );
	newHandler = LEOScriptAddCommandHandlerWithID( theScript, mouseUpID );
	ASSERT( theScript->numCommands == 2 );
	ASSERT( newHandler != NULL );
	ASSERT( newHandler->handlerName == mouseUpID );
	
	LEOHandlerID	mousedownID = LEOContextGroupHandlerIDForHandlerName( group, "mousedown" );
	foundHandler = LEOScriptFindCommandHandlerWithID( theScript, mousedownID );
	ASSERT( foundHandler != NULL );
	ASSERT( foundHandler->handlerName == mousedownID );
	ASSERT( mouseDownID == mousedownID );

	LEOHandlerID	MOUSEUPID = LEOContextGroupHandlerIDForHandlerName( group, "MOUSEUP" );
	foundHandler = LEOScriptFindCommandHandlerWithID( theScript, MOUSEUPID );
	ASSERT( foundHandler != NULL );
	ASSERT( foundHandler->handlerName == MOUSEUPID );
	ASSERT( mouseUpID == MOUSEUPID );

	// Look for a handler that doesn't exist:
	LEOHandlerID	nonexistentHandlerID = LEOContextGroupHandlerIDForHandlerName( group, "nonexistentHandler" );
	foundHandler = LEOScriptFindCommandHandlerWithID( theScript, nonexistentHandlerID );
	ASSERT( foundHandler == NULL );

	ASSERT( mouseUpID != mouseDownID );
	ASSERT( mouseUpID != nonexistentHandlerID );
	ASSERT( mouseDownID != nonexistentHandlerID );
	
	LEOScriptRelease( theScript );
	LEOContextGroupRelease( group );
}


void LEOPrintStringWithRangeMarkers( const char* theStr, size_t chunkStart, size_t chunkEnd )
{
	size_t x = 0;
	printf( "note: %s\n", theStr );
	size_t pEndOffs = chunkStart;
	if( pEndOffs > 0 )
		pEndOffs--;
	printf( "note: " );
	for( x = 0; x < pEndOffs; x++ )
		printf(" ");
	printf("^");
	if( chunkEnd != chunkStart )
	{
		pEndOffs = chunkEnd;
		if( pEndOffs > 0 )
			pEndOffs--;
		for( ; x < pEndOffs; x++ )
			printf(" ");
		printf("^");
	}
	printf("\n");
	char		substr[1024] = {0};
	size_t		currDest = 0;
	for( size_t x = chunkStart; x < chunkEnd; x++ )
	{
		substr[currDest++] = theStr[x];
	}
	substr[currDest] = 0;
	printf("note: (%s)\n",substr);
}


void	DoChunkTests( void )
{
	const char*		theStr = "this,that,more";
	size_t			outChunkStart, outChunkEnd,
					outDelChunkStart, outDelChunkEnd;
	
	printf( "\nnote: Raw chunk tests\n" );
	
	LEOGetChunkRanges( theStr, kLEOChunkTypeItem,
					0, 0,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	ASSERT_RANGE_MATCHES_STRING( theStr, outChunkStart, outChunkEnd, "this" );
	ASSERT_RANGE_MATCHES_STRING( theStr, outDelChunkStart, outDelChunkEnd, "this," );

	LEOGetChunkRanges( theStr, kLEOChunkTypeItem,
					1, 1,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	ASSERT_RANGE_MATCHES_STRING( theStr, outChunkStart, outChunkEnd, "that" );
	ASSERT_RANGE_MATCHES_STRING( theStr, outDelChunkStart, outDelChunkEnd, ",that" );
	
	LEOGetChunkRanges( theStr, kLEOChunkTypeItem,
					2, 2,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	ASSERT_RANGE_MATCHES_STRING( theStr, outChunkStart, outChunkEnd, "more" );
	ASSERT_RANGE_MATCHES_STRING( theStr, outDelChunkStart, outDelChunkEnd, ",more" );

	LEOGetChunkRanges( theStr, kLEOChunkTypeItem,
					0, 1,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	ASSERT_RANGE_MATCHES_STRING( theStr, outChunkStart, outChunkEnd, "this,that" );
	ASSERT_RANGE_MATCHES_STRING( theStr, outDelChunkStart, outDelChunkEnd, "this,that," );

	LEOGetChunkRanges( theStr, kLEOChunkTypeItem,
					1, 2,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	ASSERT_RANGE_MATCHES_STRING( theStr, outChunkStart, outChunkEnd, "that,more" );
	ASSERT_RANGE_MATCHES_STRING( theStr, outDelChunkStart, outDelChunkEnd, ",that,more" );

	LEOGetChunkRanges( theStr, kLEOChunkTypeItem,
					0, 2,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	ASSERT_RANGE_MATCHES_STRING( theStr, outChunkStart, outChunkEnd, "this,that,more" );
	ASSERT_RANGE_MATCHES_STRING( theStr, outDelChunkStart, outDelChunkEnd, "this,that,more" );

	LEOGetChunkRanges( "daniel", kLEOChunkTypeItem,
					0, 0,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	ASSERT_RANGE_MATCHES_STRING( "daniel", outChunkStart, outChunkEnd, "daniel" );
	ASSERT_RANGE_MATCHES_STRING( "daniel", outDelChunkStart, outDelChunkEnd, "daniel" );

	LEOGetChunkRanges( "", kLEOChunkTypeItem,
					0, 0,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	ASSERT_RANGE_MATCHES_STRING( "", outChunkStart, outChunkEnd, "" );
	ASSERT_RANGE_MATCHES_STRING( "", outDelChunkStart, outDelChunkEnd, "" );

	LEOGetChunkRanges( ",,", kLEOChunkTypeItem,
					0, 0,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	ASSERT( outChunkStart == 0 );
	ASSERT( outChunkEnd == 0 );
	ASSERT( outDelChunkStart == 0 );
	ASSERT( outDelChunkEnd == 1 );
	ASSERT_RANGE_MATCHES_STRING( ",,", outChunkStart, outChunkEnd, "" );
	ASSERT_RANGE_MATCHES_STRING( ",,", outDelChunkStart, outDelChunkEnd, "," );

	LEOGetChunkRanges( ",,", kLEOChunkTypeItem,
					1, 1,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	ASSERT( outChunkStart == 1 );
	ASSERT( outChunkEnd == 1 );
	ASSERT( outDelChunkStart == 0 );
	ASSERT( outDelChunkEnd == 1 );
	ASSERT_RANGE_MATCHES_STRING( ",,", outChunkStart, outChunkEnd, "" );
	ASSERT_RANGE_MATCHES_STRING( ",,", outDelChunkStart, outDelChunkEnd, "," );


	LEOGetChunkRanges( ",,", kLEOChunkTypeItem,
					2, 2,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	ASSERT( outChunkStart == 2 );
	ASSERT( outChunkEnd == 2 );
	ASSERT( outDelChunkStart == 1 );
	ASSERT( outDelChunkEnd == 2 );
	ASSERT_RANGE_MATCHES_STRING( ",,", outChunkStart, outChunkEnd, "" );
	ASSERT_RANGE_MATCHES_STRING( ",,", outDelChunkStart, outDelChunkEnd, "," );

	LEOGetChunkRanges( ",,", kLEOChunkTypeItem,
					0, 1,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	ASSERT( outChunkStart == 0 );
	ASSERT( outChunkEnd == 1 );
	ASSERT( outDelChunkStart == 0 );
	ASSERT( outDelChunkEnd == 2 );
	ASSERT_RANGE_MATCHES_STRING( ",,", outChunkStart, outChunkEnd, "," );
	ASSERT_RANGE_MATCHES_STRING( ",,", outDelChunkStart, outDelChunkEnd, ",," );

	LEOGetChunkRanges( ",,", kLEOChunkTypeItem,
					1, 2,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	ASSERT( outChunkStart == 1 );
	ASSERT( outChunkEnd == 2 );
	ASSERT( outDelChunkStart == 0 );
	ASSERT( outDelChunkEnd == 2 );
	ASSERT_RANGE_MATCHES_STRING( ",,", outChunkStart, outChunkEnd, "," );
	ASSERT_RANGE_MATCHES_STRING( ",,", outDelChunkStart, outDelChunkEnd, ",," );

	LEOGetChunkRanges( ",,", kLEOChunkTypeItem,
					0, 2,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	ASSERT( outChunkStart == 0 );
	ASSERT( outChunkEnd == 2 );
	ASSERT( outDelChunkStart == 0 );
	ASSERT( outDelChunkEnd == 2 );
	ASSERT_RANGE_MATCHES_STRING( ",,", outChunkStart, outChunkEnd, ",," );
	ASSERT_RANGE_MATCHES_STRING( ",,", outDelChunkStart, outDelChunkEnd, ",," );
}


void	DoChunkValueTests( void )
{
	LEOContext		*	ctx;
	union LEOValue		theValue;
	char				str[256];
	LEOContextGroup*	group = LEOContextGroupCreate();
	
	ctx = LEOContextCreate( group, NULL, NULL );
	LEOContextGroupRelease( group );
	
	printf( "\nnote: Value Chunk Range tests\n" );
	
    const char*     theChunkStr = "this,that,more";
	LEOInitStringValue( (LEOValuePtr) &theValue, theChunkStr, strlen(theChunkStr), kLEOInvalidateReferences, ctx );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsRangeOfString( &theValue, kLEOChunkTypeItem, 0, 0, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"this") == 0 );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsRangeOfString( &theValue, kLEOChunkTypeItem, 1, 1, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"that") == 0 );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsRangeOfString( &theValue, kLEOChunkTypeItem, 2, 2, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"more") == 0 );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsRangeOfString( &theValue, kLEOChunkTypeItem, 0, 1, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"this,that") == 0 );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsRangeOfString( &theValue, kLEOChunkTypeItem, 0, 2, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"this,that,more") == 0 );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsRangeOfString( &theValue, kLEOChunkTypeItem, 1, 2, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"that,more") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, ctx );
	
	printf( "\nnote: Modifying initially-constant string chunk tests\n" );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringConstantValue( (LEOValuePtr) &theValue, "this,that,more", kLEOInvalidateReferences, ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 0, 0, "THIS", ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"THIS,that,more") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringConstantValue( (LEOValuePtr) &theValue, "this,that,more", kLEOInvalidateReferences, ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 1, 1, "THAT", ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"this,THAT,more") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringConstantValue( (LEOValuePtr) &theValue, "this,that,more", kLEOInvalidateReferences, ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 2, 2, "MORE", ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"this,that,MORE") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringConstantValue( (LEOValuePtr) &theValue, "this,that,more", kLEOInvalidateReferences, ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 0, 1, "THISTHAT", ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"THISTHAT,more") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringConstantValue( (LEOValuePtr) &theValue, "this,that,more", kLEOInvalidateReferences, ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 0, 2, "THISTHATMORE", ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"THISTHATMORE") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringConstantValue( (LEOValuePtr) &theValue, "this,that,more", kLEOInvalidateReferences, ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 1, 2, "THATMORE", ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"this,THATMORE") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, ctx );
	
	printf( "\nnote: Modifying dynamic string chunk tests\n" );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringValue( (LEOValuePtr) &theValue, theChunkStr, strlen(theChunkStr), kLEOInvalidateReferences, ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 0, 0, "THIS", ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"THIS,that,more") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringValue( (LEOValuePtr) &theValue, theChunkStr, strlen(theChunkStr), kLEOInvalidateReferences, ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 1, 1, "THAT", ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"this,THAT,more") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringValue( (LEOValuePtr) &theValue, theChunkStr, strlen(theChunkStr), kLEOInvalidateReferences, ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 2, 2, "MORE", ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"this,that,MORE") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringValue( (LEOValuePtr) &theValue, theChunkStr, strlen(theChunkStr), kLEOInvalidateReferences, ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 0, 1, "THISTHAT", ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"THISTHAT,more") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringValue( (LEOValuePtr) &theValue, theChunkStr, strlen(theChunkStr), kLEOInvalidateReferences, ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 0, 2, "THISTHATMORE", ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"THISTHATMORE") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringValue( (LEOValuePtr) &theValue, theChunkStr, strlen(theChunkStr), kLEOInvalidateReferences, ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 1, 2, "THATMORE", ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"this,THATMORE") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, ctx );

	printf( "\nnote: Delete initially-constant string chunk tests\n" );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringConstantValue( (LEOValuePtr) &theValue, "this,that,more", kLEOInvalidateReferences, ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 0, 0, NULL, ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"that,more") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringConstantValue( (LEOValuePtr) &theValue, "this,that,more", kLEOInvalidateReferences, ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 1, 1, NULL, ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"this,more") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringConstantValue( (LEOValuePtr) &theValue, "this,that,more", kLEOInvalidateReferences, ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 2, 2, NULL, ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"this,that") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringConstantValue( (LEOValuePtr) &theValue, "this,that,more", kLEOInvalidateReferences, ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 0, 1, NULL, ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"more") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringConstantValue( (LEOValuePtr) &theValue, "this,that,more", kLEOInvalidateReferences, ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 0, 2, NULL, ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringConstantValue( (LEOValuePtr) &theValue, "this,that,more", kLEOInvalidateReferences, ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 1, 2, NULL, ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"this") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, ctx );
	
	printf( "\nnote: Delete dynamic string chunk tests\n" );
		
	memset( str, 'X', sizeof(str) );
	LEOInitStringValue( (LEOValuePtr) &theValue, theChunkStr, strlen(theChunkStr), kLEOInvalidateReferences, ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 0, 0, NULL, ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"that,more") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringValue( (LEOValuePtr) &theValue, theChunkStr, strlen(theChunkStr), kLEOInvalidateReferences, ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 1, 1, NULL, ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"this,more") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringValue( (LEOValuePtr) &theValue, theChunkStr, strlen(theChunkStr), kLEOInvalidateReferences, ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 2, 2, NULL, ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"this,that") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringValue( (LEOValuePtr) &theValue, theChunkStr, strlen(theChunkStr), kLEOInvalidateReferences, ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 0, 1, NULL, ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"more") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringValue( (LEOValuePtr) &theValue, theChunkStr, strlen(theChunkStr), kLEOInvalidateReferences, ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 0, 2, NULL, ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringValue( (LEOValuePtr) &theValue, theChunkStr, strlen(theChunkStr), kLEOInvalidateReferences, ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 1, 2, NULL, ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"this") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, ctx );
	
	LEOContextRelease( ctx );
}


void	DoReferenceTest( void )
{
	LEOContext		*	ctx;
	union LEOValue		theValue;
	union LEOValue		originalValue;
	char				str[256];
	LEOContextGroup*	group = LEOContextGroupCreate();
	
	printf( "\nnote: Reference to string value tests\n" );
		
	ctx = LEOContextCreate( group, NULL, NULL );
	LEOContextGroupRelease( group );
	
	LEOInitStringConstantValue( &originalValue, "I am as real as it gets.", kLEOInvalidateReferences, ctx );
	LEOInitReferenceValue( &theValue, &originalValue, kLEOInvalidateReferences, kLEOChunkTypeINVALID, 0, 0, ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsString( &theValue, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"I am as real as it gets.") == 0 );

	memset( str, 'X', sizeof(str) );
	LEOGetValueAsRangeOfString( &theValue, kLEOChunkTypeCharacter, 0, 4,
								str, sizeof(str), ctx );
	ASSERT( strcmp(str,"I am") == 0 );
	LEOSetValueAsCString( &theValue, "Even when set indirectly", ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsString( &originalValue, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"Even when set indirectly") == 0 );
	
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeCharacter, 14, 24,
								"by reference", ctx );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsString( &originalValue, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"Even when set by reference") == 0 );
	
	printf( "\nnote: Reference to int value tests\n" );
		
	LEOCleanUpValue( &originalValue, kLEOInvalidateReferences, ctx );	// Invalidates theValue's reference.
	LEOInitNumberValue( &originalValue, 42, kLEOUnitNone, kLEOInvalidateReferences, ctx );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, ctx );
	LEOInitReferenceValue( &theValue, &originalValue, kLEOInvalidateReferences, kLEOChunkTypeINVALID, 0, 0, ctx );
	
	LEOUnit	theUnit = kLEOUnitMegabytes;
	double	theNum = LEOGetValueAsNumber( &theValue, &theUnit, ctx );
	ASSERT( theNum == 42.0 );
	ASSERT( theUnit == kLEOUnitNone );
	theUnit = kLEOUnitSeconds;
	LEOSetValueAsNumber( &theValue, 11, kLEOUnitNone, ctx );
	theNum = LEOGetValueAsNumber( &originalValue, &theUnit, ctx );
	ASSERT( theNum == 11.0 );
	ASSERT( theUnit == kLEOUnitNone );
	
	printf( "\nnote: Reference to bool value tests\n" );
		
	LEOCleanUpValue( &originalValue, kLEOInvalidateReferences, ctx );	// Invalidates theValue's reference.
	LEOInitBooleanValue( &originalValue, true, kLEOInvalidateReferences, ctx );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, ctx );
	LEOInitReferenceValue( &theValue, &originalValue, kLEOInvalidateReferences, kLEOChunkTypeINVALID, 0, 0, ctx );

	bool	theBool = LEOGetValueAsBoolean( &theValue, ctx );
	ASSERT( theBool == true );
	
	LEOSetValueAsCString( &theValue, "false", ctx );
	theBool = LEOGetValueAsBoolean( &originalValue, ctx );
	ASSERT( theBool == false );
	
	LEOCleanUpValue( &originalValue, kLEOInvalidateReferences, ctx );
	
	printf( "\nnote: Reference to disposed value tests\n" );
		
	// Verify we got an error about disposed original:
	LEOSetValueAsNumber( &theValue, 1, kLEOUnitNone, ctx );
	ASSERT( ctx->errMsg[0] != 0 );
	ASSERT( strcmp(ctx->errMsg, "The referenced value doesn't exist anymore." ) == 0 );
	ASSERT( ctx->keepRunning == false );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, ctx );
	
	LEOContextRelease( ctx );
}


void	DoWordsTestSingleSpaced( void )
{
	printf( "\nnote: Single-spaced words chunk tests\n" );
		
	const char*		theStr = "this that more";
	size_t			outChunkStart, outChunkEnd,
					outDelChunkStart, outDelChunkEnd;

	LEOGetChunkRanges( theStr, kLEOChunkTypeWord,
					0, 0,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	ASSERT_RANGE_MATCHES_STRING( theStr, outChunkStart, outChunkEnd, "this" );
	ASSERT_RANGE_MATCHES_STRING( theStr, outDelChunkStart, outDelChunkEnd, "this" );

	LEOGetChunkRanges( theStr, kLEOChunkTypeWord,
					1, 1,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	ASSERT_RANGE_MATCHES_STRING( theStr, outChunkStart, outChunkEnd, "that" );
	ASSERT_RANGE_MATCHES_STRING( theStr, outDelChunkStart, outDelChunkEnd, "that" );

	LEOGetChunkRanges( theStr, kLEOChunkTypeWord,
					2, 2,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	ASSERT_RANGE_MATCHES_STRING( theStr, outChunkStart, outChunkEnd, "more" );
	ASSERT_RANGE_MATCHES_STRING( theStr, outDelChunkStart, outDelChunkEnd, "more" );

	LEOGetChunkRanges( theStr, kLEOChunkTypeWord,
					0, 1,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	ASSERT_RANGE_MATCHES_STRING( theStr, outChunkStart, outChunkEnd, "this that" );
	ASSERT_RANGE_MATCHES_STRING( theStr, outDelChunkStart, outDelChunkEnd, "this that" );

	LEOGetChunkRanges( theStr, kLEOChunkTypeWord,
					1, 2,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	ASSERT_RANGE_MATCHES_STRING( theStr, outChunkStart, outChunkEnd, "that more" );
	ASSERT_RANGE_MATCHES_STRING( theStr, outDelChunkStart, outDelChunkEnd, "that more" );

	LEOGetChunkRanges( theStr, kLEOChunkTypeWord,
					0, 2,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	ASSERT_RANGE_MATCHES_STRING( theStr, outChunkStart, outChunkEnd, "this that more" );
	ASSERT_RANGE_MATCHES_STRING( theStr, outDelChunkStart, outDelChunkEnd, "this that more" );
}


void	DoWordsTestDoubleSpaced( void )
{
	printf( "\nnote: Double-spaced words chunk tests\n" );
		
	const char*		theStr = "this  that  more";
	size_t			outChunkStart, outChunkEnd,
					outDelChunkStart, outDelChunkEnd;

	LEOGetChunkRanges( theStr, kLEOChunkTypeWord,
					0, 0,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	ASSERT_RANGE_MATCHES_STRING( theStr, outChunkStart, outChunkEnd, "this" );
	ASSERT_RANGE_MATCHES_STRING( theStr, outDelChunkStart, outDelChunkEnd, "this" );

	LEOGetChunkRanges( theStr, kLEOChunkTypeWord,
					1, 1,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	ASSERT_RANGE_MATCHES_STRING( theStr, outChunkStart, outChunkEnd, "that" );
	ASSERT_RANGE_MATCHES_STRING( theStr, outDelChunkStart, outDelChunkEnd, "that" );

	LEOGetChunkRanges( theStr, kLEOChunkTypeWord,
					2, 2,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	ASSERT_RANGE_MATCHES_STRING( theStr, outChunkStart, outChunkEnd, "more" );
	ASSERT_RANGE_MATCHES_STRING( theStr, outDelChunkStart, outDelChunkEnd, "more" );

	LEOGetChunkRanges( theStr, kLEOChunkTypeWord,
					0, 1,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	ASSERT_RANGE_MATCHES_STRING( theStr, outChunkStart, outChunkEnd, "this  that" );
	ASSERT_RANGE_MATCHES_STRING( theStr, outDelChunkStart, outDelChunkEnd, "this  that" );

	LEOGetChunkRanges( theStr, kLEOChunkTypeWord,
					1, 2,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	ASSERT_RANGE_MATCHES_STRING( theStr, outChunkStart, outChunkEnd, "that  more" );
	ASSERT_RANGE_MATCHES_STRING( theStr, outDelChunkStart, outDelChunkEnd, "that  more" );

	LEOGetChunkRanges( theStr, kLEOChunkTypeWord,
					0, 2,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	ASSERT_RANGE_MATCHES_STRING( theStr, outChunkStart, outChunkEnd, "this  that  more" );
	ASSERT_RANGE_MATCHES_STRING( theStr, outDelChunkStart, outDelChunkEnd, "this  that  more" );
}


void	DoWordsTestLeadingWhiteSingleSpaced( void )
{
	printf( "\nnote: Single-spaced words chunk with leading whitespace tests\n" );
		
	const char*		theStr = " this that more ";
	size_t			outChunkStart, outChunkEnd,
					outDelChunkStart, outDelChunkEnd;

	LEOGetChunkRanges( theStr, kLEOChunkTypeWord,
					0, 0,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	ASSERT_RANGE_MATCHES_STRING( theStr, outChunkStart, outChunkEnd, "this" );
	ASSERT_RANGE_MATCHES_STRING( theStr, outDelChunkStart, outDelChunkEnd, "this" );

	LEOGetChunkRanges( theStr, kLEOChunkTypeWord,
					1, 1,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	ASSERT_RANGE_MATCHES_STRING( theStr, outChunkStart, outChunkEnd, "that" );
	ASSERT_RANGE_MATCHES_STRING( theStr, outDelChunkStart, outDelChunkEnd, "that" );

	LEOGetChunkRanges( theStr, kLEOChunkTypeWord,
					2, 2,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	ASSERT_RANGE_MATCHES_STRING( theStr, outChunkStart, outChunkEnd, "more" );
	ASSERT_RANGE_MATCHES_STRING( theStr, outDelChunkStart, outDelChunkEnd, "more" );

	LEOGetChunkRanges( theStr, kLEOChunkTypeWord,
					0, 1,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	ASSERT_RANGE_MATCHES_STRING( theStr, outChunkStart, outChunkEnd, "this that" );
	ASSERT_RANGE_MATCHES_STRING( theStr, outDelChunkStart, outDelChunkEnd, "this that" );

	LEOGetChunkRanges( theStr, kLEOChunkTypeWord,
					1, 2,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	ASSERT_RANGE_MATCHES_STRING( theStr, outChunkStart, outChunkEnd, "that more" );
	ASSERT_RANGE_MATCHES_STRING( theStr, outDelChunkStart, outDelChunkEnd, "that more" );

	LEOGetChunkRanges( theStr, kLEOChunkTypeWord,
					0, 2,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	ASSERT_RANGE_MATCHES_STRING( theStr, outChunkStart, outChunkEnd, "this that more" );
	ASSERT_RANGE_MATCHES_STRING( theStr, outDelChunkStart, outDelChunkEnd, "this that more" );
}


void	DoWordsTestLeadingWhiteDoubleSpaced( void )
{
	printf( "\nnote: Double-spaced words chunk with leading whitespace tests\n" );
		
	const char*		theStr = "  this  that  more  ";
	size_t			outChunkStart, outChunkEnd,
					outDelChunkStart, outDelChunkEnd;

	LEOGetChunkRanges( theStr, kLEOChunkTypeWord,
					0, 0,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	ASSERT_RANGE_MATCHES_STRING( theStr, outChunkStart, outChunkEnd, "this" );
	ASSERT_RANGE_MATCHES_STRING( theStr, outDelChunkStart, outDelChunkEnd, "this" );

	LEOGetChunkRanges( theStr, kLEOChunkTypeWord,
					1, 1,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	ASSERT_RANGE_MATCHES_STRING( theStr, outChunkStart, outChunkEnd, "that" );
	ASSERT_RANGE_MATCHES_STRING( theStr, outDelChunkStart, outDelChunkEnd, "that" );

	LEOGetChunkRanges( theStr, kLEOChunkTypeWord,
					2, 2,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	ASSERT_RANGE_MATCHES_STRING( theStr, outChunkStart, outChunkEnd, "more" );
	ASSERT_RANGE_MATCHES_STRING( theStr, outDelChunkStart, outDelChunkEnd, "more" );

	LEOGetChunkRanges( theStr, kLEOChunkTypeWord,
					0, 1,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	ASSERT_RANGE_MATCHES_STRING( theStr, outChunkStart, outChunkEnd, "this  that" );
	ASSERT_RANGE_MATCHES_STRING( theStr, outDelChunkStart, outDelChunkEnd, "this  that" );

	LEOGetChunkRanges( theStr, kLEOChunkTypeWord,
					1, 2,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	ASSERT_RANGE_MATCHES_STRING( theStr, outChunkStart, outChunkEnd, "that  more" );
	ASSERT_RANGE_MATCHES_STRING( theStr, outDelChunkStart, outDelChunkEnd, "that  more" );

	LEOGetChunkRanges( theStr, kLEOChunkTypeWord,
					0, 2,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	ASSERT_RANGE_MATCHES_STRING( theStr, outChunkStart, outChunkEnd, "this  that  more" );
	ASSERT_RANGE_MATCHES_STRING( theStr, outDelChunkStart, outDelChunkEnd, "this  that  more" );
}


void	DoChunkReferenceTests( void )
{
	LEOContext		*	ctx;
	union LEOValue		theValue;
	union LEOValue		valueReference;
	union LEOValue		valueRefReference;
	char				str[256];
	LEOContextGroup*	group = LEOContextGroupCreate();
	
	ctx = LEOContextCreate( group, NULL, NULL );
	LEOContextGroupRelease( group );
	
	printf( "\nnote: Chunked reference tests\n" );
	
    const char*     theChunkStr = "The Chunked reference";
	LEOInitStringValue( (LEOValuePtr) &theValue, theChunkStr, strlen(theChunkStr), kLEOInvalidateReferences, ctx );
	LEOInitReferenceValue( (LEOValuePtr) &valueReference, (LEOValuePtr) &theValue, kLEOInvalidateReferences, kLEOChunkTypeWord, 1, 1, ctx );
	LEOInitReferenceValue( (LEOValuePtr) &valueRefReference, (LEOValuePtr) &valueReference, kLEOInvalidateReferences, kLEOChunkTypeItem, 0, 0, ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsString( &theValue, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"The Chunked reference") == 0 );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsString( &valueReference, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"Chunked") == 0 );
	
	LEOSetValueAsNumber( &valueReference, 3.14, kLEOUnitNone, ctx );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsString( &theValue, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"The 3.14 reference") == 0 );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsString( &valueReference, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"3.14") == 0 );
	LEOUnit	theUnit = kLEOUnitHours;
	ASSERT( ((LEOGetValueAsNumber( &valueReference, &theUnit, ctx ) - 3.14) < 0.0001) && ctx->keepRunning == true );

	LEOSetValueAsBoolean( &valueReference, true, ctx );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsString( &theValue, str, sizeof(str), ctx );
	ASSERT( strcasecmp(str,"The true reference") == 0 );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsString( &valueReference, str, sizeof(str), ctx );
	ASSERT( strcasecmp(str,"true") == 0 );
	ASSERT( LEOGetValueAsBoolean( &valueReference, ctx ) == true && ctx->keepRunning == true );

	LEOSetValueAsCString( &valueReference, "this,that", ctx );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsString( &theValue, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"The this,that reference") == 0 );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsString( &valueReference, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"this,that") == 0 );
	
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsString( &valueRefReference, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"this") == 0 );
	
	LEOSetValueAsCString( &valueRefReference, "THIS", ctx );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsString( &theValue, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"The THIS,that reference") == 0 );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsString( &valueReference, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"THIS,that") == 0 );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsString( &valueRefReference, str, sizeof(str), ctx );
	ASSERT( strcmp(str,"THIS") == 0 );
	
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, ctx );
	LEOCleanUpValue( &valueReference, kLEOInvalidateReferences, ctx );
	LEOCleanUpValue( &valueRefReference, kLEOInvalidateReferences, ctx );
	LEOContextRelease( ctx );
}


static char*	sIndividualItems[] = { "this", "that", "more" };

static bool	DoForAllItemsCallback( const char* currStr, size_t currLen, size_t currStart, size_t currEnd, void* userData )
{
	char		theCurrStr[1024] = { 0 };
	int*		currIdx = ((int*) userData);
	memmove( theCurrStr, currStr, (currLen >= sizeof(theCurrStr)) ? (sizeof(theCurrStr) -1) : currLen );
	ASSERT_STRING_MATCH( theCurrStr, sIndividualItems[*currIdx] );
	(*currIdx)++;
	
	return true;
}


static char*	sIndividualItemsWithEmpty[] = { "full", "", "full again" };

static bool	DoForAllItemsWithEmptyCallback( const char* currStr, size_t currLen, size_t currStart, size_t currEnd, void* userData )
{
	char		theCurrStr[1024] = { 0 };
	int*		currIdx = ((int*) userData);
	memmove( theCurrStr, currStr, (currLen >= sizeof(theCurrStr)) ? (sizeof(theCurrStr) -1) : currLen );
	ASSERT_STRING_MATCH( theCurrStr, sIndividualItemsWithEmpty[*currIdx] );
	(*currIdx)++;
	
	return true;
}


static char*	sIndividualItemsWithLeadingEmpty[] = { "", "full", "still full" };

static bool	DoForAllItemsWithLeadingEmptyCallback( const char* currStr, size_t currLen, size_t currStart, size_t currEnd, void* userData )
{
	char		theCurrStr[1024] = { 0 };
	int*		currIdx = ((int*) userData);
	memmove( theCurrStr, currStr, (currLen >= sizeof(theCurrStr)) ? (sizeof(theCurrStr) -1) : currLen );
	ASSERT_STRING_MATCH( theCurrStr, sIndividualItemsWithLeadingEmpty[*currIdx] );
	(*currIdx)++;
	
	return true;
}

static char*	sIndividualItemsWithTrailingEmpty[] = { "full", "still full", "" };

static bool	DoForAllItemsWithTrailingEmptyCallback( const char* currStr, size_t currLen, size_t currStart, size_t currEnd, void* userData )
{
	char		theCurrStr[1024] = { 0 };
	int*		currIdx = ((int*) userData);
	memmove( theCurrStr, currStr, (currLen >= sizeof(theCurrStr)) ? (sizeof(theCurrStr) -1) : currLen );
	ASSERT_STRING_MATCH( theCurrStr, sIndividualItemsWithTrailingEmpty[*currIdx] );
	(*currIdx)++;
	
	return true;
}



static char*	sIndividualChars[] = { "E", "r", "i", "c" };

static bool	DoForAllCharsCallback( const char* currStr, size_t currLen, size_t currStart, size_t currEnd, void* userData )
{
	char		theCurrStr[1024] = { 0 };
	int*		currIdx = ((int*) userData);
	memmove( theCurrStr, currStr, (currLen >= sizeof(theCurrStr)) ? (sizeof(theCurrStr) -1) : currLen );
	ASSERT_STRING_MATCH( theCurrStr, sIndividualChars[*currIdx] );
	(*currIdx)++;
	
	return true;
}


static char*	sIndividualUnicodeChars[] = { "G", "r", "\303\274", "b", "e", "l", " ", "\342\234\216" };

static bool	DoForAllUnicodeCharsCallback( const char* currStr, size_t currLen, size_t currStart, size_t currEnd, void* userData )
{
	char		theCurrStr[1024] = { 0 };
	int*		currIdx = ((int*) userData);
	memmove( theCurrStr, currStr, (currLen >= sizeof(theCurrStr)) ? (sizeof(theCurrStr) -1) : currLen );
	ASSERT_STRING_MATCH( theCurrStr, sIndividualUnicodeChars[*currIdx] );
	(*currIdx)++;
	
	return true;
}


static char*	sIndividualUnicodeBytes[] = { "G", "r", "\303", "\274", "b", "e", "l", " ", "\342", "\234", "\216" };

static bool	DoForAllUnicodeBytesCallback( const char* currStr, size_t currLen, size_t currStart, size_t currEnd, void* userData )
{
	char		theCurrStr[1024] = { 0 };
	int*		currIdx = ((int*) userData);
	memmove( theCurrStr, currStr, (currLen >= sizeof(theCurrStr)) ? (sizeof(theCurrStr) -1) : currLen );
	ASSERT_STRING_MATCH( theCurrStr, sIndividualUnicodeBytes[*currIdx] );
	(*currIdx)++;
	
	return true;
}


void	DoAllChunksTest()
{
	printf( "\nnote: Determine all chunks tests\n" );
	
	printf( "note: Items\n" );
	int			currIdx = 0;
	const char*	theStr = "this,that,more";
	LEODoForEachChunk( theStr, strlen(theStr), kLEOChunkTypeItem, DoForAllItemsCallback, ',', &currIdx );

	printf( "note: Items with empty items\n" );
	currIdx = 0;
	theStr = "full,,full again";
	LEODoForEachChunk( theStr, strlen(theStr), kLEOChunkTypeItem, DoForAllItemsWithEmptyCallback, ',', &currIdx );

	printf( "note: Items with leading empty item\n" );
	currIdx = 0;
	theStr = ",full,still full";
	LEODoForEachChunk( theStr, strlen(theStr), kLEOChunkTypeItem, DoForAllItemsWithLeadingEmptyCallback, ',', &currIdx );

	printf( "note: Items with trailing empty item\n" );
	currIdx = 0;
	theStr = "full,still full,";
	LEODoForEachChunk( theStr, strlen(theStr), kLEOChunkTypeItem, DoForAllItemsWithTrailingEmptyCallback, ',', &currIdx );

	printf( "note: Words\n" );
	currIdx = 0;
	theStr = "this that more";
	LEODoForEachChunk( theStr, strlen(theStr), kLEOChunkTypeWord, DoForAllItemsCallback, 0, &currIdx );

	printf( "note: Double-spaced words\n" );
	currIdx = 0;
	theStr = "this  that  more";
	LEODoForEachChunk( theStr, strlen(theStr), kLEOChunkTypeWord, DoForAllItemsCallback, 0, &currIdx );

	printf( "note: Leading/trailing spaces for words\n" );
	currIdx = 0;
	theStr = "  this  that  more  ";
	LEODoForEachChunk( theStr, strlen(theStr), kLEOChunkTypeWord, DoForAllItemsCallback, 0, &currIdx );

	printf( "note: Lines\n" );
	currIdx = 0;
	theStr = "this\nthat\rmore";
	LEODoForEachChunk( theStr, strlen(theStr), kLEOChunkTypeLine, DoForAllItemsCallback, 0, &currIdx );

	printf( "note: Lines with empty lines\n" );
	currIdx = 0;
	theStr = "full\n\rfull again";
	LEODoForEachChunk( theStr, strlen(theStr), kLEOChunkTypeLine, DoForAllItemsWithEmptyCallback, 0, &currIdx );

	printf( "note: Lines with leading empty line\n" );
	currIdx = 0;
	theStr = "\nfull\rstill full";
	LEODoForEachChunk( theStr, strlen(theStr), kLEOChunkTypeLine, DoForAllItemsWithLeadingEmptyCallback, 0, &currIdx );

	printf( "note: Lines with leading empty line\n" );
	currIdx = 0;
	theStr = "full\rstill full\n";
	LEODoForEachChunk( theStr, strlen(theStr), kLEOChunkTypeLine, DoForAllItemsWithTrailingEmptyCallback, 0, &currIdx );

	printf( "note: Characters\n" );
	currIdx = 0;
	theStr = "Eric";
	LEODoForEachChunk( theStr, strlen(theStr), kLEOChunkTypeCharacter, DoForAllCharsCallback, 0, &currIdx );

	printf( "note: Bytes\n" );
	currIdx = 0;
	theStr = "Eric";
	LEODoForEachChunk( theStr, strlen(theStr), kLEOChunkTypeByte, DoForAllCharsCallback, 0, &currIdx );
	
	printf( "note: Characters with Unicode\n" );
	currIdx = 0;
	theStr = "Gr\303\274bel \342\234\216";
	LEODoForEachChunk( theStr, strlen(theStr), kLEOChunkTypeCharacter, DoForAllUnicodeCharsCallback, 0, &currIdx );
	
	printf( "note: Bytes with Unicode\n" );
	currIdx = 0;
	theStr = "Gr\303\274bel \342\234\216";
	LEODoForEachChunk( theStr, strlen(theStr), kLEOChunkTypeByte, DoForAllUnicodeBytesCallback, 0, &currIdx );

}


int main( int argc, char** argv )
{
	LEOInitInstructionArray();
	
	DoChunkTests();
	DoChunkValueTests();
	DoReferenceTest();
	DoWordsTestSingleSpaced();
	DoWordsTestDoubleSpaced();
	DoWordsTestLeadingWhiteSingleSpaced();
	DoWordsTestLeadingWhiteDoubleSpaced();
	DoAllChunksTest();
	
	DoScriptTest();
	
	DoChunkReferenceTests();
	
	if( gAnyTestFailed )
		printf( "* BUILD FAILED *\n" );
	
	return EXIT_SUCCESS;
}