/*
Copyright (c) 2010,2011, Jeremiah Martell
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
#include "trotCommon.h"
#include "trotList.h"
#include "trotListInternal.h"

#include "testCommon.h"

#include "stdio.h"
#include "time.h"
#include "string.h"


/******************************************************************************/
int testBadTypesAndIndices()
{
	/* DATA */
	int rc = 0;

	trotListRef *lr = NULL;
	trotListRef *lr2 = NULL;
	int kind = 0;
	INT_TYPE n = 0;


	/* CODE */
	/* **************************************** */
	/* test bad indices, bad types */
	printf( "Testing bad indices and bad types...\n" ); fflush( stdout );
	TEST_ERR_IF( createHalfIntHalfList( &lr, 10 ) != TROT_LIST_SUCCESS );

	TEST_ERR_IF( trotListRefGetKind( lr, 0, &kind ) != TROT_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRefGetKind( lr, 11, &kind ) != TROT_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRefGetKind( lr, -11, &kind ) != TROT_LIST_ERROR_BAD_INDEX );

	TEST_ERR_IF( trotListRefInsertInt( lr, 0, 1 ) != TROT_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRefInsertInt( lr, 12, 1 ) != TROT_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRefInsertInt( lr, -12, 1 ) != TROT_LIST_ERROR_BAD_INDEX );
	
	TEST_ERR_IF( trotListRefInsertListTwin( lr, 0, lr ) != TROT_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRefInsertListTwin( lr, 12, lr ) != TROT_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRefInsertListTwin( lr, -12, lr ) != TROT_LIST_ERROR_BAD_INDEX );

	TEST_ERR_IF( trotListRefGetInt( lr, 0, &n	) != TROT_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRefGetInt( lr, 11, &n ) != TROT_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRefGetInt( lr, -11, &n ) != TROT_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRefGetInt( lr, 6, &n ) != TROT_LIST_ERROR_WRONG_KIND );

	TEST_ERR_IF( trotListRefGetListTwin( lr, 0, &lr2 ) != TROT_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRefGetListTwin( lr, 11, &lr2 ) != TROT_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRefGetListTwin( lr, -11, &lr2 ) != TROT_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRefGetListTwin( lr, 5, &lr2 ) != TROT_LIST_ERROR_WRONG_KIND );

	TEST_ERR_IF( trotListRefRemoveInt( lr, 0, &n ) != TROT_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRefRemoveInt( lr, 11, &n ) != TROT_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRefRemoveInt( lr, -11, &n ) != TROT_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRefRemoveInt( lr, 6, &n ) != TROT_LIST_ERROR_WRONG_KIND );

	TEST_ERR_IF( trotListRefRemoveList( lr, 0, &lr2 ) != TROT_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRefRemoveList( lr, 11, &lr2 ) != TROT_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRefRemoveList( lr, -11, &lr2 ) != TROT_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRefRemoveList( lr, 5, &lr2 ) != TROT_LIST_ERROR_WRONG_KIND );

	TEST_ERR_IF( trotListRefRemove( lr, 0 ) != TROT_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRefRemove( lr, 11 ) != TROT_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRefRemove( lr, -11 ) != TROT_LIST_ERROR_BAD_INDEX );

	TEST_ERR_IF( trotListRefEnlist( lr, 0, 1 ) != TROT_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRefEnlist( lr, -11, 1 ) != TROT_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRefEnlist( lr, 11, 1 ) != TROT_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRefEnlist( lr, 1, 0 ) != TROT_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRefEnlist( lr, 1, -11 ) != TROT_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRefEnlist( lr, 1, 11 ) != TROT_LIST_ERROR_BAD_INDEX );

	TEST_ERR_IF( trotListRefDelist( lr, 0 ) != TROT_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRefDelist( lr, -11 ) != TROT_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRefDelist( lr, 11 ) != TROT_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRefDelist( lr, 1 ) != TROT_LIST_ERROR_WRONG_KIND );

	TEST_ERR_IF( trotListRefCopySpan( &lr2, lr, 0, 1 ) != TROT_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRefCopySpan( &lr2, lr, -11, 1 ) != TROT_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRefCopySpan( &lr2, lr, 11, 1 ) != TROT_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRefCopySpan( &lr2, lr, 1, 0 ) != TROT_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRefCopySpan( &lr2, lr, 1, -11 ) != TROT_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRefCopySpan( &lr2, lr, 1, 11 ) != TROT_LIST_ERROR_BAD_INDEX );

	TEST_ERR_IF( trotListRefRemoveSpan( lr, 0, 1 ) != TROT_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRefRemoveSpan( lr, -11, 1 ) != TROT_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRefRemoveSpan( lr, 11, 1 ) != TROT_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRefRemoveSpan( lr, 1, 0 ) != TROT_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRefRemoveSpan( lr, 1, -11 ) != TROT_LIST_ERROR_BAD_INDEX );
	TEST_ERR_IF( trotListRefRemoveSpan( lr, 1, 11 ) != TROT_LIST_ERROR_BAD_INDEX );

	trotListRefFree( &lr );

	return 0;


	/* CLEANUP */
	cleanup:

	return rc;
}

