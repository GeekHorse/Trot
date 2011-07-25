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
#include "trotListInternal.h"

#include "testCommon.h"

#include "stdio.h"
#include "time.h"
#include "string.h"

/******************************************************************************/
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
		NULL
	};

/******************************************************************************/
/* test functions */
static int testPrepend( trotListRef *lr, int intsOrLists, int removeSpecificOrGeneric, int positiveOrNegativeIndices );
static int testAppend( trotListRef *lr, int intsOrLists, int removeSpecificOrGeneric, int positiveOrNegativeIndices );
static int testAddToMiddle( trotListRef *lr, int intsOrLists, int removeSpecificOrGeneric, int positiveOrNegativeIndices );
static int testAddAtOddIndices( trotListRef *lr, int intsOrLists, int removeSpecificOrGeneric, int positiveOrNegativeIndices );

static int (*testFunctions[])( trotListRef *, int, int, int ) = 
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

	trotListRef *lr = NULL;


	/* CODE */
	printf( "Testing primary functionality..." ); fflush( stdout );
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
				trotListRefFree( &lr );

				TEST_ERR_IF( createFunctions[ i ]( &lr, count ) != 0 );
				TEST_ERR_IF( testFunctions[ j ]( lr, TEST_ADDING_INTS, TEST_REMOVE_SPECIFIC_KIND, TEST_NEGATIVE_INDICES ) != 0 );
				trotListRefFree( &lr );

				TEST_ERR_IF( createFunctions[ i ]( &lr, count ) != 0 );
				TEST_ERR_IF( testFunctions[ j ]( lr, TEST_ADDING_INTS, TEST_REMOVE_GENERIC, TEST_POSITIVE_INDICES ) != 0 );
				trotListRefFree( &lr );

				TEST_ERR_IF( createFunctions[ i ]( &lr, count ) != 0 );
				TEST_ERR_IF( testFunctions[ j ]( lr, TEST_ADDING_INTS, TEST_REMOVE_GENERIC, TEST_NEGATIVE_INDICES ) != 0 );
				trotListRefFree( &lr );

				/* test adding lists */
				TEST_ERR_IF( createFunctions[ i ]( &lr, count ) != 0 );
				TEST_ERR_IF( testFunctions[ j ]( lr, TEST_ADDING_LISTS, TEST_REMOVE_SPECIFIC_KIND, TEST_POSITIVE_INDICES ) != 0 );
				trotListRefFree( &lr );

				TEST_ERR_IF( createFunctions[ i ]( &lr, count ) != 0 );
				TEST_ERR_IF( testFunctions[ j ]( lr, TEST_ADDING_LISTS, TEST_REMOVE_SPECIFIC_KIND, TEST_NEGATIVE_INDICES ) != 0 );
				trotListRefFree( &lr );

				TEST_ERR_IF( createFunctions[ i ]( &lr, count ) != 0 );
				TEST_ERR_IF( testFunctions[ j ]( lr, TEST_ADDING_LISTS, TEST_REMOVE_GENERIC, TEST_POSITIVE_INDICES ) != 0 );
				trotListRefFree( &lr );

				TEST_ERR_IF( createFunctions[ i ]( &lr, count ) != 0 );
				TEST_ERR_IF( testFunctions[ j ]( lr, TEST_ADDING_LISTS, TEST_REMOVE_GENERIC, TEST_NEGATIVE_INDICES ) != 0 );
				trotListRefFree( &lr );

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
static int testPrepend( trotListRef *lr, int intsOrLists, int removeSpecificOrGeneric, int positiveOrNegativeIndices )
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
	trotListRef *removedL = NULL;


	/* CODE */
	TEST_ERR_IF( trotListRefGetCount( lr, &countAtStart ) != 0 );

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
			TEST_ERR_IF( trotListRefInsertInt( lr, addingAtIndexB, newNumber ) != 0 );
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
			TEST_ERR_IF( trotListRefRemove( lr, addingAtIndexB ) != 0 );
		}
		else if ( removeSpecificOrGeneric == TEST_REMOVE_SPECIFIC_KIND )
		{
			if ( intsOrLists == TEST_ADDING_INTS )
			{
				TEST_ERR_IF( trotListRefRemoveInt( lr, addingAtIndexB, &removedN ) != 0 );
				TEST_ERR_IF( removedN != newNumber );
			}
			else if ( intsOrLists == TEST_ADDING_LISTS )
			{
				TEST_ERR_IF( trotListRefRemoveList( lr, addingAtIndexB, &removedL ) != 0 );
				TEST_ERR_IF( trotListRefGetInt( removedL, 1, &removedN ) != 0 );
				TEST_ERR_IF( removedN != newNumber );

				trotListRefFree( &removedL );
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

	TEST_ERR_IF( trotListRefGetCount( lr, &countAtEnd ) != 0 );
	TEST_ERR_IF( countAtEnd != countAtStart );

	return 0;
	

	/* CLEANUP */
	cleanup:

	printList( lr );
	printf( "\x1b[31;1mFAILED AT %d testNew:%d testOriginal:%d index:%d addingAtIndex:%d\n\x1b[0m", rc, testNew, testOriginal, index, addingAtIndex );

	return rc;
}

/******************************************************************************/
static int testAppend( trotListRef *lr, int intsOrLists, int removeSpecificOrGeneric, int positiveOrNegativeIndices )
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
	trotListRef *removedL = NULL;


	/* CODE */
	TEST_ERR_IF( trotListRefGetCount( lr, &countAtStart ) != 0 );

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
			TEST_ERR_IF( trotListRefInsertInt( lr, addingAtIndexB, newNumber ) != 0 );
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
			TEST_ERR_IF( trotListRefRemove( lr, addingAtIndexB ) != 0 );
		}
		else if ( removeSpecificOrGeneric == TEST_REMOVE_SPECIFIC_KIND )
		{
			if ( intsOrLists == TEST_ADDING_INTS )
			{
				TEST_ERR_IF( trotListRefRemoveInt( lr, addingAtIndexB, &removedN ) != 0 );
				TEST_ERR_IF( removedN != newNumber );
			}
			else if ( intsOrLists == TEST_ADDING_LISTS )
			{
				TEST_ERR_IF( trotListRefRemoveList( lr, addingAtIndexB, &removedL ) != 0 );
				TEST_ERR_IF( trotListRefGetInt( removedL, 1, &removedN ) != 0 );
				TEST_ERR_IF( removedN != newNumber );

				trotListRefFree( &removedL );
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

	TEST_ERR_IF( trotListRefGetCount( lr, &countAtEnd ) != 0 );
	TEST_ERR_IF( countAtEnd != countAtStart );

	return 0;
	

	/* CLEANUP */
	cleanup:

	printList( lr );
	printf( "\x1b[31;1mFAILED AT %d testNew:%d testOriginal:%d index:%d addingAtIndex:%d\n\x1b[0m", rc, testNew, testOriginal, index, addingAtIndex );

	return rc;
}

/******************************************************************************/
static int testAddToMiddle( trotListRef *lr, int intsOrLists, int removeSpecificOrGeneric, int positiveOrNegativeIndices )
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
	trotListRef *removedL = NULL;

	INT_TYPE startedAddingAtIndex = 0;


	/* CODE */
	TEST_ERR_IF( trotListRefGetCount( lr, &countAtStart ) != 0 );

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
			TEST_ERR_IF( trotListRefInsertInt( lr, addingAtIndexB, newNumber ) != 0 );
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
			TEST_ERR_IF( trotListRefRemove( lr, addingAtIndexB ) != 0 );
		}
		else if ( removeSpecificOrGeneric == TEST_REMOVE_SPECIFIC_KIND )
		{
			if ( intsOrLists == TEST_ADDING_INTS )
			{
				TEST_ERR_IF( trotListRefRemoveInt( lr, addingAtIndexB, &removedN ) != 0 );
				TEST_ERR_IF( removedN != newNumber );
			}
			else if ( intsOrLists == TEST_ADDING_LISTS )
			{
				TEST_ERR_IF( trotListRefRemoveList( lr, addingAtIndexB, &removedL ) != 0 );
				TEST_ERR_IF( trotListRefGetInt( removedL, 1, &removedN ) != 0 );
				TEST_ERR_IF( removedN != newNumber );

				trotListRefFree( &removedL );
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

	TEST_ERR_IF( trotListRefGetCount( lr, &countAtEnd ) != 0 );
	TEST_ERR_IF( countAtEnd != countAtStart );

	return 0;
	

	/* CLEANUP */
	cleanup:

	printList( lr );
	printf( "\x1b[31;1mFAILED AT %d testNew:%d testOriginal:%d index:%d addingAtIndex:%d\n\x1b[0m", rc, testNew, testOriginal, index, addingAtIndex );

	return rc;
}

/******************************************************************************/
static int testAddAtOddIndices( trotListRef *lr, int intsOrLists, int removeSpecificOrGeneric, int positiveOrNegativeIndices )
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
	trotListRef *removedL = NULL;


	/* CODE */
	TEST_ERR_IF( trotListRefGetCount( lr, &countAtStart ) != 0 );

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
			TEST_ERR_IF( trotListRefInsertInt( lr, addingAtIndexB, newNumber ) != 0 );
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
			TEST_ERR_IF( trotListRefRemove( lr, addingAtIndexB ) != 0 );
		}
		else if ( removeSpecificOrGeneric == TEST_REMOVE_SPECIFIC_KIND )
		{
			if ( intsOrLists == TEST_ADDING_INTS )
			{
				TEST_ERR_IF( trotListRefRemoveInt( lr, addingAtIndexB, &removedN ) != 0 );
				TEST_ERR_IF( removedN != newNumber );
			}
			else if ( intsOrLists == TEST_ADDING_LISTS )
			{
				TEST_ERR_IF( trotListRefRemoveList( lr, addingAtIndexB, &removedL ) != 0 );
				TEST_ERR_IF( trotListRefGetInt( removedL, 1, &removedN ) != 0 );
				TEST_ERR_IF( removedN != newNumber );

				trotListRefFree( &removedL );
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

	TEST_ERR_IF( trotListRefGetCount( lr, &countAtEnd ) != 0 );
	TEST_ERR_IF( countAtEnd != countAtStart );

	return 0;
	

	/* CLEANUP */
	cleanup:

	printList( lr );
	printf( "\x1b[31;1mFAILED AT %d testNew:%d testOriginal:%d index:%d addingAtIndex:%d\n\x1b[0m", rc, testNew, testOriginal, index, addingAtIndex );

	return rc;
}

