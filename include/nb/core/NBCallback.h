//
//  NBCallback.h
//  nbframework
//
//  Created by Marcos Ortega on 13/3/19.
//

#ifndef NBCallback_h
#define NBCallback_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBThreadMutex.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBCallbackMethod_ {
		void*	obj;
		void	(*retain)(void* obj);
		void	(*release)(void* obj);
		void	(*callback)(void* obj);
	} STNBCallbackMethod;
	
	typedef struct STNBCallback_ {
		void* opaque;
	} STNBCallback;
	
	void NBCallback_init(STNBCallback* obj);
	void NBCallback_initWithMethods(STNBCallback* obj, STNBCallbackMethod* mthds);
	void NBCallback_initAsRefOf(STNBCallback* obj, const STNBCallback* other);
	void NBCallback_release(STNBCallback* obj);
	
	void NBCallback_call(STNBCallback* obj);
	void NBCallback_setActive(STNBCallback* obj, const BOOL active);
	
#ifdef __cplusplus
} //extern "C"
#endif
		
#endif /* NBCallback_h */
