//
//  LEOObjCCallInstructions.h
//  Stacksmith
//
//  Created by Uli Kusterer on 29.03.13.
//  Copyright 2013 The Void Software. All rights reserved.
//

#ifndef LEO_OBJC_CALL_INSTRUCTIONS_H
#define LEO_OBJC_CALL_INSTRUCTIONS_H	1

#include "LEOInstructions.h"
#include "ForgeTypes.h"


#if __cplusplus
extern "C" {
#endif


enum
{
	CALL_OBJC_METHOD_INSTR,
	
	LEO_NUMBER_OF_OBJCCALL_INSTRUCTIONS
};


LEOINSTR_DECL(ObjCCall,LEO_NUMBER_OF_OBJCCALL_INSTRUCTIONS)

extern size_t					kFirstObjCCallInstruction;


#if __cplusplus
}
#endif

#endif /*LEO_OBJC_CALL_INSTRUCTIONS_H*/
