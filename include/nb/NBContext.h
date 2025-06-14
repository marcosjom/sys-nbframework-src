#ifndef NB_CONTEXT_H
#define NB_CONTEXT_H

#include "nb/NBFrameworkDefs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct STNBContext_ {
	void*	opaque;
} STNBContext;

void NBContext_init(STNBContext* obj);
void NBContext_retain(STNBContext* obj);
void NBContext_release(STNBContext* obj);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
