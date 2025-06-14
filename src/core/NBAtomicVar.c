//
//  NBAtomicVar.h
//  thinstream
//
//  Created by Marcos Ortega on 9/3/19.
//

#include "nb/NBFrameworkPch.h"
#include "nb/core/NBAtomicVar.h"
#include "nb/core/NBMemory.h"

//

//--------------------
//- NBAtomicVar
//--------------------

typedef struct STNBAtomicVarOpq_ {
	STNBObject			prnt;		//parent
    STNBAtomicVarValue  value;
} STNBAtomicVarOpq;

NB_OBJREF_BODY(NBAtomicVar, STNBAtomicVarOpq, NBObject)

//

void NBAtomicVar_initZeroed(STNBObject* obj){
	//nothing
}

void NBAtomicVar_uninitLocked(STNBObject* obj){
	//nothing
}

//get (some consumers will validate the type be defined when a AtomicVar is provided)

ENAtomicVarType NBAtomicVar_getType(STNBAtomicVarRef ref){
    NBASSERT(NBAtomicVar_isClass(ref))
    return ((STNBAtomicVarOpq*)ref.opaque)->value.type;
}

STNBAtomicVarValue NBAtomicVar_getValue(STNBAtomicVarRef ref){
    NBASSERT(NBAtomicVar_isClass(ref))
    return ((STNBAtomicVarOpq*)ref.opaque)->value;
}

BOOL NBAtomicVar_getBOOL(STNBAtomicVarRef ref){
    NBASSERT(NBAtomicVar_isClass(ref))
    return NBAtomicVarValue_getBOOL(&((STNBAtomicVarOpq*)ref.opaque)->value);
}

char NBAtomicVar_getChar(STNBAtomicVarRef ref){
    NBASSERT(NBAtomicVar_isClass(ref))
    return NBAtomicVarValue_getChar(&((STNBAtomicVarOpq*)ref.opaque)->value);
}

BYTE NBAtomicVar_getBYTE(STNBAtomicVarRef ref){
    NBASSERT(NBAtomicVar_isClass(ref))
    return NBAtomicVarValue_getBYTE(&((STNBAtomicVarOpq*)ref.opaque)->value);
}

SI16 NBAtomicVar_getSI16(STNBAtomicVarRef ref){
    NBASSERT(NBAtomicVar_isClass(ref))
    return NBAtomicVarValue_getSI16(&((STNBAtomicVarOpq*)ref.opaque)->value);
}

UI16 NBAtomicVar_getUI16(STNBAtomicVarRef ref){
    NBASSERT(NBAtomicVar_isClass(ref))
    return NBAtomicVarValue_getUI16(&((STNBAtomicVarOpq*)ref.opaque)->value);
}

SI32 NBAtomicVar_getSI32(STNBAtomicVarRef ref){
    NBASSERT(NBAtomicVar_isClass(ref))
    return NBAtomicVarValue_getSI32(&((STNBAtomicVarOpq*)ref.opaque)->value);
}

UI32 NBAtomicVar_getUI32(STNBAtomicVarRef ref){
    NBASSERT(NBAtomicVar_isClass(ref))
    return NBAtomicVarValue_getUI32(&((STNBAtomicVarOpq*)ref.opaque)->value);
}

//set (once the type is set, it will lock)

BOOL NBAtomicVar_setType(STNBAtomicVarRef ref, const ENAtomicVarType type){
    BOOL r = FALSE;
    STNBAtomicVarOpq* opq = (STNBAtomicVarOpq*)ref.opaque; NBASSERT(NBAtomicVar_isClass(ref))
    if(opq->value.type == ENAtomicVarType_Unknown){
        opq->value.type = type;
        r = TRUE;
    }
    return r;
}

BOOL NBAtomicVar_setValue(STNBAtomicVarRef ref, const STNBAtomicVarValue* v){
    BOOL r = FALSE;
    STNBAtomicVarOpq* opq = (STNBAtomicVarOpq*)ref.opaque; NBASSERT(NBAtomicVar_isClass(ref))
    if(opq->value.type == ENAtomicVarType_Unknown && v != NULL){
        opq->value = *v;
        r = TRUE;
    }
    return r;
}

BOOL NBAtomicVar_setBOOL(STNBAtomicVarRef ref, const BOOL v){
    BOOL r = FALSE;
    STNBAtomicVarOpq* opq = (STNBAtomicVarOpq*)ref.opaque; NBASSERT(NBAtomicVar_isClass(ref))
    switch(opq->value.type) {
        case ENAtomicVarType_Unknown:
            opq->value.type = ENAtomicVarType_BOOL;
            opq->value.data.b = v;
            r = TRUE;
            break;
        case ENAtomicVarType_BOOL:
            opq->value.data.b = v;
            r = TRUE;
            break;
        default:
            break;
    }
    return r;
}

BOOL NBAtomicVar_setChar(STNBAtomicVarRef ref, const char v){
    BOOL r = FALSE;
    STNBAtomicVarOpq* opq = (STNBAtomicVarOpq*)ref.opaque; NBASSERT(NBAtomicVar_isClass(ref))
    switch(opq->value.type) {
        case ENAtomicVarType_Unknown:
            opq->value.type = ENAtomicVarType_char;
            opq->value.data.c = v;
            r = TRUE;
            break;
        case ENAtomicVarType_char:
            opq->value.data.c = v;
            r = TRUE;
            break;
        default:
            break;
    }
    return r;
}

BOOL NBAtomicVar_setBYTE(STNBAtomicVarRef ref, const BYTE v){
    BOOL r = FALSE;
    STNBAtomicVarOpq* opq = (STNBAtomicVarOpq*)ref.opaque; NBASSERT(NBAtomicVar_isClass(ref))
    switch(opq->value.type) {
        case ENAtomicVarType_Unknown:
            opq->value.type = ENAtomicVarType_BYTE;
            opq->value.data.bb = v;
            r = TRUE;
            break;
        case ENAtomicVarType_BYTE:
            opq->value.data.bb = v;
            r = TRUE;
            break;
        default:
            break;
    }
    return r;
}

BOOL NBAtomicVar_setSI16(STNBAtomicVarRef ref, const SI16 v){
    BOOL r = FALSE;
    STNBAtomicVarOpq* opq = (STNBAtomicVarOpq*)ref.opaque; NBASSERT(NBAtomicVar_isClass(ref))
    switch(opq->value.type) {
        case ENAtomicVarType_Unknown:
            opq->value.type = ENAtomicVarType_SI16;
            opq->value.data.si16 = v;
            r = TRUE;
            break;
        case ENAtomicVarType_SI16:
            opq->value.data.si16 = v;
            r = TRUE;
            break;
        default:
            break;
    }
    return r;
}

BOOL NBAtomicVar_setUI16(STNBAtomicVarRef ref, const UI16 v){
    BOOL r = FALSE;
    STNBAtomicVarOpq* opq = (STNBAtomicVarOpq*)ref.opaque; NBASSERT(NBAtomicVar_isClass(ref))
    switch(opq->value.type) {
        case ENAtomicVarType_Unknown:
            opq->value.type = ENAtomicVarType_UI16;
            opq->value.data.ui16 = v;
            r = TRUE;
            break;
        case ENAtomicVarType_UI16:
            opq->value.data.ui16 = v;
            r = TRUE;
            break;
        default:
            break;
    }
    return r;
}

BOOL NBAtomicVar_setSI32(STNBAtomicVarRef ref, const SI32 v){
    BOOL r = FALSE;
    STNBAtomicVarOpq* opq = (STNBAtomicVarOpq*)ref.opaque; NBASSERT(NBAtomicVar_isClass(ref))
    switch(opq->value.type) {
        case ENAtomicVarType_Unknown:
            opq->value.type = ENAtomicVarType_SI32;
            opq->value.data.si32 = v;
            r = TRUE;
            break;
        case ENAtomicVarType_SI32:
            opq->value.data.si32 = v;
            r = TRUE;
            break;
        default:
            break;
    }
    return r;
}

BOOL NBAtomicVar_setUI32(STNBAtomicVarRef ref, const SI32 v){
    BOOL r = FALSE;
    STNBAtomicVarOpq* opq = (STNBAtomicVarOpq*)ref.opaque; NBASSERT(NBAtomicVar_isClass(ref))
    switch(opq->value.type) {
        case ENAtomicVarType_Unknown:
            opq->value.type = ENAtomicVarType_UI32;
            opq->value.data.ui32 = v;
            r = TRUE;
            break;
        case ENAtomicVarType_UI32:
            opq->value.data.ui32 = v;
            r = TRUE;
            break;
        default:
            break;
    }
    return r;
}

