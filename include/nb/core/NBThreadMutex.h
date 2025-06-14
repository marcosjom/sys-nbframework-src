#ifndef NB_THREAD_MUTEX_H
#define NB_THREAD_MUTEX_H

#include "nb/NBFrameworkDefs.h"

#ifdef __cplusplus
extern "C" {
#endif
	
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
#		define NBThreadMutex_init(OBJ)		NBThreadMutex_init_((OBJ), #OBJ, __FILE__, __LINE__, __func__)
#		define NBThreadMutex_release(OBJ)	NBThreadMutex_release_((OBJ), __FILE__, __LINE__, __func__)
#		define NBThreadMutex_lock(OBJ)		NBThreadMutex_lock_((OBJ), __FILE__, __LINE__, __func__);
#		define NBThreadMutex_unlock(OBJ)	NBThreadMutex_unlock_((OBJ), __FILE__, __LINE__, __func__);
#	else
#		define NBThreadMutex_init(OBJ)		NBThreadMutex_init_((OBJ))
#		define NBThreadMutex_release(OBJ)	NBThreadMutex_release_((OBJ))
#		define NBThreadMutex_lock(OBJ)		NBThreadMutex_lock_((OBJ));
#		define NBThreadMutex_unlock(OBJ)	NBThreadMutex_unlock_((OBJ));
#	endif

	typedef struct STNBThreadMutex_ {
		void*	opaque;
	} STNBThreadMutex;
	
	//
	
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	void NBThreadMutex_init_(STNBThreadMutex* obj, const char* name, const char* fullpath, const SI32 line, const char* func);
	void NBThreadMutex_release_(STNBThreadMutex* obj, const char* fullpath, const SI32 line, const char* func);
	BOOL NBThreadMutex_lock_(STNBThreadMutex* obj, const char* fullpath, const SI32 line, const char* func);
	BOOL NBThreadMutex_unlock_(STNBThreadMutex* obj, const char* fullpath, const SI32 line, const char* func);
	UI64 NBThreadMutex_getUid(STNBThreadMutex* obj);
#	else
	void NBThreadMutex_init_(STNBThreadMutex* obj);
	void NBThreadMutex_release_(STNBThreadMutex* obj);
	BOOL NBThreadMutex_lock_(STNBThreadMutex* obj);
	BOOL NBThreadMutex_unlock_(STNBThreadMutex* obj);
#	endif
	
		
#ifdef __cplusplus
	} //extern "C"
#endif

#endif
