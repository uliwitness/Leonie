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
#import <stdio.h>


@implementation LeonieAppDelegate

@synthesize window;
@synthesize busyIndicator;
@synthesize messageBoxField;

-(void)	applicationDidFinishLaunching: (NSNotification *)aNotification
{
	[[messageBoxField window] setBackgroundColor: [NSColor whiteColor]];
	
	[busyIndicator startAnimation: self];
	
	// === start of stuff that a parser/compiler would generate:
	LEOInstruction		instructions[] =
	{
		{ PUSH_NUMBER_INSTR, 0, 1000 },				// Create our loop counter local var and init to 1000 iterations.
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
	
	LEODebugPrintInstructions( instructions, sizeof(instructions) / sizeof(LEOInstruction) );
	
	NSTimeInterval		startTime = [NSDate timeIntervalSinceReferenceDate];
	
	// --- Start of code to run some raw code:
	LEOContext			context;
	LEOInitContext( &context );
	context.stringsTable = strings;
	context.stringsTableSize = sizeof(strings) / sizeof(const char*);
	
	context.preInstructionProc = LEODebuggerPreInstructionProc;	// Activate the debugger (not needed unless you want to debug).
	LEODebuggerAddBreakpoint( instructions );	// Set a breakpoint on the first instruction, so we can step through everything with the debugger.
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
//	LEOInitStringConstantValue( (LEOValuePtr)&theStr, "Top 'o the mornin' to ya!" );
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
