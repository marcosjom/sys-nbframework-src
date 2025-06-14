#ifndef NB_CLASS_H
#define NB_CLASS_H

#include "nb/NBFrameworkDefs.h"
#include "nb/NBContext.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NB_CLASS_HEAD_32	0xa0b1c2d3	 

struct STNBObject_;
struct STNBClass_;

//Methods
typedef void (*NBClassInitObjFnc)(struct STNBObject_* obj);
typedef void (*NBClassUninitObjFnc)(struct STNBObject_* obj);
typedef UI32 (*NBClassGetObjSzFnc)(void);
typedef const struct STNBClass_* (*NBClassGetClassFnc)(void);

/*
typedef struct STNBObjectVTbl_ {
	void (*uninitLocked)(struct STNBObject_* obj);
	UI32 (*getObjSz)(void);
	const struct STNBClass_* (*getClass)(void);
} STNBObjectVTbl;

STNBObjectVTbl* vTbl;	//virtual methods table
*/

typedef struct STNBClass_ {
	UI32			head;	//must be NB_CLASS_HEAD_32
	const char*		name;	//name of class
	struct {
		NBClassInitObjFnc		initZeroedRecursively;		//initialize an allocated opaque structure (>= opqSz)
		NBClassUninitObjFnc		uninitLockedRecursively;	//uninitialize an opaque structure internal elements
		NBClassGetObjSzFnc		getObjSz;					//retrieve the size of the object
		NBClassGetClassFnc		getParentClass;				//retrieve the parent class definition.
	} calls;
} STNBClass;

#ifdef __cplusplus
} //extern "C"
#endif

#endif
