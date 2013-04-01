//
//  LEOGlobalProperties.h
//  Leonie
//
//  Created by Uli Kusterer on 16.04.11.
//  Copyright 2011 Uli Kusterer. All rights reserved.
//

#include "LEOInstructions.h"
#include "ForgeTypes.h"


enum
{
	SET_CURSOR_INSTR = 0,
	PUSH_CURSOR_INSTR,
	SET_VERSION_INSTR,
	PUSH_VERSION_INSTR,
	
	LEO_NUMBER_OF_GLOBAL_PROPERTY_INSTRUCTIONS
};


LEOINSTR_DECL(GlobalProperty,LEO_NUMBER_OF_GLOBAL_PROPERTY_INSTRUCTIONS)

extern size_t						kFirstGlobalPropertyInstruction;


extern struct TGlobalPropertyEntry	gHostGlobalProperties[(LEO_NUMBER_OF_GLOBAL_PROPERTY_INSTRUCTIONS / 2) +1];
