//
//  LEOPropertyInstructions.c
//  Forge
//
//  Created by Uli Kusterer on 10.04.11.
//  Copyright 2011 The Void Software. All rights reserved.
//

#include "LEOPropertyInstructions.h"


size_t			kFirstPropertyInstruction = 0;


struct THostCommandEntry	gPropertyHostFunctions[] =
{
	{
		EIIdentifier, I_HAVE_PROPERTY_INSTRUCTION, 0, 0, 'X',
		{
			{ EHostParamInvisibleIdentifier, EHaveIdentifier, EHostParameterRequired, INVALID_INSTR2, 0, 0, '\0', '\0' },
			{ EHostParamInvisibleIdentifier, ETheIdentifier, EHostParameterOptional, INVALID_INSTR2, 0, 0, '\0', '\0' },
			{ EHostParamInvisibleIdentifier, EPropertyIdentifier, EHostParameterRequired, INVALID_INSTR2, 0, 0, '\0', '\0' },
			{ EHostParamIdentifier, ELastIdentifier_Sentinel, EHostParameterRequired, INVALID_INSTR2, 0, 0, '\0', 'X' },
			{ EHostParam_Sentinel, ELastIdentifier_Sentinel, EHostParameterOptional, INVALID_INSTR2, 0, 0, '\0', '\0' },
		}
	},
	{
		ELastIdentifier_Sentinel, INVALID_INSTR2, 0, 0, '\0',
		{
			{ EHostParam_Sentinel, ELastIdentifier_Sentinel, EHostParameterOptional, INVALID_INSTR2, 0, 0, '\0', '\0' },
		}
	}
};


struct TOperatorEntry	gPropertyOperators[] =
{
	{ EHasIdentifier, EPropertyIdentifier, 2500, HAS_PROPERTY_INSTR, EHasPropertyOperator },
	{ ELastIdentifier_Sentinel, ELastIdentifier_Sentinel, 0, INVALID_INSTR2, ELastIdentifier_Sentinel }
};
