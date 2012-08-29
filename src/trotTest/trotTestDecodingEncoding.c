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
static int doesFileExist( trotListRef *lrName, INT_TYPE *exist );

typedef int (*ProcessFunction)( int dirNumber, int fileNumber, trotListRef *lrName );
static int processFiles( char *directory, ProcessFunction func );

static int testTokenizingGood( int dirNumber, int fileNumber, trotListRef *lrName );
static int testTokenizingBad( int dirNumber, int fileNumber, trotListRef *lrName );
static int testEndOfLines( int dirNumber, int fileNumber, trotListRef *lrName );

static int testDecodingEncodingGood( int dirNumber, int fileNumber, trotListRef *lrName );
static int testDecodingEncodingBad( int dirNumber, int fileNumber, trotListRef *lrName );

/* TODO: name this better */
static int testTokenizeMore1();

static int testEncodingMore();

static int printTokens( trotListRef *lrTokenList );

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
static int doesFileExist( trotListRef *lrName, INT_TYPE *exist )
{
	/* DATA */
	int rc = 0;

	char *name = NULL;

	struct stat st;


	/* CODE */
	TEST_ERR_IF( lrName == NULL );
	TEST_ERR_IF( exist == NULL );

	TEST_ERR_IF( listToCString( lrName, &name ) != TROT_LIST_SUCCESS );

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

	INT_TYPE exist = 0;

	trotListRef *lrName = NULL;

	int flagTestedAtLeastOne = 0;


	/* CODE */
	TEST_ERR_IF( directory == NULL );
	TEST_ERR_IF( func == NULL );

	/* create trotList filename */
	TEST_ERR_IF( trotListRefInit( &lrName ) != TROT_LIST_SUCCESS );

	TEST_ERR_IF( appendCStringToList( lrName, directory ) != TROT_LIST_SUCCESS );

	/* foreach directory */
	while ( 1 )
	{
		TEST_ERR_IF( trotListRefAppendInt( lrName, ( dirNumber / 10 ) + '0' ) != TROT_LIST_SUCCESS );
		TEST_ERR_IF( trotListRefAppendInt( lrName, ( dirNumber % 10 ) + '0' ) != TROT_LIST_SUCCESS );

		/* does directory exist? */
		TEST_ERR_IF( doesFileExist( lrName, &exist ) != 0 );

		if ( exist == 0 )
		{
			break;
		}

		TEST_ERR_IF( trotListRefAppendInt( lrName, '/' ) != TROT_LIST_SUCCESS );

		/* foreach file in this directory */
		fileNumber = 1;

		while ( 1 )
		{
			TEST_ERR_IF( trotListRefAppendInt( lrName, ( fileNumber / 10 ) + '0' ) != TROT_LIST_SUCCESS );
			TEST_ERR_IF( trotListRefAppendInt( lrName, ( fileNumber % 10 ) + '0' ) != TROT_LIST_SUCCESS );

			/* does file exist? */
/* TODO: find better name for 'doesFileExist' and 'appendCStringToList' etc */
			TEST_ERR_IF( doesFileExist( lrName, &exist ) != 0 );

			if ( exist == 0 )
			{
				/* remove our number from end of name */
				TEST_ERR_IF( trotListRefRemove( lrName, -1 ) != TROT_LIST_SUCCESS );
				TEST_ERR_IF( trotListRefRemove( lrName, -1 ) != TROT_LIST_SUCCESS );

				break;
			}

			/* send to func */
			TEST_ERR_IF( func( dirNumber, fileNumber, lrName ) != 0 );

			/* set flag */
			flagTestedAtLeastOne = 1;

			/* remove our number from end of name */
			TEST_ERR_IF( trotListRefRemove( lrName, -1 ) != TROT_LIST_SUCCESS );
			TEST_ERR_IF( trotListRefRemove( lrName, -1 ) != TROT_LIST_SUCCESS );

			/* next */
			fileNumber += 1;
		}

		/* remove the last '/' from end of name */
		TEST_ERR_IF( trotListRefRemove( lrName, -1 ) != TROT_LIST_SUCCESS );

		/* remove our number from end of name */
		TEST_ERR_IF( trotListRefRemove( lrName, -1 ) != TROT_LIST_SUCCESS );
		TEST_ERR_IF( trotListRefRemove( lrName, -1 ) != TROT_LIST_SUCCESS );

		/* next */
		dirNumber += 1;
	}

	/* make sure we tested at least one file */
	TEST_ERR_IF( flagTestedAtLeastOne == 0 );


	/* CLEANUP */
	cleanup:

	trotListRefFree( &lrName );

	return rc;
}

/******************************************************************************/
static int testTokenizingGood( int dirNumber, int fileNumber, trotListRef *lrName )
{
	/* DATA */
	int rc = 0;

	trotListRef *lrBytes = NULL;
	trotListRef *lrCharacters = NULL;
	trotListRef *lrTokenList = NULL;

	INT_TYPE count = 0;
	INT_TYPE index = 0;
	INT_TYPE tokenType = 0;
	trotListRef *lrToken = NULL;


	/* CODE */
	(void)fileNumber;

	TEST_ERR_IF( lrName == NULL );

	/* call load */
	TEST_ERR_IF( load( lrName, &lrBytes ) != 0 );

	/* convert unicode */
	trotListRefFree( &lrCharacters );

	TEST_ERR_IF( trotListRefInit( &lrCharacters ) != TROT_LIST_SUCCESS );

	TEST_ERR_IF( trotUtf8ToCharacters( lrBytes, lrCharacters ) != TROT_LIST_SUCCESS );

	/* tokenize */
	trotListRefFree( &lrTokenList );

	TEST_ERR_IF( trotTokenize( lrCharacters, &lrTokenList ) != TROT_LIST_SUCCESS );

	TEST_ERR_IF( printTokens( lrTokenList ) != 0 );

	/* make sure each token is the appropriate type */
	TEST_ERR_IF( trotListRefGetCount( lrTokenList, &count ) != TROT_LIST_SUCCESS );

	index = 1;
	while ( index <= count )
	{
		trotListRefFree( &lrToken );

		TEST_ERR_IF( trotListRefGetListTwin( lrTokenList, index, &lrToken ) != TROT_LIST_SUCCESS );

		TEST_ERR_IF( trotListRefGetInt( lrToken, TOKEN_INDEX_TYPE, &tokenType ) != TROT_LIST_SUCCESS );

		TEST_ERR_IF( tokenType != dirNumber );

		index += 1;
	}


	/* CLEANUP */
	cleanup:

	trotListRefFree( &lrBytes );
	trotListRefFree( &lrCharacters );
	trotListRefFree( &lrTokenList );
	trotListRefFree( &lrToken );

	return rc;
}

/******************************************************************************/
static int testTokenizingBad( int dirNumber, int fileNumber, trotListRef *lrName )
{
	/* DATA */
	int rc = 0;

	trotListRef *lrBytes = NULL;
	trotListRef *lrCharacters = NULL;
	trotListRef *lrTokenList = NULL;


	/* CODE */
	(void)dirNumber;
	(void)fileNumber;

	TEST_ERR_IF( lrName == NULL );

	/* call load */
	TEST_ERR_IF( load( lrName, &lrBytes ) != 0 );

	/* convert unicode */
	TEST_ERR_IF( trotListRefInit( &lrCharacters ) != TROT_LIST_SUCCESS );

	TEST_ERR_IF( trotUtf8ToCharacters( lrBytes, lrCharacters ) != TROT_LIST_SUCCESS );

	/* tokenize, which should FAIL */
	TEST_ERR_IF( trotTokenize( lrCharacters, &lrTokenList ) != TROT_LIST_ERROR_DECODE );


	/* CLEANUP */
	cleanup:

	trotListRefFree( &lrBytes );
	trotListRefFree( &lrCharacters );
	trotListRefFree( &lrTokenList );

	return rc;
}


/******************************************************************************/
static int testEndOfLines( int dirNumber, int fileNumber, trotListRef *lrName )
{
	/* DATA */
	int rc = 0;

	trotListRef *lrBytes = NULL;
	trotListRef *lrCharacters = NULL;
	trotListRef *lrTokenList = NULL;

	INT_TYPE count = 0;
	INT_TYPE index = 0;
	INT_TYPE tokenType = 0;
	trotListRef *lrToken = NULL;

	INT_TYPE line = 0;
	INT_TYPE numberValue = 0;


	/* CODE */
	(void)dirNumber;
	(void)fileNumber;

	TEST_ERR_IF( lrName == NULL );

	/* call load */
	TEST_ERR_IF( load( lrName, &lrBytes ) != 0 );

	/* convert unicode */
	trotListRefFree( &lrCharacters );

	TEST_ERR_IF( trotListRefInit( &lrCharacters ) != TROT_LIST_SUCCESS );

	TEST_ERR_IF( trotUtf8ToCharacters( lrBytes, lrCharacters ) != TROT_LIST_SUCCESS );

	/* tokenize */
	trotListRefFree( &lrTokenList );

	TEST_ERR_IF( trotTokenize( lrCharacters, &lrTokenList ) != TROT_LIST_SUCCESS );

	/* make sure each number token is on the right line */
	TEST_ERR_IF( trotListRefGetCount( lrTokenList, &count ) != TROT_LIST_SUCCESS );

	index = 1;
	while ( index <= count )
	{
		trotListRefFree( &lrToken );

		TEST_ERR_IF( trotListRefGetListTwin( lrTokenList, index, &lrToken ) != TROT_LIST_SUCCESS );

		TEST_ERR_IF( trotListRefGetInt( lrToken, TOKEN_INDEX_TYPE, &tokenType ) != TROT_LIST_SUCCESS );

		if ( tokenType == TOKEN_NUMBER )
		{
			TEST_ERR_IF( trotListRefGetInt( lrToken, TOKEN_INDEX_LINE, &line ) != TROT_LIST_SUCCESS );

			TEST_ERR_IF( trotListRefGetInt( lrToken, TOKEN_INDEX_VALUE, &numberValue ) != TROT_LIST_SUCCESS );

			TEST_ERR_IF( line != numberValue );
		}

		index += 1;
	}


	/* CLEANUP */
	cleanup:
	
	trotListRefFree( &lrBytes );
	trotListRefFree( &lrCharacters );
	trotListRefFree( &lrTokenList );
	trotListRefFree( &lrToken );

	return rc;
}

/******************************************************************************/
static int testDecodingEncodingGood( int dirNumber, int fileNumber, trotListRef *lrName )
{
	/* DATA */
	int rc = 0;

	trotListRef *lrDecodedList1 = NULL;
	trotListRef *lrEncodedList1 = NULL;
	trotListRef *lrEmptyName = NULL;
	trotListRef *lrDecodedList2 = NULL;
	trotListRef *lrEncodedList2 = NULL;

	TROT_LIST_COMPARE_RESULT compareResult;
#if 1
	char *s = NULL;
#endif	


	/* CODE */
	(void)dirNumber;
	(void)fileNumber;

	TEST_ERR_IF( lrName == NULL );

	TEST_ERR_IF( trotDecodeFilename( load, lrName, &lrDecodedList1 ) != TROT_LIST_SUCCESS );

	TEST_ERR_IF( trotEncode( lrDecodedList1, &lrEncodedList1 ) != TROT_LIST_SUCCESS );
#if 1
	TEST_ERR_IF( listToCString( lrEncodedList1, &s ) != TROT_LIST_SUCCESS );

	printf( "lrEncodedList1: %s\n", s );

	trotFree( s );
	s = NULL;
#endif

	TEST_ERR_IF( trotListRefInit( &lrEmptyName ) != TROT_LIST_SUCCESS );

	TEST_ERR_IF( trotDecodeCharacters( load, lrEmptyName, lrEncodedList1, &lrDecodedList2 ) != TROT_LIST_SUCCESS );

	TEST_ERR_IF( trotEncode( lrDecodedList2, &lrEncodedList2 ) != TROT_LIST_SUCCESS );
#if 1
	TEST_ERR_IF( listToCString( lrEncodedList2, &s ) != TROT_LIST_SUCCESS );

	printf( "lrEncodedList2: %s\n", s );

	trotFree( s );
	s = NULL;
#endif

	TEST_ERR_IF( trotListRefCompare( lrDecodedList1, lrDecodedList2, &compareResult ) != TROT_LIST_SUCCESS );
	TEST_ERR_IF( compareResult != TROT_LIST_COMPARE_EQUAL );
	
	TEST_ERR_IF( trotListRefCompare( lrEncodedList1, lrEncodedList2, &compareResult ) != TROT_LIST_SUCCESS );
	TEST_ERR_IF( compareResult != TROT_LIST_COMPARE_EQUAL );


	/* CLEANUP */
	cleanup:

	trotListRefFree( &lrDecodedList1 );
	trotListRefFree( &lrEncodedList1 );
	trotListRefFree( &lrEmptyName );
	trotListRefFree( &lrDecodedList2 );
	trotListRefFree( &lrEncodedList2 );

	return rc;
}

/******************************************************************************/
static int testDecodingEncodingBad( int dirNumber, int fileNumber, trotListRef *lrName )
{
	/* DATA */
	int rc = 0;

	trotListRef *lrDecodedList = NULL;


	/* CODE */
	(void)dirNumber;
	(void)fileNumber;

	TEST_ERR_IF( lrName == NULL );

	TEST_ERR_IF( trotDecodeFilename( load, lrName, &lrDecodedList ) != TROT_LIST_ERROR_DECODE );


	/* CLEANUP */
	cleanup:

	trotListRefFree( &lrDecodedList );

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

	trotListRef *lrCharacters = NULL;
	trotListRef *lrTokens = NULL;
	trotListRef *lrTemp = NULL;

	char *s = "[ \"xyz\" word w[ w] w( w) w{ w} word #( abc #) # comment";
	int sLen = 0;
	char *x = NULL;

	int i = 0;



	/* CODE */
	sLen = strlen( s );

	while ( i <= sLen )
	{
		/* create lrCharacters */
		trotListRefFree( &lrCharacters );
		TEST_ERR_IF( trotListRefInit( &lrCharacters ) != TROT_LIST_SUCCESS );

		x = s;
		while ( (*x) != '\0' )
		{
			TEST_ERR_IF( trotListRefAppendInt( lrCharacters, (*x) ) != TROT_LIST_SUCCESS );

			x += 1;
		}

		/* add a list, which should never occur in a list to tokenize */
		trotListRefFree( &lrTemp );
		TEST_ERR_IF( trotListRefInit( &lrTemp ) != TROT_LIST_SUCCESS );

		TEST_ERR_IF( trotListRefInsertListTwin( lrCharacters, i + 1, lrTemp ) != TROT_LIST_SUCCESS );

		/* try to tokenize */
		TEST_ERR_IF( trotTokenize( lrCharacters, &lrTokens ) != TROT_LIST_ERROR_WRONG_KIND );

		/* increment */
		i += 1;
	}


	/* CLEANUP */
	cleanup:

	trotListRefFree( &lrCharacters );
	trotListRefFree( &lrTemp );

	return rc;
}

/******************************************************************************/
/*!
	\brief Prints out a tokenList for debugging.
	\param lrTokenList Token list.
	\return int
*/
/* TODO: we have another type function in trotDecodingEncoding.c. Merge into 1? */
static int printTokens( trotListRef *lrTokenList )
{
	/* DATA */
	int rc = 0;

	INT_TYPE count = 0;
	INT_TYPE index = 0;

	INT_TYPE currentLine = 1;

	trotListRef *lrToken = NULL;

	INT_TYPE tokenLine = 0;
	INT_TYPE tokenType = 0;
	trotListRef *lrValue = NULL;
	INT_TYPE tokenNumber = 0;

	trotListRef *lrUtf8Bytes = NULL;
	INT_TYPE utf8Count = 0;
	INT_TYPE utf8Index = 0;
	INT_TYPE utf8Byte = 0;


	/* CODE */
	TEST_ERR_IF( lrTokenList == NULL );

	if ( ! PRINT_TOKENS )
	{
		goto cleanup;
	}

	/* get count */
	TEST_ERR_IF( trotListRefGetCount( lrTokenList, &count ) != TROT_LIST_SUCCESS );

	/* foreach token */
	index = 1;
	while ( index <= count )
	{
		/* get token */
		trotListRefFree( &lrToken );

		TEST_ERR_IF( trotListRefGetListTwin( lrTokenList, index, &lrToken ) != TROT_LIST_SUCCESS );

		/* get line */
		TEST_ERR_IF( trotListRefGetInt( lrToken, TOKEN_INDEX_LINE, &tokenLine ) != TROT_LIST_SUCCESS );

		/* get type */
		TEST_ERR_IF( trotListRefGetInt( lrToken, TOKEN_INDEX_TYPE, &tokenType ) != TROT_LIST_SUCCESS );

		/* new line? */
		if ( currentLine != tokenLine )
		{
			printf( "\n" );
			currentLine = tokenLine;
		}

		/* print */
		if ( tokenType == TOKEN_L_BRACKET )
		{
			printf( "[ " );
		}
		else if ( tokenType == TOKEN_R_BRACKET )
		{
			printf( "] " );
		}
		else if ( tokenType == TOKEN_L_PARENTHESIS )
		{
			printf( "( " );
		}
		else if ( tokenType == TOKEN_R_PARENTHESIS )
		{
			printf( ") " );
		}
		else if ( tokenType == TOKEN_L_BRACE )
		{
			printf( "{ " );
		}
		else if ( tokenType == TOKEN_R_BRACE )
		{
			printf( "} " );
		}
		else if ( tokenType == TOKEN_STRING || tokenType == TOKEN_WORD )
		{
			/* get value */
			trotListRefFree( &lrValue );

			TEST_ERR_IF( trotListRefGetListTwin( lrToken, TOKEN_INDEX_VALUE, &lrValue ) != TROT_LIST_SUCCESS );

			/* convert to utf8 */
			trotListRefFree( &lrUtf8Bytes );

			TEST_ERR_IF( trotListRefInit( &lrUtf8Bytes ) != TROT_LIST_SUCCESS );

			TEST_ERR_IF( trotCharactersToUtf8( lrValue, lrUtf8Bytes ) != TROT_LIST_SUCCESS );

			if ( tokenType == TOKEN_STRING )
			{
				printf( "\"" );
			}
			else
			{
				printf( "W:" );
			}

			/* print */
			TEST_ERR_IF( trotListRefGetCount( lrUtf8Bytes, &utf8Count ) != TROT_LIST_SUCCESS );

			utf8Index = 1;
			while ( utf8Index <= utf8Count )
			{
				TEST_ERR_IF( trotListRefGetInt( lrUtf8Bytes, utf8Index, &utf8Byte ) != TROT_LIST_SUCCESS );

				printf( "%c", utf8Byte );

				/* next */
				utf8Index += 1;
			}
			

			if ( tokenType == TOKEN_STRING )
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
			TEST_ERR_IF( trotListRefGetInt( lrToken, TOKEN_INDEX_VALUE, &tokenNumber ) != TROT_LIST_SUCCESS );

			printf( "N:%d ", tokenNumber );
		}

		/* next */
		index += 1;
	}

	printf( "\n\n" );


	/* CLEANUP */
	cleanup:

	trotListRefFree( &lrToken );
	trotListRefFree( &lrValue );
	trotListRefFree( &lrUtf8Bytes );

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

	trotListRef *lr1 = NULL;
	trotListRef *lr2 = NULL;
	trotListRef *lrCharacters = NULL;


	/* CODE */
	/* *** */
	TEST_ERR_IF( trotListRefInit( &lr1 ) != TROT_LIST_SUCCESS );
	TEST_ERR_IF( trotListRefInit( &lr2 ) != TROT_LIST_SUCCESS );
	TEST_ERR_IF( trotListRefAppendListTwin( lr1, lr2 ) != TROT_LIST_SUCCESS );
	TEST_ERR_IF( trotListRefSetTag( lr1, TROT_TAG_CODE ) != TROT_LIST_SUCCESS );

	TEST_ERR_IF( trotEncode( lr1, &lrCharacters ) != TROT_LIST_ERROR_ENCODE );

	/* *** */
	trotListRefFree( &lr1 );
	trotListRefFree( &lr2 );
	trotListRefFree( &lrCharacters );

	TEST_ERR_IF( trotListRefInit( &lr1 ) != TROT_LIST_SUCCESS );
	TEST_ERR_IF( trotListRefAppendInt( lr1, TROT_OP_PUSH_INT ) != TROT_LIST_SUCCESS );
	TEST_ERR_IF( trotListRefSetTag( lr1, TROT_TAG_CODE ) != TROT_LIST_SUCCESS );

	TEST_ERR_IF( trotEncode( lr1, &lrCharacters ) != TROT_LIST_ERROR_ENCODE );

	/* *** */
	trotListRefFree( &lr1 );
	trotListRefFree( &lr2 );
	trotListRefFree( &lrCharacters );

	TEST_ERR_IF( trotListRefInit( &lr1 ) != TROT_LIST_SUCCESS );
	TEST_ERR_IF( trotListRefInit( &lr2 ) != TROT_LIST_SUCCESS );
	TEST_ERR_IF( trotListRefAppendInt( lr1, TROT_OP_PUSH_INT ) != TROT_LIST_SUCCESS );
	TEST_ERR_IF( trotListRefAppendListTwin( lr1, lr2 ) != TROT_LIST_SUCCESS );
	TEST_ERR_IF( trotListRefSetTag( lr1, TROT_TAG_CODE ) != TROT_LIST_SUCCESS );

	TEST_ERR_IF( trotEncode( lr1, &lrCharacters ) != TROT_LIST_ERROR_ENCODE );

	/* *** */
	trotListRefFree( &lr1 );
	trotListRefFree( &lr2 );
	trotListRefFree( &lrCharacters );

	TEST_ERR_IF( trotListRefInit( &lr1 ) != TROT_LIST_SUCCESS );
	TEST_ERR_IF( trotListRefAppendInt( lr1, TROT_OP_PUSH_LIST ) != TROT_LIST_SUCCESS );
	TEST_ERR_IF( trotListRefSetTag( lr1, TROT_TAG_CODE ) != TROT_LIST_SUCCESS );

	TEST_ERR_IF( trotEncode( lr1, &lrCharacters ) != TROT_LIST_ERROR_ENCODE );

	/* *** */
	trotListRefFree( &lr1 );
	trotListRefFree( &lr2 );
	trotListRefFree( &lrCharacters );

	TEST_ERR_IF( trotListRefInit( &lr1 ) != TROT_LIST_SUCCESS );
	TEST_ERR_IF( trotListRefAppendInt( lr1, TROT_OP_PUSH_LIST ) != TROT_LIST_SUCCESS );
	TEST_ERR_IF( trotListRefAppendInt( lr1, 5 ) != TROT_LIST_SUCCESS );
	TEST_ERR_IF( trotListRefSetTag( lr1, TROT_TAG_CODE ) != TROT_LIST_SUCCESS );

	TEST_ERR_IF( trotEncode( lr1, &lrCharacters ) != TROT_LIST_ERROR_ENCODE );

	/* *** */
	trotListRefFree( &lr1 );
	trotListRefFree( &lr2 );
	trotListRefFree( &lrCharacters );

	TEST_ERR_IF( trotListRefInit( &lr1 ) != TROT_LIST_SUCCESS );
	TEST_ERR_IF( trotListRefAppendInt( lr1, -1 ) != TROT_LIST_SUCCESS );
	TEST_ERR_IF( trotListRefSetTag( lr1, TROT_TAG_CODE ) != TROT_LIST_SUCCESS );

	TEST_ERR_IF( trotEncode( lr1, &lrCharacters ) != TROT_LIST_ERROR_ENCODE );

	/* *** */
	trotListRefFree( &lr1 );
	trotListRefFree( &lr2 );
	trotListRefFree( &lrCharacters );

	TEST_ERR_IF( trotListRefInit( &lr1 ) != TROT_LIST_SUCCESS );
	TEST_ERR_IF( trotListRefAppendInt( lr1, TROT_OP_MAX + 1 ) != TROT_LIST_SUCCESS );
	TEST_ERR_IF( trotListRefSetTag( lr1, TROT_TAG_CODE ) != TROT_LIST_SUCCESS );

	TEST_ERR_IF( trotEncode( lr1, &lrCharacters ) != TROT_LIST_ERROR_ENCODE );


	/* CLEANUP */
	cleanup:

	trotListRefFree( &lr1 );
	trotListRefFree( &lr2 );
	trotListRefFree( &lrCharacters );

	return rc;
}

