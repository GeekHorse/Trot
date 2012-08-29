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

#ifndef trotTestCommon_H
#define trotTestCommon_H

/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/******************************************************************************/
#define TEST_ERR_IF( x ) \
	if ( (x) ) \
	{ \
		printf( "TEST ERROR: test failed in " __FILE__ " on line %d\n", __LINE__ ); \
		fflush( stdout ); \
		rc = -__LINE__; \
		goto cleanup; \
	}

/* NOTE: Because inserting is different between positive and negative numbers,
         you need two defines. One that works with 'getters' or 'removers', and
         one that works with 'inserters'.
         Example: If your list has 5 items, and you want to get the fifth, you
         can get index 5 or index -1. However, if you want to add an element
         at the end of the list, you would add with index 6 or index -1. */
#define INDEX_TO_NEGATIVE_VERSION_GET_OR_REMOVE( index, count ) ( ( ( ( count ) + 1 ) * -1 ) + index )
#define INDEX_TO_NEGATIVE_VERSION_INSERT(        index, count ) ( ( ( ( count ) + 1 ) * -1 ) + index - 1 )

/******************************************************************************/
/* major test functions */
int testPreconditions();
int testMemory();
int testBadTypesAndIndices();
int testPrimaryFunctionality();
int testSecondaryFunctionality();
int testIntOperands();
int testBadIntegerOps();
int testUnicode();
int testDecodingEncoding();

/******************************************************************************/
/* create functions */
int createAllInts( trotListRef **lr, int count );
int createAllLists( trotListRef **lr, int count );
int createIntListAlternating( trotListRef **lr, int count );
int createListIntAlternating( trotListRef **lr, int count );
int createHalfIntHalfList( trotListRef **lr, int count );
int createHalfListHalfInt( trotListRef **lr, int count );

int createSelfRefs( trotListRef **lr, int count );

/******************************************************************************/
/* misc functions */
int addListWithValue( trotListRef *lr, INT_TYPE index, INT_TYPE value );
int check( trotListRef *lr, INT_TYPE index, INT_TYPE valueToCheckAgainst );
int checkList( trotListRef *lr );
void printList( trotListRef *lr, int indent );
int load( trotListRef *lrName, trotListRef **lrBytes );

TROT_RC listToCString( trotListRef *lr, char **cString_A );
TROT_RC appendCStringToList( trotListRef *lr, char *cString );

/******************************************************************************/
#endif

