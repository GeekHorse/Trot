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
	Only used during Compare.
	Keeps track of a stack of two lists we're comparing.
	Used for comparing, and so we don't get into an infinite loop.
*/

/******************************************************************************/
#include "trot.h"
#include "trotInternal.h"

/******************************************************************************/
/*!
	\brief Creates a new stack.
	\param stack On success, the new stack.
	\return TROT_RC
*/
TROT_RC trotStackInit( trotStack **stack )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	trotStack *newStack = NULL;

	trotStackNode *newHead = NULL;
	trotStackNode *newTail = NULL;


	/* CODE */
	PARANOID_ERR_IF( stack == NULL );
	PARANOID_ERR_IF( (*stack) != NULL );

	TROT_MALLOC( newHead, trotStackNode, 1 );
	TROT_MALLOC( newTail, trotStackNode, 1 );
	TROT_MALLOC( newStack, trotStack, 1 );

	newHead -> l1 = NULL;
	newHead -> l1Node = NULL;
	newHead -> l1Count = 0;
	newHead -> l2 = NULL;
	newHead -> l2Node = NULL;
	newHead -> l2Count = 0;
	newHead -> index = 0;
	newHead -> prev = newHead;
	newHead -> next = newTail;

	newTail -> l1 = NULL;
	newTail -> l1Node = NULL;
	newTail -> l1Count = 0;
	newTail -> l2 = NULL;
	newTail -> l2Node = NULL;
	newTail -> l2Count = 0;
	newTail -> index = 0;
	newTail -> prev = newHead;
	newTail -> next = newTail;

	newStack -> head = newHead;
	newStack -> tail = newTail;

	/* give back */
	(*stack) = newStack;
	newStack = NULL;

	return TROT_RC_SUCCESS;


	/* CLEANUP */
	cleanup:

	trotFree( newHead );
	trotFree( newTail );
	trotFree( newStack );

	return rc;
}

/******************************************************************************/
/*!
	\brief Frees a stack.
	\param stack The stack to free.
	\return void
*/
void trotStackFree( trotStack **stack )
{
	/* DATA */
	trotStackNode *node = NULL;


	/* CODE */
	PARANOID_ERR_IF( stack == NULL );

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
/*!
	\brief Pushes a new node on the stack .
	\param stack The stack to push to.
	\param l1 The first list.
	\param l2 The second list.
	\return TROT_RC
*/
TROT_RC trotStackPush( trotStack *stack, trotListActual *l1, trotListActual *l2 )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	trotStackNode *node = NULL;

	trotStackNode *newNode = NULL;


	/* CODE */
	PARANOID_ERR_IF( stack == NULL );

	/* are these two lists already in the stack? */
	node = stack -> head -> next;
	while ( node != stack -> tail )
	{
		if (    ( node -> l1 == l1 && node -> l2 == l2 )
		     || ( node -> l1 == l2 && node -> l2 == l1 )
		   )
		{
			/* yes, so nothing to do */
			return TROT_RC_SUCCESS;
		}

		node = node -> next;
	}

	/* not already in stack, so lets add */
	TROT_MALLOC( newNode, trotStackNode, 1 );

	newNode -> l1 = l1;
	newNode -> l1Node = l1 -> head;
	newNode -> l1Count = 0;
	newNode -> l2 = l2;
	newNode -> l2Node = l2 -> head;
	newNode -> l2Count = 0;
	newNode -> index = 0;
	newNode -> next = stack -> tail;
	newNode -> prev = stack -> tail -> prev;

	stack -> tail -> prev -> next = newNode;
	stack -> tail -> prev = newNode;

	return TROT_RC_SUCCESS;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Pops off the top of the stack.
	\param stack The stack to pop off from.
	\param empty After return, will be 1 if stack is empty, or 0 if stack
	       has items still on it.
	\return TROT_RC
*/
TROT_RC trotStackPop( trotStack *stack, int *empty )
{
	/* DATA */
	trotStackNode *node = NULL;


	/* CODE */
	PARANOID_ERR_IF( stack == NULL );
	PARANOID_ERR_IF( empty == NULL );

	PARANOID_ERR_IF( stack -> tail -> prev == stack -> head );

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

	return TROT_RC_SUCCESS;
}

/******************************************************************************/
/*!
	\brief Increments the data on top the stack.
	\param stack The stack we want to manipulate.
	\return TROT_RC
*/
TROT_RC trotStackIncrementTopIndex( trotStack *stack )
{
	/* DATA */
	trotStackNode *stackNode = NULL;

	/* CODE */
	PARANOID_ERR_IF( stack == NULL );

	PARANOID_ERR_IF( stack -> tail -> prev == stack -> head );

	stackNode = stack -> tail -> prev;

	stackNode -> index += 1;

	stackNode -> l1Count += 1;
	if ( stackNode -> l1Count >= stackNode -> l1Node -> count )
	{
		stackNode -> l1Node = stackNode -> l1Node -> next;
		stackNode -> l1Count = 0;
	}

	stackNode -> l2Count += 1;
	if ( stackNode -> l2Count >= stackNode -> l2Node -> count )
	{
		stackNode -> l2Node = stackNode -> l2Node -> next;
		stackNode -> l2Count = 0;
	}

	return TROT_RC_SUCCESS;
}

