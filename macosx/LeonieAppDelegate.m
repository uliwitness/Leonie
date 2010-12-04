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
#import "LEOContextGroup.h"
#import "LEOScript.h"
#import <stdio.h>


#define ACTIVATE_DEBUGGER		0
#define PRINT_BYTECODE			1


@implementation LeonieAppDelegate

@synthesize window;
@synthesize busyIndicator;
@synthesize messageBoxField;


-(void)	applicationDidFinishLaunching: (NSNotification *)aNotification
{
	[[messageBoxField window] setBackgroundColor: [NSColor whiteColor]];
	
	[busyIndicator startAnimation: self];
	
	// --- Start of code to run some raw code:
	LEOContextGroup*	group = LEOContextGroupCreate();
	LEOContext			context;
	LEOInitContext( &context, group );
	LEOContextGroupRelease(group);	// Context retains it.

	// === start of stuff that a parser/compiler would generate:
	LEOScript*			script = LEOScriptCreateForOwner(0,0);	
	
	LEOHandlerID		startUpHandlerID = LEOContextGroupHandlerIDForHandlerName( group, "startUp" );	// Handler ID is like a SEL.
	LEOHandlerID		testMeHandlerID = LEOContextGroupHandlerIDForHandlerName( group, "testMe" );	// Handler ID is like a SEL.
	
#define NUM_LOOPS		100000
//#define NUM_LOOPS		5

	// Create one handler:
	LEOHandler*			startUpHandler = LEOScriptAddCommandHandlerWithID( script, startUpHandlerID );
	LEOHandlerAddInstruction( startUpHandler, PUSH_INTEGER_INSTR, 0, NUM_LOOPS );		// Create our loop counter local var and init to 10'000 iterations.
	LEOHandlerAddInstruction( startUpHandler, JUMP_RELATIVE_IF_LT_ZERO_INSTR, 0, 4 );	// 0 at BP-relative offset, our counter. Jump past this loop if counter goes below 0.
	LEOHandlerAddInstruction( startUpHandler, PRINT_VALUE_INSTR, 0, 0 );				// Print counter.
	LEOHandlerAddInstruction( startUpHandler, ADD_INTEGER_INSTR, 0, -1 );				// Subtract 1 from counter
	LEOHandlerAddInstruction( startUpHandler, JUMP_RELATIVE_INSTR, 0, -3 );				// Jump back to loop condition.
	LEOHandlerAddInstruction( startUpHandler, PUSH_STR_FROM_TABLE_INSTR, BACK_OF_STACK, LEOScriptAddString( script, "Top 'o the mornin' to ya, sir!" ) );	// Push a string as a parameter.
	LEOHandlerAddInstruction( startUpHandler, PUSH_INTEGER_INSTR, 0, 1 );				// Push a 1 on the stack (for the parameter count).
	LEOHandlerAddInstruction( startUpHandler, CALL_HANDLER_INSTR, 0, testMeHandlerID );	// Run the Handler.
	LEOHandlerAddInstruction( startUpHandler, POP_VALUE_INSTR, 0, 0 );					// Remove the parameter.
	LEOHandlerAddInstruction( startUpHandler, POP_VALUE_INSTR, 0, 0 );					// Remove the parameters count.
	LEOHandlerAddInstruction( startUpHandler, POP_VALUE_INSTR, 0, 0 );					// Remove our local counter variable.
	LEOHandlerAddInstruction( startUpHandler, RETURN_FROM_HANDLER_INSTR, 0, 0 );		// This handler is finished.
	
	LEOHandler*			testMeHandler = LEOScriptAddCommandHandlerWithID( script, testMeHandlerID );
	startUpHandler = LEOScriptFindCommandHandlerWithID( script, startUpHandlerID );		// Need to fetch again since adding testMeHandler may have invalidated the pointer.
	LEOHandlerAddInstruction( testMeHandler, PARAMETER_INSTR, BACK_OF_STACK, 1 );		// Push the first parameter on the stack.
	LEOHandlerAddInstruction( testMeHandler, PRINT_VALUE_INSTR, BACK_OF_STACK, 0 );		// Output that string & pop off the stack.
	LEOHandlerAddInstruction( testMeHandler, RETURN_FROM_HANDLER_INSTR, 0, 0 );			// This handler is finished.
	// === end of stuff that a parser/compiler would generate:
		
	#if PRINT_BYTECODE
	LEODebugPrintInstructions( startUpHandler->instructions, startUpHandler->numInstructions );
	LEODebugPrintInstructions( testMeHandler->instructions, testMeHandler->numInstructions );
	#endif // PRINT_BYTECODE
	
	NSTimeInterval		startTime = [NSDate timeIntervalSinceReferenceDate];
	
	#if ACTIVATE_DEBUGGER
	context.preInstructionProc = LEODebuggerPreInstructionProc;	// Activate the debugger (not needed unless you want to debug).
	LEODebuggerAddBreakpoint( startUpHandler->instructions );	// Set a breakpoint on the first instruction, so we can step through everything with the debugger.
	#endif // ACTIVATE_DEBUGGER
	
	LEOContextPushHandlerScriptReturnAddressAndBasePtr( &context, startUpHandler, script, NULL, NULL );	// NULL return address is same as exit to top. basePtr is set to NULL as well.
	LEORunInContext( startUpHandler->instructions, &context );
	
	#if ACTIVATE_DEBUGGER
	LEODebugPrintContext( &context );
	#endif
	
	[busyIndicator stopAnimation: self];
	
	if( context.errMsg[0] != 0 )
	{
		LEODebugPrintContext( &context );
		NSRunAlertPanel( @"Script Aborted", @"%s", @"OK", @"", @"", context.errMsg );
	}
	
	[busyIndicator startAnimation: self];
	LEOCleanUpContext( &context );
	LEOScriptRelease( script );
	[busyIndicator stopAnimation: self];
	// --- End  of code to run some raw code:
	
	NSLog( @"Time taken: %f seconds.", [NSDate timeIntervalSinceReferenceDate] -startTime );
}


-(void)	printMessage: (NSString*)inMessage
{
	NSWindow*	msgBox = [messageBoxField window];
	[messageBoxField setStringValue: inMessage];
	if( ![msgBox isVisible] )
		[msgBox makeKeyAndOrderFront: self];
	NSEvent	*	evt = [NSApp nextEventMatchingMask: NSAnyEventMask untilDate: [NSDate date] inMode: NSModalPanelRunLoopMode dequeue: YES];
	if( evt )
		[NSApp sendEvent: evt];
}

@end
