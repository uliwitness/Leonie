//
//  LEODownloadInstructions.h
//  Stacksmith
//
//  Created by Uli Kusterer on 29.03.13.
//  Copyright 2013 The Void Software. All rights reserved.
//

#include "LEOInstructions.h"
#include "ForgeTypes.h"


enum
{
	DOWNLOAD_INSTR = 0,
	PUSH_DOWNLOADS_INSTR,
	
	LEO_NUMBER_OF_DOWNLOAD_INSTRUCTIONS
};


#define NUM_DOWNLOAD_PROPERTIES		1


LEOINSTR_DECL(Download,LEO_NUMBER_OF_DOWNLOAD_INSTRUCTIONS)

extern LEOInstructionID				kFirstDownloadInstruction;

extern struct TGlobalPropertyEntry	gDownloadGlobalProperties[];
