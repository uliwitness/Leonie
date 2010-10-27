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
	scripts running in the same context from each other. Every context belongs
	to a group, which holds global data shared by all its contexts, which
	includes safe references to objects and values that may go away.
*/

#ifndef LEO_OBJECT_H
#define LEO_OBJECT_H		1

// -----------------------------------------------------------------------------
//	Headers:
// -----------------------------------------------------------------------------

#include "LEOValue.h"


// -----------------------------------------------------------------------------
//	Types:
// -----------------------------------------------------------------------------


/*! What a LEOObjectID refers to, used by reference values. These are kept in a
	big array of "master pointers" named "references" in the LEOContextGroup.
	@field	value	The actual pointer to the referenced value. NULL for unused object entries.
	@field	seed	Whenever a referenced object entry is re-used, this seed is incremented, so people still referencing it know they're wrong.
	@seealso //leo_ref/c/tag/LEOValueReference LEOValueReference */
typedef struct LEOObject	// What a LEOObjectID refers to. These are kept in a big array of "master pointers" in the context.
{
	LEOValuePtr		value;	// NULL for unused object entries.
	LEOObjectSeed	seed;	// Whenever a referenced object entry is re-used, this seed is incremented, so people still referencing it know they're wrong.
} LEOObject;


/*! All LEOContexts belong to a Context group that contains references and other
	global data they share. You can insulate running scripts from each other by
	placing them in a different context group.
	@field	references			An array of "master pointers" to values to which references have been created.
	@field	numReferences		Number of items in the <tt>references</tt>. */
typedef struct LEOContextGroup
{
	size_t			referenceCount;		// Reference count for this object, i.e. number of contexts still attached to this object.
	size_t			numReferences;		// Available slots in "references" array.
	LEOObject		*references;		// "Master pointer" table for references so we can detect when a reference goes away.
} LEOContextGroup;


/*!
	Creates a context group for use with LEOContexts. The LEOContxtGroup* is
	reference-counted and its reference count is set to 1, so when you're done
	with it, you can release it. When a context is attached to a context group,
	the context retains its group to ensure it doesn't go away.
*/
LEOContextGroup*	LEOContextGroupCreate();	// Gives referenceCount of 1.


/*!
	Acquire ownership of the given context group, so that when the current owner
	releases it the object still stays around for your use. Increases the
	reference count by 1.
	@result Returns inGroup so you can assign it to a variable in a struct,
			or a global, or whatever makes sense.
*/
LEOContextGroup*	LEOContextGroupRetain( LEOContextGroup* inGroup );		// Adds 1 to referenceCount. Returns inGroup.

/*!
	Give up ownership of the given context group. You acquire ownership by either
	creating the context group, or by retaining it. Giving up ownership decreases
	its reference count by 1. When the last owner releases the object (and the
	reference count reaches 0), the context group is freed.
*/
void	LEOContextGroupRelease( LEOContextGroup* inGroup );	// Subtracts 1 from referenceCount. If it hits 0, disposes of inScript.


// Used to implement references to values that can disappear:
/*!
	Give up ownership of the given context group. You acquire ownership by either
	creating the context group, or by retaining it. Giving up ownership decreases
	its reference count by 1. When the last owner releases the object (and the
	reference count reaches 0), the context group is freed.
*/
LEOObjectID		LEOContextGroupCreateNewObjectIDForValue( LEOContextGroup* inContext, LEOValuePtr theValue );
LEOObjectSeed	LEOContextGroupGetSeedForObjectID( LEOContextGroup* inContext, LEOObjectID inID );
void			LEOContextGroupRecycleObjectID( LEOContextGroup* inContext, LEOObjectID inObjectID );
LEOValuePtr		LEOContextGroupGetValueForObjectIDAndSeed( LEOContextGroup* inContext, LEOObjectID inObjectID, LEOObjectSeed inObjectSeed );


#endif // LEO_OBJECT_H
