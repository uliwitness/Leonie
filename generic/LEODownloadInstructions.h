//
//  LEODownloadInstructions.h
//  Stacksmith
//
//  Created by Uli Kusterer on 29.03.13.
//  Copyright 2013 The Void Software. All rights reserved.
//

#include "LEOInstructions.h"


enum
{
	DOWNLOAD_INSTR = 0,
	
	LEO_NUMBER_OF_DOWNLOAD_INSTRUCTIONS
};


LEOINSTR_DECL(Download,LEO_NUMBER_OF_DOWNLOAD_INSTRUCTIONS)

extern size_t					kFirstDownloadInstruction;

