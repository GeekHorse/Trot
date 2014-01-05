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
/*!
	\file
	Contains hook function pointers for:
	- Malloc, Calloc, Free

	Used if we're embedded in an app that uses it's own
	memory functions. We can easily "plug into" their memory management
	system.

	Used in our unit tests for testing malloc/calloc failures.

*/

/******************************************************************************/
#include <stdlib.h> /* for malloc, calloc, free */

/******************************************************************************/
/*! This is the function that the library uses for 'malloc'. Used for unit
    testing failed mallocs and in case the user of the library has their own
    memory management routines. */
void *(*trotHookMalloc)( size_t size ) = malloc;
/*! This is the function that the library uses for 'calloc'. Used for unit
    testing failed callocs and in case the user of the library has their own
    memory management routines. */
void *(*trotHookCalloc)( size_t nmemb, size_t size ) = calloc;
/*! This is the function that the library uses for 'free'. Used in case the
    user of the library has their own memory management routines. */
void (*trotHookFree)( void *ptr ) = free;

