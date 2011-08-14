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

	trotListRef *lr = NULL;
	trotListRef *lr2 = NULL;
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

	TEST_ERR_IF( trotListRefInit( NULL ) == TROT_LIST_SUCCESS );
	TEST_ERR_IF( trotListRefInit( &lr ) != TROT_LIST_SUCCESS );
	TEST_ERR_IF( trotListRefInit( &lr ) != TROT_LIST_ERROR_PRECOND );

	TEST_ERR_IF( trotListRefTwin( NULL, lr ) != TROT_LIST_ERROR_PRECOND );
	TEST_ERR_IF( trotListRefTwin( &lr, lr ) != TROT_LIST_ERROR_PRECOND );
	TEST_ERR_IF( trotListRefTwin( &lr2, NULL ) != TROT_LIST_ERROR_PRECOND );

	TEST_ERR_IF( trotListRefFree( NULL ) != TROT_LIST_ERROR_PRECOND );

	TEST_ERR_IF( trotListRefGetCount( NULL, &n ) != TROT_LIST_ERROR_PRECOND );
	TEST_ERR_IF( trotListRefGetCount( lr, NULL ) != TROT_LIST_ERROR_PRECOND );

	TEST_ERR_IF( trotListRefGetKind( NULL, 1, &kind ) != TROT_LIST_ERROR_PRECOND );
	TEST_ERR_IF( trotListRefGetKind( lr, 1, NULL ) != TROT_LIST_ERROR_PRECOND );

	TEST_ERR_IF( trotListRefAppendInt( NULL, 0 ) != TROT_LIST_ERROR_PRECOND );

	TEST_ERR_IF( trotListRefAppendListTwin( NULL, lr ) != TROT_LIST_ERROR_PRECOND );
	TEST_ERR_IF( trotListRefAppendListTwin( lr, NULL ) != TROT_LIST_ERROR_PRECOND );

	TEST_ERR_IF( trotListRefInsertInt( NULL, 1, 1 ) != TROT_LIST_ERROR_PRECOND );

	TEST_ERR_IF( trotListRefInsertListTwin( NULL, 1, lr ) != TROT_LIST_ERROR_PRECOND );
	TEST_ERR_IF( trotListRefInsertListTwin( lr, 1, NULL ) != TROT_LIST_ERROR_PRECOND );

	TEST_ERR_IF( trotListRefGetInt( NULL, 1, &n ) != TROT_LIST_ERROR_PRECOND );
	TEST_ERR_IF( trotListRefGetInt( lr, 1, NULL ) != TROT_LIST_ERROR_PRECOND );

	TEST_ERR_IF( trotListRefGetListTwin( NULL, 1, &lr2 ) != TROT_LIST_ERROR_PRECOND );
	TEST_ERR_IF( trotListRefGetListTwin( lr, 1, NULL ) != TROT_LIST_ERROR_PRECOND );
	TEST_ERR_IF( trotListRefGetListTwin( lr, 1, &lr ) != TROT_LIST_ERROR_PRECOND );

	TEST_ERR_IF( trotListRefRemoveInt( NULL, 1, &n ) != TROT_LIST_ERROR_PRECOND );
	TEST_ERR_IF( trotListRefRemoveInt( lr, 1, NULL ) != TROT_LIST_ERROR_PRECOND );

	TEST_ERR_IF( trotListRefRemoveList( NULL, 1, &lr2 ) != TROT_LIST_ERROR_PRECOND );
	TEST_ERR_IF( trotListRefRemoveList( lr, 1, NULL ) != TROT_LIST_ERROR_PRECOND );
	TEST_ERR_IF( trotListRefRemoveList( lr, 1, &lr ) != TROT_LIST_ERROR_PRECOND );

	TEST_ERR_IF( trotListRefRemove( NULL, 1 ) != TROT_LIST_ERROR_PRECOND );

	trotListRefFree( &lr );

	return 0;


	/* CLEANUP */
	cleanup:

	return rc;
}

