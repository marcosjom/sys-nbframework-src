//
//  NBDataCursor.h
//  thinstream
//
//  Created by Marcos Ortega on 9/3/19.
//

#ifndef NBDataCursor_h
#define NBDataCursor_h

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"
#include "nb/core/NBDataPtr.h"

#ifdef __cplusplus
extern "C" {
#endif

//NBDataCursor is a lightweight chunks provider.
//Each reader/writter should create its own instance for sequential read and/or write (not safe-thread).

//NBDataCursorPart

typedef struct STNBDataCursorPart_ {
	STNBDataPtr	data;
	//hdr
	struct {
		void*	data;				//abstract, depending of the context (example: in UDP-Packets context this could be the UDP-Packet-Address)
		UI32	size;
	} hdr;
	BOOL		canContinueHint;	//more parts can be retrieved (just a hint, sequence should be continued independtly of this hint)
} STNBDataCursorPart;

void NBDataCursorPart_init(STNBDataCursorPart* obj);
void NBDataCursorPart_release(STNBDataCursorPart* obj);
//
void NBDataCursorPart_setDataPtr(STNBDataCursorPart* obj, STNBDataPtr* ptr, void* hdr, const UI32 hdrSz, const BOOL canContinueHint);
void NBDataCursorPart_setDataBytes(STNBDataCursorPart* obj, void* data, const UI32 dataSz, void* hdr, const UI32 hdrSz, const BOOL canContinueHint);

//NBDataCursorBuffDef

typedef struct STNBDataCursorBuffDef_ {
	STNBDataCursorPart	buff;		//continuos chunk of data
	void*				usrData;	//provider internal data
} STNBDataCursorBuffDef;

void NBDataCursorBuffDef_init(STNBDataCursorBuffDef* obj);
void NBDataCursorBuffDef_release(STNBDataCursorBuffDef* obj);
//
void NBDataCursorBuffDef_setDataPtr(STNBDataCursorBuffDef* obj, STNBDataPtr* ptr, void* hdr, const UI32 hdrSz, const BOOL canContinueHint, void* usrData);
void NBDataCursorBuffDef_setDataBytes(STNBDataCursorBuffDef* obj, void* data, const UI32 dataSz, void* hdr, const UI32 hdrSz, const BOOL canContinueHint, void* usrData);

//------------
//- NBDataCursor
//------------

NB_OBJREF_HEADER(NBDataCursor)

//NBDataCursorLstnrItf

typedef struct STNBDataCursorLstnrItf_ {
	void    (*newDataAvailable)(STNBDataCursorRef ref, void* usrData);	//called when data can be readed
} STNBDataCursorLstnrItf;

//NBDataCursorProviderItf

typedef struct STNBDataCursorProviderItf_ {
	void    (*retain)(void* obj); //retains the provider
	void    (*release)(void* obj); //releases the provider
	BOOL	(*isMoreAvailable)(void* obj, const STNBDataCursorBuffDef* prevDef);	//return TRUE if no more buffers are available at the moment.
	BOOL	(*startSequence)(void* obj, STNBDataCursorBuffDef* dst); //initiates a sequential reading
	BOOL	(*continueSequence)(void* obj, STNBDataCursorBuffDef* cur); //continuates a sequential reading
	void	(*endSequence)(void* obj, STNBDataCursorBuffDef* cur); //ends a seuqntial reading
} STNBDataCursorProviderItf;

//config

BOOL NBDataCursor_setProvider(STNBDataCursorRef ref, void* usrData, const STNBDataCursorProviderItf* itf);
BOOL NBDataCursor_setListener(STNBDataCursorRef ref, void* usrData, const STNBDataCursorLstnrItf* itf);

//get

BOOL NBDataCursor_isMoreAvailable(STNBDataCursorRef ref); //return TRUE if no more buffers are available at the moment.
UI32 NBDataCursor_curBuffBytesRemainCount(const STNBDataCursorRef ref);
void NBDataCursor_getNextPart(STNBDataCursorRef ref, STNBDataCursorPart* dst);	//consumes the remaining bytes of the current buffer
void NBDataCursor_getNextPartBytes(STNBDataCursorRef ref, STNBDataCursorPart* dst, const UI32 bytesMax); //consumes n-bytes from the current buffer up to provided the maximun

#ifdef __cplusplus
} //extern "C"
#endif
		
#endif /* NBDataCursor_h */
