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
	
	A LEOValue is essentially a base class (with subclasses) implemented in
	straight C. Each LEOValue has an "isa" that points to the method table.
	This way, you can e.g. run the InitCopy method on any LEOValue without
	having to know which "subclass" you are dealing with.
	
	LEOValues are how variables and stack entries are implemented in the LEO
	runtime. They can have various types, and most types can be transparently
	converted into each other.
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
#define LEOObjectIDINVALID		ULONG_MAX


// -----------------------------------------------------------------------------
//	Types:
// -----------------------------------------------------------------------------

// A LEOValueTypePtr is a pointer to a method look-up table for an object used internally.
typedef struct LEOValueType *	LEOValueTypePtr;
/*! A LEOValuePtr is a generic pointer to a LEOValue or one of its subclasses. */
typedef struct LEOValueBase	*	LEOValuePtr;
// Object IDs are indexes used internally by reference values to look up the referenced value in a way that's safe even after the original has been disposed of.
typedef unsigned long			LEOObjectID;
// An Object Seed is a value used internally to allow reference values to detect whether the slot in which their value used to be has already been reused.
typedef unsigned long			LEOObjectSeed;


struct LEOContext;


// Layout of the virtual function tables:
struct LEOValueType
{
	const char*	displayTypeName;	// Used for error messages etc. Doesn't distinguish between dynamic and constant strings.
	size_t		size;				// Minimal size required for a variable of this type.
	
	double		(*GetAsNumber)( LEOValuePtr self, struct LEOContext* inContext );
	void		(*GetAsString)( LEOValuePtr self, char* outBuf, long bufSize, struct LEOContext* inContext );
	bool		(*GetAsBoolean)( LEOValuePtr self, struct LEOContext* inContext );
	void		(*GetAsRangeOfString)( LEOValuePtr self, LEOChunkType inType,
											size_t inRangeStart, size_t inRangeEnd,
											char* outBuf, long bufSize, struct LEOContext* inContext );
	
	void		(*SetAsNumber)( LEOValuePtr self, double inNumber, struct LEOContext* inContext );
	void		(*SetAsString)( LEOValuePtr self, const char* inBuf, struct LEOContext* inContext );
	void		(*SetAsBoolean)( LEOValuePtr self, bool inBoolean, struct LEOContext* inContext );
	void		(*SetRangeAsString)( LEOValuePtr self, LEOChunkType inType,
									size_t inRangeStart, size_t inRangeEnd,
									const char* inBuf, struct LEOContext* inContext );
	void		(*InitCopy)( LEOValuePtr self, LEOValuePtr dest, struct LEOContext* inContext );	// dest is an uninitialized value.
	
	void		(*CleanUp)( LEOValuePtr self, struct LEOContext* inContext );
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
extern struct LEOValueType	kLeoValueTypeString;
extern struct LEOValueType	kLeoValueTypeStringConstant;
extern struct LEOValueType	kLeoValueTypeBoolean;
extern struct LEOValueType	kLeoValueTypeReference;


// -----------------------------------------------------------------------------
//	IVar layout for LEOValueBase subclasses:
// -----------------------------------------------------------------------------

/*!
	A number, we currently only do floating point values:
	@field	base	The instance variables inherited from the base class.
	@field	number	Where we store the actual number.
*/
struct LEOValueNumber
{
	struct LEOValueBase	base;
	double				number;
};
typedef struct LEOValueNumber	LEOValueNumber;


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
*/
struct LEOValueReference
{
	struct LEOValueBase	base;
	LEOObjectID			objectID;
	LEOObjectSeed		objectSeed;
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
	struct LEOValueString		string;
	struct LEOValueBoolean		boolean;
	struct LEOValueReference	reference;
	struct LEOValueArray		array;
};


// -----------------------------------------------------------------------------
//	IVar layout for LEOValueBase subclasses:
// -----------------------------------------------------------------------------

/*! @functiongroup Constructors & storage space measuring */
/*!
	Initialize the given storage so it's a valid number value containing the
	given number.
*/
void		LEOInitNumberValue( LEOValuePtr inStorage, double inNumber, struct LEOContext *inContext );

/*!
	Initialize the given storage so it's a valid string value containing a copy
	of the given string.
*/
void		LEOInitStringValue( LEOValuePtr inStorage, const char* inString, struct LEOContext *inContext );

/*!
	Initialize the given storage so it's a valid string constant value directly
	referencing the given string. The caller is responsible for ensuring that
	the string is not disposed of while the value still needs it. 
*/
void		LEOInitStringConstantValue( LEOValuePtr inStorage, const char* inString, struct LEOContext *inContext );

/*!
	Initialize the given storage so it's a valid boolean value containing the
	given boolean.
*/
void		LEOInitBooleanValue( LEOValuePtr inStorage, bool inBoolean, struct LEOContext *inContext );

/*!
	Initialize the given storage so it's a valid reference value that
	points to the given original value. If the original value is destructed
	while a reference still points to it, method calls to such a reference will
	fail with an error message and abort execution of the current LEOContext.
	
	However, the destructor of the value is safe to call, as is InitCopy.
*/
void		LEOInitReferenceValue( LEOValuePtr self, LEOValuePtr dest, struct LEOContext* inContext );


/*! @functiongroup LEOValue storage measuring */
/*!
	@function LEOGetValueSize
	Returns the size (in bytes) of storage you need to provide to LEOInitCopy for
	this object.
	@param	v	The value whose size you want to measure.
*/
#define		LEOGetValueSize(v)					(((LEOValuePtr)(v))->isa->size)

/*!
	@function LEOGetNumberValueSize
	Returns the size (in bytes) of storage you need to provide to LEOInitCopy for
	a new number value.
*/
#define		LEOGetNumberValueSize()				(kLeoValueTypeNumber.size)		

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

// Convenience macros for calling virtual methods w/o knowing what object it is:
/*! @functiongroup LEOValue methods */

/*!
	@function LEOGetValueAsNumber
	Returns the given value as a <tt>double</tt>, converting it, if necessary.
	If conversion isn't possible, it will fail with an error message and stop
	execution in the current LEOContext.
	@param	v	The value you wish to read.
	@param	c	The context in which your script is currently running and in
				which errors will be stored.
*/
#define 	LEOGetValueAsNumber(v,c)		((LEOValuePtr)(v))->isa->GetAsNumber(((LEOValuePtr)(v)),(c))

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
*/
#define 	LEOGetValueAsString(v,s,l,c)	((LEOValuePtr)(v))->isa->GetAsString(((LEOValuePtr)(v)),(s),(l),(c))

/*!
	@function LEOGetValueAsBoolean
	Returns the given value as a <tt>bool</tt>, converting it, if necessary.
	If conversion isn't possible, it will fail with an error message and stop
	execution in the current LEOContext.
	@param	v	The value you wish to read.
	@param	c	The context in which your script is currently running and in
				which errors will be stored.
*/
#define 	LEOGetValueAsBoolean(v,c)		((LEOValuePtr)(v))->isa->GetAsBoolean(((LEOValuePtr)(v)),(c))

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
#define 	LEOGetValueAsRangeOfString(v,t,rs,re,s,sl,c)	((LEOValuePtr)(v))->isa->GetAsRangeOfString(((LEOValuePtr)(v)),(t),(rs),(re),(s),(sl),(c))


/*!
	@function LEOSetValueAsNumber
	Assigns the given number to the value, converting it, if necessary.
	If conversion isn't possible, it will fail with an error message and stop
	execution in the current LEOContext.
	@param	v	The value you wish to change.
	@param	n	The number to write to value <tt>v</tt>.
	@param	c	The context in which your script is currently running and in
				which errors will be stored.
*/
#define 	LEOSetValueAsNumber(v,n,c)		((LEOValuePtr)(v))->isa->SetAsNumber(((LEOValuePtr)(v)),(n),(c))

/*!
	@function LEOSetValueAsString
	Assigns the given C-string to the value, converting it, if necessary.
	If conversion isn't possible, it will fail with an error message and stop
	execution in the current LEOContext.
	@param	v	The value you wish to change.
	@param	s	The zero-terminated C-String to write to value <tt>v</tt>, as a <tt>char*</tt>.
	@param	c	The context in which your script is currently running and in
				which errors will be stored.
*/
#define 	LEOSetValueAsString(v,s,c)		((LEOValuePtr)(v))->isa->SetAsString(((LEOValuePtr)(v)),(s),(c))

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
#define 	LEOSetValueAsBoolean(v,n,c)		((LEOValuePtr)(v))->isa->SetAsBoolean(((LEOValuePtr)(v)),(n),(c))

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
#define 	LEOSetValueRangeAsString(v,t,rs,re,s,c)	((LEOValuePtr)(v))->isa->SetRangeAsString(((LEOValuePtr)(v)),(t),(rs),(re),(s),(c))


/*!
	@function LEOInitCopy
	Initializes the given storage to be an exact copy of the given value. If the
	value is a reference, you will get a second reference to the original value,
	not a copy of the original value, and not a reference to the reference.
	@param	v	The value you wish to copy.
	@param	d	A LEOValuePtr to memory where you wish the copy to be stored.
				This destination must be large enough to hold the given type of
				value.
	@param	c	The context in which your script is currently running and in
				which errors will be stored.
	@seealso //apple_ref/c/func/LEOGetValueSize LEOGetValueSize
*/
#define 	LEOInitCopy(v,d,c)				((LEOValuePtr)(v))->isa->InitCopy(((LEOValuePtr)(v)),((LEOValuePtr)(d)),(c))


/*!
	@function LEOCleanUpValue
	Dispose of any additional storage the value has allocated to hold its data.
	If there are references to this value, this will cause the references to fail
	with an error when next they try to access this value.
	@param	v	The value you wish to dispose of.
*/
#define 	LEOCleanUpValue(v,c)			((LEOValuePtr)(v))->isa->CleanUp(((LEOValuePtr)(v)),(c))


// Failure indicators we re-use in many places:
double		LEOCantGetValueAsNumber( LEOValuePtr self, struct LEOContext* inContext );
bool		LEOCantGetValueAsBoolean( LEOValuePtr self, struct LEOContext* inContext );
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
void		LEOInitNumberValueCopy( LEOValuePtr self, LEOValuePtr dest, struct LEOContext* inContext );
void		LEOCleanUpNumberValue( LEOValuePtr self, struct LEOContext* inContext );

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
void		LEOInitStringValueCopy( LEOValuePtr self, LEOValuePtr dest, struct LEOContext* inContext );
void		LEOCleanUpStringValue( LEOValuePtr self, struct LEOContext* inContext );

// Replacement assignment methods and destructors for constant-referencing strings:
void		LEOSetStringConstantValueAsNumber( LEOValuePtr self, double inNumber, struct LEOContext* inContext );		// Makes it a dynamically allocated string.
void		LEOSetStringConstantValueAsString( LEOValuePtr self, const char* inString, struct LEOContext* inContext );	// Makes it a dynamically allocated string.
void		LEOSetStringConstantValueAsBoolean( LEOValuePtr self, bool inBoolean, struct LEOContext* inContext );
void		LEOSetStringConstantValueRangeAsString( LEOValuePtr self, LEOChunkType inType,
												size_t inRangeStart, size_t inRangeEnd,
												const char* inBuf, struct LEOContext* inContext );						// Makes it a dynamically allocated string.
void		LEOInitStringConstantValueCopy( LEOValuePtr self, LEOValuePtr dest, struct LEOContext* inContext );
void		LEOCleanUpStringConstantValue( LEOValuePtr self, struct LEOContext* inContext );

// Boolean instance methods:
void		LEOGetBooleanValueAsString( LEOValuePtr self, char* outBuf, long bufSize, struct LEOContext* inContext );
bool		LEOGetBooleanValueAsBoolean( LEOValuePtr self, struct LEOContext* inContext );
void		LEOSetBooleanValueAsString( LEOValuePtr self, const char* inString, struct LEOContext* inContext );
void		LEOSetBooleanValueAsBoolean( LEOValuePtr self, bool inBoolean, struct LEOContext* inContext );
void		LEOInitBooleanValueCopy( LEOValuePtr self, LEOValuePtr dest, struct LEOContext* inContext );
void		LEOCleanUpBooleanValue( LEOValuePtr self, struct LEOContext* inContext );


// Reference instance methods:
void		LEOGetReferenceValueAsString( LEOValuePtr self, char* outBuf, long bufSize, struct LEOContext* inContext );
double		LEOGetReferenceValueAsNumber( LEOValuePtr self, struct LEOContext* inContext );
bool		LEOGetReferenceValueAsBoolean( LEOValuePtr self, struct LEOContext* inContext );
void		LEOGetReferenceValueAsRangeOfString( LEOValuePtr self, LEOChunkType inType,
											size_t inRangeStart, size_t inRangeEnd,
											char* outBuf, long bufSize, struct LEOContext* inContext );
void		LEOSetReferenceValueAsString( LEOValuePtr self, const char* inString, struct LEOContext* inContext );
void		LEOSetReferenceValueAsBoolean( LEOValuePtr self, bool inBoolean, struct LEOContext* inContext );
void		LEOSetReferenceValueAsNumber( LEOValuePtr self, double inNumber, struct LEOContext* inContext );
void		LEOSetReferenceValueRangeAsString( LEOValuePtr self, LEOChunkType inType,
											size_t inRangeStart, size_t inRangeEnd,
											const char* inBuf, struct LEOContext* inContext );
void		LEOInitReferenceValueCopy( LEOValuePtr self, LEOValuePtr dest, struct LEOContext* inContext );
void		LEOCleanUpReferenceValue( LEOValuePtr self, struct LEOContext* inContext );



#endif // LEO_VALUE_H
