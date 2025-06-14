#ifndef NB_MS_EXCHANGE_CLT_H
#define NB_MS_EXCHANGE_CLT_H

#include "nb/NBFrameworkDefs.h"
#include "nb/net/NBSmtpHeader.h"
#include "nb/net/NBSmtpBody.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct STNBMsExchangeClt_ {
	void* opaque;
} STNBMsExchangeClt;

void NBMsExchangeClt_init(STNBMsExchangeClt* obj);
void NBMsExchangeClt_retain(STNBMsExchangeClt* obj);
void NBMsExchangeClt_release(STNBMsExchangeClt* obj);

BOOL NBMsExchangeClt_configure(STNBMsExchangeClt* obj, const char* server, const UI32 port, const char* target, const char* userName, const char* pass, const char* domain, const char* workstation);
BOOL NBMsExchangeClt_mailSendSync(STNBMsExchangeClt* obj, const STNBSmtpHeader* head, const STNBSmtpBody* body);

#ifdef __cplusplus
} //extern "C"
#endif


#endif
