/*
Copyright (c) 2010,2011, Jeremiah Martell
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
static int testCharacterToUtf8ToCharacter( INT_TYPE start, INT_TYPE end, int numberOfBytes );
static int testBadByte( INT_TYPE byte1, INT_TYPE byte2, INT_TYPE byte3, INT_TYPE byte4 );
static int testBadKind( INT_TYPE byte1, INT_TYPE byte2, INT_TYPE byte3, INT_TYPE byte4 );
static int testBadCharacter( INT_TYPE character );

/******************************************************************************/
int testUnicode()
{
	/* DATA */
	int rc = TROT_LIST_SUCCESS;

	trotListRef *lr = NULL;


	/* CODE */
	printf( "Testing unicode functions...\n" ); fflush( stdout );

	printf( "  Testing Character > utf8 > Character...\n" ); fflush( stdout );

	/* 1 byte: 0x00 - 0x7F */
	TEST_ERR_IF( testCharacterToUtf8ToCharacter( 0x00, 0x7F, 1 ) != TROT_LIST_SUCCESS );
	printf( "." ); fflush( stdout );

	/* 2 byte: 0x80 - 0x7FF */
	TEST_ERR_IF( testCharacterToUtf8ToCharacter( 0x80, 0x7FF, 2 ) != TROT_LIST_SUCCESS );
	printf( "." ); fflush( stdout );

	/* 3 byte: 0x800 - 0xD7FF, 0xE000 - 0xFFFF */
	TEST_ERR_IF( testCharacterToUtf8ToCharacter( 0x800, 0xD7FF, 3 ) != TROT_LIST_SUCCESS );
	printf( "." ); fflush( stdout );

	TEST_ERR_IF( testCharacterToUtf8ToCharacter( 0xE000, 0xFFFF, 3 ) != TROT_LIST_SUCCESS );
	printf( "." ); fflush( stdout );

	/* 4 byte: 0x10000 - 0x10FFFF */
	TEST_ERR_IF( testCharacterToUtf8ToCharacter( 0x10000, 0x10FFFF, 4 ) != TROT_LIST_SUCCESS );
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

	TEST_ERR_IF( trotListRefInit( &lr ) != TROT_LIST_SUCCESS );
	TEST_ERR_IF( trotListRefAppendListTwin( lr, lr ) != TROT_LIST_SUCCESS );
	TEST_ERR_IF( trotCharactersToUtf8( lr, lr ) != TROT_LIST_ERROR_WRONG_KIND );
	trotListRefFree( &lr );
	

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
static int testCharacterToUtf8ToCharacter( INT_TYPE start, INT_TYPE end, int numberOfBytesShouldBe )
{
	/* DATA */
	int rc = 0;

	INT_TYPE characterIn = 0;
	INT_TYPE characterOut = 0;
	trotListRef *lrCharacterIn = NULL;
	trotListRef *lrBytes = NULL;
	trotListRef *lrCharacterOut = NULL;

	INT_TYPE numberOfBytesActual = 0;
	INT_TYPE numberOfCharacters = 0;


	/* CODE */
	characterIn = start;
	while ( characterIn <= end )
	{
		TEST_ERR_IF( trotListRefInit( &lrCharacterIn ) != TROT_LIST_SUCCESS );
		TEST_ERR_IF( trotListRefInit( &lrBytes ) != TROT_LIST_SUCCESS );
		TEST_ERR_IF( trotListRefInit( &lrCharacterOut ) != TROT_LIST_SUCCESS );

		TEST_ERR_IF( trotListRefAppendInt( lrCharacterIn, characterIn ) != TROT_LIST_SUCCESS );

		TEST_ERR_IF( trotCharactersToUtf8( lrCharacterIn, lrBytes ) != TROT_LIST_SUCCESS );

		TEST_ERR_IF( trotListRefGetCount( lrBytes, &numberOfBytesActual ) != TROT_LIST_SUCCESS );
		TEST_ERR_IF( numberOfBytesActual != numberOfBytesShouldBe );

		TEST_ERR_IF( trotUtf8ToCharacters( lrBytes, lrCharacterOut ) != TROT_LIST_SUCCESS );

		TEST_ERR_IF( trotListRefGetCount( lrCharacterOut, &numberOfCharacters ) != TROT_LIST_SUCCESS );
		TEST_ERR_IF( numberOfCharacters != 1 );

		TEST_ERR_IF( trotListRefGetInt( lrCharacterOut, 1, &characterOut ) != TROT_LIST_SUCCESS );

		TEST_ERR_IF( characterIn != characterOut );

		trotListRefFree( &lrCharacterIn );
		trotListRefFree( &lrBytes );
		trotListRefFree( &lrCharacterOut );

		/* *** */
		characterIn += 1;
	}


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
static int testBadKind( INT_TYPE byte1, INT_TYPE byte2, INT_TYPE byte3, INT_TYPE byte4 )
{
	/* DATA */
	int rc = 0;

	trotListRef *lrIn = NULL;
	trotListRef *lrOut = NULL;


	/* CODE */
	TEST_ERR_IF( trotListRefInit( &lrIn ) != TROT_LIST_SUCCESS );
	TEST_ERR_IF( trotListRefInit( &lrOut ) != TROT_LIST_SUCCESS );

	if ( byte1 != -1 )
	{
		TEST_ERR_IF( trotListRefAppendInt( lrIn, byte1 ) != TROT_LIST_SUCCESS );
	}
	else
	{
		TEST_ERR_IF( trotListRefAppendListTwin( lrIn, lrIn ) != TROT_LIST_SUCCESS );
	}

	if ( byte2 != -1 )
	{
		TEST_ERR_IF( trotListRefAppendInt( lrIn, byte2 ) != TROT_LIST_SUCCESS );
	}
	else
	{
		TEST_ERR_IF( trotListRefAppendListTwin( lrIn, lrIn ) != TROT_LIST_SUCCESS );
	}

	if ( byte3 != -1 )
	{
		TEST_ERR_IF( trotListRefAppendInt( lrIn, byte3 ) != TROT_LIST_SUCCESS );
	}
	else
	{
		TEST_ERR_IF( trotListRefAppendListTwin( lrIn, lrIn ) != TROT_LIST_SUCCESS );
	}

	if ( byte4 != -1 )
	{
		TEST_ERR_IF( trotListRefAppendInt( lrIn, byte4 ) != TROT_LIST_SUCCESS );
	}
	else
	{
		TEST_ERR_IF( trotListRefAppendListTwin( lrIn, lrIn ) != TROT_LIST_SUCCESS );
	}

	TEST_ERR_IF( trotUtf8ToCharacters( lrIn, lrOut ) != TROT_LIST_ERROR_WRONG_KIND );

	trotListRefFree( &lrIn );
	trotListRefFree( &lrOut );


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
static int testBadByte( INT_TYPE byte1, INT_TYPE byte2, INT_TYPE byte3, INT_TYPE byte4 )
{
	/* DATA */
	int rc = 0;

	trotListRef *lrIn = NULL;
	trotListRef *lrOut = NULL;


	/* CODE */
	TEST_ERR_IF( trotListRefInit( &lrIn ) != TROT_LIST_SUCCESS );
	TEST_ERR_IF( trotListRefInit( &lrOut ) != TROT_LIST_SUCCESS );

	TEST_ERR_IF( trotListRefAppendInt( lrIn, byte1 ) != TROT_LIST_SUCCESS );
	TEST_ERR_IF( trotListRefAppendInt( lrIn, byte2 ) != TROT_LIST_SUCCESS );
	TEST_ERR_IF( trotListRefAppendInt( lrIn, byte3 ) != TROT_LIST_SUCCESS );
	TEST_ERR_IF( trotListRefAppendInt( lrIn, byte4 ) != TROT_LIST_SUCCESS );

	TEST_ERR_IF( trotUtf8ToCharacters( lrIn, lrOut ) != TROT_LIST_ERROR_UNICODE );

	trotListRefFree( &lrIn );
	trotListRefFree( &lrOut );


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
static int testBadCharacter( INT_TYPE character )
{
	/* DATA */
	int rc = 0;

	trotListRef *lrIn = NULL;
	trotListRef *lrOut = NULL;


	/* CODE */
	TEST_ERR_IF( trotListRefInit( &lrIn ) != TROT_LIST_SUCCESS );
	TEST_ERR_IF( trotListRefInit( &lrOut ) != TROT_LIST_SUCCESS );

	TEST_ERR_IF( trotListRefAppendInt( lrIn, character ) != TROT_LIST_SUCCESS );

	TEST_ERR_IF( trotCharactersToUtf8( lrIn, lrOut ) != TROT_LIST_ERROR_UNICODE );

	trotListRefFree( &lrIn );
	trotListRefFree( &lrOut );


	/* CLEANUP */
	cleanup:

	return rc;
}

