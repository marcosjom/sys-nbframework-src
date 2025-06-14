
#include "nb/NBFrameworkPch.h"
#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBMemory.h"
//
#include "nb/core/NBStructMaps.h"
#include "nb/core/NBMngrStructMaps.h"

#ifdef CONFIG_NB_INCLUDE_THREADS_STORAGE_CLEANUP
#   include "../src/core/NBThreadStoragePriv.h" //for NBThreadStorage_destroyFromData()
#endif

//Complementary PCH
#ifdef _WIN32
#	ifdef NB_COMPILE_DRIVER_MODE
//#		include <wdm.h> //for 'vDbgPrintEx', 'DPFLTR_*_LEVEL' and 'DPFLTR_IHVDRIVER_ID'
#		include <ntddk.h> //for 'vDbgPrintEx', 'DPFLTR_*_LEVEL' and 'DPFLTR_IHVDRIVER_ID'
#	else
//#		define WIN32_LEAN_AND_MEAN
#		include <windows.h>	//for CRITICAL_SECTION
#	endif
#else
#	include <string.h>		//for memset()
#	include <errno.h>		//for ETIMEDOUT
#endif

/*#include "nb/NBFrameworkPch.h"
#include "nb/NBFrameworkDefs.h"
//Complementary PCH
#ifdef _WIN32
//#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>	//for CRITICAL_SECTION
#	ifdef NB_COMPILE_DRIVER_MODE
//#		include <Wdm.h> //for 'vDbgPrintEx', 'DPFLTR_*_LEVEL' and 'DPFLTR_IHVDRIVER_ID'
#		include <ntddk.h> //for 'vDbgPrintEx', 'DPFLTR_*_LEVEL' and 'DPFLTR_IHVDRIVER_ID'
#	endif
#else
#	include <string.h>		//for memset()
#endif
*/

#if defined(__ANDROID__)
#	include <android/log.h>	//for __android_log_print()
#endif
#include <stdio.h>			//for FILE
#include <stdarg.h>			//for va_list
#include <assert.h>			//for assert()
#include <time.h>			//for struct tm
#include <stdlib.h>			//for abort();
#ifdef _WIN32
    //nothing
#else
#    include <pthread.h>
#endif
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBMngrProcess.h"
//#include "nb/core/NBThreadStorage.h"
//avoid NBMemory usage (also objects that uses it)

#ifdef NB_CONFIG_INCLUDE_ASSERTS
#	define	NB_STATIC_ASSERT(EXP)	assert(EXP);
#else
#	define	NB_STATIC_ASSERT(EXP)
#endif

struct STNBMngrProcess_;
struct STNBMngrProcessThread_;

typedef enum ENNBPrintType_ {
	ENNBPrintType_Info = 0,	//Info and verbose
	ENNBPrintType_Warn,		//Warnings
	ENNBPrintType_Err,		//Errors
	ENNBPrintType_Count
} ENNBPrintType;

typedef struct STNBPrintDateStr_ {
	char str[24];			//"YYYY-MM-DD hh:mm:ss.123\0"
} STNBPrintDateStr;

void NBMngrProcess_printfv_(const ENNBPrintType type, const char* fmt, va_list vargs);
STNBPrintDateStr NBMngrProcess_getPrintDateStr_(void);

//Thread local storage
#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
typedef struct STNBStatePerThread_ {
#   ifdef _WIN32
    UI32             tlsIdx;
#   else
    pthread_once_t   once;
    pthread_key_t    key;
#   endif
} STNBStatePerThread;
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
#   ifdef _WIN32
static STNBStatePerThread _statePerThread = { 0xFFFFFFFF /*TLS_OUT_OF_INDEXES, defined in WinBase.h*/ };
#   else
static STNBStatePerThread _statePerThread = { PTHREAD_ONCE_INIT };
#   endif
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBStatePerThreadCreateMthd_(void);
void NBStatePerThreadDestroyDataMthd_(void* data); //automtically called in some systems.
//
struct STNBMngrProcessThread_* NBStatePerThread_get(const char* fullpath, const SI32 line, const char* func, const BOOL createIfNecesary);
#endif

//Filesystem

typedef struct STNBFilesystemStatsSrcDef_ {
	ENNBFilesystemStatsSrc	iSrc;
	const char*				name;
} STNBFilesystemStatsSrcDef;

const static STNBFilesystemStatsSrcDef __fsStatsSrcDefs[] = {
	{ ENNBFilesystemStatsSrc_Native, "native" }
	, { ENNBFilesystemStatsSrc_Abstract, "abstract" }
};

typedef struct STNBFilesystemStatsResultDef_ {
	ENNBFilesystemStatsResult	iResult;
	const char*					name;
} STNBFilesystemStatsResultDef;

const static STNBFilesystemStatsResultDef __fsStatsResultDefs[] = {
	{ ENNBFilesystemStatsResult_Success, "sucess" }
	, { ENNBFilesystemStatsResult_Error, "error" }
};

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
#	define NBMngrProcess_lock(OBJ)				NBMngrProcessMutexx_lock_(&(OBJ)->mutex)
#	define NBMngrProcess_unlock(OBJ)			NBMngrProcessMutexx_unlock_(&(OBJ)->mutex)
#	define NBMngrProcess_wait(OBJ)				NBMngrProcessMutexx_wait_(&(OBJ)->mutex)
#	define NBMngrProcess_timedWait(OBJ, MSMAX)	NBMngrProcessMutexx_timedWait_(&(OBJ)->mutex, MSMAX)
#	define NBMngrProcess_signal(OBJ)			NBMngrProcessMutexx_signal_(&(OBJ)->mutex)
#	define NBMngrProcess_broadcast(OBJ)			NBMngrProcessMutexx_broadcast_(&(OBJ)->mutex)
#else
#	define NBMngrProcess_lock(OBJ)				(0)
#	define NBMngrProcess_unlock(OBJ)			(0)
#	define NBMngrProcess_wait(OBJ)				(0)
#	define NBMngrProcess_timedWait(OBJ, MSMAX)	(0)
#	define NBMngrProcess_signal(OBJ)			(0)
#	define NBMngrProcess_broadcast(OBJ)			(0)
#endif

//----------------------
//- NBMngrProcessLockPos
//----------------------

typedef struct STNBMngrProcessLockPos_ {
	STNBMngrProcessCodePos      codePos;
	UI64					lockId;
	ENNBThreadLockPushMode	pushMode;
	BOOL					started;	//lock execution started
} STNBMngrProcessLockPos;

void NBMngrProcessLockPos_init(STNBMngrProcessLockPos* obj);
void NBMngrProcessLockPos_release(STNBMngrProcessLockPos* obj);
void NBMngrProcessLockPos_set(STNBMngrProcessLockPos* obj, const char* fullpath, const SI32 line, const char* func, const UI64 lockId, const ENNBThreadLockPushMode	pushMode);
void NBMngrProcessLockPos_clone(STNBMngrProcessLockPos* obj, const STNBMngrProcessLockPos* other); 

//------------------------
//- NBMngrProcessThreadPos
//------------------------

typedef struct STNBMngrProcessThreadPos_ {
	STNBMngrProcessCodePos			codePos;
	struct STNBMngrProcessThread_*	thread;
	ENNBThreadLockPushMode			pushMode;
	BOOL							yieldAnalized;	//thread already count as yield
} STNBMngrProcessThreadPos;

void NBMngrProcessThreadPos_init(STNBMngrProcessThreadPos* obj);
void NBMngrProcessThreadPos_release(STNBMngrProcessThreadPos* obj);
void NBMngrProcessThreadPos_set(STNBMngrProcessThreadPos* obj, const char* fullpath, const SI32 line, const char* func, struct STNBMngrProcessThread_* thread, const ENNBThreadLockPushMode pushMode);
void NBMngrProcessThreadPos_clone(STNBMngrProcessThreadPos* obj, const STNBMngrProcessThreadPos* other);

//---------------------
//- NBMngrProcessThread
//---------------------
	
typedef struct STNBMngrProcessThread_ {
	//first known
	BOOL						isExplicit;		//Non-explicit threads were created by code outside this framework. Their existence is known thanks to this framework code called inside that external thread. The main-thread is one of those external threads.
	STNBMngrProcessCodePos		firstKwnonCall;
	//locks stack
#	ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION
	struct {
		STNBMngrProcessLockPos*     stack;
		SI32					use;
		SI32					size;
	} locks;
#	endif
    //storages
#   ifdef CONFIG_NB_INCLUDE_THREADS_STORAGE_CLEANUP
    struct {
        void**                  arr;
        SI32                    use;
        SI32                    size;
    } storages;
#   endif
	//stats
#	ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
	struct {
		//memEnabled
		struct {
			BOOL*				stack;
			UI32				use;	//filled
			UI32				size;	//allocated
		} memEnabled;
		//data
		STNBMngrProcessStatsData data;
	} stats;
#	endif
} STNBMngrProcessThread;

void NBMngrProcessThread_init(STNBMngrProcessThread* obj);
void NBMngrProcessThread_release(STNBMngrProcessThread* obj);
void NBMngrProcessThread_setFirstKnownFunc(STNBMngrProcessThread* obj, const char* fullpath, const SI32 line, const char* func);
#ifdef CONFIG_NB_INCLUDE_THREADS_STORAGE_CLEANUP
void NBMngrProcessThread_storageDestroyAllLocked_(STNBMngrProcessThread* obj);
#endif
//-----------------------
//- NBMngrProcessMutexRef
//-----------------------

typedef struct STNBMngrProcessMutexRef_ {
	UI64		uid;
	//stack
	struct {
		STNBMngrProcessLockPos* stack;
		UI32	size;
	} firstKnown;
} STNBMngrProcessMutexRef;

void NBMngrProcessMutexRef_init(STNBMngrProcessMutexRef* obj);
void NBMngrProcessMutexRef_release(STNBMngrProcessMutexRef* obj);
void NBMngrProcessMutexRef_setFirstKwnonStack(STNBMngrProcessMutexRef* obj, const STNBMngrProcessLockPos* stack, const UI32 stackSz);

//--------------------
//- NBMngrProcessMutex
//--------------------

typedef struct STNBMngrProcessMutex_ {
	UI64		uid;
	char*		name;
	//first known
	STNBMngrProcessCodePos firstKwnonCall;
	//threads racing for the mutex
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	struct {
		STNBMngrProcessThread*		owner;
		STNBMngrProcessThreadPos*	threads;
		SI32	use;
		SI32	size;
	} racing;
#	endif
	//locks after this one
#	ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_RISK_VALIDATION
	struct {
		STNBMngrProcessMutexRef* after;
		SI32	use;
		SI32	size;
	} locks;
#	endif
} STNBMngrProcessMutex;

void NBMngrProcessMutex_init(STNBMngrProcessMutex* obj);
void NBMngrProcessMutex_release(STNBMngrProcessMutex* obj);
void NBMngrProcessMutex_setName(STNBMngrProcessMutex* obj, const char* name);
void NBMngrProcessMutex_setFirstKnownFunc(STNBMngrProcessMutex* obj, const char* fullpath, const SI32 line, const char* func);
#ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_RISK_VALIDATION
void NBMngrProcessMutex_addLockAfter(STNBMngrProcessMutex* obj, const STNBMngrProcessMutex* other, const STNBMngrProcessThread* st, STNBMngrProcessMutex* locks, const UI32 locksSz, const UI64 locksMaxUID, const UI64 lockId, const char* fullpath, const SI32 line, const char* func);
#endif
#ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION
void NBMngrProcessMutex_printLocksStackConsoleErrorLine(const char* linesPrefix, const SI32 iStack, const SI32 stackSz, const char* tFunc, const char* tFullpath, const SI32 tLine, const UI64 lockId, const char* lockFirstKnownCallFunc, const char* lockName);
void NBMngrProcessMutex_printLocksStackConsoleError(const char* linesPrefix, const STNBMngrProcessLockPos* arr, const UI32 sz, STNBMngrProcessMutex* locks, const UI32 locksSz, const UI64 locksMaxUID);
#endif

//--------------------
//- NBMngrProcessCond
//--------------------

typedef struct STNBMngrProcessCond_ {
	UI64		uid;
	char*		name;
	//first known
	UI64		firstLockId; //first lock known (should be the same for all calls)
	STNBMngrProcessCodePos firstKwnonCall; //first method that used this cond
} STNBMngrProcessCond;

void NBMngrProcessCond_init(STNBMngrProcessCond* obj);
void NBMngrProcessCond_release(STNBMngrProcessCond* obj);
void NBMngrProcessCond_setName(STNBMngrProcessCond* obj, const char* name);
void NBMngrProcessCond_setFirstKnownFunc(STNBMngrProcessCond* obj, const char* fullpath, const SI32 line, const char* func);

//--------------------
//- NBMngrProcessHndl
//--------------------

typedef struct STNBMngrProcessHndl_ {
    UI64        uid;
    ENNBHndlNativeType type;
    //first known
    STNBMngrProcessCodePos firstKwnonCall;
} STNBMngrProcessHndl;

void NBMngrProcessHndl_init(STNBMngrProcessHndl* obj);
void NBMngrProcessHndl_release(STNBMngrProcessHndl* obj);
void NBMngrProcessHndl_setFirstKnownFunc(STNBMngrProcessHndl* obj, const char* fullpath, const SI32 line, const char* func);

//--------------------
//- NBMngrProcessObj
//--------------------

typedef struct STNBMngrProcessObj_ {
    UI64        uid;
    char*       name;
    //first known
    STNBMngrProcessCodePos firstKwnonCall;
#   ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION
    STNBMngrProcessCodePos firstKwnonLockPos;
#   endif
} STNBMngrProcessObj;

void NBMngrProcessObj_init(STNBMngrProcessObj* obj);
void NBMngrProcessObj_release(STNBMngrProcessObj* obj);
void NBMngrProcessObj_setName(STNBMngrProcessObj* obj, const char* name);
void NBMngrProcessObj_setFirstKnownFunc(STNBMngrProcessObj* obj, const char* fullpath, const SI32 line, const char* func);
#ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION
void NBMngrProcessObj_setFirstKnownLockPos(STNBMngrProcessObj* obj, const char* fullpath, const SI32 line, const char* func);
#endif

//------------
//- Mutex/cond
//------------

typedef struct STNBMngrProcessMutexx_ {
#	ifdef _WIN32
#		ifndef NB_COMPILE_DRIVER_MODE
		CRITICAL_SECTION	mutex;
#		else
		BOOL				mutexDummy;
#		endif
#	else
	pthread_mutex_t		mutex;
#	endif
	//
#	ifdef _WIN32
#		ifndef NB_COMPILE_DRIVER_MODE
			CONDITION_VARIABLE	cond; //Windows Vista
#		else
			BOOL				condDummy;
#		endif
#	else
	pthread_cond_t		cond;
#	endif
} STNBMngrProcessMutexx;

void NBMngrProcessMutexx_init_(STNBMngrProcessMutexx* obj);
void NBMngrProcessMutexx_release_(STNBMngrProcessMutexx* obj);
void NBMngrProcessMutexx_lock_(STNBMngrProcessMutexx* obj);
void NBMngrProcessMutexx_unlock_(STNBMngrProcessMutexx* obj);
BOOL NBMngrProcessMutexx_wait_(STNBMngrProcessMutexx* obj);
BOOL NBMngrProcessMutexx_timedWait_(STNBMngrProcessMutexx* obj, const UI32 msMax);
BOOL NBMngrProcessMutexx_signal_(STNBMngrProcessMutexx* obj);
BOOL NBMngrProcessMutexx_broadcast_(STNBMngrProcessMutexx* obj);

//--------------
//- Mutex method
//--------------

void NBMngrProcessMutexMethod_init_(STNBMngrProcessMutexx* obj);
void NBMngrProcessMutexMethod_release_(STNBMngrProcessMutexx* obj);
void NBMngrProcessMutexMethod_lock_(STNBMngrProcessMutexx* obj);
void NBMngrProcessMutexMethod_unlock_(STNBMngrProcessMutexx* obj);
BOOL NBMngrProcessMutexMethod_wait_(STNBMngrProcessMutexx* obj);
BOOL NBMngrProcessMutexMethod_signal_(STNBMngrProcessMutexx* obj);
BOOL NBMngrProcessMutexMethod_broadcast_(STNBMngrProcessMutexx* obj);

//---------------
//- NBMngrProcess
//---------------

typedef struct STNBMngrProcess_ {
	UI32 dummy;	//to avoid empty struct
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	//mutex/cond
	STNBMngrProcessMutexx       mutex;
	//threads
	struct {
		STNBMngrProcessThread*  main;		//main thread object
		STNBMngrProcessThread** arr;
		SI32                use;
		SI32                size;
		BOOL                waitingForAll;	//flag
	} threads;
#	endif
    //hndls
#   ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
    struct {
        UI64                seqUIDs;    //unique-ids
        STNBMngrProcessHndl*    arr;
        SI32                use;
        SI32                size;
    } hndls;
#   endif
    //objs
#   ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
    struct {
        UI64                seqUIDs;    //unique-ids
        STNBMngrProcessObj*     arr;
        SI32                use;
        SI32                size;
    } objs;
#   endif
    //locks
#	ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION
	struct {
		UI64                seqUIDs;	//unique-ids
		STNBMngrProcessMutex*   arr;
		SI32                use;
		SI32                size;
	} locks;
#	endif
	//conds
#	ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION
	struct {
		UI64                seqUIDs;	//unique-ids
		STNBMngrProcessCond*    arr;
		SI32                use;
		SI32                size;
	} conds;
#	endif
	//stats
#	ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
	STNBMngrProcessStatsData    stats;	//global
#	endif
} STNBMngrProcess;

//-----------------
//- Threads records
//-----------------

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBMngrProcess_threadAdd_(STNBMngrProcess* obj, STNBMngrProcessThread* t);
void NBMngrProcess_threadRemove_(STNBMngrProcess* obj, STNBMngrProcessThread* t);
#endif

//---------------
//- Mutex records
//---------------

#ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION
STNBMngrProcessMutex* NBMngrProcess_getMutexLocked_(STNBMngrProcess* obj, const UI64 lockId);
STNBMngrProcessMutex* NBMngrProcess_getMutexFromArr_(STNBMngrProcessMutex* arr, const UI32 arrSz, const UI64 lockId, const UI64 maxUID);
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION
void NBMngrProcess_lockPushLocked_(STNBMngrProcess* obj, STNBMngrProcessThread* st, const UI64 lockId, const char* fullpath, const SI32 line, const char* func, const ENNBThreadLockPushMode pushMode);
void NBMngrProcess_lockStartedLocked_(STNBMngrProcess* obj, STNBMngrProcessThread* st, const UI64 lockId, const char* fullpath, const SI32 line, const char* func);
void NBMngrProcess_lockPopLocked_(STNBMngrProcess* obj, STNBMngrProcessThread* st, const UI64 lockId, const char* fullpath, const SI32 line, const char* func);
#endif

//---------------
//- Conds records
//---------------

#ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION
STNBMngrProcessCond* NBMngrProcess_getCondLocked_(STNBMngrProcess* obj, const UI64 condId);
STNBMngrProcessCond* NBMngrProcess_getCondFromArr_(STNBMngrProcessCond* arr, const UI32 arrSz, const UI64 condId, const UI64 maxUID);
#endif

//-------
//- Tools
//-------

//strings

const char* NBMngrProcess_fileNameOnly_(const char* fullpath);
UI32 NBMngrProcess_startOfFileName_(const char* fullpath);
char* NBMngrProcess_strClone_(const char* string);
UI32 NBMngrProcess_strLenBytes_(const char* string);
BOOL NBMngrProcess_strIsEmpty_(const char* str);
BOOL NBMngrProcess_strIsEqual_(const char* str1, const char* str2);
BOOL NBMngrProcess_strIsLower_(const char* str1, const char* str2);
BOOL NBMngrProcess_strIsLowerOrEqual_(const char* str1, const char* str2);
BOOL NBMngrProcess_strIsGreater_(const char* str1, const char* str2);
BOOL NBMngrProcess_strIsGreaterOrEqual_(const char* str1, const char* str2);

//sorted array

SI32 NBMngrProcess_sortedIndexOf_(void** buff, SI32* buffUse, SI32* buffSz, const SI32 bytesPerItem, NBCompareFunc cmpFunc, const void* data, const SI32 dataSz);
SI32 NBMngrProcess_sortedIndexForNew_(void** buff, SI32* buffUse, SI32* buffSz, const SI32 bytesPerItem, NBCompareFunc cmpFunc, const void* dataToInsert);
void NBMngrProcess_sortedGrowBuffer_(void** buff, SI32* buffUse, SI32* buffSz, const SI32 bytesPerItem, const SI32 qItems);
void* NBMngrProcess_sortedAdd_(void** buff, SI32* buffUse, SI32* buffSz, const SI32 bytesPerItem, NBCompareFunc cmpFunc, const void* data, const SI32 itemSize);

//

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
static STNBMngrProcess* __nbMngrThreads = NULL;
#endif

static STNBMngrProcessMutexx* __printfMutex = NULL;
//format with prefix injected
static char* __printfFmtTmp = NULL;
static UI32 __printfFmtTmpSz = 0;
//final string with results applied
static char* __printfFinalTmp = NULL;
static UI32 __printfFinalTmpSz = 0;

void NBMngrProcess_init(void){
	//printf mutex
	{
		__printfMutex		= (STNBMngrProcessMutexx*)NBMemory_allocUnmanaged(sizeof(STNBMngrProcessMutexx));
		NBMngrProcessMutexx_init_(__printfMutex);
	}
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	NB_STATIC_ASSERT(__nbMngrThreads == NULL)
	if(__nbMngrThreads == NULL){
		STNBMngrProcess* obj = __nbMngrThreads = (STNBMngrProcess*)NBMemory_allocUnmanaged(sizeof(STNBMngrProcess));
		memset(obj, 0, sizeof(*obj));
		//mutex
		NBMngrProcessMutexx_init_(&obj->mutex);
		//threads
		{
			obj->threads.main = NBStatePerThread_get(__FILE__, __LINE__, __func__, TRUE);
		}
		//stats
#		ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
		{
			NBMngrProcessStatsData_init(&obj->stats);
		}
#		endif
	}
#	endif
}

void NBMngrProcess_release(void){
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	NB_STATIC_ASSERT(__nbMngrThreads != NULL)
	if(__nbMngrThreads != NULL){
		STNBMngrProcess* obj = __nbMngrThreads;
		NBMngrProcess_lock(obj);
		//threads
		{
#			if !defined(_WIN32) //in windows thread-local-vars are not released aytomatically
			while(obj->threads.use != 0){
				//Remove main and non-explicit threads (if contains no objects)
				{
					SI32 i; for(i = (SI32)obj->threads.use - 1; i >= 0; i--){
						STNBMngrProcessThread* t2 = obj->threads.arr[i];
                        //Non-explicit threads were created by code outside this framework.
                        //Their existence is known thanks to this framework code called inside that external thread.
                        //The main-thread is one of those external threads.
						if((obj->threads.main == t2 || !t2->isExplicit) && t2->locks.use == 0){
							//remove
							{
								SI32 i2; for(i2 = (i + 1); i2 < obj->threads.use; i2++){
									obj->threads.arr[i2 - 1] = obj->threads.arr[i2];
								}
								obj->threads.use--;
							}
#                           ifdef CONFIG_NB_INCLUDE_THREADS_STORAGE_CLEANUP
                            NBMngrProcessThread_storageDestroyAllLocked_(t2);
#                           endif
							NBMngrProcessThread_release(t2);
							if(obj->threads.main == t2){
								obj->threads.main = NULL;
							}
							t2 = NULL;
						}
					}
				}
				//Wait for explicit threads, their cleanup is done outside this function.
				if(obj->threads.use != 0){
					PRINTF_INFO("NBMngrProcess, waiting for %d threads.\n", obj->threads.use);
					{
						SI32 i; for(i = 0; i < obj->threads.use; i++){
							const STNBMngrProcessThread* t2 = obj->threads.arr[i];
							PRINTF_INFO("NBMngrProcess,    %d/%d: %s (%s : line %d).\n", (i + 1), obj->threads.use, t2->firstKwnonCall.func, NBMngrProcess_fileNameOnly_(t2->firstKwnonCall.fullpath), t2->firstKwnonCall.line);
						}
					}
					obj->threads.waitingForAll = TRUE;
					NBMngrProcess_timedWait(obj, 1000);
				}
			}
			NB_STATIC_ASSERT(obj->threads.main == NULL)
			NB_STATIC_ASSERT(obj->threads.use == 0)
#			endif
			if(obj->threads.arr != NULL){
				NBMemory_freeUnmanaged(obj->threads.arr);
				obj->threads.arr = NULL;
			}
			obj->threads.use	= 0;
			obj->threads.size	= 0;
            //PRINTF_INFO("Threads-seqId: %llu.\n", obj->threads.seqUIDs);
		}
		//locks
#		ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION
		{
			if(obj->locks.arr != NULL){
				UI32 i; for(i = 0; i < obj->locks.use; i++){
					STNBMngrProcessMutex* itm = &obj->locks.arr[i];
                    PRINTF_WARNING("#%d/%d non-destroyed-lock(%llu) created at: %s; %s, line %d.\n", (i + 1), obj->locks.use, itm->uid, itm->firstKwnonCall.func, NBMngrProcess_fileNameOnly_(itm->firstKwnonCall.fullpath), itm->firstKwnonCall.line);
					NBMngrProcessMutex_release(itm);
				}
				NBMemory_freeUnmanaged(obj->locks.arr);
				obj->locks.arr = NULL;
			}
			obj->locks.use	= 0;
			obj->locks.size	= 0;
            PRINTF_INFO("Locks-seqId: %llu.\n", obj->locks.seqUIDs);
		}
#		endif
		//conds
#		ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION
		{
			if(obj->conds.arr != NULL){
				UI32 i; for(i = 0; i < obj->conds.use; i++){
					STNBMngrProcessCond* itm = &obj->conds.arr[i];
                    PRINTF_WARNING("#%d/%d non-destroyed-cond(%llu) created at: %s; %s, line %d.\n", (i + 1), obj->conds.use, itm->uid, itm->firstKwnonCall.func, NBMngrProcess_fileNameOnly_(itm->firstKwnonCall.fullpath), itm->firstKwnonCall.line);
					NBMngrProcessCond_release(itm);
				}
				NBMemory_freeUnmanaged(obj->conds.arr);
				obj->conds.arr = NULL;
			}
			obj->conds.use	= 0;
			obj->conds.size	= 0;
            PRINTF_INFO("Conds-seqId: %llu.\n", obj->conds.seqUIDs);
		}
#		endif
        //hndls
#       ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
        {
            if(obj->hndls.arr != NULL){
                UI32 i; for(i = 0; i < obj->hndls.use; i++){
                    STNBMngrProcessHndl* itm = &obj->hndls.arr[i];
                    PRINTF_WARNING("#%d/%d non-closed-hnl(%llu) created at: %s; %s, line %d.\n", (i + 1), obj->hndls.use, itm->uid, itm->firstKwnonCall.func, NBMngrProcess_fileNameOnly_(itm->firstKwnonCall.fullpath), itm->firstKwnonCall.line);
                    NBMngrProcessHndl_release(itm);
                }
                NBMemory_freeUnmanaged(obj->hndls.arr);
                obj->hndls.arr = NULL;
            }
            obj->hndls.use    = 0;
            obj->hndls.size   = 0;
            PRINTF_INFO("Hndls-seqId: %llu.\n", obj->hndls.seqUIDs);
        }
#       endif
        //objs
#       ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
        {
            if(obj->objs.arr != NULL){
                UI32 i; for(i = 0; i < obj->objs.use; i++){
                    STNBMngrProcessObj* itm = &obj->objs.arr[i];
#                   ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION
                    PRINTF_WARNING("#%d/%d non-released-obj(%llu, '%s') created at lock: %s; %s, line %d.\n", (i + 1), obj->objs.use, itm->uid, itm->name, itm->firstKwnonLockPos.func, NBMngrProcess_fileNameOnly_(itm->firstKwnonLockPos.fullpath), itm->firstKwnonLockPos.line);
#                   else
                    PRINTF_WARNING("#%d/%d non-released-obj(%llu, '%s') created at: %s; %s, line %d.\n", (i + 1), obj->objs.use, itm->uid, itm->name, itm->firstKwnonCall.func, NBMngrProcess_fileNameOnly_(itm->firstKwnonCall.fullpath), itm->firstKwnonCall.line);
#                   endif
                    NBMngrProcessObj_release(itm);
                }
                NBMemory_freeUnmanaged(obj->objs.arr);
                obj->objs.arr = NULL;
            }
            obj->objs.use    = 0;
            obj->objs.size   = 0;
            PRINTF_INFO("Objs-seqId: %llu.\n", obj->objs.seqUIDs);
        }
#       endif
		//stats
#		ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
		{
			NBMngrProcessStatsData_release(&obj->stats);
		}
#		endif
		NBMngrProcess_unlock(obj);
		//
#		if defined(CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION) || defined(CONFIG_NB_INCLUDE_THREADS_METRICS)
		NBMngrProcessMutexx_release_(&obj->mutex);
#		endif
		//
		NBMemory_freeUnmanaged(obj);
		__nbMngrThreads = NULL;
	}
#	endif
	//printf mutex
	{
		//final string with results applied
		if(__printfFinalTmp != NULL){
			NBMemory_freeUnmanaged(__printfFinalTmp);
			__printfFinalTmp = NULL;
			__printfFinalTmpSz = 0;
		}
		if(__printfFmtTmp != NULL){
			NBMemory_freeUnmanaged(__printfFmtTmp);
			__printfFmtTmp = NULL;
			__printfFmtTmpSz = 0;
		}
		if(__printfMutex != NULL){
			NBMngrProcessMutexx_release_(__printfMutex);
			NBMemory_freeUnmanaged(__printfMutex);
			__printfMutex = NULL;
		}
	}
}

//Assert
#ifdef NB_CONFIG_INCLUDE_ASSERTS
void NBMngrProcess_assertFailed(const char* expression, const char* filepath, const SI32 line, const char* func){
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	NB_STATIC_ASSERT(__nbMngrThreads != NULL)
	{
		STNBMngrProcess* obj = __nbMngrThreads;
		NBMngrProcess_lock(obj);
#		ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION
		{
			STNBMngrProcessThread* st = NBStatePerThread_get(__FILE__, __LINE__, __func__, FALSE);
			if(st != NULL){
				NBMngrProcessMutex_printLocksStackConsoleError("                 ", st->locks.stack, st->locks.use, obj->locks.arr, obj->locks.use, obj->locks.seqUIDs);
			}
		}
#		endif
		PRINTF_CONSOLE_ERROR("Assertion failed: '%s', func '%s', file '%s':%d.\n", expression, func, NBMngrProcess_fileNameOnly_(filepath), line);
		NBMngrProcess_unlock(obj);
		abort();
	}
#	else
	{
		PRINTF_CONSOLE_ERROR("Assertion failed: '%s', func '%s', file '%s':%d.\n", expression, func, NBMngrProcess_fileNameOnly_(filepath), line);
		abort();
	}
#	endif
}
#endif

//Print

void NBMngrProcess_printfv_(const ENNBPrintType type, const char* fmt, va_list vargs){
	char* fmt2 = (char*)fmt;
	BOOL vargsApplied = FALSE; int vargsAppliedLen = 0;
	NBMngrProcessMutexx_lock_(__printfMutex);
	//Build new format
	{
		const UI32 fmtLen = NBMngrProcess_strLenBytes_(fmt);
		const char* prefix0	= NULL; UI32 prefix0Sz = 0;
		const char* preDate	= NULL; UI32 preDateSz = 0;
		const char* prefix1 = NULL; UI32 prefix1Sz = 0;
		UI32 fmt2Sz = 0; STNBPrintDateStr dateStr;
		//Set prefixes
#		ifndef __ANDROID__
		{
			prefix0 = "AU, ";
			dateStr = NBMngrProcess_getPrintDateStr_();
			preDate = dateStr.str;
			switch(type) {
				case ENNBPrintType_Info: prefix1 = ", "; break;
				case ENNBPrintType_Warn: prefix1 = " WARN, "; break;
				case ENNBPrintType_Err: prefix1 = " ERROR, "; break;
				default: NB_STATIC_ASSERT(FALSE) break;
			}
		}
#		endif
		//size
		{
			if(prefix0 != NULL) fmt2Sz += (prefix0Sz = NBMngrProcess_strLenBytes_(prefix0)); 
			if(preDate != NULL) fmt2Sz += (preDateSz = NBMngrProcess_strLenBytes_(preDate));
			if(prefix1 != NULL) fmt2Sz += (prefix1Sz = NBMngrProcess_strLenBytes_(prefix1));
			fmt2Sz += fmtLen;
		}
		//Build new fmt
		if(fmt2Sz == fmtLen){
			fmt2 = (char*)fmt;
		} else {
			UI32 iPos = 0;
			if(__printfFmtTmpSz < (fmt2Sz + 1)){
				if(__printfFmtTmp != NULL){
					NBMemory_freeUnmanaged(__printfFmtTmp);
				}
				__printfFmtTmpSz	= (fmt2Sz + 1);
				__printfFmtTmp		= (char*)NBMemory_allocUnmanaged(__printfFmtTmpSz);
			}
			fmt2 = __printfFmtTmp;
			if(prefix0 != NULL){ memcpy(&fmt2[iPos], prefix0, prefix0Sz); iPos += prefix0Sz; }
			if(preDate != NULL){ memcpy(&fmt2[iPos], preDate, preDateSz); iPos += preDateSz; }
			if(prefix1 != NULL){ memcpy(&fmt2[iPos], prefix1, prefix1Sz); iPos += prefix1Sz; }
			if(fmt != NULL){ memcpy(&fmt2[iPos], fmt, fmtLen); iPos += fmtLen; }
			fmt2[iPos] = '\0';
		}
	}
	//build final string
	//ToDo: remove
//#	ifdef NB_COMPILE_DRIVER_MODE
	/*{
		int len = vsprintf(NULL, fmt2, vargs);
		if(len > 0){
			if(__printfFinalTmpSz < len){
				if(__printfFinalTmp != NULL){
					NBMemory_freeUnmanaged(__printfFinalTmp);
				}
				__printfFinalTmpSz	= len;
				__printfFinalTmp	= (char*)NBMemory_allocUnmanaged(__printfFinalTmpSz);
			}
			vargsAppliedLen	= vsprintf(__printfFinalTmp, fmt2, vargs); NB_STATIC_ASSERT(vargsAppliedLen <= __printfFinalTmpSz)
			if(vargsAppliedLen > 0){
				fmt2			= __printfFinalTmp;
				vargsApplied	= TRUE;
				vargsAppliedLen--;	//remove the NULL char
			}
		}
	}*/
//#	endif
	//print
	{
#		if defined(__ANDROID__)
		{
			switch(type) {
				case ENNBPrintType_Info:
					if(vargsApplied){
						__android_log_write(ANDROID_LOG_INFO, "NB", fmt2);
					} else {
						__android_log_vprint(ANDROID_LOG_INFO, "NB", fmt2, vargs);
					}
					//fflush(stdout);
					break;
				case ENNBPrintType_Warn:
					if(vargsApplied){
						__android_log_write(ANDROID_LOG_WARN, "NB", fmt2);
					} else {
						__android_log_vprint(ANDROID_LOG_WARN, "NB", fmt2, vargs);
					}
					//fflush(stdout);
					break;
				case ENNBPrintType_Err:
					if(vargsApplied){
						__android_log_write(ANDROID_LOG_ERROR, "NB", fmt2);
					} else {
						__android_log_vprint(ANDROID_LOG_ERROR, "NB", fmt2, vargs);
					}
					//fflush(stdout);
					break;
				default:
					NB_STATIC_ASSERT(FALSE)
					break;
			}
		}
#		elif defined(NB_COMPILE_DRIVER_MODE) && defined(_WIN32)
		{
			switch(type) {
				case ENNBPrintType_Info:
					if(vargsApplied){
						DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Non-printable msg");
					} else {
						vDbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, fmt2, vargs);
					}
					break;
				case ENNBPrintType_Warn:
					if(vargsApplied){
						DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Non-printable msg");
					} else {
						vDbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, fmt2, vargs);
					}
					break;
				case ENNBPrintType_Err:
					if(vargsApplied){
						DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Non-printable msg");
					} else {
						vDbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, fmt2, vargs);
					}
					break;
				default:
					NB_STATIC_ASSERT(FALSE)
					break;
				}
		}
#		else
		{
			switch(type) {
				case ENNBPrintType_Info:
					if(vargsApplied){
						fwrite(fmt2, vargsAppliedLen, 1, stdout);
					} else {
						vfprintf(stdout, fmt2, vargs);
					}
					fflush(stdout);
					break;
				case ENNBPrintType_Warn:
					if(vargsApplied){
						fwrite(fmt2, vargsAppliedLen, 1, stderr);
					} else {
						vfprintf(stderr, fmt2, vargs);
					}
					fflush(stderr);
					break;
				case ENNBPrintType_Err:
					if(vargsApplied){
						fwrite(fmt2, vargsAppliedLen, 1, stderr);
					} else {
						vfprintf(stderr, fmt2, vargs);
					}
					fflush(stderr);
					break;
				default:
					NB_STATIC_ASSERT(FALSE)
					break;
			}
		}
#		endif
	}
	NBMngrProcessMutexx_unlock_(__printfMutex);
}

void NBMngrProcess_printfInfo(const char* fmt, ...){
	va_list vargs;
	va_start(vargs, fmt);
	{
		NBMngrProcess_printfv_(ENNBPrintType_Info, fmt, vargs);
	}
	va_end(vargs);
}

void NBMngrProcess_printfInfoV(const char* fmt, va_list vargs){
	NBMngrProcess_printfv_(ENNBPrintType_Info, fmt, vargs);
}

void NBMngrProcess_printfWarn(const char* fmt, ...){
	va_list vargs;
	va_start(vargs, fmt);
	{
		NBMngrProcess_printfv_(ENNBPrintType_Warn, fmt, vargs);
	}
	va_end(vargs);
}

void NBMngrProcess_printfWarnV(const char* fmt, va_list vargs){
	NBMngrProcess_printfv_(ENNBPrintType_Warn, fmt, vargs);
}

void NBMngrProcess_printfError(const char* fmt, ...){
	va_list vargs;
	va_start(vargs, fmt);
	{
		NBMngrProcess_printfv_(ENNBPrintType_Err, fmt, vargs);
	}
	va_end(vargs);
}

void NBMngrProcess_printfErrorV(const char* fmt, va_list vargs){
	NBMngrProcess_printfv_(ENNBPrintType_Err, fmt, vargs);
}

UI32 NBMngrProcess_getPrintDateStrConcatUInt_(const UI32 value, const UI32 digitsMin, char* dst, const UI32 dstSz){
	UI32 r = 0, m10 = 1, digits = 1, v = value, vd = 0;
	//Calculate digits
	while(v >= (m10 * 10) || digits < digitsMin){
		m10 *= 10; digits++;
	}
	//Add digits
	while((r + 2) < dstSz && digits > 0){
		vd			= (v / m10);	NB_STATIC_ASSERT(vd >= 0 && vd <= 9)
		v			-= (vd * m10);	NB_STATIC_ASSERT(v >= 0 && v <= value)
		dst[r++]	= ('0' + vd);
		m10 /= 10; digits--;
	}
	//
	return r;
}

STNBPrintDateStr NBMngrProcess_getPrintDateStr_(void) {
	STNBPrintDateStr r;
	NBMemory_setZeroSt(r, STNBPrintDateStr);
#	ifdef _WIN32
	{
#		ifndef NB_COMPILE_DRIVER_MODE
		{
			BOOL keepAdd = TRUE;
			UI32 i = 0; char* dst = r.str;
			SYSTEMTIME t_st; GetLocalTime(&t_st);
			//Year
			if ((i + 4) < sizeof(r.str) && keepAdd) { i += NBMngrProcess_getPrintDateStrConcatUInt_(t_st.wYear, 4, &dst[i], (sizeof(r.str) - i)); } else { keepAdd = FALSE; }
			//Separator
			if ((i + 1) < sizeof(r.str) && keepAdd) { dst[i++] = '-'; } else { keepAdd = FALSE; }
			//Month
			if ((i + 2) < sizeof(r.str) && keepAdd) { i += NBMngrProcess_getPrintDateStrConcatUInt_(t_st.wMonth, 2, &dst[i], (sizeof(r.str) - i)); } else { keepAdd = FALSE; }
			//Separator
			if ((i + 1) < sizeof(r.str) && keepAdd) { dst[i++] = '-'; } else { keepAdd = FALSE; }
			//Day
			if ((i + 2) < sizeof(r.str) && keepAdd) { i += NBMngrProcess_getPrintDateStrConcatUInt_(t_st.wDay, 2, &dst[i], (sizeof(r.str) - i)); } else { keepAdd = FALSE; }
			//Separator
			if ((i + 1) < sizeof(r.str) && keepAdd) { dst[i++] = ' '; } else { keepAdd = FALSE; }
			//Hour
			if ((i + 2) < sizeof(r.str) && keepAdd) { i += NBMngrProcess_getPrintDateStrConcatUInt_(t_st.wHour, 2, &dst[i], (sizeof(r.str) - i)); } else { keepAdd = FALSE; }
			//Separator
			if ((i + 1) < sizeof(r.str) && keepAdd) { dst[i++] = ':'; } else { keepAdd = FALSE; }
			//Minute
			if ((i + 2) < sizeof(r.str) && keepAdd) { i += NBMngrProcess_getPrintDateStrConcatUInt_(t_st.wMinute, 2, &dst[i], (sizeof(r.str) - i)); } else { keepAdd = FALSE; }
			//Separator
			if ((i + 1) < sizeof(r.str) && keepAdd) { dst[i++] = ':'; } else { keepAdd = FALSE; }
			//Secs
			if ((i + 2) < sizeof(r.str) && keepAdd) { i += NBMngrProcess_getPrintDateStrConcatUInt_(t_st.wSecond, 2, &dst[i], (sizeof(r.str) - i)); } else { keepAdd = FALSE; }
			//Millisecs
			/*if(t_st.wMilliseconds > 0){
				//Separator
				if((i + 1) < sizeof(r.str) && keepAdd){ dst[i++] = '.'; } else { keepAdd = FALSE; }
				//Secs
				if((i + 3) < sizeof(r.str) && keepAdd){ i += NBDatetime_setDigitStr(t_st.wMilliseconds, 3, &dst[i], (sizeof(r.str) - i)); } else { keepAdd = FALSE; }
			}*/
			//End of string
			if (sizeof(r.str) > 0) {
				dst[(i < sizeof(r.str) ? i : (sizeof(r.str) - 1))] = '\0';
			}
		}
#		else
		{
			/*
			LARGE_INTEGER sysTime, curTime;
			KeQuerySystemTime(&sysTime);
			ExSystemTimeToLocalTime(&sysTime, &curTime);
			//ToDo: translate 'curTime' to printable value
			*/
		}
#		endif
	}
#	else
	{
		BOOL keepAdd = TRUE;
		UI32 i = 0; char* dst = r.str;
		time_t now = time(NULL);
		struct tm* t_st = localtime(&now);
		//Year
		if ((i + 4) < sizeof(r.str) && keepAdd) { i += NBMngrProcess_getPrintDateStrConcatUInt_(t_st->tm_year + 1900, 4, &dst[i], (sizeof(r.str) - i)); } else { keepAdd = FALSE; }
		//Separator
		if ((i + 1) < sizeof(r.str) && keepAdd) { dst[i++] = '-'; } else { keepAdd = FALSE; }
		//Month
		if ((i + 2) < sizeof(r.str) && keepAdd) { i += NBMngrProcess_getPrintDateStrConcatUInt_(t_st->tm_mon + 1, 2, &dst[i], (sizeof(r.str) - i)); } else { keepAdd = FALSE; }
		//Separator
		if ((i + 1) < sizeof(r.str) && keepAdd) { dst[i++] = '-'; } else { keepAdd = FALSE; }
		//Day
		if ((i + 2) < sizeof(r.str) && keepAdd) { i += NBMngrProcess_getPrintDateStrConcatUInt_(t_st->tm_mday, 2, &dst[i], (sizeof(r.str) - i)); } else { keepAdd = FALSE; }
		//Separator
		if ((i + 1) < sizeof(r.str) && keepAdd) { dst[i++] = ' '; } else { keepAdd = FALSE; }
		//Hour
		if ((i + 2) < sizeof(r.str) && keepAdd) { i += NBMngrProcess_getPrintDateStrConcatUInt_(t_st->tm_hour, 2, &dst[i], (sizeof(r.str) - i)); } else { keepAdd = FALSE; }
		//Separator
		if ((i + 1) < sizeof(r.str) && keepAdd) { dst[i++] = ':'; } else { keepAdd = FALSE; }
		//Minute
		if ((i + 2) < sizeof(r.str) && keepAdd) { i += NBMngrProcess_getPrintDateStrConcatUInt_(t_st->tm_min, 2, &dst[i], (sizeof(r.str) - i)); } else { keepAdd = FALSE; }
		//Separator
		if ((i + 1) < sizeof(r.str) && keepAdd) { dst[i++] = ':'; } else { keepAdd = FALSE; }
		//Secs
		if ((i + 2) < sizeof(r.str) && keepAdd) { i += NBMngrProcess_getPrintDateStrConcatUInt_(t_st->tm_sec, 2, &dst[i], (sizeof(r.str) - i)); } else { keepAdd = FALSE; }
		//Millisecs
		/*if(t.ms > 0){
			//Separator
			if((i + 1) < sizeof(r.str) && keepAdd){ dst[i++] = '.'; } else { keepAdd = FALSE; }
			//Secs
			if((i + 3) < sizeof(r.str) && keepAdd){ i += NBDatetime_setDigitStr(t.ms, 3, &dst[i], (sizeof(r.str) - i)); } else { keepAdd = FALSE; }
		}*/
		//End of string
		if (sizeof(r.str) > 0) {
			dst[(i < sizeof(r.str) ? i : (sizeof(r.str) - 1))] = '\0';
		}
	}
#	endif
	return r;
}

//
#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBMngrProcess_mutexLockPush(const UI64 lockId, const char* fullpath, const SI32 line, const char* func, const ENNBThreadLockPushMode pushMode){
	STNBMngrProcess* obj = __nbMngrThreads;
	NBMngrProcess_lock(obj);
	NB_STATIC_ASSERT(lockId > 0 && lockId <= obj->locks.seqUIDs);
	//threads
	{
		STNBMngrProcessThread* st = NBStatePerThread_get(fullpath, line, func, TRUE);
		//Analyze
		if(st == NULL){
			PRINTF_CONSOLE_ERROR("NBMngrProcess, NBMngrProcess_mutexLockPush, NBMngrProcess_getCurThreadLocked_ failed.\n");
			NB_STATIC_ASSERT(FALSE);
		} else {
			//Validate lock
#			ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION
			NBMngrProcess_lockPushLocked_(obj, st, lockId, fullpath, line, func, pushMode);
#			endif
		}
	}
	NBMngrProcess_unlock(obj);
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBMngrProcess_mutexLockStarted(const UI64 lockId, const char* fullpath, const SI32 line, const char* func){
#	ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION
	STNBMngrProcess* obj = __nbMngrThreads;
	NBMngrProcess_lock(obj);
	NB_STATIC_ASSERT(lockId > 0 && lockId <= obj->locks.seqUIDs);
	{
		STNBMngrProcessThread* st = NBStatePerThread_get(fullpath, line, func, FALSE);
		if(st == NULL){
			PRINTF_CONSOLE_ERROR("NBMngrProcess, NBMngrProcess_mutexLockStarted, missing push.\n");
			NB_STATIC_ASSERT(FALSE);
		} else {
			NBMngrProcess_lockStartedLocked_(obj, st, lockId, fullpath, line, func);
		}
	}
	NBMngrProcess_unlock(obj);
#	endif
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBMngrProcess_mutexLockPop(const UI64 lockId, const char* fullpath, const SI32 line, const char* func){
	STNBMngrProcess* obj = __nbMngrThreads;
	NBMngrProcess_lock(obj);
	NB_STATIC_ASSERT(lockId > 0 && lockId <= obj->locks.seqUIDs);
	//threads
	{
		STNBMngrProcessThread* st = NBStatePerThread_get(fullpath, line, func, FALSE);
		if(st == NULL){
			PRINTF_CONSOLE_ERROR("NBMngrProcess, NBMngrProcess_mutexLockPop, missing push.\n");
			NB_STATIC_ASSERT(FALSE);
		} else {
#			ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION
			NBMngrProcess_lockPopLocked_(obj, st, lockId, fullpath, line, func);
#			endif
		}
	}
	NBMngrProcess_unlock(obj);
}
#endif

//Cond

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
UI64 NBMngrProcess_condCreated(const char* name, const char* fullpath, const SI32 line, const char* func){
	UI64 r = 0;
	STNBMngrProcess* obj = __nbMngrThreads;
	NBMngrProcess_lock(obj);
	{
		//binary-search and add
		const UI64 condId = r = ++obj->conds.seqUIDs;
		BOOL fnd = FALSE;
		SI32 posStart = 0;
		if(obj->conds.use > 0){		
			const STNBMngrProcessCond* dataMidd = NULL;
			SI32 posMidd = 0, posEnd = (obj->conds.use - 1);
			while(posStart <= posEnd){
				posMidd		= posStart + ((posEnd - posStart) / 2);
				dataMidd	= &obj->conds.arr[posMidd];
				NB_STATIC_ASSERT((posStart == posMidd || obj->conds.arr[posStart].uid < dataMidd->uid) && (posMidd == posEnd || dataMidd->uid < obj->conds.arr[posEnd].uid))
				if(dataMidd->uid == condId){
					NB_STATIC_ASSERT(FALSE) //lock uid registered twice
					fnd = TRUE;
					break;
				} else if(condId < dataMidd->uid){
					posEnd		= posMidd - 1;
				} else {
					posStart	= posMidd + 1;
				}
			}
		}
		//Add
		NB_STATIC_ASSERT(!fnd)
		if(!fnd){
			NB_STATIC_ASSERT(posStart >= 0 && posStart <= obj->conds.use)
			//Validate order
#			ifdef NB_CONFIG_INCLUDE_ASSERTS
			if(posStart < obj->conds.use){ 
				const STNBMngrProcessCond* itm1 = &obj->conds.arr[posStart];
				if(condId >= itm1->uid){
					PRINTF_ERROR("NBMngrProcess, !(%llu < %llu).\n", condId, itm1->uid);
				}
				NB_STATIC_ASSERT(condId > 0 && condId <= obj->conds.seqUIDs)
				NB_STATIC_ASSERT(itm1->uid > 0 && itm1->uid <= obj->conds.seqUIDs)
				NB_STATIC_ASSERT(condId < itm1->uid)
			}
			if(posStart > 0){
				const STNBMngrProcessCond* itm0 = &obj->conds.arr[posStart - 1];
				if(itm0->uid >= condId){
					PRINTF_ERROR("NBMngrProcess, !(%llu < %llu).\n", itm0->uid, condId);
				}
				NB_STATIC_ASSERT(condId > 0 && condId <= obj->conds.seqUIDs)
				NB_STATIC_ASSERT(itm0->uid > 0 && itm0->uid <= obj->conds.seqUIDs)
				NB_STATIC_ASSERT(itm0->uid < condId)
			}
#			endif
			//Increase array
			NB_STATIC_ASSERT(obj->conds.use <= obj->conds.size)
			if(obj->conds.use >= obj->conds.size){
				obj->conds.size += 64; 
				{
					STNBMngrProcessCond* arrN = (STNBMngrProcessCond*)NBMemory_allocUnmanaged(sizeof(STNBMngrProcessCond) * obj->conds.size);
					if(obj->conds.arr != NULL){
						if(obj->conds.use > 0){
							memcpy(arrN, obj->conds.arr, sizeof(STNBMngrProcessCond) * obj->conds.use);
						}
						NBMemory_freeUnmanaged(obj->conds.arr);
						obj->conds.arr = NULL;
					}
					obj->conds.arr = arrN;
				}
			}
			//Insert itm
			{
				STNBMngrProcessCond* itm = &obj->conds.arr[posStart]; 
				//open gap
				{
					SI32 i; for(i = (SI32)obj->conds.use - 1; i >= posStart; i--){
						obj->conds.arr[i + 1] = obj->conds.arr[i];
					}
				}
				NBMngrProcessCond_init(itm);
				NBMngrProcessCond_setName(itm, name);
				NBMngrProcessCond_setFirstKnownFunc(itm, fullpath, line, func);
				obj->conds.arr[posStart].uid = condId;
				obj->conds.use++;
#				ifdef NB_CONFIG_INCLUDE_ASSERTS
				//Validate order
				if(posStart > 0){
					const STNBMngrProcessCond* itm0 = &obj->conds.arr[posStart - 1]; 
					const STNBMngrProcessCond* itm1 = &obj->conds.arr[posStart];
					if(itm0->uid >= itm1->uid){
						PRINTF_ERROR("NBMngrProcess, !(%llu < %llu).\n", itm0->uid, itm1->uid);
					}
					NB_STATIC_ASSERT(itm0->uid > 0 && itm0->uid <= obj->conds.seqUIDs)
					NB_STATIC_ASSERT(itm1->uid > 0 && itm1->uid <= obj->conds.seqUIDs)
					NB_STATIC_ASSERT(itm0->uid < itm1->uid)
				}
				if((posStart + 1) < obj->conds.use){
					const STNBMngrProcessCond* itm0 = &obj->conds.arr[posStart]; 
					const STNBMngrProcessCond* itm1 = &obj->conds.arr[posStart + 1];
					if(itm0->uid >= itm1->uid){
						PRINTF_ERROR("NBMngrProcess, !(%llu < %llu).\n", itm0->uid, itm1->uid);
					}
					NB_STATIC_ASSERT(itm0->uid > 0 && itm0->uid <= obj->conds.seqUIDs)
					NB_STATIC_ASSERT(itm1->uid > 0 && itm1->uid <= obj->conds.seqUIDs)
					NB_STATIC_ASSERT(itm0->uid < itm1->uid)
				}
#				endif
			}
		}
		//Validate order
#		ifdef NB_CONFIG_INCLUDE_ASSERTS
		if(obj->conds.use > 1){
			const STNBMngrProcessCond* itmPrev = &obj->conds.arr[0];
			NB_STATIC_ASSERT(itmPrev->uid > 0 && itmPrev->uid <= obj->conds.seqUIDs)
			UI32 i; for(i = 1; i < obj->conds.use; i++){
				const STNBMngrProcessCond* itm = &obj->conds.arr[i];
				NB_STATIC_ASSERT(itmPrev->uid < itm->uid)
				NB_STATIC_ASSERT(itm->uid > 0 && itm->uid <= obj->conds.seqUIDs)
				itmPrev = itm;
			}
		}
#		endif
	}
	NBMngrProcess_unlock(obj);
	return r;
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBMngrProcess_condDestroyed(const UI64 condId){
	NB_STATIC_ASSERT(condId != 0)
	STNBMngrProcess* obj = __nbMngrThreads;
	NBMngrProcess_lock(obj);
	{
		//binary-search and remove
		BOOL fnd = FALSE;
		if(obj->conds.use > 0){		
			STNBMngrProcessCond* dataMidd = NULL;
			SI32 posStart = 0, posMidd = 0, posEnd = (obj->conds.use - 1);
			while(posStart <= posEnd){
				posMidd		= posStart + ((posEnd - posStart) / 2);
				dataMidd	= &obj->conds.arr[posMidd];
				NB_STATIC_ASSERT((posStart == posMidd || obj->conds.arr[posStart].uid < dataMidd->uid) && (posMidd == posEnd || dataMidd->uid < obj->conds.arr[posEnd].uid))
				if(dataMidd->uid == condId){
					NBMngrProcessCond_release(dataMidd);
					//Fill gap
					{
						SI32 i; for(i = posMidd + 1; i < obj->conds.use; i++){
							obj->conds.arr[i - 1] = obj->conds.arr[i];
						}
						obj->conds.use--;
					}
					fnd = TRUE;
					break;
				} else if(condId < dataMidd->uid){
					posEnd		= posMidd - 1;
				} else {
					posStart	= posMidd + 1;
				}
			}
		}
		NB_STATIC_ASSERT(fnd) //must be found
		//Validate order
#		ifdef NB_CONFIG_INCLUDE_ASSERTS
		if(obj->conds.use > 1){
			const STNBMngrProcessCond* itmPrev = &obj->conds.arr[0];
			NB_STATIC_ASSERT(itmPrev->uid >= 0 && itmPrev->uid <= obj->conds.seqUIDs)
			UI32 i; for(i = 1; i < obj->conds.use; i++){
				const STNBMngrProcessCond* itm = &obj->conds.arr[i];
				NB_STATIC_ASSERT(itmPrev->uid < itm->uid)
				NB_STATIC_ASSERT(itm->uid > 0 && itm->uid <= obj->conds.seqUIDs)
				itmPrev = itm;
			}
		}
#		endif
	}
	NBMngrProcess_unlock(obj);
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBMngrProcess_condWaitPush(const UI64 condId, const UI64 lockId, const char* fullpath, const SI32 line, const char* func){
	STNBMngrProcess* obj = __nbMngrThreads;
	NBMngrProcess_lock(obj);
	STNBMngrProcessCond* cond = NBMngrProcess_getCondLocked_(obj, condId);
	NB_STATIC_ASSERT(cond != NULL)
	//threads
	{
		STNBMngrProcessThread* st = NBStatePerThread_get(fullpath, line, func, TRUE);
		if(st == NULL){
			PRINTF_CONSOLE_ERROR("NBMngrProcess, NBMngrProcess_condWaitPush, NBMngrProcess_getCurThreadLocked_ failed.\n");
			NB_STATIC_ASSERT(FALSE);
		} else {
			if(st->locks.use <= 0){
				PRINTF_CONSOLE_ERROR("NBMngrProcess, NBMngrProcess_condWaitPush without an active lock.\n");
				NB_STATIC_ASSERT(FALSE);
			} else {
				const STNBMngrProcessLockPos* lock = &st->locks.stack[st->locks.use - 1];
				if(cond->firstLockId == 0){
					cond->firstLockId = lock->lockId;
				} else if(cond->firstLockId != lock->lockId){
					PRINTF_CONSOLE_ERROR("NBMngrProcess, NBMngrProcess_condWaitPush cond used with different mutex first(%llu) now(%llu).\n", cond->firstLockId, lock-lock);
					NB_STATIC_ASSERT(FALSE);
				}
			}
#			ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION
			NBMngrProcess_lockPopLocked_(obj, st, lockId, fullpath, line, func); 
			NBMngrProcess_lockPushLocked_(obj, st, lockId, fullpath, line, func, ENNBThreadLockPushMode_Compete); 
#			endif
			//thread stats
#			ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
			{
				//alive
				st->stats.data.alive.conds.waitStarted++;
				//accum
				st->stats.data.accum.conds.waitStarted++;
				//total
				st->stats.data.total.conds.waitStarted++;
			}
#			endif
		}
	}
	//stats
#	ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
	{
		//alive
		obj->stats.alive.conds.waitStarted++;
		//accum
		obj->stats.accum.conds.waitStarted++;
		//total
		obj->stats.total.conds.waitStarted++;
	}
#	endif
	NBMngrProcess_unlock(obj);
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBMngrProcess_condWaitPop(const UI64 condId, const UI64 lockId, const char* fullpath, const SI32 line, const char* func){
	STNBMngrProcess* obj = __nbMngrThreads;
	NBMngrProcess_lock(obj);
	STNBMngrProcessCond* cond = NBMngrProcess_getCondLocked_(obj, condId);
	NB_STATIC_ASSERT(cond != NULL)
	//threads
#	ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION
	{
		STNBMngrProcessThread* st = NBStatePerThread_get(fullpath, line, func, FALSE);
		if(st == NULL){
			PRINTF_CONSOLE_ERROR("NBMngrProcess, NBMngrProcess_condWaitPop, missing push.\n");
			NB_STATIC_ASSERT(FALSE);
		} else {
#			ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION
			NBMngrProcess_lockStartedLocked_(obj, st, lockId, fullpath, line, func);
#			endif
			if(st->locks.use <= 0){
				PRINTF_CONSOLE_ERROR("NBMngrProcess, NBMngrProcess_condWaitPop without an active lock.\n");
				NB_STATIC_ASSERT(FALSE);
			} else {
				const STNBMngrProcessLockPos* lock = &st->locks.stack[st->locks.use - 1];
				if(cond->firstLockId == 0){
					cond->firstLockId = lock->lockId;
				} else if(cond->firstLockId != lock->lockId){
					PRINTF_CONSOLE_ERROR("NBMngrProcess, NBMngrProcess_condWaitPop cond used with different mutex first(%llu) now(%llu).\n", cond->firstLockId, lock->lockId);
					NB_STATIC_ASSERT(FALSE);
				}
			}
			//thread stats
#			ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
			{
				//alive
				NB_STATIC_ASSERT(st->stats.data.alive.conds.waitStarted > 0)
				st->stats.data.alive.conds.waitStarted--;
				//accum
				st->stats.data.accum.conds.waitEnded++;
				//total
				st->stats.data.total.conds.waitEnded++;
			}
#			endif
		}
	}
#	endif
	//stats
#	ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
	{
		//alive
		NB_STATIC_ASSERT(obj->stats.alive.conds.waitStarted > 0)
		obj->stats.alive.conds.waitStarted--;
		//accum
		obj->stats.accum.conds.waitEnded++;
		//total
		obj->stats.total.conds.waitEnded++;
	}
#	endif
	NBMngrProcess_unlock(obj);
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBMngrProcess_condSignal(const UI64 condId, const char* fullpath, const SI32 line, const char* func){
#	ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
	STNBMngrProcess* obj = __nbMngrThreads;
	NBMngrProcess_lock(obj);
	STNBMngrProcessCond* cond = NBMngrProcess_getCondLocked_(obj, condId);
	NB_STATIC_ASSERT(cond != NULL)
	//threads
	{
		STNBMngrProcessThread* st = NBStatePerThread_get(fullpath, line, func, TRUE);
		if(st == NULL){
			PRINTF_CONSOLE_ERROR("NBMngrProcess, NBMngrProcess_condSignal, NBMngrProcess_getCurThreadLocked_ failed.\n");
			NB_STATIC_ASSERT(FALSE);
		} else {
			if(st->locks.use <= 0){
				PRINTF_CONSOLE_ERROR("NBMngrProcess, NBMngrProcess_condSignal without an active lock.\n");
				NB_STATIC_ASSERT(FALSE);
			} else {
				const STNBMngrProcessLockPos* lock = &st->locks.stack[st->locks.use - 1];
				if(cond->firstLockId == 0){
					cond->firstLockId = lock->lockId;
				} else if(cond->firstLockId != lock->lockId){
					PRINTF_CONSOLE_ERROR("NBMngrProcess, NBMngrProcess_condSignal cond used with different mutex first(%llu) now(%llu).\n", cond->firstLockId, lock->lockId);
					NB_STATIC_ASSERT(FALSE);
				}
			}
			//thread stats
			{
				//accum
				st->stats.data.accum.conds.signals++;
				//total
				st->stats.data.total.conds.signals++;
			}
		}
	}
	//stats
	{
		//accum
		obj->stats.accum.conds.signals++;
		//total
		obj->stats.total.conds.signals++;
	}
	NBMngrProcess_unlock(obj);
#	endif
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBMngrProcess_condBroadcast(const UI64 condId, const char* fullpath, const SI32 line, const char* func){
#	ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
	STNBMngrProcess* obj = __nbMngrThreads;
	NBMngrProcess_lock(obj);
	STNBMngrProcessCond* cond = NBMngrProcess_getCondLocked_(obj, condId);
	NB_STATIC_ASSERT(cond != NULL)
	//threads
	{
		STNBMngrProcessThread* st = NBStatePerThread_get(fullpath, line, func, TRUE);
		if(st == NULL){
			PRINTF_CONSOLE_ERROR("NBMngrProcess, NBMngrProcess_condBroadcast, NBMngrProcess_getCurThreadLocked_ failed.\n");
			NB_STATIC_ASSERT(FALSE);
		} else {
			if(st->locks.use <= 0){
				PRINTF_CONSOLE_ERROR("NBMngrProcess, NBMngrProcess_condBroadcast without an active lock.\n");
				NB_STATIC_ASSERT(FALSE);
			} else {
				const STNBMngrProcessLockPos* lock = &st->locks.stack[st->locks.use - 1];
				if(cond->firstLockId == 0){
					cond->firstLockId = lock->lockId;
				} else if(cond->firstLockId != lock->lockId){
					PRINTF_CONSOLE_ERROR("NBMngrProcess, NBMngrProcess_condBroadcast cond used with different mutex first(%llu) now(%llu).\n", cond->firstLockId, lock->lockId);
					NB_STATIC_ASSERT(FALSE);
				}
			}
			//thread stats
			{
				//accum
				st->stats.data.accum.conds.broadcasts++;
				//total
				st->stats.data.total.conds.broadcasts++;
			}
		}
	}
	//stats
	{
		//accum
		obj->stats.accum.conds.broadcasts++;
		//total
		obj->stats.total.conds.broadcasts++;
	}
	NBMngrProcess_unlock(obj);
#	endif
}
#endif

//

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBMngrProcess_memStatsEnabledThreadPush(const BOOL isEnabled, const char* fullpath, const SI32 line, const char* func){
#	ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
	STNBMngrProcess* obj = __nbMngrThreads;
	NBMngrProcess_lock(obj);
	//threads
	{
		STNBMngrProcessThread* st = NBStatePerThread_get(fullpath, line, func, TRUE);
		if(st == NULL){
			PRINTF_CONSOLE_ERROR("NBMngrProcess, NBMngrProcess_memStatsEnabledThreadPush, NBMngrProcess_getCurThreadLocked_ failed.\n");
			NB_STATIC_ASSERT(FALSE);
		} else {
			//increase stack
			if(st->stats.memEnabled.use == st->stats.memEnabled.size){
				st->stats.memEnabled.size += 8; 
				{
					BOOL* n = NBMemory_allocUnmanaged(sizeof(st->stats.memEnabled.stack[0]) * st->stats.memEnabled.size);
					if(st->stats.memEnabled.stack != NULL){
						if(st->stats.memEnabled.use != 0){
							memcpy(n, st->stats.memEnabled.stack, sizeof(st->stats.memEnabled.stack[0]) * st->stats.memEnabled.use);
						}
						NBMemory_freeUnmanaged(st->stats.memEnabled.stack);
					}
					st->stats.memEnabled.stack = n;
				}
			}
			//push
			st->stats.memEnabled.stack[st->stats.memEnabled.use++] = isEnabled;
		}
	}
	NBMngrProcess_unlock(obj);
#	endif
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBMngrProcess_memStatsEnabledThreadPop(const char* fullpath, const SI32 line, const char* func){
#	ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
	STNBMngrProcess* obj = __nbMngrThreads;
	NBMngrProcess_lock(obj);
	//threads
	{
		STNBMngrProcessThread* st = NBStatePerThread_get(fullpath, line, func, TRUE);
		if(st == NULL){
			PRINTF_CONSOLE_ERROR("NBMngrProcess, NBMngrProcess_memStatsEnabledThreadPop, NBMngrProcess_getCurThreadLocked_ failed.\n");
			NB_STATIC_ASSERT(FALSE);
		} else {
			//pop
			NB_STATIC_ASSERT(st->stats.memEnabled.use > 0)
			if(st->stats.memEnabled.use > 0){
				st->stats.memEnabled.use--;
			}
		}
	}
	NBMngrProcess_unlock(obj);
#	endif
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBMngrProcess_memAllocated(const UI64 size, const char* fullpath, const SI32 line, const char* func){
#	ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
	STNBMngrProcess* obj = __nbMngrThreads;
	BOOL ignoreMemStats = FALSE;
	NBMngrProcess_lock(obj);
	//threads
	{
		STNBMngrProcessThread* st = NBStatePerThread_get(fullpath, line, func, TRUE);
		if(st == NULL){
			PRINTF_CONSOLE_ERROR("NBMngrProcess, NBMngrProcess_memAllocated, NBMngrProcess_getCurThreadLocked_ failed.\n");
			NB_STATIC_ASSERT(FALSE);
		} else {
			if(st->stats.memEnabled.use > 0){
				ignoreMemStats = !(st->stats.memEnabled.stack[st->stats.memEnabled.use - 1]);
			}
			//thread stats
			if(!ignoreMemStats){
				//alive
				//st->stats.alive.allocs++;
				//accum
				st->stats.data.accum.mem.allocs++;
				st->stats.data.accum.mem.allocBytes += size;
				//total
				st->stats.data.total.mem.allocs++;
				st->stats.data.total.mem.allocBytes += size;
			}
		}
	}
	//stats
	if(!ignoreMemStats){
		//alive
		//obj->stats.alive.allocs++;
		//accum
		obj->stats.accum.mem.allocs++;
		obj->stats.accum.mem.allocBytes += size;
		//total
		obj->stats.total.mem.allocs++;
		obj->stats.total.mem.allocBytes += size;
	}
	NBMngrProcess_unlock(obj);
#	endif
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBMngrProcess_memFreed(const char* fullpath, const SI32 line, const char* func){
#	ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
	STNBMngrProcess* obj = __nbMngrThreads;
	BOOL ignoreMemStats = FALSE;
	NBMngrProcess_lock(obj);
	//threads
	{
		STNBMngrProcessThread* st = NBStatePerThread_get(fullpath, line, func, TRUE);
		if(st == NULL){
			PRINTF_CONSOLE_ERROR("NBMngrProcess, NBMngrProcess_memFreed, NBMngrProcess_getCurThreadLocked_ failed.\n");
			NB_STATIC_ASSERT(FALSE);
		} else {
			if(st->stats.memEnabled.use > 0){
				ignoreMemStats = !(st->stats.memEnabled.stack[st->stats.memEnabled.use - 1]);
			}
			//thread stats
			if(!ignoreMemStats){
				//alive
				//st->stats.alive.allocs--;
				//accum
				st->stats.data.accum.mem.freeds++;
				//total
				st->stats.data.total.mem.freeds++;
			}
		}
	}
	//stats
	if(!ignoreMemStats){
		//alive
		//NB_STATIC_ASSERT(obj->stats.alive.allocs > 0)
		//obj->stats.alive.allocs--;
		//accum
		obj->stats.accum.mem.freeds++;
		//total
		obj->stats.total.mem.freeds++;
	}
	NBMngrProcess_unlock(obj);
#	endif
}
#endif

//handles

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
UI64 NBMngrProcess_hndlCreated(const ENNBHndlNativeType type, const char* fullpath, const SI32 line, const char* func){
    UI64 r = 0;
    STNBMngrProcess* obj = __nbMngrThreads;
    NBMngrProcess_lock(obj);
    {
        //binary-search and add
        const UI64 hndlId = r = ++obj->hndls.seqUIDs;
        BOOL fnd = FALSE;
        SI32 posStart = 0;
        if(obj->hndls.use > 0){
            const STNBMngrProcessHndl* dataMidd = NULL;
            SI32 posMidd = 0, posEnd = (obj->hndls.use - 1);
            while(posStart <= posEnd){
                posMidd        = posStart + ((posEnd - posStart) / 2);
                dataMidd    = &obj->hndls.arr[posMidd];
                NB_STATIC_ASSERT((posStart == posMidd || obj->hndls.arr[posStart].uid < dataMidd->uid) && (posMidd == posEnd || dataMidd->uid < obj->hndls.arr[posEnd].uid))
                if(dataMidd->uid == hndlId){
                    NB_STATIC_ASSERT(FALSE) //lock uid registered twice
                    fnd = TRUE;
                    break;
                } else if(hndlId < dataMidd->uid){
                    posEnd        = posMidd - 1;
                } else {
                    posStart    = posMidd + 1;
                }
            }
        }
        //Add
        NB_STATIC_ASSERT(!fnd)
        if(!fnd){
            NB_STATIC_ASSERT(posStart >= 0 && posStart <= obj->hndls.use)
            //Validate order
#            ifdef NB_CONFIG_INCLUDE_ASSERTS
            if(posStart < obj->hndls.use){
                const STNBMngrProcessHndl* itm1 = &obj->hndls.arr[posStart];
                if(hndlId >= itm1->uid){
                    PRINTF_ERROR("NBMngrProcess, !(%llu < %llu).\n", hndlId, itm1->uid);
                }
                NB_STATIC_ASSERT(hndlId > 0 && hndlId <= obj->hndls.seqUIDs)
                NB_STATIC_ASSERT(itm1->uid > 0 && itm1->uid <= obj->hndls.seqUIDs)
                NB_STATIC_ASSERT(hndlId < itm1->uid)
            }
            if(posStart > 0){
                const STNBMngrProcessHndl* itm0 = &obj->hndls.arr[posStart - 1];
                if(itm0->uid >= hndlId){
                    PRINTF_ERROR("NBMngrProcess, !(%llu < %llu).\n", itm0->uid, hndlId);
                }
                NB_STATIC_ASSERT(hndlId > 0 && hndlId <= obj->hndls.seqUIDs)
                NB_STATIC_ASSERT(itm0->uid > 0 && itm0->uid <= obj->hndls.seqUIDs)
                NB_STATIC_ASSERT(itm0->uid < hndlId)
            }
#            endif
            //Increase array
            NB_STATIC_ASSERT(obj->hndls.use <= obj->hndls.size)
            if(obj->hndls.use >= obj->hndls.size){
                obj->hndls.size += 64;
                {
                    STNBMngrProcessHndl* arrN = (STNBMngrProcessHndl*)NBMemory_allocUnmanaged(sizeof(STNBMngrProcessHndl) * obj->hndls.size);
                    if(obj->hndls.arr != NULL){
                        if(obj->hndls.use > 0){
                            memcpy(arrN, obj->hndls.arr, sizeof(STNBMngrProcessHndl) * obj->hndls.use);
                        }
                        NBMemory_freeUnmanaged(obj->hndls.arr);
                        obj->hndls.arr = NULL;
                    }
                    obj->hndls.arr = arrN;
                }
            }
            //Insert itm
            {
                STNBMngrProcessHndl* itm = &obj->hndls.arr[posStart];
                //open gap
                {
                    SI32 i; for(i = (SI32)obj->hndls.use - 1; i >= posStart; i--){
                        obj->hndls.arr[i + 1] = obj->hndls.arr[i];
                    }
                }
                NBMngrProcessHndl_init(itm);
                NBMngrProcessHndl_setFirstKnownFunc(itm, fullpath, line, func);
                itm->type = type;
                obj->hndls.arr[posStart].uid = hndlId;
                obj->hndls.use++;
#               ifdef NB_CONFIG_INCLUDE_ASSERTS
                //Validate order
                if(posStart > 0){
                    const STNBMngrProcessHndl* itm0 = &obj->hndls.arr[posStart - 1];
                    const STNBMngrProcessHndl* itm1 = &obj->hndls.arr[posStart];
                    if(itm0->uid >= itm1->uid){
                        PRINTF_ERROR("NBMngrProcess, !(%llu < %llu).\n", itm0->uid, itm1->uid);
                    }
                    NB_STATIC_ASSERT(itm0->uid > 0 && itm0->uid <= obj->hndls.seqUIDs)
                    NB_STATIC_ASSERT(itm1->uid > 0 && itm1->uid <= obj->hndls.seqUIDs)
                    NB_STATIC_ASSERT(itm0->uid < itm1->uid)
                }
                if((posStart + 1) < obj->hndls.use){
                    const STNBMngrProcessHndl* itm0 = &obj->hndls.arr[posStart];
                    const STNBMngrProcessHndl* itm1 = &obj->hndls.arr[posStart + 1];
                    if(itm0->uid >= itm1->uid){
                        PRINTF_ERROR("NBMngrProcess, !(%llu < %llu).\n", itm0->uid, itm1->uid);
                    }
                    NB_STATIC_ASSERT(itm0->uid > 0 && itm0->uid <= obj->hndls.seqUIDs)
                    NB_STATIC_ASSERT(itm1->uid > 0 && itm1->uid <= obj->hndls.seqUIDs)
                    NB_STATIC_ASSERT(itm0->uid < itm1->uid)
                }
#                endif
            }
        }
        //Validate order
#       ifdef NB_CONFIG_INCLUDE_ASSERTS
        if(obj->hndls.use > 1){
            const STNBMngrProcessHndl* itmPrev = &obj->hndls.arr[0];
            NB_STATIC_ASSERT(itmPrev->uid > 0 && itmPrev->uid <= obj->hndls.seqUIDs)
            UI32 i; for(i = 1; i < obj->hndls.use; i++){
                const STNBMngrProcessHndl* itm = &obj->hndls.arr[i];
                NB_STATIC_ASSERT(itmPrev->uid < itm->uid)
                NB_STATIC_ASSERT(itm->uid > 0 && itm->uid <= obj->hndls.seqUIDs)
                itmPrev = itm;
            }
        }
#       endif
    }
    NBMngrProcess_unlock(obj);
    return r;
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBMngrProcess_hndlClosed(const UI64 hndlId){
    NB_STATIC_ASSERT(hndlId != 0)
    STNBMngrProcess* obj = __nbMngrThreads;
    NBMngrProcess_lock(obj);
    {
        //binary-search and remove
        BOOL fnd = FALSE;
        if(obj->hndls.use > 0){
            STNBMngrProcessHndl* dataMidd = NULL;
            SI32 posStart = 0, posMidd = 0, posEnd = (obj->hndls.use - 1);
            while(posStart <= posEnd){
                posMidd        = posStart + ((posEnd - posStart) / 2);
                dataMidd    = &obj->hndls.arr[posMidd];
                NB_STATIC_ASSERT((posStart == posMidd || obj->hndls.arr[posStart].uid < dataMidd->uid) && (posMidd == posEnd || dataMidd->uid < obj->hndls.arr[posEnd].uid))
                if(dataMidd->uid == hndlId){
                    NBMngrProcessHndl_release(dataMidd);
                    //Fill gap
                    {
                        SI32 i; for(i = posMidd + 1; i < obj->hndls.use; i++){
                            obj->hndls.arr[i - 1] = obj->hndls.arr[i];
                        }
                        obj->hndls.use--;
                    }
                    fnd = TRUE;
                    break;
                } else if(hndlId < dataMidd->uid){
                    posEnd        = posMidd - 1;
                } else {
                    posStart    = posMidd + 1;
                }
            }
        }
        NB_STATIC_ASSERT(fnd) //must be found
        //Validate order
#       ifdef NB_CONFIG_INCLUDE_ASSERTS
        if(obj->hndls.use > 1){
            const STNBMngrProcessHndl* itmPrev = &obj->hndls.arr[0];
            NB_STATIC_ASSERT(itmPrev->uid >= 0 && itmPrev->uid <= obj->hndls.seqUIDs)
            UI32 i; for(i = 1; i < obj->hndls.use; i++){
                const STNBMngrProcessHndl* itm = &obj->hndls.arr[i];
                NB_STATIC_ASSERT(itmPrev->uid < itm->uid)
                NB_STATIC_ASSERT(itm->uid > 0 && itm->uid <= obj->hndls.seqUIDs)
                itmPrev = itm;
            }
        }
#       endif
    }
    NBMngrProcess_unlock(obj);
}
#endif

//objs (NBObj)

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
UI64 NBMngrProcess_objCreated(const char* name, const char* fullpath, const SI32 line, const char* func){
    UI64 r = 0;
    STNBMngrProcess* obj = __nbMngrThreads;
    NBMngrProcess_lock(obj);
    {
        //binary-search and add
        const UI64 objId = r = ++obj->objs.seqUIDs;
        BOOL fnd = FALSE;
        SI32 posStart = 0;
        if(obj->objs.use > 0){
            const STNBMngrProcessObj* dataMidd = NULL;
            SI32 posMidd = 0, posEnd = (obj->objs.use - 1);
            while(posStart <= posEnd){
                posMidd        = posStart + ((posEnd - posStart) / 2);
                dataMidd    = &obj->objs.arr[posMidd];
                NB_STATIC_ASSERT((posStart == posMidd || obj->objs.arr[posStart].uid < dataMidd->uid) && (posMidd == posEnd || dataMidd->uid < obj->objs.arr[posEnd].uid))
                if(dataMidd->uid == objId){
                    NB_STATIC_ASSERT(FALSE) //lock uid registered twice
                    fnd = TRUE;
                    break;
                } else if(objId < dataMidd->uid){
                    posEnd        = posMidd - 1;
                } else {
                    posStart    = posMidd + 1;
                }
            }
        }
        //Add
        NB_STATIC_ASSERT(!fnd)
        if(!fnd){
            NB_STATIC_ASSERT(posStart >= 0 && posStart <= obj->objs.use)
            //Validate order
#            ifdef NB_CONFIG_INCLUDE_ASSERTS
            if(posStart < obj->objs.use){
                const STNBMngrProcessObj* itm1 = &obj->objs.arr[posStart];
                if(objId >= itm1->uid){
                    PRINTF_ERROR("NBMngrProcess, !(%llu < %llu).\n", objId, itm1->uid);
                }
                NB_STATIC_ASSERT(objId > 0 && objId <= obj->objs.seqUIDs)
                NB_STATIC_ASSERT(itm1->uid > 0 && itm1->uid <= obj->objs.seqUIDs)
                NB_STATIC_ASSERT(objId < itm1->uid)
            }
            if(posStart > 0){
                const STNBMngrProcessObj* itm0 = &obj->objs.arr[posStart - 1];
                if(itm0->uid >= objId){
                    PRINTF_ERROR("NBMngrProcess, !(%llu < %llu).\n", itm0->uid, objId);
                }
                NB_STATIC_ASSERT(objId > 0 && objId <= obj->objs.seqUIDs)
                NB_STATIC_ASSERT(itm0->uid > 0 && itm0->uid <= obj->objs.seqUIDs)
                NB_STATIC_ASSERT(itm0->uid < objId)
            }
#            endif
            //Increase array
            NB_STATIC_ASSERT(obj->objs.use <= obj->objs.size)
            if(obj->objs.use >= obj->objs.size){
                obj->objs.size += 64;
                {
                    STNBMngrProcessObj* arrN = (STNBMngrProcessObj*)NBMemory_allocUnmanaged(sizeof(STNBMngrProcessObj) * obj->objs.size);
                    if(obj->objs.arr != NULL){
                        if(obj->objs.use > 0){
                            memcpy(arrN, obj->objs.arr, sizeof(STNBMngrProcessObj) * obj->objs.use);
                        }
                        NBMemory_freeUnmanaged(obj->objs.arr);
                        obj->objs.arr = NULL;
                    }
                    obj->objs.arr = arrN;
                }
            }
            //Insert itm
            {
                STNBMngrProcessObj* itm = &obj->objs.arr[posStart];
                //open gap
                {
                    SI32 i; for(i = (SI32)obj->objs.use - 1; i >= posStart; i--){
                        obj->objs.arr[i + 1] = obj->objs.arr[i];
                    }
                }
                NBMngrProcessObj_init(itm);
                NBMngrProcessObj_setName(itm, name);
                NBMngrProcessObj_setFirstKnownFunc(itm, fullpath, line, func);
#               ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION
                {
                    STNBMngrProcessThread* st = NBStatePerThread_get(fullpath, line, func, FALSE);
                    if(st != NULL){
                        if(st->locks.use > 0){
                            STNBMngrProcessLockPos* lck = &st->locks.stack[st->locks.use - 1];
                            NBMngrProcessObj_setFirstKnownLockPos(itm, lck->codePos.fullpath, lck->codePos.line, lck->codePos.func);
                        }
                    }
                }
#               endif
                obj->objs.arr[posStart].uid = objId;
                obj->objs.use++;
#               ifdef NB_CONFIG_INCLUDE_ASSERTS
                //Validate order
                if(posStart > 0){
                    const STNBMngrProcessObj* itm0 = &obj->objs.arr[posStart - 1];
                    const STNBMngrProcessObj* itm1 = &obj->objs.arr[posStart];
                    if(itm0->uid >= itm1->uid){
                        PRINTF_ERROR("NBMngrProcess, !(%llu < %llu).\n", itm0->uid, itm1->uid);
                    }
                    NB_STATIC_ASSERT(itm0->uid > 0 && itm0->uid <= obj->objs.seqUIDs)
                    NB_STATIC_ASSERT(itm1->uid > 0 && itm1->uid <= obj->objs.seqUIDs)
                    NB_STATIC_ASSERT(itm0->uid < itm1->uid)
                }
                if((posStart + 1) < obj->objs.use){
                    const STNBMngrProcessObj* itm0 = &obj->objs.arr[posStart];
                    const STNBMngrProcessObj* itm1 = &obj->objs.arr[posStart + 1];
                    if(itm0->uid >= itm1->uid){
                        PRINTF_ERROR("NBMngrProcess, !(%llu < %llu).\n", itm0->uid, itm1->uid);
                    }
                    NB_STATIC_ASSERT(itm0->uid > 0 && itm0->uid <= obj->objs.seqUIDs)
                    NB_STATIC_ASSERT(itm1->uid > 0 && itm1->uid <= obj->objs.seqUIDs)
                    NB_STATIC_ASSERT(itm0->uid < itm1->uid)
                }
#                endif
            }
        }
        //Validate order
#       ifdef NB_CONFIG_INCLUDE_ASSERTS
        if(obj->objs.use > 1){
            const STNBMngrProcessObj* itmPrev = &obj->objs.arr[0];
            NB_STATIC_ASSERT(itmPrev->uid > 0 && itmPrev->uid <= obj->objs.seqUIDs)
            UI32 i; for(i = 1; i < obj->objs.use; i++){
                const STNBMngrProcessObj* itm = &obj->objs.arr[i];
                NB_STATIC_ASSERT(itmPrev->uid < itm->uid)
                NB_STATIC_ASSERT(itm->uid > 0 && itm->uid <= obj->objs.seqUIDs)
                itmPrev = itm;
            }
        }
#       endif
    }
    NBMngrProcess_unlock(obj);
    return r;
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBMngrProcess_objDestroyed(const UI64 objId){
    NB_STATIC_ASSERT(objId != 0)
    STNBMngrProcess* obj = __nbMngrThreads;
    NBMngrProcess_lock(obj);
    {
        //binary-search and remove
        BOOL fnd = FALSE;
        if(obj->objs.use > 0){
            STNBMngrProcessObj* dataMidd = NULL;
            SI32 posStart = 0, posMidd = 0, posEnd = (obj->objs.use - 1);
            while(posStart <= posEnd){
                posMidd        = posStart + ((posEnd - posStart) / 2);
                dataMidd    = &obj->objs.arr[posMidd];
                NB_STATIC_ASSERT((posStart == posMidd || obj->objs.arr[posStart].uid < dataMidd->uid) && (posMidd == posEnd || dataMidd->uid < obj->objs.arr[posEnd].uid))
                if(dataMidd->uid == objId){
                    NBMngrProcessObj_release(dataMidd);
                    //Fill gap
                    {
                        SI32 i; for(i = posMidd + 1; i < obj->objs.use; i++){
                            obj->objs.arr[i - 1] = obj->objs.arr[i];
                        }
                        obj->objs.use--;
                    }
                    fnd = TRUE;
                    break;
                } else if(objId < dataMidd->uid){
                    posEnd        = posMidd - 1;
                } else {
                    posStart    = posMidd + 1;
                }
            }
        }
        NB_STATIC_ASSERT(fnd) //must be found
        //Validate order
#       ifdef NB_CONFIG_INCLUDE_ASSERTS
        if(obj->objs.use > 1){
            const STNBMngrProcessObj* itmPrev = &obj->objs.arr[0];
            NB_STATIC_ASSERT(itmPrev->uid >= 0 && itmPrev->uid <= obj->objs.seqUIDs)
            UI32 i; for(i = 1; i < obj->objs.use; i++){
                const STNBMngrProcessObj* itm = &obj->objs.arr[i];
                NB_STATIC_ASSERT(itmPrev->uid < itm->uid)
                NB_STATIC_ASSERT(itm->uid > 0 && itm->uid <= obj->objs.seqUIDs)
                itmPrev = itm;
            }
        }
#       endif
    }
    NBMngrProcess_unlock(obj);
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
SI64 NBMngrProcess_getHndlsCount(void){
    STNBMngrProcess* obj = __nbMngrThreads;
    return obj->hndls.use;
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
SI64 NBMngrProcess_getHndlsByTypeCount(const ENNBHndlNativeType type){
    SI64 r = 0;
    STNBMngrProcess* obj = __nbMngrThreads;
    NBMngrProcess_lock(obj);
    {
        UI32 i; for(i = 0; i < obj->hndls.use; i++){
            const STNBMngrProcessHndl* itm = &obj->hndls.arr[i];
            if(itm->type == type){
                r++;
            }
        }
    }
    NBMngrProcess_unlock(obj);
    return r;
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBMngrProcess_getHndlsByTypesCounts(SI64* dst, const SI32 dstSz){
    STNBMngrProcess* obj = __nbMngrThreads;
    if(dst != NULL && dstSz > 0){
        memset(dst, 0, sizeof(dst[0]) * dstSz);
        NBMngrProcess_lock(obj);
        {
            UI32 i; for(i = 0; i < obj->hndls.use; i++){
                const STNBMngrProcessHndl* itm = &obj->hndls.arr[i];
                if(itm->type < dstSz){
                    dst[itm->type]++;
                }
            }
        }
        NBMngrProcess_unlock(obj);
    }
}
#endif

//folders

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
SI64 NBMngrProcess_fsGetFoldersOpenCount(void){
    SI64 r = 0;
    STNBMngrProcess* obj = __nbMngrThreads;
    NBMngrProcess_lock(obj);
    {
        int i; for(i = 0; i < ENNBFilesystemStatsSrc_Count; i++){
            const SI64 total = obj->stats.total.fs.ops[i][ENNBFilesystemStatsResult_Success].folders.open - obj->stats.total.fs.ops[i][ENNBFilesystemStatsResult_Success].folders.close;
            NBASSERT(total >= 0)
            if(total > 0){
                r += total;
            }
        }
        
    }
    NBMngrProcess_unlock(obj);
    return r;
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBMngrProcess_fsFolderCreated(const ENNBFilesystemStatsSrc iSrc, const ENNBFilesystemStatsResult iResult){
#	ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
	if(iSrc >= 0 && iSrc < ENNBFilesystemStatsSrc_Count && iResult >= 0 && iResult < ENNBFilesystemStatsResult_Count){
		STNBMngrProcess* obj = __nbMngrThreads;
		NBMngrProcess_lock(obj);
		//threads
		{
			STNBMngrProcessThread* st = NBStatePerThread_get(NULL, 0, NULL, FALSE);
			if(st != NULL){
				//thread stats
				{
					//alive
					//st->stats.alive.fs.ops[iSrc][iResult].folders.create++;
					//accum
					st->stats.data.accum.fs.ops[iSrc][iResult].folders.create++;
					//total
					st->stats.data.total.fs.ops[iSrc][iResult].folders.create++;
				}
			}
		}
		//stats
		{
			//alive
			//obj->stats.alive.fs.ops[iSrc][iResult].folders.create++;
			//accum
			obj->stats.accum.fs.ops[iSrc][iResult].folders.create++;
			//total
			obj->stats.total.fs.ops[iSrc][iResult].folders.create++;
		}
		NBMngrProcess_unlock(obj);
	}
#	endif
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBMngrProcess_fsFolderDeleted(const ENNBFilesystemStatsSrc iSrc, const ENNBFilesystemStatsResult iResult){
#	ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
	if(iSrc >= 0 && iSrc < ENNBFilesystemStatsSrc_Count && iResult >= 0 && iResult < ENNBFilesystemStatsResult_Count){
		STNBMngrProcess* obj = __nbMngrThreads;
		NBMngrProcess_lock(obj);
		//threads
		{
			STNBMngrProcessThread* st = NBStatePerThread_get(NULL, 0, NULL, FALSE);
			if(st != NULL){
				//thread stats
				{
					//alive
					//st->stats.alive.fs.ops[iSrc][iResult].folders.create--;
					//accum
					st->stats.data.accum.fs.ops[iSrc][iResult].folders.delet++;
					//total
					st->stats.data.total.fs.ops[iSrc][iResult].folders.delet++;
				}
			}
		}
		//stats
		{
			//alive
			//obj->stats.alive.fs.ops[iSrc][iResult].folders.create--;
			//accum
			obj->stats.accum.fs.ops[iSrc][iResult].folders.delet++;
			//total
			obj->stats.total.fs.ops[iSrc][iResult].folders.delet++;
		}
		NBMngrProcess_unlock(obj);
	}
#	endif
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBMngrProcess_fsFolderOpened(const ENNBFilesystemStatsSrc iSrc, const ENNBFilesystemStatsResult iResult){
#	ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
	if(iSrc >= 0 && iSrc < ENNBFilesystemStatsSrc_Count && iResult >= 0 && iResult < ENNBFilesystemStatsResult_Count){
		STNBMngrProcess* obj = __nbMngrThreads;
		NBMngrProcess_lock(obj);
		//threads
		{
			STNBMngrProcessThread* st = NBStatePerThread_get(NULL, 0, NULL, FALSE);
			if(st != NULL){
				//thread stats
				{
					//alive
					if(iResult == ENNBFilesystemStatsResult_Success){
						st->stats.data.alive.fs.ops[iSrc][iResult].folders.open++;
					}
					//accum
					st->stats.data.accum.fs.ops[iSrc][iResult].folders.open++;
					//total
					st->stats.data.total.fs.ops[iSrc][iResult].folders.open++;
				}
			}
		}
		//stats
		{
			//alive
			if(iResult == ENNBFilesystemStatsResult_Success){
				obj->stats.alive.fs.ops[iSrc][iResult].folders.open++;
			}
			//accum
			obj->stats.accum.fs.ops[iSrc][iResult].folders.open++;
			//total
			obj->stats.total.fs.ops[iSrc][iResult].folders.open++;
		}
		NBMngrProcess_unlock(obj);
	}
#	endif
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBMngrProcess_fsFolderClosed(const ENNBFilesystemStatsSrc iSrc, const ENNBFilesystemStatsResult iResult){
#	ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
	if(iSrc >= 0 && iSrc < ENNBFilesystemStatsSrc_Count && iResult >= 0 && iResult < ENNBFilesystemStatsResult_Count){
		STNBMngrProcess* obj = __nbMngrThreads;
		NBMngrProcess_lock(obj);
		//threads
		{
			STNBMngrProcessThread* st = NBStatePerThread_get(NULL, 0, NULL, FALSE);
			if(st != NULL){
				//thread stats
				{
					//alive
					if(iResult == ENNBFilesystemStatsResult_Success){
						st->stats.data.alive.fs.ops[iSrc][iResult].folders.open--;
					}
					//accum
					st->stats.data.accum.fs.ops[iSrc][iResult].folders.close++;
					//total
					st->stats.data.total.fs.ops[iSrc][iResult].folders.close++;
				}
			}
		}
		//stats
		{
			//alive
			if(iResult == ENNBFilesystemStatsResult_Success){
				NB_STATIC_ASSERT(obj->stats.alive.fs.ops[iSrc][iResult].folders.open > 0)
				obj->stats.alive.fs.ops[iSrc][iResult].folders.open--;
			}
			//accum
			obj->stats.accum.fs.ops[iSrc][iResult].folders.close++;
			//total
			obj->stats.total.fs.ops[iSrc][iResult].folders.close++;
		}
		NBMngrProcess_unlock(obj);
	}
#	endif
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBMngrProcess_fsFolderRead(const ENNBFilesystemStatsSrc iSrc, const ENNBFilesystemStatsResult iResult){
#	ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
	if(iSrc >= 0 && iSrc < ENNBFilesystemStatsSrc_Count && iResult >= 0 && iResult < ENNBFilesystemStatsResult_Count){
		STNBMngrProcess* obj = __nbMngrThreads;
		NBMngrProcess_lock(obj);
		//threads
		{
			STNBMngrProcessThread* st = NBStatePerThread_get(NULL, 0, NULL, FALSE);
			if(st != NULL){
				//thread stats
				{
					//alive
					//st->stats.alive.fs.ops[iSrc][iResult].folders.read++;
					//accum
					st->stats.data.accum.fs.ops[iSrc][iResult].folders.read++;
					//total
					st->stats.data.total.fs.ops[iSrc][iResult].folders.read++;
				}
			}
		}
		//stats
		{
			//alive
			//obj->stats.alive.fs.ops[iSrc][iResult].folders.read++;
			//accum
			obj->stats.accum.fs.ops[iSrc][iResult].folders.read++;
			//total
			obj->stats.total.fs.ops[iSrc][iResult].folders.read++;
		}
		NBMngrProcess_unlock(obj);
	}
#	endif
}
#endif

//files

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
SI64 NBMngrProcess_fsGetFilesOpenCount(void){ //currently open
    SI64 r = 0;
    STNBMngrProcess* obj = __nbMngrThreads;
    NBMngrProcess_lock(obj);
    {
        int i; for(i = 0; i < ENNBFilesystemStatsSrc_Count; i++){
            const SI64 total = obj->stats.total.fs.ops[i][ENNBFilesystemStatsResult_Success].files.open - obj->stats.total.fs.ops[i][ENNBFilesystemStatsResult_Success].files.close;
            NBASSERT(total >= 0)
            if(total > 0){
                r += total;
            }
        }
        
    }
    NBMngrProcess_unlock(obj);
    return r;
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBMngrProcess_fsFileOpened(const ENNBFilesystemStatsSrc iSrc, const ENNBFilesystemStatsResult iResult){
#	ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
	if(iSrc >= 0 && iSrc < ENNBFilesystemStatsSrc_Count && iResult >= 0 && iResult < ENNBFilesystemStatsResult_Count){
		STNBMngrProcess* obj = __nbMngrThreads;
		NBMngrProcess_lock(obj);
		//threads
		{
			STNBMngrProcessThread* st = NBStatePerThread_get(NULL, 0, NULL, FALSE);
			if(st != NULL){
				//thread stats
				{
					//alive
					if(iResult == ENNBFilesystemStatsResult_Success){
						st->stats.data.alive.fs.ops[iSrc][iResult].files.open++;
					}
					//accum
					st->stats.data.accum.fs.ops[iSrc][iResult].files.open++;
					//total
					st->stats.data.total.fs.ops[iSrc][iResult].files.open++;
				}
			}
		}
		//stats
		{
			//alive
			if(iResult == ENNBFilesystemStatsResult_Success){
				obj->stats.alive.fs.ops[iSrc][iResult].files.open++;
			}
			//accum
			obj->stats.accum.fs.ops[iSrc][iResult].files.open++;
			//total
			obj->stats.total.fs.ops[iSrc][iResult].files.open++;
		}
		NBMngrProcess_unlock(obj);
	}
#	endif
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBMngrProcess_fsFileClosed(const ENNBFilesystemStatsSrc iSrc, const ENNBFilesystemStatsResult iResult){
#	ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
	if(iSrc >= 0 && iSrc < ENNBFilesystemStatsSrc_Count && iResult >= 0 && iResult < ENNBFilesystemStatsResult_Count){
		STNBMngrProcess* obj = __nbMngrThreads;
		NBMngrProcess_lock(obj);
		//threads
		{
			STNBMngrProcessThread* st = NBStatePerThread_get(NULL, 0, NULL, FALSE);
			if(st != NULL){
				//thread stats
				{
					//alive
					if(iResult == ENNBFilesystemStatsResult_Success){
						st->stats.data.alive.fs.ops[iSrc][iResult].files.open--;
					}
					//accum
					st->stats.data.accum.fs.ops[iSrc][iResult].files.close++;
					//total
					st->stats.data.total.fs.ops[iSrc][iResult].files.close++;
				}
			}
		}
		//stats
		{
			//alive
			if(iResult == ENNBFilesystemStatsResult_Success){
				NB_STATIC_ASSERT(obj->stats.alive.fs.ops[iSrc][iResult].files.open > 0)
				obj->stats.alive.fs.ops[iSrc][iResult].files.open--;
			}
			//accum
			obj->stats.accum.fs.ops[iSrc][iResult].files.close++;
			//total
			obj->stats.total.fs.ops[iSrc][iResult].files.close++;
		}
		NBMngrProcess_unlock(obj);
	}
#	endif
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBMngrProcess_fsFileRead(const ENNBFilesystemStatsSrc iSrc, const ENNBFilesystemStatsResult iResult, const UI64 bytes){
#	ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
	if(iSrc >= 0 && iSrc < ENNBFilesystemStatsSrc_Count && iResult >= 0 && iResult < ENNBFilesystemStatsResult_Count){
		STNBMngrProcess* obj = __nbMngrThreads;
		NBMngrProcess_lock(obj);
		//threads
		{
			STNBMngrProcessThread* st = NBStatePerThread_get(NULL, 0, NULL, FALSE);
			if(st != NULL){
				//thread stats
				{
					//alive
					//st->stats.data.alive.fs.ops[iSrc][iResult].files.read.count++;
					//st->stats.data.alive.fs.ops[iSrc][iResult].files.read.bytes += bytes;
					//accum
					st->stats.data.accum.fs.ops[iSrc][iResult].files.read.count++;
					st->stats.data.accum.fs.ops[iSrc][iResult].files.read.bytes += bytes;
					//total
					st->stats.data.total.fs.ops[iSrc][iResult].files.read.count++;
					st->stats.data.total.fs.ops[iSrc][iResult].files.read.bytes += bytes;
				}
			}
		}
		//stats
		{
			//alive
			//obj->stats.alive.fs.ops[iSrc][iResult].files.read.count++;
			//obj->stats.alive.fs.ops[iSrc][iResult].files.read.bytes += bytes;
			//accum
			obj->stats.accum.fs.ops[iSrc][iResult].files.read.count++;
			obj->stats.accum.fs.ops[iSrc][iResult].files.read.bytes += bytes;
			//total
			obj->stats.total.fs.ops[iSrc][iResult].files.read.count++;
			obj->stats.total.fs.ops[iSrc][iResult].files.read.bytes += bytes;
		}
		NBMngrProcess_unlock(obj);
	}
#	endif
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBMngrProcess_fsFileWritten(const ENNBFilesystemStatsSrc iSrc, const ENNBFilesystemStatsResult iResult, const UI64 bytes){
#	ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
	if(iSrc >= 0 && iSrc < ENNBFilesystemStatsSrc_Count && iResult >= 0 && iResult < ENNBFilesystemStatsResult_Count){
		STNBMngrProcess* obj = __nbMngrThreads;
		NBMngrProcess_lock(obj);
		//threads
		{
			STNBMngrProcessThread* st = NBStatePerThread_get(NULL, 0, NULL, FALSE);
			if(st != NULL){
				//thread stats
				{
					//alive
					//st->stats.data.alive.fs.ops[iSrc][iResult].files.write.count++;
					//st->stats.data.alive.fs.ops[iSrc][iResult].files.write.bytes += bytes;
					//accum
					st->stats.data.accum.fs.ops[iSrc][iResult].files.write.count++;
					st->stats.data.accum.fs.ops[iSrc][iResult].files.write.bytes += bytes;
					//total
					st->stats.data.total.fs.ops[iSrc][iResult].files.write.count++;
					st->stats.data.total.fs.ops[iSrc][iResult].files.write.bytes += bytes;
				}
			}
		}
		//stats
		{
			//alive
			//obj->stats.alive.fs.ops[iSrc][iResult].files.write.count++;
			//obj->stats.alive.fs.ops[iSrc][iResult].files.write.bytes += bytes;
			//accum
			obj->stats.accum.fs.ops[iSrc][iResult].files.write.count++;
			obj->stats.accum.fs.ops[iSrc][iResult].files.write.bytes += bytes;
			//total
			obj->stats.total.fs.ops[iSrc][iResult].files.write.count++;
			obj->stats.total.fs.ops[iSrc][iResult].files.write.bytes += bytes;
		}
		NBMngrProcess_unlock(obj);
	}
#	endif
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBMngrProcess_fsFileSeeked(const ENNBFilesystemStatsSrc iSrc, const ENNBFilesystemStatsResult iResult){
#	ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
	if(iSrc >= 0 && iSrc < ENNBFilesystemStatsSrc_Count && iResult >= 0 && iResult < ENNBFilesystemStatsResult_Count){
		STNBMngrProcess* obj = __nbMngrThreads;
		NBMngrProcess_lock(obj);
		//threads
		{
			STNBMngrProcessThread* st = NBStatePerThread_get(NULL, 0, NULL, FALSE);
			if(st != NULL){
				//thread stats
				{
					//alive
					//st->stats.data.alive.fs.ops[iSrc][iResult].files.seek++;
					//accum
					st->stats.data.accum.fs.ops[iSrc][iResult].files.seek++;
					//total
					st->stats.data.total.fs.ops[iSrc][iResult].files.seek++;
				}
			}
		}
		//stats
		{
			//alive
			//obj->stats.alive.fs.ops[iSrc][iResult].files.seek++;
			//accum
			obj->stats.accum.fs.ops[iSrc][iResult].files.seek++;
			//total
			obj->stats.total.fs.ops[iSrc][iResult].files.seek++;
		}
		NBMngrProcess_unlock(obj);
	}
#	endif
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBMngrProcess_fsFileTell(const ENNBFilesystemStatsSrc iSrc, const ENNBFilesystemStatsResult iResult){
#	ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
	if(iSrc >= 0 && iSrc < ENNBFilesystemStatsSrc_Count && iResult >= 0 && iResult < ENNBFilesystemStatsResult_Count){
		STNBMngrProcess* obj = __nbMngrThreads;
		NBMngrProcess_lock(obj);
		//threads
		{
			STNBMngrProcessThread* st = NBStatePerThread_get(NULL, 0, NULL, FALSE);
			if(st != NULL){
				//thread stats
				{
					//alive
					//st->stats.data.alive.fs.ops[iSrc][iResult].files.tell++;
					//accum
					st->stats.data.accum.fs.ops[iSrc][iResult].files.tell++;
					//total
					st->stats.data.total.fs.ops[iSrc][iResult].files.tell++;
				}
			}
		}
		//stats
		{
			//alive
			//obj->stats.alive.fs.ops[iSrc][iResult].files.tell++;
			//accum
			obj->stats.accum.fs.ops[iSrc][iResult].files.tell++;
			//total
			obj->stats.total.fs.ops[iSrc][iResult].files.tell++;
		}
		NBMngrProcess_unlock(obj);
	}
#	endif
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBMngrProcess_fsFileFlush(const ENNBFilesystemStatsSrc iSrc, const ENNBFilesystemStatsResult iResult){
#	ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
	if(iSrc >= 0 && iSrc < ENNBFilesystemStatsSrc_Count && iResult >= 0 && iResult < ENNBFilesystemStatsResult_Count){
		STNBMngrProcess* obj = __nbMngrThreads;
		NBMngrProcess_lock(obj);
		//threads
		{
			STNBMngrProcessThread* st = NBStatePerThread_get(NULL, 0, NULL, FALSE);
			if(st != NULL){
				//thread stats
				{
					//alive
					//st->stats.data.alive.fs.ops[iSrc][iResult].files.flush++;
					//accum
					st->stats.data.accum.fs.ops[iSrc][iResult].files.flush++;
					//total
					st->stats.data.total.fs.ops[iSrc][iResult].files.flush++;
				}
			}
		}
		//stats
		{
			//alive
			//obj->stats.alive.fs.ops[iSrc][iResult].files.flush++;
			//accum
			obj->stats.accum.fs.ops[iSrc][iResult].files.flush++;
			//total
			obj->stats.total.fs.ops[iSrc][iResult].files.flush++;
		}
		NBMngrProcess_unlock(obj);
	}
#	endif
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBMngrProcess_fsFileStat(const ENNBFilesystemStatsSrc iSrc, const ENNBFilesystemStatsResult iResult){
#	ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
	if(iSrc >= 0 && iSrc < ENNBFilesystemStatsSrc_Count && iResult >= 0 && iResult < ENNBFilesystemStatsResult_Count){
		STNBMngrProcess* obj = __nbMngrThreads;
		NBMngrProcess_lock(obj);
		//threads
		{
			STNBMngrProcessThread* st = NBStatePerThread_get(NULL, 0, NULL, FALSE);
			if(st != NULL){
				//thread stats
				{
					//alive
					//st->stats.data.alive.fs.ops[iSrc][iResult].files.stat++;
					//accum
					st->stats.data.accum.fs.ops[iSrc][iResult].files.stat++;
					//total
					st->stats.data.total.fs.ops[iSrc][iResult].files.stat++;
				}
			}
		}
		//stats
		{
			//alive
			//obj->stats.alive.fs.ops[iSrc][iResult].files.stat++;
			//accum
			obj->stats.accum.fs.ops[iSrc][iResult].files.stat++;
			//total
			obj->stats.total.fs.ops[iSrc][iResult].files.stat++;
		}
		NBMngrProcess_unlock(obj);
	}
#	endif
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBMngrProcess_fsFileMoved(const ENNBFilesystemStatsSrc iSrc, const ENNBFilesystemStatsResult iResult){
#	ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
	if(iSrc >= 0 && iSrc < ENNBFilesystemStatsSrc_Count && iResult >= 0 && iResult < ENNBFilesystemStatsResult_Count){
		STNBMngrProcess* obj = __nbMngrThreads;
		NBMngrProcess_lock(obj);
		//threads
		{
			STNBMngrProcessThread* st = NBStatePerThread_get(NULL, 0, NULL, FALSE);
			if(st != NULL){
				//thread stats
				{
					//alive
					//st->stats.data.alive.fs.ops[iSrc][iResult].files.move++;
					//accum
					st->stats.data.accum.fs.ops[iSrc][iResult].files.move++;
					//total
					st->stats.data.total.fs.ops[iSrc][iResult].files.move++;
				}
			}
		}
		//stats
		{
			//alive
			//obj->stats.alive.fs.ops[iSrc][iResult].files.move++;
			//accum
			obj->stats.accum.fs.ops[iSrc][iResult].files.move++;
			//total
			obj->stats.total.fs.ops[iSrc][iResult].files.move++;
		}
		NBMngrProcess_unlock(obj);
	}
#	endif
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBMngrProcess_fsFileDeleted(const ENNBFilesystemStatsSrc iSrc, const ENNBFilesystemStatsResult iResult){
#	ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
	if(iSrc >= 0 && iSrc < ENNBFilesystemStatsSrc_Count && iResult >= 0 && iResult < ENNBFilesystemStatsResult_Count){
		STNBMngrProcess* obj = __nbMngrThreads;
		NBMngrProcess_lock(obj);
		//threads
		{
			STNBMngrProcessThread* st = NBStatePerThread_get(NULL, 0, NULL, FALSE);
			if(st != NULL){
				//thread stats
				{
					//alive
					//st->stats.data.alive.fs.ops[iSrc][iResult].files.delet++;
					//accum
					st->stats.data.accum.fs.ops[iSrc][iResult].files.delet++;
					//total
					st->stats.data.total.fs.ops[iSrc][iResult].files.delet++;
				}
			}
		}
		//stats
		{
			//alive
			//obj->stats.alive.fs.ops[iSrc][iResult].files.delet++;
			//accum
			obj->stats.accum.fs.ops[iSrc][iResult].files.delet++;
			//total
			obj->stats.total.fs.ops[iSrc][iResult].files.delet++;
		}
		NBMngrProcess_unlock(obj);
	}
#	endif
}
#endif

//------------------------
//- NBMngrProcessStatsData
//------------------------

void NBMngrProcessStatsData_init(STNBMngrProcessStatsData* obj){
	NBMemory_setZeroSt(*obj, STNBMngrProcessStatsData);
}

void NBMngrProcessStatsData_release(STNBMngrProcessStatsData* obj){
	//alive
	NBMngrProcessStatsState_release(&obj->alive);
	//accum
	NBMngrProcessStatsState_release(&obj->accum);
	//total
	NBMngrProcessStatsState_release(&obj->total);
}

void NBMngrProcessStatsData_add(STNBMngrProcessStatsData* obj, const STNBMngrProcessStatsData* other, const BOOL includeLocksByMethod){
	//alive
	NBMngrProcessStatsState_add(&obj->alive, &other->alive, includeLocksByMethod);
	//accum
	NBMngrProcessStatsState_add(&obj->accum, &other->accum, includeLocksByMethod);
	//total
	NBMngrProcessStatsState_add(&obj->total, &other->total, includeLocksByMethod);
}

//-------------------------
//- NBThreadsMethodLocksOps
//-------------------------

void NBThreadsMethodLocksOps_init(STNBThreadsMethodLocksOps* obj){
	NBMemory_setZeroSt(*obj, STNBThreadsMethodLocksOps);
}

void NBThreadsMethodLocksOps_release(STNBThreadsMethodLocksOps* obj){
	if(obj->method != NULL){
		NBMemory_freeUnmanaged(obj->method);
		obj->method = NULL;
	}
}

BOOL NBCompare_NBThreadsMethodLocksOps(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
	NB_STATIC_ASSERT(dataSz == sizeof(STNBThreadsMethodLocksOps))
	if(dataSz == sizeof(STNBThreadsMethodLocksOps)){
		const STNBThreadsMethodLocksOps* d1 = (STNBThreadsMethodLocksOps*)data1;
		const STNBThreadsMethodLocksOps* d2 = (STNBThreadsMethodLocksOps*)data2;
		switch (mode) {
			case ENCompareMode_Equal:
				return NBMngrProcess_strIsEqual_(d1->method, d2->method);
			case ENCompareMode_Lower:
				return NBMngrProcess_strIsLower_(d1->method, d2->method);
			case ENCompareMode_LowerOrEqual:
				return NBMngrProcess_strIsLowerOrEqual_(d1->method, d2->method);
			case ENCompareMode_Greater:
				return NBMngrProcess_strIsGreater_(d1->method, d2->method);
			case ENCompareMode_GreaterOrEqual:
				return NBMngrProcess_strIsGreaterOrEqual_(d1->method, d2->method);
	#		ifdef NB_CONFIG_INCLUDE_ASSERTS
			default:
				NB_STATIC_ASSERT(FALSE)
				break;
	#		endif
		}
	}
	NB_STATIC_ASSERT(FALSE)
	return FALSE;
}

// NBFilesystemStatsOp

STNBStructMapsRec NBFilesystemStatsOp_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBFilesystemStatsOp_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBFilesystemStatsOp_sharedStructMap);
    if(NBFilesystemStatsOp_sharedStructMap.map == NULL){
        STNBFilesystemStatsOp s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBFilesystemStatsOp);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addIntM(map, s, count);
        NBStructMap_addUIntM(map, s, bytes);
        NBFilesystemStatsOp_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBFilesystemStatsOp_sharedStructMap);
    return NBFilesystemStatsOp_sharedStructMap.map;
}

// NBFilesystemStatsFilesOps

STNBStructMapsRec NBFilesystemStatsFilesOps_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBFilesystemStatsFilesOps_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBFilesystemStatsFilesOps_sharedStructMap);
    if(NBFilesystemStatsFilesOps_sharedStructMap.map == NULL){
        STNBFilesystemStatsFilesOps s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBFilesystemStatsFilesOps);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addIntM(map, s, open);
        NBStructMap_addIntM(map, s, close);
        NBStructMap_addStructM(map, s, read, NBFilesystemStatsOp_getSharedStructMap());
        NBStructMap_addStructM(map, s, write, NBFilesystemStatsOp_getSharedStructMap());
        NBStructMap_addUIntM(map, s, seek);
        NBStructMap_addUIntM(map, s, tell);
        NBStructMap_addUIntM(map, s, flush);
        NBStructMap_addUIntM(map, s, stat);
        NBStructMap_addUIntM(map, s, move);
        NBStructMap_addUIntM(map, s, delet);
        NBFilesystemStatsFilesOps_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBFilesystemStatsFilesOps_sharedStructMap);
    return NBFilesystemStatsFilesOps_sharedStructMap.map;
}

// NBFilesystemStatsFoldersOps

STNBStructMapsRec NBFilesystemStatsFoldersOps_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBFilesystemStatsFoldersOps_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBFilesystemStatsFoldersOps_sharedStructMap);
    if(NBFilesystemStatsFoldersOps_sharedStructMap.map == NULL){
        STNBFilesystemStatsFoldersOps s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBFilesystemStatsFoldersOps);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addUIntM(map, s, create);
        NBStructMap_addIntM(map, s, open);
        NBStructMap_addIntM(map, s, close);
        NBStructMap_addUIntM(map, s, read);
        NBStructMap_addUIntM(map, s, delet);
        NBFilesystemStatsFoldersOps_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBFilesystemStatsFoldersOps_sharedStructMap);
    return NBFilesystemStatsFoldersOps_sharedStructMap.map;
}

// NBFilesystemStatsOps

STNBStructMapsRec NBFilesystemStatsOps_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBFilesystemStatsOps_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBFilesystemStatsOps_sharedStructMap);
    if(NBFilesystemStatsOps_sharedStructMap.map == NULL){
        STNBFilesystemStatsOps s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBFilesystemStatsOps);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addStructM(map, s, files, NBFilesystemStatsFilesOps_getSharedStructMap());
        NBStructMap_addStructM(map, s, folders, NBFilesystemStatsFoldersOps_getSharedStructMap());
        NBFilesystemStatsOps_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBFilesystemStatsOps_sharedStructMap);
    return NBFilesystemStatsOps_sharedStructMap.map;
}

// NBFilesystemStatsState

STNBStructMapsRec NBFilesystemStatsState_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBFilesystemStatsState_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBFilesystemStatsState_sharedStructMap);
    if(NBFilesystemStatsState_sharedStructMap.map == NULL){
        STNBFilesystemStatsState s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBFilesystemStatsState);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addArrayOfStructM(map, s, ops, NBFilesystemStatsOps_getSharedStructMap());
        NBFilesystemStatsState_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBFilesystemStatsState_sharedStructMap);
    return NBFilesystemStatsState_sharedStructMap.map;
}

// NBThreadsMethodLocksOps

STNBStructMapsRec NBThreadsMethodLocksOps_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBThreadsMethodLocksOps_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBThreadsMethodLocksOps_sharedStructMap);
    if(NBThreadsMethodLocksOps_sharedStructMap.map == NULL){
        STNBThreadsMethodLocksOps s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBThreadsMethodLocksOps);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addStrPtrM(map, s, method);
        NBStructMap_addIntM(map, s, locked);
        NBStructMap_addIntM(map, s, unlocked);
        NBStructMap_addIntM(map, s, yielded);
        NBThreadsMethodLocksOps_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBThreadsMethodLocksOps_sharedStructMap);
    return NBThreadsMethodLocksOps_sharedStructMap.map;
}

//NBMngrProcessStatsStateMem

STNBStructMapsRec NBMngrProcessStatsStateMem_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBMngrProcessStatsStateMem_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBMngrProcessStatsStateMem_sharedStructMap);
    if(NBMngrProcessStatsStateMem_sharedStructMap.map == NULL){
        STNBMngrProcessStatsStateMem s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBMngrProcessStatsStateMem);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addIntM(map, s, allocs);
        NBStructMap_addUIntM(map, s, allocBytes);
        NBStructMap_addIntM(map, s, freeds);
        NBMngrProcessStatsStateMem_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBMngrProcessStatsStateMem_sharedStructMap);
    return NBMngrProcessStatsStateMem_sharedStructMap.map;
}

// NBMngrProcessStatsStateLocksByMethod

STNBStructMapsRec NBMngrProcessStatsStateLocksByMethod_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBMngrProcessStatsStateLocksByMethod_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBMngrProcessStatsStateLocksByMethod_sharedStructMap);
    if(NBMngrProcessStatsStateLocksByMethod_sharedStructMap.map == NULL){
        STNBMngrProcessStatsStateLocksByMethod s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBMngrProcessStatsStateLocksByMethod);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addPtrToArrayOfStructM(map, s, arr, use, ENNBStructMapSign_Signed, NBThreadsMethodLocksOps_getSharedStructMap());
        //SI32 alloc; //alloc optimization (not a struct member)
        NBMngrProcessStatsStateLocksByMethod_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBMngrProcessStatsStateLocksByMethod_sharedStructMap);
    return NBMngrProcessStatsStateLocksByMethod_sharedStructMap.map;
}

// NBMngrProcessStatsStateLocks

STNBStructMapsRec NBMngrProcessStatsStateLocks_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBMngrProcessStatsStateLocks_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBMngrProcessStatsStateLocks_sharedStructMap);
    if(NBMngrProcessStatsStateLocks_sharedStructMap.map == NULL){
        STNBMngrProcessStatsStateLocks s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBMngrProcessStatsStateLocks);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addUIntM(map, s, created);
        NBStructMap_addUIntM(map, s, destroyed);
        NBStructMap_addUIntM(map, s, locked);
        NBStructMap_addUIntM(map, s, unlocked);
        NBStructMap_addUIntM(map, s, passby);
        NBStructMap_addStructM(map, s, byMethod, NBMngrProcessStatsStateLocksByMethod_getSharedStructMap());
        NBMngrProcessStatsStateLocks_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBMngrProcessStatsStateLocks_sharedStructMap);
    return NBMngrProcessStatsStateLocks_sharedStructMap.map;
}

// NBMngrProcessStatsStateConds

STNBStructMapsRec NBMngrProcessStatsStateConds_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBMngrProcessStatsStateConds_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBMngrProcessStatsStateConds_sharedStructMap);
    if(NBMngrProcessStatsStateConds_sharedStructMap.map == NULL){
        STNBMngrProcessStatsStateConds s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBMngrProcessStatsStateConds);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addUIntM(map, s, created);
        NBStructMap_addUIntM(map, s, destroyed);
        NBStructMap_addUIntM(map, s, waitStarted);
        NBStructMap_addUIntM(map, s, waitEnded);
        NBStructMap_addUIntM(map, s, signals);
        NBStructMap_addUIntM(map, s, broadcasts);
        NBMngrProcessStatsStateConds_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBMngrProcessStatsStateConds_sharedStructMap);
    return NBMngrProcessStatsStateConds_sharedStructMap.map;
}

// NBMngrProcessCodePos

STNBStructMapsRec NBMngrProcessCodePos_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBMngrProcessCodePos_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBMngrProcessCodePos_sharedStructMap);
    if(NBMngrProcessCodePos_sharedStructMap.map == NULL){
        STNBMngrProcessCodePos s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBMngrProcessCodePos);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addStrPtrM(map, s, fullpath);
        NBStructMap_addIntM(map, s, line);
        NBStructMap_addStrPtrM(map, s, func);
        NBMngrProcessCodePos_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBMngrProcessCodePos_sharedStructMap);
    return NBMngrProcessCodePos_sharedStructMap.map;
}

// NBMngrProcessCodePosPair

STNBStructMapsRec NBMngrProcessCodePosPair_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBMngrProcessCodePosPair_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBMngrProcessCodePosPair_sharedStructMap);
    if(NBMngrProcessCodePosPair_sharedStructMap.map == NULL){
        STNBMngrProcessCodePosPair s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBMngrProcessCodePosPair);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addStructM(map, s, one, NBMngrProcessCodePos_getSharedStructMap());
        NBStructMap_addStructM(map, s, other, NBMngrProcessCodePos_getSharedStructMap());
        NBStructMap_addUIntM(map, s, count);
        NBMngrProcessCodePosPair_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBMngrProcessCodePosPair_sharedStructMap);
    return NBMngrProcessCodePosPair_sharedStructMap.map;
}

// NBMngrProcessStatsStateYields

STNBStructMapsRec NBMngrProcessStatsStateYields_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBMngrProcessStatsStateYields_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBMngrProcessStatsStateYields_sharedStructMap);
    if(NBMngrProcessStatsStateYields_sharedStructMap.map == NULL){
        STNBMngrProcessStatsStateYields s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBMngrProcessStatsStateYields);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addPtrToArrayOfStructM(map, s, list, use, ENNBStructMapSign_Signed, NBMngrProcessCodePosPair_getSharedStructMap());
        //SI32 size;   //alloc optimization (not a struct member)
        NBMngrProcessStatsStateYields_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBMngrProcessStatsStateYields_sharedStructMap);
    return NBMngrProcessStatsStateYields_sharedStructMap.map;
}

// NBMngrProcessStatsState

STNBStructMapsRec NBMngrProcessStatsState_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBMngrProcessStatsState_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBMngrProcessStatsState_sharedStructMap);
    if(NBMngrProcessStatsState_sharedStructMap.map == NULL){
        STNBMngrProcessStatsState s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBMngrProcessStatsState);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addStructM(map, s, mem, NBMngrProcessStatsStateMem_getSharedStructMap());
        NBStructMap_addStructM(map, s, locks, NBMngrProcessStatsStateLocks_getSharedStructMap());
        NBStructMap_addStructM(map, s, conds, NBMngrProcessStatsStateConds_getSharedStructMap());
        NBStructMap_addStructM(map, s, yields, NBMngrProcessStatsStateYields_getSharedStructMap());
        NBStructMap_addStructM(map, s, fs, NBFilesystemStatsState_getSharedStructMap());
        NBMngrProcessStatsState_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBMngrProcessStatsState_sharedStructMap);
    return NBMngrProcessStatsState_sharedStructMap.map;
}

// NBMngrProcessStatsData

STNBStructMapsRec NBMngrProcessStatsData_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBMngrProcessStatsData_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBMngrProcessStatsData_sharedStructMap);
    if(NBMngrProcessStatsData_sharedStructMap.map == NULL){
        STNBMngrProcessStatsData s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBMngrProcessStatsData);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addStructM(map, s, alive, NBMngrProcessStatsData_getSharedStructMap());
        NBStructMap_addStructM(map, s, accum, NBMngrProcessStatsData_getSharedStructMap());
        NBStructMap_addStructM(map, s, total, NBMngrProcessStatsData_getSharedStructMap());
        NBMngrProcessStatsData_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBMngrProcessStatsData_sharedStructMap);
    return NBMngrProcessStatsData_sharedStructMap.map;
}

//-------------------------
//- NBMngrProcessStatsState
//-------------------------

void NBMngrProcessStatsState_init(STNBMngrProcessStatsState* obj){
	NBMemory_setZeroSt(*obj, STNBMngrProcessStatsState);
}

void NBMngrProcessStatsState_release(STNBMngrProcessStatsState* obj){
	//locks
	{
		//byMethod
		{
			if(obj->locks.byMethod.arr != NULL){
				if(obj->locks.byMethod.use > 0){
					SI32 i; for(i = 0; i < obj->locks.byMethod.use; i++){
						STNBThreadsMethodLocksOps* ops = &obj->locks.byMethod.arr[i];
						NBThreadsMethodLocksOps_release(ops);
					}
				}
				NBMemory_freeUnmanaged(obj->locks.byMethod.arr);
				obj->locks.byMethod.arr = NULL;
				obj->locks.byMethod.use = obj->locks.byMethod.alloc = 0;
			}
		}
	}
}

void NBMngrProcessStatsState_add(STNBMngrProcessStatsState* obj, const STNBMngrProcessStatsState* other, const BOOL includeLocksByMethod){
	obj->mem.allocs			+= other->mem.allocs;
	obj->mem.allocBytes		+= other->mem.allocBytes;
	obj->mem.freeds			+= other->mem.freeds;
	//
	obj->locks.created		+= other->locks.created;
	obj->locks.destroyed	+= other->locks.destroyed;
	obj->locks.locked		+= other->locks.locked;
	obj->locks.unlocked		+= other->locks.unlocked;
	obj->locks.passby		+= other->locks.passby;
	obj->locks.yield		+= other->locks.yield;
	//
	obj->conds.created		+= other->conds.created;
	obj->conds.destroyed	+= other->conds.destroyed;
	obj->conds.waitStarted	+= other->conds.waitStarted; 
	obj->conds.waitEnded	+= other->conds.waitEnded;
	obj->conds.signals		+= other->conds.signals;
	obj->conds.broadcasts	+= other->conds.broadcasts;
	//
	if(includeLocksByMethod && other->locks.byMethod.arr != NULL && other->locks.byMethod.use != 0){
		SI32 i; for(i = 0; i < other->locks.byMethod.use; i++){
			const STNBThreadsMethodLocksOps* srch = &other->locks.byMethod.arr[i];
			const SI32 fndIdx = NBMngrProcess_sortedIndexOf_((void**)&obj->locks.byMethod.arr, &obj->locks.byMethod.use, &obj->locks.byMethod.alloc, sizeof(obj->locks.byMethod.arr[0]), NBCompare_NBThreadsMethodLocksOps, srch, sizeof(*srch));
			if(fndIdx >= 0){
				//update record
				STNBThreadsMethodLocksOps* itm = &obj->locks.byMethod.arr[fndIdx];
				itm->locked		+= srch->locked;
				itm->unlocked	+= srch->unlocked;
				itm->yielded	+= srch->yielded;
			} else {
				//create record
				STNBThreadsMethodLocksOps itm;
				NBMemory_setZeroSt(itm, STNBThreadsMethodLocksOps);
				itm.method		= NBMngrProcess_strClone_(srch->method);
				itm.locked		+= srch->locked;
				itm.unlocked	+= srch->unlocked;
				itm.yielded		+= srch->yielded;
				NBMngrProcess_sortedAdd_((void**)&obj->locks.byMethod.arr, &obj->locks.byMethod.use, &obj->locks.byMethod.alloc, sizeof(obj->locks.byMethod.arr[0]), NBCompare_NBThreadsMethodLocksOps, &itm, sizeof(itm));
			}
		}
	}
}

void NBMngrProcessStatsState_mutexLocked(STNBMngrProcessStatsState* obj, const char* func){
	obj->locks.locked++;
	if(func != NULL){
		SI32 fndIdx = -1;
		STNBThreadsMethodLocksOps srch;
		NBMemory_setZeroSt(srch, STNBThreadsMethodLocksOps);
		srch.method = (char*)func;
		fndIdx = NBMngrProcess_sortedIndexOf_((void**)&obj->locks.byMethod.arr, &obj->locks.byMethod.use, &obj->locks.byMethod.alloc, sizeof(obj->locks.byMethod.arr[0]), NBCompare_NBThreadsMethodLocksOps, &srch, sizeof(srch));
		if(fndIdx >= 0){
			//update record
			STNBThreadsMethodLocksOps* itm = &obj->locks.byMethod.arr[fndIdx];
			itm->locked++;
		} else {
			//create record
			srch.method = NBMngrProcess_strClone_(func);
			srch.locked++;
			NBMngrProcess_sortedAdd_((void**)&obj->locks.byMethod.arr, &obj->locks.byMethod.use, &obj->locks.byMethod.alloc, sizeof(obj->locks.byMethod.arr[0]), NBCompare_NBThreadsMethodLocksOps, &srch, sizeof(srch));
		}
	}
}

void NBMngrProcessStatsState_mutexYielded(STNBMngrProcessStatsState* obj, const char* func){
	obj->locks.yield++;
	if(func != NULL){
		SI32 fndIdx = -1;
		STNBThreadsMethodLocksOps srch;
		NBMemory_setZeroSt(srch, STNBThreadsMethodLocksOps);
		srch.method = (char*)func;
		fndIdx = NBMngrProcess_sortedIndexOf_((void**)&obj->locks.byMethod.arr, &obj->locks.byMethod.use, &obj->locks.byMethod.alloc, sizeof(obj->locks.byMethod.arr[0]), NBCompare_NBThreadsMethodLocksOps, &srch, sizeof(srch));
		if(fndIdx >= 0){
			//update record
			STNBThreadsMethodLocksOps* itm = &obj->locks.byMethod.arr[fndIdx];
			itm->yielded++;
		} else {
			//create record
			srch.method = NBMngrProcess_strClone_(func);
			srch.yielded++;
			NBMngrProcess_sortedAdd_((void**)&obj->locks.byMethod.arr, &obj->locks.byMethod.use, &obj->locks.byMethod.alloc, sizeof(obj->locks.byMethod.arr[0]), NBCompare_NBThreadsMethodLocksOps, &srch, sizeof(srch));
		}
	}
}

void NBMngrProcessStatsState_mutexUnlocked(STNBMngrProcessStatsState* obj, const char* func){
	obj->locks.unlocked++;
	if(func != NULL){
		SI32 fndIdx = -1;
		STNBThreadsMethodLocksOps srch;
		NBMemory_setZeroSt(srch, STNBThreadsMethodLocksOps);
		srch.method = (char*)func;
		fndIdx = NBMngrProcess_sortedIndexOf_((void**)&obj->locks.byMethod.arr, &obj->locks.byMethod.use, &obj->locks.byMethod.alloc, sizeof(obj->locks.byMethod.arr[0]), NBCompare_NBThreadsMethodLocksOps, &srch, sizeof(srch));
		if(fndIdx >= 0){
			//update record
			STNBThreadsMethodLocksOps* itm = &obj->locks.byMethod.arr[fndIdx];
			itm->unlocked++;
		} else {
			//create record
			srch.method = NBMngrProcess_strClone_(func);
			srch.unlocked++;
			NBMngrProcess_sortedAdd_((void**)&obj->locks.byMethod.arr, &obj->locks.byMethod.use, &obj->locks.byMethod.alloc, sizeof(obj->locks.byMethod.arr[0]), NBCompare_NBThreadsMethodLocksOps, &srch, sizeof(srch));
		}
	}
}

//------------
//- Mutex/cond
//------------

void NBMngrProcessMutexx_init_(STNBMngrProcessMutexx* obj){
#	ifdef _WIN32
#		ifndef NB_COMPILE_DRIVER_MODE
			InitializeCriticalSection(&obj->mutex);
#		endif
#	else
	pthread_mutex_init(&obj->mutex, NULL);
#	endif
#	ifdef _WIN32
#		ifndef NB_COMPILE_DRIVER_MODE
			InitializeConditionVariable(&obj->cond); //Windows Vista
#		endif
#	else
	pthread_cond_init(&obj->cond, NULL);
#	endif
}

void NBMngrProcessMutexx_release_(STNBMngrProcessMutexx* obj){
#	ifdef _WIN32
	//No need to destroy the 'cond' variable
#	else
	pthread_cond_destroy(&obj->cond);
#	endif
#	ifdef _WIN32
#		ifndef NB_COMPILE_DRIVER_MODE
			DeleteCriticalSection(&obj->mutex);
#		endif
#	else
	pthread_mutex_destroy(&obj->mutex);
#	endif
}

void NBMngrProcessMutexx_lock_(STNBMngrProcessMutexx* obj){
#	ifdef _WIN32
#		ifndef NB_COMPILE_DRIVER_MODE
			EnterCriticalSection(&obj->mutex);
#		endif
#	else
	int rr;
	if((rr = pthread_mutex_lock(&obj->mutex)) != 0){
		NB_STATIC_ASSERT(FALSE)
	} else {
		//r = TRUE;
	}
#	endif
}

void NBMngrProcessMutexx_unlock_(STNBMngrProcessMutexx* obj){
#	ifdef _WIN32
#		ifndef NB_COMPILE_DRIVER_MODE
			LeaveCriticalSection(&obj->mutex);
#		endif
#	else
	int rr;
	if((rr = pthread_mutex_unlock(&obj->mutex)) != 0){
		NB_STATIC_ASSERT(FALSE)
	} else {
		//r = TRUE;
	}
#	endif
}

BOOL NBMngrProcessMutexx_wait_(STNBMngrProcessMutexx* obj){
	BOOL r = FALSE;
#	ifdef _WIN32
#		ifndef NB_COMPILE_DRIVER_MODE
			r = SleepConditionVariableCS(&obj->cond, &obj->mutex, INFINITE); //Windows Vista
#		endif
#	else
	int rr;
	if((rr = pthread_cond_wait(&obj->cond, &obj->mutex)) != 0){
		NB_STATIC_ASSERT(FALSE)
	} else {
		r = TRUE;
	}
#	endif
	return r;
}

BOOL NBMngrProcessMutexx_timedWait_(STNBMngrProcessMutexx* obj, const UI32 msMax){
	BOOL r = FALSE;
#	ifdef _WIN32
	r = SleepConditionVariableCS(&obj->cond, &obj->mutex, msMax); //Windows Vista
#	else
	{
		struct timespec timeout;
        if(0 == clock_gettime(CLOCK_REALTIME, &timeout)){
            int rr;
            timeout.tv_nsec += (msMax % 1000) * 1000000;
            timeout.tv_sec += (msMax / 1000) + (timeout.tv_nsec / 1000000000);
            timeout.tv_nsec %= 1000000000;
            rr = pthread_cond_timedwait(&obj->cond, &obj->mutex, &timeout);
            if (rr != 0 && rr != ETIMEDOUT) {
                NB_STATIC_ASSERT(FALSE)
            } else {
                r = TRUE;
            }
        }
	}
#	endif
	return r;
}

BOOL NBMngrProcessMutexx_signal_(STNBMngrProcessMutexx* obj){
	BOOL r = FALSE;
#	ifdef _WIN32
#		ifndef NB_COMPILE_DRIVER_MODE
		{
			WakeConditionVariable(&obj->cond); //Windows Vista
			r = TRUE;
		}
#		endif
#	else
	int rr;
	if((rr = pthread_cond_signal(&obj->cond)) != 0){
		NB_STATIC_ASSERT(FALSE)
	} else {
		r = TRUE;
	}
#	endif
	return r;
}

BOOL NBMngrProcessMutexx_broadcast_(STNBMngrProcessMutexx* obj) {
	BOOL r = FALSE;
#	ifdef _WIN32
#		ifndef NB_COMPILE_DRIVER_MODE
		{
			WakeAllConditionVariable(&obj->cond); //Windows Vista
			r = TRUE;
		}
#		endif
#	else
	int rr;
	if((rr = pthread_cond_broadcast(&obj->cond)) != 0){
		NB_STATIC_ASSERT(FALSE)
	} else {
		r = TRUE;
	}
#	endif
	return r;
}

//----------------------
//- NBMngrProcessCodePos
//----------------------

void NBMngrProcessCodePos_init(STNBMngrProcessCodePos* obj){
	memset(obj, 0, sizeof(*obj));
}

void NBMngrProcessCodePos_release(STNBMngrProcessCodePos* obj){
	if(obj->fullpath != NULL){
		NBMemory_freeUnmanaged(obj->fullpath);
		obj->fullpath = NULL;
	}
	if(obj->func != NULL){
		NBMemory_freeUnmanaged(obj->func);
		obj->func = NULL;
	}
	obj->line = 0;
}

void NBMngrProcessCodePos_set(STNBMngrProcessCodePos* obj, const char* fullpath, const SI32 line, const char* func){
	if(obj->fullpath != NULL){
		NBMemory_freeUnmanaged(obj->fullpath);
		obj->fullpath = NULL;
	}
	if(obj->func != NULL){
		NBMemory_freeUnmanaged(obj->func);
		obj->func = NULL;
	}
	if(fullpath != NULL){
		const UI32 len = NBMngrProcess_strLenBytes_(fullpath);
		obj->fullpath = NBMemory_allocUnmanaged(len + 1);
		memcpy(obj->fullpath, fullpath, len + 1);
	}
	if(func != NULL){
		const UI32 len = NBMngrProcess_strLenBytes_(func);
		obj->func = NBMemory_allocUnmanaged(len + 1);
		memcpy(obj->func, func, len + 1);
	}
	obj->line	= line;
}

BOOL NBMngrProcessCodePos_isSame(STNBMngrProcessCodePos* obj, const STNBMngrProcessCodePos* other){
	return (
			obj == other //both null or same pointer
			|| (obj != NULL && other != NULL && obj->line == other->line && NBMngrProcess_strIsEqual_(obj->func, other->func) && NBMngrProcess_strIsEqual_(obj->fullpath, other->fullpath)) //match
			);
}

void NBMngrProcessCodePos_clone(STNBMngrProcessCodePos* obj, const STNBMngrProcessCodePos* other){
	if(other != NULL){
		NBMngrProcessCodePos_set(obj, other->fullpath, other->line, other->func);
	}
}

//--------------------------
//- NBMngrProcessCodePosPair
//--------------------------

void NBMngrProcessCodePosPair_init(STNBMngrProcessCodePosPair* obj){
	NBMemory_setZeroSt(*obj, STNBMngrProcessCodePosPair);
}

void NBMngrProcessCodePosPair_release(STNBMngrProcessCodePosPair* obj){
	NBMngrProcessCodePos_release(&obj->one);
	NBMngrProcessCodePos_release(&obj->other);
}

void NBMngrProcessCodePosPair_set(STNBMngrProcessCodePosPair* obj, const STNBMngrProcessCodePos* one, const STNBMngrProcessCodePos* other){
	NBMngrProcessCodePos_clone(&obj->one, one);
	NBMngrProcessCodePos_clone(&obj->other, other);
}

BOOL NBMngrProcessCodePosPair_isSame(STNBMngrProcessCodePosPair* obj, const STNBMngrProcessCodePos* one, const STNBMngrProcessCodePos* other){
	return (
			(NBMngrProcessCodePos_isSame(&obj->one, one) && NBMngrProcessCodePos_isSame(&obj->other, other))
			|| (NBMngrProcessCodePos_isSame(&obj->one, other) && NBMngrProcessCodePos_isSame(&obj->other, one))
			);
}

void NBMngrProcessCodePosPair_clone(STNBMngrProcessCodePosPair* obj, const STNBMngrProcessCodePosPair* other){
	if(other != NULL){
		NBMngrProcessCodePos_clone(&obj->one, &other->one);
		NBMngrProcessCodePos_clone(&obj->other, &other->other);
	}
}

//----------------------
//- NBMngrProcessLockPos
//----------------------

void NBMngrProcessLockPos_init(STNBMngrProcessLockPos* obj){
	NBMemory_setZeroSt(*obj, STNBMngrProcessLockPos);
}

void NBMngrProcessLockPos_release(STNBMngrProcessLockPos* obj){
	NBMngrProcessCodePos_release(&obj->codePos);
}

void NBMngrProcessLockPos_set(STNBMngrProcessLockPos* obj, const char* fullpath, const SI32 line, const char* func, const UI64 lockId, const ENNBThreadLockPushMode	pushMode){
	NBMngrProcessCodePos_set(&obj->codePos, fullpath, line, func);
	obj->lockId					= lockId;
	obj->pushMode				= pushMode;
}

void NBMngrProcessLockPos_clone(STNBMngrProcessLockPos* obj, const STNBMngrProcessLockPos* other){
	if(other != NULL){
		NBMngrProcessLockPos_set(obj, other->codePos.fullpath, other->codePos.line, other->codePos.func, other->lockId, other->pushMode);
	}
}

//------------------------
//- NBMngrProcessThreadPos
//------------------------

void NBMngrProcessThreadPos_init(STNBMngrProcessThreadPos* obj){
	NBMemory_setZeroSt(*obj, STNBMngrProcessThreadPos);
}

void NBMngrProcessThreadPos_release(STNBMngrProcessThreadPos* obj){
	NBMngrProcessCodePos_release(&obj->codePos);
}

void NBMngrProcessThreadPos_set(STNBMngrProcessThreadPos* obj, const char* fullpath, const SI32 line, const char* func, struct STNBMngrProcessThread_* thread, const ENNBThreadLockPushMode pushMode){
	NBMngrProcessCodePos_set(&obj->codePos, fullpath, line, func);
	obj->thread		= thread;
	obj->pushMode	= pushMode;
}

void NBMngrProcessThreadPos_clone(STNBMngrProcessThreadPos* obj, const STNBMngrProcessThreadPos* other){
	if(other != NULL){
		NBMngrProcessThreadPos_set(obj, other->codePos.fullpath, other->codePos.line, other->codePos.func, other->thread, other->pushMode);
	}
}

//---------------------
//- NBMngrProcessThread
//---------------------

void NBMngrProcessThread_init(STNBMngrProcessThread* obj){
	memset(obj, 0, sizeof(*obj));
	//First known pos
	NBMngrProcessCodePos_init(&obj->firstKwnonCall);
	//stats
#	ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
	{
		//data
		NBMngrProcessStatsData_init(&obj->stats.data);
		//memEnabled
		{
			//
		}
	}
#	endif
}

void NBMngrProcessThread_release(STNBMngrProcessThread* obj){
	//First known pos
	NBMngrProcessCodePos_release(&obj->firstKwnonCall);
	//locks
#	ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION
	NB_STATIC_ASSERT(obj->locks.use == 0)
	{
		if(obj->locks.stack != NULL){
			SI32 i; for(i = 0 ; i < obj->locks.use; i++){
				STNBMngrProcessLockPos* itm = &obj->locks.stack[i];
				NBMngrProcessLockPos_release(itm);
			}
			NBMemory_freeUnmanaged(obj->locks.stack);
			obj->locks.stack = NULL;
		}
		obj->locks.use	= 0;
		obj->locks.size	= 0;
	}
#	endif
    //storages
#   ifdef CONFIG_NB_INCLUDE_THREADS_STORAGE_CLEANUP
    {
        if(obj->storages.arr != NULL){
            NBMemory_freeUnmanaged(obj->storages.arr);
            obj->storages.arr = NULL;
        }
        obj->storages.use = 0;
        obj->storages.size = 0;
    }
#   endif
	//stats
#	ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
	{
		NBMngrProcessStatsData_release(&obj->stats.data);
		//memEnabled
		{
			NB_STATIC_ASSERT(obj->stats.memEnabled.use == 0) //should be empty
			if(obj->stats.memEnabled.stack != NULL){
				NBMemory_freeUnmanaged(obj->stats.memEnabled.stack);
				obj->stats.memEnabled.stack = NULL;
			}
			obj->stats.memEnabled.use = obj->stats.memEnabled.size = 0; 
		}
	}
#	endif
}

void NBMngrProcessThread_setFirstKnownFunc(STNBMngrProcessThread* obj, const char* fullpath, const SI32 line, const char* func){
	NBMngrProcessCodePos_set(&obj->firstKwnonCall, fullpath, line, func);
}

#ifdef CONFIG_NB_INCLUDE_THREADS_STORAGE_CLEANUP
void NBMngrProcessThread_storageDestroyAllLocked_(STNBMngrProcessThread* obj){
    STNBMngrProcess* mngr = __nbMngrThreads;
    SI32 useBefore = obj->storages.use + 1;
    while(obj->storages.use > 0 && useBefore != obj->storages.use){
        void* d = obj->storages.arr[0];
        useBefore = obj->storages.use;
        NBMngrProcess_unlock(mngr);
        {
            //This call should remove
            //the storage reference.
            NBThreadStorage_destroyFromData(d);
        }
        NBMngrProcess_lock(mngr);
        NBASSERT(useBefore != obj->storages.use)
    }
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION
void NBMngrProcess_lockPushLocked_(STNBMngrProcess* obj, STNBMngrProcessThread* st, const UI64 lockId, const char* fullpath, const SI32 line, const char* func, const ENNBThreadLockPushMode pushMode){
	//Validate my locks.stack (thread lock itself?)
	if(st->locks.use > 0){
		BOOL errFnd = FALSE; const char* errDesc = NULL;
		SI32 i; for(i = (st->locks.use - 1); i >= 0 && !errFnd; i--){
			STNBMngrProcessLockPos* itm = &st->locks.stack[i];
			if(itm->lockId == lockId){
				errFnd = TRUE;
				errDesc = "Thread permanetly locked itself.";
			}
		}
		//Print error
		if(errFnd){
			PRINTF_CONSOLE_ERROR("NBMngrProcess, validation fail: '%s'\n", errDesc);
			NBMngrProcessMutex_printLocksStackConsoleError("               ", st->locks.stack, st->locks.use, obj->locks.arr, obj->locks.use, obj->locks.seqUIDs);
			NBMngrProcessMutex_printLocksStackConsoleErrorLine("           (->)", st->locks.use, st->locks.use, func, fullpath, line, 0, NULL, NULL);
			abort();
		}
	}
	//Validate against other stacks (threads mutual lock?)
	{
		SI32 i; for(i = (obj->threads.use - 1); i >= 0; i--){
			const STNBMngrProcessThread* t2 = obj->threads.arr[i];
			if(st != t2){
				SI32 i; for(i = (t2->locks.use - 1); i >= 0; i--){
					STNBMngrProcessLockPos* itm = &t2->locks.stack[i];
					if(itm->lockId == lockId){
						//This other thread already have the mutex,
						//determine if any of the mutexes is shared between stacks.
						SI32 i; for(i = (t2->locks.use - 1); i >= 0; i--){
							STNBMngrProcessLockPos* itm = &t2->locks.stack[i];
							SI32 i2; for(i2 = (st->locks.use - 1); i2 >= 0; i2--){
								STNBMngrProcessLockPos* itm2 = &st->locks.stack[i2];
								if(itm->lockId == itm2->lockId){
									PRINTF_CONSOLE_ERROR("NBMngrProcess, validation fail: 'threads mutual permanent lock'\n");
									{
										PRINTF_CONSOLE_ERROR("               current thread:\n");
										NBMngrProcessMutex_printLocksStackConsoleError("               ", st->locks.stack, st->locks.use, obj->locks.arr, obj->locks.use, obj->locks.seqUIDs);
										NBMngrProcessMutex_printLocksStackConsoleErrorLine("           (->)", st->locks.use, st->locks.use, func, fullpath, line, 0, NULL, NULL);
									}
									{
										PRINTF_CONSOLE_ERROR("               other thread:\n");
										NBMngrProcessMutex_printLocksStackConsoleError("               ", t2->locks.stack, t2->locks.use, obj->locks.arr, obj->locks.use, obj->locks.seqUIDs);
									}
									abort();
									break;
								}
							}
						}
						break;
					}
				}
			}
		}
	}
	//update lock
#	if defined(CONFIG_NB_INCLUDE_THREADS_ENABLED) || defined(CONFIG_NB_INCLUDE_THREADS_LOCKS_RISK_VALIDATION)
	{
		STNBMngrProcessMutex* lock = NBMngrProcess_getMutexLocked_(obj, lockId);
		if(lock == NULL){
			PRINTF_CONSOLE_ERROR("NBMngrProcess, NBMngrProcess_lockPushLocked_, lock not previoudly registered.\n");
			NB_STATIC_ASSERT(FALSE);
		} else {
			//Config this lock
			{
				if(lock->firstKwnonCall.func == NULL && func != NULL){
					NBMngrProcessMutex_setFirstKnownFunc(lock, fullpath, line, func);
				}
			}
			//add thread racing for the mutex
#			ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
			{
				//resize array
				if(lock->racing.use >= lock->racing.size){
					STNBMngrProcessThreadPos* arrN = NULL;
					lock->racing.size += 8;
					arrN = (STNBMngrProcessThreadPos*)NBMemory_allocUnmanaged(sizeof(lock->racing.threads[0]) * lock->racing.size);
					if(lock->racing.threads != NULL){
						if(lock->racing.use > 0){
							memcpy(arrN, lock->racing.threads, sizeof(lock->racing.threads[0]) * lock->racing.use);
						}
						NBMemory_freeUnmanaged(lock->racing.threads);
					}
					lock->racing.threads = arrN;
				}
				//add racing thread
				{
					STNBMngrProcessThreadPos* tPos = &lock->racing.threads[lock->racing.use++];
					NBMngrProcessThreadPos_init(tPos);
					NBMngrProcessThreadPos_set(tPos, fullpath, line, func, st, pushMode);
				}
			}
#			endif
			//Validate lock mutex order (risk of mutual-lock)
			//Add this lock after all others
#			ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_RISK_VALIDATION
			{
				SI32 i; for(i = (st->locks.use - 1); i >= 0; i--){
					STNBMngrProcessLockPos* itm = &st->locks.stack[i];
					STNBMngrProcessMutex* upper = NBMngrProcess_getMutexLocked_(obj, itm->lockId);
					if(upper == NULL){
						PRINTF_CONSOLE_ERROR("NBMngrProcess, NBMngrProcess_mutexLockPush, lock not previoudly registered.\n");
						NB_STATIC_ASSERT(FALSE);
					} else {
						NBMngrProcessMutex_addLockAfter(upper, lock, st, obj->locks.arr, obj->locks.use, obj->locks.seqUIDs, lockId, fullpath, line, func);
					}
				}
			}
#			endif
		}
	}
#	endif
	//Add item
	{
		STNBMngrProcessLockPos itm;
		NBMngrProcessLockPos_init(&itm);
		NBMngrProcessLockPos_set(&itm, fullpath, line, func, lockId, pushMode);
		//PRINTF_INFO("NBMngrProcess, pushing level: '%s' (%s).\n", itm.fullpath, (itm.isWriteOp ? "write" : "read"));
		NB_STATIC_ASSERT(st->locks.use <= st->locks.size)
		if(st->locks.use >= st->locks.size){
			STNBMngrProcessLockPos* arrN;
			st->locks.size += 8;
			arrN = NBMemory_allocUnmanaged(sizeof(st->locks.stack[0]) * st->locks.size);
			if(st->locks.stack != NULL){
				if(st->locks.use > 0){
					memcpy(arrN, st->locks.stack, sizeof(st->locks.stack[0]) * st->locks.use);
				}
				NBMemory_freeUnmanaged(st->locks.stack);
			}
			st->locks.stack = arrN;
		}
		//stats
#		ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
		if(pushMode == ENNBThreadLockPushMode_Compete){
			//thread stats
			{
				//alive
				NBMngrProcessStatsState_mutexLocked(&st->stats.data.alive, NULL);
				//accum
				NBMngrProcessStatsState_mutexLocked(&st->stats.data.accum, func);
				//total
				NBMngrProcessStatsState_mutexLocked(&st->stats.data.total, func);
			}
			//global stats
			{
				//alive
				NBMngrProcessStatsState_mutexLocked(&obj->stats.alive, NULL);
				//accum
				NBMngrProcessStatsState_mutexLocked(&obj->stats.accum, func);
				//total
				NBMngrProcessStatsState_mutexLocked(&obj->stats.total, func);
			}
		}
#		endif
		st->locks.stack[st->locks.use++] = itm;
	}
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION
void NBMngrProcess_lockStartedLocked_(STNBMngrProcess* obj, STNBMngrProcessThread* st, const UI64 lockId, const char* fullpath, const SI32 line, const char* func){
	//update lock
	{
		BOOL isTop = FALSE;
		if(st->locks.use > 0){
			STNBMngrProcessLockPos* itm = &st->locks.stack[st->locks.use - 1];
			if(itm->lockId == lockId){
				isTop = TRUE;
				NB_STATIC_ASSERT(!itm->started) //should be started once max
				itm->started = TRUE;
				//stats
#				ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
				if(itm->pushMode == ENNBThreadLockPushMode_NonCompete){
					//thread stats
					{
						//alive
						NBMngrProcessStatsState_mutexLocked(&st->stats.data.alive, NULL);
						//accum
						NBMngrProcessStatsState_mutexLocked(&st->stats.data.accum, func);
						//total
						NBMngrProcessStatsState_mutexLocked(&st->stats.data.total, func);
					}
					//global stats
					{
						//alive
						NBMngrProcessStatsState_mutexLocked(&obj->stats.alive, NULL);
						//accum
						NBMngrProcessStatsState_mutexLocked(&obj->stats.accum, func);
						//total
						NBMngrProcessStatsState_mutexLocked(&obj->stats.total, func);
					}
				}
#				endif
			}
		}
		//Print error
		if(!isTop){
			PRINTF_CONSOLE_ERROR("NBMngrProcess, validation fail: owning mutex that is not on the top (missing 'lock').\n");
			PRINTF_CONSOLE_ERROR("               popping: (%llu) '%s' (%s:%d)\n", lockId, func, NBMngrProcess_fileNameOnly_(fullpath), line);
			NBMngrProcessMutex_printLocksStackConsoleError("               ", st->locks.stack, st->locks.use, obj->locks.arr, obj->locks.use, obj->locks.seqUIDs);
			abort();
		}
	}
#	if defined(CONFIG_NB_INCLUDE_THREADS_ENABLED)
	{
		STNBMngrProcessMutex* lock = NBMngrProcess_getMutexLocked_(obj, lockId);
		if(lock == NULL){
			PRINTF_CONSOLE_ERROR("NBMngrProcess, NBMngrProcess_lockPushLocked_, lock not previoudly registered.\n");
			NB_STATIC_ASSERT(FALSE);
		} else {
			//Config this lock
			{
				if(lock->firstKwnonCall.func == NULL && func != NULL){
					NBMngrProcessMutex_setFirstKnownFunc(lock, fullpath, line, func);
				}
			}
			//add thread racing for the mutex
			{
				//Apply 'yield' to competing threads
				BOOL fnd = FALSE;
				SI32 i; for(i = 0; i < lock->racing.use; i++){
					STNBMngrProcessThreadPos* tPos = &lock->racing.threads[i];
					if(tPos->pushMode == ENNBThreadLockPushMode_Compete || tPos->thread == st){
						if(!tPos->yieldAnalized){
							if(tPos->thread == st){
#								ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
								//thread-stats
								tPos->thread->stats.data.accum.locks.passby++;
								tPos->thread->stats.data.total.locks.passby++;
								//global-stats
								obj->stats.accum.locks.passby++;
								obj->stats.total.locks.passby++;
#								endif
							} else {
#								ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
								//thread-stats
								{
									//accum
									NBMngrProcessStatsState_mutexYielded(&tPos->thread->stats.data.accum, func);
									//total
									NBMngrProcessStatsState_mutexYielded(&tPos->thread->stats.data.total, func);
								}
								//global
								{
									//accum
									NBMngrProcessStatsState_mutexYielded(&obj->stats.accum, func);
									//total
									NBMngrProcessStatsState_mutexYielded(&obj->stats.total, func);
								}
#								endif
							}
							tPos->yieldAnalized = TRUE;
						}
					}
					if(tPos->thread == st){
						NB_STATIC_ASSERT(!fnd) //should be only once
						fnd = TRUE;
					}
				} NB_STATIC_ASSERT(fnd) //should be found
				//set as owner
				{
					NB_STATIC_ASSERT(lock->racing.owner == NULL)
					lock->racing.owner = st;
				}
			}
		}
	}
#	endif
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION
void NBMngrProcess_lockPopLocked_(STNBMngrProcess* obj, STNBMngrProcessThread* st, const UI64 lockId, const char* fullpath, const SI32 line, const char* func){
	BOOL lockStarted = FALSE;
	//remove lock
	{
		BOOL wasTop = FALSE;
		if(st->locks.use > 0){
			STNBMngrProcessLockPos* itm = &st->locks.stack[st->locks.use - 1];
			if(itm->lockId == lockId){
				NB_STATIC_ASSERT(itm->started || itm->pushMode == ENNBThreadLockPushMode_NonCompete)
				lockStarted = itm->started;
				//validate 'lockStarted' call
				if(itm->pushMode == ENNBThreadLockPushMode_Compete && !itm->started){
					PRINTF_CONSOLE_ERROR("NBMngrProcess, validation fail: popping-lock before lock started (missing 'lockStarted' call).\n");
					PRINTF_CONSOLE_ERROR("               popping: (%llu) '%s' (%s:%d)\n", lockId, func, NBMngrProcess_fileNameOnly_(fullpath), line);
					NBMngrProcessMutex_printLocksStackConsoleError("               ", st->locks.stack, st->locks.use, obj->locks.arr, obj->locks.use, obj->locks.seqUIDs);
				}
				//stats
#				ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
				if(itm->started){
					//thread stats
					{
						//alive
						NB_STATIC_ASSERT(st->stats.data.alive.locks.locked > 0)
						st->stats.data.alive.locks.locked--;
						//accum
						NBMngrProcessStatsState_mutexUnlocked(&st->stats.data.accum, func);
						//total
						NBMngrProcessStatsState_mutexUnlocked(&st->stats.data.total, func);
					}
					//global stats
					{
						//alive
						NB_STATIC_ASSERT(obj->stats.alive.locks.locked > 0)
						obj->stats.alive.locks.locked--;
						//accum
						NBMngrProcessStatsState_mutexUnlocked(&obj->stats.accum, func);
						//total
						NBMngrProcessStatsState_mutexUnlocked(&obj->stats.total, func);
					}
				}
#				endif
				NBMngrProcessLockPos_release(itm);
				st->locks.use--;
				wasTop = TRUE;
			}
		}
		//Print error
		if(!wasTop){
			PRINTF_CONSOLE_ERROR("NBMngrProcess, validation fail: unlocking mutex that is not on the top (missing 'lock').\n");
			PRINTF_CONSOLE_ERROR("               popping: (%llu) '%s' (%s:%d)\n", lockId, func, NBMngrProcess_fileNameOnly_(fullpath), line);
			NBMngrProcessMutex_printLocksStackConsoleError("               ", st->locks.stack, st->locks.use, obj->locks.arr, obj->locks.use, obj->locks.seqUIDs);
			abort();
		}
	}
	//update lock
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	{
		STNBMngrProcessMutex* lock = NBMngrProcess_getMutexLocked_(obj, lockId);
		if(lock == NULL){
			PRINTF_CONSOLE_ERROR("NBMngrProcess, NBMngrProcess_lockPopLocked_, lock not previoudly registered.\n");
			NB_STATIC_ASSERT(FALSE);
		} else {
			ENNBThreadLockPushMode pushMode = ENNBThreadLockPushMode_Compete;
			//Config this lock
			{
				if(lock->firstKwnonCall.func == NULL && func != NULL){
					NBMngrProcessMutex_setFirstKnownFunc(lock, fullpath, line, func);
				}
			}
			//remove threads racing for the mutex
			{
				BOOL fnd = FALSE;
				SI32 i; for(i = 0; i < lock->racing.use; i++){
					STNBMngrProcessThreadPos* tPos = &lock->racing.threads[i];
					if(tPos->thread == st){
						//remove
						NB_STATIC_ASSERT(!fnd) //should be only once
						NB_STATIC_ASSERT(tPos->yieldAnalized || tPos->pushMode == ENNBThreadLockPushMode_NonCompete) //should be analized
						pushMode = tPos->pushMode;
						NBMngrProcessThreadPos_release(tPos);
						//fill gap
						{
							SI32 i2; for(i2 = i + 1; i2 < lock->racing.use; i2++){
								lock->racing.threads[i2 - 1] = lock->racing.threads[i2]; 
							}
							i--; lock->racing.use--;
						}
						fnd = TRUE;
					}
				} NB_STATIC_ASSERT(fnd) //should be found
			}
			//remove as owner
			NB_STATIC_ASSERT((lockStarted && lock->racing.owner == st) || (!lockStarted && lock->racing.owner != st))
			if(lock->racing.owner == st){
				NB_STATIC_ASSERT(lockStarted)
				lock->racing.owner = NULL;
			} else {
				NB_STATIC_ASSERT(pushMode == ENNBThreadLockPushMode_NonCompete)
			}
		}
	}
#	endif
}
#endif

//------------------------
//- NBMngrProcessMutexRef
//------------------------

void NBMngrProcessMutexRef_init(STNBMngrProcessMutexRef* obj){
	memset(obj, 0, sizeof(*obj));
}

void NBMngrProcessMutexRef_release(STNBMngrProcessMutexRef* obj){
	//stack
	{
		if(obj->firstKnown.stack != NULL){
			UI32 i; for(i = 0; i < obj->firstKnown.size; i++){
				STNBMngrProcessLockPos* itm = &obj->firstKnown.stack[i];
				NBMngrProcessLockPos_release(itm);
			}
			NBMemory_freeUnmanaged(obj->firstKnown.stack);
			obj->firstKnown.stack = NULL;
		}
		obj->firstKnown.size = 0;
	}
}

void NBMngrProcessMutexRef_setFirstKwnonStack(STNBMngrProcessMutexRef* obj, const STNBMngrProcessLockPos* stack, const UI32 stackSz){
	//Release current
	{
		if(obj->firstKnown.stack != NULL){
			UI32 i; for(i = 0; i < obj->firstKnown.size; i++){
				STNBMngrProcessLockPos* itm = &obj->firstKnown.stack[i];
				NBMngrProcessLockPos_release(itm);
			}
			NBMemory_freeUnmanaged(obj->firstKnown.stack);
			obj->firstKnown.stack = NULL;
		}
		obj->firstKnown.size = 0;
	}
	//clone stack
	if(stack != NULL && stackSz > 0){
		STNBMngrProcessLockPos* arrN = (STNBMngrProcessLockPos*)NBMemory_allocUnmanaged(sizeof(STNBMngrProcessLockPos) * stackSz); 
		UI32 i; for(i = 0; i < stackSz; i++){
			STNBMngrProcessLockPos* itm = &arrN[i];
			NBMngrProcessLockPos_init(itm);
			NBMngrProcessLockPos_clone(itm, &stack[i]);
		}
		obj->firstKnown.stack	= arrN;
		obj->firstKnown.size	= stackSz;
	}
}

//-------------------------
//- NBMngrProcessMutex
//-------------------------

void NBMngrProcessMutex_init(STNBMngrProcessMutex* obj){
	memset(obj, 0, sizeof(*obj));
	NBMngrProcessCodePos_init(&obj->firstKwnonCall);
}

void NBMngrProcessMutex_release(STNBMngrProcessMutex* obj){
	obj->uid = 0;
	//name
	if(obj->name != NULL){
		NBMemory_freeUnmanaged(obj->name);
		obj->name = NULL;
	}
	//frist known call
	NBMngrProcessCodePos_release(&obj->firstKwnonCall);
	//threads racing for the mutex
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	{
		NB_STATIC_ASSERT(obj->racing.use == 0)
		if(obj->racing.threads != NULL){
			SI32 i; for(i = 0; i < obj->racing.use; i++){
				NBMngrProcessThreadPos_release(&obj->racing.threads[i]);
			}
			NBMemory_freeUnmanaged(obj->racing.threads);
			obj->racing.threads = NULL;
		}
		obj->racing.use = 0;
		obj->racing.size = 0;
	}
#	endif
	//locks after this one
#	ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_RISK_VALIDATION
	{
		if(obj->locks.after != NULL){
			SI32 i; for(i = 0; i < obj->locks.use; i++){
				STNBMngrProcessMutexRef* itm = &obj->locks.after[i]; 
				NBMngrProcessMutexRef_release(itm);
			}
			NBMemory_freeUnmanaged(obj->locks.after);
			obj->locks.after = NULL;
		}
		obj->locks.use = 0;
		obj->locks.size = 0;
	}
#	endif
}

void NBMngrProcessMutex_setName(STNBMngrProcessMutex* obj, const char* name){
	if(obj->name != NULL){
		NBMemory_freeUnmanaged(obj->name);
		obj->name = NULL;
	}
	if(name != NULL){
		const UI32 len = NBMngrProcess_strLenBytes_(name);
		obj->name = NBMemory_allocUnmanaged(len + 1);
		memcpy(obj->name, name, len + 1);
	}
}

void NBMngrProcessMutex_setFirstKnownFunc(STNBMngrProcessMutex* obj, const char* fullpath, const SI32 line, const char* func){
	NBMngrProcessCodePos_set(&obj->firstKwnonCall, fullpath, line, func);
}

#ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_RISK_VALIDATION
void NBMngrProcessMutex_addLockAfter(STNBMngrProcessMutex* obj, const STNBMngrProcessMutex* other, const STNBMngrProcessThread* st, STNBMngrProcessMutex* locks, const UI32 locksSz, const UI64 locksMaxUID, const UI64 lockId, const char* fullpath, const SI32 line, const char* func){
	NB_STATIC_ASSERT(obj->uid != 0)
	NB_STATIC_ASSERT(other->uid != 0)
	NB_STATIC_ASSERT(obj != other)
	NB_STATIC_ASSERT(obj->uid != other->uid)
	//binary-search
	BOOL fnd = FALSE;
	SI32 posStart = 0;
	if(obj->locks.use > 0){		
		STNBMngrProcessMutexRef* dataMidd = NULL;
		SI32 posMidd = 0, posEnd = (obj->locks.use - 1);
		while(posStart <= posEnd){
			posMidd		= posStart + ((posEnd - posStart) / 2);
			dataMidd	= &obj->locks.after[posMidd];
			NB_STATIC_ASSERT((posStart == posMidd || obj->locks.after[posStart].uid < dataMidd->uid) && (posMidd == posEnd || dataMidd->uid < obj->locks.after[posEnd].uid))
			if(dataMidd->uid == other->uid){
				fnd = TRUE;
				break;
			} else if(other->uid < dataMidd->uid){
				posEnd		= posMidd - 1;
			} else {
				posStart	= posMidd + 1;
			}
		}
	}
	//Add
	if(!fnd){
		NB_STATIC_ASSERT(posStart >= 0 && posStart <= obj->locks.use)
		//Validate order
#		ifdef NB_CONFIG_INCLUDE_ASSERTS
		if(posStart < obj->locks.use){ 
			const STNBMngrProcessMutexRef* itm1 = &obj->locks.after[posStart];
			if(other->uid >= itm1->uid){
				PRINTF_ERROR("NBMngrProcessMutex, !(%llu < %llu).\n", other->uid, itm1->uid);
			}
			NB_STATIC_ASSERT(other->uid > 0 && other->uid <= locksMaxUID)
			NB_STATIC_ASSERT(itm1->uid > 0 && itm1->uid <= locksMaxUID)
			NB_STATIC_ASSERT(other->uid < itm1->uid)
		}
		if(posStart > 0){
			const STNBMngrProcessMutexRef* itm0 = &obj->locks.after[posStart - 1];
			if(itm0->uid >= other->uid){
				PRINTF_ERROR("NBMngrProcessMutex, !(%llu < %llu).\n", itm0->uid, other->uid);
			}
			NB_STATIC_ASSERT(other->uid > 0 && other->uid <= locksMaxUID)
			NB_STATIC_ASSERT(itm0->uid > 0 && itm0->uid <= locksMaxUID)
			NB_STATIC_ASSERT(itm0->uid < other->uid)
		}
#		endif
		//Increase array
		NB_STATIC_ASSERT(obj->locks.use <= obj->locks.size)
		if(obj->locks.use >= obj->locks.size){
			obj->locks.size += 16;
			{
				STNBMngrProcessMutexRef* arrN = (STNBMngrProcessMutexRef*)NBMemory_allocUnmanaged(sizeof(STNBMngrProcessMutexRef) * obj->locks.size);
				if(obj->locks.after != NULL){
					if(obj->locks.use > 0){
						memcpy(arrN, obj->locks.after, sizeof(STNBMngrProcessMutexRef) * obj->locks.use);
					}
					NBMemory_freeUnmanaged(obj->locks.after);
				}
				obj->locks.after = arrN;
			}
		}
		//Insert itm
		{
			//open gap
			{
				SI32 i; for(i = (SI32)obj->locks.use - 1; i >= posStart; i--){
					obj->locks.after[i + 1] = obj->locks.after[i];
				}
			}
			//Set
			{
				STNBMngrProcessMutexRef* itm = &obj->locks.after[posStart];
				NBMngrProcessMutexRef_init(itm);
				itm->uid = other->uid;
				//Clone first kwnon stack
				if(st != NULL){
					if(st->locks.stack != NULL && st->locks.use > 0){
						STNBMngrProcessLockPos* arrN = (STNBMngrProcessLockPos*)NBMemory_allocUnmanaged(sizeof(STNBMngrProcessLockPos) * st->locks.use);
						UI32 i; for(i = 0; i < st->locks.use; i++){
							STNBMngrProcessLockPos* itm2 = &arrN[i];
							NBMngrProcessLockPos_init(itm2);
							NBMngrProcessLockPos_clone(itm2, &st->locks.stack[i]);
						}
						itm->firstKnown.stack	= arrN;
						itm->firstKnown.size	= st->locks.use;
					}
				}
			}
			obj->locks.use++;
			//Validate order
#			ifdef NB_CONFIG_INCLUDE_ASSERTS
			if(posStart > 0){
				const STNBMngrProcessMutexRef* itm0 = &obj->locks.after[posStart - 1]; 
				const STNBMngrProcessMutexRef* itm1 = &obj->locks.after[posStart];
				if(itm0->uid >= itm1->uid){
					PRINTF_ERROR("NBMngrProcessMutex, !(%llu < %llu).\n", itm0->uid, itm1->uid);
				}
				NB_STATIC_ASSERT(itm0->uid > 0 && itm0->uid <= locksMaxUID)
				NB_STATIC_ASSERT(itm1->uid > 0 && itm1->uid <= locksMaxUID)
				NB_STATIC_ASSERT(itm0->uid < itm1->uid)
			}
			if((posStart + 1) < obj->locks.use){
				const STNBMngrProcessMutexRef* itm0 = &obj->locks.after[posStart]; 
				const STNBMngrProcessMutexRef* itm1 = &obj->locks.after[posStart + 1];
				if(itm0->uid >= itm1->uid){
					PRINTF_ERROR("NBMngrProcessMutex, !(%llu < %llu).\n", itm0->uid, itm1->uid);
				}
				NB_STATIC_ASSERT(itm0->uid > 0 && itm0->uid <= locksMaxUID)
				NB_STATIC_ASSERT(itm1->uid > 0 && itm1->uid <= locksMaxUID)
				NB_STATIC_ASSERT(itm0->uid < itm1->uid)
			}
#			endif
		}
	}
	//Validate order
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	if(obj->locks.use > 1){
		const STNBMngrProcessMutexRef* itmPrev = &obj->locks.after[0];
		NB_STATIC_ASSERT(itmPrev->uid > 0 && itmPrev->uid <= locksMaxUID)
		UI32 i; for(i = 1; i < obj->locks.use; i++){
			const STNBMngrProcessMutexRef* itm = &obj->locks.after[i];
			NB_STATIC_ASSERT(itmPrev->uid < itm->uid)
			NB_STATIC_ASSERT(itm->uid > 0 && itm->uid <= locksMaxUID)
			itmPrev = itm;
		}
	}
#	endif
	//Validate not in other
	{
		BOOL fnd = FALSE;
		STNBMngrProcessMutexRef* dataMidd = NULL;
		if(other->locks.use > 0){		
			SI32 posStart = 0, posMidd = 0, posEnd = (other->locks.use - 1);
			while(posStart <= posEnd){
				posMidd		= posStart + ((posEnd - posStart) / 2);
				dataMidd	= &other->locks.after[posMidd];
				NB_STATIC_ASSERT((posStart == posMidd || other->locks.after[posStart].uid < dataMidd->uid) && (posMidd == posEnd || dataMidd->uid < other->locks.after[posEnd].uid))
				if(dataMidd->uid == obj->uid){
					fnd = TRUE;
					break;
				} else if(obj->uid < dataMidd->uid){
					posEnd		= posMidd - 1;
				} else {
					posStart	= posMidd + 1;
				}
			}
		}
		if(fnd){
			PRINTF_CONSOLE_ERROR("NBMngrProcess, validation fail: 'risk of threads mutual permanent lock'.\n");
			PRINTF_CONSOLE_ERROR("               lockA->lockB and lockB->lockA detected (not simultaneously).\n");
			PRINTF_CONSOLE_ERROR("               lockA first call: function %s, file %s, line %d.\n", obj->firstKwnonCall.func, NBMngrProcess_fileNameOnly_(obj->firstKwnonCall.fullpath), obj->firstKwnonCall.line);
			PRINTF_CONSOLE_ERROR("               lockB first call: function %s, file %s, line %d.\n", other->firstKwnonCall.func, NBMngrProcess_fileNameOnly_(other->firstKwnonCall.fullpath), other->firstKwnonCall.line);
			PRINTF_CONSOLE_ERROR("       lockA current-stack:\n");
			NBMngrProcessMutex_printLocksStackConsoleError("               ", st->locks.stack, st->locks.use, locks, locksSz, locksMaxUID);
			NBMngrProcessMutex_printLocksStackConsoleErrorLine("           (->)", st->locks.use, st->locks.use, func, fullpath, line, 0, NULL, NULL);
			PRINTF_CONSOLE_ERROR("       lockB first-known-stack:\n");
			NBMngrProcessMutex_printLocksStackConsoleError("               ", dataMidd->firstKnown.stack, dataMidd->firstKnown.size, locks, locksSz, locksMaxUID);
			//
			abort();
		}
	}
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION
void NBMngrProcessMutex_printLocksStackConsoleErrorLine(const char* linesPrefix, const SI32 iStack, const SI32 stackSz, const char* tFunc, const char* tFullpath, const SI32 tLine, const UI64 lockId, const char* lockFirstKnownCallFunc, const char* lockName){
	if(lockId != 0 && !NBMngrProcess_strIsEmpty_(lockFirstKnownCallFunc) && !NBMngrProcess_strIsEmpty_(lockName)){
		PRINTF_CONSOLE_ERROR("%sstack (%d/%d, %llu) %s, %s:%d (%s / %s).\n", (!NBMngrProcess_strIsEmpty_(linesPrefix) ? linesPrefix : ""), iStack, stackSz, lockId, tFunc, NBMngrProcess_fileNameOnly_(tFullpath), tLine, lockFirstKnownCallFunc, lockName);
	} else {
		PRINTF_CONSOLE_ERROR("%sstack (%d/%d) %s, %s:%d.\n", (!NBMngrProcess_strIsEmpty_(linesPrefix) ? linesPrefix : ""), iStack, stackSz, tFunc, NBMngrProcess_fileNameOnly_(tFullpath), tLine);
	}
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION
void NBMngrProcessMutex_printLocksStackConsoleError(const char* linesPrefix, const STNBMngrProcessLockPos* arr, const UI32 sz, STNBMngrProcessMutex* locks, const UI32 locksSz, const UI64 locksMaxUID){
	if(arr != NULL && sz > 0){
		SI32 i; for(i = ((SI32)sz - 1); i >= 0; i--){
			const STNBMngrProcessLockPos* t = &arr[i];
			const STNBMngrProcessMutex* lock = NULL;
			if(t->lockId != 0){
				lock = NBMngrProcess_getMutexFromArr_(locks, locksSz, t->lockId, locksMaxUID);
			}
			if(lock != NULL){
				NBMngrProcessMutex_printLocksStackConsoleErrorLine(linesPrefix, (i + 1), sz, t->codePos.func, t->codePos.fullpath, t->codePos.line, lock->uid, lock->firstKwnonCall.func, lock->name);
				//PRINTF_CONSOLE_ERROR("%sstack (%d/%d, %llu) %s, %s:%d (%s / %s).\n", (linesPrefixIsNotEmpty ? linesPrefix : ""), (i + 1), sz, lock->uid, t->codePos.func, NBMngrProcess_fileNameOnly_(t->codePos.fullpath), t->codePos.line, lock->firstKwnonCall.func, lock->name);
			} else {
				NBMngrProcessMutex_printLocksStackConsoleErrorLine(linesPrefix, (i + 1), sz, t->codePos.func, t->codePos.fullpath, t->codePos.line, 0, NULL, NULL);
				//PRINTF_CONSOLE_ERROR("%sstack (%d/%d) %s, %s:%d.\n", (linesPrefixIsNotEmpty ? linesPrefix : ""), (i + 1), sz, t->codePos.func, NBMngrProcess_fileNameOnly_(t->codePos.fullpath), t->codePos.line);
			}
		}
	}
}
#endif

//-------------------------
//- NBMngrProcessCond
//-------------------------

void NBMngrProcessCond_init(STNBMngrProcessCond* obj){
	memset(obj, 0, sizeof(*obj));
	NBMngrProcessCodePos_init(&obj->firstKwnonCall);
}

void NBMngrProcessCond_release(STNBMngrProcessCond* obj){
	obj->uid = 0;
	//name
	if(obj->name != NULL){
		NBMemory_freeUnmanaged(obj->name);
		obj->name = NULL;
	}
	//frist known call
	NBMngrProcessCodePos_release(&obj->firstKwnonCall);
}

void NBMngrProcessCond_setName(STNBMngrProcessCond* obj, const char* name){
	if(obj->name != NULL){
		NBMemory_freeUnmanaged(obj->name);
		obj->name = NULL;
	}
	if(name != NULL){
		const UI32 len = NBMngrProcess_strLenBytes_(name);
		obj->name = NBMemory_allocUnmanaged(len + 1);
		memcpy(obj->name, name, len + 1);
	}
}

void NBMngrProcessCond_setFirstKnownFunc(STNBMngrProcessCond* obj, const char* fullpath, const SI32 line, const char* func){
	NBMngrProcessCodePos_set(&obj->firstKwnonCall, fullpath, line, func);
}

//--------------------
//- NBMngrProcessHndl
//--------------------

void NBMngrProcessHndl_init(STNBMngrProcessHndl* obj){
    memset(obj, 0, sizeof(*obj));
    NBMngrProcessCodePos_init(&obj->firstKwnonCall);
}

void NBMngrProcessHndl_release(STNBMngrProcessHndl* obj){
    obj->uid = 0;
    //frist known call
    NBMngrProcessCodePos_release(&obj->firstKwnonCall);
}

void NBMngrProcessHndl_setFirstKnownFunc(STNBMngrProcessHndl* obj, const char* fullpath, const SI32 line, const char* func){
    NBMngrProcessCodePos_set(&obj->firstKwnonCall, fullpath, line, func);
}

//--------------------
//- NBMngrProcessObj
//--------------------

void NBMngrProcessObj_init(STNBMngrProcessObj* obj){
    memset(obj, 0, sizeof(*obj));
    NBMngrProcessCodePos_init(&obj->firstKwnonCall);
#   ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION
    NBMngrProcessCodePos_init(&obj->firstKwnonLockPos);
#   endif
}

void NBMngrProcessObj_release(STNBMngrProcessObj* obj){
    obj->uid = 0;
    //name
    if(obj->name != NULL){
        NBMemory_freeUnmanaged(obj->name);
        obj->name = NULL;
    }
    //frist known call
    NBMngrProcessCodePos_release(&obj->firstKwnonCall);
#   ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION
    NBMngrProcessCodePos_release(&obj->firstKwnonLockPos);
#   endif
}

void NBMngrProcessObj_setName(STNBMngrProcessObj* obj, const char* name){
    if(obj->name != NULL){
        NBMemory_freeUnmanaged(obj->name);
        obj->name = NULL;
    }
    if(name != NULL){
        const UI32 len = NBMngrProcess_strLenBytes_(name);
        obj->name = NBMemory_allocUnmanaged(len + 1);
        memcpy(obj->name, name, len + 1);
    }
}


void NBMngrProcessObj_setFirstKnownFunc(STNBMngrProcessObj* obj, const char* fullpath, const SI32 line, const char* func){
    NBMngrProcessCodePos_set(&obj->firstKwnonCall, fullpath, line, func);
}

#ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION
void NBMngrProcessObj_setFirstKnownLockPos(STNBMngrProcessObj* obj, const char* fullpath, const SI32 line, const char* func){
    NBMngrProcessCodePos_set(&obj->firstKwnonLockPos, fullpath, line, func);
}
#endif

//stats

#ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
void NBMngrProcess_getGlobalStatsData(STNBMngrProcessStatsData* dst, const BOOL includeLocksByMethod, const BOOL resetAccum){
	STNBMngrProcess* obj = __nbMngrThreads;
	NBMngrProcess_lock(obj);
	{
		NBMngrProcessStatsData_add(dst, &obj->stats, includeLocksByMethod);
		if(resetAccum){
			NBMngrProcessStatsState_release(&obj->stats.accum);
			memset(&obj->stats.accum, 0, sizeof(obj->stats.accum));
		}
	}
	NBMngrProcess_unlock(obj);
}
#endif

//stacks and stats

#ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
void NBMngrProcess_getCurThreadStatsData(STNBMngrProcessStatsData* dst, const BOOL includeLocksByMethod, const BOOL resetAccum){
	STNBMngrProcess* obj = __nbMngrThreads;
	if(dst != NULL){
		NBMngrProcess_lock(obj);
		{
			STNBMngrProcessThread* st = NBStatePerThread_get(__FILE__, __LINE__, __func__, FALSE);
			if(st != NULL){
				NBMngrProcessStatsData_add(dst, &st->stats.data, includeLocksByMethod);
				if(resetAccum){
					NBMngrProcessStatsState_release(&st->stats.data.accum);
					memset(&st->stats.data.accum, 0, sizeof(st->stats.data.accum));
				}
			}
		}
		NBMngrProcess_unlock(obj);
	}
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
void NBMngrProcess_getThreadStatsDataByFirstKwnonFuncName(const char* funcName, STNBMngrProcessStatsData* dst, const BOOL includeLocksByMethod, const BOOL resetAccum){
	STNBMngrProcess* obj = __nbMngrThreads;
	if(dst != NULL){
		NBMngrProcess_lock(obj);
		{
			SI32 i; for(i = 0; i < obj->threads.use; i++){
				STNBMngrProcessThread* t2 = obj->threads.arr[i];
				if(NBMngrProcess_strIsEqual_(t2->firstKwnonCall.func, funcName)){
					NBMngrProcessStatsData_add(dst, &t2->stats.data, includeLocksByMethod);
					if(resetAccum){
						NBMngrProcessStatsState_release(&t2->stats.data.accum);
						memset(&t2->stats.data.accum, 0, sizeof(t2->stats.data.accum));
					}
				}
			}
		}
		NBMngrProcess_unlock(obj);
	}
}
#endif

//thread

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
UI64 NBMngrProcess_getCurThreadId_(const char* fullpath, const SI32 line, const char* func){
	UI64 r = 0;
	STNBMngrProcessThread* st = NBStatePerThread_get(fullpath, line, func, TRUE);
	if(st != NULL){
		r = (UI64)st;
	}
	return r;
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBMngrProcess_setCurThreadAsExplicit_(const char* fullpath, const SI32 line, const char* func){
	STNBMngrProcessThread* st = NBStatePerThread_get(fullpath, line, func, TRUE);
	if(st != NULL){
		st->isExplicit = TRUE;
	}
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
UI64 NBMngrProcess_mutexCreated(const char* name, const char* fullpath, const SI32 line, const char* func){
	UI64 r = 0;
	STNBMngrProcess* obj = __nbMngrThreads;
	NBMngrProcess_lock(obj);
	{
		//binary-search and add
		const UI64 lockId = r = ++obj->locks.seqUIDs;
		BOOL fnd = FALSE;
		SI32 posStart = 0;
		if(obj->locks.use > 0){		
			const STNBMngrProcessMutex* dataMidd = NULL;
			SI32 posMidd = 0, posEnd = (obj->locks.use - 1);
			while(posStart <= posEnd){
				posMidd		= posStart + ((posEnd - posStart) / 2);
				dataMidd	= &obj->locks.arr[posMidd];
				NB_STATIC_ASSERT((posStart == posMidd || obj->locks.arr[posStart].uid < dataMidd->uid) && (posMidd == posEnd || dataMidd->uid < obj->locks.arr[posEnd].uid))
				if(dataMidd->uid == lockId){
					NB_STATIC_ASSERT(FALSE) //lock uid registered twice
					fnd = TRUE;
					break;
				} else if(lockId < dataMidd->uid){
					posEnd		= posMidd - 1;
				} else {
					posStart	= posMidd + 1;
				}
			}
		}
		//Add
		NB_STATIC_ASSERT(!fnd)
		if(!fnd){
			NB_STATIC_ASSERT(posStart >= 0 && posStart <= obj->locks.use)
			//Validate order
#			ifdef NB_CONFIG_INCLUDE_ASSERTS
			if(posStart < obj->locks.use){ 
				const STNBMngrProcessMutex* itm1 = &obj->locks.arr[posStart];
				if(lockId >= itm1->uid){
					PRINTF_ERROR("NBMngrProcess, !(%llu < %llu).\n", lockId, itm1->uid);
				}
				NB_STATIC_ASSERT(lockId > 0 && lockId <= obj->locks.seqUIDs)
				NB_STATIC_ASSERT(itm1->uid > 0 && itm1->uid <= obj->locks.seqUIDs)
				NB_STATIC_ASSERT(lockId < itm1->uid)
			}
			if(posStart > 0){
				const STNBMngrProcessMutex* itm0 = &obj->locks.arr[posStart - 1];
				if(itm0->uid >= lockId){
					PRINTF_ERROR("NBMngrProcess, !(%llu < %llu).\n", itm0->uid, lockId);
				}
				NB_STATIC_ASSERT(lockId > 0 && lockId <= obj->locks.seqUIDs)
				NB_STATIC_ASSERT(itm0->uid > 0 && itm0->uid <= obj->locks.seqUIDs)
				NB_STATIC_ASSERT(itm0->uid < lockId)
			}
#			endif
			//Increase array
			NB_STATIC_ASSERT(obj->locks.use <= obj->locks.size)
			if(obj->locks.use >= obj->locks.size){
				obj->locks.size += 64; 
				{
					STNBMngrProcessMutex* arrN = (STNBMngrProcessMutex*)NBMemory_allocUnmanaged(sizeof(STNBMngrProcessMutex) * obj->locks.size);
					if(obj->locks.arr != NULL){
						if(obj->locks.use > 0){
							memcpy(arrN, obj->locks.arr, sizeof(STNBMngrProcessMutex) * obj->locks.use);
						}
						NBMemory_freeUnmanaged(obj->locks.arr);
						obj->locks.arr = NULL;
					}
					obj->locks.arr = arrN;
				}
			}
			//Insert itm
			{
				STNBMngrProcessMutex* itm = &obj->locks.arr[posStart]; 
				//open gap
				{
					SI32 i; for(i = (SI32)obj->locks.use - 1; i >= posStart; i--){
						obj->locks.arr[i + 1] = obj->locks.arr[i];
					}
				}
				NBMngrProcessMutex_init(itm);
				NBMngrProcessMutex_setName(itm, name);
				NBMngrProcessMutex_setFirstKnownFunc(itm, fullpath, line, func);
				obj->locks.arr[posStart].uid = lockId;
				obj->locks.use++;
#				ifdef NB_CONFIG_INCLUDE_ASSERTS
				//Validate order
				if(posStart > 0){
					const STNBMngrProcessMutex* itm0 = &obj->locks.arr[posStart - 1]; 
					const STNBMngrProcessMutex* itm1 = &obj->locks.arr[posStart];
					if(itm0->uid >= itm1->uid){
						PRINTF_ERROR("NBMngrProcess, !(%llu < %llu).\n", itm0->uid, itm1->uid);
					}
					NB_STATIC_ASSERT(itm0->uid > 0 && itm0->uid <= obj->locks.seqUIDs)
					NB_STATIC_ASSERT(itm1->uid > 0 && itm1->uid <= obj->locks.seqUIDs)
					NB_STATIC_ASSERT(itm0->uid < itm1->uid)
				}
				if((posStart + 1) < obj->locks.use){
					const STNBMngrProcessMutex* itm0 = &obj->locks.arr[posStart]; 
					const STNBMngrProcessMutex* itm1 = &obj->locks.arr[posStart + 1];
					if(itm0->uid >= itm1->uid){
						PRINTF_ERROR("NBMngrProcess, !(%llu < %llu).\n", itm0->uid, itm1->uid);
					}
					NB_STATIC_ASSERT(itm0->uid > 0 && itm0->uid <= obj->locks.seqUIDs)
					NB_STATIC_ASSERT(itm1->uid > 0 && itm1->uid <= obj->locks.seqUIDs)
					NB_STATIC_ASSERT(itm0->uid < itm1->uid)
				}
#				endif
			}
		}
		//Validate order
#		ifdef NB_CONFIG_INCLUDE_ASSERTS
		if(obj->locks.use > 1){
			const STNBMngrProcessMutex* itmPrev = &obj->locks.arr[0];
			NB_STATIC_ASSERT(itmPrev->uid > 0 && itmPrev->uid <= obj->locks.seqUIDs)
			UI32 i; for(i = 1; i < obj->locks.use; i++){
				const STNBMngrProcessMutex* itm = &obj->locks.arr[i];
				NB_STATIC_ASSERT(itmPrev->uid < itm->uid)
				NB_STATIC_ASSERT(itm->uid > 0 && itm->uid <= obj->locks.seqUIDs)
				itmPrev = itm;
			}
		}
#		endif
	}
	NBMngrProcess_unlock(obj);
	return r;
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBMngrProcess_mutexDestroyed(const UI64 lockId){
	NB_STATIC_ASSERT(lockId != 0)
	STNBMngrProcess* obj = __nbMngrThreads;
	NBMngrProcess_lock(obj);
	{
		//binary-search and remove
		BOOL fnd = FALSE;
		if(obj->locks.use > 0){		
			STNBMngrProcessMutex* dataMidd = NULL;
			SI32 posStart = 0, posMidd = 0, posEnd = (obj->locks.use - 1);
			while(posStart <= posEnd){
				posMidd		= posStart + ((posEnd - posStart) / 2);
				dataMidd	= &obj->locks.arr[posMidd];
				NB_STATIC_ASSERT((posStart == posMidd || obj->locks.arr[posStart].uid < dataMidd->uid) && (posMidd == posEnd || dataMidd->uid < obj->locks.arr[posEnd].uid))
				if(dataMidd->uid == lockId){
					NBMngrProcessMutex_release(dataMidd);
					//Fill gap
					{
						SI32 i; for(i = posMidd + 1; i < obj->locks.use; i++){
							obj->locks.arr[i - 1] = obj->locks.arr[i];
						}
						obj->locks.use--;
					}
					fnd = TRUE;
					break;
				} else if(lockId < dataMidd->uid){
					posEnd		= posMidd - 1;
				} else {
					posStart	= posMidd + 1;
				}
			}
		}
		NB_STATIC_ASSERT(fnd) //must be found
		//Validate order
#		ifdef NB_CONFIG_INCLUDE_ASSERTS
		if(obj->locks.use > 1){
			const STNBMngrProcessMutex* itmPrev = &obj->locks.arr[0];
			NB_STATIC_ASSERT(itmPrev->uid >= 0 && itmPrev->uid <= obj->locks.seqUIDs)
			UI32 i; for(i = 1; i < obj->locks.use; i++){
				const STNBMngrProcessMutex* itm = &obj->locks.arr[i];
				NB_STATIC_ASSERT(itmPrev->uid < itm->uid)
				NB_STATIC_ASSERT(itm->uid > 0 && itm->uid <= obj->locks.seqUIDs)
				itmPrev = itm;
			}
		}
#		endif
	}
	NBMngrProcess_unlock(obj);
}
#endif

//-----------------
//- Threads records
//-----------------

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBMngrProcess_threadAdd_(STNBMngrProcess* obj, STNBMngrProcessThread* t){
    NBMngrProcess_lock(obj);
    {
        //Validate non-duplicated
        SI32 i; for(i = 0; i < obj->threads.use; i++){
            const STNBMngrProcessThread* t2 = obj->threads.arr[i];
            NB_STATIC_ASSERT(t2 != t); //If fails, the thread attempted to add twice a record.
        }
        //Add
        NB_STATIC_ASSERT(obj->threads.use <= obj->threads.size)
        if(obj->threads.use >= obj->threads.size){
            STNBMngrProcessThread** arrN;
            obj->threads.size += 64;
            arrN = NBMemory_allocUnmanaged(sizeof(STNBMngrProcessThread*) * obj->threads.size);
            if(obj->threads.arr != NULL){
                if(obj->threads.use > 0){
                    memcpy(arrN, obj->threads.arr, sizeof(STNBMngrProcessThread*) * obj->threads.use);
                }
                NBMemory_freeUnmanaged(obj->threads.arr);
            }
            obj->threads.arr = arrN;
        }
        obj->threads.arr[obj->threads.use++] = t;
        //PRINTF_INFO("NBMngrProcess_threadStateAdd_ stackSize(%d).\n", obj->threads.use);
    }
    NBMngrProcess_unlock(obj);
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBMngrProcess_threadRemove_(STNBMngrProcess* obj, STNBMngrProcessThread* t){
	NBMngrProcess_lock(obj);
	{
		BOOL fnd = FALSE;
		SI32 i; for(i = 0; i < obj->threads.use; i++){
			const STNBMngrProcessThread* t2 = obj->threads.arr[i];
			if(t2 == t){
				//remove
				{
					SI32 i2; for(i2 = (i + 1); i2 < obj->threads.use; i2++){
						obj->threads.arr[i2 - 1] = obj->threads.arr[i2];
					}
					obj->threads.use--;
				}
				fnd = TRUE;
				break;
			}
		} NB_STATIC_ASSERT(fnd) //Must be found
		//Broadcast
		if(fnd){
#           ifdef CONFIG_NB_INCLUDE_THREADS_STORAGE_CLEANUP
            NBMngrProcessThread_storageDestroyAllLocked_(t);
#           endif
			if(obj->threads.waitingForAll){
				NBMngrProcess_broadcast(obj);
			}
		}
		//PRINTF_INFO("NBMngrProcess_threadRemove_ stackSize(%d).\n", obj->threads.use);
	}
	NBMngrProcess_unlock(obj);
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBStatePerThreadCreateMthd_(void){
#   ifdef _WIN32
    {
        NBASSERT(_statePerThread.tlsIdx == 0xFFFFFFFF)
        _statePerThread.tlsIdx = TlsAlloc();
        if(TLS_OUT_OF_INDEXES == _statePerThread.tlsIdx){
            PRINTF_ERROR("ERROR, NBStatePerThreadCreateMthd_, TlsAlloc failed.\n");
            _statePerThread.tlsIdx = 0xFFFFFFFF;
        } else {
            NBASSERT(NULL == TlsGetValue(_statePerThread.tlsIdx))
        }
    }
#   else
    {
        if(0 != pthread_key_create(&_statePerThread.key, NBStatePerThreadDestroyDataMthd_)){
            PRINTF_ERROR("ERROR, NBStatePerThreadCreateMthd_, pthread_key_create failed.\n");
        } else {
            NBASSERT(NULL == pthread_getspecific(_statePerThread.key))
        }
    }
#   endif
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
struct STNBMngrProcessThread_* NBStatePerThread_get(const char* fullpath, const SI32 line, const char* func, const BOOL createIfNecesary){
    struct STNBMngrProcessThread_* r = NULL;
#   ifdef _WIN32
    {
        if (_statePerThread.tlsIdx == 0xFFFFFFFF) {
            NBStatePerThreadCreateMthd_();
        }
        if (_statePerThread.tlsIdx != 0xFFFFFFFF) {
            r = (struct STNBMngrProcessThread_*)TlsGetValue(_statePerThread.tlsIdx);
            if(r == NULL && createIfNecesary){
                r = (struct STNBMngrProcessThread_*)NBMemory_allocUnmanaged(sizeof(struct STNBMngrProcessThread_));
                if(r != NULL){
                    NBMngrProcessThread_init(r);
                    NBMngrProcessThread_setFirstKnownFunc(r, fullpath, line, func);
                    if(!TlsSetValue(_statePerThread.tlsIdx, r)){
                        //ToDo: free(data) and return FALSE
                        PRINTF_ERROR("ERROR, NBStatePerThread_get, TlsSetValue failed.\n");
                    } else {
                        //Add thread to global list
                        NBMngrProcess_threadAdd_(__nbMngrThreads, r);
                    }
                }
            }
        }
    }
#   else
    {
        pthread_once(&_statePerThread.once, NBStatePerThreadCreateMthd_);
        r = (struct STNBMngrProcessThread_*)pthread_getspecific(_statePerThread.key);
        if(r == NULL && createIfNecesary){
            r = (struct STNBMngrProcessThread_*)NBMemory_allocUnmanaged(sizeof(struct STNBMngrProcessThread_));
            if(r != NULL){
                NBMngrProcessThread_init(r);
                NBMngrProcessThread_setFirstKnownFunc(r, fullpath, line, func);
                if(0 != pthread_setspecific(_statePerThread.key, r)){
                    //ToDo: free(data) and return FALSE
                    PRINTF_ERROR("ERROR, NBStatePerThread_get, pthread_setspecific failed.\n");
                } else {
                    //Add thread to global list
                    NBMngrProcess_threadAdd_(__nbMngrThreads, r);
                }
            }
        }
    }
#   endif
    return r;
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBStatePerThreadDestroyDataMthd_(void* data){ //automtically called in some systems.
    if(data != NULL){
        STNBMngrProcessThread* st = (STNBMngrProcessThread*)data;
        {
            NBMngrProcess_threadRemove_(__nbMngrThreads, st);
        }
        NBMngrProcessThread_release(st);
        NBMemory_freeUnmanaged(st);
        st = NULL;
    }
}
#endif

//-----------------
//- Mutex records
//-----------------

#ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION
STNBMngrProcessMutex* NBMngrProcess_getMutexLocked_(STNBMngrProcess* obj, const UI64 lockId){
	return NBMngrProcess_getMutexFromArr_(obj->locks.arr, obj->locks.use, lockId, obj->locks.seqUIDs);
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION
STNBMngrProcessMutex* NBMngrProcess_getMutexFromArr_(STNBMngrProcessMutex* arr, const UI32 arrSz, const UI64 lockId, const UI64 maxUID){
	STNBMngrProcessMutex* r = NULL;
	NB_STATIC_ASSERT(lockId != 0)
	{
		//binary-search
		if(arrSz > 0){		
			STNBMngrProcessMutex* dataMidd = NULL;
			SI32 posStart = 0, posMidd = 0, posEnd = (arrSz - 1);
			while(posStart <= posEnd){
				posMidd		= posStart + ((posEnd - posStart) / 2);
				dataMidd	= &arr[posMidd];
				NB_STATIC_ASSERT((posStart == posMidd || arr[posStart].uid < dataMidd->uid) && (posMidd == posEnd || dataMidd->uid < arr[posEnd].uid))
				if(dataMidd->uid == lockId){
					r = dataMidd;
					break;
				} else if(lockId < dataMidd->uid){
					posEnd		= posMidd - 1;
				} else {
					posStart	= posMidd + 1;
				}
			}
		}
		//Validate order
#		ifdef NB_CONFIG_INCLUDE_ASSERTS
		if(arrSz > 1){
			const STNBMngrProcessMutex* itmPrev = &arr[0];
			NB_STATIC_ASSERT(itmPrev->uid > 0 && itmPrev->uid <= maxUID)
			UI32 i; for(i = 1; i < arrSz; i++){
				const STNBMngrProcessMutex* itm = &arr[i];
				NB_STATIC_ASSERT(itmPrev->uid < itm->uid)
				NB_STATIC_ASSERT(itm->uid > 0 && itm->uid <= maxUID)
				itmPrev = itm;
			}
		}
#		endif
	}
	NB_STATIC_ASSERT(r != NULL) //should be found (usr logic error)
	return r;
}
#endif

//-----------------
//- Conds records
//-----------------

#ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION
STNBMngrProcessCond* NBMngrProcess_getCondLocked_(STNBMngrProcess* obj, const UI64 condId){
	return NBMngrProcess_getCondFromArr_(obj->conds.arr, obj->conds.use, condId, obj->conds.seqUIDs);
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION
STNBMngrProcessCond* NBMngrProcess_getCondFromArr_(STNBMngrProcessCond* arr, const UI32 arrSz, const UI64 condId, const UI64 maxUID){
	STNBMngrProcessCond* r = NULL;
	NB_STATIC_ASSERT(condId != 0)
	{
		//binary-search
		if(arrSz > 0){		
			STNBMngrProcessCond* dataMidd = NULL;
			SI32 posStart = 0, posMidd = 0, posEnd = (arrSz - 1);
			while(posStart <= posEnd){
				posMidd		= posStart + ((posEnd - posStart) / 2);
				dataMidd	= &arr[posMidd];
				NB_STATIC_ASSERT((posStart == posMidd || arr[posStart].uid < dataMidd->uid) && (posMidd == posEnd || dataMidd->uid < arr[posEnd].uid))
				if(dataMidd->uid == condId){
					r = dataMidd;
					break;
				} else if(condId < dataMidd->uid){
					posEnd		= posMidd - 1;
				} else {
					posStart	= posMidd + 1;
				}
			}
		}
		//Validate order
#		ifdef NB_CONFIG_INCLUDE_ASSERTS
		if(arrSz > 1){
			const STNBMngrProcessCond* itmPrev = &arr[0];
			NB_STATIC_ASSERT(itmPrev->uid > 0 && itmPrev->uid <= maxUID)
			UI32 i; for(i = 1; i < arrSz; i++){
				const STNBMngrProcessCond* itm = &arr[i];
				NB_STATIC_ASSERT(itmPrev->uid < itm->uid)
				NB_STATIC_ASSERT(itm->uid > 0 && itm->uid <= maxUID)
				itmPrev = itm;
			}
		}
#		endif
	}
	NB_STATIC_ASSERT(r != NULL)
	return r;
}
#endif

//-------
//- Tools
//-------

const char* NBMngrProcess_fileNameOnly_(const char* fullpath){
	const char* r = NULL;
	if(fullpath != NULL){
		r = &fullpath[NBMngrProcess_startOfFileName_(fullpath)];
	}
	return r;
}

UI32 NBMngrProcess_startOfFileName_(const char* fullpath){
	UI32 r = 0;
	if(fullpath != NULL){
		UI32 iPos = 0;
		while(fullpath[iPos] != '\0'){
			if(fullpath[iPos] == '/' || fullpath[iPos] == '\\'){
				r = iPos + 1;
			}
			iPos++;
		}
	}
	return r;
}

char* NBMngrProcess_strClone_(const char* string){
	char* r = NULL;
	const UI32 len = NBMngrProcess_strLenBytes_(string);
	r = NBMemory_allocUnmanaged(len + 1);
	memcpy(r, string, len);
	r[len] = '\0';
	return r;
}
	
UI32 NBMngrProcess_strLenBytes_(const char* string){
	if(string == NULL) return 0;
	const char* ptr = string;
	while(*ptr != '\0') ptr++;
	return (UI32)(ptr - string);
}

BOOL NBMngrProcess_strIsEmpty_(const char* str){
	return (str == NULL || str[0] == '\0');
}

BOOL NBMngrProcess_strIsEqual_(const char* str1, const char* str2){
	if(str1 == str2) return TRUE; //both can be NULL or the same pointer
	if((str1 == NULL && str2 != NULL) || (str1 != NULL && str2 == NULL)) return FALSE;
	//PRINTF_INFO("Comparando cadenas: '%s' vs '%s'\n", str1, str2);
	while(*str1 != '\0'){
		if(*str1 != *str2) return FALSE;
		str1++; str2++;
	}
	return (*str1 == *str2); //both should end in '\0'
}

BOOL NBMngrProcess_strIsLower_(const char* str1, const char* str2){
	while(*str1 != 0 && *str2 != 0){
		if(*str1 < *str2){
			return TRUE;
		} else if(*str1 > *str2){
			return FALSE;
		}
		str1++; str2++;
	}
	return (*str1 < *str2);
}

BOOL NBMngrProcess_strIsLowerOrEqual_(const char* str1, const char* str2){
	while(*str1 != 0 && *str2 != 0){
		if(*str1 < *str2){
			return TRUE;
		} else if(*str1 > *str2){
			return FALSE;
		}
		str1++; str2++;
	}
	return (*str1 <= *str2);
}

BOOL NBMngrProcess_strIsGreater_(const char* str1, const char* str2){
	while(*str1 != 0 && *str2 != 0){
		if(*str1 > *str2){
			return TRUE;
		} else if(*str1 < *str2){
			return FALSE;
		}
		str1++; str2++;
	}
	return (*str1 > *str2);
}

BOOL NBMngrProcess_strIsGreaterOrEqual_(const char* str1, const char* str2){
	while(*str1 != 0 && *str2 != 0){
		if(*str1 > *str2){
			return TRUE;
		} else if(*str1 < *str2){
			return FALSE;
		}
		str1++; str2++;
	}
	return (*str1 >= *str2);
}



//sorted

SI32 NBMngrProcess_sortedIndexOf_(void** buff, SI32* buffUse, SI32* buffSz, const SI32 bytesPerItem, NBCompareFunc cmpFunc, const void* data, const SI32 dataSz){
	NB_STATIC_ASSERT(dataSz == bytesPerItem)
	if(*buffUse > 0){
		SI32 posEnd		= (*buffUse - 1);
		//Binary Search
		{
			SI32 posStart	= 0;
			SI32 posMidd;
			const BYTE* dataMidd = NULL;
			NB_STATIC_ASSERT(bytesPerItem > 0)
			while(posStart <= posEnd){
				posMidd		= posStart + ((posEnd - posStart)/2);
				dataMidd	= &((BYTE*) *buff)[posMidd * bytesPerItem];
				if((*cmpFunc)(ENCompareMode_Equal, dataMidd, data, bytesPerItem)){
					return posMidd;
				} else {
					if((*cmpFunc)(ENCompareMode_Lower, data, dataMidd, bytesPerItem)){
						posEnd		= posMidd - 1;
					} else {
						posStart	= posMidd + 1;
					}
				}
			}
		}
	}
	return -1;
}

SI32 NBMngrProcess_sortedIndexForNew_(void** buff, SI32* buffUse, SI32* buffSz, const SI32 bytesPerItem, NBCompareFunc cmpFunc, const void* dataToInsert){
	//Binary search for new item's position
	SI32 r = -1;
	NB_STATIC_ASSERT(cmpFunc != NULL)
	NB_STATIC_ASSERT(bytesPerItem > 0)
	if(cmpFunc != NULL){
		r = 0;
		if(*buffUse > 0){
			SI32 posStart		= 0;
			SI32 posEnd			= (*buffUse - 1);
			do {
				if((*cmpFunc)(ENCompareMode_LowerOrEqual, &((BYTE*)*buff)[posEnd * bytesPerItem], dataToInsert, bytesPerItem)) {
					r			= posEnd + 1;
					break;
				} else if((*cmpFunc)(ENCompareMode_GreaterOrEqual, &((BYTE*)*buff)[posStart * bytesPerItem], dataToInsert, bytesPerItem)) {
					r			= posStart;
					break;
				} else {
					const UI32 posMidd = (posStart + posEnd) / 2;
					if((*cmpFunc)(ENCompareMode_LowerOrEqual, &((BYTE*)*buff)[posMidd * bytesPerItem], dataToInsert, bytesPerItem)) {
						posStart = posMidd + 1;
					} else {
						posEnd	= posMidd;
					}
				}
			} while(1);
		}
	}
	return r;
}

void NBMngrProcess_sortedGrowBuffer_(void** buff, SI32* buffUse, SI32* buffSz, const SI32 bytesPerItem, const SI32 qItems){
	if(qItems > 0){
		BYTE* newArr = NULL;
		*buffSz = *buffSz + qItems;
		newArr = (BYTE*)NBMemory_allocUnmanaged((*buffSz) * bytesPerItem);
		if((*buff) != NULL){
			memcpy(newArr, (*buff), (*buffUse * bytesPerItem));
			NBMemory_freeUnmanaged((*buff));
		}
		(*buff) = newArr;
	}
}

void* NBMngrProcess_sortedAdd_(void** buff, SI32* buffUse, SI32* buffSz, const SI32 bytesPerItem, NBCompareFunc cmpFunc, const void* data, const SI32 itemSize){
	void* r = NULL;
	NB_STATIC_ASSERT(itemSize == bytesPerItem)
	if(itemSize == bytesPerItem){
		//Grow, if necesary
		NB_STATIC_ASSERT(*buffUse <= *buffSz)
		if(*buffUse == *buffSz){
			NBMngrProcess_sortedGrowBuffer_(buff, buffUse, buffSz, bytesPerItem, 8);
			NB_STATIC_ASSERT(*buffUse < *buffSz)
		}
		//Add
		const SI32 dstIndex = NBMngrProcess_sortedIndexForNew_(buff, buffUse, buffSz, bytesPerItem, cmpFunc, (BYTE*)data); NB_STATIC_ASSERT(dstIndex >= 0 && dstIndex <= *buffUse)
		//PRINTF_INFO("Adding to index %d of %d.\n", dstIndex, *buffUse);
		//Make room for the new record
		if(dstIndex < *buffUse){
			SI32 i; for(i = (*buffUse - 1); i >= dstIndex; i--){
				memcpy(&((BYTE*)*buff)[(i + 1) * bytesPerItem], &((BYTE*)*buff)[i * bytesPerItem], bytesPerItem);
			}
		}
		//Set record value
		memcpy(&((BYTE*)*buff)[dstIndex * bytesPerItem], data, bytesPerItem);
		*buffUse = *buffUse + 1;
		r = &((BYTE*)*buff)[dstIndex * bytesPerItem];
	}
	return r;
}

//Unlocked methods (safe to use 'NBMemory' direct or indirectly)

#include "nb/core/NBString.h"

#ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION
void NBMngrProcess_mutexStackConcat(void* pDst /*STNBString*/, const char* itmSeparator){
	if(pDst != NULL){
		STNBMngrProcess* obj = __nbMngrThreads;
		char* strTmp = NULL;
		UI32 strTmpUse = 0, strTmpSz = 0;
		NBMngrProcess_lock(obj);
		{
			STNBMngrProcessThread* st = NBStatePerThread_get(__FILE__, __LINE__, __func__, FALSE);
			if(st != NULL){
				//calculate length
				{
					const UI32 sepLen = NBMngrProcess_strLenBytes_(itmSeparator);
					SI32 i; for(i = ((SI32)st->locks.use - 1); i >= 0; i--){
						const STNBMngrProcessLockPos* t = &st->locks.stack[i];
						strTmpSz += 1 + 16 + 1 + 16 + 1;	// "(" + i + "/" + use + ")"
						strTmpSz += 10; // " function "
						strTmpSz += NBMngrProcess_strLenBytes_(t->codePos.func);
						strTmpSz += 7; // ", file "
						strTmpSz += NBMngrProcess_strLenBytes_(NBMngrProcess_fileNameOnly_(t->codePos.fullpath));
						strTmpSz += 7 + 16; // ", line " + line
						strTmpSz += sepLen;
					}
					strTmpSz++;
				}
				//allocate string
				{
					strTmp		= (char*)NBMemory_allocUnmanaged(strTmpSz);
					strTmp[0]	= '\0';
				}
				//concat
				{
					char* strPos = strTmp;
					const char* strSep = (NBMngrProcess_strIsEmpty_(itmSeparator) ? "" : itmSeparator);
					SI32 i; for(i = ((SI32)st->locks.use - 1); i >= 0; i--){
						const STNBMngrProcessLockPos* t = &st->locks.stack[i];
						//function %s, file %s, line %d
						sprintf(strPos, "(%d/%d) function %s, file %s, line %d%s", (i + 1), st->locks.use, t->codePos.func, NBMngrProcess_fileNameOnly_(t->codePos.fullpath), t->codePos.line, strSep);
						//detect new pos
						while(*strPos != '\0'){
							strPos++;
						}
					}
					strTmpUse = (UI32)(strPos - strTmp);
				}
			}
		}
		NBMngrProcess_unlock(obj);
		//concat (avoid using 'NBMemory' while locked)
		{
			STNBString* dst = (STNBString*)pDst;
			NBString_concatBytes(dst, strTmp, strTmpUse);
		}
		//Release
		if(strTmp != NULL){
			NBMemory_freeUnmanaged(strTmp);
			strTmp = NULL;
		}
		strTmpSz = 0;
	}
}
#endif

//per-thread-storages (STNBThreadStorage)
#ifdef CONFIG_NB_INCLUDE_THREADS_STORAGE_CLEANUP
void NBMngrProcess_storageAddCreated(void* data /*STNBThreadStorageData*/){
    STNBMngrProcess* obj = __nbMngrThreads;
    NBMngrProcess_lock(obj);
    {
        STNBMngrProcessThread* st = NBStatePerThread_get(__FILE__, __LINE__, __func__, FALSE);
        if(st != NULL){
            //resize array
            while(st->storages.use >= st->storages.size){
                const SI32 sizeN = (st->storages.use + 8);
                void** arrN = (void**)NBMemory_allocUnmanaged(sizeof(st->storages.arr[0]) * sizeN);
                if(arrN == NULL){
                    break;
                } else {
                    if(st->storages.arr != NULL){
                        if(st->storages.use > 0){
                            NBMemory_copy(arrN, st->storages.arr, sizeof(st->storages.arr[0]) * st->storages.use);
                        }
                        free(st->storages.arr);
                        st->storages.arr = NULL;
                    }
                    st->storages.arr = arrN;
                    st->storages.size = sizeN;
                }
            }
            //add
            if(st->storages.use < st->storages.size){
                st->storages.arr[st->storages.use++] = data;
            }
        }
    }
    NBMngrProcess_unlock(obj);
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_STORAGE_CLEANUP
void NBMngrProcess_storageRemoveDestroyedLocked_(STNBMngrProcess* obj, void* data /*STNBThreadStorageData*/){
    STNBMngrProcessThread* st = NBStatePerThread_get(__FILE__, __LINE__, __func__, FALSE);
    if(st != NULL){
        SI32 i; for(i = 0; i < st->storages.use; i++){
            void* d = st->storages.arr[i];
            if(d == data){
                st->storages.use--;
                //fill gap
                for(; i < st->storages.use; i++){
                    st->storages.arr[i] = st->storages.arr[i + 1];
                }
                break;
            }
        }
    }
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_STORAGE_CLEANUP
void NBMngrProcess_storageRemoveDestroyed(void* data /*STNBThreadStorageData*/){
    STNBMngrProcess* obj = __nbMngrThreads;
    NBMngrProcess_lock(obj);
    {
        NBMngrProcess_storageRemoveDestroyedLocked_(obj, data);
    }
    NBMngrProcess_unlock(obj);
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_STORAGE_CLEANUP
void NBMngrProcess_storageDestroyAll(void){ //at current thread
    STNBMngrProcess* obj = __nbMngrThreads;
    NBMngrProcess_lock(obj);
    {
        STNBMngrProcessThread* st = NBStatePerThread_get(__FILE__, __LINE__, __func__, FALSE);
        if(st != NULL){
            NBMngrProcessThread_storageDestroyAllLocked_(st);
        }
    }
    NBMngrProcess_unlock(obj);
}
#endif

void NBMngrProcess_concatFormatedBytes_(const UI64 bytesCount, STNBString* dst){
	if(dst != NULL){
		if(bytesCount >= (1024ULL * 1024ULL * 1024ULL * 1024ULL* 1024ULL)){
			const UI64 div = (1024ULL * 1024ULL * 1024ULL * 1024ULL * 1024ULL);
			const double p0 = (double)(bytesCount / div); 
			const double p1 = (double)(bytesCount % div) / (double)div;
			NBString_concatDouble(dst, p0 + p1, 1);
			NBString_concat(dst, "PB");
		} else if(bytesCount >= (1024ULL * 1024ULL * 1024ULL * 1024ULL)){
			const UI64 div = (1024ULL * 1024ULL * 1024ULL * 1024ULL);
			const double p0 = (double)(bytesCount / div); 
			const double p1 = (double)(bytesCount % div) / (double)div;
			NBString_concatDouble(dst, p0 + p1, 1);
			NBString_concat(dst, "TB");
		} else if(bytesCount >= (1024ULL * 1024ULL * 1024ULL)){
			const UI64 div = (1024ULL * 1024ULL * 1024ULL);
			const double p0 = (double)(bytesCount / div); 
			const double p1 = (double)(bytesCount % div) / (double)div;
			NBString_concatDouble(dst, p0 + p1, 1);
			NBString_concat(dst, "GB");
		} else if(bytesCount >= (1024ULL * 1024ULL * 1024ULL)){
			const UI64 div = (1024ULL * 1024ULL * 1024ULL);
			const double p0 = (double)(bytesCount / div); 
			const double p1 = (double)(bytesCount % div) / (double)div;
			NBString_concatDouble(dst, p0 + p1, 1);
			NBString_concat(dst, "GB");
		} else if(bytesCount >= (1024ULL * 1024ULL)){
			const UI64 div = (1024ULL * 1024ULL);
			const double p0 = (double)(bytesCount / div); 
			const double p1 = (double)(bytesCount % div) / (double)div;
			NBString_concatDouble(dst, p0 + p1, 1);
			NBString_concat(dst, "MB");
		} else if(bytesCount >= (1024ULL)){
			const UI64 div = (1024ULL);
			const double p0 = (double)(bytesCount / div); 
			const double p1 = (double)(bytesCount % div) / (double)div;
			NBString_concatDouble(dst, p0 + p1, 1);
			NBString_concat(dst, "KB");
		} else {
			NBString_concatUI64(dst, bytesCount);
			NBString_concat(dst, "B");
		}
	}
}

#ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
void NBMngrProcess_concatStatsData(const STNBMngrProcessStatsData* obj, const ENNBLogLevel logLvl, const BOOL loaded, const BOOL accum, const BOOL total, const char* prefixFirst, const char* prefixOthers, const BOOL ignoreEmpties, void* dstString){
	STNBString* dst = (STNBString*)dstString;
	if(dst != NULL){
		BOOL opened = FALSE;
		const char* preExtra = "        |- ";
		STNBString str, pre;
		NBString_initWithSz(&str, 0, 256, 0.1f);
		NBString_initWithSz(&pre, NBString_strLenBytes(prefixOthers) + NBString_strLenBytes(preExtra) + 1, 256, 0.10f);
		NBString_concat(&pre, prefixOthers);
		NBString_concat(&pre, preExtra);
		if(loaded){
			NBString_empty(&str);
			NBMngrProcess_concatStatsState(&obj->alive, logLvl, "", pre.str, ignoreEmpties, &str);
			if(str.length > 0){
				if(opened){
					NBString_concat(dst, "\n");
					NBString_concat(dst, prefixOthers);
				} else {
					NBString_concat(dst, prefixFirst);
				}
				NBString_concat(dst,  "alive:  ");
				NBString_concat(dst, str.str);
				opened = TRUE;
			}
		}
		if(accum){
			NBString_empty(&str);
			NBMngrProcess_concatStatsState(&obj->accum, logLvl, "", pre.str, ignoreEmpties, &str);
			if(str.length > 0){
				if(opened){
					NBString_concat(dst, "\n");
					NBString_concat(dst, prefixOthers);
				} else {
					NBString_concat(dst, prefixFirst);
				}
				NBString_concat(dst,  "accum:  ");
				NBString_concat(dst, str.str);
				opened = TRUE;
			}
		}
		if(total){
			NBString_empty(&str);
			NBMngrProcess_concatStatsState(&obj->total, logLvl, "", pre.str, ignoreEmpties, &str);
			if(str.length > 0){
				if(opened){
					NBString_concat(dst, "\n");
					NBString_concat(dst, prefixOthers);
				} else {
					NBString_concat(dst, prefixFirst);
				}
				NBString_concat(dst,  "total:  ");
				NBString_concat(dst, str.str);
				opened = TRUE;
			}
		}
		NBString_release(&pre);
		NBString_release(&str);
	}
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
void NBMngrProcess_concatStatsState(const STNBMngrProcessStatsState* obj, const ENNBLogLevel logLvl, const char* prefixFirst, const char* prefixOthers, const BOOL ignoreEmpties, void* dstString){
	STNBString* dst = (STNBString*)dstString;
	if(dst != NULL){
		BOOL opened = FALSE;
		const char* preExtra = "        |- ";
		STNBString str, pre;
		NBString_initWithSz(&str, 0, 256, 0.1f);
		NBString_initWithSz(&pre, NBString_strLenBytes(prefixOthers) + NBString_strLenBytes(preExtra) + 1, 256, 0.10f);
		NBString_concat(&pre, prefixOthers);
		NBString_concat(&pre, preExtra);
		//memory
		{
			if(!ignoreEmpties || obj->mem.allocs > 0 || obj->mem.allocBytes > 0 || obj->mem.freeds > 0){
				BOOL opened2 = FALSE;
				if(opened) NBString_concat(dst, ", ");
				if(obj->mem.allocBytes > 0){
					if(opened2) NBString_concat(dst, "/");
					NBString_concat(dst, "+");
					NBMngrProcess_concatFormatedBytes_(obj->mem.allocBytes, dst);
					opened2 = TRUE;
				}
				if(obj->mem.allocs > 0){
					if(opened2) NBString_concat(dst, "/");
					NBString_concat(dst, "+");
					NBString_concatSI64(dst, obj->mem.allocs);
					opened2 = TRUE;
				}
				if(obj->mem.freeds > 0){
					NBString_concat(dst, "-");
					NBString_concatSI64(dst, obj->mem.freeds);
				}
				NBString_concat(dst, " allocs");
				opened = TRUE;
			}
		}
		//locks
		{
			if(!ignoreEmpties || obj->locks.locked > 0 || obj->locks.unlocked > 0 || obj->locks.byMethod.use > 0){
				BOOL opened2 = FALSE;
				if(opened) NBString_concat(dst, ", ");
				if(obj->locks.locked > 0){
					NBString_concat(dst, "+");
					NBString_concatSI64(dst, obj->locks.locked);
				}
				if(obj->locks.unlocked > 0){
					NBString_concat(dst, "-");
					NBString_concatSI64(dst, obj->locks.unlocked);
				}
				NBString_concat(dst, " locks");
				if(obj->locks.passby > 0){
					NBString_concat(dst, (opened2 ? ", " : " ("));
					NBString_concatSI64(dst, obj->locks.passby);
					NBString_concat(dst, " passby");
					opened2 = TRUE;
				}
				if(obj->locks.yield > 0){
					NBString_concat(dst, (opened2 ? ", " : " ("));
					NBString_concatSI64(dst, obj->locks.yield);
					NBString_concat(dst, " yield");
					opened2 = TRUE;
				}
				if(opened2){
					NBString_concat(dst, ")");
				}
				opened = TRUE;
				//byMethod
				if(obj->locks.byMethod.arr != NULL && obj->locks.byMethod.use > 0){
					SI32 i; for(i = 0; i < obj->locks.byMethod.use; i++){
						const STNBThreadsMethodLocksOps* itm = &obj->locks.byMethod.arr[i];
						BOOL opened2 = FALSE;
						if(opened){
							NBString_concat(dst, "\n");
							opened = FALSE;
						}
						//
						NBString_concat(dst, prefixOthers);
						//locks
						if(itm->locked != 0 || itm->unlocked != 0){
							NBString_concat(dst, (opened2 ? ", " : ""));
							if(itm->locked >= 0){
								NBString_concat(dst, "+");
								NBString_concatSI64(dst, itm->locked);
							} else {
								NBString_concatSI64(dst, itm->locked);
							}
							if(itm->unlocked >= 0){
								NBString_concat(dst, "-");
								NBString_concatSI64(dst, itm->unlocked);
							} else {
								NBString_concat(dst, "+");
								NBString_concatSI64(dst, -itm->unlocked);
							}
							NBString_concat(dst, " locks");
							opened2 = TRUE;
						}
						//yields
						if(itm->yielded > 0){
							NBString_concat(dst, (opened2 ? ", " : ""));
							NBString_concatSI64(dst, itm->yielded);
							NBString_concat(dst, " yields");
							opened2 = TRUE;
						}
						//method
						NBString_concat(dst, (opened2 ? ": " : ""));
						NBString_concat(dst, itm->method);
						NBString_concat(dst, "\n");
					}
				}
			}
		}
		//conds
		{
			if(!ignoreEmpties || obj->conds.waitStarted > 0 || obj->conds.waitEnded > 0){
				if(opened) NBString_concat(dst, ", ");
				if(obj->conds.waitStarted > 0){
					NBString_concat(dst, "+");
					NBString_concatUI64(dst, obj->conds.waitStarted);
				}
				if(obj->conds.waitEnded > 0){
					NBString_concat(dst, "-");
					NBString_concatUI64(dst, obj->conds.waitEnded);
				}
				NBString_concat(dst, " waits");
				opened = TRUE;
			}
			if(!ignoreEmpties || obj->conds.broadcasts > 0){
				if(opened) NBString_concat(dst, ", ");
				NBString_concatUI64(dst, obj->conds.broadcasts);
				NBString_concat(dst, " brcasts");
				opened = TRUE;
			}
			if(!ignoreEmpties || obj->conds.signals > 0){
				if(opened) NBString_concat(dst, ", ");
				NBString_concatUI64(dst, obj->conds.signals);
				NBString_concat(dst, " signls");
				opened = TRUE;
			}
		}
		//fs
		{
			NBString_empty(&str);
			NBMngrProcess_concatFilesystemStatsState(&obj->fs, logLvl, "", pre.str, ignoreEmpties, &str);
			if(str.length > 0){
				if(opened){
					NBString_concat(dst, "\n");
					NBString_concat(dst, prefixOthers);
				} else {
					NBString_concat(dst, prefixFirst);
				}
				NBString_concat(dst,  "filesyst:  ");
				NBString_concat(dst, str.str);
				opened = TRUE;
			}
		}
		NBString_release(&pre);
		NBString_release(&str);
	}
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
void NBMngrProcess_concatFilesystemStatsStateSrc_(const STNBFilesystemStatsOps* ops, const ENNBLogLevel logLvl, const char* prefixFirst, const char* prefixOthers, const BOOL ignoreEmpties, void* dstString){
	STNBString* dst = (STNBString*)dstString;
	if(dst != NULL){
		BOOL opened = FALSE;
		const char* preExtra = "        |- ";
		STNBString str, pre;
		NBString_initWithSz(&str, 0, 256, 0.1f);
		NBString_initWithSz(&pre, NBString_strLenBytes(prefixOthers) + NBString_strLenBytes(preExtra) + 1, 256, 0.10f);
		NBString_concat(&pre, prefixOthers);
		NBString_concat(&pre, preExtra);
		NB_STATIC_ASSERT(ENNBFilesystemStatsResult_Count == (sizeof(__fsStatsResultDefs) / sizeof(__fsStatsResultDefs[0])))
		{
			UI32 i; for(i = 0; i < ENNBFilesystemStatsResult_Count; i++){
				const STNBFilesystemStatsResultDef* def = &__fsStatsResultDefs[i];
				const STNBFilesystemStatsOps* op = &ops[i];
				NB_STATIC_ASSERT(def->iResult == i)
				NBString_empty(&str);
				NBMngrProcess_concatFilesystemOps(op, logLvl, "", pre.str, ignoreEmpties, &str);
				if(str.length > 0){
					if(opened){
						NBString_concat(dst, "\n");
						NBString_concat(dst, prefixOthers);
					} else {
						NBString_concat(dst, prefixFirst);
					}
					NBString_concat(dst,  def->name);
					NBString_concat(dst, ": ");
					NBString_concat(dst, str.str);
					opened = TRUE;
				}
			}
		}
		NBString_release(&pre);
		NBString_release(&str);
	}
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
void NBMngrProcess_concatFilesystemStatsState(const STNBFilesystemStatsState* obj, const ENNBLogLevel logLvl, const char* prefixFirst, const char* prefixOthers, const BOOL ignoreEmpties, void* dstString){
	STNBString* dst = (STNBString*)dstString;
	if(dst != NULL){
		BOOL opened = FALSE;
		const char* preExtra = "        |- ";
		STNBString str, pre;
		NBString_initWithSz(&str, 0, 256, 0.1f);
		NBString_initWithSz(&pre, NBString_strLenBytes(prefixOthers) + NBString_strLenBytes(preExtra) + 1, 256, 0.10f);
		NBString_concat(&pre, prefixOthers);
		NBString_concat(&pre, preExtra);
		NB_STATIC_ASSERT(ENNBFilesystemStatsSrc_Count == (sizeof(__fsStatsSrcDefs) / sizeof(__fsStatsSrcDefs[0])))
		{
			UI32 i; for(i = 0; i < ENNBFilesystemStatsSrc_Count; i++){
				const STNBFilesystemStatsSrcDef* def = &__fsStatsSrcDefs[i]; 
				const STNBFilesystemStatsOps* op = obj->ops[i];
				NB_STATIC_ASSERT(def->iSrc == i)
				NBString_empty(&str);
				NBMngrProcess_concatFilesystemStatsStateSrc_(op, logLvl, "", pre.str, ignoreEmpties, &str);
				if(str.length > 0){
					if(opened){
						NBString_concat(dst, "\n");
						NBString_concat(dst, prefixOthers);
					} else {
						NBString_concat(dst, prefixFirst);
					}
					NBString_concat(dst,  def->name);
					NBString_concat(dst, ": ");
					NBString_concat(dst, str.str);
					opened = TRUE;
				}
			}
		}
		NBString_release(&pre);
		NBString_release(&str);
	}
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
void NBMngrProcess_concatFilesystemOps(const STNBFilesystemStatsOps* obj, const ENNBLogLevel logLvl, const char* prefixFirst, const char* prefixOthers, const BOOL ignoreEmpties, void* dstString){
	STNBString* dst = (STNBString*)dstString;
	if(dst != NULL){
		BOOL opened = FALSE;
		const char* preExtra = "        |- ";
		STNBString str, pre;
		NBString_initWithSz(&str, 0, 256, 0.1f);
		NBString_initWithSz(&pre, NBString_strLenBytes(prefixOthers) + NBString_strLenBytes(preExtra) + 1, 256, 0.10f);
		NBString_concat(&pre, prefixOthers);
		NBString_concat(&pre, preExtra);
		{
			NBString_empty(&str);
			NBMngrProcess_concatFilesOps(&obj->files, logLvl, "", pre.str, ignoreEmpties, &str);
			if(str.length > 0){
				if(opened){
					NBString_concat(dst, "\n");
					NBString_concat(dst, prefixOthers);
				} else {
					NBString_concat(dst, prefixFirst);
				}
				NBString_concat(dst,  "   files:  ");
				NBString_concat(dst, str.str);
				opened = TRUE;
			}
		}
		{
			NBString_empty(&str);
			NBMngrProcess_concatFolderOps(&obj->folders, logLvl, "", pre.str, ignoreEmpties, &str);
			if(str.length > 0){
				if(opened){
					NBString_concat(dst, "\n");
					NBString_concat(dst, prefixOthers);
				} else {
					NBString_concat(dst, prefixFirst);
				}
				NBString_concat(dst,  " folders:  ");
				NBString_concat(dst, str.str);
				opened = TRUE;
			}
		}
		NBString_release(&pre);
		NBString_release(&str);
	}
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
void NBMngrProcess_concatFolderOps(const STNBFilesystemStatsFoldersOps* obj, const ENNBLogLevel logLvl, const char* prefixFirst, const char* prefixOthers, const BOOL ignoreEmpties, void* dstString){
	STNBString* dst = (STNBString*)dstString;
	if(dst != NULL){
		BOOL opened = FALSE;
		if(!ignoreEmpties || obj->create > 0){
			if(opened) NBString_concat(dst, ", ");
			NBString_concatUI64(dst, obj->create);
			NBString_concat(dst, " create");
			opened = TRUE;
		}
		if(!ignoreEmpties || obj->open > 0){
			if(opened) NBString_concat(dst, ", ");
			NBString_concatSI64(dst, obj->open);
			NBString_concat(dst, " open");
			opened = TRUE;
		}
		if(!ignoreEmpties || obj->close > 0){
			if(opened) NBString_concat(dst, ", ");
			NBString_concatSI64(dst, obj->close);
			NBString_concat(dst, " close");
			opened = TRUE;
		}
		if(!ignoreEmpties || obj->read > 0){
			if(opened) NBString_concat(dst, ", ");
			NBString_concatUI64(dst, obj->read);
			NBString_concat(dst, " read");
			opened = TRUE;
		}
		if(!ignoreEmpties || obj->delet > 0){
			if(opened) NBString_concat(dst, ", ");
			NBString_concatUI64(dst, obj->delet);
			NBString_concat(dst, " delete");
			opened = TRUE;
		}
	}
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
void NBMngrProcess_concatFilesOps(const STNBFilesystemStatsFilesOps* obj, const ENNBLogLevel logLvl, const char* prefixFirst, const char* prefixOthers, const BOOL ignoreEmpties, void* dstString){
	STNBString* dst = (STNBString*)dstString;
	if(dst != NULL){
		BOOL opened = FALSE;
		if(!ignoreEmpties || obj->open > 0){
			if(opened) NBString_concat(dst, ", ");
			NBString_concatSI64(dst, obj->open);
			NBString_concat(dst, " open");
			opened = TRUE;
		}
		if(!ignoreEmpties || obj->close > 0){
			if(opened) NBString_concat(dst, ", ");
			NBString_concatSI64(dst, obj->close);
			NBString_concat(dst, " close");
			opened = TRUE;
		}
		if(!ignoreEmpties || obj->read.count > 0){
			if(opened) NBString_concat(dst, ", ");
			NBMngrProcess_concatFormatedBytes_(obj->read.bytes, dst);
			NBString_concat(dst, "/");
			NBString_concatUI64(dst, obj->read.count);
			NBString_concat(dst, " read");
			opened = TRUE;
		}
		if(!ignoreEmpties || obj->write.count > 0){
			if(opened) NBString_concat(dst, ", ");
			NBMngrProcess_concatFormatedBytes_(obj->write.bytes, dst);
			NBString_concat(dst, "/");
			NBString_concatUI64(dst, obj->write.count);
			NBString_concat(dst, " write");
			opened = TRUE;
		}
		if(!ignoreEmpties || obj->seek > 0){
			if(opened) NBString_concat(dst, ", ");
			NBString_concatUI64(dst, obj->seek);
			NBString_concat(dst, " seek");
			opened = TRUE;
		}
		if(!ignoreEmpties || obj->tell > 0){
			if(opened) NBString_concat(dst, ", ");
			NBString_concatUI64(dst, obj->tell);
			NBString_concat(dst, " tell");
			opened = TRUE;
		}
		if(!ignoreEmpties || obj->flush > 0){
			if(opened) NBString_concat(dst, ", ");
			NBString_concatUI64(dst, obj->flush);
			NBString_concat(dst, " flush");
			opened = TRUE;
		}
		if(!ignoreEmpties || obj->stat > 0){
			if(opened) NBString_concat(dst, ", ");
			NBString_concatUI64(dst, obj->stat);
			NBString_concat(dst, " stat");
			opened = TRUE;
		}
		if(!ignoreEmpties || obj->move > 0){
			if(opened) NBString_concat(dst, ", ");
			NBString_concatUI64(dst, obj->move);
			NBString_concat(dst, " move");
			opened = TRUE;
		}
		if(!ignoreEmpties || obj->delet > 0){
			if(opened) NBString_concat(dst, ", ");
			NBString_concatUI64(dst, obj->delet);
			NBString_concat(dst, " delete");
			opened = TRUE;
		}
	}
}
#endif
