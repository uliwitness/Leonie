/*
 *  LEOChunks.h
 *  Leonie
 *
 *  Created by Uli Kusterer on 20.09.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#ifndef LEO_CHUNKS_H
#define LEO_CHUNKS_H		1

#include <sys/types.h>


typedef enum
{
	kLEOChunkTypeCharacter,
	kLEOChunkTypeItem,
	kLEOChunkTypeLine,
	kLEOChunkTypeWord
} LEOChunkType;


void	LEOGetChunkRanges( const char* inStr, LEOChunkType inType,
						size_t inRangeStart, size_t inRangeEnd,
						size_t *outChunkStart, size_t *outChunkEnd,
						size_t *outDelChunkStart, size_t *outDelChunkEnd,
						char itemDelimiter );


#endif //LEO_CHUNKS_H
