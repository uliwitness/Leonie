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


// Layout of the virtual function tables:
struct LEOValueType
{
	size_t		size;
	
	double		(*GetAsNumber)( LEOValuePtr self );
	void		(*GetAsString)( LEOValuePtr self, char* outBuf, long bufSize );
	LEOObjectID	(*GetAsObjectID)( LEOValuePtr self );
	bool		(*GetAsBoolean)( LEOValuePtr self );
	
	void		(*SetAsNumber)( LEOValuePtr self, double inNumber );
	void		(*SetAsString)( LEOValuePtr self, const char* inBuf );
	void		(*SetAsObjectID)( LEOValuePtr self, LEOObjectID inObjectID );
	void		(*SetAsBoolean)( LEOValuePtr self, bool inBoolean );
	
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


// We build the stack out of entries of this union, so we can just add 1 to the
//	stack pointer to add another value to the stack:
//	It's probably a good idea to keep all the value structs small, so the stack
//	doesn't grow too large.
union LEOValue
{
	struct LEOValueBase		base;
	struct LEOValueNumber	number;
	struct LEOValueString	string;
	struct LEOValueObject	object;
	struct LEOValueBoolean	boolean;
};


// -----------------------------------------------------------------------------
//	IVar layout for LEOValueBase subclasses:
// -----------------------------------------------------------------------------

// Constructors & storage space measuring:
void		LEOInitNumberValue( LEOValuePtr inStorage, double inNumber );
void		LEOInitStringValue( LEOValuePtr inStorage, const char* inString );
void		LEOInitStringConstantValue( LEOValuePtr inStorage, const char* inString );
void		LEOInitBooleanValue( LEOValuePtr inStorage, bool inBoolean );

#define		LEOGetValueSize(v)					(((LEOValuePtr)(v))->isa->size)

#define		LEOGetNumberValueSize()				(kLeoValueTypeNumber.size)		
#define		LEOGetStringValueSize()				(kLeoValueTypeString.size)		
#define		LEOGetStringConstantValueSize()		(kLeoValueTypeStringConstant.size)		
#define		LEOGetBooleanValueSize()			(kLeoValueTypeBoolean.size)		

// Convenience macros for calling virtual methods w/o knowing what object it is:
#define 	LEOGetValueAsNumber(v)		((LEOValuePtr)(v))->isa->GetAsNumber(((LEOValuePtr)(v)))
#define 	LEOGetValueAsString(v,s,l)	((LEOValuePtr)(v))->isa->GetAsString(((LEOValuePtr)(v)),(s),(l))
#define 	LEOGetValueAsObjectID(v)	((LEOValuePtr)(v))->isa->GetAsObjectID(((LEOValuePtr)(v)))
#define 	LEOGetValueAsBoolean(v)		((LEOValuePtr)(v))->isa->GetAsBoolean(((LEOValuePtr)(v)))

#define 	LEOSetValueAsNumber(v,n)	((LEOValuePtr)(v))->isa->SetAsNumber(((LEOValuePtr)(v)),(n))
#define 	LEOSetValueAsString(v,s)	((LEOValuePtr)(v))->isa->SetAsString(((LEOValuePtr)(v)),(s))
#define 	LEOSetValueAsObjectID(v,i)	((LEOValuePtr)(v))->isa->SetAsObjectID(((LEOValuePtr)(v)),(i))
#define 	LEOSetValueAsBoolean(v,n)	((LEOValuePtr)(v))->isa->SetAsBoolean(((LEOValuePtr)(v)),(n))

#define 	LEOInitCopy(v,d)			((LEOValuePtr)(v))->isa->InitCopy(((LEOValuePtr)(v)),((LEOValuePtr)(n)))

#define 	LEOCleanUpValue(v)			((LEOValuePtr)(v))->isa->CleanUp(((LEOValuePtr)(v)))

// Failure indicators we re-use in many places:
LEOObjectID	LEOCantGetValueAsObjectID( LEOValuePtr self );
double		LEOCantGetValueAsNumber( LEOValuePtr self );
bool		LEOCantGetValueAsBoolean( LEOValuePtr self );
void		LEOCantSetValueAsObjectID( LEOValuePtr self, LEOObjectID inObjectID );
void		LEOCantSetValueAsNumber( LEOValuePtr self, double inNumber );
void		LEOCantSetValueAsBoolean( LEOValuePtr self, bool inState );

// Number instance methods:
double		LEOGetNumberValueAsNumber( LEOValuePtr self );
void		LEOGetNumberValueAsString( LEOValuePtr self, char* outBuf, long bufSize );
void		LEOSetNumberValueAsNumber( LEOValuePtr self, double inNumber );
void		LEOSetNumberValueAsString( LEOValuePtr self, const char* inNumber );
void		LEOInitNumberValueCopy( LEOValuePtr self, LEOValuePtr dest );
void		LEOCleanUpNumberValue( LEOValuePtr self );

// String instance methods:
double		LEOGetStringValueAsNumber( LEOValuePtr self );
bool		LEOGetStringValueAsBoolean( LEOValuePtr self );
void		LEOGetStringValueAsString( LEOValuePtr self, char* outBuf, long bufSize );
void		LEOSetStringValueAsNumber( LEOValuePtr self, double inNumber );
void		LEOSetStringValueAsString( LEOValuePtr self, const char* inString );
void		LEOSetStringValueAsBoolean( LEOValuePtr self, bool inBoolean );					// Makes it a constant string.
void 		LEOSetStringValueAsStringConstant( LEOValuePtr self, const char* inString );	// Makes it a constant string.
void		LEOInitStringValueCopy( LEOValuePtr self, LEOValuePtr dest );
void		LEOCleanUpStringValue( LEOValuePtr self );

// Replacement assignment methods and destructors for constant-referencing strings:
void		LEOSetStringConstantValueAsNumber( LEOValuePtr self, double inNumber );			// Makes it a dynamically allocated string.
void		LEOSetStringConstantValueAsString( LEOValuePtr self, const char* inString );	// Makes it a dynamically allocated string.
void		LEOSetStringConstantValueAsBoolean( LEOValuePtr self, bool inBoolean );
void		LEOInitStringConstantValueCopy( LEOValuePtr self, LEOValuePtr dest );
void		LEOCleanUpStringConstantValue( LEOValuePtr self );

// Boolean instance methods:
void		LEOGetBooleanValueAsString( LEOValuePtr self, char* outBuf, long bufSize );
bool		LEOGetBooleanValueAsBoolean( LEOValuePtr self );
void		LEOSetBooleanValueAsString( LEOValuePtr self, const char* inString );
void		LEOSetBooleanValueAsBoolean( LEOValuePtr self, bool inBoolean );
void		LEOInitBooleanValueCopy( LEOValuePtr self, LEOValuePtr dest );
void		LEOCleanUpBooleanValue( LEOValuePtr self );


#endif // LEO_VALUE_H
