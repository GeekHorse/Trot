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

/* TODO: should this be moved to tokenize.c? */
static TROT_RC tokenListToTokenTree( trotListRef *lrTokenList, trotListRef **lrTokenTree_A );

static TROT_RC createFinalList( trotListRef *lrTokenTree );

static TROT_RC handleMetaData( trotListRef *lrTokenTree, trotListRef *lrFileList );
static TROT_RC handleMetaData2( trotListRef *lrParentToken, trotListRef *lrParenthesisToken );

static TROT_RC handleAllWords( trotListRef *lrTokenTree );
static TROT_RC handleWord( trotListRef *lrParentTokenStack, trotListRef *lrTokenWord );

static TROT_RC addEnum( trotListRef *lrEnumList, trotListRef *lrEnum );
static TROT_RC getEnumValue( trotListRef *lrToken, trotListRef *lrName, INT_TYPE *found, INT_TYPE *value );

static TROT_RC compareListToCString( trotListRef *lrValue, char *cstring, TROT_LIST_COMPARE_RESULT *result );

#if 0
well probably need this for our name1.name2.name3 stuff later 
static TROT_RC splitList( trotListRef *lr, INT_TYPE separator, trotListRef **lrPartList );
#endif

/* TODO: put the above functions in the same order they appear below */
/* TODO: change name of static functions to put an underscore in front? */

#if 1
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



/* TODO: remove me */
(void)trotPrintTokens;
(void)trotPrintTokenType;

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

				/* handle meta-data */
trotPrintTokenTree( lrTokenTree, 0 );
				rc = handleMetaData( lrTokenTree, lrFileList );
				ERR_IF_PASSTHROUGH;
trotPrintTokenTree( lrTokenTree, 0 );

				/* add TokenTree to File */
				rc = trotListRefAppendListTwin( lrFile, lrTokenTree );
				ERR_IF_PASSTHROUGH;
			}
			/* handle words */
			else if ( pass == 2 )
			{
				/* get tokenTree */
				trotListRefFree( &lrTokenTree );

				rc = trotListRefGetListTwin( lrFile, -1, &lrTokenTree );
				ERR_IF_PASSTHROUGH;

				/* handle all words */
				rc = handleAllWords( lrTokenTree );
				ERR_IF_PASSTHROUGH;
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
	ERR_IF_PARANOID( lrValue_A == NULL );
	ERR_IF_PARANOID( (*lrValue_A) != NULL );

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

		/* handle token */
		switch ( tokenType )
		{
			case TOKEN_L_BRACKET:
				/* add this to our currentChildren */
				rc = trotListRefAppendListTwin( lrChildren, lrToken );
				ERR_IF_PASSTHROUGH;

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

				/* make sure parent is a L BRACKET */
				rc = trotListRefGetInt( lrParent, TOKEN_INDEX_TYPE, &tokenType );
				ERR_IF_PASSTHROUGH;

				ERR_IF( tokenType != TOKEN_L_BRACKET, TROT_LIST_ERROR_DECODE );

				/* get grandparent */
				trotListRefFree( &lrParent );

				rc = trotListRefRemoveList( lrParentStack, -1, &lrParent );
				ERR_IF_PASSTHROUGH;

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
				/* add this to our currentChildren */
				rc = trotListRefAppendListTwin( lrChildren, lrToken );
				ERR_IF_PASSTHROUGH;

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

			case TOKEN_R_PARENTHESIS:
				/* make sure we can go up */
				rc = trotListRefGetCount( lrParentStack, &parentStackCount );
				ERR_IF_PASSTHROUGH;

				ERR_IF( parentStackCount == 0, TROT_LIST_ERROR_DECODE );

				/* make sure parent is a L PARENTHESIS */
				rc = trotListRefGetInt( lrParent, TOKEN_INDEX_TYPE, &tokenType );
				ERR_IF_PASSTHROUGH;

				ERR_IF( tokenType != TOKEN_L_PARENTHESIS, TROT_LIST_ERROR_DECODE );

				/* get grandparent */
				trotListRefFree( &lrParent );

				rc = trotListRefRemoveList( lrParentStack, -1, &lrParent );
				ERR_IF_PASSTHROUGH;

				/* get children of parent */
				trotListRefFree( &lrChildren );

				rc = trotListRefGetListTwin( lrParent, TOKEN_INDEX_VALUE, &lrChildren );
				ERR_IF_PASSTHROUGH;
				break;

			case TOKEN_WORD:
			case TOKEN_NUMBER:
			case TOKEN_STRING:
				/* add this to our currentChildren */
				rc = trotListRefAppendListTwin( lrChildren, lrToken );
				ERR_IF_PASSTHROUGH;

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
	\brief Handles meta-data.
	\param lrTokenTree Token Tree.
	\param lrFileList File list.
	\return TROT_RC

	Removes (comment)
	Adds (enum), (name) to parent list.
	Adds (include) to file list (or twins if it's already there).
	Imports bytes in-place (importBytes).
	Tags parentlist (data), (text), (code), (codeGroup), TODO
*/
static TROT_RC handleMetaData( trotListRef *lrTokenTree, trotListRef *lrFileList )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotListRef *lrParentTokenStack = NULL;
	INT_TYPE parentTokenStackCount = 0;

	trotListRef *lrParentTokenIndicesStack = NULL;

	trotListRef *lrToken = NULL;
	INT_TYPE tokenType = 0;
	trotListRef *lrTokenChildren = NULL;
	INT_TYPE tokenChildrenCount = 0;
	INT_TYPE tokenChildrenIndex = 0;

	trotListRef *lrChildToken = NULL;

	INT_TYPE childTokenType = 0;


	/* PRECOND */
	PRECOND_ERR_IF( lrTokenTree == NULL );
	PRECOND_ERR_IF( lrFileList == NULL );

/* TODO: this will be removed once we actually use it */
(void)lrFileList;

	/* CODE */
/* TODO: this is almost the same code as "creatFinalList". maybe we can have a single function that goes through token trees that calls another function to do the work on each token? */
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

	/* get top token's type */
	rc = trotListRefGetInt( lrToken, TOKEN_INDEX_TYPE, &tokenType );
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

		if (    childTokenType == TOKEN_L_BRACKET
		     || childTokenType == TOKEN_L_BRACE
		   )
		{
			/* save our state */
			rc = trotListRefAppendListTwin( lrParentTokenStack, lrToken );
			ERR_IF_PASSTHROUGH;

			rc = trotListRefAppendInt( lrParentTokenIndicesStack, tokenChildrenIndex );
			ERR_IF_PASSTHROUGH;

			/* go down */
			
			/* free token, children, final list */
			trotListRefFree( &lrToken );
			trotListRefFree( &lrTokenChildren );

			/* set token equal to this child */
			lrToken = lrChildToken;
			lrChildToken = NULL;

			/* get token's children */
			rc = trotListRefGetListTwin( lrToken, TOKEN_INDEX_VALUE, &lrTokenChildren );
			ERR_IF_PASSTHROUGH;

			/* set index */
			tokenChildrenIndex = 0;

			continue;
		}
		else if ( childTokenType == TOKEN_L_PARENTHESIS )
		{
			/* handle */
			rc = handleMetaData2( lrToken, lrChildToken );
			ERR_IF_PASSTHROUGH;

			/* remove this token */
			rc = trotListRefRemove( lrTokenChildren, tokenChildrenIndex );
			ERR_IF_PASSTHROUGH;

			tokenChildrenIndex -= 1;
		}
	}


	/* CLEANUP */
	cleanup:

	trotListRefFree( &lrParentTokenStack );
	trotListRefFree( &lrParentTokenIndicesStack );
	trotListRefFree( &lrToken );
	trotListRefFree( &lrTokenChildren );

	trotListRefFree( &lrChildToken );

	return rc;
}

/******************************************************************************/
/*!
	\brief
	\param 
	\return TROT_RC
*/
static TROT_RC handleMetaData2( trotListRef *lrParentToken, trotListRef *lrParenthesisToken )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotListRef *lrChildren = NULL;
	INT_TYPE childrenCount = 0;
	INT_TYPE childrenIndex = 0;
	trotListRef *lrChildToken = NULL;
	INT_TYPE childTokenType = 0;

	trotListRef *lrChildValue = NULL;
	INT_TYPE childValueCount = 0;
	INT_TYPE childValueIndex = 0;
	INT_TYPE childValueCharacter = 0;

	TROT_LIST_COMPARE_RESULT compareResult;

	trotListRef *lrParentTokenFinalList = NULL;

	trotListRef *lrEnumList = NULL;

	trotListRef *lrName = NULL;
	INT_TYPE nameCount = 0;

	/* PRECOND */
	PRECOND_ERR_IF( lrParentToken == NULL );
	PRECOND_ERR_IF( lrParenthesisToken == NULL );


	/* CODE */
	/* get parenthesis token's children */
	rc = trotListRefGetListTwin( lrParenthesisToken, TOKEN_INDEX_VALUE, &lrChildren );
	ERR_IF_PASSTHROUGH;

	/* get first token in parenthesis */
	rc = trotListRefGetCount( lrChildren, &childrenCount );
	ERR_IF_PASSTHROUGH;

	ERR_IF( childrenCount == 0, TROT_LIST_ERROR_DECODE );

	rc = trotListRefGetListTwin( lrChildren, 1, &lrChildToken );
	ERR_IF_PASSTHROUGH;

	/* get tokenType */
	rc = trotListRefGetInt( lrChildToken, TOKEN_INDEX_TYPE, &childTokenType );
	ERR_IF_PASSTHROUGH;

	ERR_IF( childTokenType != TOKEN_WORD, TROT_LIST_ERROR_DECODE );

	/* get value */
	rc = trotListRefGetListTwin( lrChildToken, TOKEN_INDEX_VALUE, &lrChildValue );
	ERR_IF_PASSTHROUGH;

	/* comment? */
	rc = compareListToCString( lrChildValue, "comment", &compareResult );
	ERR_IF_PASSTHROUGH;

	if ( compareResult == TROT_LIST_COMPARE_EQUAL )
	{
		/* nothing to do, just allow it (don't error) */
		goto cleanup;
	}

	/* tag? */
	rc = compareListToCString( lrChildValue, "tag", &compareResult );
	ERR_IF_PASSTHROUGH;

	if ( compareResult == TROT_LIST_COMPARE_EQUAL )
	{
		/* there should be only 2 children
		   one for 'tag' and one for the tag name */
		ERR_IF( childrenCount != 2, TROT_LIST_ERROR_DECODE );

		trotListRefFree( &lrChildValue );
		trotListRefFree( &lrChildToken );

		/* get finalList of parent token */
		rc = trotListRefGetListTwin( lrParentToken, TOKEN_INDEX_FINALLIST, &lrParentTokenFinalList );
		ERR_IF_PASSTHROUGH;

		/* get second token in parenthesis */
		rc = trotListRefGetListTwin( lrChildren, 2, &lrChildToken );
		ERR_IF_PASSTHROUGH;

		/* get tokenType */
		rc = trotListRefGetInt( lrChildToken, TOKEN_INDEX_TYPE, &childTokenType );
		ERR_IF_PASSTHROUGH;

		ERR_IF( childTokenType != TOKEN_WORD, TROT_LIST_ERROR_DECODE );

		/* get value */
		rc = trotListRefGetListTwin( lrChildToken, TOKEN_INDEX_VALUE, &lrChildValue );
		ERR_IF_PASSTHROUGH;
	
		/* text? */	
		rc = compareListToCString( lrChildValue, "text", &compareResult );
		ERR_IF_PASSTHROUGH;

		if ( compareResult == TROT_LIST_COMPARE_EQUAL )
		{
			/* TODO: make setting and getting tags a function */
			ERR_IF_PARANOID( lrParentTokenFinalList -> lPointsTo == NULL );
			lrParentTokenFinalList -> lPointsTo -> tag = TROT_TAG_TEXT;
		}

		/* TODO: all tags */
	}

	/* name? */
	rc = compareListToCString( lrChildValue, "name", &compareResult );
	ERR_IF_PASSTHROUGH;

	if ( compareResult == TROT_LIST_COMPARE_EQUAL )
	{
		/* there should be only 2 children
		   one for 'name' and one for the actual name */
		ERR_IF( childrenCount != 2, TROT_LIST_ERROR_DECODE );

		trotListRefFree( &lrChildValue );
		trotListRefFree( &lrChildToken );

		/* get second token in parenthesis */
		rc = trotListRefGetListTwin( lrChildren, 2, &lrChildToken );
		ERR_IF_PASSTHROUGH;

		/* get tokenType */
		rc = trotListRefGetInt( lrChildToken, TOKEN_INDEX_TYPE, &childTokenType );
		ERR_IF_PASSTHROUGH;

		ERR_IF( childTokenType != TOKEN_WORD, TROT_LIST_ERROR_DECODE );

		/* get value */
		rc = trotListRefGetListTwin( lrChildToken, TOKEN_INDEX_VALUE, &lrChildValue );
		ERR_IF_PASSTHROUGH;

		/* name cannot contain a period */
		rc = trotListRefGetCount( lrChildValue, &childValueCount );
		ERR_IF_PASSTHROUGH;

		/* note: no need to test that name is actually non-empty because our
		         tokenizer wouldn't tokenize an empty word */

		childValueIndex = 1;
		while ( childValueIndex <= childValueCount )
		{
			rc = trotListRefGetInt( lrChildValue, childValueIndex, &childValueCharacter );
			ERR_IF_PASSTHROUGH;

			ERR_IF( childValueCharacter == '.', TROT_LIST_ERROR_DECODE );

			/* increment */
			childValueIndex += 1;
		}

		/* remove name */
		rc = trotListRefRemoveList( lrParentToken, TOKEN_INDEX_NAME, &lrName );
		ERR_IF_PASSTHROUGH;

		/* current name must have been empty */
		rc = trotListRefGetCount( lrName, &nameCount );
		ERR_IF_PASSTHROUGH;

		ERR_IF( nameCount != 0, TROT_LIST_ERROR_DECODE );

		/* put new name */
		rc = trotListRefInsertListTwin( lrParentToken, TOKEN_INDEX_NAME, lrChildValue );
		ERR_IF_PASSTHROUGH;
	}

	/* enum? */
	rc = compareListToCString( lrChildValue, "enum", &compareResult );
	ERR_IF_PASSTHROUGH;

	if ( compareResult == TROT_LIST_COMPARE_EQUAL )
	{
		/* there should be more than 1
		   one for 'enum' and one more for each [name value] */
		ERR_IF( childrenCount == 1, TROT_LIST_ERROR_DECODE );

		/* get enum list of parent */
		rc = trotListRefGetListTwin( lrParentToken, TOKEN_INDEX_ENUMS, &lrEnumList );
		ERR_IF_PASSTHROUGH;

		/* foreach new enum */
		childrenIndex = 2; /* 1 was "enum" */
		while ( childrenIndex <= childrenCount )
		{
			trotListRefFree( &lrChildValue );
			trotListRefFree( &lrChildToken );

			/* get next child */
			rc = trotListRefGetListTwin( lrChildren, childrenIndex, &lrChildToken );
			ERR_IF_PASSTHROUGH;

			/* get tokenType */
			rc = trotListRefGetInt( lrChildToken, TOKEN_INDEX_TYPE, &childTokenType );
			ERR_IF_PASSTHROUGH;

			ERR_IF( childTokenType != TOKEN_L_PARENTHESIS, TROT_LIST_ERROR_DECODE );

			/* add enum */
			rc = addEnum( lrEnumList, lrChildToken );
			ERR_IF_PASSTHROUGH;

			/* increment index */
			childrenIndex += 1;
		}
	}

	/* TODO: error if we dont' recognize first word in parenthesis */


	/* CLEANUP */
	cleanup:

	trotListRefFree( &lrChildren );
	trotListRefFree( &lrChildToken );
	trotListRefFree( &lrChildValue );
	trotListRefFree( &lrParentTokenFinalList );

	return rc;
}

/******************************************************************************/
/*!
	\brief Handles words.
	\param lrTokenTree Token Tree.
	\return TROT_RC

	A word can either be:
	- name of enum
	- name of list
	- name of op
	- TODO? name of function var
*/
static TROT_RC handleAllWords( trotListRef *lrTokenTree )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotListRef *lrParentTokenStack = NULL;
	INT_TYPE parentTokenStackCount = 0;

	trotListRef *lrParentTokenIndicesStack = NULL;

	trotListRef *lrToken = NULL;
	INT_TYPE tokenType = 0;
	trotListRef *lrTokenChildren = NULL;
	INT_TYPE tokenChildrenCount = 0;
	INT_TYPE tokenChildrenIndex = 0;

	trotListRef *lrChildToken = NULL;

	INT_TYPE childTokenType = 0;


	/* PRECOND */
	PRECOND_ERR_IF( lrTokenTree == NULL );


	/* CODE */
/* TODO: this is almost the same code as "creatFinalList". maybe we can have a single function that goes through token trees that calls another function to do the work on each token? */
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

	/* get top token's type */
	rc = trotListRefGetInt( lrToken, TOKEN_INDEX_TYPE, &tokenType );
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

		if (    childTokenType == TOKEN_L_BRACKET
		     || childTokenType == TOKEN_L_BRACE
		   )
		{
			/* save our state */
			rc = trotListRefAppendListTwin( lrParentTokenStack, lrToken );
			ERR_IF_PASSTHROUGH;

			rc = trotListRefAppendInt( lrParentTokenIndicesStack, tokenChildrenIndex );
			ERR_IF_PASSTHROUGH;

			/* go down */
			
			/* free token, children, final list */
			trotListRefFree( &lrToken );
			trotListRefFree( &lrTokenChildren );

			/* set token equal to this child */
			lrToken = lrChildToken;
			lrChildToken = NULL;

			/* get token's children */
			rc = trotListRefGetListTwin( lrToken, TOKEN_INDEX_VALUE, &lrTokenChildren );
			ERR_IF_PASSTHROUGH;

			/* set index */
			tokenChildrenIndex = 0;

			continue;
		}
		else if ( childTokenType == TOKEN_WORD )
		{
			/* lrToken isn't in parent Stack, so let's add it temporarily so handleWord will see it */
			/* TODO: we may need to rearrange this "go through token tree" code so lrToken is in the parent stack */
			rc = trotListRefAppendListTwin( lrParentTokenStack, lrToken );
			ERR_IF_PASSTHROUGH;

			rc = handleWord( lrParentTokenStack, lrChildToken );
			ERR_IF_PASSTHROUGH;

			/* remove lrToken from parent stack */
			rc = trotListRefRemove( lrParentTokenStack, -1 );
			ERR_IF_PASSTHROUGH;
		}
	}


	/* CLEANUP */
	cleanup:

	trotListRefFree( &lrParentTokenStack );
	trotListRefFree( &lrParentTokenIndicesStack );
	trotListRefFree( &lrToken );
	trotListRefFree( &lrTokenChildren );

	trotListRefFree( &lrChildToken );

	return rc;
}

/******************************************************************************/
/*!
	\brief 
	\param lrParentTokenStack Parent Token Stack.
	\param lrTokenWord Token that is the word we need to handle.
	\return TROT_RC
*/
static TROT_RC handleWord( trotListRef *lrParentTokenStack, trotListRef *lrTokenWord )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	INT_TYPE parentTokenStackIndex = 0;
	trotListRef *lrParent = NULL;

	trotListRef *lrWord = NULL;

	INT_TYPE found = 0;
	INT_TYPE value = 0;	


	/* PRECOND */
	PRECOND_ERR_IF( lrParentTokenStack == NULL );
	PRECOND_ERR_IF( lrTokenWord == NULL );


	/* CODE */
	/* get word value */
	rc = trotListRefGetListTwin( lrTokenWord, TOKEN_INDEX_VALUE, &lrWord );
	ERR_IF_PASSTHROUGH;

	/* get count */
	rc = trotListRefGetCount( lrParentTokenStack, &parentTokenStackIndex );
	ERR_IF_PASSTHROUGH;

	/* go "up" through parent stack */
	while ( parentTokenStackIndex > 0 )
	{
		/* get parent */
		trotListRefFree( &lrParent );
		rc = trotListRefGetListTwin( lrParentTokenStack, parentTokenStackIndex, &lrParent );
		ERR_IF_PASSTHROUGH;

		/* TODO: check for name */

		/* TODO: check for function var */

		/* TODO: maybe here, maybe somewhere else ... check for op value */

		/* see if it matches an enum value */
		rc = getEnumValue( lrParent, lrWord, &found, &value );
		ERR_IF_PASSTHROUGH;

		if ( found == 1 )
		{
			/* change token from word to number */
			/* set type */
			/* TODO: change this to a replace */
			rc = trotListRefRemove( lrTokenWord, TOKEN_INDEX_TYPE );
			ERR_IF_PASSTHROUGH;

			/* TODO: we should probably change our token type enums to be TOKEN_TYPE_~ instead of TOKEN_~ */
			rc = trotListRefInsertInt( lrTokenWord, TOKEN_INDEX_TYPE, TOKEN_NUMBER );
			ERR_IF_PASSTHROUGH;

			/* change value */
			/* TODO: change this to a replace */
			rc = trotListRefRemove( lrTokenWord, TOKEN_INDEX_VALUE );
			ERR_IF_PASSTHROUGH;

			rc = trotListRefInsertInt( lrTokenWord, TOKEN_INDEX_VALUE, value );
			ERR_IF_PASSTHROUGH;

			/* break */
			break;
		}

		/* decrement */
		parentTokenStackIndex -= 1;
	}


	/* CLEANUP */
	cleanup:

	trotListRefFree( &lrParent );
	trotListRefFree( &lrWord );

	return rc;
}

/******************************************************************************/
/*!
	\brief Adds enum to an enum list.
	\param lrEnumList Enum list.
	\param lrEnum Enum. Should be a L_BRACKET token with 2 children. 1 word and 1 number
	\return TROT_RC
*/
static TROT_RC addEnum( trotListRef *lrEnumList, trotListRef *lrEnum )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotListRef *lrEnumChildren = NULL;
	INT_TYPE enumChildrenCount = 0;
	trotListRef *lrEnumChild = NULL;
	INT_TYPE enumChildTokenType = 0;
	trotListRef *lrEnumName = NULL;
	INT_TYPE enumValue = 0;

	INT_TYPE enumListCount = 0;
	INT_TYPE enumListIndex = 0;
	trotListRef *lrEnumListEnum = NULL;
	trotListRef *lrEnumListEnumName = NULL;

	TROT_LIST_COMPARE_RESULT compareResult = TROT_LIST_COMPARE_EQUAL;

	trotListRef *lrNewEnum = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( lrEnumList == NULL );
	PRECOND_ERR_IF( lrEnum == NULL );


	/* CODE */
	/* *** */
	/* make sure enum has only 2 children, 1 word and 1 number */

	/* get children */
	rc = trotListRefGetListTwin( lrEnum, TOKEN_INDEX_VALUE, &lrEnumChildren );
	ERR_IF_PASSTHROUGH;

	/* get count */
	rc = trotListRefGetCount( lrEnumChildren, &enumChildrenCount );
	ERR_IF_PASSTHROUGH;

	/* must be 2 */
	ERR_IF( enumChildrenCount != 2, TROT_LIST_ERROR_DECODE );

	/* get 1st child */
	rc = trotListRefGetListTwin( lrEnumChildren, 1, &lrEnumChild );
	ERR_IF_PASSTHROUGH;

	/* make sure it's a TOKEN_WORD */
	rc = trotListRefGetInt( lrEnumChild, TOKEN_INDEX_TYPE, &enumChildTokenType );
	ERR_IF_PASSTHROUGH;

	ERR_IF( enumChildTokenType != TOKEN_WORD, TROT_LIST_ERROR_DECODE );

	/* get it's value, which is our enum name */
	rc = trotListRefGetListTwin( lrEnumChild, TOKEN_INDEX_VALUE, &lrEnumName );
	ERR_IF_PASSTHROUGH;

	/* get 2nd child */
	trotListRefFree( &lrEnumChild );
	rc = trotListRefGetListTwin( lrEnumChildren, 2, &lrEnumChild );
	ERR_IF_PASSTHROUGH;

	/* make sure it's a TOKEN_NUMBER */
	rc = trotListRefGetInt( lrEnumChild, TOKEN_INDEX_TYPE, &enumChildTokenType );
	ERR_IF_PASSTHROUGH;

	ERR_IF( enumChildTokenType != TOKEN_NUMBER, TROT_LIST_ERROR_DECODE );

	/* get it's value, which is our enum value */
	rc = trotListRefGetInt( lrEnumChild, TOKEN_INDEX_VALUE, &enumValue );
	ERR_IF_PASSTHROUGH;

	/* *** */
	/* make sure this name isn't already in our enum list */

	/* get count */
	rc = trotListRefGetCount( lrEnumList, &enumListCount );
	ERR_IF_PASSTHROUGH;

	/* foreach enum in enumList */
	enumListIndex = 1;
	while ( enumListIndex <= enumListCount )
	{
		/* get enum */
		trotListRefFree( &lrEnumListEnum );
		rc = trotListRefGetListTwin( lrEnumList, enumListIndex, &lrEnumListEnum );
		ERR_IF_PASSTHROUGH;

		/* get name */
		trotListRefFree( &lrEnumListEnumName );
		rc = trotListRefGetListTwin( lrEnumListEnum, 1, &lrEnumListEnumName );
		ERR_IF_PASSTHROUGH;

		/* compare */
		rc = trotListRefCompare( lrEnumListEnumName, lrEnumName, &compareResult );
		ERR_IF_PASSTHROUGH;

		/* if they matched, then this name is already in the list */
		ERR_IF( compareResult == TROT_LIST_COMPARE_EQUAL, TROT_LIST_ERROR_DECODE );

		/* increment index */
		enumListIndex += 1;
	}

	/* TODO: make sure it's also not name of list, or name of function var */
	/* TODO: maybe we can use a "find name" function to factor out this code? */

	/* *** */
	/* create new enum and append it to the list */
	rc = trotListRefInit( &lrNewEnum );
	ERR_IF_PASSTHROUGH;

	rc = trotListRefAppendListTwin( lrNewEnum, lrEnumName );
	ERR_IF_PASSTHROUGH;

	rc = trotListRefAppendInt( lrNewEnum, enumValue );
	ERR_IF_PASSTHROUGH;

	rc = trotListRefAppendListTwin( lrEnumList, lrNewEnum );
	ERR_IF_PASSTHROUGH;
	

	/* CLEANUP */
	cleanup:

	trotListRefFree( &lrEnumChildren );
	trotListRefFree( &lrEnumChild );
	trotListRefFree( &lrEnumName );
	trotListRefFree( &lrEnumListEnum );
	trotListRefFree( &lrEnumListEnumName );
	trotListRefFree( &lrNewEnum );

	return rc;
}

/******************************************************************************/
/*!
	\brief Gets enum value from token.
	\param lrToken Token.
	\param lrName Name of enum.
	\param found On function success, 1 if enum name was found and 0 if enum name was not found.
	\param value On function success and if found was 1, this will be the value of enum.
	\return TROT_RC
*/
static TROT_RC getEnumValue( trotListRef *lrToken, trotListRef *lrName, INT_TYPE *found, INT_TYPE *value )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotListRef *lrEnumList = NULL;
	INT_TYPE enumListCount = 0;
	INT_TYPE enumListIndex = 0;
	trotListRef *lrEnum = NULL;
	trotListRef *lrEnumName = NULL;

	TROT_LIST_COMPARE_RESULT result;


	/* PRECOND */
	PRECOND_ERR_IF( lrToken == NULL );
	PRECOND_ERR_IF( lrName == NULL );
	PRECOND_ERR_IF( found == NULL );
	PRECOND_ERR_IF( value == NULL );


	/* CODE */
	/* get enum list */
	rc = trotListRefGetListTwin( lrToken, TOKEN_INDEX_ENUMS, &lrEnumList );
	ERR_IF_PASSTHROUGH;

	/* set found to false */
	(*found) = 0;

	/* foreach enum */
	rc = trotListRefGetCount( lrEnumList, &enumListCount );
	ERR_IF_PASSTHROUGH;

	enumListIndex = 1;
	while ( enumListIndex <= enumListCount )
	{
		/* get enum */
		trotListRefFree( &lrEnum );
		rc = trotListRefGetListTwin( lrEnumList, enumListIndex, &lrEnum );
		ERR_IF_PASSTHROUGH;

		/* get name */
		trotListRefFree( &lrEnumName );
		rc = trotListRefGetListTwin( lrEnum, 1, &lrEnumName );
		ERR_IF_PASSTHROUGH;

		/* are these a match? */
		rc = trotListRefCompare( lrName, lrEnumName, &result );
		ERR_IF_PASSTHROUGH;

		if ( result == TROT_LIST_COMPARE_EQUAL )
		{
			/* get enum value */
			rc = trotListRefGetInt( lrEnum, 2, value );
			ERR_IF_PASSTHROUGH;

			/* signal found */
			(*found) = 1;
			break;
		}

		/* increment */
		enumListIndex += 1;
	}

	/* CLEANUP */
	cleanup:

	trotListRefFree( &lrEnumList );
	trotListRefFree( &lrEnum );
	trotListRefFree( &lrEnumName );

	return rc;
}


/* TODO: move this into some "tools.c" or "misc.c" file? */
/******************************************************************************/
/*!
	\brief Compares list to a c string.
	\param lrValue The list.
	\param cstring c string to compare against lrValue.
	\param result On success, the result of the comparison.
	\return TROT_RC
*/
static TROT_RC compareListToCString( trotListRef *lrValue, char *cstring, TROT_LIST_COMPARE_RESULT *result )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	trotListRef *lrCString = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( lrValue == NULL );
	PRECOND_ERR_IF( cstring == NULL );
	PRECOND_ERR_IF( result == NULL );


	/* CODE */
	rc = trotListRefInit( &lrCString );
	ERR_IF_PASSTHROUGH;

	while ( *cstring != '\0' )
	{
		rc = trotListRefAppendInt( lrCString, (*cstring) );
		ERR_IF_PASSTHROUGH;

		cstring += 1;
	}

	rc = trotListRefCompare( lrValue, lrCString, result );
	ERR_IF_PASSTHROUGH;


	/* CLEANUP */
	cleanup:

	trotListRefFree( &lrCString );

	return rc;
}

#if 0
/******************************************************************************/
/*!
	\brief Creates a new list of parts that were separated out of lr.
	\param lr List that contains parts.
	\param separator Separating character
	\param lrParts On success, contains the parts.
	\return TROT_RC

	lr is not modified.

	Example:
	IN:
		lr = ["abc.def.ghi"]
		separator = '.'
	OUT:
		lr = ["abc.def.ghi"]
		lrParts will be [["abc]["def"]["ghi"]]
*/
static TROT_RC splitList( trotListRef *lr, INT_TYPE separator trotListRef **lrPartList )
{
	/* DATA */
	TROT_RC rc = TROT_LIST_SUCCESS;

	INT_TYPE count = 0;
	INT_TYPE index = 0;
	INT_TYPE character = 0;

	trotListRef *newLrPartList = NULL;
	trotListRef *LrPart = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( lr == NULL );
	PRECOND_ERR_IF( lrPartList == NULL );
	PRECOND_ERR_IF( (*lrPartList) != NULL );


	/* CODE */
	/* create giveback */
	rc = trotListRefInit( &newLrPartList );
	ERR_IF_PASSTHROUGH;

	rc = trotListRefInit( &LrPart );
	ERR_IF_PASSTHROUGH;

	rc = trotListRefAppendListTwin( newLrPartList, lrPart );
	ERR_IF_PASSTHROUGH;

	/* get count */
	rc = trotListRefGetCount( lr, &count );
	ERR_IF_PASSTHROUGH;

	/* foreach character */
	index = 1;
	while ( index <= count )
	{
		/* get next character */
		rc = trotListRefGetInt( lr, index, &character );
		ERR_IF_PASSTHROUGH;

		/* if separator */
		if ( character == separator )
		{
			trotListRefFree( &lrPart );

			rc = trotListRefInit( &lrPart );
			ERR_IF_PASSTHROUGH;

			rc = trotListRefAppendListTwin( newLrPartList, lrPart );
			ERR_IF_PASSTHROUGH;
		}
		/* else add to current part */
		else
		{
			rc = trotListRefAppendInt( lrPart, character );
			ERR_IF_PASSTHROUGH;
		}

		/* increment index */
		index += 1;
	}

	/* give back */
	(*lrPartList) = lrNewPartList;
	lrNewPartList = NULL;


	/* CLEANUP */
	cleanup:

	trotListRefFree( &lrNewPartList );
	trotListRefFree( &lrPart );

	return rc;
}
#endif

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
	INT_TYPE childTokenValueChildrenCount = 0;
	INT_TYPE childTokenValueChildrenIndex = 0;
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
				/* get string */
				trotListRefFree( &lrChildTokenValue );

				rc = trotListRefGetListTwin( lrChildToken, TOKEN_INDEX_VALUE, &lrChildTokenValue );
				ERR_IF_PASSTHROUGH;

				rc = trotListRefGetCount( lrChildTokenValue, &childTokenValueChildrenCount );
				ERR_IF_PASSTHROUGH;

				/* append each number */
				childTokenValueChildrenIndex = 1;
				while ( childTokenValueChildrenIndex <= childTokenValueChildrenCount )
				{
					rc = trotListRefGetInt( lrChildTokenValue, childTokenValueChildrenIndex, &childTokenValueInt );
					ERR_IF_PASSTHROUGH;

					rc = trotListRefAppendInt( lrTokenFinalList, childTokenValueInt );
					ERR_IF_PASSTHROUGH;

					childTokenValueChildrenIndex += 1;
				}
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

#if 1
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

	INT_TYPE tokenType = 0; /* TODO: do we have an enum for this? */

	trotListRef *lrChildren = NULL;
	trotListRef *lrChildToken = NULL;
	INT_TYPE childTokenType = 0;
	INT_TYPE childTokenValueInt = 0;

	trotListRef *lrValue = NULL;

	trotListRef *lrUtf8Bytes = NULL;
	INT_TYPE utf8Count = 0;
	INT_TYPE utf8Index = 0;
	INT_TYPE utf8Byte = 0;


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

	/* get tokenType */
	rc = trotListRefGetInt( lrTokenTree, TOKEN_INDEX_TYPE, &tokenType );
	ERR_IF_PASSTHROUGH;

	if ( tokenType == TOKEN_L_BRACKET )
	{
		printf( "[ " );
	}
	else if ( tokenType == TOKEN_L_BRACE )
	{
		printf( "{ " );
	}
	else if ( tokenType == TOKEN_L_PARENTHESIS )
	{
		printf( "( " );
	}

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
				trotPrintTokenTree( lrChildToken, indent + 1 );
				for ( j = 0; j < indent; j += 1 )
				{
					printf( "    " );
				}
				break;

			case TOKEN_R_PARENTHESIS:
				ERR_IF_PARANOID( 1 );
				break;

			case TOKEN_WORD:
			case TOKEN_STRING:
				/* get value */
				trotListRefFree( &lrValue );

				rc = trotListRefGetListTwin( lrChildToken, TOKEN_INDEX_VALUE, &lrValue );
				ERR_IF_PASSTHROUGH;

				/* convert to utf8 */
				trotListRefFree( &lrUtf8Bytes );

				rc = trotListRefInit( &lrUtf8Bytes );
				ERR_IF_PASSTHROUGH;

				rc = trotCharactersToUtf8( lrValue, lrUtf8Bytes );
				ERR_IF_PASSTHROUGH;

				if ( childTokenType == TOKEN_STRING )
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
				

				if ( childTokenType == TOKEN_STRING )
				{
					printf( "\" " );
				}
				else
				{
					printf( " " );
				}
				break;

			case TOKEN_NUMBER:
				/* get number */
				rc = trotListRefGetInt( lrChildToken, TOKEN_INDEX_VALUE, &childTokenValueInt );
				ERR_IF_PASSTHROUGH;

				printf( "%d ", childTokenValueInt );

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

	if ( tokenType == TOKEN_L_BRACKET )
	{
		printf( "]\n" );
	}
	else if ( tokenType == TOKEN_L_BRACE )
	{
		printf( "}\n" );
	}
	else if ( tokenType == TOKEN_L_PARENTHESIS )
	{
		printf( ")\n" );
	}

	trotListRefFree( &lrChildren );
	trotListRefFree( &lrChildToken );
	trotListRefFree( &lrValue );
	trotListRefFree( &lrUtf8Bytes );

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

