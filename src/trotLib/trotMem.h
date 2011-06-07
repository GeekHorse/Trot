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
#ifndef trotMem_H
#define trotMem_H

/******************************************************************************/
#include <stdlib.h> /* for size_t for gkCalloc */

/******************************************************************************/
extern void *(*gkCalloc)( size_t nmemb, size_t size );
extern void *(*gkMalloc)( size_t size );
extern void (*gkFree)( void *ptr );

/******************************************************************************/
#define TROT_MALLOC( POINTER, POINTER_TYPE, SIZE ) \
	POINTER = ( POINTER_TYPE * ) gkMalloc( sizeof( POINTER_TYPE ) * SIZE ); \
	ERR_IF( POINTER == NULL, GK_LIST_ERROR_MEMORY_ALLOCATION_FAILED );

/******************************************************************************/
#define TROT_CALLOC( POINTER, POINTER_TYPE, SIZE ) \
	POINTER = ( POINTER_TYPE ** ) gkCalloc( SIZE, sizeof( POINTER_TYPE * ) ); \
	ERR_IF( POINTER == NULL, GK_LIST_ERROR_MEMORY_ALLOCATION_FAILED );

/******************************************************************************/
#endif

