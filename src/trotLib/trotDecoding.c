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
	Decodes textual format to trot list.
*/

/******************************************************************************/
#include "trot.h"
#include "trotInternal.h"

/******************************************************************************/
static TROT_RC trotAddListValueNameEnumFinallist( trotListRef *lrToken, trotListRef **lrValue_A );

static TROT_RC tokenListToTokenTree( trotListRef *lrTokenList, trotListRef **lrTokenTree_A );

static TROT_RC createFinalList( trotListRef *lrTokenTree );

#if 0
/* DEBUG FUNCTIONS */
static TROT_RC trotPrintTokens( trotListRef *lrTokenList );
static TROT_RC trotPrintTokenTree( trotListRef *lrTokenTree, int indent );
static void trotPrintTokenType( INT_TYPE tokenType );
#endif

/******************************************************************************/
/*!
	\brief Decodes a list of characters into a list.
	\param loadFunc Function to use to load bytes for "includes".
	\param lrGivenFilenameOfCharacters The "filename" to give the characters.
	\param lrCharacters Characters to decode.
	\param stack On success, the decoded list.
	\return TROT_RC
*/
TROT_RC trotDecodeCharacters( TrotLoadFunc loadFunc, trotListRef *lrGivenFilenameOfCharacters, trotListRef *lrCharacters, trotListRef **lrDecodedList_A )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotListRef *newLrDecodedList = NULL;

	int pass = 0;

	trotListRef *lrFileList = NULL;
	INT_TYPE fileCount = 0;
	INT_TYPE fileIndex = 0;

	trotListRef *lrFile = NULL;
	trotListRef *lrFileName = NULL;

	trotListRef *lrFileCharacters = NULL;

	trotListRef *lrTokenList = NULL;
	trotListRef *lrTokenTree = NULL;

	trotListRef *lrFinalList = NULL;
	INT_TYPE finalListCount = 0;
	int finalListKind = 0; /* TODO: this needs to be enum */


	/* PRECOND */
	PRECOND_ERR_IF( loadFunc == NULL );
	PRECOND_ERR_IF( lrGivenFilenameOfCharacters == NULL );
	PRECOND_ERR_IF( lrCharacters == NULL );
	PRECOND_ERR_IF( lrDecodedList_A == NULL );
	PRECOND_ERR_IF( (*lrDecodedList_A) != NULL );


	/* CODE */
/* TODO */
(void)loadFunc;

	/* create FileList */
	rc = trotListRefInit( &lrFileList );
	ERR_IF_PASSTHROUGH;

	/* *** */
	/* create our first File */
	rc = trotListRefInit( &lrFile );
	ERR_IF_PASSTHROUGH;

	/* *** */
	/* create FileName for first File */
	rc = trotListRefCopy( lrGivenFilenameOfCharacters, &lrFileName );
	ERR_IF_PASSTHROUGH;

	/* add FileName to File */
	rc = trotListRefAppendListTwin( lrFile, lrFileName );
	ERR_IF_PASSTHROUGH;

	trotListRefFree( &lrFileName );

	/* *** */
	/* add File to FileList */
	rc = trotListRefAppendListTwin( lrFileList, lrFile );
	ERR_IF_PASSTHROUGH;

	trotListRefFree( &lrFile );

	/* *** */
	/* twin lrCharacters so we can "carry in" our first file's characters */
	rc = trotListRefTwin( lrCharacters, &lrFileCharacters );
	ERR_IF_PASSTHROUGH;

	/* *** */
	/* foreach pass */
	pass = 1;
	do
	{
		/* foreach file */
		fileIndex = 1;
		do
		{
			/* get file */
			trotListRefFree( &lrFile );

			rc = trotListRefGetListTwin( lrFileList, fileIndex, &lrFile );
			ERR_IF_PASSTHROUGH;

			/* tokenize, tokenListToTokenTree, handle meta data */
			if ( pass == 1 )
			{
				/* if we don't have characters
				   (this will only not happen for first file) */
				if ( lrFileCharacters == NULL )
				{
					/* TODO */
					/* get name */
					/* load file */
					/* unicode conversion */
					ERR_IF_PARANOID( 1 );
				}

				/* *** */
				/* tokenize */
				trotListRefFree( &lrTokenList );

				rc = trotTokenize( lrFileCharacters, &lrTokenList );
				ERR_IF_PASSTHROUGH;

				/* change token list into a token tree */
				trotListRefFree( &lrTokenTree );

				rc = tokenListToTokenTree( lrTokenList, &lrTokenTree );
				ERR_IF_PASSTHROUGH;

				/* TODO: handle meta-data here */

				/* add TokenTree to File */
				rc = trotListRefAppendListTwin( lrFile, lrTokenTree );
				ERR_IF_PASSTHROUGH;
			}
			/* handle words */
			else if ( pass == 2 )
			{
				/* TODO */
			}
			/* handle twins: TODO merge this with pass 2? */
			else if ( pass == 3 )
			{
				/* TODO */
			}
			/* optimize code lists */
			else if ( pass == 4 )
			{
				/* TODO */
			}
			/* create finalList */
			else
			{
				/* get tokenTree */
				trotListRefFree( &lrTokenTree );

				rc = trotListRefGetListTwin( lrFile, -1, &lrTokenTree );
				ERR_IF_PASSTHROUGH;

				/* create final list */
				rc = createFinalList( lrTokenTree );
				ERR_IF_PASSTHROUGH;
			}

			/* update fileCount, in case we added more files */
			rc = trotListRefGetCount( lrFileList, &fileCount );
			ERR_IF_PASSTHROUGH;

			/* increment fileIndex */
			fileIndex += 1;

		} while ( fileIndex <= fileCount );

		/* next pass */
		pass += 1;

	} while ( pass <= 5 );

	/* get first file */
	trotListRefFree( &lrFile );

	rc = trotListRefGetListTwin( lrFileList, 1, &lrFile );
	ERR_IF_PASSTHROUGH;

	/* get it's tokenTree */
	trotListRefFree( &lrTokenTree );

	rc = trotListRefGetListTwin( lrFile, -1, &lrTokenTree );
	ERR_IF_PASSTHROUGH;

	/* get it's finalList */
	rc = trotListRefGetListTwin( lrTokenTree, -1, &lrFinalList );
	ERR_IF_PASSTHROUGH;

	/* final list here should only contain 1 single list */
	rc = trotListRefGetCount( lrFinalList, &finalListCount );
	ERR_IF_PASSTHROUGH;

	ERR_IF( finalListCount != 1, TROT_LIST_ERROR_DECODE );

	rc = trotListRefGetKind( lrFinalList, 1, &finalListKind );
	ERR_IF_PASSTHROUGH;

	ERR_IF( finalListKind != NODE_KIND_LIST, TROT_LIST_ERROR_DECODE );

	/* get decoded list */
	rc = trotListRefGetListTwin( lrFinalList, 1, &newLrDecodedList );
	ERR_IF_PASSTHROUGH;

	/* give back */
	(*lrDecodedList_A) = newLrDecodedList;
	newLrDecodedList = NULL;


	/* CLEANUP */
	cleanup:

	trotListRefFree( &newLrDecodedList );
	trotListRefFree( &lrFileList );
	trotListRefFree( &lrFile );
	trotListRefFree( &lrFileName );
	trotListRefFree( &lrFileCharacters );
	trotListRefFree( &lrTokenList );
	trotListRefFree( &lrTokenTree );
	trotListRefFree( &lrFinalList );

	return rc;
}

/******************************************************************************/
/*!
	\brief Decodes a file into a list.
	\param loadFunc Function to use to load bytes for "includes".
	\param lrFilename Filename of file to load and decode.
	\param lrDecodedList_A On success, the decoded list.
	\return TROT_RC
*/
TROT_RC trotDecodeFilename( TrotLoadFunc loadFunc, trotListRef *lrFilename, trotListRef **lrDecodedList_A )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotListRef *newDecodedList = NULL;

	trotListRef *lrBytes = NULL;
	trotListRef *lrCharacters = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( lrFilename == NULL );
	PRECOND_ERR_IF( lrDecodedList_A == NULL );
	PRECOND_ERR_IF( (*lrDecodedList_A) != NULL );


	/* CODE */
	rc = loadFunc( lrFilename, &lrBytes );
	ERR_IF_PASSTHROUGH;

	rc = trotListRefInit( &lrCharacters );
	ERR_IF_PASSTHROUGH;

	rc = trotUtf8ToCharacters( lrBytes, lrCharacters );
	ERR_IF_PASSTHROUGH;

	rc = trotDecodeCharacters( loadFunc, lrFilename, lrCharacters, &newDecodedList );
	ERR_IF_PASSTHROUGH;

	/* give back */
	(*lrDecodedList_A) = newDecodedList;
	newDecodedList = NULL;


	/* CLEANUP */
	cleanup:

	trotListRefFree( &newDecodedList );
	trotListRefFree( &lrCharacters );
	trotListRefFree( &lrBytes );

	return rc;
}

/******************************************************************************/
/*!
	\brief Adds value to a token. Which contains three lists for: name, enum, and children.
	\param lrToken token to add value to.
	\param lrChildren_A On success, ref to children inside value.
	\return TROT_RC
*/
static TROT_RC trotAddListValueNameEnumFinallist( trotListRef *lrToken, trotListRef **lrValue_A )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotListRef *newLrList = NULL;
	trotListRef *newLrValue = NULL;


	/* CODE */
	ERR_IF_PARANOID( lrToken == NULL );
	ERR_IF_PARANOID( lrChildren_A == NULL );
	ERR_IF_PARANOID( (*lrChildren_A) != NULL );

	/* add children */
	rc = trotListRefInit( &newLrValue );
	ERR_IF_PASSTHROUGH;

	rc = trotListRefAppendListTwin( lrToken, newLrValue );
	ERR_IF_PASSTHROUGH;

	/* add name */
	rc = trotListRefInit( &newLrList );
	ERR_IF_PASSTHROUGH;

	rc = trotListRefAppendListTwin( lrToken, newLrList );
	ERR_IF_PASSTHROUGH;

	trotListRefFree( &newLrList );

	/* add enum */
	rc = trotListRefInit( &newLrList );
	ERR_IF_PASSTHROUGH;

	rc = trotListRefAppendListTwin( lrToken, newLrList );
	ERR_IF_PASSTHROUGH;

	trotListRefFree( &newLrList );

	/* add 'final list' */
	rc = trotListRefInit( &newLrList );
	ERR_IF_PASSTHROUGH;

	rc = trotListRefAppendListTwin( lrToken, newLrList );
	ERR_IF_PASSTHROUGH;

	trotListRefFree( &newLrList );


	/* give back */
	(*lrValue_A) = newLrValue;
	newLrValue = NULL;
	

	/* CLEANUP */
	cleanup:

	trotListRefFree( &newLrList );
	trotListRefFree( &newLrValue );

	return rc;
}

/******************************************************************************/
/*!
	\brief Change token list into a token tree.
	\param lrTokenList Token list.
	\return TROT_RC
*/
static TROT_RC tokenListToTokenTree( trotListRef *lrTokenList, trotListRef **lrTokenTree_A )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotListRef *lrTopToken = NULL;

	trotListRef *lrParentStack = NULL;
	INT_TYPE parentStackCount = 0;

	trotListRef *lrParent = NULL;
	trotListRef *lrChildren = NULL;

	trotListRef *lrToken = NULL;

	INT_TYPE tokenCount = 0;
	INT_TYPE tokenIndex = 0;

	INT_TYPE tokenType = 0;


	/* CODE */
	ERR_IF_PARANOID( lrTokenList == NULL );
	ERR_IF_PARANOID( lrTokenTree_A == NULL );
	ERR_IF_PARANOID( (*lrTokenTree_A) != NULL );

	/* *** */
	/* create our parentStack, so we can "go up" */
	rc = trotListRefInit( &lrParentStack );
	ERR_IF_PASSTHROUGH;

	/* *** */
	/* create the implicit list token for the file */
	/* TODO: need to create three tokens "(" "decoded" )" so we can remove it later, or at least know
	         it's the list added when decoding? */
	rc = trotCreateToken( 0, 0, TOKEN_L_BRACKET, &lrTopToken );
	ERR_IF_PASSTHROUGH;
	rc = trotAddListValueNameEnumFinallist( lrTopToken, &lrChildren );
	ERR_IF_PASSTHROUGH;

	/* *** */
	rc = trotListRefTwin( lrTopToken, &lrParent );
	ERR_IF_PASSTHROUGH;

	/* *** */
	/* get count of tokens */
	rc = trotListRefGetCount( lrTokenList, &tokenCount );
	ERR_IF_PASSTHROUGH;

	/* foreach token */
	tokenIndex = 1;
	while ( tokenIndex <= tokenCount )
	{
		/* get next token */
		trotListRefFree( &lrToken );

		rc = trotListRefGetListTwin( lrTokenList, tokenIndex, &lrToken );
		ERR_IF_PASSTHROUGH;

		/* get tokenType */
		rc = trotListRefGetInt( lrToken, TOKEN_INDEX_TYPE, &tokenType );
		ERR_IF_PASSTHROUGH;

		/* add this to our currentChildren */
		rc = trotListRefAppendListTwin( lrChildren, lrToken );
		ERR_IF_PASSTHROUGH;

		/* handle token */
		switch ( tokenType )
		{
			case TOKEN_L_BRACKET:

				/* add parent to our parentStack */
				rc = trotListRefAppendListTwin( lrParentStack, lrParent );
				ERR_IF_PASSTHROUGH;

				trotListRefFree( &lrParent );
				rc = trotListRefTwin( lrToken, &lrParent );

				/* add value for this token */
				trotListRefFree( &lrChildren );

				rc = trotAddListValueNameEnumFinallist( lrToken, &lrChildren );
				ERR_IF_PASSTHROUGH;

				break;

			case TOKEN_R_BRACKET:
				/* make sure we can go up */
				rc = trotListRefGetCount( lrParentStack, &parentStackCount );
				ERR_IF_PASSTHROUGH;

				ERR_IF( parentStackCount == 0, TROT_LIST_ERROR_DECODE );

				/* get parent */
				trotListRefFree( &lrParent );

				rc = trotListRefRemoveList( lrParentStack, -1, &lrParent );
				ERR_IF_PASSTHROUGH;

				/* make sure parent is a L BRACKET */
				rc = trotListRefGetInt( lrParent, TOKEN_INDEX_TYPE, &tokenType );
				ERR_IF_PASSTHROUGH;

				ERR_IF( tokenType != TOKEN_L_BRACKET, TROT_LIST_ERROR_DECODE );

				/* get children of parent */
				trotListRefFree( &lrChildren );

				rc = trotListRefGetListTwin( lrParent, TOKEN_INDEX_VALUE, &lrChildren );
				ERR_IF_PASSTHROUGH;
				
				break;

			case TOKEN_L_BRACE:
				/* TODO */
				ERR_IF_PARANOID( 1 );
				break;

			case TOKEN_R_BRACE:
				/* TODO */
				ERR_IF_PARANOID( 1 );
				break;

			case TOKEN_L_PARENTHESIS:
				/* TODO */
				ERR_IF_PARANOID( 1 );
				break;

			case TOKEN_R_PARENTHESIS:
				/* TODO */
				ERR_IF_PARANOID( 1 );
				break;

			case TOKEN_WORD:
			case TOKEN_NUMBER:
			case TOKEN_STRING:
				break;

			default:
				ERR_IF( 1, TROT_LIST_ERROR_DECODE );
				break;
		}

		/* next */
		tokenIndex += 1;
	}

	/* make sure we have no parents */
	rc = trotListRefGetCount( lrParentStack, &parentStackCount );
	ERR_IF_PASSTHROUGH;

	ERR_IF( parentStackCount != 0, TROT_LIST_ERROR_DECODE );

	/* give back */
	(*lrTokenTree_A) = lrTopToken;
	lrTopToken = NULL;
	

	/* CLEANUP */
	cleanup:

	trotListRefFree( &lrTopToken );
	trotListRefFree( &lrParentStack );
	trotListRefFree( &lrParent );
	trotListRefFree( &lrChildren );
	trotListRefFree( &lrToken );

	return rc;
}

/******************************************************************************/
/*!
	\brief Creates the final list.
	\param lrTokenTree Token Tree.
	\return TROT_RC
*/
static TROT_RC createFinalList( trotListRef *lrTokenTree )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotListRef *lrParentTokenStack = NULL;
	INT_TYPE parentTokenStackCount = 0;

	trotListRef *lrParentTokenIndicesStack = NULL;

	trotListRef *lrToken = NULL;
	trotListRef *lrTokenChildren = NULL;
	trotListRef *lrTokenFinalList = NULL;
	INT_TYPE tokenChildrenCount = 0;
	INT_TYPE tokenChildrenIndex = 0;

	trotListRef *lrChildToken = NULL;
	trotListRef *lrChildTokenValue = NULL;
	trotListRef *lrChildTokenFinalList = NULL;
	INT_TYPE childTokenType = 0;
	INT_TYPE childTokenValueInt = 0;


	/* PRECOND */
	PRECOND_ERR_IF( lrTokenTree == NULL );


	/* CODE */
	/* create parent stack, so we can "go up" */
	rc = trotListRefInit( &lrParentTokenStack );
	ERR_IF_PASSTHROUGH;

	rc = trotListRefInit( &lrParentTokenIndicesStack );
	ERR_IF_PASSTHROUGH;

	/* *** */
	/* set token to lrTokenTree */
	rc = trotListRefTwin( lrTokenTree, &lrToken );
	ERR_IF_PASSTHROUGH;

	/* get top token's children */
	rc = trotListRefGetListTwin( lrToken, TOKEN_INDEX_VALUE, &lrTokenChildren );
	ERR_IF_PASSTHROUGH;

	/* get finalList */
	rc = trotListRefGetListTwin( lrToken, TOKEN_INDEX_FINALLIST, &lrTokenFinalList );
	ERR_IF_PASSTHROUGH;

	/* set index */
	tokenChildrenIndex = 0;

	/* go through tree */
	while ( 1 )
	{
		/* increment index */
		tokenChildrenIndex += 1;

		/* are we at end of children? */
		rc = trotListRefGetCount( lrTokenChildren, &tokenChildrenCount );
		ERR_IF_PASSTHROUGH;

		if ( tokenChildrenIndex > tokenChildrenCount )
		{
			/* free token, children, finalList */
			trotListRefFree( &lrToken );
			trotListRefFree( &lrTokenChildren );
			trotListRefFree( &lrTokenFinalList );

			/* do we have any parents? */
			rc = trotListRefGetCount( lrParentTokenStack, &parentTokenStackCount );
			ERR_IF_PASSTHROUGH;

			/* no parents, so we're done */
			if ( parentTokenStackCount == 0 )
			{
				break;
			}

			/* yes parents, so go up */

			/* get token */
			rc = trotListRefRemoveList( lrParentTokenStack, -1, &lrToken );
			ERR_IF_PASSTHROUGH;

			/* get top token's children */
			rc = trotListRefGetListTwin( lrToken, TOKEN_INDEX_VALUE, &lrTokenChildren );
			ERR_IF_PASSTHROUGH;

			/* get finalList */
			rc = trotListRefGetListTwin( lrToken, TOKEN_INDEX_FINALLIST, &lrTokenFinalList );
			ERR_IF_PASSTHROUGH;

			/* get index */
			rc = trotListRefRemoveInt( lrParentTokenIndicesStack, -1, &tokenChildrenIndex );
			ERR_IF_PASSTHROUGH;

			continue;
		}

		/* get next child token */
		trotListRefFree( &lrChildToken );

		rc = trotListRefGetListTwin( lrTokenChildren, tokenChildrenIndex, &lrChildToken );
		ERR_IF_PASSTHROUGH;

		/* get tokenType */
		rc = trotListRefGetInt( lrChildToken, TOKEN_INDEX_TYPE, &childTokenType );
		ERR_IF_PASSTHROUGH;

		/* handle token */
		switch ( childTokenType )
		{
			case TOKEN_L_BRACKET:
				/* get child final list */
				trotListRefFree( &lrChildTokenFinalList );

				rc = trotListRefGetListTwin( lrChildToken, TOKEN_INDEX_FINALLIST, &lrChildTokenFinalList );
				ERR_IF_PASSTHROUGH;

				/* append */
				rc = trotListRefAppendListTwin( lrTokenFinalList, lrChildTokenFinalList );
				ERR_IF_PASSTHROUGH;

				/* save our state */
				rc = trotListRefAppendListTwin( lrParentTokenStack, lrToken );
				ERR_IF_PASSTHROUGH;

				rc = trotListRefAppendInt( lrParentTokenIndicesStack, tokenChildrenIndex );
				ERR_IF_PASSTHROUGH;

				/* go down */
				
				/* free token, children, final list */
				trotListRefFree( &lrToken );
				trotListRefFree( &lrTokenChildren );
				trotListRefFree( &lrTokenFinalList );

				/* set token equal to this child */
				lrToken = lrChildToken;
				lrChildToken = NULL;

				/* get token's children */
				rc = trotListRefGetListTwin( lrToken, TOKEN_INDEX_VALUE, &lrTokenChildren );
				ERR_IF_PASSTHROUGH;

				/* get token's finalList */
				rc = trotListRefGetListTwin( lrToken, TOKEN_INDEX_FINALLIST, &lrTokenFinalList );
				ERR_IF_PASSTHROUGH;

				/* set index */
				tokenChildrenIndex = 0;

				continue;

			case TOKEN_R_BRACKET:
				ERR_IF_PARANOID( 1 );
				break;

			case TOKEN_L_BRACE:
				ERR_IF_PARANOID( 1 );
				break;

			case TOKEN_R_BRACE:
				ERR_IF_PARANOID( 1 );
				break;

			case TOKEN_L_PARENTHESIS:
				ERR_IF_PARANOID( 1 );
				break;

			case TOKEN_R_PARENTHESIS:
				ERR_IF_PARANOID( 1 );
				break;

			case TOKEN_WORD:
				ERR_IF_PARANOID( 1 );
				break;

			case TOKEN_NUMBER:
				/* get number */
				rc = trotListRefGetInt( lrChildToken, TOKEN_INDEX_VALUE, &childTokenValueInt );
				ERR_IF_PASSTHROUGH;

				/* append number */
				rc = trotListRefAppendInt( lrTokenFinalList, childTokenValueInt );
				ERR_IF_PASSTHROUGH;
				break;

			case TOKEN_STRING:
				/* TODO */
				ERR_IF_PARANOID( 1 );
				break;
#if 0
			case TODO:
				there is probably going to be a TOKEN_TWIN type that we will need to twin here
#endif

			default:
				ERR_IF( 1, TROT_LIST_ERROR_DECODE );
				break;
		}
	}


	/* CLEANUP */
	cleanup:

	trotListRefFree( &lrParentTokenStack );
	trotListRefFree( &lrParentTokenIndicesStack );

	trotListRefFree( &lrToken );
	trotListRefFree( &lrTokenChildren );
	trotListRefFree( &lrTokenFinalList );

	trotListRefFree( &lrChildToken );
	trotListRefFree( &lrChildTokenValue );
	trotListRefFree( &lrChildTokenFinalList );

	return rc;
}

#if 0
static TROT_RC trotPrintTokens( trotListRef *lrTokenList )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	INT_TYPE count = 0;
	INT_TYPE index = 0;

	INT_TYPE currentLine = 1;

	trotListRef *lrToken = NULL;

	INT_TYPE tokenLine = 0;
	INT_TYPE tokenType = 0;
	trotListRef *lrValue = NULL;
	INT_TYPE tokenNumber = 0;

	trotListRef *lrUtf8Bytes = NULL;
	INT_TYPE utf8Count = 0;
	INT_TYPE utf8Index = 0;
	INT_TYPE utf8Byte = 0;


	/* CODE */
	ERR_IF_PARANOID( lrTokenList == NULL );

	printf( "TokenList:\n" );

	/* get count */
	rc = trotListRefGetCount( lrTokenList, &count );
	ERR_IF_PASSTHROUGH;

	/* foreach token */
	index = 1;
	while ( index <= count )
	{
		/* get token */
		trotListRefFree( &lrToken );

		rc = trotListRefGetListTwin( lrTokenList, index, &lrToken );
		ERR_IF_PASSTHROUGH;

		/* get line */
		rc = trotListRefGetInt( lrToken, TOKEN_INDEX_LINE, &tokenLine );
		ERR_IF_PASSTHROUGH;

		/* get type */
		rc = trotListRefGetInt( lrToken, TOKEN_INDEX_TYPE, &tokenType );
		ERR_IF_PASSTHROUGH;

		/* new line? */
		if ( currentLine != tokenLine )
		{
			printf( "\n" );
			currentLine = tokenLine;
		}

		/* print */
		if ( tokenType == TOKEN_L_BRACKET )
		{
			printf( "[ " );
		}
		else if ( tokenType == TOKEN_R_BRACKET )
		{
			printf( "] " );
		}
		else if ( tokenType == TOKEN_L_PARENTHESIS )
		{
			printf( "( " );
		}
		else if ( tokenType == TOKEN_R_PARENTHESIS )
		{
			printf( ") " );
		}
		else if ( tokenType == TOKEN_L_BRACE )
		{
			printf( "{ " );
		}
		else if ( tokenType == TOKEN_R_BRACE )
		{
			printf( "} " );
		}
		else if ( tokenType == TOKEN_STRING || tokenType == TOKEN_WORD )
		{
			/* get value */
			trotListRefFree( &lrValue );

			rc = trotListRefGetListTwin( lrToken, TOKEN_INDEX_VALUE, &lrValue );
			ERR_IF_PASSTHROUGH;

			/* convert to utf8 */
			trotListRefFree( &lrUtf8Bytes );

			rc = trotListRefInit( &lrUtf8Bytes );
			ERR_IF_PASSTHROUGH;

			rc = trotCharactersToUtf8( lrValue, lrUtf8Bytes );
			ERR_IF_PASSTHROUGH;

			if ( tokenType == TOKEN_STRING )
			{
				printf( "\"" );
			}
			else
			{
				printf( "W:" );
			}

			/* print */
			rc = trotListRefGetCount( lrUtf8Bytes, &utf8Count );
			ERR_IF_PASSTHROUGH;

			utf8Index = 1;
			while ( utf8Index <= utf8Count )
			{
				rc = trotListRefGetInt( lrUtf8Bytes, utf8Index, &utf8Byte );
				ERR_IF_PASSTHROUGH;

				printf( "%c", utf8Byte );

				/* next */
				utf8Index += 1;
			}
			

			if ( tokenType == TOKEN_STRING )
			{
				printf( "\" " );
			}
			else
			{
				printf( " " );
			}
		}
		else /* NUMBER */
		{
			rc = trotListRefGetInt( lrToken, TOKEN_INDEX_VALUE, &tokenNumber );
			ERR_IF_PASSTHROUGH;

			printf( "N:%d ", tokenNumber );
		}

		/* next */
		index += 1;
	}


	/* CLEANUP */
	cleanup:

	printf( "\n\n" );

	trotListRefFree( &lrToken );
	trotListRefFree( &lrValue );
	trotListRefFree( &lrUtf8Bytes );

	return rc;
}

/******************************************************************************/
static int trotPrintTokenTree( trotListRef *lrTokenTree, int indent )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	int i = 0;
	int j = 0;
	int count = 0;

	trotListRef *lrChildren = NULL;
	trotListRef *lrChildToken = NULL;
	INT_TYPE childTokenType = 0;
	INT_TYPE childTokenValueInt = 0;


	/* PRECOND */
	PRECOND_ERR_IF( lrTokenTree == NULL );


	/* CODE */
	if ( indent == 0 )
	{
		printf( "TokenTree:\n" );
	}

	printf( "\n" );
	for ( j = 0; j < indent; j += 1 )
	{
		printf( "    " );
	}

	printf( "[ " );

	i = 1;

	rc = trotListRefGetListTwin( lrTokenTree, TOKEN_INDEX_VALUE, &lrChildren );
	ERR_IF_PASSTHROUGH;

	rc = trotListRefGetCount( lrChildren, &count );
	ERR_IF_PASSTHROUGH;

	/* go through tree */
	while ( i <= count )
	{
		/* get next child token */
		trotListRefFree( &lrChildToken );

		rc = trotListRefGetListTwin( lrChildren, i, &lrChildToken );
		ERR_IF_PASSTHROUGH;

		/* get tokenType */
		rc = trotListRefGetInt( lrChildToken, TOKEN_INDEX_TYPE, &childTokenType );
		ERR_IF_PASSTHROUGH;

		/* handle token */
		switch ( childTokenType )
		{
			case TOKEN_L_BRACKET:
				trotPrintTokenTree( lrChildToken, indent + 1 );
				for ( j = 0; j < indent; j += 1 )
				{
					printf( "    " );
				}
				break;

			case TOKEN_R_BRACKET:
				ERR_IF_PARANOID( 1 );
				break;

			case TOKEN_L_BRACE:
				ERR_IF_PARANOID( 1 );
				break;

			case TOKEN_R_BRACE:
				ERR_IF_PARANOID( 1 );
				break;

			case TOKEN_L_PARENTHESIS:
				ERR_IF_PARANOID( 1 );
				break;

			case TOKEN_R_PARENTHESIS:
				ERR_IF_PARANOID( 1 );
				break;

			case TOKEN_WORD:
				ERR_IF_PARANOID( 1 );
				break;

			case TOKEN_NUMBER:
				/* get number */
				rc = trotListRefGetInt( lrChildToken, TOKEN_INDEX_VALUE, &childTokenValueInt );
				ERR_IF_PASSTHROUGH;

				printf( "%d ", childTokenValueInt );

				break;

			case TOKEN_STRING:
				/* TODO */
				ERR_IF_PARANOID( 1 );
				break;
#if 0
			case TODO:
				there is probably going to be a TOKEN_TWIN type that we will need to twin here
#endif

			default:
				ERR_IF( 1, TROT_LIST_ERROR_DECODE );
				break;
		}

		i += 1;
	}


	/* CLEANUP */
	cleanup:

	printf( "]\n" );

	trotListRefFree( &lrChildren );
	trotListRefFree( &lrChildToken );

	return rc;
}

static void trotPrintTokenType( INT_TYPE tokenType )
{
	char *strings[] =
	{
		"TOKEN_L_BRACKET",
		"TOKEN_R_BRACKET",
		"TOKEN_L_BRACE",
		"TOKEN_R_BRACE",
		"TOKEN_L_PARENTHESIS",
		"TOKEN_R_PARENTHESIS",
		"TOKEN_WORD",
		"TOKEN_NUMBER",
		"TOKEN_STRING"
	};

	if (    tokenType >= TOKEN_L_BRACKET
	     && tokenType <= TOKEN_STRING
	   )
	{
		printf( "TokenType: %s\n", strings[ tokenType - 1 ] );
	}
	else
	{
		printf( "TokenType: UNKNOWN! %d\n", tokenType );
	}

	return;
}
#endif

