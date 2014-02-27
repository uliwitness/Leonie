//
//  UTF8UTF32Utilities.c
//  Stacksmith
//
//  Created by Uli Kusterer on 04.09.11.
//  Copyright 2011 Uli Kusterer. All rights reserved.
//

extern "C" {
#include "UTF32CaseTables.h"
#include <stdio.h>
#include "UTF8UTF32Utilities.h"
}
#include <iostream>
#include <fstream>
#include <string>
#include <locale>
#include <iomanip>
#include <string>
#include <codecvt>


size_t		GetLengthOfUTF8SequenceStartingWith( unsigned char inChar );
uint32_t	UTF8StringParseUTF32CharacterAtOffset( const char *utf8, size_t len, size_t *ioOffset );
void		UTF8BytesForUTF32Character( uint32_t utf32Char, char* utf8, size_t *outLength );
uint32_t	UTF32CharacterToLower( uint32_t inUTF32Char );


size_t	GetLengthOfUTF8SequenceStartingWith( unsigned char inChar )
{
	size_t		outLength = 0;
	inChar >>= 3;
	
	if( inChar == 0x1e )
		outLength = 4;
	else
	{
		inChar >>= 1;
		if( inChar == 0x0e )
		{
			outLength = 3;
		}
		else
		{
			inChar >>= 1;
			if( inChar == 0x06 )
				outLength = 2;
			else
				outLength = 1;
		}
	}
	return outLength;
}


extern "C" uint32_t	UTF8StringParseUTF32CharacterAtOffset( const char *utf8, size_t len, size_t *ioOffset )
{
	const uint8_t		*currUTF8Byte = (const uint8_t*) utf8 + (*ioOffset);
	uint32_t			utf32Char = 0L;
	size_t				numBytesInSequence = 0;
	numBytesInSequence = GetLengthOfUTF8SequenceStartingWith( *currUTF8Byte );
	
	if( len < numBytesInSequence )
		return 0xffffffff;
	
	switch( numBytesInSequence )
	{
		case 4:
			utf32Char = ((*currUTF8Byte) ^ 0xF0);
			break;
			
		case 3:
			utf32Char = ((*currUTF8Byte) ^ 0xE0);
			break;
			
		case 2:
			utf32Char = ((*currUTF8Byte) ^ 0xC0);
			break;
			
		case 1:
			utf32Char = (*currUTF8Byte);
			break;
	}
	
	currUTF8Byte ++;
	
	for( size_t y = numBytesInSequence; y > 1; y-- )
	{
		utf32Char <<= 6;
		utf32Char |= ((*currUTF8Byte) ^ 0x80);
		currUTF8Byte ++;;
	}
	
	(*ioOffset) += numBytesInSequence;
	
	return utf32Char;
}


extern "C" void	UTF8BytesForUTF32Character( uint32_t utf32Char, char* utf8, size_t *outLength )
{
	char*	currUTF8Byte = utf8;
	
	if( utf32Char < 0x000080U )
	{
		(*currUTF8Byte) = utf32Char;
		currUTF8Byte ++;
	}
	else if( utf32Char < 0x000800U )
	{
		(*currUTF8Byte) = 0xC0 | (utf32Char >> 6);
		currUTF8Byte ++;
		(*currUTF8Byte) = 0x80 | (utf32Char & 0x3F);
		currUTF8Byte ++;
	}
	else if( utf32Char < 0x010000U )
	{
		(*currUTF8Byte) = 0xe0 | (utf32Char >> 12);
		currUTF8Byte ++;
		(*currUTF8Byte) = 0x80 | ((utf32Char >> 6) & 0x3F);
		currUTF8Byte ++;
		(*currUTF8Byte) = 0x80 | (utf32Char & 0x3F);
		currUTF8Byte ++;
	}
	else if( utf32Char <= 0x0010FFFFU )
	{
		(*currUTF8Byte) = 0xf0 | (utf32Char >> 18);
		currUTF8Byte ++;
		(*currUTF8Byte) = 0x80 | ((utf32Char >> 12) & 0x3F);
		currUTF8Byte ++;
		(*currUTF8Byte) = 0x80 | ((utf32Char >> 6) & 0x3F);
		currUTF8Byte ++;
		(*currUTF8Byte) = 0x80 | (utf32Char & 0x3F);
		currUTF8Byte ++;
	}
	
	(*outLength) = currUTF8Byte -utf8;
}


extern "C" size_t	UTF8LengthForUTF32Char( uint32_t utf32Char )
{
	if( utf32Char < 0x000080U )
	{
		return 1;
	}
	else if( utf32Char < 0x000800U )
	{
		return 2;
	}
	else if( utf32Char < 0x010000U )
	{
		return 3;
	}
	else if( utf32Char <= 0x0010FFFFU )
	{
		return 4;
	}
	
	return 0;
}


uint32_t	UTF32CharacterToLower( uint32_t inUTF32Char )
{
	uint32_t	resultUTF32Char;
	if( inUTF32Char <= 0x02B6 )
	{
		if( inUTF32Char >= 0x0041 )
		{
			resultUTF32Char = gUTF32CaseTableFrom0041[inUTF32Char - 0x0041];
			if( resultUTF32Char != 0 )
				return resultUTF32Char;
		}
		return inUTF32Char;
	}

	if( inUTF32Char <= 0x0556 )
	{
		if( inUTF32Char >= 0x0386 )
		{
			resultUTF32Char = gUTF32CaseTableFrom0386[inUTF32Char - 0x0386];
			if( resultUTF32Char != 0 )
				return resultUTF32Char;
		}
		return inUTF32Char;
	}

	if( inUTF32Char <= 0x10C5 )
	{
		if( inUTF32Char >= 0x10A0 )
		{
			resultUTF32Char = gUTF32CaseTableFrom10A0[inUTF32Char - 0x10A0];
			if( resultUTF32Char != 0 )
				return resultUTF32Char;
		}
		return inUTF32Char;
	}

	if (inUTF32Char <= 0x1FFC)
	{
		if (inUTF32Char >= 0x1E00)
		{
			resultUTF32Char = gUTF32CaseTableFrom1E00[inUTF32Char - 0x1E00];
			if( resultUTF32Char != 0 )
				return resultUTF32Char;
		}
		return inUTF32Char;
	}

	if( inUTF32Char <= 0x2133 )
	{
		if( inUTF32Char >= 0x2102 )
		{
			resultUTF32Char = gUTF32CaseTableFrom2102[inUTF32Char - 0x2102];
			if( resultUTF32Char != 0 )
				return resultUTF32Char;
		}
		return inUTF32Char;
	}

	if( inUTF32Char <= 0x24CF )
	{
		if( inUTF32Char >= 0x24B6 )
		{
			resultUTF32Char = gUTF32CaseTableFrom24B6[inUTF32Char - 0x24B6];
			if( resultUTF32Char != 0 )
				return resultUTF32Char;
		}
		return inUTF32Char;
	}

	if (inUTF32Char <= 0xFF3A)
	{
		if (inUTF32Char >= 0xFF21)
		{
			resultUTF32Char = gUTF32CaseTableFromFF21[inUTF32Char - 0xFF21];
			if( resultUTF32Char != 0 )
				return resultUTF32Char;
		}
		return inUTF32Char;
	}

	return inUTF32Char;
}

template<class Facet>
struct deletable_facet : Facet
{
    template<class ...Args>
    deletable_facet(Args&& ...args) : Facet(std::forward<Args>(args)...) {}
    ~deletable_facet() {}
};


extern "C" uint32_t	UTF16StringParseUTF32CharacterAtOffset( const uint16_t *utf16, size_t byteLen, size_t *ioCharOffset )
{
	std::wstring_convert<
        deletable_facet<std::codecvt<char32_t, char16_t, std::mbstate_t>>, char32_t> conv16;
	std::u16string	str16((char16_t*)utf16 +*ioCharOffset, byteLen -((*ioCharOffset) *sizeof(char16_t)));
    std::string str32 = conv16.to_bytes(str16);
	
	uint32_t	U = 0;
	uint16_t	W1 = *(utf16 +(*ioCharOffset));
	if( W1 < 0xd800 || W1 > 0xDFFF )	// Single-char representation in UTF16, too.
	{
		*ioCharOffset += 1;
		return W1;
	}
	uint16_t	W2 = 0;
	if( (((*ioCharOffset) +1) * sizeof(uint16_t)) < byteLen )
		W2 = *(utf16 +((*ioCharOffset) +1));
	if( W1 < 0xD800 || W1 > 0xDBFF )	// Invalid char.
	{
		*ioCharOffset += 1;	// We skip it.
		return W1;
	}
	if( W2 < 0xDC00 || W2 > 0xDFFF )	// Invalid surrogate pair.
	{
		*ioCharOffset += 1;	// We skip the first one and try again.
		return W1;
	}
	uint32_t	HiTenBits = W1 & 49407U;
	uint32_t	LoTenBits = W2 & 49407U;
	U = 0x10000 + ((HiTenBits << 10) | LoTenBits);
	
	return U;
}

extern "C" size_t	UTF16LengthForUTF32Char( uint32_t inChar )
{
	const uint16_t	HI_SURROGATE_START = 0xD800;
	uint16_t		X = (uint16_t) inChar;
	uint32_t		U = (inChar >> 16) & ((1 << 5) - 1);
	uint16_t		W = (uint16_t) U - 1;
	uint16_t		HiSurrogate = HI_SURROGATE_START | (W << 6) | X >> 10;
	size_t			utf16len = ((HiSurrogate == 0xffc0) ? 0 : 1) +1;

	return utf16len;
}


extern "C" size_t	UTF16CharsForUTF32Char( uint32_t inChar, uint16_t outChars[2] )
{
	const uint16_t	HI_SURROGATE_START = 0xD800;
	uint16_t	X = (uint16_t) inChar;
	uint32_t	U = (inChar >> 16) & ((1 << 5) - 1);
	uint16_t	W = (uint16_t) U - 1;
	uint16_t	HiSurrogate = HI_SURROGATE_START | (W << 6) | X >> 10;
	size_t		utf16len = ((HiSurrogate == 0xffc0) ? 0 : 1) +1;

	const uint16_t	LO_SURROGATE_START = (HiSurrogate == 0xffc0) ? 0 : 0xDC00;
	uint16_t	X2 = (uint16_t) inChar;
	uint16_t	LoSurrogate = (uint16_t) (LO_SURROGATE_START | (X2 & ((1 << 10) - 1)));
	
	if( utf16len == 1 )
	{
		outChars[0] = (LoSurrogate & 0xff) | (LoSurrogate >> 8);
		outChars[1] = 0;
	}
	else
	{
		outChars[0] = (HiSurrogate & 0xff) | HiSurrogate >> 8;
		outChars[1] = (LoSurrogate & 0xff) | (LoSurrogate >> 8);
	}
	
	return utf16len;
}

