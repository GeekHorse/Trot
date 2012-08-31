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
	TROT_RC_SUCCESS =  0,
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
typedef struct trotListRef_STRUCT trotListRef;

/******************************************************************************/
/* trotHooks.c */
extern void *(*trotCalloc)( size_t nmemb, size_t size );
extern void *(*trotMalloc)( size_t size );
extern void (*trotFree)( void *ptr );

/******************************************************************************/
typedef TROT_RC (*TrotLoadFunc)( trotListRef *lrName, trotListRef **lrBytes );

/******************************************************************************/
/* trotListPrimary.c */
TROT_RC trotListRefInit( trotListRef **lr_A );
TROT_RC trotListRefTwin( trotListRef *lr, trotListRef **lrTwin_A );
void trotListRefFree( trotListRef **lr_F );

TROT_RC trotListRefGetCount( trotListRef *lr, TROT_INT *c );

TROT_RC trotListRefGetKind( trotListRef *lr, TROT_INT index, TROT_KIND *kind );

TROT_RC trotListRefAppendInt( trotListRef *lr, TROT_INT n );
TROT_RC trotListRefAppendListTwin( trotListRef *lr, trotListRef *lrToAppend );

TROT_RC trotListRefInsertInt( trotListRef *lr, TROT_INT index, TROT_INT n );
TROT_RC trotListRefInsertListTwin( trotListRef *lr, TROT_INT index, trotListRef *lrToInsert );

TROT_RC trotListRefGetInt( trotListRef *lr, TROT_INT index, TROT_INT *n );
TROT_RC trotListRefGetListTwin( trotListRef *lr, TROT_INT index, trotListRef **lrTwin_A );

TROT_RC trotListRefRemoveInt( trotListRef *lr, TROT_INT index, TROT_INT *n );
TROT_RC trotListRefRemoveList( trotListRef *lr, TROT_INT index, trotListRef **lrRemoved_A );
TROT_RC trotListRefRemove( trotListRef *lr, TROT_INT index );

TROT_RC trotListRefReplaceWithInt( trotListRef *lr, TROT_INT index, TROT_INT n );
TROT_RC trotListRefReplaceWithList( trotListRef *lr, TROT_INT index, trotListRef *lrToInsert );

TROT_RC trotListRefGetTag( trotListRef *lr, TROT_TAG *tag );
TROT_RC trotListRefSetTag( trotListRef *lr, TROT_TAG tag );

/******************************************************************************/
/* trotListSecondary.c */
TROT_RC trotListRefCompare( trotListRef *lr, trotListRef *lrCompareTo, TROT_LIST_COMPARE_RESULT *compareResult );

TROT_RC trotListRefCopy( trotListRef *lr, trotListRef **lrCopy_A );

TROT_RC trotListRefEnlist( trotListRef *lr, TROT_INT indexStart, TROT_INT indexEnd );
TROT_RC trotListRefDelist( trotListRef *lr, TROT_INT index );

TROT_RC trotListRefCopySpan( trotListRef *lr, TROT_INT indexStart, TROT_INT indexEnd, trotListRef **lrCopy_A );
TROT_RC trotListRefRemoveSpan( trotListRef *lr, TROT_INT indexStart, TROT_INT indexEnd );

/******************************************************************************/
/* trotUnicode.c */
TROT_RC trotUtf8ToCharacters( trotListRef *lrBytes, trotListRef *lrCharacters );
TROT_RC trotCharactersToUtf8( trotListRef *lrCharacters, trotListRef *lrBytes );

/******************************************************************************/
/* trotDecodingEncoding.c */
TROT_RC trotDecodeCharacters( TrotLoadFunc loadFunc, trotListRef *lrGivenFilenameOfCharacters, trotListRef *lrCharacters, trotListRef **lrDecodedList_A );
TROT_RC trotDecodeFilename( TrotLoadFunc loadFunc, trotListRef *lrFilename, trotListRef **lrDecodedList_A );
TROT_RC trotEncode( trotListRef *lr, trotListRef **lrCharacters_A );

/******************************************************************************/
#endif

