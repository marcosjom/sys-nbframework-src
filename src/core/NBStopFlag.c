//
//  NBStopFlag.h
//  thinstream
//
//  Created by Marcos Ortega on 9/3/19.
//

#include "nb/NBFrameworkPch.h"
#include "nb/core/NBStopFlag.h"
#include "nb/core/NBMemory.h"

//

//--------------------
//- NBStopFlag
//--------------------

typedef struct STNBStopFlagOpq_ {
	STNBObject			prnt;		//parent
    BOOL                stopFlag;   //my flag
    STNBStopFlagRef     stopFlagParent; //my parent's flag
} STNBStopFlagOpq;

NB_OBJREF_BODY(NBStopFlag, STNBStopFlagOpq, NBObject)

//

void NBStopFlag_initZeroed(STNBObject* obj){
	//nothing
}

void NBStopFlag_uninitLocked(STNBObject* obj){
    STNBStopFlagOpq* opq = (STNBStopFlagOpq*)obj;
	//stopFlagParent
    if(NBStopFlag_isSet(opq->stopFlagParent)){
        NBStopFlag_release(&opq->stopFlagParent);
        NBStopFlag_null(&opq->stopFlagParent);
    }
}

//cfg

void NBStopFlag_setParentFlag(STNBStopFlagRef ref, STNBStopFlagRef* parentFlag){
    STNBStopFlagOpq* opq = (STNBStopFlagOpq*)ref.opaque; NBASSERT(NBStopFlag_isClass(ref))
    NBASSERT(parentFlag == NULL || NBStopFlag_isClass(*parentFlag))
    NBObject_lock(opq);
    { //only-allowed-once
        //retain
        if(parentFlag != NULL){
            NBStopFlag_retain(*parentFlag);
        }
        //release
        if(NBStopFlag_isSet(opq->stopFlagParent)){
            NBStopFlag_release(&opq->stopFlagParent);
            NBStopFlag_null(&opq->stopFlagParent);
        }
        //set
        if(parentFlag != NULL){
            opq->stopFlagParent = *parentFlag;
        }
    }
    NBObject_unlock(opq);
}

//

void NBStopFlag_activate(STNBStopFlagRef ref){
    NBASSERT(NBStopFlag_isClass(ref))
    //interruption-safe-by-design, do not do complex actions
    //atomic-by-design, do not lock
    ((STNBStopFlagOpq*)ref.opaque)->stopFlag = TRUE;
}

BOOL NBStopFlag_isMineActivated(STNBStopFlagRef ref){   //quick validation of this level's flag.
    NBASSERT(NBStopFlag_isClass(ref))
    //interruption-safe-by-design, do not do complex actions
    //atomic-by-design, do not lock
    return ((STNBStopFlagOpq*)ref.opaque)->stopFlag;
}

BOOL NBStopFlag_isAnyActivated(STNBStopFlagRef ref){    //slower validation of this and upper levels flags.
    NBASSERT(NBStopFlag_isClass(ref))
    //ToDo: implement and validate safe-thread locks
    STNBStopFlagOpq* opq = (STNBStopFlagOpq*)ref.opaque;
    while(opq != NULL && !opq->stopFlag){
        opq = (NBStopFlag_isSet(opq->stopFlagParent) ? (STNBStopFlagOpq*)opq->stopFlagParent.opaque : NULL);
    }
    return (opq == NULL ? FALSE : opq->stopFlag);
}

void NBStopFlag_reset(STNBStopFlagRef ref){
    NBASSERT(NBStopFlag_isClass(ref))
    //atomic-by-design, do not lock
    ((STNBStopFlagOpq*)ref.opaque)->stopFlag = FALSE;
}
