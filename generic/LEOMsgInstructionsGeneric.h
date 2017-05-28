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


enum
{
	PRINT_VALUE_INSTR = 0,
	DELETE_VALUE_INSTR,
	
	LEO_NUMBER_OF_MSG_INSTRUCTIONS
};


LEOINSTR_DECL(Msg,LEO_NUMBER_OF_MSG_INSTRUCTIONS)

extern size_t						kFirstMsgInstruction;


#if __cplusplus
#include <ostream>

extern std::ostream* gLEOMsgOutputStream;
#endif // __cplusplus

#endif // LEO_MSG_INSTRUCTIONS_GENERIC_H
