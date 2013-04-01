/*
 *  LEOValue.h
 *  Leonie
 *
 *  Created by Uli Kusterer on 06.09.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

/*!
	@header LEOValue (Public)
	
	LEOValues are how variables and stack entries are implemented in the LEO
	runtime. They can have various types, and most types can be transparently
	converted into each other.
	
	A LEOValue is essentially a base class (with subclasses) implemented in
	straight C. Each LEOValue has an "isa" that points to the method table.
	This way, you can e.g. run the InitCopy method on any LEOValue without
	having to know which "subclass" you are dealing with.
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

/*! If an object ID is invalid, it is set to this value. */
#define kLEOObjectIDINVALID		0


enum eLEOKeepReferencesFlag
{
	kLEOInvalidateReferences,
	kLEOKeepReferences
	
};
typedef int		LEOKeepReferencesFlag;


// -----------------------------------------------------------------------------
//	Types:
// -----------------------------------------------------------------------------

// A LEOValueTypePtr is a pointer to a method look-up table for an object used internally.
typedef struct LEOValueType *	LEOValueTypePtr;
/*! A LEOValuePtr is a generic pointer to a LEOValue or one of its subclasses. */
typedef union LEOValue	*		LEOValuePtr;
/*! Object IDs are indexes used internally by reference values to look up the referenced value in a way that's safe even after the original has been disposed of. */
typedef unsigned long			LEOObjectID;
/*! An Object Seed is a value used internally to allow reference values to detect whether the slot in which their value used to be has already been reused. */
typedef unsigned long			LEOObjectSeed;

/*! The type of integers in the language. This is guaranteed to be signed, but large enough to hold a pointer. */
typedef long long				LEOInteger;

/*! The type of fractional numbers in the language. */
typedef double					LEONumber;


struct LEOContext;
struct LEOArrayEntry;


// Layout of the virtual function tables:
struct LEOValueType
{
	const char*	displayTypeName;	// Used for error messages etc. Doesn't distinguish between dynamic and constant strings.
	size_t		size;				// Minimal size required for a variable of this type.
	
	LEONumber	(*GetAsNumber)( LEOValuePtr self, struct LEOContext* inContext );
	LEOInteger	(*GetAsInteger)( LEOValuePtr self, struct LEOContext* inContext );
	const char*	(*GetAsString)( LEOValuePtr self, char* outBuf, size_t bufSize, struct LEOContext* inContext );	// Either returns buf, or an internal pointer holding the entire string.
	bool		(*GetAsBoolean)( LEOValuePtr self, struct LEOContext* inContext );
	void		(*GetAsRangeOfString)( LEOValuePtr self, LEOChunkType inType,
											size_t inRangeStart, size_t inRangeEnd,
											char* outBuf, size_t bufSize, struct LEOContext* inContext );
	
	void		(*SetAsNumber)( LEOValuePtr self, LEONumber inNumber, struct LEOContext* inContext );
	void		(*SetAsInteger)( LEOValuePtr self, LEOInteger inNumber, struct LEOContext* inContext );
	void		(*SetAsString)( LEOValuePtr self, const char* inBuf, size_t bufLen, struct LEOContext* inContext );
	void		(*SetAsBoolean)( LEOValuePtr self, bool inBoolean, struct LEOContext* inContext );
	void		(*SetRangeAsString)( LEOValuePtr self, LEOChunkType inType,
									size_t inRangeStart, size_t inRangeEnd,
									const char* inBuf, struct LEOContext* inContext );
	void		(*SetPredeterminedRangeAsString)( LEOValuePtr self,
									size_t inRangeStart, size_t inRangeEnd,
									const char* inBuf, struct LEOContext* inContext );
	
	void		(*InitCopy)( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext );	// dest is an uninitialized value.
	void		(*InitSimpleCopy)( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext );	// dest is an uninitialized value.
	void		(*PutValueIntoValue)( LEOValuePtr self, LEOValuePtr dest, struct LEOContext* inContext );	// dest must be a VALID, initialized value!
	LEOValuePtr	(*FollowReferencesAndReturnValueOfType)( LEOValuePtr self, LEOValueTypePtr inType, struct LEOContext* inContext );	// Returns NULL if no direct reference.
	
	void		(*DetermineChunkRangeOfSubstring)( LEOValuePtr self, size_t *ioBytesStart, size_t *ioBytesEnd,
													size_t *ioBytesDelStart, size_t *ioBytesDelEnd,
													LEOChunkType inType, size_t inRangeStart, size_t inRangeEnd,
													struct LEOContext* inContext );
	void		(*CleanUp)( LEOValuePtr self, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext );

	bool		(*CanGetAsNumber)( LEOValuePtr self, struct LEOContext* inContext );
	
	LEOValuePtr	(*GetValueForKey)( LEOValuePtr self, const char* keyName, union LEOValue* tempStorage, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext );
	void		(*SetValueForKey)( LEOValuePtr self, const char* keyName, LEOValuePtr inValue, struct LEOContext* inContext );
	
	void		(*SetValueAsArray)( LEOValuePtr self, struct LEOArrayEntry* inArray, struct LEOContext* inContext );
	
	size_t		(*GetKeyCount)( LEOValuePtr self, struct LEOContext* inContext );
	
	void		(*GetValueForKeyOfRange)( LEOValuePtr self, const char* keyName, size_t startOffset, size_t endOffset, LEOValuePtr outValue, struct LEOContext* inContext );
	void		(*SetValueForKeyOfRange)( LEOValuePtr self, const char* keyName, LEOValuePtr inValue, size_t startOffset, size_t endOffset, struct LEOContext* inContext );
};


// -----------------------------------------------------------------------------
//	IVar layout common to all LEOValueBase subclasses:
// -----------------------------------------------------------------------------

// Each type of subclass should begin with this struct:
//	That way it can be typecasted to the more generic type and methods can be
//	called on it.
struct LEOValueBase
{
	LEOValueTypePtr		isa;			// Virtual function dispatch table.
	LEOObjectID			refObjectID;	// If we have a reference to us, this is it, so we can clear it on destruction.
};


// Tables we store in the isa member for the various types:
extern struct LEOValueType	kLeoValueTypeNumber;
extern struct LEOValueType	kLeoValueTypeInteger;
extern struct LEOValueType	kLeoValueTypeString;
extern struct LEOValueType	kLeoValueTypeStringConstant;
extern struct LEOValueType	kLeoValueTypeBoolean;
extern struct LEOValueType	kLeoValueTypeReference;
extern struct LEOValueType	kLeoValueTypeArray;
extern struct LEOValueType	kLeoValueTypeArrayVariant;
extern struct LEOValueType	kLeoValueTypeNumberVariant;
extern struct LEOValueType	kLeoValueTypeIntegerVariant;
extern struct LEOValueType	kLeoValueTypeStringVariant;
extern struct LEOValueType	kLeoValueTypeBooleanVariant;


// -----------------------------------------------------------------------------
//	IVar layout for LEOValueBase subclasses:
// -----------------------------------------------------------------------------

/*!
	A number, which may have a fractional part:
	@field	base	The instance variables inherited from the base class.
	@field	number	Where we store the actual number.
*/
struct LEOValueNumber
{
	struct LEOValueBase	base;
	LEONumber			number;
};
typedef struct LEOValueNumber	LEOValueNumber;


/*!
	A number that doesn't have fractional parts:
	@field	base	The instance variables inherited from the base class.
	@field	integer	Where we store the actual number.
*/
struct LEOValueInteger
{
	struct LEOValueBase	base;
	LEOInteger			integer;
};
typedef struct LEOValueInteger	LEOValueInteger;


/*!
	This is used both for strings we dynamically allocated, and for ones referencing
	C string constants built into the program:
	It can happen that such a string value gets turned from constant into an
	dynamic one or back.
	@field	base	The instance variables inherited from the base class.
	@field	string	A pointer to the string constant, or to a malloced block
					of memory holding the string, depending on what kind of
					string class it is.
*/
struct LEOValueString
{
	struct LEOValueBase	base;
	char*				string;
	size_t				stringLen;
};
typedef struct LEOValueString	LEOValueString;


/*!
	This is a boolean. In our language, booleans and integers are distinct types,
	but the (case-insensitive) strings "true" and "false" are valid booleans as
	well.
	@field	base	The instance variables inherited from the base class.
	@field	boolean	Where we store the actual boolean.
*/
struct LEOValueBoolean
{
	struct LEOValueBase	base;
	bool				boolean;
};
typedef struct LEOValueBoolean	LEOValueBoolean;


/*!
	Essentially our castrated version of a pointer, without pointer arithmetics
	or other type-unsafeties you wouldn't like to have in our language. A
	reference can detect whether the value it points to is still valid (or has
	been disposed) and can gracefully exit in that case, instead of crashing
	the application.
	@field	base		The instance variables inherited from the base class.
	@field	objectID	Index into the LEOContext's references array to the slot
						where the actual "master pointer" to our referenced
						value is stored.
	@field	objectSeed	The "seed" number of the object in the LEOContext's
						references array. When our referenced object goes away,
						the slot's seed is incremented, and by comparing it to
						this one, we can detect that even if the slot has been
						reused in the meantime.
	@field	chunkType	The type of chunk of the original value this references,
						or kLEOChunkTypeINVALID if this references the full
						value.
	@field	chunkStart	If chunkType isn't kLEOChunkTypeINVALID, this specifies
						the start of the chunk range of the referenced object.
	@field	chunkEnd	If chunkType isn't kLEOChunkTypeINVALID, this specifies
						the end of the chunk range of the referenced object.
*/
struct LEOValueReference
{
	struct LEOValueBase	base;
	LEOObjectID			objectID;
	LEOObjectSeed		objectSeed;
	LEOChunkType		chunkType;
	size_t				chunkStart;
	size_t				chunkEnd;
};
typedef struct LEOValueReference	LEOValueReference;


/*!
	Arrays in our language are <i>associative</i> arrays, so they're not necessarily
	continuously numbered, but rather contain items associated with a string.
	@field	base	The instance variables inherited from the base class.
	@field	array	Pointer to the root of a B-tree that holds all the array items.
*/
struct LEOValueArray
{
	struct LEOValueBase		base;
	struct LEOArrayEntry	*array;
};
typedef struct LEOValueArray	LEOValueArray;


/*!
	A generic wrapper around foreign objects. The host may provide its own
	LEOValueType for an individual instance, so don't make any assumptions.
	This is mainly here so foreign objects can pass through LEO scripts and be
	stored on the stack unharmed.
*/
struct LEOValueObject
{
	struct LEOValueBase		base;
	void*					object;
};


/*!
	We build the stack out of entries of this union, so we can just add 1 to the
	stack pointer to add another value to the stack, and so we don't have to
	check the size of each item before initializing.
	
	It's probably a good idea to keep all the value structs small, so the stack
	doesn't grow too large.
*/
union LEOValue
{
	struct LEOValueBase			base;
	struct LEOValueNumber		number;
	struct LEOValueInteger		integer;
	struct LEOValueString		string;
	struct LEOValueBoolean		boolean;
	struct LEOValueReference	reference;
	struct LEOValueArray		array;
	struct LEOValueObject		object;
};


// -----------------------------------------------------------------------------
//	Methods:
// -----------------------------------------------------------------------------

/*! @functiongroup Constructors */
/*!
	Initialize the given storage so it's a valid number value containing the
	given number.

	@seealso //leo_ref/c/macro/LEOGetNumberValueSize LEOGetNumberValueSize
	@seealso //leo_ref/c/macro/LEOInitIntegerValue LEOInitIntegerValue
*/
void		LEOInitNumberValue( LEOValuePtr inStorage, LEONumber inNumber, LEOKeepReferencesFlag keepReferences, struct LEOContext *inContext );


/*!
	Initialize the given storage so it's a valid number value containing the
	given number.

	@seealso //leo_ref/c/macro/LEOInitNumberValue LEOInitNumberValue
	@seealso //leo_ref/c/macro/LEOGetIntegerValueSize LEOGetIntegerValueSize
*/
void		LEOInitIntegerValue( LEOValuePtr inStorage, LEOInteger inNumber, LEOKeepReferencesFlag keepReferences, struct LEOContext *inContext );

/*!
	Initialize the given storage so it's a valid string value containing a copy
	of the given string.

	@seealso //leo_ref/c/macro/LEOGetStringValueSize LEOGetStringValueSize
*/
void		LEOInitStringValue( LEOValuePtr inStorage, const char* inString, size_t inLen, LEOKeepReferencesFlag keepReferences, struct LEOContext *inContext );

/*!
	Initialize the given storage so it's a valid string constant value directly
	referencing the given string. The caller is responsible for ensuring that
	the string is not disposed of while the value still needs it. 

	@seealso //leo_ref/c/macro/LEOGetStringConstantValueSize LEOGetStringConstantValueSize
*/
void		LEOInitStringConstantValue( LEOValuePtr inStorage, const char* inString, LEOKeepReferencesFlag keepReferences, struct LEOContext *inContext );

/*!
	Initialize the given storage so it's a valid boolean value containing the
	given boolean.

	@seealso //leo_ref/c/macro/LEOGetBooleanValueSize LEOGetBooleanValueSize
*/
void		LEOInitBooleanValue( LEOValuePtr inStorage, bool inBoolean, LEOKeepReferencesFlag keepReferences, struct LEOContext *inContext );

/*!
	Initialize the given storage so it's a valid reference value that
	points to the given original value. If the original value is destructed
	while a reference still points to it, method calls to such a reference will
	fail with an error message and abort execution of the current LEOContext.
	
	However, the destructor of the value is safe to call, as is InitCopy.
	
	Pass kLEOChunkTypeINVALID if you want to reference the whole value and not
	only a chunk of it.

	@seealso //leo_ref/c/macro/LEOGetReferenceValueSize LEOGetReferenceValueSize
*/
void		LEOInitReferenceValue( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences,
									LEOChunkType inType, size_t startOffs, size_t endOffs, struct LEOContext* inContext );

/*!
	Initialize the given storage so it's a valid reference value that
	points to an original value referenced by object ID and seed.
	If the original value is destructed while a reference still points to it,
	method calls to such a reference will fail with an error message and abort
	execution of the current LEOContext.
	
	However, the destructor of the value is safe to call, as is InitCopy.
	
	Pass kLEOChunkTypeINVALID if you want to reference the whole value and not
	only a chunk of it.

	@seealso //leo_ref/c/macro/LEOGetReferenceValueSize LEOGetReferenceValueSize
*/
void	LEOInitReferenceValueWithIDs( LEOValuePtr self, LEOObjectID referencedObjectID, LEOObjectSeed referencedObjectSeed,
									  LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext );


/*!
	Initialize the given storage so it's a valid number variant value containing
	the given number.
	
	A variant is a value whose type changes depending on what type of data you
	put in it. So while this value is initially a number, if you call
	LEOSetValueAsString() on it, it will turn itself into a string variant
	value, or if you call LEOSetValueAsBoolean() a boolean variant value.
	
	As such, assignments to variants can never fail due to a type mismatch,
	but retrieving a variant as a certain type still can.
	
	@seealso //leo_ref/c/func/LEOInitStringVariantValue LEOInitStringVariantValue
	@seealso //leo_ref/c/func/LEOInitIntegerVariantValue LEOInitIntegerVariantValue
	@seealso //leo_ref/c/func/LEOInitBooleanVariantValue LEOInitBooleanVariantValue
	@seealso //leo_ref/c/macro/LEOGetVariantValueSize LEOGetVariantValueSize
*/
void		LEOInitNumberVariantValue( LEOValuePtr self, LEONumber inNumber, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext );


/*!
	Initialize the given storage so it's a valid number variant value containing
	the given number.
	
	A variant is a value whose type changes depending on what type of data you
	put in it. So while this value is initially a number, if you call
	LEOSetValueAsString() on it, it will turn itself into a string variant
	value, or if you call LEOSetValueAsBoolean() a boolean variant value.
	
	As such, assignments to variants can never fail due to a type mismatch,
	but retrieving a variant as a certain type still can.
	
	@seealso //leo_ref/c/func/LEOInitStringVariantValue LEOInitStringVariantValue
	@seealso //leo_ref/c/func/LEOInitNumberVariantValue LEOInitNumberVariantValue
	@seealso //leo_ref/c/func/LEOInitBooleanVariantValue LEOInitBooleanVariantValue
	@seealso //leo_ref/c/macro/LEOGetVariantValueSize LEOGetVariantValueSize
*/
void		LEOInitIntegerVariantValue( LEOValuePtr self, LEOInteger inNumber, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext );

/*!
	Initialize the given storage so it's a valid string variant value containing
	the given C-string.
	
	A variant is a value whose type changes depending on what type of data you
	put in it. So while this value is initially a string, if you call
	LEOSetValueAsNumber() on it, it will turn itself into a number variant
	value, or if you call LEOSetValueAsBoolean() a boolean variant value.
	
	As such, assignments to variants can never fail due to a type mismatch,
	but retrieving a variant as a certain type still can.
	
	@seealso //leo_ref/c/func/LEOInitNumberVariantValue LEOInitNumberVariantValue
	@seealso //leo_ref/c/func/LEOInitIntegerVariantValue LEOInitIntegerVariantValue
	@seealso //leo_ref/c/func/LEOInitBooleanVariantValue LEOInitBooleanVariantValue
	@seealso //leo_ref/c/macro/LEOGetVariantValueSize LEOGetVariantValueSize
*/
void		LEOInitStringVariantValue( LEOValuePtr self, const char* inString, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext );

/*!
	Initialize the given storage so it's a valid boolean variant value containing
	the given boolean.
	
	A variant is a value whose type changes depending on what type of data you
	put in it. So while this value is initially a boolean, if you call
	LEOSetValueAsString() on it, it will turn itself into a string variant
	value, or if you call LEOSetValueAsNumber() a number variant value.
	
	As such, assignments to variants can never fail due to a type mismatch,
	but retrieving a variant as a certain type still can.
	
	@seealso //leo_ref/c/func/LEOInitStringVariantValue LEOInitStringVariantValue
	@seealso //leo_ref/c/func/LEOInitNumberVariantValue LEOInitNumberVariantValue
	@seealso //leo_ref/c/func/LEOInitIntegerVariantValue LEOInitIntegerVariantValue
	@seealso //leo_ref/c/macro/LEOGetVariantValueSize LEOGetVariantValueSize
*/
void		LEOInitBooleanVariantValue( LEOValuePtr self, bool inBoolean, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext );


/*! @functiongroup LEOValue storage measuring */
/*!
	@function LEOGetValueSize
	Returns the size (in bytes) of storage you need to provide to LEOInitCopy for
	this object.
	@param	v	The value whose size you want to measure.
*/
#define		LEOGetValueSize(v)					(((LEOValuePtr)(v))->base.isa->size)

/*!
	@function LEOGetNumberValueSize
	Returns the size (in bytes) of storage you need to provide to LEOInitCopy for
	a new number value.
*/
#define		LEOGetNumberValueSize()				(kLeoValueTypeNumber.size)		

/*!
	@function LEOGetIntegerValueSize
	Returns the size (in bytes) of storage you need to provide to LEOInitCopy for
	a new integer value.
*/
#define		LEOGetIntegerValueSize()			(kLeoValueTypeInteger.size)		

/*!
	@function LEOGetStringValueSize
	Returns the size (in bytes) of storage you need to provide to LEOInitCopy for
	a new dynamically allocated string value.
*/
#define		LEOGetStringValueSize()				(kLeoValueTypeString.size)		

/*!
	@function LEOGetStringConstantValueSize
	Returns the size (in bytes) of storage you need to provide to LEOInitCopy for
	a new constant string value.
*/
#define		LEOGetStringConstantValueSize()		(kLeoValueTypeStringConstant.size)		

/*!
	@function LEOGetBooleanValueSize
	Returns the size (in bytes) of storage you need to provide to LEOInitCopy for
	a new boolean value.
*/
#define		LEOGetBooleanValueSize()			(kLeoValueTypeBoolean.size)		

/*!
	@function LEOGetReferenceValueSize
	Returns the size (in bytes) of storage you need to provide to LEOInitCopy for
	a new reference value.
*/
#define		LEOGetReferenceValueSize()			(kLeoValueTypeReference.size)		

/*!
	@function LEOGetVariantValueSize
	Returns the size (in bytes) of storage you need to provide to LEOInitCopy for
	a new variant value. Since variants can be converted between all types, this
	method applies to all kinds of variants.
*/
#define		LEOGetVariantValueSize()				(sizeof(union LEOValue))		

// Convenience macros for calling virtual methods w/o knowing what object it is:
/*! @functiongroup LEOValue methods */

/*!
	@function LEOGetValueAsNumber
	Returns the given value as a <tt>LEONumber</tt>, converting it, if necessary.
	If conversion isn't possible, it will fail with an error message and stop
	execution in the current LEOContext.
	@param	v	The value you wish to read.
	@param	c	The context in which your script is currently running and in
				which errors will be stored.
*/
#define 	LEOGetValueAsNumber(v,c)		((LEOValuePtr)(v))->base.isa->GetAsNumber(((LEOValuePtr)(v)),(c))

/*!
	@function LEOGetValueAsInteger
	Returns the given value as a <tt>LEOInteger</tt>, converting it, if necessary.
	If conversion isn't possible, it will fail with an error message and stop
	execution in the current LEOContext.
	@param	v	The value you wish to read.
	@param	c	The context in which your script is currently running and in
				which errors will be stored.
*/
#define 	LEOGetValueAsInteger(v,c)		((LEOValuePtr)(v))->base.isa->GetAsInteger(((LEOValuePtr)(v)),(c))

/*!
	@function LEOGetValueAsString
	Provides the given value as a <tt>char*</tt>, converting it, if necessary.
	If conversion isn't possible, it will fail with an error message and stop
	execution in the current LEOContext.
	@param	v	The value you wish to read.
	@param	s	A character buffer to hold the string value.
	@param	l	The size of character buffer <tt>s</tt>.
	@param	c	The context in which your script is currently running and in
				which errors will be stored.
	@result		Either s, or a pointer to an internal buffer, if the value has
				one that contains the entire requested value.
*/
#define 	LEOGetValueAsString(v,s,l,c)	((LEOValuePtr)(v))->base.isa->GetAsString(((LEOValuePtr)(v)),(s),(l),(c))

/*!
	@function LEOGetValueAsBoolean
	Returns the given value as a <tt>bool</tt>, converting it, if necessary.
	If conversion isn't possible, it will fail with an error message and stop
	execution in the current LEOContext.
	@param	v	The value you wish to read.
	@param	c	The context in which your script is currently running and in
				which errors will be stored.
*/
#define 	LEOGetValueAsBoolean(v,c)		((LEOValuePtr)(v))->base.isa->GetAsBoolean(((LEOValuePtr)(v)),(c))

/*!
	@function LEOGetValueAsRangeOfString
	Provides a range of the string representation of the given value as a
	<tt>char*</tt>, converting it, if necessary.
	If conversion isn't possible, it will fail with an error message and stop
	execution in the current LEOContext.
	@param	v	The value you wish to read.
	@param	t	The LEOChunkType the range is specified in. This can be characters, words, items, lines etc.
	@param	rs	The starting index of the range to get. (e.g. rs=1,es=1,t=kLEOChunkTypeWord would give the second word in the string)
	@param	re	The ending index of the range to get. (e.g. rs=1,es=1,t=kLEOChunkTypeWord would give the second word in the string)
	@param	s	A character buffer to hold the string value.
	@param	sl	The size of character buffer <tt>s</tt>.
	@param	c	The context in which your script is currently running and in
				which errors will be stored.
*/
#define 	LEOGetValueAsRangeOfString(v,t,rs,re,s,sl,c)	((LEOValuePtr)(v))->base.isa->GetAsRangeOfString(((LEOValuePtr)(v)),(t),(rs),(re),(s),(sl),(c))


/*!
	@function LEOSetValueAsNumber
	Assigns the given number to the value, converting it, if necessary.
	If conversion isn't possible, it will fail with an error message and stop
	execution in the current LEOContext.
	@param	v	The value you wish to change.
	@param	n	The number to write to value <tt>v</tt>, as a <tt>LEONumber</tt>.
	@param	c	The context in which your script is currently running and in
				which errors will be stored.
*/
#define 	LEOSetValueAsNumber(v,n,c)		((LEOValuePtr)(v))->base.isa->SetAsNumber(((LEOValuePtr)(v)),(n),(c))

/*!
	@function LEOSetValueAsInteger
	Assigns the given number to the value, converting it, if necessary.
	If conversion isn't possible, it will fail with an error message and stop
	execution in the current LEOContext.
	@param	v	The value you wish to change.
	@param	n	The number to write to value <tt>v</tt>, as a <tt>LEOInteger</tt>.
	@param	c	The context in which your script is currently running and in
				which errors will be stored.
*/
#define 	LEOSetValueAsInteger(v,n,c)		((LEOValuePtr)(v))->base.isa->SetAsInteger(((LEOValuePtr)(v)),(n),(c))

/*!
	@function LEOSetValueAsString
	Assigns the given string to the value, converting it, if necessary.
	If conversion isn't possible, it will fail with an error message and stop
	execution in the current LEOContext.
	@param	v	The value you wish to change.
	@param	s	The UTF8 string to write to value <tt>v</tt>, as a <tt>char*</tt>.
	@param	l	The length of the string provided in <tt>s</tt>.
	@param	c	The context in which your script is currently running and in
				which errors will be stored.
*/
#define 	LEOSetValueAsString(v,s,l,c)		((LEOValuePtr)(v))->base.isa->SetAsString(((LEOValuePtr)(v)),(s),(l),(c))

/*!
	@function LEOSetValueAsBoolean
	Assigns the given boolean to the value, converting it, if necessary.
	If conversion isn't possible, it will fail with an error message and stop
	execution in the current LEOContext.
	@param	v	The value you wish to change.
	@param	n	The boolean to write to value <tt>v</tt>, as a <tt>bool</tt>.
	@param	c	The context in which your script is currently running and in
				which errors will be stored.
*/
#define 	LEOSetValueAsBoolean(v,n,c)		((LEOValuePtr)(v))->base.isa->SetAsBoolean(((LEOValuePtr)(v)),(n),(c))

/*!
	@function LEOSetValueRangeAsString
	Assigns the given string to the specified range of the given value, converting
	it, if necessary.
	If conversion isn't possible, it will fail with an error message and stop
	execution in the current LEOContext.
	@param	v	The value you wish to change.
	@param	t	The LEOChunkType the range is specified in. This can be characters, words, items, lines etc.
	@param	rs	The starting index of the range to get. (e.g. rs=1,es=1,t=kLEOChunkTypeWord would give the second word in the string)
	@param	re	The ending index of the range to get. (e.g. rs=1,es=1,t=kLEOChunkTypeWord would give the second word in the string)
	@param	s	The string to insert, or NULL to delete the given range (and any delimiters).
	@param	c	The context in which your script is currently running and in
				which errors will be stored.
*/
#define 	LEOSetValueRangeAsString(v,t,rs,re,s,c)	((LEOValuePtr)(v))->base.isa->SetRangeAsString(((LEOValuePtr)(v)),(t),(rs),(re),(s),(c))


/*!
	@function LEOSetValuePredeterminedRangeAsString
	Assigns the given string to the specified range of the given value, converting
	it, if necessary.
	If conversion isn't possible, it will fail with an error message and stop
	execution in the current LEOContext.
	@param	v	The value you wish to change.
	@param	rs	The starting index of the range to get, as obtained from LEODetermineChunkRangeOfSubstring.
	@param	re	The ending index of the range to get, as obtained from LEODetermineChunkRangeOfSubstring.
	@param	s	The string to insert, or NULL to delete the given range.
	@param	c	The context in which your script is currently running and in
				which errors will be stored.
	@seealso //leo_ref/c/func/LEODetermineChunkRangeOfSubstring LEODetermineChunkRangeOfSubstring
*/
#define 	LEOSetValuePredeterminedRangeAsString(v,rs,re,s,c)	((LEOValuePtr)(v))->base.isa->SetPredeterminedRangeAsString(((LEOValuePtr)(v)),(rs),(re),(s),(c))


/*!
	@function LEOCanGetAsNumber
	Returns TRUE if the value can be converted to a number, FALSE otherwise.
	@param	v	The value you wish to know about.
	@param	c	The context in which your script is currently running and in
				which errors will be stored.
*/
#define 	LEOCanGetAsNumber(v,c)		((LEOValuePtr)(v))->base.isa->CanGetAsNumber(((LEOValuePtr)(v)),(c))


/*!
	@function LEOInitCopy
	Initializes the given storage to be an exact copy of the given value. If the
	value is a reference, you will get a second reference to the original value,
	not a copy of the original value, and not a reference to the reference.
	@param	v	The value you wish to copy.
	@param	d	A LEOValuePtr to memory where you wish the copy to be stored.
				This destination must be large enough to hold the given type of
				value.
 	@param	k	A <tt>LEOKeepReferencesFlag</tt> indicating whether to clear references
 				or whether you just want to change the type of this value and will
				keep the storage valid. Usually, you pass <tt>kLEOInvalidateReferences</tt>.
	@param	c	The context in which your script is currently running and in
				which errors will be stored.
	@seealso //leo_ref/c/func/LEOGetValueSize LEOGetValueSize
*/
#define 	LEOInitCopy(v,d,k,c)				((LEOValuePtr)(v))->base.isa->InitCopy(((LEOValuePtr)(v)),((LEOValuePtr)(d)),(k),(c))


#define 	LEOInitSimpleCopy(v,d,k,c)			((LEOValuePtr)(v))->base.isa->InitSimpleCopy(((LEOValuePtr)(v)),((LEOValuePtr)(d)),(k),(c))


/*!
	@function LEODetermineChunkRangeOfSubstring
	Parses the given substring of a value for the given chunk, and then returns
	the absolute range of that chunk.
	@param	v	The value you wish to examine.
	@param	bs	On input, a pointer to a size_t containing the start offset (in
				bytes) of the substring to examine. On output, it will be set to
				the chunk's absolute start offset.
	@param	be	On input, a pointer to a size_t containing the end offset (in
				bytes) of the substring to examine. On output, it will be set to
				the chunk's absolute end offset.
	@param	bds	The size_t pointed to by this will be set to the chunk's
				absolute start offset for deletion.
	@param	bde	The size_t pointed to by this will be set to the chunk's
				absolute end offset for deletion.
	@param	t	The chunk type to use for parsing this chunk.
	@param	rs	The start offset of the chunk (in whatever unit the 't' parameter
				defines) to find in the substring.
	@param	re	The end offset of the chunk (in whatever unit the 't' parameter
				defines) to find in the substring.
	@param	c	The context in which your script is currently running and in
				which errors will be stored.
	@seealso //leo_ref/c/func/LEOSetValuePredeterminedRangeAsString LEOSetValuePredeterminedRangeAsString
*/
#define 	LEODetermineChunkRangeOfSubstring(v,bs,be,bds,bde,t,rs,re,c)		((LEOValuePtr)(v))->base.isa->DetermineChunkRangeOfSubstring(((LEOValuePtr)(v)),(bs),(be),(bds),(bde),(t),(rs),(re),(c))


/*!
	@function LEOGetValueForKey
	Fetches the value with the given key from the array in the specified value.
	Returns NULL if there is no value under that key yet.
	@param	v	The value you wish to examine.
	@param	ak	The key of the array item to fetch.
	@param	t	Temporary storage for the retrieved value in case the value is
				not an array and had to be converted to a temporary array. 
 	@param	k	A <tt>LEOKeepReferencesFlag</tt> indicating whether to clear references
 				or whether you just want to change the type of tempStorage and will
				keep the storage valid. Usually, you pass <tt>kLEOInvalidateReferences</tt>.
	@param	c	The context in which your script is currently running and in
				which errors will be stored.
	@result		A LEOValuePtr pointing to the actual value in the array.
*/
#define 	LEOGetValueForKey(v,ak,t,k,c)			((LEOValuePtr)(v))->base.isa->GetValueForKey(((LEOValuePtr)(v)),(ak),(t),(k),(c))


/*!
	@function LEOSetValueForKey
	Stores a value under the given key in the array in the specified value.
	
	@param	v	The value you wish to examine.
	@param	ak	The key of the array item to fetch.
	@param	s	The source value to copy into the array.
	@param	c	The context in which your script is currently running and in
				which errors will be stored.
*/
#define 	LEOSetValueForKey(v,ak,s,c)			((LEOValuePtr)(v))->base.isa->SetValueForKey(((LEOValuePtr)(v)),(ak),(s),(c))


/*!
	@function LEOGetKeyCount
	Returns the number of key-value-pairs in this value.
	@param	v	The value you wish to examine.
	@param	c	The context in which your script is currently running and in
				which errors will be stored.
	@result		A LEOValuePtr pointing to the actual value in the array.
*/
#define 	LEOGetKeyCount(v,c)			((LEOValuePtr)(v))->base.isa->GetKeyCount(((LEOValuePtr)(v)),(c))


/*!
	@function LEOGetValueForKeyOfRange
	Fetches the value with the given key from the array in the specified value.
	Returns NULL if there is no value under that key yet.
	@param	v	The value you wish to examine.
	@param	k	The key of the array item to fetch.
	@param	so	The start offset of the byte range.
	@param	eo	The end offset of the byte range.
	@param	ov	This value will be set to a copy of value of the key of that range.
	@param	c	The context in which your script is currently running and in
				which errors will be stored.
*/
#define 	LEOGetValueForKeyOfRange(v,ak,so,eo,ov,c)			((LEOValuePtr)(v))->base.isa->GetValueForKeyOfRange(((LEOValuePtr)(v)),(ak),(so),(eo),(ov),(c))


/*!
	@function LEOSetValueForKeyOfRange
	Stores a value under the given key in the array in the specified value.
	
	@param	v	The value you wish to examine.
	@param	k	The key of the array item to fetch.
	@param	s	The source value to copy into the array.
	@param	so	The start offset of the byte range.
	@param	eo	The end offset of the byte range.
	@param	c	The context in which your script is currently running and in
				which errors will be stored.
*/
#define 	LEOSetValueForKeyOfRange(v,ak,s,so,eo,c)			((LEOValuePtr)(v))->base.isa->SetValueForKeyOfRange(((LEOValuePtr)(v)),(ak),(s),(so),(eo),(c))


/*!
	@function LEOCleanUpValue
	Dispose of any additional storage the value has allocated to hold its data.
	If there are references to this value, this will cause the references to fail
	with an error when next they try to access this value.
	@param	v	The value you wish to dispose of.
	@param	k	A <tt>LEOKeepReferencesFlag</tt> indicating whether to clear references
				or whether you just want to change the type of this value and will
				keep teh storage valid. Usually, you pass <tt>kLEOInvalidateReferences</tt>.
	@param	c	The context in which your script is currently running and in
				which errors will be stored.
*/
#define 	LEOCleanUpValue(v,k,c)			do { if( ((LEOValuePtr)(v)) && ((LEOValuePtr)(v))->base.isa ) ((LEOValuePtr)(v))->base.isa->CleanUp(((LEOValuePtr)(v)),(k),(c)); } while(0)


/*!
	@function LEOPutValueIntoValue
	Copy the contents of a value to another value, preserving the native type,
	if possible.
	@param	v	The value you wish to copy.
	@param	d	The value to assign to. If the value is a different type than the
				source, it will take care of conversion (raising errors if it
				can't be converted), if the destination is a variant, it will
				change its type accordingly.
	@param	c	The context in which your script is currently running and in
				which errors will be stored.
*/
#define		LEOPutValueIntoValue(v,d,c)		((LEOValuePtr)(v))->base.isa->PutValueIntoValue(((LEOValuePtr)(v)),(d),(c))


/*!
	@function LEOFollowReferencesAndReturnValueOfType
	Follow all references in the given value, until a value of a specific type is found.
	@param	v	The value you wish to examine.
	@param	t	The type of the value that is desired.
	@param	c	The context in which your script is currently running and in
				which errors will be stored.
	@result		A LEOValuePtr pointing to the value matching the given type, or NULL if no value of that type is in the chain of references.
*/
#define 	LEOFollowReferencesAndReturnValueOfType(v,t,c)			((LEOValuePtr)(v))->base.isa->FollowReferencesAndReturnValueOfType(((LEOValuePtr)(v)),(t),(c))


/*!
	@function LEOSetValueAsArray
	Replace the value of a variant or array with the given array value.
	@param	v	The value you wish to change.
	@param	a	The new value for that value.
	@param	c	The context in which your script is currently running and in
				which errors will be stored.
*/
#define		LEOSetValueAsArray(v,a,c)		((LEOValuePtr)(v))->base.isa->SetValueAsArray(((LEOValuePtr)(v)),(a),(c))


// Failure indicators we re-use in many places:
LEONumber	LEOCantGetValueAsNumber( LEOValuePtr self, struct LEOContext* inContext );
LEOInteger	LEOCantGetValueAsInteger( LEOValuePtr self, struct LEOContext* inContext );
bool		LEOCantGetValueAsBoolean( LEOValuePtr self, struct LEOContext* inContext );
LEOValuePtr	LEOCantGetValueForKey( LEOValuePtr self, const char* keyName, union LEOValue *tempStorage, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext );
void		LEOCantSetValueForKey( LEOValuePtr self, const char* keyName, LEOValuePtr inValue, struct LEOContext* inContext );
void		LEOCantSetValueAsNumber( LEOValuePtr self, LEONumber inNumber, struct LEOContext* inContext );
void		LEOCantSetValueAsInteger( LEOValuePtr self, LEOInteger inInteger, struct LEOContext* inContext );
void		LEOCantSetValueAsBoolean( LEOValuePtr self, bool inState, struct LEOContext* inContext );
void		LEOCantSetValueRangeAsString( LEOValuePtr self, LEOChunkType inType,
									size_t inRangeStart, size_t inRangeEnd,
									const char* inBuf, struct LEOContext* inContext );
void		LEOCantSetValuePredeterminedRangeAsString( LEOValuePtr self,
									size_t inRangeStart, size_t inRangeEnd,
									const char* inBuf, struct LEOContext* inContext );
bool		LEOCanGetValueAsNumber( LEOValuePtr self, struct LEOContext* inContext );
void		LEOCantSetValueAsString( LEOValuePtr self, const char* inString, size_t inStringLen, struct LEOContext* inContext );
bool		LEOCantCanGetValueAsNumber( LEOValuePtr self, struct LEOContext* inContext );
size_t		LEOCantGetKeyCount( LEOValuePtr self, struct LEOContext* inContext );
LEOValuePtr	LEOCantFollowReferencesAndReturnValueOfType( LEOValuePtr self, LEOValueTypePtr inType, struct LEOContext* inContext );
void		LEOCantSetValueAsArray( LEOValuePtr self, struct LEOArrayEntry *inArray, struct LEOContext* inContext );
void		LEOSetStringLikeValueAsArray( LEOValuePtr self, struct LEOArrayEntry *inArray, struct LEOContext* inContext );
void		LEOSetStringLikeValueForKey( LEOValuePtr self, const char* keyName, LEOValuePtr inValue, struct LEOContext* inContext );
void		LEOCantGetValueForKeyOfRange( LEOValuePtr self, const char* keyName, size_t startOffset, size_t endOffset, LEOValuePtr outValue, struct LEOContext* inContext );
void		LEOCantSetValueForKeyOfRange( LEOValuePtr self, const char* keyName, LEOValuePtr inValue, size_t startOffset, size_t endOffset, struct LEOContext* inContext );

// Other methods reusable across several types:
void		LEOGetAnyValueAsRangeOfString( LEOValuePtr self, LEOChunkType inType,
									size_t inRangeStart, size_t inRangeEnd,
									char* outBuf, size_t bufSize, struct LEOContext* inContext );	// If it can be converted to string, this gets a range from it.
void		LEODetermineChunkRangeOfSubstringOfAnyValue( LEOValuePtr self, size_t *ioBytesStart, size_t *ioBytesEnd,
														size_t *ioBytesDelStart, size_t *ioBytesDelEnd,
														LEOChunkType inType, size_t inRangeStart, size_t inRangeEnd,
														struct LEOContext* inContext );

// Number instance methods:
LEONumber	LEOGetNumberValueAsNumber( LEOValuePtr self, struct LEOContext* inContext );
LEOInteger	LEOGetNumberValueAsInteger( LEOValuePtr self, struct LEOContext* inContext );
const char*	LEOGetNumberValueAsString( LEOValuePtr self, char* outBuf, size_t bufSize, struct LEOContext* inContext );
void		LEOSetNumberValueAsNumber( LEOValuePtr self, LEONumber inNumber, struct LEOContext* inContext );
void		LEOSetNumberValueAsInteger( LEOValuePtr self, LEOInteger inNumber, struct LEOContext* inContext );
void		LEOSetNumberValueAsString( LEOValuePtr self, const char* inNumber, size_t inNumberLen, struct LEOContext* inContext );
void		LEOInitNumberValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext );
void		LEOPutNumberValueIntoValue( LEOValuePtr self, LEOValuePtr dest, struct LEOContext* inContext );
void		LEOCleanUpNumberValue( LEOValuePtr self, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext );

// Integer instance methods:
LEONumber	LEOGetIntegerValueAsNumber( LEOValuePtr self, struct LEOContext* inContext );
LEOInteger	LEOGetIntegerValueAsInteger( LEOValuePtr self, struct LEOContext* inContext );
const char*	LEOGetIntegerValueAsString( LEOValuePtr self, char* outBuf, size_t bufSize, struct LEOContext* inContext );
void		LEOSetIntegerValueAsNumber( LEOValuePtr self, LEONumber inNumber, struct LEOContext* inContext );
void		LEOSetIntegerValueAsInteger( LEOValuePtr self, LEOInteger inNumber, struct LEOContext* inContext );
void		LEOSetIntegerValueAsString( LEOValuePtr self, const char* inNumber, size_t inNumberLen, struct LEOContext* inContext );
void		LEOInitIntegerValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext );
void		LEOPutIntegerValueIntoValue( LEOValuePtr self, LEOValuePtr dest, struct LEOContext* inContext );
void		LEOCleanUpIntegerValue( LEOValuePtr self, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext );

// String instance methods:
LEONumber	LEOGetStringValueAsNumber( LEOValuePtr self, struct LEOContext* inContext );
LEOInteger	LEOGetStringValueAsInteger( LEOValuePtr self, struct LEOContext* inContext );
bool		LEOGetStringValueAsBoolean( LEOValuePtr self, struct LEOContext* inContext );
const char*	LEOGetStringValueAsString( LEOValuePtr self, char* outBuf, size_t bufSize, struct LEOContext* inContext );
void		LEOGetStringValueAsRangeOfString( LEOValuePtr self, LEOChunkType inType,
									size_t inRangeStart, size_t inRangeEnd,
									char* outBuf, size_t bufSize, struct LEOContext* inContext );
void		LEOSetStringValueAsNumber( LEOValuePtr self, LEONumber inNumber, struct LEOContext* inContext );
void		LEOSetStringValueAsInteger( LEOValuePtr self, LEOInteger inNumber, struct LEOContext* inContext );
void		LEOSetStringValueAsString( LEOValuePtr self, const char* inString, size_t inStringLen, struct LEOContext* inContext );
void		LEOSetStringValueAsBoolean( LEOValuePtr self, bool inBoolean, struct LEOContext* inContext );				// Makes it a constant string.
void 		LEOSetStringValueAsStringConstant( LEOValuePtr self, const char* inString, struct LEOContext* inContext );	// Makes it a constant string.
void		LEOSetStringValueRangeAsString( LEOValuePtr self, LEOChunkType inType,
												size_t inRangeStart, size_t inRangeEnd,
												const char* inBuf, struct LEOContext* inContext );
void		LEOSetStringValuePredeterminedRangeAsString( LEOValuePtr self,
												size_t inRangeStart, size_t inRangeEnd,
												const char* inBuf, struct LEOContext* inContext );
bool		LEOCanGetStringValueAsNumber( LEOValuePtr self, struct LEOContext* inContext );
void		LEOInitStringValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext );
void		LEOPutStringValueIntoValue( LEOValuePtr self, LEOValuePtr dest, struct LEOContext* inContext );
void		LEODetermineChunkRangeOfSubstringOfStringValue( LEOValuePtr self, size_t *ioBytesStart, size_t *ioBytesEnd,
															size_t *ioBytesDelStart, size_t *ioBytesDelEnd,
															LEOChunkType inType, size_t inRangeStart, size_t inRangeEnd,
															struct LEOContext* inContext );
LEOValuePtr	LEOGetStringValueForKey( LEOValuePtr self, const char* keyName, union LEOValue *tempStorage, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext );
void		LEOCleanUpStringValue( LEOValuePtr self, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext );

// Replacement assignment methods and destructors for constant-referencing strings:
void		LEOSetStringConstantValueAsNumber( LEOValuePtr self, LEONumber inNumber, struct LEOContext* inContext );	// Makes it a dynamically allocated string.
void		LEOSetStringConstantValueAsInteger( LEOValuePtr self, LEOInteger inNumber, struct LEOContext* inContext );	// Makes it a dynamically allocated string.
void		LEOSetStringConstantValueAsString( LEOValuePtr self, const char* inString, size_t inStringLen, struct LEOContext* inContext );	// Makes it a dynamically allocated string.
void		LEOSetStringConstantValueAsBoolean( LEOValuePtr self, bool inBoolean, struct LEOContext* inContext );
void		LEOSetStringConstantValueRangeAsString( LEOValuePtr self, LEOChunkType inType,
												size_t inRangeStart, size_t inRangeEnd,
												const char* inBuf, struct LEOContext* inContext );						// Makes it a dynamically allocated string.
void		LEOSetStringConstantValuePredeterminedRangeAsString( LEOValuePtr self,
												size_t inRangeStart, size_t inRangeEnd,
												const char* inBuf, struct LEOContext* inContext );						// Makes it a dynamically allocated string.
void		LEOInitStringConstantValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext );
void		LEOCleanUpStringConstantValue( LEOValuePtr self, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext );

// Boolean instance methods:
const char*	LEOGetBooleanValueAsString( LEOValuePtr self, char* outBuf, size_t bufSize, struct LEOContext* inContext );
bool		LEOGetBooleanValueAsBoolean( LEOValuePtr self, struct LEOContext* inContext );
void		LEOSetBooleanValueAsString( LEOValuePtr self, const char* inString, size_t inStringLen, struct LEOContext* inContext );
void		LEOSetBooleanValueAsBoolean( LEOValuePtr self, bool inBoolean, struct LEOContext* inContext );
void		LEOInitBooleanValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext );
void		LEOPutBooleanValueIntoValue( LEOValuePtr self, LEOValuePtr dest, struct LEOContext* inContext );
void		LEOCleanUpBooleanValue( LEOValuePtr self, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext );


// Reference instance methods:
const char*	LEOGetReferenceValueAsString( LEOValuePtr self, char* outBuf, size_t bufSize, struct LEOContext* inContext );
LEONumber	LEOGetReferenceValueAsNumber( LEOValuePtr self, struct LEOContext* inContext );
LEOInteger	LEOGetReferenceValueAsInteger( LEOValuePtr self, struct LEOContext* inContext );
bool		LEOGetReferenceValueAsBoolean( LEOValuePtr self, struct LEOContext* inContext );
void		LEOGetReferenceValueAsRangeOfString( LEOValuePtr self, LEOChunkType inType,
											size_t inRangeStart, size_t inRangeEnd,
											char* outBuf, size_t bufSize, struct LEOContext* inContext );
void		LEOSetReferenceValueAsString( LEOValuePtr self, const char* inString, size_t inLength, struct LEOContext* inContext );
void		LEOSetReferenceValueAsBoolean( LEOValuePtr self, bool inBoolean, struct LEOContext* inContext );
void		LEOSetReferenceValueAsNumber( LEOValuePtr self, LEONumber inNumber, struct LEOContext* inContext );
void		LEOSetReferenceValueAsInteger( LEOValuePtr self, LEOInteger inNumber, struct LEOContext* inContext );
void		LEOSetReferenceValueRangeAsString( LEOValuePtr self, LEOChunkType inType,
											size_t inRangeStart, size_t inRangeEnd,
											const char* inBuf, struct LEOContext* inContext );
void		LEOSetReferenceValuePredeterminedRangeAsString( LEOValuePtr self,
											size_t inRangeStart, size_t inRangeEnd,
											const char* inBuf, struct LEOContext* inContext );
bool		LEOCanGetReferenceValueAsNumber( LEOValuePtr self, struct LEOContext* inContext );
void		LEOInitReferenceValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext );
void		LEOPutReferenceValueIntoValue( LEOValuePtr self, LEOValuePtr dest, struct LEOContext* inContext );
LEOValuePtr	LEOReferenceValueFollowReferencesAndReturnValueOfType( LEOValuePtr self, LEOValueTypePtr inType, struct LEOContext* inContext );
void		LEOInitReferenceValueSimpleCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext );
void		LEODetermineChunkRangeOfSubstringOfReferenceValue( LEOValuePtr self, size_t *ioBytesStart, size_t *ioBytesEnd,
																size_t *ioBytesDelStart, size_t *ioBytesDelEnd,
																LEOChunkType inType, size_t inRangeStart, size_t inRangeEnd,
																struct LEOContext* inContext );
void		LEOCleanUpReferenceValue( LEOValuePtr self, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext );
LEOValuePtr	LEOGetReferenceValueValueForKey( LEOValuePtr self, const char* inKey, union LEOValue* tempStorage, LEOKeepReferencesFlag keepReferences, struct LEOContext * inContext );
void		LEOSetReferenceValueValueForKey( LEOValuePtr self, const char* inKey, LEOValuePtr inValue, struct LEOContext * inContext );
void		LEOSetReferenceValueAsArray( LEOValuePtr self, struct LEOArrayEntry * inArray, struct LEOContext * inContext );
size_t		LEOGetReferenceValueKeyCount( LEOValuePtr self, struct LEOContext * inContext );

void		LEOGetReferenceValueForKeyOfRange( LEOValuePtr self, const char* keyName, size_t startOffset, size_t endOffset, LEOValuePtr outValue, struct LEOContext* inContext );
void		LEOSetReferenceValueForKeyOfRange( LEOValuePtr self, const char* keyName, LEOValuePtr inValue, size_t startOffset, size_t endOffset, struct LEOContext* inContext );


// Variant-specific replacements for certain instance methods:
void		LEOSetVariantValueAsNumber( LEOValuePtr self, LEONumber inNumber, struct LEOContext* inContext );
void		LEOSetVariantValueAsInteger( LEOValuePtr self, LEOInteger inNumber, struct LEOContext* inContext );
void		LEOSetVariantValueAsString( LEOValuePtr self, const char* inString, size_t inStringLen, struct LEOContext* inContext );
void		LEOSetVariantValueAsBoolean( LEOValuePtr self, bool inBoolean, struct LEOContext* inContext );
void		LEOSetVariantValueRangeAsString( LEOValuePtr self, LEOChunkType inType,
											size_t inRangeStart, size_t inRangeEnd,
											const char* inBuf, struct LEOContext* inContext );
void		LEOSetVariantValuePredeterminedRangeAsString( LEOValuePtr self,
											size_t inRangeStart, size_t inRangeEnd,
											const char* inBuf, struct LEOContext* inContext );
LEOValuePtr	LEOGetStringVariantValueForKey( LEOValuePtr self, const char* keyName, union LEOValue *tempStorage, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext );	// Converts the variant into an array, otherwise references can't change the value of a key (e.g. 'add' command).
void		LEOSetStringVariantValueValueForKey( LEOValuePtr self, const char* keyName, LEOValuePtr inValue, struct LEOContext* inContext );
void		LEOInitNumberVariantValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext );
void		LEOInitIntegerVariantValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext );
void		LEOInitBooleanVariantValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext );
void		LEOInitStringVariantValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext );
void		LEOSetVariantValueAsArray( LEOValuePtr self, struct LEOArrayEntry *inArray, struct LEOContext* inContext );
void		LEOInitArrayVariantValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext );

// Array value-specific:
void		LEOInitArrayValue( struct LEOValueArray* self, struct LEOArrayEntry *inArray, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext );	// Takes over ownership of the array.
void		LEOInitArrayValueCopy( LEOValuePtr self, LEOValuePtr dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext );
void		LEOPutArrayValueIntoValue( LEOValuePtr self, LEOValuePtr dest, struct LEOContext* inContext );
const char*	LEOGetArrayValueAsString( LEOValuePtr self, char* outBuf, size_t bufSize, struct LEOContext* inContext );
void		LEOGetArrayValueAsRangeOfString( LEOValuePtr self, LEOChunkType inType,
												size_t inRangeStart, size_t inRangeEnd,
												char* outBuf, size_t bufSize, struct LEOContext* inContext );
void		LEODetermineChunkRangeOfSubstringOfArrayValue( LEOValuePtr self, size_t *ioBytesStart, size_t *ioBytesEnd,
															size_t *ioBytesDelStart, size_t *ioBytesDelEnd,
															LEOChunkType inType, size_t inRangeStart, size_t inRangeEnd,
															struct LEOContext* inContext );
void		LEOCleanUpArrayValue( LEOValuePtr self, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext );
LEOValuePtr	LEOGetArrayValueValueForKey( LEOValuePtr self, const char* inKey, union LEOValue *tempStorage, LEOKeepReferencesFlag keepReferences, struct LEOContext * inContext );
void		LEOSetArrayValueValueForKey( LEOValuePtr self, const char* inKey, LEOValuePtr inValue, struct LEOContext * inContext );
size_t		LEOGetArrayValueKeyCount( LEOValuePtr self, struct LEOContext* inContext );

void		LEOSetArrayValueAsArray( LEOValuePtr self, struct LEOArrayEntry* inArray, struct LEOContext* inContext );

// Associative arrays:
struct LEOArrayEntry	*	LEOAllocNewEntry( const char* inKey, LEOValuePtr inValue /* may be NULL */, struct LEOContext* inContext );
struct LEOArrayEntry	*	LEOCreateArrayFromString( const char* inString, size_t inStringLen, struct LEOContext* inContext );
LEOValuePtr					LEOAddArrayEntryToRoot( struct LEOArrayEntry** arrayPtrByReference, const char* inKey, LEOValuePtr inValue /* may be NULL */, struct LEOContext* inContext );
void						LEODeleteArrayEntryFromRoot( struct LEOArrayEntry** arrayPtrByReference, const char* inKey, struct LEOContext* inContext );
struct LEOArrayEntry*		LEOCopyArray( struct LEOArrayEntry* arrayPtr, struct LEOContext* inContext );
LEOValuePtr					LEOGetArrayValueForKey( struct LEOArrayEntry* arrayPtr, const char* inKey );
size_t						LEOGetArrayKeyCount( struct LEOArrayEntry* arrayPtr );
void						LEOPrintArray( struct LEOArrayEntry* arrayPtr, char* strBuf, size_t bufSize, struct LEOContext* inContext );
void						LEOCleanUpArray( struct LEOArrayEntry* arrayPtr, struct LEOContext* inContext );

// Convenience wrappers around LEOAddArrayEntryToRoot:
LEOValuePtr	LEOAddIntegerArrayEntryToRoot( struct LEOArrayEntry** arrayPtrByReference, const char* inKey, LEOInteger inNum, struct LEOContext* inContext );
LEOValuePtr	LEOAddNumberArrayEntryToRoot( struct LEOArrayEntry** arrayPtrByReference, const char* inKey, LEONumber inNum, struct LEOContext* inContext );
LEOValuePtr	LEOAddCStringArrayEntryToRoot( struct LEOArrayEntry** arrayPtrByReference, const char* inKey, const char* inCStr, struct LEOContext* inContext );
LEOValuePtr	LEOAddStringArrayEntryToRoot( struct LEOArrayEntry** arrayPtrByReference, const char* inKey, const char* inStr, size_t inLen, struct LEOContext* inContext );

// One array entry:
struct LEOArrayEntry
{
	struct LEOArrayEntry	*	smallerItem;
	struct LEOArrayEntry	*	largerItem;
	union LEOValue				value;
	char						key[0];	// Must be last, dynamically sized array.
};


#endif // LEO_VALUE_H
