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
#include "LEOInterpreter.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define OTHER_VALUE_SHORT_STRING_MAX_LENGTH		256


#pragma mark ISA v-tables
// -----------------------------------------------------------------------------
//	ISA v-tables for the subclasses:
// -----------------------------------------------------------------------------

struct LEOValueType	kLeoValueTypeNumber =
{
	"number",
	sizeof(struct LEOValueNumber),
	
	LEOGetNumberValueAsNumber,
	LEOGetNumberValueAsString,
	LEOCantGetValueAsBoolean,
	LEOGetAnyValueAsRangeOfString,	// Only works as long as numbers can't be longer than OTHER_VALUE_SHORT_STRING_MAX_LENGTH as strings.
	
	LEOSetNumberValueAsNumber,
	LEOSetNumberValueAsString,
	LEOCantSetValueAsBoolean,
	LEOCantSetValueRangeAsString,
	
	LEOInitNumberValueCopy,
	
	LEOCleanUpNumberValue
};


struct LEOValueType	kLeoValueTypeString =
{
	"string",
	sizeof(struct LEOValueString),
	
	LEOGetStringValueAsNumber,
	LEOGetStringValueAsString,
	LEOGetStringValueAsBoolean,
	LEOGetStringValueAsRangeOfString,
	
	LEOSetStringValueAsNumber,
	LEOSetStringValueAsString,
	LEOSetStringValueAsBoolean,
	LEOSetStringValueRangeAsString,
	
	LEOInitStringValueCopy,
	
	LEOCleanUpStringValue
};


struct LEOValueType	kLeoValueTypeStringConstant =
{
	"string",
	sizeof(struct LEOValueString),
	
	LEOGetStringValueAsNumber,
	LEOGetStringValueAsString,
	LEOGetStringValueAsBoolean,
	LEOGetStringValueAsRangeOfString,
	
	LEOSetStringConstantValueAsNumber,
	LEOSetStringConstantValueAsString,
	LEOSetStringConstantValueAsBoolean,
	LEOSetStringConstantValueRangeAsString,
	
	LEOInitStringConstantValueCopy,
	
	LEOCleanUpStringConstantValue
};




struct LEOValueType	kLeoValueTypeBoolean =
{
	"boolean",
	sizeof(struct LEOValueBoolean),
	
	LEOCantGetValueAsNumber,
	LEOGetBooleanValueAsString,
	LEOGetBooleanValueAsBoolean,
	LEOGetAnyValueAsRangeOfString,	// Only works as long as booleans can't be longer than OTHER_VALUE_SHORT_STRING_MAX_LENGTH as strings.
	
	LEOCantSetValueAsNumber,
	LEOSetBooleanValueAsString,
	LEOSetBooleanValueAsBoolean,
	LEOCantSetValueRangeAsString,
	
	LEOInitBooleanValueCopy,
	
	LEOCleanUpBooleanValue
};


struct LEOValueType	kLeoValueTypeReference =
{
	"reference",
	sizeof(struct LEOValueReference),
	
	LEOGetReferenceValueAsNumber,
	LEOGetReferenceValueAsString,
	LEOGetReferenceValueAsBoolean,
	LEOGetReferenceValueAsRangeOfString,
	
	LEOSetReferenceValueAsNumber,
	LEOSetReferenceValueAsString,
	LEOSetReferenceValueAsBoolean,
	LEOSetReferenceValueRangeAsString,
	
	LEOInitReferenceValueCopy,
	
	LEOCleanUpReferenceValue
};


#define LEOReferencesTableChunkSize			16


LEOObjectID	LEOGetFreeObjectID( struct LEOContext* inContext )
{
	if( inContext->references == NULL )
	{
		inContext->numReferences = LEOReferencesTableChunkSize;
		inContext->references = calloc( inContext->numReferences, sizeof(struct LEOObject) );
		
		return 0;	// Can start with first item right away.
	}
	else
	{
		// +++ Optimize: remember the last one we cleared or returned or so and start scanning there.
		
		for( size_t x = 0; x < inContext->numReferences; x++ )
		{
			if( inContext->references[x].value == NULL )	// Unused slot!
				return x;
		}
		
		// Only get here if there's no free slots left:
		size_t		oldNumReferences = inContext->numReferences;
		inContext->numReferences += LEOReferencesTableChunkSize;
		inContext->references = realloc( inContext->references, sizeof(struct LEOObject) * inContext->numReferences );
		memset( inContext->references +(oldNumReferences * sizeof(struct LEOObject)), 0, LEOReferencesTableChunkSize * sizeof(struct LEOObject) );
		
		return oldNumReferences;	// Same as index of first new item.
	}
}


void	LEOReturnObjectID( LEOObjectID inObjectID, struct LEOContext* inContext )
{
	inContext->references[inObjectID].value = NULL;
	inContext->references[inObjectID].seed += 1;	// Make sure that if this is reused, whoever still references it knows it's gone.
}


LEOValuePtr	LEOGetValueForObjectIDAndSeed( LEOObjectID inObjectID, LEOObjectSeed inObjectSeed, struct LEOContext* inContext )
{
	if( inContext->references[inObjectID].seed != inObjectSeed )
		return NULL;
	
	return inContext->references[inObjectID].value;
}


#pragma mark -
#pragma mark Shared


double	LEOCantGetValueAsNumber( LEOValuePtr self, struct LEOContext* inContext )
{
	snprintf( inContext->errMsg, sizeof(inContext->errMsg), "Can't make %s into a number", self->isa->displayTypeName );
	inContext->keepRunning = false;
	
	return 0.0;
}


bool	LEOCantGetValueAsBoolean( LEOValuePtr self, struct LEOContext* inContext )
{
	snprintf( inContext->errMsg, sizeof(inContext->errMsg), "Can't make %s into a boolean", self->isa->displayTypeName );
	inContext->keepRunning = false;
	
	return false;
}


void	LEOCantSetValueAsNumber( LEOValuePtr self, double inNumber, struct LEOContext* inContext )
{
	snprintf( inContext->errMsg, sizeof(inContext->errMsg), "Expected %s, found number", self->isa->displayTypeName );
	inContext->keepRunning = false;
}


void	LEOCantSetValueAsString( LEOValuePtr self, const char* inString, struct LEOContext* inContext )
{
	snprintf( inContext->errMsg, sizeof(inContext->errMsg), "Expected %s, found string", self->isa->displayTypeName );
	inContext->keepRunning = false;
}


void	LEOCantSetValueAsBoolean( LEOValuePtr self, bool inState, struct LEOContext* inContext )
{
	snprintf( inContext->errMsg, sizeof(inContext->errMsg), "Expected %s, found boolean", self->isa->displayTypeName );
	inContext->keepRunning = false;
}


void	LEOCantSetValueRangeAsString( LEOValuePtr self, LEOChunkType inType,
									size_t inRangeStart, size_t inRangeEnd,
									const char* inBuf, struct LEOContext* inContext )
{
	snprintf( inContext->errMsg, sizeof(inContext->errMsg), "Expected %s, found string", self->isa->displayTypeName );
	inContext->keepRunning = false;
}


void	LEOGetAnyValueAsRangeOfString( LEOValuePtr self, LEOChunkType inType,
									size_t inRangeStart, size_t inRangeEnd,
									char* outBuf, long bufSize, struct LEOContext* inContext )
{
	char		str[OTHER_VALUE_SHORT_STRING_MAX_LENGTH] = {0};	// Can get away with this as long as they're only numbers, booleans etc.
	size_t		outChunkStart = 0,
				outChunkEnd = 0,
				outDelChunkStart = 0,
				outDelChunkEnd = 0;
	LEOGetValueAsString( self, str, sizeof(str), inContext );
	LEOGetChunkRanges( str, inType, inRangeStart, inRangeEnd,
						&outChunkStart, &outChunkEnd,
						&outDelChunkStart, &outDelChunkEnd, inContext->itemDelimiter );
	size_t len = outChunkEnd -outChunkStart;
	if( len > bufSize )
		len = bufSize -1;
	memmove( outBuf, str +outChunkStart, len );
	outBuf[len] = 0;
}


void	LEOInitReferenceValue( LEOValuePtr self, LEOValuePtr originalValue, struct LEOContext* inContext )
{
	self->isa = &kLeoValueTypeReference;
	self->refObjectID = LEOObjectIDINVALID;
	
	if( originalValue->refObjectID == LEOObjectIDINVALID )
	{
		originalValue->refObjectID = LEOGetFreeObjectID( inContext );
		inContext->references[originalValue->refObjectID].value = originalValue;
	}
	
	((LEOValueReference*)self)->objectID = originalValue->refObjectID;
	((LEOValueReference*)self)->objectSeed = inContext->references[originalValue->refObjectID].seed;
}


#pragma mark -
#pragma mark Number


void	LEOInitNumberValue( LEOValuePtr inStorage, double inNumber, struct LEOContext* inContext )
{
	inStorage->isa = &kLeoValueTypeNumber;
	inStorage->refObjectID = LEOObjectIDINVALID;
	((struct LEOValueNumber*)inStorage)->number = inNumber;
}


double LEOGetNumberValueAsNumber( LEOValuePtr self, struct LEOContext* inContext )
{
	return ((struct LEOValueNumber*)self)->number;
}


void LEOGetNumberValueAsString( LEOValuePtr self, char* outBuf, long bufSize, struct LEOContext* inContext )
{
	snprintf( outBuf, bufSize -1, "%g", ((struct LEOValueNumber*)self)->number );
}


void LEOSetNumberValueAsNumber( LEOValuePtr self, double inNumber, struct LEOContext* inContext )
{
	((struct LEOValueNumber*)self)->number = inNumber;
}


void LEOSetNumberValueAsString( LEOValuePtr self, const char* inNumber, struct LEOContext* inContext )
{
	char*		endPtr = NULL;
	double		theNum = strtod( inNumber, &endPtr );
	if( endPtr != (inNumber +strlen(inNumber)) )
		LEOCantSetValueAsString( self, inNumber, inContext );
	else
		((struct LEOValueNumber*)self)->number = theNum;
}


void	LEOInitNumberValueCopy( LEOValuePtr self, LEOValuePtr dest, struct LEOContext* inContext )
{
	dest->isa = &kLeoValueTypeNumber;
	dest->refObjectID = LEOObjectIDINVALID;
	((struct LEOValueNumber*)dest)->number = ((struct LEOValueNumber*)self)->number;
}


void	LEOCleanUpNumberValue( LEOValuePtr self, struct LEOContext* inContext )
{
	self->isa = NULL;
	((struct LEOValueNumber*)self)->number = 0LL;
	if( self->refObjectID != LEOObjectIDINVALID )	// We have references? Make sure they all notice we've gone if they try to access us from now on.
		LEOReturnObjectID( self->refObjectID, inContext );
}


#pragma mark -
#pragma mark Dynamically allocated string

void	LEOInitStringValue( LEOValuePtr inStorage, const char* inString, struct LEOContext* inContext )
{
	inStorage->isa = &kLeoValueTypeString;
	inStorage->refObjectID = LEOObjectIDINVALID;
	long		theLen = strlen(inString) +1;
	((struct LEOValueString*)inStorage)->string = malloc( theLen );
	strncpy( ((struct LEOValueString*)inStorage)->string, inString, theLen );
}


double	LEOGetStringValueAsNumber( LEOValuePtr self, struct LEOContext* inContext )
{
	return strtod( ((struct LEOValueString*)self)->string, NULL );
}


void	LEOGetStringValueAsString( LEOValuePtr self, char* outBuf, long bufSize, struct LEOContext* inContext )
{
	strncpy( outBuf, ((struct LEOValueString*)self)->string, bufSize );
}


void	LEOSetStringValueAsNumber( LEOValuePtr self, double inNumber, struct LEOContext* inContext )
{
	if( ((struct LEOValueString*)self)->string )
		free( ((struct LEOValueString*)self)->string );
	((struct LEOValueString*)self)->string = malloc( 40 );
	snprintf( ((struct LEOValueString*)self)->string, 40, "%f", inNumber );
}


bool	LEOGetStringValueAsBoolean( LEOValuePtr self, struct LEOContext* inContext )
{
	if( strcasecmp( ((struct LEOValueString*)self)->string, "true" ) == 0 )
		return true;
	else if( strcasecmp( ((struct LEOValueString*)self)->string, "false" ) == 0 )
		return false;
	else
		return LEOCantGetValueAsBoolean( self, inContext );
}


void	LEOGetStringValueAsRangeOfString( LEOValuePtr self, LEOChunkType inType,
											size_t inRangeStart, size_t inRangeEnd,
											char* outBuf, long bufSize, struct LEOContext* inContext )
{
	size_t		outChunkStart = 0,
				outChunkEnd = 0,
				outDelChunkStart = 0,
				outDelChunkEnd = 0;
	LEOGetChunkRanges( ((struct LEOValueString*)self)->string, inType,
						inRangeStart, inRangeEnd,
						&outChunkStart, &outChunkEnd,
						&outDelChunkStart, &outDelChunkEnd, inContext->itemDelimiter );
	size_t		len = outChunkEnd -outChunkStart;
	if( len > bufSize )
		len = bufSize -1;
	memmove( outBuf, ((struct LEOValueString*)self)->string +outChunkStart, len );
	outBuf[len] = 0;
}



void LEOSetStringValueAsString( LEOValuePtr self, const char* inString, struct LEOContext* inContext )
{
	if( ((struct LEOValueString*)self)->string )
		free( ((struct LEOValueString*)self)->string );
	long		theLen = strlen(inString) +1;
	((struct LEOValueString*)self)->string = malloc( theLen );
	strncpy( ((struct LEOValueString*)self)->string, inString, theLen );
}


void LEOSetStringValueAsBoolean( LEOValuePtr self, bool inBoolean, struct LEOContext* inContext )
{
	LEOSetStringValueAsStringConstant( self, (inBoolean ? "true" : "false"), inContext );
}


void LEOSetStringValueAsStringConstant( LEOValuePtr self, const char* inString, struct LEOContext* inContext )
{
	if( ((struct LEOValueString*)self)->string )
		free( ((struct LEOValueString*)self)->string );
	
	self->isa = &kLeoValueTypeStringConstant;
	((struct LEOValueString*)self)->string = (char*) inString;
}


void	LEOInitStringValueCopy( LEOValuePtr self, LEOValuePtr dest, struct LEOContext* inContext )
{
	dest->isa = &kLeoValueTypeString;
	dest->refObjectID = LEOObjectIDINVALID;
	long		theLen = strlen(((struct LEOValueString*)self)->string) +1;
	((struct LEOValueString*)dest)->string = malloc( theLen );
	strncpy( ((struct LEOValueString*)dest)->string, ((struct LEOValueString*)self)->string, theLen );
}


void	LEOCleanUpStringValue( LEOValuePtr self, struct LEOContext* inContext )
{
	self->isa = NULL;
	if( ((struct LEOValueString*)self)->string )
		free( ((struct LEOValueString*)self)->string );
	((struct LEOValueString*)self)->string = NULL;
	if( self->refObjectID != LEOObjectIDINVALID )
		LEOReturnObjectID( self->refObjectID, inContext );
}


void	LEOSetStringValueRangeAsString( LEOValuePtr self, LEOChunkType inType,
											size_t inRangeStart, size_t inRangeEnd,
											const char* inBuf, struct LEOContext* inContext )
{
	size_t		outChunkStart = 0,
				outChunkEnd = 0,
				outDelChunkStart = 0,
				outDelChunkEnd = 0,
				inBufLen = inBuf ? strlen(inBuf) : 0,
				selfLen = strlen( ((struct LEOValueString*)self)->string ),
				finalLen = 0;
	LEOGetChunkRanges( ((struct LEOValueString*)self)->string, inType,
						inRangeStart, inRangeEnd,
						&outChunkStart, &outChunkEnd,
						&outDelChunkStart, &outDelChunkEnd, inContext->itemDelimiter );
	if( !inBuf )	// NULL string means 'delete'.
	{
		outChunkStart = outDelChunkStart;
		outChunkEnd = outDelChunkEnd;
	}
	size_t		chunkLen = outChunkEnd -outChunkStart;
	finalLen = selfLen -chunkLen +inBufLen;
		
	char*		newStr = malloc( finalLen +1 );
	memmove( newStr, ((struct LEOValueString*)self)->string, outChunkStart );	// Copy before chunk.
	if( inBufLen > 0 )
		memmove( newStr +outChunkStart, inBuf, inBufLen );	// Copy new value of chunk.
	memmove( newStr +outChunkStart +inBufLen, ((struct LEOValueString*)self)->string +outChunkEnd, selfLen -outChunkEnd );	// Copy after chunk.
	newStr[finalLen] = 0;
	
	free( ((struct LEOValueString*)self)->string );
	((struct LEOValueString*)self)->string = newStr;
}


#pragma mark -
#pragma mark String Constant


void	LEOInitStringConstantValue( LEOValuePtr inStorage, const char* inString, struct LEOContext* inContext )
{
	inStorage->isa = &kLeoValueTypeStringConstant;
	inStorage->refObjectID = LEOObjectIDINVALID;
	((struct LEOValueString*)inStorage)->string = (char*)inString;
}


void	LEOSetStringConstantValueAsNumber( LEOValuePtr self, double inNumber, struct LEOContext* inContext )
{
	// Turn this into a non-constant string:
	self->isa = &kLeoValueTypeString;
	((struct LEOValueString*)self)->string = malloc( 40 );
	snprintf( ((struct LEOValueString*)self)->string, 40, "%f", inNumber );
}


void	LEOSetStringConstantValueAsString( LEOValuePtr self, const char* inString, struct LEOContext* inContext )
{
	// Turn this into a non-constant string:
	self->isa = &kLeoValueTypeString;
	long		theLen = strlen(inString) +1;
	((struct LEOValueString*)self)->string = malloc( theLen );
	strncpy( ((struct LEOValueString*)self)->string, inString, theLen );
}


void	LEOSetStringConstantValueAsBoolean( LEOValuePtr self, bool inBoolean, struct LEOContext* inContext )
{
	((struct LEOValueString*)self)->string = (inBoolean ? "true" : "false");
}


void	LEOInitStringConstantValueCopy( LEOValuePtr self, LEOValuePtr dest, struct LEOContext* inContext )
{
	dest->isa = &kLeoValueTypeStringConstant;
	dest->refObjectID = LEOObjectIDINVALID;
	((struct LEOValueString*)dest)->string = ((struct LEOValueString*)self)->string;
}


void	LEOCleanUpStringConstantValue( LEOValuePtr self, struct LEOContext* inContext )
{
	self->isa = NULL;
	((struct LEOValueString*)self)->string = NULL;
	if( self->refObjectID != LEOObjectIDINVALID )
		LEOReturnObjectID( self->refObjectID, inContext );
}


void	LEOSetStringConstantValueRangeAsString( LEOValuePtr self, LEOChunkType inType,
												size_t inRangeStart, size_t inRangeEnd,
												const char* inBuf, struct LEOContext* inContext )
{
	size_t		outChunkStart = 0,
				outChunkEnd = 0,
				outDelChunkStart = 0,
				outDelChunkEnd = 0,
				inBufLen = inBuf ? strlen(inBuf) : 0,
				selfLen = strlen( ((struct LEOValueString*)self)->string ),
				finalLen = 0;
	LEOGetChunkRanges( ((struct LEOValueString*)self)->string, inType,
						inRangeStart, inRangeEnd,
						&outChunkStart, &outChunkEnd,
						&outDelChunkStart, &outDelChunkEnd, inContext->itemDelimiter );
	if( !inBuf )	// NULL string means 'delete'.
	{
		outChunkStart = outDelChunkStart;
		outChunkEnd = outDelChunkEnd;
	}
	size_t		chunkLen = outChunkEnd -outChunkStart;
	finalLen = selfLen -chunkLen +inBufLen;
		
	char*		newStr = malloc( finalLen +1 );
	memmove( newStr, ((struct LEOValueString*)self)->string, outChunkStart );	// Copy before chunk.
	if( inBufLen > 0 )
		memmove( newStr +outChunkStart, inBuf, inBufLen );	// Copy new value of chunk.
	memmove( newStr +outChunkStart +inBufLen, ((struct LEOValueString*)self)->string +outChunkEnd, selfLen -outChunkEnd );	// Copy after chunk.
	newStr[finalLen] = 0;
	
	// Turn this into a non-constant string:
	self->isa = &kLeoValueTypeString;
	((struct LEOValueString*)self)->string = newStr;
}


#pragma mark -
#pragma mark Boolean


void	LEOInitBooleanValue( LEOValuePtr self, bool inBoolean, struct LEOContext* inContext )
{
	self->isa = &kLeoValueTypeBoolean;
	self->refObjectID = LEOObjectIDINVALID;
	((struct LEOValueBoolean*)self)->boolean = inBoolean;
}


void	LEOGetBooleanValueAsString( LEOValuePtr self, char* outBuf, long bufSize, struct LEOContext* inContext )
{
	strncpy( outBuf, ((struct LEOValueBoolean*)self)->boolean ? "true" : "false", bufSize -1 );
}


bool	LEOGetBooleanValueAsBoolean( LEOValuePtr self, struct LEOContext* inContext )
{
	return ((struct LEOValueBoolean*)self)->boolean;
}


void	LEOSetBooleanValueAsString( LEOValuePtr self, const char* inString, struct LEOContext* inContext )
{
	if( strcasecmp( inString, "true" ) == 0 )
		((struct LEOValueBoolean*)self)->boolean = true;
	else if( strcasecmp( inString, "false" ) == 0 )
		((struct LEOValueBoolean*)self)->boolean = false;
	else
		LEOCantSetValueAsString( self, inString, inContext );
}


void	LEOSetBooleanValueAsBoolean( LEOValuePtr self, bool inBoolean, struct LEOContext* inContext )
{
	((struct LEOValueBoolean*)self)->boolean = inBoolean;
}


void	LEOInitBooleanValueCopy( LEOValuePtr self, LEOValuePtr dest, struct LEOContext* inContext )
{
	dest->isa = &kLeoValueTypeBoolean;
	dest->refObjectID = LEOObjectIDINVALID;
	((struct LEOValueBoolean*)dest)->boolean = ((struct LEOValueBoolean*)self)->boolean;
}


void	LEOCleanUpBooleanValue( LEOValuePtr self, struct LEOContext* inContext )
{
	self->isa = NULL;
	((struct LEOValueBoolean*)self)->boolean = false;
	if( self->refObjectID != LEOObjectIDINVALID )
		LEOReturnObjectID( self->refObjectID, inContext );
}



#pragma mark -
#pragma mark Reference


void	LEOGetReferenceValueAsString( LEOValuePtr self, char* outBuf, long bufSize, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOGetValueForObjectIDAndSeed( ((struct LEOValueReference*)self)->objectID, ((struct LEOValueReference*)self)->objectSeed, inContext );
	if( theValue == NULL )
	{
		snprintf( inContext->errMsg, sizeof(inContext->errMsg) -1, "The referenced value doesn't exist anymore." );
		inContext->keepRunning = false;
	}
	else
		LEOGetValueAsString( theValue, outBuf, bufSize, inContext );
}


double	LEOGetReferenceValueAsNumber( LEOValuePtr self, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOGetValueForObjectIDAndSeed( ((struct LEOValueReference*)self)->objectID, ((struct LEOValueReference*)self)->objectSeed, inContext );
	if( theValue == NULL )
	{
		snprintf( inContext->errMsg, sizeof(inContext->errMsg) -1, "The referenced value doesn't exist anymore." );
		inContext->keepRunning = false;
		return 0.0;
	}
	else
		return LEOGetValueAsNumber( theValue, inContext );
}


bool	LEOGetReferenceValueAsBoolean( LEOValuePtr self, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOGetValueForObjectIDAndSeed( ((struct LEOValueReference*)self)->objectID, ((struct LEOValueReference*)self)->objectSeed, inContext );
	if( theValue == NULL )
	{
		snprintf( inContext->errMsg, sizeof(inContext->errMsg) -1, "The referenced value doesn't exist anymore." );
		inContext->keepRunning = false;
		return false;
	}
	else
		return LEOGetValueAsBoolean( theValue, inContext );
}


void	LEOGetReferenceValueAsRangeOfString( LEOValuePtr self, LEOChunkType inType,
									size_t inRangeStart, size_t inRangeEnd,
									char* outBuf, long bufSize, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOGetValueForObjectIDAndSeed( ((struct LEOValueReference*)self)->objectID, ((struct LEOValueReference*)self)->objectSeed, inContext );
	if( theValue == NULL )
	{
		snprintf( inContext->errMsg, sizeof(inContext->errMsg) -1, "The referenced value doesn't exist anymore." );
		inContext->keepRunning = false;
	}
	else
		LEOGetValueAsRangeOfString( theValue, inType, inRangeStart, inRangeEnd, outBuf, bufSize, inContext );
}


void	LEOSetReferenceValueAsString( LEOValuePtr self, const char* inString, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOGetValueForObjectIDAndSeed( ((struct LEOValueReference*)self)->objectID, ((struct LEOValueReference*)self)->objectSeed, inContext );
	if( theValue == NULL )
	{
		snprintf( inContext->errMsg, sizeof(inContext->errMsg) -1, "The referenced value doesn't exist anymore." );
		inContext->keepRunning = false;
	}
	else
		LEOSetValueAsString( theValue, inString, inContext );
}


void	LEOSetReferenceValueAsBoolean( LEOValuePtr self, bool inBoolean, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOGetValueForObjectIDAndSeed( ((struct LEOValueReference*)self)->objectID, ((struct LEOValueReference*)self)->objectSeed, inContext );
	if( theValue == NULL )
	{
		snprintf( inContext->errMsg, sizeof(inContext->errMsg) -1, "The referenced value doesn't exist anymore." );
		inContext->keepRunning = false;
	}
	else
		LEOSetValueAsBoolean( theValue, inBoolean, inContext );
}


void	LEOSetReferenceValueAsNumber( LEOValuePtr self, double inNumber, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOGetValueForObjectIDAndSeed( ((struct LEOValueReference*)self)->objectID, ((struct LEOValueReference*)self)->objectSeed, inContext );
	if( theValue == NULL )
	{
		snprintf( inContext->errMsg, sizeof(inContext->errMsg) -1, "The referenced value doesn't exist anymore." );
		inContext->keepRunning = false;
	}
	else
		LEOSetValueAsNumber( theValue, inNumber, inContext );
}


void	LEOSetReferenceValueRangeAsString( LEOValuePtr self, LEOChunkType inType,
											size_t inRangeStart, size_t inRangeEnd,
											const char* inBuf, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOGetValueForObjectIDAndSeed( ((struct LEOValueReference*)self)->objectID, ((struct LEOValueReference*)self)->objectSeed, inContext );
	if( theValue == NULL )
	{
		snprintf( inContext->errMsg, sizeof(inContext->errMsg) -1, "The referenced value doesn't exist anymore." );
		inContext->keepRunning = false;
	}
	else
		LEOSetValueRangeAsString( theValue, inType, inRangeStart, inRangeEnd, inBuf, inContext );
}


void	LEOInitReferenceValueCopy( LEOValuePtr self, LEOValuePtr dest, struct LEOContext* inContext )
{
	dest->isa = &kLeoValueTypeReference;
	dest->refObjectID = LEOObjectIDINVALID;
	((struct LEOValueReference*)dest)->objectID = ((struct LEOValueReference*)self)->objectID;
	((struct LEOValueReference*)dest)->objectSeed = ((struct LEOValueReference*)self)->objectSeed;
}


void	LEOCleanUpReferenceValue( LEOValuePtr self, struct LEOContext* inContext )
{
	self->isa = NULL;
	((struct LEOValueReference*)self)->objectID = LEOObjectIDINVALID;
	((struct LEOValueReference*)self)->objectSeed = 0;
	if( self->refObjectID != LEOObjectIDINVALID )	// We have references? Make sure they all notice we've gone if they try to access us from now on.
		LEOReturnObjectID( self->refObjectID, inContext );
}


#pragma mark -


struct LEOArrayEntry
{
	struct LEOArrayEntry	*	smallerItem;
	struct LEOArrayEntry	*	largerItem;
	union LEOValue				value;
	char						key[0];	// Must be last, dynamically sized array.
};


struct LEOArrayEntry	*	LEOAllocNewEntry( const char* inKey, LEOValuePtr inValue, LEOContext* inContext )
{
	struct LEOArrayEntry	*	newEntry = NULL;
	size_t						inKeyLen = strlen(inKey);
	newEntry = malloc( sizeof(struct LEOArrayEntry) +inKeyLen +1 );
	memmove( newEntry->key, inKey, inKeyLen +1 ); 
	LEOInitCopy( inValue, &newEntry->value, inContext );
	newEntry->smallerItem = NULL;
	newEntry->largerItem = NULL;
	
	return newEntry;
}


void	LEOAddArrayEntryToRoot( struct LEOArrayEntry** arrayPtrByReference, const char* inKey, LEOValuePtr inValue, LEOContext* inContext )
{
	struct LEOArrayEntry	*	currEntry = NULL;
	
	if( *arrayPtrByReference == NULL )
		*arrayPtrByReference = LEOAllocNewEntry( inKey, inValue, inContext );
	else
	{
		currEntry = *arrayPtrByReference;
		while( true )
		{
			int			cmpResult = strcasecmp( currEntry->key, inKey );
			if( cmpResult > 0 )	// Key is larger? Go down 'larger' side one step.
			{
				if( currEntry->largerItem == NULL )
				{
					currEntry->largerItem = LEOAllocNewEntry( inKey, inValue, inContext );
					break;
				}
				else
					currEntry = currEntry->largerItem;
			}
			else if( cmpResult < 0 )	// Key is smaller? Go down 'smaller' side one step.
			{
				if( currEntry->smallerItem == NULL )
				{
					currEntry->smallerItem = LEOAllocNewEntry( inKey, inValue, inContext );
					break;
				}
				else
					currEntry = currEntry->smallerItem;
			}
			else if( cmpResult == 0 )	// Key already exists? Replace value!
			{
				LEOCleanUpValue( &currEntry->value, inContext );
				LEOInitCopy( inValue, &currEntry->value, inContext );
			}
		}
	}
}


void	LEODeleteArrayEntryFromRoot( struct LEOArrayEntry** arrayPtrByReference, const char* inKey, LEOContext* inContext )
{
	struct LEOArrayEntry**	parentPtr = arrayPtrByReference;
	struct LEOArrayEntry*	currEntry = *arrayPtrByReference;
	while( true )
	{
		if( !currEntry )
			return;
		
		int			cmpResult = strcasecmp( currEntry->key, inKey );
		if( cmpResult > 0 )	// Key is larger? Go down 'larger' side one step.
		{
			parentPtr = &currEntry->largerItem;
			currEntry = currEntry->largerItem;
		}
		else if( cmpResult < 0 )	// Key is smaller? Go down 'smaller' side one step.
		{
			parentPtr = &currEntry->smallerItem;
			currEntry = currEntry->smallerItem;
		}
		else if( cmpResult == 0 )	// Found key!
		{
			LEOCleanUpValue( &currEntry->value, inContext );
			*parentPtr = NULL;
			
			if( currEntry->smallerItem && currEntry->largerItem )
			{
				// +++ Merge and re-add sub-tree.
			}
			else if( currEntry->smallerItem )
			{
				*parentPtr = currEntry->smallerItem;
			}
			else if( currEntry->largerItem )
			{
				*parentPtr = currEntry->largerItem;
			}
		}
	}
}


