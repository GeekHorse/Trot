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
int testBadIntegerOps()
{
	/* DATA */
	int rc = 0;

	TrotList *l = NULL;


	/* CODE */
	/* **************************************** */
	/* test wrong kinds */
	printf( "Testing bad integer ops...\n" ); fflush( stdout );

	TEST_ERR_IF( trotListInit( &l ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListIntOperand( l, TROT_OP_ADD ) != TROT_RC_ERROR_WRONG_KIND );
	trotListFree( &l );

	TEST_ERR_IF( trotListInit( &l ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( addListWithValue( l, -1, 1 ) != 0 );
	TEST_ERR_IF( trotListIntOperand( l, TROT_OP_ADD ) != TROT_RC_ERROR_WRONG_KIND );
	trotListFree( &l );

	TEST_ERR_IF( trotListInit( &l ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( addListWithValue( l, -1, 1 ) != 0 );
	TEST_ERR_IF( addListWithValue( l, -1, 1 ) != 0 );
	TEST_ERR_IF( trotListIntOperand( l, TROT_OP_ADD ) != TROT_RC_ERROR_WRONG_KIND );
	trotListFree( &l );

	TEST_ERR_IF( trotListInit( &l ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListAppendInt( l, 1 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListIntOperand( l, TROT_OP_ADD ) != TROT_RC_ERROR_WRONG_KIND );
	trotListFree( &l );

	TEST_ERR_IF( trotListInit( &l ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( addListWithValue( l, -1, 1 ) != 0 );
	TEST_ERR_IF( trotListAppendInt( l, 1 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListIntOperand( l, TROT_OP_ADD ) != TROT_RC_ERROR_WRONG_KIND );
	trotListFree( &l );

	TEST_ERR_IF( trotListInit( &l ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListAppendInt( l, 1 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( addListWithValue( l, -1, 1 ) != 0 );
	TEST_ERR_IF( trotListIntOperand( l, TROT_OP_ADD ) != TROT_RC_ERROR_WRONG_KIND );
	trotListFree( &l );

	TEST_ERR_IF( trotListInit( &l ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListIntOperandValue( l, TROT_OP_ADD, 1 ) != TROT_RC_ERROR_WRONG_KIND );
	trotListFree( &l );

	TEST_ERR_IF( trotListInit( &l ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( addListWithValue( l, -1, 1 ) != 0 );
	TEST_ERR_IF( trotListIntOperandValue( l, TROT_OP_ADD, 1 ) != TROT_RC_ERROR_WRONG_KIND );
	trotListFree( &l );

	TEST_ERR_IF( trotListInit( &l ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListAppendInt( l, 1 ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( trotListIntOperandValue( l, -99, 1 ) != TROT_RC_ERROR_INVALID_OP );
	trotListFree( &l );

	return 0;


	/* CLEANUP */
	cleanup:

	return rc;
}

