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
#define TROT_NAME "Trot"

#define TROT_COPYRIGHT "Copyright (C) 2010-2014 Jeremiah Martell"

#define TROT_VERSION_STRING "0.2.00-wip"
#define TROT_VERSION 2000
#define TROT_VERSION_MAJOR       ( TROT_VERSION / 10000 )
#define TROT_VERSION_MINOR       ( ( TROT_VERSION / 1000 ) % 10 )
#define TROT_VERSION_INCREMENTAL ( ( TROT_VERSION / 10 ) % 100 )
#define TROT_VERSION_FINAL       ( TROT_VERSION % 10 )

/******************************************************************************/
/* This defines s32 as a signed 32-bit integer */
#define s32 int

/******************************************************************************/
#define TROT_RC s32

/* standard rc values */
#define TROT_RC_SUCCESS 0

#define TROT_RC_ERROR_PRECOND                  1
#define TROT_RC_ERROR_MEMORY_ALLOCATION_FAILED 2
#define TROT_RC_ERROR_STANDARD_LIBRARY_ERROR   3

/* This must be kept in sync with the above defines */
#define TROT_RC_STANDARD_ERRORS_MAX            3

/* Trot specific rc values */
#define TROT_RC_ERROR_BAD_INDEX      2001
#define TROT_RC_ERROR_WRONG_KIND     2002
#define TROT_RC_ERROR_INVALID_OP     2003
#define TROT_RC_ERROR_BAD_TAG        2004
#define TROT_RC_ERROR_DIVIDE_BY_ZERO 2005
#define TROT_RC_ERROR_UNICODE        2006
#define TROT_RC_ERROR_DECODE         2007
#define TROT_RC_ERROR_ENCODE         2008
#define TROT_RC_ERROR_LOAD           2009
#define TROT_RC_ERROR_NOT_BYTE_VALUE 2010

/* These must be kept in sync with the above defines */
#define TROT_RC_TROT_ERRORS_MIN     2001
#define TROT_RC_TROT_ERRORS_MAX     2010

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
typedef struct TrotList_STRUCT TrotList;

/******************************************************************************/
/* trotHooks.c */
extern void *(*trotHookCalloc)( size_t nmemb, size_t size );
extern void *(*trotHookMalloc)( size_t size );
extern void (*trotHookFree)( void *ptr );

/******************************************************************************/
typedef TROT_RC (*TrotLoadFunc)( TrotList *lName, TrotList **lBytes );

/******************************************************************************/
/* trotListPrimary.c */
TROT_RC trotListInit( TrotList **l_A );
TROT_RC trotListTwin( TrotList *l, TrotList **lTwin_A );
void trotListFree( TrotList **l_F );

TROT_RC trotListGetCount( TrotList *l, TROT_INT *c );

TROT_RC trotListGetKind( TrotList *l, TROT_INT index, TROT_KIND *kind );

TROT_RC trotListAppendInt( TrotList *l, TROT_INT n );
TROT_RC trotListAppendList( TrotList *l, TrotList *lToAppend );

TROT_RC trotListInsertInt( TrotList *l, TROT_INT index, TROT_INT n );
TROT_RC trotListInsertList( TrotList *l, TROT_INT index, TrotList *lToInsert );

TROT_RC trotListGetInt( TrotList *l, TROT_INT index, TROT_INT *n );
TROT_RC trotListGetList( TrotList *l, TROT_INT index, TrotList **lTwin_A );

TROT_RC trotListRemoveInt( TrotList *l, TROT_INT index, TROT_INT *n );
TROT_RC trotListRemoveList( TrotList *l, TROT_INT index, TrotList **lRemoved_A );
TROT_RC trotListRemove( TrotList *l, TROT_INT index );

TROT_RC trotListReplaceWithInt( TrotList *l, TROT_INT index, TROT_INT n );
TROT_RC trotListReplaceWithList( TrotList *l, TROT_INT index, TrotList *lToInsert );

TROT_RC trotListGetTag( TrotList *l, TROT_TAG *tag );
TROT_RC trotListSetTag( TrotList *l, TROT_TAG tag );

const char *trotRCToString( TROT_RC rc );

/******************************************************************************/
/* trotListSecondary.c */
TROT_RC trotListCompare( TrotList *l, TrotList *lCompareTo, TROT_LIST_COMPARE_RESULT *compareResult );

TROT_RC trotListCopy( TrotList *l, TrotList **lCopy_A );

TROT_RC trotListEnlist( TrotList *l, TROT_INT indexStart, TROT_INT indexEnd );
TROT_RC trotListDelist( TrotList *l, TROT_INT index );

TROT_RC trotListCopySpan( TrotList *l, TROT_INT indexStart, TROT_INT indexEnd, TrotList **lCopy_A );
TROT_RC trotListRemoveSpan( TrotList *l, TROT_INT indexStart, TROT_INT indexEnd );

/******************************************************************************/
/* trotUnicode.c */
TROT_RC trotUtf8ToCharacters( TrotList *lBytes, TrotList *lCharacters );
TROT_RC trotCharactersToUtf8( TrotList *lCharacters, TrotList *lBytes );

/******************************************************************************/
/* trotDecodingEncoding.c */
TROT_RC trotDecodeCharacters( TrotLoadFunc loadFunc, TrotList *lGivenFilenameOfCharacters, TrotList *lCharacters, TrotList **lDecodedList_A );
TROT_RC trotDecodeFilename( TrotLoadFunc loadFunc, TrotList *lFilename, TrotList **lDecodedList_A );
TROT_RC trotEncode( TrotList *l, TrotList **lCharacters_A );

/******************************************************************************/
#endif
