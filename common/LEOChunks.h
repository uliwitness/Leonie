/*
 *  LEOChunks.h
 *  Leonie
 *
 *  Created by Uli Kusterer on 20.09.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

/*!
	@header LEOChunks
	A chunk is a substring of a string. Chunks can not only be specified in
	characters, but can also be the result of some limited parsing of the
	given string.
*/

#ifndef LEO_CHUNKS_H
#define LEO_CHUNKS_H		1

// -----------------------------------------------------------------------------
//	Headers:
// -----------------------------------------------------------------------------

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>


/*! @enum LEOChunkType
	There are different kinds of chunks that are parsed differently, depending
	on which of these flags you pass in.
	@const kLEOChunkTypeINVALID		Used in some cases to indicate something
									that *can* be a chunk reference is *not*
									a chunk.
	@const kLEOChunkTypeByte		Take a byte out of the string. This may
									tear a byte out of the middle of a UTF8
									string and make it invalid as a string.
	@const kLEOChunkTypeCharacter	UTF8-characters. One character may use
									several bytes, e.g. for a Chinese or
									Japanese character.
	@const kLEOChunkTypeItem		Items are delimited by a certain character
									(by default, a comma). If there are several
									delimiters immediately in sequence, the
									items between them are considered to be
									empty. Items are assumed to be UTF8-strings.
	@const kLEOChunkTypeLine		Lines are delimited by a return or a line
									feed. Otherwise, lines behave like items.
	@const kLEOChunkTypeWord		Words are delimited by one or more spaces,
									tabs, returns or line feeds, i.e. whitespace
									characters. There can be no 'empty' words,
									and punctuation is treated just like any
									other alphabetic character.
*/
typedef enum
{
	kLEOChunkTypeINVALID,
	kLEOChunkTypeByte,
	kLEOChunkTypeCharacter,
	kLEOChunkTypeItem,
	kLEOChunkTypeLine,
	kLEOChunkTypeWord,
	kLEOChunkType_Last
} LEOChunkType;

/*!
	String names for each chunk type.
*/
extern const char*	gLEOChunkTypeNames[kLEOChunkType_Last +1];	// Last entry is NULL.


/*!
	Determine what character range corresponds to the given chunk range of inStr.
	You get back two offset pairs, one for extracting the value from the string,
	and a second pair for deleting them, which may include one delimiter.
	
	@param inStr			A UTF8-encoded string to be parsed to determine the
							range of the given chunk, or, for the byte chunk type,
							an arbitrary zero-terminated string of bytes.
	@param inType			The type of unit you wish to specify this chunk in.
	@param inRangeStart		The start offset of the range expressed in the unit
							specified by <tt>inType</tt>.
	@param inRangeEnd		The end offset of the range expressed in the unit
							specified by <tt>inType</tt>.
	@param outChunkStart	On return, this is set to a byte offset indicating
							the start of the payload of the given chunk, without
							any starting delimiters.
	@param outChunkEnd		On return, this is set to a byte offset indicating
							the end of the payload of the given chunk, without
							any ending delimiters.
	@param outDelChunkStart	On return, this is set to a byte offset indicating
							the start of the given chunk, including any starting
							or ending delimiters that would have to be deleted
							to remove this chunk completely from its string.
	@param outDelChunkEnd	On return, this is set to a byte offset indicating
							the end of the given chunk, including any starting
							or ending delimiters that would have to be deleted
							to remove this chunk completely from its string.
	@param itemDelimiter	The item delimiter to use when inType is kLEOChunkTypeItem.
*/
void	LEOGetChunkRanges( const char* inStr, LEOChunkType inType,
							size_t inRangeStart, size_t inRangeEnd,
							size_t *outChunkStart, size_t *outChunkEnd,
							size_t *outDelChunkStart, size_t *outDelChunkEnd,
							uint32_t itemDelimiter );


/*!
	Determine all the chunks of a certain type in a string and call the given
	callback for each chunk.
	
	@param inStr			A UTF8-encoded string to be parsed to determine the
							range of the given chunk, or, for the byte chunk type,
							an arbitrary buffer of bytes.
	@param inBufSize		The number of bytes in inStr to parse.
	@param inType			The type of unit for the chunk items to pass to the
							callback.
	@param inChunkCallback	A pointer to a function which will be called for
							each chunk item. If this function returns FALSE,
							parsing of the string for chunks will be aborted.
							Return TRUE to keep going.
	@param itemDelimiter	The item delimiter to use when inType is kLEOChunkTypeItem.
	@param userData			A pointer to an arbitrary block of data that will be
							passed to inChunkCallback as its userData parameter.
							Use this to pass in context information that your
							callback needs. LEODoForEachChunk() does not make
							any assumptions or do anything with this pointer
							except pass it on.
*/

void	LEODoForEachChunk( const char* inStr, size_t inBufSize, LEOChunkType inType,
							bool (*inChunkCallback)( const char* currStr, size_t currLen, size_t currStart, size_t currEnd, void* userData ),
							uint32_t itemDelimiter, void* userData );


#endif // LEO_CHUNKS_H
