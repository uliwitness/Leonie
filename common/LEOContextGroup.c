/*
 *  LEOContextGroup.c
 *  Leonie
 *
 *  Created by Uli Kusterer on 27.10.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

// -----------------------------------------------------------------------------
//	Headers:
// -----------------------------------------------------------------------------

#include "LEOContextGroup.h"
#include <stdlib.h>
#include <string.h>


// -----------------------------------------------------------------------------
//	Constants:
// -----------------------------------------------------------------------------

#define LEOReferencesTableChunkSize			16



// -----------------------------------------------------------------------------
//	Types:
// -----------------------------------------------------------------------------

/* What a LEOObjectID refers to, used by reference values. These are kept in a
	big array of "master pointers" named "references" in the LEOContextGroup. */
struct LEOObject	// What a LEOObjectID refers to. These are kept in a big array of "master pointers" in the context.
{
	void*			value;	// The actual pointer to the referenced value. NULL for unused object entries.
	LEOObjectSeed	seed;	// Whenever a referenced object entry is re-used, this seed is incremented, so people still referencing it know they're wrong.
};




LEOContextGroup*	LEOContextGroupCreate()
{
	LEOContextGroup*	theGroup = malloc( sizeof(LEOContextGroup) );
	memset( theGroup, 0, sizeof(LEOContextGroup) );
	theGroup->referenceCount = 1;
	
	return theGroup;
}


LEOContextGroup*	LEOContextGroupRetain( LEOContextGroup* inGroup )
{
	inGroup->referenceCount ++;
	return inGroup;
}


void	LEOContextGroupRelease( LEOContextGroup* inGroup )
{
	inGroup->referenceCount --;
	if( inGroup->referenceCount == 0 )
	{
		if( inGroup->references )
		{
			free( inGroup->references );
			inGroup->references = NULL;
			inGroup->numReferences = 0;
		}
		free( inGroup );
	}
}


LEOObjectID	LEOContextGroupCreateNewObjectIDForPointer( LEOContextGroup* inContext, void* theValue )
{
	LEOObjectID		newObjectID = LEOObjectIDINVALID;
	if( inContext->references == NULL )
	{
		inContext->numReferences = LEOReferencesTableChunkSize;
		inContext->references = calloc( inContext->numReferences, sizeof(struct LEOObject) );
		
		newObjectID = 0;	// Can start with first item right away.
	}
	else
	{
		// +++ Optimize: remember the last one we cleared or returned or so and start scanning there.
		
		for( size_t x = 0; x < inContext->numReferences; x++ )
		{
			if( inContext->references[x].value == NULL )	// Unused slot!
				newObjectID = x;
		}
		
		if( newObjectID == LEOObjectIDINVALID )
		{
			// No free slots left?
			size_t		oldNumReferences = inContext->numReferences;
			inContext->numReferences += LEOReferencesTableChunkSize;
			inContext->references = realloc( inContext->references, sizeof(struct LEOObject) * inContext->numReferences );
			memset( inContext->references +(oldNumReferences * sizeof(struct LEOObject)), 0, LEOReferencesTableChunkSize * sizeof(struct LEOObject) );
			
			newObjectID = oldNumReferences;	// Same as index of first new item.
		}
	}
	
	inContext->references[newObjectID].value = theValue;
	
	return newObjectID;
}


LEOObjectSeed	LEOContextGroupGetSeedForObjectID( LEOContextGroup* inContext, LEOObjectID inID )
{
	return inContext->references[inID].seed;
}


void	LEOContextGroupRecycleObjectID( LEOContextGroup* inContext, LEOObjectID inObjectID )
{
	inContext->references[inObjectID].value = NULL;
	inContext->references[inObjectID].seed += 1;	// Make sure that if this is reused, whoever still references it knows it's gone.
}


void*	LEOContextGroupGetPointerForObjectIDAndSeed( LEOContextGroup* inContext, LEOObjectID inObjectID, LEOObjectSeed inObjectSeed )
{
	if( inContext->references[inObjectID].seed != inObjectSeed )
		return NULL;
	
	return inContext->references[inObjectID].value;
}




