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

/*!
	@header LEOHandlerID
	A data type used for identifying handlers without tedious string comparisons.
*/

#include <stdint.h>


/*!
	Every handler name maps to a handler ID. Since handler names are
	case-insisensitive, "mouseUp", "MOUSEUP" and "mouseup" would all map to the
	same handler ID.
	Not unlike a "selector" in Objective C.
*/
typedef size_t LEOHandlerID;


/*!
	Indicate that a handler ID variable has not been set yet.
*/
#define kLEOHandlerIDINVALID		SIZE_MAX



#endif // LEO_HANDLER_ID_H