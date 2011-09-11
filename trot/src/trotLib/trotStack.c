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
	Handles stacks of lists. Not used by the end user nor for any
	actual data. Only used internally.
	Used for memory management to go "up" to see if a list is still
	reachable.
*/

/******************************************************************************/
#include "trotCommon.h"
#include "trotStack.h"
#include "trotMem.h"

/******************************************************************************/
int trotStackInit( trotStack **stack )
{
	/* DATA */
	int rc = TROT_LIST_SUCCESS;

	trotStack *newStack = NULL;

	trotStackNode *newHead = NULL;
	trotStackNode *newTail = NULL;


	/* CODE */
	ERR_IF_PARANOID( stack == NULL );
	ERR_IF_PARANOID( (*stack) != NULL );

	TROT_MALLOC( newHead, trotStackNode, 1 );
	TROT_MALLOC( newTail, trotStackNode, 1 );
	TROT_MALLOC( newStack, trotStack, 1 );

	newHead -> l1 = NULL;
	newHead -> l2 = NULL;
	newHead -> n = 0;
	newHead -> prev = newHead;
	newHead -> next = newTail;

	newTail -> l1 = NULL;
	newTail -> l2 = NULL;
	newTail -> n = 0;
	newTail -> prev = newHead;
	newTail -> next = newTail;

	newStack -> head = newHead;
	newStack -> tail = newTail;

	/* give back */
	(*stack) = newStack;
	newStack = NULL;

	return TROT_LIST_SUCCESS;


	/* CLEANUP */
	cleanup:

	trotFree( newHead );
	trotFree( newTail );
	trotFree( newStack );

	return rc;
}

/******************************************************************************/
void trotStackFree( trotStack **stack )
{
	/* DATA */
	trotStackNode *node = NULL;


	/* CODE */
	ERR_IF_PARANOID( stack == NULL );

	if ( (*stack) == NULL )
	{
		return;
	}

	node = (*stack) -> head;
	while ( node != (*stack) -> tail )
	{
		node = node -> next;

		trotFree( node -> prev );
	}

	trotFree( (*stack) -> tail );

	trotFree( (*stack) );
	(*stack) = NULL;

	return;
}

/******************************************************************************/
int trotStackPush( trotStack *stack, trotList *l1, trotList *l2 )
{
	/* DATA */
	int rc = TROT_LIST_SUCCESS;

	trotStackNode *node = NULL;

	trotStackNode *newNode = NULL;


	/* CODE */
	ERR_IF_PARANOID( stack == NULL );

	/* are these two lists already in the stack? */
	node = stack -> head -> next;
	while ( node != stack -> tail )
	{
		if (    ( node -> l1 == l1 && node -> l2 == l2 )
		     || ( node -> l1 == l2 && node -> l2 == l1 )
		   )
		{
			/* yes, so nothing to do */
			return TROT_LIST_SUCCESS;
		}

		node = node -> next;
	}

	/* not already in stack, so lets add */
	TROT_MALLOC( newNode, trotStackNode, 1 );

	newNode -> l1 = l1;
	newNode -> l2 = l2;
	newNode -> n = 0;
	newNode -> next = stack -> tail;
	newNode -> prev = stack -> tail -> prev;

	stack -> tail -> prev -> next = newNode;
	stack -> tail -> prev = newNode;

	return TROT_LIST_SUCCESS;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
int trotStackPop( trotStack *stack, int *empty )
{
	/* DATA */
	trotStackNode *node = NULL;


	/* CODE */
	ERR_IF_PARANOID( stack == NULL );
	ERR_IF_PARANOID( empty == NULL );

	ERR_IF_PARANOID( stack -> tail -> prev == stack -> head );

	node = stack -> tail -> prev;

	node -> prev -> next = node -> next;
	node -> next -> prev = node -> prev;

	trotFree( node );

	if ( stack -> tail -> prev == stack -> head )
	{
		(*empty) = 1;
	}
	else
	{
		(*empty) = 0;
	}

	return TROT_LIST_SUCCESS;
}

/******************************************************************************/
int trotStackIncrementTopN( trotStack *stack )
{
	/* CODE */
	ERR_IF_PARANOID( stack == NULL );

	ERR_IF_PARANOID( stack -> tail -> prev == stack -> head );

	stack -> tail -> prev -> n += 1;

	return TROT_LIST_SUCCESS;
}

/******************************************************************************/
int trotStackGet( trotStack *stack, trotList **l1, trotList **l2, int *n )
{
	/* CODE */
	ERR_IF_PARANOID( stack == NULL );
	ERR_IF_PARANOID( l1 == NULL );
	ERR_IF_PARANOID( l2 == NULL );
	ERR_IF_PARANOID( n == NULL );

	ERR_IF_PARANOID( stack -> tail -> prev == stack -> head );

	(*l1) = stack -> tail -> prev -> l1;
	(*l2) = stack -> tail -> prev -> l2;
	(*n) = stack -> tail -> prev -> n;

	return TROT_LIST_SUCCESS;
}

