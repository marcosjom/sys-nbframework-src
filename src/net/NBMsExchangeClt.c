
#include "nb/NBFrameworkPch.h"
//Complementary PCH
#include <stdlib.h>		//for rand()
//
#include "nb/net/NBMsExchangeClt.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBThreadMutex.h"
#include "nb/core/NBThreadCond.h"
//
#include "nb/ssl/NBSslContext.h"
#include "nb/ssl/NBSsl.h"
#include "nb/net/NBSocket.h"
//
#include "nb/core/NBXml.h"
#include "nb/net/NBNtln.h"
#include "nb/net/NBHttpRequest.h"
#include "nb/net/NBHttpResponse.h"
#include "nb/crypto/NBBase64.h"
//
typedef struct STNBMsExchangeCltOpq_ {
	//
	STNBSslContextRef sslContext;
	STNBSslRef		ssl;
	STNBSocketRef	socket;
	//Cfg
	struct {
		STNBString	server;
		UI32		port;
		STNBString	target;
		STNBString	user;
		STNBString	pass;
		STNBString	domain;
		STNBString	workstation;
	} cfg;
	//
	BOOL			isBussy;
	STNBThreadMutex	mutex;
	STNBThreadCond	cond;
	//
	SI32			retainCount;
} STNBMsExchangeCltOpq;

void NBMsExchangeClt_init(STNBMsExchangeClt* obj){
	STNBMsExchangeCltOpq* opq = obj->opaque = NBMemory_allocType(STNBMsExchangeCltOpq);
	NBMemory_setZeroSt(*opq, STNBMsExchangeCltOpq);
	//
	opq->isBussy	= FALSE;
	//Cfg
	{
		NBString_init(&opq->cfg.server);
		opq->cfg.port	= 0;
		NBString_init(&opq->cfg.target);
		NBString_init(&opq->cfg.user);
		NBString_init(&opq->cfg.pass);
		NBString_init(&opq->cfg.domain);
		NBString_init(&opq->cfg.workstation);
	}
	//
	NBThreadMutex_init(&opq->mutex);
	NBThreadCond_init(&opq->cond);
	opq->retainCount	= 1;
}

void NBMsExchangeClt_retain(STNBMsExchangeClt* obj){
	STNBMsExchangeCltOpq* opq = (STNBMsExchangeCltOpq*)obj->opaque;
	if(opq != NULL){
		NBThreadMutex_lock(&opq->mutex);
		NBASSERT(opq->retainCount > 0)
		opq->retainCount++;
		NBThreadMutex_unlock(&opq->mutex);
	}
}

void NBMsExchangeClt_release(STNBMsExchangeClt* obj){
	STNBMsExchangeCltOpq* opq = (STNBMsExchangeCltOpq*)obj->opaque;
	if(opq != NULL){
		NBThreadMutex_lock(&opq->mutex);
		NBASSERT(opq->retainCount > 0)
		opq->retainCount--;
		if(opq->retainCount > 0){
			NBThreadMutex_unlock(&opq->mutex);
		} else {
			//Wait until is not bussy
			while(opq->isBussy){
				NBThreadCond_wait(&opq->cond, &opq->mutex);
			}
			//Release network
			{
				if(NBSocket_isSet(opq->socket)){
					NBSocket_close(opq->socket);
					NBSocket_release(&opq->socket);
					NBSocket_null(&opq->socket);
				}
				if(NBSsl_isSet(opq->ssl)){
					NBSsl_release(&opq->ssl);
					NBSsl_null(&opq->ssl);
				}
				if(NBSslContext_isSet(opq->sslContext)){
					NBSslContext_release(&opq->sslContext);
					NBSslContext_null(&opq->sslContext);
				}
			}
			//Cfg
			{
				NBString_release(&opq->cfg.server);
				opq->cfg.port	= 0;
				NBString_release(&opq->cfg.target);
				NBString_release(&opq->cfg.user);
				NBString_release(&opq->cfg.pass);
				NBString_release(&opq->cfg.domain);
				NBString_release(&opq->cfg.workstation);
			}
			NBThreadCond_release(&opq->cond);
			NBThreadMutex_unlock(&opq->mutex);
			NBThreadMutex_release(&opq->mutex);
			NBMemory_free(opq);
		}
	}
}

BOOL NBMsExchangeClt_configure(STNBMsExchangeClt* obj, const char* server, const UI32 port, const char* target, const char* userName, const char* pass, const char* domain, const char* workstation){
	BOOL r = FALSE;
	STNBMsExchangeCltOpq* opq = (STNBMsExchangeCltOpq*)obj->opaque;
	if(opq != NULL){
		if(NBString_strIsEmpty(server) || port <= 0 || NBString_strIsEmpty(target)){
			//Required missing
		} else {
			//Configure
			NBThreadMutex_lock(&opq->mutex);
			if(!opq->isBussy){
				//Set config
				STNBSslContextRef sslContext = NBSslContext_alloc(NULL);
				if(!NBSslContext_create(sslContext, NBSslContext_getClientMode)){
					PRINTF_ERROR("NBSslContext_create failed.\n");
				} else {
					NBString_set(&opq->cfg.server, server);
					opq->cfg.port	= port;
					NBString_set(&opq->cfg.target, target);
					NBString_set(&opq->cfg.user, userName);
					NBString_set(&opq->cfg.pass, pass);
					NBString_set(&opq->cfg.domain, domain);
					NBString_set(&opq->cfg.workstation, workstation);
					//Release network (force reconnection)
					{
						if(NBSocket_isSet(opq->socket)){
							NBSocket_close(opq->socket);
							NBSocket_release(&opq->socket);
							NBSocket_null(&opq->socket);
						}
						if(NBSsl_isSet(opq->ssl)){
							NBSsl_release(&opq->ssl);
							NBSsl_null(&opq->ssl);
						}
						if(NBSslContext_isSet(opq->sslContext)){
							NBSslContext_release(&opq->sslContext);
							NBSslContext_null(&opq->sslContext);
						}
					}
					//Set new
					opq->sslContext = sslContext; NBSslContext_null(&sslContext);
					r = TRUE;
				}
				//Release (if not consumed)
				if(NBSslContext_isSet(sslContext)){
					NBSslContext_release(&sslContext);
					NBSslContext_null(&sslContext);
				}
			}
			NBThreadMutex_unlock(&opq->mutex);
		}
	}
	return r;
}

BOOL NBMsExchangeClt_sendHttpsRequest_(STNBSslRef ssl, STNBHttpRequest* req, const char* server, const SI32 port, const char* target, const char* httpBoundTag, STNBHttpResponse* dstRsp);
BOOL NBMsExchangeClt_sendEWSRequest_(STNBMsExchangeCltOpq* opq, const char* xmlPayload, const char* server, const SI32 port, const char* target, const char* httpBoundTag, STNBHttpResponse* dstRsp, STNBString* dstItemId, STNBString* dstChangeKey);
BOOL NBMsExchangeClt_negotiateNTLMAuth_(STNBSslRef ssl, const char* server, const SI32 port, const char* target, const char* user, const char* pass, const char* domain, const char* workstation, const char* httpBoundTag);

BOOL NBMsExchangeClt_mailSendSync(STNBMsExchangeClt* obj, const STNBSmtpHeader* head, const STNBSmtpBody* body){
	BOOL r = FALSE;
	STNBMsExchangeCltOpq* opq = (STNBMsExchangeCltOpq*)obj->opaque;
	if(opq != NULL){
		NBThreadMutex_lock(&opq->mutex);
		if(!opq->isBussy && NBSslContext_isSet(opq->sslContext)){
			opq->isBussy = TRUE;
			NBThreadCond_broadcast(&opq->cond);
			NBThreadMutex_unlock(&opq->mutex);
			//Send (unlocked)
			{
				const char* httpBoundTag = "aaabbbcccdddMyDBSync";
				SI32 attemptsRemain = 2;
				while(attemptsRemain > 0 && !r){
					attemptsRemain--;
					//Create connection
					if(!NBSocket_isSet(opq->socket) || !NBSsl_isSet(opq->ssl)){
						attemptsRemain = 0; //Do not retry for new connections
						//Release previous
						{
							if(NBSocket_isSet(opq->socket)){
								NBSocket_close(opq->socket);
								NBSocket_release(&opq->socket);
								NBSocket_null(&opq->socket);
							}
							if(NBSsl_isSet(opq->ssl)){
								NBSsl_release(&opq->ssl);
								NBSsl_null(&opq->ssl);
							}
						}
						//Try to connect
						if(opq->cfg.server.length <= 0 || opq->cfg.port <= 0 || opq->cfg.target.length <= 0){
							PRINTF_ERROR("NBMsExchangeClt, not configured yet.\n");
						} else {
							STNBSocketRef socket = NBSocket_alloc(NULL);
							NBSocket_setNoSIGPIPE(socket, TRUE);
							NBSocket_setDelayEnabled(socket, FALSE);
							NBSocket_setCorkEnabled(socket, FALSE);
							//PRINTF_INFO("NBMsExchangeClt, conecting to: '%s:%d'...\n", opq->cfg.server.str, opq->cfg.port);
							if(!NBSocket_connect(socket, opq->cfg.server.str, opq->cfg.port)){
								PRINTF_ERROR("NBMsExchangeClt, NBSocket_connect('%s:%d') failed.\n", opq->cfg.server.str, opq->cfg.port);
							} else {
								//PRINTF_INFO("NBMsExchangeClt, starting ssl-handshake('%s:%d') ...\n", opq->cfg.server.str, opq->cfg.port);
								STNBSslRef ssl = NBSsl_alloc(NULL);
								if(!NBSsl_connect(ssl, opq->sslContext, socket)){
									PRINTF_ERROR("NBMsExchangeClt, NBSsl_connect('%s:%d') failed.\n", opq->cfg.server.str, opq->cfg.port);
								} else {
									//PRINTF_INFO("NBMsExchangeClt, sslConnected to ('%s:%d').\n", opq->cfg.server.str, opq->cfg.port);
									//Negotiate auth with server
									if(!NBMsExchangeClt_negotiateNTLMAuth_(ssl, opq->cfg.server.str, opq->cfg.port, opq->cfg.target.str, opq->cfg.user.str, opq->cfg.pass.str, opq->cfg.domain.str, opq->cfg.workstation.str, httpBoundTag)){
										PRINTF_ERROR("NBMsExchangeClt_negotiateNTLMAuth_('%s:%d') failed.\n", opq->cfg.server.str, opq->cfg.port);
									} else {
										//PRINTF_INFO("NBMsExchangeClt, NTLM auth granted ('%s:%d').\n", opq->cfg.server.str, opq->cfg.port);
										opq->socket = socket; NBSocket_null(&socket);
										opq->ssl = ssl; NBSsl_null(&ssl);
									}
								}
								//Release (if not consumed)
								if(NBSsl_isSet(ssl)){
									NBSsl_release(&ssl);
									NBSsl_null(&ssl);
								}
							}
							//Release (if not consumed)
							if(NBSocket_isSet(socket)){
								NBSocket_close(socket);
								NBSocket_release(&socket);
								NBSocket_null(&socket);
							}
						}
					}
					//Send
					if(NBSocket_isSet(opq->socket) && NBSsl_isSet(opq->ssl)){
						//Attempt to send email
						const BOOL isSaveOnly = (body->attachs.use > 0 ? TRUE : FALSE);
						STNBString xml, itemId, itemChngKey;
						STNBHttpResponse resp;
						NBString_init(&xml);
						NBString_init(&itemId);
						NBString_init(&itemChngKey);
						NBHttpResponse_init(&resp);
						{
							NBString_concat(&xml, "<?xml version=\"1.0\" encoding=\"utf-8\"?>");
							NBString_concat(&xml, "<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:m=\"http://schemas.microsoft.com/exchange/services/2006/messages\" xmlns:t=\"http://schemas.microsoft.com/exchange/services/2006/types\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">");
							NBString_concat(&xml, "<soap:Body>");
							if(isSaveOnly){
								NBString_concat(&xml, "<m:CreateItem MessageDisposition=\"SaveOnly\">");
							} else {
								NBString_concat(&xml, "<m:CreateItem MessageDisposition=\"SendOnly\">");
							}
							NBString_concat(&xml, "<m:Items>");
							NBString_concat(&xml, "<t:Message>");
							NBString_concat(&xml, "<t:Subject>");
							NBXml_concatScaped(&xml, &head->strs.str[head->SUBJECT]);
							NBString_concat(&xml, "</t:Subject>");
							NBString_concat(&xml, "<t:Body BodyType=\"HTML\">");
							NBXml_concatScaped(&xml, &body->strs.str[body->content]);
							NBString_concat(&xml, "</t:Body>");
							if(head->TOs.use > 0){
								SI32 i;
								NBString_concat(&xml, "<t:ToRecipients>");
								for(i = 0; i < head->TOs.use; i++){
									const UI32 pos = NBArray_itmValueAtIndex(&head->TOs, UI32, i);
									NBString_concat(&xml, "<t:Mailbox>");
									NBString_concat(&xml, "<t:EmailAddress>");
									NBString_concat(&xml, &head->strs.str[pos]);
									NBString_concat(&xml, "</t:EmailAddress>");
									NBString_concat(&xml, "</t:Mailbox>");
								}
								NBString_concat(&xml, "</t:ToRecipients>");
							}
							if(head->CCs.use > 0){
								SI32 i;
								NBString_concat(&xml, "<t:CcRecipients>");
								for(i = 0; i < head->CCs.use; i++){
									const UI32 pos = NBArray_itmValueAtIndex(&head->CCs, UI32, i);
									NBString_concat(&xml, "<t:Mailbox>");
									NBString_concat(&xml, "<t:EmailAddress>");
									NBString_concat(&xml, &head->strs.str[pos]);
									NBString_concat(&xml, "</t:EmailAddress>");
									NBString_concat(&xml, "</t:Mailbox>");
								}
								NBString_concat(&xml, "</t:CcRecipients>");
							}
							if(head->CCOs.use > 0){
								SI32 i;
								NBString_concat(&xml, "<t:BccRecipients>");
								for(i = 0; i < head->CCOs.use; i++){
									const UI32 pos = NBArray_itmValueAtIndex(&head->CCOs, UI32, i);
									NBString_concat(&xml, "<t:Mailbox>");
									NBString_concat(&xml, "<t:EmailAddress>");
									NBString_concat(&xml, &head->strs.str[pos]);
									NBString_concat(&xml, "</t:EmailAddress>");
									NBString_concat(&xml, "</t:Mailbox>");
								}
								NBString_concat(&xml, "</t:BccRecipients>");
							}
							NBString_concat(&xml, "</t:Message>");
							NBString_concat(&xml, "</m:Items>");
							NBString_concat(&xml, "</m:CreateItem>");
							NBString_concat(&xml, "</soap:Body>");
							NBString_concat(&xml, "</soap:Envelope>");
							//PRINTF_INFO("%s.\n", xml.str);
						}
						if(!NBMsExchangeClt_sendEWSRequest_(opq, xml.str, opq->cfg.server.str, opq->cfg.port, opq->cfg.target.str, httpBoundTag, &resp, &itemId, &itemChngKey)){
							PRINTF_ERROR("NBMsExchangeClt_sendHttpsRequest_('%s:%d') failed.\n", opq->cfg.server.str, opq->cfg.port);
							if(resp.code >= 100){
								attemptsRemain = 0; //Do not retry for full repsonses
							}
						} else {
							//PRINTF_INFO("Mail, repsonse: %d.\n", resp.code);
							//PRINTF_INFO("Received:\nHEAD\n%s\nBODY\n%s\n----\n", resp.header.str, resp.body.str);
							if(!isSaveOnly){
								//PRINTF_INFO("Mail, repsonse: %d.\n", resp.code);
								attemptsRemain = 0; //Do not retry, sent!
								r = TRUE;
							} else {
								if(itemId.length <= 0){
									PRINTF_ERROR("Mail, expected 'ItemId/Id' tag.\n");
								} else if(itemChngKey.length <= 0){
									PRINTF_ERROR("Mail, expected 'ItemId/ChangeKey' tag.\n");
								} else {
									STNBHttpResponse resp;
									STNBString xml;
									NBHttpResponse_init(&resp);
									NBString_init(&xml);
									//PRINTF_INFO("Mail, saved itm-id('%s')-changeKey('%s').\n", itemId.str, itemChngKey.str);
									//Build request
									{
										NBString_concat(&xml, "<?xml version=\"1.0\" encoding=\"utf-8\"?>");
										NBString_concat(&xml, "<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:t=\"http://schemas.microsoft.com/exchange/services/2006/types\">");
										NBString_concat(&xml, "<soap:Body>");
										NBString_concat(&xml, "<CreateAttachment xmlns=\"http://schemas.microsoft.com/exchange/services/2006/messages\" xmlns:t=\"http://schemas.microsoft.com/exchange/services/2006/types\">");
										NBString_concat(&xml, "<ParentItemId Id=\"");
										NBString_concat(&xml, itemId.str);
										NBString_concat(&xml, "\"/>");
										if(body->attachs.use > 0){
											BOOL tagOpened = FALSE;
											SI32 i; for(i = 0; i < body->attachs.use; i++){
												STNBSmtpBodyAttach* aa = NBArray_itmPtrAtIndex(&body->attachs, STNBSmtpBodyAttach, i);
												if(aa != NULL){
													const char* mimeType	= &body->strs.str[aa->mimeType];
													const char* filename	= &body->strs.str[aa->filename];
													const char* dataBin		= &body->strs.str[aa->dataBin];
													if(!NBString_strIsEmpty(filename) && aa->dataBinSz > 0){
														if(!tagOpened){
															NBString_concat(&xml, "<Attachments>");
															tagOpened = TRUE;
														}
														//NBString_concat(&xml, "<t:FileAttachment><t:Name>FileAttachment.txt</t:Name><t:IsInline>false</t:IsInline><t:IsContactPhoto>false</t:IsContactPhoto><t:Content>VGhpcyBpcyBhIGZpbGUgYXR0YWNobWVudC4=</t:Content></t:FileAttachment>");
														NBString_concat(&xml, "<t:FileAttachment>");
														NBString_concat(&xml, "<t:Name>");
														NBString_concat(&xml, filename);
														NBString_concat(&xml, "</t:Name>");
														if(!NBString_strIsEmpty(mimeType)){
															NBString_concat(&xml, "<t:ContentType>");
															NBString_concat(&xml, mimeType);
															NBString_concat(&xml, "</t:ContentType>");
														}
														NBString_concat(&xml, "<t:IsInline>false</t:IsInline>");
														NBString_concat(&xml, "<t:IsContactPhoto>false</t:IsContactPhoto>");
														NBString_concat(&xml, "<t:Content>");
														NBBase64_codeBytes(&xml, dataBin, aa->dataBinSz);
														NBString_concat(&xml, "</t:Content>");
														NBString_concat(&xml, "</t:FileAttachment>");
													}
												}
											}
											if(tagOpened){
												NBString_concat(&xml, "</Attachments>");
											}
										}
										NBString_concat(&xml, "</CreateAttachment>");
										NBString_concat(&xml, "</soap:Body>");
										NBString_concat(&xml, "</soap:Envelope>");
										//PRINTF_INFO("%s.\n", xml.str);
									}
									//Send
									if(!NBMsExchangeClt_sendEWSRequest_(opq, xml.str, opq->cfg.server.str, opq->cfg.port, opq->cfg.target.str, httpBoundTag, &resp, &itemId, &itemChngKey)){
										PRINTF_ERROR("NBMsExchangeClt_sendHttpsRequest_('%s:%d') failed.\n", opq->cfg.server.str, opq->cfg.port);
										if(resp.code >= 100){
											attemptsRemain = 0; //Do not retry for full repsonses
										}
									} else {
										//PRINTF_INFO("Mail, repsonse: %d.\n", resp.code);
										//PRINTF_INFO("Received:\nHEAD\n%s\nBODY\n%s\n----\n", resp.header.str, resp.body.str);
										//Send email
										STNBHttpResponse resp;
										STNBString xml;
										NBHttpResponse_init(&resp);
										NBString_init(&xml);
										//PRINTF_INFO("Mail, saved itm-id('%s')-changeKey('%s').\n", itemId.str, itemChngKey.str);
										//Build request
										{
											NBString_concat(&xml, "<?xml version=\"1.0\" encoding=\"utf-8\"?>");
											NBString_concat(&xml, "<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:m=\"http://schemas.microsoft.com/exchange/services/2006/messages\" xmlns:t=\"http://schemas.microsoft.com/exchange/services/2006/types\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">");
											NBString_concat(&xml, "<soap:Body>");
											NBString_concat(&xml, "<m:GetItem>");
											NBString_concat(&xml, "<m:ItemShape>");
											NBString_concat(&xml, "<t:BaseShape>IdOnly</t:BaseShape>");
											NBString_concat(&xml, "</m:ItemShape>");
											NBString_concat(&xml, "<m:ItemIds>");
											NBString_concat(&xml, "<t:ItemId Id=\"");
											NBString_concat(&xml, itemId.str);
											NBString_concat(&xml, "\" />");
											NBString_concat(&xml, "</m:ItemIds>");
											NBString_concat(&xml, "</m:GetItem>");
											NBString_concat(&xml, "</soap:Body>");
											NBString_concat(&xml, "</soap:Envelope>");
											//PRINTF_INFO("%s.\n", xml.str);
										}
										//Send
										NBString_empty(&itemId);
										NBString_empty(&itemChngKey);
										if(!NBMsExchangeClt_sendEWSRequest_(opq, xml.str, opq->cfg.server.str, opq->cfg.port, opq->cfg.target.str, httpBoundTag, &resp, &itemId, &itemChngKey)){
											PRINTF_ERROR("NBMsExchangeClt_sendHttpsRequest_('%s:%d') failed.\n", opq->cfg.server.str, opq->cfg.port);
											if(resp.code >= 100){
												attemptsRemain = 0; //Do not retry for full repsonses
											}
										} else {
											//PRINTF_INFO("Mail, repsonse: %d.\n", resp.code);
											//PRINTF_INFO("Received:\nHEAD\n%s\nBODY\n%s\n----\n", resp.header.str, resp.body.str);
											if(itemId.length <= 0){
												PRINTF_ERROR("Mail, expected 'ItemId/Id' tag.\n");
											} else if(itemChngKey.length <= 0){
												PRINTF_ERROR("Mail, expected 'ItemId/ChangeKey' tag.\n");
											} else {
												//Send email
												STNBHttpResponse resp;
												STNBString xml;
												NBHttpResponse_init(&resp);
												NBString_init(&xml);
												//PRINTF_INFO("Mail, saved itm-id('%s')-changeKey('%s').\n", itemId.str, itemChngKey.str);
												//Build request
												{
													NBString_concat(&xml, "<?xml version=\"1.0\" encoding=\"utf-8\"?>");
													NBString_concat(&xml, "<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:m=\"http://schemas.microsoft.com/exchange/services/2006/messages\" xmlns:t=\"http://schemas.microsoft.com/exchange/services/2006/types\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">");
													NBString_concat(&xml, "<soap:Body>");
													NBString_concat(&xml, "<m:SendItem SaveItemToFolder=\"true\">");
													NBString_concat(&xml, "<m:ItemIds>");
													NBString_concat(&xml, "<t:ItemId Id=\"");
													NBString_concat(&xml, itemId.str);
													NBString_concat(&xml, "\" ChangeKey=\"");
													NBString_concat(&xml, itemChngKey.str);
													NBString_concat(&xml, "\" />");
													NBString_concat(&xml, "</m:ItemIds>");
													NBString_concat(&xml, "<m:SavedItemFolderId>");
													NBString_concat(&xml, "<t:DistinguishedFolderId Id=\"sentitems\" />");
													NBString_concat(&xml, "</m:SavedItemFolderId>");
													NBString_concat(&xml, "</m:SendItem>");
													NBString_concat(&xml, "</soap:Body>");
													NBString_concat(&xml, "</soap:Envelope>");
													//PRINTF_INFO("%s.\n", xml.str);
												}
												//Send
												NBString_empty(&itemId);
												NBString_empty(&itemChngKey);
												if(!NBMsExchangeClt_sendEWSRequest_(opq, xml.str, opq->cfg.server.str, opq->cfg.port, opq->cfg.target.str, httpBoundTag, &resp, &itemId, &itemChngKey)){
													PRINTF_ERROR("NBMsExchangeClt_sendHttpsRequest_('%s:%d') failed.\n", opq->cfg.server.str, opq->cfg.port);
													if(resp.code >= 100){
														attemptsRemain = 0; //Do not retry for full repsonses
													}
												} else {
													//PRINTF_INFO("Mail, repsonse: %d.\n", resp.code);
													//PRINTF_INFO("Received:\nHEAD\n%s\nBODY\n%s\n----\n", resp.header.str, resp.body.str);
													PRINTF_INFO("Mail, sent.\n");
													attemptsRemain = 0; //Do not retry, sent!attemptsRemain = 0; //Do not retry, sent!
													r = TRUE;
												}
												NBString_release(&xml);
												NBHttpResponse_release(&resp);
											}
										}
										NBString_release(&xml);
										NBHttpResponse_release(&resp);
									}
									NBString_release(&xml);
									NBHttpResponse_release(&resp);
								}
							}
						}
						NBHttpResponse_release(&resp);
						NBString_release(&itemId);
						NBString_release(&itemChngKey);
						NBString_release(&xml);
					}
				}
			}
			NBThreadMutex_lock(&opq->mutex);
			opq->isBussy = FALSE;
			NBThreadCond_broadcast(&opq->cond);
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
	return r;
}

BOOL NBMsExchangeClt_sendEWSRequest_(STNBMsExchangeCltOpq* opq, const char* xmlPayload, const char* server, const SI32 port, const char* target, const char* httpBoundTag, STNBHttpResponse* dstRsp, STNBString* dstItemId, STNBString* dstChangeKey){
	BOOL r = FALSE;
	if(NBSsl_isSet(opq->ssl) && !NBString_strIsEmpty(xmlPayload) && dstRsp != NULL){
		STNBHttpRequest req;
		NBHttpRequest_init(&req);
		NBHttpRequest_setContentType(&req, "text/xml");
		NBHttpRequest_setContent(&req, xmlPayload);
		if(!NBMsExchangeClt_sendHttpsRequest_(opq->ssl, &req, opq->cfg.server.str, opq->cfg.port, opq->cfg.target.str, httpBoundTag, dstRsp)){
			PRINTF_ERROR("NBMsExchangeClt_sendHttpsRequest_('%s:%d') failed.\n", opq->cfg.server.str, opq->cfg.port);
			//Force retry
			if(NBSocket_isSet(opq->socket)){
				NBSocket_close(opq->socket);
				NBSocket_release(&opq->socket);
				NBSocket_null(&opq->socket);
			}
			if(NBSsl_isSet(opq->ssl)){
				NBSsl_release(&opq->ssl);
				NBSsl_null(&opq->ssl);
			}
		} else {
			if(dstRsp->code < 200 || dstRsp->code >= 300){
				PRINTF_ERROR("Expected 2XX response for mailSend, received: %d [start-of-logId:NBMsExchangeClt_sendEWSRequest_]---\n", dstRsp->code);
				PRINTF_ERROR("HEAD---%s---\n", dstRsp->header.str);
				PRINTF_ERROR("BODY---%s---\n", dstRsp->body.str);
				PRINTF_INFO("---[end-of-logId:NBMsExchangeClt_sendEWSRequest_].\n");
				if(NBSocket_isSet(opq->socket)){
					NBSocket_close(opq->socket);
					NBSocket_release(&opq->socket);
					NBSocket_null(&opq->socket);
				}
				if(NBSsl_isSet(opq->ssl)){
					NBSsl_release(&opq->ssl);
					NBSsl_null(&opq->ssl);
				}
			} else {
				//ItemId Id
				{
					const char* idTag	= "ItemId Id=\"";
					const SI32 idTagSz	= NBString_strLenBytes(idTag);
					SI32 iIdStart = 0, iIdEnd = 0;
					iIdStart = NBString_strIndexOf(dstRsp->body.str, idTag, 0);
					if(iIdStart >= 0){
						iIdEnd = NBString_strIndexOf(dstRsp->body.str, "\"", iIdStart + idTagSz);
						if(iIdStart < iIdEnd){
							if(dstItemId != NULL){
								NBString_setBytes(dstItemId, &dstRsp->body.str[iIdStart + idTagSz], (iIdEnd - iIdStart - idTagSz));
								//PRINTF_INFO("ItemId Id: '%s'.\n", dstItemId->str);
							}
						}
					}
				}
				//ItemId ChangeKey
				{
					const char* chgTag	= "ChangeKey=\"";
					const SI32 chTagSz	= NBString_strLenBytes(chgTag);
					SI32 iChTagStart = 0, iChTagEnd = 0;
					iChTagStart = NBString_strIndexOf(dstRsp->body.str, chgTag, 0);
					if(iChTagStart >= 0){
						iChTagEnd = NBString_strIndexOf(dstRsp->body.str, "\"", iChTagStart + chTagSz);
						if(iChTagStart < iChTagEnd){
							if(dstChangeKey != NULL){
								NBString_setBytes(dstChangeKey, &dstRsp->body.str[iChTagStart + chTagSz], (iChTagEnd - iChTagStart - chTagSz));
								//PRINTF_INFO("ChangeKey: '%s'.\n", dstChangeKey->str);
							}
						}
					}
				}
				//
				r = TRUE;
			}
		}
		NBHttpRequest_release(&req);
	}
	return r;
}

BOOL NBMsExchangeClt_sendHttpsRequest_(STNBSslRef ssl, STNBHttpRequest* req, const char* server, const SI32 port, const char* target, const char* httpBoundTag, STNBHttpResponse* dstRsp){
	BOOL r = FALSE;
	{
		STNBString tmpOut, tmpIn;
		NBString_initWithSz(&tmpOut, 4096, 4096, 0.1f);
		NBString_initWithSz(&tmpIn, 4096, 4096, 0.1f);
		//Build HTTP msg
		{
			UI32 iStep = 0;
			NBHttpRequest_concatHeaderStart(req, &tmpOut, server, target, NULL, httpBoundTag);
			while(NBHttpRequest_concatBodyContinue(req, &tmpOut, httpBoundTag, NULL, NULL, iStep++)){
				//
			}
		}
		//Send
		if(NBSsl_write(ssl, tmpOut.str, tmpOut.length) != tmpOut.length){
			PRINTF_ERROR("NBSsl_write('%s:%d') failed.\n", server, port);
		} else {
			//NBString_replace(&tmpOut, "\r\n", "\n");
			//PRINTF_INFO("Request sent:\n%s.\n", tmpOut.str);
			//Receive response
			char buff[1024];
			do {
				const SI32 rcvd = NBSsl_read(ssl, buff, 1024);
				if(rcvd <= 0){
					break;
				} else {
					NBString_concatBytes(&tmpIn, buff, rcvd);
					if(!NBHttpResponse_consumeBytes(dstRsp, buff, rcvd, "HTTP")){
						break;
					} else if(dstRsp->_transfEnded){
						//NBString_replace(&tmpIn, "\r\n", "\n");
						//PRINTF_INFO("Response recvd:\n%s.\n", tmpIn.str);
						r = TRUE;
						break;
					}
				}
			} while(1);
		}
		NBString_release(&tmpIn);
		NBString_release(&tmpOut);
	}
	return r;
}

BOOL NBMsExchangeClt_negotiateNTLMAuth_(STNBSslRef ssl, const char* server, const SI32 port, const char* target, const char* user, const char* pass, const char* domain, const char* workstation, const char* httpBoundTag){
	BOOL r = FALSE;
	//Send initial request
	STNBHttpRequest req;
	STNBHttpResponse resp;
	NBHttpRequest_init(&req);
	NBHttpResponse_init(&resp);
	if(!NBMsExchangeClt_sendHttpsRequest_(ssl, &req, server, port, target, httpBoundTag, &resp)){
		PRINTF_ERROR("NBMsExchangeClt_sendHttpsRequest_('%s:%d') failed.\n", server, port);
	} else {
		if(resp.code >= 200 && resp.code < 300){
			//Already have access
			r = TRUE;
		} else if(resp.code != 401){
			PRINTF_ERROR("NBMsExchangeClt_sendHttpsRequest_('%s:%d') expected 2XX or 401.\n", server, port);
		} else {
			//Response expected
			//PRINTF_INFO("Response 401 received.\n");
			STNBString ngBin; //STNTLM_NEGOTIATE_MESSAGE with payload
			STNBHttpRequest req;
			STNBHttpResponse resp;
			NBString_init(&ngBin);
			NBHttpRequest_init(&req);
			NBHttpResponse_init(&resp);
			{
				STNBString param;
				NBString_init(&param);
				NBString_concat(&param, "NTLM ");
				{
					STNTLM_NEGOTIATE_MESSAGE msg;
					NBMemory_setZeroSt(msg, STNTLM_NEGOTIATE_MESSAGE);
					msg.signature[0]	= 'N'; msg.signature[1] = 'T'; msg.signature[2] = 'L'; msg.signature[3] = 'M';
					msg.signature[4]	= 'S'; msg.signature[5] = 'S'; msg.signature[6] = 'P'; msg.signature[7] = '\0';
					msg.messageType		= 0x00000001; //must be  0x00000001
					msg.negotiateFlags	= NTLMSSP_NEGOTIATE_TARGET_INFO | NTLMSSP_NEGOTIATE_NTLM | NTLMSSP_NEGOTIATE_ALWAYS_SIGN | NTLMSSP_NEGOTIATE_UNICODE;
					msg.version			= NBNtln_getDefaultVersionClt();
					NBString_concatBytes(&ngBin, (const char*)&msg, sizeof(msg));
					NBBase64_codeBytes(&param, ngBin.str, ngBin.length);
					//PRINT
					/*{
						STNBString tmp2;
						NBString_init(&tmp2);
						NBString_concatBytesHex(&tmp2, ngBin.str, ngBin.length);
						PRINTF_INFO("STNTLM_NEGOTIATE_MESSAGE: '%s'.\n", tmp2.str);
						NBString_release(&tmp2);
					}*/
				}
				NBHttpRequest_addHeader(&req, "Authorization", param.str);
				NBString_release(&param);
			}
			if(!NBMsExchangeClt_sendHttpsRequest_(ssl, &req, server, port, target, httpBoundTag, &resp)){
				PRINTF_ERROR("NBMsExchangeClt_sendHttpsRequest_('%s:%d') failed.\n", server, port);
			} else {
				if(resp.code != 401){
					PRINTF_ERROR("NBMsExchangeClt, expected 401 response after STNTLM_NEGOTIATE_MESSAGE.\n");
				} else {
					//PRINTF_INFO("Response 401 received.\n");
					//Search for NTLM payload
					STNBString ch64;
					NBString_init(&ch64);
					{
						SI32 i; for(i = 0; i < resp.headers.use; i++){
							const STNBHttpRespHeadr* head = NBArray_itmPtrAtIndex(&resp.headers, STNBHttpRespHeadr, i);
							if(NBString_isEqual(&head->name, "WWW-Authenticate")){
								if(NBString_startsWith(&head->value, "NTLM ")){
									if(head->value.length > 5){
										const char* v = &head->value.str[5];
										UI32 b64Len = 0;
										while(NBBase64_isToken(v[b64Len])){
											b64Len++;
										}
										if(b64Len > 0){
											NBString_setBytes(&ch64, v, b64Len);
											//PRINTF_INFO("NTLM base payload: '%s'.\n", ch64.str);
										}
									}
								}
							}
						}
					}
					if(ch64.length <= 0){
						PRINTF_ERROR("NBMsExchangeClt, expected NTLM base64 payload from server.\n");
					} else {
						STNBString chBin; //STNTLM_CHALLENGE_MESSAGE with payload
						NBString_init(&chBin);
						if(!NBBase64_decodeBytes(&chBin, ch64.str, ch64.length)){
							PRINTF_ERROR("NBMsExchangeClt, NTLM base64 payload is not valid.\n");
						} else if(chBin.length < sizeof(STNTLM_CHALLENGE_MESSAGE)){
							PRINTF_ERROR("NBMsExchangeClt, NTLM base64 payload expected sizeof(NTLM_CHALLENGE_MESSAGE) minimun.\n");
						} else {
							const UI64 clientChallenge = (((UI64)rand()) << 16) | ((UI64)rand());
							STNBString authBin;
							NBString_init(&authBin);
							if(!NBNtln_concatAuthMessageBin(ngBin.str, ngBin.length, chBin.str, chBin.length, user, pass, domain, workstation, clientChallenge, &authBin)){
								PRINTF_ERROR("NBMsExchangeClt, NTLM NBNtln_concatAuthMessageBin failed.\n");
							} else {
								//PRINTF_INFO("NTLM NBNtln_concatAuthMessageBin success.\n");
								STNBHttpRequest req;
								STNBHttpResponse resp;
								NBHttpRequest_init(&req);
								NBHttpResponse_init(&resp);
								{
									STNBString param;
									NBString_init(&param);
									NBString_concat(&param, "NTLM ");
									NBBase64_codeBytes(&param, authBin.str, authBin.length);
									NBHttpRequest_addHeader(&req, "Authorization", param.str);
									NBString_release(&param);
								}
								if(!NBMsExchangeClt_sendHttpsRequest_(ssl, &req, server, port, target, httpBoundTag, &resp)){
									PRINTF_ERROR("NBMsExchangeClt_sendHttpsRequest_('%s:%d') failed.\n", server, port);
								} else {
									if(resp.code < 200 || resp.code >= 300){
										PRINTF_ERROR("NBMsExchangeClt, expected 2XX response for authenticate, received: %d.\n", resp.code);
									} else {
										PRINTF_INFO("NBMsExchangeClt, authenticated-to('%s:%d').\n", server, port);
										r = TRUE;
									}
								}
								NBHttpResponse_release(&resp);
								NBHttpRequest_release(&req);
							}
							NBString_release(&authBin);
						}
						NBString_release(&chBin);
					}
					NBString_release(&ch64);
				}
			}
			NBHttpResponse_release(&resp);
			NBHttpRequest_release(&req);
			NBString_release(&ngBin);
		}
	}
	NBHttpResponse_release(&resp);
	NBHttpRequest_release(&req);
	return r;
}
