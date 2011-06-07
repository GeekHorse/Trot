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

#include "testCommon.h"

#include "stdio.h"
#include "time.h"
#include "string.h"


/******************************************************************************/
int testPreconditions()
{
	/* DATA */
	int rc = 0;

	gkListRef *lr = NULL;
	gkListRef *lr2 = NULL;
	int kind = 0;
	INT_TYPE n = 0;


	/* CODE */
	/* **************************************** */
	/* lets test error cases to get 100% test coverage */
	/* test preconditions */
	printf( "Testing preconditions...\n" ); fflush( stdout );

	#if ( ! TEST_PRECOND )
	printf( "  TEST_PRECOND not defined! Not doing any tests...\n" ); fflush( stdout );
	return 0;
	#endif

	TEST_ERR_IF( gkListRefInit( NULL ) == GK_LIST_SUCCESS );
	TEST_ERR_IF( gkListRefInit( &lr ) != GK_LIST_SUCCESS );
	TEST_ERR_IF( gkListRefInit( &lr ) != GK_LIST_ERROR_PRECOND );

	TEST_ERR_IF( gkListRefTwin( NULL, lr ) != GK_LIST_ERROR_PRECOND );
	TEST_ERR_IF( gkListRefTwin( &lr, lr ) != GK_LIST_ERROR_PRECOND );
	TEST_ERR_IF( gkListRefTwin( &lr2, NULL ) != GK_LIST_ERROR_PRECOND );

	TEST_ERR_IF( gkListRefFree( NULL ) != GK_LIST_ERROR_PRECOND );

	TEST_ERR_IF( gkListRefGetCount( NULL, &n ) != GK_LIST_ERROR_PRECOND );
	TEST_ERR_IF( gkListRefGetCount( lr, NULL ) != GK_LIST_ERROR_PRECOND );

	TEST_ERR_IF( gkListRefGetKind( NULL, 1, &kind ) != GK_LIST_ERROR_PRECOND );
	TEST_ERR_IF( gkListRefGetKind( lr, 1, NULL ) != GK_LIST_ERROR_PRECOND );

	TEST_ERR_IF( gkListRefAppendInt( NULL, 0 ) != GK_LIST_ERROR_PRECOND );

	TEST_ERR_IF( gkListRefAppendListTwin( NULL, lr ) != GK_LIST_ERROR_PRECOND );
	TEST_ERR_IF( gkListRefAppendListTwin( lr, NULL ) != GK_LIST_ERROR_PRECOND );

	TEST_ERR_IF( gkListRefInsertInt( NULL, 1, 1 ) != GK_LIST_ERROR_PRECOND );

	TEST_ERR_IF( gkListRefInsertListTwin( NULL, 1, lr ) != GK_LIST_ERROR_PRECOND );
	TEST_ERR_IF( gkListRefInsertListTwin( lr, 1, NULL ) != GK_LIST_ERROR_PRECOND );

	TEST_ERR_IF( gkListRefGetInt( NULL, 1, &n ) != GK_LIST_ERROR_PRECOND );
	TEST_ERR_IF( gkListRefGetInt( lr, 1, NULL ) != GK_LIST_ERROR_PRECOND );

	TEST_ERR_IF( gkListRefGetListTwin( NULL, 1, &lr2 ) != GK_LIST_ERROR_PRECOND );
	TEST_ERR_IF( gkListRefGetListTwin( lr, 1, NULL ) != GK_LIST_ERROR_PRECOND );
	TEST_ERR_IF( gkListRefGetListTwin( lr, 1, &lr ) != GK_LIST_ERROR_PRECOND );

	TEST_ERR_IF( gkListRefRemoveInt( NULL, 1, &n ) != GK_LIST_ERROR_PRECOND );
	TEST_ERR_IF( gkListRefRemoveInt( lr, 1, NULL ) != GK_LIST_ERROR_PRECOND );

	TEST_ERR_IF( gkListRefRemoveList( NULL, 1, &lr2 ) != GK_LIST_ERROR_PRECOND );
	TEST_ERR_IF( gkListRefRemoveList( lr, 1, NULL ) != GK_LIST_ERROR_PRECOND );
	TEST_ERR_IF( gkListRefRemoveList( lr, 1, &lr ) != GK_LIST_ERROR_PRECOND );

	TEST_ERR_IF( gkListRefRemove( NULL, 1 ) != GK_LIST_ERROR_PRECOND );

	gkListRefFree( &lr );

	return 0;


	/* CLEANUP */
	cleanup:

	return rc;
}

