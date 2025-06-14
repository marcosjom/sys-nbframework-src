//
//  NBJsonParser.h
//  nbframework
//
//  Created by Marcos Ortega on 19/3/19.
//

#ifndef NBZInflate_h
#define NBZInflate_h

#include "nb/NBFrameworkDefs.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct ENZInfResult_ {
		int resultCode;
		int inBytesProcessed;
		int outBytesProcessed;
		int inBytesAvailable;
		int outBytesAvailable;
	} ENZInfResult;
	
	typedef enum ENZInfBlckType_ {
		ENZInfBlckType_Partial = 0,
		ENZInfBlckType_Final
	} ENZInfBlckType;
	
	typedef struct STNBZInflate_ {
		void* opaque;
	} STNBZInflate;
	
	void			NBZInflate_init(STNBZInflate* obj);
	void			NBZInflate_release(STNBZInflate* obj);
	
	BOOL			NBZInflate_feedStart(STNBZInflate* obj);
	ENZInfResult	NBZInflate_feed(STNBZInflate* obj, void* dst, const UI32 dstSz, const void* src, const UI32 srcSz, const ENZInfBlckType blckType);
	
#ifdef __cplusplus
}	//extern "C"
#endif

#endif /* NBZInflate_h */
