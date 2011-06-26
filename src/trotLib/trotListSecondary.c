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
	trotStack *stackCompareTo = NULL;

	trotListRef *l1 = NULL;
	INT_TYPE index1 = 0;
	INT_TYPE count1 = 0;
	INT_TYPE n1 = 0;
	trotListRef *subL1 = NULL;

	trotListRef *l2 = NULL;
	INT_TYPE index2 = 0;
	INT_TYPE count2 = 0;
	INT_TYPE n2 = 0;
	trotListRef *subL2 = NULL;


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
		return 0;
	}

	/* init stacks */
	rc = trotStackInit( &stack );
	ERR_IF( rc != 0, rc );
	rc = trotStackInit( &stackCompareTo );
	ERR_IF( rc != 0, rc );

	rc = trotStackPush( stack, lr );
	ERR_IF( rc != 0, rc );
	rc = trotStackPush( stackCompareTo, lrCompareTo );
	ERR_IF( rc != 0, rc );

	/* compare loop */
	while ( 1 )
	{
		/* increment top of stack */
		rc = trotStackIncrementTopN( stack );
		ERR_IF( rc != 0, rc );
		rc = trotStackIncrementTopN( stackCompareTo );
		ERR_IF( rc != 0, rc );

		/* get both stack info */
		rc = trotStackGet( stack, &l1, &index1 );
		ERR_IF( rc != 0, rc );
		rc = trotStackGet( stackCompareTo, &l2, &index2 );
		ERR_IF( rc != 0, rc );

		/* make sure we're in index */
		rc = trotListRefGetCount( l1, &count1 );
		ERR_IF( rc != 0, rc );
		rc = trotListRefGetCount( l2, &count2 );
		ERR_IF( rc != 0, rc );

		/* if both are too big */
		if ( index1 > count1 && index2 > count2 )
		{
			rc = trotStackPop( stack );
			ERR_IF( rc != 0, rc );
			rc = trotStackPop( stackCompareTo );
			ERR_IF( rc != 0, rc );

			continue;
		}
		/* if index1 is too big and index2 is ok */
		if ( index1 > count1 && index2 <= count2 )
		{
			(*compareResult) = TROT_LIST_COMPARE_LESS_THAN;
			break;
		}
		/* if n1 is ok and n2 is too big */
		if ( index1 <= count1 && index2 > count2 )
		{
			(*compareResult) = TROT_LIST_COMPARE_GREATER_THAN;
			break;
		}

		/* get kinds */
		rc = trotListRefGetKind( l1, index1, &kind1 );
		ERR_IF( rc != 0, rc );
		rc = trotListRefGetKind( l2, index2, &kind2 );
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
		if ( kind1 == NODE_KIND_INT && kind2 == NODE_KINT_INT )
		{
			rc = trotListRefGetInt( l1, index1, &n1 );
			ERR_IF( rc != 0, rc );
			rc = trotListRefGetInt( l2, index2, &n2 );
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

		/* get lists */
		rc = trotListRefGetListTwin( l1, index1, &subL1 );
		ERR_IF( rc != 0, rc );
		rc = trotListRefGetListTwin( l2, index2, &subL2 );
		ERR_IF( rc != 0, rc );

		/* only add if different.
		   if they point to same, there's no need to compare */
		if ( subL1 -> lPointTo != subL2 -> lPointTo )
		{
			rc = trotStackPush( stack, subL1 );
			ERR_IF( rc != 0, rc );
			rc = trotStackPush( stackCompareTo, subL2 );
			ERR_IF( rc != 0, rc );
		}

		/* free our refs */
		rc = trotListRefFree( &subL1 );
		ERR_IF( rc != 0, rc );
		rc = trotListRefFree( &subL2 );
		ERR_IF( rc != 0, rc );
	}


	return 0;


	/* CLEANUP */
	cleanup:

	return rc;
}

