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
#include "trot.h"

#include "trotTestCommon.h"

#include <string.h> /* strcmp */

/******************************************************************************/
static int _getArgValue( int argc, char **argv, char *key, char **value );

/******************************************************************************/
int main( int argc, char **argv )
{
	/* DATA */
	int rc = 0;

	char *argValue = 0;

	int seed = 1;

	int flagTestAll = 0;
	int flagTestPreconditions = 0;
	int flagTestMemory = 0;
	int flagTestBadTypesIndicesIntegerOps = 0;
	int flagTestPrimaryFunctionality = 0;
	int flagTestSecondaryFunctionality = 0;
	int flagTestIntOperands = 0;
	int flagTestUnicode = 0;
	int flagTestDecodingEncoding = 0;

	int timeStart = 0;
	int timeEnd = 0;


	/* CODE */
	if ( argc <= 1 )
	{
		printf( "Usage: trotTest [options]\n" );
		printf( "  -s <NUMBER>    Seed for random number generator\n" );
		printf( "  -t <TEST>      Test to run\n" );
		printf( "                 Possible tests:\n" );
		printf( "                   all = all tests\n" );
		printf( "                   pre = preconditions\n" );
		printf( "                   mem = memory\n" );
		printf( "                   bad = bad types, indices, and integer ops\n" );
		printf( "                   f1  = primary functionality\n" );
		printf( "                   f2  = secondary functionality\n" );
		printf( "                   int = integer operands\n" );
		printf( "                   uni = unicode\n" );
		printf( "                   cod = decoding, encoding\n" );
		printf( "\n" );

		return -1;
	}

	/* **************************************** */
	rc = _getArgValue( argc, argv, "-s", &argValue );
	if ( rc == 0 )
	{
		seed = atol( argValue );
	}
	else
	{
		seed = time( NULL );
	}

	printf( "Using seed: %d\n", seed ); fflush( stdout );
	srand( seed );

	/* **************************************** */
	rc = _getArgValue( argc, argv, "-t", &argValue );
	if ( rc == 0 )
	{
		if ( strcmp( argValue, "all" ) == 0 )
		{
			flagTestAll = 1;
		}
		else if ( strcmp( argValue, "pre" ) == 0 )
		{
			flagTestPreconditions = 1;
		}
		else if ( strcmp( argValue, "mem" ) == 0 )
		{
			flagTestMemory = 1;
		}
		else if ( strcmp( argValue, "bad" ) == 0 )
		{
			flagTestBadTypesIndicesIntegerOps = 1;
		}
		else if ( strcmp( argValue, "f1" ) == 0 )
		{
			flagTestPrimaryFunctionality = 1;
		}
		else if ( strcmp( argValue, "f2" ) == 0 )
		{
			flagTestSecondaryFunctionality = 1;
		}
		else if ( strcmp( argValue, "int" ) == 0 )
		{
			flagTestIntOperands = 1;
		}
		else if ( strcmp( argValue, "uni" ) == 0 )
		{
			flagTestUnicode = 1;
		}
		else if ( strcmp( argValue, "cod" ) == 0 )
		{
			flagTestDecodingEncoding = 1;
		}
		else
		{
			printf( "UNKNOWN TEST TO RUN: \"%s\"\n", argValue );
			TEST_ERR_IF( 1 );
		}
	}

	/* **************************************** */
	timeStart = time( NULL );

	/* **************************************** */
	TEST_ERR_IF( sizeof( INT_TYPE ) != INT_TYPE_SIZE );

	/* **************************************** */
	if ( flagTestAll || flagTestPreconditions )
	{
		TEST_ERR_IF( testPreconditions() != 0 );
	}

	if ( flagTestAll || flagTestMemory )
	{
		TEST_ERR_IF( testMemory() != 0 );
	}

	if ( flagTestAll || flagTestBadTypesIndicesIntegerOps )
	{
		TEST_ERR_IF( testBadTypesAndIndices() != 0 );
		TEST_ERR_IF( testBadIntegerOps() != 0 );
	}

	if ( flagTestAll || flagTestPrimaryFunctionality )
	{
		TEST_ERR_IF( testPrimaryFunctionality() != 0 );
	}

	if ( flagTestAll || flagTestSecondaryFunctionality )
	{
		TEST_ERR_IF( testSecondaryFunctionality() != 0 );
	}

	if ( flagTestAll || flagTestIntOperands )
	{
		TEST_ERR_IF( testIntOperands() != 0 );
	}

	if ( flagTestAll || flagTestUnicode )
	{
		TEST_ERR_IF( testUnicode() != 0 );
	}

	if ( flagTestAll || flagTestDecodingEncoding )
	{
		TEST_ERR_IF( testDecodingEncoding() != 0 );
	}

	/* **************************************** */
	timeEnd = time( NULL );

	printf( "\nTests took %d:%02d:%02d to complete\n",
		( timeEnd - timeStart ) / (60 * 60),
		( ( timeEnd - timeStart ) / 60 ) % 60,
		( timeEnd - timeStart ) % 60 );

	/* **************************************** */
	/* success! */
	printf( "\x1b[32;1mSUCCESS!\n\n\x1b[0m" );

	return 0;


	/* CLEANUP */
	cleanup:

	printf( "\x1b[31;1mFAILED AT %d\n\x1b[0m", rc );

	return rc;
}

/******************************************************************************/
static int _getArgValue( int argc, char **argv, char *key, char **value )
{
	/* DATA */
	int i = 0;


	/* CODE */
	while ( i < argc )
	{
		if ( strcmp( key, argv[ i ] ) == 0 )
		{
			i += 1;
			if ( i >= argc )
			{
				printf( "ERROR: Command line argument \"%s\" had no value.\n", key );
				return -1;
			}

			(*value) = argv[ i ];
			return 0;
		}

		i += 1;
	}

	return -2;
}

