//
//  NBLinkedList.h
//  nbframework
//
//  Created by Marcos Ortega on 3/30/19.
//

#ifndef NBLinkedList_h
#define NBLinkedList_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBCompare.h"

#define NBLinkedList_addValueAtStart(OBJ, VAL)	NBLinkedList_addAtStart(OBJ, &(VAL), sizeof(VAL))
#define NBLinkedList_addValueAtEnd(OBJ, VAL)	NBLinkedList_addAtEnd(OBJ, &(VAL), sizeof(VAL))
#define NBLinkedList_removePtr(OBJ, PTR)		NBLinkedList_remove(OBJ, PTR, sizeof(*(PTR)))
#define NBLinkedList_getFirstPtr(OBJ, TYPE)		(TYPE*)NBLinkedList_getFirst(OBJ, sizeof(TYPE))
#define NBLinkedList_getLastPtr(OBJ, TYPE)		(TYPE*)NBLinkedList_getLast(OBJ, sizeof(TYPE))
#define NBLinkedList_getPrevPtr(OBJ, CUR, TYPE)	(TYPE*)NBLinkedList_getPrev(OBJ, CUR, sizeof(TYPE))
#define NBLinkedList_getNextPtr(OBJ, CUR, TYPE)	(TYPE*)NBLinkedList_getNext(OBJ, CUR, sizeof(TYPE))


struct STNBLinkedList_;

//NBLinkedListItmHdr

typedef struct STNBLinkedListItmHdr_ {
	struct STNBLinkedListItmHdr_*	prev;
	struct STNBLinkedListItmHdr_*	next;
	//dbg
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	struct {
		struct STNBLinkedList_*		list;		//owner list
		SI32						verifVal;	//should allways be NB_LINKED_LIST_VERFI_VALUE
		SI32						dataSz;		//payload size
	} dbg;
#	endif
	//Followed by the data
} STNBLinkedListItmHdr;

//NBLinkedList

typedef struct STNBLinkedList_ {
	SI32					bytesPerItem;
	NBCompareFunc			cmpFunc;
	STNBLinkedListItmHdr*	first;	//first itm
	STNBLinkedListItmHdr*	last;	//last item
	SI32					use;
} STNBLinkedList;

void NBLinkedList_init(STNBLinkedList* obj, const SI32 bytesPerItem, NBCompareFunc cmpFunc);
void NBLinkedList_release(STNBLinkedList* obj);

//
SI32  NBLinkedList_size(STNBLinkedList* obj);
void  NBLinkedList_empty(STNBLinkedList* obj);
BOOL  NBLinkedList_isEmpty(STNBLinkedList* obj);
void* NBLinkedList_getFirst(STNBLinkedList* obj, const SI32 bytesPerItem);
void* NBLinkedList_getLast(STNBLinkedList* obj, const SI32 bytesPerItem);
void* NBLinkedList_getPrev(STNBLinkedList* obj, void* cur, const SI32 bytesPerItem);
void* NBLinkedList_getNext(STNBLinkedList* obj, void* cur, const SI32 bytesPerItem);

//pre-allocation (optimization)
void* NBLinkedList_itmAlloc(const SI32 dataSz);
BOOL NBLinkedList_itmFree(const void* data, const SI32 dataSz);

//modify
void* NBLinkedList_addAtStart(STNBLinkedList* obj, const void* data, const SI32 dataSz);
void* NBLinkedList_addAtStartPreallocated(STNBLinkedList* obj, void* data, const SI32 dataSz);
void* NBLinkedList_addAtEnd(STNBLinkedList* obj, const void* data, const SI32 dataSz);
void* NBLinkedList_addAtEndPreallocated(STNBLinkedList* obj, void* data, const SI32 dataSz);
void  NBLinkedList_remove(STNBLinkedList* obj, const void* cur, const SI32 dataSz);
void  NBLinkedList_removeWithoutDeallocating(STNBLinkedList* obj, const void* cur, const SI32 dataSz);

#endif /* NBLinkedList_h */
