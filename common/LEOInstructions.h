/*
 *  LEOInstructions.h
 *  Leonie
 *
 *  Created by Uli Kusterer on 17.09.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#ifndef LEO_INSTRUCTIONS_H
#define LEO_INSTRUCTIONS_H		1

// -----------------------------------------------------------------------------
//	Headers:
// -----------------------------------------------------------------------------

#include "LEOInterpreter.h"
#include <sys/types.h>


// -----------------------------------------------------------------------------
//	Constants:
// -----------------------------------------------------------------------------

#define	INVALID_INSTR						0
#define	EXIT_TO_TOP_INSTR					1
#define	NO_OP_INSTR							2
#define	PUSH_STR_FROM_TABLE_INSTR			3
#define	PRINT_VALUE_INSTR					4
#define	POP_VALUE_INSTR						5
#define PUSH_BOOLEAN_INSTR					6
#define ASSIGN_STRING_FROM_TABLE_INSTR		7
#define JUMP_RELATIVE_INSTR					8
#define JUMP_RELATIVE_IF_TRUE_INSTR			9
#define JUMP_RELATIVE_IF_FALSE_INSTR		10
#define JUMP_RELATIVE_IF_GT_ZERO_INSTR		11
#define JUMP_RELATIVE_IF_LT_ZERO_INSTR		12
#define JUMP_RELATIVE_IF_GT_SAME_ZERO_INSTR	13
#define JUMP_RELATIVE_IF_LT_SAME_ZERO_INSTR	14
#define PUSH_NUMBER_INSTR					15
#define ADD_NUMBER_INSTR					16


// -----------------------------------------------------------------------------
//	Globals:
// -----------------------------------------------------------------------------

extern LEOInstructionFuncPtr	gInstructions[];
extern const char*				gInstructionNames[];
extern size_t					gNumInstructions;


#endif // LEO_INSTRUCTIONS_H
