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

/******************************************************************************/
inline int gkStackInit( gkStack **stack )
{
	/* DATA */
	int rc = GK_LIST_SUCCESS;

	gkStack *newStack = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( stack == NULL );
	PRECOND_ERR_IF( (*stack) != NULL );


	/* CODE */
	newStack = (gkStack *) gkMalloc( sizeof( gkStack ) );
	ERR_IF( stack == NULL, GK_LIST_ERROR_MEMORY_ALLOCATION_FAILED );

	newStack -> count = 0;
	newStack -> next = NULL;
	newStack -> l = (gkList **) gkCalloc( LIST_STACK_NODE_SIZE, sizeof( gkList * ) );
	ERR_IF( newStack -> l == NULL, GK_LIST_ERROR_MEMORY_ALLOCATION_FAILED );

	/* give back */
	(*stack) = newStack;
	newStack = NULL;

	return GK_LIST_SUCCESS;


	/* CLEANUP */
	cleanup:

	gkFree( newStack );

	return rc;
	
}

/******************************************************************************/
inline int gkStackAddList( gkStack *stack, gkList *l )
{
	/* DATA */
	int rc = GK_LIST_SUCCESS;

	int i = 0;

	gkStack *newStackNode = NULL;
	

	/* CODE */
	while ( 1 )
	{
		i = 0;
		while ( i < stack -> count )
		{
			if ( stack -> l[ i ] == l )
			{
				return GK_LIST_SUCCESS;
			}

			i += 1;
		}

		if ( stack -> next == NULL )
		{
			break;
		}

		stack = stack -> next;
	}

	/* if we get here, we've hit the end of the stack, so we need to add
	   the list to the stack */
	if ( i == LIST_STACK_NODE_SIZE )
	{
		/* no room in this node, so we must create another node */
		newStackNode = (gkStack *) gkMalloc( sizeof( gkStack ) );
		ERR_IF( newStackNode == NULL, GK_LIST_ERROR_MEMORY_ALLOCATION_FAILED );

		newStackNode -> count = 0;
		newStackNode -> next = NULL;
		newStackNode -> l = (gkList **) gkCalloc( LIST_STACK_NODE_SIZE, sizeof( gkList * ) );
		ERR_IF( stack -> l == NULL, GK_LIST_ERROR_MEMORY_ALLOCATION_FAILED );

		stack -> next = newStackNode;
		newStackNode = NULL;

		stack = stack -> next;
		i = 0;
	}

	/* add list to new node */
	stack -> l[ i ] = l;
	stack -> count += 1;

	return GK_LIST_SUCCESS;


	/* CLEANUP */
	cleanup:

	gkFree( newStackNode );

	return rc;
}

/******************************************************************************/
inline int gkStackFree( gkStack **stack )
{
	/* DATA */
	gkStack *stackNode = NULL;

	/* CODE */
	if ( stack == NULL || (*stack) == NULL )
	{
		return GK_LIST_SUCCESS;
	}

	while ( (*stack) != NULL )
	{
		stackNode = (*stack);
		(*stack) = (*stack) -> next;

		gkFree( stackNode -> l );
		gkFree( stackNode );
	}

	(*stack) = NULL;
	
	return GK_LIST_SUCCESS;
}

