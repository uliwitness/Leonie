//
//  LEOFileInstructions.h
//  Forge
//
//  Created by Uli Kusterer on 10.04.11.
//  Copyright 2011 The Void Software. All rights reserved.
//

#include "LEOInstructions.h"
#include "ForgeTypes.h"


enum
{
	WRITE_TO_FILE_INSTR = 0,
	READ_FROM_FILE_INSTR,
	
	LEO_NUMBER_OF_FILE_INSTRUCTIONS
};


LEOINSTR_DECL(File,LEO_NUMBER_OF_FILE_INSTRUCTIONS)

extern size_t						kFirstFileInstruction;


extern struct THostCommandEntry		gFileCommands[];
