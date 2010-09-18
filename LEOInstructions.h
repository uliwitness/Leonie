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

#include "LEOInterpreter.h"
#include <sys/types.h>


#define	INVALID_INSTR					0
#define	EXIT_TO_TOP_INSTR				1
#define	NO_OP_INSTR						2
#define	PUSH_STR_FROM_TABLE_INSTR		3
#define	PRINT_VALUE_INSTR				4
#define	POP_VALUE_INSTR					5
#define PUSH_BOOLEAN_INSTR				6
#define ASSIGN_STRING_FROM_TABLE_INSTR	7
#define JUMP_RELATIVE					8
#define JUMP_RELATIVE_IF_TRUE			9
#define JUMP_RELATIVE_IF_FALSE			10


extern LEOInstructionFuncPtr	gInstructions[];
extern size_t					gNumInstructions;


#endif // LEO_INSTRUCTIONS_H
