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
/*!
	\file
	TODO
*/

/******************************************************************************/
#include "trotCommon.h"
#include "trotList.h"
#include "trotListInternal.h"
#include "trotMem.h"
#include "trotStack.h"

/******************************************************************************/
/*!
	\brief Compares two lists.
	\param lr Pointer to a trotListRef.
	\param lrCompareTo Pointer to a trotListRef that you want to compare the
		first one to.
	\param compareResult On success, the result of the comparison.
	\return 0 on success, <0 on error
*/
int trotListRefCompare( trotListRef *lr, trotListRef *lrCompareTo, TROT_LIST_COMPARE_RESULT *compareResult )
{
	/* DATA */
	int rc = TROT_LIST_SUCCESS;

	trotStack *stack = NULL;
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
		return TROT_LIST_SUCCESS;
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
		rc = trotStackIncrementTopN( stack );
		ERR_IF_PASSTHROUGH;

		/* get both stack info */
		rc = trotStackGet( stack, &l1, &l2, &index );
		ERR_IF_PASSTHROUGH;

		/* make sure we're in index */
		count1 = l1 -> childrenCount;
		count2 = l2 -> childrenCount;

		/* if both are too big */
		if ( index > count1 && index > count2 )
		{
			rc = trotStackPop( stack, &stackEmpty );
			ERR_IF_PASSTHROUGH;

			if ( stackEmpty )
			{
				break;
			}

			continue;
		}

		/* if index1 is too big and index2 is ok */
		if ( index > count1 && index <= count2 )
		{
			(*compareResult) = TROT_LIST_COMPARE_LESS_THAN;
			break;
		}

		/* if n1 is ok and n2 is too big */
		if ( index <= count1 && index > count2 )
		{
			(*compareResult) = TROT_LIST_COMPARE_GREATER_THAN;
			break;
		}

		/* get kinds */
		rc = trotListGetKind( l1, index, &kind1 );
		ERR_IF_PASSTHROUGH;
		rc = trotListGetKind( l2, index, &kind2 );
		ERR_IF_PASSTHROUGH;

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
		if ( kind1 == NODE_KIND_INT && kind2 == NODE_KIND_INT )
		{
			rc = trotListGetInt( l1, index, &n1 );
			ERR_IF_PASSTHROUGH;
			rc = trotListGetInt( l2, index, &n2 );
			ERR_IF_PASSTHROUGH;

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

		/* TODO: put in a "should never happen" MACRO HERE */

		/* get lists */
		rc = trotListGetList( l1, index, &subL1 );
		ERR_IF_PASSTHROUGH;
		rc = trotListGetList( l2, index, &subL2 );
		ERR_IF_PASSTHROUGH;

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
	\param lrCopy_A Pointer to a trotListRef pointer that must be NULL.
		On success, this will be a copy of the list.
	\param lr Pointer to a trotListRef to copy.
	\return 0 on success, <0 on error
*/
int trotListRefCopy( trotListRef **lrCopy_A, trotListRef *lr )
{
	/* DATA */
	int rc = TROT_LIST_SUCCESS;


	/* PRECOND */
	PRECOND_ERR_IF( lrCopy_A == NULL );
	PRECOND_ERR_IF( (*lrCopy_A) != NULL );
	PRECOND_ERR_IF( lr == NULL );


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
		rc = trotListRefCopySpan( lrCopy_A, lr, 1, -1 );
		ERR_IF_PASSTHROUGH;
	}

	return 0;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief TODO
	\param lr Pointer to a trotListRef.
	\param indexStart start index of items you want to enlist.
	\param indexEnd end index of items you want to enlist.
	\return 0 on success, <0 on error
*/
int trotListRefEnlist( trotListRef *lr, INT_TYPE indexStart, INT_TYPE indexEnd )
{
	/* DATA */
	int rc = TROT_LIST_SUCCESS;

	trotList *l = NULL;
	INT_TYPE tempI = 0;

	trotListNode *node = NULL;
	trotListNode *tail = NULL;

	INT_TYPE count = 0;

	trotListNode *startNode = NULL;

	trotListRef *newListRef = NULL;
	trotList *newList = NULL;

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
	ERR_IF( indexStart <= 0, TROT_LIST_ERROR_BAD_INDEX );
	ERR_IF( indexStart > (l -> childrenCount), TROT_LIST_ERROR_BAD_INDEX );

	ERR_IF( indexEnd <= 0, TROT_LIST_ERROR_BAD_INDEX );
	ERR_IF( indexEnd > (l -> childrenCount), TROT_LIST_ERROR_BAD_INDEX );

	/* swap indices if end is before start */
	if ( indexEnd < indexStart )
	{
		tempI = indexStart;
		indexStart = indexEnd;
		indexEnd = tempI;
	}

	/* find start */
	tail = l -> tail;
	node = l -> head -> next;
	while ( node != tail )
	{
		if ( count + (node -> count) >= indexStart )
		{
			break;
		}

		count += node -> count;
		node = node -> next;
	}

	/* split this node if necessary */
	if ( count + 1 != indexStart )
	{
		rc = trotListNodeSplit( node, indexStart - count - 1 );
		ERR_IF_PASSTHROUGH;

		node = node -> next;
	}

	/* mark startNode */
	startNode = node;

	/* find end */
	while ( node != tail )
	{
		if ( count + (node -> count) >= indexEnd )
		{
			break;
		}

		count += node -> count;
		node = node -> next;
	}

	/* split this node if necessary */
	if ( count + node -> count != indexEnd )
	{
		rc = trotListNodeSplit( node, indexEnd - count );
		ERR_IF_PASSTHROUGH;
	}

	/* create our new list */
	rc = trotListRefInit( &newListRef );
	ERR_IF_PASSTHROUGH;

	newList = newListRef -> lPointsTo;

	/* remove nodes from old list */
	startNode -> prev -> next = node -> next;
	node -> next -> prev = startNode;

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

	l -> childrenCount -= count;
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
				node -> l[ i ] -> lParent = l;

				i += 1;
			}
		}

		node = node -> next;
	}

	return 0;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief TODO
	\param lr Pointer to a trotListRef.
	\param indexStart start index of items you want to enlist.
	\param indexEnd end index of items you want to enlist.
	\return 0 on success, <0 on error
*/
int trotListRefDelist( trotListRef *lr, INT_TYPE index )
{
	/* DATA */
	int rc = TROT_LIST_SUCCESS;

	trotList *l = NULL;

	INT_TYPE count = 0;

	trotListNode *node = NULL;
	trotListNode *tail = NULL;
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
	ERR_IF( index <= 0, TROT_LIST_ERROR_BAD_INDEX );
	ERR_IF( index > (l -> childrenCount), TROT_LIST_ERROR_BAD_INDEX );

	/* find index */
	tail = l -> tail;
	node = l -> head -> next;
	while ( node != tail )
	{
		if ( count + (node -> count) >= index )
		{
			break;
		}

		count += node -> count;
		node = node -> next;
	}

	ERR_IF( node -> kind != NODE_KIND_LIST, TROT_LIST_ERROR_WRONG_KIND );

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
	if ( delistListRef -> lPointsTo -> childrenCount > 1 )
	{
		rc = trotListRefCopySpan( &copiedListRef, delistListRef, 1, -1 );
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
		node -> prev -> next = node -> next;
		node -> next -> prev = node -> prev;

		trotFree( node -> l );
		trotFree( node );
	}

	/* adjust count */
	l -> childrenCount -= 1;

	/* free our delistListRef */
	delistListRef -> lParent = NULL;
	rc = trotListRefFree( &delistListRef );
	ERR_IF_PASSTHROUGH;

	/* was the delist empty? */
	if ( copiedListRef == NULL )
	{
		return 0;
	}

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
	insertBeforeThisNode -> prev -> next = copiedListRef -> lPointsTo -> head -> next;
	copiedListRef -> lPointsTo -> head -> next -> prev = insertBeforeThisNode -> prev;

	insertBeforeThisNode -> prev = copiedListRef -> lPointsTo -> tail -> prev;
	copiedListRef -> lPointsTo -> tail -> prev -> next = insertBeforeThisNode;

	copiedListRef -> lPointsTo -> childrenCount = 0;

	/* free our copied list */
	rc = trotListRefFree( &copiedListRef );
	ERR_IF_PASSTHROUGH;

	return 0;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Makes a copy of a span in a list.
	\param lrCopy_A Pointer to a trotListRef pointer that must be NULL.
		On success, this will be a copy of the span.
	\param lr Pointer to a trotListRef that you want to copy a span in.
	\param indexStart index of start of span.
	\param indexEnd index of end of span.
	\return 0 on success, <0 on error
*/
int trotListRefCopySpan( trotListRef **lrCopy_A, trotListRef *lr, INT_TYPE indexStart, INT_TYPE indexEnd )
{
	/* DATA */
	int rc = TROT_LIST_SUCCESS;

	trotList *l = NULL;

	INT_TYPE tempI = 0;

	trotListRef *newListRef = NULL;

	trotListNode *node = NULL;
	trotListNode *tail = NULL;

	INT_TYPE count = 0;

	int i = 0;


	/* PRECOND */
	PRECOND_ERR_IF( lrCopy_A == NULL );
	PRECOND_ERR_IF( (*lrCopy_A) != NULL );
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
	ERR_IF( indexStart <= 0, TROT_LIST_ERROR_BAD_INDEX );
	ERR_IF( indexStart > (l -> childrenCount), TROT_LIST_ERROR_BAD_INDEX );

	ERR_IF( indexEnd <= 0, TROT_LIST_ERROR_BAD_INDEX );
	ERR_IF( indexEnd > (l -> childrenCount), TROT_LIST_ERROR_BAD_INDEX );

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
	while ( node != tail )
	{
		/* if we haven't reached the startIndex, continue */
		if ( count + node -> count >= indexStart )
		{
			break;
		}

		count += node -> count;
		node = node -> next;
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
	\return 0 on success, <0 on error
*/
int trotListRefRemoveSpan( trotListRef *lr, INT_TYPE indexStart, INT_TYPE indexEnd )
{
	/* DATA */
	int rc = TROT_LIST_SUCCESS;

	trotListRef *lrRemoved = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );


	/* CODE */
	rc = trotListRefEnlist( lr, indexStart, indexEnd );
	ERR_IF_PASSTHROUGH;

	rc = trotListRefRemoveList( lr, indexStart, &lrRemoved );
	ERR_IF_PASSTHROUGH;

	rc = trotListRefFree( &lrRemoved );
	ERR_IF_PASSTHROUGH;

	return 0;


	/* CLEANUP */
	cleanup:

	return rc;
}

