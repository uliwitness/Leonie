/*
 *  LEOUserData.h
 *  Leonie
 *
 *  Created by Uli Kusterer on 17.09.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#ifndef LEO_USER_DATA_H
#define LEO_USER_DATA_H

/*! Callback you can give to a context when you attach user data to it, which
 it will call when it is cleaned up to allow disposing of the user data. */
typedef void (*LEOUserDataCleanUpFuncPtr)( void* inUserData );

#endif // LEO_USER_DATA_H
