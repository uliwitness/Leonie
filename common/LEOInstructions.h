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
#define PUSH_INTEGER_INSTR					16
#define ADD_NUMBER_INSTR					17
#define ADD_INTEGER_INSTR					18
#define CALL_HANDLER_INSTR					19
#define RETURN_FROM_HANDLER_INSTR			20
#define PUSH_REFERENCE_INSTR				21
#define PUSH_CHUNK_REFERENCE_INSTR			22
#define PARAMETER_INSTR						23
#define PARAMETER_COUNT_INSTR				24
#define SET_RETURN_VALUE_INSTR				25
#define PARAMETER_KEEPREFS_INSTR			26
#define CONCATENATE_VALUES_INSTR			27
#define AND_INSTR							28
#define OR_INSTR							29
#define CONCATENATE_VALUES_WITH_SPACE_INSTR	30
#define NEGATE_BOOL_INSTR					31
#define SUBTRACT_OPERATOR_INSTR				32
#define ADD_OPERATOR_INSTR					33
#define MULTIPLY_OPERATOR_INSTR				34
#define DIVIDE_OPERATOR_INSTR				35
#define GREATER_THAN_OPERATOR_INSTR			36
#define LESS_THAN_OPERATOR_INSTR			37
#define GREATER_THAN_EQUAL_OPERATOR_INSTR	38
#define LESS_THAN_EQUAL_OPERATOR_INSTR		39
#define NEGATE_NUMBER_INSTR					40
#define MODULO_OPERATOR_INSTR				41
#define POWER_OPERATOR_INSTR				42
#define EQUAL_OPERATOR_INSTR				43
#define NOT_EQUAL_OPERATOR_INSTR			44
#define LINE_MARKER_INSTR					45
#define ASSIGN_CHUNK_ARRAY_INSTR			46
#define GET_ARRAY_ITEM_INSTR				47
#define COUNT_CHUNKS_INSTR					48
#define GET_ARRAY_ITEM_COUNT_INSTR			49


// -----------------------------------------------------------------------------
//	Globals:
// -----------------------------------------------------------------------------

extern LEOInstructionFuncPtr	gInstructions[];
extern const char*				gInstructionNames[];
extern size_t					gNumInstructions;


#endif // LEO_INSTRUCTIONS_H
