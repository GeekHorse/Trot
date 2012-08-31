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
/*!
	\file
	Implements the secondary functionality of "Hoof", our single data
	structure for Trot.

	Secondary functionality includes:
	- Deep list compare
	- List copy
	- Enlist
	- Delist
	- Copy Span
	- Remove Span
*/

/******************************************************************************/
#include "trot.h"
#include "trotInternal.h"

/******************************************************************************/
/*!
	\brief Compares two lists.
	\param lr Pointer to a trotListRef.
	\param lrCompareTo Pointer to a trotListRef that you want to compare the
		first one to.
	\param compareResult On success, the result of the comparison.
	\return TROT_RC
*/
TROT_RC trotListRefCompare( trotListRef *lr, trotListRef *lrCompareTo, TROT_LIST_COMPARE_RESULT *compareResult )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	trotStack *stack = NULL;
	trotStackNode *stackNode = NULL;
	int stackEmpty = 0;

	trotList *l1 = NULL;
	INT_TYPE count1 = 0;
	INT_TYPE n1 = 0;
	int kind1 = NODE_KIND_HEAD_OR_TAIL;
	trotList *subL1 = NULL;

	trotList *l2 = NULL;
	INT_TYPE count2 = 0;
	INT_TYPE n2 = 0;
	int kind2 = NODE_KIND_HEAD_OR_TAIL;
	trotList *subL2 = NULL;

	INT_TYPE index = 0;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );
	PRECOND_ERR_IF( lrCompareTo == NULL );
	PRECOND_ERR_IF( compareResult == NULL );


	/* CODE */
	/* we assume equal, and set this otherwise later */
	(*compareResult) = TROT_LIST_COMPARE_EQUAL;

	/* no need to test if they point to same list */
	if ( lr -> lPointsTo == lrCompareTo -> lPointsTo )
	{
		return TROT_RC_SUCCESS;
	}

	/* init stack */
	rc = trotStackInit( &stack );
	ERR_IF_PASSTHROUGH;

	rc = trotStackPush( stack, lr -> lPointsTo, lrCompareTo -> lPointsTo );
	ERR_IF_PASSTHROUGH;

	/* compare loop */
	while ( 1 )
	{
		/* increment top of stack */
		rc = trotStackIncrementTopIndex( stack );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		/* get both stack info */
		stackNode = stack -> tail -> prev;
		index = stackNode -> index;
		l1 = stackNode -> l1;
		l2 = stackNode -> l2;

		/* make sure we're in index */
		count1 = l1 -> childrenCount;
		count2 = l2 -> childrenCount;

		/* if both are too big */
		if ( index > count1 && index > count2 )
		{
			rc = trotStackPop( stack, &stackEmpty );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			if ( stackEmpty )
			{
				break;
			}

			continue;
		}

		/* if index is too big for list 1 */
		if ( index > count1 )
		{
			(*compareResult) = TROT_LIST_COMPARE_LESS_THAN;
			break;
		}

		/* if index is too big for list 2 */
		if ( index > count2 )
		{
			(*compareResult) = TROT_LIST_COMPARE_GREATER_THAN;
			break;
		}

		/* get kinds */
		kind1 = stackNode -> l1Node -> kind;
		kind2 = stackNode -> l2Node -> kind;

		/* compare kinds */
		/* ints are considered smaller than lists */
		if ( kind1 == NODE_KIND_INT && kind2 == NODE_KIND_LIST )
		{
			(*compareResult) = TROT_LIST_COMPARE_LESS_THAN;
			break;
		}
		if ( kind1 == NODE_KIND_LIST && kind2 == NODE_KIND_INT )
		{
			(*compareResult) = TROT_LIST_COMPARE_GREATER_THAN;
			break;
		}

		/* get and compare ints */
		if ( kind1 == NODE_KIND_INT )
		{
			PARANOID_ERR_IF( kind2 != NODE_KIND_INT );

			n1 = stackNode -> l1Node -> n[ stackNode -> l1Count ];
			n2 = stackNode -> l2Node -> n[ stackNode -> l2Count ];

			if ( n1 < n2 )
			{
				(*compareResult) = TROT_LIST_COMPARE_LESS_THAN;
				break;
			}
			else if ( n1 > n2 )
			{
				(*compareResult) = TROT_LIST_COMPARE_GREATER_THAN;
				break;
			}

			continue;
		}

		PARANOID_ERR_IF( kind1 != NODE_KIND_LIST );
		PARANOID_ERR_IF( kind2 != NODE_KIND_LIST );

		/* get lists */
		subL1 = stackNode -> l1Node -> l[ stackNode -> l1Count ] -> lPointsTo;
		subL2 = stackNode -> l2Node -> l[ stackNode -> l2Count ] -> lPointsTo;

		/* only add if different.
		   if they point to same, there's no need to compare */
		if ( subL1 != subL2 )
		{
			rc = trotStackPush( stack, subL1, subL2 );
			ERR_IF_PASSTHROUGH;
		}
	}


	/* CLEANUP */
	cleanup:

	trotStackFree( &stack );

	return rc;
}

/******************************************************************************/
/*!
	\brief Copies a list.
	\param lr Pointer to a trotListRef to copy.
	\param lrCopy_A Pointer to a trotListRef pointer that must be NULL.
		On success, this will be a copy of the list.
	\return TROT_RC
*/
TROT_RC trotListRefCopy( trotListRef *lr, trotListRef **lrCopy_A )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );
	PRECOND_ERR_IF( lrCopy_A == NULL );
	PRECOND_ERR_IF( (*lrCopy_A) != NULL );


	/* CODE */
	/* if list is empty, just give back a new list */
	if ( lr -> lPointsTo -> childrenCount == 0 )
	{
		rc = trotListRefInit( lrCopy_A );
		ERR_IF_PASSTHROUGH;
	}
	/* else, use CopySpan */
	else
	{
		rc = trotListRefCopySpan( lr, 1, -1, lrCopy_A );
		ERR_IF_PASSTHROUGH;

/* TODO: add some unit tests for this */
		/* make sure copied list has same tag */
		(*lrCopy_A) -> lPointsTo -> tag = lr -> lPointsTo -> tag;
	}

	return 0;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Takes a span of children and puts them into a list.
	\param lr Pointer to a trotListRef.
	\param indexStart start index of items you want to enlist.
	\param indexEnd end index of items you want to enlist.
	\return TROT_RC
*/
TROT_RC trotListRefEnlist( trotListRef *lr, INT_TYPE indexStart, INT_TYPE indexEnd )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	trotList *l = NULL;
	INT_TYPE tempI = 0;

	trotListNode *node = NULL;

	INT_TYPE count = 0;

	trotListNode *startNode = NULL;

	trotListRef *newListRef = NULL;
	trotList *newList = NULL;

	trotListNode *newNode = NULL;

	int i = 0;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );


	/* CODE */
	l = lr -> lPointsTo;

	/* Turn negative indices into positive equivalents. */
	if ( indexStart < 0 )
	{
		indexStart = (l -> childrenCount) + indexStart + 1;
	}
	if ( indexEnd < 0 )
	{
		indexEnd = (l -> childrenCount) + indexEnd + 1;
	}

	/* Make sure indices are in range */
	ERR_IF( indexStart <= 0, TROT_RC_ERROR_BAD_INDEX );
	ERR_IF( indexStart > (l -> childrenCount), TROT_RC_ERROR_BAD_INDEX );

	ERR_IF( indexEnd <= 0, TROT_RC_ERROR_BAD_INDEX );
	ERR_IF( indexEnd > (l -> childrenCount), TROT_RC_ERROR_BAD_INDEX );

	/* swap indices if end is before start */
	if ( indexEnd < indexStart )
	{
		tempI = indexStart;
		indexStart = indexEnd;
		indexEnd = tempI;
	}

	/* find start */
	node = l -> head -> next;
	while ( 1 )
	{
		if ( count + (node -> count) >= indexStart )
		{
			break;
		}

		count += node -> count;
		node = node -> next;

		PARANOID_ERR_IF( node == l -> tail );
	}

	/* split this node if necessary */
	if ( count + 1 != indexStart )
	{
		rc = trotListNodeSplit( node, indexStart - count - 1 );
		ERR_IF_PASSTHROUGH;

		count += node -> count;
		node = node -> next;
	}

	/* mark startNode */
	startNode = node;

	/* find end */
	while ( 1 )
	{
		if ( count + (node -> count) >= indexEnd )
		{
			break;
		}

		count += node -> count;
		node = node -> next;

		PARANOID_ERR_IF( node == l -> tail );
	}

	/* split this node if necessary */
	if ( count + node -> count != indexEnd )
	{
		rc = trotListNodeSplit( node, indexEnd - count );
		ERR_IF_PASSTHROUGH;
	}

	/* create our new node */
	rc = newListNode( &newNode );
	ERR_IF_PASSTHROUGH;

	/* create our new list */
	rc = trotListRefInit( &newListRef );
	ERR_IF_PASSTHROUGH;

	/* insert our new list into our node */
	newNode -> l[ 0 ] = newListRef;
	newNode -> count = 1;
	newListRef -> lParent = l;

	/* get our new list */
	newList = newListRef -> lPointsTo;

	/* insert our new node into list */
	newNode -> prev = startNode -> prev;
	newNode -> next = startNode;

	startNode -> prev -> next = newNode;
	startNode -> prev = newNode;

	/* remove nodes from old list */
	startNode -> prev -> next = node -> next;
	node -> next -> prev = startNode -> prev;

	/* insert nodes into new list */
	newList -> head -> next = startNode;
	startNode -> prev = newList -> head;

	newList -> tail -> prev = node;
	node -> next = newList -> tail;


	/* adjust counts in both lists */
	count = 0;
	node = newList -> head -> next;
	while ( node != newList -> tail )
	{
		count += node -> count;
		node = node -> next;
	}

	/* we subtract one from count because we're adding
	   the new "enlist" list too */
	l -> childrenCount -= (count - 1); 
	newList -> childrenCount = count;

	/* adjust references in newList */
	node = newList -> head -> next;
	while ( node != newList -> tail )
	{
		if ( node -> kind == NODE_KIND_LIST )
		{
			i = 0;
			while ( i < node -> count )
			{
				node -> l[ i ] -> lParent = newList;

				i += 1;
			}
		}

		node = node -> next;
	}

	return 0;


	/* CLEANUP */
	cleanup:

	if ( newNode != NULL )
	{
		trotFree( newNode -> l );
		trotFree( newNode );
	}

	return rc;
}

/******************************************************************************/
/*!
	\brief Removes a list and puts it's children in it's place.
	\param lr Pointer to a trotListRef.
	\param index index of list you want to delist.
	\return TROT_RC
*/
TROT_RC trotListRefDelist( trotListRef *lr, INT_TYPE index )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	trotList *l = NULL;

	INT_TYPE count = 0;

	trotListNode *node = NULL;
	trotListNode *insertBeforeThisNode = NULL;

	trotListRef *delistListRef = NULL;
	trotListRef *copiedListRef = NULL;

	int i = 0;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );


	/* CODE */
	l = lr -> lPointsTo;

	/* Turn negative index into positive equivalent. */
	if ( index < 0 )
	{
		index = (l -> childrenCount) + index + 1;
	}

	/* Make sure index is in range */
	ERR_IF( index <= 0, TROT_RC_ERROR_BAD_INDEX );
	ERR_IF( index > (l -> childrenCount), TROT_RC_ERROR_BAD_INDEX );

	/* find index */
	node = l -> head -> next;
	while ( 1 )
	{
		if ( count + (node -> count) >= index )
		{
			break;
		}

		count += node -> count;
		node = node -> next;

		PARANOID_ERR_IF( node == l -> tail );
	}

	/* check kind */
	ERR_IF( node -> kind != NODE_KIND_LIST, TROT_RC_ERROR_WRONG_KIND );

	/* split this node if necessary */
	if ( count + 1 != index )
	{
		rc = trotListNodeSplit( node, index - count - 1 );
		ERR_IF_PASSTHROUGH;

		node = node -> next;
	}

	/* save our spot */
	insertBeforeThisNode = node;

	/* get our delist list */
	delistListRef = node -> l[ 0 ];

	/* copy our delist (only if it contains something) */
	if ( delistListRef -> lPointsTo -> childrenCount > 0 )
	{
		rc = trotListRefCopySpan( delistListRef, 1, -1, &copiedListRef );
		ERR_IF_PASSTHROUGH;
	}

	/* if this node contains more, the move the others over */
	if ( node -> count > 1 )
	{
		delistListRef = node -> l[ 0 ];

		i = 0;
		while ( i < ( node -> count - 1 ) )
		{
			node -> l[ i ] = node -> l[ i + 1 ];

			i += 1;
		}
		node -> l[ i ] = NULL;
		node -> count -= 1;
	}
	/* else, remove node from list */
	else
	{
		insertBeforeThisNode = insertBeforeThisNode -> next;

		node -> prev -> next = node -> next;
		node -> next -> prev = node -> prev;

		trotFree( node -> l );
		trotFree( node );
	}

	/* adjust count */
	l -> childrenCount -= 1;

	/* free our delistListRef */
	delistListRef -> lParent = NULL;
	trotListRefFree( &delistListRef );

	/* was the delist empty? */
	if ( copiedListRef == NULL )
	{
		return 0;
	}

	/* adjust count */
	l -> childrenCount += copiedListRef -> lPointsTo -> childrenCount;

	/* go ahead and adjust all ref's "parents" */
	node = copiedListRef -> lPointsTo -> head;
	while ( node != copiedListRef -> lPointsTo -> tail )
	{
		if ( node -> kind == NODE_KIND_LIST )
		{
			i = 0;
			while ( i < node -> count )
			{
				node -> l[ i ] -> lParent = l;

				i += 1;
			}
		}

		node = node -> next;
	}

	/* move copied list contents into our list */
	copiedListRef -> lPointsTo -> head -> next -> prev = insertBeforeThisNode -> prev;
	copiedListRef -> lPointsTo -> tail -> prev -> next = insertBeforeThisNode;

	insertBeforeThisNode -> prev -> next = copiedListRef -> lPointsTo -> head -> next;
	insertBeforeThisNode -> prev = copiedListRef -> lPointsTo -> tail -> prev;

	copiedListRef -> lPointsTo -> childrenCount = 0;
	copiedListRef -> lPointsTo -> head -> next = copiedListRef -> lPointsTo -> tail;
	copiedListRef -> lPointsTo -> tail -> prev = copiedListRef -> lPointsTo -> head;

	/* free our copied list */
	trotListRefFree( &copiedListRef );

	return 0;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Makes a copy of a span in a list.
	\param lr Pointer to a trotListRef that you want to copy a span in.
	\param indexStart index of start of span.
	\param indexEnd index of end of span.
	\param lrCopy_A Pointer to a trotListRef pointer that must be NULL.
		On success, this will be a copy of the span.
	\return TROT_RC
*/
TROT_RC trotListRefCopySpan( trotListRef *lr, INT_TYPE indexStart, INT_TYPE indexEnd, trotListRef **lrCopy_A )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	trotList *l = NULL;

	INT_TYPE tempI = 0;

	trotListRef *newListRef = NULL;

	trotListNode *node = NULL;
	trotListNode *tail = NULL;

	INT_TYPE count = 0;

	int i = 0;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );
	PRECOND_ERR_IF( lrCopy_A == NULL );
	PRECOND_ERR_IF( (*lrCopy_A) != NULL );


	/* CODE */
	l = lr -> lPointsTo;

	/* Turn negative indices into positive equivalents. */
	if ( indexStart < 0 )
	{
		indexStart = (l -> childrenCount) + indexStart + 1;
	}
	if ( indexEnd < 0 )
	{
		indexEnd = (l -> childrenCount) + indexEnd + 1;
	}

	/* Make sure indices are in range */
	ERR_IF( indexStart <= 0, TROT_RC_ERROR_BAD_INDEX );
	ERR_IF( indexStart > (l -> childrenCount), TROT_RC_ERROR_BAD_INDEX );

	ERR_IF( indexEnd <= 0, TROT_RC_ERROR_BAD_INDEX );
	ERR_IF( indexEnd > (l -> childrenCount), TROT_RC_ERROR_BAD_INDEX );

	/* swap indices if end is before start */
	if ( indexEnd < indexStart )
	{
		tempI = indexStart;
		indexStart = indexEnd;
		indexEnd = tempI;
	}

	/* make our new list */
	rc = trotListRefInit( &newListRef );
	ERR_IF_PASSTHROUGH;

	/* *** */
	tail = l -> tail;
	node = l -> head -> next;

	/* find node that contain indexStart */
	while ( 1 )
	{
		/* if we haven't reached the startIndex, continue */
		if ( count + node -> count >= indexStart )
		{
			break;
		}

		count += node -> count;
		node = node -> next;

		PARANOID_ERR_IF( node == tail );
	}

	/* begin to copy */
	i = indexStart - count - 1;
	while ( node != tail && count < indexEnd )
	{
		/* copy */
		while ( i < node -> count && ( i + count ) < indexEnd )
		{
			if ( node -> kind == NODE_KIND_INT )
			{
				rc = trotListRefAppendInt( newListRef, node -> n[ i ] );
				ERR_IF_PASSTHROUGH;
			}
			else
			{
				rc = trotListRefAppendListTwin( newListRef, node -> l[ i ] );
				ERR_IF_PASSTHROUGH;
			}

			i += 1;
		}

		i = 0;
		count += node -> count;
		node = node -> next;
	}


	/* give back */
	(*lrCopy_A) = newListRef;
	newListRef = NULL;

	return 0;


	/* CLEANUP */
	cleanup:

	trotListRefFree( &newListRef );

	return rc;
}

/******************************************************************************/
/*!
	\brief Removes a span in a list.
	\param lr Pointer to a trotListRef that you want to remove a span in.
	\param indexStart index of start of span.
	\param indexEnd index of end of span.
	\return TROT_RC
*/
TROT_RC trotListRefRemoveSpan( trotListRef *lr, INT_TYPE indexStart, INT_TYPE indexEnd )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	trotListRef *lrRemoved = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );


	/* CODE */
	/* Turn negative indices into positive equivalents. */
	if ( indexStart < 0 )
	{
		indexStart = (lr -> lPointsTo -> childrenCount) + indexStart + 1;
	}
	if ( indexEnd < 0 )
	{
		indexEnd = (lr -> lPointsTo -> childrenCount) + indexEnd + 1;
	}

	/* enlist */
	rc = trotListRefEnlist( lr, indexStart, indexEnd );
	ERR_IF_PASSTHROUGH;

	/* remove list */
	rc = trotListRefRemoveList( lr, indexStart < indexEnd ? indexStart : indexEnd, &lrRemoved );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* free removed list */
	trotListRefFree( &lrRemoved );

	return 0;


	/* CLEANUP */
	cleanup:

	return rc;
}

