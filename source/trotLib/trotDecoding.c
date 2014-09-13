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
#undef  TROT_FILE_NUMBER
#define TROT_FILE_NUMBER 5

/******************************************************************************/
#include "trot.h"
#include "trotInternal.h"

/******************************************************************************/
static TROT_RC skipWhitespace( TrotProgram *program, TrotList *lCharacters, TROT_INT charactersCount, TROT_INT *index, s32 mustBeOne );
static TROT_RC getWord( TrotProgram *program, TrotList *lCharacters, TROT_INT charactersCount, TROT_INT *index, TrotList **lWord_A );
static TROT_RC wordToNumber( TrotProgram *program, TrotList *lWord, TROT_INT *number );
static TROT_RC splitList( TrotProgram *program, TrotList *listToSplit, TROT_INT separator, TrotList **lPartList_A );
static TROT_RC getReferenceList( TrotProgram *program, TrotList *lTop, TrotList *lPartList, TrotList **lReference_A );

/******************************************************************************/
/*!
	\brief Decodes a list of characters into a list.
	\param[in] program List that maintains memory limit
	\param[in] lCharacters Characters to decode.
	\param[out] lDecodedList_A On success, the decoded list.
	\return TROT_RC

	lCharacters is not modified.
	lDecodedList_A is created, and caller is responsible for freeing.
*/
TROT_RC trotDecode( TrotProgram *program, TrotList *lCharacters, TrotList **lDecodedList_A )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TROT_INT charactersCount = 0;
	TROT_INT index = 1;

	TrotList *lTop = NULL;
	TrotList *lCurrent = NULL;
	TrotList *lChild = NULL;

	TrotList *lWord = NULL;
	TROT_INT ch = 0;

	TROT_INT number = 0;

	TrotList *lStack = NULL;
	TROT_INT stackCount = 0;

	TrotList *lPartList = NULL;


	/* PRECOND */
	FAILURE_POINT;
	PARANOID_ERR_IF( program == NULL );
	PARANOID_ERR_IF( lCharacters == NULL );
	PARANOID_ERR_IF( lDecodedList_A == NULL );
	PARANOID_ERR_IF( (*lDecodedList_A) != NULL );


	/* CODE */
	/* get count of characters */
	rc = trotListGetCount( program, lCharacters, &charactersCount );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* create "top" list */
	rc = trotListInit( program, &lTop );
	ERR_IF_PASSTHROUGH;

	/* create current list */
	rc = trotListTwin( program, lTop, &lCurrent );
	ERR_IF_PASSTHROUGH;

	/* create our stack */
	rc = trotListInit( program, &lStack );
	ERR_IF_PASSTHROUGH;


	/* skip whitespace */
	rc = skipWhitespace( program, lCharacters, charactersCount, &index, 0 );
	ERR_IF_PASSTHROUGH;


	/* get first character */
	rc = trotListGetInt( program, lCharacters, index, &ch );
	ERR_IF_PASSTHROUGH;

	/* must be [ */
	ERR_IF_1( ch != '[', TROT_RC_ERROR_DECODE, ch );

	/* skip past [ */
	index += 1;


	/* decode rest of characters */
	while ( 1 )
	{
		/* skip whitespace */
		rc = skipWhitespace( program, lCharacters, charactersCount, &index, 1 );
		ERR_IF_PASSTHROUGH;
		
		/* get next character */
		rc = trotListGetInt( program, lCharacters, index, &ch );
		ERR_IF_PASSTHROUGH;

		/* if left bracket, create new child list and "go down" into it */
		if ( ch == '[' )
		{
			/* skip bracket */
			index += 1;

			/* create new list */
			trotListFree( program, &lChild );
			rc = trotListInit( program, &lChild );
			ERR_IF_PASSTHROUGH;

			/* add new list to current list */
			rc = trotListAppendList( program, lCurrent, lChild );
			ERR_IF_PASSTHROUGH;

			/* push current list */
			rc = trotListAppendList( program, lStack, lCurrent );
			ERR_IF_PASSTHROUGH;

			/* switchup lCurrent and lChild ... "go down" */
			trotListFree( program, &lCurrent );
			lCurrent = lChild;
			lChild = NULL;
		}
		/* if right bracket, "go up" to parent */
		else if ( ch == ']' )
		{
			/* skip bracket */
			index += 1;

			/* is stack empty? */
			rc = trotListGetCount( program, lStack, &stackCount );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			if ( stackCount == 0 )
			{
				break;
			}

			/* pop off stack ... "go up" */
			trotListFree( program, &lCurrent );
			rc = trotListRemoveList( program, lStack, -1, &lCurrent );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );
		}
		/* if tilde, set type */
		else if ( ch == '~' )
		{
			/* skip tilde */
			index += 1;

			/* get word */
			trotListFree( program, &lWord );
			rc = getWord( program, lCharacters, charactersCount, &index, &lWord );
			ERR_IF_PASSTHROUGH;

			/* word to number */
			rc = wordToNumber( program, lWord, &number );
			ERR_IF_PASSTHROUGH;

			/* set tag */
			rc = trotListSetType( program, lCurrent, number );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );
		}
		/* if backtick, set tag */
		else if ( ch == '`' )
		{
			/* skip backtick */
			index += 1;

			/* get word */
			trotListFree( program, &lWord );
			rc = getWord( program, lCharacters, charactersCount, &index, &lWord );
			ERR_IF_PASSTHROUGH;

			/* word to number */
			rc = wordToNumber( program, lWord, &number );
			ERR_IF_PASSTHROUGH;

			/* set tag */
			rc = trotListSetTag( program, lCurrent, number );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );
		}
		/* if @, read in reference, and twin a previously-seen list */
		else if ( ch == '@' )
		{
			/* get word */
			trotListFree( program, &lWord );
			rc = getWord( program, lCharacters, charactersCount, &index, &lWord );
			ERR_IF_PASSTHROUGH;

			/* split */
			trotListFree( program, &lPartList );
			rc = splitList( program, lWord, '.', &lPartList );
			ERR_IF_PASSTHROUGH;

			/* get reference */
			trotListFree( program, &lChild );
			rc = getReferenceList( program, lTop, lPartList, &lChild );
			ERR_IF_PASSTHROUGH;

			/* add to current */
			rc = trotListAppendList( program, lCurrent, lChild );
			ERR_IF_PASSTHROUGH;
		}
		/* else, must be number */
		else
		{
			/* get word */
			trotListFree( program, &lWord );
			rc = getWord( program, lCharacters, charactersCount, &index, &lWord );
			ERR_IF_PASSTHROUGH;

			/* word to number */
			rc = wordToNumber( program, lWord, &number );
			ERR_IF_PASSTHROUGH;

			/* add number to current list */
			rc = trotListAppendInt( program, lCurrent, number );
			ERR_IF_PASSTHROUGH;
		}
	}

	/* skip whitespace */
	rc = skipWhitespace( program, lCharacters, charactersCount, &index, 0 );
	ERR_IF_PASSTHROUGH;

	/* we must be at end of characters */
	ERR_IF( index != (charactersCount + 1), TROT_RC_ERROR_DECODE );


	/* give back */
	(*lDecodedList_A) = lTop;
	lTop = NULL;


	/* CLEANUP */
	cleanup:

	trotListFree( program, &lTop );
	trotListFree( program, &lCurrent );
	trotListFree( program, &lWord );
	trotListFree( program, &lStack );
	trotListFree( program, &lPartList );
	trotListFree( program, &lChild );

	return rc;
}

/******************************************************************************/
/*!
	\brief Skips whitespace characters.
	\param[in] program List that maintains memory limit
	\param[in] lCharacters List of characters.
	\param[in] charactersCount Count of characters.
	\param[in,out] index Current index into lCharacters.
	\param[in] mustBeOne Whether there must be at least 1 whitespace character.
	\return TROT_RC

	index will be incremented to first non-whitespace character, or 1 past end of list.
*/
static TROT_RC skipWhitespace( TrotProgram *program, TrotList *lCharacters, TROT_INT charactersCount, TROT_INT *index, s32 mustBeOne )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TROT_INT ch = 0;


	/* PRECOND */
	PARANOID_ERR_IF( program == NULL );
	PARANOID_ERR_IF( lCharacters == NULL );
	PARANOID_ERR_IF( index == NULL );


	/* CODE */
	if ( mustBeOne )
	{
		rc = trotListGetInt( program, lCharacters, (*index), &ch );
		ERR_IF_PASSTHROUGH;

		ERR_IF( ch != ' ', TROT_RC_ERROR_DECODE );

		(*index) += 1;
	}

	while ( (*index) <= charactersCount )
	{
		rc = trotListGetInt( program, lCharacters, (*index), &ch );
		ERR_IF_PASSTHROUGH;

		if ( ch != ' ' )
		{
			break;
		}

		(*index) += 1;
	}


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief Gets the next word in lCharacters.
	\param[in] program List that maintains memory limit
	\param[in] lCharacters List of characters.
	\param[in] charactersCount Count of characters.
	\param[in,out] index Current index into lCharacters.
	\param[out] lWord_A The next word.
	\return TROT_RC

	index will be incremented.
	lWord_A will be created. Caller is responsible for freeing.
*/
static TROT_RC getWord( TrotProgram *program, TrotList *lCharacters, TROT_INT charactersCount, TROT_INT *index, TrotList **lWord_A )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotList *newLWord = NULL;
	TROT_INT ch = 0;


	/* PRECOND */
	PARANOID_ERR_IF( program == NULL );
	PARANOID_ERR_IF( lCharacters == NULL );
	PARANOID_ERR_IF( index == NULL );
	PARANOID_ERR_IF( lWord_A == NULL );
	PARANOID_ERR_IF( (*lWord_A) != NULL );


	/* CODE */
	rc = trotListInit( program, &newLWord );
	ERR_IF_PASSTHROUGH;

	while ( (*index) <= charactersCount )
	{
		rc = trotListGetInt( program, lCharacters, (*index), &ch );
		ERR_IF_PASSTHROUGH;

		if ( ch == ' ' )
		{
			break;
		}

		rc = trotListAppendInt( program, newLWord, ch );
		ERR_IF_PASSTHROUGH;

		(*index) += 1;
	}


	/* give back */
	(*lWord_A) = newLWord;
	newLWord = NULL;


	/* CLEANUP */
	cleanup:

	trotListFree( program, &newLWord );

	return rc;
}

/******************************************************************************/
/*!
	\brief Converts a word into a number.
	\param[in] program List that maintains memory limit
	\param[in] lWord The list to convert
	\param[out] number On success, the number.
	\return TROT_RC

	lWord must not contain lists, only ints.

	Example:
	If lWord is ["123"] number would be 123.
*/
static TROT_RC wordToNumber( TrotProgram *program, TrotList *lWord, TROT_INT *number )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TROT_INT index = 0;
	TROT_INT count = 0;

	TROT_INT character = 0;
	TROT_INT newNumber = 0;

	TROT_INT i = 0;


	/* PRECOND */
	PARANOID_ERR_IF( program == NULL );
	PARANOID_ERR_IF( lWord == NULL );
	PARANOID_ERR_IF( number == NULL );


	/* CODE */
	/* get count */
	rc = trotListGetCount( program, lWord, &count );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* get first character */
	rc = trotListGetInt( program, lWord, 1, &character );
	ERR_IF_PASSTHROUGH;

	/* make sure it's a good format */
	index = 1;
	if ( character == '-' )
	{
		index += 1;
	}

	rc = trotListGetInt( program, lWord, index, &character );
	ERR_IF_PASSTHROUGH;

	ERR_IF_1( ( ! ( character >= '0' && character <= '9' ) ), TROT_RC_ERROR_DECODE, character );

	ERR_IF_1( character == '0' && count != 1, TROT_RC_ERROR_DECODE, count );

	index += 1;

	while ( index <= count )
	{
		rc = trotListGetInt( program, lWord, index, &character );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		ERR_IF_1( ( ! ( character >= '0' && character <= '9' ) ), TROT_RC_ERROR_DECODE, character );

		index += 1;
	}

	/* make sure number can fit into our TROT_INT */
	index = 1;

	rc = trotListGetInt( program, lWord, index, &character );
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
				rc = trotListGetInt( program, lWord, index, &character );
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
				rc = trotListGetInt( program, lWord, index, &character );
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

	rc = trotListGetInt( program, lWord, index, &character );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* if negative, get real first number */
	if ( character == '-' )
	{
		index += 1;

		rc = trotListGetInt( program, lWord, index, &character );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		newNumber = character - '0';
		newNumber *= -1;

		/* build rest of number */
		index += 1;

		while ( index <= count )
		{
			rc = trotListGetInt( program, lWord, index, &character );
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
			rc = trotListGetInt( program, lWord, index, &character );
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
	\brief Creates a new list of parts that were separated out of l.
	\param[in] program List that maintains memory limit
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
static TROT_RC splitList( TrotProgram *program, TrotList *listToSplit, TROT_INT separator, TrotList **lPartList_A )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TROT_INT count = 0;
	TROT_INT index = 0;
	TROT_INT character = 0;

	TrotList *newLPartList = NULL;
	TrotList *lPart = NULL;


	/* PRECOND */
	PARANOID_ERR_IF( program == NULL );
	PARANOID_ERR_IF( listToSplit == NULL );
	PARANOID_ERR_IF( lPartList_A == NULL );
	PARANOID_ERR_IF( (*lPartList_A) != NULL );


	/* CODE */
	/* create giveback */
	rc = trotListInit( program, &newLPartList );
	ERR_IF_PASSTHROUGH;

	rc = trotListInit( program, &lPart );
	ERR_IF_PASSTHROUGH;

	rc = trotListAppendList( program, newLPartList, lPart );
	ERR_IF_PASSTHROUGH;

	/* get count */
	rc = trotListGetCount( program, listToSplit, &count );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* foreach character */
	index = 1;
	while ( index <= count )
	{
		/* get next character */
		rc = trotListGetInt( program, listToSplit, index, &character );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		/* if separator */
		if ( character == separator )
		{
			trotListFree( program, &lPart );

			rc = trotListInit( program, &lPart );
			ERR_IF_PASSTHROUGH;

			rc = trotListAppendList( program, newLPartList, lPart );
			ERR_IF_PASSTHROUGH;
		}
		/* else add to current part */
		else
		{
			rc = trotListAppendInt( program, lPart, character );
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

	trotListFree( program, &newLPartList );
	trotListFree( program, &lPart );

	return rc;
}

/******************************************************************************/
/*!
	\brief Takes a textual-reference and retrieves the correct list out of lTop
	\param[in] program List that maintains memory limit
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
static TROT_RC getReferenceList( TrotProgram *program, TrotList *lTop, TrotList *lPartList, TrotList **lReference_A )
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
	PARANOID_ERR_IF( program == NULL );
	PARANOID_ERR_IF( lTop == NULL );
	PARANOID_ERR_IF( lPartList == NULL );
	PARANOID_ERR_IF( lReference_A == NULL );
	PARANOID_ERR_IF( (*lReference_A) != NULL );


	/* CODE */
	/* get partListCount */
	rc = trotListGetCount( program, lPartList, &partListCount );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* get first part */
	rc = trotListGetList( program, lPartList, 1, &lPart );
	ERR_IF_PASSTHROUGH;

	/* first part must be '@'
	   we already know it starts with '@', so just make sure there's nothing
	   else in the part */
	rc = trotListGetCount( program, lPart, &partCount );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	ERR_IF_1( partCount != 1, TROT_RC_ERROR_DECODE, partCount );

	/* start parent */
	rc = trotListTwin( program, lTop, &lParent );
	ERR_IF_PASSTHROUGH;

	/* for each part, starting at 2 */
	partListIndex = 2;
	while ( partListIndex <= partListCount )
	{
		/* get part */
		trotListFree( program, &lPart );
		rc = trotListGetList( program, lPartList, partListIndex, &lPart );
		ERR_IF_PASSTHROUGH;

		/* part into number */
		rc = wordToNumber( program, lPart, &partNumber );
		ERR_IF_PASSTHROUGH;

		/* must be positive */
		ERR_IF_1( partNumber <= 0, TROT_RC_ERROR_DECODE, partNumber );

		/* get child of parent */
		trotListFree( program, &lChild );
		rc = trotListGetList( program, lParent, partNumber, &lChild );
		ERR_IF_PASSTHROUGH;

		/* switchup parent and child, "go down" */
		trotListFree( program, &lParent );
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

	trotListFree( program, &lPart );
	trotListFree( program, &lParent );
	trotListFree( program, &lChild );

	return rc;
}

