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
#ifndef trotListInternal_H
#define trotListInternal_H

/******************************************************************************/
#define NODE_SIZE 64

/******************************************************************************/
#define NODE_KIND_HEAD_OR_TAIL 0
#define NODE_KIND_INT 1
#define NODE_KIND_LIST 2

/******************************************************************************/
#define REF_LIST_NODE_SIZE 16

/******************************************************************************/
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
/* trotListPrimary.c */
TROT_RC trotListGetKind( trotList *l, INT_TYPE index, int *kind );
TROT_RC trotListGetInt( trotList *l, INT_TYPE index, INT_TYPE *n );

TROT_RC trotListNodeSplit( trotListNode *n, int keepInLeft );

inline TROT_RC newIntNode( trotListNode **n_A );
inline TROT_RC newListNode( trotListNode **n_A );


/******************************************************************************/
#endif

