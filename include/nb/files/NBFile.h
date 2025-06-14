

#ifndef NB_FILE_H
#define NB_FILE_H

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"
#include "nb/core/NBThreadMutex.h"
#include "nb/core/NBString.h"
#include "nb/core/NBIOLnk.h"

#ifdef __cplusplus
extern "C" {
#endif

    //ENNBFileStd

    typedef enum ENNBFileStd_ {
        ENNBFileStd_In = 0,
        ENNBFileStd_Out,
        ENNBFileStd_Err,
        //
        ENNBFileStd_Count
    } ENNBFileStd;

    //ENNBFileMode

	typedef enum ENNBFileMode_ {
		ENNBFileMode_None	= 0,
		ENNBFileMode_Read,	//1
		ENNBFileMode_Write,	//2
		ENNBFileMode_Both	//3
	} ENNBFileMode;
	
    //ENNBFileRelative

	typedef enum ENNBFileRelative_ {
		ENNBFileRelative_Start	= 0,
		ENNBFileRelative_CurPos,
		ENNBFileRelative_End,
	} ENNBFileRelative;
	
	typedef struct IFileItf_ {
		void (*close)(void** obj);
		void (*lock)(void* obj);
		void (*unlock)(void* obj);
		BOOL (*isOpen)(const void* obj);
        BOOL (*setNonBlocking)(void* obj, const BOOL nonBlocking);
		SI32 (*read)(void* obj, void* dst, const UI32 dstSz, const SI64* curPos);
		SI32 (*write)(void* obj, const void* src, const UI32 srcSz, const SI64* curPos);
		BOOL (*seek)(void* obj, const SI64 pos, const ENNBFileRelative relativeTo, const SI64* curPos);
		SI64 (*curPos)(const void* obj);
		BOOL (*flush)(void* obj);
        int  (*getFD)(void* obj);
	} IFileItf;

	//

    NB_OBJREF_HEADER(NBFile)
	
	//Dbg
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	void NBFile_dbgSetPathRef(STNBFileRef obj, const char* pathRef);
#	endif
	
	BOOL NBFilepath_isExtension(const char* filepath, const char* extension);
	
	//Open
	BOOL NBFile_open(STNBFileRef obj, const char* filepath, const ENNBFileMode mode);
    BOOL NBFile_openAsStd(STNBFileRef obj, const ENNBFileStd std);
	BOOL NBFile_openAsFileRng(STNBFileRef obj, STNBFileRef parent, const UI32 start, const UI32 size);
	BOOL NBFile_openAsDataRng(STNBFileRef obj, void* data, const UI32 dataSz);
	BOOL NBFile_openAsString(STNBFileRef obj, struct STNBString_* str);
	BOOL NBFile_openAsItf(STNBFileRef obj, const IFileItf* itf, void* itfObj);
	BOOL NBFile_isOpen(STNBFileRef obj);
	void NBFile_close(STNBFileRef obj);
    int  NBFile_getFD(STNBFileRef obj);
	SI64 NBFile_getSize(const char* filepath);

    //
    BOOL NBFile_getIOLnk(STNBFileRef ref, STNBIOLnk* dst);

	//Mutex
	void NBFile_lock(STNBFileRef obj);
	void NBFile_unlock(STNBFileRef obj);

    //Config (must be open)
    BOOL NBFile_setNonBlocking(STNBFileRef obj, const BOOL nonBlocking);    //non-blocking mode
	
	//Actions (must be locked)
	SI32 NBFile_read(STNBFileRef obj, void* dst, const UI32 dstSz);
	SI32 NBFile_write(STNBFileRef obj, const void* src, const UI32 srcSz);
	SI64 NBFile_curPos(STNBFileRef obj);
	BOOL NBFile_seek(STNBFileRef obj, const SI64 pos, const ENNBFileRelative relativeTo);
	BOOL NBFile_flush(STNBFileRef obj);
	//SI32 NBFile_truncateToZero(STNBFileRef obj);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
