//
//  NBX500Name.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/28/18.
//

#ifndef NBX500Name_h
#define NBX500Name_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBStructMap.h"

typedef struct STNBX500NamePair_ {
	char*	type;
	char*	value;
} STNBX500NamePair;

const STNBStructMap* NBX500NamePair_getSharedStructMap(void);

#endif /* NBX500Name_h */
