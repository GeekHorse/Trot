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
#define MAGIC_NUMBER (NODE_SIZE * 3)


/******************************************************************************/
/* create functions */
static int (*createFunctions[])( trotListRef **, int ) =
	{
		createAllInts,
		createAllLists,
		createIntListAlternating,
		createListIntAlternating,
		createHalfIntHalfList,
		createHalfListHalfInt,
		NULL
	};


/******************************************************************************/
int testSecondaryFunctionality()
{
	/* DATA */
	int rc = 0;

	int count = 0;
	int i = 0;
	int j = 0;

	trotListRef *lr1 = NULL;
	trotListRef *lr2 = NULL;

	TROT_LIST_COMPARE_RESULT compareResult = TROT_LIST_COMPARE_EQUAL;


	/* CODE */
	printf( "Testing secondary functionality..." ); fflush( stdout );
	count = 3; /* TODO: still need to test empty and 1 count lists? */
	while ( count <= MAGIC_NUMBER )
	{
		printf( "\n%3d ", count );

		i = 0;
		while ( createFunctions[ i ] != NULL )
		{
			printf( ":" ); fflush( stdout );

			j = 0;
			while ( createFunctions[ j ] != NULL )
			{
				TEST_ERR_IF( createFunctions[ i ]( &lr1, count ) != 0 );
				TEST_ERR_IF( createFunctions[ j ]( &lr2, count ) != 0 );

				/* compare */
				TEST_ERR_IF( trotListRefCompare( lr1, lr2, &compareResult ) != TROT_LIST_SUCCESS );

				if ( compareResult == TROT_LIST_COMPARE_EQUAL )
				{
					printf( "i(%d) j(%d) c(%d)\n", i, j, count ); fflush( stdout );
					TEST_ERR_IF( i != j && count != 0 );
				}
				else if ( compareResult == TROT_LIST_COMPARE_LESS_THAN )
				{
					TEST_ERR_IF( trotListRefCompare( lr2, lr1, &compareResult ) != TROT_LIST_SUCCESS );
					TEST_ERR_IF( compareResult != TROT_LIST_COMPARE_GREATER_THAN );
				}
				else if ( compareResult == TROT_LIST_COMPARE_GREATER_THAN )
				{
					TEST_ERR_IF( trotListRefCompare( lr2, lr1, &compareResult ) != TROT_LIST_SUCCESS );
					TEST_ERR_IF( compareResult != TROT_LIST_COMPARE_LESS_THAN );
				}
				else
				{
					TEST_ERR_IF( 1 );
				}

				trotListRefFree( &lr1 );
				trotListRefFree( &lr2 );

				j += 1;
			}


			i += 1;
		}

		if ( count == 0 )
		{
			count = 1;
		}
		else
		{
			count *= 2;
		}
	}

	printf( "\n" ); fflush( stdout );

	return 0;


	/* CLEANUP */
	cleanup:

	return rc;
}

