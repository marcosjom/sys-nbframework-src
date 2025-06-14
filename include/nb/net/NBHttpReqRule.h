#ifndef NB_HTTP_REQ_RULE_H
#define NB_HTTP_REQ_RULE_H

#include "nb/NBFrameworkDefs.h"
//
#include "nb/core/NBEnumMap.h"
#include "nb/core/NBStructMap.h"

//

#ifdef __cplusplus
extern "C" {
#endif

//Request rule action
//ToDo: remove only after 'STNBHttpReqRulesDef' was removed.
/*
typedef enum ENNBHttpReqRuleTarget_ {
	ENNBHttpReqRule_Deny = 0,
	ENNBHttpReqRule_Accept,
    ENNBHttpReqRule_Redirect,  //redirect to 'redirPath'
    //
    ENNBHttpReqRule_Count
} ENNBHttpReqRule;

const STNBEnumMap* NBHttpReqRuleTarget_getSharedEnumMap(void);
*/

//Request rule
//ToDo: remove only after 'STNBHttpReqRulesDef' was removed.
/*
typedef struct STNBHttpReqRule_ {
	ENNBHttpReqRule action;
    char*           path;           //path as rule-param, in case or 'redir' is used as destination 'https://path.dest/etc'.
	BOOL			certIsRequired;	//Requires or validates a certificate
	UI64			secsIdle;		//close conn after this amount of secs iddle (zero to use global value)
} STNBHttpReqRule;

const STNBStructMap* NBHttpReqRule_getSharedStructMap(void);
*/

//Request rule action
//ToDo: remove only after 'STNBHttpReqRulesDef' was removed.
/*
typedef struct STNBHttpReqRuleTarget_ {
    char*           target;
	STNBHttpReqRule rule;
} STNBHttpReqRuleTarget;

const STNBStructMap* NBHttpReqRuleTarget_getSharedStructMap(void);

BOOL NBCompare_NBHttpReqRuleTarget(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz);
*/

//Requests rules

//dummy struct to warn about old design, and to move this code to user's side.
typedef struct STNBHttpReqRulesDef_ {
    SI32                    dummy;
    /*
	STNBHttpReqRule         base;	    //default action
	STNBHttpReqRuleTarget*	targets;	//explicit rules
	UI32 					targetsSz;
    */
} STNBHttpReqRulesDef;

const STNBStructMap* NBHttpReqRulesDef_getSharedStructMap(void);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
