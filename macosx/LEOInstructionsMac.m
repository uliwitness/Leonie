/*
 *  LEOInstructionsMac.m
 *  Leonie
 *
 *  Created by Uli Kusterer on 09.10.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#import "LeonieAppDelegate.h"
#import "LEOInterpreter.h"


void	LEOPrintInstruction( LEOContext* inContext )
{
	char				buf[1024] = { 0 };
	
	bool				popOffStack = (inContext->currentInstruction->param1 == 0xffff);
	union LEOValue*		theValue = popOffStack ? (inContext->stackEndPtr -1) : (inContext->stackBasePtr +inContext->currentInstruction->param1);
	LEOGetValueAsString( theValue, buf, sizeof(buf), inContext );
	NSString		*	theStr = [NSString stringWithUTF8String: buf];
	[(LeonieAppDelegate*)[NSApp delegate] printMessage: theStr];
	if( popOffStack )
		LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );
	
	inContext->currentInstruction++;
}
