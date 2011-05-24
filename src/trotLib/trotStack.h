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
#ifndef gkStack_H
#define gkStack_H

/******************************************************************************/
#include "trotList.h"

/******************************************************************************/
#define LIST_STACK_NODE_SIZE 64

/******************************************************************************/
typedef struct gkStack_STRUCT gkStack;

/*! Holds a stack of gkList pointers. Used during memory management to try to
follow references "up" to see if a list is reachable. We use the stack to make
sure we don't get into an infinite loop. */
struct gkStack_STRUCT
{
	/*! how many pointers are in this node. */
	int count;
	/*! an array of size LIST_STACK_NODE_SIZE of type gkList* */
	gkList **l;
	/*! points to the next node in the stack, or NULL if this is the last
	node in the stack. */
	gkStack *next;
};

/******************************************************************************/
inline int gkStackInit( gkStack **stack );
inline int gkStackAddList( gkStack *stack, gkList *l );
inline int gkStackFree( gkStack **stack );

/******************************************************************************/
#endif

