/*
 *  LEOMsgInstructionsMac.m
 *  Leonie
 *
 *  Created by Uli Kusterer on 09.10.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#import "LeonieAppDelegate.h"
#import "LEOInterpreter.h"
#import "LEOMsgInstructionsMac.h"


size_t					kFirstMsgInstruction = 0;


void	LEOPrintInstruction( LEOContext* inContext );


void	LEOPrintInstruction( LEOContext* inContext )
{
	char				buf[1024] = { 0 };
	
	bool				popOffStack = (inContext->currentInstruction->param1 == BACK_OF_STACK);
	union LEOValue*		theValue = popOffStack ? (inContext->stackEndPtr -1) : (inContext->stackBasePtr +inContext->currentInstruction->param1);
	LEOGetValueAsString( theValue, buf, sizeof(buf), inContext );
	NSString		*	theStr = [NSString stringWithUTF8String: buf];
	[(LeonieAppDelegate*)[NSApp delegate] printMessage: theStr];
	if( popOffStack )
		LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );
	
	inContext->currentInstruction++;
}


LEOINSTR_START(Msg,LEO_NUMBER_OF_MSG_INSTRUCTIONS)
LEOINSTR_LAST(LEOPrintInstruction)