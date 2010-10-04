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
	LEOCantGetValueAsObjectID,
	LEOCantGetValueAsBoolean,
	LEOGetAnyValueAsRangeOfString,	// Only works as long as numbers can't be longer than OTHER_VALUE_SHORT_STRING_MAX_LENGTH as strings.
	
	LEOSetNumberValueAsNumber,
	LEOSetNumberValueAsString,
	LEOCantSetValueAsObjectID,
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
	LEOCantGetValueAsObjectID,
	LEOGetStringValueAsBoolean,
	LEOGetStringValueAsRangeOfString,
	
	LEOSetStringValueAsNumber,
	LEOSetStringValueAsString,
	LEOCantSetValueAsObjectID,
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
	LEOCantGetValueAsObjectID,
	LEOGetStringValueAsBoolean,
	LEOGetStringValueAsRangeOfString,
	
	LEOSetStringConstantValueAsNumber,
	LEOSetStringConstantValueAsString,
	LEOCantSetValueAsObjectID,
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
	LEOCantGetValueAsObjectID,
	LEOGetBooleanValueAsBoolean,
	LEOGetAnyValueAsRangeOfString,	// Only works as long as booleans can't be longer than OTHER_VALUE_SHORT_STRING_MAX_LENGTH as strings.
	
	LEOCantSetValueAsNumber,
	LEOSetBooleanValueAsString,
	LEOCantSetValueAsObjectID,
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
	LEOGetReferenceValueAsObjectID,
	LEOGetReferenceValueAsBoolean,
	LEOGetReferenceValueAsRangeOfString,
	
	LEOSetReferenceValueAsNumber,
	LEOSetReferenceValueAsString,
	LEOSetReferenceValueAsObjectID,
	LEOSetReferenceValueAsBoolean,
	LEOSetReferenceValueRangeAsString,
	
	LEOInitReferenceValueCopy,
	
	LEOCleanUpReferenceValue
};



#pragma mark -
#pragma mark Shared


LEOObjectID LEOCantGetValueAsObjectID( LEOValuePtr self, struct LEOContext* inContext )
{
	snprintf( inContext->errMsg, sizeof(inContext->errMsg), "Can't make %s into an object ID", self->isa->displayTypeName );
	inContext->keepRunning = false;
	
	return LEOObjectIDINVALID;
}


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


void	LEOCantSetValueAsObjectID( LEOValuePtr self, LEOObjectID inObjectID, struct LEOContext* inContext )
{
	snprintf( inContext->errMsg, sizeof(inContext->errMsg), "Expected %s, found object ID", self->isa->displayTypeName );
	inContext->keepRunning = false;
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


#pragma mark -
#pragma mark Number


void	LEOInitNumberValue( LEOValuePtr inStorage, double inNumber )
{
	inStorage->isa = &kLeoValueTypeNumber;
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


void	LEOInitStringConstantValue( LEOValuePtr inStorage, const char* inString )
{
	inStorage->isa = &kLeoValueTypeStringConstant;
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


void	LEOInitBooleanValue( LEOValuePtr self, bool inBoolean )
{
	self->isa = &kLeoValueTypeBoolean;
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



#pragma mark -
#pragma mark Reference


void	LEOInitReferenceValue( LEOValuePtr inStorage, LEOValuePtr referencedValue )
{
	inStorage->isa = &kLeoValueTypeReference;
	((struct LEOValueReference*)inStorage)->referencedValue = referencedValue;
}


void	LEOGetReferenceValueAsString( LEOValuePtr self, char* outBuf, long bufSize, struct LEOContext* inContext )
{
	LEOGetValueAsString( ((struct LEOValueReference*)self)->referencedValue, outBuf, bufSize, inContext );
}


double	LEOGetReferenceValueAsNumber( LEOValuePtr self, struct LEOContext* inContext )
{
	return LEOGetValueAsNumber( ((struct LEOValueReference*)self)->referencedValue, inContext );
}


LEOObjectID	LEOGetReferenceValueAsObjectID( LEOValuePtr self, struct LEOContext* inContext )
{
	return LEOGetValueAsObjectID( ((struct LEOValueReference*)self)->referencedValue, inContext );
}


bool	LEOGetReferenceValueAsBoolean( LEOValuePtr self, struct LEOContext* inContext )
{
	return LEOGetValueAsBoolean( ((struct LEOValueReference*)self)->referencedValue, inContext );
}


void	LEOGetReferenceValueAsRangeOfString( LEOValuePtr self, LEOChunkType inType,
									size_t inRangeStart, size_t inRangeEnd,
									char* outBuf, long bufSize, struct LEOContext* inContext )
{
	LEOGetValueAsRangeOfString( ((struct LEOValueReference*)self)->referencedValue, inType, inRangeStart, inRangeEnd, outBuf, bufSize, inContext );
}


void	LEOSetReferenceValueAsString( LEOValuePtr self, const char* inString, struct LEOContext* inContext )
{
	LEOSetValueAsString( ((struct LEOValueReference*)self)->referencedValue, inString, inContext );
}


void	LEOSetReferenceValueAsBoolean( LEOValuePtr self, bool inBoolean, struct LEOContext* inContext )
{
	LEOSetValueAsBoolean( ((struct LEOValueReference*)self)->referencedValue, inBoolean, inContext );
}


void	LEOSetReferenceValueAsNumber( LEOValuePtr self, double inNumber, struct LEOContext* inContext )
{
	LEOSetValueAsNumber( ((struct LEOValueReference*)self)->referencedValue, inNumber, inContext );
}


void	LEOSetReferenceValueAsObjectID( LEOValuePtr self, LEOObjectID inObjectID, struct LEOContext* inContext )
{
	LEOSetValueAsObjectID( ((struct LEOValueReference*)self)->referencedValue, inObjectID, inContext );
}


void	LEOSetReferenceValueRangeAsString( LEOValuePtr self, LEOChunkType inType,
											size_t inRangeStart, size_t inRangeEnd,
											const char* inBuf, struct LEOContext* inContext )
{
	LEOSetValueRangeAsString( ((struct LEOValueReference*)self)->referencedValue, inType, inRangeStart, inRangeEnd, inBuf, inContext );
}


void	LEOInitReferenceValueCopy( LEOValuePtr self, LEOValuePtr dest )
{
	dest->isa = &kLeoValueTypeReference;
	((struct LEOValueReference*)dest)->referencedValue = ((struct LEOValueReference*)self)->referencedValue;
}


void	LEOCleanUpReferenceValue( LEOValuePtr self )
{
	self->isa = NULL;
	((struct LEOValueReference*)self)->referencedValue = NULL;
}


#pragma mark -


struct LEOArrayEntry
{
	struct LEOArrayEntry	*	smallerItem;
	struct LEOArrayEntry	*	largerItem;
	union LEOValue				value;
	char						key[0];	// Must be last, dynamically sized array.
};


struct LEOArrayEntry	*	LEOAllocNewEntry( const char* inKey, LEOValuePtr inValue )
{
	struct LEOArrayEntry	*	newEntry = NULL;
	size_t						inKeyLen = strlen(inKey);
	newEntry = malloc( sizeof(struct LEOArrayEntry) +inKeyLen +1 );
	memmove( newEntry->key, inKey, inKeyLen +1 ); 
	LEOInitCopy( inValue, &newEntry->value );
	newEntry->smallerItem = NULL;
	newEntry->largerItem = NULL;
	
	return newEntry;
}


void	LEOAddArrayEntryToRoot( struct LEOArrayEntry** arrayPtrByReference, const char* inKey, LEOValuePtr inValue )
{
	struct LEOArrayEntry	*	currEntry = NULL;
	
	if( *arrayPtrByReference == NULL )
		*arrayPtrByReference = LEOAllocNewEntry( inKey, inValue );
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
					currEntry->largerItem = LEOAllocNewEntry( inKey, inValue );
					break;
				}
				else
					currEntry = currEntry->largerItem;
			}
			else if( cmpResult < 0 )	// Key is smaller? Go down 'smaller' side one step.
			{
				if( currEntry->smallerItem == NULL )
				{
					currEntry->smallerItem = LEOAllocNewEntry( inKey, inValue );
					break;
				}
				else
					currEntry = currEntry->smallerItem;
			}
			else if( cmpResult == 0 )	// Key already exists? Replace value!
			{
				LEOCleanUpValue( &currEntry->value );
				LEOInitCopy( inValue, &currEntry->value );
			}
		}
	}
}


void	LEODeleteArrayEntryFromRoot( struct LEOArrayEntry** arrayPtrByReference, const char* inKey )
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
			LEOCleanUpValue( &currEntry->value );
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


