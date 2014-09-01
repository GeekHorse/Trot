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
int testBadTypesAndIndices( TrotList *lMemLimit )
{
	/* DATA */
	int rc = 0;

	TrotList *l = NULL;
	TrotList *l2 = NULL;
	TROT_INT kind = 0;
	TROT_INT n = 0;


	/* CODE */
	/* **************************************** */
	/* test bad indices, bad types */
	printf( "Testing bad indices and bad types...\n" ); fflush( stdout );

	TEST_ERR_IF( createHalfIntHalfList( lMemLimit, &l, 10 ) != TROT_RC_SUCCESS );

	TEST_ERR_IF( trotListGetKind( lMemLimit, l, 0, &kind ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListGetKind( lMemLimit, l, 11, &kind ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListGetKind( lMemLimit, l, -11, &kind ) != TROT_RC_ERROR_BAD_INDEX );

	TEST_ERR_IF( trotListInsertInt( lMemLimit, l, 0, 1 ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListInsertInt( lMemLimit, l, 12, 1 ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListInsertInt( lMemLimit, l, -12, 1 ) != TROT_RC_ERROR_BAD_INDEX );
	
	TEST_ERR_IF( trotListInsertList( lMemLimit, l, 0, l ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListInsertList( lMemLimit, l, 12, l ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListInsertList( lMemLimit, l, -12, l ) != TROT_RC_ERROR_BAD_INDEX );

	TEST_ERR_IF( trotListGetInt( lMemLimit, l, 0, &n ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListGetInt( lMemLimit, l, 11, &n ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListGetInt( lMemLimit, l, -11, &n ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListGetInt( lMemLimit, l, 6, &n ) != TROT_RC_ERROR_WRONG_KIND );

	TEST_ERR_IF( trotListGetList( lMemLimit, l, 0, &l2 ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListGetList( lMemLimit, l, 11, &l2 ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListGetList( lMemLimit, l, -11, &l2 ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListGetList( lMemLimit, l, 5, &l2 ) != TROT_RC_ERROR_WRONG_KIND );

	TEST_ERR_IF( trotListRemoveInt( lMemLimit, l, 0, &n ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRemoveInt( lMemLimit, l, 11, &n ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRemoveInt( lMemLimit, l, -11, &n ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRemoveInt( lMemLimit, l, 6, &n ) != TROT_RC_ERROR_WRONG_KIND );

	TEST_ERR_IF( trotListRemoveList( lMemLimit, l, 0, &l2 ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRemoveList( lMemLimit, l, 11, &l2 ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRemoveList( lMemLimit, l, -11, &l2 ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRemoveList( lMemLimit, l, 5, &l2 ) != TROT_RC_ERROR_WRONG_KIND );

	TEST_ERR_IF( trotListRemove( lMemLimit, l, 0 ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRemove( lMemLimit, l, 11 ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRemove( lMemLimit, l, -11 ) != TROT_RC_ERROR_BAD_INDEX );

	TEST_ERR_IF( trotListReplaceWithInt( lMemLimit, l, 0, 1 ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListReplaceWithInt( lMemLimit, l, 11, 1 ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListReplaceWithInt( lMemLimit, l, -11, 1 ) != TROT_RC_ERROR_BAD_INDEX );

	TEST_ERR_IF( trotListReplaceWithList( lMemLimit, l, 0, l ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListReplaceWithList( lMemLimit, l, 11, l ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListReplaceWithList( lMemLimit, l, -11, l ) != TROT_RC_ERROR_BAD_INDEX );

	TEST_ERR_IF( trotListEnlist( lMemLimit, l, 0, 1 ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListEnlist( lMemLimit, l, -11, 1 ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListEnlist( lMemLimit, l, 11, 1 ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListEnlist( lMemLimit, l, 1, 0 ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListEnlist( lMemLimit, l, 1, -11 ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListEnlist( lMemLimit, l, 1, 11 ) != TROT_RC_ERROR_BAD_INDEX );

	TEST_ERR_IF( trotListDelist( lMemLimit, l, 0 ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListDelist( lMemLimit, l, -11 ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListDelist( lMemLimit, l, 11 ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListDelist( lMemLimit, l, 1 ) != TROT_RC_ERROR_WRONG_KIND );

	TEST_ERR_IF( trotListCopySpan( lMemLimit, l, 0, 1, &l2 ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListCopySpan( lMemLimit, l, -11, 1, &l2 ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListCopySpan( lMemLimit, l, 11, 1, &l2 ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListCopySpan( lMemLimit, l, 1, 0, &l2 ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListCopySpan( lMemLimit, l, 1, -11, &l2 ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListCopySpan( lMemLimit, l, 1, 11, &l2 ) != TROT_RC_ERROR_BAD_INDEX );

	TEST_ERR_IF( trotListRemoveSpan( lMemLimit, l, 0, 1 ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRemoveSpan( lMemLimit, l, -11, 1 ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRemoveSpan( lMemLimit, l, 11, 1 ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRemoveSpan( lMemLimit, l, 1, 0 ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRemoveSpan( lMemLimit, l, 1, -11 ) != TROT_RC_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRemoveSpan( lMemLimit, l, 1, 11 ) != TROT_RC_ERROR_BAD_INDEX );

	trotListFree( lMemLimit, &l );


	/* CLEANUP */
	cleanup:

	return rc;
}

