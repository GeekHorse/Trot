/*
Copyright (c) 2010,2011,2012, Jeremiah Martell
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
int createSelfRefs( trotListRef **lr, int count )
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
		TEST_ERR_IF( trotListRefAppendListTwin( newList, newList ) != 0 );

		i += 1;
	}

	/* check list */
	TEST_ERR_IF( checkList( newList ) != 0 );

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

	TROT_KIND kind = 0;

	INT_TYPE valueInList = 0;

	trotListRef *subList = NULL;


	/* CODE */
	TEST_ERR_IF( trotListRefGetKind( lr, index, &kind ) != 0 );

	if ( kind == TROT_KIND_INT )
	{
		TEST_ERR_IF( trotListRefGetInt( lr, index, &valueInList ) != 0 );
		TEST_ERR_IF( valueInList != valueToCheckAgainst );
	}
	else if ( kind == TROT_KIND_LIST )
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
	trotList *subL = NULL;
	trotListNode *node = NULL;

	trotListRefListNode *refNode = NULL;
	trotListRefListNode *subRefNode = NULL;

	int i = 0;
	int j = 0;

	int realCount = 0;

	int foundLr = 0;
	int foundRef = 0;


	/* CODE */
	if ( BE_PARANOID == 0 )
	{
		return 0;
	}

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

				foundRef = 0;
				subL = node -> l[ i ] -> lPointsTo;
				subRefNode = subL -> refListHead -> next;
				while ( subRefNode != subL -> refListTail && foundRef == 0 )
				{
					j = 0;
					while ( j < subRefNode -> count )
					{
						if ( subRefNode -> r[ j ] -> lParent == l )
						{
							foundRef = 1;
							break;
						}

						j += 1;
					}

					subRefNode = subRefNode -> next;
				}

				TEST_ERR_IF( foundRef == 0 );

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

	if ( realCount != l -> childrenCount )
	{
		printf( "realCount = (%d)\n", realCount );
		printf( "l -> childrenCount = (%d)\n", l -> childrenCount );
		fflush( stdout );

		TEST_ERR_IF( 1 );
	}

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
static void printIndent( int indent )
{
	int i = 0;
	while ( i < indent ) { printf( "    " ); i += 1; }
	return;
}
	

void printList( trotListRef *lr, int indent )
{
	/* DATA */
	trotList *l = lr -> lPointsTo;
	trotListNode *node = l -> head -> next;
	int i = 0;
	/* trotListRefListNode *refNode = l -> refListHead -> next; */


	/* CODE */
	printIndent( indent );
	printf( "LIST: %p\n", (void *)l );
/*
	printIndent( indent );
	printf( "R " );
	while ( refNode != l -> refListTail )
	{
		i = 0;
		while ( i < refNode -> count )
		{
			printf( "(%p<%p) ", (void *)refNode -> r[ i ] -> lParent, (void *)refNode -> r[ i ] );

			i += 1;
		}

		refNode = refNode -> next;
	}
	printf( "\n" );
*/

	while ( node != l -> tail )
	{
		if ( node -> kind == NODE_KIND_INT )
		{
			printIndent( indent );
			printf( "I " );
			for ( i = 0; i < (node -> count); i += 1 )
			{
				printf( "%d ", node -> n[ i ] );
			}
			printf( "\n" );
		}
		else /* node -> kind == NODE_KIND_LIST */
		{
			printIndent( indent );
			printf( "L\n" );
			for ( i = 0; i < (node -> count); i += 1 )
			{
				printIndent( indent );
				/* printf( "(%p>%p)\n", (void *)node -> l[ i ], (void *)node -> l[ i ] -> lPointsTo ); */
				printList( node -> l[ i ], indent + 1 );
			}
		}

		node = node -> next;
	}

	if ( indent == 0 )
	{
		printf( "\n\n" );
	}

	return;
}

/******************************************************************************/
#define LOAD_BUFFER_SIZE 1024
#define BYTE_TYPE unsigned char
TROT_RC load( trotListRef *lrName, trotListRef **lrBytes )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	size_t bytesRead = 0;
	size_t i = 0;

	char *name = NULL;

	FILE *fp = NULL;

	trotListRef *newLrBytes = NULL;

	BYTE_TYPE buffer[ LOAD_BUFFER_SIZE ];


	/* CODE */
	ERR_IF_PARANOID( sizeof( BYTE_TYPE ) != 1 );

	/* create our new byte list */
	rc = trotListRefInit( &newLrBytes );
	ERR_IF_PASSTHROUGH;

	/* convert our trotListRef name to a cString */
	rc = listToCString( lrName, &name );
	ERR_IF_PASSTHROUGH;

	printf( "Loading: %s\n", name );

	/* open */
	fp = fopen( name, "rb" );
	ERR_IF( fp == NULL, TROT_LIST_ERROR_LOAD );

	/* read all bytes */
	while ( 1 )
	{
		/* read as much as we can into our buffer */
		bytesRead = fread( buffer, 1, LOAD_BUFFER_SIZE, fp );

		/* append what we read to our bytes */
		i = 0;
		while ( i < bytesRead )
		{
			rc = trotListRefAppendInt( newLrBytes, buffer[ i ] );
			ERR_IF_PASSTHROUGH;

			i += 1;
		}

		/* if short read, check error, or break */
		if ( bytesRead < LOAD_BUFFER_SIZE )
		{
			ERR_IF( ! feof( fp ), TROT_LIST_ERROR_LOAD );
			break;
		}
	}

	/* give back */
	(*lrBytes) = newLrBytes;
	newLrBytes = NULL;


	/* CLEANUP */
	cleanup:

	if ( fp != NULL )
	{
		fclose( fp );
	}

	if ( name != NULL )
	{
		trotFree( name );
	}

	trotListRefFree( &newLrBytes );

	return rc;
}
#undef LOAD_BUFFER_SIZE
#undef BYTE_TYPE

/******************************************************************************/
TROT_RC listToCString( trotListRef *lr, char **cString_A )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	char *newCString = NULL;

	INT_TYPE count = 0;
	INT_TYPE i = 1;

	INT_TYPE c = 0;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );
	PRECOND_ERR_IF( cString_A == NULL );
	PRECOND_ERR_IF( (*cString_A) != NULL );


	/* CODE */
	rc = trotListRefGetCount( lr, &count );
	ERR_IF_PASSTHROUGH;

	newCString = trotMalloc( count + 1 );
	ERR_IF( newCString == NULL, TROT_LIST_ERROR_MEMORY_ALLOCATION_FAILED );

	i = 1;
	while ( i <= count )
	{
		rc = trotListRefGetInt( lr, i, &c );
		ERR_IF_PASSTHROUGH;

		ERR_IF( c < 0, TROT_LIST_ERROR_NOT_BYTE_VALUE );
		ERR_IF( c > 255, TROT_LIST_ERROR_NOT_BYTE_VALUE );

		newCString[ i - 1 ] = (char)c;

		i += 1;
	}

	newCString[ i - 1 ] = '\0';

	/* give back */
	(*cString_A) = newCString;
	newCString = NULL;


	/* CLEANUP */
	cleanup:

	if ( newCString != NULL )
	{
		trotFree( newCString );
	}

	return rc;
}

/******************************************************************************/
TROT_RC appendCStringToList( char *cString, trotListRef *lr )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;


	/* PRECOND */
	PRECOND_ERR_IF( cString == NULL );
	PRECOND_ERR_IF( lr == NULL );


	/* CODE */
	while ( *cString != '\0' )
	{
		rc = trotListRefAppendInt( lr, (*cString) );
		ERR_IF_PASSTHROUGH;

		cString += 1;
	}


	/* CLEANUP */
	cleanup:

	return rc;
}

