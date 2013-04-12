//
//  LEOMsgInstructions.h
//  Forge
//
//  Created by Uli Kusterer on 10.04.11.
//  Copyright 2011 The Void Software. All rights reserved.
//

#include "LEOInstructions.h"


enum
{
	PRINT_VALUE_INSTR = 0,
	
	LEO_NUMBER_OF_MSG_INSTRUCTIONS
};


LEOINSTR_DECL(Msg,LEO_NUMBER_OF_MSG_INSTRUCTIONS)

extern size_t						kFirstMsgInstruction;
