
#include "nb/NBFrameworkPch.h"
#include "nb/core/NBStruct.h"
#include "nb/net/NBHttpReqRules.h"
//
#include "nb/core/NBArraySorted.h"

//ToDo: remove only after 'STNBHttpReqRulesDef' was removed.
/*
typedef struct STNBHttpReqRulesOpq_ {
	STNBHttpReqRule	base;     //default rule
	STNBArraySorted	targets;  //STNBHttpReqRuleTarget
} STNBHttpReqRulesOpq;
*/

//ToDo: remove only after 'STNBHttpReqRulesDef' was removed.
/*
void NBHttpReqRules_init(STNBHttpReqRules* obj){
	STNBHttpReqRulesOpq* opq = obj->opaque = NBMemory_allocType(STNBHttpReqRulesOpq);
	NBMemory_setZeroSt(*opq, STNBHttpReqRulesOpq);
	NBArraySorted_init(&opq->targets, sizeof(STNBHttpReqRuleTarget), NBCompare_NBHttpReqRuleTarget);
}
*/

//ToDo: remove only after 'STNBHttpReqRulesDef' was removed.
/*
void NBHttpReqRules_release(STNBHttpReqRules* obj){
	STNBHttpReqRulesOpq* opq = (STNBHttpReqRulesOpq*)obj->opaque;
    //default rule
    {
        NBStruct_stRelease(NBHttpReqRule_getSharedStructMap(), &opq->base, sizeof(opq->base));
    }
    //targets
    {
        SI32 i; for(i = 0; i < opq->targets.use; i++){
            STNBHttpReqRuleTarget* target = NBArraySorted_itmPtrAtIndex(&opq->targets, STNBHttpReqRuleTarget, i);
            NBStruct_stRelease(NBHttpReqRuleTarget_getSharedStructMap(), target, sizeof(*target));
        }
        NBArraySorted_empty(&opq->targets);
        NBArraySorted_release(&opq->targets);
    }
	NBMemory_free(obj->opaque);
	obj->opaque = NULL;
}
*/

//ToDo: remove only after 'STNBHttpReqRulesDef' was removed.
/*
BOOL NBHttpReqRules_loadFromConfig(STNBHttpReqRules* obj, const STNBHttpReqRulesDef* reqsRules){
	BOOL r = TRUE;
	STNBHttpReqRulesOpq* opq = (STNBHttpReqRulesOpq*)obj->opaque;
    //default rule
    {
        NBStruct_stRelease(NBHttpReqRule_getSharedStructMap(), &opq->base, sizeof(opq->base));
        NBStruct_stClone(NBHttpReqRule_getSharedStructMap(), &reqsRules->base, sizeof(reqsRules->base), &opq->base, sizeof(opq->base));
    }
	UI32 i; for(i = 0; i < reqsRules->targetsSz; i++){
		const STNBHttpReqRuleTarget* target = &reqsRules->targets[i];
		if(!NBString_strIsEmpty(target->target)){
			if(NBArraySorted_indexOf(&opq->targets, target, sizeof(*target), NULL) < 0){
                STNBHttpReqRuleTarget target2;
                NBMemory_setZeroSt(target2, STNBHttpReqRuleTarget)
                NBStruct_stClone(NBHttpReqRuleTarget_getSharedStructMap(), target, sizeof(*target), &target2, sizeof(target2));
				NBArraySorted_addValue(&opq->targets, target2);
			}
		}
	}
	return r;
}
*/

//ToDo: remove only after 'STNBHttpReqRulesDef' was removed.
/*
 STNBHttpReqRule NBHttpReqRules_ruleForRequestTarget(const STNBHttpReqRules* obj, const char* requestTarget){
 STNBHttpReqRulesOpq* opq = (STNBHttpReqRulesOpq*)obj->opaque;
 STNBHttpReqRule r = opq->base;
 if(!NBString_strIsEmpty(requestTarget)){
 SI32 idx = -1;
 STNBHttpReqRuleTarget srch;
 srch.target = (char*)requestTarget;
 idx = NBArraySorted_indexOf(&opq->targets, &srch, sizeof(srch), NULL);
 if(idx >= 0){
 r = NBArraySorted_itmValueAtIndex(&opq->targets, STNBHttpReqRuleTarget, idx).rule;
 }
 }
 return r;
 }
 */
