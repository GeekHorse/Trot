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
#define TROT_LIST_SUCCESS 0

#define TROT_LIST_ERROR_GENERAL -1
#define TROT_LIST_ERROR_PRECOND -2
#define TROT_LIST_ERROR_MEMORY_ALLOCATION_FAILED -3
#define TROT_LIST_ERROR_BAD_INDEX -4
#define TROT_LIST_ERROR_WRONG_KIND -5

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
typedef struct trotListRef_STRUCT trotListRef;
typedef struct trotListNode_STRUCT trotListNode;
typedef struct trotList_STRUCT trotList;
typedef struct trotListRefListNode_STRUCT trotListRefListNode;

/******************************************************************************/
/* trotListPrimary.c */
int trotListRefInit( trotListRef **lr_A );
int trotListRefTwin( trotListRef **lr_A, trotListRef *lrToTwin );
int trotListRefFree( trotListRef **lr_F );

int trotListRefGetCount( trotListRef *lr, INT_TYPE *c );

int trotListRefGetKind( trotListRef *lr, INT_TYPE index, int *kind );

int trotListRefAppendInt( trotListRef *lr, INT_TYPE n );
int trotListRefAppendListTwin( trotListRef *lr, trotListRef *lrToAppend );

int trotListRefInsertInt( trotListRef *lr, INT_TYPE index, INT_TYPE n );
int trotListRefInsertListTwin( trotListRef *lr, INT_TYPE index, trotListRef *l );

int trotListRefGetInt( trotListRef *lr, INT_TYPE index, INT_TYPE *n );
int trotListRefGetListTwin( trotListRef *lr, INT_TYPE index, trotListRef **l );

int trotListRefRemoveInt( trotListRef *lr, INT_TYPE index, INT_TYPE *n );
int trotListRefRemoveList( trotListRef *lr, INT_TYPE index, trotListRef **l );
int trotListRefRemove( trotListRef *lr, INT_TYPE index );

/******************************************************************************/
/* trotListSecondary.c */
int trotListRefCompare( trotListRef *lr, trotListRef *lrCompareTo, TROT_LIST_COMPARE_RESULT *compareResult );

int trotListRefCopy( trotListRef **lrCopy_A, trotListRef *lr );

int trotListRefEnlist( trotListRef *lr, INT_TYPE indexStart, INT_TYPE indexEnd );
int trotListRefDelist( trotListRef *lr, INT_TYPE index );

int trotListRefCopySpan( trotListRef **lrCopy_A, trotListRef *lr, INT_TYPE indexStart, INT_TYPE indexEnd );
int trotListRefRemoveSpan( trotListRef *lr, INT_TYPE indexStart, INT_TYPE indexEnd );

/******************************************************************************/
#endif

