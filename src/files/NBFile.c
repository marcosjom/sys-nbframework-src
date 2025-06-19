

#include "nb/NBFrameworkPch.h"
//Complementary PCH
#ifdef _WIN32
//#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>		//for HANDLE and more
#else
#	include <errno.h>		//for errno
#	include <string.h>		//for strerror()
#endif
//
#include <stdio.h>		//for FILE
//
#ifdef NB_IS_POSIX
//#   include <sys/types.h>
//#   include <sys/stat.h>
#   include <fcntl.h>     //POSIX header
#   include <unistd.h>    //for STDIN_FILENO, STDOUT_FILENO, and STDERR_FILENO
#endif
//
#include "nb/files/NBFile.h"
#include "nb/core/NBMemory.h"
#include "nb/core/NBHndl.h"

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
#	include "nb/core/NBMngrProcess.h"
#endif

//#include <io.h> //for "_chsize_s"

//Factory

#define NBFILE_MUTEX_ACTIVATE(OPQ) NBObject_lock(OPQ); NBASSERT(!OPQ->mutexLckd) OPQ->mutexLckd = TRUE;
#define NBFILE_MUTEX_DEACTIVATE(OPQ) NBASSERT(OPQ->mutexLckd) OPQ->mutexLckd = FALSE; NBObject_unlock(OPQ);

#ifdef NB_CONFIG_INCLUDE_ASSERTS
#	define NBFILE_MUTEX_ASSERT_LOCKED(OPQ)  { if(!(OPQ)->mutexLckd){ PRINTF_ERROR("Expected file to be locked: '%s'.\n", (OPQ)->dbgPathRef.str); NBASSERT(FALSE) } }
#   define NBFILE_MUTEX_ASSERT_UNLOCKED(OPQ) { if((OPQ)->mutexLckd){ PRINTF_ERROR("Expected file to be unlocked: '%s'.\n", (OPQ)->dbgPathRef.str); NBASSERT(FALSE) } }
#else
#	define NBFILE_MUTEX_ASSERT_LOCKED(OPQ)  NBASSERT((OPQ)->mutexLckd)
#   define NBFILE_MUTEX_ASSERT_UNLOCKED(OPQ) NBASSERT(!(OPQ)->mutexLckd)
#endif

//HANDLE/INVALID_HANDLE_VALUE CReateFile(strPath, desiredAccess(GENERIC_READ | GENERIC_WRITE), shareMode(0), securityAttributes(NULL), creationDisposition(OPEN_ALWAYS), flagsAndAttributes(FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS), templateFile(NULL))
//0 != SetFilePointerEx(HANDLE hFile, LARGE_INTEGER offset, LARGE_INTEGER* dstNewOffset, FILE_BEGIN | FILE_CURRENT | FILE_END)
//BOOL = ReadFile(HANDLE hFile, void* dstBuffer, bytesToRead, DWORD* bytesReaded, NULL)
//BOOL = WriteFile(HANDLE hFile, void* srcBuffer, bytesToWrite, DWORD* bytesWritten, NULL)
//BOOL = FlushFileBuffers(HANDLE hFile)
//BOOL = SetEndOfFile(HANDLE hFile)
//CloseHandle(HANDLE hFile)

//Enable file position validation
//#define NB_CONFIG_INCLUDE_ASSERTS_FILE_POSITION

typedef struct STNBFileOpq_ {
    STNBObject      prnt;
    //
	IFileItf		itf;
	void*			itfObj;
    //
	BOOL			mutexLckd;
	SI64			curPos;
	ENNBFileMode	lastOp;
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	STNBString		dbgPathRef;
#	endif
} STNBFileOpq;

NB_OBJREF_BODY(NBFile, STNBFileOpq, NBObject)

//--------------
//- Native itf -
//--------------

typedef struct STNBFileNative_ {
    STNBHndlRef hndl; //handle/file-descriptor
    //nat; optimization, copy of handle/file-descriptor (this source-file should keep 'hndl' and 'hndlNative' in sync)
    struct {
#       ifdef NB_IS_POSIX
        int     fd;     //file descriptor (POSIX only)
#       else
#           ifdef _WIN32
#               ifndef NB_COMPILE_DRIVER_MODE
        HANDLE  hndl;
#               else
        BOOL    hndlDumy;
#               endif
#           else
        FILE*   stream; //c-standar
#           endif
#       endif
    } nat;
} STNBFileNative;

#ifdef NB_IS_POSIX
//----------------------
//- Native itf (native file, POSIX)
//----------------------
BOOL NBFile_NBHndlCloseFnc_posix_(STNBHndlNative* obj);
void NBFile_close_(void** obj);
void NBFile_closeStd_posix_(void** obj);
//void NBFile_lock_posix_(void* obj);
//void NBFile_unlock_posix_(void* obj);
BOOL NBFile_isOpen_posix_(const void* obj);
BOOL NBFile_setNonBlocking_posix_(void* obj, const BOOL nonBlocking);
SI32 NBFile_read_posix_(void* obj, void* dst, const UI32 dstSz, const SI64* curPos);
SI32 NBFile_write_posix_(void* obj, const void* src, const UI32 srcSz, const SI64* curPos);
BOOL NBFile_seek_posix_(void* obj, const SI64 pos, const ENNBFileRelative relativeTo, const SI64* curPos);
SI64 NBFile_curPos_posix_(const void* obj);
BOOL NBFile_flush_posix_(void* obj);
int  NBFile_getFD_posix_(void* obj);
#else
//----------------------
//- Native itf (native file, c-standard)
//----------------------
BOOL NBFile_NBHndlCloseFnc_(STNBHndlNative* obj);
void NBFile_close_(void** obj);
void NBFile_closeStd_(void** obj);
//void NBFile_lock_(void* obj);
//void NBFile_unlock_(void* obj);
BOOL NBFile_isOpen_(const void* obj);
SI32 NBFile_read_(void* obj, void* dst, const UI32 dstSz, const SI64* curPos);
SI32 NBFile_write_(void* obj, const void* src, const UI32 srcSz, const SI64* curPos);
BOOL NBFile_seek_(void* obj, const SI64 pos, const ENNBFileRelative relativeTo, const SI64* curPos);
SI64 NBFile_curPos_(const void* obj);
BOOL NBFile_flush_(void* obj);
int  NBFile_getFD_(void* obj);
#endif

//----------------------
//- Native itf (file-rng)
//----------------------

typedef struct STNBFileRng_ {
    STNBFileRef parent;
    struct {
        UI32    start;
        UI32    size;
    } rng;
    SI64        virtualPos;        //Updatedat lock
} STNBFileRng;

void NBFile_close_fileRng_(void** obj);
void NBFile_lock_fileRng_(void* obj);
void NBFile_unlock_fileRng_(void* obj);
BOOL NBFile_isOpen_fileRng_(const void* obj);
SI32 NBFile_read_fileRng_(void* obj, void* dst, const UI32 dstSz, const SI64* curPos);
SI32 NBFile_write_fileRng_(void* obj, const void* src, const UI32 srcSz, const SI64* curPos);
BOOL NBFile_seek_fileRng_(void* obj, const SI64 pos, const ENNBFileRelative relativeTo, const SI64* curPos);
SI64 NBFile_curPos_fileRng_(const void* obj);
BOOL NBFile_flush_fileRng_(void* obj);

//----------------------
//- Native itf (data-rng)
//----------------------

typedef struct STNBFileDataRng_ {
    BYTE*   data;
    UI32    dataSz;
    SI64    virtualPos;    //Updated at lock
} STNBFileDataRng;

void NBFile_close_dataRng_(void** obj);
//void NBFile_lock_dataRng_(void* obj);
//void NBFile_unlock_dataRng_(void* obj);
BOOL NBFile_isOpen_dataRng_(const void* obj);
SI32 NBFile_read_dataRng_(void* obj, void* dst, const UI32 dstSz, const SI64* curPos);
SI32 NBFile_write_dataRng_(void* obj, const void* src, const UI32 srcSz, const SI64* curPos);
BOOL NBFile_seek_dataRng_(void* obj, const SI64 pos, const ENNBFileRelative relativeTo, const SI64* curPos);
SI64 NBFile_curPos_dataRng_(const void* obj);
BOOL NBFile_flush_dataRng_(void* obj);

//----------------------
//- Native itf (string)
//----------------------

typedef struct STNBFileString_ {
    STNBString* str;
    SI64        virtualPos;    //Updated at lock
} STNBFileString;

void NBFile_close_str_(void** obj);
//void NBFile_lock_str_(void* obj);
//void NBFile_unlock_str_(void* obj);
BOOL NBFile_isOpen_str_(const void* obj);
SI32 NBFile_read_str_(void* obj, void* dst, const UI32 dstSz, const SI64* curPos);
SI32 NBFile_write_str_(void* obj, const void* src, const UI32 srcSz, const SI64* curPos);
BOOL NBFile_seek_str_(void* obj, const SI64 pos, const ENNBFileRelative relativeTo, const SI64* curPos);
SI64 NBFile_curPos_str_(const void* obj);
BOOL NBFile_flush_str_(void* obj);


//IOLnk
int  NBFile_ioGetFD_(void* usrData);
BOOL NBFile_ioIsObjRef_(STNBObjRef objRef, void* usrData); //to determine if the io is an specific socket
SI32 NBFile_ioRead_(void* dst, const SI32 dstSz, void* usrData); //read data to destination buffer, returns the ammount of bytes read, negative in case of error
SI32 NBFile_ioWrite_(const void* src, const SI32 srcSz, void* usrData); //write data from source buffer, returns the ammount of bytes written, negative in case of error
void NBFile_ioFlush_(void* usrData);
void NBFile_ioShutdown_(const UI8 mask, void* usrData); //NB_IO_BIT_READ | NB_IO_BIT_WRITE
void NBFile_ioClose_(void* usrData);

//----------------------
//- NBFile
//----------------------

void NBFile_initZeroed(STNBObject* obj) {
    STNBFileOpq* opq    = (STNBFileOpq*)obj;
    //
    opq->mutexLckd      = FALSE;
    opq->curPos         = 0;
    opq->lastOp         = ENNBFileMode_None;
    IF_NBASSERT(NBString_init(&opq->dbgPathRef);)
}

void NBFile_uninitLocked(STNBObject* obj){
    STNBFileOpq* opq = (STNBFileOpq*)obj;
    //
    //PRINTF_INFO("NBFile, NBFile_uninitLocked opq(%lld) itfObj(%lld) '%s'.\n", (SI64)opq, (SI64)opq->itfObj, opq->dbgPathRef.str);
    //Close (if not-a-warpper)
    if(opq->itf.close != NULL){
        (*opq->itf.close)(&opq->itfObj);
    }
    //Invalidate itf
    {
        NBMemory_set(&opq->itf, 0, sizeof(opq->itf));
        opq->itfObj = NULL;
    }
    //Unlock
    IF_NBASSERT(NBString_release(&opq->dbgPathRef);)
}

//Mutex

void NBFile_lockOpq_(STNBFileOpq* opq){
    NBFILE_MUTEX_ACTIVATE(opq)
    //Verify position
#   ifdef NB_CONFIG_INCLUDE_ASSERTS_FILE_POSITION
    if(opq->itf.curPos != NULL){
        const SI64 curPos = (*opq->itf.curPos)(opq->itfObj);
        NBASSERT(opq->curPos == curPos);
    }
#   endif
    if(opq->itf.lock != NULL){
        (*opq->itf.lock)(opq->itfObj);
    }
    if(opq->itf.seek != NULL){
        (*opq->itf.seek)(opq->itfObj, opq->curPos, ENNBFileRelative_Start, NULL);
    }
}

void NBFile_unlockOpq_(STNBFileOpq* opq){
    if(opq->itf.unlock != NULL){
        (*opq->itf.unlock)(opq->itfObj);
    }
    NBFILE_MUTEX_DEACTIVATE(opq)
}

void NBFile_lock(STNBFileRef obj){
	STNBFileOpq* opq = (STNBFileOpq*)obj.opaque; NBASSERT(NBFile_isClass(obj));
    NBFile_lockOpq_(opq);
}

void NBFile_unlock(STNBFileRef obj){
	STNBFileOpq* opq = (STNBFileOpq*)obj.opaque; NBASSERT(NBFile_isClass(obj));
    NBFile_unlockOpq_(opq);
}

//Dbg
#ifdef NB_CONFIG_INCLUDE_ASSERTS
void NBFile_dbgSetPathRef(STNBFileRef obj, const char* pathRef){
	STNBFileOpq* opq = (STNBFileOpq*)obj.opaque; NBASSERT(NBFile_isClass(obj));
	NBString_set(&opq->dbgPathRef, pathRef);
}
#endif

//

BOOL NBFilepath_isExtension(const char* filepath, const char* pExtension){
	BOOL r = FALSE;
	const char* pExt = pExtension;
	while(*pExt == '.') pExt++;
	{
		const UI32 lenExt	= NBString_strLenBytes(pExt);
		const UI32 lenPath	= NBString_strLenBytes(filepath);
		if(lenExt < lenPath){
			const SI32 lastPnt = NBString_strLastIndexOf(filepath, ".", lenPath - 1);
			if(lastPnt >= 0 && lastPnt < lenPath){
				const UI32 lenPathExt = (lenPath - lastPnt - 1);
				if(lenPathExt == lenExt){
					r = NBString_strIsLikeStrBytes(pExt, &filepath[lastPnt + 1], lenExt);
				}
			}
		}
	}
	return r;
}

//Actions

//STNBFileNative

#ifdef NB_IS_POSIX
BOOL NBFile_NBHndlCloseFnc_posix_(STNBHndlNative* obj){
    BOOL r = FALSE;
    int fd = 0;
    if(NBHndlNative_get(obj, &fd, sizeof(fd))){
        if(0 != close(fd)){
            PRINTF_ERROR("NBFile, 'close' failed with errno(%d).\n", errno);
        } else {
            fd = 0;
            r = TRUE;
        }
    }
#   ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
    NBMngrProcess_fsFileClosed(ENNBFilesystemStatsSrc_Native, r ? ENNBFilesystemStatsResult_Success : ENNBFilesystemStatsResult_Error);
#   endif
    return r;
}
#endif

#ifndef NB_IS_POSIX
BOOL NBFile_NBHndlCloseFnc_(STNBHndlNative* obj){
    BOOL r = FALSE;
#   ifdef _WIN32
#       ifndef NB_COMPILE_DRIVER_MODE
        {
            HANDLE hndl = 0;
            if(NBHndlNative_get(obj, &hndl, sizeof(hndl))){
                if(0 != CloseHandle(hndl)){
                    PRINTF_ERROR("NBFile, 'CloseHandle' failed.\n");
                } else {
                    hndl = 0;
                    r = TRUE;
                }
            }
        }
#       else
        {
            //nothing
            r = TRUE;
        }
#       endif
#   else
    {
        FILE* f = NULL;
        if(NBHndlNative_get(obj, &f, sizeof(f))){
            if(0 != fclose(f)){
                PRINTF_ERROR("NBFile, 'fclose' failed.\n");
            } else {
                hndl = 0;
                r = TRUE;
            }
        }
    }
#   endif
#   ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
    NBMngrProcess_fsFileClosed(ENNBFilesystemStatsSrc_Native, r ? ENNBFilesystemStatsResult_Success : ENNBFilesystemStatsResult_Error);
#   endif
    return r;
}
#endif

BOOL NBFile_open(STNBFileRef obj, const char* filepath, const ENNBFileMode mode){
	BOOL r = FALSE;
	STNBFileOpq* opq = (STNBFileOpq*)obj.opaque; NBASSERT(NBFile_isClass(obj));
	NBFILE_MUTEX_ACTIVATE(opq)
	NBASSERT(opq->itfObj == NULL)
	if(opq->itfObj == NULL){
#       ifdef NB_IS_POSIX
        {
            int fmode = (mode == ENNBFileMode_Read ? O_RDONLY /*rb*/: mode == ENNBFileMode_Write ? O_WRONLY | O_CREAT | O_TRUNC /*wb+*/ : mode == ENNBFileMode_Both ? O_RDWR | O_CREAT /*ab+*/: 0);
            int fd = open(filepath, fmode, 0666); //rw, rw, rw
            if(fd < 0){ //fd '0' is valid
                if(errno != 2 || mode != ENNBFileMode_Read){
                    IF_PRINTF(int errnum = errno;)
                    int fmode2 = fmode;
                    STNBString str;
                    NBString_init(&str);
                    //#define O_RDONLY        0x0000          /* open for reading only */
                    //#define O_WRONLY        0x0001          /* open for writing only */
                    //#define O_RDWR          0x0002          /* open for reading and writing */
                    //#define O_ACCMODE       0x0003          /* mask for above modes */
                    if((fmode2 & 0xFF) == O_RDONLY){
                        if(str.length > 0) NBString_concat(&str, ", ");
                        NBString_concat(&str, "O_RDONLY");
                        fmode2 &= ~O_RDONLY;
                    }
                    if(((fmode2 & 0xFF) == O_WRONLY)){
                        if(str.length > 0) NBString_concat(&str, ", ");
                        NBString_concat(&str, "O_WRONLY");
                        fmode2 &= ~O_WRONLY;
                    }
                    if(((fmode2 & 0xFF) == O_RDWR)){
                        if(str.length > 0) NBString_concat(&str, ", ");
                        NBString_concat(&str, "O_RDWR");
                        fmode2 &= ~O_RDWR;
                    }
                    if(((fmode2 & 0xFF) == O_ACCMODE)){
                        if(str.length > 0) NBString_concat(&str, ", ");
                        NBString_concat(&str, "O_ACCMODE");
                        fmode2 &= ~O_RDWR;
                    }
                    //
                    //#define O_CREAT         0x00000200      /* create if nonexistant */
                    //#define O_TRUNC         0x00000400      /* truncate to zero length */
                    //#define O_EXCL          0x00000800      /* error if already exists */
                    if((fmode2 & O_CREAT)){
                        if(str.length > 0) NBString_concat(&str, ", ");
                        NBString_concat(&str, "O_CREAT");
                        fmode2 &= ~O_CREAT;
                    }
                    if((fmode2 & O_TRUNC)){
                        if(str.length > 0) NBString_concat(&str, ", ");
                        NBString_concat(&str, "O_TRUNC");
                        fmode2 &= ~O_TRUNC;
                    }
                    if((fmode2 & O_EXCL)){
                        if(str.length > 0) NBString_concat(&str, ", ");
                        NBString_concat(&str, "O_EXCL");
                        fmode2 &= ~O_EXCL;
                    }
                    //Others
                    if(fmode2 != 0){
                        if(str.length > 0) NBString_concat(&str, ", ");
                        NBString_concat(&str, "others");
                        fmode2 &= ~fmode2;
                    }
                    PRINTF_ERROR("fopen returned errno(%d)('%s') for: '%s' file-mode(%d: %s).\n", errnum, strerror(errnum), filepath, fmode, str.str);
                    NBString_release(&str);
                    
                }
            } else {
                //move to the end of the file
                if((fmode & O_RDWR) /*ab+*/){
                    const off_t curPos = lseek(fd, 0, SEEK_END);
                    if(curPos < 0){
                        //error
                        PRINTF_ERROR("fopen(O_RDWR), lseek(0, SEEK_END) returned errno(%d)('%s') for: '%s'.\n", errno, strerror(errno), filepath, fmode);
                        close(fd);
                        fd = -1; //fd '0' is valid
                    } else {
                        opq->curPos = curPos;
                    }
                }
                //result
                if(fd >= 0){ //fd '0' is valid
                    STNBHndlRef hndl = NBHndl_alloc(NULL);
                    if(!NBHndl_setNative(hndl, ENNBHndlNativeType_FilePosix, &fd, sizeof(fd), NBFile_NBHndlCloseFnc_posix_)){
                        //error
                        PRINTF_ERROR("ERROR, NBHndl_setNative failed for: '%s'.\n", filepath);
                        close(fd);
                        fd = -1; //fd '0' is valid
                    } else {
                        STNBFileNative* nat = NBMemory_allocType(STNBFileNative);
                        NBMemory_setZeroSt(*nat, STNBFileNative);
                        //
                        nat->hndl           = hndl; NBHndl_null(&hndl); //consume
                        nat->nat.fd         = fd;
                        //
                        opq->itf.close      = NBFile_close_;
                        //opq->itf.lock     = NBFile_lock_posix_;
                        //opq->itf.unlock   = NBFile_unlock_posix_;
                        opq->itf.isOpen     = NBFile_isOpen_posix_;
                        opq->itf.setNonBlocking = NBFile_setNonBlocking_posix_;
                        opq->itf.read       = NBFile_read_posix_;
                        opq->itf.write      = NBFile_write_posix_;
                        opq->itf.seek       = NBFile_seek_posix_;
                        opq->itf.curPos     = NBFile_curPos_posix_;
                        opq->itf.flush      = NBFile_flush_posix_;
                        opq->itf.getFD      = NBFile_getFD_posix_;
                        opq->itfObj         = nat;
                        {
                            IF_NBASSERT(NBString_set(&opq->dbgPathRef, filepath);)
                        }
                        //
                        r = TRUE;
                    }
                    //release (if not conumed)
                    if(NBHndl_isSet(hndl)){
                        NBHndl_release(&hndl);
                        NBHndl_null(&hndl);
                    }
                }
            }
        }
#       else
#		    ifdef _WIN32
#			    ifndef NB_COMPILE_DRIVER_MODE
                {
                    DWORD desiredAccess		= (mode == ENNBFileMode_Read ? GENERIC_READ : mode == ENNBFileMode_Write ? GENERIC_WRITE : mode == ENNBFileMode_Both ? GENERIC_READ | GENERIC_WRITE : 0);
                    DWORD shareMode			= (mode == ENNBFileMode_Read ? FILE_SHARE_READ : mode == ENNBFileMode_Write ? 0 : mode == ENNBFileMode_Both ? 0 : 0);
                    LPSECURITY_ATTRIBUTES secAtt = NULL;
                    DWORD creationDisp		= (mode == ENNBFileMode_Read ? OPEN_EXISTING : mode == ENNBFileMode_Write ? CREATE_ALWAYS : mode == ENNBFileMode_Both ? OPEN_ALWAYS : OPEN_EXISTING);
                    DWORD flagsAndAttrb		= FILE_ATTRIBUTE_NORMAL;
                    HANDLE templateFile		= NULL;
                    HANDLE f = CreateFileA(filepath, desiredAccess, shareMode, secAtt, creationDisp, flagsAndAttrb, templateFile);
                    if(f == INVALID_HANDLE_VALUE){
                        DWORD err = GetLastError();
                        LPTSTR errorText = NULL;
                        FormatMessage(
                           // use system message tables to retrieve error text
                           FORMAT_MESSAGE_FROM_SYSTEM
                           // allocate buffer on local heap for error text
                           |FORMAT_MESSAGE_ALLOCATE_BUFFER
                           // Important! will fail otherwise, since we're not
                           // (and CANNOT) pass insertion parameters
                           |FORMAT_MESSAGE_IGNORE_INSERTS,
                           NULL,    // unused with FORMAT_MESSAGE_FROM_SYSTEM
                           err,
                           MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                           (LPTSTR)&errorText,  // output
                           0, // minimum size for output buffer
                           NULL);   // arguments - see note
                        if ( NULL != errorText ){
                           // ... do something with the string `errorText` - log it, display it to the user, etc.
                            PRINTF_ERROR("NBFile_open failed with error: '%s'.\n", errorText);
                           // release memory allocated by FormatMessage()
                           LocalFree(errorText);
                           errorText = NULL;
                        } else {
                            PRINTF_ERROR("NBFile_open failed with unknow error.\n");
                        }
                    } else {
                        STNBHndlRef hndl = NBHndl_alloc(NULL);
                        if(!NBHndl_setNative(hndl, ENNBHndlNativeType_FileWin, &f, sizeof(f), NBFile_NBHndlCloseFnc_)){
                            //error
                            PRINTF_ERROR("ERROR, NBHndl_setNative failed for: '%s'.\n", filepath);
                            CloseHandle(f);
                            f = 0;
                        } else {
                            STNBFileNative* nat	= NBMemory_allocType(STNBFileNative);
                            NBMemory_setZeroSt(*nat, STNBFileNative);
                            //
                            nat->hndl           = hndl; NBHndl_null(&hndl); //consume
                            nat->nat.hndl       = f;
                            //
                            opq->itf.close      = NBFile_close_;
                            //opq->itf.lock     = NBFile_lock_;
                            //opq->itf.unlock   = NBFile_unlock_;
                            opq->itf.isOpen     = NBFile_isOpen_;
                            opq->itf.read       = NBFile_read_;
                            opq->itf.write      = NBFile_write_;
                            opq->itf.seek       = NBFile_seek_;
                            opq->itf.curPos     = NBFile_curPos_;
                            opq->itf.flush      = NBFile_flush_;
                            opq->itf.getFD      = NBFile_getFD_;
                            opq->itfObj			= nat;
                            //
                            r = TRUE;
                        }
                        //release (if not conumed)
                        if(NBHndl_isSet(hndl)){
                            NBHndl_release(&hndl);
                            NBHndl_null(&hndl);
                        }
                    }
                }
#			    else
                {
                    //Windows / driver-mode
                    //nothing
                }
#			    endif
#		    else
            {
                const char* fmode = (mode == ENNBFileMode_Read ? "rb" : mode == ENNBFileMode_Write ? "wb+" : mode == ENNBFileMode_Both ? "ab+" : "");
                FILE* f = fopen(filepath, fmode);
                if(f == NULL){
                    if(errno != 2 || mode != ENNBFileMode_Read){
                        IF_PRINTF(int errnum = errno;)
                        PRINTF_ERROR("fopen returned errno(%d)('%s') for: '%s' (%s).\n", errnum, strerror(errnum), filepath, fmode);
                    }
                } else {
                    STNBHndlRef hndl = NBHndl_alloc(NULL);
                    if(!NBHndl_setNative(hndl, ENNBHndlNativeType_FileStd, &f, sizeof(f), NBFile_NBHndlCloseFnc_)){
                        //error
                        PRINTF_ERROR("ERROR, NBHndl_setNative failed for: '%s'.\n", filepath);
                        fclose(f);
                        f = 0;
                    } else {
                        STNBFileNative* nat	= NBMemory_allocType(STNBFileNative);
                        NBMemory_setZeroSt(*nat, STNBFileNative);
                        //
                        nat->hndl           = hndl; NBHndl_null(&hndl); //consume
                        nat->nat.stream     = f;
                        //
                        opq->itf.close      = NBFile_close_;
                        //opq->itf.lock     = NBFile_lock_;
                        //opq->itf.unlock   = NBFile_unlock_;
                        opq->itf.isOpen     = NBFile_isOpen_;
                        opq->itf.read       = NBFile_read_;
                        opq->itf.write      = NBFile_write_;
                        opq->itf.seek       = NBFile_seek_;
                        opq->itf.curPos     = NBFile_curPos_;
                        opq->itf.flush      = NBFile_flush_;
                        opq->itf.getFD      = NBFile_getFD_;
                        opq->itfObj			= nat;
                        {
                            IF_NBASSERT(NBString_set(&opq->dbgPathRef, filepath);)
                        }
                        r = TRUE;
                    }
                    //release (if not conumed)
                    if(NBHndl_isSet(hndl)){
                        NBHndl_release(&hndl);
                        NBHndl_null(&hndl);
                    }
                }
            }
#		    endif
#       endif
	}
#   ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
    NBMngrProcess_fsFileOpened(ENNBFilesystemStatsSrc_Native, r ? ENNBFilesystemStatsResult_Success : ENNBFilesystemStatsResult_Error);
#   endif
	NBFILE_MUTEX_DEACTIVATE(opq)
	return r;
}

BOOL NBFile_openAsStd(STNBFileRef obj, const ENNBFileStd std){
    BOOL r = FALSE;
    if(std >= 0 && std < ENNBFileStd_Count){
        STNBFileOpq* opq = (STNBFileOpq*)obj.opaque; NBASSERT(NBFile_isClass(obj));
        NBFILE_MUTEX_ACTIVATE(opq)
        NBASSERT(opq->itfObj == NULL)
        if(opq->itfObj == NULL){
#       ifdef NB_IS_POSIX
        {
            int fd = -1; const char* filename = "";
            //
            switch (std) {
                case ENNBFileStd_In: fd = STDIN_FILENO; filename = "stdin"; break;
                case ENNBFileStd_Out: fd = STDOUT_FILENO; filename = "stdout"; break;
                case ENNBFileStd_Err: fd = STDERR_FILENO; filename = "stderr"; break;
                default: break;
            }
            //
            if(fd >= 0){
                STNBFileNative* nat = NBMemory_allocType(STNBFileNative);
                NBMemory_setZeroSt(*nat, STNBFileNative);
                //
                nat->nat.fd         = fd;
                //
                opq->itf.close      = NBFile_closeStd_posix_;
                //opq->itf.lock     = NBFile_lock_posix_;
                //opq->itf.unlock   = NBFile_unlock_posix_;
                opq->itf.isOpen     = NBFile_isOpen_posix_;
                opq->itf.setNonBlocking = NBFile_setNonBlocking_posix_;
                opq->itf.read       = NBFile_read_posix_;
                opq->itf.write      = NBFile_write_posix_;
                opq->itf.seek       = NBFile_seek_posix_;
                opq->itf.curPos     = NBFile_curPos_posix_;
                opq->itf.flush      = NBFile_flush_posix_;
                opq->itf.getFD      = NBFile_getFD_posix_;
                opq->itfObj         = nat;
                {
                    IF_NBASSERT(NBString_set(&opq->dbgPathRef, filename);)
                }
                r = TRUE;
            }
        }
#       else
#           ifdef _WIN32
#               ifndef NB_COMPILE_DRIVER_MODE
                {
                    NBASSERT(FALSE) //ToDo: implement
                }
#               else
                {
                    //nothing
                }
#               endif
#           else
            {
                FILE* f = NULL; const char* filename = "";
                //
                switch (std) {
                    case ENNBFileStd_In: f = stdin; filename = "stdin"; break;
                    case ENNBFileStd_Out: f = stdout; filename = "stdout"; break;
                    case ENNBFileStd_Err: f = stderr; filename = "stderr"; break;
                    default: break;
                }
                //
                if(f != NULL){
                    STNBFileNative* nat = NBMemory_allocType(STNBFileNative);
                    NBMemory_setZeroSt(*nat, STNBFileNative);
                    //
                    nat->nat.stream         = f; //PRINTF_INFO("NBFile, opq(%lld) nat(%lld) handle '%s' is %lld.\n", (SI64)opq, (SI64)nat, filename, (SI64)f);
                    //
                    opq->itf.close      = NBFile_closeStd_; //special 'close' action (do not close std)
                    //opq->itf.lock     = NBFile_lock_;
                    //opq->itf.unlock   = NBFile_unlock_;
                    opq->itf.isOpen     = NBFile_isOpen_;
                    opq->itf.read       = NBFile_read_;
                    opq->itf.write      = NBFile_write_;
                    //opq->itf.seek       = NBFile_seek_;   //not allowed to seek
                    opq->itf.curPos     = NBFile_curPos_;   //not allowed to
                    opq->itf.flush      = NBFile_flush_;
                    opq->itf.getFD      = NBFile_getFD_;
                    opq->itfObj         = nat;
                    {
                        IF_NBASSERT(NBString_set(&opq->dbgPathRef, filename);)
                    }
                    r = TRUE;
                }
            }
#           endif
#       endif
        }
        NBFILE_MUTEX_DEACTIVATE(opq)
    }
#   ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
    NBMngrProcess_fsFileOpened(ENNBFilesystemStatsSrc_Native, r ? ENNBFilesystemStatsResult_Success : ENNBFilesystemStatsResult_Error);
#   endif
    return r;
}

//STNBFileRng

BOOL NBFile_openAsFileRng(STNBFileRef obj, STNBFileRef parent, const UI32 start, const UI32 size){
	BOOL r = FALSE;
	STNBFileOpq* opq = (STNBFileOpq*)obj.opaque; NBASSERT(NBFile_isClass(obj));
    NBFILE_MUTEX_ACTIVATE(opq)
	NBASSERT(opq->itfObj == NULL && NBFile_isSet(parent))
	if(opq->itfObj == NULL && NBFile_isSet(parent)){
        NBASSERT(NBFile_isClass(parent))
        NBFILE_MUTEX_ASSERT_UNLOCKED((STNBFileOpq*)parent.opaque) //Parent must be unlocked from user
        //
		IFileItf itf;
		NBMemory_setZeroSt(itf, IFileItf);
		itf.close		= NBFile_close_fileRng_;
		itf.lock		= NBFile_lock_fileRng_;
		itf.unlock		= NBFile_unlock_fileRng_;
		itf.isOpen		= NBFile_isOpen_fileRng_;
		itf.read		= NBFile_read_fileRng_;
		itf.write		= NBFile_write_fileRng_;
		itf.seek		= NBFile_seek_fileRng_;
		itf.curPos		= NBFile_curPos_fileRng_;
		itf.flush		= NBFile_flush_fileRng_;
        {
            STNBFileRng* nat = NBMemory_allocType(STNBFileRng);
            NBMemory_setZeroSt(*nat, STNBFileRng);
            nat->parent		= parent; if(NBFile_isSet(parent)) NBFile_retain(parent);
            nat->rng.start	= start;
            nat->rng.size	= size;
            nat->virtualPos	= 0;
            //
            opq->itf        = itf;
            opq->itfObj     = nat;
        }
		//
#		ifdef NB_CONFIG_INCLUDE_ASSERTS
		{
			STNBFileOpq* opq2 = (STNBFileOpq*)parent.opaque;
			NBString_setBytes(&opq->dbgPathRef, opq2->dbgPathRef.str, opq2->dbgPathRef.length);
			if(opq->dbgPathRef.length > 0){
				NBString_concat(&opq->dbgPathRef, " ");
			}
			NBString_concat(&opq->dbgPathRef, "rng[");
			NBString_concatUI32(&opq->dbgPathRef, start);
			NBString_concat(&opq->dbgPathRef, ", +");
			NBString_concatUI32(&opq->dbgPathRef, size);
			NBString_concat(&opq->dbgPathRef, "]");
		}
#		endif
		//
		r				= TRUE;
	}
	NBFILE_MUTEX_DEACTIVATE(opq)
	return r;
}

//STNBFileDataRng

BOOL NBFile_openAsDataRng(STNBFileRef obj, void* data, const UI32 dataSz){
	BOOL r = FALSE;
	STNBFileOpq* opq = (STNBFileOpq*)obj.opaque; NBASSERT(NBFile_isClass(obj));
	NBFILE_MUTEX_ACTIVATE(opq)
	NBASSERT(opq->itfObj == NULL)
	if(opq->itfObj == NULL){
		IFileItf itf;
		NBMemory_setZeroSt(itf, IFileItf);
		itf.close		= NBFile_close_dataRng_;
		//itf.lock		= NBFile_lock_dataRng_;
		//itf.unlock	= NBFile_unlock_dataRng_;
		itf.isOpen		= NBFile_isOpen_dataRng_;
		itf.read		= NBFile_read_dataRng_;
		itf.write		= NBFile_write_dataRng_;
		itf.seek		= NBFile_seek_dataRng_;
		itf.curPos		= NBFile_curPos_dataRng_;
		itf.flush		= NBFile_flush_dataRng_;
		{
			STNBFileDataRng* nat = NBMemory_allocType(STNBFileDataRng);
            NBMemory_setZeroSt(*nat, STNBFileDataRng);
			nat->data		= (BYTE*)data;
			nat->dataSz		= dataSz;
			nat->virtualPos	= 0;
			opq->itfObj		= nat;
		}
		//
		opq->itf		= itf;
#		ifdef NB_CONFIG_INCLUDE_ASSERTS
		{
			NBString_set(&opq->dbgPathRef, "data");
			if(opq->dbgPathRef.length > 0){
				NBString_concat(&opq->dbgPathRef, " ");
			}
			NBString_concat(&opq->dbgPathRef, "sz[");
			NBString_concatUI32(&opq->dbgPathRef, dataSz);
			NBString_concat(&opq->dbgPathRef, "]");
		}
#		endif
		//
		r				= TRUE;
	}
	NBFILE_MUTEX_DEACTIVATE(opq)
	return r;
}

//STNBFileString

BOOL NBFile_openAsString(STNBFileRef obj, STNBString* str){
	BOOL r = FALSE;
	STNBFileOpq* opq = (STNBFileOpq*)obj.opaque; NBASSERT(NBFile_isClass(obj));
	NBFILE_MUTEX_ACTIVATE(opq)
	NBASSERT(opq->itfObj == NULL)
	if(opq->itfObj == NULL){
		IFileItf itf;
		NBMemory_setZeroSt(itf, IFileItf);
		itf.close		= NBFile_close_str_;
		//itf.lock		= NBFile_lock_str_;
		//itf.unlock	= NBFile_unlock_str_;
		itf.isOpen		= NBFile_isOpen_str_;
		itf.read		= NBFile_read_str_;
		itf.write		= NBFile_write_str_;
		itf.seek		= NBFile_seek_str_;
		itf.curPos		= NBFile_curPos_str_;
		itf.flush		= NBFile_flush_str_;
        {
            STNBFileString* nat = NBMemory_allocType(STNBFileString);
            NBMemory_setZeroSt(*nat, STNBFileString);
            nat->str		= str;
            nat->virtualPos	= 0;
            //
            opq->itf        = itf;
            opq->itfObj     = nat;
        }
		IF_NBASSERT(NBString_set(&opq->dbgPathRef, "string");)
		//
		r				= TRUE;
	}
	NBFILE_MUTEX_DEACTIVATE(opq)
	return r;
}

BOOL NBFile_openAsItf(STNBFileRef obj, const IFileItf* pItf, void* itfObj){
	BOOL r = FALSE;
	STNBFileOpq* opq = (STNBFileOpq*)obj.opaque; NBASSERT(NBFile_isClass(obj));
	NBFILE_MUTEX_ACTIVATE(opq)
	NBASSERT(opq->itfObj == NULL)
	if(opq->itfObj == NULL){
		if(pItf == NULL){
			NBMemory_setZeroSt(opq->itf, IFileItf);
		} else {
			opq->itf	= *pItf;
		}
		opq->itfObj		= itfObj;
		//
		r				= TRUE;
	}
	NBFILE_MUTEX_DEACTIVATE(opq)
	return r;
}

BOOL NBFile_isOpen(STNBFileRef obj){
	BOOL r = FALSE;
	STNBFileOpq* opq = (STNBFileOpq*)obj.opaque; NBASSERT(NBFile_isClass(obj));
	NBFILE_MUTEX_ACTIVATE(opq)
	if(opq->itf.isOpen != NULL){
		r = (*opq->itf.isOpen)(opq->itfObj);
	}
	NBFILE_MUTEX_DEACTIVATE(opq)
	return r;
}

void NBFile_closeOpq_(STNBFileOpq* opq){
    NBFILE_MUTEX_ACTIVATE(opq)
    if(opq->itf.close != NULL){
        (*opq->itf.close)(&opq->itfObj);
    }
    NBFILE_MUTEX_DEACTIVATE(opq)
}

void NBFile_close(STNBFileRef obj){
	STNBFileOpq* opq = (STNBFileOpq*)obj.opaque; NBASSERT(NBFile_isClass(obj));
    NBFile_closeOpq_(opq);
}

int NBFile_getFDOpq_(STNBFileOpq* opq){
    int r = -1;
    NBFILE_MUTEX_ACTIVATE(opq)
    if(opq->itf.getFD != NULL){
        //PRINTF_INFO("NBFile, getFDOpq_ opq(%lld) itfObj(%lld) '%s'.\n", (SI64)opq, (SI64)opq->itfObj, opq->dbgPathRef.str);
        r = (*opq->itf.getFD)(opq->itfObj);
    }
    NBFILE_MUTEX_DEACTIVATE(opq)
    return r;
}

int NBFile_getFD(STNBFileRef obj){
    STNBFileOpq* opq = (STNBFileOpq*)obj.opaque; NBASSERT(NBFile_isClass(obj));
    return NBFile_getFDOpq_(opq);
}

SI64 NBFile_getSize(const char* filepath){
	SI64 r = NB_IO_ERROR;
    STNBFileRef f = NBFile_alloc(NULL);
	if(NBFile_open(f, filepath, ENNBFileMode_Read)){
		NBFile_lock(f);
		if(NBFile_seek(f, 0, ENNBFileRelative_End)){
			r = NBFile_curPos(f);
		}
		NBFile_unlock(f);
		NBFile_close(f);
	}
	NBFile_release(&f);
	return r;
}

BOOL NBFile_flush(STNBFileRef obj){
	STNBFileOpq* opq = (STNBFileOpq*)obj.opaque; NBASSERT(NBFile_isClass(obj));
	BOOL r = FALSE; NBFILE_MUTEX_ASSERT_LOCKED(opq) //Must be locked by user
	if(opq->itf.flush != NULL){
		if((*opq->itf.flush)(opq->itfObj)){
			opq->lastOp = ENNBFileMode_None;
			r = TRUE;
		}
	}
	return r;
}

BOOL NBFile_setNonBlocking(STNBFileRef obj, const BOOL nonBlocking){    //non-blocking mode
    STNBFileOpq* opq = (STNBFileOpq*)obj.opaque; NBASSERT(NBFile_isClass(obj));
    BOOL r = FALSE; NBFILE_MUTEX_ASSERT_LOCKED(opq) //Must be locked by user
    if(opq->itf.setNonBlocking != NULL){
        r = (*opq->itf.setNonBlocking)(opq->itfObj, nonBlocking);
    }
    return r;
}


//

SI32 NBFile_readOpq_(STNBFileOpq* opq, void* dst, const UI32 dstSz){
    SI32 r = NB_IO_ERROR; NBFILE_MUTEX_ASSERT_LOCKED(opq) //Must be locked by user
    //Flush if last operation was "write"
    if(opq->lastOp == ENNBFileMode_Write){
        if(opq->itf.flush != NULL){
            IF_NBASSERT(const BOOL r =) (*opq->itf.flush)(opq->itfObj); NBASSERT(r)
        }
    }
    //Read data
    if(opq->itf.read != NULL){
        r = (*opq->itf.read)(opq->itfObj, dst, dstSz, &opq->curPos);
        if(r >= 0){
            opq->curPos    += r;
            opq->lastOp    = ENNBFileMode_Read;
        }
    }
    //Verify position
#   ifdef NB_CONFIG_INCLUDE_ASSERTS_FILE_POSITION
    if(opq->itf.curPos != NULL){
        const SI64 curPos = (*opq->itf.curPos)(opq->itfObj);
        NBASSERT(opq->curPos == curPos);
    }
#   endif
    return r;
}

SI32 NBFile_read(STNBFileRef obj, void* dst, const UI32 dstSz){
	STNBFileOpq* opq = (STNBFileOpq*)obj.opaque; NBASSERT(NBFile_isClass(obj));
    return NBFile_readOpq_(opq, dst, dstSz);
}

SI32 NBFile_writeOpq_(STNBFileOpq* opq, const void* src, const UI32 srcSz){
    SI32 r = NB_IO_ERROR; NBFILE_MUTEX_ASSERT_LOCKED(opq) //Must be locked by user
    //Write data
    if(opq->itf.write != NULL){
        r = (*opq->itf.write)(opq->itfObj, src, srcSz, &opq->curPos);
        if(r >= 0){
            opq->curPos    += r;
            opq->lastOp    = ENNBFileMode_Write;
        }
    }
    //Verify position
#   ifdef NB_CONFIG_INCLUDE_ASSERTS_FILE_POSITION
    if(opq->itf.curPos != NULL){
        const SI64 curPos = (*opq->itf.curPos)(opq->itfObj);
        NBASSERT(opq->curPos == curPos);
    }
#   endif
    return r;
}
    
SI32 NBFile_write(STNBFileRef obj, const void* src, const UI32 srcSz){
	STNBFileOpq* opq = (STNBFileOpq*)obj.opaque; NBASSERT(NBFile_isClass(obj));
    return NBFile_writeOpq_(opq, src, srcSz);
}

SI64 NBFile_curPos(STNBFileRef obj){
	STNBFileOpq* opq = (STNBFileOpq*)obj.opaque; NBASSERT(NBFile_isClass(obj));
    NBFILE_MUTEX_ASSERT_LOCKED(opq) //Must be locked by user
	//Verify position
#	ifdef NB_CONFIG_INCLUDE_ASSERTS_FILE_POSITION
	if(opq->itf.curPos != NULL){
		const SI64 curPos = (*opq->itf.curPos)(opq->itfObj);
		NBASSERT(opq->curPos == curPos);
	}
#	endif
	return opq->curPos;
}

BOOL NBFile_seek(STNBFileRef obj, const SI64 pos, const ENNBFileRelative relativeTo){
	STNBFileOpq* opq = (STNBFileOpq*)obj.opaque; NBASSERT(NBFile_isClass(obj));
	BOOL r = FALSE; NBFILE_MUTEX_ASSERT_LOCKED(opq) //Must be locked by user
	NBASSERT(relativeTo == ENNBFileRelative_Start || relativeTo == ENNBFileRelative_End || relativeTo == ENNBFileRelative_CurPos)
	//Verify position
#	ifdef NB_CONFIG_INCLUDE_ASSERTS_FILE_POSITION
	if(opq->itf.curPos != NULL){
		const SI64 curPos = (*opq->itf.curPos)(opq->itfObj);
		NBASSERT(opq->curPos == curPos);
	}
#	endif
	if(relativeTo == ENNBFileRelative_Start){
		if(opq->curPos == pos){
			//No action required
			r = TRUE;
		} else {
			//Flush if last operation was "write"
			/*if(obj->lastOp == ENNBFileOperation_Write){
			 const SI32 r = fflush(obj->_fileHandle); NBASSERT(r == 0)
			 obj->lastOp == ENNBFileOperation_None;
			 }*/
			//Seek
			if(opq->itf.seek != NULL){
				if((*opq->itf.seek)(opq->itfObj, pos, relativeTo, &opq->curPos)){
					opq->curPos = pos;
					r = TRUE;
				}
			}
		}
	} else if(relativeTo == ENNBFileRelative_CurPos){
		if(pos == 0){
			//No action required
			r = TRUE;
		} else {
			//Flush if last operation was "write"
			/*if(obj->lastOp == ENNBFileOperation_Write){
			 const SI32 r = fflush(obj->_fileHandle); NBASSERT(r == 0)
			 obj->lastOp == ENNBFileOperation_None;
			 }*/
			//Seek
			if(opq->itf.seek != NULL){
				if((*opq->itf.seek)(opq->itfObj, pos, relativeTo, &opq->curPos)){
					opq->curPos += pos;
					r = TRUE;
				}
			}
		}
	} else if(relativeTo == ENNBFileRelative_End){
		//Flush if last operation was "write"
		/*if(obj->lastOp == ENNBFileOperation_Write){
		 const SI32 r = fflush(obj->_fileHandle); NBASSERT(r == 0)
		 obj->lastOp == ENNBFileOperation_None;
		 }*/
		//Seek
		if(opq->itf.seek != NULL){
			if((*opq->itf.seek)(opq->itfObj, pos, relativeTo, &opq->curPos)){
				opq->curPos = (*opq->itf.curPos)(opq->itfObj);
				r = TRUE;
			}
		}
	} else {
		NBASSERT(0)
	}
	//Verify position
#	ifdef NB_CONFIG_INCLUDE_ASSERTS_FILE_POSITION
	if(opq->itf.curPos != NULL){
		const SI64 curPos = (*opq->itf.curPos)(opq->itfObj);
		NBASSERT(opq->curPos == curPos);
	}
#	endif
	return r;
}


BOOL NBFile_getIOLnk(STNBFileRef obj, STNBIOLnk* dst){
    BOOL r = FALSE;
    STNBFileOpq* opq = (STNBFileOpq*)obj.opaque; NBASSERT(NBFile_isClass(obj));
    if(opq != NULL && dst != NULL){
        STNBIOLnkItf itf;
        NBMemory_setZeroSt(itf, STNBIOLnkItf);
        //no-lock required
        itf.ioRetain   = NBObjRef_retainOpq;
        itf.ioRelease  = NBObjRef_releaseOpq;
        //
        itf.ioGetFD    = NBFile_ioGetFD_;
        itf.ioIsObjRef = NBFile_ioIsObjRef_;
        //
        itf.ioRead     = NBFile_ioRead_;
        itf.ioWrite    = NBFile_ioWrite_;
        itf.ioFlush    = NBFile_ioFlush_;
        itf.ioShutdown = NBFile_ioShutdown_;
        itf.ioClose    = NBFile_ioClose_;
        //
        NBIOLnk_setItf(dst, &itf, opq);
        //
        r = TRUE;
    }
    return r;
}

int NBFile_ioGetFD_(void* usrData){
    int r = -1;
    STNBFileOpq* opq = (STNBFileOpq*)usrData; NBASSERT(NBFile_isClass(NBObjRef_fromOpqPtr(opq)));
    if(opq != NULL){
        r = NBFile_getFDOpq_(opq);
    }
    return r;
}

BOOL NBFile_ioIsObjRef_(STNBObjRef objRef, void* usrData){
    STNBFileOpq* opq = (STNBFileOpq*)usrData; NBASSERT(NBFile_isClass(NBObjRef_fromOpqPtr(opq)));
    //NBASSERT(NBFile_isClass(*objRef)); //allowed to compare NonSocket objects
    return (opq == objRef.opaque);
}

SI32 NBFile_ioRead_(void* dst, const SI32 dstSz, void* usrData){ //read data to destination buffer, returns the ammount of bytes read, negative in case of error
    SI32 r = NB_IO_ERROR;
    STNBFileOpq* opq = (STNBFileOpq*)usrData; NBASSERT(NBFile_isClass(NBObjRef_fromOpqPtr(opq)));
    if(opq != NULL){
        NBFile_lockOpq_(opq);
        {
            r = NBFile_readOpq_(opq, dst, dstSz);
        }
        NBFile_unlockOpq_(opq);
    }
    return r;
}

SI32 NBFile_ioWrite_(const void* src, const SI32 srcSz, void* usrData){ //write data from source buffer, returns the ammount of bytes written, negative in case of error
    SI32 r = NB_IO_ERROR;
    STNBFileOpq* opq = (STNBFileOpq*)usrData; NBASSERT(NBFile_isClass(NBObjRef_fromOpqPtr(opq)));
    if(opq != NULL){
        NBFile_lockOpq_(opq);
        {
            r = NBFile_writeOpq_(opq, src, srcSz);
        }
        NBFile_unlockOpq_(opq);
    }
    return r;
}

void NBFile_ioFlush_(void* usrData){
    STNBFileOpq* opq = (STNBFileOpq*)usrData; NBASSERT(NBFile_isClass(NBObjRef_fromOpqPtr(opq)));
    if(opq != NULL){
        NBASSERT(FALSE) //ToDo: implement
    }
}

void NBFile_ioShutdown_(const UI8 mask, void* usrData){ //NB_IO_BIT_READ | NB_IO_BIT_WRITE
    STNBFileOpq* opq = (STNBFileOpq*)usrData; NBASSERT(NBFile_isClass(NBObjRef_fromOpqPtr(opq)));
    if(opq != NULL){
        NBFile_closeOpq_(opq);
    }
}

void NBFile_ioClose_(void* usrData){
    STNBFileOpq* opq = (STNBFileOpq*)usrData; NBASSERT(NBFile_isClass(NBObjRef_fromOpqPtr(opq)));
    if(opq != NULL){
        NBFile_closeOpq_(opq);
    }
}


//---------------------
//-- Native Itf (native file, standard-c)
//---------------------

#ifndef NB_IS_POSIX
void NBFile_closeStd_(void** obj){
    STNBFileNative* nat = (STNBFileNative*)*obj;
    if (nat != NULL) {
#       ifdef _WIN32
        NBASSERT(FALSE); //ToDo: implement
#       else
        if(nat->nat.stream != NULL){
            nat->nat.stream = NULL;
#           ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
            NBMngrProcess_fsFileClosed(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Success);
#           endif
        }
#       endif
        NBMemory_free(nat);
        *obj = NULL;
    }
}
#endif

#ifndef NB_IS_POSIX
/*void NBFile_lock_(void* obj){
	//nothing
}*/
#endif

#ifndef NB_IS_POSIX
/*void NBFile_unlock_(void* obj){
	//nothing
}*/
#endif

#ifndef NB_IS_POSIX
BOOL NBFile_isOpen_(const void* obj){
	BOOL r = FALSE;
#	ifndef NB_COMPILE_DRIVER_MODE
	const STNBFileNative* nat = (const STNBFileNative*)obj;
	if(nat != NULL){
        r = (nat->nat.hndl != NULL ? TRUE : FALSE);
	}
#	endif
	return r;
}
#endif

#ifndef NB_IS_POSIX
SI32 NBFile_read_(void* obj, void* dst, const UI32 dstSz, const SI64* curPos){
	SI32 r = NB_IO_ERROR;
	STNBFileNative* nat = (STNBFileNative*)obj;
	if(nat != NULL){
		if (dstSz == 0) {
			r = 0;
		} else if(dstSz > 0) {
#			ifdef _WIN32
#				ifndef NB_COMPILE_DRIVER_MODE
				if (nat->nat.hndl != NULL) {
					//Read data
					DWORD rd = 0;
					if (ReadFile(nat->nat.hndl, dst, dstSz, &rd, NULL)) {
						r = rd;
					}
				}
#				else
				{
					nat->nat.hndlDumy = TRUE;
				}
#				endif
#			else
			if (nat->nat.stream != NULL) {
				//Read data
				size_t rd = fread(dst, 1, dstSz, nat->nat.stream);
				if (rd >= 0) {
					r = (SI32)rd;
				}
			}
#			endif
#			ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
			NBMngrProcess_fsFileRead(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Success, (r >= 0 ? r : 0));
#			endif
		}
	}
	return r;
}
#endif

#ifndef NB_IS_POSIX
SI32 NBFile_write_(void* obj, const void* src, const UI32 srcSz, const SI64* curPos){
	SI32 r = NB_IO_ERROR;
	STNBFileNative* nat = (STNBFileNative*)obj;
	if(nat != NULL){
		if (srcSz == 0) {
			r = 0;
		} else if(srcSz > 0){
#			ifdef _WIN32
#				ifndef NB_COMPILE_DRIVER_MODE
				if (nat->nat.hndl != NULL) {
					//Write data
					DWORD rd = 0;
					if (WriteFile(nat->nat.hndl, src, srcSz, &rd, NULL)) {
						r = rd;
					}
				}
#				else
				{
					nat->nat.hndlDumy = TRUE;
				}
#				endif
#			else
			if (nat->nat.stream != NULL) {
				//Write data
				size_t rd = fwrite(src, 1, srcSz, nat->nat.stream);
				if (rd > 0) {
					r = (SI32)rd;
				}
			}
#			endif
#			ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
			NBMngrProcess_fsFileWritten(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Success, (r >= 0 ? r : 0));
#			endif
		}
	}
	return r;
}
#endif

#ifndef NB_IS_POSIX
BOOL NBFile_seek_(void* obj, const SI64 pos, const ENNBFileRelative relativeTo, const SI64* curPos){
	BOOL r = FALSE;
	STNBFileNative* nat = (STNBFileNative*)obj;
	if(nat != NULL){
#		ifdef _WIN32
#			ifndef NB_COMPILE_DRIVER_MODE
			if(nat->nat.hndl != NULL) {
				LARGE_INTEGER dstPos, resultPos;
				dstPos.QuadPart = pos;
				SetFilePointerEx(nat->nat.hndl, dstPos, &resultPos, (relativeTo == ENNBFileRelative_CurPos ? FILE_CURRENT : relativeTo == ENNBFileRelative_End ? FILE_END : FILE_BEGIN));
				r = TRUE;
			}
#			else
			{
				nat->nat.hndlDumy = TRUE;
			}
#			endif
#		else
		if (nat->nat.stream != NULL) {
			if (fseek(nat->nat.stream, pos, (relativeTo == ENNBFileRelative_CurPos ? SEEK_CUR : relativeTo == ENNBFileRelative_End ? SEEK_END : SEEK_SET)) == 0) {
#				ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
				NBMngrProcess_fsFileSeeked(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Success);
#				endif
				r = TRUE;
			} else {
#				ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
				NBMngrProcess_fsFileSeeked(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Error);
#				endif
			}
		}
#		endif
	}
	return r;
}
#endif

#ifndef NB_IS_POSIX
SI64 NBFile_curPos_(const void* obj){
	SI64 r = NB_IO_ERROR;
	STNBFileNative* nat = (STNBFileNative*)obj;
	if(nat != NULL){
#		ifdef _WIN32
#			ifndef NB_COMPILE_DRIVER_MODE
			if(nat->nat.hndl != NULL) {
				LARGE_INTEGER dstPos, curpos;
				dstPos.QuadPart = 0;
				SetFilePointerEx(nat->nat.hndl, dstPos, &curpos, FILE_CURRENT);
				r = curpos.QuadPart;
			}
#			else
			{
				nat->nat.hndlDumy = TRUE;
			}
#			endif
#		else
		if (nat->nat.stream != NULL) {
			r = ftell(nat->nat.stream);
		}
#		endif
#		ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
			NBMngrProcess_fsFileTell(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Success);
#		endif
	}
	return r;
}
#endif

#ifndef NB_IS_POSIX
BOOL NBFile_flush_(void* obj){
	BOOL r = FALSE;
	STNBFileNative* nat = (STNBFileNative*)obj;
	if(nat != NULL){
#		ifdef _WIN32
#			ifndef NB_COMPILE_DRIVER_MODE
			if(nat->nat.hndl != NULL) {
				if (FlushFileBuffers(nat->nat.hndl)) {
					r = TRUE;
				}
			}
#			else
			{
				nat->nat.hndlDumy = TRUE;
			}
#			endif
#		else
		if (nat->nat.stream != NULL) {
			if(fflush(nat->nat.stream) == 0){
#				ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
				NBMngrProcess_fsFileFlush(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Success);
#				endif
				r = TRUE;
			} else {
#				ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
				NBMngrProcess_fsFileFlush(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Error);
#				endif				
			}
		}
#		endif
	}
	return r;
}
#endif

#ifndef NB_IS_POSIX
int NBFile_getFD_(void* obj){
    int r = -1;
    STNBFileNative* nat = (STNBFileNative*)obj;
    if(nat != NULL){
#       ifdef _WIN32
        {
            NBASSERT(FALSE); //ToDo: implement, convert HANDLE to FD on Windows.
        }
#       else
        {
            r = fileno(nat->nat.stream);
        }
#       endif
    }
    return r;
}
#endif

//----------------------
//- Native itf (native file, POSIX)
//----------------------

void NBFile_close_(void** obj){
    STNBFileNative* nat = (STNBFileNative*)*obj;
    if(nat != NULL) {
        if(NBHndl_isSet(nat->hndl)){
            NBHndl_setOrphan(nat->hndl); //flag as orphan for every external consumer (like pollster).
            NBHndl_release(&nat->hndl);
            NBHndl_null(&nat->hndl);
        }
        NBMemory_setZero(nat->nat);
        //
        NBMemory_free(nat);
        *obj = NULL;
    }
}

#ifdef NB_IS_POSIX
void NBFile_closeStd_posix_(void** obj){
    STNBFileNative* nat = (STNBFileNative*)*obj;
    if(nat != NULL) {
        if(nat->nat.fd >= 0){
            nat->nat.fd = -1;
#           ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
            NBMngrProcess_fsFileClosed(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Success);
#           endif
        }
        NBMemory_free(nat);
        *obj = NULL;
    }
}
#endif

#ifdef NB_IS_POSIX
/*void NBFile_lock_posix_(void* obj){
    //nothing
}*/
#endif

#ifdef NB_IS_POSIX
/*void NBFile_unlock_posix_(void* obj){
    //nothing
}*/
#endif

#ifdef NB_IS_POSIX
BOOL NBFile_isOpen_posix_(const void* obj){
    BOOL r = FALSE;
#   ifndef NB_COMPILE_DRIVER_MODE
    const STNBFileNative* nat = (const STNBFileNative*)obj;
    if(nat != NULL){
        r = (nat->nat.fd >= 0 ? TRUE : FALSE);
    }
#   endif
    return r;
}
#endif

#ifdef NB_IS_POSIX
BOOL NBFile_setNonBlocking_posix_(void* obj, const BOOL nonBlocking){
    BOOL r = FALSE;
#   ifndef NB_COMPILE_DRIVER_MODE
    const STNBFileNative* nat = (const STNBFileNative*)obj;
    if(nat != NULL){
        if(nat->nat.fd >= 0){
            //get flags
            int flags = fcntl(nat->nat.fd, F_GETFL, 0);
            if(flags != -1){
                //modify flags
                if(nonBlocking){
                    flags |= O_NONBLOCK;
                } else {
                    flags &= ~O_NONBLOCK;
                }
                //apply new flags
                if(fcntl(nat->nat.fd, F_SETFL, flags) != -1){
                    r = TRUE;
                }
            }
        }
    }
#   endif
    return r;
}
#endif

#ifdef NB_IS_POSIX
SI32 NBFile_read_posix_(void* obj, void* dst, const UI32 dstSz, const SI64* curPos){
    SI32 r = NB_IO_ERROR;
    STNBFileNative* nat = (STNBFileNative*)obj;
    if(nat != NULL){
        if (nat->nat.fd >= 0) {
            if (dstSz == 0) {
                r = 0;
            } else if(dstSz > 0){
                //Read data
                ssize_t rd = read(nat->nat.fd, dst, dstSz);
                if (rd > 0) {
                    r = (SI32)rd;
#                   ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
                    NBMngrProcess_fsFileRead(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Success, r);
#                   endif
                } else if (rd == 0) {
                    //EOF
                    r = NB_IO_ERR_EOF;
#                   ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
                    NBMngrProcess_fsFileRead(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Error, 0);
#                   endif
                } else if(errno == EAGAIN || errno == EWOULDBLOCK){
                    //non-blocking read
                    r = 0;
                } else {
                    //error
                    r = NB_IO_ERROR;
#                   ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
                    NBMngrProcess_fsFileRead(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Error, 0);
#                   endif
                }
            }
        }
    }
    return r;
}
#endif

#ifdef NB_IS_POSIX
SI32 NBFile_write_posix_(void* obj, const void* src, const UI32 srcSz, const SI64* curPos){
    SI32 r = NB_IO_ERROR;
    STNBFileNative* nat = (STNBFileNative*)obj;
    if(nat != NULL){
        if (nat->nat.fd >= 0) {
            if(srcSz == 0) {
                r = 0;
            } else if(srcSz > 0){
                //Write data
                size_t rd = write(nat->nat.fd, src, srcSz);
                if (rd >= 0) {
                    r = (SI32)rd;
#                   ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
                    NBMngrProcess_fsFileWritten(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Success, rd);
#                   endif
                } else if(errno == EAGAIN || errno == EWOULDBLOCK){
                    r = 0;
                } else {
                    r = NB_IO_ERROR;
#                   ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
                    NBMngrProcess_fsFileWritten(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Error, 0);
#                   endif
                }
            }
        }
    }
    return r;
}
#endif

#ifdef NB_IS_POSIX
BOOL NBFile_seek_posix_(void* obj, const SI64 pos, const ENNBFileRelative relativeTo, const SI64* curPos){
    BOOL r = FALSE;
    STNBFileNative* nat = (STNBFileNative*)obj;
    if(nat != NULL){
        if (nat->nat.fd >= 0) {
            //
            if (lseek(nat->nat.fd, pos, (relativeTo == ENNBFileRelative_CurPos ? SEEK_CUR : relativeTo == ENNBFileRelative_End ? SEEK_END : SEEK_SET)) >= 0) {
#               ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
                NBMngrProcess_fsFileSeeked(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Success);
#               endif
                r = TRUE;
            } else {
#               ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
                NBMngrProcess_fsFileSeeked(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Error);
#               endif
            }
        }
    }
    return r;
}
#endif

#ifdef NB_IS_POSIX
SI64 NBFile_curPos_posix_(const void* obj){
    SI64 r = NB_IO_ERROR;
    STNBFileNative* nat = (STNBFileNative*)obj;
    if(nat != NULL){
        if (nat->nat.fd >= 0) {
            r = lseek(nat->nat.fd, 0, SEEK_CUR);
        }
#       ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
        NBMngrProcess_fsFileTell(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Success);
#       endif
    }
    return r;
}
#endif

#ifdef NB_IS_POSIX
BOOL NBFile_flush_posix_(void* obj){
    BOOL r = FALSE;
    STNBFileNative* nat = (STNBFileNative*)obj;
    if(nat != NULL){
        if (nat->nat.fd >= 0) {
            if(fsync(nat->nat.fd) >= 0){
#               ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
                NBMngrProcess_fsFileFlush(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Success);
#               endif
                r = TRUE;
            } else {
#               ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
                NBMngrProcess_fsFileFlush(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Error);
#               endif
            }
        }
    }
    return r;
}
#endif

#ifdef NB_IS_POSIX
int NBFile_getFD_posix_(void* obj){
    int r = -1;
    STNBFileNative* nat = (STNBFileNative*)obj;
    if(nat != NULL){
        r = nat->nat.fd;
    }
    return r;
}
#endif

//---------------------
//-- Native Itf (file-rng)
//---------------------

void NBFile_close_fileRng_(void** obj){
	STNBFileRng* nat = (STNBFileRng*)*obj;
	if(nat != NULL){
		if(NBFile_isSet(nat->parent)){
			NBFile_release(&nat->parent);
            NBFile_null(&nat->parent);
		}
		NBMemory_free(nat);
		*obj = NULL;
	}
}

void NBFile_lock_fileRng_(void* obj){
	STNBFileRng* nat = (STNBFileRng*)obj;
	if(nat != NULL){
		if(NBFile_isSet(nat->parent)){
			NBFile_lock(nat->parent);
		}
	}
}

void NBFile_unlock_fileRng_(void* obj){
	STNBFileRng* nat = (STNBFileRng*)obj;
	if(nat != NULL){
		if(NBFile_isSet(nat->parent)){
			NBFile_unlock(nat->parent);
		}
	}
}

BOOL NBFile_isOpen_fileRng_(const void* obj){
	BOOL r = FALSE;
	const STNBFileRng* nat = (const STNBFileRng*)obj;
	if(nat != NULL){
		r = (NBFile_isSet(nat->parent) ? TRUE : FALSE);
	}
	return r;
}

SI32 NBFile_read_fileRng_(void* obj, void* dst, const UI32 dstSz, const SI64* curPos){
	SI32 r = NB_IO_ERROR;
	STNBFileRng* nat = (STNBFileRng*)obj;
	if(nat != NULL){
		NBASSERT(curPos == NULL || (*curPos >= 0 && *curPos <= nat->rng.size));
		NBASSERT(curPos == NULL || (nat->virtualPos == *curPos))
		if(nat->virtualPos >= 0 && NBFile_isSet(nat->parent)){
            const SI64 bytesRemain = (SI64)nat->rng.size - nat->virtualPos;
            if(bytesRemain >= 0){
                if(dstSz == 0){
                    r = 0;
                } else if(dstSz > 0){
                    r = NBFile_read(nat->parent, dst, (bytesRemain <= dstSz ? (UI32)bytesRemain : dstSz));
                    if(r > 0){
                        nat->virtualPos += r;
                    }
                }
            }
		}
	}
	return r;
}

SI32 NBFile_write_fileRng_(void* obj, const void* src, const UI32 srcSz, const SI64* curPos){
	SI32 r = NB_IO_ERROR;
	STNBFileRng* nat = (STNBFileRng*)obj;
	if(nat != NULL){
		NBASSERT(curPos == NULL || (*curPos >= 0 && *curPos <= nat->rng.size));
		NBASSERT(curPos == NULL || (nat->virtualPos == *curPos))
		if(nat->virtualPos >= 0 && NBFile_isSet(nat->parent)){
			const SI64 bytesRemain	= (SI64)nat->rng.size - nat->virtualPos;
			if(bytesRemain >= 0){
                if(srcSz == 0){
                    r = 0;
                } else if(srcSz > 0){
                    r = NBFile_write(nat->parent, src, srcSz);
                    if(r > 0){
                        nat->virtualPos += r;
                    }
                }
			}
		}
	}
	return r;
}

BOOL NBFile_seek_fileRng_(void* obj, const SI64 pos, const ENNBFileRelative relativeTo, const SI64* curPos){
	BOOL r = FALSE;
	STNBFileRng* nat = (STNBFileRng*)obj;
	if(nat != NULL){
		NBASSERT(curPos == NULL || (*curPos >= 0 && *curPos <= nat->rng.size));
		NBASSERT(curPos == NULL || (nat->virtualPos == *curPos))
		if(NBFile_isSet(nat->parent)){
			SI64 parentPos = -1;
			switch (relativeTo) {
				case ENNBFileRelative_Start:
					parentPos = (nat->rng.start + pos);
					break;
				case ENNBFileRelative_End:
					parentPos = ((SI64)nat->rng.start + (SI64)nat->rng.size - pos);
					break;
				case ENNBFileRelative_CurPos:
					parentPos = nat->virtualPos + pos;
					break;
				default:
					break;
			}
			NBASSERT(parentPos >= nat->rng.start && parentPos <= ((SI64)nat->rng.start + (SI64)nat->rng.size))
			if(parentPos >= nat->rng.start && parentPos <= ((SI64)nat->rng.start + (SI64)nat->rng.size)){
				r = NBFile_seek(nat->parent, parentPos, ENNBFileRelative_Start);
				nat->virtualPos = (parentPos - nat->rng.start);
			}
		}
	}
	return r;
}

SI64 NBFile_curPos_fileRng_(const void* obj){
	SI64 r = NB_IO_ERROR;
	const STNBFileRng* nat = (const STNBFileRng*)obj;
	if(nat != NULL){
		if(NBFile_isSet(nat->parent)){
			r = NBFile_curPos(nat->parent) - nat->rng.start;
			if(r < 0) r = 0;
			if(r > (nat->rng.start + nat->rng.size)) r = (nat->rng.start + nat->rng.size);
		}
	}
	return r;
}

BOOL NBFile_flush_fileRng_(void* obj){
	BOOL r = FALSE;
	STNBFileRng* nat = (STNBFileRng*)obj;
	if(nat != NULL){
		if(NBFile_isSet(nat->parent)){
			r = NBFile_flush(nat->parent);
		}
	}
	return r;
}

//---------------------
//-- Native Itf (data-rng)
//---------------------

void NBFile_close_dataRng_(void** obj){
	STNBFileDataRng* nat = (STNBFileDataRng*)*obj;
	if(nat != NULL){
		nat->data		= NULL;
		nat->dataSz		= 0;
		nat->virtualPos	= 0;
		NBMemory_free(nat);
		*obj = NULL;
	}
}

/*void NBFile_lock_dataRng_(void* obj){
	STNBFileDataRng* nat = (STNBFileDataRng*)obj;
	if(nat != NULL){
		//Nothing
	}
}*/

/*void NBFile_unlock_dataRng_(void* obj){
	STNBFileDataRng* nat = (STNBFileDataRng*)obj;
	if(nat != NULL){
		//Nothing
	}
}*/

BOOL NBFile_isOpen_dataRng_(const void* obj){
	BOOL r = FALSE;
	const STNBFileDataRng* nat = (const STNBFileDataRng*)obj;
	if(nat != NULL){
		r = (nat->data != NULL ? TRUE : FALSE);
	}
	return r;
}

SI32 NBFile_read_dataRng_(void* obj, void* dst, const UI32 dstSz, const SI64* curPos){
	SI32 r = NB_IO_ERROR;
	STNBFileDataRng* nat = (STNBFileDataRng*)obj;
	if(nat != NULL){
		NBASSERT(curPos == NULL || (*curPos >= 0 && *curPos <= nat->dataSz));
		NBASSERT(curPos == NULL || (nat->virtualPos == *curPos))
		if(nat->virtualPos >= 0 && nat->data != NULL){
			const SI64 bytesRemain	= (SI64)nat->dataSz - nat->virtualPos;
			if(bytesRemain >= 0){
                if(dstSz == 0){
                    r = 0;
                } else if(dstSz > 0){
                    r = (bytesRemain <= dstSz ? (SI32)bytesRemain : dstSz);
                    if(r > 0){
                        NBMemory_copy(dst, &nat->data[nat->virtualPos], r);
                        nat->virtualPos += r;
                    }
                }
			}
		}
	}
	return r;
}

SI32 NBFile_write_dataRng_(void* obj, const void* src, const UI32 srcSz, const SI64* curPos){
	SI32 r = NB_IO_ERROR;
	STNBFileDataRng* nat = (STNBFileDataRng*)obj;
	if(nat != NULL){
		NBASSERT(curPos == NULL || (*curPos >= 0 && *curPos <= nat->dataSz))
		NBASSERT(curPos == NULL || (nat->virtualPos == *curPos))
		if(nat->virtualPos >= 0 && nat->data != NULL){
			const SI64 bytesRemain = (SI64)nat->dataSz - nat->virtualPos;
			if(bytesRemain >= 0){
                if(srcSz == 0){
                    r = 0;
                } else if(srcSz > 0){
                    r = (bytesRemain <= srcSz ? (SI32)bytesRemain : srcSz);
                    if(r > 0){
                        NBMemory_copy(&nat->data[nat->virtualPos], src, r);
                        nat->virtualPos += r;
                    }
                }
			}
		}
	}
	return r;
}

BOOL NBFile_seek_dataRng_(void* obj, const SI64 pos, const ENNBFileRelative relativeTo, const SI64* curPos){
	BOOL r = FALSE;
	STNBFileDataRng* nat = (STNBFileDataRng*)obj;
	if(nat != NULL){
		NBASSERT(curPos == NULL || (*curPos >= 0 && *curPos <= nat->dataSz))
		NBASSERT(curPos == NULL || (nat->virtualPos == *curPos))
		if(nat->data != NULL){
			switch (relativeTo) {
				case ENNBFileRelative_Start:
					if(pos >= 0 && pos <= nat->dataSz){
						nat->virtualPos = pos;
						r = TRUE;
					}
					break;
				case ENNBFileRelative_End:
					if(((SI64)nat->dataSz - pos) >= 0 && ((SI64)nat->dataSz - pos) <= nat->dataSz){
						nat->virtualPos = nat->dataSz - pos;
						r = TRUE;
					}
					break;
				case ENNBFileRelative_CurPos:
					if((nat->virtualPos + pos) >= 0 && (nat->virtualPos + pos) <= nat->dataSz){
						nat->virtualPos += pos;
						r = TRUE;
					}
					break;
				default:
					break;
			}
		}
	}
	return r;
}

SI64 NBFile_curPos_dataRng_(const void* obj){
	SI64 r = NB_IO_ERROR;
	const STNBFileDataRng* nat = (const STNBFileDataRng*)obj;
	if(nat != NULL){
		if(nat->data != NULL){
			r = nat->virtualPos;
		}
	}
	return r;
}

BOOL NBFile_flush_dataRng_(void* obj){
	BOOL r = FALSE;
	STNBFileDataRng* nat = (STNBFileDataRng*)obj;
	if(nat != NULL){
		//Nothing
	}
	return r;
}

//---------------------
//-- Native Itf (string)
//---------------------

void NBFile_close_str_(void** obj){
	STNBFileString* nat = (STNBFileString*)*obj;
	if(nat != NULL){
		nat->str		= NULL;
		nat->virtualPos	= 0;
		NBMemory_free(nat);
		*obj = NULL;
	}
}

/*void NBFile_locking_str_(void* obj){
	STNBFileString* nat = (STNBFileString*)obj;
	if(nat != NULL){
		//Nothing
	}
}*/

/*void NBFile_unlocked_str_(void* obj){
	STNBFileString* nat = (STNBFileString*)obj;
	if(nat != NULL){
		//Nothing
	}
}*/

BOOL NBFile_isOpen_str_(const void* obj){
	BOOL r = FALSE;
	const STNBFileString* nat = (const STNBFileString*)obj;
	if(nat != NULL){
		r = (nat->str != NULL ? TRUE : FALSE);
	}
	return r;
}

SI32 NBFile_read_str_(void* obj, void* dst, const UI32 dstSz, const SI64* curPos){
	SI32 r = NB_IO_ERROR;
	STNBFileString* nat = (STNBFileString*)obj;
	if(nat != NULL){
		NBASSERT(curPos == NULL || (*curPos >= 0 && *curPos <= nat->str->length))
		NBASSERT(curPos == NULL || (nat->virtualPos == *curPos))
		if(nat->virtualPos >= 0 && nat->str != NULL){
			const SI64 bytesRemain = (SI64)nat->str->length - nat->virtualPos;
			if(bytesRemain >= 0){
                if(dstSz == 0){
                    r = 0;
                } else if(dstSz > 0){
                    r = (bytesRemain <= dstSz ? (SI32)bytesRemain : dstSz);
                    if(r > 0){
                        NBMemory_copy(dst, &nat->str->str[nat->virtualPos], r);
                        nat->virtualPos += r;
                    }
                }
			}
		}
	}
	return r;
}

SI32 NBFile_write_str_(void* obj, const void* src, const UI32 srcSz, const SI64* curPos){
	SI32 r = NB_IO_ERROR;
	STNBFileString* nat = (STNBFileString*)obj;
	if(nat != NULL){
		NBASSERT(curPos == NULL || (*curPos >= 0 && *curPos <= nat->str->length))
		NBASSERT(curPos == NULL || (nat->virtualPos == *curPos))
		if(nat->virtualPos >= 0 && nat->str != NULL){
            if(srcSz == 0){
                r = 0;
            } else if(srcSz > 0){
                //copy to available buffer
                const SI64 bytesRemain = (SI64)nat->str->length - nat->virtualPos;
                r = (bytesRemain <= srcSz ? (SI32)bytesRemain : srcSz);
                if(r > 0){
                    NBMemory_copy(&nat->str->str[nat->virtualPos], src, (UI32)r);
                    nat->virtualPos += r;
                }
                //copy remaining to new buffer
                if(r < srcSz){
                    NBASSERT(nat->virtualPos == nat->str->length) //should be at the end of the string
                    NBString_concatBytes(nat->str, &(((const BYTE*)src)[r]), (srcSz - r));
                    nat->virtualPos += (srcSz - r);
                    r += (srcSz - r);
                }
                NBASSERT((nat->virtualPos + srcSz) <= nat->str->length)
            }
		}
	}
	return r;
}

BOOL NBFile_seek_str_(void* obj, const SI64 pos, const ENNBFileRelative relativeTo, const SI64* curPos){
	BOOL r = FALSE;
	STNBFileString* nat = (STNBFileString*)obj;
	if(nat != NULL){
		NBASSERT(curPos == NULL || (*curPos >= 0 && *curPos <= nat->str->length));
		NBASSERT(curPos == NULL || (nat->virtualPos == *curPos))
		if(nat->str != NULL){
			switch (relativeTo) {
				case ENNBFileRelative_Start:
					if(pos >= 0 && pos <= nat->str->length){
						nat->virtualPos = pos;
						r = TRUE;
					}
					break;
				case ENNBFileRelative_End:
					if(((SI64)nat->str->length - pos) >= 0 && ((SI64)nat->str->length - pos) <= nat->str->length){
						nat->virtualPos = nat->str->length - pos;
						r = TRUE;
					}
					break;
				case ENNBFileRelative_CurPos:
					if((nat->virtualPos + pos) >= 0 && (nat->virtualPos + pos) <= nat->str->length){
						nat->virtualPos += pos;
						r = TRUE;
					}
					break;
				default:
					break;
			}
		}
	}
	return r;
}

SI64 NBFile_curPos_str_(const void* obj){
	SI64 r = NB_IO_ERROR;
	const STNBFileString* nat = (const STNBFileString*)obj;
	if(nat != NULL){
		if(nat->str != NULL){
			r = nat->virtualPos;
		}
	}
	return r;
}

BOOL NBFile_flush_str_(void* obj){
	BOOL r = FALSE;
	STNBFileString* nat = (STNBFileString*)obj;
	if(nat != NULL){
		//Nothing
	}
	return r;
}

/*SI32 NBFile_truncateToZero(STNBFileRef obj){
	SI32 r = NB_IO_ERROR;
#	ifdef _WIN32
	{
#		ifdef NB_CONFIG_INCLUDE_ASSERTS_FILE_POSITION
		{
			LARGE_INTEGER dstPos, curpos;
			dstPos.QuadPart = 0;
			SetFilePointerEx(nat->nat.hndl, dstPos, &curpos, FILE_CURRENT);
		    NBASSERT_FILE_POS(opq->curPos == curpos.QuadPart);
		}
#		endif
		{
			LARGE_INTEGER dstPos, curpos;
			dstPos.QuadPart = 0;
			SetFilePointerEx(nat->nat.hndl, dstPos, &curpos, FILE_BEGIN);
		    NBASSERT_FILE_POS(curpos.QuadPart == 0);
			if(SetEndOfFile(nat->nat.hndl)){
				r = 0;
			}
		}
		//Update cache
		opq->curPos = 0;
		opq->lastOp = ENNBFileMode_None;
#		ifdef NB_CONFIG_INCLUDE_ASSERTS_FILE_POSITION
		{
			LARGE_INTEGER dstPos, curpos;
			dstPos.QuadPart = 0;
			SetFilePointerEx(nat->nat.hndl, dstPos, &curpos, FILE_CURRENT);
		    NBASSERT_FILE_POS(opq->curPos == curpos.QuadPart);
		}
#		endif
	}
#	else
	{
#		ifdef NB_CONFIG_INCLUDE_ASSERTS_FILE_POSITION
		{
 			const SI64 curpos = (*opq->itf.curPos)(opq->itfObj);
 			NBASSERT(opq->curPos == curpos);
		}
#		endif
		{
			const SI32 fileDesc = _fileno(nat->nat.fd);
			r = _chsize_s(fileDesc, 0); NBASSERT(r == 0)
		}
		//Update cache
		opq->curPos = 0;
		opq->lastOp = ENNBFileMode_None;
#		ifdef NB_CONFIG_INCLUDE_ASSERTS_FILE_POSITION
		{
 			const SI64 curpos = (*opq->itf.curPos)(opq->itfObj);
 			NBASSERT(opq->curPos == curpos);
		}
#		endif
	}
#	endif
	//
	return r;
}*/
