#ifndef NB_HTTP_REQ_RULES_H
#define NB_HTTP_REQ_RULES_H

#include "nb/NBFrameworkDefs.h"
//
#include "nb/net/NBHttpReqRule.h"

//

#ifdef __cplusplus
extern "C" {
#endif

    //ToDo: remove only after 'STNBHttpReqRulesDef' was removed.
    /*
	typedef struct STNBHttpReqRules_ {
		void* opaque;
	} STNBHttpReqRules;

	void NBHttpReqRules_init(STNBHttpReqRules* obj);
	void NBHttpReqRules_release(STNBHttpReqRules* obj);

	BOOL NBHttpReqRules_loadFromConfig(STNBHttpReqRules* obj, const STNBHttpReqRulesDef* rules);

	STNBHttpReqRule NBHttpReqRules_ruleForRequestTarget(const STNBHttpReqRules* obj, const char* requestTarget);
	*/

#ifdef __cplusplus
} //extern "C"
#endif

#endif
