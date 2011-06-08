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
	Handles "Hoof", which is the single data structure for Trot.
	"Hoof" is a list, which can only contain signed ints and lists.
	Lists can be "twinned", which creates another reference to the list,
	which is how pointers are handled in Trot.
*/

/******************************************************************************/
#include "trotCommon.h"
#include "trotList.h"
#include "trotStack.h"
#include "trotMem.h"

/******************************************************************************/
static inline int _trotListNodeSplit( trotListNode *n, int keepInLeft );

static inline int _newIntNode( trotListNode **n_A );
static inline int _newListNode( trotListNode **n_A );

static inline int _refListAdd( trotList *l, trotListRef *r );
static inline int _refListRemove( trotList *l, trotListRef *r );

static inline int _isListReachable( trotList *l );

/******************************************************************************/
/*!
	\brief Allocates a new trotListRef reference to a new list.
	\param lr_A Pointer to a trotListRef pointer that must be NULL. On
		success, this will point to a new trotListRef reference.
	\return TROT_LIST_SUCCESS on success, <0 on error
*/
int trotListRefInit( trotListRef **lr_A )
{
	/* DATA */
	int rc = TROT_LIST_SUCCESS;

	trotListRefListNode *newRefHead = NULL;
	trotListRefListNode *newRefTail = NULL;

	trotListNode *newHead = NULL;
	trotListNode *newTail = NULL;
	trotList *newList = NULL;
	trotListRef *newListRef = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( lr_A == NULL );
	PRECOND_ERR_IF( (*lr_A) != NULL );


	/* CODE */
	/* create list of refs that point to this list */
	TROT_MALLOC( newRefHead, trotListRefListNode, 1 );
	TROT_MALLOC( newRefTail, trotListRefListNode, 1 );

	newRefHead -> count = 0;
	newRefHead -> r = NULL;
	newRefHead -> next = newRefTail;
	newRefHead -> prev = newRefHead;

	newRefTail -> count = 0;
	newRefTail -> r = NULL;
	newRefTail -> next = newRefTail;
	newRefTail -> prev = newRefHead;

	/* create the data list */
	TROT_MALLOC( newHead, trotListNode, 1 );
	TROT_MALLOC( newTail, trotListNode, 1 );

	newHead -> kind = NODE_KIND_HEAD_OR_TAIL;
	newHead -> count = 0;
	newHead -> n = NULL;
	newHead -> l = NULL;
	newHead -> prev = newHead;
	newHead -> next = newTail;

	newTail -> kind = NODE_KIND_HEAD_OR_TAIL;
	newTail -> count = 0;
	newTail -> n = NULL;
	newTail -> l = NULL;
	newTail -> prev = newHead;
	newTail -> next = newTail;

	/* create actual list structure */
	TROT_MALLOC( newList, trotList, 1 );

	newList -> reachable = 1;

	newList -> childrenCount = 0;

	newList -> refListHead = newRefHead;
	newList -> refListTail = newRefTail;
	newRefHead = NULL;
	newRefTail = NULL;

	newList -> head = newHead;
	newList -> tail = newTail;
	newHead = NULL;
	newTail = NULL;

	/* create the first ref to this list */
	TROT_MALLOC( newListRef, trotListRef, 1 );

	newListRef -> lParent = NULL;

	newListRef -> lPointsTo = newList;
	newList = NULL;

	/* add first ref to list's ref list */
	rc = _refListAdd( newListRef -> lPointsTo, newListRef );
	ERR_IF( rc != 0, rc );

	/* give back */
	(*lr_A) = newListRef;
	newListRef = NULL;

	return TROT_LIST_SUCCESS;


	/* CLEANUP */
	cleanup:

	trotFree( newRefHead );
	trotFree( newRefTail );
	trotFree( newHead );
	trotFree( newTail );
	trotFree( newList );
	trotFree( newListRef );

	return rc;
}

/******************************************************************************/
/*!
	\brief Twins a trotListRef reference.
	\param lr_A Pointer to a trotListRef pointer that must be NULL. On
		success, this will point to a new trotListRef reverence.
	\param lrToTwin Pointer to the trotListRef pointer to be twinned.
	\return TROT_LIST_SUCCESS on success, <0 on error
*/
int trotListRefTwin( trotListRef **lr_A, trotListRef *lrToTwin )
{
	/* DATA */
	int rc = TROT_LIST_SUCCESS;

	trotListRef *newListRef = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( lr_A == NULL );
	PRECOND_ERR_IF( (*lr_A) != NULL );
	PRECOND_ERR_IF( lrToTwin == NULL );


	/* CODE */
	TROT_MALLOC( newListRef, trotListRef, 1 );

	newListRef -> lParent = NULL;
	newListRef -> lPointsTo = lrToTwin -> lPointsTo;

	rc = _refListAdd( newListRef -> lPointsTo, newListRef );
	ERR_IF( rc != 0, rc );


	/* give back */
	(*lr_A) = newListRef;
	newListRef = NULL;

	return TROT_LIST_SUCCESS;


	/* CLEANUP */
	cleanup:

	trotFree( newListRef );

	return rc;
}

/******************************************************************************/
/*!
	\brief Frees a trotListRef reference. Actual list will be trotFreed if this
		is the last reference.
	\param lr_F Pointer to a trotListRef pointer.
		(*lr_F) can be NULL, in which case nothing will happen.
		On return, (*lr_F) will be NULL.
	\return void
*/
int trotListRefFree( trotListRef **lr_F )
{
	/* DATA */
	int rc = TROT_LIST_SUCCESS;

	trotList *list = NULL;

	trotStack *stack = NULL;
	trotStackNode *stackNode = NULL;
	TROT_STACK_CONTAINS contains = TROT_STACK_DOES_NOT_CONTAIN;

	trotListNode *node = NULL;

	int j = 0;
	trotList *tempList = NULL;

	trotList *currentL = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( lr_F == NULL );


	/* CODE */
	if ( lr_F == NULL || (*lr_F) == NULL )
	{
		return TROT_LIST_SUCCESS;
	}

	ERR_IF( (*lr_F) -> lParent != NULL, TROT_LIST_ERROR_GENERAL );

	list = (*lr_F) -> lPointsTo;

	/* remove ref from list's ref list */
	rc = _refListRemove( list, (*lr_F) );
	ERR_IF( rc != TROT_LIST_SUCCESS, rc );

	/* free ref */
	trotFree( (*lr_F) );
	(*lr_F) = NULL;

	/* is list reachable? */
	rc = _isListReachable( list );
	ERR_IF( rc != TROT_LIST_SUCCESS, rc );

	if ( list -> reachable )
	{
		return TROT_LIST_SUCCESS;
	}

	/* we need to free it */

	/* create our stack */
	rc = trotStackInit( &stack );
	ERR_IF( rc != TROT_LIST_SUCCESS, rc );

	/* add this list to stack */
	rc = trotStackPush( stack, list );
	ERR_IF( rc != TROT_LIST_SUCCESS, rc );

	/* go through stack */
	stackNode = stack -> head -> next;
	while ( stackNode != stack -> tail )
	{
		/* get list */
		currentL = stackNode -> l;

		/* free data */
		node = currentL -> head -> next;
		while ( node != currentL -> tail )
		{
			if ( node -> kind == NODE_KIND_INT )
			{
				trotFree( node -> n );
			}
			else /* NODE_KIND_LIST */
			{
				for ( j = 0; j < node -> count; j += 1 )
				{
					tempList = node -> l[ j ] -> lPointsTo;
			
					rc = _refListRemove( tempList, node -> l[ j ] );
					ERR_IF( rc != TROT_LIST_SUCCESS, rc ); /* TODO: what state are we in if we error here??? */

					trotFree( node -> l[ j ] );
					node -> l[ j ] = NULL;

					if ( tempList -> reachable == 1 )
					{
						rc = _isListReachable( tempList );
						ERR_IF( rc != TROT_LIST_SUCCESS, rc ); /* TODO: what state are we in if we error here??? */

						if ( tempList -> reachable == 0 )
						{
							/* need to free this list too, so add it to the stack */
							rc = trotStackQueryContains( stack, tempList, &contains );
							ERR_IF( rc != TROT_LIST_SUCCESS, rc ); /* TODO: what state are we in if we error here??? */
							if ( contains == TROT_STACK_DOES_NOT_CONTAIN )
							{
								rc = trotStackPush( stack, tempList );
								ERR_IF( rc != TROT_LIST_SUCCESS, rc ); /* TODO: what state are we in if we error here??? */
							}
						}
					}
				}

				trotFree( node -> l );
			}

			node = node -> next;
			trotFree( node -> prev );
		}

		trotFree( currentL -> head );
		currentL -> head = NULL;

		trotFree( currentL -> tail );
		currentL -> tail = NULL;

		/* *** */
		stackNode = stackNode -> next;
	}

	/* go through stack */
	stackNode = stack -> head -> next;
	while ( stackNode != stack -> tail )
	{
		/* get list */
		currentL = stackNode -> l;
	
		trotFree( currentL -> refListHead );
		trotFree( currentL -> refListTail );

		trotFree( currentL );

		/* *** */
		stackNode = stackNode -> next;
	}

	trotStackFree( &stack );

	return TROT_LIST_SUCCESS;

	/* CLEANUP */
	cleanup:

	/* TODO: what state are we in??? */

	return rc;
}

/******************************************************************************/
/*!
	\brief Gets the count of items in the list.
	\param lr Pointer to a trotListRef pointer.
	\param c On success, will contain the count of this list.
	\return TROT_LIST_SUCCESS on success, <0 on error
*/
int trotListRefGetCount( trotListRef *lr, INT_TYPE *c )
{
	/* DATA */
	//int rc = TROT_LIST_SUCCESS;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );
	PRECOND_ERR_IF( c == NULL );


	/* CODE */
	(*c) = lr -> lPointsTo -> childrenCount;

	return TROT_LIST_SUCCESS;
}

/******************************************************************************/
/*!
	\brief Gets the kind of an item in the list.
	\param lr Pointer to a trotListRef pointer.
	\param index Index of the item.
	\param kind On success, will contain the kind of the item.
	\return TROT_LIST_SUCCESS on success, <0 on error
*/
int trotListRefGetKind( trotListRef *lr, INT_TYPE index, int *kind )
{
	/* DATA */
	int rc = TROT_LIST_SUCCESS;

	trotListNode *node = NULL;
	trotListNode *tail = NULL;

	INT_TYPE count = 0;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );
	PRECOND_ERR_IF( kind == NULL );


	/* CODE */
	/* Turn negative index into positive equivalent. */
	if ( index < 0 )
	{
		index = (lr -> lPointsTo -> childrenCount) + index + 1;
	}

	/* Make sure index is in range */
	ERR_IF( index <= 0, TROT_LIST_ERROR_BAD_INDEX );
	ERR_IF( index > (lr -> lPointsTo -> childrenCount ), TROT_LIST_ERROR_BAD_INDEX );

	/* *** */
	tail = lr -> lPointsTo -> tail;
	node = lr -> lPointsTo -> head -> next;
	while ( node != tail )
	{
		count += node -> count;
		if ( count >= index )
		{
			break;
		}

		node = node -> next;
	}

	ERR_IF( node == tail, TROT_LIST_ERROR_GENERAL );

	(*kind) = node -> kind;

	return TROT_LIST_SUCCESS;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Appends an int to the end of the list.
	\param lr Pointer to a trotListRef pointer.
	\param n The int value to insert.
	\return TROT_LIST_SUCCESS on success, <0 on error
*/
int trotListRefAppendInt( trotListRef *lr, INT_TYPE n )
{
	/* DATA */
	int rc = TROT_LIST_SUCCESS;

	trotList *l = NULL;
	trotListNode *node = NULL;

	trotListNode *newNode = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );


	/* CODE */
	l = lr -> lPointsTo;

	/* *** */
	node = l -> tail -> prev;

	/* special cases to create new node */
	if (    node == l -> head             /* empty list */
	     || node -> kind != NODE_KIND_INT /* last node is not int kind */
	     || node -> count == NODE_SIZE    /* last node is full */
	   )
	{
		rc = _newIntNode( &newNode );
		ERR_IF( rc != 0, rc );

		newNode -> next = l -> tail;
		newNode -> prev = l -> tail -> prev;

		l -> tail -> prev -> next = newNode;
		l -> tail -> prev = newNode;

		node = newNode;
		newNode = NULL;
	}

	/* append */
	node -> n[ node -> count ] = n;
	node -> count += 1;

	l -> childrenCount += 1;

	return TROT_LIST_SUCCESS;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Appends a list twin to the end of the list.
	\param lr Pointer to a trotListRef pointer.
	\param lrToAppend The list to twin and append.
	\return TROT_LIST_SUCCESS on success, <0 on error
*/
int trotListRefAppendListTwin( trotListRef *lr, trotListRef *lrToAppend )
{
	/* DATA */
	int rc = TROT_LIST_SUCCESS;

	trotList *l = NULL;
	trotListNode *node = NULL;

	trotListNode *newNode = NULL;

	trotListRef *newLr = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );
	PRECOND_ERR_IF( lrToAppend == NULL );


	/* CODE */
	l = lr -> lPointsTo;

	/* *** */
	node = l -> tail -> prev;

	/* special cases to create new node */
	if (    node == l -> head              /* empty list */
	     || node -> kind != NODE_KIND_LIST /* last node is not list kind */
	     || node -> count == NODE_SIZE     /* last node is full */
	   )
	{
		rc = _newListNode( &newNode );
		ERR_IF( rc != 0, rc );

		newNode -> next = l -> tail;
		newNode -> prev = l -> tail -> prev;

		l -> tail -> prev -> next = newNode;
		l -> tail -> prev = newNode;

		node = newNode;
		newNode = NULL;
	}

	/* append */
	rc = trotListRefTwin( &newLr, lrToAppend );
	ERR_IF( rc != 0, rc );

	node -> l[ node -> count ] = newLr;
	newLr -> lParent = l;
	newLr = NULL;

	node -> count += 1;

	l -> childrenCount += 1;

	return TROT_LIST_SUCCESS;


	/* CLEANUP */
	cleanup:

	trotListRefFree( &newLr );

	return rc;
}

/******************************************************************************/
/*!
	\brief Inserts an int into the list.
	\param lr Pointer to a trotListRef pointer.
	\param index Where to insert.
	\param n The int value to insert.
	\return TROT_LIST_SUCCESS on success, <0 on error
*/
int trotListRefInsertInt( trotListRef *lr, INT_TYPE index, INT_TYPE n )
{
	/* DATA */
	int rc = TROT_LIST_SUCCESS;

	trotList *l = NULL;

	trotListNode *node = NULL;
	trotListNode *tail = NULL;

	INT_TYPE count = 0;

	int i = 0;
	int j = 0;

	trotListNode *newNode = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );


	/* CODE */
	l = lr -> lPointsTo;

	/* Turn negative index into positive equivalent. */
	if ( index < 0 )
	{
		index = (l -> childrenCount) + index + 2;
	}

	/* This handles two special cases. One, if they want to add to the end of the
	   list. And two, if they want to add to an empty list. */
	if ( index == (l -> childrenCount) + 1 )
	{
		rc = trotListRefAppendInt( lr, n );
		ERR_IF( rc != 0, rc );

		return TROT_LIST_SUCCESS;
	}

	/* Make sure index is in range */
	ERR_IF( index <= 0, TROT_LIST_ERROR_BAD_INDEX );
	ERR_IF( index > (l -> childrenCount), TROT_LIST_ERROR_BAD_INDEX );

	/* Find node where int needs to be added into */
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

	/* Paranoid. This should never happen */
	ERR_IF( node == tail, TROT_LIST_ERROR_GENERAL );

	/* *** */
	if ( node -> kind == NODE_KIND_INT )
	{
		/* If node is full */
		if ( node -> count == NODE_SIZE )
		{
			rc = _trotListNodeSplit( node, NODE_SIZE / 2 );
			ERR_IF( rc != 0, rc );

			/* Since node has been split, we may need to go to next
			   node. */
			if ( count + (node -> count) < index )
			{
				count += node -> count;
				node = node -> next;
			}
		}

		/* We now have the node where the int needs to be inserted.
		   We've made sure there is space to insert.
		   (count + 1) is the beginning index of the node */
		/* Now let's move any ints over to make room */
		i = index - count - 1;
		j = node -> count;
		while ( j != i )
		{
			node -> n[ j ] = node -> n[ j - 1 ];
			j -= 1;
		}

		/* Insert int into node */
		node -> n[ i ] = n;
		node -> count += 1;

		l -> childrenCount += 1;

		return TROT_LIST_SUCCESS;
	}
	else /* node -> kind == NODE_KIND_LIST */
	{
		i = index - count - 1;

		/* If we need to insert at spot 0, we see if the previous node
		   is an int node with room. If so, we can just append to that
		   node. */
		if (    i == 0
		     && node -> prev -> kind == NODE_KIND_INT
		     && node -> prev -> count != NODE_SIZE 
		   )
		{
			node = node -> prev;

			/* Insert int into node */
			node -> n[ node -> count ] = n;
			node -> count += 1;

			l -> childrenCount += 1;

			return TROT_LIST_SUCCESS;
		}

		/* if not at beginning, we'll have to split the node */
		if ( i != 0 )
		{
			rc = _trotListNodeSplit( node, i );
			ERR_IF( rc != 0, rc );

			node = node -> next;
		}

		/* *** */
		rc = _newIntNode( &newNode );
		ERR_IF( rc != 0, rc );

		newNode -> n[ 0 ] = n;
		newNode -> count = 1;

		/* Insert node into list */
		newNode -> next = node;
		newNode -> prev = node -> prev;

		node -> prev -> next = newNode;
		node -> prev = newNode;

		l -> childrenCount += 1;

		return TROT_LIST_SUCCESS;
	}


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Inserts a twin of a list into the list.
	\param lr Pointer to a trotListRef pointer.
	\param index Where to insert.
	\param lToInsert The listRef to insert.
	\return TROT_LIST_SUCCESS on success, <0 on error
*/
int trotListRefInsertListTwin( trotListRef *lr, INT_TYPE index, trotListRef *lToInsert )
{
	/* DATA */
	int rc = TROT_LIST_SUCCESS;

	trotList *l = NULL;

	trotListNode *node = NULL;
	trotListNode *tail = NULL;

	INT_TYPE count = 0;

	int i = 0;
	int j = 0;

	trotListNode *newNode = NULL;

	trotListRef *newL = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );
	PRECOND_ERR_IF( lToInsert == NULL );


	/* CODE */
	l = lr -> lPointsTo;

	/* Turn negative index into positive equivalent. */
	if ( index < 0 )
	{
		index = (l -> childrenCount) + index + 2;
	}

	/* This handles two special cases. One, if they want to add to the end of the
	   list. And two, if they want to add to an empty list. */
	if ( index == (l -> childrenCount) + 1 )
	{
		rc = trotListRefAppendListTwin( lr, lToInsert );
		ERR_IF( rc != 0, rc );

		return TROT_LIST_SUCCESS;
	}

	/* Make sure index is in range */
	ERR_IF( index <= 0, TROT_LIST_ERROR_BAD_INDEX );
	ERR_IF( index > (l -> childrenCount ), TROT_LIST_ERROR_BAD_INDEX );

	/* Find node where list needs to be added into */
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

	/* Paranoid. This should never happen */
	ERR_IF( node == tail, TROT_LIST_ERROR_GENERAL );

	/* *** */
	if ( node -> kind == NODE_KIND_LIST )
	{
		/* If node is full */
		if ( node -> count == NODE_SIZE )
		{
			rc = _trotListNodeSplit( node, NODE_SIZE / 2 );
			ERR_IF( rc != 0, rc );

			/* Since node has been split, we may need to go to next
			   node. */
			if ( count + (node -> count) < index )
			{
				count += node -> count;
				node = node -> next;
			}
		}

		/* We now have the node where the list needs to be inserted.
		   We've made sure there is space to insert.
		   (count + 1) is the beginning index of the node */

		/* *** */
		rc = trotListRefTwin( &newL, lToInsert );
		ERR_IF( rc != 0, rc );

		/* Now let's move any lists over to make room */
		i = index - count - 1;
		j = node -> count;
		while ( j != i )
		{
			node -> l[ j ] = node -> l[ j - 1 ];
			j -= 1;
		}

		/* Insert list into node */
		node -> l[ i ] = newL;
		newL -> lParent = l;
		newL = NULL;

		node -> count += 1;

		l -> childrenCount += 1;

		return TROT_LIST_SUCCESS;
	}
	else /* node -> kind == NODE_KIND_INT */
	{
		i = index - count - 1;

		/* If we need to insert at spot 0, we see if the previous node
		   is an list node with room. If so, we can just append to that
		   node. */
		if (    i == 0
		     && node -> prev -> kind == NODE_KIND_LIST
		     && node -> prev -> count != NODE_SIZE 
		   )
		{
			node = node -> prev;

			/* Insert list into node */
			rc = trotListRefTwin( &newL, lToInsert );
			ERR_IF( rc != 0, rc );

			node -> l[ node -> count ] = newL;
			newL -> lParent = l;
			newL = NULL;

			node -> count += 1;

			l -> childrenCount += 1;

			return TROT_LIST_SUCCESS;
		}

		/* if not at beginning, we'll have to split the node */
		if ( i != 0 )
		{
			rc = _trotListNodeSplit( node, i );
			ERR_IF( rc != 0, rc );

			node = node -> next;
		}

		/* *** */
		rc = _newListNode( &newNode );
		ERR_IF( rc != 0, rc );

		/* *** */
		rc = trotListRefTwin( &newL, lToInsert );
		ERR_IF( rc != 0, rc );

		/* Insert node into list */
		newNode -> l[ 0 ] = newL;
		newL -> lParent = l;
		newL = NULL;

		newNode -> count = 1;

		newNode -> next = node;
		newNode -> prev = node -> prev;

		node -> prev -> next = newNode;
		node -> prev = newNode;

		l -> childrenCount += 1;

		return TROT_LIST_SUCCESS;
	}


	/* CLEANUP */
	cleanup:

	trotListRefFree( &newL );

	return rc;
}

/******************************************************************************/
/*!
	\brief Gets copy of int in list.
	\param lr Pointer to a trotListRef pointer.
	\param index Which int to get.
	\param n On success, will point to int.
	\return TROT_LIST_SUCCESS on success, <0 on error
*/
int trotListRefGetInt( trotListRef *lr, INT_TYPE index, INT_TYPE *n )
{
	/* DATA */
	int rc = TROT_LIST_SUCCESS;

	trotListNode *node = NULL;
	trotListNode *tail = NULL;

	INT_TYPE count = 0;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );
	PRECOND_ERR_IF( n == NULL );


	/* CODE */
	/* Turn negative index into positive equivalent. */
	if ( index < 0 )
	{
		index = (lr -> lPointsTo -> childrenCount) + index + 1;
	}

	/* Make sure index is in range */
	ERR_IF( index <= 0, TROT_LIST_ERROR_BAD_INDEX );
	ERR_IF( index > (lr -> lPointsTo -> childrenCount ), TROT_LIST_ERROR_BAD_INDEX );

	/* *** */
	tail = lr -> lPointsTo -> tail;
	node = lr -> lPointsTo -> head -> next;
	while ( node != tail )
	{
		count += node -> count;
		if ( count >= index )
		{
			break;
		}

		node = node -> next;
	}

	ERR_IF( node == tail, TROT_LIST_ERROR_GENERAL );

	ERR_IF( node -> kind != NODE_KIND_INT, TROT_LIST_ERROR_WRONG_KIND );

	/* give back */
	(*n) = node -> n[ (node -> count) - 1 - (count - index) ];

	return TROT_LIST_SUCCESS;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Gets list ref of list in list.
	\param lr Pointer to a trotListRef pointer.
	\param index Which list to get.
	\param l On success, will point to list ref.
	\return TROT_LIST_SUCCESS on success, <0 on error
*/
int trotListRefGetListTwin( trotListRef *lr, INT_TYPE index, trotListRef **l )
{
	/* DATA */
	int rc = TROT_LIST_SUCCESS;

	trotListNode *node = NULL;
	trotListNode *tail = NULL;

	INT_TYPE count = 0;

	trotListRef *newL = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );
	PRECOND_ERR_IF( l == NULL );
	PRECOND_ERR_IF( (*l) != NULL );


	/* CODE */
	/* Turn negative index into positive equivalent. */
	if ( index < 0 )
	{
		index = (lr -> lPointsTo -> childrenCount) + index + 1;
	}

	/* Make sure index is in range */
	ERR_IF( index <= 0, TROT_LIST_ERROR_BAD_INDEX );
	ERR_IF( index > (lr -> lPointsTo -> childrenCount ), TROT_LIST_ERROR_BAD_INDEX );

	/* *** */
	tail = lr -> lPointsTo -> tail;
	node = lr -> lPointsTo -> head -> next;
	while ( node != tail )
	{
		count += node -> count;
		if ( count >= index )
		{
			break;
		}

		node = node -> next;
	}

	ERR_IF( node == tail, TROT_LIST_ERROR_GENERAL );

	ERR_IF( node -> kind != NODE_KIND_LIST, TROT_LIST_ERROR_WRONG_KIND );

	rc = trotListRefTwin( &newL, node -> l[ (node -> count) - 1 - (count - index) ] );
	ERR_IF( rc != 0, rc );

	/* give back */
	(*l) = newL;
	newL = NULL;

	return TROT_LIST_SUCCESS;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Gets and removes int ref of list in list.
	\param lr Pointer to a trotListRef pointer.
	\param index Which int to get and remove.
	\param n On success, will point to int.
	\return TROT_LIST_SUCCESS on success, <0 on error
*/
int trotListRefRemoveInt( trotListRef *lr, INT_TYPE index, INT_TYPE *n )
{
	/* DATA */
	int rc = TROT_LIST_SUCCESS;

	trotListNode *node = NULL;
	trotListNode *tail = NULL;

	INT_TYPE count = 0;

	INT_TYPE giveBackN = 0;

	int i = 0;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );
	PRECOND_ERR_IF( n == NULL );


	/* CODE */

	/* Turn negative index into positive equivalent. */
	if ( index < 0 )
	{
		index = (lr -> lPointsTo -> childrenCount) + index + 1;
	}

	/* Make sure index is in range */
	ERR_IF( index <= 0, TROT_LIST_ERROR_BAD_INDEX );
	ERR_IF( index > (lr -> lPointsTo -> childrenCount ), TROT_LIST_ERROR_BAD_INDEX );

	/* *** */
	tail = lr -> lPointsTo -> tail;
	node = lr -> lPointsTo -> head -> next;
	while ( node != tail )
	{
		count += node -> count;
		if ( count >= index )
		{
			break;
		}

		node = node -> next;
	}

	ERR_IF( node == tail, TROT_LIST_ERROR_GENERAL );

	ERR_IF( node -> kind != NODE_KIND_INT, TROT_LIST_ERROR_WRONG_KIND );

	i = (node -> count) - 1 - (count - index);
	giveBackN = node -> n[ i ];
	while ( i < ( (node -> count) - 1 ) )
	{
		node -> n[ i ] = node -> n[ i + 1 ];
		i += 1;
	}
	node -> count -= 1;
	lr -> lPointsTo -> childrenCount -= 1;

	if ( node -> count == 0 )
	{
		node -> prev -> next = node -> next;
		node -> next -> prev = node -> prev;

		trotFree( node -> n );
		trotFree( node );
	}

	/* give back */
	(*n) = giveBackN;

	return TROT_LIST_SUCCESS;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Gets and removes list ref of list in list.
	\param lr Pointer to a trotListRef pointer.
	\param index Which list to get.
	\param l On success, will point to list ref.
	\return TROT_LIST_SUCCESS on success, <0 on error
*/
int trotListRefRemoveList( trotListRef *lr, INT_TYPE index, trotListRef **l )
{
	/* DATA */
	int rc = TROT_LIST_SUCCESS;

	trotListNode *node = NULL;
	trotListNode *tail = NULL;

	INT_TYPE count = 0;

	trotListRef *giveBackL = NULL;

	int i = 0;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );
	PRECOND_ERR_IF( l == NULL );
	PRECOND_ERR_IF( (*l) != NULL );


	/* CODE */
	/* Turn negative index into positive equivalent. */
	if ( index < 0 )
	{
		index = (lr -> lPointsTo -> childrenCount) + index + 1;
	}

	/* Make sure index is in range */
	ERR_IF( index <= 0, TROT_LIST_ERROR_BAD_INDEX );
	ERR_IF( index > (lr -> lPointsTo -> childrenCount ), TROT_LIST_ERROR_BAD_INDEX );

	/* *** */
	tail = lr -> lPointsTo -> tail;
	node = lr -> lPointsTo -> head -> next;
	while ( node != tail )
	{
		count += node -> count;
		if ( count >= index )
		{
			break;
		}

		node = node -> next;
	}

	ERR_IF( node == tail, TROT_LIST_ERROR_GENERAL );

	ERR_IF( node -> kind != NODE_KIND_LIST, TROT_LIST_ERROR_WRONG_KIND );

	i = (node -> count) - 1 - (count - index);
	giveBackL = node -> l[ i ];
	giveBackL -> lParent = NULL;
	while ( i < ( (node -> count) - 1 ) )
	{
		node -> l[ i ] = node -> l[ i + 1 ];
		i += 1;
	}
	node -> l[ i ] = NULL;
	node -> count -= 1;
	lr -> lPointsTo -> childrenCount -= 1;

	if ( node -> count == 0 )
	{
		node -> prev -> next = node -> next;
		node -> next -> prev = node -> prev;

		trotFree( node -> l );
		trotFree( node );
	}

	/* give back */
	(*l) = giveBackL;

	return TROT_LIST_SUCCESS;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Removes whatever is at index in list.
	\param lr Pointer to a trotListRef pointer.
	\param index What to remove.
	\return TROT_LIST_SUCCESS on success, <0 on error
*/
int trotListRefRemove( trotListRef *lr, INT_TYPE index )
{
	/* DATA */
	int rc = TROT_LIST_SUCCESS;

	trotListNode *node = NULL;
	trotListNode *tail = NULL;

	INT_TYPE count = 0;

	trotListRef *tempL = NULL;

	int i = 0;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );


	/* CODE */
	/* Turn negative index into positive equivalent. */
	if ( index < 0 )
	{
		index = (lr -> lPointsTo -> childrenCount) + index + 1;
	}

	/* Make sure index is in range */
	ERR_IF( index <= 0, TROT_LIST_ERROR_BAD_INDEX );
	ERR_IF( index > (lr -> lPointsTo -> childrenCount ), TROT_LIST_ERROR_BAD_INDEX );

	/* *** */
	tail = lr -> lPointsTo -> tail;
	node = lr -> lPointsTo -> head -> next;
	while ( node != tail )
	{
		count += node -> count;
		if ( count >= index )
		{
			break;
		}

		node = node -> next;
	}

	ERR_IF( node == tail, TROT_LIST_ERROR_GENERAL );

	i = (node -> count) - 1 - (count - index);
	if ( node -> kind == NODE_KIND_INT )
	{
		while ( i < ( (node -> count) - 1 ) )
		{
			node -> n[ i ] = node -> n[ i + 1 ];
			i += 1;
		}
	}
	else
	{
		tempL = node -> l[ i ];
		tempL -> lParent = NULL;
		trotListRefFree( &tempL );
		while ( i < ( (node -> count) - 1 ) )
		{
			node -> l[ i ] = node -> l[ i + 1 ];
			i += 1;
		}
		node -> l[ i ] = NULL;
	}
	
	node -> count -= 1;
	lr -> lPointsTo -> childrenCount -= 1;

	if ( node -> count == 0 )
	{
		node -> prev -> next = node -> next;
		node -> next -> prev = node -> prev;

		if ( node -> kind == NODE_KIND_INT )
		{
			trotFree( node -> n );
		}
		else
		{
			trotFree( node -> l );
		}
		trotFree( node );
	}

	return TROT_LIST_SUCCESS;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Splits a node, leaving keepInLeft into the left/prev node, and
		moving the rest into the new right/next node.
	\param n Node to split.
	\param keepInLeft How many items to keep in n.
	\return TROT_LIST_SUCCESS on success, <0 on error
*/
static inline int _trotListNodeSplit( trotListNode *n, int keepInLeft )
{
	/* DATA */
	int rc = TROT_LIST_SUCCESS;

	trotListNode *newNode = NULL;

	int i = 0;


	/* PRECOND */
	PRECOND_ERR_IF( n == NULL );


	/* CODE */
	TROT_MALLOC( newNode, trotListNode, 1 );

	if ( n -> kind == NODE_KIND_INT )
	{
		newNode -> kind = NODE_KIND_INT;
		newNode -> count = (n -> count) - keepInLeft;

		newNode -> l = NULL;
		TROT_MALLOC( newNode -> n, INT_TYPE, NODE_SIZE );

		i = keepInLeft;
		while ( i < (n -> count) )
		{
			newNode -> n[ i - keepInLeft ] = n -> n[ i ];

			i += 1;
		}

		n -> count = keepInLeft;
	}
	else /* n -> kind == NODE_KIND_LIST */
	{
		newNode -> kind = NODE_KIND_LIST;
		newNode -> count = (n -> count) - keepInLeft;

		newNode -> n = NULL;
		TROT_CALLOC( newNode -> l, trotListRef, NODE_SIZE );

		i = keepInLeft;
		while ( i < (n -> count) )
		{
			newNode -> l[ i - keepInLeft ] = n -> l[ i ];
			n -> l[ i ] = NULL;

			i += 1;
		}

		n -> count = keepInLeft;
	}

	newNode -> prev = n;
	newNode -> next = n -> next;

	n -> next -> prev = newNode;
	n -> next = newNode;

	return TROT_LIST_SUCCESS;


	/* CLEANUP */
	cleanup:

	trotFree( newNode );

	return rc;
}

/******************************************************************************/
static inline int _newIntNode( trotListNode **n_A )
{
	/* DATA */
	int rc = TROT_LIST_SUCCESS;

	trotListNode *newNode = NULL;


	/* CODE */
	TROT_MALLOC( newNode, trotListNode, 1 );

	newNode -> kind = NODE_KIND_INT;
	newNode -> count = 0;

	newNode -> l = NULL;
	TROT_MALLOC( newNode -> n, INT_TYPE, NODE_SIZE );

	/* give back */
	(*n_A) = newNode;
	newNode = NULL;

	return TROT_LIST_SUCCESS;


	/* CLEANUP */
	cleanup:

	trotFree( newNode );

	return rc;
}

/******************************************************************************/
static inline int _newListNode( trotListNode **n_A )
{
	/* DATA */
	int rc = TROT_LIST_SUCCESS;

	trotListNode *newNode = NULL;


	/* CODE */
	TROT_MALLOC( newNode, trotListNode, 1 );

	newNode -> kind = NODE_KIND_LIST;
	newNode -> count = 0;

	newNode -> n = NULL;
	TROT_CALLOC( newNode -> l, trotListRef, NODE_SIZE );

	/* give back */
	(*n_A) = newNode;
	newNode = NULL;

	return TROT_LIST_SUCCESS;


	/* CLEANUP */
	cleanup:

	trotFree( newNode );

	return rc;
}

/******************************************************************************/
static inline int _refListAdd( trotList *l, trotListRef *r )
{
	/* DATA */
	int rc = TROT_LIST_SUCCESS;

	trotListRefListNode *refNode = NULL;

	trotListRefListNode *newRefNode = NULL;


	/* CODE */
	refNode = l -> refListHead -> next;
	while ( refNode != l -> refListTail )
	{
		if ( refNode -> count < REF_LIST_NODE_SIZE )
		{
			refNode -> r[ refNode -> count ] = r;
			refNode -> count += 1;

			return TROT_LIST_SUCCESS;
		}

		refNode = refNode -> next;
	}

	/* there was no room in list, so create new node, insert ref into new
	   node, and insert node into list */
	TROT_MALLOC( newRefNode, trotListRefListNode, 1 );

	TROT_CALLOC( newRefNode -> r, trotListRef, REF_LIST_NODE_SIZE );

	newRefNode -> count = 1;
	newRefNode -> r[ 0 ] = r;

	newRefNode -> prev = l -> refListTail -> prev;
	newRefNode -> next = l -> refListTail;
	l -> refListTail -> prev -> next = newRefNode;
	l -> refListTail -> prev = newRefNode;

	return TROT_LIST_SUCCESS;


	/* CLEANUP */
	cleanup:

	trotFree( newRefNode );

	return rc;
}

/******************************************************************************/
static inline int _refListRemove( trotList *l, trotListRef *r )
{
	/* DATA */
	trotListRefListNode *refNode = NULL;

	int i = 0;


	/* CODE */
	/* foreach refNode */
	refNode = l -> refListHead -> next;
	while ( refNode != l -> refListTail )
	{
		/* foreach pointer in this node */
		i = 0;
		while ( i < refNode -> count )
		{
			/* is this the ref we're looking for? */
			if ( refNode -> r[ i ] == r )
			{
				/* found, now remove it */
				while ( i < ( ( refNode -> count ) - 1 ) )
				{
					refNode -> r[ i ] = refNode -> r[ i + 1 ];

					i += 1;
				}

				refNode -> r[ i ] = NULL;
				
				refNode -> count -= 1;

				/* remove node if node is empty */
				if ( refNode -> count == 0 )
				{
					refNode -> prev -> next = refNode -> next;
					refNode -> next -> prev = refNode -> prev;

					trotFree( refNode -> r );
					trotFree( refNode );
				}

				return TROT_LIST_SUCCESS;
			}

			i += 1;
		}

		refNode = refNode -> next;
	}

	return TROT_LIST_ERROR_GENERAL;
}

/******************************************************************************/
static inline int _isListReachable( trotList *l )
{
	/* DATA */
	int rc = TROT_LIST_SUCCESS;

	trotStack *stack = NULL;
	trotStackNode *stackNode = NULL;
	TROT_STACK_CONTAINS contains = TROT_STACK_DOES_NOT_CONTAIN;

	trotList *currentL = NULL;

	trotListRefListNode *refNode = NULL;

	int j = 0;

	trotList *refParent = NULL;


	/* CODE */
	rc = trotStackInit( &stack );
	ERR_IF( rc != TROT_LIST_SUCCESS, rc );

	/* add first list to stack */
	rc = trotStackPush( stack, l );
	ERR_IF( rc != TROT_LIST_SUCCESS, rc );

	/* go through stack */
	stackNode = stack -> head -> next;
	while ( stackNode != stack -> tail )
	{
		currentL = stackNode -> l;

		/* for each reference that points to this list */
		refNode = currentL -> refListHead;
		while ( refNode != currentL -> refListTail )
		{
			j = 0;
			while ( j < refNode -> count )
			{
				/* get list this ref is in */
				refParent = refNode -> r[ j ] -> lParent;

				/* if ref has no parent, it means it's a client
				   reference, and so the list is reachable */
				if ( refParent == NULL )
				{
					goto cleanup;
				}

				if ( refParent -> reachable )
				{
					/* add list to stack if it's not already there */
					rc = trotStackQueryContains( stack, refParent, &contains );
					ERR_IF( rc != TROT_LIST_SUCCESS, rc );

					if ( contains == TROT_STACK_DOES_NOT_CONTAIN )
					{
						rc = trotStackPush( stack, refParent );
						ERR_IF( rc != TROT_LIST_SUCCESS, rc );
					}
				}

				j += 1;
			}

			refNode = refNode -> next;
		}

		/* *** */
		stackNode = stackNode -> next;
	}

	/* if we've gotten here, we haven't found any client refs, so the list
	   isn't reachable */
	l -> reachable = 0;

	/* CLEANUP */
	cleanup:

	/* free stack */
	trotStackFree( &stack );

	return rc;
}

