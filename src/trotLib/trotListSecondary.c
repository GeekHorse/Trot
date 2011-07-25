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
	TODO
*/

/******************************************************************************/
#include "trotCommon.h"
#include "trotList.h"
#include "trotListInternal.h"
#include "trotStack.h"

/******************************************************************************/
/*!
	\brief Compares two lists.
	\param lr Pointer to a trotListRef.
	\param lrCompareTo Pointer to a trotListRef that you want to compare the
		first one to.
	\param compareResult On success, the result of the comparison.
	\return 0 on success, <0 on error
*/
int trotListRefCompare( trotListRef *lr, trotListRef *lrCompareTo, TROT_LIST_COMPARE_RESULT *compareResult )
{
	/* DATA */
	int rc = TROT_LIST_SUCCESS;

	trotStack *stack = NULL;
	int stackEmpty = 0;

	trotList *l1 = NULL;
	INT_TYPE count1 = 0;
	INT_TYPE n1 = 0;
	int kind1 = NODE_KIND_HEAD_OR_TAIL;
	trotList *subL1 = NULL;

	trotList *l2 = NULL;
	INT_TYPE count2 = 0;
	INT_TYPE n2 = 0;
	int kind2 = NODE_KIND_HEAD_OR_TAIL;
	trotList *subL2 = NULL;

	INT_TYPE index = 0;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );
	PRECOND_ERR_IF( lrCompareTo == NULL );
	PRECOND_ERR_IF( compareResult == NULL );


	/* CODE */
	/* we assume equal, and set this otherwise later */
	(*compareResult) = TROT_LIST_COMPARE_EQUAL;

	/* no need to test if they point to same list */
	if ( lr -> lPointsTo == lrCompareTo -> lPointsTo )
	{
		return TROT_LIST_SUCCESS;
	}

	/* init stack */
	rc = trotStackInit( &stack );
	ERR_IF( rc != 0, rc );

	rc = trotStackPush( stack, lr -> lPointsTo, lrCompareTo -> lPointsTo );
	ERR_IF( rc != 0, rc );

	/* compare loop */
	while ( 1 )
	{
		/* increment top of stack */
		rc = trotStackIncrementTopN( stack );
		ERR_IF( rc != 0, rc );

		/* get both stack info */
		rc = trotStackGet( stack, &l1, &l2, &index );
		ERR_IF( rc != 0, rc );

		/* make sure we're in index */
		count1 = l1 -> childrenCount;
		count2 = l2 -> childrenCount;

		/* if both are too big */
		if ( index > count1 && index > count2 )
		{
			rc = trotStackPop( stack, &stackEmpty );
			ERR_IF( rc != 0, rc );

			if ( stackEmpty )
			{
				break;
			}

			continue;
		}

		/* if index1 is too big and index2 is ok */
		if ( index > count1 && index <= count2 )
		{
			(*compareResult) = TROT_LIST_COMPARE_LESS_THAN;
			break;
		}

		/* if n1 is ok and n2 is too big */
		if ( index <= count1 && index > count2 )
		{
			(*compareResult) = TROT_LIST_COMPARE_GREATER_THAN;
			break;
		}

		/* get kinds */
		rc = trotListGetKind( l1, index, &kind1 );
		ERR_IF( rc != 0, rc );
		rc = trotListGetKind( l2, index, &kind2 );
		ERR_IF( rc != 0, rc );

		/* compare kinds */
		/* ints are considered smaller than lists */
		if ( kind1 == NODE_KIND_INT && kind2 == NODE_KIND_LIST )
		{
			(*compareResult) = TROT_LIST_COMPARE_LESS_THAN;
			break;
		}
		if ( kind1 == NODE_KIND_LIST && kind2 == NODE_KIND_INT )
		{
			(*compareResult) = TROT_LIST_COMPARE_GREATER_THAN;
			break;
		}

		/* get and compare ints */
		if ( kind1 == NODE_KIND_INT && kind2 == NODE_KIND_INT )
		{
			rc = trotListGetInt( l1, index, &n1 );
			ERR_IF( rc != 0, rc );
			rc = trotListGetInt( l2, index, &n2 );
			ERR_IF( rc != 0, rc );

			if ( n1 < n2 )
			{
				(*compareResult) = TROT_LIST_COMPARE_LESS_THAN;
				break;
			}
			else if ( n1 > n2 )
			{
				(*compareResult) = TROT_LIST_COMPARE_GREATER_THAN;
				break;
			}

			continue;
		}

		/* TODO: put in a "should never happen" MACRO HERE */

		/* get lists */
		rc = trotListGetList( l1, index, &subL1 );
		ERR_IF( rc != 0, rc );
		rc = trotListGetList( l2, index, &subL2 );
		ERR_IF( rc != 0, rc );

		/* only add if different.
		   if they point to same, there's no need to compare */
		if ( subL1 != subL2 )
		{
			rc = trotStackPush( stack, subL1, subL2 );
			ERR_IF( rc != 0, rc );
		}
	}


	/* CLEANUP */
	cleanup:

	trotStackFree( &stack );

	return rc;
}

/******************************************************************************/
/*!
	\brief Makes a copy of a list.
	\param lr Pointer to a trotListRef that you want to copy.
	\param lrCopy_A Pointer to a trotListRef pointer that must be NULL.
		On success, this will be a new copy of lr.
	\return 0 on success, <0 on error
*/
int trotListRefCopy( trotListRef *lr, trotListRef **lrCopy_A )
{
	/* DATA */
	int rc = TROT_LIST_SUCCESS;

	trotListRef *newListRef = NULL;

	trotListNode *node = NULL;
	trotListNode *tail = NULL;

	int i = 0;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );
	PRECOND_ERR_IF( lrCopy_A == NULL );
	PRECOND_ERR_IF( (*lrCopy_A) != NULL );


	/* CODE */
	/* make our new list */
	rc = trotListRefInit( &newListRef );
	ERR_IF( rc != 0, rc );

	/* copy */
	tail = lr -> lPointsTo -> tail;
	node = lr -> lPointsTo -> head -> next;
	while ( node != tail )
	{
		if ( node -> kind == NODE_KIND_INT )
		{
			i = 0;
			while ( i < node -> count )
			{
				rc = trotListRefAppendInt( newListRef, node -> n[ i ] );
				ERR_IF( rc != 0, rc );

				i += 1;
			}
		}
		else if ( node -> kind == NODE_KIND_LIST )
		{
			i = 0;
			while ( i < node -> count )
			{
				rc = trotListRefAppendListTwin( newListRef, node -> l[ i ] );
				ERR_IF( rc != 0, rc );

				i += 1;
			}
		}
		else
		{
			ERR_IF( 1, TROT_LIST_ERROR_GENERAL );
		}

		node = node -> next;
	}


	/* give back */
	(*lrCopy_A) = newListRef;
	newListRef = NULL;

	return 0;


	/* CLEANUP */
	cleanup:

	trotListRefFree( &newListRef );

	return rc;
}

