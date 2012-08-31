/*
Copyright (c) 2010,2011,2012, Jeremiah Martell
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

/******************************************************************************/
#include "trot.h"
#include "trotInternal.h"

/******************************************************************************/
/*!
	\brief Decodes a list of utf8 bytes to characters.
	\param lrBytes List of bytes.
	\param lrCharacters_A List to append decoded characters.
	\return TROT_RC

*/
TROT_RC trotUtf8ToCharacters( trotListRef *lrBytes, trotListRef *lrCharacters )
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
	PRECOND_ERR_IF( lrBytes == NULL );
	PRECOND_ERR_IF( lrCharacters == NULL );


	/* CODE */
	/* get numberOfBytes */
	rc = trotListRefGetCount( lrBytes, &numberOfBytes );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* go through bytes */
	index = 1;
	while ( index <= numberOfBytes )
	{
		/* get first byte */
		rc = trotListRefGetInt( lrBytes, index, &byte1 );
		ERR_IF_PASSTHROUGH;

		/* one byte sequence */
		if ( byte1 >= 0x00 && byte1 <= 0x7F )
		{
			/* append character */
			character = byte1;
			rc = trotListRefAppendInt( lrCharacters, character );
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
			rc = trotListRefGetInt( lrBytes, index, &byte2 );
			ERR_IF_PASSTHROUGH;

			/* validate second byte */
			if ( ! ( byte2 >= 0x80 && byte2 <= 0xBF ) )
			{
				ERR_IF( 1, TROT_RC_ERROR_UNICODE );
			}

			/* append character */
			character = ((byte1 & 0x1F) << 6) | (byte2 & 0x3F);
			rc = trotListRefAppendInt( lrCharacters, character );
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
			rc = trotListRefGetInt( lrBytes, index, &byte2 );
			ERR_IF_PASSTHROUGH;

			/* validate second byte */
			if ( byte1 == 0xE0 )
			{
				if ( ! ( byte2 >= 0xA0 && byte2 <= 0xBF ) )
				{
					ERR_IF( 1, TROT_RC_ERROR_UNICODE );
				}
			}
			else if ( byte1 <= 0xEC )
			{
				if ( ! ( byte2 >= 0x80 && byte2 <= 0xBF ) )
				{
					ERR_IF( 1, TROT_RC_ERROR_UNICODE );
				}
			}
			else if ( byte1 == 0xED )
			{
				if ( ! ( byte2 >= 0x80 && byte2 <= 0x9F ) )
				{
					ERR_IF( 1, TROT_RC_ERROR_UNICODE );
				}
			}
			else 
			{
				if ( ! ( byte2 >= 0x80 && byte2 <= 0xBF ) )
				{
					ERR_IF( 1, TROT_RC_ERROR_UNICODE );
				}
			}

			/* get third byte */
			index += 1;
			rc = trotListRefGetInt( lrBytes, index, &byte3 );
			ERR_IF_PASSTHROUGH;

			/* validate third byte */
			if ( ! ( byte3 >= 0x80 && byte3 <= 0xBF ) )
			{
				ERR_IF( 1, TROT_RC_ERROR_UNICODE );
			}

			/* append character */
			character = ((byte1 & 0x0F) << 12) | ((byte2 & 0x3F) << 6) | (byte3 & 0x3F);
			rc = trotListRefAppendInt( lrCharacters, character );
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
			rc = trotListRefGetInt( lrBytes, index, &byte2 );
			ERR_IF_PASSTHROUGH;

			/* validate second byte */
			if ( byte1 == 0xF0 )
			{
				if ( ! ( byte2 >= 0x90 && byte2 <= 0xBF ) )
				{
					ERR_IF( 1, TROT_RC_ERROR_UNICODE );
				}
			}
			else if ( byte1 <= 0xF3 )
			{
				if ( ! ( byte2 >= 0x80 && byte2 <= 0xBF ) )
				{
					ERR_IF( 1, TROT_RC_ERROR_UNICODE );
				}
			}
			else 
			{
				if ( ! ( byte2 >= 0x80 && byte2 <= 0x8F ) )
				{
					ERR_IF( 1, TROT_RC_ERROR_UNICODE );
				}
			}

			/* get third byte */
			index += 1;
			rc = trotListRefGetInt( lrBytes, index, &byte3 );
			ERR_IF_PASSTHROUGH;

			/* validate third byte */
			if ( ! ( byte3 >= 0x80 && byte3 <= 0xBF ) )
			{
				ERR_IF( 1, TROT_RC_ERROR_UNICODE );
			}

			/* get fourth byte */
			index += 1;
			rc = trotListRefGetInt( lrBytes, index, &byte4 );
			ERR_IF_PASSTHROUGH;

			/* validate fourth byte */
			if ( ! ( byte4 >= 0x80 && byte4 <= 0xBF ) )
			{
				ERR_IF( 1, TROT_RC_ERROR_UNICODE );
			}

			/* append character */
			character = ((byte1 & 0x07) << 18) | ((byte2 & 0x3F) << 12) | ((byte3 & 0x3F) << 6) | (byte4 & 0x3F);
			rc = trotListRefAppendInt( lrCharacters, character );
			ERR_IF_PASSTHROUGH;

			/* *** */
			index += 1;
			continue;
		}

		/* invalid first byte */
		ERR_IF( 1, TROT_RC_ERROR_UNICODE );
	}


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Encodes a list of characters to utf8 bytes.
	\param lrBytes List of characters.
	\param lrCharacters_A List to append encoded utf8 bytes.
	\return TROT_RC
*/
TROT_RC trotCharactersToUtf8( trotListRef *lrCharacters, trotListRef *lrBytes )
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
	PRECOND_ERR_IF( lrCharacters == NULL );
	PRECOND_ERR_IF( lrBytes == NULL );


	/* CODE */
	/* get numberOfCharacters */
	rc = trotListRefGetCount( lrCharacters, &numberOfCharacters );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* go through characters */
	index = 1;
	while ( index <= numberOfCharacters )
	{
		/* get character */
		rc = trotListRefGetInt( lrCharacters, index, &character );
		ERR_IF_PASSTHROUGH;

		/* 1 byte: 0x00 - 0x7F */
		if ( character >= 0x00 && character <= 0x7F )
		{
			/* setup bytes */
			byte1 = character;

			/* append bytes */
			rc = trotListRefAppendInt( lrBytes, byte1 );
			ERR_IF_PASSTHROUGH;
		}
		/* 2 byte: 0x80 - 0x7FF */
		else if ( character >= 0x80 && character <= 0x7FF )
		{
			/* setup bytes */
			byte1 = ( ( character >> 6 ) & 0x1F ) | 0xC0;
			byte2 = ( ( character      ) & 0x3F ) | 0x80;

			/* append bytes */
			rc = trotListRefAppendInt( lrBytes, byte1 );
			ERR_IF_PASSTHROUGH;
			rc = trotListRefAppendInt( lrBytes, byte2 );
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
			rc = trotListRefAppendInt( lrBytes, byte1 );
			ERR_IF_PASSTHROUGH;
			rc = trotListRefAppendInt( lrBytes, byte2 );
			ERR_IF_PASSTHROUGH;
			rc = trotListRefAppendInt( lrBytes, byte3 );
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
			rc = trotListRefAppendInt( lrBytes, byte1 );
			ERR_IF_PASSTHROUGH;
			rc = trotListRefAppendInt( lrBytes, byte2 );
			ERR_IF_PASSTHROUGH;
			rc = trotListRefAppendInt( lrBytes, byte3 );
			ERR_IF_PASSTHROUGH;
			rc = trotListRefAppendInt( lrBytes, byte4 );
			ERR_IF_PASSTHROUGH;
		}
		else
		{
			ERR_IF( 1, TROT_RC_ERROR_UNICODE );
		}

		/* *** */
		index += 1;
	}


	/* CLEANUP */
	cleanup:

	return rc;
}

