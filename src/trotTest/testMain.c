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

#include "stdio.h"
#include "time.h"
#include "string.h"


/******************************************************************************/
#define TEST_ERR_IF( x ) if ( (x) ) { printf( "test failed on line %d\n", __LINE__ ); fflush( stdout ); rc = -__LINE__; goto cleanup; }

#define SUCCESS printf( "%d\n", __LINE__ ); fflush( stdout );

#define MAGIC_NUMBER (NODE_SIZE * 3)

#define TEST_ADDING_INTS  1
#define TEST_ADDING_LISTS 2

#define TEST_REMOVE_SPECIFIC_KIND 1
#define TEST_REMOVE_GENERIC       2

#define TEST_POSITIVE_INDICES 1
#define TEST_NEGATIVE_INDICES 2

/* NOTE: Because inserting is different between positive and negative numbers,
         you need two defines. One that works with 'getters' or 'removers', and
         one that works with 'inserters'.
         Example: If your list has 5 items, and you want to get the fifth, you
         can get index 5 or index -1. However, if you want to add an element
         at the end of the list, you would add with index 6 or index -1. */
#define INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, count ) ( ( ( ( count ) + 1 ) * -1 ) + index )
#define INDEX_TO_NEGATIVE_VERSION_INSERT(        index, count ) ( ( ( ( count ) + 1 ) * -1 ) + index - 1 )

/* *** */
#define MEMORY_MANAGEMENT_REFS_COUNT 10


/******************************************************************************/
static int _getArgValue( int argc, char **argv, char *key, char **value );

/* helper functions */
static int checkList( gkListRef *lr );
static void printList( gkListRef *lr );
static int check( gkListRef *lr, INT_TYPE index, INT_TYPE valueToCheckAgainst );
static int addListWithValue( gkListRef *lr, INT_TYPE index, INT_TYPE value );

/* create functions */
static int createAllInts( gkListRef **lr, int count );
static int createAllLists( gkListRef **lr, int count );
static int createIntListAlternating( gkListRef **lr, int count );
static int createListIntAlternating( gkListRef **lr, int count );
static int createHalfIntHalfList( gkListRef **lr, int count );
static int createHalfListHalfInt( gkListRef **lr, int count );

static int (*createFunctions[])( gkListRef **, int ) =
	{
		createAllInts,
		createAllLists,
		createIntListAlternating,
		createListIntAlternating,
		createHalfIntHalfList,
		createHalfListHalfInt,
		NULL
	};

/* test functions */
static int testPrepend( gkListRef *lr, int intsOrLists, int removeSpecificOrGeneric, int positiveOrNegativeIndices );
static int testAppend( gkListRef *lr, int intsOrLists, int removeSpecificOrGeneric, int positiveOrNegativeIndices );
static int testAddToMiddle( gkListRef *lr, int intsOrLists, int removeSpecificOrGeneric, int positiveOrNegativeIndices );
static int testAddAtOddIndices( gkListRef *lr, int intsOrLists, int removeSpecificOrGeneric, int positiveOrNegativeIndices );

static int (*testFunctions[])( gkListRef *, int, int, int ) = 
	{
		testPrepend,
		testAppend,
		testAddToMiddle,
		testAddAtOddIndices,
		NULL
	};

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

static int testMemoryManagement();
static int testDeepList();

/******************************************************************************/
int main( int argc, char **argv )
{
	/* DATA */
	int rc = 0;

	char *valueSeed = 0;
	int seed = 1;

	int timeStart = 0;
	int timeEnd = 0;

	int **iArray = NULL;

	int count = 0;
	int i = 0;
	int j = 0;

	gkListRef *lr = NULL;
	gkListRef *lr2 = NULL;
	int kind = 0;
	INT_TYPE n = 0;

	/* CODE */
	if ( argc <= 1 )
	{
		printf( "Usage: trotTest [options]\n" );
		printf( "  -s <NUMBER>    Seed for random number generator\n" );
#if 0
		printf( "  -t <TEST>      Test to run\n" );
		printf( "                 Possible tests:\n" );
		printf( "                   primary\n" );
#endif
		printf( "\n" );

		return -1;
	}

	/* **************************************** */
	rc = _getArgValue( argc, argv, "-s", &valueSeed );
	if ( rc == 0 )
	{
		seed = atol( valueSeed );
	}
	else
	{
		seed = time( NULL );
	}

	printf( "Using seed: %d\n", seed ); fflush( stdout );
	srand( seed );

	/* **************************************** */
	timeStart = time( NULL );

	/* **************************************** */
	/* test that calloc sets pointers to NULL */
	iArray = (int **) gkCalloc( 10, sizeof( int * ) );
	TEST_ERR_IF( iArray == NULL );
	TEST_ERR_IF( iArray[ 5 ] != NULL );
	gkFree( iArray );

	/* **************************************** */
	/* test memory management */
	printf( "Testing memory management and garbage collection...\n" ); fflush( stdout );
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

	/* *** */
	TEST_ERR_IF( testDeepList() != 0 );

	printf( "\n" ); fflush( stdout );

	/* **************************************** */
	/* test functionality */
	printf( "Testing functionality..." ); fflush( stdout );
	count = 0;
	while ( count <= MAGIC_NUMBER )
	{
		printf( "\n%3d ", count );

		i = 0;
		while ( createFunctions[ i ] != NULL )
		{
			printf( ":" ); fflush( stdout );

			j = 0;
			while ( testFunctions[ j ] != NULL )
			{
				printf( "." ); fflush( stdout );

				/* test adding ints */
				TEST_ERR_IF( createFunctions[ i ]( &lr, count ) != 0 );
				TEST_ERR_IF( testFunctions[ j ]( lr, TEST_ADDING_INTS, TEST_REMOVE_SPECIFIC_KIND, TEST_POSITIVE_INDICES ) != 0 );
				gkListRefFree( &lr );

				TEST_ERR_IF( createFunctions[ i ]( &lr, count ) != 0 );
				TEST_ERR_IF( testFunctions[ j ]( lr, TEST_ADDING_INTS, TEST_REMOVE_SPECIFIC_KIND, TEST_NEGATIVE_INDICES ) != 0 );
				gkListRefFree( &lr );

				TEST_ERR_IF( createFunctions[ i ]( &lr, count ) != 0 );
				TEST_ERR_IF( testFunctions[ j ]( lr, TEST_ADDING_INTS, TEST_REMOVE_GENERIC, TEST_POSITIVE_INDICES ) != 0 );
				gkListRefFree( &lr );

				TEST_ERR_IF( createFunctions[ i ]( &lr, count ) != 0 );
				TEST_ERR_IF( testFunctions[ j ]( lr, TEST_ADDING_INTS, TEST_REMOVE_GENERIC, TEST_NEGATIVE_INDICES ) != 0 );
				gkListRefFree( &lr );

				/* test adding lists */
				TEST_ERR_IF( createFunctions[ i ]( &lr, count ) != 0 );
				TEST_ERR_IF( testFunctions[ j ]( lr, TEST_ADDING_LISTS, TEST_REMOVE_SPECIFIC_KIND, TEST_POSITIVE_INDICES ) != 0 );
				gkListRefFree( &lr );

				TEST_ERR_IF( createFunctions[ i ]( &lr, count ) != 0 );
				TEST_ERR_IF( testFunctions[ j ]( lr, TEST_ADDING_LISTS, TEST_REMOVE_SPECIFIC_KIND, TEST_NEGATIVE_INDICES ) != 0 );
				gkListRefFree( &lr );

				TEST_ERR_IF( createFunctions[ i ]( &lr, count ) != 0 );
				TEST_ERR_IF( testFunctions[ j ]( lr, TEST_ADDING_LISTS, TEST_REMOVE_GENERIC, TEST_POSITIVE_INDICES ) != 0 );
				gkListRefFree( &lr );

				TEST_ERR_IF( createFunctions[ i ]( &lr, count ) != 0 );
				TEST_ERR_IF( testFunctions[ j ]( lr, TEST_ADDING_LISTS, TEST_REMOVE_GENERIC, TEST_NEGATIVE_INDICES ) != 0 );
				gkListRefFree( &lr );

				j += 1;
			}

			i += 1;
		}

		if ( count == 0 )
		{
			count = 1;
		}
		else
		{
			count *= 2;
		}
	}

	printf( "\n" ); fflush( stdout );

	/* **************************************** */
	/* now lets test error cases to get 100% test coverage */
	/* test preconditions */
	#if ( TEST_PRECOND == 1 )
	printf( "Testing preconditions..." ); fflush( stdout );

	TEST_ERR_IF( gkListRefInit( NULL ) == GK_LIST_SUCCESS );
	TEST_ERR_IF( gkListRefInit( &lr ) != GK_LIST_SUCCESS );
	TEST_ERR_IF( gkListRefInit( &lr ) != GK_LIST_ERROR_PRECOND );

	TEST_ERR_IF( gkListRefTwin( NULL, lr ) != GK_LIST_ERROR_PRECOND );
	TEST_ERR_IF( gkListRefTwin( &lr, lr ) != GK_LIST_ERROR_PRECOND );
	TEST_ERR_IF( gkListRefTwin( &lr2, NULL ) != GK_LIST_ERROR_PRECOND );

	TEST_ERR_IF( gkListRefGetCount( NULL, &n ) != GK_LIST_ERROR_PRECOND );
	TEST_ERR_IF( gkListRefGetCount( lr, NULL ) != GK_LIST_ERROR_PRECOND );

	TEST_ERR_IF( gkListRefGetKind( NULL, 1, &kind ) != GK_LIST_ERROR_PRECOND );
	TEST_ERR_IF( gkListRefGetKind( lr, 1, NULL ) != GK_LIST_ERROR_PRECOND );

	TEST_ERR_IF( gkListRefAppendInt( NULL, 0 ) != GK_LIST_ERROR_PRECOND );

	TEST_ERR_IF( gkListRefAppendListTwin( NULL, lr ) != GK_LIST_ERROR_PRECOND );
	TEST_ERR_IF( gkListRefAppendListTwin( lr, NULL ) != GK_LIST_ERROR_PRECOND );

	TEST_ERR_IF( gkListRefInsertInt( NULL, 1, 1 ) != GK_LIST_ERROR_PRECOND );

	TEST_ERR_IF( gkListRefInsertListTwin( NULL, 1, lr ) != GK_LIST_ERROR_PRECOND );
	TEST_ERR_IF( gkListRefInsertListTwin( lr, 1, NULL ) != GK_LIST_ERROR_PRECOND );

	TEST_ERR_IF( gkListRefGetInt( NULL, 1, &n ) != GK_LIST_ERROR_PRECOND );
	TEST_ERR_IF( gkListRefGetInt( lr, 1, NULL ) != GK_LIST_ERROR_PRECOND );

	TEST_ERR_IF( gkListRefGetListTwin( NULL, 1, &lr2 ) != GK_LIST_ERROR_PRECOND );
	TEST_ERR_IF( gkListRefGetListTwin( lr, 1, NULL ) != GK_LIST_ERROR_PRECOND );
	TEST_ERR_IF( gkListRefGetListTwin( lr, 1, &lr ) != GK_LIST_ERROR_PRECOND );

	TEST_ERR_IF( gkListRefRemoveInt( NULL, 1, &n ) != GK_LIST_ERROR_PRECOND );
	TEST_ERR_IF( gkListRefRemoveInt( lr, 1, NULL ) != GK_LIST_ERROR_PRECOND );

	TEST_ERR_IF( gkListRefRemoveList( NULL, 1, &lr2 ) != GK_LIST_ERROR_PRECOND );
	TEST_ERR_IF( gkListRefRemoveList( lr, 1, NULL ) != GK_LIST_ERROR_PRECOND );
	TEST_ERR_IF( gkListRefRemoveList( lr, 1, &lr ) != GK_LIST_ERROR_PRECOND );

	TEST_ERR_IF( gkListRefRemove( NULL, 1 ) != GK_LIST_ERROR_PRECOND );

	gkListRefFree( &lr );

	printf( "\n" ); fflush( stdout );
	#endif

	/* **************************************** */
	/* test bad indices, bad types */
	printf( "Testing bad indices and bad types..." ); fflush( stdout );
	TEST_ERR_IF( createHalfIntHalfList( &lr, 10 ) != GK_LIST_SUCCESS );

	TEST_ERR_IF( gkListRefGetKind( lr, 0, &kind ) != GK_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( gkListRefGetKind( lr, 11, &kind ) != GK_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( gkListRefGetKind( lr, -11, &kind ) != GK_LIST_ERROR_BAD_INDEX );

	TEST_ERR_IF( gkListRefInsertInt( lr, 0, 1 ) != GK_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( gkListRefInsertInt( lr, 12, 1 ) != GK_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( gkListRefInsertInt( lr, -12, 1 ) != GK_LIST_ERROR_BAD_INDEX );
	
	TEST_ERR_IF( gkListRefInsertListTwin( lr, 0, lr ) != GK_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( gkListRefInsertListTwin( lr, 12, lr ) != GK_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( gkListRefInsertListTwin( lr, -12, lr ) != GK_LIST_ERROR_BAD_INDEX );

	TEST_ERR_IF( gkListRefGetInt( lr, 0, &n	) != GK_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( gkListRefGetInt( lr, 11, &n ) != GK_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( gkListRefGetInt( lr, -11, &n ) != GK_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( gkListRefGetInt( lr, 6, &n ) != GK_LIST_ERROR_WRONG_KIND );

	TEST_ERR_IF( gkListRefGetListTwin( lr, 0, &lr2 ) != GK_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( gkListRefGetListTwin( lr, 11, &lr2 ) != GK_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( gkListRefGetListTwin( lr, -11, &lr2 ) != GK_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( gkListRefGetListTwin( lr, 5, &lr2 ) != GK_LIST_ERROR_WRONG_KIND );

	TEST_ERR_IF( gkListRefRemoveInt( lr, 0, &n ) != GK_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( gkListRefRemoveInt( lr, 11, &n ) != GK_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( gkListRefRemoveInt( lr, -11, &n ) != GK_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( gkListRefRemoveInt( lr, 6, &n ) != GK_LIST_ERROR_WRONG_KIND );

	TEST_ERR_IF( gkListRefRemoveList( lr, 0, &lr2 ) != GK_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( gkListRefRemoveList( lr, 11, &lr2 ) != GK_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( gkListRefRemoveList( lr, -11, &lr2 ) != GK_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( gkListRefRemoveList( lr, 5, &lr2 ) != GK_LIST_ERROR_WRONG_KIND );

	TEST_ERR_IF( gkListRefRemove( lr, 0 ) != GK_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( gkListRefRemove( lr, 11 ) != GK_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( gkListRefRemove( lr, -11 ) != GK_LIST_ERROR_BAD_INDEX );

	gkListRefFree( &lr );

	printf( "\n" ); fflush( stdout );

	/* **************************************** */
	/* testing bad mallocs */
	printf( "Testing bad mallocs..." ); fflush( stdout );

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

	printf( "\n" ); fflush( stdout );

	/* **************************************** */
	timeEnd = time( NULL );

	printf( "\nTests took %d:%02d:%02d to complete\n",
		( timeEnd - timeStart ) / (60 * 60),
		( ( timeEnd - timeStart ) / 60 ) % 60,
		( timeEnd - timeStart ) % 60 );

	/* **************************************** */
	/* success! */
	printf( "\x1b[32;1mSUCCESS!\n\n\x1b[0m" );

	return 0;


	/* CLEANUP */
	cleanup:

	printf( "\x1b[31;1mFAILED AT %d (count=%d)(createFunction i=%d)(testFunction j=%d)\n\x1b[0m", rc, count, i, j );

	return rc;
}
/******************************************************************************/
static int _getArgValue( int argc, char **argv, char *key, char **value )
{
	/* DATA */
	int i = 0;


	/* CODE */
	while ( i < argc )
	{
		if ( strcmp( key, argv[ i ] ) == 0 )
		{
			i += 1;
			if ( i >= argc )
			{
				printf( "ERROR: Command line argument \"%s\" had no value.\n", key );
				return -1;
			}

			(*value) = argv[ i ];
			return 0;
		}

		i += 1;
	}

	return -2;
}

/******************************************************************************/
static int checkList( gkListRef *lr )
{
	/* DATA */
	int rc = 0;

	gkList *l = NULL;
	gkListNode *node = NULL;

	gkListRefListNode *refNode = NULL;

	int i = 0;

	int realCount = 0;

	int foundLr = 0;


	/* CODE */
	TEST_ERR_IF( lr == NULL );

	l = lr -> lPointsTo;
	TEST_ERR_IF( l == NULL );
	TEST_ERR_IF( l -> childrenCount < 0 );

	TEST_ERR_IF( l -> head == NULL );
	TEST_ERR_IF( l -> head -> kind != NODE_KIND_HEAD_OR_TAIL );
	TEST_ERR_IF( l -> head -> n != NULL );
	TEST_ERR_IF( l -> head -> l != NULL );
	TEST_ERR_IF( l -> head -> next == NULL );
	TEST_ERR_IF( l -> head -> prev == NULL );
	TEST_ERR_IF( l -> head -> prev != l -> head );

	TEST_ERR_IF( l -> tail == NULL );
	TEST_ERR_IF( l -> tail -> kind != NODE_KIND_HEAD_OR_TAIL );
	TEST_ERR_IF( l -> tail -> n != NULL );
	TEST_ERR_IF( l -> tail -> l != NULL );
	TEST_ERR_IF( l -> tail -> next == NULL );
	TEST_ERR_IF( l -> tail -> prev == NULL );
	TEST_ERR_IF( l -> tail -> next != l -> tail );

	/* *** */
	node = l -> head -> next;
	TEST_ERR_IF( node == NULL );

	while ( node != l -> tail )
	{
		TEST_ERR_IF( node -> next == NULL );
		TEST_ERR_IF( node -> next == node );
		TEST_ERR_IF( node -> prev == NULL );
		TEST_ERR_IF( node -> prev == node );
		TEST_ERR_IF( node -> next -> prev != node );
		TEST_ERR_IF( node -> prev -> next != node );

		TEST_ERR_IF( node -> count <= 0 );
		realCount += node -> count;

		TEST_ERR_IF(    node -> kind != NODE_KIND_INT
		        && node -> kind != NODE_KIND_LIST
		      );

		if ( node -> kind == NODE_KIND_INT )
		{
			TEST_ERR_IF( node -> n == NULL );
			TEST_ERR_IF( node -> l != NULL );
		}
		else
		{
			TEST_ERR_IF( node -> n != NULL );
			TEST_ERR_IF( node -> l == NULL );

			i = 0;
			while( i < node -> count )
			{
				TEST_ERR_IF( node -> l[ i ] == NULL );
				//TEST_ERR_IF( checkList( node -> l[ i ] ) != 0 );

				i += 1;
			}
			while ( i < NODE_SIZE )
			{
				TEST_ERR_IF( node -> l[ i ] != NULL );

				i += 1;
			}
		}


		node = node -> next;
	}

	TEST_ERR_IF( realCount != l -> childrenCount );

	/* *** */
	TEST_ERR_IF( l -> refListHead == NULL );
	TEST_ERR_IF( l -> refListHead -> next == NULL );
	TEST_ERR_IF( l -> refListHead -> prev == NULL );
	TEST_ERR_IF( l -> refListHead -> prev != l -> refListHead );
	TEST_ERR_IF( l -> refListHead -> count != 0 );

	TEST_ERR_IF( l -> refListTail == NULL );
	TEST_ERR_IF( l -> refListTail -> next == NULL );
	TEST_ERR_IF( l -> refListTail -> prev == NULL );
	TEST_ERR_IF( l -> refListTail -> next != l -> refListTail );
	TEST_ERR_IF( l -> refListTail -> count != 0 );

	refNode = l -> refListHead -> next;
	while ( refNode != l -> refListTail )
	{
		TEST_ERR_IF( refNode -> next == NULL );
		TEST_ERR_IF( refNode -> next == refNode );
		TEST_ERR_IF( refNode -> prev == NULL );
		TEST_ERR_IF( refNode -> prev == refNode );
		TEST_ERR_IF( refNode -> next -> prev != refNode );
		TEST_ERR_IF( refNode -> prev -> next != refNode );

		TEST_ERR_IF( refNode -> count <= 0 );

		TEST_ERR_IF( refNode -> r == NULL );

		i = 0;
		while ( i < refNode -> count )
		{
			TEST_ERR_IF( refNode -> r[ i ] == NULL );
			TEST_ERR_IF( refNode -> r[ i ] -> lPointsTo == NULL );
			TEST_ERR_IF( refNode -> r[ i ] -> lPointsTo != l );

			if ( refNode -> r[ i ] == lr )
			{
				foundLr = 1;
			}

			i += 1;
		}
		while ( i < REF_LIST_NODE_SIZE )
		{
			TEST_ERR_IF( refNode -> r[ i ] != NULL );

			i += 1;
		}

		refNode = refNode -> next;
	}

	TEST_ERR_IF( foundLr == 0 );

	return 0;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
static void printList( gkListRef *lr )
{
	/* DATA */
	gkList *l = lr -> lPointsTo;
	gkListNode *node = l -> head -> next;
	int i = 0;


	/* CODE */
	printf( "[ " );

	while ( node != l -> tail )
	{
		if ( node -> kind == NODE_KIND_INT )
		{
			printf( "I " );
			for ( i = 0; i < (node -> count); i += 1 )
			{
				printf( "%d ", node -> n[ i ] );
			}
		}
		else /* node -> kind == NODE_KIND_LIST */
		{
			printf( "L " );
			for ( i = 0; i < (node -> count); i += 1 )
			{
				printList( node -> l[ i ] );
			}
		}

		node = node -> next;
	}

	printf( "] " );

	return;
}

/******************************************************************************/
static int check( gkListRef *lr, INT_TYPE index, INT_TYPE valueToCheckAgainst )
{
	/* DATA */
	int rc = 0;

	int kind = 0;

	INT_TYPE valueInList = 0;

	gkListRef *subList = NULL;


	/* CODE */
	TEST_ERR_IF( gkListRefGetKind( lr, index, &kind ) != 0 );

	if ( kind == NODE_KIND_INT )
	{
		TEST_ERR_IF( gkListRefGetInt( lr, index, &valueInList ) != 0 );
		TEST_ERR_IF( valueInList != valueToCheckAgainst );
	}
	else if ( kind == NODE_KIND_LIST )
	{
		TEST_ERR_IF( gkListRefGetListTwin( lr, index, &subList ) != 0 );
		TEST_ERR_IF( gkListRefGetInt( subList, 1, &valueInList ) != 0 );
		TEST_ERR_IF( valueInList != valueToCheckAgainst );
	}
	else
	{
		TEST_ERR_IF( 1 );
	}


	/* CLEANUP */
	cleanup:

	gkListRefFree( &subList );

	return rc;
}

/******************************************************************************/
static int addListWithValue( gkListRef *lr, INT_TYPE index, INT_TYPE value )
{
	/* DATA */
	int rc = 0;

	gkListRef *newList = NULL;


	/* CODE */
	TEST_ERR_IF( gkListRefInit( &newList ) != 0 );
	TEST_ERR_IF( gkListRefAppendInt( newList, value ) != 0 );
	TEST_ERR_IF( gkListRefInsertListTwin( lr, index, newList ) != 0 );


	/* CLEANUP */
	cleanup:

	gkListRefFree( &newList );

	return rc;
}

/******************************************************************************/
static int createAllInts( gkListRef **lr, int count )
{
	/* DATA */
	int rc = 0;

	gkListRef *newList = NULL;

	INT_TYPE i = 1;

	/* CODE */
	TEST_ERR_IF( gkListRefInit( &newList ) != 0 );

	i = 1;
	while ( i <= count )
	{
		TEST_ERR_IF( gkListRefAppendInt( newList, i ) != 0 );

		i += 1;
	}

	/* check list */
	TEST_ERR_IF( checkList( newList ) != 0 );

	i = 1;
	while ( i <= count )
	{
		TEST_ERR_IF( check( newList, i, i ) != 0 );
		i += 1;
	}

	/* give back */
	(*lr) = newList;
	newList = NULL;


	/* CLEANUP */
	cleanup:

	gkListRefFree( &newList );

	return rc;
}

/******************************************************************************/
static int createAllLists( gkListRef **lr, int count )
{
	/* DATA */
	int rc = 0;

	gkListRef *newList = NULL;

	INT_TYPE i = 1;

	/* CODE */
	TEST_ERR_IF( gkListRefInit( &newList ) != 0 );

	i = 1;
	while ( i <= count )
	{
		TEST_ERR_IF( addListWithValue( newList, i, i ) != 0 );

		i += 1;
	}

	/* check list */
	TEST_ERR_IF( checkList( newList ) != 0 );

	i = 1;
	while ( i <= count )
	{
		TEST_ERR_IF( check( newList, i, i ) != 0 );
		i += 1;
	}

	/* give back */
	(*lr) = newList;
	newList = NULL;


	/* CLEANUP */
	cleanup:

	gkListRefFree( &newList );

	return rc;
}

/******************************************************************************/
static int createIntListAlternating( gkListRef **lr, int count )
{
	/* DATA */
	int rc = 0;

	gkListRef *newList = NULL;

	INT_TYPE i = 1;

	/* CODE */
	TEST_ERR_IF( gkListRefInit( &newList ) != 0 );

	i = 1;
	while ( i <= count )
	{
		if ( i % 2 == 1 )
		{
			TEST_ERR_IF( gkListRefAppendInt( newList, i ) != 0 );
		}
		else
		{
			TEST_ERR_IF( addListWithValue( newList, i, i ) != 0 );
		}

		i += 1;
	}

	/* check list */
	TEST_ERR_IF( checkList( newList ) != 0 );

	i = 1;
	while ( i <= count )
	{
		TEST_ERR_IF( check( newList, i, i ) != 0 );
		i += 1;
	}

	/* give back */
	(*lr) = newList;
	newList = NULL;


	/* CLEANUP */
	cleanup:

	gkListRefFree( &newList );

	return rc;
}

/******************************************************************************/
static int createListIntAlternating( gkListRef **lr, int count )
{
	/* DATA */
	int rc = 0;

	gkListRef *newList = NULL;

	INT_TYPE i = 1;

	/* CODE */
	TEST_ERR_IF( gkListRefInit( &newList ) != 0 );

	i = 1;
	while ( i <= count )
	{
		if ( i % 2 == 1 )
		{
			TEST_ERR_IF( addListWithValue( newList, i, i ) != 0 );
		}
		else
		{
			TEST_ERR_IF( gkListRefAppendInt( newList, i ) != 0 );
		}

		i += 1;
	}

	/* check list */
	TEST_ERR_IF( checkList( newList ) != 0 );

	i = 1;
	while ( i <= count )
	{
		TEST_ERR_IF( check( newList, i, i ) != 0 );
		i += 1;
	}

	/* give back */
	(*lr) = newList;
	newList = NULL;


	/* CLEANUP */
	cleanup:

	gkListRefFree( &newList );

	return rc;
}

/******************************************************************************/
static int createHalfIntHalfList( gkListRef **lr, int count )
{
	/* DATA */
	int rc = 0;

	gkListRef *newList = NULL;

	INT_TYPE i = 1;

	/* CODE */
	TEST_ERR_IF( gkListRefInit( &newList ) != 0 );

	i = 1;
	while ( i <= ( count / 2 ) )
	{
		TEST_ERR_IF( gkListRefAppendInt( newList, i ) != 0 );

		i += 1;
	}
	while ( i <= count )
	{
		TEST_ERR_IF( addListWithValue( newList, i, i ) != 0 );

		i += 1;
	}


	/* check list */
	TEST_ERR_IF( checkList( newList ) != 0 );

	i = 1;
	while ( i <= count )
	{
		TEST_ERR_IF( check( newList, i, i ) != 0 );
		i += 1;
	}

	/* give back */
	(*lr) = newList;
	newList = NULL;


	/* CLEANUP */
	cleanup:

	gkListRefFree( &newList );

	return rc;
}

/******************************************************************************/
static int createHalfListHalfInt( gkListRef **lr, int count )
{
	/* DATA */
	int rc = 0;

	gkListRef *newList = NULL;

	INT_TYPE i = 1;

	/* CODE */
	TEST_ERR_IF( gkListRefInit( &newList ) != 0 );

	i = 1;
	while ( i <= ( count / 2 ) )
	{
		TEST_ERR_IF( addListWithValue( newList, i, i ) != 0 );

		i += 1;
	}
	while ( i <= count )
	{
		TEST_ERR_IF( gkListRefAppendInt( newList, i ) != 0 );

		i += 1;
	}


	/* check list */
	TEST_ERR_IF( checkList( newList ) != 0 );

	i = 1;
	while ( i <= count )
	{
		TEST_ERR_IF( check( newList, i, i ) != 0 );
		i += 1;
	}

	/* give back */
	(*lr) = newList;
	newList = NULL;


	/* CLEANUP */
	cleanup:

	gkListRefFree( &newList );

	return rc;
}



/******************************************************************************/
static int testPrepend( gkListRef *lr, int intsOrLists, int removeSpecificOrGeneric, int positiveOrNegativeIndices )
{
	/* DATA */
	int rc = 0;

	INT_TYPE countAtStart = 0;
	INT_TYPE countAtEnd = 0;
	INT_TYPE countAdded = 0;

	INT_TYPE newNumber = 0;
	INT_TYPE addingAtIndex = 0;
	INT_TYPE addingAtIndexB = 0; /* this may be the same, or it may be the negative version */

	INT_TYPE index = 0;

	INT_TYPE testNew = 0;
	INT_TYPE testOriginal = 0;

	INT_TYPE removedN = 0;
	gkListRef *removedL = NULL;


	/* CODE */
	TEST_ERR_IF( gkListRefGetCount( lr, &countAtStart ) != 0 );

	/* test prepending */
	addingAtIndex = 0;
	newNumber = 1000;
	while ( countAdded <= MAGIC_NUMBER )
	{
		/* insert */
		addingAtIndex += 1;
		newNumber += 1;

		if ( positiveOrNegativeIndices == TEST_POSITIVE_INDICES )
		{
			addingAtIndexB = addingAtIndex;
		}
		else if ( positiveOrNegativeIndices == TEST_NEGATIVE_INDICES )
		{
			addingAtIndexB = INDEX_TO_NEGATIVE_VERSION_INSERT( addingAtIndex, countAdded + countAtStart );
		}

		if ( intsOrLists == TEST_ADDING_INTS )
		{
			TEST_ERR_IF( gkListRefInsertInt( lr, addingAtIndexB, newNumber ) != 0 );
		}
		else if ( intsOrLists == TEST_ADDING_LISTS )
		{
			TEST_ERR_IF( addListWithValue( lr, addingAtIndexB, newNumber ) != 0 );
		}
		else
		{
			TEST_ERR_IF( 1 );
		}

		countAdded += 1;

		/* check */
		TEST_ERR_IF( checkList( lr ) != 0 );

		index = 1;
		testNew = 1000;
		testOriginal = 0;
		while ( index <= countAdded )
		{
			testNew += 1;

			TEST_ERR_IF( check( lr, index, testNew ) != 0 );
			TEST_ERR_IF( check( lr, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testNew ) != 0 );

			index += 1;
		}
		while ( index <= (countAtStart + countAdded) )
		{
			testOriginal += 1;

			TEST_ERR_IF( check( lr, index, testOriginal ) != 0 );
			TEST_ERR_IF( check( lr, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testOriginal ) != 0 );

			index += 1;
		}

		TEST_ERR_IF( testOriginal != countAtStart );
		TEST_ERR_IF( (testNew - 1000) != countAdded );
	}

	/* test removing */
	while ( countAdded > 0 )
	{
		/* remove */
		if ( positiveOrNegativeIndices == TEST_POSITIVE_INDICES )
		{
			addingAtIndexB = addingAtIndex;
		}
		else if ( positiveOrNegativeIndices == TEST_NEGATIVE_INDICES )
		{
			addingAtIndexB = INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( addingAtIndex, countAdded + countAtStart );
		}

		if ( removeSpecificOrGeneric == TEST_REMOVE_GENERIC )
		{
			TEST_ERR_IF( gkListRefRemove( lr, addingAtIndexB ) != 0 );
		}
		else if ( removeSpecificOrGeneric == TEST_REMOVE_SPECIFIC_KIND )
		{
			if ( intsOrLists == TEST_ADDING_INTS )
			{
				TEST_ERR_IF( gkListRefRemoveInt( lr, addingAtIndexB, &removedN ) != 0 );
				TEST_ERR_IF( removedN != newNumber );
			}
			else if ( intsOrLists == TEST_ADDING_LISTS )
			{
				TEST_ERR_IF( gkListRefRemoveList( lr, addingAtIndexB, &removedL ) != 0 );
				TEST_ERR_IF( gkListRefGetInt( removedL, 1, &removedN ) != 0 );
				TEST_ERR_IF( removedN != newNumber );

				gkListRefFree( &removedL );
			}
			else
			{
				TEST_ERR_IF( 1 );
			}
		}
		else
		{
			TEST_ERR_IF( 1 );
		}

		addingAtIndex -= 1;
		newNumber -= 1;
		countAdded -= 1;

		/* check */
		TEST_ERR_IF( checkList( lr ) != 0 );

		index = 1;
		testNew = 1000;
		testOriginal = 0;
		while ( index <= countAdded )
		{
			testNew += 1;

			TEST_ERR_IF( check( lr, index, testNew ) != 0 );
			TEST_ERR_IF( check( lr, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testNew ) != 0 );

			index += 1;
		}
		while ( index <= (countAtStart + countAdded) )
		{
			testOriginal += 1;

			TEST_ERR_IF( check( lr, index, testOriginal ) != 0 );
			TEST_ERR_IF( check( lr, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testOriginal ) != 0 );

			index += 1;
		}

		TEST_ERR_IF( testOriginal != countAtStart );
		TEST_ERR_IF( (testNew - 1000) != countAdded );
	}

	TEST_ERR_IF( gkListRefGetCount( lr, &countAtEnd ) != 0 );
	TEST_ERR_IF( countAtEnd != countAtStart );

	return 0;
	

	/* CLEANUP */
	cleanup:

	printList( lr );
	printf( "\x1b[31;1mFAILED AT %d testNew:%d testOriginal:%d index:%d addingAtIndex:%d\n\x1b[0m", rc, testNew, testOriginal, index, addingAtIndex );

	return rc;
}

/******************************************************************************/
static int testAppend( gkListRef *lr, int intsOrLists, int removeSpecificOrGeneric, int positiveOrNegativeIndices )
{
	/* DATA */
	int rc = 0;

	INT_TYPE countAtStart = 0;
	INT_TYPE countAtEnd = 0;
	INT_TYPE countAdded = 0;

	INT_TYPE newNumber = 0;
	INT_TYPE addingAtIndex = 0;
	INT_TYPE addingAtIndexB = 0; /* this may be the same, or it may be the negative version */

	INT_TYPE index = 0;

	INT_TYPE testNew = 0;
	INT_TYPE testOriginal = 0;

	INT_TYPE removedN = 0;
	gkListRef *removedL = NULL;


	/* CODE */
	TEST_ERR_IF( gkListRefGetCount( lr, &countAtStart ) != 0 );

	/* test appending */
	addingAtIndex = countAtStart;
	newNumber = 1000;
	while ( countAdded <= MAGIC_NUMBER )
	{
		/* insert */
		addingAtIndex += 1;
		newNumber += 1;

		if ( positiveOrNegativeIndices == TEST_POSITIVE_INDICES )
		{
			addingAtIndexB = addingAtIndex;
		}
		else if ( positiveOrNegativeIndices == TEST_NEGATIVE_INDICES )
		{
			addingAtIndexB = INDEX_TO_NEGATIVE_VERSION_INSERT( addingAtIndex, countAdded + countAtStart );
		}

		if ( intsOrLists == TEST_ADDING_INTS )
		{
			TEST_ERR_IF( gkListRefInsertInt( lr, addingAtIndexB, newNumber ) != 0 );
		}
		else if ( intsOrLists == TEST_ADDING_LISTS )
		{
			TEST_ERR_IF( addListWithValue( lr, addingAtIndexB, newNumber ) != 0 );
		}
		else
		{
			TEST_ERR_IF( 1 );
		}

		countAdded += 1;

		/* check */
		TEST_ERR_IF( checkList( lr ) != 0 );

		index = 1;
		testNew = 1000;
		testOriginal = 0;
		while ( index <= countAtStart )
		{
			testOriginal += 1;

			TEST_ERR_IF( check( lr, index, testOriginal ) != 0 );
			TEST_ERR_IF( check( lr, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testOriginal ) != 0 );

			index += 1;
		}
		while ( index <= (countAtStart + countAdded) )
		{
			testNew += 1;

			TEST_ERR_IF( check( lr, index, testNew ) != 0 );
			TEST_ERR_IF( check( lr, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testNew ) != 0 );

			index += 1;
		}

		TEST_ERR_IF( testOriginal != countAtStart );
		TEST_ERR_IF( (testNew - 1000) != countAdded );
	}

	/* test removing */
	while ( countAdded > 0 )
	{
		/* remove */
		if ( positiveOrNegativeIndices == TEST_POSITIVE_INDICES )
		{
			addingAtIndexB = addingAtIndex;
		}
		else if ( positiveOrNegativeIndices == TEST_NEGATIVE_INDICES )
		{
			addingAtIndexB = INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( addingAtIndex, countAdded + countAtStart );
		}

		if ( removeSpecificOrGeneric == TEST_REMOVE_GENERIC )
		{
			TEST_ERR_IF( gkListRefRemove( lr, addingAtIndexB ) != 0 );
		}
		else if ( removeSpecificOrGeneric == TEST_REMOVE_SPECIFIC_KIND )
		{
			if ( intsOrLists == TEST_ADDING_INTS )
			{
				TEST_ERR_IF( gkListRefRemoveInt( lr, addingAtIndexB, &removedN ) != 0 );
				TEST_ERR_IF( removedN != newNumber );
			}
			else if ( intsOrLists == TEST_ADDING_LISTS )
			{
				TEST_ERR_IF( gkListRefRemoveList( lr, addingAtIndexB, &removedL ) != 0 );
				TEST_ERR_IF( gkListRefGetInt( removedL, 1, &removedN ) != 0 );
				TEST_ERR_IF( removedN != newNumber );

				gkListRefFree( &removedL );
			}
			else
			{
				TEST_ERR_IF( 1 );
			}
		}
		else
		{
			TEST_ERR_IF( 1 );
		}

		addingAtIndex -= 1;
		newNumber -= 1;
		countAdded -= 1;

		/* check */
		TEST_ERR_IF( checkList( lr ) != 0 );

		index = 1;
		testNew = 1000;
		testOriginal = 0;
		while ( index <= countAtStart )
		{
			testOriginal += 1;

			TEST_ERR_IF( check( lr, index, testOriginal ) != 0 );
			TEST_ERR_IF( check( lr, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testOriginal ) != 0 );

			index += 1;
		}
		while ( index <= (countAtStart + countAdded) )
		{
			testNew += 1;

			TEST_ERR_IF( check( lr, index, testNew ) != 0 );
			TEST_ERR_IF( check( lr, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testNew ) != 0 );

			index += 1;
		}

		TEST_ERR_IF( testOriginal != countAtStart );
		TEST_ERR_IF( (testNew - 1000) != countAdded );
	}

	TEST_ERR_IF( gkListRefGetCount( lr, &countAtEnd ) != 0 );
	TEST_ERR_IF( countAtEnd != countAtStart );

	return 0;
	

	/* CLEANUP */
	cleanup:

	printList( lr );
	printf( "\x1b[31;1mFAILED AT %d testNew:%d testOriginal:%d index:%d addingAtIndex:%d\n\x1b[0m", rc, testNew, testOriginal, index, addingAtIndex );

	return rc;
}

/******************************************************************************/
static int testAddToMiddle( gkListRef *lr, int intsOrLists, int removeSpecificOrGeneric, int positiveOrNegativeIndices )
{
	/* DATA */
	int rc = 0;

	INT_TYPE countAtStart = 0;
	INT_TYPE countAtEnd = 0;
	INT_TYPE countAdded = 0;

	INT_TYPE newNumber = 0;
	INT_TYPE addingAtIndex = 0;
	INT_TYPE addingAtIndexB = 0; /* this may be the same, or it may be the negative version */

	INT_TYPE index = 0;

	INT_TYPE testNew = 0;
	INT_TYPE testOriginal = 0;

	INT_TYPE removedN = 0;
	gkListRef *removedL = NULL;

	INT_TYPE startedAddingAtIndex = 0;


	/* CODE */
	TEST_ERR_IF( gkListRefGetCount( lr, &countAtStart ) != 0 );

	/* test adding to middle */
	startedAddingAtIndex = (countAtStart / 2 );
	if ( startedAddingAtIndex < 1 )
	{
		startedAddingAtIndex = 1;
	}

	addingAtIndex = startedAddingAtIndex - 1;
	newNumber = 1000;
	while ( countAdded <= MAGIC_NUMBER )
	{
		/* insert */
		addingAtIndex += 1;
		newNumber += 1;

		if ( positiveOrNegativeIndices == TEST_POSITIVE_INDICES )
		{
			addingAtIndexB = addingAtIndex;
		}
		else if ( positiveOrNegativeIndices == TEST_NEGATIVE_INDICES )
		{
			addingAtIndexB = INDEX_TO_NEGATIVE_VERSION_INSERT( addingAtIndex, countAdded + countAtStart );
		}

		if ( intsOrLists == TEST_ADDING_INTS )
		{
			TEST_ERR_IF( gkListRefInsertInt( lr, addingAtIndexB, newNumber ) != 0 );
		}
		else if ( intsOrLists == TEST_ADDING_LISTS )
		{
			TEST_ERR_IF( addListWithValue( lr, addingAtIndexB, newNumber ) != 0 );
		}
		else
		{
			TEST_ERR_IF( 1 );
		}

		countAdded += 1;

		/* check */
		TEST_ERR_IF( checkList( lr ) != 0 );

		index = 1;
		testNew = 1000;
		testOriginal = 0;
		while ( index < startedAddingAtIndex )
		{
			testOriginal += 1;

			TEST_ERR_IF( check( lr, index, testOriginal ) != 0 );
			TEST_ERR_IF( check( lr, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testOriginal ) != 0 );

			index += 1;
		}
		while ( index < startedAddingAtIndex + countAdded )
		{
			testNew += 1;

			TEST_ERR_IF( check( lr, index, testNew ) != 0 );
			TEST_ERR_IF( check( lr, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testNew ) != 0 );

			index += 1;
		}
		while ( index <= (countAtStart + countAdded) )
		{
			testOriginal += 1;

			TEST_ERR_IF( check( lr, index, testOriginal ) != 0 );
			TEST_ERR_IF( check( lr, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testOriginal ) != 0 );

			index += 1;
		}

		TEST_ERR_IF( testOriginal != countAtStart );
		TEST_ERR_IF( (testNew - 1000) != countAdded );
	}

	/* test removing */
	while ( countAdded > 0 )
	{
		/* remove */
		if ( positiveOrNegativeIndices == TEST_POSITIVE_INDICES )
		{
			addingAtIndexB = addingAtIndex;
		}
		else if ( positiveOrNegativeIndices == TEST_NEGATIVE_INDICES )
		{
			addingAtIndexB = INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( addingAtIndex, countAdded + countAtStart );
		}

		if ( removeSpecificOrGeneric == TEST_REMOVE_GENERIC )
		{
			TEST_ERR_IF( gkListRefRemove( lr, addingAtIndexB ) != 0 );
		}
		else if ( removeSpecificOrGeneric == TEST_REMOVE_SPECIFIC_KIND )
		{
			if ( intsOrLists == TEST_ADDING_INTS )
			{
				TEST_ERR_IF( gkListRefRemoveInt( lr, addingAtIndexB, &removedN ) != 0 );
				TEST_ERR_IF( removedN != newNumber );
			}
			else if ( intsOrLists == TEST_ADDING_LISTS )
			{
				TEST_ERR_IF( gkListRefRemoveList( lr, addingAtIndexB, &removedL ) != 0 );
				TEST_ERR_IF( gkListRefGetInt( removedL, 1, &removedN ) != 0 );
				TEST_ERR_IF( removedN != newNumber );

				gkListRefFree( &removedL );
			}
			else
			{
				TEST_ERR_IF( 1 );
			}
		}
		else
		{
			TEST_ERR_IF( 1 );
		}

		addingAtIndex -= 1;
		newNumber -= 1;
		countAdded -= 1;

		/* check */
		TEST_ERR_IF( checkList( lr ) != 0 );

		index = 1;
		testNew = 1000;
		testOriginal = 0;
		while ( index < startedAddingAtIndex )
		{
			testOriginal += 1;

			TEST_ERR_IF( check( lr, index, testOriginal ) != 0 );
			TEST_ERR_IF( check( lr, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testOriginal ) != 0 );

			index += 1;
		}
		while ( index < startedAddingAtIndex + countAdded )
		{
			testNew += 1;

			TEST_ERR_IF( check( lr, index, testNew ) != 0 );
			TEST_ERR_IF( check( lr, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testNew ) != 0 );

			index += 1;
		}
		while ( index <= (countAtStart + countAdded) )
		{
			testOriginal += 1;

			TEST_ERR_IF( check( lr, index, testOriginal ) != 0 );
			TEST_ERR_IF( check( lr, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testOriginal ) != 0 );

			index += 1;
		}

		TEST_ERR_IF( testOriginal != countAtStart );
		TEST_ERR_IF( (testNew - 1000) != countAdded );
	}

	TEST_ERR_IF( gkListRefGetCount( lr, &countAtEnd ) != 0 );
	TEST_ERR_IF( countAtEnd != countAtStart );

	return 0;
	

	/* CLEANUP */
	cleanup:

	printList( lr );
	printf( "\x1b[31;1mFAILED AT %d testNew:%d testOriginal:%d index:%d addingAtIndex:%d\n\x1b[0m", rc, testNew, testOriginal, index, addingAtIndex );

	return rc;
}

/******************************************************************************/
static int testAddAtOddIndices( gkListRef *lr, int intsOrLists, int removeSpecificOrGeneric, int positiveOrNegativeIndices )
{
	/* DATA */
	int rc = 0;

	INT_TYPE countAtStart = 0;
	INT_TYPE countAtEnd = 0;
	INT_TYPE countAdded = 0;

	INT_TYPE newNumber = 0;
	INT_TYPE addingAtIndex = 0;
	INT_TYPE addingAtIndexB = 0; /* this may be the same, or it may be the negative version */

	INT_TYPE index = 0;

	INT_TYPE testNew = 0;
	INT_TYPE testOriginal = 0;

	INT_TYPE removedN = 0;
	gkListRef *removedL = NULL;


	/* CODE */
	TEST_ERR_IF( gkListRefGetCount( lr, &countAtStart ) != 0 );

	/* test prepending */
	addingAtIndex = -1;
	newNumber = 1000;
	while ( countAdded <= MAGIC_NUMBER )
	{
		/* insert */
		if ( addingAtIndex + 2 > countAtStart + countAdded + 1 )
		{
			break;
		}

		addingAtIndex += 2;
		newNumber += 1;

		if ( positiveOrNegativeIndices == TEST_POSITIVE_INDICES )
		{
			addingAtIndexB = addingAtIndex;
		}
		else if ( positiveOrNegativeIndices == TEST_NEGATIVE_INDICES )
		{
			addingAtIndexB = INDEX_TO_NEGATIVE_VERSION_INSERT( addingAtIndex, countAdded + countAtStart );
		}

		if ( intsOrLists == TEST_ADDING_INTS )
		{
			TEST_ERR_IF( gkListRefInsertInt( lr, addingAtIndexB, newNumber ) != 0 );
		}
		else if ( intsOrLists == TEST_ADDING_LISTS )
		{
			TEST_ERR_IF( addListWithValue( lr, addingAtIndexB, newNumber ) != 0 );
		}
		else
		{
			TEST_ERR_IF( 1 );
		}

		countAdded += 1;

		/* check */
		TEST_ERR_IF( checkList( lr ) != 0 );

		index = 1;
		testNew = 1000;
		testOriginal = 0;
		while ( index <= ( ( countAdded * 2 - 1 ) ) )
		{
			testNew += 1;

			TEST_ERR_IF( check( lr, index, testNew ) != 0 );
			TEST_ERR_IF( check( lr, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testNew ) != 0 );

			index += 1;

			if ( index > ( ( countAdded * 2 ) - 1 ) )
			{
				break;
			}

			testOriginal += 1;

			TEST_ERR_IF( check( lr, index, testOriginal ) != 0 );
			TEST_ERR_IF( check( lr, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testOriginal ) != 0 );

			index += 1;
		}
		while ( index <= countAtStart + countAdded )
		{
			testOriginal += 1;

			TEST_ERR_IF( check( lr, index, testOriginal ) != 0 );
			TEST_ERR_IF( check( lr, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testOriginal ) != 0 );

			index += 1;
		}

		TEST_ERR_IF( testOriginal != countAtStart );
		TEST_ERR_IF( (testNew - 1000) != countAdded );
	}

	/* test removing */
	while ( countAdded > 0 )
	{
		/* remove */
		if ( positiveOrNegativeIndices == TEST_POSITIVE_INDICES )
		{
			addingAtIndexB = addingAtIndex;
		}
		else if ( positiveOrNegativeIndices == TEST_NEGATIVE_INDICES )
		{
			addingAtIndexB = INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( addingAtIndex, countAdded + countAtStart );
		}

		if ( removeSpecificOrGeneric == TEST_REMOVE_GENERIC )
		{
			TEST_ERR_IF( gkListRefRemove( lr, addingAtIndexB ) != 0 );
		}
		else if ( removeSpecificOrGeneric == TEST_REMOVE_SPECIFIC_KIND )
		{
			if ( intsOrLists == TEST_ADDING_INTS )
			{
				TEST_ERR_IF( gkListRefRemoveInt( lr, addingAtIndexB, &removedN ) != 0 );
				TEST_ERR_IF( removedN != newNumber );
			}
			else if ( intsOrLists == TEST_ADDING_LISTS )
			{
				TEST_ERR_IF( gkListRefRemoveList( lr, addingAtIndexB, &removedL ) != 0 );
				TEST_ERR_IF( gkListRefGetInt( removedL, 1, &removedN ) != 0 );
				TEST_ERR_IF( removedN != newNumber );

				gkListRefFree( &removedL );
			}
			else
			{
				TEST_ERR_IF( 1 );
			}
		}
		else
		{
			TEST_ERR_IF( 1 );
		}

		addingAtIndex -= 2;
		newNumber -= 1;
		countAdded -= 1;

		/* check */
		TEST_ERR_IF( checkList( lr ) != 0 );

		index = 1;
		testNew = 1000;
		testOriginal = 0;
		while ( index <= ( ( countAdded * 2 - 1 ) ) )
		{
			testNew += 1;

			TEST_ERR_IF( check( lr, index, testNew ) != 0 );
			TEST_ERR_IF( check( lr, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testNew ) != 0 );

			index += 1;

			if ( index > ( ( countAdded * 2 ) - 1 ) )
			{
				break;
			}

			testOriginal += 1;

			TEST_ERR_IF( check( lr, index, testOriginal ) != 0 );
			TEST_ERR_IF( check( lr, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testOriginal ) != 0 );

			index += 1;
		}
		while ( index <= countAtStart + countAdded )
		{
			testOriginal += 1;

			TEST_ERR_IF( check( lr, index, testOriginal ) != 0 );
			TEST_ERR_IF( check( lr, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testOriginal ) != 0 );

			index += 1;
		}

		TEST_ERR_IF( testOriginal != countAtStart );
		TEST_ERR_IF( (testNew - 1000) != countAdded );
	}

	TEST_ERR_IF( gkListRefGetCount( lr, &countAtEnd ) != 0 );
	TEST_ERR_IF( countAtEnd != countAtStart );

	return 0;
	

	/* CLEANUP */
	cleanup:

	printList( lr );
	printf( "\x1b[31;1mFAILED AT %d testNew:%d testOriginal:%d index:%d addingAtIndex:%d\n\x1b[0m", rc, testNew, testOriginal, index, addingAtIndex );

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

