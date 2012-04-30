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
	Implements the integer math of "Hoof", our single data
	structure for Trot.
*/

/******************************************************************************/
#include "trot.h"
#include "trotInternal.h"

/******************************************************************************/
/*!
	\brief Does an int operand on the last single or last two values in the
	       list.
	\param lr Pointer to a trotListRef.
	\param op Which operand to do.
	\return TROT_RC
*/
TROT_RC trotListIntOperand( trotListRef *lr, TROT_OP op )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotList *l = NULL;
	trotListNode *node = NULL;
	INT_TYPE value = 0;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );


	/* CODE */
	l = lr -> lPointsTo;

	node = l -> tail -> prev;

	/* check that last value in list is an int */
	ERR_IF( node -> kind != NODE_KIND_INT, TROT_LIST_ERROR_WRONG_KIND );

	/* handle single value ops */
	if ( op == TROT_OP_NEG )
	{
			/* TODO: should we check if the int is the most-negative int?
			         because if so, it will still be negative. Should that be an error? */
			node -> n[ (node -> count) - 1 ] = (-(node -> n[ (node -> count) - 1 ] ) );
			return TROT_LIST_SUCCESS;
	}

	if ( op == TROT_OP_LOGICAL_NOT )
	{
			node -> n[ (node -> count) - 1 ] = ! ( node -> n[ (node -> count) - 1 ] );
			return TROT_LIST_SUCCESS;
	}

	/* check that second-to-last value in list is an int */
	/* We technically don't have to do this here, but since we're
	   going to remove an int, I would rather leave the list
	   untouched on error. So let's catch the error here. */
	if ( node -> count <= 1 ) 
	{
		ERR_IF( node -> prev -> kind != NODE_KIND_INT, TROT_LIST_ERROR_WRONG_KIND );
	}

	/* remove last int */
	value = node -> n[ (node -> count) - 1 ];
	node -> count -= 1;
	l -> childrenCount -= 1;

	if ( node -> count == 0 )
	{
		node -> prev -> next = node -> next;
		node -> next -> prev = node -> prev;

		trotFree( node -> n );
		trotFree( node );
	}

	/* now we can call trotListIntOperandValue */
	rc = trotListIntOperandValue( lr, op, value );
	return rc;


	/* CLEANUP */
	cleanup:

	return rc;
}


/******************************************************************************/
/*!
	\brief Does an int operand on the last value in the list and the passed in value.
	\param lr Pointer to a trotListRef.
	\param op Which operand to do.
	\param value Value to use with the last value in the list.
	\return TROT_RC
*/
TROT_RC trotListIntOperandValue( trotListRef *lr, TROT_OP op, INT_TYPE value )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotList *l = NULL;
	trotListNode *node = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );


	/* CODE */
	l = lr -> lPointsTo;

	node = l -> tail -> prev;

	ERR_IF( node -> kind != NODE_KIND_INT, TROT_LIST_ERROR_WRONG_KIND );

	switch ( op )
	{
		case TROT_OP_ADD:
			node -> n[ (node -> count) - 1 ] += value;
			break;
		case TROT_OP_SUB:
			node -> n[ (node -> count) - 1 ] -= value;
			break;
		case TROT_OP_MUL:
			node -> n[ (node -> count) - 1 ] *= value;
			break;
		case TROT_OP_DIV:
			ERR_IF( value == 0, TROT_LIST_ERROR_DIVIDE_BY_ZERO );
			node -> n[ (node -> count) - 1 ] /= value;
			break;
		case TROT_OP_MOD:
			ERR_IF( value == 0, TROT_LIST_ERROR_DIVIDE_BY_ZERO );
			node -> n[ (node -> count) - 1 ] %= value;
			break;

		case TROT_OP_LESS_THAN:
			node -> n[ (node -> count) - 1 ] = node -> n[ (node -> count) - 1 ] < value;
			break;
		case TROT_OP_GREATER_THAN:
			node -> n[ (node -> count) - 1 ] = node -> n[ (node -> count) - 1 ] > value;
			break;
		case TROT_OP_EQUALS:
			node -> n[ (node -> count) - 1 ] = node -> n[ (node -> count) - 1 ] == value;
			break;

		case TROT_OP_LOGICAL_AND:
			node -> n[ (node -> count) - 1 ] = node -> n[ (node -> count) - 1 ] && value;
			break;
		case TROT_OP_LOGICAL_OR:
			node -> n[ (node -> count) - 1 ] = node -> n[ (node -> count) - 1 ] || value;
			break;
		default:
			ERR_IF( 1, TROT_LIST_ERROR_INVALID_OP );
	}

	return TROT_LIST_SUCCESS;


	/* CLEANUP */
	cleanup:

	return rc;
}

