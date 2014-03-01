//
//  UTF8UTF32Utilities.c
//  Stacksmith
//
//  Created by Uli Kusterer on 04.09.11.
//  Copyright 2011 Uli Kusterer. All rights reserved.
//

#include "UTF8UTF32Utilities.h"
#include "UTF32CaseTables.h"


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


uint32_t	UTF8StringParseUTF32CharacterAtOffset( const char *utf8, size_t len, size_t *ioOffset )
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


void	UTF8BytesForUTF32Character( uint32_t utf32Char, char* utf8, size_t *outLength )
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


size_t	UTF8LengthForUTF32Char( uint32_t utf32Char )
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


uint32_t	UTF16StringParseUTF32CharacterAtOffset( const uint16_t *utf16, size_t byteLen, size_t *ioCharOffset )
{
	size_t	remainingLen = byteLen -((*ioCharOffset) * sizeof(uint16_t));
	
	if( remainingLen < 1 )
		return 0;
	else if( remainingLen < 2 || *utf16 < 0xD800 || *utf16 > 0xDBFF )
	{
		*ioCharOffset += 1;
		return *utf16;
	}
	uint32_t	codepoint = (utf16[*ioCharOffset] -0xD800) * 0x400 +(utf16[(*ioCharOffset) +1] -0xDC00) + 0x10000;
	*ioCharOffset += 2;
	return codepoint;
}


size_t	UTF16LengthForUTF32Char( uint32_t inChar )
{
	if( inChar > 0xffff )
		return 2;

	return 1;
}


size_t	UTF16CharsForUTF32Char( uint32_t inChar, uint16_t outChars[2] )
{
	if( inChar > 0xffff )
	{
		outChars[0] = (uint16_t)((inChar >> 10)   + (0xd800u - (0x10000 >> 10)));
		outChars[1] = (uint16_t)((inChar & 0x3ff) + 0xdc00u);
		
		return 2;
	}
	
	outChars[0] = (uint16_t)inChar;
	
	return 1;
}

