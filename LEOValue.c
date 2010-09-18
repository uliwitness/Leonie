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


#pragma mark ISA v-tables
// -----------------------------------------------------------------------------
//	ISA v-tables for the subclasses:
// -----------------------------------------------------------------------------

struct LEOValueType	kLeoValueTypeNumber =
{
	sizeof(struct LEOValueNumber),
	
	LEOGetNumberValueAsNumber,
	LEOGetNumberValueAsString,
	LEOCantGetValueAsObjectID,
	LEOCantGetValueAsBoolean,
	
	LEOSetNumberValueAsNumber,
	LEOSetNumberValueAsString,
	LEOCantSetValueAsObjectID,
	LEOCantSetValueAsBoolean,
	
	LEOInitNumberValueCopy,
	
	LEOCleanUpNumberValue
};


struct LEOValueType	kLeoValueTypeString =
{
	sizeof(struct LEOValueString),
	
	LEOGetStringValueAsNumber,
	LEOGetStringValueAsString,
	LEOCantGetValueAsObjectID,
	LEOGetStringValueAsBoolean,
	
	LEOSetStringValueAsNumber,
	LEOSetStringValueAsString,
	LEOCantSetValueAsObjectID,
	LEOSetStringValueAsBoolean,
	
	LEOInitStringValueCopy,
	
	LEOCleanUpStringValue
};


struct LEOValueType	kLeoValueTypeStringConstant =
{
	sizeof(struct LEOValueString),
	
	LEOGetStringValueAsNumber,
	LEOGetStringValueAsString,
	LEOCantGetValueAsObjectID,
	LEOGetStringValueAsBoolean,
	
	LEOSetStringConstantValueAsNumber,
	LEOSetStringConstantValueAsString,
	LEOCantSetValueAsObjectID,
	LEOSetStringConstantValueAsBoolean,
	
	LEOInitStringConstantValueCopy,
	
	LEOCleanUpStringConstantValue
};




struct LEOValueType	kLeoValueTypeBoolean =
{
	sizeof(struct LEOValueBoolean),
	
	LEOCantGetValueAsNumber,
	LEOGetBooleanValueAsString,
	LEOCantGetValueAsObjectID,
	LEOGetBooleanValueAsBoolean,
	
	LEOCantSetValueAsNumber,
	LEOSetBooleanValueAsString,
	LEOCantSetValueAsObjectID,
	LEOSetBooleanValueAsBoolean,
	
	LEOInitBooleanValueCopy,
	
	LEOCleanUpBooleanValue
};



#pragma mark -
#pragma mark Shared


LEOObjectID LEOCantGetValueAsObjectID( LEOValuePtr self )
{
	printf( "\n*** Can't make this into an object ID ***\n\n" );
	
	return LEOObjectIDINVALID;
}


double	LEOCantGetValueAsNumber( LEOValuePtr self )
{
	printf( "\n*** Can't make this into a number ***\n\n" );
	
	return 0.0;
}


bool	LEOCantGetValueAsBoolean( LEOValuePtr self )
{
	printf( "\n*** Can't make this into a boolean ***\n\n" );
	
	return false;
}


void	LEOCantSetValueAsObjectID( LEOValuePtr self, LEOObjectID inObjectID )
{
	printf( "\n*** Can't take an object ID here ***\n\n" );
}


void	LEOCantSetValueAsNumber( LEOValuePtr self, double inNumber )
{
	printf( "\n*** Can't take a number here ***\n\n" );
}


void	LEOCantSetValueAsString( LEOValuePtr self, const char* inString )
{
	printf( "\n*** Can't take a string here ***\n\n" );
}


void	LEOCantSetValueAsBoolean( LEOValuePtr self, bool inState )
{
	printf( "\n*** Can't take a boolean here ***\n\n" );
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


void	LEOInitNumberValueCopy( LEOValuePtr self, LEOValuePtr dest )
{
	dest->isa = &kLeoValueTypeNumber;
	((struct LEOValueNumber*)dest)->number = ((struct LEOValueNumber*)self)->number;
}


void	LEOCleanUpNumberValue( LEOValuePtr self )
{
	self->isa = NULL;
	((struct LEOValueNumber*)self)->number = 0LL;
}


#pragma mark -
#pragma mark Dynamically allocated string

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


bool	LEOGetStringValueAsBoolean( LEOValuePtr self )
{
	if( strcasecmp( ((struct LEOValueString*)self)->string, "true" ) == 0 )
		return true;
	else if( strcasecmp( ((struct LEOValueString*)self)->string, "false" ) == 0 )
		return false;
	else
		return LEOCantGetValueAsBoolean( self );
}


void LEOSetStringValueAsString( LEOValuePtr self, const char* inString )
{
	if( ((struct LEOValueString*)self)->string )
		free( ((struct LEOValueString*)self)->string );
	long		theLen = strlen(inString) +1;
	((struct LEOValueString*)self)->string = malloc( theLen );
	strncpy( ((struct LEOValueString*)self)->string, inString, theLen );
}


void LEOSetStringValueAsBoolean( LEOValuePtr self, bool inBoolean )
{
	LEOSetStringValueAsStringConstant( self, inBoolean ? "true" : "false" );
}


void LEOSetStringValueAsStringConstant( LEOValuePtr self, const char* inString )
{
	if( ((struct LEOValueString*)self)->string )
		free( ((struct LEOValueString*)self)->string );
	
	self->isa = &kLeoValueTypeStringConstant;
	((struct LEOValueString*)self)->string = (char*) inString;
}


void	LEOInitStringValueCopy( LEOValuePtr self, LEOValuePtr dest )
{
	dest->isa = &kLeoValueTypeString;
	long		theLen = strlen(((struct LEOValueString*)self)->string) +1;
	((struct LEOValueString*)dest)->string = malloc( theLen );
	strncpy( ((struct LEOValueString*)dest)->string, ((struct LEOValueString*)self)->string, theLen );
}


void	LEOCleanUpStringValue( LEOValuePtr self )
{
	self->isa = NULL;
	if( ((struct LEOValueString*)self)->string )
		free( ((struct LEOValueString*)self)->string );
	((struct LEOValueString*)self)->string = NULL;
}


#pragma mark -
#pragma mark String Constant


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


void	LEOSetStringConstantValueAsBoolean( LEOValuePtr self, bool inBoolean )
{
	((struct LEOValueString*)self)->string = (inBoolean ? "true" : "false");
}


void	LEOInitStringConstantValueCopy( LEOValuePtr self, LEOValuePtr dest )
{
	dest->isa = &kLeoValueTypeStringConstant;
	((struct LEOValueString*)dest)->string = ((struct LEOValueString*)self)->string;
}


void	LEOCleanUpStringConstantValue( LEOValuePtr self )
{
	self->isa = NULL;
	((struct LEOValueString*)self)->string = NULL;
}


#pragma mark -
#pragma mark Boolean


void	LEOInitBooleanValue( LEOValuePtr self, bool inBoolean )
{
	self->isa = &kLeoValueTypeBoolean;
	((struct LEOValueBoolean*)self)->boolean = inBoolean;
}


void	LEOGetBooleanValueAsString( LEOValuePtr self, char* outBuf, long bufSize )
{
	strncpy( outBuf, ((struct LEOValueBoolean*)self)->boolean ? "true" : "false", bufSize -1 );
}


bool	LEOGetBooleanValueAsBoolean( LEOValuePtr self )
{
	return ((struct LEOValueBoolean*)self)->boolean;
}


void	LEOSetBooleanValueAsString( LEOValuePtr self, const char* inString )
{
	if( strcasecmp( inString, "true" ) == 0 )
		((struct LEOValueBoolean*)self)->boolean = true;
	else if( strcasecmp( inString, "false" ) == 0 )
		((struct LEOValueBoolean*)self)->boolean = false;
	else
		LEOCantSetValueAsString( self, inString );
}


void	LEOSetBooleanValueAsBoolean( LEOValuePtr self, bool inBoolean )
{
	((struct LEOValueBoolean*)self)->boolean = inBoolean;
}


void	LEOInitBooleanValueCopy( LEOValuePtr self, LEOValuePtr dest )
{
	dest->isa = &kLeoValueTypeBoolean;
	((struct LEOValueBoolean*)dest)->boolean = ((struct LEOValueBoolean*)self)->boolean;
}


void	LEOCleanUpBooleanValue( LEOValuePtr self )
{
	self->isa = NULL;
	((struct LEOValueBoolean*)self)->boolean = false;
}



