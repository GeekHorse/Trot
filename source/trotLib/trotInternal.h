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
#include <stdio.h> /* for printf in ERR */
#include <stdlib.h> /* for NULL */

/******************************************************************************/
#define NODE_SIZE 16

/******************************************************************************/
/* NOTE: Need to keep this in sync with TROT_KIND */
#define NODE_KIND_HEAD_OR_TAIL 0
#define NODE_KIND_INT 1
#define NODE_KIND_LIST 2

/******************************************************************************/
#define REF_LIST_NODE_SIZE 16

/******************************************************************************/
#if ( PRINT_ERR == 1 )
#define ERR_IF( cond, error_to_return ) if ( (cond) ) { printf( "ERR: %s %d\n", __FILE__, __LINE__ ); fflush( stdout ); rc = error_to_return; goto cleanup; }
#define ERR_IF_PASSTHROUGH if ( rc != TROT_RC_SUCCESS ) { printf( "ERR: %s %d\n", __FILE__, __LINE__ ); fflush( stdout ); goto cleanup; }
#else
#define ERR_IF( cond, error_to_return ) if ( (cond) ) { rc = error_to_return; goto cleanup; }
#define ERR_IF_PASSTHROUGH if ( rc != TROT_RC_SUCCESS ) { goto cleanup; }
#endif

/******************************************************************************/
#ifndef TEST_PRECOND
#define TEST_PRECOND 1
#endif

#if ( TEST_PRECOND == 1 )

#if ( PRINT_ERR == 1 )
#define PRECOND_ERR_IF( cond ) if ( (cond) ) { printf( "PRECOND_ERR: %s %d\n", __FILE__, __LINE__ ); fflush( stdout ); return TROT_RC_ERROR_PRECOND; }
#else
#define PRECOND_ERR_IF( cond ) if ( (cond) ) { return TROT_RC_ERROR_PRECOND; }
#endif

#else

#define PRECOND_ERR_IF( cond )

#endif

/******************************************************************************/
#ifdef BE_PARANOID
#define PARANOID_ERR_IF( cond ) if ( (cond) ) { printf( "PARANOID ERROR! %s %d\n", __FILE__, __LINE__ ); fflush( stdout ); exit(-1); }
#else
#define PARANOID_ERR_IF( cond )
#endif

/******************************************************************************/
/* only for debugging */
#define dline printf( "dline:" __FILE__ ":%d\n", __LINE__ ); fflush( stdout );

/******************************************************************************/
#define TROT_MALLOC( POINTER, POINTER_TYPE, SIZE ) \
	POINTER = ( POINTER_TYPE * ) trotHookMalloc( sizeof( POINTER_TYPE ) * (SIZE) ); \
	ERR_IF( POINTER == NULL, TROT_RC_ERROR_MEMORY_ALLOCATION_FAILED );

/******************************************************************************/
#define TROT_CALLOC( POINTER, POINTER_TYPE, SIZE ) \
	POINTER = ( POINTER_TYPE * ) trotHookCalloc( SIZE, sizeof( POINTER_TYPE ) ); \
	ERR_IF( POINTER == NULL, TROT_RC_ERROR_MEMORY_ALLOCATION_FAILED );

/******************************************************************************/
#define TROT_REALLOC( POINTER2, POINTER1, POINTER_TYPE, SIZE ) \
	POINTER2 = ( POINTER_TYPE * ) trotHookRealloc( POINTER1, sizeof( POINTER_TYPE ) * (SIZE) ); \
	ERR_IF( POINTER2 == NULL, TROT_RC_ERROR_MEMORY_ALLOCATION_FAILED );

/******************************************************************************/
/* For trotDecodingEncoding and related tests */
typedef enum
{
	/* NOTE: These first 9 tokens are ones that our tokenizer produces */
	TOKEN_TYPE_L_BRACKET = 1,
	TOKEN_TYPE_R_BRACKET = 2,
	TOKEN_TYPE_L_BRACE = 3,
	TOKEN_TYPE_R_BRACE = 4,
	TOKEN_TYPE_L_PARENTHESIS = 5,
	TOKEN_TYPE_R_PARENTHESIS = 6,
	TOKEN_TYPE_STRING = 7,
	TOKEN_TYPE_WORD = 8,
	TOKEN_TYPE_NUMBER = 9, /* NOTE: this is not created, but changed from TOKEN_WORD in tokenizer */

	/* NOTE: These next 3 tokens are ones that our decoder changes tokens into */
	TOKEN_TYPE_TWIN = 10, /* from TOKEN_WORD */
	TOKEN_TYPE_INCLUDE = 11, /* from TOKEN_L_BRACKET and TOKEN_L_BRACE */
	TOKEN_TYPE_OP = 12, /* from TOKEN_WORD */

	/* NOTE: this next token is one that our decoder produces */
	TOKEN_TYPE_NUMBER_RAW = 13
} TOKEN_TYPE;
#define TOKEN_TYPE_MIN 1
#define TOKEN_TYPE_MAX 13

typedef enum
{
	TOKEN_INDEX_LINE = 1,
	TOKEN_INDEX_COLUMN = 2,
	TOKEN_INDEX_TYPE = 3,
	TOKEN_INDEX_VALUE = 4,
	TOKEN_INDEX_FINALLIST = 5,
	TOKEN_INDEX_NAME = 6,
	TOKEN_INDEX_ENUMS = 7,
	TOKEN_INDEX_VAR = 8
} TOKEN_INFO;
#define TOKEN_INDEX_MAX 8


/******************************************************************************/
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
typedef struct trotListNode_STRUCT trotListNode;
typedef struct trotListActual_STRUCT trotListActual;
typedef struct trotListRefListNode_STRUCT trotListRefListNode;

/*! Data in a TrotList is stored in a linked list of trotListNodes. */
struct trotListNode_STRUCT
{
	/*! 'kind' is either NODE_KIND_HEAD_OR_TAIL, NODE_KIND_INT, or
	NODE_KIND_LIST. */
	int kind;
	/*! count is how many TROT_INTs or trotListRefs are in this node. */
	int count;
	/*! if kind is NODE_KIND_INT, then n will point to an array of size
	NODE_SIZE of type TROT_INT, else n will be NULL. */
	TROT_INT *n;
	/*! if kind is NODE_KIND_LIST, then l will point to an array of size
	NODE_SIZE of type trotList*, else l will be NULL. */
	TrotList **l;

	/*! prev points to previous node in the linked list, or same node if
	this is the head of the list. */
	struct trotListNode_STRUCT *prev;
	/*! next points to the next node in the linked list, or same node if
	this is the tail of the list. */
	struct trotListNode_STRUCT *next;
};

/*! trotListActual is the main data structure in Trot. */
struct trotListActual_STRUCT
{
	/*! Flag that says whether this list is still reachable or not. If not
	reachable, then this list can be freed */
	int reachable;
	/*! Flag for 'is list reachable' so we don't get into an infinite
	    loop */
	int flagVisited;
	/*! Pointer to "previous" list. Used when we're seeing if a list is
	    reachable */
	trotListActual *previous;
	/*! Pointer to "nextToFree" list. Only set when this list is no longer
	    reachable. We use this to keep a linked list of lists that need to
	    be freed. */
	trotListActual *nextToFree;

	/*! Pointer to "parent" when we're encoding a list */
	trotListActual *encodingParent;
	/*! The child number this list is of it's parent, used for managing
	    twins when encoding */
	TROT_INT encodingChildNumber;

	/*! Tag. Which "type" or "kind" of list this is. */
	TROT_TAG tag;
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

/*! trotListRef is a reference to a TrotList */
struct TrotList_STRUCT
{
	/*! The list that this ref is inside of. */
	trotListActual *laParent;
	/*! The list that this ref points to. */
	trotListActual *laPointsTo;
};

/*! Structure for holding a linked list of references. Used in TrotList to keep
track of which references points to the trotList. */
struct trotListRefListNode_STRUCT
{
	/*! How many references are in this node */
	int count;
	/*! l will be NULL if this is the head or tail of the linked list.
	else this will be an array of size REF_LIST_NODE_SIZE of type
	TrotList */
	TrotList **l;
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
	trotListActual *la1;
	/*! current node in list1 */
	trotListNode *la1Node;
	/*! current item in l1Node */
	TROT_INT la1Count;

	/*! list2 */
	trotListActual *la2;
	/*! current node in list2 */
	trotListNode *la2Node;
	/*! current item in l2Node */
	TROT_INT la2Count;

	/*! current index */
	TROT_INT index;

	/*! previous trotStackNode */
	trotStackNode *prev;
	/*! next trotStackNode */
	trotStackNode *next;
};

/******************************************************************************/
/* trotListPrimary.c */
TROT_RC trotListNodeSplit( trotListNode *n, int keepInLeft );

TROT_RC newIntNode( trotListNode **n_A );
TROT_RC newListNode( trotListNode **n_A );

/******************************************************************************/
/* trotStack.c */
TROT_RC trotStackInit( trotStack **stack );
void trotStackFree( trotStack **stack );

TROT_RC trotStackPush( trotStack *stack, trotListActual *la1, trotListActual *la2 );
TROT_RC trotStackPop( trotStack *stack, int *empty );
TROT_RC trotStackIncrementTopIndex( trotStack *stack );

/******************************************************************************/
/* trotTokenize.c */
TROT_RC trotTokenize( TrotList *lCharacters, TrotList **lTokenList_A );
TROT_RC trotCreateToken( TROT_INT line, TROT_INT column, TROT_INT tokenType, TrotList **lToken_A );
TROT_RC _trotWordToNumber( TrotList *lWord, int *isNumber, TROT_INT *number );

/******************************************************************************/
/* trotListInt.c */
TROT_RC trotListIntOperand( TrotList *l, TROT_OP op );
TROT_RC trotListIntOperandValue( TrotList *l, TROT_OP op, TROT_INT value );

/******************************************************************************/
#endif

