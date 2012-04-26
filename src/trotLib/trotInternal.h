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
#ifndef trotInternal_H
#define trotInternal_H

/******************************************************************************/
#include <stdio.h> /* for printf in ERR */
#include <stdlib.h> /* for NULL */

/******************************************************************************/
/* this should be a 1-byte unsigned type that can only hold values 0-255 */
#define BYTE_TYPE unsigned char
/* TODO: so far this is only used in testing code. where should it belong? */

/******************************************************************************/
#define NODE_SIZE 64

/******************************************************************************/
#define NODE_KIND_HEAD_OR_TAIL 0
#define NODE_KIND_INT 1
#define NODE_KIND_LIST 2

/******************************************************************************/
#define REF_LIST_NODE_SIZE 16

/******************************************************************************/
/* TODO: remove this after we're done all our tests */
#define BE_PARANOID 1

/******************************************************************************/
#if ( PRINT_ERR == 1 )
#define ERR_IF( cond, error_to_return ) if ( (cond) ) { printf( "ERR: %s %d\n", __FILE__, __LINE__ ); fflush( stdout ); rc = error_to_return; goto cleanup; }
#define ERR_IF_PASSTHROUGH if ( rc != 0 ) { printf( "ERR: %s %d\n", __FILE__, __LINE__ ); fflush( stdout ); goto cleanup; }
#else
#define ERR_IF( cond, error_to_return ) if ( (cond) ) { rc = error_to_return; goto cleanup; }
#define ERR_IF_PASSTHROUGH if ( rc != 0 ) { goto cleanup; }
#endif

#if ( TEST_PRECOND == 1 )
#define PRECOND_ERR_IF( cond ) if ( (cond) ) { printf( "PRECOND_ERR: %s %d\n", __FILE__, __LINE__ ); fflush( stdout ); return TROT_LIST_ERROR_PRECOND; }
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
#define TROT_MALLOC( POINTER, POINTER_TYPE, SIZE ) \
	POINTER = ( POINTER_TYPE * ) trotMalloc( sizeof( POINTER_TYPE ) * SIZE ); \
	ERR_IF( POINTER == NULL, TROT_LIST_ERROR_MEMORY_ALLOCATION_FAILED );

/******************************************************************************/
#define TROT_CALLOC( POINTER, POINTER_TYPE, SIZE ) \
	POINTER = ( POINTER_TYPE ** ) trotCalloc( SIZE, sizeof( POINTER_TYPE * ) ); \
	ERR_IF( POINTER == NULL, TROT_LIST_ERROR_MEMORY_ALLOCATION_FAILED );

/******************************************************************************/
typedef struct trotListNode_STRUCT trotListNode;
typedef struct trotList_STRUCT trotList;
typedef struct trotListRefListNode_STRUCT trotListRefListNode;

/*! Data in a trotList is stored in a linked list of trotListNodes. */
struct trotListNode_STRUCT
{
	/*! 'kind' is either NODE_KIND_HEAD_OR_TAIL, NODE_KIND_INT, or
	NODE_KIND_LIST. */
	int kind;
	/*! count is how many INT_TYPEs or trotListRefs are in this node. */
	int count;
	/*! if kind is NODE_KIND_INT, then n will point to an array of size
	NODE_SIZE of type INT_TYPE, else n will be NULL. */
	INT_TYPE *n;
	/*! if kind is NODE_KIND_LIST, then l will point to an array of size
	NODE_SIZE of type trotListRef*, else l will be NULL. */
	trotListRef **l;

	/*! prev points to previous node in the linked list, or same node if
	this is the head of the list. */
	struct trotListNode_STRUCT *prev;
	/*! next points to the next node in the linked list, or same node if
	this is the tail of the list. */
	struct trotListNode_STRUCT *next;
};

/*! trotList is the main data structure in Trot. */
struct trotList_STRUCT
{
	/*! Flag that says whether this list is still reachable or not. If not
	reachable, then this list can be freed */
	int reachable;
	/*! Flag for 'is list reachable' so we don't get into an infinite
	    loop */
	int flagVisited;
	/*! Pointer to "previous" list. Used when we're seeing if a list is
	    reachable */
	trotList *previous;
	/*! Pointer to "nextToFree" list. Only set when this list is no longer
	    reachable. We use this to keep a linked list of lists that need to
	    be freed. */
	trotList *nextToFree;

	/*! Pointer to "parent" when we're encoding a list */
	trotList *encodingParent;
	/*! The child number this list is of it's parent, used for managing
	    twins when encoding */
	INT_TYPE encodingChildNumber;

	/*! Tag. Which "type" or "kind" of list this is. */
	int tag;
	/*! How many children are in the list */
	int childrenCount;
	/*! Pointer to the head of the linked list that contains the refs that
	point to this list. Used for checking whether this list is still
	reachable or not. */
	trotListRefListNode *refListHead;
	/*! Pointer to the tail of the linked list that contains the refs that
	point to this list. Used for checking whether this list is still
	reachable or not. */
	trotListRefListNode *refListTail;
	/*! Pointer to the head of the linked list that contains the actual data
	in the list. */
	trotListNode *head;
	/*! Pointer to the tail of the linked list that contains the actual data
	in the list. */
	trotListNode *tail;
};

/*! trotListRef is a reference to a trotList */
struct trotListRef_STRUCT
{
	/*! The list that this ref is inside of. */
	trotList *lParent;
	/*! The list that this ref points to. */
	trotList *lPointsTo;
};

/*! Structure for holding a linked list of references. Used in trotList to keep
track of which references points to the trotList. */
struct trotListRefListNode_STRUCT
{
	/*! How many references are in this node */
	int count;
	/*! r will be NULL if this is the head or tail of the linked list.
	else this will be an array of size REF_LIST_NODE_SIZE of type
	trotListRef */
	trotListRef **r;
	/*! points to the next node in the linked list, or to itself if this is
	the tail. */
	trotListRefListNode *next;
	/*! points to the prev node in the linked list, or to itself if this is
	the head. */
	trotListRefListNode *prev;
};

/******************************************************************************/
typedef struct trotStack_STRUCT trotStack;
typedef struct trotStackNode_STRUCT trotStackNode;

/*! Holds a stack of trotStackNodes.
    Only used during Compare so we don't get into an infinite loop */
struct trotStack_STRUCT
{
	/*! head of our stack */
	trotStackNode *head;
	/*! tail of our stack */
	trotStackNode *tail;
};

/*! Holds state for our progress comparing two lists */
struct trotStackNode_STRUCT
{
	/*! list1 */
	trotList *l1;
	/*! current node in list1 */
	trotListNode *l1Node;
	/*! current item in l1Node */
	INT_TYPE l1Count;

	/*! list2 */
	trotList *l2;
	/*! current node in list2 */
	trotListNode *l2Node;
	/*! current item in l2Node */
	INT_TYPE l2Count;

	/*! current index */
	INT_TYPE index;

	/*! previous trotStackNode */
	trotStackNode *prev;
	/*! next trotStackNode */
	trotStackNode *next;
};

/******************************************************************************/
/* trotListPrimary.c */
TROT_RC trotListGetKind( trotList *l, INT_TYPE index, int *kind );
TROT_RC trotListGetInt( trotList *l, INT_TYPE index, INT_TYPE *n );

TROT_RC trotListNodeSplit( trotListNode *n, int keepInLeft );

TROT_RC newIntNode( trotListNode **n_A );
TROT_RC newListNode( trotListNode **n_A );

/******************************************************************************/
/* trotStack.c */
TROT_RC trotStackInit( trotStack **stack );
void trotStackFree( trotStack **stack );

TROT_RC trotStackPush( trotStack *stack, trotList *l1, trotList *l2 );
TROT_RC trotStackPop( trotStack *stack, int *empty );
TROT_RC trotStackIncrementTopIndex( trotStack *stack );

/******************************************************************************/
/* trotTokenize.c */
TROT_RC trotTokenize( trotListRef *lrCharacters, trotListRef **lrTokenList_A );
TROT_RC trotCreateToken( INT_TYPE line, INT_TYPE column, INT_TYPE tokenType, trotListRef **lrToken_A );
TROT_RC _trotWordToNumber( trotListRef *lrWord, int *isNumber, INT_TYPE *number );

/******************************************************************************/
/* For trotDecodingEncoding and related tests */
typedef enum
{
	TOKEN_L_BRACKET = 1,
	TOKEN_R_BRACKET = 2,
	TOKEN_L_BRACE = 3,
	TOKEN_R_BRACE = 4,
	TOKEN_L_PARENTHESIS = 5,
	TOKEN_R_PARENTHESIS = 6,
	TOKEN_WORD = 7,
	TOKEN_NUMBER = 8,
	TOKEN_STRING = 9,
	TOKEN_TWIN = 10,
	TOKEN_INCLUDE = 11
} TOKEN_TYPE;

typedef enum
{
	TOKEN_INDEX_LINE = 1,
	TOKEN_INDEX_COLUMN = 2,
	TOKEN_INDEX_TYPE = 3,
	TOKEN_INDEX_VALUE = 4,
	TOKEN_INDEX_NAME = 5,
	TOKEN_INDEX_ENUMS = 6,
	TOKEN_INDEX_FINALLIST = 7
} TOKEN_INFO;

/******************************************************************************/
#endif

