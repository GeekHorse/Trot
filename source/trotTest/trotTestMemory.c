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

void *trotHookMalloc( size_t size )
{
	currentMallocCount += 1;
	if ( currentMallocCount == failOnMallocCount )
	{
		return NULL;
	}

	return malloc( size );
}

void *trotHookCalloc( size_t nmemb, size_t size )
{
	currentMallocCount += 1;
	if ( currentMallocCount == failOnMallocCount )
	{
		return NULL;
	}

	return calloc( nmemb, size );
}

void trotHookFree( void *ptr )
{
	free( ptr );
}

/******************************************************************************/
static int testMemoryManagement( TrotProgram *program );
static int testDeepList( TrotProgram *program );

static TROT_RC testFailedMallocs1( TrotProgram *program, int test );
static TROT_RC testFailedMallocs2( TrotProgram *program, int test );

typedef struct
{
	TROT_RC (*func)( TrotProgram *program, int test );
	int numberOfTests;
} FailedFunc;

static FailedFunc failedFuncs[] =
{
	{ testFailedMallocs1, 1 },
	{ testFailedMallocs2, 7 },
	{ NULL, 0 }
};

/******************************************************************************/
int testMemory( TrotProgram *program )
{
	/* DATA */
	int rc = 0;

	TROT_INT memUsed = 0;

	int **iArray = NULL;

	int i = 0;

	TrotProgram *testProgram = NULL;
	TROT_INT testMemLimit = 0;

	int j = 0;
	int flagAtLeastOneFailed = 0;
	char *spinner = "-\\|/";
	unsigned int spinnerI = 0;


	/* CODE */
	/* **************************************** */
	printf( "Testing memory management and garbage collection...\n" ); fflush( stdout );

	/* **************************************** */
	/* testing bad mallocs */
	printf( "  Testing bad mallocs...\n" ); fflush( stdout );

#ifndef TROT_DEBUG

	printf( "\n\n\n" );
	printf( "--- SKIPPING FAILED MALLOC TESTS! ------\n" );
	printf( "You need to build a debug version of Trot to test failed mallocs.\n" );
	printf( "----------------------------------------\n" );
	printf( "\n\n\n" );

#else

	i = 0;
	while ( failedFuncs[ i ].func != NULL )
	{
		j = 0;
		while ( j < failedFuncs[ i ].numberOfTests )
		{
			failOnMallocCount = 0;
			spinnerI = 0;
			flagAtLeastOneFailed = 0;

			do
			{
				spinnerI += 1;
				printf( "\r%d: %d of %d %c", i, j, failedFuncs[ i ].numberOfTests - 1, spinner[ spinnerI % 4 ]  ); fflush( stdout );

				currentMallocCount = 0;
				failOnMallocCount += 1;

				rc = failedFuncs[ i ].func( program, j );

				if ( rc == TROT_RC_ERROR_MEMORY_ALLOCATION_FAILED )
				{
					flagAtLeastOneFailed = 1;
				}
			}
			while ( rc == TROT_RC_ERROR_MEMORY_ALLOCATION_FAILED );

			if ( rc != TROT_RC_SUCCESS )
			{
				printf( "\nERROR: Expecting PINTO_RC_SUCCESS but got %s.\n", trotRCToString( rc ) );
				TEST_ERR_IF( 1 );
			}

			if ( flagAtLeastOneFailed == 0 )
			{
				printf( "There were no memory failures. There should have been at least 1.\n" );
				TEST_ERR_IF( 1 );
			}

			j += 1;
		}

		printf( "\r%d            \n", i ); fflush( stdout );

		i += 1;
	}

	failOnMallocCount = 0;
#endif

	/* **************************************** */
	/* testing mem limit */
	printf( "  Testing mem limit...\n" ); fflush( stdout );

	TEST_ERR_IF( trotProgramMemoryGetUsed( program, &memUsed ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( memUsed != 0 );

	TROT_MALLOC( iArray, 10 );

	TEST_ERR_IF( trotProgramMemoryGetUsed( program, &memUsed ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( memUsed != (10 * sizeof(int *) ) );

	TROT_FREE( iArray, 10 );

	TEST_ERR_IF( trotProgramMemoryGetUsed( program, &memUsed ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( memUsed != 0 );

	TROT_CALLOC( iArray, 10 );

	TEST_ERR_IF( trotProgramMemoryGetUsed( program, &memUsed ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( memUsed != (10 * sizeof(int *) ) );

	TROT_FREE( iArray, 10 );

	TEST_ERR_IF( trotProgramMemoryGetUsed( program, &memUsed ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( memUsed != 0 );

	/* *** */
	testMemLimit = 0;
	TEST_ERR_IF( ( testProgram = TROT_HOOK_CALLOC( 1, sizeof( *program ) ) ) == NULL );
	TEST_ERR_IF( trotProgramMemorySetLimit( testProgram, testMemLimit ) != TROT_RC_SUCCESS );

	i = 0;
	while ( failedFuncs[ i ].func != NULL )
	{
		j = 0;
		while ( j < failedFuncs[ i ].numberOfTests )
		{
			spinnerI = 0;
			flagAtLeastOneFailed = 0;
			testMemLimit = 0;

			do
			{
				spinnerI += 1;
				printf( "\r%d: %d of %d %c", i, j, failedFuncs[ i ].numberOfTests - 1, spinner[ spinnerI % 4 ]  ); fflush( stdout );

				TEST_ERR_IF( trotProgramMemorySetLimit( testProgram, testMemLimit ) != TROT_RC_SUCCESS );

				rc = failedFuncs[ i ].func( testProgram, j );

				if ( rc == TROT_RC_ERROR_MEM_LIMIT )
				{
					flagAtLeastOneFailed = 1;
				}

				testMemLimit += 1;
			}
			while ( rc == TROT_RC_ERROR_MEM_LIMIT );

			if ( rc != TROT_RC_SUCCESS )
			{
				printf( "\nERROR: Expecting PINTO_RC_SUCCESS but got %s.\n", trotRCToString( rc ) );
				TEST_ERR_IF( 1 );
			}

			if ( flagAtLeastOneFailed == 0 )
			{
				printf( "There were no mem limit failures. There should have been at least 1.\n" );
				TEST_ERR_IF( 1 );
			}

			j += 1;
		}

		printf( "\r%d            \n", i ); fflush( stdout );

		i += 1;
	}

	TROT_HOOK_FREE( testProgram );

	/* **************************************** */
	/* test that calloc sets pointers to NULL */
	printf( "  Testing calloc...\n" ); fflush( stdout );
	TROT_CALLOC( iArray, 10 );
	TEST_ERR_IF( iArray == NULL );
	TEST_ERR_IF( iArray[ 5 ] != NULL );
	TROT_FREE( iArray, 10 );

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

		TEST_ERR_IF( testMemoryManagement( program ) != 0 );

		/* *** */
		i += 1;
	}

	printf( "\n" ); fflush( stdout );

	/* **************************************** */
	/* *** */
	printf( "  Testing a \"deep list\"...\n" ); fflush( stdout );
	TEST_ERR_IF( testDeepList( program ) != 0 );

	printf( "\n" ); fflush( stdout );

	/* **************************************** */
	return 0;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
static int testMemoryManagement( TrotProgram *program )
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
	TROT_CALLOC( clientRefs, MEMORY_MANAGEMENT_REFS_COUNT );

	i = 0;
	while ( i < MEMORY_MANAGEMENT_REFS_COUNT )
	{
		r = rand() % 2;

		/* twin aleady existing list */
		if ( r == 0 && i > 0 )
		{
			j = rand() % i;
			TEST_ERR_IF( trotListTwin( program, clientRefs[ j ], &( clientRefs[ i ] ) ) != TROT_RC_SUCCESS );
		}
		/* create new list */
		else
		{
			TEST_ERR_IF( trotListInit( program, &( clientRefs[ i ] ) ) != TROT_RC_SUCCESS );
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
			TEST_ERR_IF( trotListInit( program, &ref ) != TROT_RC_SUCCESS );
		}
		/* twin client list */
		else
		{
			r = rand() % MEMORY_MANAGEMENT_REFS_COUNT;
			TEST_ERR_IF( trotListTwin( program, clientRefs[ r ], &ref ) != TROT_RC_SUCCESS );
		}

		/* how many are we going to add? */
		howManyToAdd = rand() % 10; /* MAGIC */

		j = 0;
		while ( j < howManyToAdd )
		{
			/* which client ref to add to? */
			clientRefIndex = rand() % MEMORY_MANAGEMENT_REFS_COUNT;

			/* where to add it to? */
			TEST_ERR_IF( trotListGetCount( program, clientRefs[ clientRefIndex ], &count ) != TROT_RC_SUCCESS );

			randomIndex = ( rand() % ( count + 1 ) ) + 1;

			/* add */
			TEST_ERR_IF( trotListInsertList( program, clientRefs[ clientRefIndex ], randomIndex, ref ) != TROT_RC_SUCCESS );

			countAdded += 1;

			j += 1;
		}

		/* free our temporary ref */
		trotListFree( program, &ref );

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
		TEST_ERR_IF( trotListGetCount( program, clientRefs[ clientRefIndex ], &count ) != TROT_RC_SUCCESS );

		if ( count > 0 )
		{
			randomIndex = ( rand() % count ) + 1;

			/* "remove list", or "remove" */
			r = rand() % 2;
			if ( r == 0 )
			{
				TEST_ERR_IF( trotListRemoveList( program, clientRefs[ clientRefIndex ], randomIndex, &ref ) != TROT_RC_SUCCESS );
				trotListFree( program, &ref );
			}
			else
			{
				TEST_ERR_IF( trotListRemove( program, clientRefs[ clientRefIndex ], randomIndex ) != TROT_RC_SUCCESS );
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
				trotListFree( program, &( clientRefs[ j ] ) );
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
	TROT_FREE( clientRefs, MEMORY_MANAGEMENT_REFS_COUNT );


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
static int testDeepList( TrotProgram *program )
{
	/* DATA */
	int rc = 0;

	int i = 0;

	TrotList *refHead = NULL;

	TrotList *ref1 = NULL;
	TrotList *ref2 = NULL;


	/* CODE */
	TEST_ERR_IF( trotListInit( program, &refHead ) != TROT_RC_SUCCESS );

	TEST_ERR_IF( trotListTwin( program, refHead, &ref1 ) != TROT_RC_SUCCESS );

	while ( i < 1000 ) /* MAGIC */
	{
		TEST_ERR_IF( trotListInit( program, &ref2 ) != TROT_RC_SUCCESS );

		TEST_ERR_IF( trotListAppendList( program, ref1, ref2 ) != TROT_RC_SUCCESS );

		trotListFree( program, &ref1 );

		ref1 = ref2;
		ref2 = NULL;

		if ( i % 100 == 0 )
		{
			printf( "." ); fflush( stdout );
		}

		i += 1;
	}

	TEST_ERR_IF( trotListAppendList( program, ref1, refHead ) != TROT_RC_SUCCESS );

	trotListFree( program, &ref1 );

	printf( ":" ); fflush( stdout );

	trotListFree( program, &refHead );

	return TROT_RC_SUCCESS;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
static TROT_RC testFailedMallocs1( TrotProgram *program, int test )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	int i = 0;

	TrotList *l1 = NULL;
	TrotList *l2 = NULL;
	TrotList *l3 = NULL;


	/* CODE */
	(void)test;

	/* primary functions */
	rc = trotListInit( program, &l1 );
	ERR_IF_PASSTHROUGH;

	rc = trotListInit( program, &l2 );
	ERR_IF_PASSTHROUGH;

	i = 0;
	while ( i < (TROT_NODE_SIZE * 2) )
	{
		rc = trotListAppendInt( program, l1,  1 );
		ERR_IF_PASSTHROUGH;

		i += 1;
	}

	i = 0;
	while ( i < (TROT_NODE_SIZE * 2) )
	{
		rc = trotListAppendList( program, l1, l2 );
		ERR_IF_PASSTHROUGH;

		i += 1;
	}

	i = 0;
	while ( i < (TROT_NODE_SIZE * 2) )
	{
		rc = trotListAppendInt( program, l1,  1 );
		ERR_IF_PASSTHROUGH;

		i += 1;
	}

	rc = trotListReplaceWithInt( program, l1, (TROT_NODE_SIZE * 2) + 1, 0 );
	ERR_IF_PASSTHROUGH;
	rc = trotListReplaceWithInt( program, l1, (TROT_NODE_SIZE * 4), 0 );
	ERR_IF_PASSTHROUGH;
	rc = trotListReplaceWithInt( program, l1, (TROT_NODE_SIZE * 3) + 2, 0 );
	ERR_IF_PASSTHROUGH;


	trotListFree( program, &l1 );
	trotListFree( program, &l2 );


	rc = trotListInit( program, &l1 );
	ERR_IF_PASSTHROUGH;

	rc = trotListInit( program, &l2 );
	ERR_IF_PASSTHROUGH;

	i = 0;
	while ( i < (TROT_NODE_SIZE * 2) )
	{
		rc = trotListAppendList( program, l1, l2 );
		ERR_IF_PASSTHROUGH;

		i += 1;
	}

	i = 0;
	while ( i < (TROT_NODE_SIZE * 2) )
	{
		rc = trotListAppendInt( program, l1, 1 );
		ERR_IF_PASSTHROUGH;

		i += 1;
	}

	i = 0;
	while ( i < (TROT_NODE_SIZE * 2) )
	{
		rc = trotListAppendList( program, l1, l2 );
		ERR_IF_PASSTHROUGH;

		i += 1;
	}

	rc = trotListReplaceWithList( program, l1, (TROT_NODE_SIZE * 2) + 1, l2 );
	ERR_IF_PASSTHROUGH;
	rc = trotListReplaceWithList( program, l1, (TROT_NODE_SIZE * 4), l2 );
	ERR_IF_PASSTHROUGH;
	rc = trotListReplaceWithList( program, l1, (TROT_NODE_SIZE * 3) + 2, l2 );
	ERR_IF_PASSTHROUGH;


	trotListFree( program, &l1 );
	trotListFree( program, &l2 );


	rc = trotListInit( program, &l1 );
	ERR_IF_PASSTHROUGH;

	rc = trotListInit( program, &l2 );
	ERR_IF_PASSTHROUGH;

	i = 0;
	while ( i < TROT_NODE_SIZE + 2 )
	{
		rc = trotListInsertInt( program, l1, 1, 1 );
		ERR_IF_PASSTHROUGH;

		i += 1;
	}

	rc = trotListAppendInt( program, l1, 1 );
	ERR_IF_PASSTHROUGH;

	rc = trotListInsertList( program, l1, -1, l2 );
	ERR_IF_PASSTHROUGH;

	i = 0;
	while ( i < TROT_NODE_SIZE + 2 )
	{
		rc = trotListInsertList( program, l1, -2, l2 );
		ERR_IF_PASSTHROUGH;

		i += 1;
	}

	rc = trotListAppendList( program, l1, l2 );
	ERR_IF_PASSTHROUGH;

	rc = trotListInsertList( program, l1, 2, l2 );
	ERR_IF_PASSTHROUGH;

	rc = trotListInsertInt( program, l1, -2, 1 );
	ERR_IF_PASSTHROUGH;

	rc = trotListInsertList( program, l1, 1, l2 );
	ERR_IF_PASSTHROUGH;
	rc = trotListInsertList( program, l1, 2, l2 );
	ERR_IF_PASSTHROUGH;

	rc = trotListGetList( program, l1, -1, &l3 );
	ERR_IF_PASSTHROUGH;
	trotListFree( program, &l3 );

	rc = trotListAppendInt( program, l1, 1 );
	ERR_IF_PASSTHROUGH;
	rc = trotListAppendInt( program, l1, 1 );
	ERR_IF_PASSTHROUGH;

	/* secondary functions */
	trotListFree( program, &l2 );

	rc = trotListCopy( program, l1, &l2 );
	ERR_IF_PASSTHROUGH;

	trotListFree( program, &l2 );

	rc = trotListEnlist( program, l1, 2, -2 );
	ERR_IF_PASSTHROUGH;

	rc = trotListDelist( program, l1, 2 );
	ERR_IF_PASSTHROUGH;

	rc = trotListCopySpan( program, l1, 1, -2, &l2 );
	ERR_IF_PASSTHROUGH;

	rc = trotListDelist( program, l1, -6 );
	ERR_IF_PASSTHROUGH;

	trotListFree( program, &l1 );
	trotListFree( program, &l2 );

	/* *** */
	rc = trotListInit( program, &l1 );
	ERR_IF_PASSTHROUGH;
	rc = trotListCopy( program, l1, &l2 );
	ERR_IF_PASSTHROUGH;

	trotListFree( program, &l1 );
	trotListFree( program, &l2 );

	/* *** */
	rc = trotListInit( program, &l1 );
	ERR_IF_PASSTHROUGH;
	rc = trotListInit( program, &l3 );
	ERR_IF_PASSTHROUGH;
	rc = trotListAppendInt( program, l3, 1 );
	ERR_IF_PASSTHROUGH;
	rc = trotListAppendList( program, l1, l3 );
	ERR_IF_PASSTHROUGH;
	trotListFree( program, &l3 );

	rc = trotListInit( program, &l2 );
	ERR_IF_PASSTHROUGH;
	rc = trotListInit( program, &l3 );
	ERR_IF_PASSTHROUGH;
	rc = trotListAppendInt( program, l3, 2 );
	ERR_IF_PASSTHROUGH;
	rc = trotListAppendList( program, l2, l3 );
	ERR_IF_PASSTHROUGH;
	trotListFree( program, &l3 );

	trotListFree( program, &l1 );
	trotListFree( program, &l2 );


	/* CLEANUP */
	cleanup:

	trotListFree( program, &l1 );
	trotListFree( program, &l2 );
	trotListFree( program, &l3 );

	return rc;
}

/******************************************************************************/
static TROT_RC testFailedMallocs2( TrotProgram *program, int test )
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

	/* for decoding */
	"[ [ ] @.1 1 ]",
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
	rc = trotListInit( program, &lCharacters );
	ERR_IF_PASSTHROUGH;

	rc = appendCStringToList( program, lCharacters, d[ test ] );
	ERR_IF_PASSTHROUGH;

	/* *** */
	rc = trotDecode( program, lCharacters, &lDecodedList1 );
	ERR_IF_PASSTHROUGH;


	rc = trotEncode( program, lDecodedList1, &lEncodedList1 );
	ERR_IF_PASSTHROUGH;

#if PRINT_ENCODED_LISTS
	rc = listToCString( program, lEncodedList1, &s );
	ERR_IF_PASSTHROUGH;

	printf( "lEncodedList1: %s\n", s );

	TROT_FREE( s, strlen( s ) + 1 );
	s = NULL;
#endif

	rc = trotDecode( program, lEncodedList1, &lDecodedList2 );
	ERR_IF_PASSTHROUGH;

	rc = trotEncode( program, lDecodedList2, &lEncodedList2 );
	ERR_IF_PASSTHROUGH;

#if PRINT_ENCODED_LISTS
	rc = listToCString( program, lEncodedList2, &s );
	ERR_IF_PASSTHROUGH;

	printf( "lEncodedList2: %s\n", s );

	TROT_FREE( s, strlen( s ) + 1 );
	s = NULL;
#endif


	/* CLEANUP */
	cleanup:

	trotListFree( program, &lCharacters );
	trotListFree( program, &lDecodedList1 );
	trotListFree( program, &lEncodedList1 );
	trotListFree( program, &lDecodedList2 );
	trotListFree( program, &lEncodedList2 );

	return rc;
}

