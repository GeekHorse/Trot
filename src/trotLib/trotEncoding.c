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

/* TODO: move this somewhere else */
#define ENCODING_MAX_CHARACTER_COUNT 80

/******************************************************************************/
#include "trot.h"
#include "trotInternal.h"

/******************************************************************************/
static TROT_RC appendLeftBAndTag( trotListRef *lr, trotListRef *lrCharacters, unsigned int *indentCount );
static TROT_RC appendRightB( trotListRef *lr, trotListRef *lrCharacters, unsigned int *indentCount );
static TROT_RC appendNewlineIndents( trotListRef *lrCharacters, unsigned int indentCount );
static TROT_RC appendNumber( trotListRef *lrCharacters, INT_TYPE n, int *characterCount );

/******************************************************************************/
/*!
	\brief Encodes a list into a list of characters.
	\param 
	\return TROT_RC
*/
/* TODO: this needs another structure, instead of a loop and then check for our tag,
         we need to just handle all ints, and then go up or down */
TROT_RC trotEncode( trotListRef *lr, trotListRef **lrCharacters_A )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotListRef *newLrCharacters = NULL;

	trotListRef *lrParentStack = NULL;
	INT_TYPE parentStackCount = 0;

	trotListRef *lrParentIndicesStack = NULL;

	TROT_TAG currentTag = TROT_TAG_DATA;
	trotListRef *lrCurrentList = NULL;
	INT_TYPE childrenCount = 0;
	INT_TYPE index = 0;

	unsigned int indentCount = 0;

	int flagMoreChildren = 0;

	int kind = 0; /* TODO: this needs to be changed into an enum */

	int characterCount = 0;

	INT_TYPE n = 0;

	trotListRef *lrChildList = NULL;


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

	ERR_IF_PARANOID( lrCurrentList -> lPointsTo == NULL );
	currentTag = lrCurrentList -> lPointsTo -> tag;

	/* start our encoding */
	rc = appendLeftBAndTag( lrCurrentList, newLrCharacters, &indentCount );
	ERR_IF_PASSTHROUGH;

	/* go through list */
	while ( 1 )
	{
		flagMoreChildren = 1;

		/* do we have a next child? */
		rc = trotListRefGetCount( lrCurrentList, &childrenCount );
		ERR_IF_PASSTHROUGH;

		/* check overflow */
/* TODO: need this check elsewhere! */
		if ( index == INT_TYPE_MAX )
		{
			flagMoreChildren = 0;
		}
		/* check against childrenCount */
		else
		{
			index += 1;

			if ( index > childrenCount )
			{
				flagMoreChildren = 0;
			}
		}

		/* are we out of children? */
		if ( flagMoreChildren == 0 )
		{
			/* appendRightB */
			rc = appendRightB( lrCurrentList, newLrCharacters, &indentCount );
			ERR_IF_PASSTHROUGH;

			/* do we have a parent? */
			if ( parentStackCount == 0 )
			{
				/* break, we're done */
				break;
			}

			/* go up to parent */
			trotListRefFree( &lrCurrentList );

			rc = trotListRefRemoveList( lrParentStack, -1, &lrCurrentList );
			ERR_IF_PASSTHROUGH;

			rc = trotListRefRemoveInt( lrParentIndicesStack, -1, &index );
			ERR_IF_PASSTHROUGH;

			parentStackCount -= 1;

			ERR_IF_PARANOID( lrCurrentList -> lPointsTo == NULL );
			currentTag = lrCurrentList -> lPointsTo -> tag;

			/* continue */
			continue;
		}

		/* if we're inside DATA */
		if ( currentTag == TROT_TAG_DATA )
		{
			rc = trotListRefGetKind( lrCurrentList, index, &kind );
			ERR_IF_PASSTHROUGH;

			if ( kind == NODE_KIND_INT )
			{
				if ( characterCount > ENCODING_MAX_CHARACTER_COUNT )
				{
					rc = appendNewlineIndents( newLrCharacters, indentCount );
					ERR_IF_PASSTHROUGH;

					characterCount = 0;
				}

				rc = trotListRefGetInt( lrCurrentList, index, &n );
				ERR_IF_PASSTHROUGH;

				rc = appendNumber( newLrCharacters, n, &characterCount );
				ERR_IF_PASSTHROUGH;
			}
			else /* kind == NODE_KIND_LIST */
			{
				rc = trotListRefGetListTwin( lrCurrentList, index, &lrChildList );
				ERR_IF_PASSTHROUGH;

				/* TODO: handle the case that we've already encoded this list, so we
				         add @0.4.2 or something */

				rc = appendLeftBAndTag( lrChildList, newLrCharacters, &indentCount );
				ERR_IF_PASSTHROUGH;

				/* push to parent stacks, setup vars */
				rc = trotListRefAppendListTwin( lrParentStack, lrCurrentList );
				ERR_IF_PASSTHROUGH;

				rc = trotListRefAppendInt( lrParentIndicesStack, index );
				ERR_IF_PASSTHROUGH;

				parentStackCount += 1; /* TODO: do we need to check this for overflow? */

				trotListRefFree( &lrCurrentList );

				lrCurrentList = lrChildList;
				lrChildList = NULL;

				index = 0;

				ERR_IF_PARANOID( lrCurrentList -> lPointsTo == NULL );
				currentTag = lrCurrentList -> lPointsTo -> tag;
			}
		}
		else if ( currentTag == TROT_TAG_TEXT )
		{
			rc = trotListRefAppendInt( newLrCharacters, '\"' );
			ERR_IF_PASSTHROUGH;

			rc = trotCharactersToUtf8( lrCurrentList, newLrCharacters );
			ERR_IF_PASSTHROUGH;

			rc = trotListRefAppendInt( newLrCharacters, '\"' );
			ERR_IF_PASSTHROUGH;

			/* we must flag that we're done since we handled it all here */
			index = childrenCount;
		}
		/* TODO
		else if { currentTag == TODO )
		{
		}
		*/
	}

	ERR_IF_PARANOID( indentCount != 0 );

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
static TROT_RC appendLeftBAndTag( trotListRef *lr, trotListRef *lrCharacters, unsigned int *indentCount )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	char *s = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );
	PRECOND_ERR_IF( lrCharacters == NULL );
	PRECOND_ERR_IF( indentCount == NULL );


	/* CODE */
	/* append newlines and indents */
	rc = appendNewlineIndents( lrCharacters, (*indentCount) );
	ERR_IF_PASSTHROUGH;

	/* set s according to tag */
	ERR_IF_PARANOID( lr -> lPointsTo == NULL );

	switch ( lr -> lPointsTo -> tag )
	{
		case TROT_TAG_DATA:
			s = "[";
			break;

		case TROT_TAG_TEXT:
			s = "[(tag text)";
			break;

		case TROT_TAG_ACTOR:
			s = "[(tag actor)";
			break;

		case TROT_TAG_QUEUE:
			s = "[(tag queue)";
			break;

		case TROT_TAG_CODE:
			s = "{";
			break;

		case TROT_TAG_CODE_GROUP:
			s = "[(tag codeGroup)";
			break;

		case TROT_TAG_FUNCTION:
			s = "{(tag function)";
			break;

		default:
			ERR_IF( 1, TROT_LIST_ERROR_ENCODE );
			break;
	}

	/* append s */
	/* TODO: move appendCStringToList into trotLib and use it? */
	while ( (*s) != '\0' )
	{
		rc = trotListRefAppendInt( lrCharacters, (*s) );
		ERR_IF_PASSTHROUGH;

		s += 1;
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
	TROT_RC rc = TROT_LIST_SUCCESS;

	char s = ']';


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );
	PRECOND_ERR_IF( lrCharacters == NULL );
	PRECOND_ERR_IF( indentCount == NULL );


	/* CODE */
	/* decrement indentCount */
	(*indentCount) -= 1;

	/* append newlines and indents */
	rc = appendNewlineIndents( lrCharacters, (*indentCount) );
	ERR_IF_PASSTHROUGH;

	/* set s according to tag */
	ERR_IF_PARANOID( lr -> lPointsTo == NULL );

	if (    lr -> lPointsTo -> tag == TROT_TAG_CODE
	     || lr -> lPointsTo -> tag == TROT_TAG_FUNCTION
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
	TROT_RC rc = TROT_LIST_SUCCESS;

	unsigned int i = 0;


	/* PRECOND */
	PRECOND_ERR_IF( lrCharacters == NULL );


	/* CODE */
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
static TROT_RC appendNumber( trotListRef *lrCharacters, INT_TYPE n, int *characterCount )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	INT_TYPE lastCharacter = 0;

	char numberString[ INT_TYPE_MIN_STRING_LENGTH + 1 ];
	char *s = NULL;

	int flagNegative = 0;


	/* PRECOND */
	PRECOND_ERR_IF( lrCharacters == NULL );
	PRECOND_ERR_IF( characterCount == NULL );


	/* CODE */
	/* do we need to append a space? */
	rc = trotListRefGetInt( lrCharacters, -1, &lastCharacter );
	ERR_IF_PASSTHROUGH;

	if ( lastCharacter != '\t' && lastCharacter != ' ' )
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

	/* set flagNegative */
	if ( n < 0 )
	{
		flagNegative = 1;
	}

	/* create numberString */
	s = &(numberString[ INT_TYPE_MIN_STRING_LENGTH ]);
	(*s) = '\0';

	while ( n != 0 )
	{
		s -= 1;
		(*s) =  '0' + ( n % 10 );

		n /= 10;
	}

	if ( flagNegative )
	{
		s -= 1;
		(*s) = '-';
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
