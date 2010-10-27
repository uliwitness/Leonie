/*
 *  LEOValue.c
 *  Leonie
 *
 *  Created by Uli Kusterer on 06.09.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

/*!
	@header LEOValue (Private)
	
	The actual implementation and "private API" used to implement the various
	LEOValue subclasses.
*/

// -----------------------------------------------------------------------------
//	Headers:
// -----------------------------------------------------------------------------

#include "LEOValue.h"
#include "LEOInterpreter.h"
#include "LEOContextGroup.h"
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


struct LEOValueType	kLeoValueTypeNumberVariant =
{
	"number",
	sizeof(union LEOValue),
	
	LEOGetNumberValueAsNumber,
	LEOGetNumberValueAsString,
	LEOCantGetValueAsBoolean,
	LEOGetAnyValueAsRangeOfString,	// Only works as long as numbers can't be longer than OTHER_VALUE_SHORT_STRING_MAX_LENGTH as strings.
	
	LEOSetVariantValueAsNumber,
	LEOSetVariantValueAsString,
	LEOSetVariantValueAsBoolean,
	LEOSetVariantValueRangeAsString,
	
	LEOInitNumberVariantValueCopy,
	
	LEOCleanUpNumberValue
};


struct LEOValueType	kLeoValueTypeStringVariant =
{
	"string",
	sizeof(struct LEOValueString),
	
	LEOGetStringValueAsNumber,
	LEOGetStringValueAsString,
	LEOGetStringValueAsBoolean,
	LEOGetStringValueAsRangeOfString,
	
	LEOSetVariantValueAsNumber,
	LEOSetVariantValueAsString,
	LEOSetVariantValueAsBoolean,
	LEOSetVariantValueRangeAsString,
	
	LEOInitStringVariantValueCopy,
	
	LEOCleanUpStringValue
};


struct LEOValueType	kLeoValueTypeBooleanVariant =
{
	"boolean",
	sizeof(struct LEOValueBoolean),
	
	LEOCantGetValueAsNumber,
	LEOGetBooleanValueAsString,
	LEOGetBooleanValueAsBoolean,
	LEOGetAnyValueAsRangeOfString,	// Only works as long as booleans can't be longer than OTHER_VALUE_SHORT_STRING_MAX_LENGTH as strings.
	
	LEOSetVariantValueAsNumber,
	LEOSetVariantValueAsString,
	LEOSetVariantValueAsBoolean,
	LEOSetVariantValueRangeAsString,
	
	LEOInitBooleanVariantValueCopy,
	
	LEOCleanUpBooleanValue
};


#pragma mark -
#pragma mark Shared

/*!
	@functiongroup Shared LEOValue Method Implementations
*/


/*!
	Generic method implementation used for values to return a "can't get as number"
	error message and abort execution of the current LEOContext.
*/

double	LEOCantGetValueAsNumber( LEOValuePtr self, struct LEOContext* inContext )
{
	snprintf( inContext->errMsg, sizeof(inContext->errMsg), "Can't make %s into a number", self->isa->displayTypeName );
	inContext->keepRunning = false;
	
	return 0.0;
}


/*!
	Generic method implementation used for values to return a "can't get as boolean"
	error message and abort execution of the current LEOContext.
*/

bool	LEOCantGetValueAsBoolean( LEOValuePtr self, struct LEOContext* inContext )
{
	snprintf( inContext->errMsg, sizeof(inContext->errMsg), "Can't make %s into a boolean", self->isa->displayTypeName );
	inContext->keepRunning = false;
	
	return false;
}


/*!
	Generic method implementation used for values to return a "can't set as number"
	error message and abort execution of the current LEOContext.
*/

void	LEOCantSetValueAsNumber( LEOValuePtr self, double inNumber, struct LEOContext* inContext )
{
	snprintf( inContext->errMsg, sizeof(inContext->errMsg), "Expected %s, found number", self->isa->displayTypeName );
	inContext->keepRunning = false;
}


/*!
	Generic method implementation used for values to return a "can't set as string"
	error message and abort execution of the current LEOContext.
*/

void	LEOCantSetValueAsString( LEOValuePtr self, const char* inString, struct LEOContext* inContext )
{
	snprintf( inContext->errMsg, sizeof(inContext->errMsg), "Expected %s, found string", self->isa->displayTypeName );
	inContext->keepRunning = false;
}


/*!
	Generic method implementation used for values to return a "can't set as boolean"
	error message and abort execution of the current LEOContext.
*/

void	LEOCantSetValueAsBoolean( LEOValuePtr self, bool inState, struct LEOContext* inContext )
{
	snprintf( inContext->errMsg, sizeof(inContext->errMsg), "Expected %s, found boolean", self->isa->displayTypeName );
	inContext->keepRunning = false;
}


/*!
	Generic method implementation used for values to return a "can't set range as string"
	error message and abort execution of the current LEOContext.
*/

void	LEOCantSetValueRangeAsString( LEOValuePtr self, LEOChunkType inType,
									size_t inRangeStart, size_t inRangeEnd,
									const char* inBuf, struct LEOContext* inContext )
{
	snprintf( inContext->errMsg, sizeof(inContext->errMsg), "Expected %s, found string", self->isa->displayTypeName );
	inContext->keepRunning = false;
}


/*!
	Generic method implementation used for values to generate a string from
	them and then return a range of that.
*/

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


#pragma mark -
#pragma mark Number

/*!
	@functiongroup LEOValueNumber
*/

void	LEOInitNumberValue( LEOValuePtr inStorage, double inNumber, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	inStorage->isa = &kLeoValueTypeNumber;
	if( keepReferences == kLEOInvalidateReferences )
		inStorage->refObjectID = LEOObjectIDINVALID;
	((struct LEOValueNumber*)inStorage)->number = inNumber;
}


/*!
	Implementation of GetValueAsNumber for number values.
*/

double LEOGetNumberValueAsNumber( LEOValuePtr self, struct LEOContext* inContext )
{
	return ((struct LEOValueNumber*)self)->number;
}


/*!
	Implementation of GetValueAsString for number values.
*/

void LEOGetNumberValueAsString( LEOValuePtr self, char* outBuf, long bufSize, struct LEOContext* inContext )
{
	snprintf( outBuf, bufSize -1, "%g", ((struct LEOValueNumber*)self)->number );
}


/*!
	Implementation of SetValueAsNumber for number values.
*/

void LEOSetNumberValueAsNumber( LEOValuePtr self, double inNumber, struct LEOContext* inContext )
{
	((struct LEOValueNumber*)self)->number = inNumber;
}


/*!
	Implementation of SetValueAsString for number values. If the given string
	can't be fully converted to a number, this will fail with an error message
	and abort execution of the current LEOContext.
*/

void LEOSetNumberValueAsString( LEOValuePtr self, const char* inNumber, struct LEOContext* inContext )
{
	char*		endPtr = NULL;
	double		theNum = strtod( inNumber, &endPtr );
	if( endPtr != (inNumber +strlen(inNumber)) )
		LEOCantSetValueAsString( self, inNumber, inContext );
	else
		((struct LEOValueNumber*)self)->number = theNum;
}


/*!
	Implementation of InitCopy for number values.
*/

void	LEOInitNumberValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	dest->isa = &kLeoValueTypeNumber;
	if( keepReferences == kLEOInvalidateReferences )
		dest->refObjectID = LEOObjectIDINVALID;
	((struct LEOValueNumber*)dest)->number = ((struct LEOValueNumber*)self)->number;
}


/*!
	Destructor for number values. If this value has references, this makes sure
	that they will produce an error message if they ever try to access it again.
*/

void	LEOCleanUpNumberValue( LEOValuePtr self, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	self->isa = NULL;
	((struct LEOValueNumber*)self)->number = 0LL;
	if( keepReferences == kLEOInvalidateReferences && self->refObjectID != LEOObjectIDINVALID )	// We have references? Make sure they all notice we've gone if they try to access us from now on.
		LEOContextGroupRecycleObjectID( inContext->group, self->refObjectID );
}


#pragma mark -
#pragma mark Dynamically allocated string

/*!
	@functiongroup LEOValueString
*/

void	LEOInitStringValue( LEOValuePtr inStorage, const char* inString, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	inStorage->isa = &kLeoValueTypeString;
	if( keepReferences == kLEOInvalidateReferences )
		inStorage->refObjectID = LEOObjectIDINVALID;
	long		theLen = strlen(inString) +1;
	((struct LEOValueString*)inStorage)->string = malloc( theLen );
	strncpy( ((struct LEOValueString*)inStorage)->string, inString, theLen );
}


/*!
	Implementation of GetAsNumber for string values. If the given string can't
	be completely converted into a number, this will fail with an error message
	and abort execution of the current LEOContext.
*/

double	LEOGetStringValueAsNumber( LEOValuePtr self, struct LEOContext* inContext )
{
	char*	endPtr = NULL;
	double	num = strtod( ((struct LEOValueString*)self)->string, &endPtr );
	if( endPtr != (((struct LEOValueString*)self)->string +strlen(((struct LEOValueString*)self)->string)) )
		LEOCantGetValueAsNumber( self, inContext );
	return num;
}


/*!
	Implementation of GetAsString for string values.
*/

void	LEOGetStringValueAsString( LEOValuePtr self, char* outBuf, long bufSize, struct LEOContext* inContext )
{
	strncpy( outBuf, ((struct LEOValueString*)self)->string, bufSize );
}


/*!
	Implementation of SetAsNumber for string values.
*/

void	LEOSetStringValueAsNumber( LEOValuePtr self, double inNumber, struct LEOContext* inContext )
{
	if( ((struct LEOValueString*)self)->string )
		free( ((struct LEOValueString*)self)->string );
	((struct LEOValueString*)self)->string = malloc( 40 );
	snprintf( ((struct LEOValueString*)self)->string, 40, "%f", inNumber );
}


/*!
	Implementation of GetAsBoolean for string values. If the given string can't
	be completely converted into a boolean, this will fail with an error message
	and abort execution of the current LEOContext.
*/

bool	LEOGetStringValueAsBoolean( LEOValuePtr self, struct LEOContext* inContext )
{
	if( strcasecmp( ((struct LEOValueString*)self)->string, "true" ) == 0 )
		return true;
	else if( strcasecmp( ((struct LEOValueString*)self)->string, "false" ) == 0 )
		return false;
	else
		return LEOCantGetValueAsBoolean( self, inContext );
}


/*!
	Implementation of GetAsRangeOfString for string values.
*/

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



/*!
	Implementation of SetAsString for string values.
*/

void LEOSetStringValueAsString( LEOValuePtr self, const char* inString, struct LEOContext* inContext )
{
	if( ((struct LEOValueString*)self)->string )
		free( ((struct LEOValueString*)self)->string );
	long		theLen = strlen(inString) +1;
	((struct LEOValueString*)self)->string = malloc( theLen );
	strncpy( ((struct LEOValueString*)self)->string, inString, theLen );
}


/*!
	Implementation of SetAsBoolean for string values. This turns the string
	value into a constant string.
*/

void LEOSetStringValueAsBoolean( LEOValuePtr self, bool inBoolean, struct LEOContext* inContext )
{
	LEOSetStringValueAsStringConstant( self, (inBoolean ? "true" : "false"), inContext );
}


/*!
	Helper function used to turn a string value into a string constant value,
	and assigning it the given constant string (i.e. assigning the string by
	reference and not copying it). The caller is responsible for ensuring that
	the string is not disposed of while the value still needs it.
*/

void LEOSetStringValueAsStringConstant( LEOValuePtr self, const char* inString, struct LEOContext* inContext )
{
	if( ((struct LEOValueString*)self)->string )
		free( ((struct LEOValueString*)self)->string );
	
	self->isa = &kLeoValueTypeStringConstant;
	((struct LEOValueString*)self)->string = (char*) inString;
}


/*!
	Implementation of InitCopy for string values.
*/

void	LEOInitStringValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	dest->isa = &kLeoValueTypeString;
	if( keepReferences == kLEOInvalidateReferences )
		dest->refObjectID = LEOObjectIDINVALID;
	long		theLen = strlen(((struct LEOValueString*)self)->string) +1;
	((struct LEOValueString*)dest)->string = malloc( theLen );
	strncpy( ((struct LEOValueString*)dest)->string, ((struct LEOValueString*)self)->string, theLen );
}


/*!
	Implementation of SetRangeAsString for string values.
*/

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


/*!
	Destructor for string values. If this value has references, this makes sure
	that they will produce an error message if they ever try to access it again.
*/

void	LEOCleanUpStringValue( LEOValuePtr self, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	self->isa = NULL;
	if( ((struct LEOValueString*)self)->string )
		free( ((struct LEOValueString*)self)->string );
	((struct LEOValueString*)self)->string = NULL;
	if( keepReferences == kLEOInvalidateReferences && self->refObjectID != LEOObjectIDINVALID )
		LEOContextGroupRecycleObjectID( inContext->group, self->refObjectID );
}


#pragma mark -
#pragma mark String Constant

/*!
	@functiongroup LEOValueStringConstant
*/

void	LEOInitStringConstantValue( LEOValuePtr inStorage, const char* inString, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	inStorage->isa = &kLeoValueTypeStringConstant;
	if( keepReferences == kLEOInvalidateReferences )
		inStorage->refObjectID = LEOObjectIDINVALID;
	((struct LEOValueString*)inStorage)->string = (char*)inString;
}


/*!
	Implementation of SetAsNumber for string constant values. This turns the
	value into a regular (dynamic) string value.
*/

void	LEOSetStringConstantValueAsNumber( LEOValuePtr self, double inNumber, struct LEOContext* inContext )
{
	// Turn this into a non-constant string:
	self->isa = &kLeoValueTypeString;
	((struct LEOValueString*)self)->string = malloc( 40 );
	snprintf( ((struct LEOValueString*)self)->string, 40, "%f", inNumber );
}


/*!
	Implementation of SetAsString for string constant values. This turns the
	value into a regular (dynamic) string value.
*/

void	LEOSetStringConstantValueAsString( LEOValuePtr self, const char* inString, struct LEOContext* inContext )
{
	// Turn this into a non-constant string:
	self->isa = &kLeoValueTypeString;
	long		theLen = strlen(inString) +1;
	((struct LEOValueString*)self)->string = malloc( theLen );
	strncpy( ((struct LEOValueString*)self)->string, inString, theLen );
}


/*!
	Implementation of SetAsString for string constant values.
*/

void	LEOSetStringConstantValueAsBoolean( LEOValuePtr self, bool inBoolean, struct LEOContext* inContext )
{
	((struct LEOValueString*)self)->string = (inBoolean ? "true" : "false");
}


/*!
	Implementation of InitCopy for string constant values.
*/

void	LEOInitStringConstantValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	dest->isa = &kLeoValueTypeStringConstant;
	if( keepReferences == kLEOInvalidateReferences )
		dest->refObjectID = LEOObjectIDINVALID;
	((struct LEOValueString*)dest)->string = ((struct LEOValueString*)self)->string;
}


/*!
	Implementation of SetRangeAsString for string values. This turns the
	value into a regular (dynamic) string value.
*/

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


/*!
	Destructor for string constant values. If this value has references, this
	makes sure that they will produce an error message if they ever try to
	access it again.
*/

void	LEOCleanUpStringConstantValue( LEOValuePtr self, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	self->isa = NULL;
	((struct LEOValueString*)self)->string = NULL;
	if( keepReferences == kLEOInvalidateReferences && self->refObjectID != LEOObjectIDINVALID )
		LEOContextGroupRecycleObjectID( inContext->group, self->refObjectID );
}


#pragma mark -
#pragma mark Boolean

/*!
	@functiongroup LEOValueBoolean
*/

void	LEOInitBooleanValue( LEOValuePtr self, bool inBoolean, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	self->isa = &kLeoValueTypeBoolean;
	if( keepReferences == kLEOInvalidateReferences )
		self->refObjectID = LEOObjectIDINVALID;
	((struct LEOValueBoolean*)self)->boolean = inBoolean;
}


/*!
	Implementation of GetAsString for boolean values.
*/

void	LEOGetBooleanValueAsString( LEOValuePtr self, char* outBuf, long bufSize, struct LEOContext* inContext )
{
	strncpy( outBuf, ((struct LEOValueBoolean*)self)->boolean ? "true" : "false", bufSize -1 );
}


/*!
	Implementation of GetAsBoolean for boolean values.
*/

bool	LEOGetBooleanValueAsBoolean( LEOValuePtr self, struct LEOContext* inContext )
{
	return ((struct LEOValueBoolean*)self)->boolean;
}


/*!
	Implementation of SetValueAsString for boolean values. If the given string
	can't be fully converted to a boolean, this will fail with an error message
	and abort execution of the current LEOContext.
*/

void	LEOSetBooleanValueAsString( LEOValuePtr self, const char* inString, struct LEOContext* inContext )
{
	if( strcasecmp( inString, "true" ) == 0 )
		((struct LEOValueBoolean*)self)->boolean = true;
	else if( strcasecmp( inString, "false" ) == 0 )
		((struct LEOValueBoolean*)self)->boolean = false;
	else
		LEOCantSetValueAsString( self, inString, inContext );
}


/*!
	Implementation of SetValueAsBoolean for boolean values.
*/

void	LEOSetBooleanValueAsBoolean( LEOValuePtr self, bool inBoolean, struct LEOContext* inContext )
{
	((struct LEOValueBoolean*)self)->boolean = inBoolean;
}


/*!
	Implementation of InitCopy for boolean values.
*/

void	LEOInitBooleanValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	dest->isa = &kLeoValueTypeBoolean;
	if( keepReferences == kLEOInvalidateReferences )
		dest->refObjectID = LEOObjectIDINVALID;
	((struct LEOValueBoolean*)dest)->boolean = ((struct LEOValueBoolean*)self)->boolean;
}


/*!
	Destructor for boolean values. If this value has references, this makes sure
	that they will produce an error message if they ever try to access it again.
*/

void	LEOCleanUpBooleanValue( LEOValuePtr self, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	self->isa = NULL;
	((struct LEOValueBoolean*)self)->boolean = false;
	if( keepReferences == kLEOInvalidateReferences && self->refObjectID != LEOObjectIDINVALID )
		LEOContextGroupRecycleObjectID( inContext->group, self->refObjectID );
}



#pragma mark -
#pragma mark Reference


/*!
	@functiongroup LEOValueReference
*/

/*!
	Initialize the given storage so it's a valid reference value that
	points to the given original value. If the original value is destructed
	while a reference still points to it, method calls to such a reference will
	fail with an error message and abort execution of the current LEOContext.
	
	However, the destructor of the value is safe to call, as is InitCopy.
*/

void	LEOInitReferenceValue( LEOValuePtr self, LEOValuePtr originalValue, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	self->isa = &kLeoValueTypeReference;
	if( keepReferences == kLEOInvalidateReferences )
		self->refObjectID = LEOObjectIDINVALID;
	
	if( originalValue->refObjectID == LEOObjectIDINVALID )
		LEOContextGroupCreateNewObjectIDForValue( inContext->group, originalValue );
	
	((LEOValueReference*)self)->objectID = originalValue->refObjectID;
	((LEOValueReference*)self)->objectSeed = LEOContextGroupGetSeedForObjectID( inContext->group, originalValue->refObjectID );
}


/*!
	Implementation of GetAsString for reference values.
*/

void	LEOGetReferenceValueAsString( LEOValuePtr self, char* outBuf, long bufSize, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetValueForObjectIDAndSeed( inContext->group, ((struct LEOValueReference*)self)->objectID, ((struct LEOValueReference*)self)->objectSeed );
	if( theValue == NULL )
	{
		snprintf( inContext->errMsg, sizeof(inContext->errMsg) -1, "The referenced value doesn't exist anymore." );
		inContext->keepRunning = false;
	}
	else
		LEOGetValueAsString( theValue, outBuf, bufSize, inContext );
}


/*!
	Implementation of GetAsNumber for reference values.
*/

double	LEOGetReferenceValueAsNumber( LEOValuePtr self, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetValueForObjectIDAndSeed( inContext->group, ((struct LEOValueReference*)self)->objectID, ((struct LEOValueReference*)self)->objectSeed );
	if( theValue == NULL )
	{
		snprintf( inContext->errMsg, sizeof(inContext->errMsg) -1, "The referenced value doesn't exist anymore." );
		inContext->keepRunning = false;
		return 0.0;
	}
	else
		return LEOGetValueAsNumber( theValue, inContext );
}


/*!
	Implementation of GetAsBoolean for reference values.
*/

bool	LEOGetReferenceValueAsBoolean( LEOValuePtr self, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetValueForObjectIDAndSeed( inContext->group, ((struct LEOValueReference*)self)->objectID, ((struct LEOValueReference*)self)->objectSeed );
	if( theValue == NULL )
	{
		snprintf( inContext->errMsg, sizeof(inContext->errMsg) -1, "The referenced value doesn't exist anymore." );
		inContext->keepRunning = false;
		return false;
	}
	else
		return LEOGetValueAsBoolean( theValue, inContext );
}


/*!
	Implementation of GetAsRangeOfString for reference values.
*/

void	LEOGetReferenceValueAsRangeOfString( LEOValuePtr self, LEOChunkType inType,
									size_t inRangeStart, size_t inRangeEnd,
									char* outBuf, long bufSize, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetValueForObjectIDAndSeed( inContext->group, ((struct LEOValueReference*)self)->objectID, ((struct LEOValueReference*)self)->objectSeed );
	if( theValue == NULL )
	{
		snprintf( inContext->errMsg, sizeof(inContext->errMsg) -1, "The referenced value doesn't exist anymore." );
		inContext->keepRunning = false;
	}
	else
		LEOGetValueAsRangeOfString( theValue, inType, inRangeStart, inRangeEnd, outBuf, bufSize, inContext );
}


/*!
	Implementation of SetAsString for reference values.
*/

void	LEOSetReferenceValueAsString( LEOValuePtr self, const char* inString, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetValueForObjectIDAndSeed( inContext->group, ((struct LEOValueReference*)self)->objectID, ((struct LEOValueReference*)self)->objectSeed );
	if( theValue == NULL )
	{
		snprintf( inContext->errMsg, sizeof(inContext->errMsg) -1, "The referenced value doesn't exist anymore." );
		inContext->keepRunning = false;
	}
	else
		LEOSetValueAsString( theValue, inString, inContext );
}


/*!
	Implementation of SetAsBoolean for reference values.
*/

void	LEOSetReferenceValueAsBoolean( LEOValuePtr self, bool inBoolean, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetValueForObjectIDAndSeed( inContext->group, ((struct LEOValueReference*)self)->objectID, ((struct LEOValueReference*)self)->objectSeed );
	if( theValue == NULL )
	{
		snprintf( inContext->errMsg, sizeof(inContext->errMsg) -1, "The referenced value doesn't exist anymore." );
		inContext->keepRunning = false;
	}
	else
		LEOSetValueAsBoolean( theValue, inBoolean, inContext );
}


/*!
	Implementation of SetAsNumber for reference values.
*/

void	LEOSetReferenceValueAsNumber( LEOValuePtr self, double inNumber, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetValueForObjectIDAndSeed( inContext->group, ((struct LEOValueReference*)self)->objectID, ((struct LEOValueReference*)self)->objectSeed );
	if( theValue == NULL )
	{
		snprintf( inContext->errMsg, sizeof(inContext->errMsg) -1, "The referenced value doesn't exist anymore." );
		inContext->keepRunning = false;
	}
	else
		LEOSetValueAsNumber( theValue, inNumber, inContext );
}


/*!
	Implementation of SetRangeAsString for reference values.
*/

void	LEOSetReferenceValueRangeAsString( LEOValuePtr self, LEOChunkType inType,
											size_t inRangeStart, size_t inRangeEnd,
											const char* inBuf, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetValueForObjectIDAndSeed( inContext->group, ((struct LEOValueReference*)self)->objectID, ((struct LEOValueReference*)self)->objectSeed );
	if( theValue == NULL )
	{
		snprintf( inContext->errMsg, sizeof(inContext->errMsg) -1, "The referenced value doesn't exist anymore." );
		inContext->keepRunning = false;
	}
	else
		LEOSetValueRangeAsString( theValue, inType, inRangeStart, inRangeEnd, inBuf, inContext );
}


/*!
	Implementation of InitCopy for reference values. This method is safe to call
	even if the original value this reference points to has already gone away.
*/

void	LEOInitReferenceValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	dest->isa = &kLeoValueTypeReference;
	if( keepReferences == kLEOInvalidateReferences )
		dest->refObjectID = LEOObjectIDINVALID;
	((struct LEOValueReference*)dest)->objectID = ((struct LEOValueReference*)self)->objectID;
	((struct LEOValueReference*)dest)->objectSeed = ((struct LEOValueReference*)self)->objectSeed;
}


/*!
	Destructor for reference values. If this value has references, this makes sure
	that they will produce an error message if they ever try to access it again.
	Yes, you can have references to references. Use this power wisely.
*/

void	LEOCleanUpReferenceValue( LEOValuePtr self, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	self->isa = NULL;
	((struct LEOValueReference*)self)->objectID = LEOObjectIDINVALID;
	((struct LEOValueReference*)self)->objectSeed = 0;
	if( keepReferences == kLEOInvalidateReferences && self->refObjectID != LEOObjectIDINVALID )	// We have references? Make sure they all notice we've gone if they try to access us from now on.
		LEOContextGroupRecycleObjectID( inContext->group, self->refObjectID );
}


#pragma mark -
#pragma mark Variants


void	LEOInitNumberVariantValue( LEOValuePtr self, double inNumber, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	LEOInitNumberValue( self, inNumber, keepReferences, inContext );
	self->isa = &kLeoValueTypeNumberVariant;
}


void	LEOInitStringVariantValue( LEOValuePtr self, const char* inString, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	LEOInitStringValue( self, inString, keepReferences, inContext );
	self->isa = &kLeoValueTypeStringVariant;
}


void	LEOInitBooleanVariantValue( LEOValuePtr self, bool inBoolean, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	LEOInitBooleanValue( self, inBoolean, keepReferences, inContext );
	self->isa = &kLeoValueTypeBooleanVariant;
}


void	LEOSetVariantValueAsNumber( LEOValuePtr self, double inNumber, struct LEOContext* inContext )
{
	LEOCleanUpValue( self, kLEOKeepReferences, inContext );
	LEOInitNumberValue( self, inNumber, kLEOKeepReferences, inContext );
	self->isa = &kLeoValueTypeNumberVariant;
}


void	LEOSetVariantValueAsString( LEOValuePtr self, const char* inString, struct LEOContext* inContext )
{
	LEOCleanUpValue( self, kLEOKeepReferences, inContext );
	LEOInitStringValue( self, inString, kLEOKeepReferences, inContext );
	self->isa = &kLeoValueTypeStringVariant;
}


void	LEOSetVariantValueAsBoolean( LEOValuePtr self, bool inBoolean, struct LEOContext* inContext )				// Makes it a constant string.
{
	LEOCleanUpValue( self, kLEOKeepReferences, inContext );
	LEOInitBooleanValue( self, inBoolean, kLEOKeepReferences, inContext );
	self->isa = &kLeoValueTypeBooleanVariant;
}


void	LEOSetVariantValueRangeAsString( LEOValuePtr self, LEOChunkType inType,
											size_t inRangeStart, size_t inRangeEnd,
											const char* inBuf, struct LEOContext* inContext )
{
	if( self->isa != &kLeoValueTypeStringVariant )	// Convert to string first.
	{
		char		shortStr[OTHER_VALUE_SHORT_STRING_MAX_LENGTH] = {0};
		LEOGetValueAsString( self, shortStr, sizeof(shortStr), inContext );
		LEOCleanUpValue( self, kLEOKeepReferences, inContext );
		LEOInitStringValue( self, shortStr, kLEOKeepReferences, inContext );
		self->isa = &kLeoValueTypeStringVariant;
	}
	LEOSetStringValueRangeAsString( self, inType, inRangeStart, inRangeEnd, inBuf, inContext );
}


void	LEOInitNumberVariantValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	LEOInitNumberValueCopy( self, dest, keepReferences, inContext );
	dest->isa = &kLeoValueTypeNumberVariant;
}


void	LEOInitBooleanVariantValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	LEOInitBooleanValueCopy( self, dest, keepReferences, inContext );
	dest->isa = &kLeoValueTypeBooleanVariant;
}


void	LEOInitStringVariantValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	LEOInitStringValueCopy( self, dest, keepReferences, inContext );
	dest->isa = &kLeoValueTypeStringVariant;
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
	LEOInitCopy( inValue, &newEntry->value, kLEOInvalidateReferences, inContext );
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
				LEOCleanUpValue( &currEntry->value, kLEOKeepReferences, inContext );
				LEOInitCopy( inValue, &currEntry->value, kLEOKeepReferences, inContext );
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
			LEOCleanUpValue( &currEntry->value, kLEOInvalidateReferences, inContext );
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


