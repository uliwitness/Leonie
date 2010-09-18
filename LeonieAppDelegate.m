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


@implementation LeonieAppDelegate

@synthesize window;


-(void)	applicationDidFinishLaunching: (NSNotification *)aNotification
{
	NSLog( @"Running script" );
	
	LEOInstruction		instructions[] =
	{
		{ PUSH_STR_FROM_TABLE_INSTR, 0, 1 },
		{ PRINT_VALUE_INSTR, 0, 0 },
		{ POP_VALUE_INSTR, 0, 0 },
		{ EXIT_TO_SHELL_INSTR, 0, 0 },
		{ ABNORMAL_EXIT_INSTR, 0, 0 },		// Directly produce effect of invalid instruction.
		{ 77, 9, 8 },						// Completely invalid.
	};
	
	LEOContext			context;
	LEOInitContext( &context );
	LEORunInContext( instructions, &context );
	LEOCleanUpContext( &context );
	
	NSLog( @"FINISHED Running script" );
//	char				buf[1024];
//	LEOValueString		theStr;
//	LEOInitStringConstantValue( (LEOValuePtr)&theStr, "Top 'o the mornin' to ya!" );
//	LEOGetValueAsString( &theStr, buf, sizeof(buf) );
//	
//	NSLog( @"%s", buf );
}

@end
