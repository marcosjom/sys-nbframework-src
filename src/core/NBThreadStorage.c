#include "nb/NBFrameworkPch.h"
//Complementary PCH
#ifdef _WIN32
#include <Windows.h>	//for TlsAlloc, TlsGetValue, TlsSetValue, TlsFree
#endif
#include "nb/core/NBThreadStorage.h"
#include "nb/core/NBMemory.h"
//
#include "../src/core/NBThreadStoragePriv.h"

#ifdef CONFIG_NB_INCLUDE_THREADS_STORAGE_CLEANUP
#   include "nb/core/NBMngrProcess.h"
#endif

//STNBThreadStorageData

//Usually 'STNBThreadStorage' are static variables.
//The internal references are these:
//STNBThreadStorage -> (member) key -> (data-value) STNBThreadStorageData -> (parent's poiter) STNBThreadStorage


typedef struct STNBThreadStorageData_ {
    STNBThreadStorage*               parent;
    void*                            usrData;
    NBThreadStorageDestroyDataMthd   destroyDataMthd;
} STNBThreadStorageData;

//

void NBThreadStorage_initOnce(STNBThreadStorage* obj, NBThreadStorageCreateMthd createMthd) {
	//Note: keys should be static (not allocated)
	//PRINTF_INFO("NBThreadStorage_initOnce (%lld).\n", (SI64)obj);
#	ifdef _WIN32
	{
		if (obj->tlsIdx == 0xFFFFFFFF && createMthd != NULL) {
			(*createMthd)();
		}
	}
#	else
	{
		pthread_once(&obj->keyOnce, createMthd);
		//PRINTF_INFO("NBThreadMutex, sizeof(pthread_once_t) = %d, sizeof(pthread_key_t) = %d.\n", (int)sizeof(pthread_once_t), (int)sizeof(pthread_key_t));
	}
#	endif
}

void NBThreadStorage_create(STNBThreadStorage* obj) {
	//PRINTF_INFO("NBThreadStorage_create (%lld).\n", (SI64)obj);
#	ifdef _WIN32
	{
		NBASSERT(obj->tlsIdx == 0xFFFFFFFF)
		obj->tlsIdx = TlsAlloc();
        if(TLS_OUT_OF_INDEXES == obj->tlsIdx){
            PRINTF_ERROR("ERROR, NBThreadStorage_create, TlsAlloc failed.\n");
            obj->tlsIdx = 0xFFFFFFFF;
        } else {
            NBASSERT(NULL == TlsGetValue(obj->tlsIdx))
        }
	}
#	else
	{
        if(0 != pthread_key_create(&obj->key, NBThreadStorage_destroyFromData)){
            PRINTF_ERROR("ERROR, NBThreadStorage_create, pthread_key_create failed.\n");
        } else {
            NBASSERT(NULL == pthread_getspecific(obj->key))
        }
	}
#	endif
}

void NBThreadStorage_destroyFromData(void* pData){
    //PRINTF_INFO("NBThreadStorage_destroyFromData data(%lld).\n", (SI64)pData);
    if(pData != NULL){
        STNBThreadStorageData* data = (STNBThreadStorageData*)pData;
        STNBThreadStorageData cpy = *data; //static copy (to be used after free())
#       ifdef _WIN32
        NBASSERT(TlsGetValue(data->parent->tlsIdx) == data)
#       else
        NBASSERT(pthread_getspecific(data->parent->key) == data)
#       endif
        //Nullify the key
        {
#           ifdef _WIN32
            if(!TlsSetValue(cpy.parent->tlsIdx, NULL)){
                PRINTF_ERROR("ERROR, TlsSetValue, pthread_setspecific failed.\n");
            }
#           else
            if(0 != pthread_setspecific(cpy.parent->key, NULL)){
                PRINTF_ERROR("ERROR, NBThreadStorage_destroyFromData, pthread_setspecific failed.\n");
            }
#           endif
        }
        //Destroy the key (avoid duplicate cleanup in systems where the key is automatically dstriyed at thread exit)
        {
#           ifdef _WIN32
            if(!TlsFree(cpy.parent->tlsIdx, NULL)){
                PRINTF_ERROR("ERROR, NBThreadStorage_destroyFromData, TlsFree failed.\n");
            } else {
                //ToDo: implement
                //*cpy.key = NBThreadStorage_initialValue;
            }
#           else
            if(0 != pthread_key_delete(cpy.parent->key)){
                PRINTF_ERROR("ERROR, NBThreadStorage_destroyFromData, pthread_key_delete failed.\n");
            } else {
                //ToDo: implement
                //*cpy.key = NBThreadStorage_initialValue;
            }
#           endif
        }
        //Destroy the user data
        {
            if(cpy.destroyDataMthd != NULL && cpy.usrData != NULL){
                (cpy.destroyDataMthd)(cpy.usrData);
            }
            NBMemory_setZeroSt(*data, STNBThreadStorageData);
            NBMemory_freeUnmanaged(data);    //avoid 'NBMemory_free' due to NBMngrProcess monitoring
        }
#       ifdef CONFIG_NB_INCLUDE_THREADS_STORAGE_CLEANUP
        {
            //Motify threads manager
            NBMngrProcess_storageRemoveDestroyed(pData);
        }
#       endif
    }
}

void* NBThreadStorage_getData(STNBThreadStorage* obj){
	void* r = NULL;
	//PRINTF_INFO("NBThreadStorage_getData (%lld).\n", (SI64)obj);
	if(obj != NULL){
#		ifdef _WIN32
		STNBThreadStorageData* data = (STNBThreadStorageData*)TlsGetValue(obj->tlsIdx);
#		else
		STNBThreadStorageData* data = (STNBThreadStorageData*)pthread_getspecific(obj->key);
#		endif
		if(data != NULL){
			r = data->usrData;
		}
	}
	return r;
}

void NBThreadStorage_setData(STNBThreadStorage* obj, void* usrData, NBThreadStorageDestroyDataMthd destroyDataMthd){
	//PRINTF_INFO("NBThreadStorage_setData (%lld).\n", (SI64)obj);
	if(obj != NULL){
#		ifdef _WIN32
		STNBThreadStorageData* data = (STNBThreadStorageData*)TlsGetValue(obj->tlsIdx);
#		else
		STNBThreadStorageData* data = (STNBThreadStorageData*)pthread_getspecific(obj->key);
#		endif
		if(data != NULL){
            //ToDo: validate 'destroyDataMthd' is the same as before.
			data->usrData = usrData;
		} else {
			//Create abstract-data (first time)
			STNBThreadStorageData* data = (STNBThreadStorageData*)NBMemory_allocUnmanaged(sizeof(STNBThreadStorageData)); //avoid 'NBMemory_alloc' due to NBMngrProcess monitoring
			NBMemory_setZeroSt(*data, STNBThreadStorageData);
			data->parent    = obj;
			data->usrData   = usrData;
			data->destroyDataMthd = destroyDataMthd;
#			ifdef _WIN32
            if(!TlsSetValue(obj->tlsIdx, data)){
                //ToDo: free(data) and return FALSE
                PRINTF_ERROR("ERROR, NBThreadStorage_setData, TlsSetValue failed.\n");
            } else {
#               ifdef CONFIG_NB_INCLUDE_THREADS_STORAGE_CLEANUP
                NBMngrProcess_storageAddCreated(data);
#               endif
            }
#			else
            if(0 != pthread_setspecific(obj->key, data)){
                //ToDo: free(data) and return FALSE
                PRINTF_ERROR("ERROR, NBThreadStorage_setData, pthread_setspecific failed.\n");
            } else {
#               ifdef CONFIG_NB_INCLUDE_THREADS_STORAGE_CLEANUP
                NBMngrProcess_storageAddCreated(data);
#               endif
            }
#			endif
		}
	}
}
