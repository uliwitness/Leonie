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


#if __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//	Constants:
// -----------------------------------------------------------------------------

enum
{
	INVALID_INSTR = 0,
	EXIT_TO_TOP_INSTR,
	NO_OP_INSTR,
	PUSH_STR_FROM_TABLE_INSTR,
	PUSH_UNSET_VALUE_INSTR,
	POP_VALUE_INSTR,
	PUSH_BOOLEAN_INSTR,
	ASSIGN_STRING_FROM_TABLE_INSTR,
	JUMP_RELATIVE_INSTR,
	JUMP_RELATIVE_IF_TRUE_INSTR,
	JUMP_RELATIVE_IF_FALSE_INSTR,
	JUMP_RELATIVE_IF_GT_ZERO_INSTR,
	JUMP_RELATIVE_IF_LT_ZERO_INSTR,
	JUMP_RELATIVE_IF_GT_SAME_ZERO_INSTR,
	JUMP_RELATIVE_IF_LT_SAME_ZERO_INSTR,
	PUSH_NUMBER_INSTR,
	PUSH_INTEGER_INSTR,
	PUSH_INTEGER_START_INSTR,	// First 32 of 64 bit integer, pushed on stack.
	ASSIGN_INTEGER_END_INSTR,	// Remaining 32 of 64 bit integer, merged into the last integer on the stack.
	ADD_NUMBER_INSTR,
	ADD_INTEGER_INSTR,
	CALL_HANDLER_INSTR,
	CLEAN_UP_HANDLER_STACK_INSTR,
	RETURN_FROM_HANDLER_INSTR,
	PUSH_REFERENCE_INSTR,
	PUSH_CHUNK_REFERENCE_INSTR,
	PARAMETER_INSTR,
	PARAMETER_COUNT_INSTR,
	SET_RETURN_VALUE_INSTR,
	PARAMETER_KEEPREFS_INSTR,
	PUSH_PARAMETERS_INSTR,
	CONCATENATE_VALUES_INSTR,
	AND_INSTR,
	OR_INSTR,
	CONCATENATE_VALUES_WITH_SPACE_INSTR,
	NEGATE_BOOL_INSTR,
	SUBTRACT_COMMAND_INSTR,
	ADD_COMMAND_INSTR,
	MULTIPLY_COMMAND_INSTR,
	DIVIDE_COMMAND_INSTR,
	SUBTRACT_OPERATOR_INSTR,
	ADD_OPERATOR_INSTR,
	MULTIPLY_OPERATOR_INSTR,
	DIVIDE_OPERATOR_INSTR,
	GREATER_THAN_OPERATOR_INSTR,
	LESS_THAN_OPERATOR_INSTR,
	GREATER_THAN_EQUAL_OPERATOR_INSTR,
	LESS_THAN_EQUAL_OPERATOR_INSTR,
	NEGATE_NUMBER_INSTR,
	MODULO_OPERATOR_INSTR,
	POWER_OPERATOR_INSTR,
	EQUAL_OPERATOR_INSTR,
	NOT_EQUAL_OPERATOR_INSTR,
	LINE_MARKER_INSTR,
	ASSIGN_CHUNK_ARRAY_INSTR,
	GET_ARRAY_ITEM_INSTR,
	COUNT_CHUNKS_INSTR,
	GET_ARRAY_ITEM_COUNT_INSTR,
	POP_SIMPLE_VALUE_INSTR,
	SET_STRING_INSTR,
	PUSH_CHUNK_INSTR,
	PUSH_ITEMDELIMITER_INSTR,
	SET_ITEMDELIMITER_INSTR,
	PUSH_GLOBAL_REFERENCE_INSTR,
	PUT_VALUE_INTO_VALUE_INSTR,
	PUSH_STR_VARIANT_FROM_TABLE_INSTR,
	NUM_TO_CHAR_INSTR,
	CHAR_TO_NUM_INSTR,
	NUM_TO_HEX_INSTR,
	HEX_TO_NUM_INSTR,
	NUM_TO_BINARY_INSTR,
	BINARY_TO_NUM_INSTR,
	SET_CHUNK_PROPERTY_INSTR,
	PUSH_CHUNK_PROPERTY_INSTR,
	PARSE_ERROR_INSTR,

	LEO_NUMBER_OF_INSTRUCTIONS	// MUST BE LAST.
};
// Possible LEOInstructionID values.


#define INVALID_INSTR2		UINT16_MAX


// param1 bits for CALL_HANDLER_INSTR:
enum eLEOCallHandlerFlags // at most uint16_t
{
	kLEOCallHandler_IsCommandFlag	= 0,		// For readability, you can "or" this in to indicate you're not passing IsFunctionFlag
	kLEOCallHandler_IsFunctionFlag	= (1 << 0),	// Otherwise we assume it's a command.
	kLEOCallHandler_PassMessage		= (1 << 1),	// Don't send the message to "me", start looking for a handler in the parent script.
};


// -----------------------------------------------------------------------------
//	Helper functions:
// -----------------------------------------------------------------------------

// For handling messages for which no handler exists:
void		LEOCleanUpHandlerParametersFromEndOfStack( LEOContext* inContext );
LEOValuePtr	LEOGetParameterAtIndexFromEndOfStack( LEOContext* inContext, LEOInteger paramIndex );


// -----------------------------------------------------------------------------
//	Globals:
// -----------------------------------------------------------------------------

extern struct LEOInstructionEntry*	gInstructions;
extern size_t						gNumInstructions;

#if __cplusplus
}
#endif

#endif // LEO_INSTRUCTIONS_H
