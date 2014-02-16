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
#define TROT_FILE_NUMBER 1

/* TODO: make sure inserts and appends don't add more children than our maximum positive TROT_INT. then test that it works, and that minimum negative TROT_INT still errors correctly. Also delist. */

/******************************************************************************/
#include "trot.h"
#include "trotInternal.h"

/******************************************************************************/
static TROT_RC _refListAdd( trotListActual *la, TrotList *l );
static void _refListRemove( trotListActual *la, TrotList *l );

static void _isListReachable( trotListActual *la );
static int _findNextParent( trotListActual *la, int queryVisited, trotListActual **parent );

/******************************************************************************/
/*!
	\brief Allocates a new TrotList reference to a new list.
	\param l_A Pointer to a TrotList pointer that must be NULL. On
		success, this will point to a new TrotList reference.
	\return TROT_RC
*/
TROT_RC trotListInit( TrotList **l_A )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	trotListRefListNode *newRefHead = NULL;
	trotListRefListNode *newRefTail = NULL;

	trotListNode *newHead = NULL;
	trotListNode *newTail = NULL;
	trotListActual *newLa = NULL;
	TrotList *newL = NULL;


	/* PRECOND */
	ERR_IF( l_A == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( (*l_A) != NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	/* create list of refs that point to this list */
	TROT_MALLOC( newRefHead, trotListRefListNode, 1 );
	TROT_MALLOC( newRefTail, trotListRefListNode, 1 );

	newRefHead->count = 0;
	newRefHead->l = NULL;
	newRefHead->next = newRefTail;
	newRefHead->prev = newRefHead;

	newRefTail->count = 0;
	newRefTail->l = NULL;
	newRefTail->next = newRefTail;
	newRefTail->prev = newRefHead;

	/* create the data list */
	TROT_MALLOC( newHead, trotListNode, 1 );
	TROT_MALLOC( newTail, trotListNode, 1 );

	newHead->kind = NODE_KIND_HEAD_OR_TAIL;
	newHead->count = 0;
	newHead->n = NULL;
	newHead->l = NULL;
	newHead->prev = newHead;
	newHead->next = newTail;

	newTail->kind = NODE_KIND_HEAD_OR_TAIL;
	newTail->count = 0;
	newTail->n = NULL;
	newTail->l = NULL;
	newTail->prev = newHead;
	newTail->next = newTail;

	/* create actual list structure */
	TROT_MALLOC( newLa, trotListActual, 1 );

	newLa->reachable = 1;
	newLa->flagVisited = 0;
	newLa->previous = NULL;
	newLa->nextToFree = NULL;

	newLa->encodingParent = NULL;
	newLa->encodingChildNumber = 0;

	newLa->tag = TROT_TAG_DATA;
	newLa->userTag = 0;

	newLa->childrenCount = 0;

	newLa->refListHead = newRefHead;
	newLa->refListTail = newRefTail;
	newRefHead = NULL;
	newRefTail = NULL;

	newLa->head = newHead;
	newLa->tail = newTail;
	newHead = NULL;
	newTail = NULL;

	/* create the first ref to this list */
	TROT_MALLOC( newL, TrotList, 1 );

	newL->laParent = NULL;

	newL->laPointsTo = newLa;
	newLa = NULL;

	/* add first ref to list's ref list */
	rc = _refListAdd( newL->laPointsTo, newL );
	ERR_IF_PASSTHROUGH;

	/* give back */
	(*l_A) = newL;
	newL = NULL;

	return TROT_RC_SUCCESS; /* FUTURE OPTIMIZATION: go through other functions, and see if we can return success before cleanup */


	/* CLEANUP */
	cleanup:

	trotHookFree( newRefHead );
	trotHookFree( newRefTail );
	trotHookFree( newHead );
	trotHookFree( newTail );
	if ( newLa != NULL )
	{
		trotHookFree( newLa->refListHead );
		trotHookFree( newLa->refListTail );
		trotHookFree( newLa->head );
		trotHookFree( newLa->tail );
		trotHookFree( newLa );
	}
	if ( newL != NULL )
	{
		trotHookFree( newL->laPointsTo->refListHead );
		trotHookFree( newL->laPointsTo->refListTail );
		trotHookFree( newL->laPointsTo->head );
		trotHookFree( newL->laPointsTo->tail );
		trotHookFree( newL->laPointsTo );
		trotHookFree( newL );
	}

	return rc;
}

/******************************************************************************/
/*!
	\brief Twins a TrotList reference.
	\param l Pointer to the TrotList pointer to be twinned.
	\param lTwin_A Pointer to a TrotList pointer that must be NULL. On
		success, this will point to a new TrotList reverence.
	\return TROT_RC
*/
TROT_RC trotListTwin( TrotList *l, TrotList **lTwin_A )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotList *newL = NULL;


	/* PRECOND */
	ERR_IF( l == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( lTwin_A == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( (*lTwin_A) != NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	TROT_MALLOC( newL, TrotList, 1 );

	newL->laParent = NULL;
	newL->laPointsTo = l->laPointsTo;

	rc = _refListAdd( newL->laPointsTo, newL );
	ERR_IF_PASSTHROUGH;


	/* give back */
	(*lTwin_A) = newL;
	newL = NULL;

	return TROT_RC_SUCCESS;


	/* CLEANUP */
	cleanup:

	trotHookFree( newL );

	return rc;
}

/******************************************************************************/
/*!
	\brief Frees a TrotList reference. Actual list will be trotHookFreed if this
		is the last reference.
	\param l_F Pointer to a TrotList pointer.
		(*l_F) can be NULL, in which case nothing will happen.
		On return, (*l_F) will be NULL.
	\return void
*/
void trotListFree( TrotList **l_F )
{
	/* DATA */
	trotListActual *la = NULL;

	trotListNode *node = NULL;

	int j = 0;
	trotListActual *tempList = NULL;

	trotListActual *nextL = NULL;
	trotListActual *currentL = NULL;


	/* CODE */
	PARANOID_ERR_IF( l_F == NULL ); /* TODO: make this a normal if, in below, and add test for it? */

	if ( (*l_F) == NULL )
	{
		return;
	}

	PARANOID_ERR_IF( (*l_F)->laParent != NULL );

	la = (*l_F)->laPointsTo;

	/* remove ref from list's ref list */
	_refListRemove( la, (*l_F) );

	/* free ref */
	trotHookFree( (*l_F) );
	(*l_F) = NULL;

	/* is list reachable? */
	_isListReachable( la );
	if ( la->reachable )
	{
		return;
	}

	/* we need to free it */

	/* go through stack */
	currentL = la;
	while ( currentL != NULL )
	{
		/* free data */
		node = currentL->head->next;
		while ( node != currentL->tail )
		{
			if ( node->kind == NODE_KIND_INT )
			{
				trotHookFree( node->n );
			}
			else /* NODE_KIND_LIST */
			{
				for ( j = 0; j < node->count; j += 1 )
				{
					tempList = node->l[ j ]->laPointsTo;
			
					_refListRemove( tempList, node->l[ j ] );

					trotHookFree( node->l[ j ] );
					node->l[ j ] = NULL;

					if ( tempList->reachable == 1 )
					{
						_isListReachable( tempList );
						if ( tempList->reachable == 0 )
						{
							/* need to free this list too */
							tempList->nextToFree = currentL->nextToFree;
							currentL->nextToFree = tempList;
						}
					}
				}

				trotHookFree( node->l );
			}

			node = node->next;
			trotHookFree( node->prev );
		}

		currentL = currentL->nextToFree;
	}

	/* *** */
	nextL = la;
	while ( nextL != NULL )
	{
		/* *** */
		currentL = nextL;

		/* *** */
		nextL = nextL->nextToFree;

		/* *** */
		trotHookFree( currentL->head ); /* TODO: rename these trotHookFree */
		trotHookFree( currentL->tail );
		trotHookFree( currentL->refListHead );
		trotHookFree( currentL->refListTail );
		trotHookFree( currentL );
	}

	return;
}

/******************************************************************************/
/*!
	\brief Gets the count of items in the list.
	\param l Pointer to a TrotList pointer.
	\param c On success, will contain the count of this list.
	\return TROT_RC
*/
TROT_RC trotListGetCount( TrotList *l, TROT_INT *c )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	/* PRECOND */
	ERR_IF( l == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( c == NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	(*c) = l->laPointsTo->childrenCount;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Gets the kind of an item in the list.
	\param l Pointer to a TrotList pointer.
	\param index Index of the item.
	\param kind On success, will contain the kind of the item.
	\return TROT_RC
*/
TROT_RC trotListGetKind( TrotList *l, TROT_INT index, TROT_KIND *kind )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	trotListActual *la = NULL;

	trotListNode *node = NULL;

	TROT_INT count = 0;


	/* PRECOND */
	ERR_IF( l == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( kind == NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	la = l->laPointsTo;

	/* Turn negative index into positive equivalent. */
	if ( index < 0 )
	{
		index = (la->childrenCount) + index + 1;
	}

	/* Make sure index is in range */
	ERR_IF_1( index <= 0, TROT_RC_ERROR_BAD_INDEX, index );
	ERR_IF_1( index > (la->childrenCount ), TROT_RC_ERROR_BAD_INDEX, index );

	/* *** */
	node = la->head->next;
	while ( 1 )
	{
		count += node->count;
		if ( count >= index )
		{
			break;
		}

		node = node->next;

		PARANOID_ERR_IF( node == la->tail );
	}

	(*kind) = node->kind;

	return TROT_RC_SUCCESS;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Appends an int to the end of the list.
	\param l Pointer to a TrotList pointer.
	\param n The int value to insert.
	\return TROT_RC
*/
TROT_RC trotListAppendInt( TrotList *l, TROT_INT n )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	trotListActual *la = NULL;
	trotListNode *node = NULL;

	trotListNode *newNode = NULL;


	/* PRECOND */
	ERR_IF( l == NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	la = l->laPointsTo;

	/* *** */
	node = la->tail->prev;

	/* special cases to create new node */
	if (    node == la->head             /* empty list */
	     || node->kind != NODE_KIND_INT /* last node is not int kind */
	     || node->count == NODE_SIZE    /* last node is full */
	   )
	{
		rc = newIntNode( &newNode );
		ERR_IF_PASSTHROUGH;

		newNode->next = la->tail;
		newNode->prev = la->tail->prev;

		la->tail->prev->next = newNode;
		la->tail->prev = newNode;

		node = newNode;
		newNode = NULL;
	}

	/* append */
	node->n[ node->count ] = n;
	node->count += 1;

	la->childrenCount += 1;

	return TROT_RC_SUCCESS;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Appends a list twin to the end of the list.
	\param l Pointer to a TrotList pointer.
	\param lToAppend The list to twin and append.
	\return TROT_RC
*/
TROT_RC trotListAppendList( TrotList *l, TrotList *lToAppend )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	trotListActual *la = NULL;
	trotListNode *node = NULL;

	trotListNode *newNode = NULL;

	TrotList *newL = NULL;


	/* PRECOND */
	ERR_IF( l == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( lToAppend == NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	la = l->laPointsTo;

	/* *** */
	node = la->tail->prev;

	/* special cases to create new node */
	if (    node == la->head              /* empty list */
	     || node->kind != NODE_KIND_LIST /* last node is not list kind */
	     || node->count == NODE_SIZE     /* last node is full */
	   )
	{
		rc = newListNode( &newNode );
		ERR_IF_PASSTHROUGH;

		newNode->next = la->tail;
		newNode->prev = la->tail->prev;

		la->tail->prev->next = newNode;
		la->tail->prev = newNode;

		node = newNode;
		newNode = NULL;
	}

	/* append */
	rc = trotListTwin( lToAppend, &newL );
	ERR_IF_PASSTHROUGH;

	node->l[ node->count ] = newL;
	newL->laParent = la;
	newL = NULL;

	node->count += 1;

	la->childrenCount += 1;

	return TROT_RC_SUCCESS;


	/* CLEANUP */
	cleanup:

	trotListFree( &newL );

	return rc;
}

/******************************************************************************/
/*!
	\brief Inserts an int into the list.
	\param l Pointer to a TrotList pointer.
	\param index Where to insert.
	\param n The int value to insert.
	\return TROT_RC
*/
TROT_RC trotListInsertInt( TrotList *l, TROT_INT index, TROT_INT n )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	trotListActual *la = NULL;

	trotListNode *node = NULL;

	TROT_INT count = 0;

	int i = 0;
	int j = 0;

	trotListNode *newNode = NULL;


	/* PRECOND */
	ERR_IF( l == NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	la = l->laPointsTo;

	/* Turn negative index into positive equivalent. */
	if ( index < 0 )
	{
		index = (la->childrenCount) + index + 2;
	}

	/* This handles two special cases. One, if they want to add to the end of the
	   list. And two, if they want to add to an empty list. */
	if ( index == (la->childrenCount) + 1 )
	{
		rc = trotListAppendInt( l, n );
		ERR_IF_PASSTHROUGH;

		return TROT_RC_SUCCESS;
	}

	/* Make sure index is in range */
	ERR_IF_1( index <= 0, TROT_RC_ERROR_BAD_INDEX, index );
	ERR_IF_1( index > (la->childrenCount), TROT_RC_ERROR_BAD_INDEX, index );

	/* Find node where int needs to be added into */
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

	/* *** */
	if ( node->kind == NODE_KIND_INT )
	{
		/* If node is full */
		if ( node->count == NODE_SIZE )
		{
			rc = trotListNodeSplit( node, NODE_SIZE / 2 );
			ERR_IF_PASSTHROUGH;

			/* Since node has been split, we may need to go to next
			   node. */
			if ( count + (node->count) < index )
			{
				count += node->count;
				node = node->next;
			}
		}

		/* We now have the node where the int needs to be inserted.
		   We've made sure there is space to insert.
		   (count + 1) is the beginning index of the node */
		/* Now let's move any ints over to make room */
		i = index - count - 1;
		j = node->count;
		while ( j != i )
		{
			node->n[ j ] = node->n[ j - 1 ];
			j -= 1;
		}

		/* Insert int into node */
		node->n[ i ] = n;
		node->count += 1;

		la->childrenCount += 1;

		return TROT_RC_SUCCESS;
	}
	else /* node->kind == NODE_KIND_LIST */
	{
		i = index - count - 1;

		/* If we need to insert at spot 0, we see if the previous node
		   is an int node with room. If so, we can just append to that
		   node. */
		if (    i == 0
		     && node->prev->kind == NODE_KIND_INT
		     && node->prev->count != NODE_SIZE 
		   )
		{
			node = node->prev;

			/* Insert int into node */
			node->n[ node->count ] = n;
			node->count += 1;

			la->childrenCount += 1;

			return TROT_RC_SUCCESS;
		}

		/* if not at beginning, we'll have to split the node */
		if ( i != 0 )
		{
			rc = trotListNodeSplit( node, i );
			ERR_IF_PASSTHROUGH;

			node = node->next;
		}

		/* *** */
		rc = newIntNode( &newNode );
		ERR_IF_PASSTHROUGH;

		newNode->n[ 0 ] = n;
		newNode->count = 1;

		/* Insert node into list */
		newNode->next = node;
		newNode->prev = node->prev;

		node->prev->next = newNode;
		node->prev = newNode;

		la->childrenCount += 1;

		return TROT_RC_SUCCESS;
	}


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Inserts a twin of a list into the list.
	\param l Pointer to a TrotList pointer.
	\param index Where to insert.
	\param lToInsert The list to insert.
	\return TROT_RC
*/
TROT_RC trotListInsertList( TrotList *l, TROT_INT index, TrotList *lToInsert )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	trotListActual *la = NULL;

	trotListNode *node = NULL;

	TROT_INT count = 0;

	int i = 0;
	int j = 0;

	trotListNode *newNode = NULL;

	TrotList *newL = NULL;


	/* PRECOND */
	ERR_IF( l == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( lToInsert == NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	la = l->laPointsTo;

	/* Turn negative index into positive equivalent. */
	if ( index < 0 )
	{
		index = (la->childrenCount) + index + 2;
	}

	/* This handles two special cases. One, if they want to add to the end of the
	   list. And two, if they want to add to an empty list. */
	if ( index == (la->childrenCount) + 1 )
	{
		rc = trotListAppendList( l, lToInsert );
		ERR_IF_PASSTHROUGH;

		return TROT_RC_SUCCESS;
	}

	/* Make sure index is in range */
	ERR_IF_1( index <= 0, TROT_RC_ERROR_BAD_INDEX, index );
	ERR_IF_1( index > (la->childrenCount ), TROT_RC_ERROR_BAD_INDEX, index );

	/* Find node where list needs to be added into */
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

	/* *** */
	if ( node->kind == NODE_KIND_LIST )
	{
		/* If node is full */
		if ( node->count == NODE_SIZE )
		{
			rc = trotListNodeSplit( node, NODE_SIZE / 2 );
			ERR_IF_PASSTHROUGH;

			/* Since node has been split, we may need to go to next
			   node. */
			if ( count + (node->count) < index )
			{
				count += node->count;
				node = node->next;
			}
		}

		/* We now have the node where the list needs to be inserted.
		   We've made sure there is space to insert.
		   (count + 1) is the beginning index of the node */

		/* *** */
		rc = trotListTwin( lToInsert, &newL );
		ERR_IF_PASSTHROUGH;

		/* Now let's move any lists over to make room */
		i = index - count - 1;
		j = node->count;
		while ( j != i )
		{
			node->l[ j ] = node->l[ j - 1 ];
			j -= 1;
		}

		/* Insert list into node */
		node->l[ i ] = newL;
		newL->laParent = la;
		newL = NULL;

		node->count += 1;

		la->childrenCount += 1;

		return TROT_RC_SUCCESS;
	}
	else /* node->kind == NODE_KIND_INT */
	{
		i = index - count - 1;

		/* If we need to insert at spot 0, we see if the previous node
		   is an list node with room. If so, we can just append to that
		   node. */
		if (    i == 0
		     && node->prev->kind == NODE_KIND_LIST
		     && node->prev->count != NODE_SIZE 
		   )
		{
			node = node->prev;

			/* Insert list into node */
			rc = trotListTwin( lToInsert, &newL );
			ERR_IF_PASSTHROUGH;

			node->l[ node->count ] = newL;
			newL->laParent = la;
			newL = NULL;

			node->count += 1;

			la->childrenCount += 1;

			return TROT_RC_SUCCESS;
		}

		/* if not at beginning, we'll have to split the node */
		if ( i != 0 )
		{
			rc = trotListNodeSplit( node, i );
			ERR_IF_PASSTHROUGH;

			node = node->next;
		}

		/* *** */
		rc = newListNode( &newNode );
		ERR_IF_PASSTHROUGH;

		/* *** */
		rc = trotListTwin( lToInsert, &newL );
		ERR_IF_PASSTHROUGH;

		/* Insert node into list */
		newNode->l[ 0 ] = newL;
		newL->laParent = la;
		newL = NULL;

		newNode->count = 1;

		newNode->next = node;
		newNode->prev = node->prev;

		node->prev->next = newNode;
		node->prev = newNode;

		la->childrenCount += 1;

		return TROT_RC_SUCCESS;
	}


	/* CLEANUP */
	cleanup:

	trotListFree( &newL );

	if ( newNode != NULL )
	{
		trotHookFree( newNode->l );
		trotHookFree( newNode );
	}

	return rc;
}

/******************************************************************************/
/*!
	\brief Gets copy of int in list.
	\param l Pointer to a TrotList pointer.
	\param index Which int to get.
	\param n On success, will point to int.
	\return TROT_RC
*/
TROT_RC trotListGetInt( TrotList *l, TROT_INT index, TROT_INT *n )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	trotListActual *la = NULL;

	trotListNode *node = NULL;

	TROT_INT count = 0;


	/* PRECOND */
	ERR_IF( l == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( n == NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	la = l->laPointsTo;

	/* Turn negative index into positive equivalent. */
	if ( index < 0 )
	{
		index = (la->childrenCount) + index + 1;
	}

	/* Make sure index is in range */
	ERR_IF_1( index <= 0, TROT_RC_ERROR_BAD_INDEX, index );
	ERR_IF_1( index > (la->childrenCount ), TROT_RC_ERROR_BAD_INDEX, index );

	/* *** */
	node = la->head->next;
	while ( 1 )
	{
		count += node->count;
		if ( count >= index )
		{
			break;
		}

		node = node->next;

		PARANOID_ERR_IF( node == la->tail );
	}

	ERR_IF_1( node->kind != NODE_KIND_INT, TROT_RC_ERROR_WRONG_KIND, node->kind );

	/* give back */
	(*n) = node->n[ (node->count) - 1 - (count - index) ];

	return TROT_RC_SUCCESS;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Gets list ref of list in list.
	\param l Pointer to a TrotList pointer.
	\param index Which list to get.
	\param lTwin_A On success, will point to list ref.
	\return TROT_RC
*/
TROT_RC trotListGetList( TrotList *l, TROT_INT index, TrotList **lTwin_A )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	trotListNode *node = NULL;

	TROT_INT count = 0;

	TrotList *newL = NULL;


	/* PRECOND */
	ERR_IF( l == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( lTwin_A == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( (*lTwin_A) != NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	/* Turn negative index into positive equivalent. */
	if ( index < 0 )
	{
		index = (l->laPointsTo->childrenCount) + index + 1;
	}

	/* Make sure index is in range */
	ERR_IF_1( index <= 0, TROT_RC_ERROR_BAD_INDEX, index );
	ERR_IF_1( index > (l->laPointsTo->childrenCount ), TROT_RC_ERROR_BAD_INDEX, index );

	/* *** */
	node = l->laPointsTo->head->next;
	while ( 1 )
	{
		count += node->count;
		if ( count >= index )
		{
			break;
		}

		node = node->next;

		PARANOID_ERR_IF( node == l->laPointsTo->tail );
	}

	ERR_IF_1( node->kind != NODE_KIND_LIST, TROT_RC_ERROR_WRONG_KIND, node->kind );

	rc = trotListTwin( node->l[ (node->count) - 1 - (count - index) ], &newL );
	ERR_IF_PASSTHROUGH;

	/* give back */
	(*lTwin_A) = newL;
	newL = NULL;

	return TROT_RC_SUCCESS;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Gets and removes int ref of list in list.
	\param l Pointer to a TrotList pointer.
	\param index Which int to get and remove.
	\param n On success, will point to int.
	\return TROT_RC
*/
TROT_RC trotListRemoveInt( TrotList *l, TROT_INT index, TROT_INT *n )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	trotListNode *node = NULL;

	TROT_INT count = 0;

	TROT_INT giveBackN = 0;

	int i = 0;


	/* PRECOND */
	ERR_IF( l == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( n == NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */

	/* Turn negative index into positive equivalent. */
	if ( index < 0 )
	{
		index = (l->laPointsTo->childrenCount) + index + 1;
	}

	/* Make sure index is in range */
	ERR_IF_1( index <= 0, TROT_RC_ERROR_BAD_INDEX, index );
	ERR_IF_1( index > (l->laPointsTo->childrenCount ), TROT_RC_ERROR_BAD_INDEX, index );

	/* *** */
	node = l->laPointsTo->head->next;
	while ( 1 )
	{
		count += node->count;
		if ( count >= index )
		{
			break;
		}

		node = node->next;

		PARANOID_ERR_IF( node == l->laPointsTo->tail );
	}

	ERR_IF_1( node->kind != NODE_KIND_INT, TROT_RC_ERROR_WRONG_KIND, node->kind );

	i = (node->count) - 1 - (count - index);
	giveBackN = node->n[ i ];
	while ( i < ( (node->count) - 1 ) )
	{
		node->n[ i ] = node->n[ i + 1 ];
		i += 1;
	}
	node->count -= 1;
	l->laPointsTo->childrenCount -= 1;

	if ( node->count == 0 )
	{
		node->prev->next = node->next;
		node->next->prev = node->prev;

		trotHookFree( node->n );
		trotHookFree( node );
	}

	/* give back */
	(*n) = giveBackN;

	return TROT_RC_SUCCESS;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Gets and removes list ref of list in list.
	\param l Pointer to a TrotList pointer.
	\param index Which list to get.
	\param lRemoved_A On success, will point to list ref.
	\return TROT_RC
*/
TROT_RC trotListRemoveList( TrotList *l, TROT_INT index, TrotList **lRemoved_A )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	trotListNode *node = NULL;

	TROT_INT count = 0;

	TrotList *giveBackL = NULL;

	int i = 0;


	/* PRECOND */
	ERR_IF( l == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( lRemoved_A == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( (*lRemoved_A) != NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	/* Turn negative index into positive equivalent. */
	if ( index < 0 )
	{
		index = (l->laPointsTo->childrenCount) + index + 1;
	}

	/* Make sure index is in range */
	ERR_IF_1( index <= 0, TROT_RC_ERROR_BAD_INDEX, index );
	ERR_IF_1( index > (l->laPointsTo->childrenCount ), TROT_RC_ERROR_BAD_INDEX, index );

	/* *** */
	node = l->laPointsTo->head->next;
	while ( 1 )
	{
		count += node->count;
		if ( count >= index )
		{
			break;
		}

		node = node->next;

		PARANOID_ERR_IF( node == l->laPointsTo->tail );
	}

	ERR_IF_1( node->kind != NODE_KIND_LIST, TROT_RC_ERROR_WRONG_KIND, node->kind );

	i = (node->count) - 1 - (count - index);
	giveBackL = node->l[ i ];
	giveBackL->laParent = NULL;
	while ( i < ( (node->count) - 1 ) )
	{
		node->l[ i ] = node->l[ i + 1 ];
		i += 1;
	}
	node->l[ i ] = NULL;
	node->count -= 1;
	l->laPointsTo->childrenCount -= 1;

	if ( node->count == 0 )
	{
		node->prev->next = node->next;
		node->next->prev = node->prev;

		trotHookFree( node->l );
		trotHookFree( node );
	}

	/* give back */
	(*lRemoved_A) = giveBackL;

	return TROT_RC_SUCCESS;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Removes whatever is at index in list.
	\param l Pointer to a TrotList pointer.
	\param index What to remove.
	\return TROT_RC
*/
TROT_RC trotListRemove( TrotList *l, TROT_INT index )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	trotListNode *node = NULL;

	TROT_INT count = 0;

	TrotList *tempL = NULL;

	int i = 0;


	/* PRECOND */
	ERR_IF( l == NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	/* Turn negative index into positive equivalent. */
	if ( index < 0 )
	{
		index = (l->laPointsTo->childrenCount) + index + 1;
	}

	/* Make sure index is in range */
	ERR_IF_1( index <= 0, TROT_RC_ERROR_BAD_INDEX, index );
	ERR_IF_1( index > (l->laPointsTo->childrenCount ), TROT_RC_ERROR_BAD_INDEX, index );

	/* *** */
	node = l->laPointsTo->head->next;
	while ( 1 )
	{
		count += node->count;
		if ( count >= index )
		{
			break;
		}

		node = node->next;

		PARANOID_ERR_IF( node == l->laPointsTo->tail );
	}

	i = (node->count) - 1 - (count - index);
	if ( node->kind == NODE_KIND_INT )
	{
		while ( i < ( (node->count) - 1 ) )
		{
			node->n[ i ] = node->n[ i + 1 ];
			i += 1;
		}
	}
	else
	{
		tempL = node->l[ i ];
		tempL->laParent = NULL;
		trotListFree( &tempL );
		while ( i < ( (node->count) - 1 ) )
		{
			node->l[ i ] = node->l[ i + 1 ];
			i += 1;
		}
		node->l[ i ] = NULL;
	}
	
	node->count -= 1;
	l->laPointsTo->childrenCount -= 1;

	if ( node->count == 0 )
	{
		node->prev->next = node->next;
		node->next->prev = node->prev;

		if ( node->kind == NODE_KIND_INT )
		{
			trotHookFree( node->n );
		}
		else
		{
			trotHookFree( node->l );
		}
		trotHookFree( node );
	}

	return TROT_RC_SUCCESS;


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
TROT_RC trotListReplaceWithInt( TrotList *l, TROT_INT index, TROT_INT n )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	trotListActual *la = NULL;

	trotListNode *node = NULL;

	TROT_INT count = 0;

	TrotList *tempL = NULL;

	int i = 0;
	int j = 0;

	trotListNode *newNode = NULL;


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

	/* Find node where int needs to be replaced into */
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

	/* *** */
	if ( node->kind == NODE_KIND_INT )
	{
		i = index - count - 1;

		/* replace int into node */
		node->n[ i ] = n;
	}
	else /* node->kind == NODE_KIND_LIST */
	{
		i = index - count - 1;

		/* If at beginning of node */
		if ( i == 0 )
		{
			/* If the previous node is an int node with space, we
			   can just append in that node. */
			if (    node->prev->kind == NODE_KIND_INT
			     && node->prev->count != NODE_SIZE 
			   )
			{
				/* append int into prev node */
				node->prev->n[ node->prev->count ] = n;

				node->prev->count += 1;
			}
			else
			{
				/* *** */
				rc = newIntNode( &newNode );
				ERR_IF_PASSTHROUGH;

				newNode->n[ 0 ] = n;
				newNode->count = 1;

				/* Insert node into list */
				newNode->next = node;
				newNode->prev = node->prev;

				node->prev->next = newNode;
				node->prev = newNode;
			}
		}
		/* else if end of node */
		else if ( i == (node->count) - 1 )
		{
			/* if the next node is an int node with room, we can just prepend to
			   that node. */
			if (    node->next->kind == NODE_KIND_INT
			     && node->next->count != NODE_SIZE 
			   )
			{
				/* prepend int */
				j = node->next->count;
				while ( j != 0 )
				{
					node->next->n[ j ] = node->next->n[ j - 1 ];
					j -= 1;
				}

				node->next->n[ 0 ] = n;

				node->next->count += 1;
			}
			else
			{
				/* *** */
				rc = newIntNode( &newNode );
				ERR_IF_PASSTHROUGH;

				newNode->n[ 0 ] = n;
				newNode->count = 1;

				/* Insert node into list */
				newNode->prev = node;
				newNode->next = node->next;

				node->next->prev = newNode;
				node->next = newNode;
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

			newNode->n[ 0 ] = n;
			newNode->count = 1;

			/* Insert node into list */
			newNode->prev = node;
			newNode->next = node->next;

			node->next->prev = newNode;
			node->next = newNode;
		}

		/* we've put in our int, now we need to remove a list */
		tempL = node->l[ i ];
		tempL->laParent = NULL;
		trotListFree( &tempL );
		while ( i < ( (node->count) - 1 ) )
		{
			node->l[ i ] = node->l[ i + 1 ];
			i += 1;
		}
		node->l[ i ] = NULL;

		node->count -= 1;
		if ( node->count == 0 )
		{
			node->prev->next = node->next;
			node->next->prev = node->prev;

			trotHookFree( node->l );
			trotHookFree( node );
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
TROT_RC trotListReplaceWithList( TrotList *l, TROT_INT index, TrotList *lToInsert )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	trotListActual *la = NULL;

	trotListNode *node = NULL;

	TROT_INT count = 0;

	TrotList *tempL = NULL;

	TrotList *newL = NULL;

	int i = 0;
	int j = 0;

	trotListNode *newNode = NULL;


	/* PRECOND */
	ERR_IF( l == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( lToInsert == NULL, TROT_RC_ERROR_PRECOND );


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

	/* Find node where list needs to be replaced into */
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

	/* create our new twin */
	rc = trotListTwin( lToInsert, &newL );
	ERR_IF_PASSTHROUGH;

	/* *** */
	if ( node->kind == NODE_KIND_LIST )
	{
		i = index - count - 1;

		/* free old */
		tempL = node->l[ i ];
		tempL->laParent = NULL;
		trotListFree( &tempL );

		/* replace with new */
		node->l[ i ] = newL;
		newL->laParent = la;
		newL = NULL;
	}
	else /* node->kind == NODE_KIND_INT */
	{
		i = index - count - 1;

		/* If at beginning of node */
		if ( i == 0 )
		{
			/* If the previous node is a list node with space, we
			   can just append in that node. */
			if (    node->prev->kind == NODE_KIND_LIST
			     && node->prev->count != NODE_SIZE 
			   )
			{
				/* append into prev node */
				node->prev->l[ node->prev->count ] = newL;
				newL->laParent = la;
				newL = NULL;

				node->prev->count += 1;
			}
			else
			{
				/* *** */
				rc = newListNode( &newNode );
				ERR_IF_PASSTHROUGH;

				newNode->l[ 0 ] = newL;
				newL->laParent = la;
				newL = NULL;

				newNode->count = 1;

				/* Insert node into list */
				newNode->next = node;
				newNode->prev = node->prev;

				node->prev->next = newNode;
				node->prev = newNode;
			}
		}
		/* else if end of node */
		else if ( i == (node->count) - 1 )
		{
			/* if the next node is a list node with room, we can just prepend to
			   that node. */
			if (    node->next->kind == NODE_KIND_LIST
			     && node->next->count != NODE_SIZE 
			   )
			{
				/* prepend int */
				j = node->next->count;
				while ( j != 0 )
				{
					node->next->l[ j ] = node->next->l[ j - 1 ];
					j -= 1;
				}

				node->next->l[ 0 ] = newL;
				newL->laParent = la;
				newL = NULL;

				node->next->count += 1;
			}
			else
			{
				/* *** */
				rc = newListNode( &newNode );
				ERR_IF_PASSTHROUGH;

				newNode->l[ 0 ] = newL;
				newL->laParent = la;
				newL = NULL;

				newNode->count = 1;

				/* Insert node into list */
				newNode->prev = node;
				newNode->next = node->next;

				node->next->prev = newNode;
				node->next = newNode;
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

			newNode->l[ 0 ] = newL;
			newL->laParent = la;
			newL = NULL;

			newNode->count = 1;

			/* Insert node into list */
			newNode->prev = node;
			newNode->next = node->next;

			node->next->prev = newNode;
			node->next = newNode;
		}

		/* we've put in our list, now we need to remove an int */
		while ( i < ( (node->count) - 1 ) )
		{
			node->n[ i ] = node->n[ i + 1 ];
			i += 1;
		}

		node->count -= 1;
		if ( node->count == 0 )
		{
			node->prev->next = node->next;
			node->next->prev = node->prev;

			trotHookFree( node->n );
			trotHookFree( node );
		}
	}


	/* CLEANUP */
	cleanup:

	trotListFree( &newL );

	return rc;
}

/******************************************************************************/
/*!
	\brief 
	\param 
	\return TROT_RC
*/
TROT_RC trotListGetTag( TrotList *l, TROT_INT *tag )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;


	/* PRECOND */
	ERR_IF( l == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( tag == NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	(*tag) = l->laPointsTo->tag;


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
TROT_RC trotListSetTag( TrotList *l, TROT_INT tag )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;


	/* PRECOND */
	ERR_IF( l == NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	ERR_IF_1( tag < TROT_TAG_MIN || tag > TROT_TAG_MAX, TROT_RC_ERROR_BAD_TAG, tag );

	l->laPointsTo->tag = tag;


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
TROT_RC trotListGetUserTag( TrotList *l, TROT_INT *tag )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;


	/* PRECOND */
	ERR_IF( l == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( tag == NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	(*tag) = l->laPointsTo->userTag;


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
TROT_RC trotListSetUserTag( TrotList *l, TROT_INT tag )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;


	/* PRECOND */
	ERR_IF( l == NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	l->laPointsTo->userTag = tag;


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
	TROT_RC rc = TROT_RC_SUCCESS;

	trotListNode *newNode = NULL;

	int i = 0;


	/* CODE */
	PARANOID_ERR_IF( n == NULL );

	TROT_MALLOC( newNode, trotListNode, 1 );

	if ( n->kind == NODE_KIND_INT )
	{
		newNode->kind = NODE_KIND_INT;
		newNode->count = (n->count) - keepInLeft;

		newNode->l = NULL;
		TROT_MALLOC( newNode->n, TROT_INT, NODE_SIZE );

		i = keepInLeft;
		while ( i < (n->count) )
		{
			newNode->n[ i - keepInLeft ] = n->n[ i ];

			i += 1;
		}

		n->count = keepInLeft;
	}
	else /* n->kind == NODE_KIND_LIST */
	{
		newNode->kind = NODE_KIND_LIST;
		newNode->count = (n->count) - keepInLeft;

		newNode->n = NULL;
		TROT_CALLOC( newNode->l, TrotList *, NODE_SIZE );

		i = keepInLeft;
		while ( i < (n->count) )
		{
			newNode->l[ i - keepInLeft ] = n->l[ i ];
			n->l[ i ] = NULL;

			i += 1;
		}

		n->count = keepInLeft;
	}

	newNode->prev = n;
	newNode->next = n->next;

	n->next->prev = newNode;
	n->next = newNode;

	return TROT_RC_SUCCESS;


	/* CLEANUP */
	cleanup:

	trotHookFree( newNode );

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
	TROT_RC rc = TROT_RC_SUCCESS;

	trotListNode *newNode = NULL;


	/* CODE */
	PARANOID_ERR_IF( n_A == NULL );
	PARANOID_ERR_IF( (*n_A) != NULL );

	TROT_MALLOC( newNode, trotListNode, 1 );

	newNode->kind = NODE_KIND_INT;
	newNode->count = 0;

	newNode->l = NULL;
	TROT_MALLOC( newNode->n, TROT_INT, NODE_SIZE );

	/* give back */
	(*n_A) = newNode;
	newNode = NULL;

	return TROT_RC_SUCCESS;


	/* CLEANUP */
	cleanup:

	trotHookFree( newNode );

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
	TROT_RC rc = TROT_RC_SUCCESS;

	trotListNode *newNode = NULL;


	/* CODE */
	PARANOID_ERR_IF( n_A == NULL );
	PARANOID_ERR_IF( (*n_A) != NULL );

	TROT_MALLOC( newNode, trotListNode, 1 );

	newNode->kind = NODE_KIND_LIST;
	newNode->count = 0;

	newNode->n = NULL;
	TROT_CALLOC( newNode->l, TrotList *, NODE_SIZE );

	/* give back */
	(*n_A) = newNode;
	newNode = NULL;

	return TROT_RC_SUCCESS;


	/* CLEANUP */
	cleanup:

	trotHookFree( newNode );

	return rc;
}

/******************************************************************************/
static TROT_RC _refListAdd( trotListActual *la, TrotList *l )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	trotListRefListNode *refNode = NULL;

	trotListRefListNode *newRefNode = NULL;


	/* CODE */
	PARANOID_ERR_IF( la == NULL );
	PARANOID_ERR_IF( l == NULL );

	refNode = la->refListHead->next;
	while ( refNode != la->refListTail )
	{
		if ( refNode->count < REF_LIST_NODE_SIZE )
		{
			refNode->l[ refNode->count ] = l;
			refNode->count += 1;

			return TROT_RC_SUCCESS;
		}

		refNode = refNode->next;
	}

	/* there was no room in list, so create new node, insert ref into new
	   node, and insert node into list */
	TROT_MALLOC( newRefNode, trotListRefListNode, 1 );

	TROT_CALLOC( newRefNode->l, TrotList *, REF_LIST_NODE_SIZE );

	newRefNode->count = 1;
	newRefNode->l[ 0 ] = l;

	newRefNode->prev = la->refListTail->prev;
	newRefNode->next = la->refListTail;
	la->refListTail->prev->next = newRefNode;
	la->refListTail->prev = newRefNode;

	return TROT_RC_SUCCESS;


	/* CLEANUP */
	cleanup:

	trotHookFree( newRefNode );

	return rc;
}

/******************************************************************************/
static void _refListRemove( trotListActual *la, TrotList *l )
{
	/* DATA */
	trotListRefListNode *refNode = NULL;

	int i = 0;


	/* CODE */
	PARANOID_ERR_IF( la == NULL );
	PARANOID_ERR_IF( l == NULL );

	/* foreach refNode */
	refNode = la->refListHead->next;
	while ( 1 )
	{
		/* foreach pointer in this node */
		i = 0;
		while ( i < refNode->count )
		{
			/* is this the ref we're looking for? */
			if ( refNode->l[ i ] == l )
			{
				/* found, now remove it */
				while ( i < ( ( refNode->count ) - 1 ) )
				{
					refNode->l[ i ] = refNode->l[ i + 1 ];

					i += 1;
				}

				refNode->l[ i ] = NULL;
				
				refNode->count -= 1;

				/* remove node if node is empty */
				if ( refNode->count == 0 )
				{
					refNode->prev->next = refNode->next;
					refNode->next->prev = refNode->prev;

					trotHookFree( refNode->l );
					trotHookFree( refNode );
				}

				return;
			}

			i += 1;
		}

		refNode = refNode->next;

		PARANOID_ERR_IF( refNode == la->refListTail );
	}

	PARANOID_ERR_IF( 1 );

	return;
}

/******************************************************************************/
static void _isListReachable( trotListActual *la )
{
	/* DATA */
	int flagFoundClientRef = 0;

	trotListActual *currentLa = NULL;

	trotListActual *parent = NULL;

	trotListActual *tempLa = NULL;


	/* CODE */
	PARANOID_ERR_IF( la == NULL );
	PARANOID_ERR_IF( la->reachable == 0 );

	/* go "up" trying to find a client ref */
	currentLa = la;
	currentLa->flagVisited = 1;

	while ( 1 )
	{
		if ( _findNextParent( currentLa, 0, &parent ) != 0 )
		{
			if ( currentLa->previous != NULL )
			{
				tempLa = currentLa;
				currentLa = currentLa->previous;
				tempLa->previous = NULL;

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

		parent->previous = currentLa;
		currentLa = parent;
		currentLa->flagVisited = 1;
	}

	if ( ! flagFoundClientRef )
	{
		la->reachable = 0;
	}

	/* restart, go "up", resetting all the flagVisited flags to 0 */
	currentLa = la;
	currentLa->flagVisited = 0;

	while ( 1 )
	{
		if ( _findNextParent( currentLa, 1, &parent ) != 0 )
		{
			if ( currentLa->previous != NULL )
			{
				tempLa = currentLa;
				currentLa = currentLa->previous;
				tempLa->previous = NULL;

				continue;
			}

			break;
		}

		parent->previous = currentLa;
		currentLa = parent;
		currentLa->flagVisited = 0;
	}

	return;
}

/******************************************************************************/
static int _findNextParent( trotListActual *la, int queryVisited, trotListActual **parent )
{
	/* DATA */
	trotListRefListNode *refNode = NULL;

	int i = 0;

	trotListActual *tempParent = NULL;


	/* CODE */
	PARANOID_ERR_IF( la == NULL );
	PARANOID_ERR_IF( parent == NULL );

	/* for each reference that points to this list */
	refNode = la->refListHead;
	while ( refNode != la->refListTail )
	{
		i = 0;
		while ( i < refNode->count )
		{
			/* get list this ref is in */
			tempParent = refNode->l[ i ]->laParent;

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
			else if ( tempParent->flagVisited == queryVisited )
			{
				(*parent) = tempParent;
				return 0;
			}

			i += 1;
		}

		refNode = refNode->next;
	}

	return -1;
}

/******************************************************************************/
/*!
	\brief Provides a const char string representation for a TROT_RC
	\param[in] rc A TROT_RC value.
	\return A const char representation of the passed in TROT_RC.
*/
const char *trotRCToString( TROT_RC rc )
{
	static const char *_rcStrings[] =
	{
		"Success",

		"Precondition Error",
		"Memory Allocation Error",
		"Standard Library Error",

		"Bad Index Error",
		"Wrong Kind Error",
		"Invalid Op Error",
		"Bad Tag Error",
		"Divide By Zero Error",
		"Unicode Error",
		"Decode Error",
		"Load Error",
		"Not Byte Value Error",
	};

	static const char *_rcUnknown = "Unknown Error";

	if ( rc >= 0 && rc <= TROT_RC_STANDARD_ERRORS_MAX )
	{
		return _rcStrings[ rc ];
	}
	else if ( rc >= TROT_RC_TROT_ERRORS_MIN && rc <= TROT_RC_TROT_ERRORS_MAX )
	{
		return _rcStrings[ TROT_RC_STANDARD_ERRORS_MAX + rc - TROT_RC_TROT_ERRORS_MIN + 1 ];
	}

	return _rcUnknown;
}

