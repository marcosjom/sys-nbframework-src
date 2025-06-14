
#include "nb/NBFrameworkPch.h"
#include "nb/core/NBStructMaps.h"
#include "nb/core/NBMemory.h"
#include "nb/core/NBThreadMutex.h"

typedef struct STNBStructMapsOpq_ {
	STNBThreadMutex     mutex;
    //records
    struct {
        STNBStructMapsRec** arr;
        UI32            use;
        UI32            sz;
    } records;
} STNBStructMapsOpq;

void NBStructMaps_init(STNBStructMaps* obj){
	STNBStructMapsOpq* opq = obj->opaque = NBMemory_allocType(STNBStructMapsOpq);
	NBMemory_setZeroSt(*opq, STNBStructMapsOpq);
	NBThreadMutex_init(&opq->mutex);
}

void NBStructMaps_release(STNBStructMaps* obj){
	STNBStructMapsOpq* opq = (STNBStructMapsOpq*)obj->opaque;
	NBASSERT(opq != NULL)
	if(opq != NULL){
        //maps
        {
            if(opq->records.arr != NULL){
                UI32 i; for(i = 0; i < opq->records.use; i++){
                    STNBStructMapsRec* map = opq->records.arr[i];
                    if(map->isInited){
                        if(map->map != NULL){
                            NBStructMap_release(map->map);
                            NBMemory_free(map->map);
                            map->map = NULL;
                        }
                        NBThreadMutex_release(&map->mutex);
                        map->isInited = FALSE;
                    }
                }
            }
            opq->records.use = 0;
            opq->records.sz = 0;
        }
		NBThreadMutex_release(&opq->mutex);
		NBMemory_free(opq);
	}
}

//lock

void NBStructMaps_lock(STNBStructMaps* obj, STNBStructMapsRec* map){
	STNBStructMapsOpq* opq = (STNBStructMapsOpq*)obj->opaque;
	NBASSERT(opq != NULL)
	NBASSERT(map != NULL)
	if(opq != NULL && map != NULL){
		NBThreadMutex_lock(&opq->mutex);
		{
			if(!map->isInited){
				NBThreadMutex_init(&map->mutex);
				map->isLocked = FALSE;
				map->isInited = TRUE;
                //add to records
                {
                    //resize maps
                    while(opq->records.sz <= opq->records.use){
                        const UI32 szN = (opq->records.use + 128);
                        STNBStructMapsRec** arrN = NBMemory_allocTypes(STNBStructMapsRec*, szN);
                        if(arrN == NULL){
                            break;
                        } else {
                            if(opq->records.arr != NULL){
                                if(opq->records.use > 0){
                                    NBMemory_copy(arrN, opq->records.arr, sizeof(arrN[0]) * opq->records.use);
                                }
                                NBMemory_free(opq->records.arr);
                            }
                            opq->records.arr = arrN;
                            opq->records.sz = szN;
                        }
                    }
                    //add to maps
                    if(opq->records.use < opq->records.sz){
                        opq->records.arr[opq->records.use++] = map;
                    }
                }
			}
			//ToDo: think in a way to enable with parallel calls
			//NBThreadMutex_lock(&map->mutex);
			//NBASSERT(!map->isLocked)
			//map->isLocked = TRUE;
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
}

void NBStructMaps_unlock(STNBStructMaps* obj, STNBStructMapsRec* map){
	STNBStructMapsOpq* opq = (STNBStructMapsOpq*)obj->opaque;
	NBASSERT(opq != NULL)
	NBASSERT(map != NULL)
	if(opq != NULL && map != NULL){
		NBASSERT(map->isInited)
		if(map->isInited){
			//ToDo: think in a way to enable with parallel calls
			//NBASSERT(map->isLocked)
			//map->isLocked = FALSE;
			//NBThreadMutex_unlock(&map->mutex);
		}
	}
}

//

STNBStructMap* NBStructMaps_allocType(STNBStructMaps* obj, const char* name){
	STNBStructMap* r = NULL;
	STNBStructMapsOpq* opq = (STNBStructMapsOpq*)obj->opaque;
	NBASSERT(opq != NULL)
	if(opq != NULL){
		r = NBMemory_allocType(STNBStructMap);
	}
	return  r;
}
