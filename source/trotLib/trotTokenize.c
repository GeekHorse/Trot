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
	Tokenizer.
*/

/******************************************************************************/
#include "trot.h"
#include "trotInternal.h"

/******************************************************************************/
static TROT_RC trotUpgradeWordToNumber( TrotList *lToken );
static TROT_RC trotSkipComments( TrotList *lCharacters, TROT_INT count, TROT_INT *i, TROT_INT *line, TROT_INT *column );
static int trotIsWhitespace( TROT_INT character );
static int trotIsNewline( TROT_INT character );

/******************************************************************************/
/*!
	\brief Tokenizes a list of characters.
	\param lCharacters List of characters.
	\param lTokens_A On success, list of tokens
	\return TROT_RC
*/
TROT_RC trotTokenize( TrotList *lCharacters, TrotList **lTokenList_A )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotList *newLTokenList = NULL;

	TrotList *lToken = NULL;

	TrotList *lValue = NULL;

	TROT_INT count = 0;
	TROT_INT i = 0;

	TROT_INT line = 1;
	TROT_INT column = 1;

	TROT_INT previousCharacter = -1;
	TROT_INT character = -1;


	/* PRECOND */
	PRECOND_ERR_IF( lCharacters == NULL );
	PRECOND_ERR_IF( lTokenList_A == NULL );
	PRECOND_ERR_IF( (*lTokenList_A) != NULL );


	/* CODE */
	/* create our giveback */
	rc = trotListInit( &newLTokenList );
	ERR_IF_PASSTHROUGH;

	/* get count */
	rc = trotListGetCount( lCharacters, &count );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* foreach character */
	i = 1;
	while ( i <= count )
	{
		/* get character */
		previousCharacter = character;
		rc = trotListGetInt( lCharacters, i, &character );
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
			rc = trotCreateToken( line, column, TOKEN_TYPE_L_BRACKET, &lToken );
			ERR_IF_PASSTHROUGH;
		}
		/* if ] */
		else if ( character == ']' )
		{
			rc = trotCreateToken( line, column, TOKEN_TYPE_R_BRACKET, &lToken );
			ERR_IF_PASSTHROUGH;
		}
		/* if ( */
		else if ( character == '(' )
		{
			rc = trotCreateToken( line, column, TOKEN_TYPE_L_PARENTHESIS, &lToken );
			ERR_IF_PASSTHROUGH;
		}
		/* if ) */
		else if ( character == ')' )
		{
			rc = trotCreateToken( line, column, TOKEN_TYPE_R_PARENTHESIS, &lToken );
			ERR_IF_PASSTHROUGH;
		}
		/* if { */
		else if ( character == '{' )
		{
			rc = trotCreateToken( line, column, TOKEN_TYPE_L_BRACE, &lToken );
			ERR_IF_PASSTHROUGH;
		}
		/* if } */
		else if ( character == '}' )
		{
			rc = trotCreateToken( line, column, TOKEN_TYPE_R_BRACE, &lToken );
			ERR_IF_PASSTHROUGH;
		}
		/* if " */
		else if ( character == '"' )
		{
			/* strings cannot touch strings */
			ERR_IF ( previousCharacter == '"', TROT_RC_ERROR_DECODE );

			/* create token */
			rc = trotCreateToken( line, column, TOKEN_TYPE_STRING, &lToken );
			ERR_IF_PASSTHROUGH;

			/* get value */
			trotListFree( &lValue );
			rc = trotListGetList( lToken, TOKEN_INDEX_VALUE, &lValue );
			ERR_IF_PASSTHROUGH;

			/* get value of string */
			column += 1;
			i += 1;

			/* there must be at least one character after the quote */
			ERR_IF( i > count, TROT_RC_ERROR_DECODE );

			while ( i <= count )
			{
				/* get character */
				previousCharacter = character;
				rc = trotListGetInt( lCharacters, i, &character );
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
				rc = trotListAppendInt( lValue, character );
				ERR_IF_PASSTHROUGH;

				/* next */
				column += 1;
				i += 1;
			}

			ERR_IF ( character != '"', TROT_RC_ERROR_DECODE )
		}
		/* if # */
		else if ( character == '#' )
		{
			rc = trotSkipComments( lCharacters, count, &i, &line, &column );
			ERR_IF_PASSTHROUGH;
		}
		/* else word or number */
		else 
		{
			/* strings cannot touch words */
			ERR_IF( previousCharacter == '"', TROT_RC_ERROR_DECODE );

			/* create token */
			rc = trotCreateToken( line, column, TOKEN_TYPE_WORD, &lToken );
			ERR_IF_PASSTHROUGH;

			/* get value */
			trotListFree( &lValue );
			rc = trotListGetList( lToken, TOKEN_INDEX_VALUE, &lValue );
			ERR_IF_PASSTHROUGH;

			/* append character */
			rc = trotListAppendInt( lValue, character );
			ERR_IF_PASSTHROUGH;

			/* get rest of value */
			while ( (i + 1) <= count )
			{
				/* get next character */
				rc = trotListGetInt( lCharacters, i + 1, &character );
				ERR_IF_PASSTHROUGH;
				
				/* strings cannot touch words */
				ERR_IF ( character == '"', TROT_RC_ERROR_DECODE );

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
				rc = trotListAppendInt( lValue, character );
				ERR_IF_PASSTHROUGH;
			}

			/* is word a number? */
			rc = trotUpgradeWordToNumber( lToken );
			ERR_IF_PASSTHROUGH;
		}

		/* add token */
		if ( lToken != NULL )
		{
			rc = trotListAppendList( newLTokenList, lToken );
			ERR_IF_PASSTHROUGH;

			trotListFree( &lToken );
		}

		/* next */
		column += 1;
		i += 1;
	}

	/* give back */
	(*lTokenList_A) = newLTokenList;
	newLTokenList = NULL;


	/* CLEANUP */
	cleanup:

	trotListFree( &newLTokenList );
	trotListFree( &lToken );
	trotListFree( &lValue );

	return rc;
}

/******************************************************************************/
/*!
	\brief Creates a token
	\param line Start line of token.
	\param column Start column of token.
	\param lToken_A On success, new token.
	\return TROT_RC
*/
TROT_RC trotCreateToken( TROT_INT line, TROT_INT column, TROT_INT tokenType, TrotList **lToken_A )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotList *newToken = NULL;

	TrotList *l = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( lToken_A == NULL );
	PRECOND_ERR_IF( (*lToken_A) != NULL );


	/* CODE */
	/* create new token */
	rc = trotListInit( &newToken );
	ERR_IF_PASSTHROUGH;

	/* add line, column, tokenType */
	rc = trotListAppendInt( newToken, line );
	ERR_IF_PASSTHROUGH;

#if ( NODE_SIZE < 3 )
#error NODE_SIZE TOO SMALL, MUST CHANGE THE FOLLOWING TWO PARANOID_ERR_IFS TO REAL ERR_IFs
#endif

	rc = trotListAppendInt( newToken, column );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );
	rc = trotListAppendInt( newToken, tokenType );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

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
		rc = trotListInit( &l );
		ERR_IF_PASSTHROUGH;

		rc = trotListAppendList( newToken, l );
		ERR_IF_PASSTHROUGH;

		trotListFree( &l );

		/* these tokens need finallist */
		if (    tokenType == TOKEN_TYPE_L_BRACKET
		     || tokenType == TOKEN_TYPE_L_BRACE
		     || tokenType == TOKEN_TYPE_WORD /* in case it's changed into TWIN and becomes a list */
		   )
		{
			/* add finallist */
			rc = trotListInit( &l );
			ERR_IF_PASSTHROUGH;

			rc = trotListAppendList( newToken, l );
			ERR_IF_PASSTHROUGH;

			trotListFree( &l );

			/* these tokens needs name, enums */
			if (    tokenType == TOKEN_TYPE_L_BRACKET
			     || tokenType == TOKEN_TYPE_L_BRACE
			   )
			{
				/* add name */
				rc = trotListInit( &l );
				ERR_IF_PASSTHROUGH;

				rc = trotListAppendList( newToken, l );
				ERR_IF_PASSTHROUGH;

				trotListFree( &l );

				/* add enums */
				rc = trotListInit( &l );
				ERR_IF_PASSTHROUGH;

				rc = trotListAppendList( newToken, l );
				ERR_IF_PASSTHROUGH;

				trotListFree( &l );

				/* this token needs vars */
				if ( tokenType == TOKEN_TYPE_L_BRACE )
				{
					/* add vars */
					rc = trotListInit( &l );
					ERR_IF_PASSTHROUGH;

					rc = trotListAppendList( newToken, l );
					ERR_IF_PASSTHROUGH;

					trotListFree( &l );
				} /* end adding vars */
			} /* end adding name, enums */
		} /* end adding finallist */
	} /* end adding value */

	/* give back */
	(*lToken_A) = newToken;
	newToken = NULL;


	/* CLEANUP */
	cleanup:

	trotListFree( &newToken );
	trotListFree( &l );

	return rc;
}

/******************************************************************************/
/*!
	\brief TODO: why does this function name start with underscore?
	\param 
	\return TROT_RC
*/
TROT_RC _trotWordToNumber( TrotList *lWord, int *isNumber, TROT_INT *number )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TROT_INT index = 0;
	TROT_INT count = 0;

	TROT_INT character = 0;
	TROT_INT newNumber = 0;

	int i = 0;


	/* PRECOND */
	PRECOND_ERR_IF( lWord == NULL );
	PRECOND_ERR_IF( isNumber == NULL );
	PRECOND_ERR_IF( number == NULL );


	/* CODE */
	/* mark no number initially */
	(*isNumber) = 0;

	/* get count */
	rc = trotListGetCount( lWord, &count );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* get first character */
	rc = trotListGetInt( lWord, 1, &character );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

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

	rc = trotListGetInt( lWord, index, &character );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	ERR_IF( ( ! ( character >= '0' && character <= '9' ) ), TROT_RC_ERROR_DECODE );

	ERR_IF( character == '0' && count != 1, TROT_RC_ERROR_DECODE );

	index += 1;

	while ( index <= count )
	{
		rc = trotListGetInt( lWord, index, &character );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		ERR_IF( ( ! ( character >= '0' && character <= '9' ) ), TROT_RC_ERROR_DECODE );

		index += 1;
	}

	/* make sure number can fit into our TROT_INT */
	index = 1;

	rc = trotListGetInt( lWord, index, &character );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	if ( character == '-' )
	{
		ERR_IF( count > TROT_INT_MIN_STRING_LENGTH, TROT_RC_ERROR_DECODE );

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
		ERR_IF( count > TROT_INT_MAX_STRING_LENGTH, TROT_RC_ERROR_DECODE );

		if ( count == TROT_INT_MAX_STRING_LENGTH )
		{
			i = 0;

			while ( index <= count )
			{	
				rc = trotListGetInt( lWord, index, &character );
				PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

				ERR_IF ( character > TROT_INT_MAX_STRING[ i ], TROT_RC_ERROR_DECODE );

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
	\brief Checks to see if a word is a number, and if so changes the token.
	\param lToken Token
	\return TROT_RC
*/
static TROT_RC trotUpgradeWordToNumber( TrotList *lToken )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotList *lValue = NULL;
	int isNumber = 0;
	TROT_INT number = 0;


	/* CODE */
	PARANOID_ERR_IF( lToken == NULL );

	/* get value */
	rc = trotListGetList( lToken, TOKEN_INDEX_VALUE, &lValue );
	ERR_IF_PASSTHROUGH;

	/* is number? */
	rc = _trotWordToNumber( lValue, &isNumber, &number );
	ERR_IF_PASSTHROUGH;

	if ( isNumber == 1 )
	{
		/* mark token as a number */
		rc = trotListReplaceWithInt( lToken, TOKEN_INDEX_TYPE, TOKEN_TYPE_NUMBER );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		/* remove old string value and replace with number value */
		rc = trotListReplaceWithInt( lToken, TOKEN_INDEX_VALUE, number );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );
	}
	

	/* CLEANUP */
	cleanup:

	trotListFree( &lValue );

	return rc;
}

/******************************************************************************/
/*!
	\brief 
	\param 
	\return TROT_RC
*/
static TROT_RC trotSkipComments( TrotList *lCharacters, TROT_INT count, TROT_INT *i, TROT_INT *line, TROT_INT *column )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TROT_INT character = 0;
	TROT_INT previousCharacter = 0;

	int commentDepth = 0;


	/* CODE */
	PARANOID_ERR_IF( lCharacters == NULL );
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
	rc = trotListGetInt( lCharacters, (*i), &character );
	ERR_IF_PASSTHROUGH;

	/* can't be ')' without a beginning '(' */
	ERR_IF( character == ')', TROT_RC_ERROR_DECODE );

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
			ERR_IF( (*i) > count, TROT_RC_ERROR_DECODE );

			/* get character */
			previousCharacter = character;
			rc = trotListGetInt( lCharacters, (*i), &character );
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
			rc = trotListGetInt( lCharacters, (*i), &character );
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
static int trotIsWhitespace( TROT_INT character )
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
static int trotIsNewline( TROT_INT character )
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
