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

/* TODO: document this entire process somewhere
   TODO: try to consolidate the code that goes through trees
*/

/*
	TODO: document this somewhere

	filelist
		file
			1 = filename
			2 = top token

	INCLUDE token type
		value = file's top token
		file list = file's top token's final list
*/

/******************************************************************************/
#include "trot.h"
#include "trotInternal.h"

/******************************************************************************/
static TROT_RC addFileToFileList( TrotList *lFileList, TrotList *lFileName, TrotList **lFile_A );

static TROT_RC tokenListToTokenTree( TrotList *lTokenList, TrotList *lTokenTree );

static TROT_RC handleMetaData( TrotList *lTokenTree, TrotList *lFileList );
static TROT_RC handleMetaData2( TrotList *lFileList, TrotList *lParentTokenStack, TrotList *lParentToken, TrotList *lParenthesisToken );
static TROT_RC handleMetaDataName( TrotList *lParentTokenStack, TrotList *lParentToken, TrotList *lParenthesisTokenValue );
static TROT_RC handleMetaDataEnum( TrotList *lParentToken, TrotList *lParenthesisTokenValue );
static TROT_RC handleMetaDataInclude( TrotList *lFileList, TrotList *lParentToken, TrotList *lParenthesisTokenValue );
static TROT_RC handleMetaDataFunction( TrotList *lParentToken, TrotList *lParenthesisTokenValue );

static TROT_RC handleAllWords( TrotList *lTokenTree );
static TROT_RC handleWord( TrotList *lParentTokenStack, TROT_INT parentIndex, TrotList *lTokenWord );
static TROT_RC handleWordOp( TrotList *lTokenWord, int *wasOp );

static TROT_RC findParentName( TrotList *lParentTokenStack, TrotList *lName, int *foundName, TrotList **lParent_A, int *foundVar, TROT_INT *varIndex );
static TROT_RC findChildByNameList( TrotList *lParentTokenPassedIn, TrotList *lNameList, TrotList **lTokenFound, TROT_INT *enumFound );

static TROT_RC addEnum( TrotList *lEnumList, TrotList *lEnum );
static TROT_RC getEnumValue( TrotList *lToken, TrotList *lName, TROT_INT *found, TROT_INT *value );

static TROT_RC compareListToCString( TrotList *lValue, const char *cstring, TROT_LIST_COMPARE_RESULT *result );

static TROT_RC splitList( TrotList *l, TROT_INT separator, TrotList **lPartList );

static TROT_RC createFinalList( TrotList *lTokenTree );

/* TODO: make sure names cannot be same as op names */

/* TODO: in here, and elsewhere, if we ever have an "index count" or another TROT_INT we increment, we need to think about overflow */

/* FUTURE OPTIMIZATION: Some functions have similar, but not identical, "go through token tree" code.
   We may be able to create a single "go through token tree" function to factor out this common code. */

/******************************************************************************/
const char *opNames[] = {
	"+",
	"-",
	"*",
	"/",
	"mod",
	"<",
	">",
	"=",
	"and",
	"or",
	"not",
	"neg",
	"pushInt", /* TODO: these 2 are confusing. maybe rename so we know these are used to push something onto our functional stack while we're running */
	"pushList",
	"call",
	"change",
	"return",
	"yield",
	"loadVar",
	"saveVar", /* TODO: need to figure out our coroutine functions. we have yield, but what's the "resume" or whatever? */
	NULL /* sentinel */
};

/******************************************************************************/
/*!
	\brief Decodes a list of characters into a list.
	\param loadFunc Function to use to load bytes for "includes".
	\param lGivenFilenameOfCharacters The "filename" to give the characters.
	\param lCharacters Characters to decode.
	\param stack On success, the decoded list.
	\return TROT_RC
*/
TROT_RC trotDecodeCharacters( TrotLoadFunc loadFunc, TrotList *lGivenFilenameOfCharacters, TrotList *lCharacters, TrotList **lDecodedList_A )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	int pass = 0;

	TrotList *lFileList = NULL;
	TROT_INT fileCount = 0;
	TROT_INT fileIndex = 0;

	TrotList *lFile = NULL;
	TrotList *lFileName = NULL;

	TrotList *lFileCharacters = NULL;

	TrotList *lBytes = NULL;

	TrotList *lTokenList = NULL;
	TrotList *lTokenTree = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( loadFunc == NULL );
	PRECOND_ERR_IF( lGivenFilenameOfCharacters == NULL );
	PRECOND_ERR_IF( lCharacters == NULL );
	PRECOND_ERR_IF( lDecodedList_A == NULL );
	PRECOND_ERR_IF( (*lDecodedList_A) != NULL );


	/* CODE */
	/* create FileList */
	rc = trotListInit( &lFileList );
	ERR_IF_PASSTHROUGH;

	/* *** */
	/* create our first File */
	rc = addFileToFileList( lFileList, lGivenFilenameOfCharacters, NULL );
	ERR_IF_PASSTHROUGH;

	/* *** */
	/* twin lCharacters so we can "carry in" our first file's characters */
	rc = trotListTwin( lCharacters, &lFileCharacters );
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
			trotListFree( &lFile );

			rc = trotListGetList( lFileList, fileIndex, &lFile );
			ERR_IF_PASSTHROUGH;

			/* tokenize, tokenListToTokenTree, handle meta data */
			if ( pass == 1 )
			{
				/* if we don't have characters
				   (this will only NOT happen for first file) */
				if ( lFileCharacters == NULL )
				{
					/* get name */
					trotListFree( &lFileName );
					rc = trotListGetList( lFile, 1, &lFileName );
					ERR_IF_PASSTHROUGH;

					/* load file */
					trotListFree( &lBytes );
					rc = loadFunc( lFileName, &lBytes );
					ERR_IF_PASSTHROUGH;

					/* unicode conversion */
					rc = trotListInit( &lFileCharacters );
					ERR_IF_PASSTHROUGH;

					rc = trotUtf8ToCharacters( lBytes, lFileCharacters );
					ERR_IF_PASSTHROUGH;
				}

				/* *** */
				/* tokenize */
				trotListFree( &lTokenList );
				rc = trotTokenize( lFileCharacters, &lTokenList );
				ERR_IF_PASSTHROUGH;

				/* free lFileCharacters so it'll be NULL for next file */ /* TODO: mvoe this to above the use, isntead of here */
				trotListFree( &lFileCharacters );

				/* change token list into a token tree */

				/* get tokenTree */
				trotListFree( &lTokenTree );
				rc = trotListGetList( lFile, -1, &lTokenTree );
				ERR_IF_PASSTHROUGH;

				rc = tokenListToTokenTree( lTokenList, lTokenTree );
				ERR_IF_PASSTHROUGH;

				/* handle meta-data */
				rc = handleMetaData( lTokenTree, lFileList );
				ERR_IF_PASSTHROUGH;
			}
			/* handle words */
			else if ( pass == 2 )
			{
				/* get tokenTree */
				trotListFree( &lTokenTree );

				rc = trotListGetList( lFile, -1, &lTokenTree );
				ERR_IF_PASSTHROUGH;

				/* handle all words */
				rc = handleAllWords( lTokenTree );
				ERR_IF_PASSTHROUGH;
			}
			/* optimize code lists */
			else if ( pass == 3 )
			{
				/* FUTURE OPTIMIZATION */
			}
			/* create finalList */
			else
			{
				/* get tokenTree */
				trotListFree( &lTokenTree );

				rc = trotListGetList( lFile, -1, &lTokenTree );
				ERR_IF_PASSTHROUGH;

				/* create final list */
				rc = createFinalList( lTokenTree );
				ERR_IF_PASSTHROUGH;
			}

			/* update fileCount, in case we added more files */
			rc = trotListGetCount( lFileList, &fileCount );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			/* increment fileIndex */
			fileIndex += 1;

		} while ( fileIndex <= fileCount ); /* TODO: chagne do-while to while */

		/* next pass */
		pass += 1;

	} while ( pass <= 4 ); /* TODO: change do-while to while */

	/* get first file */
	trotListFree( &lFile );

	rc = trotListGetList( lFileList, 1, &lFile );
	ERR_IF_PASSTHROUGH;

	/* get it's tokenTree */
	trotListFree( &lTokenTree );

	rc = trotListGetList( lFile, -1, &lTokenTree );
	ERR_IF_PASSTHROUGH;

	/* give back */
	rc = trotListGetList( lTokenTree, TOKEN_INDEX_FINALLIST, lDecodedList_A );
	ERR_IF_PASSTHROUGH;

	PARANOID_ERR_IF( (*lDecodedList_A) == NULL );


	/* CLEANUP */
	cleanup:

	trotListFree( &lFileList );
	trotListFree( &lFile );
	trotListFree( &lFileName );
	trotListFree( &lFileCharacters );
	trotListFree( &lBytes );
	trotListFree( &lTokenList );
	trotListFree( &lTokenTree );

	return rc;
}

/******************************************************************************/
/*!
	\brief Decodes a file into a list.
	\param loadFunc Function to use to load bytes for "includes".
	\param lFilename Filename of file to load and decode.
	\param lDecodedList_A On success, the decoded list.
	\return TROT_RC
*/
TROT_RC trotDecodeFilename( TrotLoadFunc loadFunc, TrotList *lFilename, TrotList **lDecodedList_A )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotList *newDecodedList = NULL;

	TrotList *lBytes = NULL;
	TrotList *lCharacters = NULL;


	/* PRECOND */
	PRECOND_ERR_IF( loadFunc == NULL );
	PRECOND_ERR_IF( lFilename == NULL );
	PRECOND_ERR_IF( lDecodedList_A == NULL );
	PRECOND_ERR_IF( (*lDecodedList_A) != NULL );


	/* CODE */
	rc = loadFunc( lFilename, &lBytes );
	ERR_IF_PASSTHROUGH;

	rc = trotListInit( &lCharacters );
	ERR_IF_PASSTHROUGH;

	rc = trotUtf8ToCharacters( lBytes, lCharacters );
	ERR_IF_PASSTHROUGH;

	rc = trotDecodeCharacters( loadFunc, lFilename, lCharacters, &newDecodedList );
	ERR_IF_PASSTHROUGH;

	/* give back */
	(*lDecodedList_A) = newDecodedList;
	newDecodedList = NULL;


	/* CLEANUP */
	cleanup:

	trotListFree( &newDecodedList );
	trotListFree( &lCharacters );
	trotListFree( &lBytes );

	return rc;
}

/******************************************************************************/
/*!
	\brief 
	\param 
	\return TROT_RC
*/
static TROT_RC addFileToFileList( TrotList *lFileList, TrotList *lFileName, TrotList **lFile_A )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotList *lFile = NULL;
	TrotList *lFileTokenTree = NULL;


	/* CODE */
	PARANOID_ERR_IF( lFileList == NULL );
	PARANOID_ERR_IF( lFileName == NULL );
	PARANOID_ERR_IF( lFile_A != NULL && (*lFile_A) != NULL );

	/* create file */
	rc = trotListInit( &lFile );
	ERR_IF_PASSTHROUGH;

	/* add FileName to File */
	rc = trotListAppendList( lFile, lFileName );
	ERR_IF_PASSTHROUGH;

	/* create the implicit list token for the file */
	rc = trotCreateToken( 0, 0, TOKEN_TYPE_L_BRACKET, &lFileTokenTree );
	ERR_IF_PASSTHROUGH;

	/* add token tree to file */
	rc = trotListAppendList( lFile, lFileTokenTree );
	ERR_IF_PASSTHROUGH;

	/* add file to file list */
	rc = trotListAppendList( lFileList, lFile );
	ERR_IF_PASSTHROUGH;

	/* get final list, and give back */
	if ( lFile_A != NULL )
	{
		rc = trotListTwin( lFile, lFile_A );
		ERR_IF_PASSTHROUGH;
	}


	/* CLEANUP */
	cleanup:

	trotListFree( &lFile );
	trotListFree( &lFileTokenTree );

	return rc;
}

/******************************************************************************/
/*!
	\brief Change token list into a token tree.
	\param lTokenList Token List.
	\param lTokenTree Token Tree.
	\return TROT_RC

	This function takes a list of tokens, and turns it into a token tree, by putting
	tokens that come after { ( [ in their children lists.

	When we create a new "file" list, we go ahead and populate it with a "tokenTree",
	which is just a TOKEN_TYPE_L_BRACKET token.
	One reason we do this is so that the file will aleady have a "final list", which
	we need for twinning.
	That's why lTokenTree is passed in aleady-existing and isn't created.
*/
static TROT_RC tokenListToTokenTree( TrotList *lTokenList, TrotList *lTokenTree )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotList *lParentStack = NULL;
	TROT_INT parentStackCount = 0;

	TrotList *lParent = NULL;
	TrotList *lChildren = NULL;

	TROT_INT parentTokenType = 0;

	TrotList *lToken = NULL;

	TrotList *lFinalList = NULL;

	/* We only need this for when we have a BRACE as top token */
	TrotList *lVars = NULL; 

	TROT_INT tokenCount = 0;
	TROT_INT tokenIndex = 0;

	TROT_INT tokenType = 0;



	/* CODE */
	PARANOID_ERR_IF( lTokenList == NULL );
	PARANOID_ERR_IF( lTokenTree == NULL );

	/* *** */
	/* create our parentStack, so we can "go up" */
	rc = trotListInit( &lParentStack );
	ERR_IF_PASSTHROUGH;

	/* *** */
	rc = trotListTwin( lTokenTree, &lParent );
	ERR_IF_PASSTHROUGH;

	rc = trotListGetList( lParent, TOKEN_INDEX_VALUE, &lChildren );
	ERR_IF_PASSTHROUGH;

	/* *** */
	/* get count of tokens */
	rc = trotListGetCount( lTokenList, &tokenCount );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* you must have at least 2 tokens to be valid: "[]" */
	ERR_IF( tokenCount < 2, TROT_RC_ERROR_DECODE );

	/* get first token and make sure it's a [ or { */
	rc = trotListGetList( lTokenList, 1, &lToken );
	ERR_IF_PASSTHROUGH;

	/* get tokenType */
	rc = trotListGetInt( lToken, TOKEN_INDEX_TYPE, &tokenType );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	ERR_IF( tokenType != TOKEN_TYPE_L_BRACKET && tokenType != TOKEN_TYPE_L_BRACE, TROT_RC_ERROR_DECODE );

	/* since the first token we add to all files is TOKEN_TYPE_L_BRACKET, we need to make sure it actually
	   matches the first token in the file */
	if ( tokenType == TOKEN_TYPE_L_BRACE )
	{
		/* change type to L_BRACE */
		/* NOTE: We don't have to do this outside the if because it will aleady be L_BRACKET */
		rc = trotListReplaceWithInt( lParent, TOKEN_INDEX_TYPE, tokenType );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		/* set tag of final list to TAG_CODE */
		rc = trotListGetList( lParent, TOKEN_INDEX_FINALLIST, &lFinalList );
		ERR_IF_PASSTHROUGH;

		rc = trotListSetTag( lFinalList, TROT_TAG_CODE );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		/* add in 'vars' since L_BRACE needs it but L_BRACKET doesn't */
		rc = trotListInit( &lVars );
		ERR_IF_PASSTHROUGH;

		rc = trotListAppendList( lParent, lVars );
		ERR_IF_PASSTHROUGH;
	}

	/* add parent to our parentStack */
	rc = trotListAppendList( lParentStack, lParent );
	ERR_IF_PASSTHROUGH;

	/* foreach token */
	tokenIndex = 2;
	while ( tokenIndex <= tokenCount )
	{
		/* get next token */
		trotListFree( &lToken );

		rc = trotListGetList( lTokenList, tokenIndex, &lToken );
		ERR_IF_PASSTHROUGH;

		/* get tokenType */
		rc = trotListGetInt( lToken, TOKEN_INDEX_TYPE, &tokenType );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		/* handle token */
		switch ( tokenType )
		{
			case TOKEN_TYPE_L_BRACKET:
			case TOKEN_TYPE_L_PARENTHESIS:
			case TOKEN_TYPE_L_BRACE:
				/* add this to our currentChildren */
				rc = trotListAppendList( lChildren, lToken );
				ERR_IF_PASSTHROUGH;

				/* add parent to our parentStack */
				rc = trotListAppendList( lParentStack, lParent );
				ERR_IF_PASSTHROUGH;

				trotListFree( &lParent );
				rc = trotListTwin( lToken, &lParent );
				ERR_IF_PASSTHROUGH;

				/* get value */
				trotListFree( &lChildren );
				rc = trotListGetList( lToken, TOKEN_INDEX_VALUE, &lChildren );
				ERR_IF_PASSTHROUGH;

				/* set tag if L_BRACE */
				if ( tokenType == TOKEN_TYPE_L_BRACE )
				{
					/* get final list */
					trotListFree( &lFinalList );
					rc = trotListGetList( lToken, TOKEN_INDEX_FINALLIST, &lFinalList );
					ERR_IF_PASSTHROUGH;

					rc = trotListSetTag( lFinalList, TROT_TAG_CODE );
					PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );
				}

				break;

			case TOKEN_TYPE_R_BRACKET:
			case TOKEN_TYPE_R_PARENTHESIS:
			case TOKEN_TYPE_R_BRACE:
				/* make sure we can go up */
				rc = trotListGetCount( lParentStack, &parentStackCount );
				PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

				ERR_IF( parentStackCount == 0, TROT_RC_ERROR_DECODE );

				/* make sure parent matches */
				rc = trotListGetInt( lParent, TOKEN_INDEX_TYPE, &parentTokenType );
				PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

				ERR_IF( tokenType == TOKEN_TYPE_R_BRACKET && parentTokenType != TOKEN_TYPE_L_BRACKET, TROT_RC_ERROR_DECODE );
				ERR_IF( tokenType == TOKEN_TYPE_R_PARENTHESIS && parentTokenType != TOKEN_TYPE_L_PARENTHESIS, TROT_RC_ERROR_DECODE );
				ERR_IF( tokenType == TOKEN_TYPE_R_BRACE && parentTokenType != TOKEN_TYPE_L_BRACE, TROT_RC_ERROR_DECODE );

				/* get grandparent */
				trotListFree( &lParent );

				/* NOTE: this is PARANOID because we check that lParentStack isn't empty above */
				rc = trotListRemoveList( lParentStack, -1, &lParent );
				PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

				/* get children of parent */
				trotListFree( &lChildren );

				rc = trotListGetList( lParent, TOKEN_INDEX_VALUE, &lChildren );
				ERR_IF_PASSTHROUGH;
				
				break;

			default:
				PARANOID_ERR_IF(    tokenType != TOKEN_TYPE_WORD
				                 && tokenType != TOKEN_TYPE_NUMBER
				                 && tokenType != TOKEN_TYPE_STRING
				               );

				/* add this to our currentChildren */
				rc = trotListAppendList( lChildren, lToken );
				ERR_IF_PASSTHROUGH;

				break;
		}

		/* next */
		tokenIndex += 1;
	}

	/* make sure we have no parents */
	rc = trotListGetCount( lParentStack, &parentStackCount );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	ERR_IF( parentStackCount != 0, TROT_RC_ERROR_DECODE );


	/* CLEANUP */
	cleanup:

	trotListFree( &lParentStack );
	trotListFree( &lParent );
	trotListFree( &lChildren );
	trotListFree( &lToken );
	trotListFree( &lFinalList );
	trotListFree( &lVars );

	return rc;
}

/******************************************************************************/
/*!
	\brief Goes through tree dealing with PARENTHESIS tokens.
	\param lTokenTree Token Tree.
	\param lFileList File list.
	\return TROT_RC

	Goes through lTokenTree, passing L_PARENTHESIS tokens to handleMetaData2, and then removing them from the tree.
*/
static TROT_RC handleMetaData( TrotList *lTokenTree, TrotList *lFileList )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotList *lParentTokenStack = NULL;
	TROT_INT parentTokenStackCount = 0;

	TrotList *lParentTokenIndicesStack = NULL;

	TrotList *lParentTokenStateStack = NULL;
	/* signals if we can handle metadata or not.
	   we can't if we've seen another token type in the list.
	   so we set this to 0 once we've seen a non-L_PARENTHESIS token */
	TROT_INT stateCanHandleMetaData = 1;

	TrotList *lToken = NULL;
	TROT_INT tokenType = 0;
	TrotList *lTokenChildren = NULL;
	TROT_INT tokenChildrenCount = 0;
	TROT_INT tokenChildrenIndex = 0;

	TrotList *lChildToken = NULL;

	TROT_INT childTokenType = 0;


	/* CODE */
	PARANOID_ERR_IF( lTokenTree == NULL );
	PARANOID_ERR_IF( lFileList == NULL );

	/* create parent stack, so we can "go up" */
	rc = trotListInit( &lParentTokenStack );
	ERR_IF_PASSTHROUGH;

	rc = trotListInit( &lParentTokenIndicesStack );
	ERR_IF_PASSTHROUGH;

	rc = trotListInit( &lParentTokenStateStack );
	ERR_IF_PASSTHROUGH;

	/* *** */
	/* set token to lTokenTree */
	rc = trotListTwin( lTokenTree, &lToken );
	ERR_IF_PASSTHROUGH;

	/* get top token's children */
	rc = trotListGetList( lToken, TOKEN_INDEX_VALUE, &lTokenChildren );
	ERR_IF_PASSTHROUGH;

	/* get top token's type */
	rc = trotListGetInt( lToken, TOKEN_INDEX_TYPE, &tokenType );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* set index */
	tokenChildrenIndex = 0;

	/* go through tree */
	while ( 1 )
	{
		/* increment index */
		tokenChildrenIndex += 1;

		/* are we at end of children? */
		rc = trotListGetCount( lTokenChildren, &tokenChildrenCount );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		if ( tokenChildrenIndex > tokenChildrenCount )
		{
			/* free token, children, finalList */
			trotListFree( &lToken );
			trotListFree( &lTokenChildren );

			/* do we have any parents? */
			rc = trotListGetCount( lParentTokenStack, &parentTokenStackCount );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			/* no parents, so we're done */
			if ( parentTokenStackCount == 0 )
			{
				break;
			}

			/* yes parents, so go up */

			/* get token */
			/* NOTE: this is paranoid because we made sure lParentTokenStack wasn't empty above */
			rc = trotListRemoveList( lParentTokenStack, -1, &lToken );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			/* get top token's children */
			rc = trotListGetList( lToken, TOKEN_INDEX_VALUE, &lTokenChildren );
			ERR_IF_PASSTHROUGH;

			/* get index */
			rc = trotListRemoveInt( lParentTokenIndicesStack, -1, &tokenChildrenIndex );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			/* get state */
			rc = trotListRemoveInt( lParentTokenStateStack, -1, &stateCanHandleMetaData );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			continue;
		}

		/* get next child token */
		trotListFree( &lChildToken );

		rc = trotListGetList( lTokenChildren, tokenChildrenIndex, &lChildToken );
		ERR_IF_PASSTHROUGH;

		/* get tokenType */
		rc = trotListGetInt( lChildToken, TOKEN_INDEX_TYPE, &childTokenType );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		if (    childTokenType == TOKEN_TYPE_L_BRACKET
		     || childTokenType == TOKEN_TYPE_L_BRACE
		   )
		{
			/* save our state */
			rc = trotListAppendList( lParentTokenStack, lToken );
			ERR_IF_PASSTHROUGH;

			rc = trotListAppendInt( lParentTokenIndicesStack, tokenChildrenIndex );
			ERR_IF_PASSTHROUGH;

			rc = trotListAppendInt( lParentTokenStateStack, 0 );
			ERR_IF_PASSTHROUGH;

			/* go down */
			
			/* free token, children, final list */
			trotListFree( &lToken );
			trotListFree( &lTokenChildren );

			/* set token equal to this child */
			lToken = lChildToken;
			lChildToken = NULL;

			/* get token's children */
			rc = trotListGetList( lToken, TOKEN_INDEX_VALUE, &lTokenChildren );
			ERR_IF_PASSTHROUGH;

			/* set index */
			tokenChildrenIndex = 0;

			/* set state */
			stateCanHandleMetaData = 1;

			continue;
		}
		else if ( childTokenType == TOKEN_TYPE_L_PARENTHESIS )
		{
			/* can we handle metaData?
			   (have we seen any non-L_PARENTHESIS tokens?)
			   this makes sure all metadata parenthesis are at the beginning
			   of lists */
			ERR_IF( stateCanHandleMetaData != 1, TROT_RC_ERROR_DECODE );

			/* handle */
			rc = handleMetaData2( lFileList, lParentTokenStack, lToken, lChildToken );
			ERR_IF_PASSTHROUGH;

			/* remove this token */
			rc = trotListRemove( lTokenChildren, tokenChildrenIndex );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			tokenChildrenIndex -= 1;
		}
		else
		{
			stateCanHandleMetaData = 0;
		}
	}


	/* CLEANUP */
	cleanup:

	trotListFree( &lParentTokenStack );
	trotListFree( &lParentTokenIndicesStack );
	trotListFree( &lParentTokenStateStack );
	trotListFree( &lToken );
	trotListFree( &lTokenChildren );
	trotListFree( &lChildToken );

	return rc;
}

/******************************************************************************/
/*!
	\brief Correctly handles a L_PARENTHESIS token.
	\param lFileList The top-level file list
	\param lParentTokenStack Parent token stack. Needed for (name)
	       so we can make sure it's not the same name as the
	       grand-parent's enum.
	\param lParentToken The parent token of the parenthesis token
	\param lParenthesisToken The parenthesis token
	\return TROT_RC

	Adds (name)
	Adds (enum)
	Adds (include) to file list (or twins if it's aleady there).
	Tags (function), (text), (raw) 
*/
static TROT_RC handleMetaData2( TrotList *lFileList, TrotList *lParentTokenStack, TrotList *lParentToken, TrotList *lParenthesisToken )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotList *lChildren = NULL;
	TROT_INT childrenCount = 0;
	TrotList *lChildToken = NULL;
	TROT_INT childTokenType = 0;

	TrotList *lChildValue = NULL;

	TROT_LIST_COMPARE_RESULT compareResult;

	TrotList *lParentTokenFinalList = NULL;


	static struct {
		char *tagName;
		TROT_TAG fromTag;
		TROT_TAG toTag;
	}
	tagData[ 3 ] =
	{
		{ "text",     TROT_TAG_DATA, TROT_TAG_TEXT     },
		{ "raw",      TROT_TAG_CODE, TROT_TAG_RAW_CODE },

		{ NULL,       TROT_TAG_DATA, TROT_TAG_DATA     } /* sentinel */
	};

	TROT_TAG tag;

	int i = 0;


	/* CODE */
	PARANOID_ERR_IF( lFileList == NULL );
	PARANOID_ERR_IF( lParentTokenStack == NULL );
	PARANOID_ERR_IF( lParentToken == NULL );
	PARANOID_ERR_IF( lParenthesisToken == NULL );

	/* get parenthesis token's children */
	rc = trotListGetList( lParenthesisToken, TOKEN_INDEX_VALUE, &lChildren );
	ERR_IF_PASSTHROUGH;

	/* get first token in parenthesis */
	rc = trotListGetCount( lChildren, &childrenCount );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	ERR_IF( childrenCount == 0, TROT_RC_ERROR_DECODE );

	rc = trotListGetList( lChildren, 1, &lChildToken );
	ERR_IF_PASSTHROUGH;

	/* get tokenType */
	rc = trotListGetInt( lChildToken, TOKEN_INDEX_TYPE, &childTokenType );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	ERR_IF( childTokenType != TOKEN_TYPE_WORD, TROT_RC_ERROR_DECODE );

	/* get value */
	rc = trotListGetList( lChildToken, TOKEN_INDEX_VALUE, &lChildValue );
	ERR_IF_PASSTHROUGH;

	/* name? */
	rc = compareListToCString( lChildValue, "name", &compareResult );
	ERR_IF_PASSTHROUGH;

	if ( compareResult == TROT_LIST_COMPARE_EQUAL )
	{
		rc = handleMetaDataName( lParentTokenStack, lParentToken, lChildren );
		ERR_IF_PASSTHROUGH;

		goto cleanup;
	}

	/* enum? */
	rc = compareListToCString( lChildValue, "enum", &compareResult );
	ERR_IF_PASSTHROUGH;

	if ( compareResult == TROT_LIST_COMPARE_EQUAL )
	{
		rc = handleMetaDataEnum( lParentToken, lChildren );
		ERR_IF_PASSTHROUGH;

		goto cleanup;
	}

	/* include? */
	rc = compareListToCString( lChildValue, "include", &compareResult );
	ERR_IF_PASSTHROUGH;

	if ( compareResult == TROT_LIST_COMPARE_EQUAL )
	{
		/* handle include */
		rc = handleMetaDataInclude( lFileList, lParentToken, lChildren );
		ERR_IF_PASSTHROUGH;

		goto cleanup;
	}

	/* tags... */

	/* function? */
	rc = compareListToCString( lChildValue, "function", &compareResult );
	ERR_IF_PASSTHROUGH;

	if ( compareResult == TROT_LIST_COMPARE_EQUAL )
	{
		/* get finalList of parent token */
		rc = trotListGetList( lParentToken, TOKEN_INDEX_FINALLIST, &lParentTokenFinalList );
		ERR_IF_PASSTHROUGH;

		/* make sure it wasn't tagged twice */
		rc = trotListGetTag( lParentTokenFinalList, &tag );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		ERR_IF( tag != TROT_TAG_CODE, TROT_RC_ERROR_DECODE );

		/* set tag */
		rc = trotListSetTag( lParentTokenFinalList, TROT_TAG_FUNCTION );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		/* handle vars */
		rc = handleMetaDataFunction( lParentToken, lChildren );
		ERR_IF_PASSTHROUGH;

		goto cleanup;
	}

	/* simple tags... */
	i = 0;
	while ( tagData[ i ].tagName != NULL )
	{
		/* compare */
		rc = compareListToCString( lChildValue, tagData[ i ].tagName, &compareResult );
		ERR_IF_PASSTHROUGH;

		if ( compareResult == TROT_LIST_COMPARE_EQUAL )
		{
			/* must be only token in parenthesis */
			ERR_IF( childrenCount != 1, TROT_RC_ERROR_DECODE );

			/* get finalList of parent token */
			trotListFree( &lParentTokenFinalList );
			rc = trotListGetList( lParentToken, TOKEN_INDEX_FINALLIST, &lParentTokenFinalList );
			ERR_IF_PASSTHROUGH;

			/* make sure it wasn't tagged twice */
			rc = trotListGetTag( lParentTokenFinalList, &tag );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			ERR_IF( tag != tagData[ i ].fromTag, TROT_RC_ERROR_DECODE );

			/* set tag */
			rc = trotListSetTag( lParentTokenFinalList, tagData[ i ].toTag );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			goto cleanup;
		}

		/* increment */
		i += 1;
	}

	/* error if we don't recognize first word in parenthesis */
	ERR_IF( 1, TROT_RC_ERROR_DECODE );


	/* CLEANUP */
	cleanup:

	trotListFree( &lChildren );
	trotListFree( &lChildToken );
	trotListFree( &lChildValue );
	trotListFree( &lParentTokenFinalList );

	return rc;
}

/******************************************************************************/
/*!
	\brief 
	\param 
	\return TROT_RC
*/
static TROT_RC handleMetaDataName( TrotList *lParentTokenStack, TrotList *lParentToken, TrotList *lParenthesisTokenValue )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TROT_INT parenthesisTokenValueCount = 0;
	TROT_INT childValueCount = 0;
	TROT_INT childValueIndex = 0;
	TROT_INT childValueCharacter = 0;

	TrotList *lChildToken = NULL;
	TROT_INT childTokenType = 0;
	TrotList *lChildValue = NULL;

	TROT_INT parentTokenStackCount = 0;
	TrotList *lGrandParentToken = NULL;
	TROT_INT enumFound = 0;
	TROT_INT enumValue = 0;

	TrotList *lName = NULL;
	TROT_INT nameCount = 0;


	/* CODE */
	PARANOID_ERR_IF( lParentTokenStack == NULL );
	PARANOID_ERR_IF( lParentToken == NULL );
	PARANOID_ERR_IF( lParenthesisTokenValue == NULL );

	/* get count */
	rc = trotListGetCount( lParenthesisTokenValue, &parenthesisTokenValueCount );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* there should be 2 */
	ERR_IF( parenthesisTokenValueCount != 2, TROT_RC_ERROR_DECODE );

	/* get second token in parenthesis */
	rc = trotListGetList( lParenthesisTokenValue, 2, &lChildToken );
	ERR_IF_PASSTHROUGH;

	/* get tokenType */
	rc = trotListGetInt( lChildToken, TOKEN_INDEX_TYPE, &childTokenType );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	ERR_IF( childTokenType != TOKEN_TYPE_WORD, TROT_RC_ERROR_DECODE );

	/* get value */
	rc = trotListGetList( lChildToken, TOKEN_INDEX_VALUE, &lChildValue );
	ERR_IF_PASSTHROUGH;

	/* name cannot contain a period */
	rc = trotListGetCount( lChildValue, &childValueCount );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* note: no need to test that name is actually non-empty because our
		 tokenizer wouldn't tokenize an empty word */

	childValueIndex = 1;
	while ( childValueIndex <= childValueCount )
	{
		rc = trotListGetInt( lChildValue, childValueIndex, &childValueCharacter );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		ERR_IF( childValueCharacter == '.', TROT_RC_ERROR_DECODE );

		/* increment */
		childValueIndex += 1;
	}

	/* name cannot be the name of our grand-parent's enum */

	/* do we have a grand-parent? */
	rc = trotListGetCount( lParentTokenStack, &parentTokenStackCount );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	if ( parentTokenStackCount > 0 )
	{
		/* get grand-parent */
		rc = trotListGetList( lParentTokenStack, -1, &lGrandParentToken );
		ERR_IF_PASSTHROUGH;

		/* is this name an enum in our grandparent? */
		rc = getEnumValue( lGrandParentToken, lChildValue, &enumFound, &enumValue );
		ERR_IF_PASSTHROUGH;

		ERR_IF( enumFound == 1, TROT_RC_ERROR_DECODE );
	}

	/* TODO: make sure name isn't the same as a sibling name */

	/* remove name */
	rc = trotListRemoveList( lParentToken, TOKEN_INDEX_NAME, &lName );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* current name must have been empty */
	rc = trotListGetCount( lName, &nameCount );
	PARANOID_ERR_IF( rc = TROT_RC_SUCCESS );

	ERR_IF( nameCount != 0, TROT_RC_ERROR_DECODE );

	/* put new name */
	rc = trotListInsertList( lParentToken, TOKEN_INDEX_NAME, lChildValue );
	ERR_IF_PASSTHROUGH;


	/* CLEANUP */
	cleanup:

	trotListFree( &lChildToken );
	trotListFree( &lChildValue );
	trotListFree( &lGrandParentToken );
	trotListFree( &lName );

	return rc;
}

/******************************************************************************/
/*!
	\brief 
	\param 
	\return TROT_RC
*/
static TROT_RC handleMetaDataEnum( TrotList *lParentToken, TrotList *lParenthesisTokenValue )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotList *lEnumList = NULL;

	TROT_INT parenthesisTokenValueCount = 0;
	TROT_INT parenthesisTokenValueIndex = 0;

	TrotList *lChildToken = NULL;
	TROT_INT childTokenType = 0;
	

	/* CODE */
	PARANOID_ERR_IF( lParentToken == NULL );
	PARANOID_ERR_IF( lParenthesisTokenValue == NULL );

	/* get enum list of parent */
	rc = trotListGetList( lParentToken, TOKEN_INDEX_ENUMS, &lEnumList );
	ERR_IF_PASSTHROUGH;

	/* get count */
	rc = trotListGetCount( lParenthesisTokenValue, &parenthesisTokenValueCount );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	ERR_IF( parenthesisTokenValueCount < 2, TROT_RC_ERROR_DECODE );

	/* foreach new enum */
	parenthesisTokenValueIndex = 2; /* 1 was "enum" */
	while ( parenthesisTokenValueIndex <= parenthesisTokenValueCount )
	{
		/* get next child */
		trotListFree( &lChildToken );
		rc = trotListGetList( lParenthesisTokenValue, parenthesisTokenValueIndex, &lChildToken );
		ERR_IF_PASSTHROUGH;

		/* get tokenType */
		rc = trotListGetInt( lChildToken, TOKEN_INDEX_TYPE, &childTokenType );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		ERR_IF( childTokenType != TOKEN_TYPE_L_PARENTHESIS, TROT_RC_ERROR_DECODE );

		/* add enum */
		rc = addEnum( lEnumList, lChildToken );
		ERR_IF_PASSTHROUGH;

		/* increment index */
		parenthesisTokenValueIndex += 1;
	}


	/* CLEANUP */
	cleanup:

	trotListFree( &lEnumList );
	trotListFree( &lChildToken );

	return rc;
}

/******************************************************************************/
/*!
	\brief 
	\param 
	\return TROT_RC
*/
static TROT_RC handleMetaDataInclude( TrotList *lFileList, TrotList *lParentToken, TrotList *lParenthesisTokenValue )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TROT_INT parenthesisTokenValueCount = 0;

	TrotList *lStringToken = NULL;
	TROT_INT stringTokenType = 0;
	TrotList *lStringTokenValue = NULL;
	TROT_INT stringTokenCount = 0;

	TROT_INT parentTokenType = 0;
	TrotList *lParentTokenValue = NULL;
	TROT_INT parentTokenCount = 0;

	TROT_INT fileListCount = 0;
	TROT_INT fileListIndex = 0;
	TrotList *lFile = NULL;
	TrotList *lFileName = NULL;
	TrotList *lFileToken = NULL;
	TrotList *lFileTokenFinalList = NULL;

	int fileNameFound = 0;

	TROT_LIST_COMPARE_RESULT compareResult = TROT_LIST_COMPARE_EQUAL;


	/* CODE */
	PARANOID_ERR_IF( lFileList == NULL );
	PARANOID_ERR_IF( lParentToken == NULL );
	PARANOID_ERR_IF( lParenthesisTokenValue == NULL );

	/* get count */
	rc = trotListGetCount( lParenthesisTokenValue, &parenthesisTokenValueCount );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* there should be 2 */
	ERR_IF( parenthesisTokenValueCount != 2, TROT_RC_ERROR_DECODE );

	/* get 2nd child */
	rc = trotListGetList( lParenthesisTokenValue, 2, &lStringToken );
	ERR_IF_PASSTHROUGH;

	/* get tokenType */
	rc = trotListGetInt( lStringToken, TOKEN_INDEX_TYPE, &stringTokenType );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	ERR_IF( stringTokenType != TOKEN_TYPE_STRING, TROT_RC_ERROR_DECODE );

	/* get value */
	rc = trotListGetList( lStringToken, TOKEN_INDEX_VALUE, &lStringTokenValue );
	ERR_IF_PASSTHROUGH;

	/* get count */
	rc = trotListGetCount( lStringTokenValue, &stringTokenCount );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	ERR_IF( stringTokenCount == 0, TROT_RC_ERROR_DECODE );

	/* now we have the name of the file to include in stringTokenValue */

	/* verify parentToken is a bracket and not a brace.
	   the file included may have braces as it's top-most list, but
	   we force includes to be brackets */
	rc = trotListGetInt( lParentToken, TOKEN_INDEX_TYPE, &parentTokenType );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	ERR_IF( parentTokenType != TOKEN_TYPE_L_BRACKET, TROT_RC_ERROR_DECODE );

	/* verify parentToken contains no more tokens.
	   (include) must be the last token in a list */
	rc = trotListGetList( lParentToken, TOKEN_INDEX_VALUE, &lParentTokenValue );
	ERR_IF_PASSTHROUGH;

	rc = trotListGetCount( lParentTokenValue, &parentTokenCount );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* 1 for the (include) token */
	ERR_IF( parentTokenCount != 1, TROT_RC_ERROR_DECODE );

	/* foreach file */
	rc = trotListGetCount( lFileList, &fileListCount );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	fileListIndex = 1;
	while ( fileListIndex <= fileListCount )
	{
		/* get file */
		trotListFree( &lFile );
		rc = trotListGetList( lFileList, fileListIndex, &lFile );
		ERR_IF_PASSTHROUGH;

		/* get it's file name */
		trotListFree( &lFileName );
		rc = trotListGetList( lFile, 1, &lFileName );
		ERR_IF_PASSTHROUGH;

		/* is it the same? */
		rc = trotListCompare( lFileName, lStringTokenValue, &compareResult );
		ERR_IF_PASSTHROUGH;
		
		if ( compareResult == TROT_LIST_COMPARE_EQUAL )
		{
			fileNameFound = 1;

			break;
		}

		/* increment */
		fileListIndex += 1;
	}

	/* if the filename isn't aleady in our file list, we need to create it */
	if ( fileNameFound == 0 )
	{
		trotListFree( &lFile );
		rc = addFileToFileList( lFileList, lStringTokenValue, &lFile );
		ERR_IF_PASSTHROUGH;
	}

	/* get file's token */
	rc = trotListGetList( lFile, -1, &lFileToken );
	ERR_IF_PASSTHROUGH;

	/* get file's token's final list */
	rc = trotListGetList( lFileToken, TOKEN_INDEX_FINALLIST, &lFileTokenFinalList );
	ERR_IF_PASSTHROUGH;

	/* change our parent token to be a TOKEN_TYPE_INCLUDE
	   put the lFile as it's value
	   put file's final list in token's final list */

	/* set type */
	rc = trotListReplaceWithInt( lParentToken, TOKEN_INDEX_TYPE, TOKEN_TYPE_INCLUDE );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* change value */
	rc = trotListReplaceWithList( lParentToken, TOKEN_INDEX_VALUE, lFile );
	ERR_IF_PASSTHROUGH;

	/* change final list */
	rc = trotListReplaceWithList( lParentToken, TOKEN_INDEX_FINALLIST, lFileTokenFinalList );
	ERR_IF_PASSTHROUGH;
	

	/* CLEANUP */
	cleanup:

	trotListFree( &lStringToken );
	trotListFree( &lStringTokenValue );
	trotListFree( &lParentTokenValue );
	trotListFree( &lFile );
	trotListFree( &lFileName );
	trotListFree( &lFileToken );
	trotListFree( &lFileTokenFinalList );

	return rc;
}

/******************************************************************************/
/*!
	\brief 
	\param 
	\return TROT_RC
*/
static TROT_RC handleMetaDataFunction( TrotList *lParentToken, TrotList *lParenthesisTokenValue )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotList *lVarList = NULL;

	TROT_INT parenthesisTokenValueCount = 0;
	TROT_INT parenthesisTokenValueIndex = 0;

	TrotList *lChildToken = NULL;
	TROT_INT childTokenType = 0;
	TrotList *lChildTokenValue = NULL;


	/* CODE */
	PARANOID_ERR_IF( lParentToken == NULL );
	PARANOID_ERR_IF( lParenthesisTokenValue == NULL );

	/* get var list of parent */
	rc = trotListGetList( lParentToken, TOKEN_INDEX_VAR, &lVarList );
	ERR_IF_PASSTHROUGH;

	/* get count */
	rc = trotListGetCount( lParenthesisTokenValue, &parenthesisTokenValueCount );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* foreach new var name */
	parenthesisTokenValueIndex = 2; /* 1 was "function" */
	while ( parenthesisTokenValueIndex <= parenthesisTokenValueCount )
	{
		/* get next child */
		trotListFree( &lChildToken );
		rc = trotListGetList( lParenthesisTokenValue, parenthesisTokenValueIndex, &lChildToken );
		ERR_IF_PASSTHROUGH;

		/* get tokenType */
		rc = trotListGetInt( lChildToken, TOKEN_INDEX_TYPE, &childTokenType );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		ERR_IF( childTokenType != TOKEN_TYPE_WORD, TROT_RC_ERROR_DECODE );

		/* get child value */
		trotListFree( &lChildTokenValue );
		rc = trotListGetList( lChildToken, TOKEN_INDEX_VALUE, &lChildTokenValue );
		ERR_IF_PASSTHROUGH;

		/* TODO: var name cannot contain periods or colons, other chars? */
		/* TODO: other names cannot contain periods, etc too ... elsewhere in the code we should check */

		/* add to var list */
		rc = trotListAppendList( lVarList, lChildTokenValue );
		ERR_IF_PASSTHROUGH;

		/* increment index */
		parenthesisTokenValueIndex += 1;
	}


	/* CLEANUP */
	cleanup:

	trotListFree( &lVarList );
	trotListFree( &lChildToken );
	trotListFree( &lChildTokenValue );

	return rc;
}

/******************************************************************************/
/*!
	\brief Handles words.
	\param lTokenTree Token Tree.
	\return TROT_RC

	Goes through entire token tree, and passes each word token to handleWord()
*/
static TROT_RC handleAllWords( TrotList *lTokenTree )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotList *lParentTokenStack = NULL;
	TROT_INT parentTokenStackCount = 0;

	TrotList *lParentTokenIndicesStack = NULL;

	TrotList *lToken = NULL;
	TROT_INT tokenType = 0;
	TrotList *lTokenChildren = NULL;
	TROT_INT tokenChildrenCount = 0;
	TROT_INT tokenChildrenIndex = 0;

	TrotList *lChildToken = NULL;

	TROT_INT childTokenType = 0;


	/* CODE */
	PARANOID_ERR_IF( lTokenTree == NULL );

	/* create parent stack, so we can "go up" */
	rc = trotListInit( &lParentTokenStack );
	ERR_IF_PASSTHROUGH;

	rc = trotListInit( &lParentTokenIndicesStack );
	ERR_IF_PASSTHROUGH;

	/* *** */
	/* set token to lTokenTree */
	rc = trotListTwin( lTokenTree, &lToken );
	ERR_IF_PASSTHROUGH;

	/* get top token's children */
	rc = trotListGetList( lToken, TOKEN_INDEX_VALUE, &lTokenChildren );
	ERR_IF_PASSTHROUGH;

	/* get top token's type */
	rc = trotListGetInt( lToken, TOKEN_INDEX_TYPE, &tokenType );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* set index */
	tokenChildrenIndex = 0;

	/* go through tree */
	while ( 1 )
	{
		/* increment index */
		tokenChildrenIndex += 1;

		/* are we at end of children? */
		rc = trotListGetCount( lTokenChildren, &tokenChildrenCount );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		if ( tokenChildrenIndex > tokenChildrenCount )
		{
			/* free token, children, finalList */
			trotListFree( &lToken );
			trotListFree( &lTokenChildren );

			/* do we have any parents? */
			rc = trotListGetCount( lParentTokenStack, &parentTokenStackCount );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			/* no parents, so we're done */
			if ( parentTokenStackCount == 0 )
			{
				break;
			}

			/* yes parents, so go up */

			/* get token */
			rc = trotListRemoveList( lParentTokenStack, -1, &lToken );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			/* get top token's children */
			rc = trotListGetList( lToken, TOKEN_INDEX_VALUE, &lTokenChildren );
			ERR_IF_PASSTHROUGH;

			/* get index */
			rc = trotListRemoveInt( lParentTokenIndicesStack, -1, &tokenChildrenIndex );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			continue;
		}

		/* get next child token */
		trotListFree( &lChildToken );

		rc = trotListGetList( lTokenChildren, tokenChildrenIndex, &lChildToken );
		ERR_IF_PASSTHROUGH;

		/* get tokenType */
		rc = trotListGetInt( lChildToken, TOKEN_INDEX_TYPE, &childTokenType );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		if (    childTokenType == TOKEN_TYPE_L_BRACKET
		     || childTokenType == TOKEN_TYPE_L_BRACE
		   )
		{
			/* save our state */
			rc = trotListAppendList( lParentTokenStack, lToken );
			ERR_IF_PASSTHROUGH;

			rc = trotListAppendInt( lParentTokenIndicesStack, tokenChildrenIndex );
			ERR_IF_PASSTHROUGH;

			/* go down */
			
			/* free token, children, final list */
			trotListFree( &lToken );
			trotListFree( &lTokenChildren );

			/* set token equal to this child */
			lToken = lChildToken;
			lChildToken = NULL;

			/* get token's children */
			rc = trotListGetList( lToken, TOKEN_INDEX_VALUE, &lTokenChildren );
			ERR_IF_PASSTHROUGH;

			/* set index */
			tokenChildrenIndex = 0;

			continue;
		}
		else if ( childTokenType == TOKEN_TYPE_WORD )
		{
			/* lToken isn't in parent Stack, so let's add it temporarily so handleWord will see it */
			rc = trotListAppendList( lParentTokenStack, lToken );
			ERR_IF_PASSTHROUGH;

			/* handle the word */
			rc = handleWord( lParentTokenStack, tokenChildrenIndex, lChildToken );
			ERR_IF_PASSTHROUGH;

			/* remove lToken from parent stack */
			rc = trotListRemove( lParentTokenStack, -1 );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );
		}
	}


	/* CLEANUP */
	cleanup:

	trotListFree( &lParentTokenStack );
	trotListFree( &lParentTokenIndicesStack );
	trotListFree( &lToken );
	trotListFree( &lTokenChildren );

	trotListFree( &lChildToken );

	return rc;
}

/******************************************************************************/
/*!
	\brief Handles a word token
	\param lParentTokenStack Parent Token Stack
	\param parentIndex Index of the word token in it's parent 
	\param lTokenWord Token that is the word we need to handle
	\return TROT_RC

	A word may be:
	- name of op
	- name of var in function to load
	- name of var in function to save to (if prefixed with ':')
	- name of list to twin
	- name of enum

	parentIndex is only needed because if the word is a var name, we have to change
	the token into a "save var op" or "load var op", and then append a "raw number"
	token (index of var) after the word (now op) token.
*/
static TROT_RC handleWord( TrotList *lParentTokenStack, TROT_INT parentIndex, TrotList *lTokenWord )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotList *lParent = NULL;
	TrotList *lParentValue = NULL;
	TrotList *lParentFinalList = NULL;

	TrotList *lWord = NULL;
	TrotList *lWordPartList = NULL;
	TROT_INT wordPartListCount = 0;
	TrotList *lWordPart = NULL;
	TROT_INT wordPartCount = 0;

	TROT_INT firstCharacter = 0;

	int wasOp = 0;

	int foundName = 0;
	int foundVar = 0;

	TROT_INT varIndex = 0;

	TrotList *lTokenFound = NULL;
	TrotList *lTokenFoundFinalList = NULL;
	TROT_INT enumFound = 0;

	TrotList *newLrToken = NULL;


	/* CODE */
	PARANOID_ERR_IF( lParentTokenStack == NULL );
	PARANOID_ERR_IF( lTokenWord == NULL );

	/* get word value */
	rc = trotListGetList( lTokenWord, TOKEN_INDEX_VALUE, &lWord );
	ERR_IF_PASSTHROUGH;

	/* does it start with ":" ? if so, it's a var save, and it should only have 1 part, and should match a var */
	rc = trotListGetInt( lWord, 1, &firstCharacter );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	if ( firstCharacter == ':' )
	{
		rc = trotListRemoveInt( lWord, 1, &firstCharacter );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );
	}

	/* split word up into its parts */
	rc = splitList( lWord, '.', &lWordPartList );
	ERR_IF_PASSTHROUGH;

	rc = trotListGetCount( lWordPartList, &wordPartListCount );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	if ( firstCharacter == ':' )
	{
		ERR_IF( wordPartListCount != 1, TROT_RC_ERROR_DECODE );
	}

	/* remove first word part */
	rc = trotListRemoveList( lWordPartList, 1, &lWordPart );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* it must not be empty */
	rc = trotListGetCount( lWordPart, &wordPartCount );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	ERR_IF( wordPartCount == 0, TROT_RC_ERROR_DECODE );

	/* is it an op? */
	if ( wordPartListCount == 1 )
	{
		/* TODO: this shouldnt happen if it started with ":" */
		rc = handleWordOp( lTokenWord, &wasOp );
		ERR_IF_PASSTHROUGH;

		if ( wasOp == 1 )
		{
			goto cleanup;
		}
	}

	rc = findParentName( lParentTokenStack, lWordPart, &foundName, &lParent, &foundVar, &varIndex );
	ERR_IF_PASSTHROUGH;

	PARANOID_ERR_IF( foundName == 1 && foundVar == 1 );
	PARANOID_ERR_IF( foundName == 1 && lParent == NULL );
	ERR_IF( foundName == 0 && foundVar == 0, TROT_RC_ERROR_DECODE );

	/* if we found a var */
	if ( foundVar == 1 )
	{
		ERR_IF( wordPartListCount != 1, TROT_RC_ERROR_DECODE );

		/* change word to op */

		/* set type */
		rc = trotListReplaceWithInt( lTokenWord, TOKEN_INDEX_TYPE, TOKEN_TYPE_OP );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		/* change value */
		if ( firstCharacter == ':' )
		{
			rc = trotListReplaceWithInt( lTokenWord, TOKEN_INDEX_VALUE, TROT_OP_SAVE_VAR );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );
		}
		else
		{
			rc = trotListReplaceWithInt( lTokenWord, TOKEN_INDEX_VALUE, TROT_OP_LOAD_VAR );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );
		}

		/* insert raw number token after our word token, which is now an op token */

		/* create our new raw number token, which will hold the var's index */
		rc = trotCreateToken( 1, 1, TOKEN_TYPE_NUMBER_RAW, &newLrToken );
		ERR_IF_PASSTHROUGH;

		/* add value */
		rc = trotListReplaceWithInt( newLrToken, TOKEN_INDEX_VALUE, varIndex );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		/* get parent */
		trotListFree( &lParent );
		rc = trotListGetList( lParentTokenStack, -1, &lParent );
		ERR_IF_PASSTHROUGH;

		/* get parent value */
		rc = trotListGetList( lParent, TOKEN_INDEX_VALUE, &lParentValue );
		ERR_IF_PASSTHROUGH;

		/* add new raw number token after our save/load var op */
		rc = trotListInsertList( lParentValue, parentIndex + 1, newLrToken );
		ERR_IF_PASSTHROUGH;

		goto cleanup;
	}

	/* implicit "else we found name" */

	/* was there only 1 word? */
	if ( wordPartListCount == 1 )
	{
		/* change token from word to twin */
		/* set type */
		rc = trotListReplaceWithInt( lTokenWord, TOKEN_INDEX_TYPE, TOKEN_TYPE_TWIN );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		/* change value */
		/* get parent's final list */
		rc = trotListGetList( lParent, TOKEN_INDEX_FINALLIST, &lParentFinalList );
		ERR_IF_PASSTHROUGH;

		rc = trotListReplaceWithList( lTokenWord, TOKEN_INDEX_VALUE, lParentFinalList );
		ERR_IF_PASSTHROUGH;

		goto cleanup;
	}

	/* find child */
	rc = findChildByNameList( lParent, lWordPartList, &lTokenFound, &enumFound );
	ERR_IF_PASSTHROUGH;

	/* did we find a token? */
	if ( lTokenFound != NULL )
	{
		/* change token from word to twin */
		/* set type */
		rc = trotListReplaceWithInt( lTokenWord, TOKEN_INDEX_TYPE, TOKEN_TYPE_TWIN );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		/* change value */
		/* get tokenFound's final list */
		rc = trotListGetList( lTokenFound, TOKEN_INDEX_FINALLIST, &lTokenFoundFinalList );
		ERR_IF_PASSTHROUGH;

		rc = trotListReplaceWithList( lTokenWord, TOKEN_INDEX_VALUE, lTokenFoundFinalList );
		ERR_IF_PASSTHROUGH;
	}
	/* else, we found enum */
	else
	{
		/* change token from word to number */
		/* set type */
		rc = trotListReplaceWithInt( lTokenWord, TOKEN_INDEX_TYPE, TOKEN_TYPE_NUMBER );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		/* change value */
		rc = trotListReplaceWithInt( lTokenWord, TOKEN_INDEX_VALUE, enumFound );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );
	}


	/* CLEANUP */
	cleanup:

	trotListFree( &lParent );
	trotListFree( &lParentValue );
	trotListFree( &lParentFinalList );
	trotListFree( &lWord );
	trotListFree( &lWordPartList );
	trotListFree( &lWordPart );
	trotListFree( &lTokenFound );
	trotListFree( &lTokenFoundFinalList );
	trotListFree( &newLrToken );

	return rc;
}

/******************************************************************************/
/*!
	\brief Checks to see if a word is an op, and if so changes the word token into an op token.
	\param lTokenWord Word token
	\param wasOp Is set to 0 or 1 to flag to the caller whether word was or was not an op.
	\return TROT_RC
*/
static TROT_RC handleWordOp( TrotList *lTokenWord, int *wasOp )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotList *lTokenWordValue = NULL;

	int i = 0;

	TROT_LIST_COMPARE_RESULT compareResult;


	/* CODE */
	PARANOID_ERR_IF( lTokenWord == NULL );
	PARANOID_ERR_IF( wasOp == NULL );

	(*wasOp) = 0;

	/* get value */
	rc = trotListGetList( lTokenWord, TOKEN_INDEX_VALUE, &lTokenWordValue );
	ERR_IF_PASSTHROUGH;

	i = 1;
	while ( opNames[ i - 1 ] != NULL )
	{
		rc = compareListToCString( lTokenWordValue, opNames[ i - 1 ], &compareResult );
		ERR_IF_PASSTHROUGH;

		if ( compareResult == TROT_LIST_COMPARE_EQUAL )
		{
			(*wasOp) = 1;

			/* change token */

			/* set type */
			rc = trotListReplaceWithInt( lTokenWord, TOKEN_INDEX_TYPE, TOKEN_TYPE_OP );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			/* change value */
			rc = trotListReplaceWithInt( lTokenWord, TOKEN_INDEX_VALUE, i );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			break;
		}

		/* increment */
		i += 1;
	}


	/* CLEANUP */
	cleanup:

	trotListFree( &lTokenWordValue );

	return rc;
}

/******************************************************************************/
/*!
	\brief 
	\param 
	\return TROT_RC
*/
static TROT_RC findParentName( TrotList *lParentTokenStack, TrotList *lName, int *foundName, TrotList **lParent_A, int *foundVar, TROT_INT *varIndex )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TROT_INT parentTokenStackIndex = 0;

	TrotList *lParent = NULL;
	TrotList *lParentName = NULL;
	TrotList *lParentFinalList = NULL;
	TrotList *lParentVarList = NULL;
	TrotList *lVar = NULL;
	TROT_INT parentVarListIndex = 0;
	TROT_INT parentVarListCount = 0; /* TODO: change this (and all others) to only need parentVarCount and go down to 0.
	                                      this would reduce our need for the index variable and comparing against count.
	                                      comparing against 0 may be faster? */

	int flagFunction = 0;

	TROT_LIST_COMPARE_RESULT compareResult = TROT_LIST_COMPARE_EQUAL;


	/* CODE */
	PARANOID_ERR_IF( lParentTokenStack == NULL );
	PARANOID_ERR_IF( lName == NULL );
	PARANOID_ERR_IF( foundName == NULL );
	PARANOID_ERR_IF( lParent_A == NULL );
	PARANOID_ERR_IF( (*lParent_A) != NULL );
	PARANOID_ERR_IF( foundVar == NULL );
	PARANOID_ERR_IF( varIndex == NULL );

	PARANOID_ERR_IF( (*foundName) != 0 );
	PARANOID_ERR_IF( (*foundVar) != 0 );

	/* get count */
	rc = trotListGetCount( lParentTokenStack, &parentTokenStackIndex );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* go "up" through parent stack */
	while ( parentTokenStackIndex > 0 )
	{
		/* get parent */
		trotListFree( &lParent );
		rc = trotListGetList( lParentTokenStack, parentTokenStackIndex, &lParent );
		ERR_IF_PASSTHROUGH;

		/* check for name */
		trotListFree( &lParentName );
		rc = trotListGetList( lParent, TOKEN_INDEX_NAME, &lParentName );
		ERR_IF_PASSTHROUGH;

		rc = trotListCompare( lName, lParentName, &compareResult );
		ERR_IF_PASSTHROUGH;
		
		if ( compareResult == TROT_LIST_COMPARE_EQUAL )
		{
			/* give back */
			rc = trotListTwin( lParent, lParent_A );
			ERR_IF_PASSTHROUGH;

			(*foundName) = 1;

			goto cleanup;
		}

		/* check vars */
		if ( flagFunction == 0 )
		{
			trotListFree( &lParentFinalList );
			rc = trotListGetList( lParent, TOKEN_INDEX_FINALLIST, &lParentFinalList );
			ERR_IF_PASSTHROUGH;

			/* is it a function? */
			if ( lParentFinalList -> laPointsTo -> tag == TROT_TAG_FUNCTION )
			{
				flagFunction = 1;

				/* get var list */
				trotListFree( &lParentVarList );
				rc = trotListGetList( lParent, TOKEN_INDEX_VAR, &lParentVarList );
				ERR_IF_PASSTHROUGH;

				/* get count */
				rc = trotListGetCount( lParentVarList, &parentVarListCount );
				PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

				/* foreach var */
				parentVarListIndex = 1;
				while ( parentVarListIndex <= parentVarListCount )
				{
					trotListFree( &lVar );
					rc = trotListGetList( lParentVarList, parentVarListIndex, &lVar );
					ERR_IF_PASSTHROUGH;

					/* compare */
					rc = trotListCompare( lName, lVar, &compareResult );
					ERR_IF_PASSTHROUGH;
		
					if ( compareResult == TROT_LIST_COMPARE_EQUAL )
					{
						(*foundVar) = 1;
						(*varIndex) = parentVarListIndex;

						goto cleanup;
					}
					
					/* increment */
					parentVarListIndex += 1;
				}	
			}
		}

		/* decrement */
		parentTokenStackIndex -= 1;
	}
	

	/* CLEANUP */
	cleanup:

	trotListFree( &lParent );
	trotListFree( &lParentName );
	trotListFree( &lParentFinalList );
	trotListFree( &lParentVarList );
	trotListFree( &lVar );

	return rc;
}

/******************************************************************************/
/*!
	\brief 
	\param 
	\return TROT_RC
*/
	
static TROT_RC findChildByNameList( TrotList *lParentTokenPassedIn, TrotList *lNameList, TrotList **lTokenFound, TROT_INT *enumFound )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotList *lParentToken = NULL;
	TrotList *lParentTokenValue = NULL;
	TROT_INT parentTokenValueCount = 0;
	TROT_INT parentTokenValueIndex = 0;

	TROT_INT foundEnum = 0;

	int foundChild = 0;

	TROT_INT nameListCount = 0;
	TROT_INT nameListIndex = 0;
	TrotList *lName = NULL;
	int isNumber = 0;
	TROT_INT nameNumber = 0;

	TrotList *lChildToken = NULL;
	TROT_INT childTokenType = 0; 
	TROT_INT childTokenCount = 0;
	TrotList *lChildTokenName = NULL;

	TrotList *lFile = NULL;

	TROT_LIST_COMPARE_RESULT compareResult = TROT_LIST_COMPARE_EQUAL;


	/* CODE */
	PARANOID_ERR_IF( lParentTokenPassedIn == NULL );
	PARANOID_ERR_IF( lNameList == NULL );
	PARANOID_ERR_IF( lTokenFound == NULL );
	PARANOID_ERR_IF( (*lTokenFound) != NULL );
	PARANOID_ERR_IF( enumFound == NULL );

	/* twin parentTokenPassedIn */
	rc = trotListTwin( lParentTokenPassedIn, &lParentToken );
	ERR_IF_PASSTHROUGH;

	/* get count of names */
	rc = trotListGetCount( lNameList, &nameListCount );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	PARANOID_ERR_IF( nameListCount == 0 );

	/* foreach name */
	nameListIndex = 1;
	while ( nameListIndex <= nameListCount )
	{
		/* get name */
		trotListFree( &lName );
		rc = trotListGetList( lNameList, nameListIndex, &lName );
		ERR_IF_PASSTHROUGH;

		/* get value */
		trotListFree( &lParentTokenValue );
		rc = trotListGetList( lParentToken, TOKEN_INDEX_VALUE, &lParentTokenValue );
		ERR_IF_PASSTHROUGH;

		/* get count */
		rc = trotListGetCount( lParentTokenValue, &parentTokenValueCount );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		/* set foundChild */
		foundChild = 0;

		/* is name a number? */
		rc =  _trotWordToNumber( lName, &isNumber, &nameNumber );
		ERR_IF_PASSTHROUGH;

		if ( isNumber == 1 )
		{
			trotListFree( &lChildToken );
			rc = trotListGetList( lParentTokenValue, nameNumber, &lChildToken );
			ERR_IF( rc == TROT_RC_ERROR_BAD_INDEX, TROT_RC_ERROR_DECODE );
			ERR_IF_PASSTHROUGH;

			foundChild = 1;
		}
		else
		{
			/* is last name? */
			if ( nameListIndex == nameListCount )
			{
				/* check for enums */
				rc = getEnumValue( lParentToken, lName, &foundEnum, enumFound );
				ERR_IF_PASSTHROUGH;

				if ( foundEnum == 1 )
				{
					/* break */
					break;
				}
			}

			/* foreach child of parent */
			parentTokenValueIndex = 1;
			while ( parentTokenValueIndex <= parentTokenValueCount )
			{
				/* get child */
				trotListFree( &lChildToken );
				rc = trotListGetList( lParentTokenValue, parentTokenValueIndex, &lChildToken );
				ERR_IF_PASSTHROUGH;

				/* get child count */
				rc = trotListGetCount( lChildToken, &childTokenCount );
				PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

				/* does child have a name? */
				if ( TOKEN_INDEX_NAME <= childTokenCount )
				{
					/* get child name */
					trotListFree( &lChildTokenName );
					rc = trotListGetList( lChildToken, TOKEN_INDEX_NAME, &lChildTokenName );
					ERR_IF_PASSTHROUGH;

					/* compare */
					rc = trotListCompare( lName, lChildTokenName, &compareResult );
					ERR_IF_PASSTHROUGH;

					if ( compareResult == TROT_LIST_COMPARE_EQUAL )
					{
						foundChild = 1;
						break;
					}
				}

				/* increment */
				parentTokenValueIndex += 1;
			}
		}

		ERR_IF( foundChild == 0, TROT_RC_ERROR_DECODE );

		/* go down */

		/* get child token type */
		rc = trotListGetInt( lChildToken, TOKEN_INDEX_TYPE, &childTokenType );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		/* if childToken is INCLUDE, get the token of the file that the include points to */
		if ( childTokenType == TOKEN_TYPE_INCLUDE )
		{
			rc = trotListGetList( lChildToken, TOKEN_INDEX_VALUE, &lFile );
			ERR_IF_PASSTHROUGH;

			trotListFree( &lParentToken );
			trotListFree( &lChildToken );
			rc = trotListGetList( lFile, -1, &lParentToken );
			ERR_IF_PASSTHROUGH;
		}
		/* else, go down normal */
		else
		{
			trotListFree( &lParentToken );
			lParentToken = lChildToken;
			lChildToken = NULL;
		}

		/* increment */
		nameListIndex += 1;
	}

	/* twin for "give back */
	if ( foundChild == 1 )
	{
		rc = trotListTwin( lParentToken, lTokenFound );
		ERR_IF_PASSTHROUGH;
	}
	

	/* CLEANUP */
	cleanup:

	trotListFree( &lParentToken );
	trotListFree( &lParentTokenValue );
	trotListFree( &lName );
	trotListFree( &lChildToken );
	trotListFree( &lChildTokenName );
	trotListFree( &lFile );

	return rc;
}

/******************************************************************************/
/*!
	\brief Adds enum to an enum list.
	\param lEnumList Enum list.
	\param lEnum Enum. Should be a L_BRACKET token with 2 children. 1 word and 1 number
	\return TROT_RC
*/
static TROT_RC addEnum( TrotList *lEnumList, TrotList *lEnum )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotList *lEnumChildren = NULL;
	TROT_INT enumChildrenCount = 0;
	TrotList *lEnumChild = NULL;
	TROT_INT enumChildTokenType = 0;
	TrotList *lEnumName = NULL;
	TROT_INT enumValue = 0;

	TROT_INT enumListCount = 0;
	TROT_INT enumListIndex = 0;
	TrotList *lEnumListEnum = NULL;
	TrotList *lEnumListEnumName = NULL;

	TROT_LIST_COMPARE_RESULT compareResult = TROT_LIST_COMPARE_EQUAL;

	TrotList *lNewEnum = NULL;


	/* CODE */
	PARANOID_ERR_IF( lEnumList == NULL );
	PARANOID_ERR_IF( lEnum == NULL );

	/* *** */
	/* make sure enum has only 2 children, 1 word and 1 number */

	/* get children */
	rc = trotListGetList( lEnum, TOKEN_INDEX_VALUE, &lEnumChildren );
	ERR_IF_PASSTHROUGH;

	/* get count */
	rc = trotListGetCount( lEnumChildren, &enumChildrenCount );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* must be 2 */
	ERR_IF( enumChildrenCount != 2, TROT_RC_ERROR_DECODE );

	/* get 1st child */
	rc = trotListGetList( lEnumChildren, 1, &lEnumChild );
	ERR_IF_PASSTHROUGH;

	/* make sure it's a TOKEN_TYPE_WORD */
	rc = trotListGetInt( lEnumChild, TOKEN_INDEX_TYPE, &enumChildTokenType );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	ERR_IF( enumChildTokenType != TOKEN_TYPE_WORD, TROT_RC_ERROR_DECODE );

	/* get it's value, which is our enum name */
	rc = trotListGetList( lEnumChild, TOKEN_INDEX_VALUE, &lEnumName );
	ERR_IF_PASSTHROUGH;

	/* get 2nd child */
	trotListFree( &lEnumChild );
	rc = trotListGetList( lEnumChildren, 2, &lEnumChild );
	ERR_IF_PASSTHROUGH;

	/* make sure it's a TOKEN_TYPE_NUMBER */
	rc = trotListGetInt( lEnumChild, TOKEN_INDEX_TYPE, &enumChildTokenType );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	ERR_IF( enumChildTokenType != TOKEN_TYPE_NUMBER, TROT_RC_ERROR_DECODE );

	/* get it's value, which is our enum value */
	rc = trotListGetInt( lEnumChild, TOKEN_INDEX_VALUE, &enumValue );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* *** */
	/* make sure this name isn't aleady in our enum list */

	/* get count */
	rc = trotListGetCount( lEnumList, &enumListCount );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* foreach enum in enumList */
	enumListIndex = 1;
	while ( enumListIndex <= enumListCount )
	{
		/* get enum */
		trotListFree( &lEnumListEnum );
		rc = trotListGetList( lEnumList, enumListIndex, &lEnumListEnum );
		ERR_IF_PASSTHROUGH;

		/* get name */
		trotListFree( &lEnumListEnumName );
		rc = trotListGetList( lEnumListEnum, 1, &lEnumListEnumName );
		ERR_IF_PASSTHROUGH;

		/* compare */
		rc = trotListCompare( lEnumListEnumName, lEnumName, &compareResult );
		ERR_IF_PASSTHROUGH;

		/* if they matched, then this name is aleady in the list */
		ERR_IF( compareResult == TROT_LIST_COMPARE_EQUAL, TROT_RC_ERROR_DECODE );

		/* increment index */
		enumListIndex += 1;
	}

	/* TODO: make sure it's also not name of list, or name of function var */
	/* TODO: maybe we can use a "find name" function to factor out this code? */

	/* *** */
	/* create new enum and append it to the list */
	rc = trotListInit( &lNewEnum );
	ERR_IF_PASSTHROUGH;

	rc = trotListAppendList( lNewEnum, lEnumName );
	ERR_IF_PASSTHROUGH;

	rc = trotListAppendInt( lNewEnum, enumValue );
	ERR_IF_PASSTHROUGH;

	rc = trotListAppendList( lEnumList, lNewEnum );
	ERR_IF_PASSTHROUGH;
	

	/* CLEANUP */
	cleanup:

	trotListFree( &lEnumChildren );
	trotListFree( &lEnumChild );
	trotListFree( &lEnumName );
	trotListFree( &lEnumListEnum );
	trotListFree( &lEnumListEnumName );
	trotListFree( &lNewEnum );

	return rc;
}

/******************************************************************************/
/*!
	\brief Gets enum value from token.
	\param lToken Token.
	\param lName Name of enum.
	\param found On function success, 1 if enum name was found and 0 if enum name was not found.
	\param value On function success and if found was 1, this will be the value of enum.
	\return TROT_RC
*/
static TROT_RC getEnumValue( TrotList *lToken, TrotList *lName, TROT_INT *found, TROT_INT *value )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotList *lEnumList = NULL;
	TROT_INT enumListCount = 0;
	TROT_INT enumListIndex = 0;
	TrotList *lEnum = NULL;
	TrotList *lEnumName = NULL;

	TROT_LIST_COMPARE_RESULT result;


	/* CODE */
	PARANOID_ERR_IF( lToken == NULL );
	PARANOID_ERR_IF( lName == NULL );
	PARANOID_ERR_IF( found == NULL );
	PARANOID_ERR_IF( value == NULL );

	/* get enum list */
	rc = trotListGetList( lToken, TOKEN_INDEX_ENUMS, &lEnumList );
	ERR_IF_PASSTHROUGH;

	/* set found to false */
	(*found) = 0;

	/* foreach enum */
	rc = trotListGetCount( lEnumList, &enumListCount );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	enumListIndex = 1;
	while ( enumListIndex <= enumListCount )
	{
		/* get enum */
		trotListFree( &lEnum );
		rc = trotListGetList( lEnumList, enumListIndex, &lEnum );
		ERR_IF_PASSTHROUGH;

		/* get name */
		trotListFree( &lEnumName );
		rc = trotListGetList( lEnum, 1, &lEnumName );
		ERR_IF_PASSTHROUGH;

		/* are these a match? */
		rc = trotListCompare( lName, lEnumName, &result );
		ERR_IF_PASSTHROUGH;

		if ( result == TROT_LIST_COMPARE_EQUAL )
		{
			/* get enum value */
			rc = trotListGetInt( lEnum, 2, value );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			/* signal found */
			(*found) = 1;
			break;
		}

		/* increment */
		enumListIndex += 1;
	}

	/* CLEANUP */
	cleanup:

	trotListFree( &lEnumList );
	trotListFree( &lEnum );
	trotListFree( &lEnumName );

	return rc;
}

/******************************************************************************/
/*!
	\brief Compares list to a c string.
	\param lValue The list.
	\param cstring c string to compare against lValue.
	\param result On success, the result of the comparison.
	\return TROT_RC
*/
static TROT_RC compareListToCString( TrotList *lValue, const char *cstring, TROT_LIST_COMPARE_RESULT *result )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotList *lCString = NULL;


	/* CODE */
	PARANOID_ERR_IF( lValue == NULL );
	PARANOID_ERR_IF( cstring == NULL );
	PARANOID_ERR_IF( result == NULL );

	rc = trotListInit( &lCString );
	ERR_IF_PASSTHROUGH;

	while ( *cstring != '\0' )
	{
		rc = trotListAppendInt( lCString, (*cstring) );
		ERR_IF_PASSTHROUGH;

		cstring += 1;
	}

	rc = trotListCompare( lValue, lCString, result );
	ERR_IF_PASSTHROUGH;


	/* CLEANUP */
	cleanup:

	trotListFree( &lCString );

	return rc;
}

/******************************************************************************/
/*!
	\brief Creates a new list of parts that were separated out of l.
	\param l List that contains parts.
	\param separator Separating character
	\param lParts On success, contains the parts.
	\return TROT_RC

	l is not modified.

	l must only contain characters, not lists.

	Example:
	IN:
		l = ["abc.def.ghi"]
		separator = '.'
	OUT:
		l = ["abc.def.ghi"]
		lParts will be [["abc]["def"]["ghi"]]
*/
static TROT_RC splitList( TrotList *l, TROT_INT separator, TrotList **lPartList )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TROT_INT count = 0;
	TROT_INT index = 0;
	TROT_INT character = 0;

	TrotList *newLrPartList = NULL;
	TrotList *lPart = NULL;


	/* CODE */
	PARANOID_ERR_IF( l == NULL );
	PARANOID_ERR_IF( lPartList == NULL );
	PARANOID_ERR_IF( (*lPartList) != NULL );

	/* create giveback */
	rc = trotListInit( &newLrPartList );
	ERR_IF_PASSTHROUGH;

	rc = trotListInit( &lPart );
	ERR_IF_PASSTHROUGH;

	rc = trotListAppendList( newLrPartList, lPart );
	ERR_IF_PASSTHROUGH;

	/* get count */
	rc = trotListGetCount( l, &count );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* foreach character */
	index = 1;
	while ( index <= count )
	{
		/* get next character */
		rc = trotListGetInt( l, index, &character );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		/* if separator */
		if ( character == separator )
		{
			trotListFree( &lPart );

			rc = trotListInit( &lPart );
			ERR_IF_PASSTHROUGH;

			rc = trotListAppendList( newLrPartList, lPart );
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
	(*lPartList) = newLrPartList;
	newLrPartList = NULL;


	/* CLEANUP */
	cleanup:

	trotListFree( &newLrPartList );
	trotListFree( &lPart );

	return rc;
}

/******************************************************************************/
/*!
	\brief Creates the final list.
	\param lTokenTree Token Tree.
	\return TROT_RC
*/
static TROT_RC createFinalList( TrotList *lTokenTree )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;

	TrotList *lParentTokenStack = NULL;
	TROT_INT parentTokenStackCount = 0;

	TrotList *lParentTokenIndicesStack = NULL;

	TROT_INT parentTokenType = 0;

	TrotList *lToken = NULL;
	TrotList *lTokenChildren = NULL;
	TrotList *lTokenFinalList = NULL;
	TROT_INT tokenChildrenCount = 0;
	TROT_INT tokenChildrenIndex = 0;

	TrotList *lChildToken = NULL;
	TrotList *lChildTokenValue = NULL;
	TROT_INT childTokenValueCount = 0;
	TROT_INT childTokenValueIndex = 0;
	TrotList *lChildTokenFinalList = NULL;
	TROT_INT childTokenType = 0;
	TROT_INT childTokenValueInt = 0;


	/* CODE */
	PARANOID_ERR_IF( lTokenTree == NULL );

	/* create parent stack, so we can "go up" */
	rc = trotListInit( &lParentTokenStack );
	ERR_IF_PASSTHROUGH;

	rc = trotListInit( &lParentTokenIndicesStack );
	ERR_IF_PASSTHROUGH;

	/* *** */
	/* set token to lTokenTree */
	rc = trotListTwin( lTokenTree, &lToken );
	ERR_IF_PASSTHROUGH;

	/* get top token's children */
	rc = trotListGetList( lToken, TOKEN_INDEX_VALUE, &lTokenChildren );
	ERR_IF_PASSTHROUGH;

	/* get finalList */
	rc = trotListGetList( lToken, TOKEN_INDEX_FINALLIST, &lTokenFinalList );
	ERR_IF_PASSTHROUGH;

	/* get parent token type */
	rc = trotListGetInt( lToken, TOKEN_INDEX_TYPE, &parentTokenType );
	PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

	/* special case: INCLUDE, already good to go */
	if ( parentTokenType == TOKEN_TYPE_INCLUDE )
	{
		goto cleanup;
	}

	/* set index */
	tokenChildrenIndex = 0;

	/* go through tree */
	while ( 1 )
	{
		/* increment index */
		tokenChildrenIndex += 1;

		/* are we at end of children? */
		rc = trotListGetCount( lTokenChildren, &tokenChildrenCount );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		if ( tokenChildrenIndex > tokenChildrenCount )
		{
			/* free token, children, finalList */
			trotListFree( &lToken );
			trotListFree( &lTokenChildren );
			trotListFree( &lTokenFinalList );

			/* do we have any parents? */
			rc = trotListGetCount( lParentTokenStack, &parentTokenStackCount );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			/* no parents, so we're done */
			if ( parentTokenStackCount == 0 )
			{
				break;
			}

			/* yes parents, so go up */

			/* get token */
			rc = trotListRemoveList( lParentTokenStack, -1, &lToken );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			/* get top token's children */
			rc = trotListGetList( lToken, TOKEN_INDEX_VALUE, &lTokenChildren );
			ERR_IF_PASSTHROUGH;

			/* get finalList */
			rc = trotListGetList( lToken, TOKEN_INDEX_FINALLIST, &lTokenFinalList );
			ERR_IF_PASSTHROUGH;

			/* get parent token type */
			rc = trotListGetInt( lToken, TOKEN_INDEX_TYPE, &parentTokenType );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			/* get index */
			rc = trotListRemoveInt( lParentTokenIndicesStack, -1, &tokenChildrenIndex );
			PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

			continue;
		}

		/* get next child token */
		trotListFree( &lChildToken );

		rc = trotListGetList( lTokenChildren, tokenChildrenIndex, &lChildToken );
		ERR_IF_PASSTHROUGH;

		/* get tokenType */
		rc = trotListGetInt( lChildToken, TOKEN_INDEX_TYPE, &childTokenType );
		PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

		/* handle token */
		switch ( childTokenType )
		{
			case TOKEN_TYPE_L_BRACKET:
			case TOKEN_TYPE_L_BRACE:
				/* get child final list */
				trotListFree( &lChildTokenFinalList );

				rc = trotListGetList( lChildToken, TOKEN_INDEX_FINALLIST, &lChildTokenFinalList );
				ERR_IF_PASSTHROUGH;

				/* if in brace, add push first */
				if (    lTokenFinalList -> laPointsTo -> tag == TROT_TAG_CODE
				     || lTokenFinalList -> laPointsTo -> tag == TROT_TAG_FUNCTION
				   )
				{
					rc = trotListAppendInt( lTokenFinalList, TROT_OP_PUSH_LIST );
					ERR_IF_PASSTHROUGH;
				}

				/* append */
				rc = trotListAppendList( lTokenFinalList, lChildTokenFinalList );
				ERR_IF_PASSTHROUGH;

				/* save our state */
				rc = trotListAppendList( lParentTokenStack, lToken );
				ERR_IF_PASSTHROUGH;

				rc = trotListAppendInt( lParentTokenIndicesStack, tokenChildrenIndex );
				ERR_IF_PASSTHROUGH;

				/* go down */
				
				/* free token, children, final list */
				trotListFree( &lToken );
				trotListFree( &lTokenChildren );
				trotListFree( &lTokenFinalList );

				/* set token equal to this child */
				lToken = lChildToken;
				lChildToken = NULL;

				/* get token's children */
				rc = trotListGetList( lToken, TOKEN_INDEX_VALUE, &lTokenChildren );
				ERR_IF_PASSTHROUGH;

				/* get token's finalList */
				rc = trotListGetList( lToken, TOKEN_INDEX_FINALLIST, &lTokenFinalList );
				ERR_IF_PASSTHROUGH;

				/* set parent token type */
				parentTokenType = childTokenType;

				/* set index */
				tokenChildrenIndex = 0;

				continue;

			case TOKEN_TYPE_NUMBER:
				/* if in brace, add push first */
				if (    lTokenFinalList -> laPointsTo -> tag == TROT_TAG_CODE
				     || lTokenFinalList -> laPointsTo -> tag == TROT_TAG_FUNCTION
				   )
				{
					rc = trotListAppendInt( lTokenFinalList, TROT_OP_PUSH_INT );
					ERR_IF_PASSTHROUGH;
				}

				/* fall through */

			case TOKEN_TYPE_NUMBER_RAW:
				/* get number */
				rc = trotListGetInt( lChildToken, TOKEN_INDEX_VALUE, &childTokenValueInt );
				PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

				/* append number */
				rc = trotListAppendInt( lTokenFinalList, childTokenValueInt );
				ERR_IF_PASSTHROUGH;

				break;

			case TOKEN_TYPE_STRING:
				/* get string */
				trotListFree( &lChildTokenValue );

				rc = trotListGetList( lChildToken, TOKEN_INDEX_VALUE, &lChildTokenValue );
				ERR_IF_PASSTHROUGH;

				rc = trotListGetCount( lChildTokenValue, &childTokenValueCount );
				PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

				/* append each number */
				childTokenValueIndex = 1;
				while ( childTokenValueIndex <= childTokenValueCount )
				{
					rc = trotListGetInt( lChildTokenValue, childTokenValueIndex, &childTokenValueInt );
					PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

					rc = trotListAppendInt( lTokenFinalList, childTokenValueInt );
					ERR_IF_PASSTHROUGH;

					childTokenValueIndex += 1;
				}

				break;

			case TOKEN_TYPE_TWIN:
				/* get child value */
				trotListFree( &lChildTokenValue );

				rc = trotListGetList( lChildToken, TOKEN_INDEX_VALUE, &lChildTokenValue );
				ERR_IF_PASSTHROUGH;

				/* if in brace, add push first */
				if (    lTokenFinalList -> laPointsTo -> tag == TROT_TAG_CODE
				     || lTokenFinalList -> laPointsTo -> tag == TROT_TAG_FUNCTION
				   )
				{
					rc = trotListAppendInt( lTokenFinalList, TROT_OP_PUSH_LIST );
					ERR_IF_PASSTHROUGH;
				}

				/* append */
				rc = trotListAppendList( lTokenFinalList, lChildTokenValue );
				ERR_IF_PASSTHROUGH;

				break;

			case TOKEN_TYPE_INCLUDE:
				/* NOTE: include tokens VALUE is a file, and it's FINALLIST
				         is the FINALLIST of the file's token  */
				/* get token tree's final list */
				trotListFree( &lChildTokenFinalList );
				rc = trotListGetList( lChildToken, TOKEN_INDEX_FINALLIST, &lChildTokenFinalList );
				ERR_IF_PASSTHROUGH;

				/* if in brace, add push first */
				if (    lTokenFinalList -> laPointsTo -> tag == TROT_TAG_CODE
				     || lTokenFinalList -> laPointsTo -> tag == TROT_TAG_FUNCTION
				   )
				{
					rc = trotListAppendInt( lTokenFinalList, TROT_OP_PUSH_LIST );
					ERR_IF_PASSTHROUGH;
				}

				/* append */
				rc = trotListAppendList( lTokenFinalList, lChildTokenFinalList );
				ERR_IF_PASSTHROUGH;

				break;

			default:
				PARANOID_ERR_IF( childTokenType != TOKEN_TYPE_OP );

				/* get op value */
				rc = trotListGetInt( lChildToken, TOKEN_INDEX_VALUE, &childTokenValueInt );
				PARANOID_ERR_IF( rc != TROT_RC_SUCCESS );

				/* append op value */
				rc = trotListAppendInt( lTokenFinalList, childTokenValueInt );
				ERR_IF_PASSTHROUGH;

				break;
		}
	}


	/* CLEANUP */
	cleanup:

	trotListFree( &lParentTokenStack );
	trotListFree( &lParentTokenIndicesStack );

	trotListFree( &lToken );
	trotListFree( &lTokenChildren );
	trotListFree( &lTokenFinalList );

	trotListFree( &lChildToken );
	trotListFree( &lChildTokenValue );
	trotListFree( &lChildTokenFinalList );

	return rc;
}
