

#include "nb/NBFrameworkPch.h"
#include "nb/core/NBStringsPool.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBLog.h"

//Init and release
void NBStringsPool_init(STNBStringsPool* obj, const UI32 strsLenInitial, const UI32 strsLenMinGrowth){
    NBArray_init(&obj->itms, sizeof(STNBStringsPoolItm), NULL);
    NBThreadMutex_init(&obj->itmsMutex);
	obj->strsLenInitial		= strsLenInitial;
	obj->strsLenMinGrowth	= strsLenMinGrowth;
}

void NBStringsPool_release(STNBStringsPool* obj){
    NBThreadMutex_lock(&obj->itmsMutex);
	{
		SI32 i;
		for(i = (obj->itms.use - 1); i >= 0; i--){
			STNBStringsPoolItm* itm = (STNBStringsPoolItm*)NBArray_itemAtIndex(&obj->itms, i);
		    NBASSERT(itm->retainCount == 0) //User logic error, must wait untill all connections are released.
			{
			    NBString_release(itm->str);
			    NBMemory_free(itm->str);
				itm->str = NULL;
			}
		}
	    NBArray_empty(&obj->itms);
	    NBArray_release(&obj->itms);
	}
    NBThreadMutex_unlock(&obj->itmsMutex);
    NBThreadMutex_release(&obj->itmsMutex);
	obj->strsLenInitial		= 0;
	obj->strsLenMinGrowth	= 0;
}

void NBStringsPool_empty(STNBStringsPool* obj){
	NBThreadMutex_lock(&obj->itmsMutex);
	{
		SI32 i;
		for(i = (obj->itms.use - 1); i >= 0; i--){
			STNBStringsPoolItm* itm = (STNBStringsPoolItm*)NBArray_itemAtIndex(&obj->itms, i);
		    NBASSERT(itm->retainCount == 0) //User logic error, must wait untill all connections are released.
			{
			    NBString_release(itm->str);
			    NBMemory_free(itm->str);
				itm->str = NULL;
			}
		    NBArray_removeItemAtIndex(&obj->itms, i);
		}
	}
    NBThreadMutex_unlock(&obj->itmsMutex);
}

//Get or add conn

STNBString* NBStringsPool_get(STNBStringsPool* obj){
	STNBString* r = NULL;
	{
	    NBThreadMutex_lock(&obj->itmsMutex);
		//Search for current free equivalent connection
		{
			SI32 i; for(i = 0; i < obj->itms.use; i++){
				STNBStringsPoolItm* itm = (STNBStringsPoolItm*)NBArray_itemAtIndex(&obj->itms, i);
				if(itm->retainCount == 0){
					itm->retainCount++;
					r = itm->str;
					break;
				}
			}
		}
		//Create new connection usgin base as reference
		if(r == NULL){
			STNBStringsPoolItm itm;
			itm.str			= (STNBString*)NBMemory_alloc(sizeof(STNBString));
			itm.retainCount	= 1;
		    NBString_initWithSz(itm.str, obj->strsLenInitial, obj->strsLenMinGrowth, 0.5f);
		    NBArray_addValue(&obj->itms, itm);
			r = itm.str;
		}
	    NBThreadMutex_unlock(&obj->itmsMutex);
	}
	return r;
}

BOOL NBStringsPool_isChild(STNBStringsPool* obj, const STNBString* str){
	BOOL r = FALSE;
	if(str != NULL){
	    NBThreadMutex_lock(&obj->itmsMutex);
		SI32 i; for(i = 0; i < obj->itms.use; i++){
			STNBStringsPoolItm* itm = (STNBStringsPoolItm*)NBArray_itemAtIndex(&obj->itms, i);
			if(itm->str == str){
				r = TRUE;
				break;
			}
		}
	    NBThreadMutex_unlock(&obj->itmsMutex);
	}
	return r;
}

BOOL NBStringsPool_return(STNBStringsPool* obj, const STNBString* str){
	BOOL r = FALSE;
	if(str != NULL){
	    NBThreadMutex_lock(&obj->itmsMutex);
		SI32 i; for(i = 0; i < obj->itms.use; i++){
			STNBStringsPoolItm* itm = (STNBStringsPoolItm*)NBArray_itemAtIndex(&obj->itms, i);
			if(itm->str == str){
			    NBASSERT(itm->retainCount > 0)
				if(itm->retainCount > 0){
					itm->retainCount--;
					r = TRUE;
				}
				break;
			}
		}
	    NBThreadMutex_unlock(&obj->itmsMutex);
	}
    NBASSERT(r) //the string is beign returned to the wrong pool?
	return r;
}


