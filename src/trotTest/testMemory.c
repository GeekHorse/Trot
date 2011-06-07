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
#include "trotCommon.h"
#include "trotList.h"
#include "trotMem.h"

#include "testCommon.h"

#include "stdio.h"
#include "time.h"
#include "string.h"

/******************************************************************************/
#define MEMORY_MANAGEMENT_REFS_COUNT 10

/******************************************************************************/
/* malloc/calloc that will always fail */
void *badCalloc( size_t nmemb, size_t size )
{
	(void)nmemb;
	(void)size;
	return NULL;
}

void *badMalloc( size_t size )
{
	(void)size;
	return NULL;
}

/******************************************************************************/
static int testMemoryManagement();
static int testDeepList();

/******************************************************************************/
int testMemory()
{
	/* DATA */
	int rc = 0;

	int **iArray = NULL;

	int i = 0;

	gkListRef *lr = NULL;
	gkListRef *lr2 = NULL;


	/* CODE */
	/* **************************************** */
	printf( "Testing memory management and garbage collection...\n" ); fflush( stdout );

	/* **************************************** */
	/* test that calloc sets pointers to NULL */
	printf( "  Testing calloc...\n" ); fflush( stdout );
	iArray = (int **) gkCalloc( 10, sizeof( int * ) );
	TEST_ERR_IF( iArray == NULL );
	TEST_ERR_IF( iArray[ 5 ] != NULL );
	gkFree( iArray );

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
	/* testing bad mallocs */
	printf( "  Testing bad mallocs...\n" ); fflush( stdout );

	TEST_ERR_IF( gkListRefInit( &lr ) != GK_LIST_SUCCESS );

	gkCalloc = badCalloc;
	gkMalloc = badMalloc;

	TEST_ERR_IF( gkListRefInit( &lr2 ) != GK_LIST_ERROR_MEMORY_ALLOCATION_FAILED );
	TEST_ERR_IF( gkListRefTwin( &lr2, lr ) != GK_LIST_ERROR_MEMORY_ALLOCATION_FAILED );
	TEST_ERR_IF( gkListRefAppendInt( lr, 1 ) != GK_LIST_ERROR_MEMORY_ALLOCATION_FAILED );
	TEST_ERR_IF( gkListRefAppendListTwin( lr, lr ) != GK_LIST_ERROR_MEMORY_ALLOCATION_FAILED );
	TEST_ERR_IF( gkListRefInsertInt( lr, 1, 1 ) != GK_LIST_ERROR_MEMORY_ALLOCATION_FAILED );
	TEST_ERR_IF( gkListRefInsertListTwin( lr, 1, lr ) != GK_LIST_ERROR_MEMORY_ALLOCATION_FAILED );

	gkCalloc = calloc;
	gkMalloc = malloc;

	gkListRefFree( &lr );

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

	gkListRef **clientRefs = NULL;

	int r = 0;

	gkListRef *ref = NULL;

	int howManyToAdd = 0;
	INT_TYPE count = 0;
	INT_TYPE randomIndex = 0;

	int clientRefIndex = 0;

	int countAdded = 0;
	int howManyToRemove = 0;


	/* CODE */
	/* create our client refs */
	clientRefs = (gkListRef **) gkCalloc( MEMORY_MANAGEMENT_REFS_COUNT, sizeof( gkListRef * ) );
	TEST_ERR_IF( clientRefs == NULL );

	i = 0;
	while ( i < MEMORY_MANAGEMENT_REFS_COUNT )
	{
		r = rand() % 2;

		/* twin already existing list */
		if ( r == 0 && i > 0 )
		{
			j = rand() % i;
			TEST_ERR_IF( gkListRefTwin( &( clientRefs[ i ] ), clientRefs[ j ] ) != GK_LIST_SUCCESS );
		}
		/* create new list */
		else
		{
			TEST_ERR_IF( gkListRefInit( &( clientRefs[ i ] ) ) != GK_LIST_SUCCESS );
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
			TEST_ERR_IF( gkListRefInit( &ref ) != GK_LIST_SUCCESS );
		}
		/* twin client list */
		else
		{
			r = rand() % MEMORY_MANAGEMENT_REFS_COUNT;
			TEST_ERR_IF( gkListRefTwin( &ref, clientRefs[ r ] ) != GK_LIST_SUCCESS );
		}

		/* how many are we going to add? */
		howManyToAdd = rand() % 10; /* MAGIC */

		j = 0;
		while ( j < howManyToAdd )
		{
			/* which client ref to add to? */
			clientRefIndex = rand() % MEMORY_MANAGEMENT_REFS_COUNT;

			/* where to add it to? */
			TEST_ERR_IF( gkListRefGetCount( clientRefs[ clientRefIndex ], &count ) != GK_LIST_SUCCESS );

			randomIndex = ( rand() % ( count + 1 ) ) + 1;

			/* add */
			TEST_ERR_IF( gkListRefInsertListTwin( clientRefs[ clientRefIndex ], randomIndex, ref ) != GK_LIST_SUCCESS );

			countAdded += 1;

			j += 1;
		}

		/* free our temporary ref */
		TEST_ERR_IF( gkListRefFree( &ref ) != GK_LIST_SUCCESS );

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
		TEST_ERR_IF( gkListRefGetCount( clientRefs[ clientRefIndex ], &count ) != GK_LIST_SUCCESS );

		if ( count > 0 )
		{
			randomIndex = ( rand() % count ) + 1;

			/* "remove list", or "remove" */
			r = rand() % 2;
			if ( r == 0 )
			{
				TEST_ERR_IF( gkListRefRemoveList( clientRefs[ clientRefIndex ], randomIndex, &ref ) != GK_LIST_SUCCESS );
				TEST_ERR_IF( gkListRefFree( &ref ) != GK_LIST_SUCCESS );
			}
			else
			{
				TEST_ERR_IF( gkListRefRemove( clientRefs[ clientRefIndex ], randomIndex ) != GK_LIST_SUCCESS );
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
				TEST_ERR_IF( gkListRefFree( &( clientRefs[ j ] ) ) != GK_LIST_SUCCESS );
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
	gkFree( clientRefs );


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
static int testDeepList()
{
	/* DATA */
	int rc = GK_LIST_SUCCESS;

	int i = 0;

	gkListRef *refHead = NULL;

	gkListRef *ref1 = NULL;
	gkListRef *ref2 = NULL;


	/* CODE */
	TEST_ERR_IF( gkListRefInit( &refHead ) != GK_LIST_SUCCESS );

	TEST_ERR_IF( gkListRefTwin( &ref1, refHead ) != GK_LIST_SUCCESS );

	while ( i < 1000 ) /* MAGIC */
	{
		TEST_ERR_IF( gkListRefInit( &ref2 ) != GK_LIST_SUCCESS );

		TEST_ERR_IF( gkListRefAppendListTwin( ref1, ref2 ) != GK_LIST_SUCCESS );

		TEST_ERR_IF( gkListRefFree( &ref1 ) != GK_LIST_SUCCESS );

		ref1 = ref2;
		ref2 = NULL;

		if ( i % 100 == 0 )
		{
			printf( "." ); fflush( stdout );
		}

		i += 1;
	}

	TEST_ERR_IF( gkListRefAppendListTwin( ref1, refHead ) != GK_LIST_SUCCESS );

	TEST_ERR_IF( gkListRefFree( &ref1 ) != GK_LIST_SUCCESS );

	printf( ":" ); fflush( stdout );

	TEST_ERR_IF( gkListRefFree( &refHead ) != GK_LIST_SUCCESS );

	return GK_LIST_SUCCESS;


	/* CLEANUP */
	cleanup:

	return rc;
}
