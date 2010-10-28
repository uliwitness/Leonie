/*
 *  LEOHandlerID.h
 *  Leonie
 *
 *  Created by Uli Kusterer on 28.10.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#ifndef LEO_HANDLER_ID_H
#define LEO_HANDLER_ID_H		1

#include <stdint.h>



typedef size_t LEOHandlerID;		// Every (case-insensitive) handler name maps to a handler ID. Kinda like a "selector" in Objective C.


#define LEOHandlerIDINVALID		SIZE_MAX



#endif // LEO_HANDLER_ID_H