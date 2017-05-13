//
//  LEOValue.hpp
//  Stacksmith
//
//  Created by Uli Kusterer on 18/12/2016.
//  Copyright © 2016 Uli Kusterer. All rights reserved.
//

#ifndef LEOValue_hpp
#define LEOValue_hpp

//
//  main.cpp
//  CppVariants
//
//  Created by Uli Kusterer on 13/11/2016.
//  Copyright © 2016 Uli Kusterer. All rights reserved.
//

#include <iostream>
#include <string>
#include <cmath>
#include <sstream>


using namespace std;


#include "LEOChunks.h"
#include "LEOInterpreter.h"


// -----------------------------------------------------------------------------
//	Constants:
// -----------------------------------------------------------------------------

/*! If an object ID is invalid, it is set to this value. */
#define kLEOObjectIDINVALID		0


/*! Flag that decides whether to clear references to this object or keep them.
	This is used e.g. by variants when re-creating a value as a different type
	to make sure that any references to it remain valid, or for returning a result.
	OTOH, you generally want to invalidate references when e.g. creating a stack
	slot to make sure that references from previous iterations notice that while
	the address is identical, the old value is gone. */
enum eLEOKeepReferencesFlag
{
	kLEOInvalidateReferences,	//! Break all references to this slot and make sure we're treated as a fresh object.
	kLEOKeepReferences			//! Keep all references to this slot, we're really just a new value that happens to have a different type.
	
};
typedef int		LEOKeepReferencesFlag;


typedef unsigned long			LEOObjectID;
/*! An Object Seed is a value used internally to allow reference values to detect whether the slot in which their value used to be has already been reused. */
typedef unsigned long			LEOObjectSeed;

/*! The type of integers in the language. This is guaranteed to be signed, but large enough to hold a pointer. */
typedef long long				LEOInteger;

/*! The type of fractional numbers in the language. */
typedef float					LEONumber;


/*! A unit attached to a numerical value. */
typedef uint8_t					LEOUnit;

/*! A group of units. We can convert between numbers of the same group, but not
	between members of different groups. */
typedef uint8_t					LEOUnitGroup;
enum
{
	kLEOUnitGroupNone,	//! A number with no unit attached. This is what you'll generally want.
	kLEOUnitGroupTime,	//! A number that indicates a time interval.
	kLEOUnitGroupBytes,	//! A number that indicates a byte size.
	kLEOUnitGroup_Last
};

#define LEO_UNITS		X4(kLEOUnitNone,"",ELastIdentifier_Sentinel,kLEOUnitGroupNone) \
						X4(kLEOUnitTicks," ticks",ETicksIdentifier,kLEOUnitGroupTime) \
						X4(kLEOUnitSeconds," seconds",ESecondsIdentifier,kLEOUnitGroupTime) \
						X4(kLEOUnitMinutes," minutes",EMinutesIdentifier,kLEOUnitGroupTime) \
						X4(kLEOUnitHours," hours",EHoursIdentifier,kLEOUnitGroupTime) \
						X4(kLEOUnitBytes," bytes",EBytesIdentifier,kLEOUnitGroupBytes) \
						X4(kLEOUnitKilobytes," kilobytes",EKilobytesIdentifier,kLEOUnitGroupBytes) \
						X4(kLEOUnitMegabytes," megabytes",EMegabytesIdentifier,kLEOUnitGroupBytes) \
						X4(kLEOUnitGigabytes," gigabytes",EGigabytesIdentifier,kLEOUnitGroupBytes) \
						X4(kLEOUnitTerabytes," terabytes",ETerabytesIdentifier,kLEOUnitGroupBytes)

enum
{
#define X4(constName,stringSuffix,identifierSubtype,unitGroup)	constName,
	LEO_UNITS
#undef X4
	kLEOUnit_Last
};


extern const char*	gUnitLabels[kLEOUnit_Last +1];	// first item is none (empty string), last is a NULL (hence +1). Rest is suffixes for each unit.
extern LEOUnitGroup	gUnitGroupsForLabels[kLEOUnit_Last +1];

struct LEOContext;
struct LEOArrayEntry;


class LEOValue;


class CppVariantBase
{
public:
	CppVariantBase() { cout << "CppVariantBase constructor." << endl;  }
	virtual ~CppVariantBase() { cout << "CppVariantBase destructor." << endl; }
	
	virtual const string	GetDisplayTypeName() = 0;	//! Used for error messages etc. Doesn't distinguish between dynamic and constant strings.
	
	virtual LEONumber		GetAsNumber( LEOUnit *outUnit, struct LEOContext* inContext ) = 0;
	virtual LEOInteger		GetAsInteger( LEOUnit *outUnit, struct LEOContext* inContext ) = 0;
	virtual const string	GetAsString( struct LEOContext* inContext ) = 0;
	virtual bool			GetAsBoolean( struct LEOContext* inContext ) = 0;
	virtual void			GetAsRangeOfString( LEOChunkType inType,
											size_t inRangeStart, size_t inRangeEnd,
											string& outBuf, struct LEOContext* inContext );
	
	virtual void		SetAsNumber( LEONumber inNumber, LEOUnit inUnit, struct LEOContext* inContext ) = 0;
	virtual void		SetAsInteger( LEOInteger inNumber, LEOUnit inUnit, struct LEOContext* inContext ) = 0;
	virtual void		SetAsString( const string inBuf, struct LEOContext* inContext ) = 0;
	virtual void		SetAsBoolean( bool inBoolean, struct LEOContext* inContext ) = 0;
	virtual void		SetRangeAsString( LEOChunkType inType,
									size_t inRangeStart, size_t inRangeEnd,
									const string inBuf, struct LEOContext* inContext ) = 0;
	virtual void		SetPredeterminedRangeAsString(
									size_t inRangeStart, size_t inRangeEnd,
									const string inBuf, struct LEOContext* inContext ) = 0;
	
	virtual void		InitCopy( LEOValue& dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext ) = 0;	//! dest is an uninitialized value.
	virtual void		InitSimpleCopy( LEOValue& dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext ) = 0;	//! dest is an uninitialized value.
	virtual void		PutValueIntoValue( LEOValue& dest, struct LEOContext* inContext ) = 0;	//! dest must be a VALID, initialized value!
	template<class T>
	CppVariantBase*		FollowReferencesAndReturnValueOfType( struct LEOContext* inContext );	// Returns NULL if no direct reference.
	CppVariantBase*		FollowReferencesAndReturnValue( struct LEOContext* inContext );	// Returns NULL if no direct reference.
	
	virtual void		DetermineChunkRangeOfSubstring( size_t *ioBytesStart, size_t *ioBytesEnd,
													size_t *ioBytesDelStart, size_t *ioBytesDelEnd,
													LEOChunkType inType, size_t inRangeStart, size_t inRangeEnd,
													struct LEOContext* inContext );
	virtual void		CleanUp( LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
	{
		((CppVariantBase*)dest.mBuf)->~CppVariantBase(); new (dest.mBuf) CppVariantInteger( 0, kLEOUnitNone ); if( keepReferences == kLEOInvalidateReferences ) dest.refObjectID = kLEOObjectIDINVALID;
	}

	virtual bool		CanGetAsNumber( struct LEOContext* inContext ) = 0;
	
	virtual LEOValue&	GetValueForKey( const string keyName, LEOValue& tempStorage, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext ) = 0;
	virtual void		SetValueForKey( const string keyName, LEOValue& inValue, struct LEOContext* inContext ) = 0;
	
	virtual void		SetValueAsArray( struct LEOArrayEntry* inArray, struct LEOContext* inContext ) = 0;
	
	virtual size_t		GetKeyCount( struct LEOContext* inContext ) = 0;
	
	virtual void		GetValueForKeyOfRange( const string keyName, size_t startOffset, size_t endOffset, LEOValue& outValue, struct LEOContext* inContext ) = 0;
	virtual void		SetValueForKeyOfRange( const string keyName, LEOValue& inValue, size_t startOffset, size_t endOffset, struct LEOContext* inContext ) = 0;
	
	virtual void		SetValueAsNativeObject( void* inNativeObject, struct LEOContext* inContext ) = 0;
	
	virtual void		SetValueAsRect( LEOInteger l, LEOInteger t, LEOInteger r, LEOInteger b, struct LEOContext* inContext ) = 0;
	virtual void		GetValueAsRect( LEOInteger *l, LEOInteger *t, LEOInteger *r, LEOInteger *b, struct LEOContext* inContext ) = 0;
	virtual void		SetValueAsPoint( LEOInteger l, LEOInteger t, struct LEOContext* inContext ) = 0;
	virtual void		GetValueAsPoint( LEOInteger *l, LEOInteger *t, struct LEOContext* inContext ) = 0;
	virtual bool		GetValueIsUnset( struct LEOContext* inContext ) = 0;
	virtual void		SetValueAsRange( LEOInteger s, LEOInteger e, LEOChunkType t, struct LEOContext* inContext ) = 0;
	virtual void		GetValueAsRange( LEOInteger *s, LEOInteger *e, LEOChunkType *t, struct LEOContext* inContext ) = 0;
};


class CppVariantInteger : public CppVariantBase
{
public:
	explicit CppVariantInteger( LEOInteger n = 0, LEOUnit u = kLEOUnitNone )
		: mInteger(n), mUnit(u)
	{
		cout << "CppVariantInteger constructor." << endl;
	}
	~CppVariantInteger() { cout << "CppVariantInteger destructor." << endl; }

	virtual const string	GetDisplayTypeName() { return "integer"; }
	
	virtual LEONumber		GetAsNumber( LEOUnit *outUnit, struct LEOContext* inContext ) { *outUnit = mUnit; return mInteger; }
	virtual LEOInteger		GetAsInteger( LEOUnit *outUnit, struct LEOContext* inContext ) { *outUnit = mUnit; return mInteger; }
	virtual const string	GetAsString( struct LEOContext* inContext ) { return to_string(mInteger); }
	virtual bool			GetAsBoolean( struct LEOContext* inContext );
	
	virtual void		SetAsNumber( LEONumber inNumber, LEOUnit inUnit, struct LEOContext* inContext ) = 0;
	virtual void		SetAsInteger( LEOInteger inNumber, LEOUnit inUnit, struct LEOContext* inContext ) { mInteger = inNumber; mUnit = inUnit; }
	virtual void		SetAsString( const string inBuf, struct LEOContext* inContext );
	virtual void		SetAsBoolean( bool inBoolean, struct LEOContext* inContext ) = 0;
	virtual void		SetRangeAsString( LEOChunkType inType,
									size_t inRangeStart, size_t inRangeEnd,
									const string inBuf, struct LEOContext* inContext ) = 0;
	virtual void		SetPredeterminedRangeAsString(
									size_t inRangeStart, size_t inRangeEnd,
									const string inBuf, struct LEOContext* inContext ) = 0;
	
	virtual void		InitCopy( LEOValue& dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext ) { ((CppVariantBase*)dest.mBuf)->~CppVariantBase(); new (dest.mBuf) CppVariantInteger( mInteger, mUnit ); if( keepReferences == kLEOInvalidateReferences ) dest.refObjectID = kLEOObjectIDINVALID; }
	virtual void		InitSimpleCopy( LEOValue& dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext ) { ((CppVariantBase*)dest.mBuf)->~CppVariantBase(); new (dest.mBuf) CppVariantInteger( mInteger, mUnit ); if( keepReferences == kLEOInvalidateReferences ) dest.refObjectID = kLEOObjectIDINVALID; }
	virtual void		PutValueIntoValue( LEOValue& dest, struct LEOContext* inContext ) { dest.SetAsInteger( mInteger, mUnit ); }
	
	virtual void		CleanUp( LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext ) = 0;

	virtual bool		CanGetAsNumber( struct LEOContext* inContext ) { return true; }
	
	virtual LEOValue&	GetValueForKey( const string keyName, LEOValue& tempStorage, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext ) = 0;
	virtual void		SetValueForKey( const string keyName, LEOValue& inValue, struct LEOContext* inContext ) = 0;
	
	virtual void		SetValueAsArray( struct LEOArrayEntry* inArray, struct LEOContext* inContext ) = 0;
	
	virtual size_t		GetKeyCount( struct LEOContext* inContext ) = 0;
	
	virtual void		GetValueForKeyOfRange( const string keyName, size_t startOffset, size_t endOffset, LEOValue& outValue, struct LEOContext* inContext ) = 0;
	virtual void		SetValueForKeyOfRange( const string keyName, LEOValue& inValue, size_t startOffset, size_t endOffset, struct LEOContext* inContext ) = 0;
	
	virtual void		SetValueAsNativeObject( void* inNativeObject, struct LEOContext* inContext ) = 0;
	
	virtual void		SetValueAsRect( LEOInteger l, LEOInteger t, LEOInteger r, LEOInteger b, struct LEOContext* inContext ) = 0;
	virtual void		GetValueAsRect( LEOInteger *l, LEOInteger *t, LEOInteger *r, LEOInteger *b, struct LEOContext* inContext ) = 0;
	virtual void		SetValueAsPoint( LEOInteger l, LEOInteger t, struct LEOContext* inContext ) = 0;
	virtual void		GetValueAsPoint( LEOInteger *l, LEOInteger *t, struct LEOContext* inContext ) = 0;
	virtual bool		GetValueIsUnset( struct LEOContext* inContext ) = 0;
	virtual void		SetValueAsRange( LEOInteger s, LEOInteger e, LEOChunkType t, struct LEOContext* inContext ) = 0;
	virtual void		GetValueAsRange( LEOInteger *s, LEOInteger *e, LEOChunkType *t, struct LEOContext* inContext ) = 0;
	
protected:
	LEOInteger	mInteger;
	LEOUnit		mUnit;
};


class CppVariantNumber : public CppVariantBase
{
public:
	CppVariantNumber( double n = 0 ) : mDouble(n)
	{
		cout << "CppVariantNumber constructor." << endl;
	}
	~CppVariantNumber()
	{
		cout << "CppVariantNumber destructor." << endl;
	}

	virtual int		GetAsInt()
	{
		if( int(mDouble) == mDouble )
			return mDouble;
		else
			return round(mDouble);
	}
	virtual double	GetAsDouble()			{ return mDouble; }
	
	virtual void	SetAsDouble( double n )	{ mDouble = n; }

protected:
	double mDouble;
};


class CppVariantReference : public CppVariantBase
{
public:
	virtual CppVariantBase*	FollowReferencesAndReturnValue( struct LEOContext* inContext );

protected:
	LEOObjectID			mObjectID;
	LEOObjectSeed		mObjectSeed;
	LEOChunkType		mChunkType;
	size_t				mChunkStart = 0;
	size_t				mChunkEnd = 0;
};


union LEOValueUnion { CppVariantInteger a; CppVariantNumber b; };

class LEOValue
{
public:
	explicit LEOValue( int n = 0, LEOUnit inUnit = kLEOUnitNone )	{ new (mBuf) CppVariantInteger(n,inUnit); }
	explicit LEOValue( double n, LEOUnit inUnit = kLEOUnitNone )	{ new (mBuf) CppVariantNumber(n,inUnit); }
	~LEOValue()						{ ((CppVariantBase*)mBuf)->~CppVariantBase(); }
	
	operator CppVariantBase* () { return (CppVariantBase*)mBuf; }
	CppVariantBase* operator -> () { return (CppVariantBase*)mBuf; }

protected:
	uint8_t			mBuf[sizeof(LEOValueUnion)];
	LEOObjectID		refObjectID = 0;	//! If we have a reference to us, this is our index in the LEOContextGroup's reference table, so we can clear it on destruction.
	
	friend class CppVariantBase;
};


void	MakeMacros( size_t maxNumArgs );


#endif /* LEOValue_hpp */
