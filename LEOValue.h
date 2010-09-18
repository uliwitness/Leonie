/*
 *  LEOValue.h
 *  Leonie
 *
 *  Created by Uli Kusterer on 06.09.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include <limits.h>

#define LEOObjectIDINVALID		ULONG_LONG_MAX

typedef unsigned long long		LEOObjectID;
typedef struct LEOValueType *	LEOValueTypePtr;
typedef struct LEOValueBase	*	LEOValuePtr;


struct LEOValueType
{
	double		(*GetAsNumber)( LEOValuePtr self );
	void		(*GetAsString)( LEOValuePtr self, char* outBuf, long bufSize );
	LEOObjectID	(*GetAsObjectID)( LEOValuePtr self );
	
	void		(*SetAsNumber)( LEOValuePtr self, double inNumber );
	void		(*SetAsString)( LEOValuePtr self, const char* inBuf );
	void		(*SetAsObjectID)( LEOValuePtr self, LEOObjectID inObjectID );
	
	void		(*CleanUp)( LEOValuePtr self );
};


struct LEOValueBase
{
	LEOValueTypePtr		isa;
};


extern struct LEOValueType	kLeoValueTypeNumber;
extern struct LEOValueType	kLeoValueTypeString;
extern struct LEOValueType	kLeoValueTypeStringConstant;


// -----------------------------------------------------------------------------
//	IVar layout for LEOValueBase subclasses:
// -----------------------------------------------------------------------------

struct LEOValueNumber
{
	struct LEOValueBase	base;
	double				number;
};
typedef struct LEOValueNumber	LEOValueNumber;


struct LEOValueString
{
	struct LEOValueBase	base;
	char*				string;
};
typedef struct LEOValueString	LEOValueString;


struct LEOValueObject
{
	struct LEOValueBase	base;
	LEOObjectID			objectID;
};
typedef struct LEOValueObject	LEOValueObject;


// -----------------------------------------------------------------------------
//	IVar layout for LEOValueBase subclasses:
// -----------------------------------------------------------------------------

#define		LEOGetNumberValueSize(n)			(sizeof(struct LEOValueNumber))		
void		LEOInitNumberValue( LEOValuePtr inStorage, double inNumber );

#define		LEOGetStringValueSize(n)			(sizeof(struct LEOValueString))		
void		LEOInitStringValue( LEOValuePtr inStorage, const char* inString );

#define		LEOGetStringConstantValueSize(n)	(sizeof(struct LEOValueString))		
void		LEOInitStringConstantValue( LEOValuePtr inStorage, const char* inString );


#define 	LEOGetValueAsNumber(v)		((LEOValuePtr)v)->isa->GetAsNumber(((LEOValuePtr)v))
#define 	LEOGetValueAsString(v,s,l)	((LEOValuePtr)v)->isa->GetAsString(((LEOValuePtr)v),(s),(l))
#define 	LEOGetValueAsObjectID(v)	((LEOValuePtr)v)->isa->GetAsObjectID(((LEOValuePtr)v))

#define 	LEOSetValueAsNumber(v,n)	((LEOValuePtr)v)->isa->SetAsNumber(((LEOValuePtr)v),(n))
#define 	LEOSetValueAsString(v,s)	((LEOValuePtr)v)->isa->SetAsString(((LEOValuePtr)v),(s))
#define 	LEOSetValueAsObjectID(v,i)	((LEOValuePtr)v)->isa->SetAsObjectID(((LEOValuePtr)v),(i))

#define 	LEOCleanUpValue(v)			((LEOValuePtr)v)->isa->CleanUp(((LEOValuePtr)v))


LEOObjectID	LEOCantGetValueAsObjectID( LEOValuePtr self );
void		LEOCantSetValueAsObjectID( LEOValuePtr self, LEOObjectID inObjectID );
double		LEOGetNumberValueAsNumber( LEOValuePtr self );
void		LEOGetNumberValueAsString( LEOValuePtr self, char* outBuf, long bufSize );
void		LEOSetNumberValueAsNumber( LEOValuePtr self, double inNumber );
void		LEOSetNumberValueAsString( LEOValuePtr self, const char* inNumber );
void		LEOCleanUpNumberValue( LEOValuePtr self );

double		LEOGetStringValueAsNumber( LEOValuePtr self );
void		LEOGetStringValueAsString( LEOValuePtr self, char* outBuf, long bufSize );
void		LEOSetStringValueAsNumber( LEOValuePtr self, double inNumber );
void		LEOSetStringValueAsString( LEOValuePtr self, const char* inString );
void		LEOCleanUpStringValue( LEOValuePtr self );

void		LEOSetStringConstantValueAsNumber( LEOValuePtr self, double inNumber );
void		LEOSetStringConstantValueAsString( LEOValuePtr self, const char* inString );
void		LEOCleanUpStringConstantValue( LEOValuePtr self );


