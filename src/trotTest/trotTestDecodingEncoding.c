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

/******************************************************************************/
#define PRINT_TOKENS 0

/******************************************************************************/
static TROT_RC load( trotListRef *lrName, trotListRef **lrBytes );

static TROT_RC listToCString( trotListRef *lr, char **cString_A );
static TROT_RC appendCStringToList( char *cString, trotListRef *lr );
static TROT_RC doesFileExist( trotListRef *lrName, INT_TYPE *exist );

typedef TROT_RC (*ProcessFunction)( int dirNumber, int fileNumber, trotListRef *lrName );
static TROT_RC processFiles( char *directory, ProcessFunction func );

static TROT_RC testTokenizingGood( int dirNumber, int fileNumber, trotListRef *lrName );
static TROT_RC testTokenizingBad( int dirNumber, int fileNumber, trotListRef *lrName );
static TROT_RC testEndOfLines( int dirNumber, int fileNumber, trotListRef *lrName );

static TROT_RC testDecodingEncodingGood( int dirNumber, int fileNumber, trotListRef *lrName );
static TROT_RC testDecodingEncodingBad( int dirNumber, int fileNumber, trotListRef *lrName );

static TROT_RC trotPrintTokens( trotListRef *lrTokenList );

/******************************************************************************/
int testDecodingEncoding()
{
	/* DATA */
	int rc = TROT_LIST_SUCCESS;


	/* CODE */
	printf( "Testing tokenizing...\n" ); fflush( stdout );

	rc = processFiles( "./trotTest/testData/TokenFiles/good/", testTokenizingGood );
	TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

	rc = processFiles( "./trotTest/testData/TokenFiles/bad/", testTokenizingBad );
	TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

	rc = processFiles( "./trotTest/testData/EndOfLineFiles/", testEndOfLines );
	TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

	printf( "Testing decoding and encoding...\n" ); fflush( stdout );

	rc = processFiles( "./trotTest/testData/DecodeFiles/good/", testDecodingEncodingGood );
	TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

	rc = processFiles( "./trotTest/testData/DecodeFiles/bad/", testDecodingEncodingBad );
	TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

	/* TODO: more */	


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
#define LOAD_BUFFER_SIZE 1024
static TROT_RC load( trotListRef *lrName, trotListRef **lrBytes )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	size_t bytesRead = 0;
	size_t i = 0;

	char *name = NULL;

	FILE *fp = NULL;

	trotListRef *newLrBytes = NULL;

	BYTE_TYPE buffer[ LOAD_BUFFER_SIZE ];


	/* CODE */
	/* create our new byte list */
	rc = trotListRefInit( &newLrBytes );
	ERR_IF_PASSTHROUGH;

	/* convert our trotListRef name to a cString */
	rc = listToCString( lrName, &name );
	ERR_IF_PASSTHROUGH;

	/* open */
	fp = fopen( name, "rb" );
	ERR_IF( fp == NULL, TROT_LIST_ERROR_LOAD );

	/* read all bytes */
	while ( 1 )
	{
		/* read as much as we can into our buffer */
		bytesRead = fread( buffer, 1, LOAD_BUFFER_SIZE, fp );

		/* append what we read to our bytes */
		i = 0;
		while ( i < bytesRead )
		{
			rc = trotListRefAppendInt( newLrBytes, buffer[ i ] );
			ERR_IF_PASSTHROUGH;

			i += 1;
		}

		/* if short read, check error, or break */
		if ( bytesRead < LOAD_BUFFER_SIZE )
		{
			ERR_IF( ! feof( fp ), TROT_LIST_ERROR_LOAD );
			break;
		}
	}

	/* give back */
	(*lrBytes) = newLrBytes;
	newLrBytes = NULL;


	/* CLEANUP */
	cleanup:

	if ( fp != NULL )
	{
		fclose( fp );
	}

	if ( name != NULL )
	{
		trotFree( name );
	}

	trotListRefFree( &newLrBytes );

	return rc;
}
#undef LOAD_BUFFER_SIZE

/******************************************************************************/
static TROT_RC listToCString( trotListRef *lr, char **cString_A )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	char *newCString = NULL;

	INT_TYPE count = 0;
	INT_TYPE i = 1;

	INT_TYPE c = 0;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );
	PRECOND_ERR_IF( cString_A == NULL );
	PRECOND_ERR_IF( (*cString_A) != NULL );


	/* CODE */
	rc = trotListRefGetCount( lr, &count );
	ERR_IF_PASSTHROUGH;

	newCString = trotMalloc( count + 1 );
	ERR_IF( newCString == NULL, TROT_LIST_ERROR_MEMORY_ALLOCATION_FAILED );

	i = 1;
	while ( i <= count )
	{
		rc = trotListRefGetInt( lr, i, &c );
		ERR_IF_PASSTHROUGH;

		ERR_IF( c < 0, TROT_LIST_ERROR_NOT_BYTE_VALUE );
		ERR_IF( c > 255, TROT_LIST_ERROR_NOT_BYTE_VALUE );

		newCString[ i - 1 ] = (char)c;

		i += 1;
	}

	newCString[ i - 1 ] = '\0';

	/* give back */
	(*cString_A) = newCString;
	newCString = NULL;


	/* CLEANUP */
	cleanup:

	if ( newCString != NULL )
	{
		trotFree( newCString );
	}

	return rc;
}

/******************************************************************************/
static TROT_RC appendCStringToList( char *cString, trotListRef *lr )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;


	/* PRECOND */
	PRECOND_ERR_IF( cString == NULL );
	PRECOND_ERR_IF( lr == NULL );


	/* CODE */
	while ( *cString != '\0' )
	{
		rc = trotListRefAppendInt( lr, (*cString) );
		ERR_IF_PASSTHROUGH;

		cString += 1;
	}


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
static TROT_RC doesFileExist( trotListRef *lrName, INT_TYPE *exist )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	char *name = NULL;

	struct stat st;


	/* PRECOND */
	PRECOND_ERR_IF( lrName == NULL );
	PRECOND_ERR_IF( exist == NULL );


	/* CODE */
	rc = listToCString( lrName, &name );
	ERR_IF_PASSTHROUGH;

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
		ERR_IF( 1, TROT_LIST_ERROR_STANDARD_LIBRARY_ERROR );
	}


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Goes through directory sending files to func.
	\param directory Directory to process.
	\param func Processing function to pass each file to.
	\return TROT_RC
*/
static TROT_RC processFiles( char *directory, ProcessFunction func )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	int dirNumber = 1;
	int fileNumber = 1;

	INT_TYPE exist = 0;

	trotListRef *lrName = NULL;

	int flagTestedAtLeastOne = 0;


	/* PRECOND */
	PRECOND_ERR_IF( directory == NULL );
	PRECOND_ERR_IF( func == NULL );


	/* CODE */
	/* create trotList filename */
	rc = trotListRefInit( &lrName );
	TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

	rc = appendCStringToList( directory, lrName );
	TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

	/* foreach directory */
	while ( 1 )
	{
		rc = trotListRefAppendInt( lrName, ( dirNumber / 10 ) + '0' );
		TEST_ERR_IF( rc != TROT_LIST_SUCCESS );
		rc = trotListRefAppendInt( lrName, ( dirNumber % 10 ) + '0' );
		TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

		/* does directory exist? */
		rc = doesFileExist( lrName, &exist );
		TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

		if ( exist == 0 )
		{
			break;
		}

		rc = trotListRefAppendInt( lrName, '/' );
		TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

		/* foreach file in this directory */
		fileNumber = 1;

		while ( 1 )
		{
			rc = trotListRefAppendInt( lrName, ( fileNumber / 10 ) + '0' );
			TEST_ERR_IF( rc != TROT_LIST_SUCCESS );
			rc = trotListRefAppendInt( lrName, ( fileNumber % 10 ) + '0' );
			TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

			/* does file exist? */
/* TODO: find better name for 'doesFileExist' and 'appendCStringToList' etc */
			rc = doesFileExist( lrName, &exist );
			TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

			if ( exist == 0 )
			{
				/* remove our number from end of name */
				rc = trotListRefRemove( lrName, -1 );
				TEST_ERR_IF( rc != TROT_LIST_SUCCESS );
				rc = trotListRefRemove( lrName, -1 );
				TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

				break;
			}

			/* send to func */
			rc = func( dirNumber, fileNumber, lrName );
			TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

			/* set flag */
			flagTestedAtLeastOne = 1;

			/* remove our number from end of name */
			rc = trotListRefRemove( lrName, -1 );
			TEST_ERR_IF( rc != TROT_LIST_SUCCESS );
			rc = trotListRefRemove( lrName, -1 );
			TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

			/* next */
			fileNumber += 1;
		}

		/* remove the last '/' from end of name */
		rc = trotListRefRemove( lrName, -1 );
		TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

		/* remove our number from end of name */
		rc = trotListRefRemove( lrName, -1 );
		TEST_ERR_IF( rc != TROT_LIST_SUCCESS );
		rc = trotListRefRemove( lrName, -1 );
		TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

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
static TROT_RC testTokenizingGood( int dirNumber, int fileNumber, trotListRef *lrName )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotListRef *lrBytes = NULL;
	trotListRef *lrCharacters = NULL;
	trotListRef *lrTokenList = NULL;

	INT_TYPE count = 0;
	INT_TYPE index = 0;
	INT_TYPE tokenType = 0;
	trotListRef *lrToken = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( lrName == NULL );


	/* CODE */
	(void)fileNumber;

	/* call load */
	rc = load( lrName, &lrBytes );
	TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

	/* convert unicode */
	trotListRefFree( &lrCharacters );

	rc = trotListRefInit( &lrCharacters );
	TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

	rc = trotUtf8ToCharacters( lrBytes, lrCharacters );
	TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

	/* tokenize */
	trotListRefFree( &lrTokenList );

	rc = trotTokenize( lrCharacters, &lrTokenList );
	TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

	rc = trotPrintTokens( lrTokenList );
	TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

	/* make sure each token is the appropriate type */
	rc = trotListRefGetCount( lrTokenList, &count );
	TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

	index = 1;
	while ( index <= count )
	{
		trotListRefFree( &lrToken );

		rc = trotListRefGetListTwin( lrTokenList, index, &lrToken );
		TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

		rc = trotListRefGetInt( lrToken, TOKEN_INDEX_TYPE, &tokenType );
		TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

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
static TROT_RC testTokenizingBad( int dirNumber, int fileNumber, trotListRef *lrName )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotListRef *lrBytes = NULL;
	trotListRef *lrCharacters = NULL;
	trotListRef *lrTokenList = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( lrName == NULL );


	/* CODE */
	(void)dirNumber;
	(void)fileNumber;

	/* call load */
	rc = load( lrName, &lrBytes );
	TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

	/* convert unicode */
	rc = trotListRefInit( &lrCharacters );
	TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

	rc = trotUtf8ToCharacters( lrBytes, lrCharacters );
	TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

	/* tokenize, which should FAIL */
	rc = trotTokenize( lrCharacters, &lrTokenList );
	TEST_ERR_IF( rc != TROT_LIST_ERROR_DECODE );

	rc = TROT_LIST_SUCCESS;


	/* CLEANUP */
	cleanup:

	trotListRefFree( &lrBytes );
	trotListRefFree( &lrCharacters );
	trotListRefFree( &lrTokenList );

	return rc;
}


/******************************************************************************/
static TROT_RC testEndOfLines( int dirNumber, int fileNumber, trotListRef *lrName )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotListRef *lrBytes = NULL;
	trotListRef *lrCharacters = NULL;
	trotListRef *lrTokenList = NULL;

	INT_TYPE count = 0;
	INT_TYPE index = 0;
	INT_TYPE tokenType = 0;
	trotListRef *lrToken = NULL;

	INT_TYPE line = 0;
	INT_TYPE numberValue = 0;


	/* PRECOND */
	PRECOND_ERR_IF( lrName == NULL );


	/* CODE */
	(void)dirNumber;
	(void)fileNumber;

	/* call load */
	rc = load( lrName, &lrBytes );
	TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

	/* convert unicode */
	trotListRefFree( &lrCharacters );

	rc = trotListRefInit( &lrCharacters );
	TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

	rc = trotUtf8ToCharacters( lrBytes, lrCharacters );
	TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

	/* tokenize */
	trotListRefFree( &lrTokenList );

	rc = trotTokenize( lrCharacters, &lrTokenList );
	TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

	/* make sure each number token is on the right line */
	rc = trotListRefGetCount( lrTokenList, &count );
	TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

	index = 1;
	while ( index <= count )
	{
		trotListRefFree( &lrToken );

		rc = trotListRefGetListTwin( lrTokenList, index, &lrToken );
		TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

		rc = trotListRefGetInt( lrToken, TOKEN_INDEX_TYPE, &tokenType );
		TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

		if ( tokenType == TOKEN_NUMBER )
		{
			rc = trotListRefGetInt( lrToken, TOKEN_INDEX_LINE, &line );
			TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

			rc = trotListRefGetInt( lrToken, TOKEN_INDEX_VALUE, &numberValue );
			TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

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
static TROT_RC testDecodingEncodingGood( int dirNumber, int fileNumber, trotListRef *lrName )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotListRef *lrDecodedList1 = NULL;
	trotListRef *lrEncodedList1 = NULL;
	trotListRef *lrEmptyName = NULL;
	trotListRef *lrDecodedList2 = NULL;
	trotListRef *lrEncodedList2 = NULL;

	TROT_LIST_COMPARE_RESULT compareResult;
#if 0
	char *s = NULL;
#endif	


	/* PRECOND */
	PRECOND_ERR_IF( lrName == NULL );


	/* CODE */
	(void)dirNumber;
	(void)fileNumber;

	rc = trotDecodeFilename( load, lrName, &lrDecodedList1 );
	TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

	rc = trotEncode( lrDecodedList1, &lrEncodedList1 );
	TEST_ERR_IF( rc != TROT_LIST_SUCCESS );
#if 0
	rc = listToCString( lrEncodedList1, &s );
	TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

	printf( "lrEncodedList1: %s\n", s );

	trotFree( s );
	s = NULL;
#endif

	rc = trotListRefInit( &lrEmptyName );
	TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

	rc = trotDecodeCharacters( load, lrEmptyName, lrEncodedList1, &lrDecodedList2 );
	TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

	rc = trotEncode( lrDecodedList2, &lrEncodedList2 );
	TEST_ERR_IF( rc != TROT_LIST_SUCCESS );
#if 0
	rc = listToCString( lrEncodedList2, &s );
	TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

	printf( "lrEncodedList2: %s\n", s );

	trotFree( s );
	s = NULL;
#endif

	rc = trotListRefCompare( lrDecodedList1, lrDecodedList2, &compareResult );
	TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

	TEST_ERR_IF( compareResult != TROT_LIST_COMPARE_EQUAL );
	
	rc = trotListRefCompare( lrEncodedList1, lrEncodedList2, &compareResult );
	TEST_ERR_IF( rc != TROT_LIST_SUCCESS );

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
static TROT_RC testDecodingEncodingBad( int dirNumber, int fileNumber, trotListRef *lrName )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotListRef *lrDecodedList = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( lrName == NULL );


	/* CODE */
	(void)dirNumber;
	(void)fileNumber;

	rc = trotDecodeFilename( load, lrName, &lrDecodedList );
	TEST_ERR_IF( rc != TROT_LIST_ERROR_DECODE );

	rc = TROT_LIST_SUCCESS;


	/* CLEANUP */
	cleanup:

	trotListRefFree( &lrDecodedList );

	return rc;
}

/******************************************************************************/
/*!
	\brief Prints out a tokenList for debugging.
	\param lrTokenList Token list.
	\return TROT_RC
*/
/* TODO: we have another type function in trotDecodingEncoding.c. Merge into 1? */
static TROT_RC trotPrintTokens( trotListRef *lrTokenList )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

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
	ERR_IF_PARANOID( lrTokenList == NULL );

	if ( ! PRINT_TOKENS )
	{
		goto cleanup;
	}

	/* get count */
	rc = trotListRefGetCount( lrTokenList, &count );
	ERR_IF_PASSTHROUGH;

	/* foreach token */
	index = 1;
	while ( index <= count )
	{
		/* get token */
		trotListRefFree( &lrToken );

		rc = trotListRefGetListTwin( lrTokenList, index, &lrToken );
		ERR_IF_PASSTHROUGH;

		/* get line */
		rc = trotListRefGetInt( lrToken, TOKEN_INDEX_LINE, &tokenLine );
		ERR_IF_PASSTHROUGH;

		/* get type */
		rc = trotListRefGetInt( lrToken, TOKEN_INDEX_TYPE, &tokenType );
		ERR_IF_PASSTHROUGH;

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

			rc = trotListRefGetListTwin( lrToken, TOKEN_INDEX_VALUE, &lrValue );
			ERR_IF_PASSTHROUGH;

			/* convert to utf8 */
			trotListRefFree( &lrUtf8Bytes );

			rc = trotListRefInit( &lrUtf8Bytes );
			ERR_IF_PASSTHROUGH;

			rc = trotCharactersToUtf8( lrValue, lrUtf8Bytes );
			ERR_IF_PASSTHROUGH;

			if ( tokenType == TOKEN_STRING )
			{
				printf( "\"" );
			}
			else
			{
				printf( "W:" );
			}

			/* print */
			rc = trotListRefGetCount( lrUtf8Bytes, &utf8Count );
			ERR_IF_PASSTHROUGH;

			utf8Index = 1;
			while ( utf8Index <= utf8Count )
			{
				rc = trotListRefGetInt( lrUtf8Bytes, utf8Index, &utf8Byte );
				ERR_IF_PASSTHROUGH;

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
			rc = trotListRefGetInt( lrToken, TOKEN_INDEX_VALUE, &tokenNumber );
			ERR_IF_PASSTHROUGH;

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

