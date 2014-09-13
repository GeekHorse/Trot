/*
Copyright (C) 2014 Jeremiah Martell
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    - Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    - Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    - Neither the name of Jeremiah Martell nor the name of GeekHorse nor the
      name of Trot nor the names of its contributors may be used to endorse
      or promote products derived from this software without specific prior
      written permission.

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
#include <string.h> /* strcmp */

#include "trot.h"
#include "trotInternal.h"

#include "trotTestCommon.h"

/******************************************************************************/
int testMisc( TrotProgram *program )
{
	/* DATA */
	int rc = 0;

	const char *string = NULL;


	/* CODE */
	/* **************************************** */
	printf( "Testing misc...\n" ); fflush( stdout );

	(void)program;

	/* make sure we haven't forgotten any text for our RCs */
	string = trotRCToString( TROT_RC_SUCCESS );
	TEST_ERR_IF( strcmp( string, "Success" ) != 0 );

	string = trotRCToString( TROT_RC_STANDARD_ERRORS_MAX );
	TEST_ERR_IF( strcmp( string, "Unknown Error" ) == 0 );

	string = trotRCToString( TROT_RC_STANDARD_ERRORS_MAX + 1 );
	TEST_ERR_IF( strcmp( string, "Unknown Error" ) != 0 );

	string = trotRCToString( TROT_RC_TROT_ERRORS_MIN - 1 );
	TEST_ERR_IF( strcmp( string, "Unknown Error" ) != 0 );

	string = trotRCToString( TROT_RC_TROT_ERRORS_MIN );
	TEST_ERR_IF( strcmp( string, "Unknown Error" ) == 0 );

	string = trotRCToString( TROT_RC_TROT_ERRORS_MAX );
	TEST_ERR_IF( strcmp( string, "Unknown Error" ) == 0 );

	string = trotRCToString( TROT_RC_TROT_ERRORS_MAX + 1 );
	TEST_ERR_IF( strcmp( string, "Unknown Error" ) != 0 );

	string = trotRCToString( -1 );
	TEST_ERR_IF( strcmp( string, "Unknown Error" ) != 0 );

	string = trotRCToString( -2 );
	TEST_ERR_IF( strcmp( string, "Unknown Error" ) != 0 );

	printf( "\n" );


	/* CLEANUP */
	cleanup:

	return rc;
}

