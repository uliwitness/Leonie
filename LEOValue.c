/*
 *  LEOValue.c
 *  Leonie
 *
 *  Created by Uli Kusterer on 06.09.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

// -----------------------------------------------------------------------------
//	Headers:
// -----------------------------------------------------------------------------

#include "LEOValue.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


// -----------------------------------------------------------------------------
//	ISA v-tables for the subclasses:
// -----------------------------------------------------------------------------

struct LEOValueType	kLeoValueTypeNumber =
{
	LEOGetNumberValueAsNumber,
	LEOGetNumberValueAsString,
	LEOCantGetValueAsObjectID,
	LEOSetNumberValueAsNumber,
	LEOSetNumberValueAsString,
	LEOCantSetValueAsObjectID,
	LEOCleanUpNumberValue
};


struct LEOValueType	kLeoValueTypeString =
{
	LEOGetStringValueAsNumber,
	LEOGetStringValueAsString,
	LEOCantGetValueAsObjectID,
	LEOSetStringValueAsNumber,
	LEOSetStringValueAsString,
	LEOCantSetValueAsObjectID,
	LEOCleanUpStringValue
};


struct LEOValueType	kLeoValueTypeStringConstant =
{
	LEOGetStringValueAsNumber,
	LEOGetStringValueAsString,
	LEOCantGetValueAsObjectID,
	LEOSetStringConstantValueAsNumber,
	LEOSetStringConstantValueAsString,
	LEOCantSetValueAsObjectID,
	LEOCleanUpStringConstantValue
};


#pragma mark -
#pragma mark Shared


LEOObjectID LEOCantGetValueAsObjectID( LEOValuePtr self )
{
	return LEOObjectIDINVALID;
}


void	LEOCantSetValueAsObjectID( LEOValuePtr self, LEOObjectID inObjectID )
{
	
}


#pragma mark -
#pragma mark Number


void	LEOInitNumberValue( LEOValuePtr inStorage, double inNumber )
{
	inStorage->isa = &kLeoValueTypeNumber;
	((struct LEOValueNumber*)inStorage)->number = inNumber;
}


double LEOGetNumberValueAsNumber( LEOValuePtr self )
{
	return ((struct LEOValueNumber*)self)->number;
}


void LEOGetNumberValueAsString( LEOValuePtr self, char* outBuf, long bufSize )
{
	snprintf( outBuf, bufSize -1, "%f", ((struct LEOValueNumber*)self)->number );
}


void LEOSetNumberValueAsNumber( LEOValuePtr self, double inNumber )
{
	((struct LEOValueNumber*)self)->number = inNumber;
}


void LEOSetNumberValueAsString( LEOValuePtr self, const char* inNumber )
{
	((struct LEOValueNumber*)self)->number = strtod( inNumber, NULL );
}


void	LEOCleanUpNumberValue( LEOValuePtr self )
{
	self->isa = NULL;
	((struct LEOValueNumber*)self)->number = 0LL;
}


#pragma mark -
#pragma mark Number


void	LEOInitStringValue( LEOValuePtr inStorage, const char* inString )
{
	inStorage->isa = &kLeoValueTypeString;
	long		theLen = strlen(inString) +1;
	((struct LEOValueString*)inStorage)->string = malloc( theLen );
	strncpy( ((struct LEOValueString*)inStorage)->string, inString, theLen );
}


double	LEOGetStringValueAsNumber( LEOValuePtr self )
{
	return strtod( ((struct LEOValueString*)self)->string, NULL );
}


void	LEOGetStringValueAsString( LEOValuePtr self, char* outBuf, long bufSize )
{
	strncpy( outBuf, ((struct LEOValueString*)self)->string, bufSize );
}


void	LEOSetStringValueAsNumber( LEOValuePtr self, double inNumber )
{
	if( ((struct LEOValueString*)self)->string )
		free( ((struct LEOValueString*)self)->string );
	((struct LEOValueString*)self)->string = malloc( 40 );
	snprintf( ((struct LEOValueString*)self)->string, 40, "%f", inNumber );
}


void LEOSetStringValueAsString( LEOValuePtr self, const char* inString )
{
	if( ((struct LEOValueString*)self)->string )
		free( ((struct LEOValueString*)self)->string );
	long		theLen = strlen(inString) +1;
	((struct LEOValueString*)self)->string = malloc( theLen );
	strncpy( ((struct LEOValueString*)self)->string, inString, theLen );
}


void	LEOCleanUpStringValue( LEOValuePtr self )
{
	self->isa = NULL;
	if( ((struct LEOValueString*)self)->string )
		free( ((struct LEOValueString*)self)->string );
	((struct LEOValueString*)self)->string = NULL;
}


#pragma mark -
#pragma mark Number


void	LEOInitStringConstantValue( LEOValuePtr inStorage, const char* inString )
{
	inStorage->isa = &kLeoValueTypeStringConstant;
	((struct LEOValueString*)inStorage)->string = (char*)inString;
}


void	LEOSetStringConstantValueAsNumber( LEOValuePtr self, double inNumber )
{
	// Turn this into a non-constant string:
	self->isa = &kLeoValueTypeString;
	((struct LEOValueString*)self)->string = malloc( 40 );
	snprintf( ((struct LEOValueString*)self)->string, 40, "%f", inNumber );
}


void	LEOSetStringConstantValueAsString( LEOValuePtr self, const char* inString )
{
	// Turn this into a non-constant string:
	self->isa = &kLeoValueTypeString;
	long		theLen = strlen(inString) +1;
	((struct LEOValueString*)self)->string = malloc( theLen );
	strncpy( ((struct LEOValueString*)self)->string, inString, theLen );
}


void	LEOCleanUpStringConstantValue( LEOValuePtr self )
{
	self->isa = NULL;
	((struct LEOValueString*)self)->string = NULL;
}


