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
	Implements the primary funcitonality of "Hoof", our single data
	structure for Trot.

	Primary functionality includes:
	- List Init
	- List Twin (create another reference to a list, all lists are
	  manipulated by reference)
	- List Free
	- Get Count (how many children does the list have?)
	- Get Kind (is child at index N an int or list?)
	- Append Int
	- Append List Twin
	- Get Int
	- Get List Twin
	- Remove Int
	- Remove List Twin
	- Remove
*/

/* TODO: make sure inserts and appends don't add more children than our maximum positive INT_TYPE. then test that it works, and that minimum negative INT_TYPE still errors correctly */

/******************************************************************************/
#include "trot.h"
#include "trotInternal.h"

/******************************************************************************/
static TROT_RC _refListAdd( trotList *l, trotListRef *r );
static void _refListRemove( trotList *l, trotListRef *r );

static void _isListReachable( trotList *l );
static int _findNextParent( trotList *l, int queryVisited, trotList **parent );

/******************************************************************************/
/*!
	\brief Allocates a new trotListRef reference to a new list.
	\param lr_A Pointer to a trotListRef pointer that must be NULL. On
		success, this will point to a new trotListRef reference.
	\return TROT_RC
*/
TROT_RC trotListRefInit( trotListRef **lr_A )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

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
	newList -> flagVisited = 0;
	newList -> previous = NULL;
	newList -> nextToFree = NULL;

	newList -> encodingParent = NULL;
	newList -> encodingChildNumber = 0;

	newList -> tag = TROT_TAG_DATA;

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
	ERR_IF_PASSTHROUGH;

	/* give back */
	(*lr_A) = newListRef;
	newListRef = NULL;

	return TROT_LIST_SUCCESS; /* TODO: go through other functions, and see if we can return success before cleanup */


	/* CLEANUP */
	cleanup:

	trotFree( newRefHead );
	trotFree( newRefTail );
	trotFree( newHead );
	trotFree( newTail );
	if ( newList != NULL )
	{
		trotFree( newList -> refListHead );
		trotFree( newList -> refListTail );
		trotFree( newList -> head );
		trotFree( newList -> tail );
		trotFree( newList );
	}
	if ( newListRef != NULL )
	{
		trotFree( newListRef -> lPointsTo -> refListHead );
		trotFree( newListRef -> lPointsTo -> refListTail );
		trotFree( newListRef -> lPointsTo -> head );
		trotFree( newListRef -> lPointsTo -> tail );
		trotFree( newListRef -> lPointsTo );
		trotFree( newListRef );
	}

	return rc;
}

/******************************************************************************/
/*!
	\brief Twins a trotListRef reference.
	\param lr Pointer to the trotListRef pointer to be twinned.
	\param lrTwin_A Pointer to a trotListRef pointer that must be NULL. On
		success, this will point to a new trotListRef reverence.
	\return TROT_RC
*/
TROT_RC trotListRefTwin( trotListRef *lr, trotListRef **lrTwin_A )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotListRef *newListRef = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );
	PRECOND_ERR_IF( lrTwin_A == NULL );
	PRECOND_ERR_IF( (*lrTwin_A) != NULL );


	/* CODE */
	TROT_MALLOC( newListRef, trotListRef, 1 );

	newListRef -> lParent = NULL;
	newListRef -> lPointsTo = lr -> lPointsTo;

	rc = _refListAdd( newListRef -> lPointsTo, newListRef );
	ERR_IF_PASSTHROUGH;


	/* give back */
	(*lrTwin_A) = newListRef;
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
void trotListRefFree( trotListRef **lr_F )
{
	/* DATA */
	trotList *list = NULL;

	trotListNode *node = NULL;

	int j = 0;
	trotList *tempList = NULL;

	trotList *nextL = NULL;
	trotList *currentL = NULL;


	/* CODE */
	ERR_IF_PARANOID( lr_F == NULL );

	if ( (*lr_F) == NULL )
	{
		return;
	}

	ERR_IF_PARANOID( (*lr_F) -> lParent != NULL );

	list = (*lr_F) -> lPointsTo;

	/* remove ref from list's ref list */
	_refListRemove( list, (*lr_F) );

	/* free ref */
	trotFree( (*lr_F) );
	(*lr_F) = NULL;

	/* is list reachable? */
	_isListReachable( list );
	if ( list -> reachable )
	{
		return;
	}

	/* we need to free it */

	/* go through stack */
	currentL = list;
	while ( currentL != NULL )
	{
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
			
					_refListRemove( tempList, node -> l[ j ] );

					trotFree( node -> l[ j ] );
					node -> l[ j ] = NULL;

					if ( tempList -> reachable == 1 )
					{
						_isListReachable( tempList );
						if ( tempList -> reachable == 0 )
						{
							/* need to free this list too */
							tempList -> nextToFree = currentL -> nextToFree;
							currentL -> nextToFree = tempList;
						}
					}
				}

				trotFree( node -> l );
			}

			node = node -> next;
			trotFree( node -> prev );
		}

		currentL = currentL -> nextToFree;
	}

	/* *** */
	nextL = list;
	while ( nextL != NULL )
	{
		/* *** */
		currentL = nextL;

		/* *** */
		nextL = nextL -> nextToFree;

		/* *** */
		trotFree( currentL -> head );
		trotFree( currentL -> tail );
		trotFree( currentL -> refListHead );
		trotFree( currentL -> refListTail );
		trotFree( currentL );
	}

	return;
}

/******************************************************************************/
/*!
	\brief Gets the count of items in the list.
	\param lr Pointer to a trotListRef pointer.
	\param c On success, will contain the count of this list.
	\return TROT_RC
*/
TROT_RC trotListRefGetCount( trotListRef *lr, INT_TYPE *c )
{
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
	\return TROT_RC
*/
TROT_RC trotListRefGetKind( trotListRef *lr, INT_TYPE index, TROT_KIND *kind )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotList *l = NULL;

	trotListNode *node = NULL;

	INT_TYPE count = 0;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );
	PRECOND_ERR_IF( kind == NULL );


	/* CODE */
	l = lr -> lPointsTo;

	/* Turn negative index into positive equivalent. */
	if ( index < 0 )
	{
		index = (l -> childrenCount) + index + 1;
	}

	/* Make sure index is in range */
	ERR_IF( index <= 0, TROT_LIST_ERROR_BAD_INDEX );
	ERR_IF( index > (l -> childrenCount ), TROT_LIST_ERROR_BAD_INDEX );

	/* *** */
	node = l -> head -> next;
	while ( 1 )
	{
		count += node -> count;
		if ( count >= index )
		{
			break;
		}

		node = node -> next;

		ERR_IF_PARANOID( node == l -> tail );
	}

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
	\return TROT_RC
*/
TROT_RC trotListRefAppendInt( trotListRef *lr, INT_TYPE n )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotList *l = NULL;
	trotListNode *node = NULL;

	trotListNode *newNode = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );


	/* CODE */
	l = lr -> lPointsTo;

/* TODO: for all insert/appends, we need to make sure list doesn't contain maximum amount of children */

	/* *** */
	node = l -> tail -> prev;

	/* special cases to create new node */
	if (    node == l -> head             /* empty list */
	     || node -> kind != NODE_KIND_INT /* last node is not int kind */
	     || node -> count == NODE_SIZE    /* last node is full */
	   )
	{
		rc = newIntNode( &newNode );
		ERR_IF_PASSTHROUGH;

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
	\return TROT_RC
*/
TROT_RC trotListRefAppendListTwin( trotListRef *lr, trotListRef *lrToAppend )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

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
		rc = newListNode( &newNode );
		ERR_IF_PASSTHROUGH;

		newNode -> next = l -> tail;
		newNode -> prev = l -> tail -> prev;

		l -> tail -> prev -> next = newNode;
		l -> tail -> prev = newNode;

		node = newNode;
		newNode = NULL;
	}

	/* append */
	rc = trotListRefTwin( lrToAppend, &newLr );
	ERR_IF_PASSTHROUGH;

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
	\return TROT_RC
*/
TROT_RC trotListRefInsertInt( trotListRef *lr, INT_TYPE index, INT_TYPE n )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotList *l = NULL;

	trotListNode *node = NULL;

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
		ERR_IF_PASSTHROUGH;

		return TROT_LIST_SUCCESS;
	}

	/* Make sure index is in range */
	ERR_IF( index <= 0, TROT_LIST_ERROR_BAD_INDEX );
	ERR_IF( index > (l -> childrenCount), TROT_LIST_ERROR_BAD_INDEX );

	/* Find node where int needs to be added into */
	node = l -> head -> next;
	while ( 1 )
	{
		if ( count + (node -> count) >= index )
		{
			break;
		}

		count += node -> count;
		node = node -> next;

		ERR_IF_PARANOID( node == l -> tail );
	}

	/* *** */
	if ( node -> kind == NODE_KIND_INT )
	{
		/* If node is full */
		if ( node -> count == NODE_SIZE )
		{
			rc = trotListNodeSplit( node, NODE_SIZE / 2 );
			ERR_IF_PASSTHROUGH;

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
			rc = trotListNodeSplit( node, i );
			ERR_IF_PASSTHROUGH;

			node = node -> next;
		}

		/* *** */
		rc = newIntNode( &newNode );
		ERR_IF_PASSTHROUGH;

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
	\param lrToInsert The listRef to insert.
	\return TROT_RC
*/
TROT_RC trotListRefInsertListTwin( trotListRef *lr, INT_TYPE index, trotListRef *lrToInsert )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotList *l = NULL;

	trotListNode *node = NULL;

	INT_TYPE count = 0;

	int i = 0;
	int j = 0;

	trotListNode *newNode = NULL;

	trotListRef *newL = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );
	PRECOND_ERR_IF( lrToInsert == NULL );


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
		rc = trotListRefAppendListTwin( lr, lrToInsert );
		ERR_IF_PASSTHROUGH;

		return TROT_LIST_SUCCESS;
	}

	/* Make sure index is in range */
	ERR_IF( index <= 0, TROT_LIST_ERROR_BAD_INDEX );
	ERR_IF( index > (l -> childrenCount ), TROT_LIST_ERROR_BAD_INDEX );

	/* Find node where list needs to be added into */
	node = l -> head -> next;
	while ( 1 )
	{
		if ( count + (node -> count) >= index )
		{
			break;
		}

		count += node -> count;
		node = node -> next;

		ERR_IF_PARANOID( node == l -> tail );
	}

	/* *** */
	if ( node -> kind == NODE_KIND_LIST )
	{
		/* If node is full */
		if ( node -> count == NODE_SIZE )
		{
			rc = trotListNodeSplit( node, NODE_SIZE / 2 );
			ERR_IF_PASSTHROUGH;

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
		rc = trotListRefTwin( lrToInsert, &newL );
		ERR_IF_PASSTHROUGH;

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
			rc = trotListRefTwin( lrToInsert, &newL );
			ERR_IF_PASSTHROUGH;

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
			rc = trotListNodeSplit( node, i );
			ERR_IF_PASSTHROUGH;

			node = node -> next;
		}

		/* *** */
		rc = newListNode( &newNode );
		ERR_IF_PASSTHROUGH;

		/* *** */
		rc = trotListRefTwin( lrToInsert, &newL );
		ERR_IF_PASSTHROUGH;

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

	if ( newNode != NULL )
	{
		trotFree( newNode -> l );
		trotFree( newNode );
	}

	return rc;
}

/******************************************************************************/
/*!
	\brief Gets copy of int in list.
	\param lr Pointer to a trotListRef pointer.
	\param index Which int to get.
	\param n On success, will point to int.
	\return TROT_RC
*/
TROT_RC trotListRefGetInt( trotListRef *lr, INT_TYPE index, INT_TYPE *n )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotList *l = NULL;

	trotListNode *node = NULL;

	INT_TYPE count = 0;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );
	PRECOND_ERR_IF( n == NULL );


	/* CODE */
	l = lr -> lPointsTo;

	/* Turn negative index into positive equivalent. */
	if ( index < 0 )
	{
		index = (l -> childrenCount) + index + 1;
	}

	/* Make sure index is in range */
	ERR_IF( index <= 0, TROT_LIST_ERROR_BAD_INDEX );
	ERR_IF( index > (l -> childrenCount ), TROT_LIST_ERROR_BAD_INDEX );

	/* *** */
	node = l -> head -> next;
	while ( 1 )
	{
		count += node -> count;
		if ( count >= index )
		{
			break;
		}

		node = node -> next;

		ERR_IF_PARANOID( node == l -> tail );
	}

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
	\param lrTwin_A On success, will point to list ref.
	\return TROT_RC
*/
TROT_RC trotListRefGetListTwin( trotListRef *lr, INT_TYPE index, trotListRef **lrTwin_A )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotListNode *node = NULL;

	INT_TYPE count = 0;

	trotListRef *newL = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );
	PRECOND_ERR_IF( lrTwin_A == NULL );
	PRECOND_ERR_IF( (*lrTwin_A) != NULL );


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
	node = lr -> lPointsTo -> head -> next;
	while ( 1 )
	{
		count += node -> count;
		if ( count >= index )
		{
			break;
		}

		node = node -> next;

		ERR_IF_PARANOID( node == lr -> lPointsTo -> tail );
	}

	ERR_IF( node -> kind != NODE_KIND_LIST, TROT_LIST_ERROR_WRONG_KIND );

	rc = trotListRefTwin( node -> l[ (node -> count) - 1 - (count - index) ], &newL );
	ERR_IF_PASSTHROUGH;

	/* give back */
	(*lrTwin_A) = newL;
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
	\return TROT_RC
*/
TROT_RC trotListRefRemoveInt( trotListRef *lr, INT_TYPE index, INT_TYPE *n )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotListNode *node = NULL;

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
	node = lr -> lPointsTo -> head -> next;
	while ( 1 )
	{
		count += node -> count;
		if ( count >= index )
		{
			break;
		}

		node = node -> next;

		ERR_IF_PARANOID( node == lr -> lPointsTo -> tail );
	}

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
	\param lrRemoved_A On success, will point to list ref.
	\return TROT_RC
*/
TROT_RC trotListRefRemoveList( trotListRef *lr, INT_TYPE index, trotListRef **lrRemoved_A )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotListNode *node = NULL;

	INT_TYPE count = 0;

	trotListRef *giveBackL = NULL;

	int i = 0;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );
	PRECOND_ERR_IF( lrRemoved_A == NULL );
	PRECOND_ERR_IF( (*lrRemoved_A) != NULL );


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
	node = lr -> lPointsTo -> head -> next;
	while ( 1 )
	{
		count += node -> count;
		if ( count >= index )
		{
			break;
		}

		node = node -> next;

		ERR_IF_PARANOID( node == lr -> lPointsTo -> tail );
	}

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
	(*lrRemoved_A) = giveBackL;

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
	\return TROT_RC
*/
TROT_RC trotListRefRemove( trotListRef *lr, INT_TYPE index )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotListNode *node = NULL;

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
	node = lr -> lPointsTo -> head -> next;
	while ( 1 )
	{
		count += node -> count;
		if ( count >= index )
		{
			break;
		}

		node = node -> next;

		ERR_IF_PARANOID( node == lr -> lPointsTo -> tail );
	}

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
	\brief 
	\param 
	\return TROT_RC
*/
TROT_RC trotListRefReplaceWithInt( trotListRef *lr, INT_TYPE index, INT_TYPE n )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotList *l = NULL;

	trotListNode *node = NULL;

	INT_TYPE count = 0;

	trotListRef *tempL = NULL;

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
		index = (l -> childrenCount) + index + 1;
	}

	/* Make sure index is in range */
	ERR_IF( index <= 0, TROT_LIST_ERROR_BAD_INDEX );
	ERR_IF( index > (l -> childrenCount), TROT_LIST_ERROR_BAD_INDEX );

	/* Find node where int needs to be replaced into */
	node = l -> head -> next;
	while ( 1 )
	{
		if ( count + (node -> count) >= index )
		{
			break;
		}

		count += node -> count;
		node = node -> next;

		ERR_IF_PARANOID( node == l -> tail );
	}

	/* *** */
	if ( node -> kind == NODE_KIND_INT )
	{
		i = index - count - 1;

		/* replace int into node */
		node -> n[ i ] = n;
	}
	else /* node -> kind == NODE_KIND_LIST */
	{
		i = index - count - 1;

		/* If at beginning of node */
		if ( i == 0 )
		{
			/* If the previous node is an int node with space, we
			   can just append in that node. */
			if (    node -> prev -> kind == NODE_KIND_INT
			     && node -> prev -> count != NODE_SIZE 
			   )
			{
				/* append int into prev node */
				node -> prev -> n[ node -> prev -> count ] = n;

				node -> prev -> count += 1;
			}
			else
			{
				/* *** */
				rc = newIntNode( &newNode );
				ERR_IF_PASSTHROUGH;

				newNode -> n[ 0 ] = n;
				newNode -> count = 1;

				/* Insert node into list */
				newNode -> next = node;
				newNode -> prev = node -> prev;

				node -> prev -> next = newNode;
				node -> prev = newNode;
			}
		}
		/* else if end of node */
		else if ( i == (node -> count) - 1 )
		{
			/* if the next node is an int node with room, we can just prepend to
			   that node. */
			if (    node -> next -> kind == NODE_KIND_INT
			     && node -> next -> count != NODE_SIZE 
			   )
			{
				/* prepend int */
				j = node -> next -> count;
				while ( j != 0 )
				{
					node -> next -> n[ j ] = node -> next -> n[ j - 1 ];
					j -= 1;
				}

				node -> next -> n[ 0 ] = n;

				node -> next -> count += 1;
			}
			else
			{
				/* *** */
				rc = newIntNode( &newNode );
				ERR_IF_PASSTHROUGH;

				newNode -> n[ 0 ] = n;
				newNode -> count = 1;

				/* Insert node into list */
				newNode -> prev = node;
				newNode -> next = node -> next;

				node -> next -> prev = newNode;
				node -> next = newNode;
			}
		}
		/* we'll have to split the node */
		else
		{
			rc = trotListNodeSplit( node, i + 1 );
			ERR_IF_PASSTHROUGH;

			/* *** */
			rc = newIntNode( &newNode );
			ERR_IF_PASSTHROUGH;

			newNode -> n[ 0 ] = n;
			newNode -> count = 1;

			/* Insert node into list */
			newNode -> prev = node;
			newNode -> next = node -> next;

			node -> next -> prev = newNode;
			node -> next = newNode;
		}

		/* we've put in our int, now we need to remove a list */
		tempL = node -> l[ i ];
		tempL -> lParent = NULL;
		trotListRefFree( &tempL );
		while ( i < ( (node -> count) - 1 ) )
		{
			node -> l[ i ] = node -> l[ i + 1 ];
			i += 1;
		}
		node -> l[ i ] = NULL;

		node -> count -= 1;
		if ( node -> count == 0 )
		{
			node -> prev -> next = node -> next;
			node -> next -> prev = node -> prev;

			trotFree( node -> l );
			trotFree( node );
		}
	}


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief 
	\param 
	\return TROT_RC
*/
TROT_RC trotListRefReplaceWithList( trotListRef *lr, INT_TYPE index, trotListRef *lrToInsert )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotList *l = NULL;

	trotListNode *node = NULL;

	INT_TYPE count = 0;

	trotListRef *tempL = NULL;

	trotListRef *newLr = NULL;

	int i = 0;
	int j = 0;

	trotListNode *newNode = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );
	PRECOND_ERR_IF( lrToInsert == NULL );


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

	/* Find node where list needs to be replaced into */
	node = l -> head -> next;
	while ( 1 )
	{
		if ( count + (node -> count) >= index )
		{
			break;
		}

		count += node -> count;
		node = node -> next;

		ERR_IF_PARANOID( node == l -> tail );
	}

	/* create our new twin */
	rc = trotListRefTwin( lrToInsert, &newLr );
	ERR_IF_PASSTHROUGH;

	/* *** */
	if ( node -> kind == NODE_KIND_LIST )
	{
		i = index - count - 1;

		/* free old */
		tempL = node -> l[ i ];
		tempL -> lParent = NULL;
		trotListRefFree( &tempL );

		/* replace with new */
		node -> l[ i ] = newLr;
		newLr -> lParent = l;
		newLr = NULL;
	}
	else /* node -> kind == NODE_KIND_INT */
	{
		i = index - count - 1;

		/* If at beginning of node */
		if ( i == 0 )
		{
			/* If the previous node is a list node with space, we
			   can just append in that node. */
			if (    node -> prev -> kind == NODE_KIND_LIST
			     && node -> prev -> count != NODE_SIZE 
			   )
			{
				/* append into prev node */
				node -> prev -> l[ node -> prev -> count ] = newLr;
				newLr -> lParent = l;
				newLr = NULL;

				node -> prev -> count += 1;
			}
			else
			{
				/* *** */
				rc = newListNode( &newNode );
				ERR_IF_PASSTHROUGH;

				newNode -> l[ 0 ] = newLr;
				newLr -> lParent = l;
				newLr = NULL;

				newNode -> count = 1;

				/* Insert node into list */
				newNode -> next = node;
				newNode -> prev = node -> prev;

				node -> prev -> next = newNode;
				node -> prev = newNode;
			}
		}
		/* else if end of node */
		else if ( i == (node -> count) - 1 )
		{
			/* if the next node is a list node with room, we can just prepend to
			   that node. */
			if (    node -> next -> kind == NODE_KIND_LIST
			     && node -> next -> count != NODE_SIZE 
			   )
			{
				/* prepend int */
				j = node -> next -> count;
				while ( j != 0 )
				{
					node -> next -> l[ j ] = node -> next -> l[ j - 1 ];
					j -= 1;
				}

				node -> next -> l[ 0 ] = newLr;
				newLr -> lParent = l;
				newLr = NULL;

				node -> next -> count += 1;
			}
			else
			{
				/* *** */
				rc = newListNode( &newNode );
				ERR_IF_PASSTHROUGH;

				newNode -> l[ 0 ] = newLr;
				newLr -> lParent = l;
				newLr = NULL;

				newNode -> count = 1;

				/* Insert node into list */
				newNode -> prev = node;
				newNode -> next = node -> next;

				node -> next -> prev = newNode;
				node -> next = newNode;
			}
		}
		/* we'll have to split the node */
		else
		{
			rc = trotListNodeSplit( node, i + 1 );
			ERR_IF_PASSTHROUGH;

			/* *** */
			rc = newListNode( &newNode );
			ERR_IF_PASSTHROUGH;

			newNode -> l[ 0 ] = newLr;
			newLr -> lParent = l;
			newLr = NULL;

			newNode -> count = 1;

			/* Insert node into list */
			newNode -> prev = node;
			newNode -> next = node -> next;

			node -> next -> prev = newNode;
			node -> next = newNode;
		}

		/* we've put in our list, now we need to remove an int */
		while ( i < ( (node -> count) - 1 ) )
		{
			node -> n[ i ] = node -> n[ i + 1 ];
			i += 1;
		}

		node -> count -= 1;
		if ( node -> count == 0 )
		{
			node -> prev -> next = node -> next;
			node -> next -> prev = node -> prev;

			trotFree( node -> n );
			trotFree( node );
		}
	}


	/* CLEANUP */
	cleanup:

	trotListRefFree( &newLr );

	return rc;
}

/******************************************************************************/
/*!
	\brief 
	\param 
	\return TROT_RC
*/
TROT_RC trotListRefGetTag( trotListRef *lr, TROT_TAG *tag )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );
	PRECOND_ERR_IF( tag == NULL );


	/* CODE */
	(*tag) = lr -> lPointsTo -> tag;


	/* CLEANUP */
	/* cleanup: */

	return rc;
}

/******************************************************************************/
/*!
	\brief 
	\param 
	\return TROT_RC
*/
TROT_RC trotListRefSetTag( trotListRef *lr, TROT_TAG tag )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );


	/* CODE */
	ERR_IF( tag < TROT_TAG_MIN || tag > TROT_TAG_MAX, TROT_LIST_ERROR_BAD_TAG );

	lr -> lPointsTo -> tag = tag;


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
	\return TROT_RC
*/
TROT_RC trotListNodeSplit( trotListNode *n, int keepInLeft )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotListNode *newNode = NULL;

	int i = 0;


	/* CODE */
	ERR_IF_PARANOID( n == NULL );

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
/*!
	\brief Creates a new trotListNode for Int.
	\param n_A On success, the new malloc'd node.
	\return TROT_RC
*/
TROT_RC newIntNode( trotListNode **n_A )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotListNode *newNode = NULL;


	/* CODE */
	ERR_IF_PARANOID( n_A == NULL );
	ERR_IF_PARANOID( (*n_A) != NULL );

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
/*!
	\brief Creates a new trotListNode for List.
	\param n_A On success, the new malloc'd node.
	\return TROT_RC
*/
TROT_RC newListNode( trotListNode **n_A )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotListNode *newNode = NULL;


	/* CODE */
	ERR_IF_PARANOID( n_A == NULL );
	ERR_IF_PARANOID( (*n_A) != NULL );

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
static TROT_RC _refListAdd( trotList *l, trotListRef *r )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotListRefListNode *refNode = NULL;

	trotListRefListNode *newRefNode = NULL;


	/* CODE */
	ERR_IF_PARANOID( l == NULL );
	ERR_IF_PARANOID( r == NULL );

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
static void _refListRemove( trotList *l, trotListRef *r )
{
	/* DATA */
	trotListRefListNode *refNode = NULL;

	int i = 0;


	/* CODE */
	ERR_IF_PARANOID( l == NULL );
	ERR_IF_PARANOID( r == NULL );

	/* foreach refNode */
	refNode = l -> refListHead -> next;
	while ( 1 )
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

				return;
			}

			i += 1;
		}

		refNode = refNode -> next;

		ERR_IF_PARANOID( refNode == l -> refListTail );
	}

	ERR_IF_PARANOID( 1 );

	return;
}

/******************************************************************************/
static void _isListReachable( trotList *l )
{
	/* DATA */
	int flagFoundClientRef = 0;

	trotList *currentL = NULL;

	trotList *parent = NULL;

	trotList *tempL = NULL;


	/* CODE */
	ERR_IF_PARANOID( l == NULL );
	ERR_IF_PARANOID( l -> reachable == 0 );

	/* go "up" trying to find a client ref */
	currentL = l;
	currentL -> flagVisited = 1;

	while ( 1 )
	{
		if ( _findNextParent( currentL, 0, &parent ) != 0 )
		{
			if ( currentL -> previous != NULL )
			{
				tempL = currentL;
				currentL = currentL -> previous;
				tempL -> previous = NULL;

				continue;
			}

			break;
		}

		/* did we find a client ref? */
		if ( parent == NULL )
		{
			flagFoundClientRef = 1;
			break;
		}

		parent -> previous = currentL;
		currentL = parent;
		currentL -> flagVisited = 1;
	}

	if ( ! flagFoundClientRef )
	{
		l -> reachable = 0;
	}

	/* restart, go "up", resetting all the flagVisited flags to 0 */
	currentL = l;
	currentL -> flagVisited = 0;

	while ( 1 )
	{
		if ( _findNextParent( currentL, 1, &parent ) != 0 )
		{
			if ( currentL -> previous != NULL )
			{
				tempL = currentL;
				currentL = currentL -> previous;
				tempL -> previous = NULL;

				continue;
			}

			break;
		}

		parent -> previous = currentL;
		currentL = parent;
		currentL -> flagVisited = 0;
	}

	return;
}

/******************************************************************************/
static int _findNextParent( trotList *l, int queryVisited, trotList **parent )
{
	/* DATA */
	trotListRefListNode *refNode = NULL;

	int i = 0;

	trotList *tempParent = NULL;


	/* CODE */
	ERR_IF_PARANOID( l == NULL );
	ERR_IF_PARANOID( parent == NULL );

	/* for each reference that points to this list */
	refNode = l -> refListHead;
	while ( refNode != l -> refListTail )
	{
		i = 0;
		while ( i < refNode -> count )
		{
			/* get list this ref is in */
			tempParent = refNode -> r[ i ] -> lParent;

			/* if ref has no parent, it means it's a client
			   reference, and so the list is reachable, but we only
			   care if our caller is looking for not visited
			   parents */
			if ( tempParent == NULL )
			{
				if ( queryVisited == 0 )
				{
					(*parent) = NULL;
					return 0;
				}
			}
			/* if this matches our 'visit' query, then return it */
			else if ( tempParent -> flagVisited == queryVisited )
			{
				(*parent) = tempParent;
				return 0;
			}

			i += 1;
		}

		refNode = refNode -> next;
	}

	return -1;
}

