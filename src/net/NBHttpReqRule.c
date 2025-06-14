
#include "nb/NBFrameworkPch.h"
#include "nb/net/NBHttpReqRule.h"
//
#include "nb/core/NBStruct.h"
#include "nb/core/NBMngrStructMaps.h"

//enumMap

/*
STNBEnumMapRecord NBHttpReqRuleTarget_sharedEnumMapRecs[] = {
	{ ENNBHttpReqRule_Deny, "ENNBHttpReqRule_Deny", "deny" }
	, { ENNBHttpReqRule_Accept, "ENNBHttpReqRule_Accept", "accept"}
    , { ENNBHttpReqRule_Redirect, "ENNBHttpReqRule_Redirect", "redir"}
};

STNBEnumMap NBHttpReqRuleTarget_sharedEnumMap = {
	"ENNBHttpReqRule"
	, NBHttpReqRuleTarget_sharedEnumMapRecs
	, (sizeof(NBHttpReqRuleTarget_sharedEnumMapRecs) / sizeof(NBHttpReqRuleTarget_sharedEnumMapRecs[0]))
};

const STNBEnumMap* NBHttpReqRuleTarget_getSharedEnumMap(void){
	return &NBHttpReqRuleTarget_sharedEnumMap;
}
*/

//NBHttpReqRule

/*
STNBStructMapsRec NBHttpReqRule_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBHttpReqRule_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBHttpReqRule_sharedStructMap);
	if(NBHttpReqRule_sharedStructMap.map == NULL){
		STNBHttpReqRule s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBHttpReqRule);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addEnumM(map, s, action, NBHttpReqRuleTarget_getSharedEnumMap());
		NBStructMap_addBoolM(map, s, certIsRequired);
		NBStructMap_addUIntM(map, s, secsIdle);
		NBHttpReqRule_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBHttpReqRule_sharedStructMap);
	return NBHttpReqRule_sharedStructMap.map;
}
*/

//NBHttpReqRule

/*
STNBStructMapsRec NBHttpReqRuleTarget_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBHttpReqRuleTarget_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBHttpReqRuleTarget_sharedStructMap);
	if(NBHttpReqRuleTarget_sharedStructMap.map == NULL){
		STNBHttpReqRuleTarget s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBHttpReqRuleTarget);
		NBStructMap_init(map, sizeof(s));
		//NBStructMap_addEnumM(map, s, path, TSRequestId_getSharedEnumMap());
		NBStructMap_addStrPtrM(map, s, target);
		NBStructMap_addStructM(map, s, rule, NBHttpReqRule_getSharedStructMap());
		NBHttpReqRuleTarget_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBHttpReqRuleTarget_sharedStructMap);
	return NBHttpReqRuleTarget_sharedStructMap.map;
}
*/

//NBHttpReqRulesDef

STNBStructMapsRec NBHttpReqRulesDef_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBHttpReqRulesDef_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBHttpReqRulesDef_sharedStructMap);
	if(NBHttpReqRulesDef_sharedStructMap.map == NULL){
		STNBHttpReqRulesDef s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBHttpReqRulesDef);
		NBStructMap_init(map, sizeof(s));
        NBStructMap_addIntM(map, s, dummy);
        /*
		NBStructMap_addStructM(map, s, base, NBHttpReqRule_getSharedStructMap());
		NBStructMap_addPtrToArrayOfStructM(map, s, targets, targetsSz, ENNBStructMapSign_Unsigned, NBHttpReqRuleTarget_getSharedStructMap());
        */
		NBHttpReqRulesDef_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBHttpReqRulesDef_sharedStructMap);
	return NBHttpReqRulesDef_sharedStructMap.map;
}

//

/*
BOOL NBCompare_NBHttpReqRuleTarget(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
	NBASSERT(dataSz == sizeof(STNBHttpReqRuleTarget))
	if(dataSz == sizeof(STNBHttpReqRuleTarget)){
		const STNBHttpReqRuleTarget* o1 = (const STNBHttpReqRuleTarget*)data1;
		const STNBHttpReqRuleTarget* o2 = (const STNBHttpReqRuleTarget*)data2;
		switch (mode) {
			case ENCompareMode_Equal:
				return NBString_strIsEqual(o1->target, o2->target);
				//return o1->reqId == o2->reqId ? TRUE : FALSE;
			case ENCompareMode_Lower:
				return NBString_strIsLower(o1->target, o2->target);
				//return o1->reqId < o2->reqId ? TRUE : FALSE;
			case ENCompareMode_LowerOrEqual:
				return NBString_strIsLowerOrEqual(o1->target, o2->target);
				//return o1->reqId <= o2->reqId ? TRUE : FALSE;
			case ENCompareMode_Greater:
				return NBString_strIsGreater(o1->target, o2->target);
				//return o1->reqId > o2->reqId ? TRUE : FALSE;
			case ENCompareMode_GreaterOrEqual:
				return NBString_strIsGreaterOrEqual(o1->target, o2->target);
				//return o1->reqId >= o2->reqId ? TRUE : FALSE;
			default:
				NBASSERT(FALSE)
				break;
		}
	}
	return FALSE;
}
*/
