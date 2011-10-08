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
#ifndef trotList_H
#define trotList_H

/******************************************************************************/
typedef enum
{
	TROT_LIST_SUCCESS =  0,
	TROT_LIST_ERROR_PRECOND = -1,
	TROT_LIST_ERROR_PARANOID = -2,
	TROT_LIST_ERROR_MEMORY_ALLOCATION_FAILED = -3,
	TROT_LIST_ERROR_BAD_INDEX = -4,
	TROT_LIST_ERROR_WRONG_KIND = -5,
	TROT_LIST_ERROR_INVALID_OP = -6,
	TROT_LIST_ERROR_DIVIDE_BY_ZERO = -7
} TROT_RC;

/******************************************************************************/
#define INT_TYPE int

/******************************************************************************/
typedef enum
{
	TROT_LIST_COMPARE_LESS_THAN    = -1,
	TROT_LIST_COMPARE_EQUAL        =  0,
	TROT_LIST_COMPARE_GREATER_THAN =  1
} TROT_LIST_COMPARE_RESULT;

/******************************************************************************/
typedef enum
{
	TROT_INT_OPERAND_ADD = 1,
	TROT_INT_OPERAND_SUB = 2,
	TROT_INT_OPERAND_MUL = 3,
	TROT_INT_OPERAND_DIV = 4,
	TROT_INT_OPERAND_MOD = 5,
	TROT_INT_OPERAND_NEG = 6,

	TROT_INT_OPERAND_LOGICAL_AND = 7,
	TROT_INT_OPERAND_LOGICAL_OR =  8,
	TROT_INT_OPERAND_LOGICAL_NOT = 9
} TROT_INT_OPERAND;

/* NOTE: keep these in sync with the above enum */
#define TROT_INT_OPERAND_MIN 1
#define TROT_INT_OPERAND_MAX 9

/******************************************************************************/
typedef struct trotListRef_STRUCT trotListRef;
typedef struct trotListNode_STRUCT trotListNode;
typedef struct trotList_STRUCT trotList;
typedef struct trotListRefListNode_STRUCT trotListRefListNode;

/******************************************************************************/
/* trotListPrimary.c */
TROT_RC trotListRefInit( trotListRef **lr_A );
TROT_RC trotListRefTwin( trotListRef **lr_A, trotListRef *lrToTwin );
void trotListRefFree( trotListRef **lr_F );

TROT_RC trotListRefGetCount( trotListRef *lr, INT_TYPE *c );

TROT_RC trotListRefGetKind( trotListRef *lr, INT_TYPE index, int *kind );

TROT_RC trotListRefAppendInt( trotListRef *lr, INT_TYPE n );
TROT_RC trotListRefAppendListTwin( trotListRef *lr, trotListRef *lrToAppend );

TROT_RC trotListRefInsertInt( trotListRef *lr, INT_TYPE index, INT_TYPE n );
TROT_RC trotListRefInsertListTwin( trotListRef *lr, INT_TYPE index, trotListRef *l );

TROT_RC trotListRefGetInt( trotListRef *lr, INT_TYPE index, INT_TYPE *n );
TROT_RC trotListRefGetListTwin( trotListRef *lr, INT_TYPE index, trotListRef **l );

TROT_RC trotListRefRemoveInt( trotListRef *lr, INT_TYPE index, INT_TYPE *n );
TROT_RC trotListRefRemoveList( trotListRef *lr, INT_TYPE index, trotListRef **l );
TROT_RC trotListRefRemove( trotListRef *lr, INT_TYPE index );

/******************************************************************************/
/* trotListSecondary.c */
TROT_RC trotListRefCompare( trotListRef *lr, trotListRef *lrCompareTo, TROT_LIST_COMPARE_RESULT *compareResult );

TROT_RC trotListRefCopy( trotListRef **lrCopy_A, trotListRef *lr );

TROT_RC trotListRefEnlist( trotListRef *lr, INT_TYPE indexStart, INT_TYPE indexEnd );
TROT_RC trotListRefDelist( trotListRef *lr, INT_TYPE index );

TROT_RC trotListRefCopySpan( trotListRef **lrCopy_A, trotListRef *lr, INT_TYPE indexStart, INT_TYPE indexEnd );
TROT_RC trotListRefRemoveSpan( trotListRef *lr, INT_TYPE indexStart, INT_TYPE indexEnd );

/******************************************************************************/
/* trotListInt.c */
TROT_RC trotListIntOperand( trotListRef *lr, TROT_INT_OPERAND op );
TROT_RC trotListIntOperandValue( trotListRef *lr, TROT_INT_OPERAND op, INT_TYPE value );

/******************************************************************************/
#endif

