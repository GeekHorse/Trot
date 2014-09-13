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
#include <stdio.h> /* for printf, fprintf, fflush */
#include <stdlib.h> /* for NULL */

#include "trot.h"

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
#define TROT_MALLOC( POINTER, SIZE ) \
	ERR_IF( ( program->memoryLimit - ((TROT_INT)( sizeof( * (POINTER) ) * (SIZE) )) ) < program->memoryUsed, \
	        TROT_RC_ERROR_MEM_LIMIT ); \
	POINTER = TROT_HOOK_MALLOC( sizeof( * (POINTER) ) * (SIZE) ); \
	ERR_IF( (POINTER) == NULL, TROT_RC_ERROR_MEMORY_ALLOCATION_FAILED ); \
	program->memoryUsed += ( sizeof( * (POINTER) ) * (SIZE) );

/******************************************************************************/
#define TROT_CALLOC( POINTER, SIZE ) \
	ERR_IF( ( program->memoryLimit - ((TROT_INT)( sizeof( * (POINTER) ) * (SIZE) )) ) < program->memoryUsed, \
	        TROT_RC_ERROR_MEM_LIMIT ); \
	POINTER = TROT_HOOK_CALLOC( SIZE, sizeof( * (POINTER) ) ); \
	ERR_IF( (POINTER) == NULL, TROT_RC_ERROR_MEMORY_ALLOCATION_FAILED ); \
	program->memoryUsed += ( sizeof( * (POINTER) ) * (SIZE) );

/******************************************************************************/
#define TROT_FREE( POINTER, SIZE ) \
	if ( POINTER != NULL ) \
	{ \
		program->memoryUsed -= ( sizeof( * (POINTER) ) * (SIZE) ); \
		TROT_HOOK_FREE( POINTER ); \
	}
/* TODO: TROT_FREE should set pointer to NULL? */

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
#ifndef TROT_MAX_CHILDREN
#define TROT_MAX_CHILDREN TROT_INT_MAX
#endif

/******************************************************************************/
typedef struct TrotList_STRUCT TrotList;
typedef struct TrotListActual_STRUCT TrotListActual;
typedef struct TrotListNode_STRUCT TrotListNode;
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

/* FUTURE: we don't need a double linked list, change this to single */
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

	/*! Type. Which type of list this is. */
	TROT_INT type;
	/*! Tag. Allows user to tag this list */
	TROT_INT tag;
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
/*! Structure to hold a Trot program. */
struct TrotProgram_STRUCT
{
	/*! The memory limit for this program */
	TROT_INT memoryLimit;
	/*! The memory used for this program */
	TROT_INT memoryUsed;
	/*! The cycles allocated for this program */
	TROT_INT cycles;
	/*! The list of threads in this program */
	TrotList *lThreadList;
};

/******************************************************************************/
/* trotListPrimary.c */
TROT_RC trotListInit( TrotProgram *program, TrotList **l_A );
TROT_RC trotListTwin( TrotProgram *program, TrotList *l, TrotList **lTwin_A );
void trotListFree( TrotProgram *program, TrotList **l_F );

TROT_RC trotListRefCompare( TrotProgram *program, TrotList *l1, TrotList *l2, TROT_INT *isSame );

TROT_RC trotListGetCount( TrotProgram *program, TrotList *l, TROT_INT *count );

TROT_RC trotListGetKind( TrotProgram *program, TrotList *l, TROT_INT index, TROT_INT *kind );

TROT_RC trotListAppendInt( TrotProgram *program, TrotList *l, TROT_INT n );
TROT_RC trotListAppendList( TrotProgram *program, TrotList *l, TrotList *lToAppend );

TROT_RC trotListInsertInt( TrotProgram *program, TrotList *l, TROT_INT index, TROT_INT n );
TROT_RC trotListInsertList( TrotProgram *program, TrotList *l, TROT_INT index, TrotList *lToInsert );

TROT_RC trotListGetInt( TrotProgram *program, TrotList *l, TROT_INT index, TROT_INT *n );
TROT_RC trotListGetList( TrotProgram *program, TrotList *l, TROT_INT index, TrotList **lTwin_A );

TROT_RC trotListRemoveInt( TrotProgram *program, TrotList *l, TROT_INT index, TROT_INT *n );
TROT_RC trotListRemoveList( TrotProgram *program, TrotList *l, TROT_INT index, TrotList **lRemoved_A );
TROT_RC trotListRemove( TrotProgram *program, TrotList *l, TROT_INT index );

TROT_RC trotListReplaceWithInt( TrotProgram *program, TrotList *l, TROT_INT index, TROT_INT n );
TROT_RC trotListReplaceWithList( TrotProgram *program, TrotList *l, TROT_INT index, TrotList *lToInsert );

TROT_RC trotListGetType( TrotProgram *program, TrotList *l, TROT_INT *type );
TROT_RC trotListSetType( TrotProgram *program, TrotList *l, TROT_INT type );

TROT_RC trotListGetTag( TrotProgram *program, TrotList *l, TROT_INT *tag );
TROT_RC trotListSetTag( TrotProgram *program, TrotList *l, TROT_INT tag );

TROT_RC trotListNodeSplit( TrotProgram *program, TrotListNode *n, TROT_INT keepInLeft );

TROT_RC newIntNode( TrotProgram *program, TrotListNode *insertBeforeThis, TrotListNode **n_A );
TROT_RC newListNode( TrotProgram *program, TrotListNode *insertBeforeThis, TrotListNode **n_A );

/******************************************************************************/
/* trotListSecondary.c */
/* FUTURE: rename this so people know it's only 1 level copying? */
TROT_RC trotListCopy( TrotProgram *program, TrotList *l, TrotList **lCopy_A );

TROT_RC trotListEnlist( TrotProgram *program, TrotList *l, TROT_INT indexStart, TROT_INT indexEnd );
TROT_RC trotListDelist( TrotProgram *program, TrotList *l, TROT_INT index );

TROT_RC trotListCopySpan( TrotProgram *program, TrotList *l, TROT_INT indexStart, TROT_INT indexEnd, TrotList **lCopy_A );
TROT_RC trotListRemoveSpan( TrotProgram *program, TrotList *l, TROT_INT indexStart, TROT_INT indexEnd );

/******************************************************************************/
/* trotDecoding.c */
TROT_RC trotDecode( TrotProgram *program, TrotList *lCharacters, TrotList **lDecodedList_A );

/******************************************************************************/
/* trotDecoding.c */
TROT_RC trotEncode( TrotProgram *program, TrotList *listToEncode, TrotList **lCharacters_A );

/******************************************************************************/
#ifdef TROT_DEBUG

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

	extern void trotHookLog( s32 library, s32 file, s32 line, s32 rc, s32 a, s32 b, s32 c );
	#ifndef TROT_HOOK_LOG
	#define TROT_HOOK_LOG( p, f, l, r, a, b, c ) trotHookLog( p, f, l, r, a, b, c )
	#endif

#else

	#ifndef TROT_HOOK_LOG
	#define TROT_HOOK_LOG( p, f, l, r, a, b, c )
	#endif

#endif

/******************************************************************************/
#ifdef ENABLE_FAILURE_POINT

	extern int failure_point_count;
	#ifndef FAILURE_POINT
	#define FAILURE_POINT \
		failure_point_count -= 1; \
		if ( failure_point_count <= 0 ) \
		{ \
			return TROT_RC_ERROR_FAILURE_POINT; \
		}
	#endif

#else

	#define FAILURE_POINT

#endif

/******************************************************************************/
#endif

