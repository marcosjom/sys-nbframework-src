
#include "nb/NBFrameworkPch.h"
#include "nb/NBObjectLnk.h"
//
#include "nb/NBObject.h"
#include "nb/core/NBMemory.h"


void NBObjectLnk_init(STNBObjectLnk* obj){
    NBMemory_setZeroSt(*obj, STNBObjectLnk);
}

void NBObjectLnk_retain(STNBObjectLnk* obj){
    if(obj != NULL && obj->itf.objLnkRetain != NULL){
        (*obj->itf.objLnkRetain)(obj->usrData);
    }
}

void NBObjectLnk_release(STNBObjectLnk* obj){
    if(obj != NULL && obj->itf.objLnkRelease != NULL){
        (*obj->itf.objLnkRelease)(obj->usrData);
    }
}

void NBObjectLnk_null(STNBObjectLnk* obj){
    if(obj != NULL){
        NBMemory_setZeroSt(*obj, STNBObjectLnk);
    }
}

//cfg

BOOL NBObjectLnk_isSet(STNBObjectLnk* obj){
    return (obj != NULL && obj->itf.objLnkRetain != NULL && obj->itf.objLnkRelease != NULL);
}

void NBObjectLnk_set(STNBObjectLnk* obj, const STNBObjectLnk* other){
    //retain new
    if(other != NULL && other->itf.objLnkRetain != NULL){
        (*other->itf.objLnkRetain)(other->usrData);
    }
    //release current
    if(obj != NULL && obj->itf.objLnkRelease != NULL){
        (*obj->itf.objLnkRelease)(obj->usrData);
    }
    //apply
    if(other == NULL){
        NBMemory_setZeroSt(obj->itf, STNBObjectLnkItf);
        obj->usrData = NULL;
    } else {
        obj->itf = other->itf;
        obj->usrData = other->usrData;
    }
}

void NBObjectLnk_setItf(STNBObjectLnk* obj, const STNBObjectLnkItf* itf, void* usrData){
    //retain new
    if(itf != NULL && itf->objLnkRetain != NULL){
        (*itf->objLnkRetain)(usrData);
    }
    //release current
    if(obj != NULL && obj->itf.objLnkRelease != NULL){
        (*obj->itf.objLnkRelease)(obj->usrData);
    }
    //apply
    if(itf == NULL){
        NBMemory_setZeroSt(obj->itf, STNBObjectLnkItf);
        obj->usrData = NULL;
    } else {
        obj->itf = *itf;
        obj->usrData = usrData;
    }
}

//

BOOL NBObjectLnk_isSameObject(STNBObjectLnk* obj, STNBObject* otherObj){
    BOOL r = FALSE;
    if(obj != NULL && obj->itf.objLnkIsSameObject != NULL && otherObj != NULL){
        r = (*obj->itf.objLnkIsSameObject)(otherObj, obj->usrData);
    }
    return r;
}

BOOL NBObjectLnk_isSameObjRef(STNBObjectLnk* obj, STNBObjRef* ref){
    BOOL r = FALSE;
    if(obj != NULL && obj->itf.objLnkIsSameObjRef != NULL && ref != NULL){
        r = (*obj->itf.objLnkIsSameObjRef)(ref, obj->usrData);
    }
    return r;
}

