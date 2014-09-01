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

/******************************************************************************/
int testPreconditions( TrotList *lMemLimit )
{
	/* DATA */
	int rc = 0;

	TrotList *l1 = NULL;
	TrotList *l2 = NULL;
	TROT_INT kind = 0;
	TROT_INT n = 0;
	TROT_INT type = TROT_TYPE_DATA;
	TROT_INT tag = TROT_TYPE_DATA;
	TROT_LIST_COMPARE_RESULT compareResult = TROT_LIST_COMPARE_EQUAL;


	/* CODE */
	/* **************************************** */
	/* test preconditions */
	printf( "Testing preconditions...\n" ); fflush( stdout );

	TEST_ERR_IF( trotListInit( lMemLimit, NULL ) == TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListInit( lMemLimit, &l1 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListInit( lMemLimit, &l1 ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListTwin( lMemLimit, NULL, &l2 ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListTwin( lMemLimit, l1, NULL ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListTwin( lMemLimit, l1, &l1 ) != TROT_RC_ERROR_PRECOND );

	trotListFree( lMemLimit, NULL );
	trotListFree( lMemLimit, &l2 );

	TEST_ERR_IF( trotListRefCompare( lMemLimit, NULL, l1, &n ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListRefCompare( lMemLimit, l1, NULL, &n ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListRefCompare( lMemLimit, l1, l1, NULL ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListGetCount( lMemLimit, NULL, &n ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListGetCount( lMemLimit, l1, NULL ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListGetKind( lMemLimit, NULL, 1, &kind ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListGetKind( lMemLimit, l1, 1, NULL ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListAppendInt( lMemLimit, NULL, 0 ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListAppendList( lMemLimit, NULL, l1 ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListAppendList( lMemLimit, l1, NULL ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListInsertInt( lMemLimit, NULL, 1, 1 ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListInsertList( lMemLimit, NULL, 1, l1 ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListInsertList( lMemLimit, l1, 1, NULL ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListGetInt( lMemLimit, NULL, 1, &n ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListGetInt( lMemLimit, l1, 1, NULL ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListGetList( lMemLimit, NULL, 1, &l2 ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListGetList( lMemLimit, l1, 1, NULL ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListGetList( lMemLimit, l1, 1, &l1 ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListRemoveInt( lMemLimit, NULL, 1, &n ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListRemoveInt( lMemLimit, l1, 1, NULL ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListRemoveList( lMemLimit, NULL, 1, &l2 ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListRemoveList( lMemLimit, l1, 1, NULL ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListRemoveList( lMemLimit, l1, 1, &l1 ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListRemove( lMemLimit, NULL, 1 ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListReplaceWithInt( lMemLimit, NULL, 1, 1 ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListReplaceWithList( lMemLimit, NULL, 1, l1 ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListReplaceWithList( lMemLimit, l1, 1, NULL ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListGetType( lMemLimit, NULL, &type ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListGetType( lMemLimit, l1, NULL ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListSetType( lMemLimit, NULL, TROT_TYPE_DATA ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListGetTag( lMemLimit, NULL, &tag ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListGetTag( lMemLimit, l1, NULL ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListSetTag( lMemLimit, NULL, TROT_TYPE_DATA ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListCompare( lMemLimit, NULL, l1, &compareResult ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListCompare( lMemLimit, l1, NULL, &compareResult ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListCompare( lMemLimit, l1, l1, NULL ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListCopy( lMemLimit, NULL, &l2 ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListCopy( lMemLimit, l1, NULL ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListCopy( lMemLimit, l1, &l1 ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListEnlist( lMemLimit, NULL, 1, 1 ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListDelist( lMemLimit, NULL, 1 ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListCopySpan( lMemLimit, l1, 1, 1, NULL ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListCopySpan( lMemLimit, l1, 1, 1, &l1 ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotListCopySpan( lMemLimit, NULL, 1, 1, &l2 ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotListRemoveSpan( lMemLimit, NULL, 1, 1 ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotUtf8ToCharacters( lMemLimit, NULL, l1 ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotUtf8ToCharacters( lMemLimit, l1, NULL ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotCharactersToUtf8( lMemLimit, NULL, l1 ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotCharactersToUtf8( lMemLimit, l1, NULL ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotEncode( lMemLimit, NULL, &l2 ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotEncode( lMemLimit, l1, NULL ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotEncode( lMemLimit, l1, &l1 ) != TROT_RC_ERROR_PRECOND );

	TEST_ERR_IF( trotDecode( lMemLimit, NULL, &l2 ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotDecode( lMemLimit, l1, NULL ) != TROT_RC_ERROR_PRECOND );
	TEST_ERR_IF( trotDecode( lMemLimit, l1, &l1 ) != TROT_RC_ERROR_PRECOND );

	trotListFree( lMemLimit, &l1 );

	return 0;


	/* CLEANUP */
	cleanup:

	return rc;
}

