//
//  UTF8UTF32Utilities.h
//  Stacksmith
//
//  Created by Uli Kusterer on 04.09.11.
//  Copyright 2011 Uli Kusterer. All rights reserved.
//

#ifndef Stacksmith_UTF8UTF32Utilities_h
#define Stacksmith_UTF8UTF32Utilities_h

#include <stdint.h>
#include <stdio.h>


#if __cplusplus
extern "C" {
#endif
	
	uint32_t	UTF32CharacterToLower( uint32_t inUTF32Char );

	size_t		GetLengthOfUTF8SequenceStartingWith( unsigned char inChar );
	uint32_t	UTF8StringParseUTF32CharacterAtOffset( const char *utf8, size_t len, size_t *ioOffset );
	void		UTF8BytesForUTF32Character( uint32_t utf32Char, char* utf8, size_t *outLength );
	uint32_t	UTF8StringParseUTF32CharacterAtOffset( const char *utf8, size_t len, size_t *ioOffset );
	size_t		UTF8LengthForUTF32Char( uint32_t utf32Char );

	uint32_t	UTF16StringParseUTF32CharacterAtOffset( const uint16_t *utf16, size_t bytesLen, size_t *ioCharOffset );
	size_t		UTF16LengthForUTF32Char( uint32_t inChar );
	size_t		UTF16CharsForUTF32Char( uint32_t inChar, uint16_t outChars[2] );

#if __cplusplus
}
#endif

#endif
