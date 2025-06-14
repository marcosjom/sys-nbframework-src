#ifndef NB_MNGR_THREADS_H
#define NB_MNGR_THREADS_H

#include "nb/NBFrameworkDefs.h"
#include <stdarg.h> //for va_list
//avoid NBMemory usage (also objects that uses it)
#include "nb/core/NBLog.h"
#include "nb/core/NBHndlNative.h"

#ifdef __cplusplus
extern "C" {
#endif


	typedef enum ENNBThreadLockPushMode_ {
		ENNBThreadLockPushMode_Compete = 0,	//Current thread will compete with others for the lock, and own it later.
		ENNBThreadLockPushMode_NonCompete,	//Current thread will-not compete for the lock, if already taken will continue without yielding.
	} ENNBThreadLockPushMode;

	//--------------------
	//- NBMngrProcessStats
	//--------------------

    //NBFilesystemStatsOp

	typedef struct STNBFilesystemStatsOp_ {
		SI64	                count; //can be negative in thread-stats, not in global stats
		UI64	                bytes;
	} STNBFilesystemStatsOp;

    const STNBStructMap* NBFilesystemStatsOp_getSharedStructMap(void);

    //NBFilesystemStatsFilesOps

	typedef struct STNBFilesystemStatsFilesOps_ {
		SI64					open;	//can be negative in thread-stats, not in global stats
		SI64					close;
		STNBFilesystemStatsOp	read;
		STNBFilesystemStatsOp	write;
		UI64					seek;
		UI64					tell;
		UI64					flush;
		UI64					stat;
		UI64					move;
		UI64					delet;
	} STNBFilesystemStatsFilesOps;

    const STNBStructMap* NBFilesystemStatsFilesOps_getSharedStructMap(void);

    //NBFilesystemStatsFoldersOps

	typedef struct STNBFilesystemStatsFoldersOps_ {
		UI64					create;
		SI64					open;	//can be negative in thread-stats, not in global stats
		SI64					close;
		UI64					read;
		UI64					delet;
	} STNBFilesystemStatsFoldersOps;

    const STNBStructMap* NBFilesystemStatsFoldersOps_getSharedStructMap(void);

    //NBFilesystemStatsOps

	typedef struct STNBFilesystemStatsOps_ {
		STNBFilesystemStatsFilesOps		files;
		STNBFilesystemStatsFoldersOps	folders;
	} STNBFilesystemStatsOps;

    const STNBStructMap* NBFilesystemStatsOps_getSharedStructMap(void);

    //NBFilesystemStatsSrc
    
	typedef enum ENNBFilesystemStatsSrc_ {
		ENNBFilesystemStatsSrc_Native = 0,		//OS native
		ENNBFilesystemStatsSrc_Abstract,		//abstract, like android-assets
		ENNBFilesystemStatsSrc_Count
	} ENNBFilesystemStatsSrc;

    //NBFilesystemStatsResult

	typedef enum ENNBFilesystemStatsResult_ {
		ENNBFilesystemStatsResult_Success = 0,	//success op
		ENNBFilesystemStatsResult_Error,		//error op
		ENNBFilesystemStatsResult_Count
	} ENNBFilesystemStatsResult;

    //NBFilesystemStatsState

	typedef struct STNBFilesystemStatsState_ {
		STNBFilesystemStatsOps ops[ENNBFilesystemStatsSrc_Count][ENNBFilesystemStatsResult_Count];
	} STNBFilesystemStatsState;

    const STNBStructMap* NBFilesystemStatsState_getSharedStructMap(void);

	//

	typedef struct STNBThreadsMethodLocksOps_ {
		char*			method;		//parent method
		SI64			locked;		//the lock was pushed
		SI64			unlocked;	//the lock was poped
		SI64			yielded;	//the lock yielded because other thread owning the lock
	} STNBThreadsMethodLocksOps;

    const STNBStructMap* NBThreadsMethodLocksOps_getSharedStructMap(void);

	void NBThreadsMethodLocksOps_init(STNBThreadsMethodLocksOps* obj);
	void NBThreadsMethodLocksOps_release(STNBThreadsMethodLocksOps* obj);

    //NBMngrProcessStatsStateMem
    
    typedef struct STNBMngrProcessStatsStateMem_{
        SI64        allocs;        //can be negative in thread-stats, not in global stats
        UI64        allocBytes;    //
        SI64        freeds;        //freed
    } STNBMngrProcessStatsStateMem;

    const STNBStructMap* NBMngrProcessStatsStateMem_getSharedStructMap(void);

    //NBMngrProcessStatsStateLocksByMethod

    typedef struct STNBMngrProcessStatsStateLocksByMethod_ {
        STNBThreadsMethodLocksOps*  arr;
        SI32                        use;
        SI32                        alloc;    //alloc optimization (not a struct member)
    } STNBMngrProcessStatsStateLocksByMethod;

    const STNBStructMap* NBMngrProcessStatsStateLocksByMethod_getSharedStructMap(void);

    //NBMngrProcessStatsStateLocks

    typedef struct STNBMngrProcessStatsStateLocks_ {
        UI64        created;
        UI64        destroyed;
        UI64        locked;
        UI64        unlocked;
        UI64        passby;       //locks became owner inmediatly
        UI64        yield;        //locks woth other owner and made me yield and wait
        STNBMngrProcessStatsStateLocksByMethod byMethod; //ops by method
    } STNBMngrProcessStatsStateLocks;

    const STNBStructMap* NBMngrProcessStatsStateLocks_getSharedStructMap(void);

    //NBMngrProcessStatsStateConds

    typedef struct STNBMngrProcessStatsStateConds_ {
        UI64        created;
        UI64        destroyed;
        UI64        waitStarted;
        UI64        waitEnded;
        UI64        signals;
        UI64        broadcasts;
    } STNBMngrProcessStatsStateConds;

    const STNBStructMap* NBMngrProcessStatsStateConds_getSharedStructMap(void);

    //----------------------
    //- NBMngrProcessCodePos
    //----------------------

    typedef struct STNBMngrProcessCodePos_ {
        char*       fullpath;
        SI32        line;
        char*       func;
    } STNBMngrProcessCodePos;

    const STNBStructMap* NBMngrProcessCodePos_getSharedStructMap(void);

    void NBMngrProcessCodePos_init(STNBMngrProcessCodePos* obj);
    void NBMngrProcessCodePos_release(STNBMngrProcessCodePos* obj);
    void NBMngrProcessCodePos_set(STNBMngrProcessCodePos* obj, const char* fullpath, const SI32 line, const char* func);
    BOOL NBMngrProcessCodePos_isSame(STNBMngrProcessCodePos* obj, const STNBMngrProcessCodePos* other);
    void NBMngrProcessCodePos_clone(STNBMngrProcessCodePos* obj, const STNBMngrProcessCodePos* other);

    //--------------------------
    //- NBMngrProcessCodePosPair
    //--------------------------

    typedef struct STNBMngrProcessCodePosPair_ {
        STNBMngrProcessCodePos  one;
        STNBMngrProcessCodePos  other;
        UI32                    count;
    } STNBMngrProcessCodePosPair;

    const STNBStructMap* NBMngrProcessCodePosPair_getSharedStructMap(void);

    void NBMngrProcessCodePosPair_init(STNBMngrProcessCodePosPair* obj);
    void NBMngrProcessCodePosPair_release(STNBMngrProcessCodePosPair* obj);
    void NBMngrProcessCodePosPair_set(STNBMngrProcessCodePosPair* obj, const STNBMngrProcessCodePos* one, const STNBMngrProcessCodePos* other);
    BOOL NBMngrProcessCodePosPair_isSame(STNBMngrProcessCodePosPair* obj, const STNBMngrProcessCodePos* one, const STNBMngrProcessCodePos* other);
    void NBMngrProcessCodePosPair_clone(STNBMngrProcessCodePosPair* obj, const STNBMngrProcessCodePosPair* other);

    //NBMngrProcessStatsStateYields

    typedef struct STNBMngrProcessStatsStateYields_ {
        STNBMngrProcessCodePosPair* list;
        SI32                        use;
        SI32                        size;   //alloc optimization (not a struct member)
    } STNBMngrProcessStatsStateYields;

    const STNBStructMap* NBMngrProcessStatsStateYields_getSharedStructMap(void);

	//

	typedef struct STNBMngrProcessStatsState_ {
        STNBMngrProcessStatsStateMem    mem;    //memory
        STNBMngrProcessStatsStateLocks  locks;  //locks
        STNBMngrProcessStatsStateConds  conds;  //conds
        STNBMngrProcessStatsStateYields yields; //forced yield codePos
		STNBFilesystemStatsState        fs;     //filesystem
	} STNBMngrProcessStatsState;

    const STNBStructMap* NBMngrProcessStatsState_getSharedStructMap(void);

	void NBMngrProcessStatsState_init(STNBMngrProcessStatsState* obj);
	void NBMngrProcessStatsState_release(STNBMngrProcessStatsState* obj);
	void NBMngrProcessStatsState_add(STNBMngrProcessStatsState* obj, const STNBMngrProcessStatsState* other, const BOOL includeLocksByMethod);
	void NBMngrProcessStatsState_mutexLocked(STNBMngrProcessStatsState* obj, const char* func);
	void NBMngrProcessStatsState_mutexYielded(STNBMngrProcessStatsState* obj, const char* func);
	void NBMngrProcessStatsState_mutexUnlocked(STNBMngrProcessStatsState* obj, const char* func);


	typedef struct STNBMngrProcessStatsData_ {
		STNBMngrProcessStatsState	alive;
		STNBMngrProcessStatsState	accum;
		STNBMngrProcessStatsState	total;
	} STNBMngrProcessStatsData;

    const STNBStructMap* NBMngrProcessStatsData_getSharedStructMap(void);

	void NBMngrProcessStatsData_init(STNBMngrProcessStatsData* obj);
	void NBMngrProcessStatsData_release(STNBMngrProcessStatsData* obj);
	void NBMngrProcessStatsData_add(STNBMngrProcessStatsData* obj, const STNBMngrProcessStatsData* other, const BOOL includeLocksByMethod);
	
	//---------------
	//- NBMngrProcess
	//---------------

	void NBMngrProcess_init(void);
	void NBMngrProcess_release(void);
	
	//Assert
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	void NBMngrProcess_assertFailed(const char* expression, const char* filepath, const SI32 line, const char* func);
#	endif

	//Print
	void NBMngrProcess_printfInfo(const char* fmt, ...);
	void NBMngrProcess_printfInfoV(const char* fmt, va_list vargs);
	void NBMngrProcess_printfWarn(const char* fmt, ...);
	void NBMngrProcess_printfWarnV(const char* fmt, va_list vargs);
	void NBMngrProcess_printfError(const char* fmt, ...);
	void NBMngrProcess_printfErrorV(const char* fmt, va_list vargs);

#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	//thread
#	define NBMngrProcess_getCurThreadId()		NBMngrProcess_getCurThreadId_(__FILE__, __LINE__, __func__) 
	UI64 NBMngrProcess_getCurThreadId_(const char* fullpath, const SI32 line, const char* func);
#	define NBMngrProcess_setCurThreadAsExplicit() NBMngrProcess_setCurThreadAsExplicit_(__FILE__, __LINE__, __func__) 
	void NBMngrProcess_setCurThreadAsExplicit_(const char* fullpath, const SI32 line, const char* func);
	//mutex
	UI64 NBMngrProcess_mutexCreated(const char* name, const char* fullpath, const SI32 line, const char* func);
	void NBMngrProcess_mutexDestroyed(const UI64 lockId);
	void NBMngrProcess_mutexLockPush(const UI64 lockId, const char* fullpath, const SI32 line, const char* func, const ENNBThreadLockPushMode pushMode);
	void NBMngrProcess_mutexLockStarted(const UI64 lockId, const char* fullpath, const SI32 line, const char* func);
	void NBMngrProcess_mutexLockPop(const UI64 lockId, const char* fullpath, const SI32 line, const char* func);
	//conds
	UI64 NBMngrProcess_condCreated(const char* name, const char* fullpath, const SI32 line, const char* func);
	void NBMngrProcess_condDestroyed(const UI64 condId);
	void NBMngrProcess_condWaitPush(const UI64 condId, const UI64 lockId, const char* fullpath, const SI32 line, const char* func);
	void NBMngrProcess_condWaitPop(const UI64 condId, const UI64 lockId, const char* fullpath, const SI32 line, const char* func);
	void NBMngrProcess_condSignal(const UI64 condId, const char* fullpath, const SI32 line, const char* func);
	void NBMngrProcess_condBroadcast(const UI64 condId, const char* fullpath, const SI32 line, const char* func);
	//memory
	void NBMngrProcess_memStatsEnabledThreadPush(const BOOL isEnabled, const char* fullpath, const SI32 line, const char* func);
	void NBMngrProcess_memStatsEnabledThreadPop(const char* fullpath, const SI32 line, const char* func);
	void NBMngrProcess_memAllocated(const UI64 size, const char* fullpath, const SI32 line, const char* func);
	void NBMngrProcess_memFreed(const char* fullpath, const SI32 line, const char* func);
    //handles
    UI64 NBMngrProcess_hndlCreated(const ENNBHndlNativeType type, const char* fullpath, const SI32 line, const char* func);
    void NBMngrProcess_hndlClosed(const UI64 hndlId);
    SI64 NBMngrProcess_getHndlsCount(void);
    SI64 NBMngrProcess_getHndlsByTypeCount(const ENNBHndlNativeType type);
    void NBMngrProcess_getHndlsByTypesCounts(SI64* dst, const SI32 dstSz);
    //objs (NBObj)
    UI64 NBMngrProcess_objCreated(const char* name, const char* fullpath, const SI32 line, const char* func);
    void NBMngrProcess_objDestroyed(const UI64 hndlId);
	//folders
    SI64 NBMngrProcess_fsGetFoldersOpenCount(void); //currently open
	void NBMngrProcess_fsFolderCreated(const ENNBFilesystemStatsSrc iSrc, const ENNBFilesystemStatsResult iResult);
	void NBMngrProcess_fsFolderDeleted(const ENNBFilesystemStatsSrc iSrc, const ENNBFilesystemStatsResult iResult);
	void NBMngrProcess_fsFolderOpened(const ENNBFilesystemStatsSrc iSrc, const ENNBFilesystemStatsResult iResult);
	void NBMngrProcess_fsFolderRead(const ENNBFilesystemStatsSrc iSrc, const ENNBFilesystemStatsResult iResult);
	void NBMngrProcess_fsFolderClosed(const ENNBFilesystemStatsSrc iSrc, const ENNBFilesystemStatsResult iResult);
	//files
    SI64 NBMngrProcess_fsGetFilesOpenCount(void); //currently open
	void NBMngrProcess_fsFileOpened(const ENNBFilesystemStatsSrc iSrc, const ENNBFilesystemStatsResult iResult);
	void NBMngrProcess_fsFileClosed(const ENNBFilesystemStatsSrc iSrc, const ENNBFilesystemStatsResult iResult);
	void NBMngrProcess_fsFileRead(const ENNBFilesystemStatsSrc iSrc, const ENNBFilesystemStatsResult iResult, const UI64 bytes);
	void NBMngrProcess_fsFileWritten(const ENNBFilesystemStatsSrc iSrc, const ENNBFilesystemStatsResult iResult, const UI64 bytes);
	void NBMngrProcess_fsFileSeeked(const ENNBFilesystemStatsSrc iSrc, const ENNBFilesystemStatsResult iResult);
	void NBMngrProcess_fsFileTell(const ENNBFilesystemStatsSrc iSrc, const ENNBFilesystemStatsResult iResult);
	void NBMngrProcess_fsFileFlush(const ENNBFilesystemStatsSrc iSrc, const ENNBFilesystemStatsResult iResult);
	void NBMngrProcess_fsFileStat(const ENNBFilesystemStatsSrc iSrc, const ENNBFilesystemStatsResult iResult);
	void NBMngrProcess_fsFileMoved(const ENNBFilesystemStatsSrc iSrc, const ENNBFilesystemStatsResult iResult);
	void NBMngrProcess_fsFileDeleted(const ENNBFilesystemStatsSrc iSrc, const ENNBFilesystemStatsResult iResult);
	//locks
#	ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION
	void NBMngrProcess_mutexStackConcat(void* dst /*STNBString*/, const char* itmSeparator);
#	endif
    //per-thread-storages (STNBThreadStorage)
#   ifdef CONFIG_NB_INCLUDE_THREADS_STORAGE_CLEANUP
    void NBMngrProcess_storageAddCreated(void* data /*STNBThreadStorageData*/);    //at current thread
    void NBMngrProcess_storageRemoveDestroyed(void* data /*STNBThreadStorageData*/);  //at current thread
    void NBMngrProcess_storageDestroyAll(void); //at current thread
#   endif
	//stats
#	ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
	void NBMngrProcess_getGlobalStatsData(STNBMngrProcessStatsData* dst, const BOOL includeLocksByMethod, const BOOL resetAccum);
	void NBMngrProcess_getCurThreadStatsData(STNBMngrProcessStatsData* dst, const BOOL includeLocksByMethod, const BOOL resetAccum);
	void NBMngrProcess_getThreadStatsDataByFirstKwnonFuncName(const char* funcName, STNBMngrProcessStatsData* dst, const BOOL includeLocksByMethod, const BOOL resetAccum);
#	endif
	void NBMngrProcess_concatStatsData(const STNBMngrProcessStatsData* data, const ENNBLogLevel logLvl, const BOOL loaded, const BOOL accum, const BOOL total, const char* prefixFirst, const char* prefixOthers, const BOOL ignoreEmpties, void* dstString);
	void NBMngrProcess_concatStatsState(const STNBMngrProcessStatsState* state, const ENNBLogLevel logLvl, const char* prefixFirst, const char* prefixOthers, const BOOL ignoreEmpties, void* dstString);
	void NBMngrProcess_concatFilesystemStatsState(const STNBFilesystemStatsState* state, const ENNBLogLevel logLvl, const char* prefixFirst, const char* prefixOthers, const BOOL ignoreEmpties, void* dstString);
	void NBMngrProcess_concatFilesystemOps(const STNBFilesystemStatsOps* obj, const ENNBLogLevel logLvl, const char* prefixFirst, const char* prefixOthers, const BOOL ignoreEmpties, void* dstString);
	void NBMngrProcess_concatFolderOps(const STNBFilesystemStatsFoldersOps* obj, const ENNBLogLevel logLvl, const char* prefixFirst, const char* prefixOthers, const BOOL ignoreEmpties, void* dstString);
	void NBMngrProcess_concatFilesOps(const STNBFilesystemStatsFilesOps* obj, const ENNBLogLevel logLvl, const char* prefixFirst, const char* prefixOthers, const BOOL ignoreEmpties, void* dstString);
#	endif //CONFIG_NB_INCLUDE_THREADS_ENABLED

#ifdef __cplusplus
} //extern "C"
#endif

#endif
