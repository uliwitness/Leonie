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
#include <ctype.h>
#include <math.h>
#include "LEOInstructions.h"
#include "AnsiStrings.h"


#define OTHER_VALUE_SHORT_STRING_MAX_LENGTH		256
#define LEO_MAX_ARRAY_KEY_SIZE					1024


// Users shouldn't care if something is a variant, but it helps when debugging the engine:
#if DEBUG
#define VARIANT_NAME(n)		(n " variant")
#else
#define VARIANT_NAME(n)		n
#endif


const char*		gUnitLabels[kLEOUnit_Last +1] =
{
#define X4(constName,stringSuffix,identifierSubtype,unitGroup)	stringSuffix,
	LEO_UNITS
#undef X4
	NULL
};


LEOUnitGroup		gUnitGroupsForLabels[kLEOUnit_Last +1] =
{
#define X4(constName,stringSuffix,identifierSubtype,unitGroup)	unitGroup,
	LEO_UNITS
#undef X4
	kLEOUnitGroup_Last
};


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
	LEOPutNumberValueIntoValue,
	LEOCantFollowReferencesAndReturnValueOfType,
	LEODetermineChunkRangeOfSubstringOfAnyValue,
	
	LEOCleanUpNumberValue,
	
	LEOCanGetValueAsNumber,
	
	LEOCantGetValueForKey,
	LEOCantSetValueForKey,
	LEOCantSetValueAsArray,
	LEOCantGetKeyCount,
	
	LEOCantGetValueForKeyOfRange,
	LEOCantSetValueForKeyOfRange,
	
	LEOCantSetValueAsNativeObject,
	
	LEOCantSetValueAsRect,
	LEOCantGetValueAsRect,
	LEOCantSetValueAsPoint,
	LEOCantGetValueAsPoint,
	
	LEOValueIsNotUnset,
	
	LEOCantSetValueAsRange,
	LEOCantGetValueAsRange,
	
	LEOCanGetNumberValueAsInteger
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
	LEOPutIntegerValueIntoValue,
	LEOCantFollowReferencesAndReturnValueOfType,
	LEODetermineChunkRangeOfSubstringOfAnyValue,
	
	LEOCleanUpIntegerValue,
	
	LEOCanGetValueAsNumber,
	
	LEOCantGetValueForKey,
	LEOCantSetValueForKey,
	LEOCantSetValueAsArray,
	LEOCantGetKeyCount,
	
	LEOCantGetValueForKeyOfRange,
	LEOCantSetValueForKeyOfRange,
	
	LEOCantSetValueAsNativeObject,
	
	LEOCantSetValueAsRect,
	LEOCantGetValueAsRect,
	LEOCantSetValueAsPoint,
	LEOCantGetValueAsPoint,
	
	LEOValueIsNotUnset,
	
	LEOCantSetValueAsRange,
	LEOCantGetValueAsRange,
	
	LEOCanGetValueAsInteger
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
	LEOPutStringValueIntoValue,
	LEOCantFollowReferencesAndReturnValueOfType,
	LEODetermineChunkRangeOfSubstringOfStringValue,
	
	LEOCleanUpStringValue,
	
	LEOCanGetStringValueAsNumber,
	
	LEOGetStringValueForKey,
	LEOSetStringLikeValueForKey,
	LEOSetStringLikeValueAsArray,
	LEOGetStringLikeValueKeyCount,
	
	LEOCantGetValueForKeyOfRange,
	LEOCantSetValueForKeyOfRange,
	
	LEOSetStringValueAsNativeObject,
	
	LEOSetStringValueAsRect,
	LEOGetStringValueAsRect,
	LEOSetStringValueAsPoint,
	LEOGetStringValueAsPoint,
	
	LEOValueIsNotUnset,
	
	LEOSetStringValueAsRange,
	LEOGetStringValueAsRange,
	
	LEOCanGetStringValueAsInteger
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
	LEOPutStringValueIntoValue,
	LEOCantFollowReferencesAndReturnValueOfType,
	LEODetermineChunkRangeOfSubstringOfStringValue,
	
	LEOCleanUpStringConstantValue,
	
	LEOCanGetStringValueAsNumber,
	
	LEOCantGetValueForKey,
	LEOCantSetValueForKey,
	LEOSetStringLikeValueAsArray,
	LEOGetStringLikeValueKeyCount,
	
	LEOCantGetValueForKeyOfRange,
	LEOCantSetValueForKeyOfRange,
	
	LEOCantSetValueAsNativeObject,
	
	LEOSetStringConstantValueAsRect,
	LEOGetStringValueAsRect,
	LEOSetStringConstantValueAsPoint,
	LEOGetStringValueAsPoint,
	
	LEOGetStringConstantValueIsUnset,
	
	LEOSetStringConstantValueAsRange,
	LEOGetStringValueAsRange,
	
	LEOCanGetStringValueAsInteger
};


struct LEOValueType	kLeoValueTypeRect =
{
	"rectangle",
	sizeof(struct LEOValueRect),
	
	LEOCantGetValueAsNumber,
	LEOCantGetValueAsInteger,
	LEOGetRectValueAsString,
	LEOCantGetValueAsBoolean,
	LEOGetAnyValueAsRangeOfString,	// Only works as long as rects can't be longer than OTHER_VALUE_SHORT_STRING_MAX_LENGTH as strings.
	
	LEOCantSetValueAsNumber,
	LEOCantSetValueAsInteger,
	LEOSetRectValueAsString,
	LEOCantSetValueAsBoolean,
	LEOCantSetValueRangeAsString,
	LEOCantSetValuePredeterminedRangeAsString,
	
	LEOInitRectValueCopy,
	LEOInitRectValueCopy,
	LEOPutRectValueIntoValue,
	LEOCantFollowReferencesAndReturnValueOfType,
	LEODetermineChunkRangeOfSubstringOfAnyValue,
	
	LEOCleanUpRectValue,
	
	LEOCantCanGetValueAsNumber,
	
	LEOGetRectValueValueForKey,
	LEOSetRectValueValueForKey,
	LEOSetStringLikeValueAsArray,
	LEOGetRectValueKeyCount,
	
	LEOCantGetValueForKeyOfRange,
	LEOCantSetValueForKeyOfRange,
	
	LEOCantSetValueAsNativeObject,
	
	LEOSetRectValueAsRect,
	LEOGetRectValueAsRect,
	LEOCantSetValueAsPoint,
	LEOCantGetValueAsPoint,
	
	LEOValueIsNotUnset,
	
	LEOCantSetValueAsRange,
	LEOCantGetValueAsRange,
	
	LEOCantCanGetValueAsInteger
};


struct LEOValueType	kLeoValueTypePoint =
{
	"point",
	sizeof(struct LEOValuePoint),
	
	LEOCantGetValueAsNumber,
	LEOCantGetValueAsInteger,
	LEOGetPointValueAsString,
	LEOCantGetValueAsBoolean,
	LEOGetAnyValueAsRangeOfString,	// Only works as long as points can't be longer than OTHER_VALUE_SHORT_STRING_MAX_LENGTH as strings.
	
	LEOCantSetValueAsNumber,
	LEOCantSetValueAsInteger,
	LEOSetPointValueAsString,
	LEOCantSetValueAsBoolean,
	LEOCantSetValueRangeAsString,
	LEOCantSetValuePredeterminedRangeAsString,
	
	LEOInitPointValueCopy,
	LEOInitPointValueCopy,
	LEOPutPointValueIntoValue,
	LEOCantFollowReferencesAndReturnValueOfType,
	LEODetermineChunkRangeOfSubstringOfAnyValue,
	
	LEOCleanUpPointValue,
	
	LEOCantCanGetValueAsNumber,
	
	LEOGetPointValueValueForKey,
	LEOSetPointValueValueForKey,
	LEOSetStringLikeValueAsArray,
	LEOGetPointValueKeyCount,
	
	LEOCantGetValueForKeyOfRange,
	LEOCantSetValueForKeyOfRange,
	
	LEOCantSetValueAsNativeObject,
	
	LEOCantSetValueAsRect,
	LEOCantGetValueAsRect,
	LEOSetPointValueAsPoint,
	LEOGetPointValueAsPoint,
	
	LEOValueIsNotUnset,
	
	LEOCantSetValueAsRange,
	LEOCantGetValueAsRange,
	
	LEOCantCanGetValueAsInteger
};


struct LEOValueType	kLeoValueTypeRange =
{
	"range",
	sizeof(struct LEOValueRange),
	
	LEOCantGetValueAsNumber,
	LEOCantGetValueAsInteger,
	LEOGetRangeValueAsString,
	LEOCantGetValueAsBoolean,
	LEOGetAnyValueAsRangeOfString,	// Only works as long as rects can't be longer than OTHER_VALUE_SHORT_STRING_MAX_LENGTH as strings.
	
	LEOCantSetValueAsNumber,
	LEOCantSetValueAsInteger,
	LEOSetRangeValueAsString,
	LEOCantSetValueAsBoolean,
	LEOCantSetValueRangeAsString,
	LEOCantSetValuePredeterminedRangeAsString,
	
	LEOInitRangeValueCopy,
	LEOInitRangeValueCopy,
	LEOPutRangeValueIntoValue,
	LEOCantFollowReferencesAndReturnValueOfType,
	LEODetermineChunkRangeOfSubstringOfAnyValue,
	
	LEOCleanUpRangeValue,
	
	LEOCantCanGetValueAsNumber,
	
	LEOCantGetValueForKey,
	LEOCantSetValueForKey,
	LEOCantSetValueAsArray,
	LEOCantGetKeyCount,
	
	LEOCantGetValueForKeyOfRange,
	LEOCantSetValueForKeyOfRange,
	
	LEOCantSetValueAsNativeObject,
	
	LEOCantSetValueAsRect,
	LEOCantGetValueAsRect,
	LEOCantSetValueAsPoint,
	LEOCantGetValueAsPoint,
	
	LEOValueIsNotUnset,
	
	LEOSetRangeValueAsRange,
	LEOGetRangeValueAsRange,
	
	LEOCantCanGetValueAsInteger
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
	LEOPutBooleanValueIntoValue,
	LEOCantFollowReferencesAndReturnValueOfType,
	LEODetermineChunkRangeOfSubstringOfAnyValue,
	
	LEOCleanUpBooleanValue,
	
	LEOCantCanGetValueAsNumber,
	
	LEOCantGetValueForKey,
	LEOCantSetValueForKey,
	LEOCantSetValueAsArray,
	LEOCantGetKeyCount,
	
	LEOCantGetValueForKeyOfRange,
	LEOCantSetValueForKeyOfRange,
	
	LEOCantSetValueAsNativeObject,
	
	LEOCantSetValueAsRect,
	LEOCantGetValueAsRect,
	LEOCantSetValueAsPoint,
	LEOCantGetValueAsPoint,
	
	LEOValueIsNotUnset,
	
	LEOCantSetValueAsRange,
	LEOCantGetValueAsRange,
	
	LEOCantCanGetValueAsInteger
};


struct LEOValueType	kLeoValueTypeNativeObject =
{
	"native object",
	sizeof(struct LEOValueObject),
	
	LEOCantGetValueAsNumber,
	LEOCantGetValueAsInteger,
	LEOGetNativeObjectValueAsString,
	LEOCantGetValueAsBoolean,
	LEOGetNativeObjectValueAsRangeOfString,
	
	LEOCantSetValueAsNumber,
	LEOCantSetValueAsInteger,
	LEOCantSetValueAsString,
	LEOCantSetValueAsBoolean,
	LEOCantSetValueRangeAsString,
	LEOCantSetValuePredeterminedRangeAsString,
	
	LEOInitNativeObjectValueCopy,
	LEOInitNativeObjectValueCopy,
	LEOPutNativeObjectValueIntoValue,
	LEOCantFollowReferencesAndReturnValueOfType,
	LEOCantDetermineChunkRangeOfSubstringOfValue,
	
	LEOCleanUpNativeObjectValue,
	
	LEOCantCanGetValueAsNumber,
	
	LEOCantGetValueForKey,
	LEOCantSetValueForKey,
	LEOCantSetValueAsArray,
	LEOCantGetKeyCount,
	
	LEOCantGetValueForKeyOfRange,
	LEOCantSetValueForKeyOfRange,
	
	LEOSetNativeObjectValueAsNativeObject,
	
	LEOCantSetValueAsRect,
	LEOCantGetValueAsRect,
	LEOCantSetValueAsPoint,
	LEOCantGetValueAsPoint,
	
	LEOGetNativeObjectValueIsUnset,
	
	LEOCantSetValueAsRange,
	LEOCantGetValueAsRange,
	
	LEOCantCanGetValueAsInteger
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
	LEOPutReferenceValueIntoValue,
	LEOReferenceValueFollowReferencesAndReturnValueOfType,
	LEODetermineChunkRangeOfSubstringOfReferenceValue,
	
	LEOCleanUpReferenceValue,
	
	LEOCanGetReferenceValueAsNumber,
	
	LEOGetReferenceValueValueForKey,
	LEOSetReferenceValueValueForKey,
	LEOSetReferenceValueAsArray,
	LEOGetReferenceValueKeyCount,
	
	LEOGetReferenceValueForKeyOfRange,
	LEOSetReferenceValueForKeyOfRange,
	
	LEOSetReferenceValueAsNativeObject,
	
	LEOSetReferenceValueAsRect,
	LEOGetReferenceValueAsRect,
	LEOSetReferenceValueAsPoint,
	LEOGetReferenceValueAsPoint,
	
	LEOGetReferenceValueIsUnset,
	
	LEOSetReferenceValueAsRange,
	LEOGetReferenceValueAsRange,
	
	LEOCanGetReferenceValueAsInteger
};


struct LEOValueType	kLeoValueTypeNumberVariant =
{
	VARIANT_NAME("number"),
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
	LEOPutNumberValueIntoValue,
	LEOCantFollowReferencesAndReturnValueOfType,
	LEODetermineChunkRangeOfSubstringOfAnyValue,
	
	LEOCleanUpNumberValue,
	
	LEOCanGetValueAsNumber,
	
	LEOCantGetValueForKey,
	LEOCantSetValueForKey,
	LEOSetVariantValueAsArray,
	LEOCantGetKeyCount,
	
	LEOCantGetValueForKeyOfRange,
	LEOCantSetValueForKeyOfRange,
	
	LEOSetVariantValueAsNativeObject,
	
	LEOSetVariantValueAsRect,
	LEOCantGetValueAsRect,
	LEOSetVariantValueAsPoint,
	LEOCantGetValueAsPoint,
	
	LEOValueIsNotUnset,
	
	LEOSetVariantValueAsRange,
	LEOCantGetValueAsRange,
	
	LEOCanGetNumberValueAsInteger
};


struct LEOValueType	kLeoValueTypeIntegerVariant =
{
	VARIANT_NAME("integer"),
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
	LEOPutIntegerValueIntoValue,
	LEOCantFollowReferencesAndReturnValueOfType,
	LEODetermineChunkRangeOfSubstringOfAnyValue,
	
	LEOCleanUpIntegerValue,
	
	LEOCanGetValueAsNumber,
	
	LEOCantGetValueForKey,
	LEOCantSetValueForKey,
	LEOSetVariantValueAsArray,
	LEOCantGetKeyCount,
	
	LEOCantGetValueForKeyOfRange,
	LEOCantSetValueForKeyOfRange,
	
	LEOSetVariantValueAsNativeObject,
	
	LEOSetVariantValueAsRect,
	LEOCantGetValueAsRect,
	LEOSetVariantValueAsPoint,
	LEOCantGetValueAsPoint,
	
	LEOValueIsNotUnset,
	
	LEOSetVariantValueAsRange,
	LEOCantGetValueAsRange,
	
	LEOCanGetValueAsInteger
};


struct LEOValueType	kLeoValueTypeStringVariant =
{
	VARIANT_NAME("string"),
	sizeof(union LEOValue),
	
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
	LEOPutStringValueIntoValue,
	LEOCantFollowReferencesAndReturnValueOfType,
	LEODetermineChunkRangeOfSubstringOfStringValue,
	
	LEOCleanUpStringValue,
	
	LEOCanGetStringValueAsNumber,
	
	LEOGetStringVariantValueForKey,
	LEOSetStringVariantValueValueForKey,
	LEOSetVariantValueAsArray,
	LEOGetStringLikeValueKeyCount,
	
	LEOCantGetValueForKeyOfRange,
	LEOCantSetValueForKeyOfRange,
	
	LEOSetVariantValueAsNativeObject,
	
	LEOSetVariantValueAsRect,
	LEOGetStringValueAsRect,
	LEOSetVariantValueAsPoint,
	LEOCantGetValueAsPoint,
	
	LEOValueIsNotUnset,
	
	LEOSetVariantValueAsRange,
	LEOGetStringValueAsRange,
	
	LEOCanGetStringValueAsInteger
};


struct LEOValueType	kLeoValueTypeBooleanVariant =
{
	VARIANT_NAME("boolean"),
	sizeof(union LEOValue),
	
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
	LEOPutBooleanValueIntoValue,
	LEOCantFollowReferencesAndReturnValueOfType,
	LEODetermineChunkRangeOfSubstringOfAnyValue,
	
	LEOCleanUpBooleanValue,
	
	LEOCantCanGetValueAsNumber,
	
	LEOCantGetValueForKey,
	LEOCantSetValueForKey,
	LEOSetVariantValueAsArray,
	LEOCantGetKeyCount,
	
	LEOCantGetValueForKeyOfRange,
	LEOCantSetValueForKeyOfRange,
	
	LEOSetVariantValueAsNativeObject,
	
	LEOSetVariantValueAsRect,
	LEOCantGetValueAsRect,
	LEOSetVariantValueAsPoint,
	LEOCantGetValueAsPoint,
	
	LEOValueIsNotUnset,
	
	LEOSetVariantValueAsRange,
	LEOCantGetValueAsRange,
	
	LEOCantCanGetValueAsInteger
};


struct LEOValueType	kLeoValueTypeRectVariant =
{
	VARIANT_NAME("rectangle"),
	sizeof(union LEOValue),
	
	LEOCantGetValueAsNumber,
	LEOCantGetValueAsInteger,
	LEOGetRectValueAsString,
	LEOCantGetValueAsBoolean,
	LEOGetAnyValueAsRangeOfString,	// Only works as long as rects can't be longer than OTHER_VALUE_SHORT_STRING_MAX_LENGTH as strings.
	
	LEOSetVariantValueAsNumber,
	LEOSetVariantValueAsInteger,
	LEOSetVariantValueAsString,
	LEOSetVariantValueAsBoolean,
	LEOSetVariantValueRangeAsString,
	LEOSetVariantValuePredeterminedRangeAsString,
	
	LEOInitRectVariantValueCopy,
	LEOInitRectValueCopy,
	LEOPutRectValueIntoValue,
	LEOCantFollowReferencesAndReturnValueOfType,
	LEODetermineChunkRangeOfSubstringOfAnyValue,
	
	LEOCleanUpRectValue,
	
	LEOCantCanGetValueAsNumber,
	
	LEOGetRectValueValueForKey,
	LEOSetRectValueValueForKey,
	LEOSetVariantValueAsArray,
	LEOGetRectValueKeyCount,
	
	LEOCantGetValueForKeyOfRange,
	LEOCantSetValueForKeyOfRange,
	
	LEOSetVariantValueAsNativeObject,
	
	LEOSetRectValueAsRect,
	LEOGetRectValueAsRect,
	LEOSetVariantValueAsPoint,
	LEOCantGetValueAsPoint,
	
	LEOValueIsNotUnset,
	
	LEOSetVariantValueAsRange,
	LEOCantGetValueAsRange,
	
	LEOCantCanGetValueAsInteger
};


struct LEOValueType	kLeoValueTypePointVariant =
{
	VARIANT_NAME("point"),
	sizeof(union LEOValue),
	
	LEOCantGetValueAsNumber,
	LEOCantGetValueAsInteger,
	LEOGetPointValueAsString,
	LEOCantGetValueAsBoolean,
	LEOGetAnyValueAsRangeOfString,	// Only works as long as rects can't be longer than OTHER_VALUE_SHORT_STRING_MAX_LENGTH as strings.
	
	LEOSetVariantValueAsNumber,
	LEOSetVariantValueAsInteger,
	LEOSetVariantValueAsString,
	LEOSetVariantValueAsBoolean,
	LEOSetVariantValueRangeAsString,
	LEOSetVariantValuePredeterminedRangeAsString,
	
	LEOInitPointVariantValueCopy,
	LEOInitPointValueCopy,
	LEOPutPointValueIntoValue,
	LEOCantFollowReferencesAndReturnValueOfType,
	LEODetermineChunkRangeOfSubstringOfAnyValue,
	
	LEOCleanUpPointValue,
	
	LEOCantCanGetValueAsNumber,
	
	LEOGetPointValueValueForKey,
	LEOSetPointValueValueForKey,
	LEOSetVariantValueAsArray,
	LEOGetPointValueKeyCount,
	
	LEOCantGetValueForKeyOfRange,
	LEOCantSetValueForKeyOfRange,
	
	LEOSetVariantValueAsNativeObject,
	
	LEOCantSetValueAsRect,
	LEOCantGetValueAsRect,
	LEOSetVariantValueAsPoint,
	LEOGetPointValueAsPoint,
	
	LEOValueIsNotUnset,
	
	LEOSetVariantValueAsRange,
	LEOCantGetValueAsRange,
	
	LEOCantCanGetValueAsInteger
};


struct LEOValueType	kLeoValueTypeRangeVariant =
{
	VARIANT_NAME("range"),
	sizeof(union LEOValue),
	
	LEOCantGetValueAsNumber,
	LEOCantGetValueAsInteger,
	LEOGetRangeValueAsString,
	LEOCantGetValueAsBoolean,
	LEOGetAnyValueAsRangeOfString,	// Only works as long as ranges can't be longer than OTHER_VALUE_SHORT_STRING_MAX_LENGTH as strings.
	
	LEOSetVariantValueAsNumber,
	LEOSetVariantValueAsInteger,
	LEOSetVariantValueAsString,
	LEOSetVariantValueAsBoolean,
	LEOSetVariantValueRangeAsString,
	LEOSetVariantValuePredeterminedRangeAsString,
	
	LEOInitRangeVariantValueCopy,
	LEOInitRangeValueCopy,
	LEOPutRangeValueIntoValue,
	LEOCantFollowReferencesAndReturnValueOfType,
	LEODetermineChunkRangeOfSubstringOfAnyValue,
	
	LEOCleanUpRangeValue,
	
	LEOCantCanGetValueAsNumber,
	
	LEOCantGetValueForKey,
	LEOCantSetValueForKey,
	LEOSetVariantValueAsArray,
	LEOCantGetKeyCount,
	
	LEOCantGetValueForKeyOfRange,
	LEOCantSetValueForKeyOfRange,
	
	LEOSetVariantValueAsNativeObject,
	
	LEOCantSetValueAsRect,
	LEOCantGetValueAsRect,
	LEOSetVariantValueAsPoint,
	LEOCantGetValueAsPoint,
	
	LEOValueIsNotUnset,
	
	LEOSetVariantValueAsRange,
	LEOGetRangeValueAsRange,
	
	LEOCantCanGetValueAsInteger
};


struct LEOValueType	kLeoValueTypeNativeObjectVariant =
{
	VARIANT_NAME("native object"),
	sizeof(union LEOValue),
	
	LEOCantGetValueAsNumber,
	LEOCantGetValueAsInteger,
	LEOCantGetValueAsString,
	LEOCantGetValueAsBoolean,
	LEOCantGetValueAsRangeOfString,	// Only works as long as booleans can't be longer than OTHER_VALUE_SHORT_STRING_MAX_LENGTH as strings.
	
	LEOSetVariantValueAsNumber,
	LEOSetVariantValueAsInteger,
	LEOSetVariantValueAsString,
	LEOSetVariantValueAsBoolean,
	LEOSetVariantValueRangeAsString,
	LEOSetVariantValuePredeterminedRangeAsString,
	
	LEOInitNativeObjectVariantValueCopy,
	LEOInitNativeObjectValueCopy,
	LEOPutNativeObjectValueIntoValue,
	LEOCantFollowReferencesAndReturnValueOfType,
	LEODetermineChunkRangeOfSubstringOfAnyValue,
	
	LEOCleanUpNativeObjectValue,
	
	LEOCantCanGetValueAsNumber,
	
	LEOCantGetValueForKey,
	LEOCantSetValueForKey,
	LEOSetVariantValueAsArray,
	LEOCantGetKeyCount,
	
	LEOCantGetValueForKeyOfRange,
	LEOCantSetValueForKeyOfRange,
	
	LEOSetVariantValueAsNativeObject,
	
	LEOSetVariantValueAsRect,
	LEOCantGetValueAsRect,
	LEOCantSetValueAsPoint,
	LEOCantGetValueAsPoint,
	
	LEOValueIsNotUnset,
	
	LEOSetVariantValueAsRange,
	LEOCantGetValueAsRange,
	
	LEOCantCanGetValueAsInteger
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
	LEOPutArrayValueIntoValue,
	LEOCantFollowReferencesAndReturnValueOfType,
	LEODetermineChunkRangeOfSubstringOfArrayValue,
	
	LEOCleanUpArrayValue,
	
	LEOCantCanGetValueAsNumber,
	
	LEOGetArrayValueValueForKey,
	LEOSetArrayValueValueForKey,
	LEOSetArrayValueAsArray,
	LEOGetArrayValueKeyCount,
	
	LEOCantGetValueForKeyOfRange,
	LEOCantSetValueForKeyOfRange,
	
	LEOCantSetValueAsNativeObject,
	
	LEOSetVariantValueAsRect,
	LEOGetArrayValueAsRect,
	LEOSetVariantValueAsPoint,
	LEOGetArrayValueAsPoint,
	
	LEOValueIsNotUnset,
	
	LEOCantSetValueAsRange,
	LEOCantGetValueAsRange,
	
	LEOCantCanGetValueAsInteger
};


struct LEOValueType	kLeoValueTypeArrayVariant =
{
	VARIANT_NAME("array"),
	sizeof(union LEOValue),
	
	LEOCantGetValueAsNumber,
	LEOCantGetValueAsInteger,
	LEOGetArrayValueAsString,
	LEOCantGetValueAsBoolean,
	LEOGetArrayValueAsRangeOfString,
	
	LEOSetVariantValueAsNumber,
	LEOSetVariantValueAsInteger,
	LEOSetVariantValueAsString,
	LEOSetVariantValueAsBoolean,
	LEOSetVariantValueRangeAsString,
	LEOSetVariantValuePredeterminedRangeAsString,
	
	LEOInitArrayVariantValueCopy,
	LEOInitArrayValueCopy,
	LEOPutArrayValueIntoValue,
	LEOCantFollowReferencesAndReturnValueOfType,
	LEODetermineChunkRangeOfSubstringOfArrayValue,
	
	LEOCleanUpArrayValue,
	
	LEOCantCanGetValueAsNumber,
	
	LEOGetArrayValueValueForKey,
	LEOSetArrayValueValueForKey,
	LEOSetArrayValueAsArray,
	LEOGetArrayValueKeyCount,
	
	LEOCantGetValueForKeyOfRange,
	LEOCantSetValueForKeyOfRange,
	
	LEOSetVariantValueAsNativeObject,
	
	LEOSetArrayValueAsRect,
	LEOGetArrayValueAsRect,
	LEOSetArrayValueAsPoint,
	LEOGetArrayValueAsPoint,
	
	LEOValueIsNotUnset,
	
	LEOSetVariantValueAsRange,
	LEOCantGetValueAsRange,
	
	LEOCantCanGetValueAsInteger
};



static char		sUnsetConstantString[1] = {0};


#pragma mark -


LEONumber	LEONumberWithUnitAsUnit( LEONumber inNumber, LEOUnit fromUnit, LEOUnit toUnit )
{
	assert( gUnitGroupsForLabels[fromUnit] == gUnitGroupsForLabels[toUnit] );
	assert( fromUnit >= toUnit );
	
	if( gUnitGroupsForLabels[fromUnit] == kLEOUnitGroupTime )
	{
		while( fromUnit > toUnit )
		{
			inNumber *= 60;
			
			fromUnit--;
		}
	}
	else if( gUnitGroupsForLabels[fromUnit] == kLEOUnitGroupBytes )
	{
		while( fromUnit > toUnit )
		{
			inNumber *= 1024;
			
			fromUnit--;
		}
	}
	
	return inNumber;
}


LEOUnit	LEOConvertNumbersToCommonUnit( LEONumber* firstArgument, LEOUnit firstUnit, LEONumber* secondArgument, LEOUnit secondUnit )
{
	if( firstUnit == secondUnit )	// Already the same! Nothing to do! Math away!
		return firstUnit;
	
	if( gUnitGroupsForLabels[firstUnit] != gUnitGroupsForLabels[secondUnit] )	// Comparing apples and oranges, fail!
		return kLEOUnit_Last;
	
	if( firstUnit > secondUnit )
	{
		*firstArgument = LEONumberWithUnitAsUnit(*firstArgument,firstUnit,secondUnit);
		return secondUnit;
	}
	else
	{
		*secondArgument = LEONumberWithUnitAsUnit(*secondArgument,secondUnit,firstUnit);
		return firstUnit;
	}
}


const char* LEOUnitSuffixForUnit( LEOUnit inUnit )
{
	if( inUnit >= kLEOUnit_Last )
		return "";
	return gUnitLabels[inUnit];
}


#pragma mark -
#pragma mark Shared

/*!
	@functiongroup Shared LEOValue Method Implementations
*/


/*!
	Generic method implementation used for values to return a "can't get as string"
	error message and abort execution of the current LEOContext.
 
	Note that you should not need this as *all* types should convert to string.
	The one exception are native pointers when we wrap them in a NativeObject value.
*/

const char*	LEOCantGetValueAsString( LEOValuePtr self, char* outBuf, size_t bufSize, struct LEOContext* inContext )
{
	size_t		lineNo = SIZE_MAX;
	uint16_t	fileID = 0;
	LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
	LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Can't make %s into a string", self->base.isa->displayTypeName );
	return "";
}


void	LEOCantDetermineChunkRangeOfSubstringOfValue( LEOValuePtr self, size_t *ioBytesStart, size_t *ioBytesEnd,
													 size_t *ioBytesDelStart, size_t *ioBytesDelEnd,
													 LEOChunkType inType, size_t inRangeStart, size_t inRangeEnd,
													 struct LEOContext* inContext )
{
	size_t		lineNo = SIZE_MAX;
	uint16_t	fileID = 0;
	LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
	LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Can't make %s into a string.", self->base.isa->displayTypeName );
}


/*!
	Generic method implementation used for values to return a "can't get as number"
	error message and abort execution of the current LEOContext.
*/

LEONumber	LEOCantGetValueAsNumber( LEOValuePtr self, LEOUnit *outUnit, struct LEOContext* inContext )
{
	size_t		lineNo = SIZE_MAX;
	uint16_t	fileID = 0;
	LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
	LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Can't make %s into a number", self->base.isa->displayTypeName );
	
	return 0.0;
}


/*!
	Generic method implementation used for values to return a "can't get as rect"
	error message and abort execution of the current LEOContext.
*/

void	LEOCantGetValueAsRect( LEOValuePtr self, LEOInteger *l, LEOInteger *t, LEOInteger *r, LEOInteger *b, struct LEOContext* inContext )
{
	size_t		lineNo = SIZE_MAX;
	uint16_t	fileID = 0;
	LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
	LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Can't make %s into a rect", self->base.isa->displayTypeName );
}


/*!
	Generic method implementation used for values to return a "can't get as point"
	error message and abort execution of the current LEOContext.
*/

void	LEOCantGetValueAsPoint( LEOValuePtr self, LEOInteger *l, LEOInteger *t, struct LEOContext* inContext )
{
	size_t		lineNo = SIZE_MAX;
	uint16_t	fileID = 0;
	LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
	LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Can't make %s into a point", self->base.isa->displayTypeName );
}


/*!
	Generic method implementation used for values to return a "can't get as range"
	error message and abort execution of the current LEOContext.
*/

void	LEOCantGetValueAsRange( LEOValuePtr self, LEOInteger *s, LEOInteger *e, LEOChunkType *t, struct LEOContext* inContext )
{
	size_t		lineNo = SIZE_MAX;
	uint16_t	fileID = 0;
	LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
	LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Can't make %s into a range", self->base.isa->displayTypeName );
}


/*!
	Generic method implementation used for values to return a "can't get as integer"
	error message and abort execution of the current LEOContext.
*/

LEOInteger	LEOCantGetValueAsInteger( LEOValuePtr self, LEOUnit *outUnit, struct LEOContext* inContext )
{
	size_t		lineNo = SIZE_MAX;
	uint16_t	fileID = 0;
	LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
	LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Can't make %s into an integer", self->base.isa->displayTypeName );
	
	return 0LL;
}


/*!
	Generic method implementation used for values to return a "can't get as boolean"
	error message and abort execution of the current LEOContext.
*/

bool	LEOCantGetValueAsBoolean( LEOValuePtr self, struct LEOContext* inContext )
{
	size_t		lineNo = SIZE_MAX;
	uint16_t	fileID = 0;
	LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
	LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Can't make %s into a boolean", self->base.isa->displayTypeName );
	
	return false;
}


/*!
 Generic method implementation used for values to return a "can't get as string"
 error message and abort execution of the current LEOContext.
 
 Note that you should not need this as *all* types should convert to string.
 The one exception are native pointers when we wrap them in a NativeObject value.
*/

void	LEOCantGetValueAsRangeOfString( LEOValuePtr self, LEOChunkType inType,
										size_t inRangeStart, size_t inRangeEnd,
										char* outBuf, size_t bufSize, struct LEOContext* inContext )
{
	size_t		lineNo = SIZE_MAX;
	uint16_t	fileID = 0;
	LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
	LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Can't make %s into a string", self->base.isa->displayTypeName );
}


/*!
	Generic method implementation used for values to return a "can't get as array"
	error message and abort execution of the current LEOContext.
*/

LEOValuePtr	LEOCantGetValueForKey( LEOValuePtr self, const char* keyName, union LEOValue *tempStorage, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	size_t		lineNo = SIZE_MAX;
	uint16_t	fileID = 0;
	LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
	LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Can't make %s into an array", self->base.isa->displayTypeName );
	
	return NULL;
}


/*!
	Generic method implementation used for values to return a "can't get as array"
	error message and abort execution of the current LEOContext.
*/

void	LEOCantSetValueForKey( LEOValuePtr self, const char* keyName, LEOValuePtr inValue, struct LEOContext* inContext )
{
	size_t		lineNo = SIZE_MAX;
	uint16_t	fileID = 0;
	LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
	LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected array, found %s", self->base.isa->displayTypeName );
}


LEOValuePtr	LEOGetStringValueForKey( LEOValuePtr self, const char* keyName, union LEOValue *tempStorage, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	if( !tempStorage )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Internal error converting %s to array.", self->base.isa->displayTypeName );
		return NULL;
	}
	
	struct LEOArrayEntry	*	convertedArray = NULL;
	if( self->string.string != NULL && self->string.stringLen != 0 )
	{
		convertedArray = LEOCreateArrayFromString( self->string.string, self->string.stringLen, inContext );
		if( !convertedArray )
		{
			size_t		lineNo = SIZE_MAX;
			uint16_t	fileID = 0;
			LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
			LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected array, found %s", self->base.isa->displayTypeName );
			return NULL;
		}
	}
	else
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected array, found %s", self->base.isa->displayTypeName );
		return NULL;
	}
	
	LEOValuePtr	theValue = LEOGetArrayValueForKey( convertedArray, keyName );
	if( theValue == NULL )
		return NULL;
	LEOInitCopy( theValue, tempStorage, keepReferences, inContext );
	LEOCleanUpArray( convertedArray, inContext );
	
	return tempStorage;
}


/*!
	Generic method implementation used for values that can hold an empty
	string. A NULL native object is considered the same as the "unset" value
	by us, which is an empty string.
*/

void	LEOSetStringLikeValueAsNativeObject( LEOValuePtr self, void* inNativeObject, struct LEOContext* inContext )
{
	if( inNativeObject != NULL )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected %s, found native object", self->base.isa->displayTypeName );
	}
	else
	{
		LEOSetValueAsString( self, "", 0, inContext );
	}
}


void	LEOSetStringLikeValueForKey( LEOValuePtr self, const char* keyName, LEOValuePtr inValue, struct LEOContext* inContext )
{
	struct LEOArrayEntry	*	convertedArray = NULL;
	if( self->string.string != NULL && self->string.stringLen != 0 )
	{
		convertedArray = LEOCreateArrayFromString( self->string.string, self->string.stringLen, inContext );
		if( !convertedArray )
		{
			size_t		lineNo = SIZE_MAX;
			uint16_t	fileID = 0;
			LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
			LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected array, found %s", self->base.isa->displayTypeName );
			return;
		}
	}
	LEOAddArrayEntryToRoot( &convertedArray, keyName, inValue, inContext );
	
	char	str[1024] = { 0 };	// TODO: Make work with arbitrary string sizes.
	LEOPrintArray( convertedArray, str, sizeof(str), inContext );
	LEOSetValueAsString( self, str, strlen(str), inContext );	// TODO: Make this binary data safe.
	
	LEOCleanUpArray( convertedArray, inContext );
}


/*!
	Generic method implementation used for values to return a "can't set as number"
	error message and abort execution of the current LEOContext.
*/

void	LEOCantSetValueAsNumber( LEOValuePtr self, LEONumber inNumber, LEOUnit inUnit, struct LEOContext* inContext )
{
	size_t		lineNo = SIZE_MAX;
	uint16_t	fileID = 0;
	LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
	LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected %s, found number", self->base.isa->displayTypeName );
}


/*!
	Generic method implementation used for values to return a "can't set as integer"
	error message and abort execution of the current LEOContext.
*/

void	LEOCantSetValueAsInteger( LEOValuePtr self, LEOInteger inInteger, LEOUnit inUnit, struct LEOContext* inContext )
{
	size_t		lineNo = SIZE_MAX;
	uint16_t	fileID = 0;
	LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
	LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected %s, found integer", self->base.isa->displayTypeName );
}


/*!
	Generic method implementation used for values to return a "can't set as string"
	error message and abort execution of the current LEOContext.
*/

void	LEOCantSetValueAsString( LEOValuePtr self, const char* inString, size_t inStringLen, struct LEOContext* inContext )
{
	size_t		lineNo = SIZE_MAX;
	uint16_t	fileID = 0;
	LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
	LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected %s, found string", self->base.isa->displayTypeName );
}


/*!
	Generic method implementation used for values to return a "can't set as boolean"
	error message and abort execution of the current LEOContext.
*/

void	LEOCantSetValueAsBoolean( LEOValuePtr self, bool inState, struct LEOContext* inContext )
{
	size_t		lineNo = SIZE_MAX;
	uint16_t	fileID = 0;
	LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
	LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected %s, found boolean", self->base.isa->displayTypeName );
}


/*!
	Generic method implementation used for values to return a "can't set as rect"
	error message and abort execution of the current LEOContext.
*/

void	LEOCantSetValueAsRect( LEOValuePtr self, LEOInteger l, LEOInteger t, LEOInteger r, LEOInteger b, struct LEOContext* inContext )
{
	size_t		lineNo = SIZE_MAX;
	uint16_t	fileID = 0;
	LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
	LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected %s, found rect", self->base.isa->displayTypeName );
}


/*!
	Generic method implementation used for values to return a "can't set as point"
	error message and abort execution of the current LEOContext.
*/

void	LEOCantSetValueAsPoint( LEOValuePtr self, LEOInteger l, LEOInteger t, struct LEOContext* inContext )
{
	size_t		lineNo = SIZE_MAX;
	uint16_t	fileID = 0;
	LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
	LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected %s, found point", self->base.isa->displayTypeName );
}


/*!
	Generic method implementation used for values to return a "can't set as range"
	error message and abort execution of the current LEOContext.
*/

void	LEOCantSetValueAsRange( LEOValuePtr self, LEOInteger s, LEOInteger e, LEOChunkType t, struct LEOContext* inContext )
{
	size_t		lineNo = SIZE_MAX;
	uint16_t	fileID = 0;
	LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
	LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected %s, found range", self->base.isa->displayTypeName );
}


/*!
	Generic method implementation used for values to return a "can't set as native object"
	error message and abort execution of the current LEOContext.
*/

void	LEOCantSetValueAsNativeObject( LEOValuePtr self, void* inNativeObject, struct LEOContext* inContext )
{
	size_t		lineNo = SIZE_MAX;
	uint16_t	fileID = 0;
	LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
	LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected %s, found native object", self->base.isa->displayTypeName );
}


/*!
	Generic method implementation used for values to return a "can't set range as string"
	error message and abort execution of the current LEOContext.
*/

void	LEOCantSetValueRangeAsString( LEOValuePtr self, LEOChunkType inType,
									size_t inRangeStart, size_t inRangeEnd,
									const char* inBuf, struct LEOContext* inContext )
{
	size_t		lineNo = SIZE_MAX;
	uint16_t	fileID = 0;
	LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
	LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected %s, found string", self->base.isa->displayTypeName );
}


void	LEOCantSetValuePredeterminedRangeAsString( LEOValuePtr self, size_t inRangeStart, size_t inRangeEnd,
													const char* inBuf, struct LEOContext* inContext )
{
	size_t		lineNo = SIZE_MAX;
	uint16_t	fileID = 0;
	LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
	LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected %s, found string", self->base.isa->displayTypeName );
}


/*!
	Generic method implementation used for values to generate a string from
	them and then return a range of that.
*/

void	LEOGetAnyValueAsRangeOfString( LEOValuePtr self, LEOChunkType inType,
									size_t inRangeStart, size_t inRangeEnd,
									char* outBuf, size_t bufSize, struct LEOContext* inContext )
{
	char		strBuf[OTHER_VALUE_SHORT_STRING_MAX_LENGTH] = {0};	// Can get away with this as long as they're only numbers, booleans etc.
	size_t		outChunkStart = 0,
				outChunkEnd = 0,
				outDelChunkStart = 0,
				outDelChunkEnd = 0;
	const char*	str = LEOGetValueAsString( self, strBuf, sizeof(strBuf), inContext );
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
	char			strBuf[OTHER_VALUE_SHORT_STRING_MAX_LENGTH] = {0};	// Can get away with this as long as they're only numbers, booleans etc.
	const char*		str = LEOGetValueAsString( self, strBuf, sizeof(strBuf), inContext );
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


bool	LEOCanGetValueAsInteger( LEOValuePtr self, struct LEOContext* inContext )
{
	return true;
}


bool	LEOCantCanGetValueAsInteger( LEOValuePtr self, struct LEOContext* inContext )
{
	return false;
}


size_t	LEOCantGetKeyCount( LEOValuePtr self, struct LEOContext* inContext )
{
	return 0;
}


size_t	LEOGetStringLikeValueKeyCount( LEOValuePtr self, struct LEOContext* inContext )
{
	size_t		numKeys = 0;
	char		strBuf[1024] = { 0 };	// TODO: Make work with arbitrary string sizes.
	const char* str = LEOGetValueAsString( self, strBuf, sizeof(strBuf), inContext );

	struct LEOArrayEntry*	theArray = LEOCreateArrayFromString( str, strlen(str), inContext );
	if( theArray )
	{
		numKeys = LEOGetArrayKeyCount( theArray );
		LEOCleanUpArray( theArray, inContext );
	}
	else
	{
		size_t		lineNo = 0;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected an array, found %s.", self->base.isa->displayTypeName );
	}
	return numKeys;
}


LEOValuePtr	LEOCantFollowReferencesAndReturnValueOfType( LEOValuePtr self, LEOValueTypePtr inType, struct LEOContext* inContext )
{
	if( inType == self->base.isa )
		return self;
	else
		return NULL;
}


void	LEOCantSetValueAsArray( LEOValuePtr self, struct LEOArrayEntry *inArray, struct LEOContext* inContext )
{
	size_t		lineNo = SIZE_MAX;
	uint16_t	fileID = 0;
	LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
	LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected %s, found array", self->base.isa->displayTypeName );
}


void	LEOSetStringLikeValueAsArray( LEOValuePtr self, struct LEOArrayEntry *inArray, struct LEOContext* inContext )
{
	char	str[1024] = { 0 };	// TODO: Make work with arbitrary string sizes.
	LEOPrintArray( inArray, str, sizeof(str), inContext );
	LEOSetValueAsString( self, str, strlen(str), inContext );	// TODO: Make this binary data safe.
}


void	LEOCantGetValueForKeyOfRange( LEOValuePtr self, const char* keyName, size_t startOffset, size_t endOffset, LEOValuePtr outValue, struct LEOContext* inContext )
{
	size_t		lineNo = SIZE_MAX;
	uint16_t	fileID = 0;
	LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
	LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Can't get properties of ranges of a %s", self->base.isa->displayTypeName );
}


void	LEOCantSetValueForKeyOfRange( LEOValuePtr self, const char* keyName, LEOValuePtr inValue, size_t startOffset, size_t endOffset, struct LEOContext* inContext )
{
	size_t		lineNo = SIZE_MAX;
	uint16_t	fileID = 0;
	LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
	LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Ranges of a %s can't have properties", self->base.isa->displayTypeName );
}


bool	LEOValueIsNotUnset( LEOValuePtr self, struct LEOContext* inContext )
{
	return false;
}


#pragma mark -
#pragma mark Number

/*!
	@functiongroup LEOValueNumber
*/

void	LEOInitNumberValue( LEOValuePtr inStorage, LEONumber inNumber, LEOUnit inUnit, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	assert( inStorage != NULL );
	inStorage->base.isa = &kLeoValueTypeNumber;
	if( keepReferences == kLEOInvalidateReferences )
		inStorage->base.refObjectID = kLEOObjectIDINVALID;
	inStorage->number.number = inNumber;
	inStorage->number.unit = inUnit;
}


/*!
	Implementation of GetValueAsNumber for number values.
*/

LEONumber LEOGetNumberValueAsNumber( LEOValuePtr self, LEOUnit *outUnit, struct LEOContext* inContext )
{
	if( outUnit )
		*outUnit = self->number.unit;
	return self->number.number;
}


/*!
	Implementation of GetValueAsInteger for number values.
*/

LEOInteger LEOGetNumberValueAsInteger( LEOValuePtr self, LEOUnit *outUnit, struct LEOContext* inContext )
{
	if( trunc(self->number.number) != self->number.number )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected integer, not fractional number." );
	}
	
	if( outUnit )
		*outUnit = self->number.unit;
	
	return (LEOInteger)self->number.number;
}


/*!
	Implementation of GetValueAsString for number values.
*/

const char* LEOGetNumberValueAsString( LEOValuePtr self, char* outBuf, size_t bufSize, struct LEOContext* inContext )
{
	if( outBuf )	// Can never return as a string if we're not given a buffer.
		snprintf( outBuf, bufSize -1, "%g%s", self->number.number, gUnitLabels[self->number.unit] );
	return outBuf;
}


/*!
	Implementation of SetValueAsNumber for number values.
*/

void LEOSetNumberValueAsNumber( LEOValuePtr self, LEONumber inNumber, LEOUnit inUnit, struct LEOContext* inContext )
{
	self->number.number = inNumber; self->number.unit = inUnit;
}


/*!
	Implementation of SetValueAsInteger for number values.
*/

void LEOSetNumberValueAsInteger( LEOValuePtr self, LEOInteger inInteger, LEOUnit inUnit, struct LEOContext* inContext )
{
	self->number.number = (LEONumber)inInteger; self->number.unit = inUnit;
}


/*!
	Implementation of SetValueAsString for number values. If the given string
	can't be fully converted to a number, this will fail with an error message
	and abort execution of the current LEOContext.
*/

void LEOSetNumberValueAsString( LEOValuePtr self, const char* inNumber, size_t inNumberLen, struct LEOContext* inContext )
{
	if( inNumber == NULL || inNumberLen == 0 )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected %s, found empty.", self->base.isa->displayTypeName );
		return;
	}
	char		buf[OTHER_VALUE_SHORT_STRING_MAX_LENGTH +1] = {0};
	if( inNumberLen > OTHER_VALUE_SHORT_STRING_MAX_LENGTH )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected %s, found string or number that is too large.", self->base.isa->displayTypeName );
		return;
	}
	strlcpy( buf, inNumber, inNumberLen );
	
	// Determine if there's a unit on this number, remove it but remember it:
	self->number.unit = kLEOUnitNone;
	
	for( int x = 1; x < kLEOUnit_Last; x++ )	// Skip first one, which is empty string for 'no unit' and would match anything.
	{
		size_t	unitLen = strlen(gUnitLabels[x]);
		if( unitLen < inNumberLen )
		{
			if( strcasecmp( buf +(inNumberLen -unitLen), gUnitLabels[x] ) == 0 )
			{
				inNumberLen -= unitLen;
				buf[inNumberLen] = 0;
				self->number.unit = x;
				break;
			}
		}
	}
	
	// Actually convert the string into a number:
	char*		endPtr = NULL;
	LEONumber	theNum = strtof( buf, &endPtr );
	if( endPtr != (buf +inNumberLen) )
		LEOCantSetValueAsString( self, inNumber, inNumberLen, inContext );
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
	dest->number.unit = self->number.unit;
}


void	LEOPutNumberValueIntoValue( LEOValuePtr self, LEOValuePtr dest, struct LEOContext* inContext )
{
	LEOSetValueAsNumber( dest, self->number.number, self->number.unit, inContext );
}


bool	LEOCanGetNumberValueAsInteger( LEOValuePtr self, struct LEOContext* inContext )
{
	return trunc(self->number.number) == self->number.number;
}


/*!
	Destructor for number values. If this value has references, this makes sure
	that they will produce an error message if they ever try to access it again.
*/

void	LEOCleanUpNumberValue( LEOValuePtr self, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	self->base.isa = NULL;
	self->number.number = 0.0;
	self->number.unit = kLEOUnitNone;
	if( keepReferences == kLEOInvalidateReferences && self->base.refObjectID != kLEOObjectIDINVALID )	// We have references? Make sure they all notice we've gone if they try to access us from now on.
	{
		LEOContextGroupRecycleObjectID( inContext->group, self->base.refObjectID );
		self->base.refObjectID = 0;
	}
}


#pragma mark -
#pragma mark Integer

/*!
	@functiongroup LEOValueInteger
*/

void	LEOInitIntegerValue( LEOValuePtr inStorage, LEOInteger inInteger, LEOUnit inUnit, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	inStorage->base.isa = &kLeoValueTypeInteger;
	if( keepReferences == kLEOInvalidateReferences )
		inStorage->base.refObjectID = kLEOObjectIDINVALID;
	inStorage->integer.integer = inInteger;
	inStorage->integer.unit = inUnit;
}


/*!
	Implementation of GetValueAsNumber for integer values.
*/

LEONumber LEOGetIntegerValueAsNumber( LEOValuePtr self, LEOUnit *outUnit, struct LEOContext* inContext )
{
	if( outUnit )
		*outUnit = self->integer.unit;
	return (LEONumber)self->integer.integer;
}


/*!
	Implementation of GetValueAsInteger for integer values.
*/

LEOInteger LEOGetIntegerValueAsInteger( LEOValuePtr self, LEOUnit *outUnit, struct LEOContext* inContext )
{
	if( outUnit )
		*outUnit = self->integer.unit;
	return self->integer.integer;
}


/*!
	Implementation of GetValueAsString for integer values.
*/

const char*	LEOGetIntegerValueAsString( LEOValuePtr self, char* outBuf, size_t bufSize, struct LEOContext* inContext )
{
	if( outBuf )	// Can never return as string without buffer.
		snprintf( outBuf, bufSize -1, "%lld%s", self->integer.integer, gUnitLabels[self->integer.unit] );
	return outBuf;
}


/*!
	Implementation of SetValueAsNumber for integer values.
*/

void LEOSetIntegerValueAsNumber( LEOValuePtr self, LEONumber inNumber, LEOUnit inUnit, struct LEOContext* inContext )
{
	if( trunc(inNumber) != inNumber )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Can't make fractional number into integer." );
	}
	else
	{
		self->integer.integer = (LEOInteger)inNumber;
		self->integer.unit = inUnit;
	}
}


/*!
	Implementation of SetValueAsInteger for integer values.
*/

void LEOSetIntegerValueAsInteger( LEOValuePtr self, LEOInteger inInteger, LEOUnit inUnit, struct LEOContext* inContext )
{
	self->integer.integer = inInteger;
	self->integer.unit = inUnit;
}


/*!
	Implementation of SetValueAsString for integer values. If the given string
	can't be fully converted to an integer, this will fail with an error message
	and abort execution of the current LEOContext.
*/

void LEOSetIntegerValueAsString( LEOValuePtr self, const char* inInteger, size_t inIntegerLen, struct LEOContext* inContext )
{
	if( inInteger == NULL || inIntegerLen == 0 )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected %s, found empty.", self->base.isa->displayTypeName );
		return;
	}
	char	buf[OTHER_VALUE_SHORT_STRING_MAX_LENGTH +1] = { 0 };
	if( inIntegerLen > OTHER_VALUE_SHORT_STRING_MAX_LENGTH )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected a %s here, found a string, or a number that is too large.", self->base.isa->displayTypeName );
		return;
	}
	strlcpy( buf, inInteger, inIntegerLen );
	
	// Determine if there's a unit on this number, remove it but remember it:
	self->integer.unit = kLEOUnitNone;
	
	for( int x = 1; x < kLEOUnit_Last; x++ )	// Skip first one, which is empty string for 'no unit' and would match anything.
	{
		size_t	unitLen = strlen(gUnitLabels[x]);
		if( unitLen < inIntegerLen )
		{
			if( strcasecmp( buf +(inIntegerLen -unitLen), gUnitLabels[x] ) == 0 )
			{
				inIntegerLen -= unitLen;
				buf[inIntegerLen] = 0;
				self->integer.unit = x;
				break;
			}
		}
	}
	
	char*		endPtr = NULL;
	LEOInteger	theNum = strtoll( buf, &endPtr, 10 );
	if( endPtr != (buf +inIntegerLen) )
		LEOCantSetValueAsString( self, inInteger, inIntegerLen, inContext );
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
	dest->integer.unit = self->integer.unit;
}


void	LEOPutIntegerValueIntoValue( LEOValuePtr self, LEOValuePtr dest, struct LEOContext* inContext )
{
	LEOSetValueAsInteger( dest, self->integer.integer, self->integer.unit, inContext );
}


/*!
	Destructor for integer values. If this value has references, this makes sure
	that they will produce an error message if they ever try to access it again.
*/

void	LEOCleanUpIntegerValue( LEOValuePtr self, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	self->base.isa = NULL;
	self->integer.integer = 0LL;
	self->integer.unit = kLEOUnitNone;
	if( keepReferences == kLEOInvalidateReferences && self->base.refObjectID != kLEOObjectIDINVALID )	// We have references? Make sure they all notice we've gone if they try to access us from now on.
	{
		LEOContextGroupRecycleObjectID( inContext->group, self->base.refObjectID );
		self->base.refObjectID = 0;
	}
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
	inStorage->string.stringLen = inLen;
	inStorage->string.string = calloc( inLen +1, sizeof(char) );
	memmove( inStorage->string.string, inString, inLen );
}


/*!
	Implementation of GetAsNumber for string values. If the given string can't
	be completely converted into a number, this will fail with an error message
	and abort execution of the current LEOContext.
*/

LEONumber	LEOGetStringValueAsNumber( LEOValuePtr self, LEOUnit *outUnit, struct LEOContext* inContext )
{
	// Determine if there's a unit on this number, remove it but remember it:
	LEOUnit		theUnit = kLEOUnitNone;
	size_t		lengthToParse = self->string.stringLen;
	
	for( int x = 1; x < kLEOUnit_Last; x++ )	// Skip first one, which is empty string for 'no unit' and would match anything.
	{
		size_t	unitLen = strlen(gUnitLabels[x]);
		if( unitLen < lengthToParse )
		{
			if( strcasecmp( self->string.string +(lengthToParse -unitLen), gUnitLabels[x] ) == 0 )
			{
				lengthToParse -= unitLen;
				theUnit = x;
				break;
			}
		}
	}

	char*		endPtr = NULL;
	LEONumber	num = strtof( self->string.string, &endPtr );
	if( endPtr != (self->string.string +lengthToParse) )
		LEOCantGetValueAsNumber( self, outUnit, inContext );
	
	if( outUnit )
		*outUnit = theUnit;
	
	return num;
}


/*!
	Implementation of GetAsInteger for string values. If the given string can't
	be completely converted into an integer, this will fail with an error message
	and abort execution of the current LEOContext.
*/

LEOInteger	LEOGetStringValueAsInteger( LEOValuePtr self, LEOUnit* outUnit, struct LEOContext* inContext )
{
	// Determine if there's a unit on this number, remove it but remember it:
	LEOUnit		theUnit = kLEOUnitNone;
	size_t		lengthToParse = self->string.stringLen;
	
	for( int x = 1; x < kLEOUnit_Last; x++ )	// Skip first one, which is empty string for 'no unit' and would match anything.
	{
		size_t	unitLen = strlen(gUnitLabels[x]);
		if( unitLen < lengthToParse )
		{
			if( strcasecmp( self->string.string +(lengthToParse -unitLen), gUnitLabels[x] ) == 0 )
			{
				lengthToParse -= unitLen;
				theUnit = x;
				break;
			}
		}
	}

	char*		endPtr = NULL;
	LEOInteger	num = strtoll( self->string.string, &endPtr, 10 );
	if( endPtr != (self->string.string +lengthToParse) )
		LEOCantGetValueAsInteger( self, outUnit, inContext );
	
	if( outUnit )
		*outUnit = theUnit;

	return num;
}


/*!
	Implementation of GetAsString for string values.
*/

const char*	LEOGetStringValueAsString( LEOValuePtr self, char* outBuf, size_t bufSize, struct LEOContext* inContext )
{
	if( outBuf )	// If given a buffer, copy over, caller may really want a copy. Always return our internal buffer, which contains the whole string.
		strlcpy( outBuf, self->string.string, bufSize );	// TODO: Not NUL-safe!
	return self->string.string;
}


/*!
	Implementation of SetAsNumber for string values.
*/

void	LEOSetStringValueAsNumber( LEOValuePtr self, LEONumber inNumber, LEOUnit inUnit, struct LEOContext* inContext )
{
	if( self->string.string )
		free( self->string.string );
	self->string.string = calloc( OTHER_VALUE_SHORT_STRING_MAX_LENGTH, sizeof(char) );
	self->string.stringLen = snprintf( self->string.string, OTHER_VALUE_SHORT_STRING_MAX_LENGTH, "%g%s", inNumber, gUnitLabels[inUnit] );
}


/*!
	Implementation of SetAsInteger for string values.
*/

void	LEOSetStringValueAsInteger( LEOValuePtr self, LEOInteger inInteger, LEOUnit inUnit, struct LEOContext* inContext )
{
	if( self->string.string )
		free( self->string.string );
	self->string.string = calloc( OTHER_VALUE_SHORT_STRING_MAX_LENGTH, sizeof(char) );
	self->string.stringLen = snprintf( self->string.string, OTHER_VALUE_SHORT_STRING_MAX_LENGTH, "%lld%s", inInteger, gUnitLabels[inUnit] );
}


/*!
	Implementation of GetAsBoolean for string values. If the given string can't
	be completely converted into a boolean, this will fail with an error message
	and abort execution of the current LEOContext.
*/

bool	LEOGetStringValueAsBoolean( LEOValuePtr self, struct LEOContext* inContext )
{
	if( strcasecmp( self->string.string, "true" ) == 0 && self->string.stringLen == 4 )
		return true;
	else if( strcasecmp( self->string.string, "false" ) == 0 && self->string.stringLen == 5 )
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
	LEOGetChunkRanges( self->string.string, inType,	// TODO: Make NUL-safe.
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
	Implementation of SetAsString for string values. This turns the string value
	into a constant string if the passed-in string is NULL (from delete) or 0-length.
*/

void LEOSetStringValueAsString( LEOValuePtr self, const char* inString, size_t inStringLen, struct LEOContext* inContext )
{
	if( inString == NULL || inStringLen == 0 )
	{
		LEOSetStringValueAsStringConstant( self, "", inContext );
		return;
	}
	if( self->string.string )
		free( self->string.string );
	self->string.stringLen = inStringLen;
	self->string.string = calloc( self->string.stringLen +1, sizeof(char) );
	memmove( self->string.string, inString, self->string.stringLen );
}


void	LEOSetStringValueAsNativeObject( LEOValuePtr self, void* inNativeObject, struct LEOContext* inContext )
{
	if( inNativeObject != NULL )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected %s, found native object", self->base.isa->displayTypeName );
	}
	else
	{
		LEOSetStringValueAsStringConstant( self, sUnsetConstantString, inContext );
	}
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
	self->string.stringLen = strlen(inString);
}


/*!
	Implementation of InitCopy for string values.
*/

void	LEOInitStringValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	dest->base.isa = &kLeoValueTypeString;
	if( keepReferences == kLEOInvalidateReferences )
		dest->base.refObjectID = kLEOObjectIDINVALID;
	size_t		theLen = self->string.stringLen +1;
	dest->string.string = calloc( theLen, sizeof(char) );
	dest->string.stringLen = self->string.stringLen;
	strlcpy( dest->string.string, self->string.string, theLen );
}


void	LEOPutStringValueIntoValue( LEOValuePtr self, LEOValuePtr dest, struct LEOContext* inContext )
{
	LEOSetValueAsString( dest, self->string.string, self->string.stringLen, inContext );
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
	
	LEOGetChunkRanges( str, inType,					// TODO: Make NUL-safe.
						inRangeStart, inRangeEnd,
						&chunkStart, &chunkEnd,
						&delChunkStart, &delChunkEnd,
						inContext->itemDelimiter );
	if( chunkStart > maxOffs )
		chunkStart = maxOffs;
	if( chunkEnd > maxOffs )
		chunkEnd = maxOffs;
	size_t	oldBytesStart = *ioBytesStart;
	(*ioBytesStart) += chunkStart;
	(*ioBytesEnd) = (*ioBytesStart) + (chunkEnd -chunkStart);
	if( delChunkStart > maxOffs )
		delChunkStart = maxOffs;
	if( delChunkEnd > maxOffs )
		delChunkEnd = maxOffs;
	(*ioBytesDelStart) = oldBytesStart +delChunkStart;
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
				selfLen = self->string.stringLen,
				finalLen = 0;
	LEOGetChunkRanges( self->string.string, inType,	// TODO: Make NUL-safe.
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
	self->string.stringLen = finalLen;
}


/*!
	Implementation of SetPredeterminedRangeAsString for string values.
*/

void	LEOSetStringValuePredeterminedRangeAsString( LEOValuePtr self,
											size_t inRangeStart, size_t inRangeEnd,
											const char* inBuf, struct LEOContext* inContext )
{
	size_t		inBufLen = inBuf ? strlen(inBuf) : 0,
				selfLen = self->string.stringLen,
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
	self->string.stringLen = finalLen;
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
	self->string.stringLen = 0;
	if( keepReferences == kLEOInvalidateReferences && self->base.refObjectID != kLEOObjectIDINVALID )
	{
		LEOContextGroupRecycleObjectID( inContext->group, self->base.refObjectID );
		self->base.refObjectID = 0;
	}
}


bool	LEOCanGetStringValueAsNumber( LEOValuePtr self, struct LEOContext* inContext )
{
	if( self->string.stringLen == 0 )	// Empty string? Not a number!
		return false;
	
	bool hadDot = false;
	bool isFirst = true;
	
	for( size_t x = 0; x < self->string.stringLen; x++ )
	{
		if( isFirst && self->string.string[x] == '-' )
			;	// It's OK to have negative numbers.
		else if( !hadDot && self->string.string[x] == '.' )
		{
			hadDot = true;
		}
		else if( self->string.string[x] < '0' || self->string.string[x] > '9' )
		{
			return false;
		}
		isFirst = false;
	}
	
	return true;
}


bool	LEOCanGetStringValueAsInteger( LEOValuePtr self, struct LEOContext* inContext )
{
	if( self->string.stringLen == 0 )	// Empty string? Not a number!
		return false;
	
	bool isFirst = true;

	for( size_t x = 0; x < self->string.stringLen; x++ )
	{
		if( isFirst && self->string.string[x] == '-' )
			;	// It's OK to have negative numbers.
		else if( self->string.string[x] < '0' || self->string.string[x] > '9' )
			return false;
		
		isFirst = false;
	}
	
	return true;
}


void	LEOSetStringValueAsRect( LEOValuePtr self, LEOInteger l, LEOInteger t, LEOInteger r, LEOInteger b, struct LEOContext* inContext )
{
	if( self->string.string )
		free( self->string.string );
	self->string.string = calloc(sizeof(char), OTHER_VALUE_SHORT_STRING_MAX_LENGTH);	// +++ realloc when we know the size?
	if( inContext->group->flags & kLEOContextGroupFlagHyperCardCompatibility )
		self->string.stringLen = snprintf( self->string.string, OTHER_VALUE_SHORT_STRING_MAX_LENGTH -1, "%lld,%lld,%lld,%lld", l, t, r, b );
	else
		self->string.stringLen = snprintf( self->string.string, OTHER_VALUE_SHORT_STRING_MAX_LENGTH -1, "left:%lld\ntop:%lld\nright:%lld\nbottom:%lld", l, t, r, b );
}


void	LEOSetStringValueAsPoint( LEOValuePtr self, LEOInteger l, LEOInteger t, struct LEOContext* inContext )
{
	if( self->string.string )
		free( self->string.string );
	self->string.string = calloc(sizeof(char), OTHER_VALUE_SHORT_STRING_MAX_LENGTH);	// +++ realloc when we know the size?
	if( inContext->group->flags & kLEOContextGroupFlagHyperCardCompatibility )
		self->string.stringLen = snprintf( self->string.string, OTHER_VALUE_SHORT_STRING_MAX_LENGTH -1, "%lld,%lld", l, t );
	else
		self->string.stringLen = snprintf( self->string.string, OTHER_VALUE_SHORT_STRING_MAX_LENGTH -1, "horizontal:%lld\nvertical:%lld", l, t );
}


void	LEOGetStringValueAsRect( LEOValuePtr self, LEOInteger *l, LEOInteger *t, LEOInteger *r, LEOInteger *b, struct LEOContext* inContext )
{
	LEOStringToRect( self->string.string, self->string.stringLen, l, t, r, b, inContext );
}


void	LEOGetStringValueAsPoint( LEOValuePtr self, LEOInteger *l, LEOInteger *t, struct LEOContext* inContext )
{
	LEOStringToPoint( self->string.string, self->string.stringLen, l, t, inContext );
}


void	LEOSetStringValueAsRange( LEOValuePtr self, LEOInteger s, LEOInteger e, LEOChunkType t, struct LEOContext* inContext )
{
	if( self->string.string )
		free( self->string.string );
	self->string.string = calloc(sizeof(char), OTHER_VALUE_SHORT_STRING_MAX_LENGTH);	// +++ realloc when we know the size?
	if( s == e )
		self->string.stringLen = snprintf( self->string.string, OTHER_VALUE_SHORT_STRING_MAX_LENGTH -1, "%s %lld", gLEOChunkTypeNames[t], s );
	else
		self->string.stringLen = snprintf( self->string.string, OTHER_VALUE_SHORT_STRING_MAX_LENGTH -1, "%s %lld to %lld", gLEOChunkTypeNames[t], s, e );
}


void	LEOGetStringValueAsRange( LEOValuePtr self, LEOInteger *s, LEOInteger *e, LEOChunkType *t, struct LEOContext* inContext )
{
	LEOStringToRange( self->string.string, self->string.stringLen, s, e, t, inContext );
}


void	LEOSetStringLikeValueAsRect( LEOValuePtr self, LEOInteger l, LEOInteger t, LEOInteger r, LEOInteger b, struct LEOContext* inContext )
{
	char	buf[OTHER_VALUE_SHORT_STRING_MAX_LENGTH] = {0};
	size_t	usedLen = 0;
	if( inContext->group->flags & kLEOContextGroupFlagHyperCardCompatibility )
		usedLen = snprintf( buf, sizeof(buf) -1, "%lld,%lld,%lld,%lld", l, t, r, b );
	else
		usedLen = snprintf( buf, sizeof(buf) -1, "left:%lld\ntop:%lld\nright:%lld\nbottom:%lld", l, t, r, b );
	LEOSetValueAsString( self, buf, usedLen, inContext );
}


void	LEOSetStringLikeValueAsPoint( LEOValuePtr self, LEOInteger l, LEOInteger t, struct LEOContext* inContext )
{
	char	buf[OTHER_VALUE_SHORT_STRING_MAX_LENGTH] = {0};
	size_t	usedLen = 0;
	if( inContext->group->flags & kLEOContextGroupFlagHyperCardCompatibility )
		usedLen = snprintf( buf, sizeof(buf) -1, "%lld,%lld", l, t );
	else
		usedLen = snprintf( buf, sizeof(buf) -1, "horizontal:%lld\nvertical:%lld", l, t );
	LEOSetValueAsString( self, buf, usedLen, inContext );
}


void	LEOGetStringLikeValueAsRect( LEOValuePtr self, LEOInteger *l, LEOInteger *t, LEOInteger *r, LEOInteger *b, struct LEOContext* inContext )
{
	char	buf[OTHER_VALUE_SHORT_STRING_MAX_LENGTH] = {0};
	const char*		str = LEOGetValueAsString( self, buf, sizeof(buf), inContext );
	LEOStringToRect( str, strlen(str), l, t, r, b, inContext );
}


void	LEOGetStringLikeValueAsPoint( LEOValuePtr self, LEOInteger *l, LEOInteger *t, struct LEOContext* inContext )
{
	char			buf[OTHER_VALUE_SHORT_STRING_MAX_LENGTH] = {0};
	const char*		str = LEOGetValueAsString( self, buf, sizeof(buf), inContext );
	LEOStringToPoint( str, strlen(str), l, t, inContext );
}


void	LEOSetStringLikeValueAsRange( LEOValuePtr self, LEOInteger s, LEOInteger e, LEOChunkType t, struct LEOContext* inContext )
{
	char	buf[OTHER_VALUE_SHORT_STRING_MAX_LENGTH] = {0};
	size_t	usedLen = 0;
	if( s == e )
		usedLen = snprintf( buf, sizeof(buf) -1, "%s %lld", gLEOChunkTypeNames[t], s );
	else
		usedLen = snprintf( buf, sizeof(buf) -1, "%s %lld to %lld", gLEOChunkTypeNames[t], s, e );
}


void	LEOGetStringLikeValueAsRange( LEOValuePtr self, LEOInteger *s, LEOInteger *e, LEOChunkType *t, struct LEOContext* inContext )
{
	char			buf[OTHER_VALUE_SHORT_STRING_MAX_LENGTH] = {0};
	const char*		str = LEOGetValueAsString( self, buf, sizeof(buf), inContext );
	LEOStringToRange( str, strlen(str), s, e, t, inContext );
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
	inStorage->string.stringLen = strlen(inString);
}


void	LEOInitUnsetValue( LEOValuePtr inStorage, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	inStorage->base.isa = &kLeoValueTypeStringConstant;
	if( keepReferences == kLEOInvalidateReferences )
		inStorage->base.refObjectID = kLEOObjectIDINVALID;
	inStorage->string.string = sUnsetConstantString;
	inStorage->string.stringLen = 0;
}


/*!
	Implementation of SetAsNumber for string constant values. This turns the
	value into a regular (dynamic) string value.
*/

void	LEOSetStringConstantValueAsNumber( LEOValuePtr self, LEONumber inNumber, LEOUnit inUnit, struct LEOContext* inContext )
{
	// Turn this into a non-constant string:
	self->base.isa = &kLeoValueTypeString;
	self->string.string = calloc( OTHER_VALUE_SHORT_STRING_MAX_LENGTH, sizeof(char) );
	self->string.stringLen = snprintf( self->string.string, OTHER_VALUE_SHORT_STRING_MAX_LENGTH, "%g%s", inNumber, gUnitLabels[inUnit] );
}


/*!
	Implementation of SetAsInteger for string constant values. This turns the
	value into a regular (dynamic) string value.
*/

void	LEOSetStringConstantValueAsInteger( LEOValuePtr self, LEOInteger inInteger, LEOUnit inUnit, struct LEOContext* inContext )
{
	// Turn this into a non-constant string:
	self->base.isa = &kLeoValueTypeString;
	self->string.string = calloc( OTHER_VALUE_SHORT_STRING_MAX_LENGTH, sizeof(char) );
	self->string.stringLen = snprintf( self->string.string, OTHER_VALUE_SHORT_STRING_MAX_LENGTH, "%lld%s", inInteger, gUnitLabels[inUnit] );
}


/*!
	Implementation of SetAsString for string constant values. This turns the
	value into a regular (dynamic) string value.
*/

void	LEOSetStringConstantValueAsString( LEOValuePtr self, const char* inString, size_t inStringLen, struct LEOContext* inContext )
{
	if( inString == NULL || inStringLen == 0 )
	{
		self->string.string = "";
		self->string.stringLen = 0;
		return;
	}
	// Turn this into a non-constant string:
	self->base.isa = &kLeoValueTypeString;
	self->string.string = calloc( inStringLen +1, sizeof(char) );
	self->string.stringLen = inStringLen;
	memmove( self->string.string, inString, inStringLen );
}


/*!
	Implementation of SetAsString for string constant values.
*/

void	LEOSetStringConstantValueAsBoolean( LEOValuePtr self, bool inBoolean, struct LEOContext* inContext )
{
	self->string.string = (inBoolean ? "true" : "false");
	self->string.stringLen = (inBoolean ? 4 : 5);
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
	dest->string.stringLen = self->string.stringLen;
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
				selfLen = self->string.stringLen,
				finalLen = 0;
	LEOGetChunkRanges( self->string.string, inType,		// Make NUL-safe
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
	self->string.stringLen = finalLen;
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
	self->string.stringLen = 0;
	if( keepReferences == kLEOInvalidateReferences && self->base.refObjectID != kLEOObjectIDINVALID )
	{
		LEOContextGroupRecycleObjectID( inContext->group, self->base.refObjectID );
		self->base.refObjectID = 0;
	}
}


void	LEOSetStringConstantValueAsRect( LEOValuePtr self, LEOInteger l, LEOInteger t, LEOInteger r, LEOInteger b, struct LEOContext* inContext )
{
	self->base.isa = &kLeoValueTypeString;
	self->string.string = calloc(sizeof(char),OTHER_VALUE_SHORT_STRING_MAX_LENGTH);	// +++ realloc when we know the size?
	if( inContext->group->flags & kLEOContextGroupFlagHyperCardCompatibility )
		self->string.stringLen = snprintf( self->string.string, OTHER_VALUE_SHORT_STRING_MAX_LENGTH -1, "%lld,%lld,%lld,%lld", l, t, r, b );
	else
		self->string.stringLen = snprintf( self->string.string, OTHER_VALUE_SHORT_STRING_MAX_LENGTH -1, "left:%lld\ntop:%lld\nright:%lld\nbottom:%lld", l, t, r, b );
}


void	LEOSetStringConstantValueAsPoint( LEOValuePtr self, LEOInteger l, LEOInteger t, struct LEOContext* inContext )
{
	self->base.isa = &kLeoValueTypeString;
	self->string.string = calloc(sizeof(char),OTHER_VALUE_SHORT_STRING_MAX_LENGTH);	// +++ realloc when we know the size?
	if( inContext->group->flags & kLEOContextGroupFlagHyperCardCompatibility )
		self->string.stringLen = snprintf( self->string.string, OTHER_VALUE_SHORT_STRING_MAX_LENGTH -1, "%lld,%lld", l, t );
	else
		self->string.stringLen = snprintf( self->string.string, OTHER_VALUE_SHORT_STRING_MAX_LENGTH -1, "horizontal:%lld\nvertical:%lld", l, t );
}


void	LEOSetStringConstantValueAsRange( LEOValuePtr self, LEOInteger s, LEOInteger e, LEOChunkType t, struct LEOContext* inContext )
{
	self->base.isa = &kLeoValueTypeString;
	self->string.string = calloc(sizeof(char),OTHER_VALUE_SHORT_STRING_MAX_LENGTH);	// +++ realloc when we know the size?
	if( s == e )
		self->string.stringLen = snprintf( self->string.string, OTHER_VALUE_SHORT_STRING_MAX_LENGTH -1, "%s %lld", gLEOChunkTypeNames[t], s );
	else
		self->string.stringLen = snprintf( self->string.string, OTHER_VALUE_SHORT_STRING_MAX_LENGTH -1, "%s %lld to %lld", gLEOChunkTypeNames[t], s, e );
}


bool	LEOGetStringConstantValueIsUnset( LEOValuePtr self, struct LEOContext* inContext )
{
	return( self->string.string == sUnsetConstantString );
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

const char*	LEOGetBooleanValueAsString( LEOValuePtr self, char* outBuf, size_t bufSize, struct LEOContext* inContext )
{
	if( outBuf )
	{
		strlcpy( outBuf, (self->boolean.boolean ? "true" : "false"), bufSize -1 );
		return outBuf;
	}
	else
		return (self->boolean.boolean ? "true" : "false");
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

void	LEOSetBooleanValueAsString( LEOValuePtr self, const char* inString, size_t inStringLen, struct LEOContext* inContext )
{
	if( strcasecmp( inString, "true" ) == 0 && inStringLen == 4 )
		((struct LEOValueBoolean*)self)->boolean = true;
	else if( strcasecmp( inString, "false" ) == 0 && inStringLen == 5 )
		((struct LEOValueBoolean*)self)->boolean = false;
	else
		LEOCantSetValueAsString( self, inString, inStringLen, inContext );
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


void	LEOPutBooleanValueIntoValue( LEOValuePtr self, LEOValuePtr dest, struct LEOContext* inContext )
{
	LEOSetValueAsBoolean( dest, self->boolean.boolean, inContext );
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
	{
		LEOContextGroupRecycleObjectID( inContext->group, self->base.refObjectID );
		self->base.refObjectID = 0;
	}
}


#pragma mark -
#pragma mark Rect


void	LEOInitRectValue( LEOValuePtr self, LEOInteger l, LEOInteger t, LEOInteger r, LEOInteger b, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	self->base.isa = &kLeoValueTypeRect;
	if( keepReferences == kLEOInvalidateReferences )
		self->base.refObjectID = kLEOObjectIDINVALID;
	self->rect.left = l;
	self->rect.top = t;
	self->rect.right = r;
	self->rect.bottom = b;
}


/*!
	Implementation of GetAsString for rectangle values.
*/

const char*	LEOGetRectValueAsString( LEOValuePtr self, char* outBuf, size_t bufSize, struct LEOContext* inContext )
{
	if( outBuf )	// Can never return as string without buffer.
	{
		if( inContext->group->flags & kLEOContextGroupFlagHyperCardCompatibility )
			snprintf( outBuf, bufSize -1, "%lld,%lld,%lld,%lld", self->rect.left, self->rect.top, self->rect.right, self->rect.bottom );
		else
			snprintf( outBuf, bufSize -1, "left:%lld\ntop:%lld\nright:%lld\nbottom:%lld", self->rect.left, self->rect.top, self->rect.right, self->rect.bottom );
	}
	return outBuf;
}


void	LEOArrayToRect( struct LEOArrayEntry* convertedArray, LEOInteger *l, LEOInteger *t, LEOInteger *r, LEOInteger *b, struct LEOContext* inContext )
{
	if( LEOGetArrayKeyCount(convertedArray) != 4 )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected rectangle here." );
		return;
	}
	
	// left:
	LEOValuePtr	theNumObj = LEOGetArrayValueForKey( convertedArray, "left" );
	if( !theNumObj )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected rectangle here." );
		return;
	}
	LEOUnit		theUnit = kLEOUnitNone;
	LEOInteger	theNum = LEOGetValueAsInteger( theNumObj, &theUnit, inContext );
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected rectangle here." );
		return;
	}
	*l = theNum;
	
	// top:
	theNumObj = LEOGetArrayValueForKey( convertedArray, "top" );
	if( !theNumObj )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected rectangle here." );
		return;
	}
	theNum = LEOGetValueAsInteger( theNumObj, &theUnit, inContext );
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected rectangle here." );
		return;
	}
	*t = theNum;
	
	// right:
	theNumObj = LEOGetArrayValueForKey( convertedArray, "right" );
	if( !theNumObj )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected rectangle here." );
		return;
	}
	theNum = LEOGetValueAsInteger( theNumObj, &theUnit, inContext );
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected rectangle here." );
		return;
	}
	*r = theNum;
	
	// bottom:
	theNumObj = LEOGetArrayValueForKey( convertedArray, "bottom" );
	if( !theNumObj )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected rectangle here." );
		return;
	}
	theNum = LEOGetValueAsInteger( theNumObj, &theUnit, inContext );
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected rectangle here." );
		return;
	}
	*b = theNum;
}


void	LEOStringToRect( const char* inString, size_t inStringLen, LEOInteger *l, LEOInteger *t, LEOInteger *r, LEOInteger *b, struct LEOContext* inContext )
{
	if( inContext->group->flags & kLEOContextGroupFlagHyperCardCompatibility )
	{
		// left:
		char	numPart[OTHER_VALUE_SHORT_STRING_MAX_LENGTH] = {0};
		char*	endPtr = NULL;
		size_t	numPartLen = 0,
				x = 0;
		for( ; x < inStringLen && inString[x] != ',' && numPartLen < (OTHER_VALUE_SHORT_STRING_MAX_LENGTH -1); x++ )
			numPart[numPartLen++] = inString[x];
		numPart[numPartLen] = '\0';
		if( numPartLen == 0 )
		{
			size_t		lineNo = SIZE_MAX;
			uint16_t	fileID = 0;
			LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
			LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected rectangle, found string." );
			return;
		}
		*l = strtoll( numPart, &endPtr, 10 );
		if( endPtr != (numPart +numPartLen) )
		{
			size_t		lineNo = SIZE_MAX;
			uint16_t	fileID = 0;
			LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
			LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected rectangle, found string." );
			return;
		}
		
		// top:
		x++;
		numPartLen = 0;
		for( ; x < inStringLen && inString[x] != ',' && numPartLen < (OTHER_VALUE_SHORT_STRING_MAX_LENGTH -1); x++ )
			numPart[numPartLen++] = inString[x];
		numPart[numPartLen] = '\0';
		if( numPartLen == 0 )
		{
			size_t		lineNo = SIZE_MAX;
			uint16_t	fileID = 0;
			LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
			LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected rectangle, found string." );
			return;
		}
		*t = strtoll( numPart, &endPtr, 10 );
		if( endPtr != (numPart +numPartLen) )
		{
			size_t		lineNo = SIZE_MAX;
			uint16_t	fileID = 0;
			LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
			LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected rectangle, found string." );
			return;
		}
		
		// right:
		x++;
		numPartLen = 0;
		for( ; x < inStringLen && inString[x] != ',' && numPartLen < (OTHER_VALUE_SHORT_STRING_MAX_LENGTH -1); x++ )
			numPart[numPartLen++] = inString[x];
		numPart[numPartLen] = '\0';
		if( numPartLen == 0 )
		{
			size_t		lineNo = SIZE_MAX;
			uint16_t	fileID = 0;
			LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
			LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected rectangle, found string." );
			return;
		}
		*r = strtoll( numPart, &endPtr, 10 );
		if( endPtr != (numPart +numPartLen) )
		{
			size_t		lineNo = SIZE_MAX;
			uint16_t	fileID = 0;
			LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
			LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected rectangle, found string." );
			return;
		}
		
		// bottom:
		x++;
		numPartLen = 0;
		for( ; x < inStringLen && numPartLen < (OTHER_VALUE_SHORT_STRING_MAX_LENGTH -1); x++ )
			numPart[numPartLen++] = inString[x];
		numPart[numPartLen] = '\0';
		if( numPartLen == 0 )
		{
			size_t		lineNo = SIZE_MAX;
			uint16_t	fileID = 0;
			LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
			LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected rectangle, found string." );
			return;
		}
		*b = strtoll( numPart, &endPtr, 10 );
		if( endPtr != (numPart +numPartLen) )
		{
			size_t		lineNo = SIZE_MAX;
			uint16_t	fileID = 0;
			LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
			LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected rectangle, found string." );
			return;
		}
	}
	else
	{
		struct LEOArrayEntry*	convertedArray = LEOCreateArrayFromString( inString, inStringLen, inContext );
		if( !convertedArray || LEOGetArrayKeyCount( convertedArray ) != 4 )
		{
			size_t		lineNo = SIZE_MAX;
			uint16_t	fileID = 0;
			LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
			LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected rectangle, found string." );
			return;
		}
		
		LEOArrayToRect( convertedArray, l, t, r, b, inContext );
	}
}

/*!
	Implementation of SetValueAsString for rectangle values. If the given string
	can't be fully converted to a rectangle, this will fail with an error message
	and abort execution of the current LEOContext.
*/

void	LEOSetRectValueAsString( LEOValuePtr self, const char* inString, size_t inStringLen, struct LEOContext* inContext )
{
	LEOStringToRect( inString, inStringLen, &self->rect.left, &self->rect.top, &self->rect.right, &self->rect.bottom, inContext );
}


void	LEOInitRectValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	dest->base.isa = &kLeoValueTypeRect;
	if( keepReferences == kLEOInvalidateReferences )
		dest->base.refObjectID = kLEOObjectIDINVALID;
	dest->rect.left = self->rect.left;
	dest->rect.top = self->rect.top;
	dest->rect.right = self->rect.right;
	dest->rect.bottom = self->rect.bottom;
}


void	LEOPutRectValueIntoValue( LEOValuePtr self, LEOValuePtr dest, struct LEOContext* inContext )
{
	LEOSetValueAsRect( dest, self->rect.left, self->rect.top, self->rect.right, self->rect.bottom, inContext );
}


void	LEOCleanUpRectValue( LEOValuePtr self, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	self->base.isa = NULL;
	self->rect.left = 0;
	self->rect.top = 0;
	self->rect.right = 0;
	self->rect.bottom = 0;
	if( keepReferences == kLEOInvalidateReferences && self->base.refObjectID != kLEOObjectIDINVALID )
	{
		LEOContextGroupRecycleObjectID( inContext->group, self->base.refObjectID );
		self->base.refObjectID = 0;
	}
}


LEOValuePtr		LEOGetRectValueValueForKey( LEOValuePtr self, const char* inKey, union LEOValue *tempStorage, LEOKeepReferencesFlag keepReferences, struct LEOContext * inContext )
{
	if( inContext->group->flags & kLEOContextGroupFlagHyperCardCompatibility )
		return LEOCantGetValueForKey( self, inKey, tempStorage, keepReferences, inContext );
	
	LEOInteger		theNum = 0;
	if( strcasecmp(inKey, "left") == 0 )
		theNum = self->rect.left;
	else if( strcasecmp(inKey, "top") == 0 )
		theNum = self->rect.top;
	else if( strcasecmp(inKey, "right") == 0 )
		theNum = self->rect.right;
	else if( strcasecmp(inKey, "bottom") == 0 )
		theNum = self->rect.bottom;
	LEOInitIntegerValue( tempStorage, theNum, kLEOUnitNone, keepReferences, inContext );
	return tempStorage;
}


void		LEOSetRectValueValueForKey( LEOValuePtr self, const char* inKey, LEOValuePtr inValue, struct LEOContext * inContext )
{
	if( inContext->group->flags & kLEOContextGroupFlagHyperCardCompatibility )
	{
		LEOCantSetValueForKey( self, inKey, inValue, inContext );
		return;
	}
	bool		mustConvertToArray = false;
	LEOUnit		theUnit = kLEOUnitNone;
	LEOInteger	theNum = LEOGetValueAsInteger( inValue, &theUnit, inContext );
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
	{
		if( self->base.isa == &kLeoValueTypeRectVariant )	// Variant? Convert to an array.
		{
			inContext->flags |= kLEOContextKeepRunning;
			mustConvertToArray = true;
			theNum = 0;
		}
		else
		{
			size_t		lineNo = SIZE_MAX;
			uint16_t	fileID = 0;
			LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
			LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected integer, found %s.", inValue->base.isa->displayTypeName );
			return;
		}
	}
	if( strcasecmp(inKey, "left") == 0 && !mustConvertToArray )
		self->rect.left = theNum;
	else if( strcasecmp(inKey, "top") == 0 && !mustConvertToArray )
		self->rect.top = theNum;
	else if( strcasecmp(inKey, "right") == 0 && !mustConvertToArray )
		self->rect.right = theNum;
	else if( strcasecmp(inKey, "bottom") == 0 && !mustConvertToArray )
		self->rect.bottom = theNum;
	else if( self->base.isa == &kLeoValueTypeRectVariant )
	{
		struct LEOArrayEntry	*	array = NULL;
		LEOAddIntegerArrayEntryToRoot( &array, "left", self->rect.left, kLEOUnitNone, inContext );
		LEOAddIntegerArrayEntryToRoot( &array, "top", self->rect.top, kLEOUnitNone, inContext );
		LEOAddIntegerArrayEntryToRoot( &array, "right", self->rect.right, kLEOUnitNone, inContext );
		LEOAddIntegerArrayEntryToRoot( &array, "bottom", self->rect.bottom, kLEOUnitNone, inContext );
		LEOAddArrayEntryToRoot( &array, inKey, inValue, inContext );
		
		LEOCleanUpValue( self, kLEOKeepReferences, inContext );
		LEOInitArrayVariantValue( self, array, kLEOKeepReferences, inContext );
	}
	else
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Can't set key %s on a rectangle, must be \"left\", \"top\", \"right\" or \"bottom\".", inKey );
		return;
	}
}


size_t		LEOGetRectValueKeyCount( LEOValuePtr self, struct LEOContext * inContext )
{
	if( inContext->group->flags & kLEOContextGroupFlagHyperCardCompatibility )
		return LEOCantGetKeyCount( self, inContext );

	return 4;
}


void	LEOSetRectValueAsRect( LEOValuePtr self, LEOInteger l, LEOInteger t, LEOInteger r, LEOInteger b, struct LEOContext * inContext )
{
	self->rect.left = l;
	self->rect.top = t;
	self->rect.right = r;
	self->rect.bottom = b;
}


void	LEOGetRectValueAsRect( LEOValuePtr self, LEOInteger* l, LEOInteger* t, LEOInteger* r, LEOInteger* b, struct LEOContext * inContext )
{
	*l = self->rect.left;
	*t = self->rect.top;
	*r = self->rect.right;
	*b = self->rect.bottom;
}


#pragma mark -
#pragma mark Point


void	LEOInitPointValue( LEOValuePtr self, LEOInteger l, LEOInteger t, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	self->base.isa = &kLeoValueTypePoint;
	if( keepReferences == kLEOInvalidateReferences )
		self->base.refObjectID = kLEOObjectIDINVALID;
	self->point.horizontal = l;
	self->point.vertical = t;
}


/*!
	Implementation of GetAsString for point values.
*/

const char*	LEOGetPointValueAsString( LEOValuePtr self, char* outBuf, size_t bufSize, struct LEOContext* inContext )
{
	if( outBuf )	// Can never return as string without buffer.
	{
		if( inContext->group->flags & kLEOContextGroupFlagHyperCardCompatibility )
			snprintf( outBuf, bufSize -1, "%lld,%lld", self->point.horizontal, self->point.vertical );
		else
			snprintf( outBuf, bufSize -1, "horizontal:%lld\nvertical:%lld", self->point.horizontal, self->point.vertical );
	}
	return outBuf;
}


void	LEOArrayToPoint( struct LEOArrayEntry* convertedArray, LEOInteger *l, LEOInteger *t, struct LEOContext* inContext )
{
	if( LEOGetArrayKeyCount(convertedArray) != 2 )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected point here." );
		return;
	}
	
	// horizontal:
	LEOValuePtr	theNumObj = LEOGetArrayValueForKey( convertedArray, "horizontal" );
	if( !theNumObj )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected point here." );
		return;
	}
	LEOUnit		theUnit = kLEOUnitNone;
	LEOInteger	theNum = LEOGetValueAsInteger( theNumObj, &theUnit, inContext );
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected point here." );
		return;
	}
	*l = theNum;
	
	// vertical:
	theNumObj = LEOGetArrayValueForKey( convertedArray, "vertical" );
	if( !theNumObj )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected point here." );
		return;
	}
	theNum = LEOGetValueAsInteger( theNumObj, &theUnit, inContext );
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected point here." );
		return;
	}
	*t = theNum;
}


void	LEOStringToPoint( const char* inString, size_t inStringLen, LEOInteger *l, LEOInteger *t, struct LEOContext* inContext )
{
	if( inContext->group->flags & kLEOContextGroupFlagHyperCardCompatibility )
	{
		// horizontal:
		char	numPart[OTHER_VALUE_SHORT_STRING_MAX_LENGTH] = {0};
		char*	endPtr = NULL;
		size_t	numPartLen = 0,
				x = 0;
		for( ; x < inStringLen && inString[x] != ',' && numPartLen < (OTHER_VALUE_SHORT_STRING_MAX_LENGTH -1); x++ )
			numPart[numPartLen++] = inString[x];
		numPart[numPartLen] = '\0';
		if( numPartLen == 0 )
		{
			size_t		lineNo = SIZE_MAX;
			uint16_t	fileID = 0;
			LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
			LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected point, found string." );
			return;
		}
		*l = strtoll( numPart, &endPtr, 10 );
		if( endPtr != (numPart +numPartLen) )
		{
			size_t		lineNo = SIZE_MAX;
			uint16_t	fileID = 0;
			LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
			LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected point, found string." );
			return;
		}
		
		// vertical:
		x++;
		numPartLen = 0;
		for( ; x < inStringLen && inString[x] != ',' && numPartLen < (OTHER_VALUE_SHORT_STRING_MAX_LENGTH -1); x++ )
			numPart[numPartLen++] = inString[x];
		numPart[numPartLen] = '\0';
		if( numPartLen == 0 )
		{
			size_t		lineNo = SIZE_MAX;
			uint16_t	fileID = 0;
			LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
			LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected point, found string." );
			return;
		}
		*t = strtoll( numPart, &endPtr, 10 );
		if( endPtr != (numPart +numPartLen) )
		{
			size_t		lineNo = SIZE_MAX;
			uint16_t	fileID = 0;
			LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
			LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected point, found string." );
			return;
		}
	}
	else
	{
		struct LEOArrayEntry*	convertedArray = LEOCreateArrayFromString( inString, inStringLen, inContext );
		if( !convertedArray || LEOGetArrayKeyCount( convertedArray ) != 4 )
		{
			size_t		lineNo = SIZE_MAX;
			uint16_t	fileID = 0;
			LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
			LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected point, found string." );
			return;
		}
		
		LEOArrayToPoint( convertedArray, l, t, inContext );
	}
}

/*!
	Implementation of SetValueAsString for rectangle values. If the given string
	can't be fully converted to a rectangle, this will fail with an error message
	and abort execution of the current LEOContext.
*/

void	LEOSetPointValueAsString( LEOValuePtr self, const char* inString, size_t inStringLen, struct LEOContext* inContext )
{
	LEOStringToPoint( inString, inStringLen, &self->point.horizontal, &self->point.vertical, inContext );
}


void	LEOInitPointValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	dest->base.isa = &kLeoValueTypeRect;
	if( keepReferences == kLEOInvalidateReferences )
		dest->base.refObjectID = kLEOObjectIDINVALID;
	dest->point.horizontal = self->point.horizontal;
	dest->point.vertical = self->point.vertical;
}


void	LEOPutPointValueIntoValue( LEOValuePtr self, LEOValuePtr dest, struct LEOContext* inContext )
{
	LEOSetValueAsPoint( dest, self->point.horizontal, self->point.vertical, inContext );
}


void	LEOCleanUpPointValue( LEOValuePtr self, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	self->base.isa = NULL;
	self->point.vertical = 0;
	self->point.horizontal = 0;
	if( keepReferences == kLEOInvalidateReferences && self->base.refObjectID != kLEOObjectIDINVALID )
	{
		LEOContextGroupRecycleObjectID( inContext->group, self->base.refObjectID );
		self->base.refObjectID = 0;
	}
}


LEOValuePtr		LEOGetPointValueValueForKey( LEOValuePtr self, const char* inKey, union LEOValue *tempStorage, LEOKeepReferencesFlag keepReferences, struct LEOContext * inContext )
{
	if( inContext->group->flags & kLEOContextGroupFlagHyperCardCompatibility )
		return LEOCantGetValueForKey( self, inKey, tempStorage, keepReferences, inContext );
	
	LEOInteger		theNum = 0;
	if( strcasecmp(inKey, "horizontal") == 0 )
		theNum = self->point.horizontal;
	else if( strcasecmp(inKey, "vertical") == 0 )
		theNum = self->point.vertical;
	else
		return NULL;
	LEOInitIntegerValue( tempStorage, theNum, kLEOUnitNone, keepReferences, inContext );
	return tempStorage;
}


void		LEOSetPointValueValueForKey( LEOValuePtr self, const char* inKey, LEOValuePtr inValue, struct LEOContext * inContext )
{
	if( inContext->group->flags & kLEOContextGroupFlagHyperCardCompatibility )
	{
		LEOCantSetValueForKey( self, inKey, inValue, inContext );
		return;
	}
	bool		mustConvertToArray = false;
	LEOUnit		theUnit = kLEOUnitNone;
	LEOInteger	theNum = LEOGetValueAsInteger( inValue, &theUnit, inContext );
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
	{
		if( self->base.isa == &kLeoValueTypeRectVariant )	// Variant? Convert to an array.
		{
			inContext->flags |= kLEOContextKeepRunning;
			mustConvertToArray = true;
			theNum = 0;
		}
		else
		{
			size_t		lineNo = SIZE_MAX;
			uint16_t	fileID = 0;
			LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
			LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected integer, found %s.", inValue->base.isa->displayTypeName );
			return;
		}
	}
	if( strcasecmp(inKey, "horizontal") == 0 && !mustConvertToArray )
		self->point.horizontal = theNum;
	else if( strcasecmp(inKey, "vertical") == 0 && !mustConvertToArray )
		self->point.vertical = theNum;
	else if( self->base.isa == &kLeoValueTypeRectVariant )
	{
		struct LEOArrayEntry	*	array = NULL;
		LEOAddIntegerArrayEntryToRoot( &array, "horizontal", self->point.horizontal, kLEOUnitNone, inContext );
		LEOAddIntegerArrayEntryToRoot( &array, "vertical", self->point.vertical, kLEOUnitNone, inContext );
		LEOAddArrayEntryToRoot( &array, inKey, inValue, inContext );
		
		LEOCleanUpValue( self, kLEOKeepReferences, inContext );
		LEOInitArrayVariantValue( self, array, kLEOKeepReferences, inContext );
	}
	else
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Can't set key %s on a point, must be \"horizontal\" or \"vertical\".", inKey );
		return;
	}
}


size_t		LEOGetPointValueKeyCount( LEOValuePtr self, struct LEOContext * inContext )
{
	if( inContext->group->flags & kLEOContextGroupFlagHyperCardCompatibility )
		return LEOCantGetKeyCount( self, inContext );

	return 2;
}


void	LEOSetPointValueAsPoint( LEOValuePtr self, LEOInteger l, LEOInteger t, struct LEOContext * inContext )
{
	self->point.horizontal = l;
	self->point.vertical = t;
}


void	LEOGetPointValueAsPoint( LEOValuePtr self, LEOInteger* l, LEOInteger* t, struct LEOContext * inContext )
{
	*l = self->point.horizontal;
	*t = self->point.vertical;
}


#pragma mark -
#pragma mark Range


void	LEOInitRangeValue( LEOValuePtr self, LEOInteger s, LEOInteger e, LEOChunkType t, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	self->base.isa = &kLeoValueTypeRange;
	if( keepReferences == kLEOInvalidateReferences )
		self->base.refObjectID = kLEOObjectIDINVALID;
	self->range.start = s;
	self->range.end = e;
	self->range.type = t;
}


/*!
	Implementation of GetAsString for rectangle values.
*/

const char*	LEOGetRangeValueAsString( LEOValuePtr self, char* outBuf, size_t bufSize, struct LEOContext* inContext )
{
	if( outBuf )	// Can never return as string without buffer.
	{
		if( self->range.start == self->range.end )
			self->string.stringLen = snprintf( outBuf, bufSize -1, "%s %lld", gLEOChunkTypeNames[self->range.type], self->range.start );
		else
			self->string.stringLen = snprintf( outBuf, bufSize -1, "%s %lld to %lld", gLEOChunkTypeNames[self->range.type], self->range.start, self->range.end );
	}
	return outBuf;
}


void	LEOStringToRange( const char* inString, size_t inStringLen, LEOInteger *s, LEOInteger *e, LEOChunkType *t, struct LEOContext* inContext )
{
	LEOChunkType	foundType = kLEOChunkTypeINVALID;
	size_t			currLen = 0;
	for( size_t	x = 0; gLEOChunkTypeNames[x] != NULL; x++ )
	{
		currLen = strlen(gLEOChunkTypeNames[x]);
		if( strncasecmp(inString, gLEOChunkTypeNames[x], currLen) == 0 )
		{
			while( inString[currLen] == ' ' || inString[currLen] == '\t' )
				currLen++;
			
			foundType = (LEOChunkType)x;
			break;
		}
	}
	
	*t = foundType;
	if( foundType == kLEOChunkTypeINVALID )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected range, found string." );
		return;
	}
	
	// start:
	char	numPart[OTHER_VALUE_SHORT_STRING_MAX_LENGTH] = {0};
	char*	endPtr = NULL;
	size_t	numPartLen = 0,
			x = currLen;
	for( ; x < inStringLen && inString[x] != ' ' && inString[x] != '\t' && numPartLen < (OTHER_VALUE_SHORT_STRING_MAX_LENGTH -1); x++ )
		numPart[numPartLen++] = inString[x];
	numPart[numPartLen] = '\0';
	if( numPartLen == 0 )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected range, found string." );
		return;
	}
	*s = strtoll( numPart, &endPtr, 10 );
	if( endPtr != (numPart +numPartLen) )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected range, found string." );
		return;
	}
	
	// Whitespace:
	while( x < inStringLen && (inString[x] == ' ' || inString[x] == '\t') )
		x++;
	
	// 'to'?
	bool	foundTo = false;
	if( x >= inStringLen )	// Range is just one line/character/item?
	{
		*e = *s;	// That's fine with us. Make end & start same and we're done!
		return;
	}
	
	if( tolower(inString[x]) == 't' )
		x++;
	if( x < inStringLen && tolower(inString[x]) == 'o' )
	{
		foundTo = true;
		x++;
	}
	
	if( !foundTo )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected range, found string." );
		return;
	}
	
	// Whitespace:
	while( x < inStringLen && (inString[x] == ' ' || inString[x] == '\t') )
		x++;
	
	// end:
	numPartLen = 0;
	for( ; x < inStringLen && inString[x] != ',' && numPartLen < (OTHER_VALUE_SHORT_STRING_MAX_LENGTH -1); x++ )
		numPart[numPartLen++] = inString[x];
	numPart[numPartLen] = '\0';
	if( numPartLen == 0 )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected range, found string." );
		return;
	}
	*e = strtoll( numPart, &endPtr, 10 );
	if( endPtr != (numPart +numPartLen) )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected range, found string." );
		return;
	}
	
	// Trailing whitespace:
	while( x < inStringLen && (inString[x] == ' ' || inString[x] == '\t') )
		x++;
	
	if( x != inStringLen )	// Invalid chars beyond end?
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected range, found string." );
		return;
	}
}

/*!
	Implementation of SetValueAsString for range values. If the given string
	can't be fully converted to a range, this will fail with an error message
	and abort execution of the current LEOContext.
*/

void	LEOSetRangeValueAsString( LEOValuePtr self, const char* inString, size_t inStringLen, struct LEOContext* inContext )
{
	LEOStringToRange( inString, inStringLen, &self->range.start, &self->range.end, &self->range.type, inContext );
}


void	LEOInitRangeValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	dest->base.isa = &kLeoValueTypeRange;
	if( keepReferences == kLEOInvalidateReferences )
		dest->base.refObjectID = kLEOObjectIDINVALID;
	dest->range.start = self->range.start;
	dest->range.end = self->range.end;
	dest->range.type = self->range.type;
}


void	LEOPutRangeValueIntoValue( LEOValuePtr self, LEOValuePtr dest, struct LEOContext* inContext )
{
	LEOSetValueAsRange( dest, self->range.start, self->range.end, self->range.type, inContext );
}


void	LEOCleanUpRangeValue( LEOValuePtr self, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	self->base.isa = NULL;
	self->range.start = 0;
	self->range.end = 0;
	self->range.type = 0;
	if( keepReferences == kLEOInvalidateReferences && self->base.refObjectID != kLEOObjectIDINVALID )
	{
		LEOContextGroupRecycleObjectID( inContext->group, self->base.refObjectID );
		self->base.refObjectID = 0;
	}
}


void	LEOSetRangeValueAsRange( LEOValuePtr self, LEOInteger s, LEOInteger e, LEOChunkType t, struct LEOContext * inContext )
{
	self->range.start = s;
	self->range.end = e;
	self->range.type = t;
}


void	LEOGetRangeValueAsRange( LEOValuePtr self, LEOInteger* s, LEOInteger* e, LEOChunkType* t, struct LEOContext * inContext )
{
	*s = self->range.start;
	*e = self->range.end;
	*t = self->range.type;
}


#pragma mark -
#pragma mark Native Object


void	LEOInitNativeObjectValue( LEOValuePtr self, void* inNativeObject, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	self->base.isa = &kLeoValueTypeNativeObject;
	if( keepReferences == kLEOInvalidateReferences )
		self->base.refObjectID = kLEOObjectIDINVALID;
	self->object.object = inNativeObject;
}


const char*	LEOGetNativeObjectValueAsString( LEOValuePtr self, char* outBuf, size_t bufSize, struct LEOContext* inContext )
{
	if( self->object.object != NULL )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Can't make %s into a string", self->base.isa->displayTypeName );
	}
	else if( outBuf && bufSize > 0 )	// A NIL object is the same as an "unset" value for us.
	{
		outBuf[0] = 0;
		return sUnsetConstantString;
	}
	return "";
}


void	LEOGetNativeObjectValueAsRangeOfString( LEOValuePtr self, LEOChunkType inType,
											size_t inRangeStart, size_t inRangeEnd,
											char* outBuf, size_t bufSize, struct LEOContext* inContext )
{
	if( self->object.object != NULL )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Can't make %s into a string", self->base.isa->displayTypeName );
	}
	else if( outBuf && bufSize > 0 )	// A NIL object is the same as an "unset" value for us.
	{
		outBuf[0] = 0;
	}
}



void	LEOSetNativeObjectValueAsNativeObject( LEOValuePtr self, void* inNativeObject, struct LEOContext* inContext )
{
	self->object.object = inNativeObject;
}


bool	LEOGetNativeObjectValueIsUnset( LEOValuePtr self, struct LEOContext* inContext )
{
	return self->object.object == NULL;
}


/*!
 Implementation of InitCopy for native object values.
*/

void	LEOInitNativeObjectValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	dest->base.isa = &kLeoValueTypeNativeObject;
	if( keepReferences == kLEOInvalidateReferences )
		dest->base.refObjectID = kLEOObjectIDINVALID;
	dest->object.object = self->object.object;
}


void	LEOPutNativeObjectValueIntoValue( LEOValuePtr self, LEOValuePtr dest, struct LEOContext* inContext )
{
	LEOSetValueAsNativeObject( dest, self->object.object, inContext );
}


/*!
	Destructor for native object values. If this value has references, this makes sure
	that they will produce an error message if they ever try to access it again.
*/

void	LEOCleanUpNativeObjectValue( LEOValuePtr self, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	self->base.isa = NULL;
	self->object.object = NULL;
	if( keepReferences == kLEOInvalidateReferences && self->base.refObjectID != kLEOObjectIDINVALID )
	{
		LEOContextGroupRecycleObjectID( inContext->group, self->base.refObjectID );
		self->base.refObjectID = 0;
	}
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


void	LEOInitReferenceValueWithIDs( LEOValuePtr self, LEOObjectID referencedObjectID, LEOObjectSeed referencedObjectSeed,
									  LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	self->base.isa = &kLeoValueTypeReference;
	if( keepReferences == kLEOInvalidateReferences )
		self->base.refObjectID = kLEOObjectIDINVALID;
	
	self->reference.objectID = referencedObjectID;
	self->reference.objectSeed = referencedObjectSeed;
	
	self->reference.chunkType = kLEOChunkTypeINVALID;
	self->reference.chunkStart = 0;
	self->reference.chunkEnd = 0;
}


/*!
	Implementation of GetAsString for reference values.
*/

const char*	LEOGetReferenceValueAsString( LEOValuePtr self, char* outBuf, size_t bufSize, struct LEOContext* inContext )
{
	const char*		theStr = outBuf;
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue == NULL )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "The referenced value doesn't exist anymore." );
	}
	else if( self->reference.chunkType != kLEOChunkTypeINVALID )
	{
		LEOGetValueAsRangeOfString( theValue, self->reference.chunkType, self->reference.chunkStart, self->reference.chunkEnd, outBuf, bufSize, inContext );
	}
	else
		theStr = LEOGetValueAsString( theValue, outBuf, bufSize, inContext );
	
	return theStr;
}


/*!
	Implementation of GetAsNumber for reference values.
*/

LEONumber	LEOGetReferenceValueAsNumber( LEOValuePtr self, LEOUnit *outUnit, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue == NULL )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "The referenced value doesn't exist anymore." );
		return 0.0;
	}
	else if( self->reference.chunkType != kLEOChunkTypeINVALID )
	{
		char		str[OTHER_VALUE_SHORT_STRING_MAX_LENGTH] = {0};	// Can get away with this as long as they're only numbers, booleans etc.
		LEOGetValueAsRangeOfString( theValue, self->reference.chunkType, self->reference.chunkStart, self->reference.chunkEnd, str, sizeof(str), inContext );

		// Determine if there's a unit on this number, remove it but remember it:
		LEOUnit		theUnit = kLEOUnitNone;
		
		size_t	strLen = strlen(str);
		for( int x = 1; x < kLEOUnit_Last; x++ )	// Skip first one, which is empty string for 'no unit' and would match anything.
		{
			size_t	unitLen = strlen(gUnitLabels[x]);
			if( unitLen < strLen )
			{
				if( strcasecmp( str +(strLen -unitLen), gUnitLabels[x] ) == 0 )
				{
					strLen -= unitLen;
					str[strLen] = 0;
					theUnit = x;
					break;
				}
			}
		}

		char*		endPtr = NULL;
		LEONumber	num = strtof( str, &endPtr );
		if( endPtr != (str +strLen) )
			LEOCantGetValueAsNumber( self, outUnit, inContext );
		
		if( outUnit )
			*outUnit = theUnit;
		
		return num;
	}
	else
		return LEOGetValueAsNumber( theValue, outUnit, inContext );
}


/*!
	Implementation of GetAsInteger for reference values.
*/

LEOInteger	LEOGetReferenceValueAsInteger( LEOValuePtr self, LEOUnit *outUnit, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue == NULL )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "The referenced value doesn't exist anymore." );
		return 0LL;
	}
	else if( self->reference.chunkType != kLEOChunkTypeINVALID )
	{
		char		str[OTHER_VALUE_SHORT_STRING_MAX_LENGTH] = {0};	// Can get away with this as long as they're only numbers, booleans etc.
		LEOGetValueAsRangeOfString( theValue, self->reference.chunkType, self->reference.chunkStart, self->reference.chunkEnd, str, sizeof(str), inContext );

		// Determine if there's a unit on this number, remove it but remember it:
		LEOUnit		theUnit = kLEOUnitNone;
		
		size_t	strLen = strlen(str);
		for( int x = 1; x < kLEOUnit_Last; x++ )	// Skip first one, which is empty string for 'no unit' and would match anything.
		{
			size_t	unitLen = strlen(gUnitLabels[x]);
			if( unitLen < strLen )
			{
				if( strcasecmp( str +(strLen -unitLen), gUnitLabels[x] ) == 0 )
				{
					strLen -= unitLen;
					str[strLen] = 0;
					theUnit = x;
					break;
				}
			}
		}

		char*		endPtr = NULL;
		LEOInteger	num = strtoll( str, &endPtr, 10 );
		if( endPtr != (str +strLen) )
			LEOCantGetValueAsInteger( self, outUnit, inContext );
		
		if( outUnit )
			*outUnit = theUnit;
		
		return num;
	}
	else
		return LEOGetValueAsInteger( theValue, outUnit, inContext );
}


/*!
	Implementation of GetAsBoolean for reference values.
*/

bool	LEOGetReferenceValueAsBoolean( LEOValuePtr self, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue == NULL )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "The referenced value doesn't exist anymore." );
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
	Implementation of GetAsRect for reference values.
*/

void	LEOGetReferenceValueAsRect( LEOValuePtr self, LEOInteger* l, LEOInteger* t, LEOInteger* r, LEOInteger* b, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue == NULL )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "The referenced value doesn't exist anymore." );
	}
	else if( self->reference.chunkType != kLEOChunkTypeINVALID )
	{
		char		str[OTHER_VALUE_SHORT_STRING_MAX_LENGTH] = {0};	// Can get away with this as long as they're only numbers, booleans etc.
		LEOGetValueAsRangeOfString( theValue, self->reference.chunkType, self->reference.chunkStart, self->reference.chunkEnd, str, sizeof(str), inContext );
		LEOStringToRect( str, strlen(str), l, t, r, b, inContext );
	}
	else
		LEOGetValueAsRect( theValue, l, t, r, b, inContext );
}


/*!
	Implementation of GetAsPoint for reference values.
*/

void	LEOGetReferenceValueAsPoint( LEOValuePtr self, LEOInteger* l, LEOInteger* t, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue == NULL )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "The referenced value doesn't exist anymore." );
	}
	else if( self->reference.chunkType != kLEOChunkTypeINVALID )
	{
		char		str[OTHER_VALUE_SHORT_STRING_MAX_LENGTH] = {0};	// Can get away with this as long as they're only numbers, booleans etc.
		LEOGetValueAsRangeOfString( theValue, self->reference.chunkType, self->reference.chunkStart, self->reference.chunkEnd, str, sizeof(str), inContext );
		LEOStringToPoint( str, strlen(str), l, t, inContext );
	}
	else
		LEOGetValueAsPoint( theValue, l, t, inContext );
}


/*!
	Implementation of GetAsRange for reference values.
*/

void	LEOGetReferenceValueAsRange( LEOValuePtr self, LEOInteger* s, LEOInteger* e, LEOChunkType *t, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue == NULL )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "The referenced value doesn't exist anymore." );
	}
	else if( self->reference.chunkType != kLEOChunkTypeINVALID )
	{
		char		str[OTHER_VALUE_SHORT_STRING_MAX_LENGTH] = {0};	// Can get away with this as long as they're only numbers, booleans etc.
		LEOGetValueAsRangeOfString( theValue, self->reference.chunkType, self->reference.chunkStart, self->reference.chunkEnd, str, sizeof(str), inContext );
		LEOStringToRange( str, strlen(str), s, e, t, inContext );
	}
	else
		LEOGetValueAsRange( theValue, s, e, t, inContext );
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
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "The referenced value doesn't exist anymore." );
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

void	LEOSetReferenceValueAsString( LEOValuePtr self, const char* inString, size_t inStringLen, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue == NULL )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "The referenced value doesn't exist anymore." );
	}
	else if( self->reference.chunkType != kLEOChunkTypeINVALID )
	{
		// TODO: Make NUL-safe.
		size_t		chunkStart = 0, chunkEnd = SIZE_MAX, chunkDelStart = 0, chunkDelEnd = 0;
		LEODetermineChunkRangeOfSubstring( theValue, &chunkStart, &chunkEnd, &chunkDelStart, &chunkDelEnd,
											self->reference.chunkType, self->reference.chunkStart, self->reference.chunkEnd, inContext );
		LEOSetValuePredeterminedRangeAsString( theValue, (inString ? chunkStart : chunkDelStart), (inString ? chunkEnd : chunkDelEnd), inString, inContext );
	}
	else
		LEOSetValueAsString( theValue, inString, inStringLen, inContext );
}


/*!
	Implementation of SetAsBoolean for reference values.
*/

void	LEOSetReferenceValueAsBoolean( LEOValuePtr self, bool inBoolean, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue == NULL )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "The referenced value doesn't exist anymore." );
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
	Implementation of SetAsRect for reference values.
*/

void	LEOSetReferenceValueAsRect( LEOValuePtr self, LEOInteger l, LEOInteger t, LEOInteger r, LEOInteger b, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue == NULL )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "The referenced value doesn't exist anymore." );
	}
	else if( self->reference.chunkType != kLEOChunkTypeINVALID )
	{
		char	strBuf[OTHER_VALUE_SHORT_STRING_MAX_LENGTH] = {0};
		if( inContext->group->flags == kLEOContextGroupFlagHyperCardCompatibility )
			snprintf( strBuf, OTHER_VALUE_SHORT_STRING_MAX_LENGTH -1, "%lld,%lld,%lld,%lld", l, t, r, b );
		else
			snprintf( strBuf, OTHER_VALUE_SHORT_STRING_MAX_LENGTH -1, "left:%lld\ntop:%lld\nright:%lld\nbottom:%lld", l, t, r, b );
		LEOSetValueRangeAsString( theValue, self->reference.chunkType, self->reference.chunkStart, self->reference.chunkEnd,
									strBuf, inContext );
	}
	else
		LEOSetValueAsRect( theValue, l, t, r, b, inContext );
}


/*!
	Implementation of SetAsRect for reference values.
*/

void	LEOSetReferenceValueAsPoint( LEOValuePtr self, LEOInteger l, LEOInteger t, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue == NULL )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "The referenced value doesn't exist anymore." );
	}
	else if( self->reference.chunkType != kLEOChunkTypeINVALID )
	{
		char	strBuf[OTHER_VALUE_SHORT_STRING_MAX_LENGTH] = {0};
		if( inContext->group->flags == kLEOContextGroupFlagHyperCardCompatibility )
			snprintf( strBuf, OTHER_VALUE_SHORT_STRING_MAX_LENGTH -1, "%lld,%lld", l, t );
		else
			snprintf( strBuf, OTHER_VALUE_SHORT_STRING_MAX_LENGTH -1, "horizontal:%lld\nvertical:%lld", l, t );
		LEOSetValueRangeAsString( theValue, self->reference.chunkType, self->reference.chunkStart, self->reference.chunkEnd,
									strBuf, inContext );
	}
	else
		LEOSetValueAsPoint( theValue, l, t, inContext );
}


/*!
	Implementation of SetAsRange for reference values.
*/

void	LEOSetReferenceValueAsRange( LEOValuePtr self, LEOInteger s, LEOInteger e, LEOChunkType t, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue == NULL )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "The referenced value doesn't exist anymore." );
	}
	else if( self->reference.chunkType != kLEOChunkTypeINVALID )
	{
		char	strBuf[OTHER_VALUE_SHORT_STRING_MAX_LENGTH] = {0};
		if( s == e )
			self->string.stringLen = snprintf( strBuf, OTHER_VALUE_SHORT_STRING_MAX_LENGTH -1, "%s %lld", gLEOChunkTypeNames[t], s );
		else
			self->string.stringLen = snprintf( strBuf, OTHER_VALUE_SHORT_STRING_MAX_LENGTH -1, "%s %lld to %lld", gLEOChunkTypeNames[t], s, e );
		LEOSetValueRangeAsString( theValue, self->reference.chunkType, self->reference.chunkStart, self->reference.chunkEnd,
									strBuf, inContext );
	}
	else
		LEOSetValueAsRange( theValue, s, e, t, inContext );
}


/*!
	Implementation of SetAsNativeObject for reference values.
*/

void	LEOSetReferenceValueAsNativeObject( LEOValuePtr self, void* inNativeObject, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue == NULL )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "The referenced value doesn't exist anymore." );
	}
	else if( self->reference.chunkType != kLEOChunkTypeINVALID )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Can't combine chunk expressions and native strings." );
	}
	else
		LEOSetValueAsNativeObject( theValue, inNativeObject, inContext );
}


/*!
	Implementation of SetAsNumber for reference values.
*/

void	LEOSetReferenceValueAsNumber( LEOValuePtr self, LEONumber inNumber, LEOUnit inUnit, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue == NULL )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "The referenced value doesn't exist anymore." );
	}
	else if( self->reference.chunkType != kLEOChunkTypeINVALID )
	{
		char		str[OTHER_VALUE_SHORT_STRING_MAX_LENGTH] = {0};	// Can get away with this as long as they're only numbers, booleans etc.
		snprintf( str, sizeof(str), "%g%s", inNumber, gUnitLabels[inUnit] );
		LEOSetValueRangeAsString( theValue, self->reference.chunkType, self->reference.chunkStart, self->reference.chunkEnd,
									str, inContext );
	}
	else
		LEOSetValueAsNumber( theValue, inNumber, inUnit, inContext );
}


/*!
	Implementation of SetAsInteger for reference values.
*/

void	LEOSetReferenceValueAsInteger( LEOValuePtr self, LEOInteger inInteger, LEOUnit inUnit, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue == NULL )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "The referenced value doesn't exist anymore." );
	}
	else if( self->reference.chunkType != kLEOChunkTypeINVALID )
	{
		char		str[OTHER_VALUE_SHORT_STRING_MAX_LENGTH] = {0};	// Can get away with this as long as they're only numbers, booleans etc.
		snprintf( str, sizeof(str), "%lld%s", inInteger, gUnitLabels[inUnit] );
		LEOSetValueRangeAsString( theValue, self->reference.chunkType, self->reference.chunkStart, self->reference.chunkEnd,
									str, inContext );
	}
	else
		LEOSetValueAsInteger( theValue, inInteger, inUnit, inContext );
}


/*!
	Implementation of SetAsArray for reference values.
*/

void		LEOSetReferenceValueAsArray( LEOValuePtr self, struct LEOArrayEntry * inArray, struct LEOContext * inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue == NULL )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "The referenced value doesn't exist anymore." );
	}
	else if( self->reference.chunkType != kLEOChunkTypeINVALID )
	{
		char		str[1024] = {0};	// TODO: Make this work for all lengths.
		LEOPrintArray( inArray, str, sizeof(str), inContext );
		LEOSetValueRangeAsString( theValue, self->reference.chunkType, self->reference.chunkStart, self->reference.chunkEnd, str, inContext );
	}
	else
		LEOSetValueAsArray( theValue, inArray, inContext );
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
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "The referenced value doesn't exist anymore." );
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
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "The referenced value doesn't exist anymore." );
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
	if( theValue == self )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Internal error: A value is referencing itself." );
	}
	if( theValue == NULL )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "The referenced value doesn't exist anymore." );
	}
	else
		LEOInitSimpleCopy(theValue, dest, keepReferences, inContext);
}


void	LEOPutReferenceValueIntoValue( LEOValuePtr self, LEOValuePtr dest, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue == NULL )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "The referenced value doesn't exist anymore." );
	}
	else
		LEOPutValueIntoValue(theValue, dest, inContext);
}


void	LEODetermineChunkRangeOfSubstringOfReferenceValue( LEOValuePtr self, size_t *ioBytesStart, size_t *ioBytesEnd,
															size_t *ioBytesDelStart, size_t *ioBytesDelEnd,
															LEOChunkType inType, size_t inRangeStart, size_t inRangeEnd,
															struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue == NULL )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "The referenced value doesn't exist anymore." );
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


bool	LEOCanGetReferenceValueAsInteger( LEOValuePtr self, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue != NULL )
		return LEOCanGetAsInteger( theValue, inContext );
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
	{
		LEOContextGroupRecycleObjectID( inContext->group, self->base.refObjectID );
		self->base.refObjectID = 0;
	}
}


LEOValuePtr		LEOGetReferenceValueValueForKey( LEOValuePtr self, const char* inKey, union LEOValue *tempStorage, LEOKeepReferencesFlag keepReferences, struct LEOContext * inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue == NULL )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "The referenced value doesn't exist anymore." );
		
		return NULL;
	}
	else
		return LEOGetValueForKey( theValue, inKey, tempStorage, keepReferences, inContext );
}


void		LEOSetReferenceValueValueForKey( LEOValuePtr self, const char* inKey, LEOValuePtr inValue, struct LEOContext * inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue == NULL )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "The referenced value doesn't exist anymore." );
	}
	else
		LEOSetValueForKey( theValue, inKey, inValue, inContext );
}


size_t		LEOGetReferenceValueKeyCount( LEOValuePtr self, struct LEOContext * inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue == NULL )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "The referenced value doesn't exist anymore." );
		
		return 0;
	}
	else
		return LEOGetKeyCount( theValue, inContext );
}


void		LEOGetReferenceValueForKeyOfRange( LEOValuePtr self, const char* keyName, size_t startOffset, size_t endOffset, LEOValuePtr outValue, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue == NULL )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "The referenced value doesn't exist anymore." );
	}
	else
		LEOGetValueForKeyOfRange( theValue, keyName, startOffset, endOffset, outValue, inContext );
}


void		LEOSetReferenceValueForKeyOfRange( LEOValuePtr self, const char* keyName, LEOValuePtr inValue, size_t startOffset, size_t endOffset, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue == NULL )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "The referenced value doesn't exist anymore." );
	}
	else
		LEOSetValueForKeyOfRange( theValue, keyName, inValue, startOffset, endOffset, inContext );
}


LEOValuePtr	LEOReferenceValueFollowReferencesAndReturnValueOfType( LEOValuePtr self, LEOValueTypePtr inType, struct LEOContext* inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( self->base.isa == inType )
		return self;
	else if( theValue == NULL )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "The referenced value doesn't exist anymore." );
		
		return NULL;
	}
	else if( theValue->base.isa == inType )
		return theValue;
	else if( self->reference.chunkType != kLEOChunkTypeINVALID )
		return NULL;
	else
		return LEOFollowReferencesAndReturnValueOfType( theValue, inType, inContext );
}


bool	LEOGetReferenceValueIsUnset( LEOValuePtr self, struct LEOContext * inContext )
{
	LEOValuePtr		theValue = LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, self->reference.objectID, self->reference.objectSeed );
	if( theValue == NULL )
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "The referenced value doesn't exist anymore." );
		
		return 0;
	}
	else
		return LEOGetValueIsUnset( theValue, inContext );
}


#pragma mark -
#pragma mark Variants


void	LEOInitNumberVariantValue( LEOValuePtr self, LEONumber inNumber, LEOUnit inUnit, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	LEOInitNumberValue( self, inNumber, inUnit, keepReferences, inContext );
	self->base.isa = &kLeoValueTypeNumberVariant;
}


void	LEOInitIntegerVariantValue( LEOValuePtr self, LEOInteger inInteger, LEOUnit inUnit, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	LEOInitIntegerValue( self, inInteger, inUnit, keepReferences, inContext );
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


void	LEOInitNativeObjectVariantValue( LEOValuePtr self, void* inNativeObject, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	LEOInitNativeObjectValue( self, inNativeObject, keepReferences, inContext );
	self->base.isa = &kLeoValueTypeNativeObjectVariant;
}


void	LEOInitArrayVariantValue( LEOValuePtr self, struct LEOArrayEntry* array, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	LEOInitArrayValue( &self->array, array, keepReferences, inContext );
	self->base.isa = &kLeoValueTypeArrayVariant;
}


void	LEOSetVariantValueAsNumber( LEOValuePtr self, LEONumber inNumber, LEOUnit inUnit, struct LEOContext* inContext )
{
	LEOCleanUpValue( self, kLEOKeepReferences, inContext );
	LEOInitNumberValue( self, inNumber, inUnit, kLEOKeepReferences, inContext );
	self->base.isa = &kLeoValueTypeNumberVariant;
}


void	LEOSetVariantValueAsInteger( LEOValuePtr self, LEOInteger inInteger, LEOUnit inUnit, struct LEOContext* inContext )
{
	LEOCleanUpValue( self, kLEOKeepReferences, inContext );
	LEOInitIntegerValue( self, inInteger, inUnit, kLEOKeepReferences, inContext );
	self->base.isa = &kLeoValueTypeIntegerVariant;
}


void	LEOSetVariantValueAsString( LEOValuePtr self, const char* inString, size_t inStringLen, struct LEOContext* inContext )
{
	LEOCleanUpValue( self, kLEOKeepReferences, inContext );
	LEOInitStringValue( self, (inString ? inString : ""), (inString ? inStringLen : 0), kLEOKeepReferences, inContext );
	self->base.isa = &kLeoValueTypeStringVariant;
}


void	LEOSetVariantValueAsBoolean( LEOValuePtr self, bool inBoolean, struct LEOContext* inContext )
{
	LEOCleanUpValue( self, kLEOKeepReferences, inContext );
	LEOInitBooleanValue( self, inBoolean, kLEOKeepReferences, inContext );
	self->base.isa = &kLeoValueTypeBooleanVariant;
}


void	LEOSetVariantValueAsRect( LEOValuePtr self, LEOInteger l, LEOInteger t, LEOInteger r, LEOInteger b, struct LEOContext* inContext )
{
	LEOCleanUpValue( self, kLEOKeepReferences, inContext );
	LEOInitRectValue( self, l, t, r, b, kLEOKeepReferences, inContext );
	self->base.isa = &kLeoValueTypeRectVariant;
}


void	LEOSetVariantValueAsPoint( LEOValuePtr self, LEOInteger l, LEOInteger t, struct LEOContext* inContext )
{
	LEOCleanUpValue( self, kLEOKeepReferences, inContext );
	LEOInitPointValue( self, l, t, kLEOKeepReferences, inContext );
	self->base.isa = &kLeoValueTypePointVariant;
}


void	LEOSetVariantValueAsRange( LEOValuePtr self, LEOInteger s, LEOInteger e, LEOChunkType t, struct LEOContext* inContext )
{
	LEOCleanUpValue( self, kLEOKeepReferences, inContext );
	LEOInitRangeValue( self, s, e, t, kLEOKeepReferences, inContext );
	self->base.isa = &kLeoValueTypeRangeVariant;
}


void	LEOSetVariantValueAsNativeObject( LEOValuePtr self, void* inNativeObject, struct LEOContext* inContext )
{
	LEOCleanUpValue( self, kLEOKeepReferences, inContext );
	LEOInitNativeObjectValue( self, inNativeObject, kLEOKeepReferences, inContext );
	self->base.isa = &kLeoValueTypeNativeObjectVariant;
}


void	LEOSetVariantValueAsArray( LEOValuePtr self, struct LEOArrayEntry *inArray, struct LEOContext* inContext )
{
	LEOCleanUpValue( self, kLEOKeepReferences, inContext );
	LEOInitArrayValue( &self->array, NULL, kLEOKeepReferences, inContext );
	self->array.array = LEOCopyArray( inArray, inContext );
	self->base.isa = &kLeoValueTypeArrayVariant;
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


LEOValuePtr	LEOGetStringVariantValueForKey( LEOValuePtr self, const char* keyName, union LEOValue *tempStorage, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	struct LEOArrayEntry	*	convertedArray = NULL;
	if( self->string.string != NULL && self->string.stringLen != 0 )
	{
		convertedArray = LEOCreateArrayFromString( self->string.string, self->string.stringLen, inContext );
		if( !convertedArray )
		{
			size_t		lineNo = SIZE_MAX;
			uint16_t	fileID = 0;
			LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
			LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected array, found %s", self->base.isa->displayTypeName );
			return NULL;
		}
	}
	else
	{
		size_t		lineNo = SIZE_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected array, found %s", self->base.isa->displayTypeName );
		return NULL;
	}
	
	// Transform us into an array:
	LEOCleanUpValue( self, kLEOKeepReferences, inContext );
	LEOInitArrayValue( &self->array, NULL, kLEOKeepReferences, inContext );
	self->array.array = convertedArray;
	self->base.isa = &kLeoValueTypeArrayVariant;
	
	LEOValuePtr		foundValue = LEOGetArrayValueForKey( self->array.array, keyName );
	if( foundValue == NULL )
		return NULL;
	
	LEOInitReferenceValue( tempStorage, foundValue, keepReferences, kLEOChunkTypeINVALID, 0, 0, inContext );
	
	return tempStorage;
}



void	LEOSetStringVariantValueValueForKey( LEOValuePtr self, const char* inKey, LEOValuePtr inValue, struct LEOContext * inContext )
{
	if( self->string.stringLen != 0 )	// Not an empty string
	{
		struct LEOArrayEntry	*	convertedArray = LEOCreateArrayFromString( self->string.string, self->string.stringLen, inContext );
		if( !convertedArray )
		{
			size_t		lineNo = SIZE_MAX;
			uint16_t	fileID = 0;
			LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
			LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Expected array here, found \"%s\".", self->string.string );
			return;
		}
		
		LEOCleanUpValue( self, kLEOKeepReferences, inContext );
		LEOInitArrayValue( &self->array, convertedArray, kLEOKeepReferences, inContext );
	}
	else
	{
		LEOCleanUpValue( self, kLEOKeepReferences, inContext );
		LEOInitArrayValue( &self->array, NULL, kLEOKeepReferences, inContext );
	}
	self->base.isa = &kLeoValueTypeArrayVariant;
	LEOAddArrayEntryToRoot( &self->array.array, inKey, inValue, inContext );
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


void	LEOInitRectVariantValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	LEOInitRectValueCopy( self, dest, keepReferences, inContext );
	dest->base.isa = &kLeoValueTypeRectVariant;
}


void	LEOInitPointVariantValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	LEOInitPointValueCopy( self, dest, keepReferences, inContext );
	dest->base.isa = &kLeoValueTypePointVariant;
}


void	LEOInitRangeVariantValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	LEOInitRangeValueCopy( self, dest, keepReferences, inContext );
	dest->base.isa = &kLeoValueTypeRangeVariant;
}


void	LEOInitNativeObjectVariantValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	LEOInitNativeObjectValueCopy( self, dest, keepReferences, inContext );
	dest->base.isa = &kLeoValueTypeNativeObjectVariant;
}


void	LEOInitStringVariantValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	LEOInitStringValueCopy( self, dest, keepReferences, inContext );
	dest->base.isa = &kLeoValueTypeStringVariant;
}


void	LEOInitArrayVariantValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	LEOInitArrayValueCopy( self, dest, keepReferences, inContext );
	dest->base.isa = &kLeoValueTypeArrayVariant;
}

#pragma mark -
#pragma mark Arrays

void	LEOInitArrayValue( struct LEOValueArray* self, struct LEOArrayEntry *inArray, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	self->base.isa = &kLeoValueTypeArray;
	if( keepReferences == kLEOInvalidateReferences )
		self->base.refObjectID = kLEOObjectIDINVALID;
	self->array = inArray;	// *** takes over ownership.
}


const char*	LEOGetArrayValueAsString( LEOValuePtr self, char* outBuf, size_t bufSize, struct LEOContext* inContext )
{
	if( !outBuf )
		return NULL;
	
	LEOPrintArray( self->array.array, outBuf, bufSize, inContext );
	size_t	lastCh = strlen(outBuf)-1;
	if( outBuf[lastCh] == '\n' )	// Remove trailing return, if there is one.
		outBuf[lastCh] = 0;
	return outBuf;
}


void	LEOGetArrayValueAsRangeOfString( LEOValuePtr self, LEOChunkType inType,
									size_t inRangeStart, size_t inRangeEnd,
									char* outBuf, size_t bufSize, struct LEOContext* inContext )
{
	size_t		lineNo = SIZE_MAX;
	uint16_t	fileID = 0;
	LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
	LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Can't make %s into a string", self->base.isa->displayTypeName );
}


void	LEODetermineChunkRangeOfSubstringOfArrayValue( LEOValuePtr self, size_t *ioBytesStart, size_t *ioBytesEnd,
														size_t *ioBytesDelStart, size_t *ioBytesDelEnd,
														LEOChunkType inType, size_t inRangeStart, size_t inRangeEnd,
														struct LEOContext* inContext )
{
	size_t		lineNo = SIZE_MAX;
	uint16_t	fileID = 0;
	LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
	LEOContextStopWithError( inContext, lineNo, SIZE_MAX, fileID, "Can't make %s into a string", self->base.isa->displayTypeName );
}


void	LEOInitArrayValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	dest->base.isa = &kLeoValueTypeArray;
	if( keepReferences == kLEOInvalidateReferences )
		dest->base.refObjectID = kLEOObjectIDINVALID;
	dest->array.array = LEOCopyArray( self->array.array, inContext );
}


void	LEOPutArrayValueIntoValue( LEOValuePtr self, LEOValuePtr dest, struct LEOContext* inContext )
{
	LEOSetValueAsArray( dest, self->array.array, inContext );
}


void	LEOCleanUpArrayValue( LEOValuePtr self, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	self->base.isa = NULL;
	LEOCleanUpArray( self->array.array, inContext );
	self->array.array = NULL;
	if( keepReferences == kLEOInvalidateReferences && self->base.refObjectID != kLEOObjectIDINVALID )	// We have references? Make sure they all notice we've gone if they try to access us from now on.
	{
		LEOContextGroupRecycleObjectID( inContext->group, self->base.refObjectID );
		self->base.refObjectID = 0;
	}
}


LEOValuePtr		LEOGetArrayValueValueForKey( LEOValuePtr self, const char* inKey, union LEOValue *tempStorage, LEOKeepReferencesFlag keepReferences, struct LEOContext * inContext )
{
	LEOValuePtr	foundValue = LEOGetArrayValueForKey( self->array.array, inKey );
	if( foundValue )
	{
		LEOInitReferenceValue( tempStorage, foundValue, keepReferences, kLEOChunkTypeINVALID, 0, 0, inContext );
		return tempStorage;
	}
	
	return NULL;
}


void	LEOSetArrayValueValueForKey( LEOValuePtr self, const char* inKey, LEOValuePtr inValue, struct LEOContext * inContext )
{
	LEOAddArrayEntryToRoot( &self->array.array, inKey, inValue, inContext );
}


size_t	LEOGetArrayValueKeyCount( LEOValuePtr self, struct LEOContext* inContext )
{
	return LEOGetArrayKeyCount( self->array.array );
}


void	LEOSetArrayValueAsArray( LEOValuePtr self, struct LEOArrayEntry *inArray, struct LEOContext* inContext )
{
	LEOCleanUpArray( self->array.array, inContext );
	self->array.array = LEOCopyArray( inArray, inContext );
}


void	LEOSetArrayValueAsRect( LEOValuePtr self, LEOInteger l, LEOInteger t, LEOInteger r, LEOInteger b, struct LEOContext* inContext )
{
	if( inContext->group->flags & kLEOContextGroupFlagHyperCardCompatibility )
	{
		LEOCantSetValueAsRect( self, l, t, r, b, inContext );
		return;
	}
	
	LEOCleanUpArray( self->array.array, inContext );
	self->array.array = NULL;
	LEOAddIntegerArrayEntryToRoot( &self->array.array, "left", l, kLEOUnitNone, inContext );
	LEOAddIntegerArrayEntryToRoot( &self->array.array, "top", t, kLEOUnitNone, inContext );
	LEOAddIntegerArrayEntryToRoot( &self->array.array, "right", r, kLEOUnitNone, inContext );
	LEOAddIntegerArrayEntryToRoot( &self->array.array, "bottom", b, kLEOUnitNone, inContext );
}


void	LEOGetArrayValueAsRect( LEOValuePtr self, LEOInteger *l, LEOInteger *t, LEOInteger *r, LEOInteger *b, struct LEOContext* inContext )
{
	if( inContext->group->flags & kLEOContextGroupFlagHyperCardCompatibility )
	{
		LEOCantGetValueAsRect( self, l, t, r, b, inContext );
		return;
	}
	
	LEOArrayToRect( self->array.array, l, t, r, b, inContext );
}


void	LEOSetArrayValueAsPoint( LEOValuePtr self, LEOInteger l, LEOInteger t, struct LEOContext* inContext )
{
	if( inContext->group->flags & kLEOContextGroupFlagHyperCardCompatibility )
	{
		LEOCantSetValueAsPoint( self, l, t, inContext );
		return;
	}
	
	LEOCleanUpArray( self->array.array, inContext );
	self->array.array = NULL;
	LEOAddIntegerArrayEntryToRoot( &self->array.array, "horizontal", l, kLEOUnitNone, inContext );
	LEOAddIntegerArrayEntryToRoot( &self->array.array, "vertical", t, kLEOUnitNone, inContext );
}


void	LEOGetArrayValueAsPoint( LEOValuePtr self, LEOInteger *l, LEOInteger *t, struct LEOContext* inContext )
{
	if( inContext->group->flags & kLEOContextGroupFlagHyperCardCompatibility )
	{
		LEOCantGetValueAsPoint( self, l, t, inContext );
		return;
	}
	
	LEOArrayToPoint( self->array.array, l, t, inContext );
}


#pragma mark -


struct LEOArrayEntry	*	LEOCreateArrayFromString( const char* inString, size_t inStringLen, struct LEOContext* inContext )
{
	struct LEOArrayEntry*	theArray = NULL;
	size_t					x = 0, keyStartOffs = 0, keyEndOffs = 0,
							valueStartOffs = 0, valueEndOffs = 0;
	bool					isInKey = true;
	char					keyStr[LEO_MAX_ARRAY_KEY_SIZE] = { 0 };
	
	for( ; x < inStringLen; x++ )
	{
		if( isInKey && inString[x] == ':' )
		{
			keyEndOffs = x;
			valueStartOffs = valueEndOffs = x+1;
			isInKey = false;
		}
		else if( !isInKey && (inString[x] == '\n' || inString[x] == '\r') )
		{
			if( x <= 1 || inString[x-2] != ((char)0xc2) || inString[x-1] != ((char)0xac) )	// Is a real return end-of-entry, not an escaped return in data?
			{
				valueEndOffs = x;	// No +1, the return at the end is a delimiter that should be removed.
				size_t	keyLen = keyEndOffs -keyStartOffs;
				if( keyLen >= LEO_MAX_ARRAY_KEY_SIZE )
					keyLen = LEO_MAX_ARRAY_KEY_SIZE -1;
				if( keyLen > 0 )
					memmove( keyStr, inString +keyStartOffs, keyLen );
				else	// Error, not a valid array!
				{
					LEOCleanUpArray( theArray, inContext );
					return NULL;
				}
				keyStr[keyLen] = 0;
				
				LEOValuePtr	newValue = LEOAddArrayEntryToRoot( &theArray, keyStr, NULL, inContext );
				LEOInitStringValue( newValue, inString +valueStartOffs, valueEndOffs -valueStartOffs, kLEOInvalidateReferences, inContext );
				
				isInKey = true;
				keyEndOffs = keyStartOffs = valueEndOffs = valueStartOffs = x +1;
			}
		}
		else if( !isInKey )
			valueEndOffs = x +1;
		else if( isInKey )
			keyEndOffs = x+1;
	}
	
	if( !isInKey && (keyEndOffs > keyStartOffs || valueEndOffs > valueStartOffs) )	// No return at end, we have one leftover key-value pair?
	{
		size_t	keyLen = keyEndOffs -keyStartOffs;
		if( keyLen >= LEO_MAX_ARRAY_KEY_SIZE )
			keyLen = LEO_MAX_ARRAY_KEY_SIZE -1;
		if( keyLen > 0 )
			memmove( keyStr, inString +keyStartOffs, keyLen );
		else	// Error, not a valid array!
		{
			LEOCleanUpArray( theArray, inContext );
			return NULL;
		}
		keyStr[keyLen] = 0;
		
		LEOValuePtr	newValue = LEOAddArrayEntryToRoot( &theArray, keyStr, NULL, inContext );
		LEOInitStringValue( newValue, inString +valueStartOffs, valueEndOffs -valueStartOffs, kLEOInvalidateReferences, inContext );
	}
	
	return theArray;
}


struct LEOArrayEntry	*	LEOAllocNewEntry( const char* inKey, LEOValuePtr inValue, struct LEOContext* inContext )
{
	struct LEOArrayEntry	*	newEntry = NULL;
	size_t						inKeyLen = strlen(inKey);
	newEntry = calloc( sizeof(struct LEOArrayEntry) +inKeyLen, 1 ); // String's NUL byte is already size of the array in the struct.
	memmove( newEntry->key, inKey, inKeyLen +1 );
	if( inValue )
		LEOInitCopy( inValue, &newEntry->value, kLEOInvalidateReferences, inContext );
	newEntry->smallerItem = NULL;
	newEntry->largerItem = NULL;
	
	return newEntry;
}


LEOValuePtr	LEOAddIntegerArrayEntryToRoot( struct LEOArrayEntry** arrayPtrByReference, const char* inKey, LEOInteger inNum, LEOUnit inUnit, struct LEOContext* inContext )
{
	LEOValuePtr	outValue = LEOAddArrayEntryToRoot( arrayPtrByReference, inKey, NULL, inContext );
	LEOInitIntegerValue( outValue, inNum, inUnit, kLEOInvalidateReferences, inContext );
	return outValue;
}


LEOValuePtr	LEOAddNumberArrayEntryToRoot( struct LEOArrayEntry** arrayPtrByReference, const char* inKey, LEONumber inNum, LEOUnit inUnit, struct LEOContext* inContext )
{
	LEOValuePtr	outValue = LEOAddArrayEntryToRoot( arrayPtrByReference, inKey, NULL, inContext );
	LEOInitNumberValue( outValue, inNum, inUnit, kLEOInvalidateReferences, inContext );
	return outValue;
}


LEOValuePtr	LEOAddCStringArrayEntryToRoot( struct LEOArrayEntry** arrayPtrByReference, const char* inKey, const char* inCStr, struct LEOContext* inContext )
{
	LEOValuePtr	outValue = LEOAddArrayEntryToRoot( arrayPtrByReference, inKey, NULL, inContext );
	LEOInitStringValue( outValue, inCStr, strlen(inCStr), kLEOInvalidateReferences, inContext );
	return outValue;
}


LEOValuePtr	LEOAddStringConstantArrayEntryToRoot( struct LEOArrayEntry** arrayPtrByReference, const char* inKey, const char* inCStr, struct LEOContext* inContext )
{
	LEOValuePtr	outValue = LEOAddArrayEntryToRoot( arrayPtrByReference, inKey, NULL, inContext );
	LEOInitStringConstantValue( outValue, inCStr, kLEOInvalidateReferences, inContext );
	return outValue;
}


LEOValuePtr	LEOAddStringArrayEntryToRoot( struct LEOArrayEntry** arrayPtrByReference, const char* inKey, const char* inStr, size_t inLen, struct LEOContext* inContext )
{
	LEOValuePtr	outValue = LEOAddArrayEntryToRoot( arrayPtrByReference, inKey, NULL, inContext );
	LEOInitStringValue( outValue, inStr, inLen, kLEOInvalidateReferences, inContext );
	return outValue;
}


LEOValuePtr	LEOAddRectArrayEntryToRoot( struct LEOArrayEntry** arrayPtrByReference, const char* inKey, LEOInteger l, LEOInteger t, LEOInteger r, LEOInteger b, struct LEOContext* inContext )
{
	LEOValuePtr	outValue = LEOAddArrayEntryToRoot( arrayPtrByReference, inKey, NULL, inContext );
	LEOInitRectValue( outValue, l, t, r, b, kLEOInvalidateReferences, inContext );
	return outValue;
}


LEOValuePtr	LEOAddPointArrayEntryToRoot( struct LEOArrayEntry** arrayPtrByReference, const char* inKey, LEOInteger l, LEOInteger t, struct LEOContext* inContext )
{
	LEOValuePtr	outValue = LEOAddArrayEntryToRoot( arrayPtrByReference, inKey, NULL, inContext );
	LEOInitPointValue( outValue, l, t, kLEOInvalidateReferences, inContext );
	return outValue;
}


LEOValuePtr	LEOAddArrayArrayEntryToRoot( struct LEOArrayEntry** arrayPtrByReference, const char* inKey, LEOValueArray* inArray, struct LEOContext* inContext )
{
	LEOValuePtr	outValue = LEOAddArrayEntryToRoot( arrayPtrByReference, inKey, (LEOValuePtr)inArray, inContext );
	if( !inArray )
	{
		LEOInitArrayValue( &outValue->array, NULL, kLEOInvalidateReferences, inContext );
	}
	return outValue;
}


LEOValuePtr	LEOAddArrayEntryToRoot( struct LEOArrayEntry** arrayPtrByReference, const char* inKey, LEOValuePtr inValue, struct LEOContext* inContext )
{
	struct LEOArrayEntry	*	currEntry = NULL;
	
	if( *arrayPtrByReference == NULL )
	{
		*arrayPtrByReference = LEOAllocNewEntry( inKey, inValue, inContext );
		return &(*arrayPtrByReference)->value;
	}
	else
	{
		currEntry = *arrayPtrByReference;
		while( true )
		{
			int			cmpResult = strcasecmp( currEntry->key, inKey );
			if( cmpResult > 0 )	// inKey is larger? Go down 'larger' side one step.
			{
				if( currEntry->largerItem == NULL )
				{
					currEntry->largerItem = LEOAllocNewEntry( inKey, inValue, inContext );
					return &currEntry->largerItem->value;
				}
				else
					currEntry = currEntry->largerItem;
			}
			else if( cmpResult < 0 )	// inKey is smaller? Go down 'smaller' side one step.
			{
				if( currEntry->smallerItem == NULL )
				{
					currEntry->smallerItem = LEOAllocNewEntry( inKey, inValue, inContext );
					return &currEntry->smallerItem->value;
				}
				else
					currEntry = currEntry->smallerItem;
			}
			else if( cmpResult == 0 )	// Key already exists? Replace value!
			{
				LEOCleanUpValue( &currEntry->value, kLEOKeepReferences, inContext );
				if( inValue )
					LEOInitCopy( inValue, &currEntry->value, kLEOKeepReferences, inContext );
				return &currEntry->value;
			}
		}
	}
	
	return NULL;
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
	if( !arrayPtr )
		return NULL;
	
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


void	LEOPrintArray( struct LEOArrayEntry* arrayPtr, char* strBuf, size_t bufSize, struct LEOContext* inContext )
{
	if( bufSize <= 1 )
		return;
	
	strBuf[0] = 0;
	if( arrayPtr == NULL )
		return;
	
	char valBuf[1024];
	
	const char* valStr = LEOGetValueAsString( &arrayPtr->value, valBuf, sizeof(valBuf), inContext );
	if( (inContext->flags & kLEOContextKeepRunning) == 0 )
		return;
	
	size_t	offs = snprintf( strBuf, bufSize, "%s:", arrayPtr->key );
	for( int x = 0; true; x++ )
	{
		if( (bufSize -offs) == 0 )
		{
			strBuf[offs] = 0;
			break;
		}
		if( valStr[x] == '\n' || valStr[x] == '\r' )	// Replace with "¬\n" resp. "¬\r" (2-byte char + line feed).
		{
			strBuf[offs++] = 0xc2;
			if( (bufSize -offs) == 0 )
			{
				strBuf[offs -1] = 0;
				break;
			}
			strBuf[offs++] = 0xac;
			if( (bufSize -offs) == 0 )
			{
				strBuf[offs -2] = 0;
				break;
			}
			strBuf[offs++] = valStr[x];
		}
		else if( valStr[x] == (char)0xc2 )
		{
			if( valStr[x+1] == (char)0xac )
			{
				x++;
				
				strBuf[offs++] = 0xc2;
				if( (bufSize -offs) == 0 )
				{
					strBuf[offs -1] = 0;
					break;
				}
				strBuf[offs++] = 0xac;
				if( (bufSize -offs) == 0 )
				{
					strBuf[offs -2] = 0;
					break;
				}
				strBuf[offs++] = 0xc2;
				if( (bufSize -offs) == 0 )
				{
					strBuf[offs -1] = 0;
					break;
				}
				strBuf[offs++] = 0xac;
				if( (bufSize -offs) == 0 )
				{
					strBuf[offs -2] = 0;
					break;
				}
			}
			else
				strBuf[offs++] = valStr[x];
		}
		else
			strBuf[offs++] = valStr[x];
		if( (bufSize -offs) <= 1 )
		{
			strBuf[offs] = 0;
			break;
		}
		if( valStr[x] == 0 )
			break;
	}
	if( (bufSize -offs) > 0 )
	{
		strBuf[offs -1] = '\n';
		strBuf[offs++] = 0;
	}
	
	if( arrayPtr->smallerItem )
	{
		offs = strlen(strBuf);
		LEOPrintArray( arrayPtr->smallerItem, strBuf +offs, bufSize -offs, inContext );
	}
	if( arrayPtr->largerItem )
	{
		offs = strlen(strBuf);
		LEOPrintArray( arrayPtr->largerItem, strBuf +offs, bufSize -offs, inContext );
	}
}


size_t	LEOGetArrayKeyCount( struct LEOArrayEntry* arrayPtr )
{
	if( arrayPtr == NULL )
		return 0;
	
	size_t	numEntries = 1;
	
	if( arrayPtr->smallerItem )
		numEntries += LEOGetArrayKeyCount( arrayPtr->smallerItem );
	
	if( arrayPtr->largerItem )
		numEntries += LEOGetArrayKeyCount( arrayPtr->largerItem );
	
	return numEntries;
}


void	LEOCleanUpArray( struct LEOArrayEntry* arrayPtr, struct LEOContext* inContext )
{
	if( !arrayPtr )
		return;	// Nothing to do, never added a value to the array.
	
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


