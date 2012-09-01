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

#include <sys/stat.h> /* stat */
#include <errno.h> /* errno */
#include <string.h> /* strlen */

/******************************************************************************/
#define PRINT_TOKENS 0

/******************************************************************************/
extern const char *opNames[];

/******************************************************************************/
static int doesFileExist( trotList *lName, TROT_INT *exist );

typedef int (*ProcessFunction)( int dirNumber, int fileNumber, trotList *lName );
static int processFiles( char *directory, ProcessFunction func );

static int testTokenizingGood( int dirNumber, int fileNumber, trotList *lName );
static int testTokenizingBad( int dirNumber, int fileNumber, trotList *lName );
static int testEndOfLines( int dirNumber, int fileNumber, trotList *lName );

static int testDecodingEncodingGood( int dirNumber, int fileNumber, trotList *lName );
static int testDecodingEncodingBad( int dirNumber, int fileNumber, trotList *lName );

/* TODO: name this better */
static int testTokenizeMore1();

static int testEncodingMore();

static int printTokens( trotList *lTokenList );

/******************************************************************************/
int testDecodingEncoding()
{
	/* DATA */
	int rc = 0;


	/* CODE */
	printf( "Testing tokenizing...\n" ); fflush( stdout );

	TEST_ERR_IF( opNames[ TROT_OP_MAX - 1 ] == NULL );
	TEST_ERR_IF( opNames[ TROT_OP_MAX ] != NULL );

	TEST_ERR_IF( processFiles( "./trotTest/testData/TokenFiles/good/", testTokenizingGood ) != 0 );
	TEST_ERR_IF( processFiles( "./trotTest/testData/TokenFiles/bad/", testTokenizingBad ) != 0 );
	TEST_ERR_IF( processFiles( "./trotTest/testData/EndOfLineFiles/", testEndOfLines ) != 0 );

	TEST_ERR_IF( testTokenizeMore1() != 0 );

	printf( "Testing decoding and encoding...\n" ); fflush( stdout );

	TEST_ERR_IF( processFiles( "./trotTest/testData/DecodeFiles/good/", testDecodingEncodingGood ) != 0 );
	TEST_ERR_IF( processFiles( "./trotTest/testData/DecodeFiles/bad/", testDecodingEncodingBad ) != 0 );

	TEST_ERR_IF( testEncodingMore() != 0 );


/* TODO: more */


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
static int doesFileExist( trotList *lName, TROT_INT *exist )
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

	if ( name != NULL )
	{
		trotFree( name );
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

	trotList *lName = NULL;

	int flagTestedAtLeastOne = 0;


	/* CODE */
	TEST_ERR_IF( directory == NULL );
	TEST_ERR_IF( func == NULL );

	/* create trotList filename */
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
/* TODO: find better name for 'doesFileExist' and 'appendCStringToList' etc */
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
static int testTokenizingGood( int dirNumber, int fileNumber, trotList *lName )
{
	/* DATA */
	int rc = 0;

	trotList *lBytes = NULL;
	trotList *lCharacters = NULL;
	trotList *lTokenList = NULL;

	TROT_INT count = 0;
	TROT_INT index = 0;
	TROT_INT tokenType = 0;
	trotList *lToken = NULL;


	/* CODE */
	(void)fileNumber;

	TEST_ERR_IF( lName == NULL );

	/* call load */
	TEST_ERR_IF( load( lName, &lBytes ) != 0 );

	/* convert unicode */
	trotListFree( &lCharacters );

	TEST_ERR_IF( trotListInit( &lCharacters ) != TROT_RC_SUCCESS );

	TEST_ERR_IF( trotUtf8ToCharacters( lBytes, lCharacters ) != TROT_RC_SUCCESS );

	/* tokenize */
	trotListFree( &lTokenList );

	TEST_ERR_IF( trotTokenize( lCharacters, &lTokenList ) != TROT_RC_SUCCESS );

	TEST_ERR_IF( printTokens( lTokenList ) != 0 );

	/* make sure each token is the appropriate type */
	TEST_ERR_IF( trotListGetCount( lTokenList, &count ) != TROT_RC_SUCCESS );

	index = 1;
	while ( index <= count )
	{
		trotListFree( &lToken );

		TEST_ERR_IF( trotListGetList( lTokenList, index, &lToken ) != TROT_RC_SUCCESS );

		TEST_ERR_IF( trotListGetInt( lToken, TOKEN_INDEX_TYPE, &tokenType ) != TROT_RC_SUCCESS );

		TEST_ERR_IF( tokenType != dirNumber );

		index += 1;
	}


	/* CLEANUP */
	cleanup:

	trotListFree( &lBytes );
	trotListFree( &lCharacters );
	trotListFree( &lTokenList );
	trotListFree( &lToken );

	return rc;
}

/******************************************************************************/
static int testTokenizingBad( int dirNumber, int fileNumber, trotList *lName )
{
	/* DATA */
	int rc = 0;

	trotList *lBytes = NULL;
	trotList *lCharacters = NULL;
	trotList *lTokenList = NULL;


	/* CODE */
	(void)dirNumber;
	(void)fileNumber;

	TEST_ERR_IF( lName == NULL );

	/* call load */
	TEST_ERR_IF( load( lName, &lBytes ) != 0 );

	/* convert unicode */
	TEST_ERR_IF( trotListInit( &lCharacters ) != TROT_RC_SUCCESS );

	TEST_ERR_IF( trotUtf8ToCharacters( lBytes, lCharacters ) != TROT_RC_SUCCESS );

	/* tokenize, which should FAIL */
	TEST_ERR_IF( trotTokenize( lCharacters, &lTokenList ) != TROT_RC_ERROR_DECODE );


	/* CLEANUP */
	cleanup:

	trotListFree( &lBytes );
	trotListFree( &lCharacters );
	trotListFree( &lTokenList );

	return rc;
}


/******************************************************************************/
static int testEndOfLines( int dirNumber, int fileNumber, trotList *lName )
{
	/* DATA */
	int rc = 0;

	trotList *lBytes = NULL;
	trotList *lCharacters = NULL;
	trotList *lTokenList = NULL;

	TROT_INT count = 0;
	TROT_INT index = 0;
	TROT_INT tokenType = 0;
	trotList *lToken = NULL;

	TROT_INT line = 0;
	TROT_INT numberValue = 0;


	/* CODE */
	(void)dirNumber;
	(void)fileNumber;

	TEST_ERR_IF( lName == NULL );

	/* call load */
	TEST_ERR_IF( load( lName, &lBytes ) != 0 );

	/* convert unicode */
	trotListFree( &lCharacters );

	TEST_ERR_IF( trotListInit( &lCharacters ) != TROT_RC_SUCCESS );

	TEST_ERR_IF( trotUtf8ToCharacters( lBytes, lCharacters ) != TROT_RC_SUCCESS );

	/* tokenize */
	trotListFree( &lTokenList );

	TEST_ERR_IF( trotTokenize( lCharacters, &lTokenList ) != TROT_RC_SUCCESS );

	/* make sure each number token is on the right line */
	TEST_ERR_IF( trotListGetCount( lTokenList, &count ) != TROT_RC_SUCCESS );

	index = 1;
	while ( index <= count )
	{
		trotListFree( &lToken );

		TEST_ERR_IF( trotListGetList( lTokenList, index, &lToken ) != TROT_RC_SUCCESS );

		TEST_ERR_IF( trotListGetInt( lToken, TOKEN_INDEX_TYPE, &tokenType ) != TROT_RC_SUCCESS );

		if ( tokenType == TOKEN_TYPE_NUMBER )
		{
			TEST_ERR_IF( trotListGetInt( lToken, TOKEN_INDEX_LINE, &line ) != TROT_RC_SUCCESS );

			TEST_ERR_IF( trotListGetInt( lToken, TOKEN_INDEX_VALUE, &numberValue ) != TROT_RC_SUCCESS );

			TEST_ERR_IF( line != numberValue );
		}

		index += 1;
	}


	/* CLEANUP */
	cleanup:
	
	trotListFree( &lBytes );
	trotListFree( &lCharacters );
	trotListFree( &lTokenList );
	trotListFree( &lToken );

	return rc;
}

/******************************************************************************/
static int testDecodingEncodingGood( int dirNumber, int fileNumber, trotList *lName )
{
	/* DATA */
	int rc = 0;

	trotList *lDecodedList1 = NULL;
	trotList *lEncodedList1 = NULL;
	trotList *lEmptyName = NULL;
	trotList *lDecodedList2 = NULL;
	trotList *lEncodedList2 = NULL;

	TROT_LIST_COMPARE_RESULT compareResult;
#if 1
	char *s = NULL;
#endif	


	/* CODE */
	(void)dirNumber;
	(void)fileNumber;

	TEST_ERR_IF( lName == NULL );

	TEST_ERR_IF( trotDecodeFilename( load, lName, &lDecodedList1 ) != TROT_RC_SUCCESS );

	TEST_ERR_IF( trotEncode( lDecodedList1, &lEncodedList1 ) != TROT_RC_SUCCESS );
#if 1
	TEST_ERR_IF( listToCString( lEncodedList1, &s ) != TROT_RC_SUCCESS );

	printf( "lEncodedList1: %s\n", s );

	trotFree( s );
	s = NULL;
#endif

	TEST_ERR_IF( trotListInit( &lEmptyName ) != TROT_RC_SUCCESS );

	TEST_ERR_IF( trotDecodeCharacters( load, lEmptyName, lEncodedList1, &lDecodedList2 ) != TROT_RC_SUCCESS );

	TEST_ERR_IF( trotEncode( lDecodedList2, &lEncodedList2 ) != TROT_RC_SUCCESS );
#if 1
	TEST_ERR_IF( listToCString( lEncodedList2, &s ) != TROT_RC_SUCCESS );

	printf( "lEncodedList2: %s\n", s );

	trotFree( s );
	s = NULL;
#endif

	TEST_ERR_IF( trotListCompare( lDecodedList1, lDecodedList2, &compareResult ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( compareResult != TROT_LIST_COMPARE_EQUAL );
	
	TEST_ERR_IF( trotListCompare( lEncodedList1, lEncodedList2, &compareResult ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( compareResult != TROT_LIST_COMPARE_EQUAL );


	/* CLEANUP */
	cleanup:

	trotListFree( &lDecodedList1 );
	trotListFree( &lEncodedList1 );
	trotListFree( &lEmptyName );
	trotListFree( &lDecodedList2 );
	trotListFree( &lEncodedList2 );

	return rc;
}

/******************************************************************************/
static int testDecodingEncodingBad( int dirNumber, int fileNumber, trotList *lName )
{
	/* DATA */
	int rc = 0;

	trotList *lDecodedList = NULL;


	/* CODE */
	(void)dirNumber;
	(void)fileNumber;

	TEST_ERR_IF( lName == NULL );

	TEST_ERR_IF( trotDecodeFilename( load, lName, &lDecodedList ) != TROT_RC_ERROR_DECODE );


	/* CLEANUP */
	cleanup:

	trotListFree( &lDecodedList );

	return rc;
}

/******************************************************************************/
/*!
	\brief 
	\param 
	\return TROT_RC
*/
static int testTokenizeMore1()
{
	/* DATA */
	int rc = 0;

	trotList *lCharacters = NULL;
	trotList *lTokens = NULL;
	trotList *lTemp = NULL;

	char *s = "[ \"xyz\" word w[ w] w( w) w{ w} word #( abc #) # comment";
	int sLen = 0;
	char *x = NULL;

	int i = 0;



	/* CODE */
	sLen = strlen( s );

	while ( i <= sLen )
	{
		/* create lCharacters */
		trotListFree( &lCharacters );
		TEST_ERR_IF( trotListInit( &lCharacters ) != TROT_RC_SUCCESS );

		x = s;
		while ( (*x) != '\0' )
		{
			TEST_ERR_IF( trotListAppendInt( lCharacters, (*x) ) != TROT_RC_SUCCESS );

			x += 1;
		}

		/* add a list, which should never occur in a list to tokenize */
		trotListFree( &lTemp );
		TEST_ERR_IF( trotListInit( &lTemp ) != TROT_RC_SUCCESS );

		TEST_ERR_IF( trotListInsertList( lCharacters, i + 1, lTemp ) != TROT_RC_SUCCESS );

		/* try to tokenize */
		TEST_ERR_IF( trotTokenize( lCharacters, &lTokens ) != TROT_RC_ERROR_WRONG_KIND );

		/* increment */
		i += 1;
	}


	/* CLEANUP */
	cleanup:

	trotListFree( &lCharacters );
	trotListFree( &lTemp );

	return rc;
}

/******************************************************************************/
/*!
	\brief Prints out a tokenList for debugging.
	\param lTokenList Token list.
	\return int
*/
/* TODO: we have another type function in trotDecodingEncoding.c. Merge into 1? */
static int printTokens( trotList *lTokenList )
{
	/* DATA */
	int rc = 0;

	TROT_INT count = 0;
	TROT_INT index = 0;

	TROT_INT currentLine = 1;

	trotList *lToken = NULL;

	TROT_INT tokenLine = 0;
	TROT_INT tokenType = 0;
	trotList *lValue = NULL;
	TROT_INT tokenNumber = 0;

	trotList *lUtf8Bytes = NULL;
	TROT_INT utf8Count = 0;
	TROT_INT utf8Index = 0;
	TROT_INT utf8Byte = 0;


	/* CODE */
	TEST_ERR_IF( lTokenList == NULL );

	if ( ! PRINT_TOKENS )
	{
		goto cleanup;
	}

	/* get count */
	TEST_ERR_IF( trotListGetCount( lTokenList, &count ) != TROT_RC_SUCCESS );

	/* foreach token */
	index = 1;
	while ( index <= count )
	{
		/* get token */
		trotListFree( &lToken );

		TEST_ERR_IF( trotListGetList( lTokenList, index, &lToken ) != TROT_RC_SUCCESS );

		/* get line */
		TEST_ERR_IF( trotListGetInt( lToken, TOKEN_INDEX_LINE, &tokenLine ) != TROT_RC_SUCCESS );

		/* get type */
		TEST_ERR_IF( trotListGetInt( lToken, TOKEN_INDEX_TYPE, &tokenType ) != TROT_RC_SUCCESS );

		/* new line? */
		if ( currentLine != tokenLine )
		{
			printf( "\n" );
			currentLine = tokenLine;
		}

		/* print */
		if ( tokenType == TOKEN_TYPE_L_BRACKET )
		{
			printf( "[ " );
		}
		else if ( tokenType == TOKEN_TYPE_R_BRACKET )
		{
			printf( "] " );
		}
		else if ( tokenType == TOKEN_TYPE_L_PARENTHESIS )
		{
			printf( "( " );
		}
		else if ( tokenType == TOKEN_TYPE_R_PARENTHESIS )
		{
			printf( ") " );
		}
		else if ( tokenType == TOKEN_TYPE_L_BRACE )
		{
			printf( "{ " );
		}
		else if ( tokenType == TOKEN_TYPE_R_BRACE )
		{
			printf( "} " );
		}
		else if ( tokenType == TOKEN_TYPE_STRING || tokenType == TOKEN_TYPE_WORD )
		{
			/* get value */
			trotListFree( &lValue );

			TEST_ERR_IF( trotListGetList( lToken, TOKEN_INDEX_VALUE, &lValue ) != TROT_RC_SUCCESS );

			/* convert to utf8 */
			trotListFree( &lUtf8Bytes );

			TEST_ERR_IF( trotListInit( &lUtf8Bytes ) != TROT_RC_SUCCESS );

			TEST_ERR_IF( trotCharactersToUtf8( lValue, lUtf8Bytes ) != TROT_RC_SUCCESS );

			if ( tokenType == TOKEN_TYPE_STRING )
			{
				printf( "\"" );
			}
			else
			{
				printf( "W:" );
			}

			/* print */
			TEST_ERR_IF( trotListGetCount( lUtf8Bytes, &utf8Count ) != TROT_RC_SUCCESS );

			utf8Index = 1;
			while ( utf8Index <= utf8Count )
			{
				TEST_ERR_IF( trotListGetInt( lUtf8Bytes, utf8Index, &utf8Byte ) != TROT_RC_SUCCESS );

				printf( "%c", utf8Byte );

				/* next */
				utf8Index += 1;
			}
			

			if ( tokenType == TOKEN_TYPE_STRING )
			{
				printf( "\" " );
			}
			else
			{
				printf( " " );
			}
		}
		else /* NUMBER */
		{
			TEST_ERR_IF( trotListGetInt( lToken, TOKEN_INDEX_VALUE, &tokenNumber ) != TROT_RC_SUCCESS );

			printf( "N:%d ", tokenNumber );
		}

		/* next */
		index += 1;
	}

	printf( "\n\n" );


	/* CLEANUP */
	cleanup:

	trotListFree( &lToken );
	trotListFree( &lValue );
	trotListFree( &lUtf8Bytes );

	return rc;
}

/******************************************************************************/
/*!
	\brief 
	\param 
	\return int
*/
static int testEncodingMore()
{
	/* DATA */
	int rc = 0;

	trotList *l1 = NULL;
	trotList *l2 = NULL;
	trotList *lCharacters = NULL;


	/* CODE */
	/* *** */
	TEST_ERR_IF( trotListInit( &l1 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListInit( &l2 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListAppendList( l1, l2 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListSetTag( l1, TROT_TAG_CODE ) != TROT_RC_SUCCESS );

	TEST_ERR_IF( trotEncode( l1, &lCharacters ) != TROT_RC_ERROR_ENCODE );

	/* *** */
	trotListFree( &l1 );
	trotListFree( &l2 );
	trotListFree( &lCharacters );

	TEST_ERR_IF( trotListInit( &l1 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListAppendInt( l1, TROT_OP_PUSH_INT ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListSetTag( l1, TROT_TAG_CODE ) != TROT_RC_SUCCESS );

	TEST_ERR_IF( trotEncode( l1, &lCharacters ) != TROT_RC_ERROR_ENCODE );

	/* *** */
	trotListFree( &l1 );
	trotListFree( &l2 );
	trotListFree( &lCharacters );

	TEST_ERR_IF( trotListInit( &l1 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListInit( &l2 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListAppendInt( l1, TROT_OP_PUSH_INT ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListAppendList( l1, l2 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListSetTag( l1, TROT_TAG_CODE ) != TROT_RC_SUCCESS );

	TEST_ERR_IF( trotEncode( l1, &lCharacters ) != TROT_RC_ERROR_ENCODE );

	/* *** */
	trotListFree( &l1 );
	trotListFree( &l2 );
	trotListFree( &lCharacters );

	TEST_ERR_IF( trotListInit( &l1 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListAppendInt( l1, TROT_OP_PUSH_LIST ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListSetTag( l1, TROT_TAG_CODE ) != TROT_RC_SUCCESS );

	TEST_ERR_IF( trotEncode( l1, &lCharacters ) != TROT_RC_ERROR_ENCODE );

	/* *** */
	trotListFree( &l1 );
	trotListFree( &l2 );
	trotListFree( &lCharacters );

	TEST_ERR_IF( trotListInit( &l1 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListAppendInt( l1, TROT_OP_PUSH_LIST ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListAppendInt( l1, 5 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListSetTag( l1, TROT_TAG_CODE ) != TROT_RC_SUCCESS );

	TEST_ERR_IF( trotEncode( l1, &lCharacters ) != TROT_RC_ERROR_ENCODE );

	/* *** */
	trotListFree( &l1 );
	trotListFree( &l2 );
	trotListFree( &lCharacters );

	TEST_ERR_IF( trotListInit( &l1 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListAppendInt( l1, -1 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListSetTag( l1, TROT_TAG_CODE ) != TROT_RC_SUCCESS );

	TEST_ERR_IF( trotEncode( l1, &lCharacters ) != TROT_RC_ERROR_ENCODE );

	/* *** */
	trotListFree( &l1 );
	trotListFree( &l2 );
	trotListFree( &lCharacters );

	TEST_ERR_IF( trotListInit( &l1 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListAppendInt( l1, TROT_OP_MAX + 1 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListSetTag( l1, TROT_TAG_CODE ) != TROT_RC_SUCCESS );

	TEST_ERR_IF( trotEncode( l1, &lCharacters ) != TROT_RC_ERROR_ENCODE );


	/* CLEANUP */
	cleanup:

	trotListFree( &l1 );
	trotListFree( &l2 );
	trotListFree( &lCharacters );

	return rc;
}

