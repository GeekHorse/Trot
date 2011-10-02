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

/******************************************************************************/
static int testOp( INT_TYPE value1, INT_TYPE value2, TROT_INT_OPERAND op, INT_TYPE resultWanted );

/******************************************************************************/
int testIntOperands()
{
	/* DATA */
	int rc = 0;


	/* CODE */
	TEST_ERR_IF( testOp(  3,  5, TROT_INT_OPERAND_ADD,   8 ) != 0 );
	TEST_ERR_IF( testOp(  3, -5, TROT_INT_OPERAND_ADD,  -2 ) != 0 );
	TEST_ERR_IF( testOp( -3,  5, TROT_INT_OPERAND_ADD,   2 ) != 0 );
	TEST_ERR_IF( testOp( -3, -5, TROT_INT_OPERAND_ADD,  -8 ) != 0 );

	TEST_ERR_IF( testOp(  3,  5, TROT_INT_OPERAND_SUB,  -2 ) != 0 );
	TEST_ERR_IF( testOp(  3, -5, TROT_INT_OPERAND_SUB,   8 ) != 0 );
	TEST_ERR_IF( testOp( -3,  5, TROT_INT_OPERAND_SUB,  -8 ) != 0 );
	TEST_ERR_IF( testOp( -3, -5, TROT_INT_OPERAND_SUB,   2 ) != 0 );

	TEST_ERR_IF( testOp(  3,  5, TROT_INT_OPERAND_MUL,  15 ) != 0 );
	TEST_ERR_IF( testOp(  3, -5, TROT_INT_OPERAND_MUL, -15 ) != 0 );
	TEST_ERR_IF( testOp( -3,  5, TROT_INT_OPERAND_MUL, -15 ) != 0 );
	TEST_ERR_IF( testOp( -3, -5, TROT_INT_OPERAND_MUL,  15 ) != 0 );

	TEST_ERR_IF( testOp(  15,  6, TROT_INT_OPERAND_DIV,   2 ) != 0 );
	TEST_ERR_IF( testOp(  15, -6, TROT_INT_OPERAND_DIV,  -2 ) != 0 );
	TEST_ERR_IF( testOp( -15,  6, TROT_INT_OPERAND_DIV,  -2 ) != 0 );
	TEST_ERR_IF( testOp( -15, -6, TROT_INT_OPERAND_DIV,   2 ) != 0 );

	TEST_ERR_IF( testOp(  15,  6, TROT_INT_OPERAND_MOD,   3 ) != 0 );
	TEST_ERR_IF( testOp(  15, -6, TROT_INT_OPERAND_MOD,   3 ) != 0 );
	TEST_ERR_IF( testOp( -15,  6, TROT_INT_OPERAND_MOD,  -3 ) != 0 );
	TEST_ERR_IF( testOp( -15, -6, TROT_INT_OPERAND_MOD,  -3 ) != 0 );

	TEST_ERR_IF( testOp(  0,  0, TROT_INT_OPERAND_LOGICAL_AND,   0 ) != 0 );
	TEST_ERR_IF( testOp(  5,  0, TROT_INT_OPERAND_LOGICAL_AND,   0 ) != 0 );
	TEST_ERR_IF( testOp(  0, -5, TROT_INT_OPERAND_LOGICAL_AND,   0 ) != 0 );
	TEST_ERR_IF( testOp( -5,  5, TROT_INT_OPERAND_LOGICAL_AND,   1 ) != 0 );
	TEST_ERR_IF( testOp(  5, -5, TROT_INT_OPERAND_LOGICAL_AND,   1 ) != 0 );
	TEST_ERR_IF( testOp( -5, -5, TROT_INT_OPERAND_LOGICAL_AND,   1 ) != 0 );

	TEST_ERR_IF( testOp(  0,  0, TROT_INT_OPERAND_LOGICAL_OR,    0 ) != 0 );
	TEST_ERR_IF( testOp( -5,  0, TROT_INT_OPERAND_LOGICAL_OR,    1 ) != 0 );
	TEST_ERR_IF( testOp(  0,  5, TROT_INT_OPERAND_LOGICAL_OR,    1 ) != 0 );
	TEST_ERR_IF( testOp(  5, -5, TROT_INT_OPERAND_LOGICAL_OR,    1 ) != 0 );
	TEST_ERR_IF( testOp( -5,  5, TROT_INT_OPERAND_LOGICAL_OR,    1 ) != 0 );
	TEST_ERR_IF( testOp( -5, -5, TROT_INT_OPERAND_LOGICAL_OR,    1 ) != 0 );


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
static int testOp( INT_TYPE value1, INT_TYPE value2, TROT_INT_OPERAND op, INT_TYPE resultWanted )
{
	/* DATA */
	int rc = 0;

	trotListRef *lr = NULL;

	INT_TYPE resultActual = 0;


	/* CODE */
	TEST_ERR_IF( trotListRefInit( &lr ) != TROT_LIST_SUCCESS );

	TEST_ERR_IF( trotListRefAppendInt( lr, value1 ) != TROT_LIST_SUCCESS );
	TEST_ERR_IF( trotListRefAppendInt( lr, value2 ) != TROT_LIST_SUCCESS );

	TEST_ERR_IF( checkList( lr ) != 0 );
	TEST_ERR_IF( trotListIntOperand( lr, op ) != TROT_LIST_SUCCESS );
	TEST_ERR_IF( checkList( lr ) != 0 );

	TEST_ERR_IF( trotListRefGetInt( lr, -1, &resultActual ) != TROT_LIST_SUCCESS );
	TEST_ERR_IF( resultActual != resultWanted );


	/* CLEANUP */
	cleanup:

	trotListRefFree( &lr );

	return rc;	
}

