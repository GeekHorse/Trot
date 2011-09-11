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
#ifndef trotCommon_H
#define trotCommon_H

/******************************************************************************/
#include <stdio.h> /* for printf in ERR */
#include <stdlib.h> /* for NULL */

/******************************************************************************/
#define BE_PARANOID 0

/******************************************************************************/
#if ( PRINT_ERR == 1 )
#define ERR_IF( cond, error_to_return ) if ( (cond) ) { printf( "ERR: %s %d\n", __FILE__, __LINE__ ); fflush( stdout ); rc = error_to_return; goto cleanup; }
#define ERR_IF_PASSTHROUGH if ( rc != 0 ) { printf( "ERR: %s %d\n", __FILE__, __LINE__ ); fflush( stdout ); goto cleanup; }
#else
#define ERR_IF( cond, error_to_return ) if ( (cond) ) { rc = error_to_return; goto cleanup; }
#define ERR_IF_PASSTHROUGH if ( rc != 0 ) { goto cleanup; }
#endif

#if ( TEST_PRECOND == 1 )
#define PRECOND_ERR_IF( cond ) if ( (cond) ) { return TROT_LIST_ERROR_PRECOND; }
#else
#define PRECOND_ERR_IF( cond )
#endif

#if ( BE_PARANOID == 1 )
#define ERR_IF_PARANOID( cond ) if ( (cond) ) { printf( "PARANOID ERROR! %s %d\n", __FILE__, __LINE__ ); fflush( stdout ); exit(-1); }
#else
#define ERR_IF_PARANOID( cond )
#endif

/******************************************************************************/
/* only for debugging */
#define dline printf( "dline:" __FILE__ ":%d\n", __LINE__ ); fflush( stdout );

/******************************************************************************/
#endif

