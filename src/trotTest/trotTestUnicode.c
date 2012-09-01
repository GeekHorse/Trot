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
#include "trot.h"
#include "trotInternal.h"

#include "trotTestCommon.h"

/******************************************************************************/
static int testCharacterToUtf8ToCharacter( TROT_INT start, TROT_INT end, int numberOfBytes );
static int testBadByte( TROT_INT byte1, TROT_INT byte2, TROT_INT byte3, TROT_INT byte4 );
static int testBadKind( TROT_INT byte1, TROT_INT byte2, TROT_INT byte3, TROT_INT byte4 );
static int testBadCharacter( TROT_INT character );

/******************************************************************************/
int testUnicode()
{
	/* DATA */
	int rc = TROT_RC_SUCCESS;

	trotList *l = NULL;


	/* CODE */
	printf( "Testing unicode functions...\n" ); fflush( stdout );

	printf( "  Testing Character > utf8 > Character...\n" ); fflush( stdout );

	/* 1 byte: 0x00 - 0x7F */
	TEST_ERR_IF( testCharacterToUtf8ToCharacter( 0x00, 0x7F, 1 ) != TROT_RC_SUCCESS );
	printf( "." ); fflush( stdout );

	/* 2 byte: 0x80 - 0x7FF */
	TEST_ERR_IF( testCharacterToUtf8ToCharacter( 0x80, 0x7FF, 2 ) != TROT_RC_SUCCESS );
	printf( "." ); fflush( stdout );

	/* 3 byte: 0x800 - 0xD7FF, 0xE000 - 0xFFFF */
	TEST_ERR_IF( testCharacterToUtf8ToCharacter( 0x800, 0xD7FF, 3 ) != TROT_RC_SUCCESS );
	printf( "." ); fflush( stdout );

	TEST_ERR_IF( testCharacterToUtf8ToCharacter( 0xE000, 0xFFFF, 3 ) != TROT_RC_SUCCESS );
	printf( "." ); fflush( stdout );

	/* 4 byte: 0x10000 - 0x10FFFF */
	TEST_ERR_IF( testCharacterToUtf8ToCharacter( 0x10000, 0x10FFFF, 4 ) != TROT_RC_SUCCESS );
	printf( "." ); fflush( stdout );

	printf( "\n" ); fflush( stdout );


	/* test bad bytes */
	printf( "  Testing bad bytes...\n" ); fflush( stdout );

	TEST_ERR_IF( testBadByte( -1, 0, 0, 0 ) != 0 );
	TEST_ERR_IF( testBadByte( 0x80, 0, 0, 0 ) != 0 );
	TEST_ERR_IF( testBadByte( 0xC1, 0, 0, 0 ) != 0 );
	TEST_ERR_IF( testBadByte( 0xF5, 0, 0, 0 ) != 0 );
	TEST_ERR_IF( testBadByte( 0xFF, 0, 0, 0 ) != 0 );

	TEST_ERR_IF( testBadByte( 0xC2, 0x7F, 0, 0 ) != 0 );
	TEST_ERR_IF( testBadByte( 0xC2, 0xC0, 0, 0 ) != 0 );

	TEST_ERR_IF( testBadByte( 0xE0, 0x9F, 0, 0 ) != 0 );
	TEST_ERR_IF( testBadByte( 0xE0, 0xC0, 0, 0 ) != 0 );
	TEST_ERR_IF( testBadByte( 0xE0, 0xA0, 0x7F, 0 ) != 0 );
	TEST_ERR_IF( testBadByte( 0xE0, 0xA0, 0xC0, 0 ) != 0 );

	TEST_ERR_IF( testBadByte( 0xE1, 0x7F, 0, 0 ) != 0 );
	TEST_ERR_IF( testBadByte( 0xE1, 0xC0, 0, 0 ) != 0 );
	TEST_ERR_IF( testBadByte( 0xE1, 0x80, 0x7F, 0 ) != 0 );
	TEST_ERR_IF( testBadByte( 0xE1, 0x80, 0xC0, 0 ) != 0 );

	TEST_ERR_IF( testBadByte( 0xED, 0x7F, 0, 0 ) != 0 );
	TEST_ERR_IF( testBadByte( 0xED, 0xA0, 0, 0 ) != 0 );
	TEST_ERR_IF( testBadByte( 0xED, 0x80, 0x7F, 0 ) != 0 );
	TEST_ERR_IF( testBadByte( 0xED, 0x80, 0xC0, 0 ) != 0 );

	TEST_ERR_IF( testBadByte( 0xEE, 0x7F, 0, 0 ) != 0 );
	TEST_ERR_IF( testBadByte( 0xEE, 0xC0, 0, 0 ) != 0 );
	TEST_ERR_IF( testBadByte( 0xEE, 0x80, 0x7F, 0 ) != 0 );
	TEST_ERR_IF( testBadByte( 0xEE, 0x80, 0xC0, 0 ) != 0 );

	TEST_ERR_IF( testBadByte( 0xF0, 0x8F, 0, 0 ) != 0 );
	TEST_ERR_IF( testBadByte( 0xF0, 0xC0, 0, 0 ) != 0 );
	TEST_ERR_IF( testBadByte( 0xF0, 0x80, 0x7F, 0 ) != 0 );
	TEST_ERR_IF( testBadByte( 0xF0, 0x80, 0xC0, 0 ) != 0 );
	TEST_ERR_IF( testBadByte( 0xF0, 0x80, 0x80, 0x7F ) != 0 );
	TEST_ERR_IF( testBadByte( 0xF0, 0x80, 0x80, 0xC0 ) != 0 );

	TEST_ERR_IF( testBadByte( 0xF1, 0x7F, 0, 0 ) != 0 );
	TEST_ERR_IF( testBadByte( 0xF1, 0xC0, 0, 0 ) != 0 );
	TEST_ERR_IF( testBadByte( 0xF1, 0x80, 0x7F, 0 ) != 0 );
	TEST_ERR_IF( testBadByte( 0xF1, 0x80, 0xC0, 0 ) != 0 );
	TEST_ERR_IF( testBadByte( 0xF1, 0x80, 0x80, 0x7F ) != 0 );
	TEST_ERR_IF( testBadByte( 0xF1, 0x80, 0x80, 0xC0 ) != 0 );

	TEST_ERR_IF( testBadByte( 0xF4, 0x7F, 0, 0 ) != 0 );
	TEST_ERR_IF( testBadByte( 0xF4, 0x90, 0, 0 ) != 0 );
	TEST_ERR_IF( testBadByte( 0xF4, 0x80, 0x7F, 0 ) != 0 );
	TEST_ERR_IF( testBadByte( 0xF4, 0x80, 0xC0, 0 ) != 0 );
	TEST_ERR_IF( testBadByte( 0xF4, 0x80, 0x80, 0x7F ) != 0 );
	TEST_ERR_IF( testBadByte( 0xF4, 0x80, 0x80, 0xC0 ) != 0 );

	/* test bad kind */
	printf( "  Testing bad kind when doing unicode conversion...\n" );
	TEST_ERR_IF( testBadKind( -1, 0, 0, 0 ) != 0 );
	TEST_ERR_IF( testBadKind( 0xC2, -1, 0, 0 ) != 0 );
	TEST_ERR_IF( testBadKind( 0xE0, -1, 0, 0 ) != 0 );
	TEST_ERR_IF( testBadKind( 0xE0, 0xA0, -1, 0 ) != 0 );
	TEST_ERR_IF( testBadKind( 0xF0, -1, 0, 0 ) != 0 );
	TEST_ERR_IF( testBadKind( 0xF0, 0x90, -1, 0 ) != 0 );
	TEST_ERR_IF( testBadKind( 0xF0, 0x90, 0x80, -1 ) != 0 );

	TEST_ERR_IF( trotListInit( &l ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListAppendList( l, l ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotCharactersToUtf8( l, l ) != TROT_RC_ERROR_WRONG_KIND );
	trotListFree( &l );
	

	/* test bad characters */
	printf( "  Testing bad characters...\n" ); fflush( stdout );
	TEST_ERR_IF( testBadCharacter( -1 ) != 0 );
	TEST_ERR_IF( testBadCharacter( 0xD800 ) != 0 );
	TEST_ERR_IF( testBadCharacter( 0xDFFF ) != 0 );
	TEST_ERR_IF( testBadCharacter( 0x110000 ) != 0 );



	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
static int testCharacterToUtf8ToCharacter( TROT_INT start, TROT_INT end, int numberOfBytesShouldBe )
{
	/* DATA */
	int rc = 0;

	TROT_INT characterIn = 0;
	TROT_INT characterOut = 0;
	trotList *lCharacterIn = NULL;
	trotList *lBytes = NULL;
	trotList *lCharacterOut = NULL;

	TROT_INT numberOfBytesActual = 0;
	TROT_INT numberOfCharacters = 0;


	/* CODE */
	characterIn = start;
	while ( characterIn <= end )
	{
		TEST_ERR_IF( trotListInit( &lCharacterIn ) != TROT_RC_SUCCESS );
		TEST_ERR_IF( trotListInit( &lBytes ) != TROT_RC_SUCCESS );
		TEST_ERR_IF( trotListInit( &lCharacterOut ) != TROT_RC_SUCCESS );

		TEST_ERR_IF( trotListAppendInt( lCharacterIn, characterIn ) != TROT_RC_SUCCESS );

		TEST_ERR_IF( trotCharactersToUtf8( lCharacterIn, lBytes ) != TROT_RC_SUCCESS );

		TEST_ERR_IF( trotListGetCount( lBytes, &numberOfBytesActual ) != TROT_RC_SUCCESS );
		TEST_ERR_IF( numberOfBytesActual != numberOfBytesShouldBe );

		TEST_ERR_IF( trotUtf8ToCharacters( lBytes, lCharacterOut ) != TROT_RC_SUCCESS );

		TEST_ERR_IF( trotListGetCount( lCharacterOut, &numberOfCharacters ) != TROT_RC_SUCCESS );
		TEST_ERR_IF( numberOfCharacters != 1 );

		TEST_ERR_IF( trotListGetInt( lCharacterOut, 1, &characterOut ) != TROT_RC_SUCCESS );

		TEST_ERR_IF( characterIn != characterOut );

		trotListFree( &lCharacterIn );
		trotListFree( &lBytes );
		trotListFree( &lCharacterOut );

		/* *** */
		characterIn += 1;
	}


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
static int testBadKind( TROT_INT byte1, TROT_INT byte2, TROT_INT byte3, TROT_INT byte4 )
{
	/* DATA */
	int rc = 0;

	trotList *lIn = NULL;
	trotList *lOut = NULL;


	/* CODE */
	TEST_ERR_IF( trotListInit( &lIn ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListInit( &lOut ) != TROT_RC_SUCCESS );

	if ( byte1 != -1 )
	{
		TEST_ERR_IF( trotListAppendInt( lIn, byte1 ) != TROT_RC_SUCCESS );
	}
	else
	{
		TEST_ERR_IF( trotListAppendList( lIn, lIn ) != TROT_RC_SUCCESS );
	}

	if ( byte2 != -1 )
	{
		TEST_ERR_IF( trotListAppendInt( lIn, byte2 ) != TROT_RC_SUCCESS );
	}
	else
	{
		TEST_ERR_IF( trotListAppendList( lIn, lIn ) != TROT_RC_SUCCESS );
	}

	if ( byte3 != -1 )
	{
		TEST_ERR_IF( trotListAppendInt( lIn, byte3 ) != TROT_RC_SUCCESS );
	}
	else
	{
		TEST_ERR_IF( trotListAppendList( lIn, lIn ) != TROT_RC_SUCCESS );
	}

	if ( byte4 != -1 )
	{
		TEST_ERR_IF( trotListAppendInt( lIn, byte4 ) != TROT_RC_SUCCESS );
	}
	else
	{
		TEST_ERR_IF( trotListAppendList( lIn, lIn ) != TROT_RC_SUCCESS );
	}

	TEST_ERR_IF( trotUtf8ToCharacters( lIn, lOut ) != TROT_RC_ERROR_WRONG_KIND );

	trotListFree( &lIn );
	trotListFree( &lOut );


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
static int testBadByte( TROT_INT byte1, TROT_INT byte2, TROT_INT byte3, TROT_INT byte4 )
{
	/* DATA */
	int rc = 0;

	trotList *lIn = NULL;
	trotList *lOut = NULL;


	/* CODE */
	TEST_ERR_IF( trotListInit( &lIn ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListInit( &lOut ) != TROT_RC_SUCCESS );

	TEST_ERR_IF( trotListAppendInt( lIn, byte1 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListAppendInt( lIn, byte2 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListAppendInt( lIn, byte3 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListAppendInt( lIn, byte4 ) != TROT_RC_SUCCESS );

	TEST_ERR_IF( trotUtf8ToCharacters( lIn, lOut ) != TROT_RC_ERROR_UNICODE );

	trotListFree( &lIn );
	trotListFree( &lOut );


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
static int testBadCharacter( TROT_INT character )
{
	/* DATA */
	int rc = 0;

	trotList *lIn = NULL;
	trotList *lOut = NULL;


	/* CODE */
	TEST_ERR_IF( trotListInit( &lIn ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListInit( &lOut ) != TROT_RC_SUCCESS );

	TEST_ERR_IF( trotListAppendInt( lIn, character ) != TROT_RC_SUCCESS );

	TEST_ERR_IF( trotCharactersToUtf8( lIn, lOut ) != TROT_RC_ERROR_UNICODE );

	trotListFree( &lIn );
	trotListFree( &lOut );


	/* CLEANUP */
	cleanup:

	return rc;
}

