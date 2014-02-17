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
#define MAGIC_NUMBER (NODE_SIZE * 4)

#define TEST_ADDING_INTS  1
#define TEST_ADDING_LISTS 2

#define TEST_REMOVE_SPECIFIC_KIND 1
#define TEST_REMOVE_GENERIC       2

#define TEST_POSITIVE_INDICES 1
#define TEST_NEGATIVE_INDICES 2

/******************************************************************************/
/* create functions */
static int (*createFunctions[])( TrotList **, int ) =
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
static int testPrepend( TrotList *l, int intsOrLists, int removeSpecificOrGeneric, int positiveOrNegativeIndices );
static int testAppend( TrotList *l, int intsOrLists, int removeSpecificOrGeneric, int positiveOrNegativeIndices );
static int testAddToMiddle( TrotList *l, int intsOrLists, int removeSpecificOrGeneric, int positiveOrNegativeIndices );
static int testAddAtOddIndices( TrotList *l, int intsOrLists, int removeSpecificOrGeneric, int positiveOrNegativeIndices );

static int testReplace( TrotList *l, int intsOrLists, int positiveOrNegativeIndices );

static int (*testFunctions[])( TrotList *, int, int, int ) = 
	{
		testPrepend,
		testAppend,
		testAddToMiddle,
		testAddAtOddIndices,
		NULL
	};

/******************************************************************************/
int testPrimaryFunctionality()
{
	/* DATA */
	int rc = 0;

	int count = 0;
	int i = 0;
	int j = 0;

	TrotList *l = NULL;
	TROT_INT tag = 0;

	TrotList *l1 = NULL;
	TrotList *l2 = NULL;
	TROT_INT isSame = 0;

	TROT_INT index = 0;


	/* CODE */
	printf( "Testing primary functionality..." ); fflush( stdout );

	/* test refCompare */
	TEST_ERR_IF( trotListInit( &l ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListTwin( l, &l1 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListInit( &l2 ) != TROT_RC_SUCCESS );

	TEST_ERR_IF( trotListRefCompare( l, l, &isSame ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( isSame != 1 );
	TEST_ERR_IF( trotListRefCompare( l, l1, &isSame ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( isSame != 1 );
	TEST_ERR_IF( trotListRefCompare( l1, l2, &isSame ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( isSame != 0 );

	trotListFree( &l );
	trotListFree( &l1 );
	trotListFree( &l2 );

	/* test tags */
	TEST_ERR_IF( trotListInit( &l ) != TROT_RC_SUCCESS );

	TEST_ERR_IF( trotListGetTag( l, &tag ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( tag != 0 );
	TEST_ERR_IF( trotListSetTag( l, 1 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListGetTag( l, &tag ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( tag != 1 );

	TEST_ERR_IF( trotListGetUserTag( l, &tag ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( tag != 0 );
	TEST_ERR_IF( trotListSetUserTag( l, 60 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListGetUserTag( l, &tag ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( tag != 60 );
	trotListFree( &l );

	/* test TROT_MAX_CHILDREN */
	TEST_ERR_IF( trotListInit( &l ) != TROT_RC_SUCCESS );
	index = 1;
	while ( index <= TROT_MAX_CHILDREN )
	{
		TEST_ERR_IF( trotListAppendInt( l, 1 ) != TROT_RC_SUCCESS );

		/* increment */
		index += 1;
	}

	TEST_ERR_IF( trotListAppendInt( l, 1 ) != TROT_RC_ERROR_LIST_OVERFLOW );
	TEST_ERR_IF( trotListAppendList( l, l ) != TROT_RC_ERROR_LIST_OVERFLOW );
	TEST_ERR_IF( trotListInsertInt( l, 1, 1 ) != TROT_RC_ERROR_LIST_OVERFLOW );
	TEST_ERR_IF( trotListInsertList( l, 1, l ) != TROT_RC_ERROR_LIST_OVERFLOW );

	trotListFree( &l );
	

	/* *** */
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
				TEST_ERR_IF( createFunctions[ i ]( &l, count ) != 0 );
				TEST_ERR_IF( testFunctions[ j ]( l, TEST_ADDING_INTS, TEST_REMOVE_SPECIFIC_KIND, TEST_POSITIVE_INDICES ) != 0 );
				trotListFree( &l );

				TEST_ERR_IF( createFunctions[ i ]( &l, count ) != 0 );
				TEST_ERR_IF( testFunctions[ j ]( l, TEST_ADDING_INTS, TEST_REMOVE_SPECIFIC_KIND, TEST_NEGATIVE_INDICES ) != 0 );
				trotListFree( &l );

				TEST_ERR_IF( createFunctions[ i ]( &l, count ) != 0 );
				TEST_ERR_IF( testFunctions[ j ]( l, TEST_ADDING_INTS, TEST_REMOVE_GENERIC, TEST_POSITIVE_INDICES ) != 0 );
				trotListFree( &l );

				TEST_ERR_IF( createFunctions[ i ]( &l, count ) != 0 );
				TEST_ERR_IF( testFunctions[ j ]( l, TEST_ADDING_INTS, TEST_REMOVE_GENERIC, TEST_NEGATIVE_INDICES ) != 0 );
				trotListFree( &l );

				/* test adding lists */
				TEST_ERR_IF( createFunctions[ i ]( &l, count ) != 0 );
				TEST_ERR_IF( testFunctions[ j ]( l, TEST_ADDING_LISTS, TEST_REMOVE_SPECIFIC_KIND, TEST_POSITIVE_INDICES ) != 0 );
				trotListFree( &l );

				TEST_ERR_IF( createFunctions[ i ]( &l, count ) != 0 );
				TEST_ERR_IF( testFunctions[ j ]( l, TEST_ADDING_LISTS, TEST_REMOVE_SPECIFIC_KIND, TEST_NEGATIVE_INDICES ) != 0 );
				trotListFree( &l );

				TEST_ERR_IF( createFunctions[ i ]( &l, count ) != 0 );
				TEST_ERR_IF( testFunctions[ j ]( l, TEST_ADDING_LISTS, TEST_REMOVE_GENERIC, TEST_POSITIVE_INDICES ) != 0 );
				trotListFree( &l );

				TEST_ERR_IF( createFunctions[ i ]( &l, count ) != 0 );
				TEST_ERR_IF( testFunctions[ j ]( l, TEST_ADDING_LISTS, TEST_REMOVE_GENERIC, TEST_NEGATIVE_INDICES ) != 0 );
				trotListFree( &l );

				j += 1;
			}

			/* replace is different, we don't care about removing. so we just call it here instead */
			printf( "." ); fflush( stdout );

			TEST_ERR_IF( createFunctions[ i ]( &l, count ) != 0 );
			TEST_ERR_IF( testReplace( l, TEST_ADDING_INTS, TEST_POSITIVE_INDICES ) != 0 );

			trotListFree( &l );

			TEST_ERR_IF( createFunctions[ i ]( &l, count ) != 0 );
			TEST_ERR_IF( testReplace( l, TEST_ADDING_INTS, TEST_NEGATIVE_INDICES ) != 0 );

			trotListFree( &l );

			TEST_ERR_IF( createFunctions[ i ]( &l, count ) != 0 );
			TEST_ERR_IF( testReplace( l, TEST_ADDING_LISTS, TEST_POSITIVE_INDICES ) != 0 );

			trotListFree( &l );

			TEST_ERR_IF( createFunctions[ i ]( &l, count ) != 0 );
			TEST_ERR_IF( testReplace( l, TEST_ADDING_LISTS, TEST_NEGATIVE_INDICES ) != 0 );

			trotListFree( &l );

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
static int testPrepend( TrotList *l, int intsOrLists, int removeSpecificOrGeneric, int positiveOrNegativeIndices )
{
	/* DATA */
	int rc = 0;

	TROT_INT countAtStart = 0;
	TROT_INT countAtEnd = 0;
	TROT_INT countAdded = 0;

	TROT_INT newNumber = 0;
	TROT_INT addingAtIndex = 0;
	TROT_INT addingAtIndexB = 0; /* this may be the same, or it may be the negative version */

	TROT_INT index = 0;

	TROT_INT testNew = 0;
	TROT_INT testOriginal = 0;

	TROT_INT removedN = 0;
	TrotList *removedL = NULL;


	/* CODE */
	TEST_ERR_IF( trotListGetCount( l, &countAtStart ) != TROT_RC_SUCCESS );

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
			TEST_ERR_IF( trotListInsertInt( l, addingAtIndexB, newNumber ) != TROT_RC_SUCCESS );
		}
		else if ( intsOrLists == TEST_ADDING_LISTS )
		{
			TEST_ERR_IF( addListWithValue( l, addingAtIndexB, newNumber ) != 0 );
		}
		else
		{
			TEST_ERR_IF( 1 );
		}

		countAdded += 1;

		/* check */
		TEST_ERR_IF( checkList( l ) != 0 );

		index = 1;
		testNew = 1000;
		testOriginal = 0;
		while ( index <= countAdded )
		{
			testNew += 1;

			TEST_ERR_IF( check( l, index, testNew ) != 0 );
			TEST_ERR_IF( check( l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testNew ) != 0 );

			index += 1;
		}
		while ( index <= (countAtStart + countAdded) )
		{
			testOriginal += 1;

			TEST_ERR_IF( check( l, index, testOriginal ) != 0 );
			TEST_ERR_IF( check( l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testOriginal ) != 0 );

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
			TEST_ERR_IF( trotListRemove( l, addingAtIndexB ) != TROT_RC_SUCCESS );
		}
		else if ( removeSpecificOrGeneric == TEST_REMOVE_SPECIFIC_KIND )
		{
			if ( intsOrLists == TEST_ADDING_INTS )
			{
				TEST_ERR_IF( trotListRemoveInt( l, addingAtIndexB, &removedN ) != TROT_RC_SUCCESS );
				TEST_ERR_IF( removedN != newNumber );
			}
			else if ( intsOrLists == TEST_ADDING_LISTS )
			{
				TEST_ERR_IF( trotListRemoveList( l, addingAtIndexB, &removedL ) != TROT_RC_SUCCESS );
				TEST_ERR_IF( trotListGetInt( removedL, 1, &removedN ) != TROT_RC_SUCCESS );
				TEST_ERR_IF( removedN != newNumber );

				trotListFree( &removedL );
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
		TEST_ERR_IF( checkList( l ) != 0 );

		index = 1;
		testNew = 1000;
		testOriginal = 0;
		while ( index <= countAdded )
		{
			testNew += 1;

			TEST_ERR_IF( check( l, index, testNew ) != 0 );
			TEST_ERR_IF( check( l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testNew ) != 0 );

			index += 1;
		}
		while ( index <= (countAtStart + countAdded) )
		{
			testOriginal += 1;

			TEST_ERR_IF( check( l, index, testOriginal ) != 0 );
			TEST_ERR_IF( check( l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testOriginal ) != 0 );

			index += 1;
		}

		TEST_ERR_IF( testOriginal != countAtStart );
		TEST_ERR_IF( (testNew - 1000) != countAdded );
	}

	TEST_ERR_IF( trotListGetCount( l, &countAtEnd ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( countAtEnd != countAtStart );

	return 0;
	

	/* CLEANUP */
	cleanup:

	printf( "\x1b[31;1mFAILED AT %d testNew:%d testOriginal:%d index:%d addingAtIndex:%d\n\x1b[0m", rc, testNew, testOriginal, index, addingAtIndex );

	return rc;
}

/******************************************************************************/
static int testAppend( TrotList *l, int intsOrLists, int removeSpecificOrGeneric, int positiveOrNegativeIndices )
{
	/* DATA */
	int rc = 0;

	TROT_INT countAtStart = 0;
	TROT_INT countAtEnd = 0;
	TROT_INT countAdded = 0;

	TROT_INT newNumber = 0;
	TROT_INT addingAtIndex = 0;
	TROT_INT addingAtIndexB = 0; /* this may be the same, or it may be the negative version */

	TROT_INT index = 0;

	TROT_INT testNew = 0;
	TROT_INT testOriginal = 0;

	TROT_INT removedN = 0;
	TrotList *removedL = NULL;


	/* CODE */
	TEST_ERR_IF( trotListGetCount( l, &countAtStart ) != TROT_RC_SUCCESS );

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
			TEST_ERR_IF( trotListInsertInt( l, addingAtIndexB, newNumber ) != TROT_RC_SUCCESS );
		}
		else if ( intsOrLists == TEST_ADDING_LISTS )
		{
			TEST_ERR_IF( addListWithValue( l, addingAtIndexB, newNumber ) != 0 );
		}
		else
		{
			TEST_ERR_IF( 1 );
		}

		countAdded += 1;

		/* check */
		TEST_ERR_IF( checkList( l ) != 0 );

		index = 1;
		testNew = 1000;
		testOriginal = 0;
		while ( index <= countAtStart )
		{
			testOriginal += 1;

			TEST_ERR_IF( check( l, index, testOriginal ) != 0 );
			TEST_ERR_IF( check( l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testOriginal ) != 0 );

			index += 1;
		}
		while ( index <= (countAtStart + countAdded) )
		{
			testNew += 1;

			TEST_ERR_IF( check( l, index, testNew ) != 0 );
			TEST_ERR_IF( check( l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testNew ) != 0 );

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
			TEST_ERR_IF( trotListRemove( l, addingAtIndexB ) != TROT_RC_SUCCESS );
		}
		else if ( removeSpecificOrGeneric == TEST_REMOVE_SPECIFIC_KIND )
		{
			if ( intsOrLists == TEST_ADDING_INTS )
			{
				TEST_ERR_IF( trotListRemoveInt( l, addingAtIndexB, &removedN ) != TROT_RC_SUCCESS );
				TEST_ERR_IF( removedN != newNumber );
			}
			else if ( intsOrLists == TEST_ADDING_LISTS )
			{
				TEST_ERR_IF( trotListRemoveList( l, addingAtIndexB, &removedL ) != TROT_RC_SUCCESS );
				TEST_ERR_IF( trotListGetInt( removedL, 1, &removedN ) != TROT_RC_SUCCESS );
				TEST_ERR_IF( removedN != newNumber );

				trotListFree( &removedL );
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
		TEST_ERR_IF( checkList( l ) != 0 );

		index = 1;
		testNew = 1000;
		testOriginal = 0;
		while ( index <= countAtStart )
		{
			testOriginal += 1;

			TEST_ERR_IF( check( l, index, testOriginal ) != 0 );
			TEST_ERR_IF( check( l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testOriginal ) != 0 );

			index += 1;
		}
		while ( index <= (countAtStart + countAdded) )
		{
			testNew += 1;

			TEST_ERR_IF( check( l, index, testNew ) != 0 );
			TEST_ERR_IF( check( l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testNew ) != 0 );

			index += 1;
		}

		TEST_ERR_IF( testOriginal != countAtStart );
		TEST_ERR_IF( (testNew - 1000) != countAdded );
	}

	TEST_ERR_IF( trotListGetCount( l, &countAtEnd ) != 0 );
	TEST_ERR_IF( countAtEnd != countAtStart );

	return 0;
	

	/* CLEANUP */
	cleanup:

	printf( "\x1b[31;1mFAILED AT %d testNew:%d testOriginal:%d index:%d addingAtIndex:%d\n\x1b[0m", rc, testNew, testOriginal, index, addingAtIndex );

	return rc;
}

/******************************************************************************/
static int testAddToMiddle( TrotList *l, int intsOrLists, int removeSpecificOrGeneric, int positiveOrNegativeIndices )
{
	/* DATA */
	int rc = 0;

	TROT_INT countAtStart = 0;
	TROT_INT countAtEnd = 0;
	TROT_INT countAdded = 0;

	TROT_INT newNumber = 0;
	TROT_INT addingAtIndex = 0;
	TROT_INT addingAtIndexB = 0; /* this may be the same, or it may be the negative version */

	TROT_INT index = 0;

	TROT_INT testNew = 0;
	TROT_INT testOriginal = 0;

	TROT_INT removedN = 0;
	TrotList *removedL = NULL;

	TROT_INT startedAddingAtIndex = 0;


	/* CODE */
	TEST_ERR_IF( trotListGetCount( l, &countAtStart ) != TROT_RC_SUCCESS );

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
			TEST_ERR_IF( trotListInsertInt( l, addingAtIndexB, newNumber ) != TROT_RC_SUCCESS );
		}
		else if ( intsOrLists == TEST_ADDING_LISTS )
		{
			TEST_ERR_IF( addListWithValue( l, addingAtIndexB, newNumber ) != 0 );
		}
		else
		{
			TEST_ERR_IF( 1 );
		}

		countAdded += 1;

		/* check */
		TEST_ERR_IF( checkList( l ) != 0 );

		index = 1;
		testNew = 1000;
		testOriginal = 0;
		while ( index < startedAddingAtIndex )
		{
			testOriginal += 1;

			TEST_ERR_IF( check( l, index, testOriginal ) != 0 );
			TEST_ERR_IF( check( l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testOriginal ) != 0 );

			index += 1;
		}
		while ( index < startedAddingAtIndex + countAdded )
		{
			testNew += 1;

			TEST_ERR_IF( check( l, index, testNew ) != 0 );
			TEST_ERR_IF( check( l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testNew ) != 0 );

			index += 1;
		}
		while ( index <= (countAtStart + countAdded) )
		{
			testOriginal += 1;

			TEST_ERR_IF( check( l, index, testOriginal ) != 0 );
			TEST_ERR_IF( check( l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testOriginal ) != 0 );

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
			TEST_ERR_IF( trotListRemove( l, addingAtIndexB ) != TROT_RC_SUCCESS );
		}
		else if ( removeSpecificOrGeneric == TEST_REMOVE_SPECIFIC_KIND )
		{
			if ( intsOrLists == TEST_ADDING_INTS )
			{
				TEST_ERR_IF( trotListRemoveInt( l, addingAtIndexB, &removedN ) != TROT_RC_SUCCESS );
				TEST_ERR_IF( removedN != newNumber );
			}
			else if ( intsOrLists == TEST_ADDING_LISTS )
			{
				TEST_ERR_IF( trotListRemoveList( l, addingAtIndexB, &removedL ) != TROT_RC_SUCCESS );
				TEST_ERR_IF( trotListGetInt( removedL, 1, &removedN ) != TROT_RC_SUCCESS );
				TEST_ERR_IF( removedN != newNumber );

				trotListFree( &removedL );
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
		TEST_ERR_IF( checkList( l ) != 0 );

		index = 1;
		testNew = 1000;
		testOriginal = 0;
		while ( index < startedAddingAtIndex )
		{
			testOriginal += 1;

			TEST_ERR_IF( check( l, index, testOriginal ) != 0 );
			TEST_ERR_IF( check( l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testOriginal ) != 0 );

			index += 1;
		}
		while ( index < startedAddingAtIndex + countAdded )
		{
			testNew += 1;

			TEST_ERR_IF( check( l, index, testNew ) != 0 );
			TEST_ERR_IF( check( l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testNew ) != 0 );

			index += 1;
		}
		while ( index <= (countAtStart + countAdded) )
		{
			testOriginal += 1;

			TEST_ERR_IF( check( l, index, testOriginal ) != 0 );
			TEST_ERR_IF( check( l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testOriginal ) != 0 );

			index += 1;
		}

		TEST_ERR_IF( testOriginal != countAtStart );
		TEST_ERR_IF( (testNew - 1000) != countAdded );
	}

	TEST_ERR_IF( trotListGetCount( l, &countAtEnd ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( countAtEnd != countAtStart );

	return 0;
	

	/* CLEANUP */
	cleanup:

	printf( "\x1b[31;1mFAILED AT %d testNew:%d testOriginal:%d index:%d addingAtIndex:%d\n\x1b[0m", rc, testNew, testOriginal, index, addingAtIndex );

	return rc;
}

/******************************************************************************/
static int testAddAtOddIndices( TrotList *l, int intsOrLists, int removeSpecificOrGeneric, int positiveOrNegativeIndices )
{
	/* DATA */
	int rc = 0;

	TROT_INT countAtStart = 0;
	TROT_INT countAtEnd = 0;
	TROT_INT countAdded = 0;

	TROT_INT newNumber = 0;
	TROT_INT addingAtIndex = 0;
	TROT_INT addingAtIndexB = 0; /* this may be the same, or it may be the negative version */

	TROT_INT index = 0;

	TROT_INT testNew = 0;
	TROT_INT testOriginal = 0;

	TROT_INT removedN = 0;
	TrotList *removedL = NULL;


	/* CODE */
	TEST_ERR_IF( trotListGetCount( l, &countAtStart ) != TROT_RC_SUCCESS );

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
			TEST_ERR_IF( trotListInsertInt( l, addingAtIndexB, newNumber ) != TROT_RC_SUCCESS );
		}
		else if ( intsOrLists == TEST_ADDING_LISTS )
		{
			TEST_ERR_IF( addListWithValue( l, addingAtIndexB, newNumber ) != 0 );
		}
		else
		{
			TEST_ERR_IF( 1 );
		}

		countAdded += 1;

		/* check */
		TEST_ERR_IF( checkList( l ) != 0 );

		index = 1;
		testNew = 1000;
		testOriginal = 0;
		while ( index <= ( ( countAdded * 2 - 1 ) ) )
		{
			testNew += 1;

			TEST_ERR_IF( check( l, index, testNew ) != 0 );
			TEST_ERR_IF( check( l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testNew ) != 0 );

			index += 1;

			if ( index > ( ( countAdded * 2 ) - 1 ) )
			{
				break;
			}

			testOriginal += 1;

			TEST_ERR_IF( check( l, index, testOriginal ) != 0 );
			TEST_ERR_IF( check( l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testOriginal ) != 0 );

			index += 1;
		}
		while ( index <= countAtStart + countAdded )
		{
			testOriginal += 1;

			TEST_ERR_IF( check( l, index, testOriginal ) != 0 );
			TEST_ERR_IF( check( l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testOriginal ) != 0 );

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
			TEST_ERR_IF( trotListRemove( l, addingAtIndexB ) != TROT_RC_SUCCESS );
		}
		else if ( removeSpecificOrGeneric == TEST_REMOVE_SPECIFIC_KIND )
		{
			if ( intsOrLists == TEST_ADDING_INTS )
			{
				TEST_ERR_IF( trotListRemoveInt( l, addingAtIndexB, &removedN ) != TROT_RC_SUCCESS );
				TEST_ERR_IF( removedN != newNumber );
			}
			else if ( intsOrLists == TEST_ADDING_LISTS )
			{
				TEST_ERR_IF( trotListRemoveList( l, addingAtIndexB, &removedL ) != TROT_RC_SUCCESS );
				TEST_ERR_IF( trotListGetInt( removedL, 1, &removedN ) != TROT_RC_SUCCESS );
				TEST_ERR_IF( removedN != newNumber );

				trotListFree( &removedL );
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
		TEST_ERR_IF( checkList( l ) != 0 );

		index = 1;
		testNew = 1000;
		testOriginal = 0;
		while ( index <= ( ( countAdded * 2 - 1 ) ) )
		{
			testNew += 1;

			TEST_ERR_IF( check( l, index, testNew ) != 0 );
			TEST_ERR_IF( check( l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testNew ) != 0 );

			index += 1;

			if ( index > ( ( countAdded * 2 ) - 1 ) )
			{
				break;
			}

			testOriginal += 1;

			TEST_ERR_IF( check( l, index, testOriginal ) != 0 );
			TEST_ERR_IF( check( l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testOriginal ) != 0 );

			index += 1;
		}
		while ( index <= countAtStart + countAdded )
		{
			testOriginal += 1;

			TEST_ERR_IF( check( l, index, testOriginal ) != 0 );
			TEST_ERR_IF( check( l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testOriginal ) != 0 );

			index += 1;
		}

		TEST_ERR_IF( testOriginal != countAtStart );
		TEST_ERR_IF( (testNew - 1000) != countAdded );
	}

	TEST_ERR_IF( trotListGetCount( l, &countAtEnd ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( countAtEnd != countAtStart );

	return 0;
	

	/* CLEANUP */
	cleanup:

	printf( "\x1b[31;1mFAILED AT %d testNew:%d testOriginal:%d index:%d addingAtIndex:%d\n\x1b[0m", rc, testNew, testOriginal, index, addingAtIndex );

	return rc;
}

/******************************************************************************/
static int testReplace( TrotList *l, int intsOrLists, int positiveOrNegativeIndices )
{
	/* DATA */
	int rc = 0;

	TROT_INT countAtStart = 0;
	TROT_INT countAfter = 0;

	TROT_INT addingAtIndex = 0;
	TROT_INT addingAtIndexB = 0; /* this may be the same, or it may be the negative version */

	TROT_KIND kind;

	TrotList *newL = NULL;

	TROT_INT index = 0;


	/* CODE */
	TEST_ERR_IF( trotListGetCount( l, &countAtStart ) != TROT_RC_SUCCESS );

	/* test replace */
	addingAtIndex = 1;
	while ( addingAtIndex <= countAtStart )
	{
		/* replace */
		if ( positiveOrNegativeIndices == TEST_POSITIVE_INDICES )
		{
			addingAtIndexB = addingAtIndex;
		}
		else if ( positiveOrNegativeIndices == TEST_NEGATIVE_INDICES )
		{
			addingAtIndexB = INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( addingAtIndex, countAtStart );
		}

		TEST_ERR_IF( trotListGetKind( l, addingAtIndexB, &kind ) != TROT_RC_SUCCESS );

		if ( intsOrLists == TEST_ADDING_INTS )
		{
			TEST_ERR_IF( trotListReplaceWithInt( l, addingAtIndexB, -1 ) != TROT_RC_SUCCESS )
		}
		else if ( intsOrLists == TEST_ADDING_LISTS )
		{
			TEST_ERR_IF( trotListInit( &newL ) != TROT_RC_SUCCESS );
			TEST_ERR_IF( trotListAppendInt( newL, -1 ) != TROT_RC_SUCCESS );
			TEST_ERR_IF( trotListReplaceWithList( l, addingAtIndexB, newL ) != TROT_RC_SUCCESS );
			trotListFree( &newL );
		}
		else
		{
			TEST_ERR_IF( 1 );
		}

		/* check */
		TEST_ERR_IF( checkList( l ) != 0 );

		TEST_ERR_IF( trotListGetCount( l, &countAfter ) != TROT_RC_SUCCESS );
		TEST_ERR_IF( countAfter != countAtStart );

		index = 1;
		while ( index <= countAtStart )
		{
			if ( index == addingAtIndex )
			{
				TEST_ERR_IF( check( l, index, -1 ) != 0 );
			}
			else
			{
				TEST_ERR_IF( check( l, index, index ) != 0 );
			}

			index += 1;
		}

		/* replace back to original */
		if ( kind == TROT_KIND_INT )
		{
			TEST_ERR_IF( trotListReplaceWithInt( l, addingAtIndexB, addingAtIndex ) != TROT_RC_SUCCESS )
		}
		else
		{
			TEST_ERR_IF( trotListInit( &newL ) != TROT_RC_SUCCESS );
			TEST_ERR_IF( trotListAppendInt( newL, addingAtIndex ) != TROT_RC_SUCCESS );
			TEST_ERR_IF( trotListReplaceWithList( l, addingAtIndexB, newL ) != TROT_RC_SUCCESS );
			trotListFree( &newL );
		}

		/* increment */
		addingAtIndex += 1;
	}

	return 0;
	

	/* CLEANUP */
	cleanup:

	printf( "\x1b[31;1mFAILED AT %d index:%d addingAtIndex:%d\n\x1b[0m", rc, index, addingAtIndex );

	return rc;
}

