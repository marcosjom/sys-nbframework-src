//
//  NBStopFlag.h
//  thinstream
//
//  Created by Marcos Ortega on 9/3/19.
//

#ifndef NBStopFlag_h
#define NBStopFlag_h

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"

#ifdef __cplusplus
extern "C" {
#endif

//Atomic-stop-flags can be shared by ownership of multiple objects, also in vertical hierarchy (parent flag).
//Atomic-stop-flags are considerated safe to be read/write at interruptions, like stop-signals.
//NBStopFlag

NB_OBJREF_HEADER(NBStopFlag)

//cfg

void NBStopFlag_setParentFlag(STNBStopFlagRef ref, STNBStopFlagRef* parentFlag);

//

void NBStopFlag_activate(STNBStopFlagRef ref);
BOOL NBStopFlag_isMineActivated(STNBStopFlagRef ref); //quick validation of this level's flag.
BOOL NBStopFlag_isAnyActivated(STNBStopFlagRef ref);  //slower validation of this and upper levels flags.
void NBStopFlag_reset(STNBStopFlagRef ref);

#ifdef __cplusplus
} //extern "C"
#endif
		
#endif /* NBStopFlag_h */
