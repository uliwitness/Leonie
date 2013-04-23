//
//  LEOObjCCallInstructions.h
//  Stacksmith
//
//  Created by Uli Kusterer on 29.03.13.
//  Copyright 2013 The Void Software. All rights reserved.
//

#include "LEOInstructions.h"
#include "ForgeTypes.h"


enum
{
	CALL_OBJC_METHOD_INSTR,
	
	LEO_NUMBER_OF_OBJCCALL_INSTRUCTIONS
};


LEOINSTR_DECL(ObjCCall,LEO_NUMBER_OF_OBJCCALL_INSTRUCTIONS)

extern size_t					kFirstObjCCallInstruction;
