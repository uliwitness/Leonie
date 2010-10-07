//
//  LeonieAppDelegate.m
//  Leonie
//
//  Created by Uli Kusterer on 06.09.10.
//  Copyright 2010 Uli Kusterer. All rights reserved.
//

#import "LeonieAppDelegate.h"
#import "LEOInterpreter.h"
#import "LEOInstructions.h"
#import "LEODebugger.h"
#import "LEOChunks.h"
#import <stdio.h>


#ifndef RUN_TESTS
#define RUN_TESTS				1
#endif // RUN_TESTS

#define ACTIVATE_DEBUGGER		0
#define PRINT_BYTECODE			0


@implementation LeonieAppDelegate

@synthesize window;
@synthesize busyIndicator;
@synthesize messageBoxField;


void LEOPrintStringWithRangeMarkers( const char* theStr, size_t chunkStart, size_t chunkEnd )
{
	size_t x = 0;
	printf("%s\n",theStr);
	size_t pEndOffs = chunkStart;
	if( pEndOffs > 0 )
		pEndOffs--;
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
	printf("(%s)\n",substr);
}


void	DoChunkTests()
{
	const char*		theStr = "this,that,more";
	size_t			outChunkStart, outChunkEnd,
					outDelChunkStart, outDelChunkEnd;
	
	printf( "==== chunk ====\n" );
	
	LEOGetChunkRanges( theStr, kLEOChunkTypeItem,
					0, 0,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	LEOPrintStringWithRangeMarkers( theStr, outChunkStart, outChunkEnd );
	LEOPrintStringWithRangeMarkers( theStr, outDelChunkStart, outDelChunkEnd );

	LEOGetChunkRanges( theStr, kLEOChunkTypeItem,
					1, 1,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	LEOPrintStringWithRangeMarkers( theStr, outChunkStart, outChunkEnd );
	LEOPrintStringWithRangeMarkers( theStr, outDelChunkStart, outDelChunkEnd );
	
	LEOGetChunkRanges( theStr, kLEOChunkTypeItem,
					2, 2,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	LEOPrintStringWithRangeMarkers( theStr, outChunkStart, outChunkEnd );
	LEOPrintStringWithRangeMarkers( theStr, outDelChunkStart, outDelChunkEnd );

	LEOGetChunkRanges( theStr, kLEOChunkTypeItem,
					0, 1,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	LEOPrintStringWithRangeMarkers( theStr, outChunkStart, outChunkEnd );
	LEOPrintStringWithRangeMarkers( theStr, outDelChunkStart, outDelChunkEnd );

	LEOGetChunkRanges( theStr, kLEOChunkTypeItem,
					1, 2,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	LEOPrintStringWithRangeMarkers( theStr, outChunkStart, outChunkEnd );
	LEOPrintStringWithRangeMarkers( theStr, outDelChunkStart, outDelChunkEnd );

	LEOGetChunkRanges( theStr, kLEOChunkTypeItem,
					0, 2,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	LEOPrintStringWithRangeMarkers( theStr, outChunkStart, outChunkEnd );
	LEOPrintStringWithRangeMarkers( theStr, outDelChunkStart, outDelChunkEnd );

	LEOGetChunkRanges( "daniel", kLEOChunkTypeItem,
					0, 0,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	LEOPrintStringWithRangeMarkers( "daniel", outChunkStart, outChunkEnd );
	LEOPrintStringWithRangeMarkers( "daniel", outDelChunkStart, outDelChunkEnd );

	LEOGetChunkRanges( "", kLEOChunkTypeItem,
					0, 0,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	LEOPrintStringWithRangeMarkers( "", outChunkStart, outChunkEnd );
	LEOPrintStringWithRangeMarkers( "", outDelChunkStart, outDelChunkEnd );

	LEOGetChunkRanges( ",,", kLEOChunkTypeItem,
					0, 0,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	LEOPrintStringWithRangeMarkers( ",,", outChunkStart, outChunkEnd );
	LEOPrintStringWithRangeMarkers( ",,", outDelChunkStart, outDelChunkEnd );

	LEOGetChunkRanges( ",,", kLEOChunkTypeItem,
					1, 1,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	LEOPrintStringWithRangeMarkers( ",,", outChunkStart, outChunkEnd );
	LEOPrintStringWithRangeMarkers( ",,", outDelChunkStart, outDelChunkEnd );


	LEOGetChunkRanges( ",,", kLEOChunkTypeItem,
					2, 2,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	LEOPrintStringWithRangeMarkers( ",,", outChunkStart, outChunkEnd );
	LEOPrintStringWithRangeMarkers( ",,", outDelChunkStart, outDelChunkEnd );

	LEOGetChunkRanges( ",,", kLEOChunkTypeItem,
					0, 1,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	LEOPrintStringWithRangeMarkers( ",,", outChunkStart, outChunkEnd );
	LEOPrintStringWithRangeMarkers( ",,", outDelChunkStart, outDelChunkEnd );

	LEOGetChunkRanges( ",,", kLEOChunkTypeItem,
					1, 2,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	LEOPrintStringWithRangeMarkers( ",,", outChunkStart, outChunkEnd );
	LEOPrintStringWithRangeMarkers( ",,", outDelChunkStart, outDelChunkEnd );

	LEOGetChunkRanges( ",,", kLEOChunkTypeItem,
					0, 2,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	LEOPrintStringWithRangeMarkers( ",,", outChunkStart, outChunkEnd );
	LEOPrintStringWithRangeMarkers( ",,", outDelChunkStart, outDelChunkEnd );
}


void	DoChunkValueTests()
{
	LEOContext			ctx;
	union LEOValue		theValue;
	char				str[256];
	
	LEOInitContext( &ctx );
	
	printf( "==== getters ====\n" );
	
	LEOInitStringValue( (LEOValuePtr) &theValue, "this,that,more", &ctx );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsRangeOfString( &theValue, kLEOChunkTypeItem, 0, 0, str, sizeof(str), &ctx );
	printf( "%s,THAT,MORE\n", str );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsRangeOfString( &theValue, kLEOChunkTypeItem, 1, 1, str, sizeof(str), &ctx );
	printf( "THIS,%s,MORE\n", str );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsRangeOfString( &theValue, kLEOChunkTypeItem, 2, 2, str, sizeof(str), &ctx );
	printf( "THIS,THAT,%s\n", str );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsRangeOfString( &theValue, kLEOChunkTypeItem, 0, 1, str, sizeof(str), &ctx );
	printf( "%s,MORE\n", str );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsRangeOfString( &theValue, kLEOChunkTypeItem, 0, 2, str, sizeof(str), &ctx );
	printf( "%s\n", str );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsRangeOfString( &theValue, kLEOChunkTypeItem, 1, 2, str, sizeof(str), &ctx );
	printf( "THIS,%s\n", str );
	LEOCleanUpValue( &theValue, &ctx );
	
	printf( "==== constant ====\n" );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringConstantValue( (LEOValuePtr) &theValue, "this,that,more", &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 0, 0, "THIS", &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	printf( "%s\n", str );
	LEOCleanUpValue( &theValue, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringConstantValue( (LEOValuePtr) &theValue, "this,that,more", &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 1, 1, "THAT", &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	printf( "%s\n", str );
	LEOCleanUpValue( &theValue, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringConstantValue( (LEOValuePtr) &theValue, "this,that,more", &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 2, 2, "MORE", &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	printf( "%s\n", str );
	LEOCleanUpValue( &theValue, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringConstantValue( (LEOValuePtr) &theValue, "this,that,more", &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 0, 1, "THISTHAT", &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	printf( "%s\n", str );
	LEOCleanUpValue( &theValue, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringConstantValue( (LEOValuePtr) &theValue, "this,that,more", &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 0, 2, "THISTHATMORE", &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	printf( "%s\n", str );
	LEOCleanUpValue( &theValue, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringConstantValue( (LEOValuePtr) &theValue, "this,that,more", &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 1, 2, "THATMORE", &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	printf( "%s\n", str );
	LEOCleanUpValue( &theValue, &ctx );
	
	printf( "==== dynamic ====\n" );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringValue( (LEOValuePtr) &theValue, "this,that,more", &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 0, 0, "THIS", &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	printf( "%s\n", str );
	LEOCleanUpValue( &theValue, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringValue( (LEOValuePtr) &theValue, "this,that,more", &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 1, 1, "THAT", &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	printf( "%s\n", str );
	LEOCleanUpValue( &theValue, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringValue( (LEOValuePtr) &theValue, "this,that,more", &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 2, 2, "MORE", &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	printf( "%s\n", str );
	LEOCleanUpValue( &theValue, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringValue( (LEOValuePtr) &theValue, "this,that,more", &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 0, 1, "THISTHAT", &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	printf( "%s\n", str );
	LEOCleanUpValue( &theValue, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringValue( (LEOValuePtr) &theValue, "this,that,more", &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 0, 2, "THISTHATMORE", &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	printf( "%s\n", str );
	LEOCleanUpValue( &theValue, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringValue( (LEOValuePtr) &theValue, "this,that,more", &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 1, 2, "THATMORE", &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	printf( "%s\n", str );
	LEOCleanUpValue( &theValue, &ctx );

	printf( "==== delete constant ====\n" );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringConstantValue( (LEOValuePtr) &theValue, "this,that,more", &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 0, 0, NULL, &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	printf( "%s\n", str );
	LEOCleanUpValue( &theValue, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringConstantValue( (LEOValuePtr) &theValue, "this,that,more", &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 1, 1, NULL, &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	printf( "%s\n", str );
	LEOCleanUpValue( &theValue, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringConstantValue( (LEOValuePtr) &theValue, "this,that,more", &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 2, 2, NULL, &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	printf( "%s\n", str );
	LEOCleanUpValue( &theValue, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringConstantValue( (LEOValuePtr) &theValue, "this,that,more", &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 0, 1, NULL, &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	printf( "%s\n", str );
	LEOCleanUpValue( &theValue, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringConstantValue( (LEOValuePtr) &theValue, "this,that,more", &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 0, 2, NULL, &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	printf( "%s\n", str );
	LEOCleanUpValue( &theValue, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringConstantValue( (LEOValuePtr) &theValue, "this,that,more", &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 1, 2, NULL, &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	printf( "%s\n", str );
	LEOCleanUpValue( &theValue, &ctx );
	
	printf( "==== delete dynamic ====\n" );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringValue( (LEOValuePtr) &theValue, "this,that,more", &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 0, 0, NULL, &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	printf( "%s\n", str );
	LEOCleanUpValue( &theValue, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringValue( (LEOValuePtr) &theValue, "this,that,more", &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 1, 1, NULL, &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	printf( "%s\n", str );
	LEOCleanUpValue( &theValue, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringValue( (LEOValuePtr) &theValue, "this,that,more", &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 2, 2, NULL, &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	printf( "%s\n", str );
	LEOCleanUpValue( &theValue, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringValue( (LEOValuePtr) &theValue, "this,that,more", &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 0, 1, NULL, &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	printf( "%s\n", str );
	LEOCleanUpValue( &theValue, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringValue( (LEOValuePtr) &theValue, "this,that,more", &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 0, 2, NULL, &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	printf( "%s\n", str );
	LEOCleanUpValue( &theValue, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOInitStringValue( (LEOValuePtr) &theValue, "this,that,more", &ctx );
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeItem, 1, 2, NULL, &ctx );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	printf( "%s\n", str );
	LEOCleanUpValue( &theValue, &ctx );
	
	LEOCleanUpContext( &ctx );
}


void	DoWordsTest( const char* theStr )
{
	size_t			outChunkStart, outChunkEnd,
					outDelChunkStart, outDelChunkEnd;

	LEOGetChunkRanges( theStr, kLEOChunkTypeWord,
					0, 0,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	LEOPrintStringWithRangeMarkers( theStr, outChunkStart, outChunkEnd );
	LEOPrintStringWithRangeMarkers( theStr, outDelChunkStart, outDelChunkEnd );

	LEOGetChunkRanges( theStr, kLEOChunkTypeWord,
					1, 1,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	LEOPrintStringWithRangeMarkers( theStr, outChunkStart, outChunkEnd );
	LEOPrintStringWithRangeMarkers( theStr, outDelChunkStart, outDelChunkEnd );

	LEOGetChunkRanges( theStr, kLEOChunkTypeWord,
					2, 2,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	LEOPrintStringWithRangeMarkers( theStr, outChunkStart, outChunkEnd );
	LEOPrintStringWithRangeMarkers( theStr, outDelChunkStart, outDelChunkEnd );

	LEOGetChunkRanges( theStr, kLEOChunkTypeWord,
					0, 1,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	LEOPrintStringWithRangeMarkers( theStr, outChunkStart, outChunkEnd );
	LEOPrintStringWithRangeMarkers( theStr, outDelChunkStart, outDelChunkEnd );

	LEOGetChunkRanges( theStr, kLEOChunkTypeWord,
					1, 2,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	LEOPrintStringWithRangeMarkers( theStr, outChunkStart, outChunkEnd );
	LEOPrintStringWithRangeMarkers( theStr, outDelChunkStart, outDelChunkEnd );

	LEOGetChunkRanges( theStr, kLEOChunkTypeWord,
					0, 2,
					&outChunkStart, &outChunkEnd,
					&outDelChunkStart, &outDelChunkEnd, ',' );
	LEOPrintStringWithRangeMarkers( theStr, outChunkStart, outChunkEnd );
	LEOPrintStringWithRangeMarkers( theStr, outDelChunkStart, outDelChunkEnd );
}


void	DoReferenceTest()
{
	LEOContext			ctx;
	union LEOValue		theValue;
	union LEOValue		originalValue;
	char				str[256];
	
	printf( "==== references ====\n" );
	
	LEOInitContext( &ctx );
	
	LEOInitStringConstantValue( &originalValue.base, "I am as real as it gets.", &ctx );
	LEOInitReferenceValue( &theValue.base, &originalValue.base, &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsString( &theValue, str, sizeof(str), &ctx );
	printf( "%s\n", str );

	memset( str, 'X', sizeof(str) );
	LEOGetValueAsRangeOfString( &theValue, kLEOChunkTypeCharacter, 0, 4,
								str, sizeof(str), &ctx );
	printf( "%s\n", str );
	LEOSetValueAsString( &theValue, "Even when set indirectly", &ctx );
	
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsString( &originalValue, str, sizeof(str), &ctx );
	printf( "%s\n", str );
	
	LEOSetValueRangeAsString( &theValue, kLEOChunkTypeCharacter, 14, 24,
								"by reference", &ctx );
	memset( str, 'X', sizeof(str) );
	LEOGetValueAsString( &originalValue, str, sizeof(str), &ctx );
	printf( "%s\n", str );
	
	LEOCleanUpValue( &originalValue.base, &ctx );
	LEOInitNumberValue( &originalValue.base, 42, &ctx );
	double	theNum = LEOGetValueAsNumber( &theValue, &ctx );
	printf( "%f\n", theNum );
	LEOSetValueAsNumber( &theValue, 11, &ctx );
	theNum = LEOGetValueAsNumber( &originalValue, &ctx );
	printf( "%f\n", theNum );
	
	LEOCleanUpValue( &originalValue.base, &ctx );
	LEOInitBooleanValue( &originalValue.base, true, &ctx );
	bool	theBool = LEOGetValueAsBoolean( &theValue, &ctx );
	printf( "%s\n", theBool ? "true" : "false" );
	
	LEOSetValueAsString( &theValue.base, "false", &ctx );
	theBool = LEOGetValueAsBoolean( &originalValue, &ctx );
	printf( "%s\n", theBool ? "true" : "false" );
	
	LEOCleanUpValue( &theValue, &ctx );
	LEOCleanUpValue( &originalValue, &ctx );
	
	LEOCleanUpContext( &ctx );
}


-(void)	applicationDidFinishLaunching: (NSNotification *)aNotification
{
	[[messageBoxField window] setBackgroundColor: [NSColor whiteColor]];
	
	#if RUN_TESTS
	DoChunkTests();
	DoChunkValueTests();
	DoReferenceTest();
	DoWordsTest( "this  that  more" );
	DoWordsTest( "  this  that  more  " );
	DoWordsTest( " this that more " );
	DoWordsTest( "this that more" );
	
	[NSApp terminate: self];
	#endif
		
	[busyIndicator startAnimation: self];
	
	// === start of stuff that a parser/compiler would generate:
	LEOInstruction		instructions[] =
	{
		{ PUSH_NUMBER_INSTR, 0, 10000 },			// Create our loop counter local var and init to 10'000 iterations.
		{ JUMP_RELATIVE_IF_LT_ZERO_INSTR, 0, 4 },	// 0 at BP-relative offset, our counter. Jump past this loop if counter goes below 0.
			{ PRINT_VALUE_INSTR, 0, 0 },				// Print counter.
			{ ADD_NUMBER_INSTR, 0, -1 },				// Subtract 1 from counter
		{ JUMP_RELATIVE_INSTR, 0, -3 },				// Jump back to loop condition.
		{ PUSH_STR_FROM_TABLE_INSTR, 0xffff, 1 },	// Get a string.
		{ PRINT_VALUE_INSTR, 0xffff, 0 },			// Output that string & pop off the stack.
		{ EXIT_TO_TOP_INSTR, 0, 0 },				// *** Exit, don't execute the following commands.
		{ PUSH_BOOLEAN_INSTR, 0, true },				// Create a boolean on the stack.
		{ ASSIGN_STRING_FROM_TABLE_INSTR, 0xffff, 2 },	// Assign a string to the last item on the stack (that's a boolean, so it'll fail).
		{ POP_VALUE_INSTR, 0, 0 },						// Remove the boolean from the stack again.
		{ INVALID_INSTR, 0, 0 },		// Directly produce effect of invalid instruction.
		{ 77, 9, 8 },					// Completely invalid.
	};
	const char*			strings[] =
	{
		"Hi, world!",
		"Top 'o the mornin' to ya, sir!",
		"I am the very model of a modern major-general",
		"Everybody loves you, honey!"
	};
	// === end of stuff that a parser/compiler would generate:
	
	#if PRINT_BYTECODE
	LEODebugPrintInstructions( instructions, sizeof(instructions) / sizeof(LEOInstruction) );
	#endif // PRINT_BYTECODE
	
	NSTimeInterval		startTime = [NSDate timeIntervalSinceReferenceDate];
	
	// --- Start of code to run some raw code:
	LEOContext			context;
	LEOInitContext( &context );
	context.stringsTable = strings;
	context.stringsTableSize = sizeof(strings) / sizeof(const char*);
	
	#if ACTIVATE_DEBUGGER
	context.preInstructionProc = LEODebuggerPreInstructionProc;	// Activate the debugger (not needed unless you want to debug).
	LEODebuggerAddBreakpoint( instructions );	// Set a breakpoint on the first instruction, so we can step through everything with the debugger.
	#endif // ACTIVATE_DEBUGGER
	LEORunInContext( instructions, &context );
	
	[busyIndicator stopAnimation: self];
	
	if( context.errMsg[0] != 0 )
	{
		LEODebugPrintContext( &context );
		NSRunAlertPanel( @"Script Aborted", @"%s", @"OK", @"", @"", context.errMsg );
	}
	
	[busyIndicator startAnimation: self];
	LEOCleanUpContext( &context );
	[busyIndicator stopAnimation: self];
	// --- End  of code to run some raw code:
	
	NSLog( @"Time taken: %f seconds.", [NSDate timeIntervalSinceReferenceDate] -startTime );
	

//	char				buf[1024];
//	LEOValueString		theStr;
//	LEOInitStringConstantValue( (LEOValuePtr)&theStr, "Top 'o the mornin' to ya!", &ctx );
//	LEOGetValueAsString( &theStr, buf, sizeof(buf) );
//	
//	NSLog( @"%s", buf );
}


-(void)	printMessage: (NSString*)inMessage
{
	static NSTimeInterval		lastUpdateTime = 0;
	
	NSWindow*	msgBox = [messageBoxField window];
	[messageBoxField setStringValue: inMessage];
	if( ![msgBox isVisible] )
		[msgBox makeKeyAndOrderFront: self];
	if( lastUpdateTime < [NSDate timeIntervalSinceReferenceDate] )
	{
		[messageBoxField displayIfNeeded];
		lastUpdateTime = [NSDate timeIntervalSinceReferenceDate] + 0.25;	// Update at most every quarter second.
	}
}

@end
