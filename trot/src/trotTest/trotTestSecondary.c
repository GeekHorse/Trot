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
#define MAGIC_NUMBER (NODE_SIZE * 3)

/******************************************************************************/
/* helper functions */
static int compare( trotListRef *lr1, trotListRef *lr2, int shouldBeEqual );

/******************************************************************************/
/* create functions */
static int (*createFunctions[])( trotListRef **, int ) =
	{
		createAllInts,
		createAllLists,
		createIntListAlternating,
		createListIntAlternating,
		createHalfIntHalfList,
		createHalfListHalfInt,

/*		createSelfRefs, */

		NULL
	};

/******************************************************************************/
/* test functions */
static int testCopyCompare( int (*createFunction)( trotListRef **, int ), int size );
static int testEnlistDelist( int (*createFunction)( trotListRef **, int ), int size );
static int testSpans( int (*createFunction)( trotListRef **, int ), int size );

static int (*testFunctions[])( int (*)( trotListRef **, int ), int ) = 
	{
		testCopyCompare,
		testEnlistDelist,
		testSpans,
		NULL
	};

/******************************************************************************/
int testSecondaryFunctionality()
{
	/* DATA */
	int rc = 0;

	int count = 0;
	int i = 0;
	int j = 0;


	/* CODE */
	printf( "Testing secondary functionality..." ); fflush( stdout );
	count = 1;
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

				TEST_ERR_IF( testFunctions[ j ]( createFunctions[ i ], count ) != 0 );

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

	return 0;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
static int compare( trotListRef *lr1, trotListRef *lr2, int shouldBeEqual )
{
	/* DATA */
	int rc = 0;

	TROT_LIST_COMPARE_RESULT compareResult1 = TROT_LIST_COMPARE_EQUAL;
	TROT_LIST_COMPARE_RESULT compareResult2 = TROT_LIST_COMPARE_EQUAL;


	/* CODE */
	if ( shouldBeEqual )
	{
		TEST_ERR_IF( trotListRefCompare( lr1, lr2, &compareResult1 ) != 0 );
		TEST_ERR_IF( compareResult1 != TROT_LIST_COMPARE_EQUAL );

		TEST_ERR_IF( trotListRefCompare( lr2, lr1, &compareResult1 ) != 0 );
		TEST_ERR_IF( compareResult1 != TROT_LIST_COMPARE_EQUAL );
	}
	else
	{
		TEST_ERR_IF( trotListRefCompare( lr1, lr2, &compareResult1 ) != 0 );
		TEST_ERR_IF( compareResult1 == TROT_LIST_COMPARE_EQUAL );

		TEST_ERR_IF( trotListRefCompare( lr2, lr1, &compareResult2 ) != 0 );
		TEST_ERR_IF( compareResult2 == TROT_LIST_COMPARE_EQUAL );

		if ( compareResult1 == TROT_LIST_COMPARE_LESS_THAN )
		{
			TEST_ERR_IF( compareResult2 != TROT_LIST_COMPARE_GREATER_THAN );
		}
		else
		{
			TEST_ERR_IF( compareResult2 != TROT_LIST_COMPARE_LESS_THAN );
		}
	}

	return 0;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
static int testCopyCompare( int (*createFunction)( trotListRef **, int ), int size )
{
	/* DATA */
	int rc = 0;

	trotListRef *lr1 = NULL;
	trotListRef *lr1Copy = NULL;

	trotListRef *lr2 = NULL;

	INT_TYPE index = 0;


	/* CODE */
	/* create l1 */
	TEST_ERR_IF( (*createFunction)( &lr1, 0 ) != 0 );

	TEST_ERR_IF( compare( lr1, lr1, 1 ) != 0 );

	/* create copy */
	TEST_ERR_IF( trotListRefCopy( &lr1Copy, lr1 ) != 0 );

	TEST_ERR_IF( compare( lr1, lr1Copy, 1 ) != 0 );

	/* *** */
	TEST_ERR_IF( (*createFunction)( &lr2, 0 ) != 0 );

	TEST_ERR_IF( trotListRefAppendListTwin( lr1, lr2 ) != 0 );
	TEST_ERR_IF( trotListRefAppendListTwin( lr2, lr1 ) != 0 );

	TEST_ERR_IF( compare( lr1, lr2, 1 ) != 0 );

	/* free lists */
	trotListRefFree( &lr1 );
	trotListRefFree( &lr2 );
	trotListRefFree( &lr1Copy );
	

	index = 1;
	while ( index <= size )
	{
		/* create l1 */
		TEST_ERR_IF( (*createFunction)( &lr1, size ) != 0 );

		TEST_ERR_IF( compare( lr1, lr1, 1 ) != 0 );

		/* create copy */
		TEST_ERR_IF( trotListRefCopy( &lr1Copy, lr1 ) != 0 );

		TEST_ERR_IF( compare( lr1, lr1Copy, 1 ) != 0 );

		/* create l2 with same create function */
		TEST_ERR_IF( (*createFunction)( &lr2, size ) != 0 );

		TEST_ERR_IF( compare( lr1, lr2, 1 ) != 0 );

		/* test adding int */
		TEST_ERR_IF( trotListRefInsertInt( lr2, index, 0 ) != 0 );
		TEST_ERR_IF( compare( lr1, lr2, 0 ) != 0 );

		TEST_ERR_IF( trotListRefInsertInt( lr1, index, 0 ) != 0 );
		TEST_ERR_IF( compare( lr1, lr2, 1 ) != 0 );

		TEST_ERR_IF( trotListRefRemove( lr1, index ) != 0 );
		TEST_ERR_IF( trotListRefRemove( lr2, index ) != 0 );
		TEST_ERR_IF( compare( lr1, lr2, 1 ) != 0 );

		/* test adding twin of self */
		TEST_ERR_IF( trotListRefInsertListTwin( lr2, index, lr2 ) != 0 );
		TEST_ERR_IF( compare( lr1, lr2, 0 ) != 0 );

		TEST_ERR_IF( trotListRefInsertListTwin( lr1, index, lr1 ) != 0 );
		TEST_ERR_IF( compare( lr1, lr2, 1 ) != 0 );

		TEST_ERR_IF( trotListRefRemove( lr1, index ) != 0 );
		TEST_ERR_IF( trotListRefRemove( lr2, index ) != 0 );
		TEST_ERR_IF( compare( lr1, lr2, 1 ) != 0 );

		/* test adding twin of l1 */
		TEST_ERR_IF( trotListRefInsertListTwin( lr2, index, lr1 ) != 0 );
		TEST_ERR_IF( compare( lr1, lr2, 0 ) != 0 );

		TEST_ERR_IF( trotListRefInsertListTwin( lr1, index, lr1 ) != 0 );
		TEST_ERR_IF( compare( lr1, lr2, 1 ) != 0 );

		TEST_ERR_IF( trotListRefRemove( lr1, index ) != 0 );
		TEST_ERR_IF( trotListRefRemove( lr2, index ) != 0 );
		TEST_ERR_IF( compare( lr1, lr2, 1 ) != 0 );

		/* test adding copy of l1 */
		TEST_ERR_IF( trotListRefInsertListTwin( lr2, index, lr1Copy ) != 0 );
		TEST_ERR_IF( compare( lr1, lr2, 0 ) != 0 );

		TEST_ERR_IF( trotListRefInsertListTwin( lr1, index, lr1Copy ) != 0 );
		TEST_ERR_IF( compare( lr1, lr2, 1 ) != 0 );

		TEST_ERR_IF( trotListRefRemove( lr1, index ) != 0 );
		TEST_ERR_IF( trotListRefRemove( lr2, index ) != 0 );
		TEST_ERR_IF( compare( lr1, lr2, 1 ) != 0 );

		/* test deleting item */
		TEST_ERR_IF( trotListRefRemove( lr1, index ) != 0 );
		TEST_ERR_IF( compare( lr1, lr2, 0 ) != 0 );

		TEST_ERR_IF( trotListRefRemove( lr2, index ) != 0 );
		TEST_ERR_IF( compare( lr1, lr2, 1 ) != 0 );

		/* free our lists */
		trotListRefFree( &lr1 );
		trotListRefFree( &lr1Copy );
		trotListRefFree( &lr2 );

		index += 1;
	}

	return 0;


	/* CLEANUP */
	cleanup:

	printf( "\x1b[31;1mFAILED AT %d size:%d index:%d\n\x1b[0m", rc, size, index );
	return rc;
}

/******************************************************************************/
static int testEnlistDelist( int (*createFunction)( trotListRef **, int ), int size )
{
	/* DATA */
	int rc = 0;

	trotListRef *lr1 = NULL;
	trotListRef *lr2 = NULL;

	trotListRef *lrEmpty = NULL;

	INT_TYPE index1 = 0;
	INT_TYPE index2 = 0;

	INT_TYPE usedIndex1 = 0;
	INT_TYPE usedIndex2 = 0;

	INT_TYPE count = 0;

	INT_TYPE delistIndex = 0;


	/* CODE */
	index1 = 1;
	while( index1 <= size )
	{
		index2 = 1;
		while ( index2 <= size )
		{
			/* create our lists */
			TEST_ERR_IF( (*createFunction)( &lr1, size ) != 0 );
			TEST_ERR_IF( (*createFunction)( &lr2, size ) != 0 );
			TEST_ERR_IF( compare( lr1, lr2, 1 ) != 0 );

			/* enlist */
			TEST_ERR_IF( trotListRefGetCount( lr1, &count ) != 0 );

			usedIndex1 = index1;
			if ( rand() % 100 > 50 )
			{
				usedIndex1 = INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index1, count );
			}

			usedIndex2 = index2;
			if ( rand() % 100 > 50 )
			{
				usedIndex2 = INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index2, count );
			}

			TEST_ERR_IF( trotListRefEnlist( lr1, usedIndex1, usedIndex2 ) != 0 );
			TEST_ERR_IF( checkList( lr1 ) != 0 );
			TEST_ERR_IF( compare( lr1, lr2, 0 ) != 0 );

			/* delist */
			TEST_ERR_IF( trotListRefGetCount( lr1, &count ) != 0 );

			delistIndex = index1 < index2 ? index1 : index2;

			if ( rand() % 100 > 50 )
			{
				delistIndex = INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( delistIndex, count );
			}

			TEST_ERR_IF( trotListRefDelist( lr1, delistIndex ) != 0 );
			TEST_ERR_IF( checkList( lr1 ) != 0 );
			TEST_ERR_IF( compare( lr1, lr2, 1 ) != 0 );

			/* delist empty list */
			TEST_ERR_IF( trotListRefInit( &lrEmpty ) != 0 );
			TEST_ERR_IF( trotListRefInsertListTwin( lr1, 1, lrEmpty ) != 0 );
			TEST_ERR_IF( trotListRefDelist( lr1, 1 ) != 0 );
			TEST_ERR_IF( compare( lr1, lr2, 1 ) != 0 );

			/* free our lists */
			trotListRefFree( &lr1 );
			trotListRefFree( &lr2 );
			trotListRefFree( &lrEmpty );

			index2 += 1;
		}

		index1 += 1;
	}

	return 0;


	/* CLEANUP */
	cleanup:

	printf( "\x1b[31;1mFAILED AT %d size:%d count:%d index1:%d usedIndex1:%d index2:%d usedIndex2:%d delistIndex:%d\n\x1b[0m", rc, size, count, index1, usedIndex1, index2, usedIndex2, delistIndex );
	return rc;
}

static int testSpans( int (*createFunction)( trotListRef **, int ), int size )
{
	/* DATA */
	int rc = 0;

	trotListRef *lr1 = NULL;
	trotListRef *lr2 = NULL;

	trotListRef *lrSpan = NULL;

	INT_TYPE index1 = 0;
	INT_TYPE index2 = 0;

	INT_TYPE usedIndex1 = 0;
	INT_TYPE usedIndex2 = 0;

	INT_TYPE count = 0;

	INT_TYPE delistIndex = 0;


	/* CODE */
	index1 = 1;
	while( index1 <= size )
	{
		index2 = 1;
		while ( index2 <= size )
		{
			/* create our lists */
			TEST_ERR_IF( (*createFunction)( &lr1, size ) != 0 );
			TEST_ERR_IF( (*createFunction)( &lr2, size ) != 0 );
			TEST_ERR_IF( compare( lr1, lr2, 1 ) != 0 );

			/* create span copy */
			TEST_ERR_IF( trotListRefGetCount( lr1, &count ) != 0 );

			usedIndex1 = index1;
			if ( rand() % 100 > 50 )
			{
				usedIndex1 = INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index1, count );
			}

			usedIndex2 = index2;
			if ( rand() % 100 > 50 )
			{
				usedIndex2 = INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index2, count );
			}

			TEST_ERR_IF( trotListRefCopySpan( &lrSpan, lr1, usedIndex1, usedIndex2 ) != 0 );
			TEST_ERR_IF( checkList( lrSpan ) != 0 );
			TEST_ERR_IF( checkList( lr1 ) != 0 );

			/* remove span */
			TEST_ERR_IF( trotListRefRemoveSpan( lr1, usedIndex1, usedIndex2 ) != 0 );
			TEST_ERR_IF( checkList( lr1 ) != 0 );

			/* compare */
			TEST_ERR_IF( compare( lr1, lr2, 0 ) != 0 );

			/* insert and delist span */
			TEST_ERR_IF( trotListRefGetCount( lr1, &count ) != 0 );

			delistIndex = index1 < index2 ? index1 : index2;

			if ( rand() % 100 > 50 )
			{
				delistIndex = INDEX_TO_NEGATIVE_VERSION_INSERT( delistIndex, count );
			}

			TEST_ERR_IF( trotListRefInsertListTwin( lr1, delistIndex, lrSpan ) != 0 );

			TEST_ERR_IF( trotListRefDelist( lr1, delistIndex ) != 0 );

			/* compare */
			TEST_ERR_IF( compare( lr1, lr2, 1 ) != 0 );

			/* free our lists */
			trotListRefFree( &lr1 );
			trotListRefFree( &lr2 );
			trotListRefFree( &lrSpan );

			index2 += 1;
		}

		index1 += 1;
	}


	return 0;


	/* CLEANUP */
	cleanup:

	printf( "\x1b[31;1mFAILED AT %d size:%d count:%d index1:%d usedIndex1:%d index2:%d usedIndex2:%d delistIndex:%d\n\x1b[0m", rc, size, count, index1, usedIndex1, index2, usedIndex2, delistIndex );
	return rc;
}

