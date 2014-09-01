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
static TROT_RC appendLeftBracketAndTags( TrotList *lMemLimit, TrotList *lCharacters, TrotList *l );
static TROT_RC appendAbsTwinLocation( TrotList *lMemLimit, TrotList *lCharacters, TrotList *l );
static TROT_RC appendNumber( TrotList *lMemLimit, TrotList *lCharacters, TROT_INT n );
static TROT_RC appendText( TrotList *lMemLimit, TrotList *lCharacters, TrotList *l, TROT_INT count, TROT_INT *index );

/******************************************************************************/
/*!
	\brief Encodes a list into a list of characters.
	\param[in] lMemLimit List that maintains memory limit
	\param[in] listToEncode The list to encode
	\param[out] lCharacters_A On success, the encoding.
	\return TROT_RC

	listToEncode is not modified.
	lCharacters_A is created, and caller is responsible for freeing.
*/
TROT_RC trotEncode( TrotList *lMemLimit, TrotList *listToEncode, TrotList **lCharacters_A )
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
	ERR_IF( listToEncode == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( lCharacters_A == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( (*lCharacters_A) != NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	/* create "give back" list */
	rc = trotListInit( lMemLimit, &newCharacters );
	ERR_IF_PASSTHROUGH;

	/* create parent stack, so we can "go up" */
	rc = trotListInit( lMemLimit, &lParentStack );
	ERR_IF_PASSTHROUGH;

	rc = trotListInit( lMemLimit, &lParentIndicesStack );
	ERR_IF_PASSTHROUGH;

	/* setup */
	rc = trotListTwin( lMemLimit, listToEncode, &lCurrentList );
	ERR_IF_PASSTHROUGH;

	lCurrentList->laPointsTo->encodingChildNumber = -1;

	/* start our encoding */
	rc = appendLeftBracketAndTags( lMemLimit, newCharacters, lCurrentList );
	ERR_IF_PASSTHROUGH;

	/* go through list */
	while ( 1 )
	{
		/* do we have a next child? */

		/* get count */
		rc = trotListGetCount( lMemLimit, lCurrentList, &childrenCount );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		/* are we out of children? */
		if ( index > childrenCount )
		{
			/* append "]" */
			rc = trotListAppendInt( lMemLimit, newCharacters, ']' );
			ERR_IF_PASSTHROUGH;

			/* append space */
			rc = trotListAppendInt( lMemLimit, newCharacters, ' ' );
			ERR_IF_PASSTHROUGH;

			/* do we have a parent? */
			rc = trotListGetCount( lMemLimit, lParentStack, &parentStackCount );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			if ( parentStackCount == 0 )
			{
				/* break, we're done */
				break;
			}

			/* go up to parent */
			trotListFree( lMemLimit, &lCurrentList );
			rc = trotListRemoveList( lMemLimit, lParentStack, -1, &lCurrentList );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			rc = trotListRemoveInt( lMemLimit, lParentIndicesStack, -1, &index );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			/* increment */
			index += 1;

			/* continue */
			continue;
		}

		/* get kind */
		rc = trotListGetKind( lMemLimit, lCurrentList, index, &kind );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		if ( kind == TROT_KIND_INT )
		{
			/* get type */
			rc = trotListGetType( lMemLimit, lCurrentList, &type );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			/* if type text, append in text format */
			if ( type == TROT_TYPE_TEXT )
			{
				rc = appendText( lMemLimit, newCharacters, lCurrentList, childrenCount, &index );
				ERR_IF_PASSTHROUGH;
			}
			/* else append in number format */
			else
			{
				/* get int */
				rc = trotListGetInt( lMemLimit, lCurrentList, index, &n );
				PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

				/* append number */
				rc = appendNumber( lMemLimit, newCharacters, n );
				ERR_IF_PASSTHROUGH;

				/* append space */
				rc = trotListAppendInt( lMemLimit, newCharacters, ' ' );
				ERR_IF_PASSTHROUGH;
			}
		}
		else /* kind == TROT_KIND_LIST */
		{
			PARANOID_ERR_IF( kind != TROT_KIND_LIST );

			/* get child list */
			trotListFree( lMemLimit, &lChildList );
			rc = trotListGetList( lMemLimit, lCurrentList, index, &lChildList );
			ERR_IF_PASSTHROUGH;

			PARANOID_ERR_IF( lChildList->laPointsTo == NULL );
			/* if we've already encoded this list, then append the reference location
			   Example: @.1.2.3
			*/
			if ( lChildList->laPointsTo->encodingChildNumber != 0 )
			{
				/* append reference location */
				rc = appendAbsTwinLocation( lMemLimit, newCharacters, lChildList );
				ERR_IF_PASSTHROUGH;
			}
			/* else we havent encoded this list yet, so encode it normally */
			else
			{
				lChildList->laPointsTo->encodingChildNumber = index;
				lChildList->laPointsTo->encodingParent = lCurrentList->laPointsTo;

				rc = appendLeftBracketAndTags( lMemLimit, newCharacters, lChildList );
				ERR_IF_PASSTHROUGH;

				/* push to parent stacks, setup vars */
				rc = trotListAppendList( lMemLimit, lParentStack, lCurrentList );
				ERR_IF_PASSTHROUGH;

				rc = trotListAppendInt( lMemLimit, lParentIndicesStack, index );
				ERR_IF_PASSTHROUGH;

				trotListFree( lMemLimit, &lCurrentList );
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
	trotListFree( lMemLimit, &lParentStack );
	rc = trotListInit( lMemLimit, &lParentStack );
	ERR_IF_PASSTHROUGH;

	trotListFree( lMemLimit, &lParentIndicesStack );
	rc = trotListInit( lMemLimit, &lParentIndicesStack );
	ERR_IF_PASSTHROUGH;

	/* setup */
	trotListFree( lMemLimit, &lCurrentList );
	rc = trotListTwin( lMemLimit, listToEncode, &lCurrentList );
	ERR_IF_PASSTHROUGH;

	index = 1;

	PARANOID_ERR_IF( lCurrentList->laPointsTo == NULL );

	/* go through list */
	while ( 1 )
	{
		/* do we have a next child? */

		/* get count */
		rc = trotListGetCount( lMemLimit, lCurrentList, &childrenCount );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		/* are we out of children? */
		if ( index > childrenCount )
		{
			/* do we have a parent? */
			rc = trotListGetCount( lMemLimit, lParentStack, &parentStackCount );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			if ( parentStackCount == 0 )
			{
				/* break, we're done */
				break;
			}

			/* go up to parent */
			trotListFree( lMemLimit, &lCurrentList );
			rc = trotListRemoveList( lMemLimit, lParentStack, -1, &lCurrentList );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			rc = trotListRemoveInt( lMemLimit, lParentIndicesStack, -1, &index );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			/* increment */
			index += 1;

			/* continue */
			continue;
		}

		/* get kind */
		rc = trotListGetKind( lMemLimit, lCurrentList, index, &kind );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		if ( kind == TROT_KIND_LIST )
		{
			/* get child list */
			trotListFree( lMemLimit, &lChildList );
			rc = trotListGetList( lMemLimit, lCurrentList, index, &lChildList );
			ERR_IF_PASSTHROUGH;

			PARANOID_ERR_IF( lChildList->laPointsTo == NULL );
			/* if this list has an encoding number */
			if ( lChildList->laPointsTo->encodingChildNumber != 0 )
			{
				/* reset encoding members */
				lChildList->laPointsTo->encodingChildNumber = 0;
				lChildList->laPointsTo->encodingParent = NULL;

				/* push to parent stacks, setup vars */
				rc = trotListAppendList( lMemLimit, lParentStack, lCurrentList );
				ERR_IF_PASSTHROUGH;

				rc = trotListAppendInt( lMemLimit, lParentIndicesStack, index );
				ERR_IF_PASSTHROUGH;

				trotListFree( lMemLimit, &lCurrentList );
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

	trotListFree( lMemLimit, &newCharacters );
	trotListFree( lMemLimit, &lParentStack );
	trotListFree( lMemLimit, &lParentIndicesStack );
	trotListFree( lMemLimit, &lCurrentList );
	trotListFree( lMemLimit, &lChildList );

	return rc;
}

/******************************************************************************/
/*!
	\brief Append encoding of left bracket and it's tags.
	\param[in] lMemLimit List that maintains memory limit
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
static TROT_RC appendLeftBracketAndTags( TrotList *lMemLimit, TrotList *lCharacters, TrotList *l )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TROT_INT type = 0;
	TROT_INT tag = 0;


	/* PRECOND */
	PARANOID_ERR_IF( lMemLimit == NULL );
	PARANOID_ERR_IF( lCharacters == NULL );
	PARANOID_ERR_IF( l == NULL );


	/* CODE */
	/* append "[ " */
	rc = trotListAppendInt( lMemLimit, lCharacters, '[' );
	ERR_IF_PASSTHROUGH;
	rc = trotListAppendInt( lMemLimit, lCharacters, ' ' );
	ERR_IF_PASSTHROUGH;

	/* append type */
	type = l->laPointsTo->type;

	if ( type != 0 )
	{
		rc = trotListAppendInt( lMemLimit, lCharacters, '~' );
		ERR_IF_PASSTHROUGH;
		rc = appendNumber( lMemLimit, lCharacters, type );
		ERR_IF_PASSTHROUGH;

		rc = trotListAppendInt( lMemLimit, lCharacters, ' ' );
		ERR_IF_PASSTHROUGH;
	}

	/* append tag */
	tag = l->laPointsTo->tag;

	if ( tag != 0 )
	{
		rc = trotListAppendInt( lMemLimit, lCharacters, '`' );
		ERR_IF_PASSTHROUGH;
		rc = appendNumber( lMemLimit, lCharacters, tag );
		ERR_IF_PASSTHROUGH;

		rc = trotListAppendInt( lMemLimit, lCharacters, ' ' );
		ERR_IF_PASSTHROUGH;
	}

	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Appends encoding of a textual-reference.
	\param[in] lMemLimit List that maintains memory limit
	\param[in] lCharacters List of characters to append to.
	\param[in] l List we're appending the textual-reference encoding of.
	\return TROT_RC

	lCharacters will have the encoding text appended to it.
	l will not be modified.
*/
static TROT_RC appendAbsTwinLocation( TrotList *lMemLimit, TrotList *lCharacters, TrotList *l )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotList *lAddress = NULL;
	TROT_INT address = 0;

	TROT_INT index = 0;
	TROT_INT count = 0;

	TrotListActual *laParent = NULL;


	/* PRECOND */
	PARANOID_ERR_IF( lMemLimit == NULL );
	PARANOID_ERR_IF( lCharacters == NULL );
	PARANOID_ERR_IF( l == NULL );


	/* CODE */
	/* append "@" */
	rc = trotListAppendInt( lMemLimit, lCharacters, '@' );
	ERR_IF_PASSTHROUGH;

	/* if encodingChildNumber is -1, just need "@" */
	if ( l->laPointsTo->encodingChildNumber == -1 )
	{
		/* append space */
		rc = trotListAppendInt( lMemLimit, lCharacters, ' ' );
		ERR_IF_PASSTHROUGH;

		goto cleanup;
	}

	/* create lAddress */
	rc = trotListInit( lMemLimit, &lAddress );
	ERR_IF_PASSTHROUGH;

	laParent = l->laPointsTo;
	while ( laParent->encodingChildNumber > 0 )
	{
		rc = trotListInsertInt( lMemLimit, lAddress, 1, laParent->encodingChildNumber );
		ERR_IF_PASSTHROUGH;

		laParent = laParent->encodingParent;
		PARANOID_ERR_IF( laParent == NULL );
	}

	/* get count */
	rc = trotListGetCount( lMemLimit, lAddress, &count );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* foreach parent */
	index = 1;
	while ( index <= count )
	{
		/* get parent */
		rc = trotListGetInt( lMemLimit, lAddress, index, &address );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		/* append '.' */
		rc = trotListAppendInt( lMemLimit, lCharacters, '.' );
		ERR_IF_PASSTHROUGH;

		/* append number */
		rc = appendNumber( lMemLimit, lCharacters, address );
		ERR_IF_PASSTHROUGH;

		/* increment */
		index += 1;
	}

	/* append space */
	rc = trotListAppendInt( lMemLimit, lCharacters, ' ' );
	ERR_IF_PASSTHROUGH;


	/* CLEANUP */
	cleanup:

	trotListFree( lMemLimit, &lAddress );

	return rc;
}

/******************************************************************************/
/*!
	\brief Appends encoding of a number.
	\param[in] lMemLimit List that maintains memory limit
	\param[in] lCharacters List of characters to append to.
	\param[in] n Number to append.
	\return TROT_RC

	lCharacters will have encoding text appended to it.
*/
static TROT_RC appendNumber( TrotList *lMemLimit, TrotList *lCharacters, TROT_INT n )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TROT_INT numberString[ TROT_INT_MIN_STRING_LENGTH + 1 ];
	TROT_INT *s = NULL;


	/* PRECOND */
	PARANOID_ERR_IF( lMemLimit == NULL );
	PARANOID_ERR_IF( lCharacters == NULL );


	/* CODE */
	/* special case: 0 */
	if ( n == 0 )
	{
		rc = trotListAppendInt( lMemLimit, lCharacters, '0' );
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
		rc = trotListAppendInt( lMemLimit, lCharacters, (*s) );
		ERR_IF_PASSTHROUGH;

		s += 1;
	}


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Appends ints in text format
	\param[in] lMemLimit List that maintains memory limit
	\param[in] lCharacters List of characters to append to.
	\param[in] l The list we're encoding
	\param[in] count Count of l
	\param[in] index Current index of l
	\return TROT_RC

	lCharacters will have encoding text appended to it.
	index will be incremented
*/
static TROT_RC appendText( TrotList *lMemLimit, TrotList *lCharacters, TrotList *l, TROT_INT count, TROT_INT *index )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TROT_INT kind = 0;
	TROT_INT character = 0;
	TROT_INT state = 0;


	/* PRECOND */
	PARANOID_ERR_IF( lMemLimit == NULL );
	PARANOID_ERR_IF( lCharacters == NULL );
	PARANOID_ERR_IF( l == NULL );
	PARANOID_ERR_IF( index == NULL );


	/* CODE */
	/* go through ints, break if we hit a list */
	while ( (*index) <= count )
	{
		/* get kind */
		rc = trotListGetKind( lMemLimit, l, (*index), &kind );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		if ( kind != TROT_KIND_INT )
		{
			/* go back */
			(*index) -= 1;

			break;
		}

		/* get character */
		rc = trotListGetInt( lMemLimit, l, (*index), &character );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		/* is printable character? */
		/* not fully unicode correct, but good enough for now */
		/* FUTURE IMPROVEMENT */
		if (    character != '\"' 
		     && ( character >= ' ' && character <= '~' )
		   )
		{
			if ( state == 0 )
			{
				/* append " */
				rc = trotListAppendInt( lMemLimit, lCharacters, '\"' );
				ERR_IF_PASSTHROUGH;

				state = 1;
			}

			/* append character */
			rc = trotListAppendInt( lMemLimit, lCharacters, character );
			ERR_IF_PASSTHROUGH;
		}
		else
		{
			if ( state == 1 )
			{
				/* append " */
				rc = trotListAppendInt( lMemLimit, lCharacters, '\"' );
				ERR_IF_PASSTHROUGH;

				/* append space */
				rc = trotListAppendInt( lMemLimit, lCharacters, ' ' );
				ERR_IF_PASSTHROUGH;

				state = 0;
			}

			/* append in number format */
			rc = appendNumber( lMemLimit, lCharacters, character );
			ERR_IF_PASSTHROUGH;

			/* append space */
			rc = trotListAppendInt( lMemLimit, lCharacters, ' ' );
			ERR_IF_PASSTHROUGH;
		}

		/* increment */
		(*index) += 1;
	}

	if ( state == 1 )
	{
		/* append " */
		rc = trotListAppendInt( lMemLimit, lCharacters, '\"' );
		ERR_IF_PASSTHROUGH;

		/* append space */
		rc = trotListAppendInt( lMemLimit, lCharacters, ' ' );
		ERR_IF_PASSTHROUGH;
	}


	/* CLEANUP */
	cleanup:

	return rc;
}

