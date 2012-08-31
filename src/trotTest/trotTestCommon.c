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
int addListWithValue( trotListRef *lr, TROT_INT index, TROT_INT value )
{
	/* DATA */
	int rc = 0;

	trotListRef *newList = NULL;


	/* CODE */
	TEST_ERR_IF( trotListRefInit( &newList ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListRefAppendInt( newList, value ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListRefInsertListTwin( lr, index, newList ) != TROT_RC_SUCCESS );


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

	TROT_INT i = 1;

	/* CODE */
	TEST_ERR_IF( trotListRefInit( &newList ) != TROT_RC_SUCCESS );

	i = 1;
	while ( i <= count )
	{
		TEST_ERR_IF( trotListRefAppendInt( newList, i ) != TROT_RC_SUCCESS );

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

	TROT_INT i = 1;

	/* CODE */
	TEST_ERR_IF( trotListRefInit( &newList ) != TROT_RC_SUCCESS );

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

	TROT_INT i = 1;

	/* CODE */
	TEST_ERR_IF( trotListRefInit( &newList ) != TROT_RC_SUCCESS );

	i = 1;
	while ( i <= count )
	{
		if ( i % 2 == 1 )
		{
			TEST_ERR_IF( trotListRefAppendInt( newList, i ) != TROT_RC_SUCCESS );
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

	TROT_INT i = 1;

	/* CODE */
	TEST_ERR_IF( trotListRefInit( &newList ) != TROT_RC_SUCCESS );

	i = 1;
	while ( i <= count )
	{
		if ( i % 2 == 1 )
		{
			TEST_ERR_IF( addListWithValue( newList, i, i ) != 0 );
		}
		else
		{
			TEST_ERR_IF( trotListRefAppendInt( newList, i ) != TROT_RC_SUCCESS );
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

	TROT_INT i = 1;

	/* CODE */
	TEST_ERR_IF( trotListRefInit( &newList ) != TROT_RC_SUCCESS );

	i = 1;
	while ( i <= ( count / 2 ) )
	{
		TEST_ERR_IF( trotListRefAppendInt( newList, i ) != TROT_RC_SUCCESS );

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

	TROT_INT i = 1;

	/* CODE */
	TEST_ERR_IF( trotListRefInit( &newList ) != TROT_RC_SUCCESS );

	i = 1;
	while ( i <= ( count / 2 ) )
	{
		TEST_ERR_IF( addListWithValue( newList, i, i ) != 0 );

		i += 1;
	}
	while ( i <= count )
	{
		TEST_ERR_IF( trotListRefAppendInt( newList, i ) != TROT_RC_SUCCESS );

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

	TROT_INT i = 1;

	/* CODE */
	TEST_ERR_IF( trotListRefInit( &newList ) != TROT_RC_SUCCESS );

	i = 1;
	while ( i <= count )
	{
		TEST_ERR_IF( trotListRefAppendListTwin( newList, newList ) != TROT_RC_SUCCESS );

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
int check( trotListRef *lr, TROT_INT index, TROT_INT valueToCheckAgainst )
{
	/* DATA */
	int rc = 0;

	TROT_KIND kind = 0;

	TROT_INT valueInList = 0;

	trotListRef *subList = NULL;


	/* CODE */
	TEST_ERR_IF( trotListRefGetKind( lr, index, &kind ) != TROT_RC_SUCCESS );

	if ( kind == TROT_KIND_INT )
	{
		TEST_ERR_IF( trotListRefGetInt( lr, index, &valueInList ) != TROT_RC_SUCCESS );
		TEST_ERR_IF( valueInList != valueToCheckAgainst );
	}
	else if ( kind == TROT_KIND_LIST )
	{
		TEST_ERR_IF( trotListRefGetListTwin( lr, index, &subList ) != TROT_RC_SUCCESS );
		TEST_ERR_IF( trotListRefGetInt( subList, 1, &valueInList ) != TROT_RC_SUCCESS );
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
int load( trotListRef *lrName, trotListRef **lrBytes )
{
	/* DATA */
	int rc = 0;

	size_t bytesRead = 0;
	size_t i = 0;

	char *name = NULL;

	FILE *fp = NULL;

	trotListRef *newLrBytes = NULL;

	BYTE_TYPE buffer[ LOAD_BUFFER_SIZE ];


	/* CODE */
	TEST_ERR_IF( lrName == NULL );
	TEST_ERR_IF( lrBytes == NULL );
	TEST_ERR_IF( (*lrBytes) != NULL );
	TEST_ERR_IF( sizeof( BYTE_TYPE ) != 1 );

	/* create our new byte list */
	TEST_ERR_IF( trotListRefInit( &newLrBytes ) != TROT_RC_SUCCESS );

	/* convert our trotListRef name to a cString */
	TEST_ERR_IF( listToCString( lrName, &name ) != TROT_RC_SUCCESS );

	printf( "Loading: %s\n", name );

	/* open */
	fp = fopen( name, "rb" );
	TEST_ERR_IF( fp == NULL );

	/* read all bytes */
	while ( 1 )
	{
		/* read as much as we can into our buffer */
		bytesRead = fread( buffer, 1, LOAD_BUFFER_SIZE, fp );

		/* append what we read to our bytes */
		i = 0;
		while ( i < bytesRead )
		{
			TEST_ERR_IF( trotListRefAppendInt( newLrBytes, buffer[ i ] ) != TROT_RC_SUCCESS );

			i += 1;
		}

		/* if short read, check error, or break */
		if ( bytesRead < LOAD_BUFFER_SIZE )
		{
			TEST_ERR_IF( ! feof( fp ) );
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
	TROT_RC rc = TROT_RC_SUCCESS;

	char *newCString = NULL;

	TROT_INT count = 0;
	TROT_INT i = 1;

	TROT_INT c = 0;


	/* CODE */
	PARANOID_ERR_IF( lr == NULL );
	PARANOID_ERR_IF( cString_A == NULL );
	PARANOID_ERR_IF( (*cString_A) != NULL );

	rc = trotListRefGetCount( lr, &count );
	ERR_IF_PASSTHROUGH;

	TROT_MALLOC( newCString, char, count + 1 );

	i = 1;
	while ( i <= count )
	{
		rc = trotListRefGetInt( lr, i, &c );
		ERR_IF_PASSTHROUGH;

		PARANOID_ERR_IF( c < 0 );
		PARANOID_ERR_IF( c > 255 );

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
TROT_RC appendCStringToList( trotListRef *lr, char *cString )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;


	/* CODE */
	PARANOID_ERR_IF( lr == NULL );
	PARANOID_ERR_IF( cString == NULL );

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

