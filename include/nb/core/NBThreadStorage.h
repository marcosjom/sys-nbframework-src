#ifndef NB_THREAD_STORAGE_H
#define NB_THREAD_STORAGE_H

#include "nb/NBFrameworkDefs.h"
#ifdef _WIN32
	//nothing
#else
#	include <pthread.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif
	
    typedef void (*NBThreadStorageCreateMthd)(void);
	typedef void (*NBThreadStorageDestroyDataMthd)(void* data);
	
    //STNBThreadStorage

	//Note: keys should be static (not allocated)
	typedef struct STNBThreadStorage_ {
#		ifdef _WIN32
		UI32 			tlsIdx;		//TLS_OUT_OF_INDEXES(0xFFFFFFFF) means error or not-initialized
#		else
		pthread_once_t	keyOnce;	//@osx, sizeof(pthread_once_t) = 16
		pthread_key_t	key;		//@osx, sizeof(pthread_key_t) = 8
#		endif
	} STNBThreadStorage;

#	ifdef _WIN32
#		define NBThreadStorage_initialValue		{ 0xFFFFFFFF /*TLS_OUT_OF_INDEXES, defined in WinBase.h*/ }
#	else
#		define NBThreadStorage_initialValue		{ PTHREAD_ONCE_INIT }
#	endif
	
	void NBThreadStorage_initOnce(STNBThreadStorage* obj, NBThreadStorageCreateMthd createMthd);
	void NBThreadStorage_create(STNBThreadStorage* obj);
	void* NBThreadStorage_getData(STNBThreadStorage* obj);
	void NBThreadStorage_setData(STNBThreadStorage* obj, void* usrData, NBThreadStorageDestroyDataMthd destroyDataMthd);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
