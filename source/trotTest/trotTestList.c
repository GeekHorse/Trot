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
#define MAGIC_NUMBER (TROT_NODE_SIZE * 4)

#define TEST_ADDING_INTS  1
#define TEST_ADDING_LISTS 2

#define TEST_REMOVE_SPECIFIC_KIND 1
#define TEST_REMOVE_GENERIC       2

#define TEST_POSITIVE_INDICES 1
#define TEST_NEGATIVE_INDICES 2

/******************************************************************************/
/* create functions */
static int (*createFunctions[])( TrotProgram *, TrotList **, int ) =
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
static int testPrepend( TrotProgram *program, TrotList *l, int intsOrLists, int removeSpecificOrGeneric, int positiveOrNegativeIndices );
static int testAppend( TrotProgram *program, TrotList *l, int intsOrLists, int removeSpecificOrGeneric, int positiveOrNegativeIndices );
static int testAddToMiddle( TrotProgram *program, TrotList *l, int intsOrLists, int removeSpecificOrGeneric, int positiveOrNegativeIndices );
static int testAddAtOddIndices( TrotProgram *program, TrotList *l, int intsOrLists, int removeSpecificOrGeneric, int positiveOrNegativeIndices );

static int testReplace( TrotProgram *program, TrotList *l, int intsOrLists, int positiveOrNegativeIndices );

static int (*testFunctions[])( TrotProgram *, TrotList *, int, int, int ) = 
	{
		testPrepend,
		testAppend,
		testAddToMiddle,
		testAddAtOddIndices,
		NULL
	};

/******************************************************************************/
int testPrimaryFunctionality( TrotProgram *program )
{
	/* DATA */
	int rc = 0;

	int count = 0;
	int i = 0;
	int j = 0;

	TrotList *l = NULL;
	TROT_INT type = 0;
	TROT_INT tag = 0;

	TrotList *l1 = NULL;
	TrotList *l2 = NULL;
	TROT_INT isSame = 0;

	TROT_INT index = 0;


	/* CODE */
	printf( "Testing primary functionality...\n" ); fflush( stdout );

	/* test refCompare */
	printf( "  Testing refCompare...\n" ); fflush( stdout );
	TEST_ERR_IF( trotListInit( program, &l ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListTwin( program, l, &l1 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListInit( program, &l2 ) != TROT_RC_SUCCESS );

	TEST_ERR_IF( trotListRefCompare( program, l, l, &isSame ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( isSame != 1 );
	TEST_ERR_IF( trotListRefCompare( program, l, l1, &isSame ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( isSame != 1 );
	TEST_ERR_IF( trotListRefCompare( program, l1, l2, &isSame ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( isSame != 0 );

	trotListFree( program, &l );
	trotListFree( program, &l1 );
	trotListFree( program, &l2 );

	/* test tags */
	printf( "  Testing tags...\n" ); fflush( stdout );
	TEST_ERR_IF( trotListInit( program, &l ) != TROT_RC_SUCCESS );

	TEST_ERR_IF( trotListGetType( program, l, &type ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( type != 0 );
	TEST_ERR_IF( trotListSetType( program, l, 1 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListGetType( program, l, &type ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( type != 1 );

	TEST_ERR_IF( trotListGetTag( program, l, &tag ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( tag != 0 );
	TEST_ERR_IF( trotListSetTag( program, l, 60 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListGetTag( program, l, &tag ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( tag != 60 );
	trotListFree( program, &l );

	/* test TROT_MAX_CHILDREN */
	printf( "  Testing TROT_MAX_CHILDREN...\n" ); fflush( stdout );

	if ( TROT_MAX_CHILDREN == TROT_INT_MAX )
	{
		printf( "\n\n\n" );
		printf( "--- SKIPPING TROT_MAX_CHILDREN TEST! ---\n" );
		printf( "It's too memory intensive to test TROT_MAX_CHILDREN when it's\n" );
		printf( "the same as TROT_INT_MAX. To test TROT_MAX_CHILDREN, run the\n" );
		printf( "tests against a debug version of Trot. ex: make debug\n" );
		printf( "----------------------------------------\n" );
		printf( "\n\n\n" );
	}
	else
	{
		TEST_ERR_IF( trotListInit( program, &l ) != TROT_RC_SUCCESS );
		index = 1;
		while ( index <= TROT_MAX_CHILDREN )
		{
			TEST_ERR_IF( trotListAppendInt( program, l, 1 ) != TROT_RC_SUCCESS );

			/* increment */
			index += 1;
		}

		TEST_ERR_IF( trotListAppendInt( program, l, 1 ) != TROT_RC_ERROR_LIST_OVERFLOW );
		TEST_ERR_IF( trotListAppendList( program, l, l ) != TROT_RC_ERROR_LIST_OVERFLOW );
		TEST_ERR_IF( trotListInsertInt( program, l, 1, 1 ) != TROT_RC_ERROR_LIST_OVERFLOW );
		TEST_ERR_IF( trotListInsertList( program, l, 1, l ) != TROT_RC_ERROR_LIST_OVERFLOW );

		trotListFree( program, &l );
	}

	/* *** */
	printf( "  Testing rest of primary functions...\n" ); fflush( stdout );
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
				TEST_ERR_IF( createFunctions[ i ]( program, &l, count ) != 0 );
				TEST_ERR_IF( testFunctions[ j ]( program, l, TEST_ADDING_INTS, TEST_REMOVE_SPECIFIC_KIND, TEST_POSITIVE_INDICES ) != 0 );
				trotListFree( program, &l );

				TEST_ERR_IF( createFunctions[ i ]( program, &l, count ) != 0 );
				TEST_ERR_IF( testFunctions[ j ]( program, l, TEST_ADDING_INTS, TEST_REMOVE_SPECIFIC_KIND, TEST_NEGATIVE_INDICES ) != 0 );
				trotListFree( program, &l );

				TEST_ERR_IF( createFunctions[ i ]( program, &l, count ) != 0 );
				TEST_ERR_IF( testFunctions[ j ]( program, l, TEST_ADDING_INTS, TEST_REMOVE_GENERIC, TEST_POSITIVE_INDICES ) != 0 );
				trotListFree( program, &l );

				TEST_ERR_IF( createFunctions[ i ]( program, &l, count ) != 0 );
				TEST_ERR_IF( testFunctions[ j ]( program, l, TEST_ADDING_INTS, TEST_REMOVE_GENERIC, TEST_NEGATIVE_INDICES ) != 0 );
				trotListFree( program, &l );

				/* test adding lists */
				TEST_ERR_IF( createFunctions[ i ]( program, &l, count ) != 0 );
				TEST_ERR_IF( testFunctions[ j ]( program, l, TEST_ADDING_LISTS, TEST_REMOVE_SPECIFIC_KIND, TEST_POSITIVE_INDICES ) != 0 );
				trotListFree( program, &l );

				TEST_ERR_IF( createFunctions[ i ]( program, &l, count ) != 0 );
				TEST_ERR_IF( testFunctions[ j ]( program, l, TEST_ADDING_LISTS, TEST_REMOVE_SPECIFIC_KIND, TEST_NEGATIVE_INDICES ) != 0 );
				trotListFree( program, &l );

				TEST_ERR_IF( createFunctions[ i ]( program, &l, count ) != 0 );
				TEST_ERR_IF( testFunctions[ j ]( program, l, TEST_ADDING_LISTS, TEST_REMOVE_GENERIC, TEST_POSITIVE_INDICES ) != 0 );
				trotListFree( program, &l );

				TEST_ERR_IF( createFunctions[ i ]( program, &l, count ) != 0 );
				TEST_ERR_IF( testFunctions[ j ]( program, l, TEST_ADDING_LISTS, TEST_REMOVE_GENERIC, TEST_NEGATIVE_INDICES ) != 0 );
				trotListFree( program, &l );

				j += 1;
			}

			/* replace is different, we don't care about removing. so we just call it here instead */
			printf( "." ); fflush( stdout );

			TEST_ERR_IF( createFunctions[ i ]( program, &l, count ) != 0 );
			TEST_ERR_IF( testReplace( program, l, TEST_ADDING_INTS, TEST_POSITIVE_INDICES ) != 0 );

			trotListFree( program, &l );

			TEST_ERR_IF( createFunctions[ i ]( program, &l, count ) != 0 );
			TEST_ERR_IF( testReplace( program, l, TEST_ADDING_INTS, TEST_NEGATIVE_INDICES ) != 0 );

			trotListFree( program, &l );

			TEST_ERR_IF( createFunctions[ i ]( program, &l, count ) != 0 );
			TEST_ERR_IF( testReplace( program, l, TEST_ADDING_LISTS, TEST_POSITIVE_INDICES ) != 0 );

			trotListFree( program, &l );

			TEST_ERR_IF( createFunctions[ i ]( program, &l, count ) != 0 );
			TEST_ERR_IF( testReplace( program, l, TEST_ADDING_LISTS, TEST_NEGATIVE_INDICES ) != 0 );

			trotListFree( program, &l );

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
static int testPrepend( TrotProgram *program, TrotList *l, int intsOrLists, int removeSpecificOrGeneric, int positiveOrNegativeIndices )
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
	TEST_ERR_IF( trotListGetCount( program, l, &countAtStart ) != TROT_RC_SUCCESS );

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
			TEST_ERR_IF( trotListInsertInt( program, l, addingAtIndexB, newNumber ) != TROT_RC_SUCCESS );
		}
		else if ( intsOrLists == TEST_ADDING_LISTS )
		{
			TEST_ERR_IF( addListWithValue( program, l, addingAtIndexB, newNumber ) != 0 );
		}
		else
		{
			TEST_ERR_IF( 1 );
		}

		countAdded += 1;

		/* check */
		TEST_ERR_IF( checkList( program, l ) != 0 );

		index = 1;
		testNew = 1000;
		testOriginal = 0;
		while ( index <= countAdded )
		{
			testNew += 1;

			TEST_ERR_IF( check( program, l, index, testNew ) != 0 );
			TEST_ERR_IF( check( program, l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testNew ) != 0 );

			index += 1;
		}
		while ( index <= (countAtStart + countAdded) )
		{
			testOriginal += 1;

			TEST_ERR_IF( check( program, l, index, testOriginal ) != 0 );
			TEST_ERR_IF( check( program, l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testOriginal ) != 0 );

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
			TEST_ERR_IF( trotListRemove( program, l, addingAtIndexB ) != TROT_RC_SUCCESS );
		}
		else if ( removeSpecificOrGeneric == TEST_REMOVE_SPECIFIC_KIND )
		{
			if ( intsOrLists == TEST_ADDING_INTS )
			{
				TEST_ERR_IF( trotListRemoveInt( program, l, addingAtIndexB, &removedN ) != TROT_RC_SUCCESS );
				TEST_ERR_IF( removedN != newNumber );
			}
			else if ( intsOrLists == TEST_ADDING_LISTS )
			{
				TEST_ERR_IF( trotListRemoveList( program, l, addingAtIndexB, &removedL ) != TROT_RC_SUCCESS );
				TEST_ERR_IF( trotListGetInt( program, removedL, 1, &removedN ) != TROT_RC_SUCCESS );
				TEST_ERR_IF( removedN != newNumber );

				trotListFree( program, &removedL );
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
		TEST_ERR_IF( checkList( program, l ) != 0 );

		index = 1;
		testNew = 1000;
		testOriginal = 0;
		while ( index <= countAdded )
		{
			testNew += 1;

			TEST_ERR_IF( check( program, l, index, testNew ) != 0 );
			TEST_ERR_IF( check( program, l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testNew ) != 0 );

			index += 1;
		}
		while ( index <= (countAtStart + countAdded) )
		{
			testOriginal += 1;

			TEST_ERR_IF( check( program, l, index, testOriginal ) != 0 );
			TEST_ERR_IF( check( program, l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testOriginal ) != 0 );

			index += 1;
		}

		TEST_ERR_IF( testOriginal != countAtStart );
		TEST_ERR_IF( (testNew - 1000) != countAdded );
	}

	TEST_ERR_IF( trotListGetCount( program, l, &countAtEnd ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( countAtEnd != countAtStart );

	return 0;
	

	/* CLEANUP */
	cleanup:

	printf( "\x1b[31;1mFAILED AT %d testNew:%d testOriginal:%d index:%d addingAtIndex:%d\n\x1b[0m", rc, testNew, testOriginal, index, addingAtIndex );

	return rc;
}

/******************************************************************************/
static int testAppend( TrotProgram *program, TrotList *l, int intsOrLists, int removeSpecificOrGeneric, int positiveOrNegativeIndices )
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
	TEST_ERR_IF( trotListGetCount( program, l, &countAtStart ) != TROT_RC_SUCCESS );

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
			TEST_ERR_IF( trotListInsertInt( program, l, addingAtIndexB, newNumber ) != TROT_RC_SUCCESS );
		}
		else if ( intsOrLists == TEST_ADDING_LISTS )
		{
			TEST_ERR_IF( addListWithValue( program, l, addingAtIndexB, newNumber ) != 0 );
		}
		else
		{
			TEST_ERR_IF( 1 );
		}

		countAdded += 1;

		/* check */
		TEST_ERR_IF( checkList( program, l ) != 0 );

		index = 1;
		testNew = 1000;
		testOriginal = 0;
		while ( index <= countAtStart )
		{
			testOriginal += 1;

			TEST_ERR_IF( check( program, l, index, testOriginal ) != 0 );
			TEST_ERR_IF( check( program, l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testOriginal ) != 0 );

			index += 1;
		}
		while ( index <= (countAtStart + countAdded) )
		{
			testNew += 1;

			TEST_ERR_IF( check( program, l, index, testNew ) != 0 );
			TEST_ERR_IF( check( program, l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testNew ) != 0 );

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
			TEST_ERR_IF( trotListRemove( program, l, addingAtIndexB ) != TROT_RC_SUCCESS );
		}
		else if ( removeSpecificOrGeneric == TEST_REMOVE_SPECIFIC_KIND )
		{
			if ( intsOrLists == TEST_ADDING_INTS )
			{
				TEST_ERR_IF( trotListRemoveInt( program, l, addingAtIndexB, &removedN ) != TROT_RC_SUCCESS );
				TEST_ERR_IF( removedN != newNumber );
			}
			else if ( intsOrLists == TEST_ADDING_LISTS )
			{
				TEST_ERR_IF( trotListRemoveList( program, l, addingAtIndexB, &removedL ) != TROT_RC_SUCCESS );
				TEST_ERR_IF( trotListGetInt( program, removedL, 1, &removedN ) != TROT_RC_SUCCESS );
				TEST_ERR_IF( removedN != newNumber );

				trotListFree( program, &removedL );
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
		TEST_ERR_IF( checkList( program, l ) != 0 );

		index = 1;
		testNew = 1000;
		testOriginal = 0;
		while ( index <= countAtStart )
		{
			testOriginal += 1;

			TEST_ERR_IF( check( program, l, index, testOriginal ) != 0 );
			TEST_ERR_IF( check( program, l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testOriginal ) != 0 );

			index += 1;
		}
		while ( index <= (countAtStart + countAdded) )
		{
			testNew += 1;

			TEST_ERR_IF( check( program, l, index, testNew ) != 0 );
			TEST_ERR_IF( check( program, l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testNew ) != 0 );

			index += 1;
		}

		TEST_ERR_IF( testOriginal != countAtStart );
		TEST_ERR_IF( (testNew - 1000) != countAdded );
	}

	TEST_ERR_IF( trotListGetCount( program, l, &countAtEnd ) != 0 );
	TEST_ERR_IF( countAtEnd != countAtStart );

	return 0;
	

	/* CLEANUP */
	cleanup:

	printf( "\x1b[31;1mFAILED AT %d testNew:%d testOriginal:%d index:%d addingAtIndex:%d\n\x1b[0m", rc, testNew, testOriginal, index, addingAtIndex );

	return rc;
}

/******************************************************************************/
static int testAddToMiddle( TrotProgram *program, TrotList *l, int intsOrLists, int removeSpecificOrGeneric, int positiveOrNegativeIndices )
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
	TEST_ERR_IF( trotListGetCount( program, l, &countAtStart ) != TROT_RC_SUCCESS );

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
			TEST_ERR_IF( trotListInsertInt( program, l, addingAtIndexB, newNumber ) != TROT_RC_SUCCESS );
		}
		else if ( intsOrLists == TEST_ADDING_LISTS )
		{
			TEST_ERR_IF( addListWithValue( program, l, addingAtIndexB, newNumber ) != 0 );
		}
		else
		{
			TEST_ERR_IF( 1 );
		}

		countAdded += 1;

		/* check */
		TEST_ERR_IF( checkList( program, l ) != 0 );

		index = 1;
		testNew = 1000;
		testOriginal = 0;
		while ( index < startedAddingAtIndex )
		{
			testOriginal += 1;

			TEST_ERR_IF( check( program, l, index, testOriginal ) != 0 );
			TEST_ERR_IF( check( program, l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testOriginal ) != 0 );

			index += 1;
		}
		while ( index < startedAddingAtIndex + countAdded )
		{
			testNew += 1;

			TEST_ERR_IF( check( program, l, index, testNew ) != 0 );
			TEST_ERR_IF( check( program, l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testNew ) != 0 );

			index += 1;
		}
		while ( index <= (countAtStart + countAdded) )
		{
			testOriginal += 1;

			TEST_ERR_IF( check( program, l, index, testOriginal ) != 0 );
			TEST_ERR_IF( check( program, l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testOriginal ) != 0 );

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
			TEST_ERR_IF( trotListRemove( program, l, addingAtIndexB ) != TROT_RC_SUCCESS );
		}
		else if ( removeSpecificOrGeneric == TEST_REMOVE_SPECIFIC_KIND )
		{
			if ( intsOrLists == TEST_ADDING_INTS )
			{
				TEST_ERR_IF( trotListRemoveInt( program, l, addingAtIndexB, &removedN ) != TROT_RC_SUCCESS );
				TEST_ERR_IF( removedN != newNumber );
			}
			else if ( intsOrLists == TEST_ADDING_LISTS )
			{
				TEST_ERR_IF( trotListRemoveList( program, l, addingAtIndexB, &removedL ) != TROT_RC_SUCCESS );
				TEST_ERR_IF( trotListGetInt( program, removedL, 1, &removedN ) != TROT_RC_SUCCESS );
				TEST_ERR_IF( removedN != newNumber );

				trotListFree( program, &removedL );
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
		TEST_ERR_IF( checkList( program, l ) != 0 );

		index = 1;
		testNew = 1000;
		testOriginal = 0;
		while ( index < startedAddingAtIndex )
		{
			testOriginal += 1;

			TEST_ERR_IF( check( program, l, index, testOriginal ) != 0 );
			TEST_ERR_IF( check( program, l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testOriginal ) != 0 );

			index += 1;
		}
		while ( index < startedAddingAtIndex + countAdded )
		{
			testNew += 1;

			TEST_ERR_IF( check( program, l, index, testNew ) != 0 );
			TEST_ERR_IF( check( program, l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testNew ) != 0 );

			index += 1;
		}
		while ( index <= (countAtStart + countAdded) )
		{
			testOriginal += 1;

			TEST_ERR_IF( check( program, l, index, testOriginal ) != 0 );
			TEST_ERR_IF( check( program, l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testOriginal ) != 0 );

			index += 1;
		}

		TEST_ERR_IF( testOriginal != countAtStart );
		TEST_ERR_IF( (testNew - 1000) != countAdded );
	}

	TEST_ERR_IF( trotListGetCount( program, l, &countAtEnd ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( countAtEnd != countAtStart );

	return 0;
	

	/* CLEANUP */
	cleanup:

	printf( "\x1b[31;1mFAILED AT %d testNew:%d testOriginal:%d index:%d addingAtIndex:%d\n\x1b[0m", rc, testNew, testOriginal, index, addingAtIndex );

	return rc;
}

/******************************************************************************/
static int testAddAtOddIndices( TrotProgram *program, TrotList *l, int intsOrLists, int removeSpecificOrGeneric, int positiveOrNegativeIndices )
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
	TEST_ERR_IF( trotListGetCount( program, l, &countAtStart ) != TROT_RC_SUCCESS );

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
			TEST_ERR_IF( trotListInsertInt( program, l, addingAtIndexB, newNumber ) != TROT_RC_SUCCESS );
		}
		else if ( intsOrLists == TEST_ADDING_LISTS )
		{
			TEST_ERR_IF( addListWithValue( program, l, addingAtIndexB, newNumber ) != 0 );
		}
		else
		{
			TEST_ERR_IF( 1 );
		}

		countAdded += 1;

		/* check */
		TEST_ERR_IF( checkList( program, l ) != 0 );

		index = 1;
		testNew = 1000;
		testOriginal = 0;
		while ( index <= ( ( countAdded * 2 - 1 ) ) )
		{
			testNew += 1;

			TEST_ERR_IF( check( program, l, index, testNew ) != 0 );
			TEST_ERR_IF( check( program, l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testNew ) != 0 );

			index += 1;

			if ( index > ( ( countAdded * 2 ) - 1 ) )
			{
				break;
			}

			testOriginal += 1;

			TEST_ERR_IF( check( program, l, index, testOriginal ) != 0 );
			TEST_ERR_IF( check( program, l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testOriginal ) != 0 );

			index += 1;
		}
		while ( index <= countAtStart + countAdded )
		{
			testOriginal += 1;

			TEST_ERR_IF( check( program, l, index, testOriginal ) != 0 );
			TEST_ERR_IF( check( program, l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testOriginal ) != 0 );

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
			TEST_ERR_IF( trotListRemove( program, l, addingAtIndexB ) != TROT_RC_SUCCESS );
		}
		else if ( removeSpecificOrGeneric == TEST_REMOVE_SPECIFIC_KIND )
		{
			if ( intsOrLists == TEST_ADDING_INTS )
			{
				TEST_ERR_IF( trotListRemoveInt( program, l, addingAtIndexB, &removedN ) != TROT_RC_SUCCESS );
				TEST_ERR_IF( removedN != newNumber );
			}
			else if ( intsOrLists == TEST_ADDING_LISTS )
			{
				TEST_ERR_IF( trotListRemoveList( program, l, addingAtIndexB, &removedL ) != TROT_RC_SUCCESS );
				TEST_ERR_IF( trotListGetInt( program, removedL, 1, &removedN ) != TROT_RC_SUCCESS );
				TEST_ERR_IF( removedN != newNumber );

				trotListFree( program, &removedL );
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
		TEST_ERR_IF( checkList( program, l ) != 0 );

		index = 1;
		testNew = 1000;
		testOriginal = 0;
		while ( index <= ( ( countAdded * 2 - 1 ) ) )
		{
			testNew += 1;

			TEST_ERR_IF( check( program, l, index, testNew ) != 0 );
			TEST_ERR_IF( check( program, l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testNew ) != 0 );

			index += 1;

			if ( index > ( ( countAdded * 2 ) - 1 ) )
			{
				break;
			}

			testOriginal += 1;

			TEST_ERR_IF( check( program, l, index, testOriginal ) != 0 );
			TEST_ERR_IF( check( program, l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testOriginal ) != 0 );

			index += 1;
		}
		while ( index <= countAtStart + countAdded )
		{
			testOriginal += 1;

			TEST_ERR_IF( check( program, l, index, testOriginal ) != 0 );
			TEST_ERR_IF( check( program, l, INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, countAtStart + countAdded ), testOriginal ) != 0 );

			index += 1;
		}

		TEST_ERR_IF( testOriginal != countAtStart );
		TEST_ERR_IF( (testNew - 1000) != countAdded );
	}

	TEST_ERR_IF( trotListGetCount( program, l, &countAtEnd ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( countAtEnd != countAtStart );

	return 0;
	

	/* CLEANUP */
	cleanup:

	printf( "\x1b[31;1mFAILED AT %d testNew:%d testOriginal:%d index:%d addingAtIndex:%d\n\x1b[0m", rc, testNew, testOriginal, index, addingAtIndex );

	return rc;
}

/******************************************************************************/
static int testReplace( TrotProgram *program, TrotList *l, int intsOrLists, int positiveOrNegativeIndices )
{
	/* DATA */
	int rc = 0;

	TROT_INT countAtStart = 0;
	TROT_INT countAfter = 0;

	TROT_INT addingAtIndex = 0;
	TROT_INT addingAtIndexB = 0; /* this may be the same, or it may be the negative version */

	TROT_INT kind = 0;

	TrotList *newL = NULL;

	TROT_INT index = 0;


	/* CODE */
	TEST_ERR_IF( trotListGetCount( program, l, &countAtStart ) != TROT_RC_SUCCESS );

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

		TEST_ERR_IF( trotListGetKind( program, l, addingAtIndexB, &kind ) != TROT_RC_SUCCESS );

		if ( intsOrLists == TEST_ADDING_INTS )
		{
			TEST_ERR_IF( trotListReplaceWithInt( program, l, addingAtIndexB, -1 ) != TROT_RC_SUCCESS )
		}
		else if ( intsOrLists == TEST_ADDING_LISTS )
		{
			TEST_ERR_IF( trotListInit( program, &newL ) != TROT_RC_SUCCESS );
			TEST_ERR_IF( trotListAppendInt( program, newL, -1 ) != TROT_RC_SUCCESS );
			TEST_ERR_IF( trotListReplaceWithList( program, l, addingAtIndexB, newL ) != TROT_RC_SUCCESS );
			trotListFree( program, &newL );
		}
		else
		{
			TEST_ERR_IF( 1 );
		}

		/* check */
		TEST_ERR_IF( checkList( program, l ) != 0 );

		TEST_ERR_IF( trotListGetCount( program, l, &countAfter ) != TROT_RC_SUCCESS );
		TEST_ERR_IF( countAfter != countAtStart );

		index = 1;
		while ( index <= countAtStart )
		{
			if ( index == addingAtIndex )
			{
				TEST_ERR_IF( check( program, l, index, -1 ) != 0 );
			}
			else
			{
				TEST_ERR_IF( check( program, l, index, index ) != 0 );
			}

			index += 1;
		}

		/* replace back to original */
		if ( kind == TROT_KIND_INT )
		{
			TEST_ERR_IF( trotListReplaceWithInt( program, l, addingAtIndexB, addingAtIndex ) != TROT_RC_SUCCESS )
		}
		else
		{
			TEST_ERR_IF( trotListInit( program, &newL ) != TROT_RC_SUCCESS );
			TEST_ERR_IF( trotListAppendInt( program, newL, addingAtIndex ) != TROT_RC_SUCCESS );
			TEST_ERR_IF( trotListReplaceWithList( program, l, addingAtIndexB, newL ) != TROT_RC_SUCCESS );
			trotListFree( program, &newL );
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

