#ifndef NB_OBJECT_H
#define NB_OBJECT_H

#include "nb/NBFrameworkDefs.h"
#include "nb/NBClass.h"
#include "nb/NBContext.h"
#include "nb/NBObjectLnk.h"

#ifdef __cplusplus
extern "C" {
#endif

//----------
//- NBObject
//- Referenceable object (must be allocated at heap,
//- and can be locked and retained)
//----------

typedef struct STNBObject_ {
	const STNBClass*	cls;		//class			
	STNBContext*		ctx;		//Context
	void*				mutex;		//mutex
	UI32				refCount;	//retain count
#   ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
    UI64                objId;      //given by NBMngrProcess for dbg
#   endif
} STNBObject;

//NBObject

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
#	define NBObject_initZeroedInternal(OBJ, CLS, CTX, NAME, PATH, LINE, FUNC)	NBObject_initZeroedInternal_((OBJ), (CLS), (CTX), (NAME), (PATH), (LINE), (FUNC))
#	define NBObject_retainInternal(OBJ, PATH, LINE, FUNC)				NBObject_retainInternal_((OBJ), (PATH), (LINE), (FUNC))
#	define NBObject_releaseInternal(OBJ, PATH, LINE, FUNC)				NBObject_releaseInternal_((OBJ), (PATH), (LINE), (FUNC))
#	define NBObject_lock(OBJ)											NBObject_lock_((OBJ), __FILE__, __LINE__, __func__)
#	define NBObject_unlock(OBJ)											NBObject_unlock_((OBJ), __FILE__, __LINE__, __func__)
#else
#	define NBObject_initZeroedInternal(OBJ, CLS, CTX, NAME, PATH, LINE, FUNC)	NBObject_initZeroedInternal_((OBJ), (CLS), (CTX))
#	define NBObject_retainInternal(OBJ, PATH, LINE, FUNC)				NBObject_retainInternal_((OBJ))
#	define NBObject_releaseInternal(OBJ, PATH, LINE, FUNC)				NBObject_releaseInternal_((OBJ))
#	define NBObject_lock(OBJ)											NBObject_lock_((OBJ))
#	define NBObject_unlock(OBJ)											NBObject_unlock_((OBJ))
#endif

#define NBObject_isObject(PTR)					(((STNBObject*)(PTR))->cls != NULL && ((STNBObject*)(PTR))->cls.head == NB_CLASS_HEAD_32)

void NBObject_initZeroedRecursively(STNBObject* obj);
void NBObject_uninitLockedRecursively(STNBObject* obj);
void NBObject_initZeroed(STNBObject* obj);
void NBObject_uninitLocked(STNBObject* obj);
UI32 NBObject_retainCount(STNBObject* obj);
#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBObject_initZeroedInternal_(STNBObject* obj, const STNBClass* cls, STNBContext* ctx, const char* name, const char* fullpath, const SI32 line, const char* func);
void NBObject_retainInternal_(STNBObject* obj, const char* fullpath, const SI32 line, const char* func);
BOOL NBObject_releaseInternal_(STNBObject* obj, const char* fullpath, const SI32 line, const char* func);
void NBObject_lock_(void* obj, const char* fullpath, const SI32 line, const char* func);
void NBObject_unlock_(void* obj, const char* fullpath, const SI32 line, const char* func);
UI64 NBObject_getMutexId(void* obj);
#else
void NBObject_initZeroedInternal_(STNBObject* obj, const STNBClass* cls, STNBContext* ctx);
void NBObject_retainInternal_(STNBObject* obj);
BOOL NBObject_releaseInternal_(STNBObject* obj);
void NBObject_lock_(void* obj);
void NBObject_unlock_(void* obj);
#endif

//Class

//typedef STNBObjRef STNBObjectRef;
//STNBObjectRef NBObject_alloc(STNBContext* ctx);
BOOL NBObject_isClass(void* ptr);
UI32 NBObject_getObjSz(void);
const STNBClass* NBObject_getClass(void);

//----------
//- NBObjRef
//- Referenceable object (object is allocated at heap,
//- and can be locked and retained)
//----------

typedef struct STNBObjRef_ {
	void*			opaque;
} STNBObjRef;

#define NB_OBJREF_NULL						NBST_P(STNBObjRef, NULL)

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
#	define NBObjRef_create(CTX, CLS)		NBObjRef_create_((CTX), (CLS), "NBObjRef_create", __func__, __FILE__, __LINE__, __func__)
#	define NBObjRef_retain(OBJ)				NBObjRef_retain_((OBJ), __FILE__, __LINE__, __func__)
#	define NBObjRef_release(OBJ)			NBObjRef_release_((OBJ), __FILE__, __LINE__, __func__)
#	define NBObjRef_set(PTR, OTHER)			NBObjRef_set_((PTR), (OTHER), __FILE__, __LINE__, __func__)
#	define NBObjRef_isSet(OBJ)				((OBJ).opaque != NULL)
#	define NBObjRef_isSame(OBJ, OTHER)		((OBJ).opaque == (OTHER).opaque)
#	define NBObjRef_null(PTR)				(PTR)->opaque = NULL
#	define NBObjRef_asOpqPtr(REF)			(REF).opaque
#	define NBObjRef_fromOpqPtr(OPQ_PTR)		NBST_P(STNBObjRef, OPQ_PTR)
#else
#	define NBObjRef_create(CTX, CLS)		NBObjRef_create_((CTX), (CLS))
#	define NBObjRef_retain(OBJ)				NBObjRef_retain_((OBJ))
#	define NBObjRef_release(OBJ)			NBObjRef_release_((OBJ))
#	define NBObjRef_set(PTR, OTHER)			NBObjRef_set_((PTR), (OTHER))
#	define NBObjRef_isSet(OBJ)				((OBJ).opaque != NULL)
#	define NBObjRef_isSame(OBJ, OTHER)		((OBJ).opaque == (OTHER).opaque)
#	define NBObjRef_null(PTR)				(PTR)->opaque = NULL
#	define NBObjRef_asOpqPtr(REF)			(REF).opaque
#	define NBObjRef_fromOpqPtr(OPQ_PTR)		NBST_P(STNBObjRef, OPQ_PTR)
#endif

//Itf methods
void NBObjRef_retainOpq(void* opqPtr);
void NBObjRef_releaseOpq(void* opqPtr);
//
BOOL NBObjRef_objLnkIsSameObjectOpq_(STNBObject* obj, void* opqPtr);
BOOL NBObjRef_objLnkIsSameObjRefOpq_(STNBObjRef* ref, void* opqPtr);

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
STNBObjRef NBObjRef_create_(STNBContext* ctx, const STNBClass* cls, const char* name, const char* fullpath, const SI32 line, const char* func);
void NBObjRef_retain_(STNBObjRef ref, const char* fullpath, const SI32 line, const char* func);
void NBObjRef_release_(STNBObjRef* ref, const char* fullpath, const SI32 line, const char* func);
void NBObjRef_set_(STNBObjRef* ref, const STNBObjRef* other, const char* fullpath, const SI32 line, const char* func);
#else
STNBObjRef NBObjRef_create_(STNBContext* ctx, const STNBClass* cls);
void NBObjRef_retain_(STNBObjRef ref);
void NBObjRef_release_(STNBObjRef* ref);
void NBObjRef_set_(STNBObjRef* ref, const STNBObjRef* other);
#endif
UI32 NBObjRef_retainCount(STNBObjRef ref);
BOOL NBObjRef_isClass(STNBObjRef ref, const STNBClass* cls);

//---------------------------------------------------
//- Declares a class in a header file (.h-like)
//- with the expected standard names for this framework.
//---------------------------------------------------
#define NB_OBJREF_HEADER(CLS_NAME)	\
	/*typedef ST+CLS_NAME+Ref to STNBObjRef*/ \
	typedef STNBObjRef ST ## CLS_NAME ## Ref; \
	/*alloc*/ \
	ST ## CLS_NAME ## Ref CLS_NAME ##_alloc(STNBContext* ctx); \
	/*getObjSz*/ \
	UI32 CLS_NAME ##_getObjSz(void); \
	/*getClass*/ \
	const STNBClass* CLS_NAME ##_getClass(void); \
    /*getLnk*/ \
    void CLS_NAME ##_getLnk(ST ## CLS_NAME ## Ref ref, STNBObjectLnk* dst); \
	/*retain*/ \
	void CLS_NAME ##_retain(ST ## CLS_NAME ## Ref ref); \
	/*release*/ \
	void CLS_NAME ##_release(ST ## CLS_NAME ## Ref* ref); \
	/*retainCount*/ \
	UI32 CLS_NAME ##_retainCount(ST ## CLS_NAME ## Ref ref); \
	/*isClass*/ \
	BOOL CLS_NAME ##_isClass(ST ## CLS_NAME ## Ref ref); \
    /*swap*/ \
    void CLS_NAME ##_swap(ST ## CLS_NAME ## Ref* ref, ST ## CLS_NAME ## Ref* other); \
	/*set*/ \
	void CLS_NAME ##_set(ST ## CLS_NAME ## Ref* ref, const ST ## CLS_NAME ## Ref* other); \
	/*isSet*/ \
	BOOL CLS_NAME ##_isSet(ST ## CLS_NAME ## Ref ref); \
	/*isSame*/ \
	BOOL CLS_NAME ##_isSame(ST ## CLS_NAME ## Ref ref, const ST ## CLS_NAME ## Ref other); \
	/*null*/ \
	void CLS_NAME ##_null(ST ## CLS_NAME ## Ref* ref); \
	/*asOpqPtr*/ \
	void* CLS_NAME ##_asOpqPtr(ST ## CLS_NAME ## Ref ref); \
	/*fromOpqPtr*/ \
	ST ## CLS_NAME ## Ref CLS_NAME ##_fromOpqPtr(void* ptr); \
	/*initZeroedRecursively*/ \
	void CLS_NAME ##_initZeroedRecursively(STNBObject* obj); \
	/*uninitLockedRecursively*/ \
	void CLS_NAME ##_uninitLockedRecursively(STNBObject* obj);

//---------------------------------------------------
//- Declares a class in a body file (.c-like)
//- with the expected standard names for this framework.
//- Note: this should be added after the declaration of
//        the object CLS_NAME structure.
//---------------------------------------------------

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
#	define NB_OBJREF_BODY_ALLOC(CLS_NAME, ST_NAME, PRNT_NAME) \
	/* alloc function */ \
	ST ## CLS_NAME ## Ref CLS_NAME ##_alloc(STNBContext* ctx){ \
		STNBObjRef r; \
		ST_NAME* obj	= (ST_NAME*)NBMemory_allocType(ST_NAME); \
		NBMemory_setZeroSt(*obj, ST_NAME); \
		NBObject_initZeroedInternal_((STNBObject*)obj, & CLS_NAME ## _class_, ctx, #CLS_NAME "_alloc", __FILE__, __LINE__, __func__); \
		/*validate struct definition*/ \
		NBASSERT((ST_NAME*)&obj->prnt == obj) /*first member must be 'prnt'*/ \
		CLS_NAME ## _initZeroedRecursively((STNBObject*)obj); \
		/*validate init-final-state*/ \
		NBASSERT(((STNBObject*)obj)->ctx == ctx && ((STNBObject*)obj)->cls == & CLS_NAME ## _class_ && ((STNBObject*)obj)->refCount > 0) \
		r.opaque		= obj; \
		return r; \
	}
#else
#	define NB_OBJREF_BODY_ALLOC(CLS_NAME, ST_NAME, PRNT_NAME) \
	/* alloc function */ \
	ST ## CLS_NAME ## Ref CLS_NAME ##_alloc(STNBContext* ctx){ \
		STNBObjRef r; \
		ST_NAME* obj	= (ST_NAME*)NBMemory_allocType(ST_NAME); \
		NBMemory_setZeroSt(*obj, ST_NAME); \
		NBObject_initZeroedInternal_((STNBObject*)obj, & CLS_NAME ## _class_, ctx); \
		/*validate struct definition*/ \
		NBASSERT((ST_NAME*)&obj->prnt == obj) /*first member must be 'prnt'*/ \
		CLS_NAME ## _initZeroedRecursively((STNBObject*)obj); \
		/*validate init-final-state*/ \
		NBASSERT(((STNBObject*)obj)->ctx == ctx && ((STNBObject*)obj)->cls == & CLS_NAME ## _class_ && ((STNBObject*)obj)->refCount > 0) \
		r.opaque		= obj; \
		return r; \
	}
#endif

#define NB_OBJREF_BODY(CLS_NAME, ST_NAME, PRNT_NAME)	\
	/* init/uninit functions (user must define them) */ \
	void CLS_NAME ## _initZeroed(STNBObject* obj); \
	void CLS_NAME ## _uninitLocked(STNBObject* obj); \
	/* STNBClass definition */ \
	static const STNBClass CLS_NAME ## _class_ = { \
		NB_CLASS_HEAD_32, \
		#CLS_NAME, \
		{ \
			CLS_NAME ## _initZeroedRecursively, \
			CLS_NAME ## _uninitLockedRecursively, \
			CLS_NAME ## _getObjSz, \
			PRNT_NAME ## _getClass \
		} \
	}; \
	/* alloc function*/ \
	NB_OBJREF_BODY_ALLOC(CLS_NAME, ST_NAME, PRNT_NAME) \
	/* getObjSz function */ \
	UI32 CLS_NAME ## _getObjSz(void){ \
		return sizeof(ST_NAME); \
	} \
	/* getClass function */ \
	const STNBClass* CLS_NAME ## _getClass(void){ \
		return & CLS_NAME ## _class_; \
	} \
    /* getLnk function */ \
    void CLS_NAME ## _getLnk(ST ## CLS_NAME ## Ref ref, STNBObjectLnk* dst){ \
        NBASSERT(!NBObjRef_isSet(ref) || NBObjRef_isClass(ref, & CLS_NAME ## _class_)) \
        if(dst != NULL){ \
            if(ref.opaque == NULL){ \
                NBObjectLnk_setItf(dst, NULL, NULL); \
            } else { \
                STNBObjectLnkItf itf; \
                NBMemory_setZeroSt(itf, STNBObjectLnkItf); \
                itf.objLnkRetain        = NBObjRef_retainOpq; \
                itf.objLnkRelease       = NBObjRef_releaseOpq; \
                itf.objLnkIsSameObject  = NBObjRef_objLnkIsSameObjectOpq_; \
                itf.objLnkIsSameObjRef  = NBObjRef_objLnkIsSameObjRefOpq_; \
                NBObjectLnk_setItf(dst, &itf, ref.opaque); \
            } \
        } \
    } \
	/*fromOpqPtr*/ \
	ST ## CLS_NAME ## Ref CLS_NAME ##_fromOpqPtr(void* ptr){ \
		ST ## CLS_NAME ## Ref ref = NBObjRef_fromOpqPtr(ptr);\
		NBASSERT(!NBObjRef_isSet(ref) || NBObjRef_isClass(ref, & CLS_NAME ## _class_)) \
		return ref; \
	} \
	/*initZeroedRecursively*/ \
	void CLS_NAME ##_initZeroedRecursively(STNBObject* obj){ \
		ST ## CLS_NAME ## Opq* opq = (ST ## CLS_NAME ## Opq*)obj; \
		PRNT_NAME ## _initZeroed((STNBObject*)&opq->prnt); \
		CLS_NAME ## _initZeroed((STNBObject*)opq); \
	} \
	/*uninitLockedRecursively*/ \
	void CLS_NAME ##_uninitLockedRecursively(STNBObject* obj){ \
		ST ## CLS_NAME ## Opq* opq = (ST ## CLS_NAME ## Opq*)obj; \
		CLS_NAME ## _uninitLocked((STNBObject*)opq); \
		PRNT_NAME ## _uninitLocked((STNBObject*)&opq->prnt); \
	} \
	/*retain*/ \
	void CLS_NAME ##_retain(ST ## CLS_NAME ## Ref ref){ \
		NBASSERT(NBObjRef_isClass(ref, & CLS_NAME ## _class_)) \
		NBObjRef_retain(ref); \
	} \
	/*release*/ \
	void CLS_NAME ##_release(ST ## CLS_NAME ## Ref* ref){ \
		NBASSERT(!NBObjRef_isSet(*ref) || NBObjRef_isClass(*ref, & CLS_NAME ## _class_)) \
		NBObjRef_release(ref); \
	} \
	/*retainCount*/ \
	UI32 CLS_NAME ##_retainCount(ST ## CLS_NAME ## Ref ref){ \
		NBASSERT(!NBObjRef_isSet(ref) || NBObjRef_isClass(ref, & CLS_NAME ## _class_)) \
		return NBObjRef_retainCount(ref); \
	} \
	/*isClass*/ \
	BOOL CLS_NAME ##_isClass(ST ## CLS_NAME ## Ref ref){ \
		return NBObjRef_isSet(ref) && NBObjRef_isClass(ref, & CLS_NAME ## _class_); \
	} \
    /*swap*/ \
    void CLS_NAME ##_swap(ST ## CLS_NAME ## Ref* ref, ST ## CLS_NAME ## Ref* other){ \
        NBASSERT(!NBObjRef_isSet(*ref) || NBObjRef_isClass(*ref, & CLS_NAME ## _class_)) \
        NBASSERT(other == NULL || !NBObjRef_isSet(*other) || NBObjRef_isClass(*other, & CLS_NAME ## _class_)) \
        ST ## CLS_NAME ## Ref tmp = *ref; \
        *ref = *other; *other = tmp;\
    } \
	/*set*/ \
	void CLS_NAME ##_set(ST ## CLS_NAME ## Ref* ref, const ST ## CLS_NAME ## Ref* other){ \
		NBASSERT(!NBObjRef_isSet(*ref) || NBObjRef_isClass(*ref, & CLS_NAME ## _class_)) \
		NBASSERT(other == NULL || !NBObjRef_isSet(*other) || NBObjRef_isClass(*other, & CLS_NAME ## _class_)) \
		NBObjRef_set(ref, other); \
	} \
	/*isSet*/ \
	BOOL CLS_NAME ##_isSet(ST ## CLS_NAME ## Ref ref){ \
		NBASSERT(!NBObjRef_isSet(ref) || NBObjRef_isClass(ref, & CLS_NAME ## _class_)) \
		return NBObjRef_isSet(ref); \
	} \
	/*isSame*/ \
	BOOL CLS_NAME ##_isSame(ST ## CLS_NAME ## Ref ref, ST ## CLS_NAME ## Ref other){ \
		NBASSERT(!NBObjRef_isSet(ref) || NBObjRef_isClass(ref, & CLS_NAME ## _class_)) \
		NBASSERT(!NBObjRef_isSet(other) || NBObjRef_isClass(other, & CLS_NAME ## _class_)) \
		return NBObjRef_isSame(ref, other); \
	} \
	/*null*/ \
	void CLS_NAME ##_null(ST ## CLS_NAME ## Ref* ref){ \
		NBASSERT(!NBObjRef_isSet(*ref) || NBObjRef_isClass(*ref, & CLS_NAME ## _class_)) \
		NBObjRef_null(ref); \
	} \
	/*asOpqPtr*/ \
	void* CLS_NAME ##_asOpqPtr(ST ## CLS_NAME ## Ref ref){ \
		NBASSERT(!NBObjRef_isSet(ref) || NBObjRef_isClass(ref, & CLS_NAME ## _class_)) \
		return NBObjRef_asOpqPtr(ref);\
	}

//

#ifdef __cplusplus
} //extern "C"
#endif

#endif
