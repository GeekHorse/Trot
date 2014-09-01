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
#define TROT_FILE_NUMBER 501

/******************************************************************************/
#include <string.h> /* strlen */

#include "trot.h"
#include "trotInternal.h"

#include "trotTestCommon.h"

/******************************************************************************/
int addListWithValue( TrotList *lMemLimit, TrotList *l, TROT_INT index, TROT_INT value )
{
	/* DATA */
	int rc = 0;

	TrotList *newList = NULL;


	/* CODE */
	TEST_ERR_IF( trotListInit( lMemLimit, &newList ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListAppendInt( lMemLimit, newList, value ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListInsertList( lMemLimit, l, index, newList ) != TROT_RC_SUCCESS );


	/* CLEANUP */
	cleanup:

	trotListFree( lMemLimit, &newList );

	return rc;
}

/******************************************************************************/
int createAllInts( TrotList *lMemLimit, TrotList **l, int count )
{
	/* DATA */
	int rc = 0;

	TrotList *newList = NULL;

	TROT_INT i = 1;

	/* CODE */
	TEST_ERR_IF( trotListInit( lMemLimit, &newList ) != TROT_RC_SUCCESS );

	i = 1;
	while ( i <= count )
	{
		TEST_ERR_IF( trotListAppendInt( lMemLimit, newList, i ) != TROT_RC_SUCCESS );

		i += 1;
	}

	/* check list */
	TEST_ERR_IF( checkList( lMemLimit, newList ) != 0 );

	i = 1;
	while ( i <= count )
	{
		TEST_ERR_IF( check( lMemLimit, newList, i, i ) != 0 );
		i += 1;
	}

	/* give back */
	(*l) = newList;
	newList = NULL;


	/* CLEANUP */
	cleanup:

	trotListFree( lMemLimit, &newList );

	return rc;
}

/******************************************************************************/
int createAllLists( TrotList *lMemLimit, TrotList **l, int count )
{
	/* DATA */
	int rc = 0;

	TrotList *newList = NULL;

	TROT_INT i = 1;

	/* CODE */
	TEST_ERR_IF( trotListInit( lMemLimit, &newList ) != TROT_RC_SUCCESS );

	i = 1;
	while ( i <= count )
	{
		TEST_ERR_IF( addListWithValue( lMemLimit, newList, i, i ) != 0 );

		i += 1;
	}

	/* check list */
	TEST_ERR_IF( checkList( lMemLimit, newList ) != 0 );

	i = 1;
	while ( i <= count )
	{
		TEST_ERR_IF( check( lMemLimit, newList, i, i ) != 0 );
		i += 1;
	}

	/* give back */
	(*l) = newList;
	newList = NULL;


	/* CLEANUP */
	cleanup:

	trotListFree( lMemLimit, &newList );

	return rc;
}

/******************************************************************************/
int createIntListAlternating( TrotList *lMemLimit, TrotList **l, int count )
{
	/* DATA */
	int rc = 0;

	TrotList *newList = NULL;

	TROT_INT i = 1;

	/* CODE */
	TEST_ERR_IF( trotListInit( lMemLimit, &newList ) != TROT_RC_SUCCESS );

	i = 1;
	while ( i <= count )
	{
		if ( i % 2 == 1 )
		{
			TEST_ERR_IF( trotListAppendInt( lMemLimit, newList, i ) != TROT_RC_SUCCESS );
		}
		else
		{
			TEST_ERR_IF( addListWithValue( lMemLimit, newList, i, i ) != 0 );
		}

		i += 1;
	}

	/* check list */
	TEST_ERR_IF( checkList( lMemLimit, newList ) != 0 );

	i = 1;
	while ( i <= count )
	{
		TEST_ERR_IF( check( lMemLimit, newList, i, i ) != 0 );
		i += 1;
	}

	/* give back */
	(*l) = newList;
	newList = NULL;


	/* CLEANUP */
	cleanup:

	trotListFree( lMemLimit, &newList );

	return rc;
}

/******************************************************************************/
int createListIntAlternating( TrotList *lMemLimit, TrotList **l, int count )
{
	/* DATA */
	int rc = 0;

	TrotList *newList = NULL;

	TROT_INT i = 1;

	/* CODE */
	TEST_ERR_IF( trotListInit( lMemLimit, &newList ) != TROT_RC_SUCCESS );

	i = 1;
	while ( i <= count )
	{
		if ( i % 2 == 1 )
		{
			TEST_ERR_IF( addListWithValue( lMemLimit, newList, i, i ) != 0 );
		}
		else
		{
			TEST_ERR_IF( trotListAppendInt( lMemLimit, newList, i ) != TROT_RC_SUCCESS );
		}

		i += 1;
	}

	/* check list */
	TEST_ERR_IF( checkList( lMemLimit, newList ) != 0 );

	i = 1;
	while ( i <= count )
	{
		TEST_ERR_IF( check( lMemLimit, newList, i, i ) != 0 );
		i += 1;
	}

	/* give back */
	(*l) = newList;
	newList = NULL;


	/* CLEANUP */
	cleanup:

	trotListFree( lMemLimit, &newList );

	return rc;
}

/******************************************************************************/
int createHalfIntHalfList( TrotList *lMemLimit, TrotList **l, int count )
{
	/* DATA */
	int rc = 0;

	TrotList *newList = NULL;

	TROT_INT i = 1;

	/* CODE */
	TEST_ERR_IF( trotListInit( lMemLimit, &newList ) != TROT_RC_SUCCESS );

	i = 1;
	while ( i <= ( count / 2 ) )
	{
		TEST_ERR_IF( trotListAppendInt( lMemLimit, newList, i ) != TROT_RC_SUCCESS );

		i += 1;
	}
	while ( i <= count )
	{
		TEST_ERR_IF( addListWithValue( lMemLimit, newList, i, i ) != 0 );

		i += 1;
	}


	/* check list */
	TEST_ERR_IF( checkList( lMemLimit, newList ) != 0 );

	i = 1;
	while ( i <= count )
	{
		TEST_ERR_IF( check( lMemLimit, newList, i, i ) != 0 );
		i += 1;
	}

	/* give back */
	(*l) = newList;
	newList = NULL;


	/* CLEANUP */
	cleanup:

	trotListFree( lMemLimit, &newList );

	return rc;
}

/******************************************************************************/
int createHalfListHalfInt( TrotList *lMemLimit, TrotList **l, int count )
{
	/* DATA */
	int rc = 0;

	TrotList *newList = NULL;

	TROT_INT i = 1;

	/* CODE */
	TEST_ERR_IF( trotListInit( lMemLimit, &newList ) != TROT_RC_SUCCESS );

	i = 1;
	while ( i <= ( count / 2 ) )
	{
		TEST_ERR_IF( addListWithValue( lMemLimit, newList, i, i ) != 0 );

		i += 1;
	}
	while ( i <= count )
	{
		TEST_ERR_IF( trotListAppendInt( lMemLimit, newList, i ) != TROT_RC_SUCCESS );

		i += 1;
	}


	/* check list */
	TEST_ERR_IF( checkList( lMemLimit, newList ) != 0 );

	i = 1;
	while ( i <= count )
	{
		TEST_ERR_IF( check( lMemLimit, newList, i, i ) != 0 );
		i += 1;
	}

	/* give back */
	(*l) = newList;
	newList = NULL;


	/* CLEANUP */
	cleanup:

	trotListFree( lMemLimit, &newList );

	return rc;
}

/******************************************************************************/
int check( TrotList *lMemLimit, TrotList *l, TROT_INT index, TROT_INT valueToCheckAgainst )
{
	/* DATA */
	int rc = 0;

	TROT_INT kind = 0;

	TROT_INT valueInList = 0;

	TrotList *subList = NULL;


	/* CODE */
	TEST_ERR_IF( trotListGetKind( lMemLimit, l, index, &kind ) != TROT_RC_SUCCESS );

	if ( kind == TROT_KIND_INT )
	{
		TEST_ERR_IF( trotListGetInt( lMemLimit, l, index, &valueInList ) != TROT_RC_SUCCESS );
		TEST_ERR_IF( valueInList != valueToCheckAgainst );
	}
	else if ( kind == TROT_KIND_LIST )
	{
		TEST_ERR_IF( trotListGetList( lMemLimit, l, index, &subList ) != TROT_RC_SUCCESS );
		TEST_ERR_IF( trotListGetInt( lMemLimit, subList, 1, &valueInList ) != TROT_RC_SUCCESS );
		TEST_ERR_IF( valueInList != valueToCheckAgainst );
	}
	else
	{
		TEST_ERR_IF( 1 );
	}


	/* CLEANUP */
	cleanup:

	trotListFree( lMemLimit, &subList );

	return rc;
}

/******************************************************************************/
int checkList( TrotList *lMemLimit, TrotList *l )
{
	/* DATA */
	int rc = 0;

	TrotListActual *la = NULL;
	TrotListActual *subLa = NULL;
	TrotListNode *node = NULL;

	TrotListRefListNode *refNode = NULL;
	TrotListRefListNode *subRefNode = NULL;

	int i = 0;
	int j = 0;

	int realCount = 0;

	int foundLr = 0;
	int foundRef = 0;


	/* CODE */
	#ifndef BE_PARANOID
	return 0;
	#endif
	(void)lMemLimit;

	TEST_ERR_IF( l == NULL );

	la = l->laPointsTo;
	TEST_ERR_IF( la == NULL );
	TEST_ERR_IF( la->childrenCount < 0 );

	TEST_ERR_IF( la->head == NULL );
	TEST_ERR_IF( la->head->kind != NODE_KIND_HEAD_OR_TAIL );
	TEST_ERR_IF( la->head->n != NULL );
	TEST_ERR_IF( la->head->l != NULL );
	TEST_ERR_IF( la->head->next == NULL );
	TEST_ERR_IF( la->head->prev == NULL );
	TEST_ERR_IF( la->head->prev != la->head );

	TEST_ERR_IF( la->tail == NULL );
	TEST_ERR_IF( la->tail->kind != NODE_KIND_HEAD_OR_TAIL );
	TEST_ERR_IF( la->tail->n != NULL );
	TEST_ERR_IF( la->tail->l != NULL );
	TEST_ERR_IF( la->tail->next == NULL );
	TEST_ERR_IF( la->tail->prev == NULL );
	TEST_ERR_IF( la->tail->next != la->tail );

	/* *** */
	node = la->head->next;
	TEST_ERR_IF( node == NULL );

	while ( node != la->tail )
	{
		TEST_ERR_IF( node->next == NULL );
		TEST_ERR_IF( node->next == node );
		TEST_ERR_IF( node->prev == NULL );
		TEST_ERR_IF( node->prev == node );
		TEST_ERR_IF( node->next->prev != node );
		TEST_ERR_IF( node->prev->next != node );

		TEST_ERR_IF( node->count <= 0 );
		realCount += node->count;

		TEST_ERR_IF(    node->kind != NODE_KIND_INT
		             && node->kind != NODE_KIND_LIST
		           );

		if ( node->kind == NODE_KIND_INT )
		{
			TEST_ERR_IF( node->n == NULL );
			TEST_ERR_IF( node->l != NULL );
		}
		else
		{
			TEST_ERR_IF( node->n != NULL );
			TEST_ERR_IF( node->l == NULL );

			i = 0;
			while( i < node->count )
			{
				TEST_ERR_IF( node->l[ i ] == NULL );

				foundRef = 0;
				subLa = node->l[ i ]->laPointsTo;
				subRefNode = subLa->refListHead->next;
				while ( subRefNode != subLa->refListTail && foundRef == 0 )
				{
					j = 0;
					while ( j < subRefNode->count )
					{
						if ( subRefNode->l[ j ]->laParent == la )
						{
							foundRef = 1;
							break;
						}

						j += 1;
					}

					subRefNode = subRefNode->next;
				}

				TEST_ERR_IF( foundRef == 0 );

				i += 1;
			}
			while ( i < TROT_NODE_SIZE )
			{
				TEST_ERR_IF( node->l[ i ] != NULL );

				i += 1;
			}
		}


		node = node->next;
	}

	if ( realCount != la->childrenCount )
	{
		printf( "realCount = (%d)\n", realCount );
		printf( "la->childrenCount = (%d)\n", la->childrenCount );
		fflush( stdout );

		TEST_ERR_IF( 1 );
	}

	/* *** */
	TEST_ERR_IF( la->refListHead == NULL );
	TEST_ERR_IF( la->refListHead->next == NULL );
	TEST_ERR_IF( la->refListHead->prev == NULL );
	TEST_ERR_IF( la->refListHead->prev != la->refListHead );
	TEST_ERR_IF( la->refListHead->count != 0 );

	TEST_ERR_IF( la->refListTail == NULL );
	TEST_ERR_IF( la->refListTail->next == NULL );
	TEST_ERR_IF( la->refListTail->prev == NULL );
	TEST_ERR_IF( la->refListTail->next != la->refListTail );
	TEST_ERR_IF( la->refListTail->count != 0 );

	refNode = la->refListHead->next;
	while ( refNode != la->refListTail )
	{
		TEST_ERR_IF( refNode->next == NULL );
		TEST_ERR_IF( refNode->next == refNode );
		TEST_ERR_IF( refNode->prev == NULL );
		TEST_ERR_IF( refNode->prev == refNode );
		TEST_ERR_IF( refNode->next->prev != refNode );
		TEST_ERR_IF( refNode->prev->next != refNode );

		TEST_ERR_IF( refNode->count <= 0 );

		TEST_ERR_IF( refNode->l == NULL );

		i = 0;
		while ( i < refNode->count )
		{
			TEST_ERR_IF( refNode->l[ i ] == NULL );
			TEST_ERR_IF( refNode->l[ i ]->laPointsTo == NULL );
			TEST_ERR_IF( refNode->l[ i ]->laPointsTo != la );

			if ( refNode->l[ i ] == l )
			{
				foundLr = 1;
			}

			i += 1;
		}
		while ( i < REF_LIST_NODE_SIZE )
		{
			TEST_ERR_IF( refNode->l[ i ] != NULL );

			i += 1;
		}

		refNode = refNode->next;
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
	

void printList( TrotList *lMemLimit, TrotList *l, int indent )
{
	/* DATA */
	TrotListActual *la = l->laPointsTo;
	TrotListNode *node = la->head->next;
	int i = 0;
	/* TrotListRefListNode *refNode = la->refListHead->next; */


	/* CODE */
	printIndent( indent );
	printf( "LIST: %p\n", (void *)la );
/*
	printIndent( indent );
	printf( "R " );
	while ( refNode != la->refListTail )
	{
		i = 0;
		while ( i < refNode->count )
		{
			printf( "(%p<%p) ", (void *)refNode->r[ i ]->laParent, (void *)refNode->r[ i ] );

			i += 1;
		}

		refNode = refNode->next;
	}
	printf( "\n" );
*/

	while ( node != la->tail )
	{
		if ( node->kind == NODE_KIND_INT )
		{
			printIndent( indent );
			printf( "I " );
			for ( i = 0; i < (node->count); i += 1 )
			{
				printf( "%d ", node->n[ i ] );
			}
			printf( "\n" );
		}
		else /* node->kind == NODE_KIND_LIST */
		{
			printIndent( indent );
			printf( "L\n" );
			for ( i = 0; i < (node->count); i += 1 )
			{
				printIndent( indent );
				/* printf( "(%p>%p)\n", (void *)node->l[ i ], (void *)node->l[ i ]->laPointsTo ); */
				printList( lMemLimit, node->l[ i ], indent + 1 );
			}
		}

		node = node->next;
	}

	if ( indent == 0 )
	{
		printf( "\n\n" );
	}

	return;
}

/******************************************************************************/
#define LOAD_BUFFER_SIZE 1024
int load( TrotList *lMemLimit, TrotList *lName, TrotList **lBytes )
{
	/* DATA */
	int rc = 0;

	size_t bytesRead = 0;
	size_t i = 0;

	char *name = NULL;

	FILE *fp = NULL;

	TrotList *newLrBytes = NULL;

	u8 buffer[ LOAD_BUFFER_SIZE ];


	/* CODE */
	TEST_ERR_IF( lName == NULL );
	TEST_ERR_IF( lBytes == NULL );
	TEST_ERR_IF( (*lBytes) != NULL );

	/* create our new byte list */
	TEST_ERR_IF( trotListInit( lMemLimit, &newLrBytes ) != TROT_RC_SUCCESS );

	/* convert our TrotList name to a cString */
	TEST_ERR_IF( listToCString( lMemLimit, lName, &name ) != TROT_RC_SUCCESS );

#if 0
	printf( "Loading: %s\n", name );
#endif

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
			TEST_ERR_IF( trotListAppendInt( lMemLimit, newLrBytes, buffer[ i ] ) != TROT_RC_SUCCESS );

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
	(*lBytes) = newLrBytes;
	newLrBytes = NULL;


	/* CLEANUP */
	cleanup:

	if ( fp != NULL )
	{
		fclose( fp );
	}

	if ( name != NULL )
	{
		TROT_FREE( name, strlen(name) + 1 );
	}

	trotListFree( lMemLimit, &newLrBytes );

	return rc;
}
#undef LOAD_BUFFER_SIZE

/******************************************************************************/
TROT_RC printListLikeCString( TrotList *lMemLimit, TrotList *l )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	char *s = NULL;


	/* CODE */
	PARANOID_ERR_IF( l == NULL );

	rc = listToCString( lMemLimit, l, &s );
	ERR_IF_PASSTHROUGH;

	printf( "%s\n", s );


	/* CLEANUP */
	cleanup:

	TROT_FREE( s, strlen(s) + 1 );
	
	return rc;
}

/******************************************************************************/
TROT_RC listToCString( TrotList *lMemLimit, TrotList *l, char **cString_A )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	char *newCString = NULL;

	TROT_INT count = 0;
	TROT_INT i = 1;

	TROT_INT c = 0;


	/* CODE */
	PARANOID_ERR_IF( l == NULL );
	PARANOID_ERR_IF( cString_A == NULL );
	PARANOID_ERR_IF( (*cString_A) != NULL );

	rc = trotListGetCount( lMemLimit, l, &count );
	ERR_IF_PASSTHROUGH;

	TROT_MALLOC( newCString, char, count + 1 );

	i = 1;
	while ( i <= count )
	{
		rc = trotListGetInt( lMemLimit, l, i, &c );
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
		TROT_FREE( newCString, strlen(newCString) + 1 );
	}

	return rc;
}

/******************************************************************************/
TROT_RC appendCStringToList( TrotList *lMemLimit, TrotList *l, char *cString )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;


	/* CODE */
	PARANOID_ERR_IF( l == NULL );
	PARANOID_ERR_IF( cString == NULL );

	while ( *cString != '\0' )
	{
		rc = trotListAppendInt( lMemLimit, l, (*cString) );
		ERR_IF_PASSTHROUGH;

		cString += 1;
	}


	/* CLEANUP */
	cleanup:

	return rc;
}

