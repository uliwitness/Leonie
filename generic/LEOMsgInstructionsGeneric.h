//
//  LEOMsgInstructions.h
//  Forge
//
//  Created by Uli Kusterer on 10.04.11.
//  Copyright 2011 The Void Software. All rights reserved.
//

#ifndef LEO_MSG_INSTRUCTIONS_GENERIC_H
#define LEO_MSG_INSTRUCTIONS_GENERIC_H

#include "LEOInstructions.h"

#if __cplusplus
extern "C" {
#endif

enum
{
	PRINT_VALUE_INSTR = 0,
	DELETE_VALUE_INSTR,
	START_RECORDING_OUTPUT_INSTR,
	STOP_RECORDING_OUTPUT_INSTR,
	
	LEO_NUMBER_OF_MSG_INSTRUCTIONS
};


LEOINSTR_DECL(Msg, LEO_NUMBER_OF_MSG_INSTRUCTIONS)

extern LEOInstructionID			kFirstMsgInstruction;


#if __cplusplus
}
#include <ostream>

extern "C" std::ostream* gLEOMsgOutputStream;
#endif // __cplusplus

#endif // LEO_MSG_INSTRUCTIONS_GENERIC_H
