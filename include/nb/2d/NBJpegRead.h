//
//  NBJpeg.h
//  nbframework
//
//  Created by Marcos Ortega on 18/3/19.
//

#ifndef NBJpegRead_h
#define NBJpegRead_h

#include "nb/NBFrameworkDefs.h"
#include "nb/2d/NBBitmap.h"
#include "nb/files/NBFile.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef enum ENJpegReadResult_ {
		ENJpegReadResult_error = 0,	//error
		ENJpegReadResult_partial,	//still data to read.
		ENJpegReadResult_end,		//end of data to read (only header or color data).
	} ENJpegReadResult;
	
	typedef struct STNBJpegRead_ {
		void* opaque;
	} STNBJpegRead;
	
	void				NBJpegRead_init(STNBJpegRead* obj);
	void				NBJpegRead_release(STNBJpegRead* obj);
	
	BOOL				NBJpegRead_feedStart(STNBJpegRead* obj, STNBFileRef stream);
	ENJpegReadResult	NBJpegRead_feedRead(STNBJpegRead* obj, BYTE* dst, const UI32 dstSz, UI32* dstLinesReadCount);
	STNBBitmapProps		NBJpegRead_feedGetProps(STNBJpegRead* obj);
	void				NBJpegRead_feedRelease(STNBJpegRead* obj);
	
#ifdef __cplusplus
}	//extern "C"
#endif


#endif /* NBJpegRead_h */
