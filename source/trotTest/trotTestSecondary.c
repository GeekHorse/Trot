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

/******************************************************************************/
#define MAGIC_NUMBER (TROT_NODE_SIZE * 3)

/******************************************************************************/
/* helper functions */
static int compare( TrotList *lMemLimit, TrotList *l1, TrotList *l2, int shouldBeEqual );

/******************************************************************************/
/* create functions */
static int (*createFunctions[])( TrotList *lMemLimit, TrotList **, int ) =
	{
		createAllInts,
		createAllLists,
		createIntListAlternating,
		createListIntAlternating,
		createHalfIntHalfList,
		createHalfListHalfInt,

		NULL
	};

/******************************************************************************/
/* test functions */
static int testCopyCompare( TrotList *lMemLimit, int (*createFunction)( TrotList *, TrotList **, int ), int size );
static int testEnlistDelist( TrotList *lMemLimit, int (*createFunction)( TrotList *, TrotList **, int ), int size );
static int testSpans( TrotList *lMemLimit, int (*createFunction)( TrotList *, TrotList **, int ), int size );
static int testMaxChildrenWithDelist( TrotList *lMemLimit );

static int (*testFunctions[])( TrotList *, int (*)( TrotList *, TrotList **, int ), int ) = 
	{
		testCopyCompare,
		testEnlistDelist,
		testSpans,
		NULL
	};

/******************************************************************************/
int testSecondaryFunctionality( TrotList *lMemLimit )
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

				TEST_ERR_IF( testFunctions[ j ]( lMemLimit, createFunctions[ i ], count ) != 0 );

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

	/* test TROT_MAX_CHILDREN with delist */
	TEST_ERR_IF( testMaxChildrenWithDelist( lMemLimit ) != 0 );


	printf( "\n" ); fflush( stdout );

	return 0;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
static int compare( TrotList *lMemLimit, TrotList *l1, TrotList *l2, int shouldBeEqual )
{
	/* DATA */
	int rc = 0;

	TROT_LIST_COMPARE_RESULT compareResult1 = TROT_LIST_COMPARE_EQUAL;
	TROT_LIST_COMPARE_RESULT compareResult2 = TROT_LIST_COMPARE_EQUAL;


	/* CODE */
	if ( shouldBeEqual )
	{
		TEST_ERR_IF( trotListCompare( lMemLimit, l1, l2, &compareResult1 ) != TROT_RC_SUCCESS );
		TEST_ERR_IF( compareResult1 != TROT_LIST_COMPARE_EQUAL );

		TEST_ERR_IF( trotListCompare( lMemLimit, l2, l1, &compareResult1 ) != TROT_RC_SUCCESS );
		TEST_ERR_IF( compareResult1 != TROT_LIST_COMPARE_EQUAL );
	}
	else
	{
		TEST_ERR_IF( trotListCompare( lMemLimit, l1, l2, &compareResult1 ) != TROT_RC_SUCCESS );
		TEST_ERR_IF( compareResult1 == TROT_LIST_COMPARE_EQUAL );

		TEST_ERR_IF( trotListCompare( lMemLimit, l2, l1, &compareResult2 ) != TROT_RC_SUCCESS );
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
static int testCopyCompare( TrotList *lMemLimit, int (*createFunction)( TrotList *, TrotList **, int ), int size )
{
	/* DATA */
	int rc = 0;

	TrotList *l1 = NULL;
	TrotList *l1Copy = NULL;

	TROT_INT type1 = TROT_TYPE_DATA;
	TROT_INT type2 = TROT_TYPE_DATA;
	TROT_INT tag1 = TROT_TYPE_DATA;
	TROT_INT tag2 = TROT_TYPE_DATA;

	TrotList *l2 = NULL;

	TROT_INT index = 0;


	/* CODE */
	/* create l1 */
	TEST_ERR_IF( (*createFunction)( lMemLimit, &l1, 0 ) != 0 );

	TEST_ERR_IF( compare( lMemLimit, l1, l1, 1 ) != 0 );

	/* create copy */
	TEST_ERR_IF( trotListCopy( lMemLimit, l1, &l1Copy ) != TROT_RC_SUCCESS );

	TEST_ERR_IF( compare( lMemLimit, l1, l1Copy, 1 ) != 0 );

	/* *** */
	TEST_ERR_IF( (*createFunction)( lMemLimit, &l2, 0 ) != 0 );

	TEST_ERR_IF( trotListAppendList( lMemLimit, l1, l2 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListAppendList( lMemLimit, l2, l1 ) != TROT_RC_SUCCESS );

	TEST_ERR_IF( compare( lMemLimit, l1, l2, 1 ) != 0 );

	/* free lists */
	trotListFree( lMemLimit, &l1 );
	trotListFree( lMemLimit, &l2 );
	trotListFree( lMemLimit, &l1Copy );

	/* test tags */

	/* empty list, default tag */
	TEST_ERR_IF( (*createFunction)( lMemLimit, &l1, 0 ) != 0 );
	TEST_ERR_IF( trotListGetType( lMemLimit, l1, &type1 ) != TROT_RC_SUCCESS );

	TEST_ERR_IF( trotListCopy( lMemLimit, l1, &l1Copy ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListGetType( lMemLimit, l1Copy, &type2 ) != TROT_RC_SUCCESS );

	TEST_ERR_IF( type1 != type2 );

	trotListFree( lMemLimit, &l1 );
	trotListFree( lMemLimit, &l1Copy );

	/* empty list, new tag */
	TEST_ERR_IF( (*createFunction)( lMemLimit, &l1, 0 ) != 0 );
	type1 = TROT_TYPE_DATA;
	TEST_ERR_IF( trotListSetType( lMemLimit, l1, TROT_TYPE_CODE ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListGetType( lMemLimit, l1, &type1 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( type1 != TROT_TYPE_CODE );
	tag1 = TROT_TYPE_DATA;
	TEST_ERR_IF( trotListSetTag( lMemLimit, l1, TROT_TYPE_CODE ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListGetTag( lMemLimit, l1, &tag1 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( tag1 != TROT_TYPE_CODE );

	TEST_ERR_IF( trotListCopy( lMemLimit, l1, &l1Copy ) != TROT_RC_SUCCESS );
	type2 = TROT_TYPE_DATA;
	TEST_ERR_IF( trotListGetType( lMemLimit, l1Copy, &type2 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( type2 != TROT_TYPE_CODE );
	tag2 = TROT_TYPE_DATA;
	TEST_ERR_IF( trotListGetTag( lMemLimit, l1Copy, &tag2 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( tag2 != TROT_TYPE_CODE );

	trotListFree( lMemLimit, &l1 );
	trotListFree( lMemLimit, &l1Copy );

	/* list with data, new tag */
	TEST_ERR_IF( (*createFunction)( lMemLimit, &l1, size ) != 0 );
	type1 = TROT_TYPE_DATA;
	TEST_ERR_IF( trotListSetType( lMemLimit, l1, TROT_TYPE_CODE ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListGetType( lMemLimit, l1, &type1 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( type1 != TROT_TYPE_CODE );
	TEST_ERR_IF( trotListSetTag( lMemLimit, l1, TROT_TYPE_CODE ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListGetTag( lMemLimit, l1, &tag1 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( tag1 != TROT_TYPE_CODE );

	TEST_ERR_IF( trotListCopy( lMemLimit, l1, &l1Copy ) != TROT_RC_SUCCESS );
	type2 = TROT_TYPE_DATA;
	TEST_ERR_IF( trotListGetType( lMemLimit, l1Copy, &type2 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( type2 != TROT_TYPE_CODE );
	tag2 = TROT_TYPE_DATA;
	TEST_ERR_IF( trotListGetTag( lMemLimit, l1Copy, &tag2 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( tag2 != TROT_TYPE_CODE );

	trotListFree( lMemLimit, &l1 );
	trotListFree( lMemLimit, &l1Copy );

	/* list with data, new tag, copy a span */
	TEST_ERR_IF( (*createFunction)( lMemLimit, &l1, size ) != 0 );
	type1 = TROT_TYPE_DATA;
	TEST_ERR_IF( trotListSetType( lMemLimit, l1, TROT_TYPE_CODE ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListGetType( lMemLimit, l1, &type1 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( type1 != TROT_TYPE_CODE );
	tag1 = TROT_TYPE_DATA;
	TEST_ERR_IF( trotListSetTag( lMemLimit, l1, TROT_TYPE_CODE ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListGetTag( lMemLimit, l1, &tag1 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( tag1 != TROT_TYPE_CODE );

	TEST_ERR_IF( trotListCopySpan( lMemLimit, l1, (rand() % size) + 1, (rand() % size) + 1, &l1Copy ) != TROT_RC_SUCCESS );
	type2 = TROT_TYPE_DATA;
	TEST_ERR_IF( trotListGetType( lMemLimit, l1Copy, &type2 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( type2 != TROT_TYPE_CODE );
	tag2 = TROT_TYPE_DATA;
	TEST_ERR_IF( trotListGetTag( lMemLimit, l1Copy, &tag2 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( tag2 != TROT_TYPE_CODE );

	trotListFree( lMemLimit, &l1 );
	trotListFree( lMemLimit, &l1Copy );
	
	
	/* *** */
	index = 1;
	while ( index <= size )
	{
		/* create l1 */
		TEST_ERR_IF( (*createFunction)( lMemLimit, &l1, size ) != 0 );

		TEST_ERR_IF( compare( lMemLimit, l1, l1, 1 ) != 0 );

		/* create copy */
		TEST_ERR_IF( trotListCopy( lMemLimit, l1, &l1Copy ) != TROT_RC_SUCCESS );

		TEST_ERR_IF( compare( lMemLimit, l1, l1Copy, 1 ) != 0 );

		/* create l2 with same create function */
		TEST_ERR_IF( (*createFunction)( lMemLimit, &l2, size ) != 0 );

		TEST_ERR_IF( compare( lMemLimit, l1, l2, 1 ) != 0 );

		/* test adding int */
		TEST_ERR_IF( trotListInsertInt( lMemLimit, l2, index, 0 ) != TROT_RC_SUCCESS );
		TEST_ERR_IF( compare( lMemLimit, l1, l2, 0 ) != 0 );

		TEST_ERR_IF( trotListInsertInt( lMemLimit, l1, index, 0 ) != TROT_RC_SUCCESS );
		TEST_ERR_IF( compare( lMemLimit, l1, l2, 1 ) != 0 );

		TEST_ERR_IF( trotListRemove( lMemLimit, l1, index ) != TROT_RC_SUCCESS );
		TEST_ERR_IF( trotListRemove( lMemLimit, l2, index ) != TROT_RC_SUCCESS );
		TEST_ERR_IF( compare( lMemLimit, l1, l2, 1 ) != 0 );

		/* test adding twin of self */
		TEST_ERR_IF( trotListInsertList( lMemLimit, l2, index, l2 ) != TROT_RC_SUCCESS );
		TEST_ERR_IF( compare( lMemLimit, l1, l2, 0 ) != 0 );

		TEST_ERR_IF( trotListInsertList( lMemLimit, l1, index, l1 ) != TROT_RC_SUCCESS );
		TEST_ERR_IF( compare( lMemLimit, l1, l2, 1 ) != 0 );

		TEST_ERR_IF( trotListRemove( lMemLimit, l1, index ) != TROT_RC_SUCCESS );
		TEST_ERR_IF( trotListRemove( lMemLimit, l2, index ) != TROT_RC_SUCCESS );
		TEST_ERR_IF( compare( lMemLimit, l1, l2, 1 ) != 0 );

		/* test adding twin of l1 */
		TEST_ERR_IF( trotListInsertList( lMemLimit, l2, index, l1 ) != TROT_RC_SUCCESS );
		TEST_ERR_IF( compare( lMemLimit, l1, l2, 0 ) != 0 );

		TEST_ERR_IF( trotListInsertList( lMemLimit, l1, index, l1 ) != TROT_RC_SUCCESS );
		TEST_ERR_IF( compare( lMemLimit, l1, l2, 1 ) != 0 );

		TEST_ERR_IF( trotListRemove( lMemLimit, l1, index ) != TROT_RC_SUCCESS );
		TEST_ERR_IF( trotListRemove( lMemLimit, l2, index ) != TROT_RC_SUCCESS );
		TEST_ERR_IF( compare( lMemLimit, l1, l2, 1 ) != 0 );

		/* test adding copy of l1 */
		TEST_ERR_IF( trotListInsertList( lMemLimit, l2, index, l1Copy ) != TROT_RC_SUCCESS );
		TEST_ERR_IF( compare( lMemLimit, l1, l2, 0 ) != 0 );

		TEST_ERR_IF( trotListInsertList( lMemLimit, l1, index, l1Copy ) != TROT_RC_SUCCESS );
		TEST_ERR_IF( compare( lMemLimit, l1, l2, 1 ) != 0 );

		TEST_ERR_IF( trotListRemove( lMemLimit, l1, index ) != TROT_RC_SUCCESS );
		TEST_ERR_IF( trotListRemove( lMemLimit, l2, index ) != TROT_RC_SUCCESS );
		TEST_ERR_IF( compare( lMemLimit, l1, l2, 1 ) != 0 );

		/* test deleting item */
		TEST_ERR_IF( trotListRemove( lMemLimit, l1, index ) != TROT_RC_SUCCESS );
		TEST_ERR_IF( compare( lMemLimit, l1, l2, 0 ) != 0 );

		TEST_ERR_IF( trotListRemove( lMemLimit, l2, index ) != TROT_RC_SUCCESS );
		TEST_ERR_IF( compare( lMemLimit, l1, l2, 1 ) != 0 );

		/* free our lists */
		trotListFree( lMemLimit, &l1 );
		trotListFree( lMemLimit, &l1Copy );
		trotListFree( lMemLimit, &l2 );

		index += 1;
	}

	return 0;


	/* CLEANUP */
	cleanup:

	printf( "\x1b[31;1mFAILED AT %d size:%d index:%d\n\x1b[0m", rc, size, index );
	return rc;
}

/******************************************************************************/
static int testEnlistDelist( TrotList *lMemLimit, int (*createFunction)( TrotList *, TrotList **, int ), int size )
{
	/* DATA */
	int rc = 0;

	TrotList *l1 = NULL;
	TrotList *l2 = NULL;

	TrotList *lEmpty = NULL;

	TROT_INT index1 = 0;
	TROT_INT index2 = 0;

	TROT_INT usedIndex1 = 0;
	TROT_INT usedIndex2 = 0;

	TROT_INT count = 0;

	TROT_INT delistIndex = 0;


	/* CODE */
	index1 = 1;
	while( index1 <= size )
	{
		index2 = 1;
		while ( index2 <= size )
		{
			/* create our lists */
			TEST_ERR_IF( (*createFunction)( lMemLimit, &l1, size ) != 0 );
			TEST_ERR_IF( (*createFunction)( lMemLimit, &l2, size ) != 0 );
			TEST_ERR_IF( compare( lMemLimit, l1, l2, 1 ) != 0 );

			/* enlist */
			TEST_ERR_IF( trotListGetCount( lMemLimit, l1, &count ) != TROT_RC_SUCCESS );

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

			TEST_ERR_IF( trotListEnlist( lMemLimit, l1, usedIndex1, usedIndex2 ) != TROT_RC_SUCCESS );
			TEST_ERR_IF( checkList( lMemLimit, l1 ) != 0 );
			TEST_ERR_IF( compare( lMemLimit, l1, l2, 0 ) != 0 );

			/* delist */
			TEST_ERR_IF( trotListGetCount( lMemLimit, l1, &count ) != TROT_RC_SUCCESS );

			delistIndex = index1 < index2 ? index1 : index2;

			if ( rand() % 100 > 50 )
			{
				delistIndex = INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( delistIndex, count );
			}

			TEST_ERR_IF( trotListDelist( lMemLimit, l1, delistIndex ) != TROT_RC_SUCCESS );
			TEST_ERR_IF( checkList( lMemLimit, l1 ) != 0 );
			TEST_ERR_IF( compare( lMemLimit, l1, l2, 1 ) != 0 );

			/* delist empty list */
			TEST_ERR_IF( trotListInit( lMemLimit, &lEmpty ) != TROT_RC_SUCCESS );
			TEST_ERR_IF( trotListInsertList( lMemLimit, l1, 1, lEmpty ) != TROT_RC_SUCCESS );
			TEST_ERR_IF( trotListDelist( lMemLimit, l1, 1 ) != TROT_RC_SUCCESS );
			TEST_ERR_IF( compare( lMemLimit, l1, l2, 1 ) != 0 );

			/* free our lists */
			trotListFree( lMemLimit, &l1 );
			trotListFree( lMemLimit, &l2 );
			trotListFree( lMemLimit, &lEmpty );

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

/******************************************************************************/
static int testSpans( TrotList *lMemLimit, int (*createFunction)( TrotList *, TrotList **, int ), int size )
{
	/* DATA */
	int rc = 0;

	TrotList *l1 = NULL;
	TrotList *l2 = NULL;

	TrotList *lSpan = NULL;

	TROT_INT index1 = 0;
	TROT_INT index2 = 0;

	TROT_INT usedIndex1 = 0;
	TROT_INT usedIndex2 = 0;

	TROT_INT count = 0;

	TROT_INT delistIndex = 0;


	/* CODE */
	index1 = 1;
	while( index1 <= size )
	{
		index2 = 1;
		while ( index2 <= size )
		{
			/* create our lists */
			TEST_ERR_IF( (*createFunction)( lMemLimit, &l1, size ) != 0 );
			TEST_ERR_IF( (*createFunction)( lMemLimit, &l2, size ) != 0 );
			TEST_ERR_IF( compare( lMemLimit, l1, l2, 1 ) != 0 );

			/* create span copy */
			TEST_ERR_IF( trotListGetCount( lMemLimit, l1, &count ) != TROT_RC_SUCCESS );

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

			TEST_ERR_IF( trotListCopySpan( lMemLimit, l1, usedIndex1, usedIndex2, &lSpan ) != TROT_RC_SUCCESS );
			TEST_ERR_IF( checkList( lMemLimit, lSpan ) != 0 );
			TEST_ERR_IF( checkList( lMemLimit, l1 ) != 0 );

			/* remove span */
			TEST_ERR_IF( trotListRemoveSpan( lMemLimit, l1, usedIndex1, usedIndex2 ) != TROT_RC_SUCCESS );
			TEST_ERR_IF( checkList( lMemLimit, l1 ) != 0 );

			/* compare */
			TEST_ERR_IF( compare( lMemLimit, l1, l2, 0 ) != 0 );

			/* insert and delist span */
			TEST_ERR_IF( trotListGetCount( lMemLimit, l1, &count ) != TROT_RC_SUCCESS );

			delistIndex = index1 < index2 ? index1 : index2;

			if ( rand() % 100 > 50 )
			{
				delistIndex = INDEX_TO_NEGATIVE_VERSION_INSERT( delistIndex, count );
			}

			TEST_ERR_IF( trotListInsertList( lMemLimit, l1, delistIndex, lSpan ) != TROT_RC_SUCCESS );

			TEST_ERR_IF( trotListDelist( lMemLimit, l1, delistIndex ) != TROT_RC_SUCCESS );

			/* compare */
			TEST_ERR_IF( compare( lMemLimit, l1, l2, 1 ) != 0 );

			/* free our lists */
			trotListFree( lMemLimit, &l1 );
			trotListFree( lMemLimit, &l2 );
			trotListFree( lMemLimit, &lSpan );

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

/******************************************************************************/
static int testMaxChildrenWithDelist( TrotList *lMemLimit )
{
	/* DATA */
	int rc = 0;

	TROT_INT count = 0;
	TrotList *l = NULL;
	TrotList *lwith1 = NULL;
	TrotList *lwith2 = NULL;


	/* CODE */
	if ( TROT_MAX_CHILDREN == TROT_INT_MAX )
	{
		printf( "\n\n\n" );
		printf( "--- SKIPPING TROT_MAX_CHILDREN TEST! ---\n" );
		printf( "It's too memory intensive to test TROT_MAX_CHILDREN when it's\n" );
		printf( "the same as TROT_INT_MAX. To test TROT_MAX_CHILDREN, run the\n" );
		printf( "tests against a debug version of Trot. ex: make debug\n" );
		printf( "----------------------------------------\n" );
		printf( "\n\n\n" );

		goto cleanup;
	}

	/* create our main l, to have TROT_MAX_CHILDREN - 1 children */
	TEST_ERR_IF( trotListInit( lMemLimit, &l ) != TROT_RC_SUCCESS );
	
	count = 1;
	while ( count < TROT_MAX_CHILDREN )
	{
		TEST_ERR_IF( trotListAppendInt( lMemLimit, l, 1 ) != TROT_RC_SUCCESS );

		/* increment */
		count += 1;
	}

	/* create lwith1, to have 1 child */
	TEST_ERR_IF( trotListInit( lMemLimit, &lwith1 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListAppendInt( lMemLimit, lwith1, 1 ) != TROT_RC_SUCCESS );

	/* create lwith2, to have 2 children */
	TEST_ERR_IF( trotListInit( lMemLimit, &lwith2 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListAppendInt( lMemLimit, lwith2, 1 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListAppendInt( lMemLimit, lwith2, 1 ) != TROT_RC_SUCCESS );

	/* verify counts */
	TEST_ERR_IF( trotListGetCount( lMemLimit, l, &count ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( count != ( TROT_MAX_CHILDREN - 1 ) );

	TEST_ERR_IF( trotListGetCount( lMemLimit, lwith1, &count ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( count != 1 );

	TEST_ERR_IF( trotListGetCount( lMemLimit, lwith2, &count ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( count != 2 );

	/* test adding lwith1 and delisting, which should work */
	TEST_ERR_IF( trotListAppendList( lMemLimit, l, lwith1 ) != TROT_RC_SUCCESS );

	TEST_ERR_IF( trotListGetCount( lMemLimit, l, &count ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( count != TROT_MAX_CHILDREN );

	TEST_ERR_IF( trotListDelist( lMemLimit, l, -1 ) != TROT_RC_SUCCESS );

	TEST_ERR_IF( trotListGetCount( lMemLimit, l, &count ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( count != TROT_MAX_CHILDREN );

	/* test adding lwith2 and delisting, which should fail */
	TEST_ERR_IF( trotListRemove( lMemLimit, l, -1 ) != TROT_RC_SUCCESS );

	TEST_ERR_IF( trotListAppendList( lMemLimit, l, lwith2 ) != TROT_RC_SUCCESS );

	TEST_ERR_IF( trotListGetCount( lMemLimit, l, &count ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( count != TROT_MAX_CHILDREN );

	TEST_ERR_IF( trotListDelist( lMemLimit, l, -1 ) != TROT_RC_ERROR_LIST_OVERFLOW );

	TEST_ERR_IF( trotListGetCount( lMemLimit, l, &count ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( count != TROT_MAX_CHILDREN );

	trotListFree( lMemLimit, &l );
	trotListFree( lMemLimit, &lwith1 );
	trotListFree( lMemLimit, &lwith2 );


	/* CLEANUP */
	cleanup:

	return rc;
}

