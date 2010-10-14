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

#ifndef gkList_H
#define gkList_H

#include <stdlib.h> /* for size_t for gkCalloc */

/******************************************************************************/
#define GK_LIST_SUCCESS 0

#define GK_LIST_ERROR_GENERAL -1
#define GK_LIST_ERROR_PRECOND -2
#define GK_LIST_ERROR_MEMORY_ALLOCATION_FAILED -3
#define GK_LIST_ERROR_BAD_INDEX -4
#define GK_LIST_ERROR_WRONG_KIND -5

/******************************************************************************/
#define INT_TYPE int

#define NODE_SIZE 64

#define NODE_KIND_HEAD_OR_TAIL 0
#define NODE_KIND_INT 1
#define NODE_KIND_LIST 2

/******************************************************************************/
#define REF_LIST_NODE_SIZE 16

#define LIST_STACK_NODE_SIZE 64

/******************************************************************************/
extern void *(*gkCalloc)( size_t nmemb, size_t size );
extern void *(*gkMalloc)( size_t size );
extern void (*gkFree)( void *ptr );

/******************************************************************************/
typedef struct gkListRef_STRUCT gkListRef;
typedef struct gkListNode_STRUCT gkListNode;
typedef struct gkList_STRUCT gkList;
typedef struct gkListRefListNode_STRUCT gkListRefListNode;
typedef struct gkListListStack_STRUCT gkListListStack;

/*! Data in a gkList is stored in a linked list of gkListNodes. */
struct gkListNode_STRUCT
{
	/*! 'kind' is either NODE_KIND_HEAD_OR_TAIL, NODE_KIND_INT, or
	NODE_KIND_LIST. */
	int kind;
	/*! count is how many INT_TYPEs or gkListRefs are in this node. */
	int count;
	/*! if kind is NODE_KIND_INT, then n will point to an array of size
	NODE_SIZE of type INT_TYPE, else n will be NULL. */
	INT_TYPE *n;
	/*! if kind is NODE_KIND_LIST, then l will point to an array of size
	NODE_SIZE of type gkListRef*, else l will be NULL. */
	gkListRef **l;

	/*! prev points to previous node in the linked list, or same node if
	this is the head of the list. */
	struct gkListNode_STRUCT *prev;
	/*! next points to the next node in the linked list, or same node if
	this is the tail of the list. */
	struct gkListNode_STRUCT *next;
};

/*! gkList is the main data structure in Trot. */
struct gkList_STRUCT
{
	/*! Flag that says whether this list is still reachable or not. If not
	reachable, then this list can be freed */
	int reachable;
	/*! How many children are in the list */
	int childrenCount;
	/*! Pointer to the head of the linked list that contains the refs that
	point to this list. Used for checking whether this list is still
	reachable or not. */
	gkListRefListNode *refListHead;
	/*! Pointer to the tail of the linked list that contains the refs that
	point to this list. Used for checking whether this list is still
	reachable or not. */
	gkListRefListNode *refListTail;
	/*! Pointer to the head of the linked list that contains the actual data
	in the list. */
	gkListNode *head;
	/*! Pointer to the tail of the linked list that contains the actual data
	in the list. */
	gkListNode *tail;
};

/*! gkListRef is a reference to a gkList */
struct gkListRef_STRUCT
{
	/*! The list that this ref is inside of. */
	gkList *lParent;
	/*! The list that this ref points to. */
	gkList *lPointsTo;
};

/*! Structure for holding a linked list of references. Used in gkList to keep
track of which references points to the gkList. */
struct gkListRefListNode_STRUCT
{
	/*! How many references are in this node */
	int count;
	/*! r will be NULL if this is the head or tail of the linked list.
	else this will be an array of size REF_LIST_NODE_SIZE of type
	gkListRef */
	gkListRef **r;
	/*! points to the next node in the linked list, or to itself if this is
	the tail. */
	gkListRefListNode *next;
	/*! points to the prev node in the linked list, or to itself if this is
	the head. */
	gkListRefListNode *prev;
};

/*! Holds a stack of gkList pointers. Used during memory management to try to
follow references "up" to see if a list is reachable. We use the stack to make
sure we don't get into an infinite loop. */
struct gkListListStack_STRUCT
{
	/*! how many pointers are in this node. */
	int count;
	/*! an array of size LIST_STACK_NODE_SIZE of type gkList* */
	gkList **l;
	/*! points to the next node in the stack, or NULL if this is the last
	node in the stack. */
	gkListListStack *next;
};

/******************************************************************************/
/* trotListPrimary.c */
int gkListRefInit( gkListRef **lr_A );
int gkListRefTwin( gkListRef **lr_A, gkListRef *lrToTwin );
int gkListRefFree( gkListRef **lr_F );

int gkListRefGetCount( gkListRef *lr, INT_TYPE *c );

int gkListRefGetKind( gkListRef *lr, INT_TYPE index, int *kind );

int gkListRefAppendInt( gkListRef *lr, INT_TYPE n );
int gkListRefAppendListTwin( gkListRef *lr, gkListRef *lrToAppend );

int gkListRefInsertInt( gkListRef *lr, INT_TYPE index, INT_TYPE n );
int gkListRefInsertListTwin( gkListRef *lr, INT_TYPE index, gkListRef *l );

int gkListRefGetInt( gkListRef *lr, INT_TYPE index, INT_TYPE *n );
int gkListRefGetListTwin( gkListRef *lr, INT_TYPE index, gkListRef **l );

int gkListRefRemoveInt( gkListRef *lr, INT_TYPE index, INT_TYPE *n );
int gkListRefRemoveList( gkListRef *lr, INT_TYPE index, gkListRef **l );
int gkListRefRemove( gkListRef *lr, INT_TYPE index );

/******************************************************************************/
#endif

