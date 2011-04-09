/*
 *  LEOContextGroup.h
 *  Leonie
 *
 *  Created by Uli Kusterer on 27.10.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

/*!
	@header LEOContextGroup
	A context group is a crude, basic security mechanism that can insulate
	scripts running in different contexts from each other. Every context belongs
	to a group, which holds global data shared by all its contexts, which
	includes safe references to objects and values that may go away.
*/

#ifndef LEO_CONTEXT_GROUP_H
#define LEO_CONTEXT_GROUP_H		1

// -----------------------------------------------------------------------------
//	Headers:
// -----------------------------------------------------------------------------

#include "LEOValue.h"
#include "LEOHandlerID.h"


// -----------------------------------------------------------------------------
//	Types:
// -----------------------------------------------------------------------------

typedef struct LEOObject LEOObject;


/*! All LEOContexts belong to a Context group that contains references and other
	global data they share. You can insulate running scripts from each other by
	placing them in a different context group.
	@field	referenceCount		Reference count for this object, i.e. number of contexts still attached to this object.
	@field	numReferences		Number of items in the <tt>references</tt> array.
	@field	references			An array of "master pointers" to values to which references have been created.
	@seealso //leo_ref/c/func/LEOContextGroupCreate LEOContextGroupCreate
*/
typedef struct LEOContextGroup
{
	size_t			referenceCount;		// Reference count for this object, i.e. number of contexts still attached to this object.
	LEOHandlerCount	numHandlerNames;	// Number of slots in handlerNames array.
	char**			handlerNames;		// Array of handler names. The indexes into this array are 'handler IDs' used throughout the bytecode.
	size_t			numReferences;		// Available slots in "references" array.
	LEOObject		*references;		// "Master pointer" table for references so we can detect when a reference goes away.
} LEOContextGroup;


/*!
	Creates a context group for use with LEOContexts. The LEOContextGroup* is
	reference-counted and its reference count is set to 1, so when you're done
	with it, you can release it. When a context is attached to a context group,
	the context retains its group to ensure it doesn't go away.
	@seealso //leo_ref/c/func/LEOInitContext	LEOInitContext
	@seealso //leo_ref/c/func/LEOContextGroupRetain LEOContextGroupRetain
	@seealso //leo_ref/c/func/LEOContextGroupRelease LEOContextGroupRelease
*/
LEOContextGroup*	LEOContextGroupCreate();	// Gives referenceCount of 1.


/*!
	Acquire ownership of the given context group, so that when the current owner
	releases it the object still stays around for your use. Increases the
	reference count by 1.
	@result Returns inGroup so you can assign it to a variable in a struct,
			or a global, or whatever makes sense.
	@seealso //leo_ref/c/func/LEOContextGroupCreate LEOContextGroupCreate
	@seealso //leo_ref/c/func/LEOContextGroupRelease LEOContextGroupRelease
*/
LEOContextGroup*	LEOContextGroupRetain( LEOContextGroup* inGroup );		// Adds 1 to referenceCount. Returns inGroup.

/*!
	Give up ownership of the given context group. You acquire ownership by either
	creating the context group, or by retaining it. Giving up ownership decreases
	its reference count by 1. When the last owner releases the object (and the
	reference count reaches 0), the context group is freed.
	@seealso //leo_ref/c/func/LEOContextGroupCreate LEOContextGroupCreate
	@seealso //leo_ref/c/func/LEOContextGroupRetain LEOContextGroupRetain
*/
void	LEOContextGroupRelease( LEOContextGroup* inGroup );	// Subtracts 1 from referenceCount. If it hits 0, disposes of inScript.


// Used to implement references to values that can disappear:
/*!
	Create a new object ID that can be used to reference the given pointer.
	@seealso //leo_ref/c/func/LEOContextGroupGetSeedForObjectID LEOContextGroupGetSeedForObjectID
	@seealso //leo_ref/c/func/LEOContextGroupRecycleObjectID LEOContextGroupRecycleObjectID
	@seealso //leo_ref/c/func/LEOContextGroupGetPointerForObjectIDAndSeed LEOContextGroupGetPointerForObjectIDAndSeed
*/
LEOObjectID		LEOContextGroupCreateNewObjectIDForPointer( LEOContextGroup* inContext, void* theValue );

/*! Get the seed for the given object ID. References to an object ID must store
	this seed to ensure they are getting the actual pointer they originally
	wanted to reference and not a pointer that is in there because the slot has
	been reused. Use this to get the initial value for storing the seed along
	with an object ID, not to retrieve the seed for an object ID you've stashed
	away alone.
	@seealso //leo_ref/c/func/LEOContextGroupCreateNewObjectIDForPointer LEOContextGroupCreateNewObjectIDForPointer
	@seealso //leo_ref/c/func/LEOContextGroupRecycleObjectID LEOContextGroupRecycleObjectID
	@seealso //leo_ref/c/func/LEOContextGroupGetPointerForObjectIDAndSeed LEOContextGroupGetPointerForObjectIDAndSeed
*/
LEOObjectSeed	LEOContextGroupGetSeedForObjectID( LEOContextGroup* inContext, LEOObjectID inID );

/*! If a pointer to which references exist is going away, it needs to unregister
	using this call. Anyone requesting the pointer for this object ID and seed
	will from then on get NULL back.
	@seealso //leo_ref/c/func/LEOContextGroupCreateNewObjectIDForPointer LEOContextGroupCreateNewObjectIDForPointer
	@seealso //leo_ref/c/func/LEOContextGroupGetSeedForObjectID LEOContextGroupGetSeedForObjectID
	@seealso //leo_ref/c/func/LEOContextGroupGetPointerForObjectIDAndSeed LEOContextGroupGetPointerForObjectIDAndSeed
*/
void	LEOContextGroupRecycleObjectID( LEOContextGroup* inContext, LEOObjectID inObjectID );

/*! Obtain the pointer that corresponds to the given object ID and seed pair. If
	the given pointer has already been recycled, this will return NULL.
	@seealso //leo_ref/c/func/LEOContextGroupCreateNewObjectIDForPointer LEOContextGroupCreateNewObjectIDForPointer
	@seealso //leo_ref/c/func/LEOContextGroupGetSeedForObjectID LEOContextGroupGetSeedForObjectID
	@seealso //leo_ref/c/func/LEOContextGroupRecycleObjectID LEOContextGroupRecycleObjectID
*/
void*	LEOContextGroupGetPointerForObjectIDAndSeed( LEOContextGroup* inContext, LEOObjectID inObjectID, LEOObjectSeed inObjectSeed );


/*!
	Convert the provided handler-name into a LEOHandlerID. All different spellings
	of the (case-insensitive) handler name map to the same handler ID.
	@seealso //leo_ref/c/func/LEOContextGroupHandlerNameForHandlerID LEOContextGroupHandlerNameForHandlerID
*/
LEOHandlerID	LEOContextGroupHandlerIDForHandlerName( LEOContextGroup* inContext, const char* handlerName );

/*!
	Convert the provided LEOHandlerID back into a handler name. Note that, since
	handler names are case insensitive, you may get a different string than you
	originally passed to <tt>LEOContextGroupHandlerIDForHandlerName</tt>.
	@seealso //leo_ref/c/func/LEOContextGroupHandlerIDForHandlerName LEOContextGroupHandlerIDForHandlerName
*/
const char*		LEOContextGroupHandlerNameForHandlerID( LEOContextGroup* inContext, LEOHandlerID inHandlerID );




#endif // LEO_CONTEXT_GROUP_H
