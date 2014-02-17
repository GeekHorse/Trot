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
#include "trot.h"
#include "trotInternal.h"

#include "trotTestCommon.h"

#include <sys/stat.h> /* stat */
#include <errno.h> /* errno */
#include <string.h> /* strlen */

/******************************************************************************/
#define PRINT_GOOD_TEST_ENCODINGS 0

/******************************************************************************/
static int doesFileExist( TrotList *lName, TROT_INT *exist );

typedef int (*ProcessFunction)( int dirNumber, int fileNumber, TrotList *lName );
static int processFiles( char *directory, ProcessFunction func );

static int testDecodingEncodingGood( int dirNumber, int fileNumber, TrotList *lName );
static int testDecodingEncodingBad( int dirNumber, int fileNumber, TrotList *lName );
static int testDecodingAddLists();

/******************************************************************************/
int testDecodingEncoding()
{
	/* DATA */
	int rc = 0;


	/* CODE */
	printf( "Testing decoding and encoding...\n" ); fflush( stdout );

	TEST_ERR_IF( processFiles( "./trotTest/testData/DecodeFiles/good/", testDecodingEncodingGood ) != 0 );
	TEST_ERR_IF( processFiles( "./trotTest/testData/DecodeFiles/bad/", testDecodingEncodingBad ) != 0 );
	TEST_ERR_IF( testDecodingAddLists() != 0 );


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
static int doesFileExist( TrotList *lName, TROT_INT *exist )
{
	/* DATA */
	int rc = 0;

	char *name = NULL;

	struct stat st;


	/* CODE */
	TEST_ERR_IF( lName == NULL );
	TEST_ERR_IF( exist == NULL );

	TEST_ERR_IF( listToCString( lName, &name ) != TROT_RC_SUCCESS );

	if ( stat( name, &st ) == 0 )
	{
		(*exist) = 1;
	}
	else if ( errno == ENOENT )
	{
		(*exist) = 0;
	}
	else
	{
		TEST_ERR_IF( 1 );
	}


	/* CLEANUP */
	cleanup:

	errno = 0;

	if ( name != NULL )
	{
		trotHookFree( name );
	}

	return rc;
}

/******************************************************************************/
/*!
	\brief Goes through directory sending files to func.
	\param directory Directory to process.
	\param func Processing function to pass each file to.
	\return int
*/
static int processFiles( char *directory, ProcessFunction func )
{
	/* DATA */
	int rc = 0;

	int dirNumber = 1;
	int fileNumber = 1;

	TROT_INT exist = 0;

	TrotList *lName = NULL;

	int flagTestedAtLeastOne = 0;


	/* CODE */
	TEST_ERR_IF( directory == NULL );
	TEST_ERR_IF( func == NULL );

	/* create TrotList filename */
	TEST_ERR_IF( trotListInit( &lName ) != TROT_RC_SUCCESS );

	TEST_ERR_IF( appendCStringToList( lName, directory ) != TROT_RC_SUCCESS );

	/* foreach directory */
	while ( 1 )
	{
		TEST_ERR_IF( trotListAppendInt( lName, ( dirNumber / 10 ) + '0' ) != TROT_RC_SUCCESS );
		TEST_ERR_IF( trotListAppendInt( lName, ( dirNumber % 10 ) + '0' ) != TROT_RC_SUCCESS );

		/* does directory exist? */
		TEST_ERR_IF( doesFileExist( lName, &exist ) != 0 );

		if ( exist == 0 )
		{
			break;
		}

		TEST_ERR_IF( trotListAppendInt( lName, '/' ) != TROT_RC_SUCCESS );

		/* foreach file in this directory */
		fileNumber = 1;

		while ( 1 )
		{
			TEST_ERR_IF( trotListAppendInt( lName, ( fileNumber / 10 ) + '0' ) != TROT_RC_SUCCESS );
			TEST_ERR_IF( trotListAppendInt( lName, ( fileNumber % 10 ) + '0' ) != TROT_RC_SUCCESS );

			/* does file exist? */
			TEST_ERR_IF( doesFileExist( lName, &exist ) != 0 );

			if ( exist == 0 )
			{
				/* remove our number from end of name */
				TEST_ERR_IF( trotListRemove( lName, -1 ) != TROT_RC_SUCCESS );
				TEST_ERR_IF( trotListRemove( lName, -1 ) != TROT_RC_SUCCESS );

				break;
			}

			/* send to func */
			TEST_ERR_IF( func( dirNumber, fileNumber, lName ) != 0 );

			/* set flag */
			flagTestedAtLeastOne = 1;

			/* remove our number from end of name */
			TEST_ERR_IF( trotListRemove( lName, -1 ) != TROT_RC_SUCCESS );
			TEST_ERR_IF( trotListRemove( lName, -1 ) != TROT_RC_SUCCESS );

			/* next */
			fileNumber += 1;
		}

		/* remove the last '/' from end of name */
		TEST_ERR_IF( trotListRemove( lName, -1 ) != TROT_RC_SUCCESS );

		/* remove our number from end of name */
		TEST_ERR_IF( trotListRemove( lName, -1 ) != TROT_RC_SUCCESS );
		TEST_ERR_IF( trotListRemove( lName, -1 ) != TROT_RC_SUCCESS );

		/* next */
		dirNumber += 1;
	}

	/* make sure we tested at least one file */
	TEST_ERR_IF( flagTestedAtLeastOne == 0 );


	/* CLEANUP */
	cleanup:

	trotListFree( &lName );

	return rc;
}

/******************************************************************************/
static int testDecodingEncodingGood( int dirNumber, int fileNumber, TrotList *lName )
{
	/* DATA */
	int rc = 0;

	TrotList *lBytes = NULL;

	TrotList *lDecodedList1 = NULL;
	TrotList *lEncodedList1 = NULL;
	TrotList *lDecodedList2 = NULL;
	TrotList *lEncodedList2 = NULL;
	TrotList *lEncodedList3 = NULL;

	TrotList *lExpectedEncoding = NULL;

	TROT_LIST_COMPARE_RESULT compareResult;
#if PRINT_GOOD_TEST_ENCODINGS
	char *s = NULL;
#endif	


	/* CODE */
	(void)dirNumber;
	(void)fileNumber;

	TEST_ERR_IF( lName == NULL );

	TEST_ERR_IF( load( lName, &lBytes ) != 0 );

	TEST_ERR_IF( trotDecode( lBytes, &lDecodedList1 ) != TROT_RC_SUCCESS );

	TEST_ERR_IF( trotEncode( lDecodedList1, &lEncodedList1 ) != TROT_RC_SUCCESS );
#if PRINT_GOOD_TEST_ENCODINGS
	TEST_ERR_IF( listToCString( lEncodedList1, &s ) != TROT_RC_SUCCESS );

	printf( "lEncodedList1:     \"%s\"\n", s );

	trotHookFree( s );
	s = NULL;
#endif

	TEST_ERR_IF( trotDecode( lEncodedList1, &lDecodedList2 ) != TROT_RC_SUCCESS );

	TEST_ERR_IF( trotEncode( lDecodedList2, &lEncodedList2 ) != TROT_RC_SUCCESS );
#if PRINT_GOOD_TEST_ENCODINGS
	TEST_ERR_IF( listToCString( lEncodedList2, &s ) != TROT_RC_SUCCESS );

	printf( "lEncodedList2:     \"%s\"\n", s );

	trotHookFree( s );
	s = NULL;
#endif

	TEST_ERR_IF( trotListCompare( lEncodedList1, lEncodedList2, &compareResult ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( compareResult != TROT_LIST_COMPARE_EQUAL );

	/* we encode twice to make sure we've correctly reset the internal encoding
	   numbers in the lists */
	TEST_ERR_IF( trotEncode( lDecodedList2, &lEncodedList3 ) != TROT_RC_SUCCESS );
#if PRINT_GOOD_TEST_ENCODINGS
	TEST_ERR_IF( listToCString( lEncodedList3, &s ) != TROT_RC_SUCCESS );

	printf( "lEncodedList3:     \"%s\"\n", s );

	trotHookFree( s );
	s = NULL;
#endif

	TEST_ERR_IF( trotListCompare( lEncodedList1, lEncodedList3, &compareResult ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( compareResult != TROT_LIST_COMPARE_EQUAL );

	/* read in the "expected" encoding, and see if it matches */
	TEST_ERR_IF( trotListAppendInt( lName, 'E' ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( load( lName, &lExpectedEncoding ) != 0 );
	TEST_ERR_IF( trotListRemove( lName, -1 ) != TROT_RC_SUCCESS );
	
#if PRINT_GOOD_TEST_ENCODINGS
	TEST_ERR_IF( listToCString( lExpectedEncoding, &s ) != TROT_RC_SUCCESS );

	printf( "lExpectedEncoding: \"%s\"\n", s );

	trotHookFree( s );
	s = NULL;
#endif

	TEST_ERR_IF( trotListCompare( lEncodedList1, lExpectedEncoding, &compareResult ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( compareResult != TROT_LIST_COMPARE_EQUAL );


	/* CLEANUP */
	cleanup:

	trotListFree( &lBytes );
	trotListFree( &lDecodedList1 );
	trotListFree( &lEncodedList1 );
	trotListFree( &lDecodedList2 );
	trotListFree( &lEncodedList2 );
	trotListFree( &lExpectedEncoding );

	return rc;
}

/******************************************************************************/
static int testDecodingEncodingBad( int dirNumber, int fileNumber, TrotList *lName )
{
	/* DATA */
	int rc = 0;
	TROT_RC trot_rc = TROT_RC_SUCCESS;

	TrotList *lBytes = NULL;

	TrotList *lDecodedList = NULL;


	/* CODE */
	(void)dirNumber;
	(void)fileNumber;

	TEST_ERR_IF( lName == NULL );

	TEST_ERR_IF( load( lName, &lBytes ) != 0 );

	trot_rc = trotDecode( lBytes, &lDecodedList );
	TEST_ERR_IF( trot_rc == TROT_RC_SUCCESS );


	/* CLEANUP */
	cleanup:

	trotListFree( &lBytes );
	trotListFree( &lDecodedList );

	return rc;
}

/******************************************************************************/
static int testDecodingAddLists()
{
	/* DATA */
	int rc = 0;
	TROT_RC trot_rc = TROT_RC_SUCCESS;

	TROT_INT i = 0;
	TROT_INT count = 0;
	char *characters = "[ [ ] @.1 1 ]";
	TrotList *lCharacters = NULL;
	TrotList *lDecoded = NULL;


	/* CODE */
	i = 1;
	while ( 1 )
	{
		/* create lCharacters */
		trotListFree( &lCharacters );
		trot_rc = trotListInit( &lCharacters );
		TEST_ERR_IF( trot_rc != TROT_RC_SUCCESS );

		trot_rc = appendCStringToList( lCharacters, characters );
		TEST_ERR_IF( trot_rc != TROT_RC_SUCCESS );

		/* get count */
		trotListGetCount( lCharacters, &count );

		if ( i > count + 1 )
		{
			break;
		}

		/* add a list at i, we reuse lCharacters because it already exists */
		trot_rc = trotListInsertList( lCharacters, i, lCharacters );
		TEST_ERR_IF( trot_rc != TROT_RC_SUCCESS );

		/* decode, which should fail */
		trotListFree( &lDecoded );
		trot_rc = trotDecode( lCharacters, &lDecoded );
		TEST_ERR_IF( trot_rc == TROT_RC_SUCCESS );

		/* increment */
		i += 1;
	}


	/* CLEANUP */
	cleanup:

	trotListFree( &lCharacters );
	trotListFree( &lDecoded );

	return rc;
}

