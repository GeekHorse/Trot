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

/******************************************************************************/
int addListWithValue( trotListRef *lr, INT_TYPE index, INT_TYPE value )
{
	/* DATA */
	int rc = 0;

	trotListRef *newList = NULL;


	/* CODE */
	TEST_ERR_IF( trotListRefInit( &newList ) != 0 );
	TEST_ERR_IF( trotListRefAppendInt( newList, value ) != 0 );
	TEST_ERR_IF( trotListRefInsertListTwin( lr, index, newList ) != 0 );


	/* CLEANUP */
	cleanup:

	trotListRefFree( &newList );

	return rc;
}

/******************************************************************************/
int createAllInts( trotListRef **lr, int count )
{
	/* DATA */
	int rc = 0;

	trotListRef *newList = NULL;

	INT_TYPE i = 1;

	/* CODE */
	TEST_ERR_IF( trotListRefInit( &newList ) != 0 );

	i = 1;
	while ( i <= count )
	{
		TEST_ERR_IF( trotListRefAppendInt( newList, i ) != 0 );

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

	trotListRefFree( &newList );

	return rc;
}

/******************************************************************************/
int createAllLists( trotListRef **lr, int count )
{
	/* DATA */
	int rc = 0;

	trotListRef *newList = NULL;

	INT_TYPE i = 1;

	/* CODE */
	TEST_ERR_IF( trotListRefInit( &newList ) != 0 );

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

	trotListRefFree( &newList );

	return rc;
}

/******************************************************************************/
int createIntListAlternating( trotListRef **lr, int count )
{
	/* DATA */
	int rc = 0;

	trotListRef *newList = NULL;

	INT_TYPE i = 1;

	/* CODE */
	TEST_ERR_IF( trotListRefInit( &newList ) != 0 );

	i = 1;
	while ( i <= count )
	{
		if ( i % 2 == 1 )
		{
			TEST_ERR_IF( trotListRefAppendInt( newList, i ) != 0 );
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

	trotListRefFree( &newList );

	return rc;
}

/******************************************************************************/
int createListIntAlternating( trotListRef **lr, int count )
{
	/* DATA */
	int rc = 0;

	trotListRef *newList = NULL;

	INT_TYPE i = 1;

	/* CODE */
	TEST_ERR_IF( trotListRefInit( &newList ) != 0 );

	i = 1;
	while ( i <= count )
	{
		if ( i % 2 == 1 )
		{
			TEST_ERR_IF( addListWithValue( newList, i, i ) != 0 );
		}
		else
		{
			TEST_ERR_IF( trotListRefAppendInt( newList, i ) != 0 );
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

	trotListRefFree( &newList );

	return rc;
}

/******************************************************************************/
int createHalfIntHalfList( trotListRef **lr, int count )
{
	/* DATA */
	int rc = 0;

	trotListRef *newList = NULL;

	INT_TYPE i = 1;

	/* CODE */
	TEST_ERR_IF( trotListRefInit( &newList ) != 0 );

	i = 1;
	while ( i <= ( count / 2 ) )
	{
		TEST_ERR_IF( trotListRefAppendInt( newList, i ) != 0 );

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

	trotListRefFree( &newList );

	return rc;
}

/******************************************************************************/
int createHalfListHalfInt( trotListRef **lr, int count )
{
	/* DATA */
	int rc = 0;

	trotListRef *newList = NULL;

	INT_TYPE i = 1;

	/* CODE */
	TEST_ERR_IF( trotListRefInit( &newList ) != 0 );

	i = 1;
	while ( i <= ( count / 2 ) )
	{
		TEST_ERR_IF( addListWithValue( newList, i, i ) != 0 );

		i += 1;
	}
	while ( i <= count )
	{
		TEST_ERR_IF( trotListRefAppendInt( newList, i ) != 0 );

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

	trotListRefFree( &newList );

	return rc;
}

/******************************************************************************/
int check( trotListRef *lr, INT_TYPE index, INT_TYPE valueToCheckAgainst )
{
	/* DATA */
	int rc = 0;

	int kind = 0;

	INT_TYPE valueInList = 0;

	trotListRef *subList = NULL;


	/* CODE */
	TEST_ERR_IF( trotListRefGetKind( lr, index, &kind ) != 0 );

	if ( kind == NODE_KIND_INT )
	{
		TEST_ERR_IF( trotListRefGetInt( lr, index, &valueInList ) != 0 );
		TEST_ERR_IF( valueInList != valueToCheckAgainst );
	}
	else if ( kind == NODE_KIND_LIST )
	{
		TEST_ERR_IF( trotListRefGetListTwin( lr, index, &subList ) != 0 );
		TEST_ERR_IF( trotListRefGetInt( subList, 1, &valueInList ) != 0 );
		TEST_ERR_IF( valueInList != valueToCheckAgainst );
	}
	else
	{
		TEST_ERR_IF( 1 );
	}


	/* CLEANUP */
	cleanup:

	trotListRefFree( &subList );

	return rc;
}

/******************************************************************************/
int checkList( trotListRef *lr )
{
	/* DATA */
	int rc = 0;

	trotList *l = NULL;
	trotListNode *node = NULL;

	trotListRefListNode *refNode = NULL;

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
void printList( trotListRef *lr )
{
	/* DATA */
	trotList *l = lr -> lPointsTo;
	trotListNode *node = l -> head -> next;
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

