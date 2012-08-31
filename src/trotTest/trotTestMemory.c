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
#define MEMORY_MANAGEMENT_REFS_COUNT 10

#define PRINT_ENCODED_LISTS 0

/******************************************************************************/
/* which malloc to fail on */
static int failOnMallocCount = 0;
static int currentMallocCount = 0;

/* malloc/calloc that will always fail */
void *badCalloc( size_t nmemb, size_t size )
{
	currentMallocCount += 1;
	if ( currentMallocCount == failOnMallocCount )
	{
		return NULL;
	}

	return calloc( nmemb, size );
}

void *badMalloc( size_t size )
{
	currentMallocCount += 1;
	if ( currentMallocCount == failOnMallocCount )
	{
		return NULL;
	}

	return malloc( size );
}

TROT_RC badLoad( trotListRef *lrName, trotListRef **lrBytes )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	trotListRef *newLrBytes = NULL;


	/* CODE */
	(void)lrName;

	/* create our new byte list */
	rc = trotListRefInit( &newLrBytes );
	ERR_IF_PASSTHROUGH;

	rc = trotListRefAppendInt( newLrBytes, '[' );
	ERR_IF_PASSTHROUGH;
	rc = trotListRefAppendInt( newLrBytes, ']' );
	ERR_IF_PASSTHROUGH;

	/* give back */
	(*lrBytes) = newLrBytes;
	newLrBytes = NULL;


	/* CLEANUP */
	cleanup:

	trotListRefFree( &newLrBytes );

	return rc;
}

/******************************************************************************/
static int testMemoryManagement();
static int testDeepList();

static TROT_RC testFailedMallocs1( int test );
static TROT_RC testFailedMallocs2( int test );
static TROT_RC testFailedMallocs3( int test );
static TROT_RC testFailedMallocs4( int test );

typedef struct
{
	TROT_RC (*func)( int test );
	int numberOfTests;
} FailedFunc;

FailedFunc failedFuncs[] =
{
	{ testFailedMallocs1, 1 },
	{ testFailedMallocs2, 1 },
	{ testFailedMallocs3, 18 },
	{ testFailedMallocs4, 1 },
	{ NULL, 0 }
};

/******************************************************************************/
int testMemory()
{
	/* DATA */
	int rc = 0;

	int **iArray = NULL;

	int i = 0;
	int j = 0;

	char *spinner = "-\\|/";
	unsigned int spinnerI = 0;


	/* CODE */
	/* **************************************** */
	printf( "Testing memory management and garbage collection...\n" ); fflush( stdout );

	/* **************************************** */
	/* testing bad mallocs */
	printf( "  Testing bad mallocs...\n" ); fflush( stdout );

	trotCalloc = badCalloc;
	trotMalloc = badMalloc;

	i = 0;
	while ( failedFuncs[ i ].func != NULL )
	{
		j = 0;
		while ( j < failedFuncs[ i ].numberOfTests )
		{
			failOnMallocCount = 0;
			spinnerI = 0;

			do
			{
				currentMallocCount = 0;
				failOnMallocCount += 1;

				rc = failedFuncs[ i ].func( j );

				spinnerI += 1;
				printf( "\r%d %c", i, spinner[ spinnerI % 4 ]  ); fflush( stdout );
			}
			while ( rc == TROT_RC_ERROR_MEMORY_ALLOCATION_FAILED );

			TEST_ERR_IF( rc != TROT_RC_SUCCESS );

			j += 1;
		}

		printf( "\r%d  \n", i ); fflush( stdout );

		i += 1;
	}

	/* *** */
	trotCalloc = calloc;
	trotMalloc = malloc;

	/* **************************************** */
	/* test that calloc sets pointers to NULL */
	printf( "  Testing calloc...\n" ); fflush( stdout );
	iArray = (int **) trotCalloc( 10, sizeof( int * ) );
	TEST_ERR_IF( iArray == NULL );
	TEST_ERR_IF( iArray[ 5 ] != NULL );
	trotFree( iArray );

	/* **************************************** */
	/* test memory management */
	printf( "  Testing garbage collection...\n" ); fflush( stdout );
	i = 0;
	while ( i < 100 ) /* MAGIC */
	{
		if ( i % 5 == 0 )
		{
			printf( "." ); fflush( stdout );
		}

		TEST_ERR_IF( testMemoryManagement() != 0 );

		/* *** */
		i += 1;
	}

	printf( "\n" ); fflush( stdout );

	/* **************************************** */
	/* *** */
	printf( "  Testing a \"deep list\"...\n" ); fflush( stdout );
	TEST_ERR_IF( testDeepList() != 0 );

	printf( "\n" ); fflush( stdout );

	/* **************************************** */
	return 0;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
static int testMemoryManagement()
{
	/* DATA */
	int rc = 0;

	int i = 0;
	int j = 0;

	trotListRef **clientRefs = NULL;

	int r = 0;

	trotListRef *ref = NULL;

	int howManyToAdd = 0;
	TROT_INT count = 0;
	TROT_INT randomIndex = 0;

	int clientRefIndex = 0;

	int countAdded = 0;
	int howManyToRemove = 0;


	/* CODE */
	/* create our client refs */
	clientRefs = (trotListRef **) trotCalloc( MEMORY_MANAGEMENT_REFS_COUNT, sizeof( trotListRef * ) );
	TEST_ERR_IF( clientRefs == NULL );

	i = 0;
	while ( i < MEMORY_MANAGEMENT_REFS_COUNT )
	{
		r = rand() % 2;

		/* twin already existing list */
		if ( r == 0 && i > 0 )
		{
			j = rand() % i;
			TEST_ERR_IF( trotListRefTwin( clientRefs[ j ], &( clientRefs[ i ] ) ) != TROT_RC_SUCCESS );
		}
		/* create new list */
		else
		{
			TEST_ERR_IF( trotListRefInit( &( clientRefs[ i ] ) ) != TROT_RC_SUCCESS );
		}

		i += 1;
	}

	/* create/twin new lists and insert them into client refs */
	i = 0;
	while ( i < 1000 ) /* MAGIC */
	{
		r = rand() % 10; /* MAGIC */
		/* create new list */
		if ( r <= 7 )
		{
			TEST_ERR_IF( trotListRefInit( &ref ) != TROT_RC_SUCCESS );
		}
		/* twin client list */
		else
		{
			r = rand() % MEMORY_MANAGEMENT_REFS_COUNT;
			TEST_ERR_IF( trotListRefTwin( clientRefs[ r ], &ref ) != TROT_RC_SUCCESS );
		}

		/* how many are we going to add? */
		howManyToAdd = rand() % 10; /* MAGIC */

		j = 0;
		while ( j < howManyToAdd )
		{
			/* which client ref to add to? */
			clientRefIndex = rand() % MEMORY_MANAGEMENT_REFS_COUNT;

			/* where to add it to? */
			TEST_ERR_IF( trotListRefGetCount( clientRefs[ clientRefIndex ], &count ) != TROT_RC_SUCCESS );

			randomIndex = ( rand() % ( count + 1 ) ) + 1;

			/* add */
			TEST_ERR_IF( trotListRefInsertListTwin( clientRefs[ clientRefIndex ], randomIndex, ref ) != TROT_RC_SUCCESS );

			countAdded += 1;

			j += 1;
		}

		/* free our temporary ref */
		trotListRefFree( &ref );

		/* *** */
		i += 1;
	}

	/* remove random lists */
	howManyToRemove = rand() % countAdded;

	i = 0;
	while ( i < howManyToRemove )
	{
		/* which client ref to remove from? */
		clientRefIndex = rand() % MEMORY_MANAGEMENT_REFS_COUNT;

		/* which index to remove? */
		TEST_ERR_IF( trotListRefGetCount( clientRefs[ clientRefIndex ], &count ) != TROT_RC_SUCCESS );

		if ( count > 0 )
		{
			randomIndex = ( rand() % count ) + 1;

			/* "remove list", or "remove" */
			r = rand() % 2;
			if ( r == 0 )
			{
				TEST_ERR_IF( trotListRefRemoveList( clientRefs[ clientRefIndex ], randomIndex, &ref ) != TROT_RC_SUCCESS );
				trotListRefFree( &ref );
			}
			else
			{
				TEST_ERR_IF( trotListRefRemove( clientRefs[ clientRefIndex ], randomIndex ) != TROT_RC_SUCCESS );
			}
		}

		/* *** */
		i += 1;
	}

	/* free our client refs */
	i = 0;
	while ( i < MEMORY_MANAGEMENT_REFS_COUNT )
	{
		/* which client ref to free */
		r = rand() % MEMORY_MANAGEMENT_REFS_COUNT;

		/* free that client ref, or the next one that isn't NULL */
		j = r;
		while ( 1 )
		{
			if ( clientRefs[ j ] != NULL )
			{
				trotListRefFree( &( clientRefs[ j ] ) );
				break;
			}

			/* *** */
			j += 1;
			j %= MEMORY_MANAGEMENT_REFS_COUNT;
		}

		/* *** */
		i += 1;
	}

	/* free our clientRef array */
	trotFree( clientRefs );


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
static int testDeepList()
{
	/* DATA */
	int rc = 0;

	int i = 0;

	trotListRef *refHead = NULL;

	trotListRef *ref1 = NULL;
	trotListRef *ref2 = NULL;


	/* CODE */
	TEST_ERR_IF( trotListRefInit( &refHead ) != TROT_RC_SUCCESS );

	TEST_ERR_IF( trotListRefTwin( refHead, &ref1 ) != TROT_RC_SUCCESS );

	while ( i < 1000 ) /* MAGIC */
	{
		TEST_ERR_IF( trotListRefInit( &ref2 ) != TROT_RC_SUCCESS );

		TEST_ERR_IF( trotListRefAppendListTwin( ref1, ref2 ) != TROT_RC_SUCCESS );

		trotListRefFree( &ref1 );

		ref1 = ref2;
		ref2 = NULL;

		if ( i % 100 == 0 )
		{
			printf( "." ); fflush( stdout );
		}

		i += 1;
	}

	TEST_ERR_IF( trotListRefAppendListTwin( ref1, refHead ) != TROT_RC_SUCCESS );

	trotListRefFree( &ref1 );

	printf( ":" ); fflush( stdout );

	trotListRefFree( &refHead );

	return TROT_RC_SUCCESS;


	/* CLEANUP */
	cleanup:

	return rc;
}

static TROT_RC testFailedMallocs1( int test )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	int i = 0;
	int j = 0;

	trotListRef *lr1 = NULL;
	trotListRef *lr2 = NULL;
	trotListRef *lr3 = NULL;

	TROT_LIST_COMPARE_RESULT compareResult;


	/* CODE */
	(void)test;

	/* primary functions */
	rc = trotListRefInit( &lr1 );
	ERR_IF_PASSTHROUGH;

	rc = trotListRefInit( &lr2 );
	ERR_IF_PASSTHROUGH;

	i = 0;
	while ( i < (NODE_SIZE * 2) )
	{
		rc = trotListRefAppendInt( lr1,  1 );
		ERR_IF_PASSTHROUGH;

		i += 1;
	}

	i = 0;
	while ( i < (NODE_SIZE * 2) )
	{
		rc = trotListRefAppendListTwin( lr1, lr2 );
		ERR_IF_PASSTHROUGH;

		i += 1;
	}

	i = 0;
	while ( i < (NODE_SIZE * 2) )
	{
		rc = trotListRefAppendInt( lr1,  1 );
		ERR_IF_PASSTHROUGH;

		i += 1;
	}

	rc = trotListRefReplaceWithInt( lr1, (NODE_SIZE * 2) + 1, 0 );
	ERR_IF_PASSTHROUGH;
	rc = trotListRefReplaceWithInt( lr1, (NODE_SIZE * 4), 0 );
	ERR_IF_PASSTHROUGH;
	rc = trotListRefReplaceWithInt( lr1, (NODE_SIZE * 3) + 2, 0 );
	ERR_IF_PASSTHROUGH;


	trotListRefFree( &lr1 );
	trotListRefFree( &lr2 );


	rc = trotListRefInit( &lr1 );
	ERR_IF_PASSTHROUGH;

	rc = trotListRefInit( &lr2 );
	ERR_IF_PASSTHROUGH;

	i = 0;
	while ( i < (NODE_SIZE * 2) )
	{
		rc = trotListRefAppendListTwin( lr1, lr2 );
		ERR_IF_PASSTHROUGH;

		i += 1;
	}

	i = 0;
	while ( i < (NODE_SIZE * 2) )
	{
		rc = trotListRefAppendInt( lr1, 1 );
		ERR_IF_PASSTHROUGH;

		i += 1;
	}

	i = 0;
	while ( i < (NODE_SIZE * 2) )
	{
		rc = trotListRefAppendListTwin( lr1, lr2 );
		ERR_IF_PASSTHROUGH;

		i += 1;
	}

	rc = trotListRefReplaceWithList( lr1, (NODE_SIZE * 2) + 1, lr2 );
	ERR_IF_PASSTHROUGH;
	rc = trotListRefReplaceWithList( lr1, (NODE_SIZE * 4), lr2 );
	ERR_IF_PASSTHROUGH;
	rc = trotListRefReplaceWithList( lr1, (NODE_SIZE * 3) + 2, lr2 );
	ERR_IF_PASSTHROUGH;


	trotListRefFree( &lr1 );
	trotListRefFree( &lr2 );


	rc = trotListRefInit( &lr1 );
	ERR_IF_PASSTHROUGH;

	rc = trotListRefInit( &lr2 );
	ERR_IF_PASSTHROUGH;

	i = 0;
	while ( i < NODE_SIZE + 1 )
	{
		rc = trotListRefInsertInt( lr1, 1, 1 );
		ERR_IF_PASSTHROUGH;

		i += 1;
	}

	rc = trotListRefAppendInt( lr1, 1 );
	ERR_IF_PASSTHROUGH;

	rc = trotListRefInsertListTwin( lr1, -1, lr2 );
	ERR_IF_PASSTHROUGH;

	i = 0;
	while ( i < NODE_SIZE + 1 )
	{
		rc = trotListRefInsertListTwin( lr1, -2, lr2 );
		ERR_IF_PASSTHROUGH;

		i += 1;
	}

	rc = trotListRefAppendListTwin( lr1, lr2 );
	ERR_IF_PASSTHROUGH;

	rc = trotListRefInsertListTwin( lr1, 2, lr2 );
	ERR_IF_PASSTHROUGH;

	rc = trotListRefInsertInt( lr1, -2, 1 );
	ERR_IF_PASSTHROUGH;

	rc = trotListRefInsertListTwin( lr1, 1, lr2 );
	ERR_IF_PASSTHROUGH;
	rc = trotListRefInsertListTwin( lr1, 2, lr2 );
	ERR_IF_PASSTHROUGH;

	rc = trotListRefGetListTwin( lr1, -1, &lr3 );
	ERR_IF_PASSTHROUGH;
	trotListRefFree( &lr3 );

	rc = trotListRefAppendInt( lr1, 1 );
	ERR_IF_PASSTHROUGH;
	rc = trotListRefAppendInt( lr1, 1 );
	ERR_IF_PASSTHROUGH;

	/* secondary functions */
	rc = trotListRefCompare( lr1, lr2, &compareResult );
	ERR_IF_PASSTHROUGH;

	trotListRefFree( &lr2 );

	rc = trotListRefCopy( lr1, &lr2 );
	ERR_IF_PASSTHROUGH;

	trotListRefFree( &lr2 );

	rc = trotListRefEnlist( lr1, 2, -2 );
	ERR_IF_PASSTHROUGH;

	rc = trotListRefDelist( lr1, 2 );
	ERR_IF_PASSTHROUGH;

	rc = trotListRefCopySpan( lr1, 1, -2, &lr2 );
	ERR_IF_PASSTHROUGH;

	rc = trotListRefDelist( lr1, -10 );
	ERR_IF_PASSTHROUGH;

	trotListRefFree( &lr1 );
	trotListRefFree( &lr2 );

	/* *** */
	rc = trotListRefInit( &lr1 );
	ERR_IF_PASSTHROUGH;
	rc = trotListRefCopy( lr1, &lr2 );
	ERR_IF_PASSTHROUGH;

	trotListRefFree( &lr1 );
	trotListRefFree( &lr2 );

	/* *** */
	rc = trotListRefInit( &lr1 );
	ERR_IF_PASSTHROUGH;
	rc = trotListRefInit( &lr3 );
	ERR_IF_PASSTHROUGH;
	rc = trotListRefAppendInt( lr3, 1 );
	ERR_IF_PASSTHROUGH;
	rc = trotListRefAppendListTwin( lr1, lr3 );
	ERR_IF_PASSTHROUGH;
	trotListRefFree( &lr3 );

	rc = trotListRefInit( &lr2 );
	ERR_IF_PASSTHROUGH;
	rc = trotListRefInit( &lr3 );
	ERR_IF_PASSTHROUGH;
	rc = trotListRefAppendInt( lr3, 2 );
	ERR_IF_PASSTHROUGH;
	rc = trotListRefAppendListTwin( lr2, lr3 );
	ERR_IF_PASSTHROUGH;
	trotListRefFree( &lr3 );
	
	rc = trotListRefCompare( lr1, lr2, &compareResult );
	ERR_IF_PASSTHROUGH;

	trotListRefFree( &lr1 );
	trotListRefFree( &lr2 );


	/* It just so happens these values are 1 byte, 2 byte,
	   3 byte, and 4 byte utf8 characters */
	j = 0;
	while ( j < 4 )
	{
		/* *** */
		rc = trotListRefInit( &lr1 );
		ERR_IF_PASSTHROUGH;
		rc = trotListRefInit( &lr2 );
		ERR_IF_PASSTHROUGH;
		rc = trotListRefInit( &lr3 );
		ERR_IF_PASSTHROUGH;

		/* *** */
		i = 0;
		while ( i <= j )
		{
			rc = trotListRefAppendInt( lr1, 0x10 );
			ERR_IF_PASSTHROUGH;

			i += 1;
		}

		/* *** */
		i = 0;
		while ( i < NODE_SIZE + 1 )
		{
			rc = trotListRefAppendInt( lr1, 0x10 );
			ERR_IF_PASSTHROUGH;
			rc = trotListRefAppendInt( lr1, 0x100 );
			ERR_IF_PASSTHROUGH;
			rc = trotListRefAppendInt( lr1, 0x1000 );
			ERR_IF_PASSTHROUGH;
			rc = trotListRefAppendInt( lr1, 0x100000 );
			ERR_IF_PASSTHROUGH;
	
			i += 1;
		}

		rc = trotCharactersToUtf8( lr1, lr2 );
		ERR_IF_PASSTHROUGH;
		rc = trotUtf8ToCharacters( lr2, lr3 );
		ERR_IF_PASSTHROUGH;

		trotListRefFree( &lr1 );
		trotListRefFree( &lr2 );
		trotListRefFree( &lr3 );

		j += 1;
	}


	/* CLEANUP */
	cleanup:

	trotListRefFree( &lr1 );
	trotListRefFree( &lr2 );
	trotListRefFree( &lr3 );

	return rc;
}

/******************************************************************************/
/*!
	\brief 
	\param 
	\return TROT_RC
*/
static TROT_RC testFailedMallocs2( int test )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	trotListRef *lrCharacters = NULL;
	trotListRef *lrTokens = NULL;

	char *s1 = "[](){} 1 \"a\" xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";


	/* CODE */
	(void)test;

	rc = trotListRefInit( &lrCharacters );
	ERR_IF_PASSTHROUGH;

	rc = appendCStringToList( lrCharacters, s1 );
	ERR_IF_PASSTHROUGH;

	rc = trotTokenize( lrCharacters, &lrTokens );
	ERR_IF_PASSTHROUGH;


	/* CLEANUP */
	cleanup:

	trotListRefFree( &lrCharacters );
	trotListRefFree( &lrTokens );

	return rc;
}

/******************************************************************************/
/*!
	\brief 
	\param 
	\return TROT_RC
*/
static TROT_RC testFailedMallocs3( int test )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	char *d[] = {
	/* for encoding */
	"[]",
	"[100 10000 10000 10000 10000 10000 10000 10000 10000 10000 10000 10000 10000 10000 10000 ]",
	"[(name A) [(name B)] [A.B] ]",
	"{1000 1000 1000 1000 1000 1000 1000 1000}",
	"{[]}",
	"[(name A) [(name B)] { A.B } ]",
	"[(text)\"HelloHelloHelloHello\"]",
	"[ 1000 [(text)\"T\"] ]",
	"[ 100000 [(text)\"T\"] ]",
	"[[[[[[[[[[[[[[[[]]]]]]]]]]]]]]]]",
	"[12 3]",
	"[1 0]",
	"[(name top)[(name A)][(name B) 1 top.A]]",
	"[(name top)[(name A)][(name B) 1 1234567890 top.A]]",

	/* for decoding */
	"[ [(include \"x\")] [(include \"x\")] ]",
	"{(function x y) x}",
	"[(name x)(enum (cats 99) (dogs 98) ) x.cats]",
	"[(name x) x]"
	};

	trotListRef *lrCharacters = NULL;
	trotListRef *lrDecodedList1 = NULL;
	trotListRef *lrEncodedList1 = NULL;
	trotListRef *lrEmptyName = NULL;
	trotListRef *lrDecodedList2 = NULL;
	trotListRef *lrEncodedList2 = NULL;

#if PRINT_ENCODED_LISTS
	char *s = NULL;
#endif	


	/* CODE */
	rc = trotListRefInit( &lrEmptyName );
	ERR_IF_PASSTHROUGH;

	/* create characters */
	rc = trotListRefInit( &lrCharacters );
	ERR_IF_PASSTHROUGH;

	rc = appendCStringToList( lrCharacters, d[ test ] );
	ERR_IF_PASSTHROUGH;

	/* *** */
	rc = trotDecodeCharacters( badLoad, lrEmptyName, lrCharacters, &lrDecodedList1 );
	ERR_IF_PASSTHROUGH;

	rc = trotEncode( lrDecodedList1, &lrEncodedList1 );
	ERR_IF_PASSTHROUGH;

#if PRINT_ENCODED_LISTS
	rc = listToCString( lrEncodedList1, &s );
	ERR_IF_PASSTHROUGH;

	printf( "lrEncodedList1: %s\n", s );

	trotFree( s );
	s = NULL;
#endif


	rc = trotDecodeCharacters( badLoad, lrEmptyName, lrEncodedList1, &lrDecodedList2 );
	ERR_IF_PASSTHROUGH;

	rc = trotEncode( lrDecodedList2, &lrEncodedList2 );
	ERR_IF_PASSTHROUGH;

#if PRINT_ENCODED_LISTS
	rc = listToCString( lrEncodedList2, &s );
	ERR_IF_PASSTHROUGH;

	printf( "lrEncodedList2: %s\n", s );

	trotFree( s );
	s = NULL;
#endif


	/* CLEANUP */
	cleanup:

	trotListRefFree( &lrCharacters );
	trotListRefFree( &lrEmptyName );
	trotListRefFree( &lrDecodedList1 );
	trotListRefFree( &lrEncodedList1 );
	trotListRefFree( &lrDecodedList2 );
	trotListRefFree( &lrEncodedList2 );

	return rc;
}

/******************************************************************************/
/*!
	\brief 
	\param 
	\return TROT_RC
*/
static TROT_RC testFailedMallocs4( int test )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	trotListRef *lrDecodedList1 = NULL;
	trotListRef *lrEncodedList1 = NULL;
	trotListRef *lrEmptyName = NULL;
	trotListRef *lrDecodedList2 = NULL;
	trotListRef *lrEncodedList2 = NULL;

#if PRINT_ENCODED_LISTS
	char *s = NULL;
#endif	


	/* CODE */
	(void)test;

	rc = trotListRefInit( &lrEmptyName );
	ERR_IF_PASSTHROUGH;

	/* *** */
	rc = trotDecodeFilename( badLoad, lrEmptyName, &lrDecodedList1 );
	ERR_IF_PASSTHROUGH;

	rc = trotEncode( lrDecodedList1, &lrEncodedList1 );
	ERR_IF_PASSTHROUGH;

#if PRINT_ENCODED_LISTS
	rc = listToCString( lrEncodedList1, &s );
	ERR_IF_PASSTHROUGH;

	printf( "lrEncodedList1: %s\n", s );

	trotFree( s );
	s = NULL;
#endif


	rc = trotDecodeCharacters( badLoad, lrEmptyName, lrEncodedList1, &lrDecodedList2 );
	ERR_IF_PASSTHROUGH;

	rc = trotEncode( lrDecodedList2, &lrEncodedList2 );
	ERR_IF_PASSTHROUGH;

#if PRINT_ENCODED_LISTS
	rc = listToCString( lrEncodedList2, &s );
	ERR_IF_PASSTHROUGH;

	printf( "lrEncodedList2: %s\n", s );

	trotFree( s );
	s = NULL;
#endif


	/* CLEANUP */
	cleanup:

	trotListRefFree( &lrEmptyName );
	trotListRefFree( &lrDecodedList1 );
	trotListRefFree( &lrEncodedList1 );
	trotListRefFree( &lrDecodedList2 );
	trotListRefFree( &lrEncodedList2 );

	return rc;
}

