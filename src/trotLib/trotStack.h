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
#ifndef trotStack_H
#define trotStack_H

/******************************************************************************/
#include "trotList.h"

/******************************************************************************/
typedef struct trotStack_STRUCT trotStack;
typedef struct trotStackNode_STRUCT trotStackNode;

/*! Holds a stack of trotList pointers. Used during memory management to try to
follow references "up" to see if a list is reachable. We use the stack to make
sure we don't get into an infinite loop. */
struct trotStack_STRUCT
{
	/*! head of our stack */
	trotStackNode *head;
	/*! tail of our stack */
	trotStackNode *tail;
};

/*! TODO */
struct trotStackNode_STRUCT
{
	trotList *l1;
	trotList *l2;
	INT_TYPE n;

	trotStackNode *prev;
	trotStackNode *next;
};

/******************************************************************************/
int trotStackInit( trotStack **stack );
void trotStackFree( trotStack **stack );

int trotStackPush( trotStack *stack, trotList *l1, trotList *l2 );
int trotStackPop( trotStack *stack, int *empty );
int trotStackIncrementTopN( trotStack *stack );
int trotStackGet( trotStack *stack, trotList **l1, trotList **l2, INT_TYPE *n );

/******************************************************************************/
#endif

