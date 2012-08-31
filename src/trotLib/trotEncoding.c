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
	Encodes trot lists to a textual format.
*/

/******************************************************************************/
#define ENCODING_MAX_CHARACTER_COUNT 80

/******************************************************************************/
#include "trot.h"
#include "trotInternal.h"

/******************************************************************************/
extern const char *opNames[];

/******************************************************************************/
static TROT_RC appendLeftBAndTag( trotListRef *lr, int topList, trotListRef *lrCharacters, unsigned int *indentCount );
static TROT_RC appendRightB( trotListRef *lr, trotListRef *lrCharacters, unsigned int *indentCount );
static TROT_RC appendNewlineIndents( trotListRef *lrCharacters, unsigned int indentCount );
static TROT_RC appendNumber( trotListRef *lrCharacters, int *characterCount, INT_TYPE n );
static TROT_RC appendAbsTwinLocation( trotListRef *lrCharacters, int *characterCount, trotListRef *lr );

/******************************************************************************/
/*!
	\brief Encodes a list into a list of characters.
	\param 
	\return TROT_RC
*/
TROT_RC trotEncode( trotListRef *lr, trotListRef **lrCharacters_A )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	trotListRef *newLrCharacters = NULL;
	INT_TYPE lastCharacter = 0;

	trotListRef *lrParentStack = NULL;
	INT_TYPE parentStackCount = 0;

	trotListRef *lrParentIndicesStack = NULL;

	TROT_TAG currentTag = TROT_TAG_DATA;
	trotListRef *lrCurrentList = NULL;
	INT_TYPE childrenCount = 0;
	INT_TYPE index = 0;

	unsigned int indentCount = 0;

	TROT_KIND kind = 0;

	int characterCount = 0;

	INT_TYPE n = 0;

	trotListRef *lrChildList = NULL;

	const char *s = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );
	PRECOND_ERR_IF( lrCharacters_A == NULL );
	PRECOND_ERR_IF( (*lrCharacters_A) != NULL );


	/* CODE */
	/* create "give back" list */
	rc = trotListRefInit( &newLrCharacters );
	ERR_IF_PASSTHROUGH;

	/* create parent stack, so we can "go up" */
	rc = trotListRefInit( &lrParentStack );
	ERR_IF_PASSTHROUGH;

	rc = trotListRefInit( &lrParentIndicesStack );
	ERR_IF_PASSTHROUGH;

	/* setup */
	rc = trotListRefTwin( lr, &lrCurrentList );
	ERR_IF_PASSTHROUGH;

	index = 0;

	PARANOID_ERR_IF( lrCurrentList -> lPointsTo == NULL );
	currentTag = lrCurrentList -> lPointsTo -> tag;

	lrCurrentList -> lPointsTo -> encodingChildNumber = -1;

	/* start our encoding */
	rc = appendLeftBAndTag( lrCurrentList, 1, newLrCharacters, &indentCount );
	ERR_IF_PASSTHROUGH;

	/* go through list */
	while ( 1 )
	{
		/* do we have a next child? */

		/* get count */
		rc = trotListRefGetCount( lrCurrentList, &childrenCount );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		/* are we out of children? */
		if ( index == childrenCount )
		{
			/* appendRightB */
			rc = appendRightB( lrCurrentList, newLrCharacters, &indentCount );
			ERR_IF_PASSTHROUGH;

			/* do we have a parent? */
			rc = trotListRefGetCount( lrParentStack, &parentStackCount );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			if ( parentStackCount == 0 )
			{
				/* break, we're done */
				break;
			}

			/* go up to parent */
			trotListRefFree( &lrCurrentList );
			rc = trotListRefRemoveList( lrParentStack, -1, &lrCurrentList );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			rc = trotListRefRemoveInt( lrParentIndicesStack, -1, &index );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			PARANOID_ERR_IF( lrCurrentList -> lPointsTo == NULL );
			currentTag = lrCurrentList -> lPointsTo -> tag;

			/* continue */
			continue;
		}

		/* increment */
		index += 1;

		if ( characterCount > ENCODING_MAX_CHARACTER_COUNT )
		{
			rc = appendNewlineIndents( newLrCharacters, indentCount );
			ERR_IF_PASSTHROUGH;

			characterCount = 0;
		}

		switch ( currentTag )
		{
			case TROT_TAG_CODE:
			case TROT_TAG_FUNCTION:
			case TROT_TAG_RAW_CODE:

				rc = trotListRefGetKind( lrCurrentList, index, &kind );
				PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

				ERR_IF( kind != TROT_KIND_INT, TROT_RC_ERROR_ENCODE );

				rc = trotListRefGetInt( lrCurrentList, index, &n );
				PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

				ERR_IF( n < TROT_OP_MIN || n > TROT_OP_MAX, TROT_RC_ERROR_ENCODE );

				/* do we need to append a space? */
				rc = trotListRefGetInt( newLrCharacters, -1, &lastCharacter );
				PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

				if ( lastCharacter != '\t' )
				{
					rc = trotListRefAppendInt( newLrCharacters, ' ' );
					ERR_IF_PASSTHROUGH;

					characterCount += 1;
				}

				s = opNames[ n - 1 ];
				while ( (*s) != '\0' )
				{
					rc = trotListRefAppendInt( newLrCharacters, (*s) );
					ERR_IF_PASSTHROUGH;

					characterCount += 1;
					s += 1;
				}

				if (    n == TROT_OP_PUSH_INT
				     || n == TROT_OP_LOAD_VAR
				     || n == TROT_OP_SAVE_VAR
				   )
				{
					index += 1;

					rc = trotListRefGetKind( lrCurrentList, index, &kind );
					ERR_IF( rc != TROT_RC_SUCCESS, TROT_RC_ERROR_ENCODE );

					ERR_IF( kind != TROT_KIND_INT, TROT_RC_ERROR_ENCODE );

					rc = trotListRefGetInt( lrCurrentList, index, &n );
					PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

					rc = appendNumber( newLrCharacters, &characterCount, n );
					ERR_IF_PASSTHROUGH;
				}
				else if ( n == TROT_OP_PUSH_LIST )
				{
					index += 1;

					rc = trotListRefGetKind( lrCurrentList, index, &kind );
					ERR_IF( rc != TROT_RC_SUCCESS, TROT_RC_ERROR_ENCODE );

					ERR_IF( kind != TROT_KIND_LIST, TROT_RC_ERROR_ENCODE );

					trotListRefFree( &lrChildList );
					rc = trotListRefGetListTwin( lrCurrentList, index, &lrChildList );
					ERR_IF_PASSTHROUGH;

					PARANOID_ERR_IF( lrChildList -> lPointsTo == NULL );
					if ( lrChildList -> lPointsTo -> encodingChildNumber == 0 )
					{
						lrChildList -> lPointsTo -> encodingChildNumber = index;
						lrChildList -> lPointsTo -> encodingParent = lrCurrentList -> lPointsTo;

						rc = appendLeftBAndTag( lrChildList, 0, newLrCharacters, &indentCount );
						ERR_IF_PASSTHROUGH;

						/* push to parent stacks, setup vars */
						rc = trotListRefAppendListTwin( lrParentStack, lrCurrentList );
						ERR_IF_PASSTHROUGH;

						rc = trotListRefAppendInt( lrParentIndicesStack, index );
						ERR_IF_PASSTHROUGH;

						trotListRefFree( &lrCurrentList );
						lrCurrentList = lrChildList;
						lrChildList = NULL;

						index = 0;

						PARANOID_ERR_IF( lrCurrentList -> lPointsTo == NULL );
						currentTag = lrCurrentList -> lPointsTo -> tag;
					}
					else
					{
						rc = appendAbsTwinLocation( newLrCharacters, &characterCount, lrChildList );
						ERR_IF_PASSTHROUGH;
					}
				}

				break;

			case TROT_TAG_TEXT:

				rc = trotListRefAppendInt( newLrCharacters, '\"' );
				ERR_IF_PASSTHROUGH;

				rc = trotCharactersToUtf8( lrCurrentList, newLrCharacters );
				ERR_IF_PASSTHROUGH;

				rc = trotListRefAppendInt( newLrCharacters, '\"' );
				ERR_IF_PASSTHROUGH;

				/* we must flag that we're done since we handled it all here */
				index = childrenCount;

				break;

			/* if we're inside DATA */
			default:

				rc = trotListRefGetKind( lrCurrentList, index, &kind );
				PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

				if ( kind == TROT_KIND_INT )
				{
					rc = trotListRefGetInt( lrCurrentList, index, &n );
					PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

					rc = appendNumber( newLrCharacters, &characterCount, n );
					ERR_IF_PASSTHROUGH;
				}
				else /* kind == TROT_KIND_LIST */
				{
					trotListRefFree( &lrChildList );
					rc = trotListRefGetListTwin( lrCurrentList, index, &lrChildList );
					ERR_IF_PASSTHROUGH;

					PARANOID_ERR_IF( lrChildList -> lPointsTo == NULL );
					if ( lrChildList -> lPointsTo -> encodingChildNumber == 0 )
					{
						lrChildList -> lPointsTo -> encodingChildNumber = index;
						lrChildList -> lPointsTo -> encodingParent = lrCurrentList -> lPointsTo;

						rc = appendLeftBAndTag( lrChildList, 0, newLrCharacters, &indentCount );
						ERR_IF_PASSTHROUGH;

						/* push to parent stacks, setup vars */
						rc = trotListRefAppendListTwin( lrParentStack, lrCurrentList );
						ERR_IF_PASSTHROUGH;

						rc = trotListRefAppendInt( lrParentIndicesStack, index );
						ERR_IF_PASSTHROUGH;

						trotListRefFree( &lrCurrentList );
						lrCurrentList = lrChildList;
						lrChildList = NULL;

						index = 0;

						PARANOID_ERR_IF( lrCurrentList -> lPointsTo == NULL );
						currentTag = lrCurrentList -> lPointsTo -> tag;
					}
					else
					{
						rc = appendAbsTwinLocation( newLrCharacters, &characterCount, lrChildList );
						ERR_IF_PASSTHROUGH;
					}
				}

				break;
		}
	}

	PARANOID_ERR_IF( indentCount != 0 );

	/* go through tree again, resetting encodingChildNumber and encodingParent */
	/* create parent stack, so we can "go up" */
	trotListRefFree( &lrParentStack );
	rc = trotListRefInit( &lrParentStack );
	ERR_IF_PASSTHROUGH;

	trotListRefFree( &lrParentIndicesStack );
	rc = trotListRefInit( &lrParentIndicesStack );
	ERR_IF_PASSTHROUGH;

	/* setup */
	trotListRefFree( &lrCurrentList );
	rc = trotListRefTwin( lr, &lrCurrentList );
	ERR_IF_PASSTHROUGH;

	index = 0;

	PARANOID_ERR_IF( lrCurrentList -> lPointsTo == NULL );

	/* go through list */
	while ( 1 )
	{
		/* do we have a next child? */

		/* get count */
		rc = trotListRefGetCount( lrCurrentList, &childrenCount );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		/* are we out of children? */
		if ( index == childrenCount )
		{
			/* do we have a parent? */
			rc = trotListRefGetCount( lrParentStack, &parentStackCount );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			if ( parentStackCount == 0 )
			{
				/* break, we're done */
				break;
			}

			/* go up to parent */
			trotListRefFree( &lrCurrentList );
			rc = trotListRefRemoveList( lrParentStack, -1, &lrCurrentList );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			rc = trotListRefRemoveInt( lrParentIndicesStack, -1, &index );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			PARANOID_ERR_IF( lrCurrentList -> lPointsTo == NULL );
			currentTag = lrCurrentList -> lPointsTo -> tag;

			/* continue */
			continue;
		}

		/* increment */
		index += 1;

		/* if we're inside TEXT */
		if ( currentTag == TROT_TAG_TEXT )
		{
			/* there will be no sub-lists here */

			/* we must flag that we're done since we handled it all here */
			index = childrenCount;
		}
		else
		{
			rc = trotListRefGetKind( lrCurrentList, index, &kind );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			if ( kind == TROT_KIND_LIST )
			{
				trotListRefFree( &lrChildList );
				rc = trotListRefGetListTwin( lrCurrentList, index, &lrChildList );
				ERR_IF_PASSTHROUGH;

				PARANOID_ERR_IF( lrChildList -> lPointsTo == NULL );
				if ( lrChildList -> lPointsTo -> encodingChildNumber > 0 )
				{
					lrChildList -> lPointsTo -> encodingChildNumber = 0;
					lrChildList -> lPointsTo -> encodingParent = NULL;

					/* push to parent stacks, setup vars */
					rc = trotListRefAppendListTwin( lrParentStack, lrCurrentList );
					ERR_IF_PASSTHROUGH;

					rc = trotListRefAppendInt( lrParentIndicesStack, index );
					ERR_IF_PASSTHROUGH;

					trotListRefFree( &lrCurrentList );
					lrCurrentList = lrChildList;
					lrChildList = NULL;

					index = 0;

					PARANOID_ERR_IF( lrCurrentList -> lPointsTo == NULL );
				}
			}
		}
	}

	lr -> lPointsTo -> encodingChildNumber = 0;

	PARANOID_ERR_IF( indentCount != 0 );


	/* give back */
	(*lrCharacters_A) = newLrCharacters;
	newLrCharacters = NULL;


	/* CLEANUP */
	cleanup:

	trotListRefFree( &newLrCharacters );
	trotListRefFree( &lrParentStack );
	trotListRefFree( &lrParentIndicesStack );
	trotListRefFree( &lrCurrentList );
	trotListRefFree( &lrChildList );

	return rc;
}

/******************************************************************************/
/*!
	\brief Append left bracket or brace and it's tag.
	\param lr List we're appending for. We need this to get the tag.
	\param lrCharacters List of characters we're going to append to.
	\param indentCount Current indent count.
	\return TROT_RC
*/
static TROT_RC appendLeftBAndTag( trotListRef *lr, int topList, trotListRef *lrCharacters, unsigned int *indentCount )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	char *s1 = NULL;
	char *s2 = NULL;
	char *t = "(name top)";


	/* CODE */
	PARANOID_ERR_IF( lr == NULL );
	PARANOID_ERR_IF( lrCharacters == NULL );
	PARANOID_ERR_IF( indentCount == NULL );

	/* append newlines and indents */
	rc = appendNewlineIndents( lrCharacters, (*indentCount) );
	ERR_IF_PASSTHROUGH;

	/* set s according to tag */
	PARANOID_ERR_IF( lr -> lPointsTo == NULL );

	switch ( lr -> lPointsTo -> tag )
	{
		case TROT_TAG_TEXT:
			s1 = "[";
			s2 = "(text)";
			break;

		case TROT_TAG_CODE:
		case TROT_TAG_FUNCTION:
		case TROT_TAG_RAW_CODE:
			s1 = "{";
			s2 = "(raw)";
			break;

		default:
			s1 = "[";
			break;
	}

	/* append s */
	while ( (*s1) != '\0' )
	{
		rc = trotListRefAppendInt( lrCharacters, (*s1) );
		ERR_IF_PASSTHROUGH;

		s1 += 1;
	}

	if ( topList == 1 )
	{
		while ( (*t) != '\0' )
		{
			rc = trotListRefAppendInt( lrCharacters, (*t) );
#if ( NODE_SIZE < 11 )
#error NODE_SIZE TOO SMALL, MUST UNCOMMENT THE ERR_IF_PASSTHROUGH BELOW
#endif
			/* ERR_IF_PASSTHROUGH; */

			t += 1;
		}
	}

	if ( s2 != NULL )
	{
		while ( (*s2) != '\0' )
		{
			rc = trotListRefAppendInt( lrCharacters, (*s2) );
			ERR_IF_PASSTHROUGH;

			s2 += 1;
		}
	}

	/* increment indentCount */
	(*indentCount) += 1;

	/* append newline and indents */
	rc = appendNewlineIndents( lrCharacters, (*indentCount) );
	ERR_IF_PASSTHROUGH;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Append left bracket or brace and it's tag.
	\param lr List we're appending for. We need this to get the tag.
	\param lrCharacters List of characters we're going to append to.
	\param indentCount Current indent count.
	\return TROT_RC
*/
static TROT_RC appendRightB( trotListRef *lr, trotListRef *lrCharacters, unsigned int *indentCount )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	char s = ']';


	/* CODE */
	PARANOID_ERR_IF( lr == NULL );
	PARANOID_ERR_IF( lrCharacters == NULL );
	PARANOID_ERR_IF( indentCount == NULL );

	/* decrement indentCount */
	(*indentCount) -= 1;

	/* append newlines and indents */
	rc = appendNewlineIndents( lrCharacters, (*indentCount) );
	ERR_IF_PASSTHROUGH;

	/* set s according to tag */
	PARANOID_ERR_IF( lr -> lPointsTo == NULL );

	if (    lr -> lPointsTo -> tag == TROT_TAG_CODE
	     || lr -> lPointsTo -> tag == TROT_TAG_FUNCTION
	     || lr -> lPointsTo -> tag == TROT_TAG_RAW_CODE
	   )
	{
		s = '}';
	}

	/* append s */
	rc = trotListRefAppendInt( lrCharacters, s );
	ERR_IF_PASSTHROUGH;

	/* append newline and indents */
	rc = appendNewlineIndents( lrCharacters, (*indentCount) );
	ERR_IF_PASSTHROUGH;


	/* CLEANUP */
	cleanup:

	return rc;
}


/******************************************************************************/
/*!
	\brief Appends a newline and indentCount tabs.
	\param lrCharacters List we're appending to. 
	\param indentCount Current indent count.
	\return TROT_RC
*/
static TROT_RC appendNewlineIndents( trotListRef *lrCharacters, unsigned int indentCount )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	unsigned int i = 0;


	/* CODE */
	PARANOID_ERR_IF( lrCharacters == NULL );

	/* append newline */
	rc = trotListRefAppendInt( lrCharacters, '\n' );
	ERR_IF_PASSTHROUGH;

	/* append indents */
	while ( i < indentCount )
	{
		rc = trotListRefAppendInt( lrCharacters, '\t' );
		ERR_IF_PASSTHROUGH;

		i += 1;
	}


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Appends a number.
	\param lrCharacters List we're appending to.
	\param n Number to append.
	\param characterCount Current character count that we'll update.
	\return TROT_RC
*/
static TROT_RC appendNumber( trotListRef *lrCharacters, int *characterCount, INT_TYPE n )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	INT_TYPE lastCharacter = 0;

	char numberString[ INT_TYPE_MIN_STRING_LENGTH + 1 ];
	char *s = NULL;


	/* CODE */
	PARANOID_ERR_IF( lrCharacters == NULL );
	PARANOID_ERR_IF( characterCount == NULL );

	/* do we need to append a space? */
	rc = trotListRefGetInt( lrCharacters, -1, &lastCharacter );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	if ( lastCharacter != '\t' && lastCharacter != '.' )
	{
		rc = trotListRefAppendInt( lrCharacters, ' ' );
		ERR_IF_PASSTHROUGH;

		(*characterCount) += 1;
	}

	/* special case: 0 */
	if ( n == 0 )
	{
		rc = trotListRefAppendInt( lrCharacters, '0' );
		ERR_IF_PASSTHROUGH;

		(*characterCount) += 1;

		goto cleanup;
	}

	/* create numberString */
	s = &(numberString[ INT_TYPE_MIN_STRING_LENGTH ]);
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

	/* append numberString to lrCharacters */
	while ( (*s) != '\0' )
	{
		rc = trotListRefAppendInt( lrCharacters, (*s) );
		ERR_IF_PASSTHROUGH;

		(*characterCount) += 1;
		s += 1;
	}


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief 
	\param 
	\return TROT_RC
*/
static TROT_RC appendAbsTwinLocation( trotListRef *lrCharacters, int *characterCount, trotListRef *lr )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	INT_TYPE lastCharacter = 0;

	char *s = "top";

	trotListRef *lrAddress = NULL;
	INT_TYPE address = 0;

	INT_TYPE index = 0;
	INT_TYPE count = 0;

/* TODO: this will need to change (and all other trotList's when we change trotListRef to trotList and trotList to trotListInternal */
	trotList *lParent = NULL;


	/* CODE */
	PARANOID_ERR_IF( lrCharacters == NULL );
	PARANOID_ERR_IF( characterCount == NULL );
	PARANOID_ERR_IF( lr == NULL );

	/* do we need to append a space? */
	rc = trotListRefGetInt( lrCharacters, -1, &lastCharacter );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	if ( lastCharacter != '\t' )
	{
		rc = trotListRefAppendInt( lrCharacters, ' ' );
		ERR_IF_PASSTHROUGH;

		(*characterCount) += 1;
	}

	/* append "top" */
	while ( (*s) != '\0' )
	{
		rc = trotListRefAppendInt( lrCharacters, (*s) );
		ERR_IF_PASSTHROUGH;

		(*characterCount) += 1;
		s += 1;
	}

	/* if encodingChildNumber is -1, just need "top" */
	if ( lr -> lPointsTo -> encodingChildNumber == -1 )
	{
		goto cleanup;
	}

	/* create lrAddress */
	rc = trotListRefInit( &lrAddress );
	ERR_IF_PASSTHROUGH;

	lParent = lr -> lPointsTo;
	while ( lParent -> encodingChildNumber > 0 )
	{
		rc = trotListRefInsertInt( lrAddress, 1, lParent -> encodingChildNumber );
		ERR_IF_PASSTHROUGH;

		lParent = lParent -> encodingParent;
		PARANOID_ERR_IF( lParent == NULL );
	}


	/* get count */
	rc = trotListRefGetCount( lrAddress, &count );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* foreach parent */
	index = 1;
	while ( index <= count )
	{
		/* get parent */
		rc = trotListRefGetInt( lrAddress, index, &address );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		/* append '.' */
		rc = trotListRefAppendInt( lrCharacters, '.' );
		ERR_IF_PASSTHROUGH;

		(*characterCount) += 1;

		/* append number */
		rc = appendNumber( lrCharacters, characterCount, address );
		ERR_IF_PASSTHROUGH;

		/* increment */
		index += 1;
	}


	/* CLEANUP */
	cleanup:

	trotListRefFree( &lrAddress );

	return rc;
}

