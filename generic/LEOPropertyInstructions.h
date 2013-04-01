//
//  LEOPropertyInstructions.h
//  Forge
//
//  Created by Uli Kusterer on 10.04.11.
//  Copyright 2011 The Void Software. All rights reserved.
//

#include "LEOInstructions.h"


enum
{
	PUSH_PROPERTY_OF_OBJECT_INSTR = 0,
	SET_PROPERTY_OF_OBJECT_INSTR,
	PUSH_ME_INSTR,
	
	LEO_NUMBER_OF_PROPERTY_INSTRUCTIONS
};


LEOINSTR_DECL(Property,LEO_NUMBER_OF_PROPERTY_INSTRUCTIONS)

extern size_t					kFirstPropertyInstruction;

