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
	Encodes a trot list into a textual format.
*/
#undef  TROT_FILE_NUMBER
#define TROT_FILE_NUMBER 6

/******************************************************************************/
#include "trot.h"
#include "trotInternal.h"

/******************************************************************************/
static TROT_RC appendLeftBracketAndTags( TrotProgram *program, TrotList *lCharacters, TrotList *l );
static TROT_RC appendAbsTwinLocation( TrotProgram *program, TrotList *lCharacters, TrotList *l );
static TROT_RC appendNumber( TrotProgram *program, TrotList *lCharacters, TROT_INT n );

/******************************************************************************/
/*!
	\brief Encodes a list into a list of characters.
	\param[in] program List that maintains memory limit
	\param[in] listToEncode The list to encode
	\param[out] lCharacters_A On success, the encoding.
	\return TROT_RC

	listToEncode is not modified.
	lCharacters_A is created, and caller is responsible for freeing.
*/
TROT_RC trotEncode( TrotProgram *program, TrotList *listToEncode, TrotList **lCharacters_A )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotList *newCharacters = NULL;

	TrotList *lParentStack = NULL;
	TROT_INT parentStackCount = 0;

	TrotList *lParentIndicesStack = NULL;

	TrotList *lCurrentList = NULL;

	TROT_INT type = 0;
	TROT_INT childrenCount = 0;
	TROT_INT index = 1;

	TROT_INT kind = 0;

	TROT_INT n = 0;

	TrotList *lChildList = NULL;


	/* PRECOND */
	FAILURE_POINT;
	PARANOID_ERR_IF( program == NULL );
	PARANOID_ERR_IF( listToEncode == NULL );
	PARANOID_ERR_IF( lCharacters_A == NULL );
	PARANOID_ERR_IF( (*lCharacters_A) != NULL );


	/* CODE */
	/* create "give back" list */
	rc = trotListInit( program, &newCharacters );
	ERR_IF_PASSTHROUGH;

	/* create parent stack, so we can "go up" */
	rc = trotListInit( program, &lParentStack );
	ERR_IF_PASSTHROUGH;

	rc = trotListInit( program, &lParentIndicesStack );
	ERR_IF_PASSTHROUGH;

	/* setup */
	rc = trotListTwin( program, listToEncode, &lCurrentList );
	ERR_IF_PASSTHROUGH;

	lCurrentList->laPointsTo->encodingChildNumber = -1;

	/* start our encoding */
	rc = appendLeftBracketAndTags( program, newCharacters, lCurrentList );
	ERR_IF_PASSTHROUGH;

	/* go through list */
	while ( 1 )
	{
		/* do we have a next child? */

		/* get count */
		rc = trotListGetCount( program, lCurrentList, &childrenCount );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		/* are we out of children? */
		if ( index > childrenCount )
		{
			/* append "]" */
			rc = trotListAppendInt( program, newCharacters, ']' );
			ERR_IF_PASSTHROUGH;

			/* append space */
			rc = trotListAppendInt( program, newCharacters, ' ' );
			ERR_IF_PASSTHROUGH;

			/* do we have a parent? */
			rc = trotListGetCount( program, lParentStack, &parentStackCount );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			if ( parentStackCount == 0 )
			{
				/* break, we're done */
				break;
			}

			/* go up to parent */
			trotListFree( program, &lCurrentList );
			rc = trotListRemoveList( program, lParentStack, -1, &lCurrentList );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			rc = trotListRemoveInt( program, lParentIndicesStack, -1, &index );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			/* increment */
			index += 1;

			/* continue */
			continue;
		}

		/* get kind */
		rc = trotListGetKind( program, lCurrentList, index, &kind );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		if ( kind == TROT_KIND_INT )
		{
			/* get type */
			rc = trotListGetType( program, lCurrentList, &type );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			/* get int */
			rc = trotListGetInt( program, lCurrentList, index, &n );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			/* append number */
			rc = appendNumber( program, newCharacters, n );
			ERR_IF_PASSTHROUGH;

			/* append space */
			rc = trotListAppendInt( program, newCharacters, ' ' );
			ERR_IF_PASSTHROUGH;
		}
		else /* kind == TROT_KIND_LIST */
		{
			PARANOID_ERR_IF( kind != TROT_KIND_LIST );

			/* get child list */
			trotListFree( program, &lChildList );
			rc = trotListGetList( program, lCurrentList, index, &lChildList );
			ERR_IF_PASSTHROUGH;

			PARANOID_ERR_IF( lChildList->laPointsTo == NULL );
			/* if we've already encoded this list, then append the reference location
			   Example: @.1.2.3
			*/
			if ( lChildList->laPointsTo->encodingChildNumber != 0 )
			{
				/* append reference location */
				rc = appendAbsTwinLocation( program, newCharacters, lChildList );
				ERR_IF_PASSTHROUGH;
			}
			/* else we havent encoded this list yet, so encode it normally */
			else
			{
				lChildList->laPointsTo->encodingChildNumber = index;
				lChildList->laPointsTo->encodingParent = lCurrentList->laPointsTo;

				rc = appendLeftBracketAndTags( program, newCharacters, lChildList );
				ERR_IF_PASSTHROUGH;

				/* push to parent stacks, setup vars */
				rc = trotListAppendList( program, lParentStack, lCurrentList );
				ERR_IF_PASSTHROUGH;

				rc = trotListAppendInt( program, lParentIndicesStack, index );
				ERR_IF_PASSTHROUGH;

				trotListFree( program, &lCurrentList );
				lCurrentList = lChildList;
				lChildList = NULL;

				index = 0;

				PARANOID_ERR_IF( lCurrentList->laPointsTo == NULL );
			}

		} /* end if (kind) */

		/* increment */
		index += 1;

	} /* end while(1) */

	/* go through tree again, resetting encodingChildNumber and encodingParent */

	/* create parent stack, so we can "go up" */
	trotListFree( program, &lParentStack );
	rc = trotListInit( program, &lParentStack );
	ERR_IF_PASSTHROUGH;

	trotListFree( program, &lParentIndicesStack );
	rc = trotListInit( program, &lParentIndicesStack );
	ERR_IF_PASSTHROUGH;

	/* setup */
	trotListFree( program, &lCurrentList );
	rc = trotListTwin( program, listToEncode, &lCurrentList );
	ERR_IF_PASSTHROUGH;

	index = 1;

	PARANOID_ERR_IF( lCurrentList->laPointsTo == NULL );

	/* go through list */
	while ( 1 )
	{
		/* do we have a next child? */

		/* get count */
		rc = trotListGetCount( program, lCurrentList, &childrenCount );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		/* are we out of children? */
		if ( index > childrenCount )
		{
			/* do we have a parent? */
			rc = trotListGetCount( program, lParentStack, &parentStackCount );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			if ( parentStackCount == 0 )
			{
				/* break, we're done */
				break;
			}

			/* go up to parent */
			trotListFree( program, &lCurrentList );
			rc = trotListRemoveList( program, lParentStack, -1, &lCurrentList );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			rc = trotListRemoveInt( program, lParentIndicesStack, -1, &index );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			/* increment */
			index += 1;

			/* continue */
			continue;
		}

		/* get kind */
		rc = trotListGetKind( program, lCurrentList, index, &kind );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		if ( kind == TROT_KIND_LIST )
		{
			/* get child list */
			trotListFree( program, &lChildList );
			rc = trotListGetList( program, lCurrentList, index, &lChildList );
			ERR_IF_PASSTHROUGH;

			PARANOID_ERR_IF( lChildList->laPointsTo == NULL );
			/* if this list has an encoding number */
			if ( lChildList->laPointsTo->encodingChildNumber != 0 )
			{
				/* reset encoding members */
				lChildList->laPointsTo->encodingChildNumber = 0;
				lChildList->laPointsTo->encodingParent = NULL;

				/* push to parent stacks, setup vars */
				rc = trotListAppendList( program, lParentStack, lCurrentList );
				ERR_IF_PASSTHROUGH;

				rc = trotListAppendInt( program, lParentIndicesStack, index );
				ERR_IF_PASSTHROUGH;

				trotListFree( program, &lCurrentList );
				lCurrentList = lChildList;
				lChildList = NULL;

				index = 0;

				PARANOID_ERR_IF( lCurrentList->laPointsTo == NULL );
			}

		} /* end if ( kind == TROT_KIND_LIST ) */

		/* increment */
		index += 1;

	} /* end while ( 1 ) */

	listToEncode->laPointsTo->encodingChildNumber = 0;


	/* give back */
	(*lCharacters_A) = newCharacters;
	newCharacters = NULL;


	/* CLEANUP */
	cleanup:

	trotListFree( program, &newCharacters );
	trotListFree( program, &lParentStack );
	trotListFree( program, &lParentIndicesStack );
	trotListFree( program, &lCurrentList );
	trotListFree( program, &lChildList );

	return rc;
}

/******************************************************************************/
/*!
	\brief Append encoding of left bracket and it's tags.
	\param[in] program List that maintains memory limit
	\param[in] lCharacters List of characters to append to.
	\param[in] l List we're appending the encoding of. We need this to get the
		tags.
	\return TROT_RC

	lCharacters will have encoding text appended to it.
	l will not be modified.

	Example:
	If l had a type of 1 and a tag of 55, then we would append to lCharacters
	this:
	"[ ~1 `55 "
*/
static TROT_RC appendLeftBracketAndTags( TrotProgram *program, TrotList *lCharacters, TrotList *l )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TROT_INT type = 0;
	TROT_INT tag = 0;


	/* PRECOND */
	PARANOID_ERR_IF( program == NULL );
	PARANOID_ERR_IF( lCharacters == NULL );
	PARANOID_ERR_IF( l == NULL );


	/* CODE */
	/* append "[ " */
	rc = trotListAppendInt( program, lCharacters, '[' );
	ERR_IF_PASSTHROUGH;
	rc = trotListAppendInt( program, lCharacters, ' ' );
	ERR_IF_PASSTHROUGH;

	/* append type */
	type = l->laPointsTo->type;

	if ( type != 0 )
	{
		rc = trotListAppendInt( program, lCharacters, '~' );
		ERR_IF_PASSTHROUGH;
		rc = appendNumber( program, lCharacters, type );
		ERR_IF_PASSTHROUGH;

		rc = trotListAppendInt( program, lCharacters, ' ' );
		ERR_IF_PASSTHROUGH;
	}

	/* append tag */
	tag = l->laPointsTo->tag;

	if ( tag != 0 )
	{
		rc = trotListAppendInt( program, lCharacters, '`' );
		ERR_IF_PASSTHROUGH;
		rc = appendNumber( program, lCharacters, tag );
		ERR_IF_PASSTHROUGH;

		rc = trotListAppendInt( program, lCharacters, ' ' );
		ERR_IF_PASSTHROUGH;
	}

	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Appends encoding of a textual-reference.
	\param[in] program List that maintains memory limit
	\param[in] lCharacters List of characters to append to.
	\param[in] l List we're appending the textual-reference encoding of.
	\return TROT_RC

	lCharacters will have the encoding text appended to it.
	l will not be modified.
*/
static TROT_RC appendAbsTwinLocation( TrotProgram *program, TrotList *lCharacters, TrotList *l )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotList *lAddress = NULL;
	TROT_INT address = 0;

	TROT_INT index = 0;
	TROT_INT count = 0;

	TrotListActual *laParent = NULL;


	/* PRECOND */
	PARANOID_ERR_IF( program == NULL );
	PARANOID_ERR_IF( lCharacters == NULL );
	PARANOID_ERR_IF( l == NULL );


	/* CODE */
	/* append "@" */
	rc = trotListAppendInt( program, lCharacters, '@' );
	ERR_IF_PASSTHROUGH;

	/* if encodingChildNumber is -1, just need "@" */
	if ( l->laPointsTo->encodingChildNumber == -1 )
	{
		/* append space */
		rc = trotListAppendInt( program, lCharacters, ' ' );
		ERR_IF_PASSTHROUGH;

		goto cleanup;
	}

	/* create lAddress */
	rc = trotListInit( program, &lAddress );
	ERR_IF_PASSTHROUGH;

	laParent = l->laPointsTo;
	while ( laParent->encodingChildNumber > 0 )
	{
		rc = trotListInsertInt( program, lAddress, 1, laParent->encodingChildNumber );
		ERR_IF_PASSTHROUGH;

		laParent = laParent->encodingParent;
		PARANOID_ERR_IF( laParent == NULL );
	}

	/* get count */
	rc = trotListGetCount( program, lAddress, &count );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* foreach parent */
	index = 1;
	while ( index <= count )
	{
		/* get parent */
		rc = trotListGetInt( program, lAddress, index, &address );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		/* append '.' */
		rc = trotListAppendInt( program, lCharacters, '.' );
		ERR_IF_PASSTHROUGH;

		/* append number */
		rc = appendNumber( program, lCharacters, address );
		ERR_IF_PASSTHROUGH;

		/* increment */
		index += 1;
	}

	/* append space */
	rc = trotListAppendInt( program, lCharacters, ' ' );
	ERR_IF_PASSTHROUGH;


	/* CLEANUP */
	cleanup:

	trotListFree( program, &lAddress );

	return rc;
}

/******************************************************************************/
/*!
	\brief Appends encoding of a number.
	\param[in] program List that maintains memory limit
	\param[in] lCharacters List of characters to append to.
	\param[in] n Number to append.
	\return TROT_RC

	lCharacters will have encoding text appended to it.
*/
static TROT_RC appendNumber( TrotProgram *program, TrotList *lCharacters, TROT_INT n )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TROT_INT numberString[ TROT_INT_MIN_STRING_LENGTH + 1 ];
	TROT_INT *s = NULL;


	/* PRECOND */
	PARANOID_ERR_IF( program == NULL );
	PARANOID_ERR_IF( lCharacters == NULL );


	/* CODE */
	/* special case: 0 */
	if ( n == 0 )
	{
		rc = trotListAppendInt( program, lCharacters, '0' );
		ERR_IF_PASSTHROUGH;

		goto cleanup;
	}

	/* create numberString */
	s = &(numberString[ TROT_INT_MIN_STRING_LENGTH ]);
	(*s) = '\0';

	/* create number string */
	if ( n < 0 )
	{
		while ( n != 0 )
		{
			s -= 1;
			(*s) =  '0' - ( n % 10 );

			n /= 10;
		}

		s -= 1;
		(*s) = '-';
	}
	else
	{
		while ( n != 0 )
		{
			s -= 1;
			(*s) =  '0' + ( n % 10 );

			n /= 10;
		}
	}

	/* append numberString to lCharacters */
	while ( (*s) != '\0' )
	{
		rc = trotListAppendInt( program, lCharacters, (*s) );
		ERR_IF_PASSTHROUGH;

		s += 1;
	}


	/* CLEANUP */
	cleanup:

	return rc;
}

