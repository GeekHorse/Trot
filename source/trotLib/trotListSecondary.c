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
/*!
	\file
	Implements the secondary functionality for our lists.

	Secondary functionality includes:
	- Deep list compare by value
	- List copy (1 level deep only)
	- Enlist
	- Delist
	- Copy Span
	- Remove Span
*/
#undef  TROT_FILE_NUMBER
#define TROT_FILE_NUMBER 2

/******************************************************************************/
#include "trot.h"
#include "trotInternal.h"

/******************************************************************************/
/*!
	\brief Copies a list.
	\param[in] lMemLimit List that maintains memory limit
	\param[in] l The list.
	\param[out] lCopy_A New copied list.
	\return TROT_RC

	This only copies the 1st level, it does not recurse. If you want a "deep"
	copy, then encode then decode a list.

	Tags are copied.
*/
TROT_RC trotListCopy( TrotList *lMemLimit, TrotList *l, TrotList **lCopy_A )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;


	/* PRECOND */
	FAILURE_POINT;
	ERR_IF( l == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( lCopy_A == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( (*lCopy_A) != NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	/* if list is empty, just give back a new list */
	if ( l->laPointsTo->childrenCount == 0 )
	{
		rc = trotListInit( lMemLimit, lCopy_A );
		ERR_IF_PASSTHROUGH;

		/* make sure copied list has same type and tag */
		(*lCopy_A)->laPointsTo->type = l->laPointsTo->type;
		(*lCopy_A)->laPointsTo->tag  = l->laPointsTo->tag;
	}
	/* else, use CopySpan */
	else
	{
		rc = trotListCopySpan( lMemLimit, l, 1, -1, lCopy_A );
		ERR_IF_PASSTHROUGH;

		/* copy span copys the type and tag too */
	}


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Takes a span of children and puts them into a list.
	\param[in] lMemLimit List that maintains memory limit
	\param[in] l The list.
	\param[in] indexStart Start index of items you want to enlist.
	\param[in] indexEnd End index of items you want to enlist.
	\return TROT_RC

	Example:
	If list is [ 1 2 3 4 5 ], indexStart is 3, and indexEnd is 5 then list will
	become:
	[ 1 2 [ 3 4 5 ] ]
*/
TROT_RC trotListEnlist( TrotList *lMemLimit, TrotList *l, TROT_INT indexStart, TROT_INT indexEnd )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotListActual *la = NULL;
	TROT_INT tempI = 0;

	TrotListNode *node = NULL;

	TROT_INT count = 0;

	TrotListNode *startNode = NULL;

	TrotList *newL = NULL;
	TrotListActual *newLa = NULL;

	TrotListNode *newNode = NULL;

	TROT_INT i = 0;


	/* PRECOND */
	FAILURE_POINT;
	ERR_IF( l == NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	la = l->laPointsTo;

	/* Turn negative indices into positive equivalents. */
	if ( indexStart < 0 )
	{
		indexStart = (la->childrenCount) + indexStart + 1;
	}
	if ( indexEnd < 0 )
	{
		indexEnd = (la->childrenCount) + indexEnd + 1;
	}

	/* Make sure indices are in range */
	ERR_IF_1( indexStart <= 0, TROT_RC_ERROR_BAD_INDEX, indexStart );
	ERR_IF_1( indexStart > (la->childrenCount), TROT_RC_ERROR_BAD_INDEX, indexStart );

	ERR_IF_1( indexEnd <= 0, TROT_RC_ERROR_BAD_INDEX, indexEnd );
	ERR_IF_1( indexEnd > (la->childrenCount), TROT_RC_ERROR_BAD_INDEX, indexEnd );

	/* swap indices if end is before start */
	if ( indexEnd < indexStart )
	{
		tempI = indexStart;
		indexStart = indexEnd;
		indexEnd = tempI;
	}

	/* find start */
	node = la->head->next;
	while ( 1 )
	{
		if ( count + (node->count) >= indexStart )
		{
			break;
		}

		count += node->count;
		node = node->next;

		PARANOID_ERR_IF( node == la->tail );
	}

	/* split this node if necessary */
	if ( count + 1 != indexStart )
	{
		rc = trotListNodeSplit( lMemLimit, node, indexStart - count - 1 );
		ERR_IF_PASSTHROUGH;

		count += node->count;
		node = node->next;
	}

	/* mark startNode */
	startNode = node;

	/* find end */
	while ( 1 )
	{
		if ( count + (node->count) >= indexEnd )
		{
			break;
		}

		count += node->count;
		node = node->next;

		PARANOID_ERR_IF( node == la->tail );
	}

	/* split this node if necessary */
	if ( count + node->count != indexEnd )
	{
		rc = trotListNodeSplit( lMemLimit, node, indexEnd - count );
		ERR_IF_PASSTHROUGH;
	}

	/* create our new node */
	rc = newListNode( lMemLimit, &newNode );
	ERR_IF_PASSTHROUGH;

	/* create our new list */
	rc = trotListInit( lMemLimit, &newL );
	ERR_IF_PASSTHROUGH;

	/* insert our new list into our node */
	newNode->l[ 0 ] = newL;
	newNode->count = 1;
	newL->laParent = la;

	/* get our new list */
	newLa = newL->laPointsTo;

	/* insert our new node into list */
	newNode->prev = startNode->prev;
	newNode->next = startNode;

	startNode->prev->next = newNode;
	startNode->prev = newNode;

	newNode = NULL;

	/* remove nodes from old list */
	startNode->prev->next = node->next;
	node->next->prev = startNode->prev;

	/* insert nodes into new list */
	newLa->head->next = startNode;
	startNode->prev = newLa->head;

	newLa->tail->prev = node;
	node->next = newLa->tail;


	/* adjust counts in both lists */
	count = 0;
	node = newLa->head->next;
	while ( node != newLa->tail )
	{
		count += node->count;
		node = node->next;
	}

	/* we subtract one from count because we're adding
	   the new "enlist" list too */
	la->childrenCount -= (count - 1); 
	newLa->childrenCount = count;

	/* adjust references in newList */
	node = newLa->head->next;
	while ( node != newLa->tail )
	{
		if ( node->kind == NODE_KIND_LIST )
		{
			i = 0;
			while ( i < node->count )
			{
				node->l[ i ]->laParent = newLa;

				i += 1;
			}
		}

		node = node->next;
	}


	/* CLEANUP */
	cleanup:

	if ( newNode != NULL )
	{
		TROT_FREE( newNode->l, TROT_NODE_SIZE );
		TROT_FREE( newNode, 1 );
	}

	return rc;
}

/******************************************************************************/
/*!
	\brief Removes a list and puts it's children in it's place.
	\param[in] lMemLimit List that maintains memory limit
	\param[in] l The list.
	\param[in] index Index of list you want to delist.
	\return TROT_RC

	Example:
	If list is [ 1 2 [ 3 4 5 ] ] and index is 3 then list will become:
	[ 1 2 3 4 5 ]
*/
TROT_RC trotListDelist( TrotList *lMemLimit, TrotList *l, TROT_INT index )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotListActual *la = NULL;

	TROT_INT count = 0;

	TrotListNode *node = NULL;
	TrotListNode *insertBeforeThisNode = NULL;

	TrotList *delistL = NULL;
	TrotList *copiedL = NULL;

	TROT_INT i = 0;


	/* PRECOND */
	FAILURE_POINT;
	ERR_IF( l == NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	la = l->laPointsTo;

	/* Turn negative index into positive equivalent. */
	if ( index < 0 )
	{
		index = (la->childrenCount) + index + 1;
	}

	/* Make sure index is in range */
	ERR_IF_1( index <= 0, TROT_RC_ERROR_BAD_INDEX, index );
	ERR_IF_1( index > (la->childrenCount), TROT_RC_ERROR_BAD_INDEX, index );

	/* find index */
	node = la->head->next;
	while ( 1 )
	{
		if ( count + (node->count) >= index )
		{
			break;
		}

		count += node->count;
		node = node->next;

		PARANOID_ERR_IF( node == la->tail );
	}

	/* check kind */
	ERR_IF_1( node->kind != NODE_KIND_LIST, TROT_RC_ERROR_WRONG_KIND, node->kind );

	/* split this node if necessary */
	if ( count + 1 != index )
	{
		rc = trotListNodeSplit( lMemLimit, node, index - count - 1 );
		ERR_IF_PASSTHROUGH;

		node = node->next;
	}

	/* save our spot */
	insertBeforeThisNode = node;

	/* get our delist list */
	delistL = node->l[ 0 ];

	/* lists cannot hold more than TROT_MAX_CHILDREN, so make sure we have room */
	/* plus 1 because when you delist, you're removing a list, and then adding the children */
	ERR_IF( TROT_MAX_CHILDREN - la->childrenCount + 1 < delistL->laPointsTo->childrenCount, TROT_RC_ERROR_LIST_OVERFLOW );

	/* copy our delist (only if it contains something) */
	if ( delistL->laPointsTo->childrenCount > 0 )
	{
		rc = trotListCopySpan( lMemLimit, delistL, 1, -1, &copiedL );
		ERR_IF_PASSTHROUGH;
	}

	/* if this node contains more, the move the others over */
	if ( node->count > 1 )
	{
		delistL = node->l[ 0 ];

		i = 0;
		while ( i < ( node->count - 1 ) )
		{
			node->l[ i ] = node->l[ i + 1 ];

			i += 1;
		}
		node->l[ i ] = NULL;
		node->count -= 1;
	}
	/* else, remove node from list */
	else
	{
		insertBeforeThisNode = insertBeforeThisNode->next;

		node->prev->next = node->next;
		node->next->prev = node->prev;

		TROT_FREE( node->l, TROT_NODE_SIZE );
		TROT_FREE( node, 1 );
	}

	/* adjust count */
	la->childrenCount -= 1;

	/* free our delistL */
	delistL->laParent = NULL;
	trotListFree( lMemLimit, &delistL );

	/* was the delist empty? */
	if ( copiedL == NULL )
	{
		goto cleanup;
	}

	/* adjust count */
	la->childrenCount += copiedL->laPointsTo->childrenCount;

	/* go ahead and adjust all ref's "parents" */
	node = copiedL->laPointsTo->head;
	while ( node != copiedL->laPointsTo->tail )
	{
		if ( node->kind == NODE_KIND_LIST )
		{
			i = 0;
			while ( i < node->count )
			{
				node->l[ i ]->laParent = la;

				i += 1;
			}
		}

		node = node->next;
	}

	/* move copied list contents into our list */
	copiedL->laPointsTo->head->next->prev = insertBeforeThisNode->prev;
	copiedL->laPointsTo->tail->prev->next = insertBeforeThisNode;

	insertBeforeThisNode->prev->next = copiedL->laPointsTo->head->next;
	insertBeforeThisNode->prev = copiedL->laPointsTo->tail->prev;

	copiedL->laPointsTo->childrenCount = 0;
	copiedL->laPointsTo->head->next = copiedL->laPointsTo->tail;
	copiedL->laPointsTo->tail->prev = copiedL->laPointsTo->head;

	/* free our copied list */
	trotListFree( lMemLimit, &copiedL );


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Makes a copy of a span in a list.
	\param[in] lMemLimit List that maintains memory limit
	\param[in] l The list.
	\param[in] indexStart Index of start of span.
	\param[in] indexEnd Index of end of span.
	\param[out] lCopy_A Copy of span.
	\return TROT_RC

	l is not modified.

	Example:
	If list is [ 1 2 3 4 5 ], indexStart is 3, and indexEnd is 5, then the new
	list will contain:
	[ 3 4 5 ]

	FUTURE: This could potentially be removed and reimplemented when the Trot
	virtual machine is finished. Which would make it slower, but would make
	the trot library smaller. Do we want to eliminate everything from the
	library that can be implemented in Trot?
*/
TROT_RC trotListCopySpan( TrotList *lMemLimit, TrotList *l, TROT_INT indexStart, TROT_INT indexEnd, TrotList **lCopy_A )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotListActual *la = NULL;

	TROT_INT tempI = 0;

	TrotList *newL = NULL;

	TrotListNode *node = NULL;
	TrotListNode *tail = NULL;

	TROT_INT count = 0;

	TROT_INT i = 0;


	/* PRECOND */
	FAILURE_POINT;
	ERR_IF( l == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( lCopy_A == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( (*lCopy_A) != NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	la = l->laPointsTo;

	/* Turn negative indices into positive equivalents. */
	if ( indexStart < 0 )
	{
		indexStart = (la->childrenCount) + indexStart + 1;
	}
	if ( indexEnd < 0 )
	{
		indexEnd = (la->childrenCount) + indexEnd + 1;
	}

	/* Make sure indices are in range */
	ERR_IF_1( indexStart <= 0, TROT_RC_ERROR_BAD_INDEX, indexStart );
	ERR_IF_1( indexStart > (la->childrenCount), TROT_RC_ERROR_BAD_INDEX, indexStart );

	ERR_IF_1( indexEnd <= 0, TROT_RC_ERROR_BAD_INDEX, indexEnd );
	ERR_IF_1( indexEnd > (la->childrenCount), TROT_RC_ERROR_BAD_INDEX, indexEnd );

	/* swap indices if end is before start */
	if ( indexEnd < indexStart )
	{
		tempI = indexStart;
		indexStart = indexEnd;
		indexEnd = tempI;
	}

	/* make our new list */
	rc = trotListInit( lMemLimit, &newL );
	ERR_IF_PASSTHROUGH;

	/* *** */
	tail = la->tail;
	node = la->head->next;

	/* find node that contain indexStart */
	while ( 1 )
	{
		/* if we haven't reached the startIndex, continue */
		if ( count + node->count >= indexStart )
		{
			break;
		}

		count += node->count;
		node = node->next;

		PARANOID_ERR_IF( node == tail );
	}

	/* begin to copy */
	i = indexStart - count - 1;
	while ( node != tail && count < indexEnd )
	{
		/* copy */
		while ( i < node->count && ( i + count ) < indexEnd )
		{
			if ( node->kind == NODE_KIND_INT )
			{
				rc = trotListAppendInt( lMemLimit, newL, node->n[ i ] );
				ERR_IF_PASSTHROUGH;
			}
			else
			{
				rc = trotListAppendList( lMemLimit, newL, node->l[ i ] );
				ERR_IF_PASSTHROUGH;
			}

			i += 1;
		}

		i = 0;
		count += node->count;
		node = node->next;
	}

	/* make sure copied span has same type and tag */
	newL->laPointsTo->type = l->laPointsTo->type;
	newL->laPointsTo->tag  = l->laPointsTo->tag;


	/* give back */
	(*lCopy_A) = newL;
	newL = NULL;


	/* CLEANUP */
	cleanup:

	trotListFree( lMemLimit, &newL );

	return rc;
}

/******************************************************************************/
/*!
	\brief Removes a span in a list.
	\param[in] lMemLimit List that maintains memory limit
	\param[in] l The list.
	\param[in] indexStart Index of start of span.
	\param[in] indexEnd Index of end of span.
	\return TROT_RC

	Example:
	If list is [ 1 2 3 4 5 ], indexStart is 3, and indexEnd is 5, then list will
	become:
	[ 1 2 ]
*/
TROT_RC trotListRemoveSpan( TrotList *lMemLimit, TrotList *l, TROT_INT indexStart, TROT_INT indexEnd )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotList *lRemoved = NULL;


	/* PRECOND */
	FAILURE_POINT;
	ERR_IF( l == NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	/* Turn negative indices into positive equivalents. */
	if ( indexStart < 0 )
	{
		indexStart = (l->laPointsTo->childrenCount) + indexStart + 1;
	}
	if ( indexEnd < 0 )
	{
		indexEnd = (l->laPointsTo->childrenCount) + indexEnd + 1;
	}

	/* enlist */
	rc = trotListEnlist( lMemLimit, l, indexStart, indexEnd );
	ERR_IF_PASSTHROUGH;

	/* remove list */
	rc = trotListRemoveList( lMemLimit, l, indexStart < indexEnd ? indexStart : indexEnd, &lRemoved );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* free removed list */
	trotListFree( lMemLimit, &lRemoved );


	/* CLEANUP */
	cleanup:

	return rc;
}

