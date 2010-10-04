/*
 *  LEOValue.h
 *  Leonie
 *
 *  Created by Uli Kusterer on 06.09.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#ifndef LEO_VALUE_H
#define LEO_VALUE_H		1

// -----------------------------------------------------------------------------
//	Headers:
// -----------------------------------------------------------------------------

#include <limits.h>
#include <sys/types.h>
#include <stdbool.h>
#include "LEOChunks.h"


// -----------------------------------------------------------------------------
//	Constants:
// -----------------------------------------------------------------------------

#define LEOObjectIDINVALID		ULONG_LONG_MAX


// -----------------------------------------------------------------------------
//	Types:
// -----------------------------------------------------------------------------

typedef unsigned long long		LEOObjectID;
typedef struct LEOValueType *	LEOValueTypePtr;
typedef struct LEOValueBase	*	LEOValuePtr;


struct LEOContext;


// Layout of the virtual function tables:
struct LEOValueType
{
	const char*	displayTypeName;	// Used for error messages etc. Doesn't distinguish between dynamic and constant strings.
	size_t		size;				// Minimal size required for a variable of this type.
	
	double		(*GetAsNumber)( LEOValuePtr self, struct LEOContext* inContext );
	void		(*GetAsString)( LEOValuePtr self, char* outBuf, long bufSize, struct LEOContext* inContext );
	LEOObjectID	(*GetAsObjectID)( LEOValuePtr self, struct LEOContext* inContext );
	bool		(*GetAsBoolean)( LEOValuePtr self, struct LEOContext* inContext );
	void		(*GetAsRangeOfString)( LEOValuePtr self, LEOChunkType inType,
											size_t inRangeStart, size_t inRangeEnd,
											char* outBuf, long bufSize, struct LEOContext* inContext );
	
	void		(*SetAsNumber)( LEOValuePtr self, double inNumber, struct LEOContext* inContext );
	void		(*SetAsString)( LEOValuePtr self, const char* inBuf, struct LEOContext* inContext );
	void		(*SetAsObjectID)( LEOValuePtr self, LEOObjectID inObjectID, struct LEOContext* inContext );
	void		(*SetAsBoolean)( LEOValuePtr self, bool inBoolean, struct LEOContext* inContext );
	void		(*SetRangeAsString)( LEOValuePtr self, LEOChunkType inType,
									size_t inRangeStart, size_t inRangeEnd,
									const char* inBuf, struct LEOContext* inContext );
	void		(*InitCopy)( LEOValuePtr self, LEOValuePtr dest );	// dest is an uninitialized value.
	
	void		(*CleanUp)( LEOValuePtr self );
};


// -----------------------------------------------------------------------------
//	IVar layout common to all LEOValueBase subclasses:
// -----------------------------------------------------------------------------

// Each type of subclass should begin with this struct:
//	That way it can be typecasted to the more generic type and .
struct LEOValueBase
{
	LEOValueTypePtr		isa;	// Virtual function dispatch table.
};


// Tables we store in the isa member for the various types:
extern struct LEOValueType	kLeoValueTypeNumber;
extern struct LEOValueType	kLeoValueTypeString;
extern struct LEOValueType	kLeoValueTypeStringConstant;
extern struct LEOValueType	kLeoValueTypeBoolean;


// -----------------------------------------------------------------------------
//	IVar layout for LEOValueBase subclasses:
// -----------------------------------------------------------------------------

// A number, we currently only do floating point values:
struct LEOValueNumber
{
	struct LEOValueBase	base;
	double				number;
};
typedef struct LEOValueNumber	LEOValueNumber;


// This is used both for strings we dynamically allocated, and for ones referencing
//	C string constants built into the program:
//	It can happen that such a string value gets turned from constant into an
//	dynamic one or back.
struct LEOValueString
{
	struct LEOValueBase	base;
	char*				string;
};
typedef struct LEOValueString	LEOValueString;


// An object is anything we can't stuff in a small quantity, and anything that
//	might disappear while we're executing:
// A LEOObjectID MUST NOT be a direct pointer, it must reference a quantity that
//	is either guaranteed to stay around, or can be detected as gone before it is
//	accessed and we crash. 
struct LEOValueObject
{
	struct LEOValueBase	base;
	LEOObjectID			objectID;
};
typedef struct LEOValueObject	LEOValueObject;


// This is a boolean. In our language, booleans and integers are distinct types,
//	but the (case-insensitive) strings "true" and "false" are valid booleans as
//	well.
struct LEOValueBoolean
{
	struct LEOValueBase	base;
	bool				boolean;
};
typedef struct LEOValueBoolean	LEOValueBoolean;


// Essentially our castrated version of a pointer, without pointer arithmetics
//	 or other type-unsafeties you wouldn't like to have in our language.
struct LEOValueReference
{
	struct LEOValueBase	base;
	LEOValuePtr			referencedValue;
};
typedef struct LEOValueReference	LEOValueReference;


// Arrays in our language are *associative* arrays, so they're not necessarily
//	continuously numbered, but rather contain items associated with a string.
struct LEOValueArray
{
	struct LEOValueBase		base;
	struct LEOArrayEntry	*array;		// B-tree of associative array items.
};
typedef struct LEOValueArray	LEOValueArray;


// We build the stack out of entries of this union, so we can just add 1 to the
//	stack pointer to add another value to the stack:
//	It's probably a good idea to keep all the value structs small, so the stack
//	doesn't grow too large.
union LEOValue
{
	struct LEOValueBase			base;
	struct LEOValueNumber		number;
	struct LEOValueString		string;
	struct LEOValueObject		object;
	struct LEOValueBoolean		boolean;
	struct LEOValueReference	reference;
	struct LEOValueArray		array;
};


// -----------------------------------------------------------------------------
//	IVar layout for LEOValueBase subclasses:
// -----------------------------------------------------------------------------

// Constructors & storage space measuring:
void		LEOInitNumberValue( LEOValuePtr inStorage, double inNumber );
void		LEOInitStringValue( LEOValuePtr inStorage, const char* inString );
void		LEOInitStringConstantValue( LEOValuePtr inStorage, const char* inString );
void		LEOInitBooleanValue( LEOValuePtr inStorage, bool inBoolean );
void		LEOInitReferenceValue( LEOValuePtr inStorage, LEOValuePtr referencedValue );

#define		LEOGetValueSize(v)					(((LEOValuePtr)(v))->isa->size)

#define		LEOGetNumberValueSize()				(kLeoValueTypeNumber.size)		
#define		LEOGetStringValueSize()				(kLeoValueTypeString.size)		
#define		LEOGetStringConstantValueSize()		(kLeoValueTypeStringConstant.size)		
#define		LEOGetBooleanValueSize()			(kLeoValueTypeBoolean.size)		

// Convenience macros for calling virtual methods w/o knowing what object it is:
#define 	LEOGetValueAsNumber(v,c)		((LEOValuePtr)(v))->isa->GetAsNumber(((LEOValuePtr)(v)),(c))
#define 	LEOGetValueAsString(v,s,l,c)	((LEOValuePtr)(v))->isa->GetAsString(((LEOValuePtr)(v)),(s),(l),(c))
#define 	LEOGetValueAsObjectID(v,c)		((LEOValuePtr)(v))->isa->GetAsObjectID(((LEOValuePtr)(v)),(c))
#define 	LEOGetValueAsBoolean(v,c)		((LEOValuePtr)(v))->isa->GetAsBoolean(((LEOValuePtr)(v)),(c))
#define 	LEOGetValueAsRangeOfString(v,t,rs,re,s,sl,c)	((LEOValuePtr)(v))->isa->GetAsRangeOfString(((LEOValuePtr)(v)),(t),(rs),(re),(s),(sl),(c))

#define 	LEOSetValueAsNumber(v,n,c)		((LEOValuePtr)(v))->isa->SetAsNumber(((LEOValuePtr)(v)),(n),(c))
#define 	LEOSetValueAsString(v,s,c)		((LEOValuePtr)(v))->isa->SetAsString(((LEOValuePtr)(v)),(s),(c))
#define 	LEOSetValueAsObjectID(v,i,c)	((LEOValuePtr)(v))->isa->SetAsObjectID(((LEOValuePtr)(v)),(i),(c))
#define 	LEOSetValueAsBoolean(v,n,c)		((LEOValuePtr)(v))->isa->SetAsBoolean(((LEOValuePtr)(v)),(n),(c))
#define 	LEOSetValueRangeAsString(v,t,rs,re,s,c)	((LEOValuePtr)(v))->isa->SetRangeAsString(((LEOValuePtr)(v)),(t),(rs),(re),(s),(c))

#define 	LEOInitCopy(v,d)				((LEOValuePtr)(v))->isa->InitCopy(((LEOValuePtr)(v)),((LEOValuePtr)(d)))

#define 	LEOCleanUpValue(v)				((LEOValuePtr)(v))->isa->CleanUp(((LEOValuePtr)(v)))

// Failure indicators we re-use in many places:
LEOObjectID	LEOCantGetValueAsObjectID( LEOValuePtr self, struct LEOContext* inContext );
double		LEOCantGetValueAsNumber( LEOValuePtr self, struct LEOContext* inContext );
bool		LEOCantGetValueAsBoolean( LEOValuePtr self, struct LEOContext* inContext );
void		LEOCantSetValueAsObjectID( LEOValuePtr self, LEOObjectID inObjectID, struct LEOContext* inContext );
void		LEOCantSetValueAsNumber( LEOValuePtr self, double inNumber, struct LEOContext* inContext );
void		LEOCantSetValueAsBoolean( LEOValuePtr self, bool inState, struct LEOContext* inContext );
void		LEOCantSetValueRangeAsString( LEOValuePtr self, LEOChunkType inType,
									size_t inRangeStart, size_t inRangeEnd,
									const char* inBuf, struct LEOContext* inContext );

// Other methods reusable across several types:
void		LEOGetAnyValueAsRangeOfString( LEOValuePtr self, LEOChunkType inType,
									size_t inRangeStart, size_t inRangeEnd,
									char* outBuf, long bufSize, struct LEOContext* inContext );	// If it can be converted to string, this gets a range from it.

// Number instance methods:
double		LEOGetNumberValueAsNumber( LEOValuePtr self, struct LEOContext* inContext );
void		LEOGetNumberValueAsString( LEOValuePtr self, char* outBuf, long bufSize, struct LEOContext* inContext );
void		LEOSetNumberValueAsNumber( LEOValuePtr self, double inNumber, struct LEOContext* inContext );
void		LEOSetNumberValueAsString( LEOValuePtr self, const char* inNumber, struct LEOContext* inContext );
void		LEOInitNumberValueCopy( LEOValuePtr self, LEOValuePtr dest );
void		LEOCleanUpNumberValue( LEOValuePtr self );

// String instance methods:
double		LEOGetStringValueAsNumber( LEOValuePtr self, struct LEOContext* inContext );
bool		LEOGetStringValueAsBoolean( LEOValuePtr self, struct LEOContext* inContext );
void		LEOGetStringValueAsString( LEOValuePtr self, char* outBuf, long bufSize, struct LEOContext* inContext );
void		LEOGetStringValueAsRangeOfString( LEOValuePtr self, LEOChunkType inType,
									size_t inRangeStart, size_t inRangeEnd,
									char* outBuf, long bufSize, struct LEOContext* inContext );
void		LEOSetStringValueAsNumber( LEOValuePtr self, double inNumber, struct LEOContext* inContext );
void		LEOSetStringValueAsString( LEOValuePtr self, const char* inString, struct LEOContext* inContext );
void		LEOSetStringValueAsBoolean( LEOValuePtr self, bool inBoolean, struct LEOContext* inContext );				// Makes it a constant string.
void 		LEOSetStringValueAsStringConstant( LEOValuePtr self, const char* inString, struct LEOContext* inContext );	// Makes it a constant string.
void		LEOSetStringValueRangeAsString( LEOValuePtr self, LEOChunkType inType,
												size_t inRangeStart, size_t inRangeEnd,
												const char* inBuf, struct LEOContext* inContext );
void		LEOInitStringValueCopy( LEOValuePtr self, LEOValuePtr dest );
void		LEOCleanUpStringValue( LEOValuePtr self );

// Replacement assignment methods and destructors for constant-referencing strings:
void		LEOSetStringConstantValueAsNumber( LEOValuePtr self, double inNumber, struct LEOContext* inContext );		// Makes it a dynamically allocated string.
void		LEOSetStringConstantValueAsString( LEOValuePtr self, const char* inString, struct LEOContext* inContext );	// Makes it a dynamically allocated string.
void		LEOSetStringConstantValueAsBoolean( LEOValuePtr self, bool inBoolean, struct LEOContext* inContext );
void		LEOSetStringConstantValueRangeAsString( LEOValuePtr self, LEOChunkType inType,
												size_t inRangeStart, size_t inRangeEnd,
												const char* inBuf, struct LEOContext* inContext );						// Makes it a dynamically allocated string.
void		LEOInitStringConstantValueCopy( LEOValuePtr self, LEOValuePtr dest );
void		LEOCleanUpStringConstantValue( LEOValuePtr self );

// Boolean instance methods:
void		LEOGetBooleanValueAsString( LEOValuePtr self, char* outBuf, long bufSize, struct LEOContext* inContext );
bool		LEOGetBooleanValueAsBoolean( LEOValuePtr self, struct LEOContext* inContext );
void		LEOSetBooleanValueAsString( LEOValuePtr self, const char* inString, struct LEOContext* inContext );
void		LEOSetBooleanValueAsBoolean( LEOValuePtr self, bool inBoolean, struct LEOContext* inContext );
void		LEOInitBooleanValueCopy( LEOValuePtr self, LEOValuePtr dest );
void		LEOCleanUpBooleanValue( LEOValuePtr self );


// Reference instance methods:
void		LEOGetReferenceValueAsString( LEOValuePtr self, char* outBuf, long bufSize, struct LEOContext* inContext );
double		LEOGetReferenceValueAsNumber( LEOValuePtr self, struct LEOContext* inContext );
LEOObjectID	LEOGetReferenceValueAsObjectID( LEOValuePtr self, struct LEOContext* inContext );
bool		LEOGetReferenceValueAsBoolean( LEOValuePtr self, struct LEOContext* inContext );
void		LEOGetReferenceValueAsRangeOfString( LEOValuePtr self, LEOChunkType inType,
											size_t inRangeStart, size_t inRangeEnd,
											char* outBuf, long bufSize, struct LEOContext* inContext );
void		LEOSetReferenceValueAsString( LEOValuePtr self, const char* inString, struct LEOContext* inContext );
void		LEOSetReferenceValueAsBoolean( LEOValuePtr self, bool inBoolean, struct LEOContext* inContext );
void		LEOSetReferenceValueAsNumber( LEOValuePtr self, double inNumber, struct LEOContext* inContext );
void		LEOSetReferenceValueAsObjectID( LEOValuePtr self, LEOObjectID inObjectID, struct LEOContext* inContext );
void		LEOSetReferenceValueRangeAsString( LEOValuePtr self, LEOChunkType inType,
											size_t inRangeStart, size_t inRangeEnd,
											const char* inBuf, struct LEOContext* inContext );
void		LEOInitReferenceValueCopy( LEOValuePtr self, LEOValuePtr dest );
void		LEOCleanUpReferenceValue( LEOValuePtr self );



#endif // LEO_VALUE_H
