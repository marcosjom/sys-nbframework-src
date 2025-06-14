//
//  NBAtomicVar.h
//  thinstream
//
//  Created by Marcos Ortega on 9/3/19.
//

#ifndef NBAtomicVar_h
#define NBAtomicVar_h

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"

#ifdef __cplusplus
extern "C" {
#endif

//Atomic-vars can be shared by ownership of multiple objects.
//Atomic-vars are considerated safe to be read/write at interruptions, like stop-signals.

//ENAtomicVarType

typedef enum ENAtomicVarType_ {
    ENAtomicVarType_Unknown = 0,    //not set yet
    //1-byte
    ENAtomicVarType_BOOL,           //BOOL value
    ENAtomicVarType_char,           //char value
    ENAtomicVarType_BYTE,           //BYTE value
    //2-bytes
    ENAtomicVarType_SI16,           //SI16 value
    ENAtomicVarType_UI16,           //UI32 value
    //4-bytes
    ENAtomicVarType_SI32,           //SI32 value
    ENAtomicVarType_UI32,           //UI32 value
    //
    ENAtomicVarType_Count
} ENAtomicVarType;

//NBAtomicVarValue

typedef struct STNBAtomicVarValue_ {
    ENAtomicVarType     type;
    union {
        //1-byte
        BOOL            b;
        char            c;
        BYTE            bb;
        //2-byte
        SI16            si16;
        UI16            ui16;
        //4-byte
        SI32            si32;
        UI32            ui32;
    } data;
} STNBAtomicVarValue;

#define NBAtomicVarValue_getBOOL(PTR)   ((PTR) == NULL ? 0 : (PTR)->data.b)
#define NBAtomicVarValue_getChar(PTR)   ((PTR) == NULL ? 0 : (PTR)->data.c)
#define NBAtomicVarValue_getBYTE(PTR)   ((PTR) == NULL ? 0 : (PTR)->data.bb)
#define NBAtomicVarValue_getSI16(PTR)   ((PTR) == NULL ? 0 : (PTR)->data.si16)
#define NBAtomicVarValue_getUI16(PTR)   ((PTR) == NULL ? 0 : (PTR)->data.ui16)
#define NBAtomicVarValue_getSI32(PTR)   ((PTR) == NULL ? 0 : (PTR)->data.si32)
#define NBAtomicVarValue_getUI32(PTR)   ((PTR) == NULL ? 0 : (PTR)->data.ui32)

//NBAtomicVar

NB_OBJREF_HEADER(NBAtomicVar)

//get (some consumers will validate the type be defined when a AtomicVar is provided)
ENAtomicVarType     NBAtomicVar_getType(STNBAtomicVarRef ref);
STNBAtomicVarValue  NBAtomicVar_getValue(STNBAtomicVarRef ref);
BOOL NBAtomicVar_getBOOL(STNBAtomicVarRef ref);
char NBAtomicVar_getChar(STNBAtomicVarRef ref);
BYTE NBAtomicVar_getBYTE(STNBAtomicVarRef ref);
SI16 NBAtomicVar_getSI16(STNBAtomicVarRef ref);
UI16 NBAtomicVar_getUI16(STNBAtomicVarRef ref);
SI32 NBAtomicVar_getSI32(STNBAtomicVarRef ref);
UI32 NBAtomicVar_getUI32(STNBAtomicVarRef ref);

//set (once the type is set, it will lock)
BOOL NBAtomicVar_setType(STNBAtomicVarRef ref, const ENAtomicVarType type);
BOOL NBAtomicVar_setValue(STNBAtomicVarRef ref, const STNBAtomicVarValue* v);
BOOL NBAtomicVar_setBOOL(STNBAtomicVarRef ref, const BOOL v);
BOOL NBAtomicVar_setChar(STNBAtomicVarRef ref, const char v);
BOOL NBAtomicVar_setBYTE(STNBAtomicVarRef ref, const BYTE v);
BOOL NBAtomicVar_setSI16(STNBAtomicVarRef ref, const SI16 v);
BOOL NBAtomicVar_setUI16(STNBAtomicVarRef ref, const UI16 v);
BOOL NBAtomicVar_setSI32(STNBAtomicVarRef ref, const SI32 v);
BOOL NBAtomicVar_setUI32(STNBAtomicVarRef ref, const SI32 v);

#ifdef __cplusplus
} //extern "C"
#endif
		
#endif /* NBAtomicVar_h */
