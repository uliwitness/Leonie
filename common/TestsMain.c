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


#define ASSERT(expr)	({ if( !(expr) ) printf( "error: Test failed: %s\n", #expr ); else printf( "note: Test passed: %s\n", #expr ); })


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
	LEOScript		*	theScript = LEOScriptCreateForOwner( 0, 0 );
	LEOHandler		*	newHandler = NULL;
	LEOHandler		*	foundHandler = NULL;
	
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
	LEOContext			ctx;
	union LEOValue		theValue;
	char				str[256];
	LEOContextGroup*	group = LEOContextGroupCreate();
	
	LEOInitContext( &ctx, group );
	LEOContextGroupRelease( group );
	
	printf( "\nnote: Value Chunk Range tests\n" );
	
	LEOInitStringValue( (LEOValuePtr) &theValue, "this,that,more", kLEOInvalidateReferences, &ctx );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsRangeOfString( &theValue, kLEOChunkTypeItem, 0, 0, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"this") == 0 );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsRangeOfString( &theValue, kLEOChunkTypeItem, 1, 1, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"that") == 0 );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsRangeOfString( &theValue, kLEOChunkTypeItem, 2, 2, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"more") == 0 );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsRangeOfString( &theValue, kLEOChunkTypeItem, 0, 1, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"this,that") == 0 );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsRangeOfString( &theValue, kLEOChunkTypeItem, 0, 2, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"this,that,more") == 0 );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsRangeOfString( &theValue, kLEOChunkTypeItem, 1, 2, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"that,more") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, &ctx );
	
	printf( "\nnote: Modifying initially-constant string chunk tests\n" );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringConstantValue( (LEOValuePtr) &theValue, "this,that,more", kLEOInvalidateReferences, &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 0, 0, "THIS", &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"THIS,that,more") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringConstantValue( (LEOValuePtr) &theValue, "this,that,more", kLEOInvalidateReferences, &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 1, 1, "THAT", &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"this,THAT,more") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringConstantValue( (LEOValuePtr) &theValue, "this,that,more", kLEOInvalidateReferences, &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 2, 2, "MORE", &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"this,that,MORE") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringConstantValue( (LEOValuePtr) &theValue, "this,that,more", kLEOInvalidateReferences, &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 0, 1, "THISTHAT", &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"THISTHAT,more") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringConstantValue( (LEOValuePtr) &theValue, "this,that,more", kLEOInvalidateReferences, &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 0, 2, "THISTHATMORE", &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"THISTHATMORE") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringConstantValue( (LEOValuePtr) &theValue, "this,that,more", kLEOInvalidateReferences, &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 1, 2, "THATMORE", &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"this,THATMORE") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, &ctx );
	
	printf( "\nnote: Modifying dynamic string chunk tests\n" );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringValue( (LEOValuePtr) &theValue, "this,that,more", kLEOInvalidateReferences, &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 0, 0, "THIS", &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"THIS,that,more") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringValue( (LEOValuePtr) &theValue, "this,that,more", kLEOInvalidateReferences, &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 1, 1, "THAT", &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"this,THAT,more") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringValue( (LEOValuePtr) &theValue, "this,that,more", kLEOInvalidateReferences, &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 2, 2, "MORE", &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"this,that,MORE") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringValue( (LEOValuePtr) &theValue, "this,that,more", kLEOInvalidateReferences, &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 0, 1, "THISTHAT", &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"THISTHAT,more") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringValue( (LEOValuePtr) &theValue, "this,that,more", kLEOInvalidateReferences, &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 0, 2, "THISTHATMORE", &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"THISTHATMORE") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringValue( (LEOValuePtr) &theValue, "this,that,more", kLEOInvalidateReferences, &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 1, 2, "THATMORE", &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"this,THATMORE") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, &ctx );

	printf( "\nnote: Delete initially-constant string chunk tests\n" );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringConstantValue( (LEOValuePtr) &theValue, "this,that,more", kLEOInvalidateReferences, &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 0, 0, NULL, &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"that,more") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringConstantValue( (LEOValuePtr) &theValue, "this,that,more", kLEOInvalidateReferences, &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 1, 1, NULL, &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"this,more") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringConstantValue( (LEOValuePtr) &theValue, "this,that,more", kLEOInvalidateReferences, &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 2, 2, NULL, &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"this,that") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringConstantValue( (LEOValuePtr) &theValue, "this,that,more", kLEOInvalidateReferences, &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 0, 1, NULL, &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"more") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringConstantValue( (LEOValuePtr) &theValue, "this,that,more", kLEOInvalidateReferences, &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 0, 2, NULL, &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringConstantValue( (LEOValuePtr) &theValue, "this,that,more", kLEOInvalidateReferences, &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 1, 2, NULL, &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"this") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, &ctx );
	
	printf( "\nnote: Delete dynamic string chunk tests\n" );
		
	memset( str, 'X', sizeof(str) );
	LEOInitStringValue( (LEOValuePtr) &theValue, "this,that,more", kLEOInvalidateReferences, &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 0, 0, NULL, &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"that,more") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringValue( (LEOValuePtr) &theValue, "this,that,more", kLEOInvalidateReferences, &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 1, 1, NULL, &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"this,more") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringValue( (LEOValuePtr) &theValue, "this,that,more", kLEOInvalidateReferences, &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 2, 2, NULL, &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"this,that") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringValue( (LEOValuePtr) &theValue, "this,that,more", kLEOInvalidateReferences, &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 0, 1, NULL, &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"more") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringValue( (LEOValuePtr) &theValue, "this,that,more", kLEOInvalidateReferences, &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 0, 2, NULL, &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringValue( (LEOValuePtr) &theValue, "this,that,more", kLEOInvalidateReferences, &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 1, 2, NULL, &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"this") == 0 );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, &ctx );
	
	LEOCleanUpContext( &ctx );
}


void	DoReferenceTest( void )
{
	LEOContext			ctx;
	union LEOValue		theValue;
	union LEOValue		originalValue;
	char				str[256];
	LEOContextGroup*	group = LEOContextGroupCreate();
	
	printf( "\nnote: Reference to string value tests\n" );
		
	LEOInitContext( &ctx, group );
	LEOContextGroupRelease( group );
	
	LEOInitStringConstantValue( &originalValue, "I am as real as it gets.", kLEOInvalidateReferences, &ctx );
	LEOInitReferenceValue( &theValue, &originalValue, kLEOInvalidateReferences, kLEOChunkTypeINVALID, 0, 0, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"I am as real as it gets.") == 0 );

	memset( str, 'X', sizeof(str) );
	LEOGetValueAsRangeOfString( &theValue, kLEOChunkTypeCharacter, 0, 4,
								str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"I am") == 0 );
	LEOSetValueAsString( &theValue, "Even when set indirectly", &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsString( &originalValue, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"Even when set indirectly") == 0 );
	
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeCharacter, 14, 24,
								"by reference", &ctx );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsString( &originalValue, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"Even when set by reference") == 0 );
	
	printf( "\nnote: Reference to int value tests\n" );
		
	LEOCleanUpValue( &originalValue, kLEOInvalidateReferences, &ctx );	// Invalidates theValue's reference.
	LEOInitNumberValue( &originalValue, 42, kLEOInvalidateReferences, &ctx );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, &ctx );
	LEOInitReferenceValue( &theValue, &originalValue, kLEOInvalidateReferences, kLEOChunkTypeINVALID, 0, 0, &ctx );
	
	double	theNum = LEOGetValueAsNumber( &theValue, &ctx );
	ASSERT( theNum == 42.0 );
	LEOSetValueAsNumber( &theValue, 11, &ctx );
	theNum = LEOGetValueAsNumber( &originalValue, &ctx );
	ASSERT( theNum == 11.0 );
	
	printf( "\nnote: Reference to bool value tests\n" );
		
	LEOCleanUpValue( &originalValue, kLEOInvalidateReferences, &ctx );	// Invalidates theValue's reference.
	LEOInitBooleanValue( &originalValue, true, kLEOInvalidateReferences, &ctx );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, &ctx );
	LEOInitReferenceValue( &theValue, &originalValue, kLEOInvalidateReferences, kLEOChunkTypeINVALID, 0, 0, &ctx );

	bool	theBool = LEOGetValueAsBoolean( &theValue, &ctx );
	ASSERT( theBool == true );
	
	LEOSetValueAsString( &theValue, "false", &ctx );
	theBool = LEOGetValueAsBoolean( &originalValue, &ctx );
	ASSERT( theBool == false );
	
	LEOCleanUpValue( &originalValue, kLEOInvalidateReferences, &ctx );
	
	printf( "\nnote: Reference to disposed value tests\n" );
		
	// Verify we got an error about disposed original:
	LEOSetValueAsNumber( &theValue, 1, &ctx );
	ASSERT( ctx.errMsg[0] != 0 );
	ASSERT( strcmp(ctx.errMsg, "The referenced value doesn't exist anymore." ) == 0 );
	ASSERT( ctx.keepRunning == false );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, &ctx );
	
	LEOCleanUpContext( &ctx );
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
	LEOContext			ctx;
	union LEOValue		theValue;
	union LEOValue		valueReference;
	union LEOValue		valueRefReference;
	char				str[256];
	LEOContextGroup*	group = LEOContextGroupCreate();
	
	LEOInitContext( &ctx, group );
	LEOContextGroupRelease( group );
	
	printf( "\nnote: Chunked reference tests\n" );
	
	LEOInitStringValue( (LEOValuePtr) &theValue, "The Chunked reference", kLEOInvalidateReferences, &ctx );
	LEOInitReferenceValue( (LEOValuePtr) &valueReference, (LEOValuePtr) &theValue, kLEOInvalidateReferences, kLEOChunkTypeWord, 1, 1, &ctx );
	LEOInitReferenceValue( (LEOValuePtr) &valueRefReference, (LEOValuePtr) &valueReference, kLEOInvalidateReferences, kLEOChunkTypeItem, 0, 0, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"The Chunked reference") == 0 );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsString( &valueReference, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"Chunked") == 0 );
	
	LEOSetValueAsNumber( &valueReference, 3.14, &ctx );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"The 3.14 reference") == 0 );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsString( &valueReference, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"3.14") == 0 );
	ASSERT( ((LEOGetValueAsNumber( &valueReference, &ctx ) - 3.14) < 0.0001) && ctx.keepRunning == true );

	LEOSetValueAsBoolean( &valueReference, true, &ctx );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	ASSERT( strcasecmp(str,"The true reference") == 0 );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsString( &valueReference, str, sizeof(str), &ctx );
	ASSERT( strcasecmp(str,"true") == 0 );
	ASSERT( LEOGetValueAsBoolean( &valueReference, &ctx ) == true && ctx.keepRunning == true );

	LEOSetValueAsString( &valueReference, "this,that", &ctx );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"The this,that reference") == 0 );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsString( &valueReference, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"this,that") == 0 );
	
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsString( &valueRefReference, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"this") == 0 );
	
	LEOSetValueAsString( &valueRefReference, "THIS", &ctx );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"The THIS,that reference") == 0 );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsString( &valueReference, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"THIS,that") == 0 );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsString( &valueRefReference, str, sizeof(str), &ctx );
	ASSERT( strcmp(str,"THIS") == 0 );
	
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, &ctx );
	LEOCleanUpValue( &valueReference, kLEOInvalidateReferences, &ctx );	
	LEOCleanUpValue( &valueRefReference, kLEOInvalidateReferences, &ctx );	
	LEOCleanUpContext( &ctx );
}


int main( int argc, char** argv )
{
	DoChunkTests();
	DoChunkValueTests();
	DoReferenceTest();
	DoWordsTestSingleSpaced();
	DoWordsTestDoubleSpaced();
	DoWordsTestLeadingWhiteSingleSpaced();
	DoWordsTestLeadingWhiteDoubleSpaced();
	
	DoScriptTest();
	
	DoChunkReferenceTests();
	
	return EXIT_SUCCESS;
}