//
//  NBJpeg.h
//  nbframework
//
//  Created by Marcos Ortega on 18/3/19.
//

#ifndef NBJpegWrite_h
#define NBJpegWrite_h

#include "nb/NBFrameworkDefs.h"
#include "nb/2d/NBBitmap.h"
#include "nb/files/NBFile.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef enum ENJpegWriteResult_ {
		ENJpegWriteResult_error = 0,	//error
		ENJpegWriteResult_partial,		//still data to write.
		ENJpegWriteResult_end			//end of data to write.
	} ENJpegWriteResult;
	
	typedef struct STNBJpegWrite_ {
		void* opaque;
	} STNBJpegWrite;
	
	void NBJpegWrite_init(STNBJpegWrite* obj);
	void NBJpegWrite_release(STNBJpegWrite* obj);
	
	BOOL				NBJpegWrite_feedStart(STNBJpegWrite* obj, const void* data, const UI32 dataSz, const STNBBitmapProps dataFmt, STNBFileRef stream, const UI8 quality100, const UI8 smooth100);
	BOOL				NBJpegWrite_feedReset(STNBJpegWrite* obj, const void* data, const UI32 dataSz, const STNBBitmapProps dataFmt, STNBFileRef stream, const UI8 quality100, const UI8 smooth100);
	ENJpegWriteResult	NBJpegWrite_feedWrite(STNBJpegWrite* obj);
	void				NBJpegWrite_feedEnd(STNBJpegWrite* obj);
	void				NBJpegWrite_feedRelease(STNBJpegWrite* obj);
	
#ifdef __cplusplus
}	//extern "C"
#endif


#endif /* NBJpegWrite_h */
