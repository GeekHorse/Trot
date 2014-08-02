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
#ifndef trotInternal_H
#define trotInternal_H

/******************************************************************************/
#include <stdio.h> /* for printf */
#include <stdlib.h> /* for NULL */

#include "trot.h"

/******************************************************************************/
#ifndef TROT_NODE_SIZE
#define TROT_NODE_SIZE 16
#endif

/******************************************************************************/
/* NOTE: Need to keep this in sync with TROT_KIND */
#define NODE_KIND_HEAD_OR_TAIL 0
#define NODE_KIND_INT 1
#define NODE_KIND_LIST 2

/******************************************************************************/
#define REF_LIST_NODE_SIZE 16

/******************************************************************************/
#define ERR_IF( cond, error_to_return ) \
	if ( (cond) ) \
	{ \
		TROT_HOOK_LOG( TROT_LIBRARY_NUMBER, TROT_FILE_NUMBER, __LINE__, error_to_return, 0, 0, 0 ); \
		rc = error_to_return; \
		goto cleanup; \
	}

#define ERR_IF_1( cond, error_to_return, a ) \
	if ( (cond) ) \
	{ \
		TROT_HOOK_LOG( TROT_LIBRARY_NUMBER, TROT_FILE_NUMBER, __LINE__, error_to_return, a, 0, 0 ); \
		rc = error_to_return; \
		goto cleanup; \
	}

#define ERR_IF_2( cond, error_to_return, a, b ) \
	if ( (cond) ) \
	{ \
		TROT_HOOK_LOG( TROT_LIBRARY_NUMBER, TROT_FILE_NUMBER, __LINE__, error_to_return, a, b, 0 ); \
		rc = error_to_return; \
		goto cleanup; \
	}

#define ERR_IF_3( cond, error_to_return, a, b, c ) \
	if ( (cond) ) \
	{ \
		TROT_HOOK_LOG( TROT_LIBRARY_NUMBER, TROT_FILE_NUMBER, __LINE__, error_to_return, a, b, c ); \
		rc = error_to_return; \
		goto cleanup; \
	}

#define ERR_IF_PASSTHROUGH \
	ERR_IF( rc != TROT_RC_SUCCESS, rc );

/******************************************************************************/
#ifdef BE_PARANOID
#define PARANOID_ERR_IF( cond ) if ( (cond) ) { fprintf( stderr, "PARANOID ERROR! %s %d\n", __FILE__, __LINE__ ); fflush( stderr ); exit(-1); }
#else
#define PARANOID_ERR_IF( cond )
#endif

/******************************************************************************/
/* only for debugging */
#define dline printf( "dline:" __FILE__ ":%d\n", __LINE__ ); fflush( stdout );

/******************************************************************************/
#define TROT_MALLOC( POINTER, POINTER_TYPE, SIZE ) \
	POINTER = ( POINTER_TYPE * ) TROT_HOOK_MALLOC( sizeof( POINTER_TYPE ) * (SIZE) ); \
	ERR_IF( POINTER == NULL, TROT_RC_ERROR_MEMORY_ALLOCATION_FAILED );

/******************************************************************************/
#define TROT_CALLOC( POINTER, POINTER_TYPE, SIZE ) \
	POINTER = ( POINTER_TYPE * ) TROT_HOOK_CALLOC( SIZE, sizeof( POINTER_TYPE ) ); \
	ERR_IF( POINTER == NULL, TROT_RC_ERROR_MEMORY_ALLOCATION_FAILED );

/******************************************************************************/
/* FUTURE: these may change, or how we execute them may change */
typedef enum
{
	TROT_OP_ADD = 1,
	TROT_OP_SUB = 2,
	TROT_OP_MUL = 3,
	TROT_OP_DIV = 4,
	TROT_OP_MOD = 5,

	TROT_OP_LESS_THAN = 6,
	TROT_OP_GREATER_THAN = 7,
	TROT_OP_EQUALS = 8,

	TROT_OP_LOGICAL_AND = 9,
	TROT_OP_LOGICAL_OR =  10,
	TROT_OP_LOGICAL_NOT = 11,

	TROT_OP_NEG = 12,

	TROT_OP_PUSH_INT = 13,
	TROT_OP_PUSH_LIST = 14,

	TROT_OP_CALL = 15,
	TROT_OP_CHANGE = 16,
	TROT_OP_RETURN = 17,
	TROT_OP_YIELD = 18,

	TROT_OP_LOAD_VAR = 19,
	TROT_OP_SAVE_VAR = 20
} TROT_OP; 
#define TROT_OP_MIN 1
#define TROT_OP_MAX 20

/******************************************************************************/
typedef struct TrotListNode_STRUCT TrotListNode;
typedef struct TrotListActual_STRUCT TrotListActual;
typedef struct TrotListRefListNode_STRUCT TrotListRefListNode;

/*! Data in a TrotList is stored in a linked list of trotListNodes. */
struct TrotListNode_STRUCT
{
	/*! 'kind' is either NODE_KIND_HEAD_OR_TAIL, NODE_KIND_INT, or
	NODE_KIND_LIST. */
/* FUTURE: this can go away, since we know what kind it is based on the state of the l and n pointers */
	TROT_INT kind;
	/*! count is how many TROT_INTs or trotListRefs are in this node. */
/* FUTURE: this could be made smaller by using a u8, since we're not going to have more than 256 sized nodes? */
	TROT_INT count;
	/*! if kind is NODE_KIND_INT, then n will point to an array of size
	TROT_NODE_SIZE of type TROT_INT, else n will be NULL. */
	TROT_INT *n;
	/*! if kind is NODE_KIND_LIST, then l will point to an array of size
	TROT_NODE_SIZE of type trotList*, else l will be NULL. */
	TrotList **l;

	/*! prev points to previous node in the linked list, or same node if
	this is the head of the list. */
	struct TrotListNode_STRUCT *prev;
	/*! next points to the next node in the linked list, or same node if
	this is the tail of the list. */
	struct TrotListNode_STRUCT *next;
};

/*! trotListActual is the main data structure in Trot. */
struct TrotListActual_STRUCT
{
	/*! Flag that says whether this list is still reachable or not. If not
	reachable, then this list can be freed */
	TROT_INT reachable;
	/*! Flag for 'is list reachable' so we don't get into an infinite
	    loop */
	TROT_INT flagVisited;
	/*! Pointer to "previous" list. Used when we're seeing if a list is
	    reachable */
	TrotListActual *previous;
	/*! Pointer to "nextToFree" list. Only set when this list is no longer
	    reachable. We use this to keep a linked list of lists that need to
	    be freed. */
/* FUTURE: we could save space by combining these 3 trotlistactual pointers,
since it doesnt seem they're ever used at the same time */
	TrotListActual *nextToFree;

/* FUTURE: we could change our encoding to only need 1 value, not parent and number
if it kept track of each lists reference number */
	/*! Pointer to "parent" when we're encoding a list */
	TrotListActual *encodingParent;
	/*! The child number this list is of it's parent, used for managing
	    twins when encoding */
	TROT_INT encodingChildNumber;

	/*! Tag. Which "type" of list this is. */
	TROT_INT tag;
	/*! userTag. Allows user to tag this list as a user-defined type */
	TROT_INT userTag;
	/*! How many children are in the list */
	TROT_INT childrenCount;
	/*! Pointer to the head of the linked list that contains the refs that
	point to this list. Used for checking whether this list is still
	reachable or not. */
	TrotListRefListNode *refListHead;
	/*! Pointer to the tail of the linked list that contains the refs that
	point to this list. Used for checking whether this list is still
	reachable or not. */
/* FUTURE: do we need a tail pointer here? */
	TrotListRefListNode *refListTail;
	/*! Pointer to the head of the linked list that contains the actual data
	in the list. */
	TrotListNode *head;
	/*! Pointer to the tail of the linked list that contains the actual data
	in the list. */
	TrotListNode *tail;
};

/*! TrotList is a reference to a TrotListActual */
struct TrotList_STRUCT
{
	/*! The list that this ref is inside of. */
	TrotListActual *laParent;
	/*! The list that this ref points to. */
	TrotListActual *laPointsTo;
};

/*! Structure for holding a linked list of references. Used in TrotList to keep
track of which references points to the trotList. */
/* FUTURE: does this need to be double-linked? */
struct TrotListRefListNode_STRUCT
{
	/*! How many references are in this node */
	TROT_INT count;
	/*! l will be NULL if this is the head or tail of the linked list.
	else this will be an array of size REF_LIST_NODE_SIZE of type
	TrotList */
	TrotList **l;
	/*! points to the next node in the linked list, or to itself if this is
	the tail. */
	TrotListRefListNode *next;
	/*! points to the prev node in the linked list, or to itself if this is
	the head. */
	TrotListRefListNode *prev;
};

/******************************************************************************/
typedef struct TrotStack_STRUCT TrotStack;
typedef struct TrotStackNode_STRUCT TrotStackNode;

/*! Holds a stack of trotStackNodes.
    Only used during Compare so we don't get into an infinite loop */
struct TrotStack_STRUCT
{
	/*! head of our stack */
	TrotStackNode *head;
	/*! tail of our stack */
	TrotStackNode *tail;
};

/*! Holds state for our progress comparing two lists */
struct TrotStackNode_STRUCT
{
	/*! list1 */
	TrotListActual *la1;
	/*! current node in list1 */
	TrotListNode *la1Node;
	/*! current item in l1Node */
	TROT_INT la1Count;

	/*! list2 */
	TrotListActual *la2;
	/*! current node in list2 */
	TrotListNode *la2Node;
	/*! current item in l2Node */
	TROT_INT la2Count;

	/*! current index */
	TROT_INT index;

	/*! previous trotStackNode */
	TrotStackNode *prev;
	/*! next trotStackNode */
	TrotStackNode *next;
};

/******************************************************************************/
/* trotListPrimary.c */
TROT_RC trotListNodeSplit( TrotListNode *n, TROT_INT keepInLeft );

TROT_RC newIntNode( TrotListNode **n_A );
TROT_RC newListNode( TrotListNode **n_A );

/******************************************************************************/
/* trotStack.c */
TROT_RC trotStackInit( TrotStack **stack );
void trotStackFree( TrotStack **stack );

TROT_RC trotStackPush( TrotStack *stack, TrotListActual *la1, TrotListActual *la2 );
TROT_RC trotStackPop( TrotStack *stack, TROT_INT *empty );
TROT_RC trotStackIncrementTopIndex( TrotStack *stack );

/******************************************************************************/
/* trotListInt.c */
TROT_RC trotListIntOperand( TrotList *l, TROT_OP op );
TROT_RC trotListIntOperandValue( TrotList *l, TROT_OP op, TROT_INT value );

/******************************************************************************/
#ifdef DEBUG

	extern void *trotHookMalloc( size_t size );
	#ifndef TROT_HOOK_MALLOC
	#define TROT_HOOK_MALLOC trotHookMalloc
	#endif

	extern void *trotHookCalloc( size_t nmemb, size_t size );
	#ifndef TROT_HOOK_CALLOC
	#define TROT_HOOK_CALLOC trotHookCalloc
	#endif

	extern void trotHookFree( void *ptr );
	#ifndef TROT_HOOK_FREE
	#define TROT_HOOK_FREE trotHookFree
	#endif

#else

	#ifndef TROT_HOOK_MALLOC
	#define TROT_HOOK_MALLOC malloc
	#endif

	#ifndef TROT_HOOK_CALLOC
	#define TROT_HOOK_CALLOC calloc
	#endif

	#ifndef TROT_HOOK_FREE
	#define TROT_HOOK_FREE free
	#endif

#endif

/******************************************************************************/
#ifdef TROT_ENABLE_LOGGING

	extern void trotHookLog( s32 library, s32 file, s32 line, TROT_INT rc, TROT_INT a, TROT_INT b, TROT_INT c );
	#ifndef TROT_HOOK_LOG
	#define TROT_HOOK_LOG( p, f, l, r, a, b, c ) trotHookLog( p, f, l, r, a, b, c )
	#endif

#else

	#ifndef TROT_HOOK_LOG
	#define TROT_HOOK_LOG( p, f, l, r, a, b, c )
	#endif

#endif

/******************************************************************************/
#endif

