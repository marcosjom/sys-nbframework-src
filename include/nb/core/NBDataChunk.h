//
//  NBDataChunk.h
//  thinstream
//
//  Created by Marcos Ortega on 9/3/19.
//

#ifndef NBDataChunk_h
#define NBDataChunk_h

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"

#ifdef __cplusplus
extern "C" {
#endif

//DataPrt

typedef struct STNBDataChunkPtr_ {
	const void*		data;	//data
	UI32			size;	//size in bytes
} STNBDataChunkPtr;

//DataChunk

typedef struct STNBDataChunk_ {
	const void*		data;			//data
	UI32			size;			//size
	STNBObjRef		buff;			//buffer that contains the chunk
	UI32			rdrCount;		//amount of reader accumulated by this chunk, usually 1, except when given to other chunk or already decreased to buffer
	//Optimization (reduces locks and internal data cloning)
	BOOL			preResigned;	//owner flagged internal data as 'free to take by other'
} STNBDataChunk;

//

void NBDataChunk_init(STNBDataChunk* obj);
void NBDataChunk_release(STNBDataChunk* obj);

//Optimization (reduces locks and internal data cloning)
void NBDataChunk_preResign(STNBDataChunk* obj);
BOOL NBDataChunk_absorbResources(STNBDataChunk* obj, STNBDataChunk* other);

//data
BOOL NBDataChunk_setAsOther(STNBDataChunk* obj, const STNBDataChunk* other);
BOOL NBDataChunk_setAsSubchunk(STNBDataChunk* obj, const STNBDataChunk* other, const UI32 iStart, const UI32 dataSz);

#ifdef __cplusplus
} //extern "C"
#endif
		
#endif /* NBDataChunk_h */
