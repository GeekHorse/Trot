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
#include "trot.h"
#include "trotInternal.h"

#include "trotTestCommon.h"

static int testLoadFunc( TrotList *lName, TrotList **lBytes )
{
	(void)lName;
	(void)lBytes;
	return 0;
}

/******************************************************************************/
int testPreconditions()
{
	/* DATA */
	int rc = 0;

	TrotList *l1 = NULL;
	TrotList *l2 = NULL;
	int i = 0;
	TROT_KIND kind = 0;
	TROT_INT n = 0;
	TROT_TAG tag = TROT_TAG_DATA;
	TROT_LIST_COMPARE_RESULT compareResult = TROT_LIST_COMPARE_EQUAL;


	/* CODE */
	/* **************************************** */
	/* test preconditions */
	printf( "Testing preconditions...\n" ); fflush( stdout );

	TEST_ERR_IF( trotListInit( NULL ) == TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListInit( &l1 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListInit( &l1 ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListTwin( NULL, &l2 ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListTwin( l1, NULL ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListTwin( l1, &l1 ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListGetCount( NULL, &n ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListGetCount( l1, NULL ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListGetKind( NULL, 1, &kind ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListGetKind( l1, 1, NULL ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListAppendInt( NULL, 0 ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListAppendList( NULL, l1 ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListAppendList( l1, NULL ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListInsertInt( NULL, 1, 1 ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListInsertList( NULL, 1, l1 ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListInsertList( l1, 1, NULL ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListGetInt( NULL, 1, &n ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListGetInt( l1, 1, NULL ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListGetList( NULL, 1, &l2 ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListGetList( l1, 1, NULL ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListGetList( l1, 1, &l1 ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListRemoveInt( NULL, 1, &n ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListRemoveInt( l1, 1, NULL ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListRemoveList( NULL, 1, &l2 ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListRemoveList( l1, 1, NULL ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListRemoveList( l1, 1, &l1 ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListRemove( NULL, 1 ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListReplaceWithInt( NULL, 1, 1 ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListReplaceWithList( NULL, 1, l1 ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListReplaceWithList( l1, 1, NULL ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListGetTag( NULL, &tag ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListGetTag( l1, NULL ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListSetTag( NULL, TROT_TAG_DATA ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListCompare( NULL, l1, &compareResult ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListCompare( l1, NULL, &compareResult ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListCompare( l1, l1, NULL ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListCopy( NULL, &l2 ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListCopy( l1, NULL ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListCopy( l1, &l1 ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListEnlist( NULL, 1, 1 ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListDelist( NULL, 1 ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListCopySpan( l1, 1, 1, NULL ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListCopySpan( l1, 1, 1, &l1 ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListCopySpan( NULL, 1, 1, &l2 ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListRemoveSpan( NULL, 1, 1 ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListIntOperand( NULL, TROT_OP_ADD ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListIntOperandValue( NULL, TROT_OP_ADD, 0 ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotUtf8ToCharacters( NULL, l1 ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotUtf8ToCharacters( l1, NULL ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotCharactersToUtf8( NULL, l1 ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotCharactersToUtf8( l1, NULL ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotTokenize( NULL, &l2 ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotTokenize( l1, NULL ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotTokenize( l1, &l1 ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotCreateToken( 1, 1, 1, NULL ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotCreateToken( 1, 1, 1, &l1 ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( _trotWordToNumber( NULL, &i, &n ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( _trotWordToNumber( l1, NULL, &n ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( _trotWordToNumber( l1, &i, NULL ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotEncode( NULL, &l2 ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotEncode( l1, NULL ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotEncode( l1, &l1 ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotDecodeCharacters( NULL, l1, l1, &l2 ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotDecodeCharacters( testLoadFunc, NULL, l1, &l2 ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotDecodeCharacters( testLoadFunc, l1, NULL, &l2 ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotDecodeCharacters( testLoadFunc, l1, l1, NULL ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotDecodeCharacters( testLoadFunc, l1, l1, &l1 ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotDecodeFilename( NULL, l1, &l2 ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotDecodeFilename( testLoadFunc, NULL, &l2 ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotDecodeFilename( testLoadFunc, l1, NULL ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotDecodeFilename( testLoadFunc, l1, &l1 ) != TROT_RC_ERROR_PRECOND );

	trotListFree( &l1 );

	return 0;


	/* CLEANUP */
	cleanup:

	return rc;
}

