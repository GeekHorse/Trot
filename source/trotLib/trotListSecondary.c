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
	\brief Does a full-depth compare of 2 lists by value. Does not compare tags.
	\param[in] l The first list.
	\param[in] lCompareTo The second list.
	\param[out] compareResult Result of the comparison.
	\return TROT_RC

	FUTURE: it doesnt compare references, just structure and values, which is ok,
	just need to document it. if you want a "real" compare, you must encode
	both lists, then compare the outputs.
	maybe we just need a "string compare" that will only compare 1 level deep
	and then a true compare that uses encoding. ... same with copy?

	FUTURE: need to remove this, and just implement it in Trot itself
	that way, it will be easier, plus, people can modify it to suit their
	needs. What if they want tags to match, but not usertags, etc.

	FUTURE: a true compare can be done by using encode, and comparing the
	resulting character lists.
*/
TROT_RC trotListCompare( TrotList *l, TrotList *lCompareTo, TROT_LIST_COMPARE_RESULT *compareResult )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotStack *stack = NULL;
	TrotStackNode *stackNode = NULL;
	TROT_INT stackEmpty = 0;

	TrotListActual *la1 = NULL;
	TROT_INT count1 = 0;
	TROT_INT n1 = 0;
	TROT_INT kind1 = NODE_KIND_HEAD_OR_TAIL;
	TrotListActual *subLa1 = NULL;

	TrotListActual *la2 = NULL;
	TROT_INT count2 = 0;
	TROT_INT n2 = 0;
	TROT_INT kind2 = NODE_KIND_HEAD_OR_TAIL;
	TrotListActual *subLa2 = NULL;

	TROT_INT index = 0;


	/* PRECOND */
	ERR_IF( l == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( lCompareTo == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( compareResult == NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	/* we assume equal, and set this otherwise later */
	(*compareResult) = TROT_LIST_COMPARE_EQUAL;

	/* no need to test if they point to same list */
	if ( l->laPointsTo == lCompareTo->laPointsTo )
	{
		return TROT_RC_SUCCESS;
	}

	/* init stack */
	rc = trotStackInit( &stack );
	ERR_IF_PASSTHROUGH;

	rc = trotStackPush( stack, l->laPointsTo, lCompareTo->laPointsTo );
	ERR_IF_PASSTHROUGH;

	/* compare loop */
	while ( 1 )
	{
		/* increment top of stack */
		rc = trotStackIncrementTopIndex( stack );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		/* get both stack info */
		stackNode = stack->tail->prev;
		index = stackNode->index;
		la1 = stackNode->la1;
		la2 = stackNode->la2;

		/* make sure we're in index */
		count1 = la1->childrenCount;
		count2 = la2->childrenCount;

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
		kind1 = stackNode->la1Node->kind;
		kind2 = stackNode->la2Node->kind;

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

			n1 = stackNode->la1Node->n[ stackNode->la1Count ];
			n2 = stackNode->la2Node->n[ stackNode->la2Count ];

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
		subLa1 = stackNode->la1Node->l[ stackNode->la1Count ]->laPointsTo;
		subLa2 = stackNode->la2Node->l[ stackNode->la2Count ]->laPointsTo;

		/* only add if different.
		   if they point to same, there's no need to compare */
		if ( subLa1 != subLa2 )
		{
			rc = trotStackPush( stack, subLa1, subLa2 );
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
	\param[in] l The list.
	\param[out] lCopy_A New copied list.
	\return TROT_RC

	This only copies the 1st level, it does not recurse. If you want a "deep"
	copy, then encode then decode a list.

	Tags are copied.
*/
TROT_RC trotListCopy( TrotList *l, TrotList **lCopy_A )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;


	/* PRECOND */
	ERR_IF( l == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( lCopy_A == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( (*lCopy_A) != NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	/* if list is empty, just give back a new list */
	if ( l->laPointsTo->childrenCount == 0 )
	{
		rc = trotListInit( lCopy_A );
		ERR_IF_PASSTHROUGH;

		/* make sure copied list has same tags */
		(*lCopy_A)->laPointsTo->tag     = l->laPointsTo->tag;
		(*lCopy_A)->laPointsTo->userTag = l->laPointsTo->userTag;
	}
	/* else, use CopySpan */
	else
	{
		rc = trotListCopySpan( l, 1, -1, lCopy_A );
		ERR_IF_PASSTHROUGH;

		/* copy span copys the tags too */
	}


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Takes a span of children and puts them into a list.
	\param[in] l The list.
	\param[in] indexStart Start index of items you want to enlist.
	\param[in] indexEnd End index of items you want to enlist.
	\return TROT_RC

	Example:
	If list is [ 1 2 3 4 5 ], indexStart is 3, and indexEnd is 5 then list will
	become:
	[ 1 2 [ 3 4 5 ] ]
*/
TROT_RC trotListEnlist( TrotList *l, TROT_INT indexStart, TROT_INT indexEnd )
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
		rc = trotListNodeSplit( node, indexStart - count - 1 );
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
		rc = trotListNodeSplit( node, indexEnd - count );
		ERR_IF_PASSTHROUGH;
	}

	/* create our new node */
	rc = newListNode( &newNode );
	ERR_IF_PASSTHROUGH;

	/* create our new list */
	rc = trotListInit( &newL );
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
		TROT_HOOK_FREE( newNode->l );
		TROT_HOOK_FREE( newNode );
	}

	return rc;
}

/******************************************************************************/
/*!
	\brief Removes a list and puts it's children in it's place.
	\param[in] l The list.
	\param[in] index Index of list you want to delist.
	\return TROT_RC

	Example:
	If list is [ 1 2 [ 3 4 5 ] ] and index is 3 then list will become:
	[ 1 2 3 4 5 ]
*/
TROT_RC trotListDelist( TrotList *l, TROT_INT index )
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
		rc = trotListNodeSplit( node, index - count - 1 );
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
		rc = trotListCopySpan( delistL, 1, -1, &copiedL );
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

		TROT_HOOK_FREE( node->l );
		TROT_HOOK_FREE( node );
	}

	/* adjust count */
	la->childrenCount -= 1;

	/* free our delistL */
	delistL->laParent = NULL;
	trotListFree( &delistL );

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
	trotListFree( &copiedL );


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Makes a copy of a span in a list.
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
TROT_RC trotListCopySpan( TrotList *l, TROT_INT indexStart, TROT_INT indexEnd, TrotList **lCopy_A )
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
	rc = trotListInit( &newL );
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
				rc = trotListAppendInt( newL, node->n[ i ] );
				ERR_IF_PASSTHROUGH;
			}
			else
			{
				rc = trotListAppendList( newL, node->l[ i ] );
				ERR_IF_PASSTHROUGH;
			}

			i += 1;
		}

		i = 0;
		count += node->count;
		node = node->next;
	}

	/* make sure copied span has same tag */
	newL->laPointsTo->tag     = l->laPointsTo->tag;
	newL->laPointsTo->userTag = l->laPointsTo->userTag;


	/* give back */
	(*lCopy_A) = newL;
	newL = NULL;


	/* CLEANUP */
	cleanup:

	trotListFree( &newL );

	return rc;
}

/******************************************************************************/
/*!
	\brief Removes a span in a list.
	\param[in] l The list.
	\param[in] indexStart Index of start of span.
	\param[in] indexEnd Index of end of span.
	\return TROT_RC

	Example:
	If list is [ 1 2 3 4 5 ], indexStart is 3, and indexEnd is 5, then list will
	become:
	[ 1 2 ]
*/
TROT_RC trotListRemoveSpan( TrotList *l, TROT_INT indexStart, TROT_INT indexEnd )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotList *lRemoved = NULL;


	/* PRECOND */
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
	rc = trotListEnlist( l, indexStart, indexEnd );
	ERR_IF_PASSTHROUGH;

	/* remove list */
	rc = trotListRemoveList( l, indexStart < indexEnd ? indexStart : indexEnd, &lRemoved );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* free removed list */
	trotListFree( &lRemoved );


	/* CLEANUP */
	cleanup:

	return rc;
}

