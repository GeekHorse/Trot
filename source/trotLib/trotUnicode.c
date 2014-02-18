/*
Copyright (c) 2010-2014 Jeremiah Martell
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    - Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    - Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    - Neither the name of Jeremiah Martell nor the name of GeekHorse nor the
      name of Trot nor the names of its contributors may be used to endorse or
      promote products derived from this software without specific prior written
      permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/******************************************************************************/
/*!
	\file
	Unicode conversion functions.

	Reference:
	http://unicode.org
	The Unicode Standard, Version 6.0
	Chapter 3, Conformance
	Table 3-6. UTF-8 Bit Distribution
	Table 3-7. Well-Formed UTF-8 Byte Sequences
*/
#define TROT_FILE_NUMBER 4

/******************************************************************************/
#include "trot.h"
#include "trotInternal.h"

/******************************************************************************/
/*!
	\brief Decodes a list of utf8 bytes to characters.
	\param[in] lBytes List of bytes.
	\param[in] lCharacters List to append decoded characters.
	\return TROT_RC

*/
TROT_RC trotUtf8ToCharacters( TrotList *lBytes, TrotList *lCharacters )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TROT_INT numberOfBytes = 0;
	TROT_INT index = 1;

	TROT_INT byte1 = 0;
	TROT_INT byte2 = 0;
	TROT_INT byte3 = 0;
	TROT_INT byte4 = 0;
	TROT_INT character = 0;


	/* PRECOND */
	ERR_IF( lBytes == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( lCharacters == NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	/* get numberOfBytes */
	rc = trotListGetCount( lBytes, &numberOfBytes );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* go through bytes */
	index = 1;
	while ( index <= numberOfBytes )
	{
		/* get first byte */
		rc = trotListGetInt( lBytes, index, &byte1 );
		ERR_IF_PASSTHROUGH;

		/* one byte sequence */
		if ( byte1 >= 0x00 && byte1 <= 0x7F )
		{
			/* append character */
			character = byte1;
			rc = trotListAppendInt( lCharacters, character );
			ERR_IF_PASSTHROUGH;

			/* *** */
			index += 1;
			continue;
		}

		/* two byte sequence */
		if ( byte1 >= 0xC2 && byte1 <= 0xDF )
		{
			/* get second byte */
			index += 1;
			rc = trotListGetInt( lBytes, index, &byte2 );
			ERR_IF_PASSTHROUGH;

			/* validate second byte */
			if ( ! ( byte2 >= 0x80 && byte2 <= 0xBF ) )
			{
				ERR_IF_1( 1, TROT_RC_ERROR_UNICODE, byte2 );
			}

			/* append character */
			character = ((byte1 & 0x1F) << 6) | (byte2 & 0x3F);
			rc = trotListAppendInt( lCharacters, character );
			ERR_IF_PASSTHROUGH;

			/* *** */
			index += 1;
			continue;
		}

		/* three byte sequence */
		if ( byte1 >= 0xE0 && byte1 <= 0xEF )
		{
			/* get second byte */
			index += 1;
			rc = trotListGetInt( lBytes, index, &byte2 );
			ERR_IF_PASSTHROUGH;

			/* validate second byte */
			if ( byte1 == 0xE0 )
			{
				if ( ! ( byte2 >= 0xA0 && byte2 <= 0xBF ) )
				{
					ERR_IF_1( 1, TROT_RC_ERROR_UNICODE, byte2 );
				}
			}
			else if ( byte1 <= 0xEC )
			{
				if ( ! ( byte2 >= 0x80 && byte2 <= 0xBF ) )
				{
					ERR_IF_1( 1, TROT_RC_ERROR_UNICODE, byte2 );
				}
			}
			else if ( byte1 == 0xED )
			{
				if ( ! ( byte2 >= 0x80 && byte2 <= 0x9F ) )
				{
					ERR_IF_1( 1, TROT_RC_ERROR_UNICODE, byte2 );
				}
			}
			else 
			{
				if ( ! ( byte2 >= 0x80 && byte2 <= 0xBF ) )
				{
					ERR_IF_1( 1, TROT_RC_ERROR_UNICODE, byte2 );
				}
			}

			/* get third byte */
			index += 1;
			rc = trotListGetInt( lBytes, index, &byte3 );
			ERR_IF_PASSTHROUGH;

			/* validate third byte */
			if ( ! ( byte3 >= 0x80 && byte3 <= 0xBF ) )
			{
				ERR_IF_1( 1, TROT_RC_ERROR_UNICODE, byte3 );
			}

			/* append character */
			character = ((byte1 & 0x0F) << 12) | ((byte2 & 0x3F) << 6) | (byte3 & 0x3F);
			rc = trotListAppendInt( lCharacters, character );
			ERR_IF_PASSTHROUGH;

			/* *** */
			index += 1;
			continue;
		}

		/* four byte sequence */
		if ( byte1 >= 0xF0 && byte1 <= 0xF4 )
		{
			/* get second byte */
			index += 1;
			rc = trotListGetInt( lBytes, index, &byte2 );
			ERR_IF_PASSTHROUGH;

			/* validate second byte */
			if ( byte1 == 0xF0 )
			{
				if ( ! ( byte2 >= 0x90 && byte2 <= 0xBF ) )
				{
					ERR_IF_1( 1, TROT_RC_ERROR_UNICODE, byte2 );
				}
			}
			else if ( byte1 <= 0xF3 )
			{
				if ( ! ( byte2 >= 0x80 && byte2 <= 0xBF ) )
				{
					ERR_IF_1( 1, TROT_RC_ERROR_UNICODE, byte2 );
				}
			}
			else 
			{
				if ( ! ( byte2 >= 0x80 && byte2 <= 0x8F ) )
				{
					ERR_IF_1( 1, TROT_RC_ERROR_UNICODE, byte2 );
				}
			}

			/* get third byte */
			index += 1;
			rc = trotListGetInt( lBytes, index, &byte3 );
			ERR_IF_PASSTHROUGH;

			/* validate third byte */
			if ( ! ( byte3 >= 0x80 && byte3 <= 0xBF ) )
			{
				ERR_IF_1( 1, TROT_RC_ERROR_UNICODE, byte3 );
			}

			/* get fourth byte */
			index += 1;
			rc = trotListGetInt( lBytes, index, &byte4 );
			ERR_IF_PASSTHROUGH;

			/* validate fourth byte */
			if ( ! ( byte4 >= 0x80 && byte4 <= 0xBF ) )
			{
				ERR_IF_1( 1, TROT_RC_ERROR_UNICODE, byte4 );
			}

			/* append character */
			character = ((byte1 & 0x07) << 18) | ((byte2 & 0x3F) << 12) | ((byte3 & 0x3F) << 6) | (byte4 & 0x3F);
			rc = trotListAppendInt( lCharacters, character );
			ERR_IF_PASSTHROUGH;

			/* *** */
			index += 1;
			continue;
		}

		/* invalid first byte */
		ERR_IF_1( 1, TROT_RC_ERROR_UNICODE, byte1 );
	}


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Encodes a list of characters to utf8 bytes.
	\param[in] lCharacters List of characters.
	\param[in] lBytes List to append bytes to.
	\return TROT_RC
*/
TROT_RC trotCharactersToUtf8( TrotList *lCharacters, TrotList *lBytes )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TROT_INT numberOfCharacters = 0;
	TROT_INT index = 1;

	TROT_INT character = 0;
	TROT_INT byte1 = 0;
	TROT_INT byte2 = 0;
	TROT_INT byte3 = 0;
	TROT_INT byte4 = 0;


	/* PRECOND */
	ERR_IF( lCharacters == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( lBytes == NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	/* get numberOfCharacters */
	rc = trotListGetCount( lCharacters, &numberOfCharacters );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* go through characters */
	index = 1;
	while ( index <= numberOfCharacters )
	{
		/* get character */
		rc = trotListGetInt( lCharacters, index, &character );
		ERR_IF_PASSTHROUGH;

		/* 1 byte: 0x00 - 0x7F */
		if ( character >= 0x00 && character <= 0x7F )
		{
			/* setup bytes */
			byte1 = character;

			/* append bytes */
			rc = trotListAppendInt( lBytes, byte1 );
			ERR_IF_PASSTHROUGH;
		}
		/* 2 byte: 0x80 - 0x7FF */
		else if ( character >= 0x80 && character <= 0x7FF )
		{
			/* setup bytes */
			byte1 = ( ( character >> 6 ) & 0x1F ) | 0xC0;
			byte2 = ( ( character      ) & 0x3F ) | 0x80;

			/* append bytes */
			rc = trotListAppendInt( lBytes, byte1 );
			ERR_IF_PASSTHROUGH;
			rc = trotListAppendInt( lBytes, byte2 );
			ERR_IF_PASSTHROUGH;
		}
		/* 3 byte: 0x800 - 0xD7FF, 0xE000 - 0xFFFF */
		else if (
			     ( character >= 0x800 && character <= 0xD7FF )
			  || ( character >= 0xE000 && character <= 0xFFFF )
			)
		{
			/* setup bytes */
			byte1 = ( ( character >> 12 ) & 0x0F ) | 0xE0;
			byte2 = ( ( character >>  6 ) & 0x3F ) | 0x80;
			byte3 = ( ( character       ) & 0x3F ) | 0x80;

			/* append bytes */
			rc = trotListAppendInt( lBytes, byte1 );
			ERR_IF_PASSTHROUGH;
			rc = trotListAppendInt( lBytes, byte2 );
			ERR_IF_PASSTHROUGH;
			rc = trotListAppendInt( lBytes, byte3 );
			ERR_IF_PASSTHROUGH;
		}
		/* 4 byte: 0x10000 - 0x10FFFF */
		else if ( character >= 0x10000 && character <= 0x10FFFF )
		{
			/* setup bytes */
			byte1 = ( ( character >> 18 ) & 0x07 ) | 0xF0;
			byte2 = ( ( character >> 12 ) & 0x3F ) | 0x80;
			byte3 = ( ( character >>  6 ) & 0x3F ) | 0x80;
			byte4 = ( ( character       ) & 0x3F ) | 0x80;

			/* append bytes */
			rc = trotListAppendInt( lBytes, byte1 );
			ERR_IF_PASSTHROUGH;
			rc = trotListAppendInt( lBytes, byte2 );
			ERR_IF_PASSTHROUGH;
			rc = trotListAppendInt( lBytes, byte3 );
			ERR_IF_PASSTHROUGH;
			rc = trotListAppendInt( lBytes, byte4 );
			ERR_IF_PASSTHROUGH;
		}
		else
		{
			ERR_IF_1( 1, TROT_RC_ERROR_UNICODE, character );
		}

		/* *** */
		index += 1;
	}


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Is a character a whitespace character?
	\param[in] character The character to check.
	\return s32 1 if whitespace, 0 if not.
*/
s32 trotUnicodeIsWhitespace( TROT_INT character )
{
	if (    character == '\t'   /* horizontal tab */
	     || character == '\n'   /* newline */
	     || character == 0x0B   /* vertical tab */
	     || character == 0x0C   /* form feed */ 
	     || character == '\r'   /* carriage return */
	     || character == ' '    /* space */
	     || character == 0x85   /* next line */
	     || character == 0xA0   /* no break space */
	     || character == 0x1680 /* ogham space mark */
	     || character == 0x180E /* mongolian vowel separator */
	     || character == 0x2000 /* en quad */
	     || character == 0x2001 /* em quad */
	     || character == 0x2002 /* en space */
	     || character == 0x2003 /* em space */
	     || character == 0x2004 /* three-per-em space */
	     || character == 0x2005 /* four-per-em space */
	     || character == 0x2006 /* six-per-em space */
	     || character == 0x2007 /* figure space */
	     || character == 0x2008 /* punctuation space */
	     || character == 0x2009 /* thin space */
	     || character == 0x200A /* hair space */
	     || character == 0x2028 /* line separator */
	     || character == 0x2029 /* paragraph separator */
	     || character == 0x202F /* narrow no-break space */
	     || character == 0x205F /* medium mathematical space */
	     || character == 0x3000 /* ideographic space */
	   )
	{
		return 1;
	}

	return 0;
}

