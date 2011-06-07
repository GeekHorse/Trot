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


	/* PRECOND */
	PRECOND_ERR_IF( stack == NULL );
	PRECOND_ERR_IF( (*stack) != NULL );


	/* CODE */
	TROT_MALLOC( newHead, trotStackNode, 1 );
	TROT_MALLOC( newTail, trotStackNode, 1 );
	TROT_MALLOC( newStack, trotStack, 1 );

	newHead -> l = NULL;
	newHead -> n = 0;
	newHead -> prev = newHead;
	newHead -> next = newTail;

	newTail -> l = NULL;
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
int trotStackFree( trotStack **stack )
{
	/* DATA */
	trotStackNode *node = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( stack == NULL );


	/* CODE */
	if ( (*stack) == NULL )
	{
		return TROT_LIST_SUCCESS;
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

	return TROT_LIST_SUCCESS;
}

/******************************************************************************/
int trotStackPush( trotStack *stack, trotList *l )
{
	/* DATA */
	int rc = TROT_LIST_SUCCESS;

	trotStackNode *newNode = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( stack == NULL );
	PRECOND_ERR_IF( l == NULL );


	/* CODE */
	TROT_MALLOC( newNode, trotStackNode, 1 );

	newNode -> l = l;
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
int trotStackPop( trotStack *stack )
{
	/* DATA */
	int rc = TROT_LIST_SUCCESS;

	trotStackNode *node = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( stack == NULL );


	/* CODE */
	ERR_IF( stack -> tail -> prev == stack -> head, TROT_LIST_ERROR_GENERAL );

	node = stack -> tail -> prev;

	node -> prev -> next = node -> next;
	node -> next -> prev = node -> prev;

	trotFree( node );

	return TROT_LIST_SUCCESS;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
int trotStackIncrementTopN( trotStack *stack )
{
	/* DATA */
	int rc = TROT_LIST_SUCCESS;


	/* PRECOND */
	PRECOND_ERR_IF( stack == NULL );


	/* CODE */
	ERR_IF( stack -> tail -> prev == stack -> head, TROT_LIST_ERROR_GENERAL );

	stack -> tail -> prev -> n += 1;

	return TROT_LIST_SUCCESS;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
int trotStackGet( trotStack *stack, trotList **l, int *n )
{
	/* DATA */
	int rc = TROT_LIST_SUCCESS;


	/* PRECOND */
	PRECOND_ERR_IF( stack == NULL );
	PRECOND_ERR_IF( l == NULL );
	PRECOND_ERR_IF( (*l) != NULL );
	PRECOND_ERR_IF( n == NULL );


	/* CODE */
	ERR_IF( stack -> tail -> prev == stack -> head, TROT_LIST_ERROR_GENERAL );

	(*l) = stack -> tail -> prev -> l;
	(*n) = stack -> tail -> prev -> n;

	return TROT_LIST_SUCCESS;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
int trotStackQueryContains( trotStack *stack, trotList *l, TROT_STACK_CONTAINS *contains )
{
	/* DATA */
	trotStackNode *node = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( stack == NULL );
	PRECOND_ERR_IF( l == NULL );
	PRECOND_ERR_IF( contains == NULL );


	/* CODE */
	(*contains) = TROT_STACK_DOES_NOT_CONTAIN;

	node = stack -> head -> next;
	while ( node != stack -> tail )
	{
		if ( node -> l == l )
		{
			(*contains) = TROT_STACK_DOES_CONTAIN;
			return TROT_LIST_SUCCESS;
		}

		node = node -> next;
	}

	return TROT_LIST_SUCCESS;
}

