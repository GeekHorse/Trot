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
static TROT_RC appendLeftBAndTag( trotList *l, int topList, trotList *lCharacters, unsigned int *indentCount );
static TROT_RC appendRightB( trotList *l, trotList *lCharacters, unsigned int *indentCount );
static TROT_RC appendNewlineIndents( trotList *lCharacters, unsigned int indentCount );
static TROT_RC appendNumber( trotList *lCharacters, int *characterCount, TROT_INT n );
static TROT_RC appendAbsTwinLocation( trotList *lCharacters, int *characterCount, trotList *l );

/******************************************************************************/
/*!
	\brief Encodes a list into a list of characters.
	\param 
	\return TROT_RC
*/
TROT_RC trotEncode( trotList *l, trotList **lCharacters_A )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	trotList *newLrCharacters = NULL;
	TROT_INT lastCharacter = 0;

	trotList *laParentStack = NULL;
	TROT_INT parentStackCount = 0;

	trotList *laParentIndicesStack = NULL;

	TROT_TAG currentTag = TROT_TAG_DATA;
	trotList *lCurrentList = NULL;
	TROT_INT childrenCount = 0;
	TROT_INT index = 0;

	unsigned int indentCount = 0;

	TROT_KIND kind = 0;

	int characterCount = 0;

	TROT_INT n = 0;

	trotList *lChildList = NULL;

	const char *s = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( l == NULL );
	PRECOND_ERR_IF( lCharacters_A == NULL );
	PRECOND_ERR_IF( (*lCharacters_A) != NULL );


	/* CODE */
	/* create "give back" list */
	rc = trotListInit( &newLrCharacters );
	ERR_IF_PASSTHROUGH;

	/* create parent stack, so we can "go up" */
	rc = trotListInit( &laParentStack );
	ERR_IF_PASSTHROUGH;

	rc = trotListInit( &laParentIndicesStack );
	ERR_IF_PASSTHROUGH;

	/* setup */
	rc = trotListTwin( l, &lCurrentList );
	ERR_IF_PASSTHROUGH;

	index = 0;

	PARANOID_ERR_IF( lCurrentList -> laPointsTo == NULL );
	currentTag = lCurrentList -> laPointsTo -> tag;

	lCurrentList -> laPointsTo -> encodingChildNumber = -1;

	/* start our encoding */
	rc = appendLeftBAndTag( lCurrentList, 1, newLrCharacters, &indentCount );
	ERR_IF_PASSTHROUGH;

	/* go through list */
	while ( 1 )
	{
		/* do we have a next child? */

		/* get count */
		rc = trotListGetCount( lCurrentList, &childrenCount );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		/* are we out of children? */
		if ( index == childrenCount )
		{
			/* appendRightB */
			rc = appendRightB( lCurrentList, newLrCharacters, &indentCount );
			ERR_IF_PASSTHROUGH;

			/* do we have a parent? */
			rc = trotListGetCount( laParentStack, &parentStackCount );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			if ( parentStackCount == 0 )
			{
				/* break, we're done */
				break;
			}

			/* go up to parent */
			trotListFree( &lCurrentList );
			rc = trotListRemoveList( laParentStack, -1, &lCurrentList );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			rc = trotListRemoveInt( laParentIndicesStack, -1, &index );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			PARANOID_ERR_IF( lCurrentList -> laPointsTo == NULL );
			currentTag = lCurrentList -> laPointsTo -> tag;

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

				rc = trotListGetKind( lCurrentList, index, &kind );
				PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

				ERR_IF( kind != TROT_KIND_INT, TROT_RC_ERROR_ENCODE );

				rc = trotListGetInt( lCurrentList, index, &n );
				PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

				ERR_IF( n < TROT_OP_MIN || n > TROT_OP_MAX, TROT_RC_ERROR_ENCODE );

				/* do we need to append a space? */
				rc = trotListGetInt( newLrCharacters, -1, &lastCharacter );
				PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

				if ( lastCharacter != '\t' )
				{
					rc = trotListAppendInt( newLrCharacters, ' ' );
					ERR_IF_PASSTHROUGH;

					characterCount += 1;
				}

				s = opNames[ n - 1 ];
				while ( (*s) != '\0' )
				{
					rc = trotListAppendInt( newLrCharacters, (*s) );
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

					rc = trotListGetKind( lCurrentList, index, &kind );
					ERR_IF( rc != TROT_RC_SUCCESS, TROT_RC_ERROR_ENCODE );

					ERR_IF( kind != TROT_KIND_INT, TROT_RC_ERROR_ENCODE );

					rc = trotListGetInt( lCurrentList, index, &n );
					PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

					rc = appendNumber( newLrCharacters, &characterCount, n );
					ERR_IF_PASSTHROUGH;
				}
				else if ( n == TROT_OP_PUSH_LIST )
				{
					index += 1;

					rc = trotListGetKind( lCurrentList, index, &kind );
					ERR_IF( rc != TROT_RC_SUCCESS, TROT_RC_ERROR_ENCODE );

					ERR_IF( kind != TROT_KIND_LIST, TROT_RC_ERROR_ENCODE );

					trotListFree( &lChildList );
					rc = trotListGetList( lCurrentList, index, &lChildList );
					ERR_IF_PASSTHROUGH;

					PARANOID_ERR_IF( lChildList -> laPointsTo == NULL );
					if ( lChildList -> laPointsTo -> encodingChildNumber == 0 )
					{
						lChildList -> laPointsTo -> encodingChildNumber = index;
						lChildList -> laPointsTo -> encodingParent = lCurrentList -> laPointsTo;

						rc = appendLeftBAndTag( lChildList, 0, newLrCharacters, &indentCount );
						ERR_IF_PASSTHROUGH;

						/* push to parent stacks, setup vars */
						rc = trotListAppendList( laParentStack, lCurrentList );
						ERR_IF_PASSTHROUGH;

						rc = trotListAppendInt( laParentIndicesStack, index );
						ERR_IF_PASSTHROUGH;

						trotListFree( &lCurrentList );
						lCurrentList = lChildList;
						lChildList = NULL;

						index = 0;

						PARANOID_ERR_IF( lCurrentList -> laPointsTo == NULL );
						currentTag = lCurrentList -> laPointsTo -> tag;
					}
					else
					{
						rc = appendAbsTwinLocation( newLrCharacters, &characterCount, lChildList );
						ERR_IF_PASSTHROUGH;
					}
				}

				break;

			case TROT_TAG_TEXT:

				rc = trotListAppendInt( newLrCharacters, '\"' );
				ERR_IF_PASSTHROUGH;

				rc = trotCharactersToUtf8( lCurrentList, newLrCharacters );
				ERR_IF_PASSTHROUGH;

				rc = trotListAppendInt( newLrCharacters, '\"' );
				ERR_IF_PASSTHROUGH;

				/* we must flag that we're done since we handled it all here */
				index = childrenCount;

				break;

			/* if we're inside DATA */
			default:

				rc = trotListGetKind( lCurrentList, index, &kind );
				PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

				if ( kind == TROT_KIND_INT )
				{
					rc = trotListGetInt( lCurrentList, index, &n );
					PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

					rc = appendNumber( newLrCharacters, &characterCount, n );
					ERR_IF_PASSTHROUGH;
				}
				else /* kind == TROT_KIND_LIST */
				{
					trotListFree( &lChildList );
					rc = trotListGetList( lCurrentList, index, &lChildList );
					ERR_IF_PASSTHROUGH;

					PARANOID_ERR_IF( lChildList -> laPointsTo == NULL );
					if ( lChildList -> laPointsTo -> encodingChildNumber == 0 )
					{
						lChildList -> laPointsTo -> encodingChildNumber = index;
						lChildList -> laPointsTo -> encodingParent = lCurrentList -> laPointsTo;

						rc = appendLeftBAndTag( lChildList, 0, newLrCharacters, &indentCount );
						ERR_IF_PASSTHROUGH;

						/* push to parent stacks, setup vars */
						rc = trotListAppendList( laParentStack, lCurrentList );
						ERR_IF_PASSTHROUGH;

						rc = trotListAppendInt( laParentIndicesStack, index );
						ERR_IF_PASSTHROUGH;

						trotListFree( &lCurrentList );
						lCurrentList = lChildList;
						lChildList = NULL;

						index = 0;

						PARANOID_ERR_IF( lCurrentList -> laPointsTo == NULL );
						currentTag = lCurrentList -> laPointsTo -> tag;
					}
					else
					{
						rc = appendAbsTwinLocation( newLrCharacters, &characterCount, lChildList );
						ERR_IF_PASSTHROUGH;
					}
				}

				break;
		}
	}

	PARANOID_ERR_IF( indentCount != 0 );

	/* go through tree again, resetting encodingChildNumber and encodingParent */
	/* create parent stack, so we can "go up" */
	trotListFree( &laParentStack );
	rc = trotListInit( &laParentStack );
	ERR_IF_PASSTHROUGH;

	trotListFree( &laParentIndicesStack );
	rc = trotListInit( &laParentIndicesStack );
	ERR_IF_PASSTHROUGH;

	/* setup */
	trotListFree( &lCurrentList );
	rc = trotListTwin( l, &lCurrentList );
	ERR_IF_PASSTHROUGH;

	index = 0;

	PARANOID_ERR_IF( lCurrentList -> laPointsTo == NULL );

	/* go through list */
	while ( 1 )
	{
		/* do we have a next child? */

		/* get count */
		rc = trotListGetCount( lCurrentList, &childrenCount );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		/* are we out of children? */
		if ( index == childrenCount )
		{
			/* do we have a parent? */
			rc = trotListGetCount( laParentStack, &parentStackCount );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			if ( parentStackCount == 0 )
			{
				/* break, we're done */
				break;
			}

			/* go up to parent */
			trotListFree( &lCurrentList );
			rc = trotListRemoveList( laParentStack, -1, &lCurrentList );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			rc = trotListRemoveInt( laParentIndicesStack, -1, &index );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			PARANOID_ERR_IF( lCurrentList -> laPointsTo == NULL );
			currentTag = lCurrentList -> laPointsTo -> tag;

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
			rc = trotListGetKind( lCurrentList, index, &kind );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			if ( kind == TROT_KIND_LIST )
			{
				trotListFree( &lChildList );
				rc = trotListGetList( lCurrentList, index, &lChildList );
				ERR_IF_PASSTHROUGH;

				PARANOID_ERR_IF( lChildList -> laPointsTo == NULL );
				if ( lChildList -> laPointsTo -> encodingChildNumber > 0 )
				{
					lChildList -> laPointsTo -> encodingChildNumber = 0;
					lChildList -> laPointsTo -> encodingParent = NULL;

					/* push to parent stacks, setup vars */
					rc = trotListAppendList( laParentStack, lCurrentList );
					ERR_IF_PASSTHROUGH;

					rc = trotListAppendInt( laParentIndicesStack, index );
					ERR_IF_PASSTHROUGH;

					trotListFree( &lCurrentList );
					lCurrentList = lChildList;
					lChildList = NULL;

					index = 0;

					PARANOID_ERR_IF( lCurrentList -> laPointsTo == NULL );
				}
			}
		}
	}

	l -> laPointsTo -> encodingChildNumber = 0;

	PARANOID_ERR_IF( indentCount != 0 );


	/* give back */
	(*lCharacters_A) = newLrCharacters;
	newLrCharacters = NULL;


	/* CLEANUP */
	cleanup:

	trotListFree( &newLrCharacters );
	trotListFree( &laParentStack );
	trotListFree( &laParentIndicesStack );
	trotListFree( &lCurrentList );
	trotListFree( &lChildList );

	return rc;
}

/******************************************************************************/
/*!
	\brief Append left bracket or brace and it's tag.
	\param l List we're appending for. We need this to get the tag.
	\param lCharacters List of characters we're going to append to.
	\param indentCount Current indent count.
	\return TROT_RC
*/
static TROT_RC appendLeftBAndTag( trotList *l, int topList, trotList *lCharacters, unsigned int *indentCount )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	char *s1 = NULL;
	char *s2 = NULL;
	char *t = "(name top)";


	/* CODE */
	PARANOID_ERR_IF( l == NULL );
	PARANOID_ERR_IF( lCharacters == NULL );
	PARANOID_ERR_IF( indentCount == NULL );

	/* append newlines and indents */
	rc = appendNewlineIndents( lCharacters, (*indentCount) );
	ERR_IF_PASSTHROUGH;

	/* set s according to tag */
	PARANOID_ERR_IF( l -> laPointsTo == NULL );

	switch ( l -> laPointsTo -> tag )
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
		rc = trotListAppendInt( lCharacters, (*s1) );
		ERR_IF_PASSTHROUGH;

		s1 += 1;
	}

	if ( topList == 1 )
	{
		while ( (*t) != '\0' )
		{
			rc = trotListAppendInt( lCharacters, (*t) );
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
			rc = trotListAppendInt( lCharacters, (*s2) );
			ERR_IF_PASSTHROUGH;

			s2 += 1;
		}
	}

	/* increment indentCount */
	(*indentCount) += 1;

	/* append newline and indents */
	rc = appendNewlineIndents( lCharacters, (*indentCount) );
	ERR_IF_PASSTHROUGH;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Append left bracket or brace and it's tag.
	\param l List we're appending for. We need this to get the tag.
	\param lCharacters List of characters we're going to append to.
	\param indentCount Current indent count.
	\return TROT_RC
*/
static TROT_RC appendRightB( trotList *l, trotList *lCharacters, unsigned int *indentCount )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	char s = ']';


	/* CODE */
	PARANOID_ERR_IF( l == NULL );
	PARANOID_ERR_IF( lCharacters == NULL );
	PARANOID_ERR_IF( indentCount == NULL );

	/* decrement indentCount */
	(*indentCount) -= 1;

	/* append newlines and indents */
	rc = appendNewlineIndents( lCharacters, (*indentCount) );
	ERR_IF_PASSTHROUGH;

	/* set s according to tag */
	PARANOID_ERR_IF( l -> laPointsTo == NULL );

	if (    l -> laPointsTo -> tag == TROT_TAG_CODE
	     || l -> laPointsTo -> tag == TROT_TAG_FUNCTION
	     || l -> laPointsTo -> tag == TROT_TAG_RAW_CODE
	   )
	{
		s = '}';
	}

	/* append s */
	rc = trotListAppendInt( lCharacters, s );
	ERR_IF_PASSTHROUGH;

	/* append newline and indents */
	rc = appendNewlineIndents( lCharacters, (*indentCount) );
	ERR_IF_PASSTHROUGH;


	/* CLEANUP */
	cleanup:

	return rc;
}


/******************************************************************************/
/*!
	\brief Appends a newline and indentCount tabs.
	\param lCharacters List we're appending to. 
	\param indentCount Current indent count.
	\return TROT_RC
*/
static TROT_RC appendNewlineIndents( trotList *lCharacters, unsigned int indentCount )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	unsigned int i = 0;


	/* CODE */
	PARANOID_ERR_IF( lCharacters == NULL );

	/* append newline */
	rc = trotListAppendInt( lCharacters, '\n' );
	ERR_IF_PASSTHROUGH;

	/* append indents */
	while ( i < indentCount )
	{
		rc = trotListAppendInt( lCharacters, '\t' );
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
	\param lCharacters List we're appending to.
	\param n Number to append.
	\param characterCount Current character count that we'll update.
	\return TROT_RC
*/
static TROT_RC appendNumber( trotList *lCharacters, int *characterCount, TROT_INT n )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TROT_INT lastCharacter = 0;

	char numberString[ TROT_INT_MIN_STRING_LENGTH + 1 ];
	char *s = NULL;


	/* CODE */
	PARANOID_ERR_IF( lCharacters == NULL );
	PARANOID_ERR_IF( characterCount == NULL );

	/* do we need to append a space? */
	rc = trotListGetInt( lCharacters, -1, &lastCharacter );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	if ( lastCharacter != '\t' && lastCharacter != '.' )
	{
		rc = trotListAppendInt( lCharacters, ' ' );
		ERR_IF_PASSTHROUGH;

		(*characterCount) += 1;
	}

	/* special case: 0 */
	if ( n == 0 )
	{
		rc = trotListAppendInt( lCharacters, '0' );
		ERR_IF_PASSTHROUGH;

		(*characterCount) += 1;

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
		rc = trotListAppendInt( lCharacters, (*s) );
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
static TROT_RC appendAbsTwinLocation( trotList *lCharacters, int *characterCount, trotList *l )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TROT_INT lastCharacter = 0;

	char *s = "top";

	trotList *lAddress = NULL;
	TROT_INT address = 0;

	TROT_INT index = 0;
	TROT_INT count = 0;

	trotListActual *laParent = NULL;


	/* CODE */
	PARANOID_ERR_IF( lCharacters == NULL );
	PARANOID_ERR_IF( characterCount == NULL );
	PARANOID_ERR_IF( l == NULL );

	/* do we need to append a space? */
	rc = trotListGetInt( lCharacters, -1, &lastCharacter );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	if ( lastCharacter != '\t' )
	{
		rc = trotListAppendInt( lCharacters, ' ' );
		ERR_IF_PASSTHROUGH;

		(*characterCount) += 1;
	}

	/* append "top" */
	while ( (*s) != '\0' )
	{
		rc = trotListAppendInt( lCharacters, (*s) );
		ERR_IF_PASSTHROUGH;

		(*characterCount) += 1;
		s += 1;
	}

	/* if encodingChildNumber is -1, just need "top" */
	if ( l -> laPointsTo -> encodingChildNumber == -1 )
	{
		goto cleanup;
	}

	/* create lAddress */
	rc = trotListInit( &lAddress );
	ERR_IF_PASSTHROUGH;

	laParent = l -> laPointsTo;
	while ( laParent -> encodingChildNumber > 0 )
	{
		rc = trotListInsertInt( lAddress, 1, laParent -> encodingChildNumber );
		ERR_IF_PASSTHROUGH;

		laParent = laParent -> encodingParent;
		PARANOID_ERR_IF( laParent == NULL );
	}


	/* get count */
	rc = trotListGetCount( lAddress, &count );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* foreach parent */
	index = 1;
	while ( index <= count )
	{
		/* get parent */
		rc = trotListGetInt( lAddress, index, &address );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		/* append '.' */
		rc = trotListAppendInt( lCharacters, '.' );
		ERR_IF_PASSTHROUGH;

		(*characterCount) += 1;

		/* append number */
		rc = appendNumber( lCharacters, characterCount, address );
		ERR_IF_PASSTHROUGH;

		/* increment */
		index += 1;
	}


	/* CLEANUP */
	cleanup:

	trotListFree( &lAddress );

	return rc;
}

