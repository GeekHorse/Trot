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
	Implements the primary funcitonality of our lists.

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

/******************************************************************************/
#include "trot.h"
#include "trotInternal.h"

/******************************************************************************/
static TROT_RC refListAdd( TrotListActual *la, TrotList *l );
static void refListRemove( TrotListActual *la, TrotList *l );

static void isListReachable( TrotListActual *la );
static TROT_INT findNextParent( TrotListActual *la, TROT_INT queryVisited, TrotListActual **parent );

/******************************************************************************/
/*!
	\brief Allocates a new list and new reference to the list.
	\param[out] l_A On success, will be new reference to a new list.
	\return TROT_RC
*/
TROT_RC trotListInit( TrotList **l_A )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotListRefListNode *newRefHead = NULL;
	TrotListRefListNode *newRefTail = NULL;

	TrotListNode *newHead = NULL;
	TrotListNode *newTail = NULL;
	TrotListActual *newLa = NULL;

	TrotList *newL = NULL;


	/* PRECOND */
	ERR_IF( l_A == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( (*l_A) != NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	/* create list of refs that point to this list */
	TROT_MALLOC( newRefHead, TrotListRefListNode, 1 );
	TROT_MALLOC( newRefTail, TrotListRefListNode, 1 );

	newRefHead->count = 0;
	newRefHead->l = NULL;
	newRefHead->next = newRefTail;
	newRefHead->prev = newRefHead;

	newRefTail->count = 0;
	newRefTail->l = NULL;
	newRefTail->next = newRefTail;
	newRefTail->prev = newRefHead;

	/* create the data list */
	TROT_MALLOC( newHead, TrotListNode, 1 );
	TROT_MALLOC( newTail, TrotListNode, 1 );

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
	TROT_MALLOC( newLa, TrotListActual, 1 );

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
	rc = refListAdd( newL->laPointsTo, newL );
	ERR_IF_PASSTHROUGH;

	/* give back */
	(*l_A) = newL;
	newL = NULL;

	/* special case where we return success here instead of going through cleanup. */
	return TROT_RC_SUCCESS;


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
	\brief Creates a new reference to a list.
	\param[in] l List to create a new reference to.
	\param[out] l_A On success, new reference to the list.
	\return TROT_RC
*/
TROT_RC trotListTwin( TrotList *l, TrotList **l_A )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotList *newL = NULL;


	/* PRECOND */
	ERR_IF( l == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( l_A == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( (*l_A) != NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	TROT_MALLOC( newL, TrotList, 1 );

	newL->laParent = NULL;
	newL->laPointsTo = l->laPointsTo;

	rc = refListAdd( newL->laPointsTo, newL );
	ERR_IF_PASSTHROUGH;


	/* give back */
	(*l_A) = newL;
	newL = NULL;

	return TROT_RC_SUCCESS;


	/* CLEANUP */
	cleanup:

	trotHookFree( newL );

	return rc;
}

/******************************************************************************/
/*!
	\brief Frees a list reference. Actual list will be freed if the list is no
		longer reachable.
	\param[in] l_F Address of list reference to free.
	\return void

	l_F can be NULL, or the address of a NULL pointer, and this will be a noop.
*/
void trotListFree( TrotList **l_F )
{
	/* DATA */
	TrotListActual *la = NULL;

	TrotListNode *node = NULL;

	TROT_INT j = 0;
	TrotListActual *laTemp = NULL;

	TrotListActual *laNext = NULL;
	TrotListActual *laCurrent = NULL;


	/* CODE */
	if ( l_F == NULL || (*l_F) == NULL )
	{
		return;
	}

	PARANOID_ERR_IF( (*l_F)->laParent != NULL );

	la = (*l_F)->laPointsTo;

	/* remove ref from list's ref list */
	refListRemove( la, (*l_F) );

	/* free ref */
	trotHookFree( (*l_F) );
	(*l_F) = NULL;

	/* is list reachable? */
	isListReachable( la );
	if ( la->reachable )
	{
		return;
	}

	/* we need to free it */

	/* go through stack */
	laCurrent = la;
	while ( laCurrent != NULL )
	{
		/* free data */
		node = laCurrent->head->next;
		while ( node != laCurrent->tail )
		{
			if ( node->kind == NODE_KIND_INT )
			{
				trotHookFree( node->n );
			}
			else /* NODE_KIND_LIST */
			{
				for ( j = 0; j < node->count; j += 1 )
				{
					laTemp = node->l[ j ]->laPointsTo;
			
					refListRemove( laTemp, node->l[ j ] );

					trotHookFree( node->l[ j ] );
					node->l[ j ] = NULL;

					if ( laTemp->reachable == 1 )
					{
						isListReachable( laTemp );
						if ( laTemp->reachable == 0 )
						{
							/* need to free this list too */
							laTemp->nextToFree = laCurrent->nextToFree;
							laCurrent->nextToFree = laTemp;
						}
					}
				}

				trotHookFree( node->l );
			}

			node = node->next;
			trotHookFree( node->prev );
		}

		laCurrent = laCurrent->nextToFree;
	}

	/* *** */
	laNext = la;
	while ( laNext != NULL )
	{
		/* *** */
		laCurrent = laNext;

		/* *** */
		laNext = laNext->nextToFree;

		/* *** */
		trotHookFree( laCurrent->head );
		trotHookFree( laCurrent->tail );
		trotHookFree( laCurrent->refListHead );
		trotHookFree( laCurrent->refListTail );
		trotHookFree( laCurrent );
	}

	return;
}

/******************************************************************************/
/*!
	\brief Compares list references to see if they point to the same list.
	\param[in] l1 First reference.
	\param[in] l2 Second reference.
	\param[out] isSame 0 if refs point to different lists, 1 if they point to
		the same lists.
	\return TROT_RC
*/
TROT_RC trotListRefCompare( TrotList *l1, TrotList *l2, TROT_INT *isSame )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;


	/* PRECOND */
	ERR_IF( l1 == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( l2 == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( isSame == NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	(*isSame) = 0;

	if ( l1->laPointsTo == l2->laPointsTo )
	{
		(*isSame) = 1;
	}


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Gets the count of items in the list.
	\param[in] l Pointer to a TrotList pointer.
	\param[out] count On success, will contain the count of this list.
	\return TROT_RC
*/
TROT_RC trotListGetCount( TrotList *l, TROT_INT *count )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	/* PRECOND */
	ERR_IF( l == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( count == NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	(*count) = l->laPointsTo->childrenCount;

	PARANOID_ERR_IF( (*count) > TROT_MAX_CHILDREN );


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Gets the kind of an item in the list.
	\param[in] l The list.
	\param[in] index Index of the item in the list to ge the kind of.
	\param[out] kind On success, will contain the kind of the item.
	\return TROT_RC
*/
TROT_RC trotListGetKind( TrotList *l, TROT_INT index, TROT_INT *kind )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotListActual *la = NULL;

	TrotListNode *node = NULL;

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
	\param[in] l The list.
	\param[in] n The int to append.
	\return TROT_RC
*/
TROT_RC trotListAppendInt( TrotList *l, TROT_INT n )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotListActual *la = NULL;
	TrotListNode *node = NULL;

	TrotListNode *newNode = NULL;


	/* PRECOND */
	ERR_IF( l == NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	la = l->laPointsTo;

	/* lists cannot hold more than TROT_MAX_CHILDREN, so make sure we have room */
	ERR_IF( TROT_MAX_CHILDREN - la->childrenCount < 1, TROT_RC_ERROR_LIST_OVERFLOW );

	/* *** */
	node = la->tail->prev;

	/* special cases to create new node */
	if (    node == la->head             /* empty list */
	     || node->kind != NODE_KIND_INT /* last node is not int kind */
	     || node->count == TROT_NODE_SIZE    /* last node is full */
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
	\brief Appends a list to the end of the list.
	\param[in] l The list to append to.
	\param[in] lToAppend The list to append.
	\return TROT_RC
*/
TROT_RC trotListAppendList( TrotList *l, TrotList *lToAppend )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotListActual *la = NULL;
	TrotListNode *node = NULL;

	TrotListNode *newNode = NULL;

	TrotList *newL = NULL;


	/* PRECOND */
	ERR_IF( l == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( lToAppend == NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	la = l->laPointsTo;

	/* lists cannot hold more than TROT_MAX_CHILDREN, so make sure we have room */
	ERR_IF( TROT_MAX_CHILDREN - la->childrenCount < 1, TROT_RC_ERROR_LIST_OVERFLOW );

	/* *** */
	node = la->tail->prev;

	/* special cases to create new node */
	if (    node == la->head              /* empty list */
	     || node->kind != NODE_KIND_LIST /* last node is not list kind */
	     || node->count == TROT_NODE_SIZE     /* last node is full */
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
	\param[in] l The list to insert into.
	\param[in] index Where to insert.
	\param[in] n The int to insert.
	\return TROT_RC
*/
TROT_RC trotListInsertInt( TrotList *l, TROT_INT index, TROT_INT n )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotListActual *la = NULL;

	TrotListNode *node = NULL;

	TROT_INT count = 0;

	TROT_INT i = 0;
	TROT_INT j = 0;

	TrotListNode *newNode = NULL;


	/* PRECOND */
	ERR_IF( l == NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	la = l->laPointsTo;

	/* lists cannot hold more than TROT_MAX_CHILDREN, so make sure we have room */
	ERR_IF( TROT_MAX_CHILDREN - la->childrenCount < 1, TROT_RC_ERROR_LIST_OVERFLOW );

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
		if ( node->count == TROT_NODE_SIZE )
		{
			rc = trotListNodeSplit( node, TROT_NODE_SIZE / 2 );
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
		     && node->prev->count != TROT_NODE_SIZE 
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
		rc = newIntNode( &newNode ); /* FUTURE: newNode functions may be able to also place themselves in the list too, to consolidate some code */
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
	\brief Inserts a list into the list.
	\param[in] l The list to insert into.
	\param[in] index Where to insert.
	\param[in] lToInsert The list to insert.
	\return TROT_RC
*/
TROT_RC trotListInsertList( TrotList *l, TROT_INT index, TrotList *lToInsert )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotListActual *la = NULL;

	TrotListNode *node = NULL;

	TROT_INT count = 0;

	TROT_INT i = 0;
	TROT_INT j = 0;

	TrotListNode *newNode = NULL;

	TrotList *newL = NULL;


	/* PRECOND */
	ERR_IF( l == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( lToInsert == NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	la = l->laPointsTo;

	/* lists cannot hold more than TROT_MAX_CHILDREN, so make sure we have room */
	ERR_IF( TROT_MAX_CHILDREN - la->childrenCount < 1, TROT_RC_ERROR_LIST_OVERFLOW );

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
		if ( node->count == TROT_NODE_SIZE )
		{
			rc = trotListNodeSplit( node, TROT_NODE_SIZE / 2 );
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
		     && node->prev->count != TROT_NODE_SIZE 
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

		/* Insert node into list */ /* FUTURE: inserting a node into a list should be a function */
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
	\brief Get int at index in list.
	\param[in] l The list.
	\param[in] index Which int to get.
	\param[out] n On success, the int that was at index in l.
	\return TROT_RC
*/
TROT_RC trotListGetInt( TrotList *l, TROT_INT index, TROT_INT *n )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotListActual *la = NULL;

	TrotListNode *node = NULL;

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
	\brief Gets list at index in list.
	\param[in] l The list.
	\param[in] index Which list to get.
	\param[out] lTwin_A On success, the list that was at index in l.
	\return TROT_RC
*/
TROT_RC trotListGetList( TrotList *l, TROT_INT index, TrotList **lTwin_A )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotListNode *node = NULL;

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
	\brief Gets and removes int in list.
	\param[in] l The list.
	\param[in] index Which int to get and remove.
	\param[out] n On success, the int that was at index in l.
	\return TROT_RC
*/
TROT_RC trotListRemoveInt( TrotList *l, TROT_INT index, TROT_INT *n )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotListNode *node = NULL;

	TROT_INT count = 0;

	TROT_INT giveBackN = 0;

	TROT_INT i = 0;


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

	if ( node->count == 0 ) /* FUTURE: this code may be able to be factored out into a function */
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
	\brief Gets and removes list in list.
	\param[in] l The list.
	\param[in] index Which list to get.
	\param[out] lRemoved_A On success, list that was at index in l.
	\return TROT_RC
*/
TROT_RC trotListRemoveList( TrotList *l, TROT_INT index, TrotList **lRemoved_A )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotListNode *node = NULL;

	TROT_INT count = 0;

	TrotList *giveBackL = NULL;

	TROT_INT i = 0;


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
	\param[in] l The list.
	\param[in] index What to remove.
	\return TROT_RC
*/
TROT_RC trotListRemove( TrotList *l, TROT_INT index )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotListNode *node = NULL;

	TROT_INT count = 0;

	TrotList *tempL = NULL;

	TROT_INT i = 0;


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
	\brief Replaces whatever is at index in l with an int.
	\param[in] l The list.
	\param[in] index Which item to replace.
	\param[in] n The int to put at index in l.
	\return TROT_RC
*/
TROT_RC trotListReplaceWithInt( TrotList *l, TROT_INT index, TROT_INT n )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotListActual *la = NULL;

	TrotListNode *node = NULL;

	TROT_INT count = 0;

	TrotList *tempL = NULL;

	TROT_INT i = 0;
	TROT_INT j = 0;

	TrotListNode *newNode = NULL;


	/* PRECOND */
	ERR_IF( l == NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	la = l->laPointsTo;

/* FUTURE: turn negative into positive, make sure in range, and find node could
	all be factored out into a function.
	as well as other code, to make this file smaller */
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
			     && node->prev->count != TROT_NODE_SIZE 
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
			     && node->next->count != TROT_NODE_SIZE 
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
	\brief Replaces whatever is at index in l with a list.
	\param[in] l The list.
	\param[in] index Which item to replace.
	\param[in[ lToInsert The list to put at index in l.
	\return TROT_RC
*/
TROT_RC trotListReplaceWithList( TrotList *l, TROT_INT index, TrotList *lToInsert )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotListActual *la = NULL;

	TrotListNode *node = NULL;

	TROT_INT count = 0;

	TrotList *tempL = NULL;

	TrotList *newL = NULL;

	TROT_INT i = 0;
	TROT_INT j = 0;

	TrotListNode *newNode = NULL;


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
			     && node->prev->count != TROT_NODE_SIZE 
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
			     && node->next->count != TROT_NODE_SIZE 
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
	\brief Gets tag of list.
	\param[in] l The list.
	\param[out] tag The tag of the list.
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
	\brief Sets tag of list.
	\param[in] l The list.
	\param[in] tag Tag value to set.
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
	\brief Gets user tag of list.
	\param[in] l The list.
	\param[out] tag The user tag of the list.
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
	\brief Sets the uer tag of a list.
	\param[in] l The list.
	\param[in] tag The tag value to set.
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
	\param[in] n Node to split.
	\param[in] keepInLeft How many items to keep in n.
	\return TROT_RC
*/
TROT_RC trotListNodeSplit( TrotListNode *n, TROT_INT keepInLeft )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotListNode *newNode = NULL;

	TROT_INT i = 0;


	/* PRECOND */
	PARANOID_ERR_IF( n == NULL );


	/* CODE */
	TROT_MALLOC( newNode, TrotListNode, 1 );

	if ( n->kind == NODE_KIND_INT )
	{
		newNode->kind = NODE_KIND_INT;
		newNode->count = (n->count) - keepInLeft;

		newNode->l = NULL;
		TROT_MALLOC( newNode->n, TROT_INT, TROT_NODE_SIZE );

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
		TROT_CALLOC( newNode->l, TrotList *, TROT_NODE_SIZE );

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
	\brief Creates a new TrotListNode for Int.
	\param[out] n_A On success, the new node.
	\return TROT_RC
*/
TROT_RC newIntNode( TrotListNode **n_A )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotListNode *newNode = NULL;


	/* PRECOND */
	PARANOID_ERR_IF( n_A == NULL );
	PARANOID_ERR_IF( (*n_A) != NULL );


	/* CODE */
	TROT_MALLOC( newNode, TrotListNode, 1 );

	newNode->kind = NODE_KIND_INT;
	newNode->count = 0;

	newNode->l = NULL;
	TROT_MALLOC( newNode->n, TROT_INT, TROT_NODE_SIZE );

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
	\brief Creates a new TrotListNode for List.
	\param[out] n_A On success, the new node.
	\return TROT_RC
*/
TROT_RC newListNode( TrotListNode **n_A )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotListNode *newNode = NULL;


	/* PRECOND */
	PARANOID_ERR_IF( n_A == NULL );
	PARANOID_ERR_IF( (*n_A) != NULL );


	/* CODE */
	TROT_MALLOC( newNode, TrotListNode, 1 );

	newNode->kind = NODE_KIND_LIST;
	newNode->count = 0;

	newNode->n = NULL;
	TROT_CALLOC( newNode->l, TrotList *, TROT_NODE_SIZE );

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
static TROT_RC refListAdd( TrotListActual *la, TrotList *l )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotListRefListNode *refNode = NULL;

	TrotListRefListNode *newRefNode = NULL;


	/* PRECOND */
	PARANOID_ERR_IF( la == NULL );
	PARANOID_ERR_IF( l == NULL );


	/* CODE */
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
	TROT_MALLOC( newRefNode, TrotListRefListNode, 1 );

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
static void refListRemove( TrotListActual *la, TrotList *l )
{
	/* DATA */
	TrotListRefListNode *refNode = NULL;

	TROT_INT i = 0;


	/* PRECOND */
	PARANOID_ERR_IF( la == NULL );
	PARANOID_ERR_IF( l == NULL );


	/* CODE */
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
static void isListReachable( TrotListActual *la )
{
	/* DATA */
	int flagFoundClientRef = 0;

	TrotListActual *currentLa = NULL;

	TrotListActual *parent = NULL;

	TrotListActual *tempLa = NULL;


	/* PRECOND */
	PARANOID_ERR_IF( la == NULL );
	PARANOID_ERR_IF( la->reachable == 0 );


	/* CODE */
	/* go "up" trying to find a client ref */
	currentLa = la;
	currentLa->flagVisited = 1;

	while ( 1 )
	{
		if ( findNextParent( currentLa, 0, &parent ) != 0 )
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
		if ( findNextParent( currentLa, 1, &parent ) != 0 )
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
static TROT_INT findNextParent( TrotListActual *la, TROT_INT queryVisited, TrotListActual **parent )
{
	/* DATA */
	TrotListRefListNode *refNode = NULL;

	TROT_INT i = 0;

	TrotListActual *tempParent = NULL;


	/* PRECOND */
	PARANOID_ERR_IF( la == NULL );
	PARANOID_ERR_IF( parent == NULL );


	/* CODE */
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
		"List Overflow Error",
		"Invalid Op Error",
		"Bad Tag Error",
		"Divide By Zero Error",
		"Unicode Error",
		"Decode Error"
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

