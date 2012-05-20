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
	Tokenizer.
*/

/******************************************************************************/
#include "trot.h"
#include "trotInternal.h"

/******************************************************************************/
static TROT_RC trotUpgradeWordToNumber( trotListRef *lrToken );
static TROT_RC trotSkipComments( trotListRef *lrCharacters, INT_TYPE count, INT_TYPE *i, INT_TYPE *line, INT_TYPE *column );

/******************************************************************************/
/*!
	\brief Tokenizes a list of characters.
	\param lrCharacters List of characters.
	\param lrTokens_A On success, list of tokens
	\return TROT_RC
*/
TROT_RC trotTokenize( trotListRef *lrCharacters, trotListRef **lrTokenList_A )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotListRef *newLrTokenList = NULL;

	trotListRef *lrToken = NULL;

	trotListRef *lrValue = NULL;

	INT_TYPE count = 0;
	INT_TYPE i = 0;

	INT_TYPE line = 1;
	INT_TYPE column = 1;

	INT_TYPE previousCharacter = 0;
	INT_TYPE character = 0;

	int flagPreviousWasCR = 0;


	/* PRECOND */
	PRECOND_ERR_IF( lrCharacters == NULL );
	PRECOND_ERR_IF( lrTokenList_A == NULL );
	PRECOND_ERR_IF( (*lrTokenList_A) != NULL );


	/* CODE */
	/* create our giveback */
	rc = trotListRefInit( &newLrTokenList );
	ERR_IF_PASSTHROUGH;

	/* get count */
	rc = trotListRefGetCount( lrCharacters, &count );
	ERR_IF_PASSTHROUGH;

	/* foreach character */
	i = 1;
	while ( i <= count )
	{
		/* get character */
		rc = trotListRefGetInt( lrCharacters, i, &character );
		ERR_IF_PASSTHROUGH;

/* TODO: get list of all unicode whitespace */
		/* if whitespace */
		if (
			   character == ' '
			|| character == '\t'
			|| character == '\n'
			|| character == '\r'
		   )
		{
			/* keep track of lines */
			if ( character == '\n' )
			{
				if ( ! flagPreviousWasCR )
				{
					line += 1;
				}
				column = 0; /* this will be incremented to 1 below */

				flagPreviousWasCR = 0;
			}
			else if ( character == '\r' )
			{
				line += 1;
				column = 0; /* this will be incremented to 1 below */

				flagPreviousWasCR = 1;
			}
		}
		/* if [ */
		else if ( character == '[' )
		{
			rc = trotCreateToken( line, column, TOKEN_L_BRACKET, &lrToken );
			ERR_IF_PASSTHROUGH;
		}
		/* if ] */
		else if ( character == ']' )
		{
			rc = trotCreateToken( line, column, TOKEN_R_BRACKET, &lrToken );
			ERR_IF_PASSTHROUGH;
		}
		/* if ( */
		else if ( character == '(' )
		{
			rc = trotCreateToken( line, column, TOKEN_L_PARENTHESIS, &lrToken );
			ERR_IF_PASSTHROUGH;
		}
		/* if ) */
		else if ( character == ')' )
		{
			rc = trotCreateToken( line, column, TOKEN_R_PARENTHESIS, &lrToken );
			ERR_IF_PASSTHROUGH;
		}
		/* if { */
		else if ( character == '{' )
		{
			rc = trotCreateToken( line, column, TOKEN_L_BRACE, &lrToken );
			ERR_IF_PASSTHROUGH;
		}
		/* if } */
		else if ( character == '}' )
		{
			rc = trotCreateToken( line, column, TOKEN_R_BRACE, &lrToken );
			ERR_IF_PASSTHROUGH;
		}
		/* if " */
		else if ( character == '"' )
		{
			/* create token */
			rc = trotCreateToken( line, column, TOKEN_STRING, &lrToken );
			ERR_IF_PASSTHROUGH;

			/* create value */
			rc = trotListRefInit( &lrValue );
			ERR_IF_PASSTHROUGH;

			/* get value of string */
			column += 1;
			i += 1;

			/* there must be at least one character after the quote */
			ERR_IF( i > count, TROT_LIST_ERROR_DECODE );

			while ( i <= count )
			{
				/* get character */
				rc = trotListRefGetInt( lrCharacters, i, &character );
				ERR_IF_PASSTHROUGH;
				
				/* if " */
				if ( character == '"' )
				{
					break;
				}
				/* keep track of lines */
				else if ( character == '\n' )
				{
					if ( ! flagPreviousWasCR )
					{
						line += 1;
					}
					column = 0; /* this will be incremented to 1 below */

					flagPreviousWasCR = 0;
				}
				else if ( character == '\r' )
				{
					line += 1;
					column = 0; /* this will be incremented to 1 below */

					flagPreviousWasCR = 1;
				}

				/* append character */
				rc = trotListRefAppendInt( lrValue, character );
				ERR_IF_PASSTHROUGH;

				/* next */
				column += 1;
				i += 1;
			}

			ERR_IF ( character != '"', TROT_LIST_ERROR_DECODE )

			/* strings cannot touch strings */
			if ( (i + 1) <= count )
			{
				/* get next character */
				rc = trotListRefGetInt( lrCharacters, i + 1, &character );
				ERR_IF_PASSTHROUGH;
				
				/* strings cannot touch words */
				ERR_IF ( character == '"', TROT_LIST_ERROR_DECODE );
			}

			/* append value */
			rc = trotListRefAppendListTwin( lrToken, lrValue );
			ERR_IF_PASSTHROUGH;

			trotListRefFree( &lrValue );
		}
		/* if # */
		else if ( character == '#' )
		{
			rc = trotSkipComments( lrCharacters, count, &i, &line, &column );
			ERR_IF_PASSTHROUGH;
		}
		/* else word or number */
		else 
		{
			/* strings cannot touch words */
			if ( (i - 1) >= 1 )
			{
				/* get previous character */
				rc = trotListRefGetInt( lrCharacters, i - 1, &previousCharacter );
				ERR_IF_PASSTHROUGH;

				ERR_IF( previousCharacter == '"', TROT_LIST_ERROR_DECODE );
			}

			/* create token */
			rc = trotCreateToken( line, column, TOKEN_WORD, &lrToken );
			ERR_IF_PASSTHROUGH;

			/* create value */
			rc = trotListRefInit( &lrValue );
			ERR_IF_PASSTHROUGH;

			/* append character */
			rc = trotListRefAppendInt( lrValue, character );
			ERR_IF_PASSTHROUGH;

			/* get rest of value */
			while ( (i + 1) <= count )
			{
				/* get next character */
				rc = trotListRefGetInt( lrCharacters, i + 1, &character );
				ERR_IF_PASSTHROUGH;
				
				/* strings cannot touch words */
				ERR_IF ( character == '"', TROT_LIST_ERROR_DECODE );

				/* if end of word */
				if (
					/* whitespace */
					   character == ' '
					|| character == '\t'
					|| character == '\n'
					|| character == '\r'
					/* list character */
					|| character == '['
					|| character == ']'
					|| character == '('
					|| character == ')'
					|| character == '{'
					|| character == '}'
				   )
				{
					break;
				}

				/* *** */
				column += 1;
				i += 1;

				/* append character */
				rc = trotListRefAppendInt( lrValue, character );
				ERR_IF_PASSTHROUGH;
			}

			/* append value */
			rc = trotListRefAppendListTwin( lrToken, lrValue );
			ERR_IF_PASSTHROUGH;

			trotListRefFree( &lrValue );

			/* is word a number? */
			rc = trotUpgradeWordToNumber( lrToken );
			ERR_IF_PASSTHROUGH;
		}

		/* add token */
		if ( lrToken != NULL )
		{
			rc = trotListRefAppendListTwin( newLrTokenList, lrToken );
			ERR_IF_PASSTHROUGH;

			trotListRefFree( &lrToken );
		}

		/* next */
		column += 1;
		i += 1;
	}

	/* give back */
	(*lrTokenList_A) = newLrTokenList;
	newLrTokenList = NULL;


	/* CLEANUP */
	cleanup:

	trotListRefFree( &newLrTokenList );
	trotListRefFree( &lrToken );
	trotListRefFree( &lrValue );

	return rc;
}

/******************************************************************************/
/*!
	\brief Creates a token
	\param line Start line of token.
	\param column Start column of token.
	\param lrToken_A On success, new token.
	\return TROT_RC
*/
TROT_RC trotCreateToken( INT_TYPE line, INT_TYPE column, INT_TYPE tokenType, trotListRef **lrToken_A )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotListRef *newToken = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( lrToken_A == NULL );
	PRECOND_ERR_IF( (*lrToken_A) != NULL );


	/* CODE */
	/* create new token */
	rc = trotListRefInit( &newToken );
	ERR_IF_PASSTHROUGH;

	/* add line, column, tokenType */
	rc = trotListRefAppendInt( newToken, line );
	ERR_IF_PASSTHROUGH;
	rc = trotListRefAppendInt( newToken, column );
	ERR_IF_PASSTHROUGH;
	rc = trotListRefAppendInt( newToken, tokenType );
	ERR_IF_PASSTHROUGH;

	/* give back */
	(*lrToken_A) = newToken;
	newToken = NULL;


	/* CLEANUP */
	cleanup:

	trotListRefFree( &newToken );

	return rc;
}

/******************************************************************************/
/*!
	\brief 
	\param 
	\return TROT_RC
*/
TROT_RC _trotWordToNumber( trotListRef *lrWord, int *isNumber, INT_TYPE *number )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	INT_TYPE index = 0;
	INT_TYPE count = 0;

	INT_TYPE character = 0;
	INT_TYPE newNumber = 0;

	int i = 0;


	/* PRECOND */
	PRECOND_ERR_IF( lrWord == NULL );
	PRECOND_ERR_IF( isNumber == NULL );
	PRECOND_ERR_IF( number == NULL );


	/* CODE */
	/* mark no number initially */
	(*isNumber) = 0;

	/* get count */
	rc = trotListRefGetCount( lrWord, &count );
	ERR_IF_PASSTHROUGH;

	/* get first character */
	rc = trotListRefGetInt( lrWord, 1, &character );
	ERR_IF_PASSTHROUGH;

	/* SPECIAL CASE: if just '-', then it's a word */
	if ( count == 1 && character == '-' )
	{
		goto cleanup;
	}

	/* if begin with '-' or 0-9, then it's a number */
	if ( ! ( character == '-' || ( character >= '0' && character <= '9' ) ) )
	{
		goto cleanup;
	}

	/* it's a number, so now it must be in a proper format */
	(*isNumber) = 1;

	/* make sure it's a good format */
	index = 1;
	if ( character == '-' )
	{
		index += 1;
	}

	rc = trotListRefGetInt( lrWord, index, &character );
	ERR_IF_PASSTHROUGH;

	ERR_IF( ( ! ( character >= '0' && character <= '9' ) ), TROT_LIST_ERROR_DECODE );

	ERR_IF( character == '0' && count != 1, TROT_LIST_ERROR_DECODE );

	index += 1;

	while ( index <= count )
	{
		rc = trotListRefGetInt( lrWord, index, &character );
		ERR_IF_PASSTHROUGH;

		ERR_IF( ( ! ( character >= '0' && character <= '9' ) ), TROT_LIST_ERROR_DECODE );

		index += 1;
	}

	/* make sure number can fit into our INT_TYPE */
	index = 1;

	rc = trotListRefGetInt( lrWord, index, &character );
	ERR_IF_PASSTHROUGH;

	if ( character == '-' )
	{
		ERR_IF( count > INT_TYPE_MIN_STRING_LENGTH, TROT_LIST_ERROR_DECODE );

		if ( count == INT_TYPE_MIN_STRING_LENGTH )
		{
			index += 1;
			i = 1; /* to skip the '-' at the beginning of INT_TYPE_MIN */

			while ( index <= count )
			{	
				rc = trotListRefGetInt( lrWord, index, &character );
				ERR_IF_PASSTHROUGH;

				ERR_IF ( character > INT_TYPE_MIN_STRING[ i ], TROT_LIST_ERROR_DECODE );

				if ( character < INT_TYPE_MIN_STRING[ i ] )
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
		ERR_IF( count > INT_TYPE_MAX_STRING_LENGTH, TROT_LIST_ERROR_DECODE );

		if ( count == INT_TYPE_MAX_STRING_LENGTH )
		{
			i = 0;

			while ( index <= count )
			{	
				rc = trotListRefGetInt( lrWord, index, &character );
				ERR_IF_PASSTHROUGH;

				ERR_IF ( character > INT_TYPE_MAX_STRING[ i ], TROT_LIST_ERROR_DECODE );

				if ( character < INT_TYPE_MAX_STRING[ i ] )
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

	rc = trotListRefGetInt( lrWord, index, &character );
	ERR_IF_PASSTHROUGH;

	/* if negative, get real first number */
	if ( character == '-' )
	{
		index += 1;

		rc = trotListRefGetInt( lrWord, index, &character );
		ERR_IF_PASSTHROUGH;

		newNumber = character - '0';
		newNumber *= -1;

		/* build rest of number */
		index += 1;

		while ( index <= count )
		{
			rc = trotListRefGetInt( lrWord, index, &character );
			ERR_IF_PASSTHROUGH;
	
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
			rc = trotListRefGetInt( lrWord, index, &character );
			ERR_IF_PASSTHROUGH;
	
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
	\brief Checks to see if a word is a number, and if so changes the token.
	\param lrToken Token
	\return TROT_RC
*/
static TROT_RC trotUpgradeWordToNumber( trotListRef *lrToken )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotListRef *lrValue = NULL;
	int isNumber = 0;
	INT_TYPE number = 0;


	/* PRECOND */
	PRECOND_ERR_IF( lrToken == NULL );


	/* CODE */
	/* get value */
	rc = trotListRefGetListTwin( lrToken, TOKEN_INDEX_VALUE, &lrValue );
	ERR_IF_PASSTHROUGH;

	/* is number? */
	rc = _trotWordToNumber( lrValue, &isNumber, &number );
	ERR_IF_PASSTHROUGH;

	if ( isNumber == 1 )
	{
		/* mark token as a number */
		/* TODO: replace this with replace */
		rc = trotListRefRemove( lrToken, TOKEN_INDEX_TYPE );
		ERR_IF_PASSTHROUGH;

		rc = trotListRefInsertInt( lrToken, TOKEN_INDEX_TYPE, TOKEN_NUMBER );
		ERR_IF_PASSTHROUGH;

		/* remove old string value and replace with number value */
		rc = trotListRefRemove( lrToken, TOKEN_INDEX_VALUE );
		ERR_IF_PASSTHROUGH;

		rc = trotListRefInsertInt( lrToken, TOKEN_INDEX_VALUE, number );
		ERR_IF_PASSTHROUGH;
	}
	

	/* CLEANUP */
	cleanup:

	trotListRefFree( &lrValue );

	return rc;
}

/******************************************************************************/
/*!
	\brief 
	\param 
	\return TROT_RC
*/
static TROT_RC trotSkipComments( trotListRef *lrCharacters, INT_TYPE count, INT_TYPE *i, INT_TYPE *line, INT_TYPE *column )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	INT_TYPE character = 0;
	INT_TYPE previousCharacter = 0;

	int commentDepth = 0;


	/* PRECOND */
	PRECOND_ERR_IF( lrCharacters == NULL );
	PRECOND_ERR_IF( i == NULL );
	PRECOND_ERR_IF( line == NULL  );
	PRECOND_ERR_IF( column == NULL );


	/* CODE */
	/* next */
	(*column) += 1;
	(*i) += 1;

	if ( (*i) > count )
	{
		goto cleanup;
	}

	/* get character */
	previousCharacter = character;
	rc = trotListRefGetInt( lrCharacters, (*i), &character );
	ERR_IF_PASSTHROUGH;

	/* can't be ')' without a beginning '(' */
	ERR_IF( character == ')', TROT_LIST_ERROR_DECODE );

	/* if multi-line comment */
	if ( character == '(' )
	{
		commentDepth = 1;

		/* skip comment */
		while ( 1 )
		{
			/* increment */
			(*column) += 1;
			(*i) += 1;

			/* are we past the end? */
			/* this is an error because we shouldn't get to the end
			   before we get to the last #) */
			ERR_IF( (*i) > count, TROT_LIST_ERROR_DECODE );

			/* get character */
			previousCharacter = character;
			rc = trotListRefGetInt( lrCharacters, (*i), &character );
			ERR_IF_PASSTHROUGH;

			if ( previousCharacter == '#' )
			{
				if ( character == ')' )
				{
					commentDepth -= 1;

					if ( commentDepth == 0 )
					{
						break;
					}
				}
				else if ( character == '(' )
				{
					commentDepth += 1;
				}
			}

			/* keep track of lines */
			if ( character == '\n' )
			{
				if ( previousCharacter != '\r' )
				{
					(*line) += 1;
				}
				(*column) = 0; /* this will be incremented to 1 below */
			}
			else if ( character == '\r' )
			{
				(*line) += 1;
				(*column) = 0; /* this will be incremented to 1 below */
			}
		}
	}
	/* # to end of line comment */
	else
	{
		/* skip comment */
		while ( 1 )
		{
			/* increment */
			(*column) += 1;
			(*i) += 1;

			/* are we past the end? */
			if ( (*i) > count )
			{
				break;
			}

			/* get character */
			rc = trotListRefGetInt( lrCharacters, (*i), &character );
			ERR_IF_PASSTHROUGH;

			/* if end of line */
			if ( character == '\n' || character == '\r' )
			{
				/* set i back 1, so we'll handle the newline in caller */
				(*i) -= 1;

				break;
			}
		}
	}


	/* CLEANUP */
	cleanup:

	return rc;
}

