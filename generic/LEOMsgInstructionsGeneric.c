/*
 *  LEOMsgInstructionsGeneric.c
 *  Leonie
 *
 *  Created by Uli Kusterer on 09.10.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

/*!
	@header LEOMsgInstructionsGeneric
	Generic implementation of console I/O instructions. Should work on all ANSI C
	systems.
*/

#include <stdio.h>
#include "LEOMsgInstructionsGeneric.h"
#include "LEOInterpreter.h"


size_t					kFirstMsgInstruction = 0;


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
	printf( "%s", buf );
	if( popOffStack )
		LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );
	
	inContext->currentInstruction++;
}


void	LEODeleteInstruction( LEOContext* inContext )
{
	union LEOValue*	theValue = inContext->stackEndPtr -1;
	LEOSetValueAsString( theValue, NULL, 0, inContext );
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );
	
	inContext->currentInstruction++;
}


LEOINSTR_START(Msg,LEO_NUMBER_OF_MSG_INSTRUCTIONS)
LEOINSTR(LEOPrintInstruction)
LEOINSTR_LAST(LEODeleteInstruction)

