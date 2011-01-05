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
#include <math.h>


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
	LEOGetNumberValueAsInteger,
	LEOGetNumberValueAsString,
	LEOCantGetValueAsBoolean,
	LEOGetAnyValueAsRangeOfString,	// Only works as long as numbers can't be longer than OTHER_VALUE_SHORT_STRING_MAX_LENGTH as strings.
	
	LEOSetNumberValueAsNumber,
	LEOSetNumberValueAsInteger,
	LEOSetNumberValueAsString,
	LEOCantSetValueAsBoolean,
	LEOCantSetValueRangeAsString,
	LEOCantSetValuePredeterminedRangeAsString,
	
	LEOInitNumberValueCopy,
	LEOInitNumberValueCopy,
	LEODetermineChunkRangeOfSubstringOfAnyValue,
	
	LEOCleanUpNumberValue,
	
	LEOCanGetValueAsNumber
};


struct LEOValueType	kLeoValueTypeInteger =
{
	"integer",
	sizeof(struct LEOValueInteger),
	
	LEOGetIntegerValueAsNumber,
	LEOGetIntegerValueAsInteger,
	LEOGetIntegerValueAsString,
	LEOCantGetValueAsBoolean,
	LEOGetAnyValueAsRangeOfString,	// Only works as long as integers can't be longer than OTHER_VALUE_SHORT_STRING_MAX_LENGTH as strings.
	
	LEOSetIntegerValueAsNumber,
	LEOSetIntegerValueAsInteger,
	LEOSetIntegerValueAsString,
	LEOCantSetValueAsBoolean,
	LEOCantSetValueRangeAsString,
	LEOCantSetValuePredeterminedRangeAsString,
	
	LEOInitIntegerValueCopy,
	LEOInitIntegerValueCopy,
	LEODetermineChunkRangeOfSubstringOfAnyValue,
	
	LEOCleanUpIntegerValue,
	
	LEOCanGetValueAsNumber
};


struct LEOValueType	kLeoValueTypeString =
{
	"string",
	sizeof(struct LEOValueString),
	
	LEOGetStringValueAsNumber,
	LEOGetStringValueAsInteger,
	LEOGetStringValueAsString,
	LEOGetStringValueAsBoolean,
	LEOGetStringValueAsRangeOfString,
	
	LEOSetStringValueAsNumber,
	LEOSetStringValueAsInteger,
	LEOSetStringValueAsString,
	LEOSetStringValueAsBoolean,
	LEOSetStringValueRangeAsString,
	LEOSetStringValuePredeterminedRangeAsString,
	
	LEOInitStringValueCopy,
	LEOInitStringValueCopy,
	LEODetermineChunkRangeOfSubstringOfStringValue,
	
	LEOCleanUpStringValue,
	
	LEOCanGetStringValueAsNumber
};


struct LEOValueType	kLeoValueTypeStringConstant =
{
	"string",
	sizeof(struct LEOValueString),
	
	LEOGetStringValueAsNumber,
	LEOGetStringValueAsInteger,
	LEOGetStringValueAsString,
	LEOGetStringValueAsBoolean,
	LEOGetStringValueAsRangeOfString,
	
	LEOSetStringConstantValueAsNumber,
	LEOSetStringConstantValueAsInteger,
	LEOSetStringConstantValueAsString,
	LEOSetStringConstantValueAsBoolean,
	LEOSetStringConstantValueRangeAsString,
	LEOSetStringConstantValuePredeterminedRangeAsString,
	
	LEOInitStringConstantValueCopy,
	LEOInitStringConstantValueCopy,
	LEODetermineChunkRangeOfSubstringOfStringValue,
	
	LEOCleanUpStringConstantValue,
	
	LEOCanGetStringValueAsNumber
};




struct LEOValueType	kLeoValueTypeBoolean =
{
	"boolean",
	sizeof(struct LEOValueBoolean),
	
	LEOCantGetValueAsNumber,
	LEOCantGetValueAsInteger,
	LEOGetBooleanValueAsString,
	LEOGetBooleanValueAsBoolean,
	LEOGetAnyValueAsRangeOfString,	// Only works as long as booleans can't be longer than OTHER_VALUE_SHORT_STRING_MAX_LENGTH as strings.
	
	LEOCantSetValueAsNumber,
	LEOCantSetValueAsInteger,
	LEOSetBooleanValueAsString,
	LEOSetBooleanValueAsBoolean,
	LEOCantSetValueRangeAsString,
	LEOCantSetValuePredeterminedRangeAsString,
	
	LEOInitBooleanValueCopy,
	LEOInitBooleanValueCopy,
	LEODetermineChunkRangeOfSubstringOfAnyValue,
	
	LEOCleanUpBooleanValue,
	
	LEOCantCanGetValueAsNumber
};


struct LEOValueType	kLeoValueTypeReference =
{
	"reference",
	sizeof(struct LEOValueReference),
	
	LEOGetReferenceValueAsNumber,
	LEOGetReferenceValueAsInteger,
	LEOGetReferenceValueAsString,
	LEOGetReferenceValueAsBoolean,
	LEOGetReferenceValueAsRangeOfString,
	
	LEOSetReferenceValueAsNumber,
	LEOSetReferenceValueAsInteger,
	LEOSetReferenceValueAsString,
	LEOSetReferenceValueAsBoolean,
	LEOSetReferenceValueRangeAsString,
	LEOSetReferenceValuePredeterminedRangeAsString,
	
	LEOInitReferenceValueCopy,
	LEOInitReferenceValueSimpleCopy,
	LEODetermineChunkRangeOfSubstringOfReferenceValue,
	
	LEOCleanUpReferenceValue,
	
	LEOCanGetReferenceValueAsNumber
};


struct LEOValueType	kLeoValueTypeNumberVariant =
{
	"number",
	sizeof(union LEOValue),
	
	LEOGetNumberValueAsNumber,
	LEOGetNumberValueAsInteger,
	LEOGetNumberValueAsString,
	LEOCantGetValueAsBoolean,
	LEOGetAnyValueAsRangeOfString,	// Only works as long as numbers can't be longer than OTHER_VALUE_SHORT_STRING_MAX_LENGTH as strings.
	
	LEOSetVariantValueAsNumber,
	LEOSetVariantValueAsInteger,
	LEOSetVariantValueAsString,
	LEOSetVariantValueAsBoolean,
	LEOSetVariantValueRangeAsString,
	LEOSetVariantValuePredeterminedRangeAsString,
	
	LEOInitNumberVariantValueCopy,
	LEOInitNumberValueCopy,
	LEODetermineChunkRangeOfSubstringOfAnyValue,
	
	LEOCleanUpNumberValue,
	
	LEOCanGetValueAsNumber
};


struct LEOValueType	kLeoValueTypeIntegerVariant =
{
	"integer",
	sizeof(union LEOValue),
	
	LEOGetIntegerValueAsNumber,
	LEOGetIntegerValueAsInteger,
	LEOGetIntegerValueAsString,
	LEOCantGetValueAsBoolean,
	LEOGetAnyValueAsRangeOfString,	// Only works as long as numbers can't be longer than OTHER_VALUE_SHORT_STRING_MAX_LENGTH as strings.
	
	LEOSetVariantValueAsNumber,
	LEOSetVariantValueAsInteger,
	LEOSetVariantValueAsString,
	LEOSetVariantValueAsBoolean,
	LEOSetVariantValueRangeAsString,
	LEOSetVariantValuePredeterminedRangeAsString,
	
	LEOInitIntegerVariantValueCopy,
	LEOInitIntegerValueCopy,
	LEODetermineChunkRangeOfSubstringOfAnyValue,
	
	LEOCleanUpIntegerValue,
	
	LEOCanGetValueAsNumber
};


struct LEOValueType	kLeoValueTypeStringVariant =
{
	"string",
	sizeof(struct LEOValueString),
	
	LEOGetStringValueAsNumber,
	LEOGetStringValueAsInteger,
	LEOGetStringValueAsString,
	LEOGetStringValueAsBoolean,
	LEOGetStringValueAsRangeOfString,
	
	LEOSetVariantValueAsNumber,
	LEOSetVariantValueAsInteger,
	LEOSetVariantValueAsString,
	LEOSetVariantValueAsBoolean,
	LEOSetVariantValueRangeAsString,
	LEOSetVariantValuePredeterminedRangeAsString,
	
	LEOInitStringVariantValueCopy,
	LEOInitStringValueCopy,
	LEODetermineChunkRangeOfSubstringOfStringValue,
	
	LEOCleanUpStringValue,
	
	LEOCanGetStringValueAsNumber
};


struct LEOValueType	kLeoValueTypeBooleanVariant =
{
	"boolean",
	sizeof(struct LEOValueBoolean),
	
	LEOCantGetValueAsNumber,
	LEOCantGetValueAsInteger,
	LEOGetBooleanValueAsString,
	LEOGetBooleanValueAsBoolean,
	LEOGetAnyValueAsRangeOfString,	// Only works as long as booleans can't be longer than OTHER_VALUE_SHORT_STRING_MAX_LENGTH as strings.
	
	LEOSetVariantValueAsNumber,
	LEOSetVariantValueAsInteger,
	LEOSetVariantValueAsString,
	LEOSetVariantValueAsBoolean,
	LEOSetVariantValueRangeAsString,
	LEOSetVariantValuePredeterminedRangeAsString,
	
	LEOInitBooleanVariantValueCopy,
	LEOInitBooleanValueCopy,
	LEODetermineChunkRangeOfSubstringOfAnyValue,
	
	LEOCleanUpBooleanValue,
	
	LEOCantCanGetValueAsNumber
};


struct LEOValueType	kLeoValueTypeArray =
{
	"array",
	sizeof(struct LEOValueArray),
	
	LEOCantGetValueAsNumber,
	LEOCantGetValueAsInteger,
	LEOGetArrayValueAsString,
	LEOCantGetValueAsBoolean,
	LEOGetArrayValueAsRangeOfString,
	
	LEOCantSetValueAsNumber,
	LEOCantSetValueAsInteger,
	LEOCantSetValueAsString,
	LEOCantSetValueAsBoolean,
	LEOCantSetValueRangeAsString,
	LEOCantSetValuePredeterminedRangeAsString,
	
	LEOInitArrayValueCopy,
	LEOInitArrayValueCopy,
	LEODetermineChunkRangeOfSubstringOfArrayValue,
	
	LEOCleanUpArrayValue,
	
	LEOCantCanGetValueAsNumber
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

LEONumber	LEOCantGetValueAsNumber( LEOValuePtr self, struct LEOContext* inContext )
{
	snprintf( inContext->errMsg, sizeof(inContext->errMsg), "Can't make %s into a number", self->base.isa->displayTypeName );
	inContext->keepRunning = false;
	
	return 0.0;
}


/*!
	Generic method implementation used for values to return a "can't get as integer"
	error message and abort execution of the current LEOContext.
*/

LEOInteger	LEOCantGetValueAsInteger( LEOValuePtr self, struct LEOContext* inContext )
{
	snprintf( inContext->errMsg, sizeof(inContext->errMsg), "Can't make %s into an integer", self->base.isa->displayTypeName );
	inContext->keepRunning = false;
	
	return 0LL;
}


/*!
	Generic method implementation used for values to return a "can't get as boolean"
	error message and abort execution of the current LEOContext.
*/

bool	LEOCantGetValueAsBoolean( LEOValuePtr self, struct LEOContext* inContext )
{
	snprintf( inContext->errMsg, sizeof(inContext->errMsg), "Can't make %s into a boolean", self->base.isa->displayTypeName );
	inContext->keepRunning = false;
	
	return false;
}


/*!
	Generic method implementation used for values to return a "can't set as number"
	error message and abort execution of the current LEOContext.
*/

void	LEOCantSetValueAsNumber( LEOValuePtr self, LEONumber inNumber, struct LEOContext* inContext )
{
	snprintf( inContext->errMsg, sizeof(inContext->errMsg), "Expected %s, found number", self->base.isa->displayTypeName );
	inContext->keepRunning = false;
}


/*!
	Generic method implementation used for values to return a "can't set as integer"
	error message and abort execution of the current LEOContext.
*/

void	LEOCantSetValueAsInteger( LEOValuePtr self, LEOInteger inInteger, struct LEOContext* inContext )
{
	snprintf( inContext->errMsg, sizeof(inContext->errMsg), "Expected %s, found integer", self->base.isa->displayTypeName );
	inContext->keepRunning = false;
}


/*!
	Generic method implementation used for values to return a "can't set as string"
	error message and abort execution of the current LEOContext.
*/

void	LEOCantSetValueAsString( LEOValuePtr self, const char* inString, struct LEOContext* inContext )
{
	snprintf( inContext->errMsg, sizeof(inContext->errMsg), "Expected %s, found string", self->base.isa->displayTypeName );
	inContext->keepRunning = false;
}


/*!
	Generic method implementation used for values to return a "can't set as boolean"
	error message and abort execution of the current LEOContext.
*/

void	LEOCantSetValueAsBoolean( LEOValuePtr self, bool inState, struct LEOContext* inContext )
{
	snprintf( inContext->errMsg, sizeof(inContext->errMsg), "Expected %s, found boolean", self->base.isa->displayTypeName );
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
	snprintf( inContext->errMsg, sizeof(inContext->errMsg), "Expected %s, found string", self->base.isa->displayTypeName );
	inContext->keepRunning = false;
}


void	LEOCantSetValuePredeterminedRangeAsString( LEOValuePtr self, size_t inRangeStart, size_t inRangeEnd,
													const char* inBuf, struct LEOContext* inContext )
{
	snprintf( inContext->errMsg, sizeof(inContext->errMsg), "Expected %s, found string", self->base.isa->displayTypeName );
	inContext->keepRunning = false;
}


/*!
	Generic method implementation used for values to generate a string from
	them and then return a range of that.
*/

void	LEOGetAnyValueAsRangeOfString( LEOValuePtr self, LEOChunkType inType,
									size_t inRangeStart, size_t inRangeEnd,
									char* outBuf, size_t bufSize, struct LEOContext* inContext )
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


void	LEODetermineChunkRangeOfSubstringOfAnyValue( LEOValuePtr self, size_t *ioBytesStart, size_t *ioBytesEnd,
														size_t *ioBytesDelStart, size_t *ioBytesDelEnd,
														LEOChunkType inType, size_t inRangeStart, size_t inRangeEnd,
														struct LEOContext* inContext )
{
	char		strBuf[OTHER_VALUE_SHORT_STRING_MAX_LENGTH] = {0};	// Can get away with this as long as they're only numbers, booleans etc.
	char*		str = strBuf;
	LEOGetValueAsString( self, strBuf, sizeof(strBuf), inContext );
	if( (*ioBytesEnd) < sizeof(strBuf) )
		str[(*ioBytesEnd)] = '\0';
	str += (*ioBytesStart);
	
	size_t		chunkStart, chunkEnd, delChunkStart, delChunkEnd;
	
	LEOGetChunkRanges( str, inType,
						inRangeStart, inRangeEnd,
						&chunkStart, &chunkEnd,
						&delChunkStart, &delChunkEnd,
						inContext->itemDelimiter );
	(*ioBytesStart) += chunkStart;
	(*ioBytesEnd) = (*ioBytesStart) + (chunkEnd -chunkStart);
	(*ioBytesDelStart) = (*ioBytesStart) +delChunkStart;
	(*ioBytesDelEnd) = (*ioBytesDelStart) + (delChunkEnd -delChunkStart);
}


bool	LEOCanGetValueAsNumber( LEOValuePtr self, struct LEOContext* inContext )
{
	return true;
}


bool	LEOCantCanGetValueAsNumber( LEOValuePtr self, struct LEOContext* inContext )
{
	return false;
}

#pragma mark -
#pragma mark Number

/*!
	@functiongroup LEOValueNumber
*/

void	LEOInitNumberValue( LEOValuePtr inStorage, LEONumber inNumber, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	inStorage->base.isa = &kLeoValueTypeNumber;
	if( keepReferences == kLEOInvalidateReferences )
		inStorage->base.refObjectID = kLEOObjectIDINVALID;
	inStorage->number.number = inNumber;
}


/*!
	Implementation of GetValueAsNumber for number values.
*/

LEONumber LEOGetNumberValueAsNumber( LEOValuePtr self, struct LEOContext* inContext )
{
	return self->number.number;
}


/*!
	Implementation of GetValueAsInteger for number values.
*/

LEOInteger LEOGetNumberValueAsInteger( LEOValuePtr self, struct LEOContext* inContext )
{
	if( trunc(self->number.number) != self->number.number )
	{
		snprintf( inContext->errMsg, sizeof(inContext->errMsg), "Expected integer, not fractional number." );
		inContext->keepRunning = false;
	}
	
	return self->number.number;
}


/*!
	Implementation of GetValueAsString for number values.
*/

void LEOGetNumberValueAsString( LEOValuePtr self, char* outBuf, size_t bufSize, struct LEOContext* inContext )
{
	snprintf( outBuf, bufSize -1, "%g", self->number.number );
}


/*!
	Implementation of SetValueAsNumber for number values.
*/

void LEOSetNumberValueAsNumber( LEOValuePtr self, LEONumber inNumber, struct LEOContext* inContext )
{
	self->number.number = inNumber;
}


/*!
	Implementation of SetValueAsInteger for number values.
*/

void LEOSetNumberValueAsInteger( LEOValuePtr self, LEOInteger inInteger, struct LEOContext* inContext )
{
	self->number.number = inInteger;
}


/*!
	Implementation of SetValueAsString for number values. If the given string
	can't be fully converted to a number, this will fail with an error message
	and abort execution of the current LEOContext.
*/

void LEOSetNumberValueAsString( LEOValuePtr self, const char* inNumber, struct LEOContext* inContext )
{
	char*		endPtr = NULL;
	LEONumber	theNum = strtod( inNumber, &endPtr );
	if( endPtr != (inNumber +strlen(inNumber)) )
		LEOCantSetValueAsString( self, inNumber, inContext );
	else
		self->number.number = theNum;
}


/*!
	Implementation of InitCopy for number values.
*/

void	LEOInitNumberValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	dest->base.isa = &kLeoValueTypeNumber;
	if( keepReferences == kLEOInvalidateReferences )
		dest->base.refObjectID = kLEOObjectIDINVALID;
	dest->number.number = self->number.number;
}


/*!
	Destructor for number values. If this value has references, this makes sure
	that they will produce an error message if they ever try to access it again.
*/

void	LEOCleanUpNumberValue( LEOValuePtr self, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	self->base.isa = NULL;
	self->number.number = 0.0;
	if( keepReferences == kLEOInvalidateReferences && self->base.refObjectID != kLEOObjectIDINVALID )	// We have references? Make sure they all notice we've gone if they try to access us from now on.
		LEOContextGroupRecycleObjectID( inContext->group, self->base.refObjectID );
}


#pragma mark -
#pragma mark Integer

/*!
	@functiongroup LEOValueInteger
*/

void	LEOInitIntegerValue( LEOValuePtr inStorage, LEOInteger inInteger, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	inStorage->base.isa = &kLeoValueTypeInteger;
	if( keepReferences == kLEOInvalidateReferences )
		inStorage->base.refObjectID = kLEOObjectIDINVALID;
	inStorage->integer.integer = inInteger;
}


/*!
	Implementation of GetValueAsNumber for integer values.
*/

LEONumber LEOGetIntegerValueAsNumber( LEOValuePtr self, struct LEOContext* inContext )
{
	return self->integer.integer;
}


/*!
	Implementation of GetValueAsInteger for integer values.
*/

LEOInteger LEOGetIntegerValueAsInteger( LEOValuePtr self, struct LEOContext* inContext )
{
	return self->integer.integer;
}


/*!
	Implementation of GetValueAsString for integer values.
*/

void LEOGetIntegerValueAsString( LEOValuePtr self, char* outBuf, size_t bufSize, struct LEOContext* inContext )
{
	snprintf( outBuf, bufSize -1, "%lld", self->integer.integer );
}


/*!
	Implementation of SetValueAsNumber for integer values.
*/

void LEOSetIntegerValueAsNumber( LEOValuePtr self, LEONumber inNumber, struct LEOContext* inContext )
{
	if( trunc(inNumber) != inNumber )
	{
		snprintf( inContext->errMsg, sizeof(inContext->errMsg), "Can't make fractional number into integer." );
		inContext->keepRunning = false;
	}
	else
		self->integer.integer = inNumber;
}


/*!
	Implementation of SetValueAsInteger for integer values.
*/

void LEOSetIntegerValueAsInteger( LEOValuePtr self, LEOInteger inInteger, struct LEOContext* inContext )
{
	self->integer.integer = inInteger;
}


/*!
	Implementation of SetValueAsString for integer values. If the given string
	can't be fully converted to an integer, this will fail with an error message
	and abort execution of the current LEOContext.
*/

void LEOSetIntegerValueAsString( LEOValuePtr self, const char* inInteger, struct LEOContext* inContext )
{
	char*		endPtr = NULL;
	LEOInteger	theNum = strtoll( inInteger, &endPtr, 10 );
	if( endPtr != (inInteger +strlen(inInteger)) )
		LEOCantSetValueAsString( self, inInteger, inContext );
	else
		self->integer.integer = theNum;
}


/*!
	Implementation of InitCopy for integer values.
*/

void	LEOInitIntegerValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	dest->base.isa = &kLeoValueTypeInteger;
	if( keepReferences == kLEOInvalidateReferences )
		dest->base.refObjectID = kLEOObjectIDINVALID;
	dest->integer.integer = self->integer.integer;
}


/*!
	Destructor for integer values. If this value has references, this makes sure
	that they will produce an error message if they ever try to access it again.
*/

void	LEOCleanUpIntegerValue( LEOValuePtr self, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	self->base.isa = NULL;
	self->integer.integer = 0LL;
	if( keepReferences == kLEOInvalidateReferences && self->base.refObjectID != kLEOObjectIDINVALID )	// We have references? Make sure they all notice we've gone if they try to access us from now on.
		LEOContextGroupRecycleObjectID( inContext->group, self->base.refObjectID );
}


#pragma mark -
#pragma mark Dynamically allocated string

/*!
	@functiongroup LEOValueString
*/

void	LEOInitStringValue( LEOValuePtr inStorage, const char* inString, size_t inLen, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	inStorage->base.isa = &kLeoValueTypeString;
	if( keepReferences == kLEOInvalidateReferences )
		inStorage->base.refObjectID = kLEOObjectIDINVALID;
	size_t		theLen = inLen +1;
	inStorage->string.string = calloc( theLen, sizeof(char) );
	strncpy( inStorage->string.string, inString, theLen );
}


/*!
	Implementation of GetAsNumber for string values. If the given string can't
	be completely converted into a number, this will fail with an error message
	and abort execution of the current LEOContext.
*/

LEONumber	LEOGetStringValueAsNumber( LEOValuePtr self, struct LEOContext* inContext )
{
	char*		endPtr = NULL;
	LEONumber	num = strtod( self->string.string, &endPtr );
	if( endPtr != (self->string.string +strlen(self->string.string)) )
		LEOCantGetValueAsNumber( self, inContext );
	return num;
}


/*!
	Implementation of GetAsInteger for string values. If the given string can't
	be completely converted into an integer, this will fail with an error message
	and abort execution of the current LEOContext.
*/

LEOInteger	LEOGetStringValueAsInteger( LEOValuePtr self, struct LEOContext* inContext )
{
	char*		endPtr = NULL;
	LEOInteger	num = strtoll( self->string.string, &endPtr, 10 );
	if( endPtr != (self->string.string +strlen(self->string.string)) )
		LEOCantGetValueAsInteger( self, inContext );
	return num;
}


/*!
	Implementation of GetAsString for string values.
*/

void	LEOGetStringValueAsString( LEOValuePtr self, char* outBuf, size_t bufSize, struct LEOContext* inContext )
{
	strncpy( outBuf, self->string.string, bufSize );
}


/*!
	Implementation of SetAsNumber for string values.
*/

void	LEOSetStringValueAsNumber( LEOValuePtr self, LEONumber inNumber, struct LEOContext* inContext )
{
	if( self->string.string )
		free( self->string.string );
	self->string.string = calloc( OTHER_VALUE_SHORT_STRING_MAX_LENGTH, sizeof(char) );
	snprintf( self->string.string, OTHER_VALUE_SHORT_STRING_MAX_LENGTH, "%g", inNumber );
}


/*!
	Implementation of SetAsInteger for string values.
*/

void	LEOSetStringValueAsInteger( LEOValuePtr self, LEOInteger inInteger, struct LEOContext* inContext )
{
	if( self->string.string )
		free( self->string.string );
	self->string.string = calloc( OTHER_VALUE_SHORT_STRING_MAX_LENGTH, sizeof(char) );
	snprintf( self->string.string, OTHER_VALUE_SHORT_STRING_MAX_LENGTH, "%lld", inInteger );
}


/*!
	Implementation of GetAsBoolean for string values. If the given string can't
	be completely converted into a boolean, this will fail with an error message
	and abort execution of the current LEOContext.
*/

bool	LEOGetStringValueAsBoolean( LEOValuePtr self, struct LEOContext* inContext )
{
	if( strcasecmp( self->string.string, "true" ) == 0 )
		return true;
	else if( strcasecmp( self->string.string, "false" ) == 0 )
		return false;
	else
		return LEOCantGetValueAsBoolean( self, inContext );
}


/*!
	Implementation of GetAsRangeOfString for string values.
*/

void	LEOGetStringValueAsRangeOfString( LEOValuePtr self, LEOChunkType inType,
											size_t inRangeStart, size_t inRangeEnd,
											char* outBuf, size_t bufSize, struct LEOContext* inContext )
{
	size_t		outChunkStart = 0,
				outChunkEnd = 0,
				outDelChunkStart = 0,
				outDelChunkEnd = 0;
	LEOGetChunkRanges( self->string.string, inType,
						inRangeStart, inRangeEnd,
						&outChunkStart, &outChunkEnd,
						&outDelChunkStart, &outDelChunkEnd, inContext->itemDelimiter );
	size_t		len = outChunkEnd -outChunkStart;
	if( len > bufSize )
		len = bufSize -1;
	memmove( outBuf, self->string.string +outChunkStart, len );
	outBuf[len] = 0;
}



/*!
	Implementation of SetAsString for string values.
*/

void LEOSetStringValueAsString( LEOValuePtr self, const char* inString, struct LEOContext* inContext )
{
	if( self->string.string )
		free( self->string.string );
	size_t		theLen = strlen(inString) +1;
	self->string.string = calloc( theLen, sizeof(char) );
	strncpy( self->string.string, inString, theLen );
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
	if( self->string.string )
		free( self->string.string );
	
	self->base.isa = &kLeoValueTypeStringConstant;
	self->string.string = (char*) inString;
}


/*!
	Implementation of InitCopy for string values.
*/

void	LEOInitStringValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	dest->base.isa = &kLeoValueTypeString;
	if( keepReferences == kLEOInvalidateReferences )
		dest->base.refObjectID = kLEOObjectIDINVALID;
	size_t		theLen = strlen( self->string.string ) +1;
	dest->string.string = calloc( theLen, sizeof(char) );
	strncpy( dest->string.string, self->string.string, theLen );
}


void	LEODetermineChunkRangeOfSubstringOfStringValue( LEOValuePtr self, size_t *ioBytesStart, size_t *ioBytesEnd,
														size_t *ioBytesDelStart, size_t *ioBytesDelEnd,
														LEOChunkType inType, size_t inRangeStart, size_t inRangeEnd,
														struct LEOContext* inContext )
{
	char*		str = self->string.string;
	size_t		maxOffs = *ioBytesEnd -((size_t)str);
	str += (*ioBytesStart);
	
	size_t		chunkStart, chunkEnd, delChunkStart, delChunkEnd;
	
	LEOGetChunkRanges( str, inType,
						inRangeStart, inRangeEnd,
						&chunkStart, &chunkEnd,
						&delChunkStart, &delChunkEnd,
						inContext->itemDelimiter );
	if( chunkStart > maxOffs )
		chunkStart = maxOffs;
	if( chunkEnd > maxOffs )
		chunkEnd = maxOffs;
	(*ioBytesStart) += chunkStart;
	(*ioBytesEnd) = (*ioBytesStart) + (chunkEnd -chunkStart);
	if( delChunkStart > maxOffs )
		delChunkStart = maxOffs;
	if( delChunkEnd > maxOffs )
		delChunkEnd = maxOffs;
	(*ioBytesDelStart) = (*ioBytesStart) +delChunkStart;
	(*ioBytesDelEnd) = (*ioBytesDelStart) + (delChunkEnd -delChunkStart);
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
				selfLen = strlen( self->string.string ),
				finalLen = 0;
	LEOGetChunkRanges( self->string.string, inType,
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
		
	char*		newStr = calloc( finalLen +1, sizeof(char) );
	memmove( newStr, self->string.string, outChunkStart );	// Copy before chunk.
	if( inBufLen > 0 )
		memmove( newStr +outChunkStart, inBuf, inBufLen );	// Copy new value of chunk.
	memmove( newStr +outChunkStart +inBufLen, self->string.string +outChunkEnd, selfLen -outChunkEnd );	// Copy after chunk.
	newStr[finalLen] = 0;
	
	free( self->string.string );
	self->string.string = newStr;
}


/*!
	Implementation of SetPredeterminedRangeAsString for string values.
*/

void	LEOSetStringValuePredeterminedRangeAsString( LEOValuePtr self,
											size_t inRangeStart, size_t inRangeEnd,
											const char* inBuf, struct LEOContext* inContext )
{
	size_t		inBufLen = inBuf ? strlen(inBuf) : 0,
				selfLen = strlen( self->string.string ),
				finalLen = 0,
				chunkLen = inRangeEnd -inRangeStart;
	finalLen = selfLen -chunkLen +inBufLen;
		
	char*		newStr = calloc( finalLen +1, sizeof(char) );
	memmove( newStr, self->string.string, inRangeStart );	// Copy before chunk.
	if( inBufLen > 0 )
		memmove( newStr +inRangeStart, inBuf, inBufLen );	// Copy new value of chunk.
	memmove( newStr +inRangeStart +inBufLen, self->string.string +inRangeEnd, selfLen -inRangeEnd );	// Copy after chunk.
	newStr[finalLen] = 0;
	
	free( self->string.string );
	self->string.string = newStr;
}


/*!
	Destructor for string values. If this value has references, this makes sure
	that they will produce an error message if they ever try to access it again.
*/

void	LEOCleanUpStringValue( LEOValuePtr self, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	self->base.isa = NULL;
	if( self->string.string )
		free( self->string.string );
	self->string.string = NULL;
	if( keepReferences == kLEOInvalidateReferences && self->base.refObjectID != kLEOObjectIDINVALID )
		LEOContextGroupRecycleObjectID( inContext->group, self->base.refObjectID );
}


bool	LEOCanGetStringValueAsNumber( LEOValuePtr self, struct LEOContext* inContext )
{
	for( size_t x = 0; self->string.string[x] != 0; x++ )
	{
		if( self->string.string[x] < '0' || self->string.string[x] > '9' )
			return false;
	}
	
	return true;
}


#pragma mark -
#pragma mark String Constant

/*!
	@functiongroup LEOValueStringConstant
*/

void	LEOInitStringConstantValue( LEOValuePtr inStorage, const char* inString, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	inStorage->base.isa = &kLeoValueTypeStringConstant;
	if( keepReferences == kLEOInvalidateReferences )
		inStorage->base.refObjectID = kLEOObjectIDINVALID;
	inStorage->string.string = (char*)inString;
}


/*!
	Implementation of SetAsNumber for string constant values. This turns the
	value into a regular (dynamic) string value.
*/

void	LEOSetStringConstantValueAsNumber( LEOValuePtr self, LEONumber inNumber, struct LEOContext* inContext )
{
	// Turn this into a non-constant string:
	self->base.isa = &kLeoValueTypeString;
	self->string.string = calloc( OTHER_VALUE_SHORT_STRING_MAX_LENGTH, sizeof(char) );
	snprintf( self->string.string, OTHER_VALUE_SHORT_STRING_MAX_LENGTH, "%g", inNumber );
}


/*!
	Implementation of SetAsInteger for string constant values. This turns the
	value into a regular (dynamic) string value.
*/

void	LEOSetStringConstantValueAsInteger( LEOValuePtr self, LEOInteger inInteger, struct LEOContext* inContext )
{
	// Turn this into a non-constant string:
	self->base.isa = &kLeoValueTypeString;
	self->string.string = calloc( OTHER_VALUE_SHORT_STRING_MAX_LENGTH, sizeof(char) );
	snprintf( self->string.string, OTHER_VALUE_SHORT_STRING_MAX_LENGTH, "%lld", inInteger );
}


/*!
	Implementation of SetAsString for string constant values. This turns the
	value into a regular (dynamic) string value.
*/

void	LEOSetStringConstantValueAsString( LEOValuePtr self, const char* inString, struct LEOContext* inContext )
{
	// Turn this into a non-constant string:
	self->base.isa = &kLeoValueTypeString;
	size_t		theLen = strlen(inString) +1;
	self->string.string = calloc( theLen, sizeof(char) );
	strncpy( self->string.string, inString, theLen );
}


/*!
	Implementation of SetAsString for string constant values.
*/

void	LEOSetStringConstantValueAsBoolean( LEOValuePtr self, bool inBoolean, struct LEOContext* inContext )
{
	self->string.string = (inBoolean ? "true" : "false");
}


/*!
	Implementation of InitCopy for string constant values.
*/

void	LEOInitStringConstantValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	dest->base.isa = &kLeoValueTypeStringConstant;
	if( keepReferences == kLEOInvalidateReferences )
		dest->base.refObjectID = kLEOObjectIDINVALID;
	dest->string.string = self->string.string;
}


/*!
	Implementation of SetRangeAsString for string constant values. This turns the
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
				selfLen = strlen( self->string.string ),
				finalLen = 0;
	LEOGetChunkRanges( self->string.string, inType,
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
		
	char*		newStr = calloc( finalLen +1, sizeof(char) );
	memmove( newStr, self->string.string, outChunkStart );	// Copy before chunk.
	if( inBufLen > 0 )
		memmove( newStr +outChunkStart, inBuf, inBufLen );	// Copy new value of chunk.
	memmove( newStr +outChunkStart +inBufLen, self->string.string +outChunkEnd, selfLen -outChunkEnd );	// Copy after chunk.
	newStr[finalLen] = 0;
	
	// Turn this into a non-constant string:
	self->base.isa = &kLeoValueTypeString;
	self->string.string = newStr;
}


/*!
	Implementation of SetPredeterminedRangeAsString for string constant values.
*/

void	LEOSetStringConstantValuePredeterminedRangeAsString( LEOValuePtr self,
											size_t inRangeStart, size_t inRangeEnd,
											const char* inBuf, struct LEOContext* inContext )
{
	LEOSetStringConstantValueRangeAsString( self, kLEOChunkTypeByte, inRangeStart, inRangeEnd, inBuf, inContext );
}


/*!
	Destructor for string constant values. If this value has references, this
	makes sure that they will produce an error message if they ever try to
	access it again.
*/

void	LEOCleanUpStringConstantValue( LEOValuePtr self, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	self->base.isa = NULL;
	self->string.string = NULL;
	if( keepReferences == kLEOInvalidateReferences && self->base.refObjectID != kLEOObjectIDINVALID )
		LEOContextGroupRecycleObjectID( inContext->group, self->base.refObjectID );
}


#pragma mark -
#pragma mark Boolean

/*!
	@functiongroup LEOValueBoolean
*/

void	LEOInitBooleanValue( LEOValuePtr self, bool inBoolean, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	self->base.isa = &kLeoValueTypeBoolean;
	if( keepReferences == kLEOInvalidateReferences )
		self->base.refObjectID = kLEOObjectIDINVALID;
	self->boolean.boolean = inBoolean;
}


/*!
	Implementation of GetAsString for boolean values.
*/

void	LEOGetBooleanValueAsString( LEOValuePtr self, char* outBuf, size_t bufSize, struct LEOContext* inContext )
{
	strncpy( outBuf, (self->boolean.boolean ? "true" : "false"), bufSize -1 );
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
	dest->base.isa = &kLeoValueTypeBoolean;
	if( keepReferences == kLEOInvalidateReferences )
		dest->base.refObjectID = kLEOObjectIDINVALID;
	dest->boolean.boolean = self->boolean.boolean;
}


/*!
	Destructor for boolean values. If this value has references, this makes sure
	that they will produce an error message if they ever try to access it again.
*/

void	LEOCleanUpBooleanValue( LEOValuePtr self, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	self->base.isa = NULL;
	self->boolean.boolean = false;
	if( keepReferences == kLEOInvalidateReferences && self->base.refObjectID != kLEOObjectIDINVALID )
		LEOContextGroupRecycleObjectID( inContext->group, self->base.refObjectID );
}



#pragma mark -
#pragma mark Reference


/*!
	@functiongroup LEOValueReference
*/

void	LEOInitReferenceValue( LEOValuePtr self, LEOValuePtr originalValue, LEOKeepReferencesFlag keepReferences,
								LEOChunkType inType, size_t startOffs, size_t endOffs, struct LEOContext* inContext )
{
	self->base.isa = &kLeoValueTypeReference;
	if( keepReferences == kLEOInvalidateReferences )
		self->base.refObjectID = kLEOObjectIDINVALID;
	
	if( originalValue->base.refObjectID == kLEOObjectIDINVALID )
	{
		originalValue->base.refObjectID = LEOContextGroupCreateNewObjectIDForPointer( inContext->group, originalValue );
	}
	
	self->reference.objectID = originalValue->base.refObjectID;
	self->reference.objectSeed = LEOContextGroupGetSeedForObjectID( inContext->group, originalValue->base.refObjectID );
	
	self->reference.chunkType = inType;
	self->reference.chunkStart = startOffs;
	self->reference.chunkEnd = endOffs;
}


/*!
	Implementation of GetAsString for reference values.
*/

void	LEOGetReferenceValueAsString( LEOValuePtr self, char* outBuf, size_t bufSize, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue == NULL )
	{
		snprintf( inContext->errMsg, sizeof(inContext->errMsg) -1, "The referenced value doesn't exist anymore." );
		inContext->keepRunning = false;
	}
	else if( self->reference.chunkType != kLEOChunkTypeINVALID )
		LEOGetValueAsRangeOfString( theValue, self->reference.chunkType, self->reference.chunkStart, self->reference.chunkEnd, outBuf, bufSize, inContext );
	else
		LEOGetValueAsString( theValue, outBuf, bufSize, inContext );
}


/*!
	Implementation of GetAsNumber for reference values.
*/

LEONumber	LEOGetReferenceValueAsNumber( LEOValuePtr self, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue == NULL )
	{
		snprintf( inContext->errMsg, sizeof(inContext->errMsg) -1, "The referenced value doesn't exist anymore." );
		inContext->keepRunning = false;
		return 0.0;
	}
	else if( self->reference.chunkType != kLEOChunkTypeINVALID )
	{
		char		str[OTHER_VALUE_SHORT_STRING_MAX_LENGTH] = {0};	// Can get away with this as long as they're only numbers, booleans etc.
		LEOGetValueAsRangeOfString( theValue, self->reference.chunkType, self->reference.chunkStart, self->reference.chunkEnd, str, sizeof(str), inContext );
		char*		endPtr = NULL;
		LEONumber	num = strtod( str, &endPtr );
		if( endPtr != (str +strlen(str)) )
			LEOCantGetValueAsNumber( self, inContext );
		return num;
	}
	else
		return LEOGetValueAsNumber( theValue, inContext );
}


/*!
	Implementation of GetAsInteger for reference values.
*/

LEOInteger	LEOGetReferenceValueAsInteger( LEOValuePtr self, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue == NULL )
	{
		snprintf( inContext->errMsg, sizeof(inContext->errMsg) -1, "The referenced value doesn't exist anymore." );
		inContext->keepRunning = false;
		return 0LL;
	}
	else if( self->reference.chunkType != kLEOChunkTypeINVALID )
	{
		char		str[OTHER_VALUE_SHORT_STRING_MAX_LENGTH] = {0};	// Can get away with this as long as they're only numbers, booleans etc.
		LEOGetValueAsRangeOfString( theValue, self->reference.chunkType, self->reference.chunkStart, self->reference.chunkEnd, str, sizeof(str), inContext );
		char*		endPtr = NULL;
		LEOInteger	num = strtoll( str, &endPtr, 10 );
		if( endPtr != (str +strlen(str)) )
			LEOCantGetValueAsInteger( self, inContext );
		return num;
	}
	else
		return LEOGetValueAsInteger( theValue, inContext );
}


/*!
	Implementation of GetAsBoolean for reference values.
*/

bool	LEOGetReferenceValueAsBoolean( LEOValuePtr self, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue == NULL )
	{
		snprintf( inContext->errMsg, sizeof(inContext->errMsg) -1, "The referenced value doesn't exist anymore." );
		inContext->keepRunning = false;
		return false;
	}
	else if( self->reference.chunkType != kLEOChunkTypeINVALID )
	{
		char		str[OTHER_VALUE_SHORT_STRING_MAX_LENGTH] = {0};	// Can get away with this as long as they're only numbers, booleans etc.
		LEOGetValueAsRangeOfString( theValue, self->reference.chunkType, self->reference.chunkStart, self->reference.chunkEnd, str, sizeof(str), inContext );
		if( strcasecmp( str, "true" ) == 0 )
			return true;
		else if( strcasecmp( str, "false" ) == 0 )
			return false;
		else
			return LEOCantGetValueAsBoolean( self, inContext );
	}
	else
		return LEOGetValueAsBoolean( theValue, inContext );
}


/*!
	Implementation of GetAsRangeOfString for reference values.
*/

void	LEOGetReferenceValueAsRangeOfString( LEOValuePtr self, LEOChunkType inType,
											size_t inRangeStart, size_t inRangeEnd,
											char* outBuf, size_t bufSize, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue == NULL )
	{
		snprintf( inContext->errMsg, sizeof(inContext->errMsg) -1, "The referenced value doesn't exist anymore." );
		inContext->keepRunning = false;
	}
	else if( self->reference.chunkType != kLEOChunkTypeINVALID )
	{
		size_t		chunkStart = 0, chunkEnd = SIZE_MAX, chunkDelStart, chunkDelEnd;
		LEODetermineChunkRangeOfSubstring( self, &chunkStart, &chunkEnd, &chunkDelStart, &chunkDelEnd,
											inType, inRangeStart, inRangeEnd, inContext );
		LEOGetValueAsRangeOfString( theValue, kLEOChunkTypeByte, chunkStart, chunkEnd, outBuf, bufSize, inContext );
	}
	else
		LEOGetValueAsRangeOfString( theValue, inType, inRangeStart, inRangeEnd, outBuf, bufSize, inContext );
}


/*!
	Implementation of SetAsString for reference values.
*/

void	LEOSetReferenceValueAsString( LEOValuePtr self, const char* inString, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue == NULL )
	{
		snprintf( inContext->errMsg, sizeof(inContext->errMsg) -1, "The referenced value doesn't exist anymore." );
		inContext->keepRunning = false;
	}
	else if( self->reference.chunkType != kLEOChunkTypeINVALID )
	{
		size_t		chunkStart = 0, chunkEnd = SIZE_MAX, chunkDelStart, chunkDelEnd;
		LEODetermineChunkRangeOfSubstring( theValue, &chunkStart, &chunkEnd, &chunkDelStart, &chunkDelEnd,
											self->reference.chunkType, self->reference.chunkStart, self->reference.chunkEnd, inContext );
		LEOSetValuePredeterminedRangeAsString( theValue, chunkStart, chunkEnd, inString, inContext );
	}
	else
		LEOSetValueAsString( theValue, inString, inContext );
}


/*!
	Implementation of SetAsBoolean for reference values.
*/

void	LEOSetReferenceValueAsBoolean( LEOValuePtr self, bool inBoolean, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue == NULL )
	{
		snprintf( inContext->errMsg, sizeof(inContext->errMsg) -1, "The referenced value doesn't exist anymore." );
		inContext->keepRunning = false;
	}
	else if( self->reference.chunkType != kLEOChunkTypeINVALID )
	{
		LEOSetValueRangeAsString( theValue, self->reference.chunkType, self->reference.chunkStart, self->reference.chunkEnd,
									(inBoolean) ? "true" : "false", inContext );
	}
	else
		LEOSetValueAsBoolean( theValue, inBoolean, inContext );
}


/*!
	Implementation of SetAsNumber for reference values.
*/

void	LEOSetReferenceValueAsNumber( LEOValuePtr self, LEONumber inNumber, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue == NULL )
	{
		snprintf( inContext->errMsg, sizeof(inContext->errMsg) -1, "The referenced value doesn't exist anymore." );
		inContext->keepRunning = false;
	}
	else if( self->reference.chunkType != kLEOChunkTypeINVALID )
	{
		char		str[OTHER_VALUE_SHORT_STRING_MAX_LENGTH] = {0};	// Can get away with this as long as they're only numbers, booleans etc.
		snprintf( str, sizeof(str), "%g", inNumber );
		LEOSetValueRangeAsString( theValue, self->reference.chunkType, self->reference.chunkStart, self->reference.chunkEnd,
									str, inContext );
	}
	else
		LEOSetValueAsNumber( theValue, inNumber, inContext );
}


/*!
	Implementation of SetAsInteger for reference values.
*/

void	LEOSetReferenceValueAsInteger( LEOValuePtr self, LEOInteger inInteger, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue == NULL )
	{
		snprintf( inContext->errMsg, sizeof(inContext->errMsg) -1, "The referenced value doesn't exist anymore." );
		inContext->keepRunning = false;
	}
	else if( self->reference.chunkType != kLEOChunkTypeINVALID )
	{
		char		str[OTHER_VALUE_SHORT_STRING_MAX_LENGTH] = {0};	// Can get away with this as long as they're only numbers, booleans etc.
		snprintf( str, sizeof(str), "%lld", inInteger );
		LEOSetValueRangeAsString( theValue, self->reference.chunkType, self->reference.chunkStart, self->reference.chunkEnd,
									str, inContext );
	}
	else
		LEOSetValueAsInteger( theValue, inInteger, inContext );
}


/*!
	Implementation of SetRangeAsString for reference values.
*/

void	LEOSetReferenceValueRangeAsString( LEOValuePtr self, LEOChunkType inType,
											size_t inRangeStart, size_t inRangeEnd,
											const char* inBuf, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue == NULL )
	{
		snprintf( inContext->errMsg, sizeof(inContext->errMsg) -1, "The referenced value doesn't exist anymore." );
		inContext->keepRunning = false;
	}
	else if( self->reference.chunkType != kLEOChunkTypeINVALID )
	{
		size_t		chunkStart = 0, chunkEnd = SIZE_MAX, chunkDelStart, chunkDelEnd;
		LEODetermineChunkRangeOfSubstring( self, &chunkStart, &chunkEnd, &chunkDelStart, &chunkDelEnd,
											inType, inRangeStart, inRangeEnd, inContext );
		LEOSetValuePredeterminedRangeAsString( theValue, chunkStart, chunkEnd, inBuf, inContext );
	}
	else
		LEOSetValueRangeAsString( theValue, inType, inRangeStart, inRangeEnd, inBuf, inContext );
}


/*!
	Implementation of SetPredeterminedRangeAsString for reference values.
*/

void	LEOSetReferenceValuePredeterminedRangeAsString( LEOValuePtr self, size_t inRangeStart, size_t inRangeEnd,
														const char* inBuf, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue == NULL )
	{
		snprintf( inContext->errMsg, sizeof(inContext->errMsg) -1, "The referenced value doesn't exist anymore." );
		inContext->keepRunning = false;
	}
	else
		LEOSetValuePredeterminedRangeAsString( theValue, inRangeStart, inRangeEnd, inBuf, inContext );
}


/*!
	Implementation of InitCopy for reference values. This method is safe to call
	even if the original value this reference points to has already gone away.
*/

void	LEOInitReferenceValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	dest->base.isa = &kLeoValueTypeReference;
	if( keepReferences == kLEOInvalidateReferences )
		dest->base.refObjectID = kLEOObjectIDINVALID;
	dest->reference.objectID = self->reference.objectID;
	dest->reference.objectSeed = self->reference.objectSeed;
	
	dest->reference.chunkType = self->reference.chunkType;
	dest->reference.chunkStart = self->reference.chunkStart;
	dest->reference.chunkEnd = self->reference.chunkEnd;
}


/*!
	Implementation of InitCopy for reference values. This method is safe to call
	even if the original value this reference points to has already gone away.
*/

void	LEOInitReferenceValueSimpleCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue == NULL )
	{
		snprintf( inContext->errMsg, sizeof(inContext->errMsg) -1, "The referenced value doesn't exist anymore." );
		inContext->keepRunning = false;
	}
	else
		LEOInitSimpleCopy(theValue, dest, keepReferences, inContext);
}


void	LEODetermineChunkRangeOfSubstringOfReferenceValue( LEOValuePtr self, size_t *ioBytesStart, size_t *ioBytesEnd,
															size_t *ioBytesDelStart, size_t *ioBytesDelEnd,
															LEOChunkType inType, size_t inRangeStart, size_t inRangeEnd,
															struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue == NULL )
	{
		snprintf( inContext->errMsg, sizeof(inContext->errMsg) -1, "The referenced value doesn't exist anymore." );
		inContext->keepRunning = false;
	}
	else if( self->reference.chunkType != kLEOChunkTypeINVALID )
	{
		size_t		bytesStart = 0, bytesEnd = SIZE_MAX,
					bytesDelStart, bytesDelEnd; 
		LEODetermineChunkRangeOfSubstring( theValue, &bytesStart, &bytesEnd, &bytesDelStart, &bytesDelEnd,
											self->reference.chunkType, self->reference.chunkStart, self->reference.chunkEnd, inContext );
		LEODetermineChunkRangeOfSubstring( theValue, &bytesStart, &bytesEnd, &bytesDelStart, &bytesDelEnd,
											kLEOChunkTypeByte, *ioBytesStart, *ioBytesEnd, inContext );
		LEODetermineChunkRangeOfSubstring( theValue, &bytesStart, &bytesEnd, &bytesDelStart, &bytesDelEnd,
											inType, inRangeStart, inRangeEnd, inContext );
		*ioBytesStart = bytesStart;
		*ioBytesEnd = bytesEnd;
		*ioBytesDelStart = bytesDelStart;
		*ioBytesDelEnd = bytesDelEnd;
	}
	else
	{
		LEODetermineChunkRangeOfSubstring( theValue, ioBytesStart, ioBytesEnd, ioBytesDelStart, ioBytesDelEnd,
											inType, inRangeStart, inRangeEnd, inContext );
	}
}


bool	LEOCanGetReferenceValueAsNumber( LEOValuePtr self, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue != NULL )
		return LEOCanGetAsNumber( theValue, inContext );
	else
		return false;
}


/*!
	Destructor for reference values. If this value has references, this makes sure
	that they will produce an error message if they ever try to access it again.
	Yes, you can have references to references. Use this power wisely.
*/

void	LEOCleanUpReferenceValue( LEOValuePtr self, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	self->base.isa = NULL;
	self->reference.objectID = kLEOObjectIDINVALID;
	self->reference.objectSeed = 0;
	if( keepReferences == kLEOInvalidateReferences && self->base.refObjectID != kLEOObjectIDINVALID )	// We have references? Make sure they all notice we've gone if they try to access us from now on.
		LEOContextGroupRecycleObjectID( inContext->group, self->base.refObjectID );
}

#pragma mark -
#pragma mark Variants


void	LEOInitNumberVariantValue( LEOValuePtr self, LEONumber inNumber, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	LEOInitNumberValue( self, inNumber, keepReferences, inContext );
	self->base.isa = &kLeoValueTypeNumberVariant;
}


void	LEOInitIntegerVariantValue( LEOValuePtr self, LEOInteger inInteger, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	LEOInitIntegerValue( self, inInteger, keepReferences, inContext );
	self->base.isa = &kLeoValueTypeIntegerVariant;
}


void	LEOInitStringVariantValue( LEOValuePtr self, const char* inString, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	LEOInitStringValue( self, inString, strlen(inString), keepReferences, inContext );
	self->base.isa = &kLeoValueTypeStringVariant;
}


void	LEOInitBooleanVariantValue( LEOValuePtr self, bool inBoolean, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	LEOInitBooleanValue( self, inBoolean, keepReferences, inContext );
	self->base.isa = &kLeoValueTypeBooleanVariant;
}


void	LEOSetVariantValueAsNumber( LEOValuePtr self, LEONumber inNumber, struct LEOContext* inContext )
{
	LEOCleanUpValue( self, kLEOKeepReferences, inContext );
	LEOInitNumberValue( self, inNumber, kLEOKeepReferences, inContext );
	self->base.isa = &kLeoValueTypeNumberVariant;
}


void	LEOSetVariantValueAsInteger( LEOValuePtr self, LEOInteger inInteger, struct LEOContext* inContext )
{
	LEOCleanUpValue( self, kLEOKeepReferences, inContext );
	LEOInitIntegerValue( self, inInteger, kLEOKeepReferences, inContext );
	self->base.isa = &kLeoValueTypeIntegerVariant;
}


void	LEOSetVariantValueAsString( LEOValuePtr self, const char* inString, struct LEOContext* inContext )
{
	LEOCleanUpValue( self, kLEOKeepReferences, inContext );
	LEOInitStringValue( self, inString, strlen(inString), kLEOKeepReferences, inContext );
	self->base.isa = &kLeoValueTypeStringVariant;
}


void	LEOSetVariantValueAsBoolean( LEOValuePtr self, bool inBoolean, struct LEOContext* inContext )				// Makes it a constant string.
{
	LEOCleanUpValue( self, kLEOKeepReferences, inContext );
	LEOInitBooleanValue( self, inBoolean, kLEOKeepReferences, inContext );
	self->base.isa = &kLeoValueTypeBooleanVariant;
}


void	LEOSetVariantValueRangeAsString( LEOValuePtr self, LEOChunkType inType,
											size_t inRangeStart, size_t inRangeEnd,
											const char* inBuf, struct LEOContext* inContext )
{
	if( self->base.isa != &kLeoValueTypeStringVariant )	// Convert to string first.
	{
		char		shortStr[OTHER_VALUE_SHORT_STRING_MAX_LENGTH] = {0};
		LEOGetValueAsString( self, shortStr, sizeof(shortStr), inContext );
		LEOCleanUpValue( self, kLEOKeepReferences, inContext );
		LEOInitStringValue( self, shortStr, strlen(shortStr), kLEOKeepReferences, inContext );
		self->base.isa = &kLeoValueTypeStringVariant;
	}
	LEOSetStringValueRangeAsString( self, inType, inRangeStart, inRangeEnd, inBuf, inContext );
}


void	LEOSetVariantValuePredeterminedRangeAsString( LEOValuePtr self,
													size_t inRangeStart, size_t inRangeEnd,
													const char* inBuf, struct LEOContext* inContext )
{
	LEOSetVariantValueRangeAsString( self, kLEOChunkTypeByte,
										inRangeStart, inRangeEnd,
										inBuf, inContext );
}


void	LEOInitNumberVariantValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	LEOInitNumberValueCopy( self, dest, keepReferences, inContext );
	dest->base.isa = &kLeoValueTypeNumberVariant;
}


void	LEOInitIntegerVariantValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	LEOInitIntegerValueCopy( self, dest, keepReferences, inContext );
	dest->base.isa = &kLeoValueTypeIntegerVariant;
}


void	LEOInitBooleanVariantValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	LEOInitBooleanValueCopy( self, dest, keepReferences, inContext );
	dest->base.isa = &kLeoValueTypeBooleanVariant;
}


void	LEOInitStringVariantValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	LEOInitStringValueCopy( self, dest, keepReferences, inContext );
	dest->base.isa = &kLeoValueTypeStringVariant;
}


#pragma mark -
#pragma mark Arrays

void	LEOInitArrayValue( LEOValuePtr self, struct LEOArrayEntry *inArray, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	self->base.isa = &kLeoValueTypeArray;
	if( keepReferences == kLEOInvalidateReferences )
		self->base.refObjectID = kLEOObjectIDINVALID;
	self->array.array = inArray;	// *** takes over ownership.
}


void	LEOGetArrayValueAsString( LEOValuePtr self, char* outBuf, size_t bufSize, struct LEOContext* inContext )
{
	inContext->keepRunning = false;
	snprintf( inContext->errMsg, sizeof(inContext->errMsg), "Can't make %s into a string", self->base.isa->displayTypeName );
}


void	LEOGetArrayValueAsRangeOfString( LEOValuePtr self, LEOChunkType inType,
									size_t inRangeStart, size_t inRangeEnd,
									char* outBuf, size_t bufSize, struct LEOContext* inContext )
{
	inContext->keepRunning = false;
	snprintf( inContext->errMsg, sizeof(inContext->errMsg), "Can't make %s into a string", self->base.isa->displayTypeName );
}


void	LEODetermineChunkRangeOfSubstringOfArrayValue( LEOValuePtr self, size_t *ioBytesStart, size_t *ioBytesEnd,
														size_t *ioBytesDelStart, size_t *ioBytesDelEnd,
														LEOChunkType inType, size_t inRangeStart, size_t inRangeEnd,
														struct LEOContext* inContext )
{
	inContext->keepRunning = false;
	snprintf( inContext->errMsg, sizeof(inContext->errMsg), "Can't make %s into a string", self->base.isa->displayTypeName );
}


void	LEOInitArrayValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	dest->base.isa = &kLeoValueTypeArray;
	if( keepReferences == kLEOInvalidateReferences )
		dest->base.refObjectID = kLEOObjectIDINVALID;
	dest->array.array = LEOCopyArray( self->array.array, inContext );
}


void	LEOCleanUpArrayValue( LEOValuePtr self, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	self->base.isa = NULL;
	LEOCleanUpArray( self->array.array, inContext );
	self->array.array = NULL;
	if( keepReferences == kLEOInvalidateReferences && self->base.refObjectID != kLEOObjectIDINVALID )	// We have references? Make sure they all notice we've gone if they try to access us from now on.
		LEOContextGroupRecycleObjectID( inContext->group, self->base.refObjectID );
}


#pragma mark -


struct LEOArrayEntry
{
	struct LEOArrayEntry	*	smallerItem;
	struct LEOArrayEntry	*	largerItem;
	union LEOValue				value;
	char						key[0];	// Must be last, dynamically sized array.
};


struct LEOArrayEntry	*	LEOAllocNewEntry( const char* inKey, LEOValuePtr inValue, struct LEOContext* inContext )
{
	struct LEOArrayEntry	*	newEntry = NULL;
	size_t						inKeyLen = strlen(inKey);
	newEntry = calloc( sizeof(struct LEOArrayEntry) +inKeyLen +1, 1 );
	memmove( newEntry->key, inKey, inKeyLen +1 ); 
	LEOInitCopy( inValue, &newEntry->value, kLEOInvalidateReferences, inContext );
	newEntry->smallerItem = NULL;
	newEntry->largerItem = NULL;
	
	return newEntry;
}


void	LEOAddArrayEntryToRoot( struct LEOArrayEntry** arrayPtrByReference, const char* inKey, LEOValuePtr inValue, struct LEOContext* inContext )
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


void	LEODeleteArrayEntryFromRoot( struct LEOArrayEntry** arrayPtrByReference, const char* inKey, struct LEOContext* inContext )
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
			
			if( currEntry->smallerItem && currEntry->largerItem )	// Have two sub-trees?
			{
				// Append the smaller subtree to the larger subtree:
				*parentPtr = currEntry->largerItem;
				struct LEOArrayEntry*	currEntry2 = currEntry->smallerItem;
				
				while( true )
				{
					int		cmpResult2 = strcasecmp( currEntry2->key, currEntry->smallerItem->key );
					assert( cmpResult2 != 0 );
					if( cmpResult2 > 0 )	// Key is larger? Go down 'larger' side one step.
					{
						if( currEntry2->largerItem == NULL )	// No larger side? Insert here!
						{
							currEntry2->largerItem = currEntry->smallerItem;
							break;
						}
						currEntry2 = currEntry2->largerItem;
					}
					else if( cmpResult2 < 0 )	// Key is smaller? Go down 'smaller' side one step.
					{
						if( currEntry2->smallerItem == NULL )	// No smaller side? Insert here!
						{
							currEntry2->smallerItem = currEntry->smallerItem;
							break;
						}
						currEntry2 = currEntry2->smallerItem;
					}
				}
			}
			else if( currEntry->smallerItem )
			{
				*parentPtr = currEntry->smallerItem;
			}
			else if( currEntry->largerItem )
			{
				*parentPtr = currEntry->largerItem;
			}
			
			free( currEntry );
		}
	}
}


struct LEOArrayEntry*	LEOCopyArray( struct LEOArrayEntry* arrayPtr, struct LEOContext* inContext )
{
	struct LEOArrayEntry*	entryCopy = LEOAllocNewEntry( arrayPtr->key, &arrayPtr->value, inContext );
	
	if( arrayPtr->smallerItem )
		entryCopy->smallerItem = LEOCopyArray( arrayPtr->smallerItem, inContext );
	
	if( arrayPtr->largerItem )
		entryCopy->largerItem = LEOCopyArray( arrayPtr->largerItem, inContext );
	
	return entryCopy;
}


LEOValuePtr		LEOGetArrayValueForKey( struct LEOArrayEntry* arrayPtr, const char* inKey )
{
	struct LEOArrayEntry*	currEntry = arrayPtr;
	
	while( currEntry )
	{
		int			cmpResult = strcasecmp( currEntry->key, inKey );
		if( cmpResult > 0 )	// Key is larger? Go down 'larger' side one step.
			currEntry = currEntry->largerItem;
		else if( cmpResult < 0 )	// Key is smaller? Go down 'smaller' side one step.
			currEntry = currEntry->smallerItem;
		else if( cmpResult == 0 )	// Found key!
			return &currEntry->value;
	}
	
	return NULL;
}


void	LEOCleanUpArray( struct LEOArrayEntry* arrayPtr, struct LEOContext* inContext )
{
	if( arrayPtr->smallerItem )
	{
		LEOCleanUpArray( arrayPtr->smallerItem, inContext );
		arrayPtr->smallerItem = NULL;
	}
	if( arrayPtr->largerItem )
	{
		LEOCleanUpArray( arrayPtr->largerItem, inContext );
		arrayPtr->smallerItem = NULL;
	}
	
	LEOCleanUpValue( &arrayPtr->value, kLEOInvalidateReferences, inContext );
	free( arrayPtr );
}


