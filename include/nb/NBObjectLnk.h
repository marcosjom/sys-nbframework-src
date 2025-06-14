#ifndef NB_OBJECT_LNK_H
#define NB_OBJECT_LNK_H

#include "nb/NBFrameworkDefs.h"

#ifdef __cplusplus
extern "C" {
#endif

//NBObjectLnk is a mechanism to keep an identifiable retained-reference to an object without access to the object itself.
//Is intended to allow classes to keep internal objects as private but identifiable by external classes.
//As example: NBIOPollster takes ownership of sockets that are fed to protect the synchronization logic from external interference.
//            The original creator of the socket can keep a NBObjectLnk to identify the socket in the callback-methods,
//            without gaining access to the socket objects or methods. This way the NBIOPollster can operate in a "safer" invironment.

struct STNBObject_;
struct STNBObjRef_;

//NBObjectLnkItf

typedef struct STNBObjectLnkItf_ {
    //refs
    void    (*objLnkRetain)(void* usrData);
    void    (*objLnkRelease)(void* usrData);
    //
    BOOL    (*objLnkIsSameObject)(struct STNBObject_* obj, void* usrData);
    BOOL    (*objLnkIsSameObjRef)(struct STNBObjRef_* ref, void* usrData);
} STNBObjectLnkItf;

//NBObjectLnk

typedef struct STNBObjectLnk_ {
    STNBObjectLnkItf    itf;
    void*           usrData;
} STNBObjectLnk;

void NBObjectLnk_init(STNBObjectLnk* obj);
void NBObjectLnk_retain(STNBObjectLnk* obj);
void NBObjectLnk_release(STNBObjectLnk* obj);
void NBObjectLnk_null(STNBObjectLnk* obj);

//cfg
BOOL NBObjectLnk_isSet(STNBObjectLnk* obj);
void NBObjectLnk_set(STNBObjectLnk* obj, const STNBObjectLnk* other);
void NBObjectLnk_setItf(STNBObjectLnk* obj, const STNBObjectLnkItf* itf, void* usrData);

//
BOOL NBObjectLnk_isSameObject(STNBObjectLnk* obj, struct STNBObject_* otherObj);
BOOL NBObjectLnk_isSameObjRef(STNBObjectLnk* obj, struct STNBObjRef_* ref);

#ifdef __cplusplus
} //extern "C"
#endif


#endif
