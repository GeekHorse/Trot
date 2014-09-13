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
#ifndef trot_H
#define trot_H

/******************************************************************************/
#define TROT_NAME "Trot"

#define TROT_COPYRIGHT "Copyright (C) 2010-2014 Jeremiah Martell"

#define TROT_VERSION_STRING "0.3.00-wip"
#define TROT_VERSION 3000
#define TROT_VERSION_MAJOR       ( TROT_VERSION / 10000 )
#define TROT_VERSION_MINOR       ( ( TROT_VERSION / 1000 ) % 10 )
#define TROT_VERSION_SUBMINOR    ( ( TROT_VERSION / 10 ) % 100 )
#define TROT_VERSION_FINAL       ( TROT_VERSION % 10 )

/******************************************************************************/
/* This defines s32 as a signed 32-bit integer */
#ifndef s32
#define s32 signed int
#endif
/* This defines u8 as an unsigned 8-bit integer */
#ifndef u8
#define u8  unsigned char
#endif

/******************************************************************************/
#define TROT_RC s32

/******************************************************************************/
#define TROT_LIBRARY_NUMBER 2000

/******************************************************************************/
/* standard rc values */
#define TROT_RC_SUCCESS 0

#define TROT_RC_ERROR_PRECOND                  1
#define TROT_RC_ERROR_MEMORY_ALLOCATION_FAILED 2
#define TROT_RC_ERROR_STANDARD_LIBRARY_ERROR   3
#define TROT_RC_ERROR_FAILURE_POINT            4

/* This must be kept in sync with the above defines */
#define TROT_RC_STANDARD_ERRORS_MAX            4

/* Trot specific rc values */
#define TROT_RC_ERROR_BAD_INDEX      2001
#define TROT_RC_ERROR_WRONG_KIND     2002
#define TROT_RC_ERROR_LIST_OVERFLOW  2003
#define TROT_RC_ERROR_INVALID_OP     2004
#define TROT_RC_ERROR_BAD_TYPE       2005
#define TROT_RC_ERROR_DIVIDE_BY_ZERO 2006
#define TROT_RC_ERROR_UNICODE        2007
#define TROT_RC_ERROR_DECODE         2008
#define TROT_RC_ERROR_MEM_LIMIT      2009

/* These must be kept in sync with the above defines */
#define TROT_RC_TROT_ERRORS_MIN     2001
#define TROT_RC_TROT_ERRORS_MAX     2009

/******************************************************************************/
#define TROT_KIND_INT 1
#define TROT_KIND_LIST 2

/******************************************************************************/
#define TROT_TYPE_DATA 0
#define TROT_TYPE_CODE 1
/* FUTURE: we'll eventually have CODE, FUNCTION, VM_IMAGE, FUNCTION_STACK, ERROR, etc */

/******************************************************************************/
#define TROT_INT_SIZE 4 /* TODO: do we even need this? We may want to just
hardcode TROT_INT to s32 since we're going to be using that everywhere */
/* TODO: else we'd have to use TROT_INT everywhere else */

#if ( TROT_INT_SIZE == 4 )
#define TROT_INT signed int
#define TROT_INT_MAX         2147483647
#define TROT_INT_MAX_STRING "2147483647"
#define TROT_INT_MAX_STRING_LENGTH 10
#define TROT_INT_MIN_STRING "-2147483648"
#define TROT_INT_MIN_STRING_LENGTH 11
#else
#error NEED TO DEFINE TROT_INT FOR TROT_INT_SIZE
#endif

/******************************************************************************/
typedef struct TrotProgram_STRUCT TrotProgram;
typedef struct TrotData_STRUCT TrotData;

/******************************************************************************/
TROT_RC trotProgramLoad( TROT_INT memoryLimit, const char *savedProgram, TrotProgram **program_A );

TROT_RC trotProgramMemoryGetUsed( TrotProgram *program, TROT_INT *used );
TROT_RC trotProgramMemoryGetLimit( TrotProgram *program, TROT_INT *limit );
TROT_RC trotProgramMemorySetLimit( TrotProgram *program, TROT_INT limit );

TROT_RC trotProgramCyclesGet( TrotProgram *program, TROT_INT *cycles );
TROT_RC trotProgramCyclesSet( TrotProgram *program, TROT_INT cycles );
TROT_RC trotProgramCyclesModify( TrotProgram *program, TROT_INT cycles );

TROT_RC trotProgramStackAppendInt( TrotProgram *program, TROT_INT n );
TROT_RC trotProgramStackAppendError( TrotProgram *program, TROT_INT tag );
TROT_RC trotProgramStackAppendString( TrotProgram *program, const char *string );
TROT_RC trotProgramStackPack( TrotProgram *program, TROT_INT n );

TROT_RC trotProgramRun( TrotProgram *program, TROT_INT *signalCode, TROT_INT *signalValue, TrotData **output_A );

TROT_RC trotProgramSave( TrotProgram **program_F, char **savedProgram_A );
TROT_RC trotProgramFree( TrotProgram **program_F );

const char *trotRCToString( TROT_RC rc );

/******************************************************************************/
#endif

