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
#undef  TROT_FILE_NUMBER
#define TROT_FILE_NUMBER 1

/******************************************************************************/
#include "trot.h"
#include "trotInternal.h"

/******************************************************************************/
static TROT_RC trotListNodeSplit( TrotProgram *program, TrotListNode *n, TROT_INT keepInLeft );

static TROT_RC newIntNode( TrotProgram *program, TrotListNode *insertBeforeThis, TROT_INT n );
static TROT_RC newListNode( TrotProgram *program, TrotListActual *la, TrotListNode *insertBeforeThis, TrotList *l );

static TROT_RC refListAdd( TrotProgram *program, TrotListActual *la, TrotList *l );
static void refListRemove( TrotProgram *program, TrotListActual *la, TrotList *l );

static void isListReachable( TrotListActual *la );
static TROT_INT findNextParent( TrotListActual *la, TROT_INT queryVisited, TrotListActual **parent );

/******************************************************************************/
/*!
	\brief Allocates a new list and new reference to the list.
	\param[in] program List that maintains memory limit
	\param[out] l_A On success, will be new reference to a new list.
	\return TROT_RC
*/
TROT_RC trotListInit( TrotProgram *program, TrotList **l_A )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotListNode *newHead = NULL;
	TrotListNode *newTail = NULL;
	TrotListActual *newLa = NULL;

	TrotList *newL = NULL;


	/* PRECOND */
	FAILURE_POINT;
	PARANOID_ERR_IF( program == NULL );
	PARANOID_ERR_IF( l_A == NULL );
	PARANOID_ERR_IF( (*l_A) != NULL );


	/* CODE */
	/* create the data list */
	TROT_CALLOC( newHead, 1 );
	TROT_CALLOC( newTail, 1 );

	newHead->prev = newHead;
	newHead->next = newTail;

	newTail->prev = newHead;
	newTail->next = newTail;

	/* create actual list structure */
	TROT_CALLOC( newLa, 1 );

	newLa->reachable = 1;

	newLa->head = newHead;
	newLa->tail = newTail;
	newHead = NULL;
	newTail = NULL;

	/* create the first ref to this list */
	TROT_CALLOC( newL, 1 );

	newL->laPointsTo = newLa;
	newLa = NULL;

	/* add first ref to list's ref list */
	rc = refListAdd( program, newL->laPointsTo, newL );
	ERR_IF_PASSTHROUGH;

	/* give back */
	(*l_A) = newL;
	newL = NULL;

	/* special case where we return success here instead of going through cleanup. */
	return TROT_RC_SUCCESS;


	/* CLEANUP */
	cleanup:

	TROT_FREE( newHead, 1 );
	TROT_FREE( newTail, 1 );
	if ( newLa != NULL )
	{
		TROT_FREE( newLa->refList, 1 );
		TROT_FREE( newLa->head, 1 );
		TROT_FREE( newLa->tail, 1 );
		TROT_FREE( newLa, 1 );
	}
	if ( newL != NULL )
	{
		TROT_FREE( newL->laPointsTo->refList, 1 );
		TROT_FREE( newL->laPointsTo->head, 1 );
		TROT_FREE( newL->laPointsTo->tail, 1 );
		TROT_FREE( newL->laPointsTo, 1 );
		TROT_FREE( newL, 1 );
	}

	return rc;
}

/******************************************************************************/
/*!
	\brief Creates a new reference to a list.
	\param[in] program List that maintains memory limit
	\param[in] l List to create a new reference to.
	\param[out] l_A On success, new reference to the list.
	\return TROT_RC
*/
TROT_RC trotListTwin( TrotProgram *program, TrotList *l, TrotList **l_A )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotList *newL = NULL;


	/* PRECOND */
	FAILURE_POINT;
	PARANOID_ERR_IF( program == NULL );
	PARANOID_ERR_IF( l == NULL );
	PARANOID_ERR_IF( l_A == NULL );
	PARANOID_ERR_IF( (*l_A) != NULL );


	/* CODE */
	TROT_MALLOC( newL, 1 );

	newL->laParent = NULL;
	newL->laPointsTo = l->laPointsTo;

	rc = refListAdd( program, newL->laPointsTo, newL );
	ERR_IF_PASSTHROUGH;


	/* give back */
	(*l_A) = newL;
	newL = NULL;

	return TROT_RC_SUCCESS;


	/* CLEANUP */
	cleanup:

	TROT_FREE( newL, 1 );

	return rc;
}

/******************************************************************************/
/*!
	\brief Frees a list reference. Actual list will be freed if the list is no
		longer reachable.
	\param[in] program List that maintains memory limit
	\param[in] l_F Address of list reference to free.
	\return void

	l_F can be NULL, or the address of a NULL pointer, and this will be a noop.
*/
void trotListFree( TrotProgram *program, TrotList **l_F )
{
	/* DATA */
	TrotListActual *la = NULL;

	TrotListNode *node = NULL;

	TROT_INT j = 0;
	TrotListActual *laTemp = NULL;

	TrotListActual *laNext = NULL;
	TrotListActual *laCurrent = NULL;


	/* CODE */
	PARANOID_ERR_IF( program == NULL );

	if ( l_F == NULL || (*l_F) == NULL )
	{
		return;
	}

	PARANOID_ERR_IF( (*l_F)->laParent != NULL );

	la = (*l_F)->laPointsTo;

	/* remove ref from list's ref list */
	refListRemove( program, la, (*l_F) );

	/* free ref */
	TROT_FREE( (*l_F), 1 );
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
			if ( node->n != NULL )
			{
				PARANOID_ERR_IF( node->l != NULL );

				TROT_FREE( node->n, TROT_NODE_SIZE );
			}
			else
			{
				PARANOID_ERR_IF( node->n != NULL );
				PARANOID_ERR_IF( node->l == NULL );

				for ( j = 0; j < node->count; j += 1 )
				{
					laTemp = node->l[ j ]->laPointsTo;
			
					refListRemove( program, laTemp, node->l[ j ] );

					TROT_FREE( node->l[ j ], 1 );
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

				TROT_FREE( node->l, TROT_NODE_SIZE );
			}

			node = node->next;
			TROT_FREE( node->prev, 1 );
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
		TROT_FREE( laCurrent->head, 1 );
		TROT_FREE( laCurrent->tail, 1 );
		TROT_FREE( laCurrent->refList, 1 );
		TROT_FREE( laCurrent, 1 );
	}

	return;
}

/******************************************************************************/
/*!
	\brief Compares list references to see if they point to the same list.
	\param[in] program List that maintains memory limit
	\param[in] l1 First reference.
	\param[in] l2 Second reference.
	\param[out] isSame 0 if refs point to different lists, 1 if they point to
		the same lists.
	\return TROT_RC
*/
TROT_RC trotListRefCompare( TrotProgram *program, TrotList *l1, TrotList *l2, TROT_INT *isSame )
{
	/* PRECOND */
	FAILURE_POINT;
	PARANOID_ERR_IF( program == NULL );
	PARANOID_ERR_IF( l1 == NULL );
	PARANOID_ERR_IF( l2 == NULL );
	PARANOID_ERR_IF( isSame == NULL );


	/* CODE */
	(void)program;

	(*isSame) = 0;

	if ( l1->laPointsTo == l2->laPointsTo )
	{
		(*isSame) = 1;
	}

	return TROT_RC_SUCCESS;
}

/******************************************************************************/
/*!
	\brief Gets the count of items in the list.
	\param[in] program List that maintains memory limit
	\param[in] l Pointer to a TrotList pointer.
	\param[out] count On success, will contain the count of this list.
	\return TROT_RC
*/
TROT_RC trotListGetCount( TrotProgram *program, TrotList *l, TROT_INT *count )
{
	/* PRECOND */
	FAILURE_POINT;
	PARANOID_ERR_IF( program == NULL );
	PARANOID_ERR_IF( l == NULL );
	PARANOID_ERR_IF( count == NULL );


	/* CODE */
	(void)program;

	(*count) = l->laPointsTo->childrenCount;

	PARANOID_ERR_IF( (*count) > TROT_MAX_CHILDREN );

	return TROT_RC_SUCCESS;
}

/******************************************************************************/
/*!
	\brief Gets the kind of an item in the list.
	\param[in] program List that maintains memory limit
	\param[in] l The list.
	\param[in] index Index of the item in the list to ge the kind of.
	\param[out] kind On success, will contain the kind of the item.
	\return TROT_RC
*/
TROT_RC trotListGetKind( TrotProgram *program, TrotList *l, TROT_INT index, TROT_INT *kind )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotListActual *la = NULL;

	TrotListNode *node = NULL;

	TROT_INT count = 0;


	/* PRECOND */
	FAILURE_POINT;
	PARANOID_ERR_IF( program == NULL );
	PARANOID_ERR_IF( l == NULL );
	PARANOID_ERR_IF( kind == NULL );


	/* CODE */
	(void)program;

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

	if ( node->n != NULL )
	{
		(*kind) = NODE_KIND_INT;
	}
	else
	{
		(*kind) = NODE_KIND_LIST;
	}

	return TROT_RC_SUCCESS;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Appends an int to the end of the list.
	\param[in] program List that maintains memory limit
	\param[in] l The list.
	\param[in] n The int to append.
	\return TROT_RC
*/
TROT_RC trotListAppendInt( TrotProgram *program, TrotList *l, TROT_INT n )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotListActual *la = NULL;
	TrotListNode *node = NULL;


	/* PRECOND */
	FAILURE_POINT;
	PARANOID_ERR_IF( program == NULL );
	PARANOID_ERR_IF( l == NULL );


	/* CODE */
	la = l->laPointsTo;

	/* lists cannot hold more than TROT_MAX_CHILDREN, so make sure we have room */
	ERR_IF( TROT_MAX_CHILDREN - la->childrenCount < 1, TROT_RC_ERROR_LIST_OVERFLOW );

	/* *** */
	node = la->tail->prev;

	/* special cases to create new node */
	if (    node == la->head             /* empty list */
	     || node->n == NULL /* last node is not int kind */
	     || node->count == TROT_NODE_SIZE    /* last node is full */
	   )
	{
		rc = newIntNode( program, la->tail, n );
		ERR_IF_PASSTHROUGH;
	}
	else
	{
		node->n[ node->count ] = n;
		node->count += 1;
	}


	la->childrenCount += 1;

	return TROT_RC_SUCCESS;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Appends a list to the end of the list.
	\param[in] program List that maintains memory limit
	\param[in] l The list to append to.
	\param[in] lToAppend The list to append.
	\return TROT_RC
*/
TROT_RC trotListAppendList( TrotProgram *program, TrotList *l, TrotList *lToAppend )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotListActual *la = NULL;
	TrotListNode *node = NULL;

	TrotList *newL = NULL;


	/* PRECOND */
	FAILURE_POINT;
	PARANOID_ERR_IF( program == NULL );
	PARANOID_ERR_IF( l == NULL );
	PARANOID_ERR_IF( lToAppend == NULL );


	/* CODE */
	la = l->laPointsTo;

	/* lists cannot hold more than TROT_MAX_CHILDREN, so make sure we have room */
	ERR_IF( TROT_MAX_CHILDREN - la->childrenCount < 1, TROT_RC_ERROR_LIST_OVERFLOW );

	/* *** */
	node = la->tail->prev;


	/* special cases to create new node */
	if (    node == la->head              /* empty list */
	     || node->l == NULL /* last node is not list kind */
	     || node->count == TROT_NODE_SIZE     /* last node is full */
	   )
	{
		rc = newListNode( program, la, la->tail, lToAppend );
		ERR_IF_PASSTHROUGH;
	}
	else
	{
		/* *** */
		rc = trotListTwin( program, lToAppend, &newL );
		ERR_IF_PASSTHROUGH;

		/* append */
		node->l[ node->count ] = newL;
		newL->laParent = la;
		newL = NULL;

		node->count += 1;
	}

	la->childrenCount += 1;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Inserts an int into the list.
	\param[in] program List that maintains memory limit
	\param[in] l The list to insert into.
	\param[in] index Where to insert.
	\param[in] n The int to insert.
	\return TROT_RC
*/
TROT_RC trotListInsertInt( TrotProgram *program, TrotList *l, TROT_INT index, TROT_INT n )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotListActual *la = NULL;

	TrotListNode *node = NULL;

	TROT_INT count = 0;

	TROT_INT i = 0;
	TROT_INT j = 0;


	/* PRECOND */
	FAILURE_POINT;
	PARANOID_ERR_IF( program == NULL );
	PARANOID_ERR_IF( l == NULL );


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
		rc = trotListAppendInt( program, l, n );
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
	if ( node->n != NULL )
	{
		/* If node is full */
		if ( node->count == TROT_NODE_SIZE )
		{
			rc = trotListNodeSplit( program, node, TROT_NODE_SIZE / 2 );
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
	else /* node is list kind */
	{
		i = index - count - 1;

		/* If we need to insert at spot 0, we see if the previous node
		   is an int node with room. If so, we can just append to that
		   node. */
		if (    i == 0
		     && node->prev->n != NULL
		     && node->prev->count != TROT_NODE_SIZE 
		   )
		{
			node = node->prev;

			/* Insert int into node */
			node->n[ node->count ] = n;
			node->count += 1;

			la->childrenCount += 1;

			return TROT_RC_SUCCESS; /* TODO: we need to get rid of early returns */
		}

		/* if not at beginning, we'll have to split the node */
		if ( i != 0 )
		{
			rc = trotListNodeSplit( program, node, i );
			ERR_IF_PASSTHROUGH;

			node = node->next;
		}

		/* *** */
		rc = newIntNode( program, node, n );
		ERR_IF_PASSTHROUGH;

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
	\param[in] program List that maintains memory limit
	\param[in] l The list to insert into.
	\param[in] index Where to insert.
	\param[in] lToInsert The list to insert.
	\return TROT_RC
*/
TROT_RC trotListInsertList( TrotProgram *program, TrotList *l, TROT_INT index, TrotList *lToInsert )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotListActual *la = NULL;

	TrotListNode *node = NULL;

	TROT_INT count = 0;

	TROT_INT i = 0;
	TROT_INT j = 0;

	TrotList *newL = NULL;


	/* PRECOND */
	FAILURE_POINT;
	PARANOID_ERR_IF( program == NULL );
	PARANOID_ERR_IF( l == NULL );
	PARANOID_ERR_IF( lToInsert == NULL );


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
		rc = trotListAppendList( program, l, lToInsert );
		ERR_IF_PASSTHROUGH;

		return TROT_RC_SUCCESS;
	}

	/* Make sure index is in range */
	ERR_IF_1( index <= 0, TROT_RC_ERROR_BAD_INDEX, index );
	ERR_IF_1( index > (la->childrenCount ), TROT_RC_ERROR_BAD_INDEX, index );

	/* FUTURE: factor this out into a function, maybe include index check above too */
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
	if ( node->l != NULL )
	{
		/* If node is full */
		if ( node->count == TROT_NODE_SIZE )
		{
			rc = trotListNodeSplit( program, node, TROT_NODE_SIZE / 2 );
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
		rc = trotListTwin( program, lToInsert, &newL );
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
	else /* node is int kind */
	{
		i = index - count - 1;

		/* If we need to insert at spot 0, we see if the previous node
		   is an list node with room. If so, we can just append to that
		   node. */
		if (    i == 0
		     && node->prev->l != NULL
		     && node->prev->count != TROT_NODE_SIZE 
		   )
		{
			node = node->prev;

			/* Insert list into node */
			rc = trotListTwin( program, lToInsert, &newL );
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
			rc = trotListNodeSplit( program, node, i );
			ERR_IF_PASSTHROUGH;

			node = node->next;
		}

		/* *** */
		rc = newListNode( program, la, node, lToInsert );
		ERR_IF_PASSTHROUGH;

		la->childrenCount += 1;
	}


	/* CLEANUP */
	cleanup:

	trotListFree( program, &newL );

	return rc;
}

/******************************************************************************/
/*!
	\brief Get int at index in list.
	\param[in] program List that maintains memory limit
	\param[in] l The list.
	\param[in] index Which int to get.
	\param[out] n On success, the int that was at index in l.
	\return TROT_RC
*/
TROT_RC trotListGetInt( TrotProgram *program, TrotList *l, TROT_INT index, TROT_INT *n )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotListActual *la = NULL;

	TrotListNode *node = NULL;

	TROT_INT count = 0;


	/* PRECOND */
	FAILURE_POINT;
	PARANOID_ERR_IF( program == NULL );
	PARANOID_ERR_IF( l == NULL );
	PARANOID_ERR_IF( n == NULL );


	/* CODE */
	(void)program;

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

	ERR_IF( node->n == NULL, TROT_RC_ERROR_WRONG_KIND );

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
	\param[in] program List that maintains memory limit
	\param[in] l The list.
	\param[in] index Which list to get.
	\param[out] lTwin_A On success, the list that was at index in l.
	\return TROT_RC
*/
TROT_RC trotListGetList( TrotProgram *program, TrotList *l, TROT_INT index, TrotList **lTwin_A )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotListNode *node = NULL;

	TROT_INT count = 0;

	TrotList *newL = NULL;


	/* PRECOND */
	FAILURE_POINT;
	PARANOID_ERR_IF( program == NULL );
	PARANOID_ERR_IF( l == NULL );
	PARANOID_ERR_IF( lTwin_A == NULL );
	PARANOID_ERR_IF( (*lTwin_A) != NULL );


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

	ERR_IF( node->l == NULL, TROT_RC_ERROR_WRONG_KIND );

	rc = trotListTwin( program, node->l[ (node->count) - 1 - (count - index) ], &newL );
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
	\param[in] program List that maintains memory limit
	\param[in] l The list.
	\param[in] index Which int to get and remove.
	\param[out] n On success, the int that was at index in l.
	\return TROT_RC
*/
TROT_RC trotListRemoveInt( TrotProgram *program, TrotList *l, TROT_INT index, TROT_INT *n )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotListNode *node = NULL;

	TROT_INT count = 0;

	TROT_INT giveBackN = 0;

	TROT_INT i = 0;


	/* PRECOND */
	FAILURE_POINT;
	PARANOID_ERR_IF( program == NULL );
	PARANOID_ERR_IF( l == NULL );
	PARANOID_ERR_IF( n == NULL );


	/* CODE */
	(void)program;

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

	ERR_IF( node->n == NULL, TROT_RC_ERROR_WRONG_KIND );

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

		TROT_FREE( node->n, TROT_NODE_SIZE );
		TROT_FREE( node, 1 );
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
	\param[in] program List that maintains memory limit
	\param[in] l The list.
	\param[in] index Which list to get.
	\param[out] lRemoved_A On success, list that was at index in l.
	\return TROT_RC
*/
TROT_RC trotListRemoveList( TrotProgram *program, TrotList *l, TROT_INT index, TrotList **lRemoved_A )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotListNode *node = NULL;

	TROT_INT count = 0;

	TrotList *giveBackL = NULL;

	TROT_INT i = 0;


	/* PRECOND */
	FAILURE_POINT;
	PARANOID_ERR_IF( program == NULL );
	PARANOID_ERR_IF( l == NULL );
	PARANOID_ERR_IF( lRemoved_A == NULL );
	PARANOID_ERR_IF( (*lRemoved_A) != NULL );


	/* CODE */
	(void)program;

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

	ERR_IF( node->l == NULL, TROT_RC_ERROR_WRONG_KIND );

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

		TROT_FREE( node->l, TROT_NODE_SIZE );
		TROT_FREE( node, 1 );
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
	\param[in] program List that maintains memory limit
	\param[in] l The list.
	\param[in] index What to remove.
	\return TROT_RC
*/
TROT_RC trotListRemove( TrotProgram *program, TrotList *l, TROT_INT index )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotListNode *node = NULL;

	TROT_INT count = 0;

	TrotList *tempL = NULL;

	TROT_INT i = 0;


	/* PRECOND */
	FAILURE_POINT;
	PARANOID_ERR_IF( program == NULL );
	PARANOID_ERR_IF( l == NULL );


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
	if ( node->n != NULL )
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
		trotListFree( program, &tempL );
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

		if ( node->n != NULL )
		{
			TROT_FREE( node->n, TROT_NODE_SIZE );
		}
		else
		{
			TROT_FREE( node->l, TROT_NODE_SIZE );
		}
		TROT_FREE( node, 1 );
	}

	return TROT_RC_SUCCESS;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Replaces whatever is at index in l with an int.
	\param[in] program List that maintains memory limit
	\param[in] l The list.
	\param[in] index Which item to replace.
	\param[in] n The int to put at index in l.
	\return TROT_RC
*/
TROT_RC trotListReplaceWithInt( TrotProgram *program, TrotList *l, TROT_INT index, TROT_INT n )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotListActual *la = NULL;

	TrotListNode *node = NULL;

	TROT_INT count = 0;

	TrotList *tempL = NULL;

	TROT_INT i = 0;
	TROT_INT j = 0;


	/* PRECOND */
	FAILURE_POINT;
	PARANOID_ERR_IF( program == NULL );
	PARANOID_ERR_IF( l == NULL );


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
	if ( node->n != NULL )
	{
		i = index - count - 1;

		/* replace int into node */
		node->n[ i ] = n;
	}
	else /* node is list kind */
	{
		i = index - count - 1;

		/* If at beginning of node */
		if ( i == 0 )
		{
			/* If the previous node is an int node with space, we
			   can just append in that node. */
			if (    node->prev->n != NULL
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
				rc = newIntNode( program, node, n );
				ERR_IF_PASSTHROUGH;
			}
		}
		/* else if end of node */
		else if ( i == (node->count) - 1 )
		{
			/* if the next node is an int node with room, we can just prepend to
			   that node. */
			if (    node->next->n != NULL
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
				rc = newIntNode( program, node->next, n );
				ERR_IF_PASSTHROUGH;
			}
		}
		/* we'll have to split the node */
		else
		{
			rc = trotListNodeSplit( program, node, i + 1 );
			ERR_IF_PASSTHROUGH;

			/* *** */
			rc = newIntNode( program, node->next, n );
			ERR_IF_PASSTHROUGH;
		}

		/* we've put in our int, now we need to remove a list */
		tempL = node->l[ i ];
		tempL->laParent = NULL;
		trotListFree( program, &tempL );
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

			TROT_FREE( node->l, TROT_NODE_SIZE );
			TROT_FREE( node, 1 );
		}
	}


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Replaces whatever is at index in l with a list.
	\param[in] program List that maintains memory limit
	\param[in] l The list.
	\param[in] index Which item to replace.
	\param[in[ lToInsert The list to put at index in l.
	\return TROT_RC
*/
TROT_RC trotListReplaceWithList( TrotProgram *program, TrotList *l, TROT_INT index, TrotList *lToInsert )
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


	/* PRECOND */
	FAILURE_POINT;
	PARANOID_ERR_IF( program == NULL );
	PARANOID_ERR_IF( l == NULL );
	PARANOID_ERR_IF( lToInsert == NULL );


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

	/* *** */
	if ( node->l != NULL )
	{
		/* create our new twin */
		rc = trotListTwin( program, lToInsert, &newL );
		ERR_IF_PASSTHROUGH;

		i = index - count - 1;

		/* free old */
		tempL = node->l[ i ];
		tempL->laParent = NULL;
		trotListFree( program, &tempL );

		/* replace with new */
		node->l[ i ] = newL;
		newL->laParent = la;
		newL = NULL;
	}
	else /* node is int kind */
	{
		i = index - count - 1;

		/* If at beginning of node */
		if ( i == 0 )
		{
			/* If the previous node is a list node with space, we
			   can just append in that node. */
			if (    node->prev->l != NULL
			     && node->prev->count != TROT_NODE_SIZE 
			   )
			{
				/* create our new twin */
				rc = trotListTwin( program, lToInsert, &newL );
				ERR_IF_PASSTHROUGH;

				/* append into prev node */
				node->prev->l[ node->prev->count ] = newL;
				newL->laParent = la;
				newL = NULL;

				node->prev->count += 1;
			}
			else
			{
				/* *** */
				rc = newListNode( program, la, node, lToInsert );
				ERR_IF_PASSTHROUGH;
			}
		}
		/* else if end of node */
		else if ( i == (node->count) - 1 )
		{
			/* if the next node is a list node with room, we can just prepend to
			   that node. */
			if (    node->next->l != NULL
			     && node->next->count != TROT_NODE_SIZE 
			   )
			{
				/* create our new twin */
				rc = trotListTwin( program, lToInsert, &newL );
				ERR_IF_PASSTHROUGH;

				/* prepend list */
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
				rc = newListNode( program, la, node->next, lToInsert );
				ERR_IF_PASSTHROUGH;
			}
		}
		/* we'll have to split the node */
		else
		{
			rc = trotListNodeSplit( program, node, i + 1 );
			ERR_IF_PASSTHROUGH;

			/* *** */
			rc = newListNode( program, la, node->next, lToInsert );
			ERR_IF_PASSTHROUGH;
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

			TROT_FREE( node->n, TROT_NODE_SIZE );
			TROT_FREE( node, 1 );
		}
	}


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Gets type of list.
	\param[in] program List that maintains memory limit
	\param[in] l The list.
	\param[out] type The type of the list.
	\return TROT_RC
*/
TROT_RC trotListGetType( TrotProgram *program, TrotList *l, TROT_INT *type )
{
	/* PRECOND */
	FAILURE_POINT;
	PARANOID_ERR_IF( program == NULL );
	PARANOID_ERR_IF( l == NULL );
	PARANOID_ERR_IF( type == NULL );


	/* CODE */
	(void)program;

	(*type) = l->laPointsTo->type;

	return TROT_RC_SUCCESS;
}

/******************************************************************************/
/*!
	\brief Sets type of list.
	\param[in] program List that maintains memory limit
	\param[in] l The list.
	\param[in] tag Type value to set.
	\return TROT_RC
*/
TROT_RC trotListSetType( TrotProgram *program, TrotList *l, TROT_INT type )
{
	/* PRECOND */
	FAILURE_POINT;
	PARANOID_ERR_IF( program == NULL );
	PARANOID_ERR_IF( l == NULL );


	/* CODE */
	(void)program;

	l->laPointsTo->type = type;

	return TROT_RC_SUCCESS;
}

/******************************************************************************/
/*!
	\brief Gets tag of list.
	\param[in] program List that maintains memory limit
	\param[in] l The list.
	\param[out] tag The tag of the list.
	\return TROT_RC
*/
TROT_RC trotListGetTag( TrotProgram *program, TrotList *l, TROT_INT *tag )
{
	/* PRECOND */
	FAILURE_POINT;
	PARANOID_ERR_IF( program == NULL );
	PARANOID_ERR_IF( l == NULL );
	PARANOID_ERR_IF( tag == NULL );


	/* CODE */
	(void)program;

	(*tag) = l->laPointsTo->tag;

	return TROT_RC_SUCCESS;
}

/******************************************************************************/
/*!
	\brief Sets the tag of a list.
	\param[in] program List that maintains memory limit
	\param[in] l The list.
	\param[in] tag The tag value to set.
	\return TROT_RC
*/
TROT_RC trotListSetTag( TrotProgram *program, TrotList *l, TROT_INT tag )
{
	/* PRECOND */
	FAILURE_POINT;
	PARANOID_ERR_IF( program == NULL );
	PARANOID_ERR_IF( l == NULL );


	/* CODE */
	(void)program;

	l->laPointsTo->tag = tag;

	return TROT_RC_SUCCESS;
}

/******************************************************************************/
/*!
	\brief Splits a node, leaving keepInLeft into the left/prev node, and
		moving the rest into the new right/next node.
	\param[in] program List that maintains memory limit
	\param[in] n Node to split.
	\param[in] keepInLeft How many items to keep in n.
	\return TROT_RC
*/
static TROT_RC trotListNodeSplit( TrotProgram *program, TrotListNode *n, TROT_INT keepInLeft )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotListNode *newNode = NULL;

	TROT_INT i = 0;


	/* PRECOND */
	PARANOID_ERR_IF( program == NULL );
	PARANOID_ERR_IF( n == NULL );


	/* CODE */
	TROT_MALLOC( newNode, 1 );

	if ( n->n != NULL )
	{
		newNode->count = (n->count) - keepInLeft;

		newNode->l = NULL;
		TROT_MALLOC( newNode->n, TROT_NODE_SIZE );

		i = keepInLeft;
		while ( i < (n->count) )
		{
			newNode->n[ i - keepInLeft ] = n->n[ i ];

			i += 1;
		}

		n->count = keepInLeft;
	}
	else /* n is list kind */
	{
		newNode->count = (n->count) - keepInLeft;

		newNode->n = NULL;
		TROT_CALLOC( newNode->l, TROT_NODE_SIZE );

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

	TROT_FREE( newNode, 1 );

	return rc;
}

/******************************************************************************/
/*!
	\brief Creates a new TrotListNode for Int.
	\param[in] program List that maintains memory limit
	\param[in] insertBeforeThis Node in list to insert before
	\param[in] n Int to insert into new node.
	\return TROT_RC
*/
static TROT_RC newIntNode( TrotProgram *program, TrotListNode *insertBeforeThis, TROT_INT n )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotListNode *newNode = NULL;


	/* PRECOND */
	PARANOID_ERR_IF( program == NULL );
	PARANOID_ERR_IF( insertBeforeThis == NULL );


	/* CODE */
	TROT_MALLOC( newNode, 1 );

	newNode->l = NULL;
	TROT_MALLOC( newNode->n, TROT_NODE_SIZE );

	newNode->count = 1;

	newNode->n[ 0 ] = n;

	/* insert node in list */
	newNode->next = insertBeforeThis;
	newNode->prev = insertBeforeThis->prev;

	insertBeforeThis->prev->next = newNode;
	insertBeforeThis->prev = newNode;

	return TROT_RC_SUCCESS;


	/* CLEANUP */
	cleanup:

	TROT_FREE( newNode, 1 );

	return rc;
}

/******************************************************************************/
/*!
	\brief Creates a new TrotListNode for List.
	\param[in] program List that maintains memory limit
	\param[in] insertBeforeThis Node in list to insert before
	\param[in] l List to twin and insert into new node
	\return TROT_RC
*/
static TROT_RC newListNode( TrotProgram *program, TrotListActual *la, TrotListNode *insertBeforeThis, TrotList *l )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotListNode *newNode = NULL;

	TrotList *newL = NULL;


	/* PRECOND */
	PARANOID_ERR_IF( program == NULL );
	PARANOID_ERR_IF( la == NULL );
	PARANOID_ERR_IF( insertBeforeThis == NULL );
	PARANOID_ERR_IF( l == NULL );


	/* CODE */
	rc = trotListTwin( program, l, &newL );
	ERR_IF_PASSTHROUGH;

	TROT_MALLOC( newNode, 1 );

	newNode->n = NULL;
	TROT_CALLOC( newNode->l, TROT_NODE_SIZE );

	newNode->count = 1;

	newNode->l[ 0 ] = newL;
	newL->laParent = la;
	newL = NULL;

	/* insert node in list */
	newNode->next = insertBeforeThis;
	newNode->prev = insertBeforeThis->prev;

	insertBeforeThis->prev->next = newNode;
	insertBeforeThis->prev = newNode;

	return TROT_RC_SUCCESS;


	/* CLEANUP */
	cleanup:

	trotListFree( program, &newL );
	TROT_FREE( newNode, 1 );

	return rc;
}

/******************************************************************************/
static TROT_RC refListAdd( TrotProgram *program, TrotListActual *la, TrotList *l )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotListRefListNode *newRefNode = NULL;


	/* PRECOND */
	PARANOID_ERR_IF( program == NULL );
	PARANOID_ERR_IF( la == NULL );
	PARANOID_ERR_IF( l == NULL );


	/* CODE */
	TROT_MALLOC( newRefNode, 1 );

	newRefNode->l = l;

	newRefNode->next = la->refList;
	la->refList = newRefNode;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
static void refListRemove( TrotProgram *program, TrotListActual *la, TrotList *l )
{
	/* DATA */
	TrotListRefListNode *refPrev = NULL;
	TrotListRefListNode *refNode = NULL;


	/* PRECOND */
	PARANOID_ERR_IF( program == NULL );
	PARANOID_ERR_IF( la == NULL );
	PARANOID_ERR_IF( l == NULL );


	/* CODE */
	refNode = la->refList;

	/* edge case */
	if ( refNode->l == l )
	{
		la->refList = refNode->next;
		TROT_FREE( refNode, 1 );

		return;
	}

	/* foreach refNode */
	while ( 1 )
	{
		refPrev = refNode;
		refNode = refNode->next;

		if ( refNode->l == l )
		{
			refPrev->next = refNode->next;
			TROT_FREE( refNode, 1 );

			return;
		}

		PARANOID_ERR_IF( refNode->next == NULL );
	}

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

	TrotListActual *tempParent = NULL;


	/* PRECOND */
	PARANOID_ERR_IF( la == NULL );
	PARANOID_ERR_IF( parent == NULL );


	/* CODE */
	/* for each reference that points to this list */
	refNode = la->refList;
	while ( refNode != NULL )
	{
		/* get list this ref is in */
		tempParent = refNode->l->laParent;

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
		"Failure Point Error",

		"Bad Index Error",
		"Wrong Kind Error",
		"List Overflow Error",
		"Invalid Op Error",
		"Bad Type Error",
		"Divide By Zero Error",
		"Decode Error",
		"Mem Limit Error"
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

