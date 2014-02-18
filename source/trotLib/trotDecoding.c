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
	Decodes textual format to trot list.
*/
#define TROT_FILE_NUMBER 5

/******************************************************************************/
#include "trot.h"
#include "trotInternal.h"

/******************************************************************************/
static TROT_RC wordToNumber( TrotList *lWord, TROT_INT *number );
static TROT_RC splitList( TrotList *listToSplit, TROT_INT separator, TrotList **lPartList_A );
static TROT_RC getReferenceList( TrotList *lTop, TrotList *lPartList, TrotList **lReference_A );

/******************************************************************************/
/*!
	\brief Decodes a list of characters into a list.
	\param[in] lCharacters Characters to decode.
	\param[out] lDecodedList_A On success, the decoded list.
	\return TROT_RC

	lCharacters is not modified.
	lDecodedList_A is created, and caller is responsible for freeing.
*/
TROT_RC trotDecode( TrotList *lCharacters, TrotList **lDecodedList_A )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TROT_INT charactersCount = 0;
	TROT_INT index = 1;

	TrotList *lTop = NULL;
	TrotList *lCurrent = NULL;
	TrotList *lChild = NULL;

	TrotList *lWord = NULL;
	TROT_INT wordCount = 0;
	TROT_INT ch = 0;

	TROT_INT number = 0;

	TrotList *lStack = NULL;
	TROT_INT stackCount = 0;

	TrotList *lPartList = NULL;


	/* PRECOND */
	ERR_IF( lCharacters == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( lDecodedList_A == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( (*lDecodedList_A) != NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	/* get count of characters */
	rc = trotListGetCount( lCharacters, &charactersCount );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* create "top" list */
	rc = trotListInit( &lTop );
	ERR_IF_PASSTHROUGH;

	/* create current list */
	rc = trotListTwin( lTop, &lCurrent );
	ERR_IF_PASSTHROUGH;

	/* create our stack */
	rc = trotListInit( &lStack );
	ERR_IF_PASSTHROUGH;

	/* skip whitespace */
	while ( index <= charactersCount )
	{
		rc = trotListGetInt( lCharacters, index, &ch );
		ERR_IF_PASSTHROUGH;

		if ( ! trotUnicodeIsWhitespace( ch ) )
		{
			break;
		}

		index += 1;
	}

	/* get first word */
	rc = trotListInit( &lWord );
	ERR_IF_PASSTHROUGH;

	while ( index <= charactersCount )
	{
		rc = trotListGetInt( lCharacters, index, &ch );
		ERR_IF_PASSTHROUGH;

		if ( trotUnicodeIsWhitespace( ch ) )
		{
			break;
		}

		rc = trotListAppendInt( lWord, ch );
		ERR_IF_PASSTHROUGH;

		index += 1;
	}

	rc = trotListGetCount( lWord, &wordCount );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* first word must be '[' */

	/* must be 1 for [ */
	ERR_IF_1( wordCount != 1, TROT_RC_ERROR_DECODE, wordCount );

	/* get character */
	rc = trotListGetInt( lWord, 1, &ch );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* must be [ */
	ERR_IF_1( ch != '[', TROT_RC_ERROR_DECODE, ch );

	/* skip whitespace */
	while ( index <= charactersCount )
	{
		rc = trotListGetInt( lCharacters, index, &ch );
		ERR_IF_PASSTHROUGH;

		if ( ! trotUnicodeIsWhitespace( ch ) )
		{
			break;
		}

		index += 1;
	}

	/* decode rest of characters */
	while ( 1 )
	{
		/* get next word */
		trotListFree( &lWord );
		rc = trotListInit( &lWord );
		ERR_IF_PASSTHROUGH;

		while ( index <= charactersCount )
		{
			rc = trotListGetInt( lCharacters, index, &ch );
			ERR_IF_PASSTHROUGH;

			if ( trotUnicodeIsWhitespace( ch ) )
			{
				break;
			}

			rc = trotListAppendInt( lWord, ch );
			ERR_IF_PASSTHROUGH;

			index += 1;
		}

		rc = trotListGetCount( lWord, &wordCount );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		/* go ahead and skip whitespace */
		while ( index <= charactersCount )
		{
			rc = trotListGetInt( lCharacters, index, &ch );
			ERR_IF_PASSTHROUGH;

			if ( ! trotUnicodeIsWhitespace( ch ) )
			{
				break;
			}

			index += 1;
		}

		/* get first character of word */
		rc = trotListGetInt( lWord, 1, &ch );
		ERR_IF_PASSTHROUGH;

		/* if [ */
		if ( ch == '[' )
		{
			/* count must be 1 */
			ERR_IF_1( wordCount != 1, TROT_RC_ERROR_DECODE, wordCount );

			/* create new list */
			trotListFree( &lChild );
			rc = trotListInit( &lChild );
			ERR_IF_PASSTHROUGH;

			/* add new list to current list */
			rc = trotListAppendList( lCurrent, lChild );
			ERR_IF_PASSTHROUGH;

			/* push current list */
			rc = trotListAppendList( lStack, lCurrent );
			ERR_IF_PASSTHROUGH;

			/* switchup lCurrent and lChild ... "go down" */
			trotListFree( &lCurrent );
			lCurrent = lChild;
			lChild = NULL;
		}
		/* if ] */
		else if ( ch == ']' )
		{
			/* count must be 1 */
			ERR_IF_1( wordCount != 1, TROT_RC_ERROR_DECODE, wordCount );

			/* if we're at the end of our characters, we're done */
			if ( index > charactersCount )
			{
				break;
			}

			/* stack must not be empty */
			rc = trotListGetCount( lStack, &stackCount );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			ERR_IF_1( stackCount == 0, TROT_RC_ERROR_DECODE, stackCount );

			/* pop off stack */
			trotListFree( &lCurrent );
			rc = trotListRemoveList( lStack, -1, &lCurrent );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );
		}
		/* if tilde */
		else if ( ch == '~' )
		{
			/* remove the tilde */
			rc = trotListRemove( lWord, 1 );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			/* the rest of the word must be a number */
			rc = wordToNumber( lWord, &number );
			ERR_IF_PASSTHROUGH;

			/* set tag */
			rc = trotListSetTag( lCurrent, number );
			ERR_IF_PASSTHROUGH;
		}
		/* if backtick */
		else if ( ch == '`' )
		{
			/* remove the tilde */
			rc = trotListRemove( lWord, 1 );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			/* the rest of the word must be a number */
			rc = wordToNumber( lWord, &number );
			ERR_IF_PASSTHROUGH;

			/* set tag */
			rc = trotListSetUserTag( lCurrent, number );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );
		}
		/* if @ */
		else if ( ch == '@' )
		{
			/* split */
			trotListFree( &lPartList );
			rc = splitList( lWord, '.', &lPartList );
			ERR_IF_PASSTHROUGH;

			/* get reference */
			trotListFree( &lChild );
			rc = getReferenceList( lTop, lPartList, &lChild );
			ERR_IF_PASSTHROUGH;

			/* add to current */
			rc = trotListAppendList( lCurrent, lChild );
			ERR_IF_PASSTHROUGH;
		}
		/* else, must be number */
		else
		{
			/* the rest of the word must be a number */
			rc = wordToNumber( lWord, &number );
			ERR_IF_PASSTHROUGH;

			rc = trotListAppendInt( lCurrent, number );
			ERR_IF_PASSTHROUGH;
		}

	}

	/* stack must be empty */
	rc = trotListGetCount( lStack, &stackCount );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	ERR_IF_1( stackCount != 0, TROT_RC_ERROR_DECODE, stackCount );


	/* give back */
	(*lDecodedList_A) = lTop;
	lTop = NULL;


	/* CLEANUP */
	cleanup:

	trotListFree( &lTop );
	trotListFree( &lCurrent );
	trotListFree( &lWord );
	trotListFree( &lStack );
	trotListFree( &lPartList );
	trotListFree( &lChild );

	return rc;
}

/******************************************************************************/
/*!
	\brief Creates a new list of parts that were separated out of l.
	\param[in] listToSplit List that contains parts.
	\param[in] separator Separating character
	\param[out] lPartList_A On success, contains the parts.
	\return TROT_RC

	listToSplit is not modified.
	listToSplit must only contain characters, not lists.

	lPartList_A is created, and caller is responsible for freeing.

	Example:
	IN:
		listToSplit = ["abc.def.ghi"]
		separator = '.'
	OUT:
		listToSplit = ["abc.def.ghi"]
		lPartList_A will be [["abc"]["def"]["ghi"]]
*/
static TROT_RC splitList( TrotList *listToSplit, TROT_INT separator, TrotList **lPartList_A )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TROT_INT count = 0;
	TROT_INT index = 0;
	TROT_INT character = 0;

	TrotList *newLPartList = NULL;
	TrotList *lPart = NULL;


	/* PRECOND */
	PARANOID_ERR_IF( listToSplit == NULL );
	PARANOID_ERR_IF( lPartList_A == NULL );
	PARANOID_ERR_IF( (*lPartList_A) != NULL );


	/* CODE */
	/* create giveback */
	rc = trotListInit( &newLPartList );
	ERR_IF_PASSTHROUGH;

	rc = trotListInit( &lPart );
	ERR_IF_PASSTHROUGH;

	rc = trotListAppendList( newLPartList, lPart );
	ERR_IF_PASSTHROUGH;

	/* get count */
	rc = trotListGetCount( listToSplit, &count );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* foreach character */
	index = 1;
	while ( index <= count )
	{
		/* get next character */
		rc = trotListGetInt( listToSplit, index, &character );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		/* if separator */
		if ( character == separator )
		{
			trotListFree( &lPart );

			rc = trotListInit( &lPart );
			ERR_IF_PASSTHROUGH;

			rc = trotListAppendList( newLPartList, lPart );
			ERR_IF_PASSTHROUGH;
		}
		/* else add to current part */
		else
		{
			rc = trotListAppendInt( lPart, character );
			ERR_IF_PASSTHROUGH;
		}

		/* increment index */
		index += 1;
	}

	/* give back */
	(*lPartList_A) = newLPartList;
	newLPartList = NULL;


	/* CLEANUP */
	cleanup:

	trotListFree( &newLPartList );
	trotListFree( &lPart );

	return rc;
}

/******************************************************************************/
/*!
	\brief Converts a word into a number.
	\param[in] lWord The list to convert
	\param[out] number On success, the number.
	\return TROT_RC

	lWord must not contain lists, only ints.

	Example:
	If lWord is ["123"] number would be 123.
*/
static TROT_RC wordToNumber( TrotList *lWord, TROT_INT *number )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TROT_INT index = 0;
	TROT_INT count = 0;

	TROT_INT character = 0;
	TROT_INT newNumber = 0;

	TROT_INT i = 0;


	/* PRECOND */
	PARANOID_ERR_IF( lWord == NULL );
	PARANOID_ERR_IF( number == NULL );


	/* CODE */
	/* get count */
	rc = trotListGetCount( lWord, &count );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* get first character */
	rc = trotListGetInt( lWord, 1, &character );
	ERR_IF_PASSTHROUGH;

	/* make sure it's a good format */
	index = 1;
	if ( character == '-' )
	{
		index += 1;
	}

	rc = trotListGetInt( lWord, index, &character );
	ERR_IF_PASSTHROUGH;

	ERR_IF_1( ( ! ( character >= '0' && character <= '9' ) ), TROT_RC_ERROR_DECODE, character );

	ERR_IF_1( character == '0' && count != 1, TROT_RC_ERROR_DECODE, count );

	index += 1;

	while ( index <= count )
	{
		rc = trotListGetInt( lWord, index, &character );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		ERR_IF_1( ( ! ( character >= '0' && character <= '9' ) ), TROT_RC_ERROR_DECODE, character );

		index += 1;
	}

	/* make sure number can fit into our TROT_INT */
	index = 1;

	rc = trotListGetInt( lWord, index, &character );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	if ( character == '-' )
	{
		ERR_IF_1( count > TROT_INT_MIN_STRING_LENGTH, TROT_RC_ERROR_DECODE, count );

		if ( count == TROT_INT_MIN_STRING_LENGTH )
		{
			index += 1;
			i = 1; /* to skip the '-' at the beginning of TROT_INT_MIN */

			while ( index <= count )
			{	
				rc = trotListGetInt( lWord, index, &character );
				PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

				ERR_IF ( character > TROT_INT_MIN_STRING[ i ], TROT_RC_ERROR_DECODE );

				if ( character < TROT_INT_MIN_STRING[ i ] )
				{
					break;
				}

				index += 1;
				i += 1;
			}
		}
	}
	else
	{
		ERR_IF_1( count > TROT_INT_MAX_STRING_LENGTH, TROT_RC_ERROR_DECODE, count );

		if ( count == TROT_INT_MAX_STRING_LENGTH )
		{
			i = 0;

			while ( index <= count )
			{	
				rc = trotListGetInt( lWord, index, &character );
				PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

				ERR_IF_1( character > TROT_INT_MAX_STRING[ i ], TROT_RC_ERROR_DECODE, character );

				if ( character < TROT_INT_MAX_STRING[ i ] )
				{
					break;
				}

				index += 1;
				i += 1;
			}
		}
	}

	/* get our number started */
	index = 1;

	rc = trotListGetInt( lWord, index, &character );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* if negative, get real first number */
	if ( character == '-' )
	{
		index += 1;

		rc = trotListGetInt( lWord, index, &character );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		newNumber = character - '0';
		newNumber *= -1;

		/* build rest of number */
		index += 1;

		while ( index <= count )
		{
			rc = trotListGetInt( lWord, index, &character );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );
	
			newNumber *= 10;
			newNumber -= character - '0';
	
			index += 1;
		}
	}
	else
	{
		newNumber = character - '0';

		/* build rest of number */
		index += 1;

		while ( index <= count )
		{
			rc = trotListGetInt( lWord, index, &character );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );
	
			newNumber *= 10;
			newNumber += character - '0';
	
			index += 1;
		}
	}

	/* give back */
	(*number) = newNumber;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Takes a textual-reference and retrieves the correct list out of lTop
	\param[in] lTop The top list.
	\param[in] lPartList List that contains the parts of the textual-reference
	\param[out] lReference_A On success, the list that the textual-reference
		specified
	\return TROT_RC

	Example:
	If lTop is [ 85 [ 86 ] [ [ 87 ] ] ]
	and lPartList is [ [ "@" ] [ "3" ] [ "1" ] ]
	then lReference_A would the the list that's [ 87 ].
*/
static TROT_RC getReferenceList( TrotList *lTop, TrotList *lPartList, TrotList **lReference_A )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TROT_INT partListCount = 0;
	TROT_INT partListIndex = 0;

	TrotList *lPart = NULL;
	TROT_INT partNumber = 0;
	TROT_INT partCount = 0;

	TrotList *lParent = NULL;
	TrotList *lChild = NULL;


	/* PRECOND */
	PARANOID_ERR_IF( lTop == NULL );
	PARANOID_ERR_IF( lPartList == NULL );
	PARANOID_ERR_IF( lReference_A == NULL );
	PARANOID_ERR_IF( (*lReference_A) != NULL );


	/* CODE */
	/* get partListCount */
	rc = trotListGetCount( lPartList, &partListCount );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* get first part */
	rc = trotListGetList( lPartList, 1, &lPart );
	ERR_IF_PASSTHROUGH;

	/* first part must be '@'
	   we already know it starts with '@', so just make sure there's nothing
	   else in the part */
	rc = trotListGetCount( lPart, &partCount );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	ERR_IF_1( partCount != 1, TROT_RC_ERROR_DECODE, partCount );

	/* start parent */
	rc = trotListTwin( lTop, &lParent );
	ERR_IF_PASSTHROUGH;

	/* for each part, starting at 2 */
	partListIndex = 2;
	while ( partListIndex <= partListCount )
	{
		/* get part */
		trotListFree( &lPart );
		rc = trotListGetList( lPartList, partListIndex, &lPart );
		ERR_IF_PASSTHROUGH;

		/* part into number */
		rc = wordToNumber( lPart, &partNumber );
		ERR_IF_PASSTHROUGH;

		/* get child of parent */
		trotListFree( &lChild );
		rc = trotListGetList( lParent, partNumber, &lChild );
		ERR_IF_PASSTHROUGH;

		/* switchup parent and child, "go down" */
		trotListFree( &lParent );
		lParent = lChild;
		lChild = NULL;

		/* increment */
		partListIndex += 1;
	}


	/* give back */
	(*lReference_A) = lParent;
	lParent = NULL;


	/* CLEANUP */
	cleanup:

	trotListFree( &lPart );
	trotListFree( &lParent );
	trotListFree( &lChild );

	return rc;
}

