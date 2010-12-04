/*
 *  LEOInstructionsGeneric.c
 *  Leonie
 *
 *  Created by Uli Kusterer on 09.10.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

/*!
	@header LEOInstructionsGeneric
	These functions implement the actual instructions the Leonie bytecode
	interpreter actually understands. Or at least those that are not portable
	between platforms. These implementations are generic versions that try to
	rely on ANSI (or at least POSIX) ways of implementing functionality.
	
	Some may even simply abort and return an error because they're not available.
	This file should be suitable for implementing command line tools that run
	bytecode, or as a starting point for porting Leonie to a new platform or host.
*/

#include "LEOInterpreter.h"
#include <stdio.h>


/*!
	Pop a value off the back of the stack (or just read it from the given
	BasePointer-relative address) and present it to the user in string form.
	(PRINT_VALUE_INSTR)
	
	param1	-	If this is BACK_OF_STACK, we're supposed to pop the last item
				off the stack. Otherwise, this is a basePtr-relative address
				where a value will just be read.
*/

void	LEOPrintInstruction( LEOContext* inContext )
{
	char			buf[1024] = { 0 };
	
	bool			popOffStack = (inContext->currentInstruction->param1 == BACK_OF_STACK);
	union LEOValue*	theValue = popOffStack ? (inContext->stackEndPtr -1) : (inContext->stackBasePtr +inContext->currentInstruction->param1);
	LEOGetValueAsString( theValue, buf, sizeof(buf), inContext );
	printf( "%s\n", buf );
	if( popOffStack )
		LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );
	
	inContext->currentInstruction++;
}
