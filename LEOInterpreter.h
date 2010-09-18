/*
 *  LEOInterpreter.h
 *  Leonie
 *
 *  Created by Uli Kusterer on 17.09.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#ifndef LEO_INTERPRETER_H
#define LEO_INTERPRETER_H		1

#include "LEOValue.h"
#include <stdint.h>


#define LEO_STACK_SIZE			1024


typedef uint16_t	LEOInstructionID;	// Index into our instruction function array. Index 0 contains the "unimplemented" error exit function.


typedef struct
{
	LEOInstructionID		instructionID;
	uint16_t				param1;
	uint32_t				param2;
} LEOInstruction;


typedef struct
{
	LEOInstruction	*currentInstruction;	// PC
	union LEOValue	*stackBasePtr;			// BP
	union LEOValue	*stackEndPtr;			// SP (always points at element after last element)
	union LEOValue	stack[LEO_STACK_SIZE];	// The stack.
} LEOContext;


// All instructions are implemented as functions with the following signature:
//	Apart from branching instructions, every instruction has to increment the
//	inContext->currentInstruction so it'll advance to the next instruction. The
//	instruction functions are looked up in the gInstructions array.
typedef void (*LEOInstructionFuncPtr)( LEOContext* inContext );


void	LEOInitContext( LEOContext* theContext );
void	LEORunInContext( LEOInstruction instructions[], LEOContext *inContext );
void	LEOCleanUpStackToPtr( LEOContext* theContext, union LEOValue* lastItemToDelete );
void	LEOCleanUpContext( LEOContext* theContext );

#endif // LEO_INTERPRETER_H
