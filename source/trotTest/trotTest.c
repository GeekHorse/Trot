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

#include <stdio.h> /* printf(), fprintf(), fflush() */ 
#include <string.h> /* strcmp() */
#include <time.h> /* time(), for seeding random number generator */
#include <errno.h> /* errno, for log function */
#include <string.h> /* strerror, for log function */

/******************************************************************************/
static int getArgValue( int argc, char **argv, char *key, char **value );

/******************************************************************************/
int main( int argc, char **argv )
{
	/* DATA */
	int rc = 0;

	char *argValue = 0;

	int seed = 1;

	int flagTestAll = 0;
	int flagTestPreconditions = 0;
	int flagTestMisc = 0;
	int flagTestMemory = 0;
	int flagTestBadTypesIndices = 0;
	int flagTestListFunctions = 0;
	int flagTestDecodingEncoding = 0;

	int flagTestAnySet = 0;

	TrotProgram *program = NULL;
	TROT_INT memUsed = 0;


	/* CODE */
	printf( "Testing:\n" );
	printf( "     %s %s\n", TROT_NAME, TROT_VERSION_STRING );
	printf( "     %s\n", TROT_COPYRIGHT );
	printf( "\n" );

	/* **************************************** */
	rc = getArgValue( argc, argv, "-s", &argValue );
	if ( rc == 0 )
	{
		seed = atol( argValue );
	}
	else
	{
		seed = time( NULL );
	}

	/* **************************************** */
	rc = getArgValue( argc, argv, "-t", &argValue );
	if ( rc == 0 )
	{
		if ( strcmp( argValue, "all" ) == 0 )
		{
			flagTestAll = 1;
			flagTestAnySet = 1;
		}
		else if ( strcmp( argValue, "pre" ) == 0 )
		{
			flagTestPreconditions = 1;
			flagTestAnySet = 1;
		}
		else if ( strcmp( argValue, "misc" ) == 0 )
		{
			flagTestMisc = 1;
			flagTestAnySet = 1;
		}
		else if ( strcmp( argValue, "mem" ) == 0 )
		{
			flagTestMemory = 1;
			flagTestAnySet = 1;
		}
		else if ( strcmp( argValue, "bad" ) == 0 )
		{
			flagTestBadTypesIndices = 1;
			flagTestAnySet = 1;
		}
		else if ( strcmp( argValue, "lst" ) == 0 )
		{
			flagTestListFunctions = 1;
			flagTestAnySet = 1;
		}
		else if ( strcmp( argValue, "cod" ) == 0 )
		{
			flagTestDecodingEncoding = 1;
			flagTestAnySet = 1;
		}
		else
		{
			fprintf( stderr, "UNKNOWN TEST TO RUN: \"%s\"\n", argValue );
			TEST_ERR_IF( 1 );
		}
	}

	if ( flagTestAnySet == 0 )
	{
		fprintf( stderr, "Usage: trotTest [options]\n" );
		fprintf( stderr, "  -s <NUMBER>    Seed for random number generator\n" );
		fprintf( stderr, "  -t <TEST>      Test to run\n" );
		fprintf( stderr, "                 Possible tests:\n" );
		fprintf( stderr, "                   all = all tests\n" );
		fprintf( stderr, "                   pre = preconditions\n" );
		fprintf( stderr, "                   mem = memory\n" );
		fprintf( stderr, "                   bad = bad types and indices\n" );
		fprintf( stderr, "                   lst = list functions\n" );
		fprintf( stderr, "                   cod = decoding, encoding\n" );
		fprintf( stderr, "\n" );

		return -1;
	}

	/* **************************************** */
	printf( "Using seed: %d\n", seed ); fflush( stdout );
	srand( seed );

	/* **************************************** */
	TEST_ERR_IF( sizeof( TROT_INT ) != TROT_INT_SIZE );
	TEST_ERR_IF( sizeof( s32 ) != 4 );
	TEST_ERR_IF( sizeof( u8 ) != 1 );
	TEST_ERR_IF( TROT_MAX_CHILDREN > TROT_INT_MAX );

	/* Technically TROT_NODE_SIZE could probably be as small as 1, but then it
	   becomes impossible to get 100% code coverage since certain edge cases
	   would never happen. 4 is small, but still large enough so we can get
	   100% code coverage without modifying our test cases */
	TEST_ERR_IF( TROT_NODE_SIZE < 4 );

	/* **************************************** */
	TEST_ERR_IF( ( program = TROT_HOOK_CALLOC( 1, sizeof( *program ) ) ) == NULL );

	TEST_ERR_IF( trotProgramMemorySetLimit( program, TROT_INT_MAX ) != TROT_RC_SUCCESS );

	TEST_ERR_IF( trotProgramMemoryGetUsed( program, &memUsed ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( memUsed != 0 );

	if ( flagTestAll || flagTestMisc )
	{
		TEST_ERR_IF( testMisc( program ) != 0 );
	}

	TEST_ERR_IF( trotProgramMemoryGetUsed( program, &memUsed ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( memUsed != 0 );

	if ( flagTestAll || flagTestPreconditions )
	{
		TEST_ERR_IF( testPreconditions( program ) != 0 );
	}

	TEST_ERR_IF( trotProgramMemoryGetUsed( program, &memUsed ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( memUsed != 0 );

	if ( flagTestAll || flagTestBadTypesIndices )
	{
		TEST_ERR_IF( testBadTypesAndIndices( program ) != 0 );
	}

	TEST_ERR_IF( trotProgramMemoryGetUsed( program, &memUsed ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( memUsed != 0 );

	if ( flagTestAll || flagTestListFunctions )
	{
		TEST_ERR_IF( testListFunctions( program ) != 0 );
	}

	TEST_ERR_IF( trotProgramMemoryGetUsed( program, &memUsed ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( memUsed != 0 );

	if ( flagTestAll || flagTestDecodingEncoding )
	{
		TEST_ERR_IF( testDecodingEncoding( program ) != 0 );
	}

	TEST_ERR_IF( trotProgramMemoryGetUsed( program, &memUsed ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( memUsed != 0 );

	if ( flagTestAll || flagTestMemory )
	{
		TEST_ERR_IF( testMemory( program ) != 0 );
	}

	TEST_ERR_IF( trotProgramMemoryGetUsed( program, &memUsed ) != TROT_RC_SUCCESS );
	TEST_ERR_IF( memUsed != 0 );

	trotProgramFree( &program );

	/* **************************************** */
	/* success! */
	printf( "SUCCESS!\n\n" );

	return 0;


	/* CLEANUP */
	cleanup:

	printf( "FAILED AT %d\n\n", rc );

	return rc;
}

/******************************************************************************/
static int getArgValue( int argc, char **argv, char *key, char **value )
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
				fprintf( stderr, "ERROR: Command line argument \"%s\" had no value.\n", key );
				return -1;
			}

			(*value) = argv[ i ];
			return 0;
		}

		i += 1;
	}

	return -2;
}

/******************************************************************************/
/*!
	\brief The log function. Sends log messages to stderr.
	\param[in] library 
	\param[in] file
	\param[in] line
	\param[in] rc
	\param[in] a
	\param[in] b
	\param[in] c
	\return void
*/
#ifdef TROT_ENABLE_LOGGING
void trotHookLog( s32 library, s32 file, s32 line, TROT_INT rc, TROT_INT a, TROT_INT b, TROT_INT c )
{
	if ( errno == 0 )
	{
		fprintf( stderr, "%d %d %5d - %2d %s - - %d %d %d\n",
			library, file, line,
			rc, ( rc == 0 ? "" : trotRCToString( rc ) ),
			a, b, c
		);
	}
	else
	{
		fprintf( stderr, "%d %d %5d - %2d %s - %d %s - %d %d %d\n",
			library, file, line,
			rc, ( rc == 0 ? "" : trotRCToString( rc ) ),
			errno, strerror( errno ),
			a, b, c
		);
	}
	fflush( stderr );
}
#endif

