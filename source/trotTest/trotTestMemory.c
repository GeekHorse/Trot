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
#define TROT_FILE_NUMBER 500

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

/******************************************************************************/
static int testMemoryManagement();
static int testDeepList();

static TROT_RC testFailedMallocs1( int test );
static TROT_RC testFailedMallocs2( int test );

typedef struct
{
	TROT_RC (*func)( int test );
	int numberOfTests;
} FailedFunc;

FailedFunc failedFuncs[] =
{
	{ testFailedMallocs1, 1 },
	{ testFailedMallocs2, 11 },
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

	trotHookCalloc = badCalloc;
	trotHookMalloc = badMalloc;

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
				spinnerI += 1;
				printf( "\r%d: %d of %d %c", i, j, failedFuncs[ i ].numberOfTests - 1, spinner[ spinnerI % 4 ]  ); fflush( stdout );

				currentMallocCount = 0;
				failOnMallocCount += 1;

				rc = failedFuncs[ i ].func( j );
			}
			while ( rc == TROT_RC_ERROR_MEMORY_ALLOCATION_FAILED );

			if ( rc != TROT_RC_SUCCESS )
			{
				printf( "rc = %d", rc );
				TEST_ERR_IF( 1 );
			}

			j += 1;
		}

		printf( "\r%d            \n", i ); fflush( stdout );

		i += 1;
	}

	/* *** */
	trotHookCalloc = calloc;
	trotHookMalloc = malloc;

	/* **************************************** */
	/* test that calloc sets pointers to NULL */
	printf( "  Testing calloc...\n" ); fflush( stdout );
	iArray = (int **) trotHookCalloc( 10, sizeof( int * ) );
	TEST_ERR_IF( iArray == NULL );
	TEST_ERR_IF( iArray[ 5 ] != NULL );
	trotHookFree( iArray );

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

	TrotList **clientRefs = NULL;

	int r = 0;

	TrotList *ref = NULL;

	int howManyToAdd = 0;
	TROT_INT count = 0;
	TROT_INT randomIndex = 0;

	int clientRefIndex = 0;

	int countAdded = 0;
	int howManyToRemove = 0;


	/* CODE */
	/* create our client refs */
	clientRefs = (TrotList **) trotHookCalloc( MEMORY_MANAGEMENT_REFS_COUNT, sizeof( TrotList * ) );
	TEST_ERR_IF( clientRefs == NULL );

	i = 0;
	while ( i < MEMORY_MANAGEMENT_REFS_COUNT )
	{
		r = rand() % 2;

		/* twin aleady existing list */
		if ( r == 0 && i > 0 )
		{
			j = rand() % i;
			TEST_ERR_IF( trotListTwin( clientRefs[ j ], &( clientRefs[ i ] ) ) != TROT_RC_SUCCESS );
		}
		/* create new list */
		else
		{
			TEST_ERR_IF( trotListInit( &( clientRefs[ i ] ) ) != TROT_RC_SUCCESS );
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
			TEST_ERR_IF( trotListInit( &ref ) != TROT_RC_SUCCESS );
		}
		/* twin client list */
		else
		{
			r = rand() % MEMORY_MANAGEMENT_REFS_COUNT;
			TEST_ERR_IF( trotListTwin( clientRefs[ r ], &ref ) != TROT_RC_SUCCESS );
		}

		/* how many are we going to add? */
		howManyToAdd = rand() % 10; /* MAGIC */

		j = 0;
		while ( j < howManyToAdd )
		{
			/* which client ref to add to? */
			clientRefIndex = rand() % MEMORY_MANAGEMENT_REFS_COUNT;

			/* where to add it to? */
			TEST_ERR_IF( trotListGetCount( clientRefs[ clientRefIndex ], &count ) != TROT_RC_SUCCESS );

			randomIndex = ( rand() % ( count + 1 ) ) + 1;

			/* add */
			TEST_ERR_IF( trotListInsertList( clientRefs[ clientRefIndex ], randomIndex, ref ) != TROT_RC_SUCCESS );

			countAdded += 1;

			j += 1;
		}

		/* free our temporary ref */
		trotListFree( &ref );

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
		TEST_ERR_IF( trotListGetCount( clientRefs[ clientRefIndex ], &count ) != TROT_RC_SUCCESS );

		if ( count > 0 )
		{
			randomIndex = ( rand() % count ) + 1;

			/* "remove list", or "remove" */
			r = rand() % 2;
			if ( r == 0 )
			{
				TEST_ERR_IF( trotListRemoveList( clientRefs[ clientRefIndex ], randomIndex, &ref ) != TROT_RC_SUCCESS );
				trotListFree( &ref );
			}
			else
			{
				TEST_ERR_IF( trotListRemove( clientRefs[ clientRefIndex ], randomIndex ) != TROT_RC_SUCCESS );
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
				trotListFree( &( clientRefs[ j ] ) );
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
	trotHookFree( clientRefs );


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

	TrotList *refHead = NULL;

	TrotList *ref1 = NULL;
	TrotList *ref2 = NULL;


	/* CODE */
	TEST_ERR_IF( trotListInit( &refHead ) != TROT_RC_SUCCESS );

	TEST_ERR_IF( trotListTwin( refHead, &ref1 ) != TROT_RC_SUCCESS );

	while ( i < 1000 ) /* MAGIC */
	{
		TEST_ERR_IF( trotListInit( &ref2 ) != TROT_RC_SUCCESS );

		TEST_ERR_IF( trotListAppendList( ref1, ref2 ) != TROT_RC_SUCCESS );

		trotListFree( &ref1 );

		ref1 = ref2;
		ref2 = NULL;

		if ( i % 100 == 0 )
		{
			printf( "." ); fflush( stdout );
		}

		i += 1;
	}

	TEST_ERR_IF( trotListAppendList( ref1, refHead ) != TROT_RC_SUCCESS );

	trotListFree( &ref1 );

	printf( ":" ); fflush( stdout );

	trotListFree( &refHead );

	return TROT_RC_SUCCESS;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief 
	\param 
	\return TROT_RC
*/
static TROT_RC testFailedMallocs1( int test )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	int i = 0;
	int j = 0;

	TrotList *l1 = NULL;
	TrotList *l2 = NULL;
	TrotList *l3 = NULL;

	TROT_LIST_COMPARE_RESULT compareResult;


	/* CODE */
	(void)test;

	/* primary functions */
	rc = trotListInit( &l1 );
	ERR_IF_PASSTHROUGH;

	rc = trotListInit( &l2 );
	ERR_IF_PASSTHROUGH;

	i = 0;
	while ( i < (TROT_NODE_SIZE * 2) )
	{
		rc = trotListAppendInt( l1,  1 );
		ERR_IF_PASSTHROUGH;

		i += 1;
	}

	i = 0;
	while ( i < (TROT_NODE_SIZE * 2) )
	{
		rc = trotListAppendList( l1, l2 );
		ERR_IF_PASSTHROUGH;

		i += 1;
	}

	i = 0;
	while ( i < (TROT_NODE_SIZE * 2) )
	{
		rc = trotListAppendInt( l1,  1 );
		ERR_IF_PASSTHROUGH;

		i += 1;
	}

	rc = trotListReplaceWithInt( l1, (TROT_NODE_SIZE * 2) + 1, 0 );
	ERR_IF_PASSTHROUGH;
	rc = trotListReplaceWithInt( l1, (TROT_NODE_SIZE * 4), 0 );
	ERR_IF_PASSTHROUGH;
	rc = trotListReplaceWithInt( l1, (TROT_NODE_SIZE * 3) + 2, 0 );
	ERR_IF_PASSTHROUGH;


	trotListFree( &l1 );
	trotListFree( &l2 );


	rc = trotListInit( &l1 );
	ERR_IF_PASSTHROUGH;

	rc = trotListInit( &l2 );
	ERR_IF_PASSTHROUGH;

	i = 0;
	while ( i < (TROT_NODE_SIZE * 2) )
	{
		rc = trotListAppendList( l1, l2 );
		ERR_IF_PASSTHROUGH;

		i += 1;
	}

	i = 0;
	while ( i < (TROT_NODE_SIZE * 2) )
	{
		rc = trotListAppendInt( l1, 1 );
		ERR_IF_PASSTHROUGH;

		i += 1;
	}

	i = 0;
	while ( i < (TROT_NODE_SIZE * 2) )
	{
		rc = trotListAppendList( l1, l2 );
		ERR_IF_PASSTHROUGH;

		i += 1;
	}

	rc = trotListReplaceWithList( l1, (TROT_NODE_SIZE * 2) + 1, l2 );
	ERR_IF_PASSTHROUGH;
	rc = trotListReplaceWithList( l1, (TROT_NODE_SIZE * 4), l2 );
	ERR_IF_PASSTHROUGH;
	rc = trotListReplaceWithList( l1, (TROT_NODE_SIZE * 3) + 2, l2 );
	ERR_IF_PASSTHROUGH;


	trotListFree( &l1 );
	trotListFree( &l2 );


	rc = trotListInit( &l1 );
	ERR_IF_PASSTHROUGH;

	rc = trotListInit( &l2 );
	ERR_IF_PASSTHROUGH;

	i = 0;
	while ( i < TROT_NODE_SIZE + 2 )
	{
		rc = trotListInsertInt( l1, 1, 1 );
		ERR_IF_PASSTHROUGH;

		i += 1;
	}

	rc = trotListAppendInt( l1, 1 );
	ERR_IF_PASSTHROUGH;

	rc = trotListInsertList( l1, -1, l2 );
	ERR_IF_PASSTHROUGH;

	i = 0;
	while ( i < TROT_NODE_SIZE + 2 )
	{
		rc = trotListInsertList( l1, -2, l2 );
		ERR_IF_PASSTHROUGH;

		i += 1;
	}

	rc = trotListAppendList( l1, l2 );
	ERR_IF_PASSTHROUGH;

	rc = trotListInsertList( l1, 2, l2 );
	ERR_IF_PASSTHROUGH;

	rc = trotListInsertInt( l1, -2, 1 );
	ERR_IF_PASSTHROUGH;

	rc = trotListInsertList( l1, 1, l2 );
	ERR_IF_PASSTHROUGH;
	rc = trotListInsertList( l1, 2, l2 );
	ERR_IF_PASSTHROUGH;

	rc = trotListGetList( l1, -1, &l3 );
	ERR_IF_PASSTHROUGH;
	trotListFree( &l3 );

	rc = trotListAppendInt( l1, 1 );
	ERR_IF_PASSTHROUGH;
	rc = trotListAppendInt( l1, 1 );
	ERR_IF_PASSTHROUGH;

	/* secondary functions */
	rc = trotListCompare( l1, l2, &compareResult );
	ERR_IF_PASSTHROUGH;

	trotListFree( &l2 );

	rc = trotListCopy( l1, &l2 );
	ERR_IF_PASSTHROUGH;

	trotListFree( &l2 );

	rc = trotListEnlist( l1, 2, -2 );
	ERR_IF_PASSTHROUGH;

	rc = trotListDelist( l1, 2 );
	ERR_IF_PASSTHROUGH;

	rc = trotListCopySpan( l1, 1, -2, &l2 );
	ERR_IF_PASSTHROUGH;

	rc = trotListDelist( l1, -6 );
	ERR_IF_PASSTHROUGH;

	trotListFree( &l1 );
	trotListFree( &l2 );

	/* *** */
	rc = trotListInit( &l1 );
	ERR_IF_PASSTHROUGH;
	rc = trotListCopy( l1, &l2 );
	ERR_IF_PASSTHROUGH;

	trotListFree( &l1 );
	trotListFree( &l2 );

	/* *** */
	rc = trotListInit( &l1 );
	ERR_IF_PASSTHROUGH;
	rc = trotListInit( &l3 );
	ERR_IF_PASSTHROUGH;
	rc = trotListAppendInt( l3, 1 );
	ERR_IF_PASSTHROUGH;
	rc = trotListAppendList( l1, l3 );
	ERR_IF_PASSTHROUGH;
	trotListFree( &l3 );

	rc = trotListInit( &l2 );
	ERR_IF_PASSTHROUGH;
	rc = trotListInit( &l3 );
	ERR_IF_PASSTHROUGH;
	rc = trotListAppendInt( l3, 2 );
	ERR_IF_PASSTHROUGH;
	rc = trotListAppendList( l2, l3 );
	ERR_IF_PASSTHROUGH;
	trotListFree( &l3 );
	
	rc = trotListCompare( l1, l2, &compareResult );
	ERR_IF_PASSTHROUGH;

	trotListFree( &l1 );
	trotListFree( &l2 );


	/* It just so happens these values are 1 byte, 2 byte,
	   3 byte, and 4 byte utf8 characters */
	j = 0;
	while ( j < 4 )
	{
		/* *** */
		rc = trotListInit( &l1 );
		ERR_IF_PASSTHROUGH;
		rc = trotListInit( &l2 );
		ERR_IF_PASSTHROUGH;
		rc = trotListInit( &l3 );
		ERR_IF_PASSTHROUGH;

		/* *** */
		i = 0;
		while ( i <= j )
		{
			rc = trotListAppendInt( l1, 0x10 );
			ERR_IF_PASSTHROUGH;

			i += 1;
		}

		/* *** */
		i = 0;
		while ( i < TROT_NODE_SIZE + 1 )
		{
			rc = trotListAppendInt( l1, 0x10 );
			ERR_IF_PASSTHROUGH;
			rc = trotListAppendInt( l1, 0x100 );
			ERR_IF_PASSTHROUGH;
			rc = trotListAppendInt( l1, 0x1000 );
			ERR_IF_PASSTHROUGH;
			rc = trotListAppendInt( l1, 0x100000 );
			ERR_IF_PASSTHROUGH;
	
			i += 1;
		}

		rc = trotCharactersToUtf8( l1, l2 );
		ERR_IF_PASSTHROUGH;
		rc = trotUtf8ToCharacters( l2, l3 );
		ERR_IF_PASSTHROUGH;

		trotListFree( &l1 );
		trotListFree( &l2 );
		trotListFree( &l3 );

		j += 1;
	}


	/* CLEANUP */
	cleanup:

	trotListFree( &l1 );
	trotListFree( &l2 );
	trotListFree( &l3 );

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

	char *d[] = {
	/* for encoding */
	"[ [ 11 ] [ 11 ] [ 11 ] [ 11 ] [ 11 ] [ 11 ] [ 11 ] [ 11 ] [ 11 ] [ 11 ] [ 11 ] [ 11 ] [ 11 ] [ 11 ] [ 11 ] [ 11 ] [ 11 ] ]",
	"[ [ ] @.1 11 @.1 11 @.1 11 @.1 11 @.1 11 @.1 11 @.1 11 @.1 11 @.1 11 @.1 11 @.1 11 @.1 11 @.1 11 @.1 11 @.1 11 @.1 11 @.1 11 @.1 11 @.1 ]",
	"[ 0 11 0 11 0 11 0 11 0 11 0 11 0 11 0 11 0 11 0 11 0 11 0 ]",
	"[ [ ~1 ] [ ~1 ] [ ~1 ] [ ~1 ] [ ~1 ] [ ~1 ] [ ~1 ] [ ~1 ] [ ~1 ] [ ~1 ] [ ~1 ] [ ~1 ] [ ~1 ] [ ~1 ] [ ~1 ] ]",
	"[ [ `1 ] [ `1 ] [ `1 ] [ `1 ] [ `1 ] [ `1 ] [ `1 ] [ `1 ] [ `1 ] [ `1 ] [ `1 ] [ `1 ] [ `1 ] [ `1 ] [ `1 ] ]",
	"[ @ 11 @ 11 @ 11 @ 11 @ 11 @ 11 @ 11 @ 11 @ 11 @ 11 @ 11 @ 11 @ 11 @ 11 @ 11 @ 11 @ 11 @ 11 @ 11 @ 11 @ 11 @ 11 @ 11 @ 11 @ 11 @ 11 @ ]",
	"[ ~1 \"x\" 2000 \"x\" 2000 \"x\" 2000 \"x\" 2000 \"x\" 2000 \"x\" 2000 \"x\" 2000 \"x\" 2000 \"x\" 2000 \"x\" 2000 \"x\" 2000 \"x\" 2000 \"x\" 2000 \"x\" 2000 \"x\" 2000 \"x\" 2000 \"x\" 2000 ]",
	"[ ~1 \"xxxxxxxxxx\" ]",
	"[ ~1 \"xxxxxxxxx\" ]",

	/* for decoding */
	"[ [ ] @.1 1 ]",
	"[ ~1 \"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\" ]"
	};

	TrotList *lCharacters = NULL;
	TrotList *lDecodedList1 = NULL;
	TrotList *lEncodedList1 = NULL;
	TrotList *lDecodedList2 = NULL;
	TrotList *lEncodedList2 = NULL;

#if PRINT_ENCODED_LISTS
	char *s = NULL;
#endif	


	/* CODE */
	/* create characters */
	rc = trotListInit( &lCharacters );
	ERR_IF_PASSTHROUGH;

	rc = appendCStringToList( lCharacters, d[ test ] );
	ERR_IF_PASSTHROUGH;

	/* *** */
	rc = trotDecode( lCharacters, &lDecodedList1 );
	ERR_IF_PASSTHROUGH;


	rc = trotEncode( lDecodedList1, &lEncodedList1 );
	ERR_IF_PASSTHROUGH;

#if PRINT_ENCODED_LISTS
	rc = listToCString( lEncodedList1, &s );
	ERR_IF_PASSTHROUGH;

	printf( "lEncodedList1: %s\n", s );

	trotHookFree( s );
	s = NULL;
#endif

	rc = trotDecode( lEncodedList1, &lDecodedList2 );
	ERR_IF_PASSTHROUGH;

	rc = trotEncode( lDecodedList2, &lEncodedList2 );
	ERR_IF_PASSTHROUGH;

#if PRINT_ENCODED_LISTS
	rc = listToCString( lEncodedList2, &s );
	ERR_IF_PASSTHROUGH;

	printf( "lEncodedList2: %s\n", s );

	trotHookFree( s );
	s = NULL;
#endif


	/* CLEANUP */
	cleanup:

	trotListFree( &lCharacters );
	trotListFree( &lDecodedList1 );
	trotListFree( &lEncodedList1 );
	trotListFree( &lDecodedList2 );
	trotListFree( &lEncodedList2 );

	return rc;
}


