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
static int trotIsWhitespace( INT_TYPE character );
static int trotIsNewline( INT_TYPE character );

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

	INT_TYPE previousCharacter = -1;
	INT_TYPE character = -1;


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
	PARANOID_ERR_IF( rc != TROT_LIST_SUCCESS );

	/* foreach character */
	i = 1;
	while ( i <= count )
	{
		/* get character */
		previousCharacter = character;
		rc = trotListRefGetInt( lrCharacters, i, &character );
		ERR_IF_PASSTHROUGH;

		/* if whitespace */
		if ( trotIsWhitespace( character ) )
		{
			/* keep track of lines */
			if ( trotIsNewline( character ) )
			{
				if ( ! ( character == '\n' && previousCharacter == '\r' ) )
				{
					line += 1;
				}

				column = 0;
			}
		}
		/* if [ */
		else if ( character == '[' )
		{
			rc = trotCreateToken( line, column, TOKEN_TYPE_L_BRACKET, &lrToken );
			ERR_IF_PASSTHROUGH;
		}
		/* if ] */
		else if ( character == ']' )
		{
			rc = trotCreateToken( line, column, TOKEN_TYPE_R_BRACKET, &lrToken );
			ERR_IF_PASSTHROUGH;
		}
		/* if ( */
		else if ( character == '(' )
		{
			rc = trotCreateToken( line, column, TOKEN_TYPE_L_PARENTHESIS, &lrToken );
			ERR_IF_PASSTHROUGH;
		}
		/* if ) */
		else if ( character == ')' )
		{
			rc = trotCreateToken( line, column, TOKEN_TYPE_R_PARENTHESIS, &lrToken );
			ERR_IF_PASSTHROUGH;
		}
		/* if { */
		else if ( character == '{' )
		{
			rc = trotCreateToken( line, column, TOKEN_TYPE_L_BRACE, &lrToken );
			ERR_IF_PASSTHROUGH;
		}
		/* if } */
		else if ( character == '}' )
		{
			rc = trotCreateToken( line, column, TOKEN_TYPE_R_BRACE, &lrToken );
			ERR_IF_PASSTHROUGH;
		}
		/* if " */
		else if ( character == '"' )
		{
			/* strings cannot touch strings */
			ERR_IF ( previousCharacter == '"', TROT_LIST_ERROR_DECODE );

			/* create token */
			rc = trotCreateToken( line, column, TOKEN_TYPE_STRING, &lrToken );
			ERR_IF_PASSTHROUGH;

			/* get value */
			trotListRefFree( &lrValue );
			rc = trotListRefGetListTwin( lrToken, TOKEN_INDEX_VALUE, &lrValue );
			ERR_IF_PASSTHROUGH;

			/* get value of string */
			column += 1;
			i += 1;

			/* there must be at least one character after the quote */
			ERR_IF( i > count, TROT_LIST_ERROR_DECODE );

			while ( i <= count )
			{
				/* get character */
				previousCharacter = character;
				rc = trotListRefGetInt( lrCharacters, i, &character );
				ERR_IF_PASSTHROUGH;
				
				/* if " */
				if ( character == '"' )
				{
					break;
				}
				/* keep track of lines */
				else if ( trotIsNewline( character ) )
				{
					if ( ! ( character == '\n' && previousCharacter == '\r' ) )
					{
						line += 1;
					}

					column = 0;
				}

				/* append character */
				rc = trotListRefAppendInt( lrValue, character );
				ERR_IF_PASSTHROUGH;

				/* next */
				column += 1;
				i += 1;
			}

			ERR_IF ( character != '"', TROT_LIST_ERROR_DECODE )
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
			ERR_IF( previousCharacter == '"', TROT_LIST_ERROR_DECODE );

			/* create token */
			rc = trotCreateToken( line, column, TOKEN_TYPE_WORD, &lrToken );
			ERR_IF_PASSTHROUGH;

			/* get value */
			trotListRefFree( &lrValue );
			rc = trotListRefGetListTwin( lrToken, TOKEN_INDEX_VALUE, &lrValue );
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
					   trotIsWhitespace( character )
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

	trotListRef *lr = NULL;


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

#if ( NODE_SIZE < 3 )
#error NODE_SIZE TOO SMALL, MUST CHANGE THE FOLLOWING TWO PARANOID_ERR_IFS TO REAL ERR_IFs
#endif

	rc = trotListRefAppendInt( newToken, column );
	PARANOID_ERR_IF( rc != TROT_LIST_SUCCESS );
	rc = trotListRefAppendInt( newToken, tokenType );
	PARANOID_ERR_IF( rc != TROT_LIST_SUCCESS );

	/* these need a value */
	if (    tokenType == TOKEN_TYPE_L_BRACKET
	     || tokenType == TOKEN_TYPE_L_BRACE
	     || tokenType == TOKEN_TYPE_L_PARENTHESIS
	     || tokenType == TOKEN_TYPE_STRING
	     || tokenType == TOKEN_TYPE_WORD
	     || tokenType == TOKEN_TYPE_NUMBER_RAW
	   )
	{
		/* add value */
		rc = trotListRefInit( &lr );
		ERR_IF_PASSTHROUGH;

		rc = trotListRefAppendListTwin( newToken, lr );
		ERR_IF_PASSTHROUGH;

		trotListRefFree( &lr );

		/* these tokens need finallist */
		if (    tokenType == TOKEN_TYPE_L_BRACKET
		     || tokenType == TOKEN_TYPE_L_BRACE
		     || tokenType == TOKEN_TYPE_WORD /* in case it's changed into TWIN and becomes a list */
		   )
		{
			/* add finallist */
			rc = trotListRefInit( &lr );
			ERR_IF_PASSTHROUGH;

			rc = trotListRefAppendListTwin( newToken, lr );
			ERR_IF_PASSTHROUGH;

			trotListRefFree( &lr );

			/* these tokens needs name, enums */
			if (    tokenType == TOKEN_TYPE_L_BRACKET
			     || tokenType == TOKEN_TYPE_L_BRACE
			   )
			{
				/* add name */
				rc = trotListRefInit( &lr );
				ERR_IF_PASSTHROUGH;

				rc = trotListRefAppendListTwin( newToken, lr );
				ERR_IF_PASSTHROUGH;

				trotListRefFree( &lr );

				/* add enums */
				rc = trotListRefInit( &lr );
				ERR_IF_PASSTHROUGH;

				rc = trotListRefAppendListTwin( newToken, lr );
				ERR_IF_PASSTHROUGH;

				trotListRefFree( &lr );

				/* this token needs vars */
				if ( tokenType == TOKEN_TYPE_L_BRACE )
				{
					/* add vars */
					rc = trotListRefInit( &lr );
					ERR_IF_PASSTHROUGH;

					rc = trotListRefAppendListTwin( newToken, lr );
					ERR_IF_PASSTHROUGH;

					trotListRefFree( &lr );
				} /* end adding vars */
			} /* end adding name, enums */
		} /* end adding finallist */
	} /* end adding value */

	/* give back */
	(*lrToken_A) = newToken;
	newToken = NULL;


	/* CLEANUP */
	cleanup:

	trotListRefFree( &newToken );
	trotListRefFree( &lr );

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
	PARANOID_ERR_IF( rc != TROT_LIST_SUCCESS );

	/* get first character */
	rc = trotListRefGetInt( lrWord, 1, &character );
	PARANOID_ERR_IF( rc != TROT_LIST_SUCCESS );

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
	PARANOID_ERR_IF( rc != TROT_LIST_SUCCESS );

	ERR_IF( ( ! ( character >= '0' && character <= '9' ) ), TROT_LIST_ERROR_DECODE );

	ERR_IF( character == '0' && count != 1, TROT_LIST_ERROR_DECODE );

	index += 1;

	while ( index <= count )
	{
		rc = trotListRefGetInt( lrWord, index, &character );
		PARANOID_ERR_IF( rc != TROT_LIST_SUCCESS );

		ERR_IF( ( ! ( character >= '0' && character <= '9' ) ), TROT_LIST_ERROR_DECODE );

		index += 1;
	}

	/* make sure number can fit into our INT_TYPE */
	index = 1;

	rc = trotListRefGetInt( lrWord, index, &character );
	PARANOID_ERR_IF( rc != TROT_LIST_SUCCESS );

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
				PARANOID_ERR_IF( rc != TROT_LIST_SUCCESS );

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
				PARANOID_ERR_IF( rc != TROT_LIST_SUCCESS );

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
	PARANOID_ERR_IF( rc != TROT_LIST_SUCCESS );

	/* if negative, get real first number */
	if ( character == '-' )
	{
		index += 1;

		rc = trotListRefGetInt( lrWord, index, &character );
		PARANOID_ERR_IF( rc != TROT_LIST_SUCCESS );

		newNumber = character - '0';
		newNumber *= -1;

		/* build rest of number */
		index += 1;

		while ( index <= count )
		{
			rc = trotListRefGetInt( lrWord, index, &character );
			PARANOID_ERR_IF( rc != TROT_LIST_SUCCESS );
	
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
			PARANOID_ERR_IF( rc != TROT_LIST_SUCCESS );
	
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


	/* CODE */
	PARANOID_ERR_IF( lrToken == NULL );

	/* get value */
	rc = trotListRefGetListTwin( lrToken, TOKEN_INDEX_VALUE, &lrValue );
	ERR_IF_PASSTHROUGH;

	/* is number? */
	rc = _trotWordToNumber( lrValue, &isNumber, &number );
	ERR_IF_PASSTHROUGH;

	if ( isNumber == 1 )
	{
		/* mark token as a number */
		rc = trotListRefReplaceWithInt( lrToken, TOKEN_INDEX_TYPE, TOKEN_TYPE_NUMBER );
		PARANOID_ERR_IF( rc != TROT_LIST_SUCCESS );

		/* remove old string value and replace with number value */
		rc = trotListRefReplaceWithInt( lrToken, TOKEN_INDEX_VALUE, number );
		PARANOID_ERR_IF( rc != TROT_LIST_SUCCESS );
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


	/* CODE */
	PARANOID_ERR_IF( lrCharacters == NULL );
	PARANOID_ERR_IF( i == NULL );
	PARANOID_ERR_IF( line == NULL  );
	PARANOID_ERR_IF( column == NULL );

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
			if ( trotIsNewline( character ) )
			{
				if ( ! ( character == '\n' && previousCharacter == '\r' ) )
				{
					(*line) += 1;
				}

				(*column) = 0;
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
			if ( trotIsNewline( character ) )
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

/******************************************************************************/
/*!
	\brief 
	\param 
	\return TROT_RC
*/
static int trotIsWhitespace( INT_TYPE character )
{
	if (    character == '\t'   /* horizontal tab */
	     || character == '\n'   /* newline */
	     || character == 0x0B   /* vertical tab */
	     || character == 0x0C   /* form feed */ 
	     || character == '\r'   /* carriage return */
	     || character == ' '    /* space */
	     || character == 0x85   /* next line */
	     || character == 0xA0   /* no break space */
	     || character == 0x1680 /* ogham space mark */
	     || character == 0x180E /* mongolian vowel separator */
	     || character == 0x2000 /* en quad */
	     || character == 0x2001 /* em quad */
	     || character == 0x2002 /* en space */
	     || character == 0x2003 /* em space */
	     || character == 0x2004 /* three-per-em space */
	     || character == 0x2005 /* four-per-em space */
	     || character == 0x2006 /* six-per-em space */
	     || character == 0x2007 /* figure space */
	     || character == 0x2008 /* punctuation space */
	     || character == 0x2009 /* thin space */
	     || character == 0x200A /* hair space */
	     || character == 0x2028 /* line separator */
	     || character == 0x2029 /* paragraph separator */
	     || character == 0x202F /* narrow no-break space */
	     || character == 0x205F /* medium mathematical space */
	     || character == 0x3000 /* ideographic space */
	   )
	{
		return 1;
	}

	return 0;
}

/******************************************************************************/
/*!
	\brief 
	\param 
	\return TROT_RC
*/
static int trotIsNewline( INT_TYPE character )
{
	if (    character == '\n'   /* newline */
	     || character == 0x0B   /* vertical tab */
	     || character == 0x0C   /* form feed */
	     || character == '\r'   /* carriage return */
	     || character == 0x85   /* next line */
	     || character == 0x2028 /* line separator */
	     || character == 0x2029 /* paragraph separator */
	   )
	{
		return 1;
	}

	return 0;
}
