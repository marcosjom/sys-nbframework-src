//
//  NBJsonParser.h
//  nbframework
//
//  Created by Marcos Ortega on 19/3/19.
//

#ifndef NBZDeflate_h
#define NBZDeflate_h

#include "nb/NBFrameworkDefs.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct ENZDefResult_ {
		int resultCode;
		int inBytesProcessed;
		int outBytesProcessed;
		int inBytesAvailable;
		int outBytesAvailable;
	} ENZDefResult;
	
	typedef enum ENZDefBlckType_ {
		ENZDefBlckType_Partial = 0,
		ENZDefBlckType_Final
	} ENZDefBlckType;
	
	typedef struct STNBZDeflate_ {
		void* opaque;
	} STNBZDeflate;
	
	void NBZDeflate_init(STNBZDeflate* obj);
	void NBZDeflate_release(STNBZDeflate* obj);
	
	BOOL			NBZDeflate_feedStart(STNBZDeflate* obj, const SI8 compLv9);
	ENZDefResult	NBZDeflate_feed(STNBZDeflate* obj, void* dst, const UI32 dstSz, const void* src, const UI32 srcSz, const ENZDefBlckType blckType);
	
#ifdef __cplusplus
}	//extern "C"
#endif

#endif /* NBZDeflate_h */
