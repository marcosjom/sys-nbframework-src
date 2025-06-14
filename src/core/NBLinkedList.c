//
//  NBLinkedList.c
//  nbframework
//
//  Created by Marcos Ortega on 3/30/19.
//

#include "nb/NBFrameworkPch.h"
#include "nb/core/NBLinkedList.h"
//
#include "nb/core/NBMemory.h"

#define	NB_LINKED_LIST_VERFI_VALUE	874512

void NBLinkedList_emptyOpq_(STNBLinkedList* obj);

void NBLinkedList_init(STNBLinkedList* obj, const SI32 bytesPerItem, NBCompareFunc cmpFunc){
	NBMemory_setZeroSt(*obj, STNBLinkedList);
	obj->bytesPerItem	= bytesPerItem;
	obj->cmpFunc		= cmpFunc;
}

void NBLinkedList_release(STNBLinkedList* obj){
	NBASSERT((obj->first == NULL && obj->last == NULL && obj->use == 0) || (obj->first != NULL && obj->last != NULL && obj->use > 0))
	NBLinkedList_emptyOpq_(obj);
	NBASSERT(obj->first == NULL && obj->last == NULL && obj->use == 0)
}

void NBLinkedList_emptyOpq_(STNBLinkedList* obj){
	STNBLinkedListItmHdr* itm = obj->first;
	STNBLinkedListItmHdr* nxt = NULL;
	IF_NBASSERT(STNBLinkedListItmHdr* lst = NULL;)
	IF_NBASSERT(SI32 count = 0;)
	NBASSERT((obj->first == NULL && obj->last == NULL && obj->use == 0) || (obj->first != NULL && obj->last != NULL && obj->use > 0))
	while(itm != NULL){
		NBASSERT((itm->prev == NULL || itm != obj->first) && (itm->next == NULL || itm != obj->last))
		NBASSERT(itm->dbg.verifVal == NB_LINKED_LIST_VERFI_VALUE && itm->dbg.dataSz == obj->bytesPerItem && itm->dbg.list == obj)
		nxt			= itm->next;
		itm->prev	= itm->next = NULL;
		NBMemory_free(itm);
		IF_NBASSERT(lst = itm;)
		IF_NBASSERT(count++;)
		itm = nxt;
	}
	NBASSERT(obj->last == lst) //unsynced list
	NBASSERT(obj->use == count)
	obj->first	= obj->last = NULL;
	obj->use	= 0;
}

//

SI32 NBLinkedList_size(STNBLinkedList* obj){
	//validation
	NBASSERT((obj->first == NULL && obj->last == NULL && obj->use == 0) || (obj->first != NULL && obj->last != NULL && obj->use > 0))
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	{
		SI32 count = 0;
		STNBLinkedListItmHdr* itm = obj->first;
		while(itm != NULL){
			NBASSERT((itm->prev == NULL || itm != obj->first) && (itm->next == NULL || itm != obj->last))
			NBASSERT(itm->dbg.verifVal == NB_LINKED_LIST_VERFI_VALUE && itm->dbg.dataSz == obj->bytesPerItem && itm->dbg.list == obj)
			NBASSERT(count < obj->use) //corrupted list or circular reference
			itm = itm->next;
			count++;
		}
		NBASSERT(count == obj->use)
	}
#	endif
	return obj->use;
}

void NBLinkedList_empty(STNBLinkedList* obj){
	NBASSERT((obj->first == NULL && obj->last == NULL && obj->use == 0) || (obj->first != NULL && obj->last != NULL && obj->use > 0))
	NBLinkedList_emptyOpq_(obj);
}

BOOL NBLinkedList_isEmpty(STNBLinkedList* obj){
	NBASSERT((obj->first == NULL && obj->last == NULL && obj->use == 0) || (obj->first != NULL && obj->last != NULL && obj->use > 0))
	return (obj->first == NULL);
}

void* NBLinkedList_getFirst(STNBLinkedList* obj, const SI32 bytesPerItem){
	void* r = NULL;
	NBASSERT((obj->first == NULL && obj->last == NULL && obj->use == 0) || (obj->first != NULL && obj->last != NULL && obj->use > 0))
	NBASSERT(obj->bytesPerItem == bytesPerItem)
	if(obj->bytesPerItem == bytesPerItem && obj->first != NULL){
		NBASSERT(obj->first->dbg.verifVal == NB_LINKED_LIST_VERFI_VALUE && obj->first->dbg.dataSz == obj->bytesPerItem && obj->first->dbg.list == obj)
		r = obj->first + 1;
	}
	return r;
}

void* NBLinkedList_getLast(STNBLinkedList* obj, const SI32 bytesPerItem){
	void* r = NULL;
	NBASSERT((obj->first == NULL && obj->last == NULL && obj->use == 0) || (obj->first != NULL && obj->last != NULL && obj->use > 0))
	NBASSERT(obj->bytesPerItem == bytesPerItem)
	if(obj->bytesPerItem == bytesPerItem && obj->last != NULL){
		NBASSERT(obj->last->dbg.verifVal == NB_LINKED_LIST_VERFI_VALUE && obj->last->dbg.dataSz == obj->bytesPerItem && obj->last->dbg.list == obj)
		r = obj->last + 1;
	}
	return r;
}

void* NBLinkedList_getPrev(STNBLinkedList* obj, void* cur, const SI32 bytesPerItem){
	void* r = NULL;
	NBASSERT((obj->first == NULL && obj->last == NULL && obj->use == 0) || (obj->first != NULL && obj->last != NULL && obj->use > 0))
	NBASSERT(cur != NULL && obj->bytesPerItem == bytesPerItem)
	if(cur != NULL && obj->bytesPerItem == bytesPerItem){
		STNBLinkedListItmHdr* itm = (STNBLinkedListItmHdr*)cur - 1;
		NBASSERT(itm->dbg.verifVal == NB_LINKED_LIST_VERFI_VALUE && itm->dbg.dataSz == obj->bytesPerItem && itm->dbg.list == obj)
		if(obj->bytesPerItem == bytesPerItem && itm->prev != NULL){
			NBASSERT(itm->prev->dbg.verifVal == NB_LINKED_LIST_VERFI_VALUE && itm->prev->dbg.dataSz == obj->bytesPerItem && itm->prev->dbg.list == obj)
			r = itm->prev + 1;
		}
	}
	return r;
}

void* NBLinkedList_getNext(STNBLinkedList* obj, void* cur, const SI32 bytesPerItem){
	void* r = NULL;
	NBASSERT((obj->first == NULL && obj->last == NULL && obj->use == 0) || (obj->first != NULL && obj->last != NULL && obj->use > 0))
	NBASSERT(cur != NULL && obj->bytesPerItem == bytesPerItem)
	if(cur != NULL && obj->bytesPerItem == bytesPerItem){
		STNBLinkedListItmHdr* itm = (STNBLinkedListItmHdr*)cur - 1;
		NBASSERT(itm->dbg.verifVal == NB_LINKED_LIST_VERFI_VALUE && itm->dbg.dataSz == obj->bytesPerItem && itm->dbg.list == obj)
		if(obj->bytesPerItem == bytesPerItem && itm->next != NULL){
			NBASSERT(itm->next->dbg.verifVal == NB_LINKED_LIST_VERFI_VALUE && itm->next->dbg.dataSz == obj->bytesPerItem && itm->next->dbg.list == obj)
			r = itm->next + 1;
		}
	}
	return r;
}

//pre-allocation (optimization)

void* NBLinkedList_itmAlloc(const SI32 dataSz){
	STNBLinkedListItmHdr* itm = (STNBLinkedListItmHdr*)NBMemory_alloc(sizeof(STNBLinkedListItmHdr) + dataSz);
	NBMemory_setZeroSt(*itm, STNBLinkedListItmHdr);
	IF_NBASSERT(itm->dbg.verifVal	= NB_LINKED_LIST_VERFI_VALUE;)
	IF_NBASSERT(itm->dbg.dataSz		= dataSz;)
	return (itm + 1);
}

BOOL NBLinkedList_itmFree(const void* data, const SI32 dataSz){
	BOOL r = FALSE;
	NBASSERT(data != NULL)
	if(data != NULL){
		STNBLinkedListItmHdr* itm = (STNBLinkedListItmHdr*)data - 1;
		NBASSERT(itm->dbg.verifVal == NB_LINKED_LIST_VERFI_VALUE && itm->dbg.dataSz == dataSz && itm->dbg.list == NULL)
		NBASSERT(itm->prev == NULL && itm->next == NULL)
		if(itm->prev == NULL && itm->next == NULL){
			NBMemory_free(itm);
			r = TRUE;
		}
	}
	return r;
}

//modify

void* NBLinkedList_addAtStart(STNBLinkedList* obj, const void* data, const SI32 dataSz){
	void* r = NULL;
	NBASSERT((obj->first == NULL && obj->last == NULL && obj->use == 0) || (obj->first != NULL && obj->last != NULL && obj->use > 0))
	NBASSERT(data != NULL && obj->bytesPerItem == dataSz)
	if(data != NULL && obj->bytesPerItem == dataSz){
		IF_NBASSERT(const SI32 szBefore = NBLinkedList_size(obj);)
		STNBLinkedListItmHdr* itm = (STNBLinkedListItmHdr*)NBMemory_alloc(sizeof(STNBLinkedListItmHdr) + obj->bytesPerItem);
		itm->prev			= NULL;
		itm->next			= NULL;
		IF_NBASSERT(itm->dbg.verifVal	= NB_LINKED_LIST_VERFI_VALUE;)
		IF_NBASSERT(itm->dbg.list		= obj;)
		IF_NBASSERT(itm->dbg.dataSz		= obj->bytesPerItem;)
		r					= itm + 1;
		NBMemory_copy(r, data, obj->bytesPerItem);
		//Link
		if(obj->first != NULL){
			NBASSERT(obj->first->prev == NULL)
			NBASSERT(obj->first->dbg.verifVal == NB_LINKED_LIST_VERFI_VALUE && obj->first->dbg.dataSz == obj->bytesPerItem && obj->first->dbg.list == obj)
			obj->first->prev = itm;
			itm->next = obj->first;
		}
		//Set as first
		obj->first = itm;
		//Set as last
		if(obj->last == NULL){
			obj->last = itm;
		}
		//Count
		obj->use++;
		//Validate
		NBASSERT((szBefore + 1) == NBLinkedList_size(obj))
	}
	return r;
}

void* NBLinkedList_addAtStartPreallocated(STNBLinkedList* obj, void* data, const SI32 dataSz){
	void* r = NULL;
	NBASSERT((obj->first == NULL && obj->last == NULL && obj->use == 0) || (obj->first != NULL && obj->last != NULL && obj->use > 0))
	NBASSERT(data != NULL && obj->bytesPerItem == dataSz)
	if(data != NULL && obj->bytesPerItem == dataSz){
		STNBLinkedListItmHdr* itm = ((STNBLinkedListItmHdr*)data) - 1;
		NBASSERT(itm->prev == NULL && itm->next == NULL)
		NBASSERT(itm->dbg.verifVal == NB_LINKED_LIST_VERFI_VALUE && itm->dbg.dataSz == obj->bytesPerItem && itm->dbg.list == NULL)
		if(itm->prev == NULL && itm->next == NULL){
			IF_NBASSERT(const SI32 szBefore = NBLinkedList_size(obj);)
			IF_NBASSERT(itm->dbg.list		= obj;)
			r					= data;
			//Link
			if(obj->first != NULL){
				NBASSERT(obj->first->prev == NULL)
				NBASSERT(obj->first->dbg.verifVal == NB_LINKED_LIST_VERFI_VALUE && obj->first->dbg.dataSz == obj->bytesPerItem && obj->first->dbg.list == obj)
				obj->first->prev = itm;
				itm->next = obj->first;
			}
			//Set as first
			obj->first = itm;
			//Set as last
			if(obj->last == NULL){
				obj->last = itm;
			}
			//Count
			obj->use++;
			//Validate
			NBASSERT((szBefore + 1) == NBLinkedList_size(obj))
		}
	}
	return r;
}

void* NBLinkedList_addAtEnd(STNBLinkedList* obj, const void* data, const SI32 dataSz){
	void* r = NULL;
	NBASSERT((obj->first == NULL && obj->last == NULL && obj->use == 0) || (obj->first != NULL && obj->last != NULL && obj->use > 0))
	NBASSERT(data != NULL && obj->bytesPerItem == dataSz)
	if(data != NULL && obj->bytesPerItem == dataSz){
		IF_NBASSERT(const SI32 szBefore = NBLinkedList_size(obj);)
		STNBLinkedListItmHdr* itm = (STNBLinkedListItmHdr*)NBMemory_alloc(sizeof(STNBLinkedListItmHdr) + obj->bytesPerItem);
		itm->prev			= NULL;
		itm->next			= NULL;
		IF_NBASSERT(itm->dbg.verifVal	= NB_LINKED_LIST_VERFI_VALUE;)
		IF_NBASSERT(itm->dbg.list		= obj;)
		IF_NBASSERT(itm->dbg.dataSz		= obj->bytesPerItem;)
		r					= itm + 1;
		NBMemory_copy(r, data, obj->bytesPerItem);
		//Link
		if(obj->last != NULL){
			NBASSERT(obj->last->next == NULL)
			NBASSERT(obj->last->dbg.verifVal == NB_LINKED_LIST_VERFI_VALUE && obj->last->dbg.dataSz == obj->bytesPerItem && obj->last->dbg.list == obj)
			obj->last->next = itm;
			itm->prev = obj->last;
		}
		//Set as last
		obj->last = itm;
		//Set as first
		if(obj->first == NULL){
			obj->first = itm;
		}
		//Count
		obj->use++;
		//Validate
		NBASSERT((szBefore + 1) == NBLinkedList_size(obj))
	}
	return r;
}

void* NBLinkedList_addAtEndPreallocated(STNBLinkedList* obj, void* data, const SI32 dataSz){
	void* r = NULL;
	NBASSERT((obj->first == NULL && obj->last == NULL && obj->use == 0) || (obj->first != NULL && obj->last != NULL && obj->use > 0))
	NBASSERT(data != NULL && obj->bytesPerItem == dataSz)
	if(data != NULL && obj->bytesPerItem == dataSz){
		STNBLinkedListItmHdr* itm = ((STNBLinkedListItmHdr*)data) - 1;
		NBASSERT(itm->prev == NULL && itm->next == NULL)
		NBASSERT(itm->dbg.verifVal == NB_LINKED_LIST_VERFI_VALUE && itm->dbg.dataSz == obj->bytesPerItem && itm->dbg.list == NULL)
		if(itm->prev == NULL && itm->next == NULL){
			IF_NBASSERT(const SI32 szBefore = NBLinkedList_size(obj);)
			IF_NBASSERT(itm->dbg.list		= obj;)
			r					= data;
			//Link
			if(obj->last != NULL){
				NBASSERT(obj->last->next == NULL)
				NBASSERT(obj->last->dbg.verifVal == NB_LINKED_LIST_VERFI_VALUE && obj->last->dbg.dataSz == obj->bytesPerItem && obj->last->dbg.list == obj)
				obj->last->next = itm;
				itm->prev = obj->last;
			}
			//Set as last
			obj->last = itm;
			//Set as first
			if(obj->first == NULL){
				obj->first = itm;
			}
			//Count
			obj->use++;
			//Validate
			NBASSERT((szBefore + 1) == NBLinkedList_size(obj))
		}
	}
	return r;
}

void NBLinkedList_remove_(STNBLinkedList* obj, const void* cur, const SI32 dataSz, const BOOL deallocate){
	NBASSERT((obj->first == NULL && obj->last == NULL && obj->use == 0) || (obj->first != NULL && obj->last != NULL && obj->use > 0))
	NBASSERT(cur != NULL && dataSz == obj->bytesPerItem)
	if(cur != NULL && dataSz == obj->bytesPerItem){
		STNBLinkedListItmHdr* itm = (STNBLinkedListItmHdr*)cur - 1;
		NBASSERT(itm->dbg.verifVal == NB_LINKED_LIST_VERFI_VALUE && itm->dbg.dataSz == obj->bytesPerItem && itm->dbg.list == obj)
		IF_NBASSERT(const SI32 szBefore = NBLinkedList_size(obj);)
		//Update links
		if(itm->prev != NULL){
			NBASSERT(itm->prev->dbg.verifVal == NB_LINKED_LIST_VERFI_VALUE && itm->prev->dbg.dataSz == obj->bytesPerItem && itm->prev->dbg.list == obj)
			itm->prev->next = itm->next;
		}
		if(itm->next != NULL){
			NBASSERT(itm->next->dbg.verifVal == NB_LINKED_LIST_VERFI_VALUE && itm->next->dbg.dataSz == obj->bytesPerItem && itm->next->dbg.list == obj)
			itm->next->prev = itm->prev;
		}
		if(obj->first == itm){
			obj->first = itm->next;
		}
		if(obj->last == itm){
			obj->last = itm->prev;
		}
		//Release
		itm->prev		= itm->next = NULL;
		IF_NBASSERT(itm->dbg.list	= NULL;)
		if(deallocate){
			NBMemory_free(itm);
		}
		//Count
		NBASSERT(obj->use > 0)
		obj->use--;
		//Validate
		NBASSERT(szBefore == (NBLinkedList_size(obj) + 1))
	}
}
	
void NBLinkedList_remove(STNBLinkedList* obj, const void* cur, const SI32 dataSz){
	NBLinkedList_remove_(obj, cur, dataSz, TRUE);
}

void NBLinkedList_removeWithoutDeallocating(STNBLinkedList* obj, const void* cur, const SI32 dataSz){
	NBLinkedList_remove_(obj, cur, dataSz, FALSE);
}
