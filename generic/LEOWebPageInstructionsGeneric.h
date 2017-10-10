//
//  LEOWebPageInstructions.h
//  Forge
//
//  Created by Uli Kusterer on 28.05.17.
//  Copyright 2017 Uli Kusterer. All rights reserved.
//

#include "LEOInstructions.h"
#include "ForgeTypes.h"


enum
{
	LEO_HTML_ENCODED_INSTR = 0,
	LEO_MARKDOWN_TO_HTML_INSTR,
	LEO_OFFSET_FUNC_INSTR,
	
	LEO_NUMBER_OF_WEB_PAGE_INSTRUCTIONS
};


LEOINSTR_DECL(WebPage,LEO_NUMBER_OF_WEB_PAGE_INSTRUCTIONS)

extern size_t						kFirstWebPageInstruction;


extern struct TBuiltInFunctionEntry	gWebPageBuiltInFunctions[];
