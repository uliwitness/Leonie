/*
 *  TestsMain.c
 *  Leonie
 *
 *  Created by Uli Kusterer on 09.10.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "LEOInterpreter.h"
#include "LEOChunks.h"


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


void	DoChunkTests()
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


void	DoChunkValueTests()
{
	LEOContext			ctx;
	union LEOValue		theValue;
	char				str[256];
	
	LEOInitContext( &ctx );
	
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


void	DoReferenceTest()
{
	LEOContext			ctx;
	union LEOValue		theValue;
	union LEOValue		originalValue;
	char				str[256];
	
	printf( "\nnote: Reference to string value tests\n" );
		
	LEOInitContext( &ctx );
	
	LEOInitStringConstantValue( &originalValue.base, "I am as real as it gets.", kLEOInvalidateReferences, &ctx );
	LEOInitReferenceValue( &theValue.base, &originalValue.base, kLEOInvalidateReferences, &ctx );
	
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
		
	LEOCleanUpValue( &originalValue.base, kLEOInvalidateReferences, &ctx );	// Invalidates theValue's reference.
	LEOInitNumberValue( &originalValue.base, 42, kLEOInvalidateReferences, &ctx );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, &ctx );
	LEOInitReferenceValue( &theValue.base, &originalValue.base, kLEOInvalidateReferences, &ctx );
	
	double	theNum = LEOGetValueAsNumber( &theValue, &ctx );
	ASSERT( theNum == 42.0 );
	LEOSetValueAsNumber( &theValue, 11, &ctx );
	theNum = LEOGetValueAsNumber( &originalValue, &ctx );
	ASSERT( theNum == 11.0 );
	
	printf( "\nnote: Reference to bool value tests\n" );
		
	LEOCleanUpValue( &originalValue.base, kLEOInvalidateReferences, &ctx );	// Invalidates theValue's reference.
	LEOInitBooleanValue( &originalValue.base, true, kLEOInvalidateReferences, &ctx );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, &ctx );
	LEOInitReferenceValue( &theValue.base, &originalValue.base, kLEOInvalidateReferences, &ctx );

	bool	theBool = LEOGetValueAsBoolean( &theValue, &ctx );
	ASSERT( theBool == true );
	
	LEOSetValueAsString( &theValue.base, "false", &ctx );
	theBool = LEOGetValueAsBoolean( &originalValue, &ctx );
	ASSERT( theBool == false );
	
	LEOCleanUpValue( &originalValue, kLEOInvalidateReferences, &ctx );
	
	printf( "\nnote: Reference to disposed value tests\n" );
		
	// Verify we got an error about disposed original:
	LEOSetValueAsNumber( &theValue.base, 1, &ctx );
	ASSERT( ctx.errMsg[0] != 0 );
	ASSERT( strcmp(ctx.errMsg, "The referenced value doesn't exist anymore." ) == 0 );
	ASSERT( ctx.keepRunning == false );
	LEOCleanUpValue( &theValue, kLEOInvalidateReferences, &ctx );
	
	LEOCleanUpContext( &ctx );
}


void	DoWordsTestSingleSpaced()
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


void	DoWordsTestDoubleSpaced()
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


void	DoWordsTestLeadingWhiteSingleSpaced()
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


void	DoWordsTestLeadingWhiteDoubleSpaced()
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


int main( int argc, char** argv )
{
	DoChunkTests();
	DoChunkValueTests();
	DoReferenceTest();
	DoWordsTestSingleSpaced();
	DoWordsTestDoubleSpaced();
	DoWordsTestLeadingWhiteSingleSpaced();
	DoWordsTestLeadingWhiteDoubleSpaced();
	
	return EXIT_SUCCESS;
}