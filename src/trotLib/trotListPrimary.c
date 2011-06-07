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
static inline int _gkListNodeSplit( gkListNode *n, int keepInLeft );

static inline int _newIntNode( gkListNode **n_A );
static inline int _newListNode( gkListNode **n_A );

static inline int _refListAdd( gkList *l, gkListRef *r );
static inline int _refListRemove( gkList *l, gkListRef *r );

static inline int _isListReachable( gkList *l );

/******************************************************************************/
/*!
	\brief Allocates a new gkListRef reference to a new list.
	\param lr_A Pointer to a gkListRef pointer that must be NULL. On
		success, this will point to a new gkListRef reference.
	\return 0 on success, <0 on error
*/
int gkListRefInit( gkListRef **lr_A )
{
	/* DATA */
	int rc = GK_LIST_SUCCESS;

	gkListRefListNode *newRefHead = NULL;
	gkListRefListNode *newRefTail = NULL;

	gkListNode *newHead = NULL;
	gkListNode *newTail = NULL;
	gkList *newList = NULL;
	gkListRef *newListRef = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( lr_A == NULL );
	PRECOND_ERR_IF( (*lr_A) != NULL );


	/* CODE */
	/* create list of refs that point to this list */
	newRefHead = (gkListRefListNode *) gkMalloc( sizeof( gkListRefListNode ) );
	ERR_IF( newRefHead == NULL, GK_LIST_ERROR_MEMORY_ALLOCATION_FAILED );

	newRefTail = (gkListRefListNode *) gkMalloc( sizeof( gkListRefListNode ) );
	ERR_IF( newRefTail == NULL, GK_LIST_ERROR_MEMORY_ALLOCATION_FAILED );

	newRefHead -> count = 0;
	newRefHead -> r = NULL;
	newRefHead -> next = newRefTail;
	newRefHead -> prev = newRefHead;

	newRefTail -> count = 0;
	newRefTail -> r = NULL;
	newRefTail -> next = newRefTail;
	newRefTail -> prev = newRefHead;

	/* create the data list */
	newHead = (gkListNode *) gkMalloc( sizeof( gkListNode ) );
	ERR_IF( newHead == NULL, GK_LIST_ERROR_MEMORY_ALLOCATION_FAILED );

	newTail = (gkListNode *) gkMalloc( sizeof( gkListNode ) );
	ERR_IF( newTail == NULL, GK_LIST_ERROR_MEMORY_ALLOCATION_FAILED );

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
	newList = (gkList *) gkMalloc( sizeof( gkList ) );
	ERR_IF( newList == NULL, GK_LIST_ERROR_MEMORY_ALLOCATION_FAILED );

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
	newListRef = (gkListRef *) gkMalloc( sizeof( gkListRef ) );
	ERR_IF( newListRef == NULL, GK_LIST_ERROR_MEMORY_ALLOCATION_FAILED );

	newListRef -> lParent = NULL;

	newListRef -> lPointsTo = newList;
	newList = NULL;

	/* add first ref to list's ref list */
	rc = _refListAdd( newListRef -> lPointsTo, newListRef );
	ERR_IF( rc != 0, rc );

	/* give back */
	(*lr_A) = newListRef;
	newListRef = NULL;

	return 0;


	/* CLEANUP */
	cleanup:

	gkFree( newRefHead );
	gkFree( newRefTail );
	gkFree( newHead );
	gkFree( newTail );
	gkFree( newList );
	gkFree( newListRef );

	return rc;
}

/******************************************************************************/
/*!
	\brief Twins a gkListRef reference.
	\param lr_A Pointer to a gkListRef pointer that must be NULL. On
		success, this will point to a new gkListRef reverence.
	\param lrToTwin Pointer to the gkListRef pointer to be twinned.
	\return 0 on success, <0 on error
*/
int gkListRefTwin( gkListRef **lr_A, gkListRef *lrToTwin )
{
	/* DATA */
	int rc = GK_LIST_SUCCESS;

	gkListRef *newListRef = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( lr_A == NULL );
	PRECOND_ERR_IF( (*lr_A) != NULL );
	PRECOND_ERR_IF( lrToTwin == NULL );


	/* CODE */
	newListRef = (gkListRef *) gkMalloc( sizeof( gkListRef ) );
	ERR_IF( newListRef == NULL, GK_LIST_ERROR_MEMORY_ALLOCATION_FAILED );

	newListRef -> lParent = NULL;
	newListRef -> lPointsTo = lrToTwin -> lPointsTo;

	rc = _refListAdd( newListRef -> lPointsTo, newListRef );
	ERR_IF( rc != 0, rc );


	/* give back */
	(*lr_A) = newListRef;
	newListRef = NULL;

	return 0;


	/* CLEANUP */
	cleanup:

	gkFree( newListRef );

	return rc;
}

/******************************************************************************/
/*!
	\brief Frees a gkListRef reference. Actual list will be gkFreed if this
		is the last reference.
	\param lr_F Pointer to a gkListRef pointer.
		(*lr_F) can be NULL, in which case nothing will happen.
		On return, (*lr_F) will be NULL.
	\return void
*/
int gkListRefFree( gkListRef **lr_F )
{
	/* DATA */
	int rc = GK_LIST_SUCCESS;

	gkList *list = NULL;

	trotStack *stack = NULL;
	trotStackNode *stackNode = NULL;
	TROT_STACK_CONTAINS contains = TROT_STACK_DOES_NOT_CONTAIN;

	gkListNode *node = NULL;

	int j = 0;
	gkList *tempList = NULL;

	gkList *currentL = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( lr_F == NULL );


	/* CODE */
	if ( lr_F == NULL || (*lr_F) == NULL )
	{
		return GK_LIST_SUCCESS;
	}

	ERR_IF( (*lr_F) -> lParent != NULL, GK_LIST_ERROR_GENERAL );

	list = (*lr_F) -> lPointsTo;

	/* remove ref from list's ref list */
	rc = _refListRemove( list, (*lr_F) );
	ERR_IF( rc != GK_LIST_SUCCESS, rc );

	/* free ref */
	gkFree( (*lr_F) );
	(*lr_F) = NULL;

	/* is list reachable? */
	rc = _isListReachable( list );
	ERR_IF( rc != GK_LIST_SUCCESS, rc );

	if ( list -> reachable )
	{
		return GK_LIST_SUCCESS;
	}

	/* we need to free it */

	/* create our stack */
	rc = trotStackInit( &stack );
	ERR_IF( rc != GK_LIST_SUCCESS, rc );

	/* add this list to stack */
	rc = trotStackPush( stack, list );
	ERR_IF( rc != GK_LIST_SUCCESS, rc );

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
				gkFree( node -> n );
			}
			else /* NODE_KIND_LIST */
			{
				for ( j = 0; j < node -> count; j += 1 )
				{
					tempList = node -> l[ j ] -> lPointsTo;
			
					rc = _refListRemove( tempList, node -> l[ j ] );
					ERR_IF( rc != GK_LIST_SUCCESS, rc ); /* TODO: what state are we in if we error here??? */

					gkFree( node -> l[ j ] );
					node -> l[ j ] = NULL;

					if ( tempList -> reachable == 1 )
					{
						rc = _isListReachable( tempList );
						ERR_IF( rc != GK_LIST_SUCCESS, rc ); /* TODO: what state are we in if we error here??? */

						if ( tempList -> reachable == 0 )
						{
							/* need to free this list too, so add it to the stack */
							rc = trotStackQueryContains( stack, tempList, &contains );
							ERR_IF( rc != GK_LIST_SUCCESS, rc ); /* TODO: what state are we in if we error here??? */
							if ( contains == TROT_STACK_DOES_NOT_CONTAIN )
							{
								rc = trotStackPush( stack, tempList );
								ERR_IF( rc != GK_LIST_SUCCESS, rc ); /* TODO: what state are we in if we error here??? */
							}
						}
					}
				}

				gkFree( node -> l );
			}

			node = node -> next;
			gkFree( node -> prev );
		}

		gkFree( currentL -> head );
		currentL -> head = NULL;

		gkFree( currentL -> tail );
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
	
		gkFree( currentL -> refListHead );
		gkFree( currentL -> refListTail );

		gkFree( currentL );

		/* *** */
		stackNode = stackNode -> next;
	}

	trotStackFree( &stack );

	return GK_LIST_SUCCESS;

	/* CLEANUP */
	cleanup:

	/* TODO: what state are we in??? */

	return rc;
}

/******************************************************************************/
/*!
	\brief Gets the count of items in the list.
	\param lr Pointer to a gkListRef pointer.
	\param c On success, will contain the count of this list.
	\return 0 on success, <0 on error
*/
int gkListRefGetCount( gkListRef *lr, INT_TYPE *c )
{
	/* DATA */
	//int rc = GK_LIST_SUCCESS;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );
	PRECOND_ERR_IF( c == NULL );


	/* CODE */
	(*c) = lr -> lPointsTo -> childrenCount;

	return 0;
}

/******************************************************************************/
/*!
	\brief Gets the kind of an item in the list.
	\param lr Pointer to a gkListRef pointer.
	\param index Index of the item.
	\param kind On success, will contain the kind of the item.
	\return 0 on success, <0 on error
*/
int gkListRefGetKind( gkListRef *lr, INT_TYPE index, int *kind )
{
	/* DATA */
	int rc = GK_LIST_SUCCESS;

	gkListNode *node = NULL;
	gkListNode *tail = NULL;

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
	ERR_IF( index <= 0, GK_LIST_ERROR_BAD_INDEX );
	ERR_IF( index > (lr -> lPointsTo -> childrenCount ), GK_LIST_ERROR_BAD_INDEX );

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

	ERR_IF( node == tail, GK_LIST_ERROR_GENERAL );

	(*kind) = node -> kind;

	return 0;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Appends an int to the end of the list.
	\param lr Pointer to a gkListRef pointer.
	\param n The int value to insert.
	\return 0 on success, <0 on error
*/
int gkListRefAppendInt( gkListRef *lr, INT_TYPE n )
{
	/* DATA */
	int rc = GK_LIST_SUCCESS;

	gkList *l = NULL;
	gkListNode *node = NULL;

	gkListNode *newNode = NULL;


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

	return 0;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Appends a list twin to the end of the list.
	\param lr Pointer to a gkListRef pointer.
	\param lrToAppend The list to twin and append.
	\return 0 on success, <0 on error
*/
int gkListRefAppendListTwin( gkListRef *lr, gkListRef *lrToAppend )
{
	/* DATA */
	int rc = GK_LIST_SUCCESS;

	gkList *l = NULL;
	gkListNode *node = NULL;

	gkListNode *newNode = NULL;

	gkListRef *newLr = NULL;


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
	rc = gkListRefTwin( &newLr, lrToAppend );
	ERR_IF( rc != 0, rc );

	node -> l[ node -> count ] = newLr;
	newLr -> lParent = l;
	newLr = NULL;

	node -> count += 1;

	l -> childrenCount += 1;

	return 0;


	/* CLEANUP */
	cleanup:

	gkListRefFree( &newLr );

	return rc;
}

/******************************************************************************/
/*!
	\brief Inserts an int into the list.
	\param lr Pointer to a gkListRef pointer.
	\param index Where to insert.
	\param n The int value to insert.
	\return 0 on success, <0 on error
*/
int gkListRefInsertInt( gkListRef *lr, INT_TYPE index, INT_TYPE n )
{
	/* DATA */
	int rc = GK_LIST_SUCCESS;

	gkList *l = NULL;

	gkListNode *node = NULL;
	gkListNode *tail = NULL;

	INT_TYPE count = 0;

	int i = 0;
	int j = 0;

	gkListNode *newNode = NULL;


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
		rc = gkListRefAppendInt( lr, n );
		ERR_IF( rc != 0, rc );

		return 0;
	}

	/* Make sure index is in range */
	ERR_IF( index <= 0, GK_LIST_ERROR_BAD_INDEX );
	ERR_IF( index > (l -> childrenCount), GK_LIST_ERROR_BAD_INDEX );

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
	ERR_IF( node == tail, GK_LIST_ERROR_GENERAL );

	/* *** */
	if ( node -> kind == NODE_KIND_INT )
	{
		/* If node is full */
		if ( node -> count == NODE_SIZE )
		{
			rc = _gkListNodeSplit( node, NODE_SIZE / 2 );
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

		return 0;
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

			return 0;
		}

		/* if not at beginning, we'll have to split the node */
		if ( i != 0 )
		{
			rc = _gkListNodeSplit( node, i );
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

		return 0;
	}


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Inserts a twin of a list into the list.
	\param lr Pointer to a gkListRef pointer.
	\param index Where to insert.
	\param lToInsert The listRef to insert.
	\return 0 on success, <0 on error
*/
int gkListRefInsertListTwin( gkListRef *lr, INT_TYPE index, gkListRef *lToInsert )
{
	/* DATA */
	int rc = GK_LIST_SUCCESS;

	gkList *l = NULL;

	gkListNode *node = NULL;
	gkListNode *tail = NULL;

	INT_TYPE count = 0;

	int i = 0;
	int j = 0;

	gkListNode *newNode = NULL;

	gkListRef *newL = NULL;


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
		rc = gkListRefAppendListTwin( lr, lToInsert );
		ERR_IF( rc != 0, rc );

		return 0;
	}

	/* Make sure index is in range */
	ERR_IF( index <= 0, GK_LIST_ERROR_BAD_INDEX );
	ERR_IF( index > (l -> childrenCount ), GK_LIST_ERROR_BAD_INDEX );

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
	ERR_IF( node == tail, GK_LIST_ERROR_GENERAL );

	/* *** */
	if ( node -> kind == NODE_KIND_LIST )
	{
		/* If node is full */
		if ( node -> count == NODE_SIZE )
		{
			rc = _gkListNodeSplit( node, NODE_SIZE / 2 );
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
		rc = gkListRefTwin( &newL, lToInsert );
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

		return 0;
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
			rc = gkListRefTwin( &newL, lToInsert );
			ERR_IF( rc != 0, rc );

			node -> l[ node -> count ] = newL;
			newL -> lParent = l;
			newL = NULL;

			node -> count += 1;

			l -> childrenCount += 1;

			return 0;
		}

		/* if not at beginning, we'll have to split the node */
		if ( i != 0 )
		{
			rc = _gkListNodeSplit( node, i );
			ERR_IF( rc != 0, rc );

			node = node -> next;
		}

		/* *** */
		rc = _newListNode( &newNode );
		ERR_IF( rc != 0, rc );

		/* *** */
		rc = gkListRefTwin( &newL, lToInsert );
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

		return 0;
	}


	/* CLEANUP */
	cleanup:

	gkListRefFree( &newL );

	return rc;
}

/******************************************************************************/
/*!
	\brief Gets copy of int in list.
	\param lr Pointer to a gkListRef pointer.
	\param index Which int to get.
	\param n On success, will point to int.
	\return 0 on success, <0 on error
*/
int gkListRefGetInt( gkListRef *lr, INT_TYPE index, INT_TYPE *n )
{
	/* DATA */
	int rc = GK_LIST_SUCCESS;

	gkListNode *node = NULL;
	gkListNode *tail = NULL;

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
	ERR_IF( index <= 0, GK_LIST_ERROR_BAD_INDEX );
	ERR_IF( index > (lr -> lPointsTo -> childrenCount ), GK_LIST_ERROR_BAD_INDEX );

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

	ERR_IF( node == tail, GK_LIST_ERROR_GENERAL );

	ERR_IF( node -> kind != NODE_KIND_INT, GK_LIST_ERROR_WRONG_KIND );

	/* give back */
	(*n) = node -> n[ (node -> count) - 1 - (count - index) ];

	return 0;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Gets list ref of list in list.
	\param lr Pointer to a gkListRef pointer.
	\param index Which list to get.
	\param l On success, will point to list ref.
	\return 0 on success, <0 on error
*/
int gkListRefGetListTwin( gkListRef *lr, INT_TYPE index, gkListRef **l )
{
	/* DATA */
	int rc = GK_LIST_SUCCESS;

	gkListNode *node = NULL;
	gkListNode *tail = NULL;

	INT_TYPE count = 0;

	gkListRef *newL = NULL;


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
	ERR_IF( index <= 0, GK_LIST_ERROR_BAD_INDEX );
	ERR_IF( index > (lr -> lPointsTo -> childrenCount ), GK_LIST_ERROR_BAD_INDEX );

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

	ERR_IF( node == tail, GK_LIST_ERROR_GENERAL );

	ERR_IF( node -> kind != NODE_KIND_LIST, GK_LIST_ERROR_WRONG_KIND );

	rc = gkListRefTwin( &newL, node -> l[ (node -> count) - 1 - (count - index) ] );
	ERR_IF( rc != 0, rc );

	/* give back */
	(*l) = newL;
	newL = NULL;

	return 0;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Gets and removes int ref of list in list.
	\param lr Pointer to a gkListRef pointer.
	\param index Which int to get and remove.
	\param n On success, will point to int.
	\return 0 on success, <0 on error
*/
int gkListRefRemoveInt( gkListRef *lr, INT_TYPE index, INT_TYPE *n )
{
	/* DATA */
	int rc = GK_LIST_SUCCESS;

	gkListNode *node = NULL;
	gkListNode *tail = NULL;

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
	ERR_IF( index <= 0, GK_LIST_ERROR_BAD_INDEX );
	ERR_IF( index > (lr -> lPointsTo -> childrenCount ), GK_LIST_ERROR_BAD_INDEX );

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

	ERR_IF( node == tail, GK_LIST_ERROR_GENERAL );

	ERR_IF( node -> kind != NODE_KIND_INT, GK_LIST_ERROR_WRONG_KIND );

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

		gkFree( node -> n );
		gkFree( node );
	}

	/* give back */
	(*n) = giveBackN;

	return 0;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Gets and removes list ref of list in list.
	\param lr Pointer to a gkListRef pointer.
	\param index Which list to get.
	\param l On success, will point to list ref.
	\return 0 on success, <0 on error
*/
int gkListRefRemoveList( gkListRef *lr, INT_TYPE index, gkListRef **l )
{
	/* DATA */
	int rc = GK_LIST_SUCCESS;

	gkListNode *node = NULL;
	gkListNode *tail = NULL;

	INT_TYPE count = 0;

	gkListRef *giveBackL = NULL;

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
	ERR_IF( index <= 0, GK_LIST_ERROR_BAD_INDEX );
	ERR_IF( index > (lr -> lPointsTo -> childrenCount ), GK_LIST_ERROR_BAD_INDEX );

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

	ERR_IF( node == tail, GK_LIST_ERROR_GENERAL );

	ERR_IF( node -> kind != NODE_KIND_LIST, GK_LIST_ERROR_WRONG_KIND );

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

		gkFree( node -> l );
		gkFree( node );
	}

	/* give back */
	(*l) = giveBackL;

	return 0;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Removes whatever is at index in list.
	\param lr Pointer to a gkListRef pointer.
	\param index What to remove.
	\return 0 on success, <0 on error
*/
int gkListRefRemove( gkListRef *lr, INT_TYPE index )
{
	/* DATA */
	int rc = GK_LIST_SUCCESS;

	gkListNode *node = NULL;
	gkListNode *tail = NULL;

	INT_TYPE count = 0;

	gkListRef *tempL = NULL;

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
	ERR_IF( index <= 0, GK_LIST_ERROR_BAD_INDEX );
	ERR_IF( index > (lr -> lPointsTo -> childrenCount ), GK_LIST_ERROR_BAD_INDEX );

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

	ERR_IF( node == tail, GK_LIST_ERROR_GENERAL );

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
		gkListRefFree( &tempL );
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
			gkFree( node -> n );
		}
		else
		{
			gkFree( node -> l );
		}
		gkFree( node );
	}

	return 0;


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
	\return 0 on success, <0 on error
*/
static inline int _gkListNodeSplit( gkListNode *n, int keepInLeft )
{
	/* DATA */
	int rc = GK_LIST_SUCCESS;

	gkListNode *newNode = NULL;

	int i = 0;


	/* PRECOND */
	PRECOND_ERR_IF( n == NULL );


	/* CODE */
	newNode = (gkListNode *) gkMalloc( sizeof( gkListNode ) );
	ERR_IF( newNode == NULL, GK_LIST_ERROR_MEMORY_ALLOCATION_FAILED );

	if ( n -> kind == NODE_KIND_INT )
	{
		newNode -> kind = NODE_KIND_INT;
		newNode -> count = (n -> count) - keepInLeft;

		newNode -> l = NULL;
		newNode -> n = (INT_TYPE *) gkMalloc( sizeof( INT_TYPE * ) * NODE_SIZE );
		ERR_IF( newNode -> n == NULL, GK_LIST_ERROR_MEMORY_ALLOCATION_FAILED );

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
		newNode -> l = (gkListRef **) gkCalloc( NODE_SIZE, sizeof( gkListRef * ) );
		ERR_IF( newNode -> l == NULL, GK_LIST_ERROR_MEMORY_ALLOCATION_FAILED );

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

	return 0;


	/* CLEANUP */
	cleanup:

	gkFree( newNode );

	return rc;
}

/******************************************************************************/
static inline int _newIntNode( gkListNode **n_A )
{
	/* DATA */
	int rc = GK_LIST_SUCCESS;

	gkListNode *newNode = NULL;


	/* CODE */
	newNode = (gkListNode *) gkMalloc( sizeof( gkListNode ) );
	ERR_IF( newNode == NULL, GK_LIST_ERROR_MEMORY_ALLOCATION_FAILED );

	newNode -> kind = NODE_KIND_INT;
	newNode -> count = 0;

	newNode -> l = NULL;
	newNode -> n = (INT_TYPE *) gkMalloc( sizeof( INT_TYPE * ) * NODE_SIZE );
	ERR_IF( newNode -> n == NULL, GK_LIST_ERROR_MEMORY_ALLOCATION_FAILED );

	/* give back */
	(*n_A) = newNode;
	newNode = NULL;

	return 0;


	/* CLEANUP */
	cleanup:

	gkFree( newNode );

	return rc;
}

/******************************************************************************/
static inline int _newListNode( gkListNode **n_A )
{
	/* DATA */
	int rc = GK_LIST_SUCCESS;

	gkListNode *newNode = NULL;


	/* CODE */
	newNode = (gkListNode *) gkMalloc( sizeof( gkListNode ) );
	ERR_IF( newNode == NULL, GK_LIST_ERROR_MEMORY_ALLOCATION_FAILED );

	newNode -> kind = NODE_KIND_LIST;
	newNode -> count = 0;

	newNode -> n = NULL;
	newNode -> l = (gkListRef **) gkCalloc( NODE_SIZE, sizeof( gkListRef * ) );
	ERR_IF( newNode -> l == NULL, GK_LIST_ERROR_MEMORY_ALLOCATION_FAILED );

	/* give back */
	(*n_A) = newNode;
	newNode = NULL;

	return 0;


	/* CLEANUP */
	cleanup:

	gkFree( newNode );

	return rc;
}

/******************************************************************************/
static inline int _refListAdd( gkList *l, gkListRef *r )
{
	/* DATA */
	int rc = GK_LIST_SUCCESS;

	gkListRefListNode *refNode = NULL;

	gkListRefListNode *newRefNode = NULL;


	/* CODE */
	refNode = l -> refListHead -> next;
	while ( refNode != l -> refListTail )
	{
		if ( refNode -> count < REF_LIST_NODE_SIZE )
		{
			refNode -> r[ refNode -> count ] = r;
			refNode -> count += 1;

			return 0;
		}

		refNode = refNode -> next;
	}

	/* there was no room in list, so create new node, insert ref into new
	   node, and insert node into list */
	newRefNode = (gkListRefListNode *) gkMalloc( sizeof( gkListRefListNode ) );
	ERR_IF( newRefNode == NULL, GK_LIST_ERROR_MEMORY_ALLOCATION_FAILED );

	newRefNode -> r = (gkListRef **) gkCalloc( REF_LIST_NODE_SIZE, sizeof( gkListRef * ) );
	ERR_IF( newRefNode -> r == NULL, GK_LIST_ERROR_MEMORY_ALLOCATION_FAILED );

	newRefNode -> count = 1;
	newRefNode -> r[ 0 ] = r;

	newRefNode -> prev = l -> refListTail -> prev;
	newRefNode -> next = l -> refListTail;
	l -> refListTail -> prev -> next = newRefNode;
	l -> refListTail -> prev = newRefNode;

	return 0;


	/* CLEANUP */
	cleanup:

	gkFree( newRefNode );

	return rc;
}

/******************************************************************************/
static inline int _refListRemove( gkList *l, gkListRef *r )
{
	/* DATA */
	gkListRefListNode *refNode = NULL;

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

					gkFree( refNode -> r );
					gkFree( refNode );
				}

				return GK_LIST_SUCCESS;
			}

			i += 1;
		}

		refNode = refNode -> next;
	}

	return GK_LIST_ERROR_GENERAL;
}

/******************************************************************************/
static inline int _isListReachable( gkList *l )
{
	/* DATA */
	int rc = GK_LIST_SUCCESS;

	trotStack *stack = NULL;
	trotStackNode *stackNode = NULL;
	TROT_STACK_CONTAINS contains = TROT_STACK_DOES_NOT_CONTAIN;

	gkList *currentL = NULL;

	gkListRefListNode *refNode = NULL;

	int j = 0;

	gkList *refParent = NULL;


	/* CODE */
	rc = trotStackInit( &stack );
	ERR_IF( rc != GK_LIST_SUCCESS, rc );

	/* add first list to stack */
	rc = trotStackPush( stack, l );
	ERR_IF( rc != GK_LIST_SUCCESS, rc );

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
					ERR_IF( rc != GK_LIST_SUCCESS, rc );

					if ( contains == TROT_STACK_DOES_NOT_CONTAIN )
					{
						rc = trotStackPush( stack, refParent );
						ERR_IF( rc != GK_LIST_SUCCESS, rc );
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

