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
	Implements the integer math of "Hoof", our single data
	structure for Trot.
*/

/******************************************************************************/
#include "trotCommon.h"
#include "trotList.h"
#include "trotListInternal.h"
#include "trotMem.h"

/******************************************************************************/
/*!
	\brief Does an int operand on the last single or last two values in the
	       list.
	\param lr Pointer to a trotListRef.
	\param op Which operand to do.
	\return TROT_RC
*/
TROT_RC trotListIntOperand( trotListRef *lr, TROT_INT_OPERAND op )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotList *l = NULL;
	trotListNode *node = NULL;
	INT_TYPE value = 0;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );
	PRECOND_ERR_IF( op < TROT_INT_OPERAND_MIN );
	PRECOND_ERR_IF( op > TROT_INT_OPERAND_MAX );


	/* CODE */
	l = lr -> lPointsTo;

	node = l -> tail -> prev;

	/* check that last value in list is an int */
	ERR_IF( node -> kind != NODE_KIND_INT, TROT_LIST_ERROR_WRONG_KIND );

	/* if not NEG or NOT, we need to remove an int */
	if ( op != TROT_INT_OPERAND_NEG && op != TROT_INT_OPERAND_LOGICAL_NOT )
	{
		/* check that second-to-last value in list is an int */
		/* We technically don't have to do this here, but since we're
		   going to remove an int, I would rather leave the list
		   untouched on error. So let's catch the error here. */
		if ( node -> count < 1 ) 
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
	}

	/* now we can call trotListIntOperandValue */
	rc = trotListIntOperandValue( lr, op, value );
	ERR_IF_PASSTHROUGH;

	return TROT_LIST_SUCCESS;


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
TROT_RC trotListIntOperandValue( trotListRef *lr, TROT_INT_OPERAND op, INT_TYPE value )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotList *l = NULL;
	trotListNode *node = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );
	PRECOND_ERR_IF( op < TROT_INT_OPERAND_MIN );
	PRECOND_ERR_IF( op > TROT_INT_OPERAND_MAX );


	/* CODE */
	l = lr -> lPointsTo;

	node = l -> tail -> prev;

	ERR_IF( node -> kind != NODE_KIND_INT, TROT_LIST_ERROR_WRONG_KIND );

	switch ( op )
	{
		case TROT_INT_OPERAND_ADD:
			node -> n[ (node -> count) - 1 ] += value;
			break;
		case TROT_INT_OPERAND_SUB:
			node -> n[ (node -> count) - 1 ] -= value;
			break;
		case TROT_INT_OPERAND_MUL:
			node -> n[ (node -> count) - 1 ] *= value;
			break;
		case TROT_INT_OPERAND_DIV:
			ERR_IF( value == 0, TROT_LIST_ERROR_DIVIDE_BY_ZERO );
			node -> n[ (node -> count) - 1 ] /= value;
			break;
		case TROT_INT_OPERAND_MOD:
			ERR_IF( value == 0, TROT_LIST_ERROR_DIVIDE_BY_ZERO );
			node -> n[ (node -> count) - 1 ] %= value;
			break;
		case TROT_INT_OPERAND_NEG:
			/* NOTE: value not used */
			node -> n[ (node -> count) - 1 ] = (-(node -> n[ (node -> count) - 1 ] ) );
			break;

		case TROT_INT_OPERAND_LOGICAL_AND:
			node -> n[ (node -> count) - 1 ] = node -> n[ (node -> count) - 1 ] && value;
			break;
		case TROT_INT_OPERAND_LOGICAL_OR:
			node -> n[ (node -> count) - 1 ] = node -> n[ (node -> count) - 1 ] || value;
			break;
		case TROT_INT_OPERAND_LOGICAL_NOT:
			/* NOTE: value not used */
			node -> n[ (node -> count) - 1 ] = ! ( node -> n[ (node -> count) - 1 ] );
			break;
	}

	return TROT_LIST_SUCCESS;


	/* CLEANUP */
	cleanup:

	return rc;
}

