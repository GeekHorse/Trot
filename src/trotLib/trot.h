/*
Copyright (c) 2010,2011,2012, Jeremiah Martell
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
#ifndef trot_H
#define trot_H

/******************************************************************************/
#include <stdlib.h> /* for size_t, in memory hook functions */

/******************************************************************************/
typedef enum
{
	TROT_RC_SUCCESS =  0, /* TODO: paranoid is -1, rest are positive, and we need an rc-to-string function */
	TROT_RC_ERROR_PRECOND = -1,
	TROT_RC_ERROR_PARANOID = -2,
	TROT_RC_ERROR_MEMORY_ALLOCATION_FAILED = -3,
	TROT_RC_ERROR_STANDARD_LIBRARY_ERROR = -4,
	TROT_RC_ERROR_BAD_INDEX = -5,
	TROT_RC_ERROR_WRONG_KIND = -6,
	TROT_RC_ERROR_INVALID_OP = -7,
	TROT_RC_ERROR_BAD_TAG = -8,
	TROT_RC_ERROR_DIVIDE_BY_ZERO = -9,
	TROT_RC_ERROR_UNICODE = -10,
	TROT_RC_ERROR_DECODE = -11,
	TROT_RC_ERROR_ENCODE = -12,
	TROT_RC_ERROR_LOAD = -13,
	TROT_RC_ERROR_NOT_BYTE_VALUE = -14
} TROT_RC;

/******************************************************************************/
typedef enum
{
	TROT_KIND_INT = 1,
	TROT_KIND_LIST = 2
} TROT_KIND;

/******************************************************************************/
typedef enum
{
	TROT_TAG_DATA =  1,
	TROT_TAG_TEXT = 2,

	TROT_TAG_CODE = 3,
	TROT_TAG_FUNCTION = 4,
	TROT_TAG_RAW_CODE = 5
} TROT_TAG;
#define TROT_TAG_MIN 1
#define TROT_TAG_MAX 5

/******************************************************************************/
#define TROT_INT_SIZE 4

#if ( TROT_INT_SIZE == 4 )
#define TROT_INT int
#define TROT_INT_MAX         2147483647
#define TROT_INT_MAX_STRING "2147483647"
#define TROT_INT_MAX_STRING_LENGTH 10
#define TROT_INT_MIN_STRING "-2147483648"
#define TROT_INT_MIN_STRING_LENGTH 11
#else
#error NEED TO DEFINE TROT_INT FOR TROT_INT_SIZE
#endif

/******************************************************************************/
typedef enum
{
	TROT_LIST_COMPARE_LESS_THAN    = -1,
	TROT_LIST_COMPARE_EQUAL        =  0,
	TROT_LIST_COMPARE_GREATER_THAN =  1
} TROT_LIST_COMPARE_RESULT;

/******************************************************************************/
typedef struct trotList_STRUCT trotList; /* TODO: rename this TrotList */

/******************************************************************************/
/* trotHooks.c */
extern void *(*trotHookCalloc)( size_t nmemb, size_t size );
extern void *(*trotHookMalloc)( size_t size );
extern void (*trotHookFree)( void *ptr );

/******************************************************************************/
typedef TROT_RC (*TrotLoadFunc)( trotList *lName, trotList **lBytes );

/******************************************************************************/
/* trotListPrimary.c */
TROT_RC trotListInit( trotList **l_A );
TROT_RC trotListTwin( trotList *l, trotList **lTwin_A );
void trotListFree( trotList **l_F );

TROT_RC trotListGetCount( trotList *l, TROT_INT *c );

TROT_RC trotListGetKind( trotList *l, TROT_INT index, TROT_KIND *kind );

TROT_RC trotListAppendInt( trotList *l, TROT_INT n );
TROT_RC trotListAppendList( trotList *l, trotList *lToAppend );

TROT_RC trotListInsertInt( trotList *l, TROT_INT index, TROT_INT n );
TROT_RC trotListInsertList( trotList *l, TROT_INT index, trotList *lToInsert );

TROT_RC trotListGetInt( trotList *l, TROT_INT index, TROT_INT *n );
TROT_RC trotListGetList( trotList *l, TROT_INT index, trotList **lTwin_A );

TROT_RC trotListRemoveInt( trotList *l, TROT_INT index, TROT_INT *n );
TROT_RC trotListRemoveList( trotList *l, TROT_INT index, trotList **lRemoved_A );
TROT_RC trotListRemove( trotList *l, TROT_INT index );

TROT_RC trotListReplaceWithInt( trotList *l, TROT_INT index, TROT_INT n );
TROT_RC trotListReplaceWithList( trotList *l, TROT_INT index, trotList *lToInsert );

TROT_RC trotListGetTag( trotList *l, TROT_TAG *tag );
TROT_RC trotListSetTag( trotList *l, TROT_TAG tag );

/******************************************************************************/
/* trotListSecondary.c */
TROT_RC trotListCompare( trotList *l, trotList *lCompareTo, TROT_LIST_COMPARE_RESULT *compareResult );

TROT_RC trotListCopy( trotList *l, trotList **lCopy_A );

TROT_RC trotListEnlist( trotList *l, TROT_INT indexStart, TROT_INT indexEnd );
TROT_RC trotListDelist( trotList *l, TROT_INT index );

TROT_RC trotListCopySpan( trotList *l, TROT_INT indexStart, TROT_INT indexEnd, trotList **lCopy_A );
TROT_RC trotListRemoveSpan( trotList *l, TROT_INT indexStart, TROT_INT indexEnd );

/******************************************************************************/
/* trotUnicode.c */
TROT_RC trotUtf8ToCharacters( trotList *lBytes, trotList *lCharacters );
TROT_RC trotCharactersToUtf8( trotList *lCharacters, trotList *lBytes );

/******************************************************************************/
/* trotDecodingEncoding.c */
TROT_RC trotDecodeCharacters( TrotLoadFunc loadFunc, trotList *lGivenFilenameOfCharacters, trotList *lCharacters, trotList **lDecodedList_A );
TROT_RC trotDecodeFilename( TrotLoadFunc loadFunc, trotList *lFilename, trotList **lDecodedList_A );
TROT_RC trotEncode( trotList *l, trotList **lCharacters_A );

/******************************************************************************/
#endif

