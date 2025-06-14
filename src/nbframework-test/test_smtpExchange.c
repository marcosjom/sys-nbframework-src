//
//  test_smtpExchange.c
//  nbframework-test-osx
//
//  Created by Marcos Ortega on 19/12/21.
//

#include "test_smtpExchange.h"

//Exchange Mail send test
/*{
	const char* user			= "noreply@signitsafe.ch";
	const char* pass			= "Noreply2019";
	const char* domain			= "";
	const char* workstation		= "";
	const char* server			= "exchange.omcolo.net";
	const SI32 port				= 443;
	const char* target			= "/EWS/Exchange.asmx";
	const char* httpBoundTag	= "aaabbbcccdddMyDBSync";
	const char* sendEmailReqXml	= "<?xml version=\"1.0\" encoding=\"utf-8\"?>\
<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\" \
xmlns:t=\"http://schemas.microsoft.com/exchange/services/2006/types\">\
<soap:Body>\
<CreateItem MessageDisposition=\"SendOnly\" xmlns=\"http://schemas.microsoft.com/exchange/services/2006/messages\">\
<SavedItemFolderId>\
<t:DistinguishedFolderId Id=\"drafts\" />\
</SavedItemFolderId>\
<Items>\
<t:Message>\
<t:ItemClass>IPM.Note</t:ItemClass>\
<t:Subject>Signitsafe's code</t:Subject>\
<t:Body BodyType=\"HTML\">&lt;img src=&quot;https://www.signitsafe.ch/_logos/signit.logo.red@0.5x.png&quot; width=&quot;184&quot; height=&quot;48&quot;&gt;&lt;br&gt;&lt;b&gt;Welcome,&lt;/b&gt;&lt;br&gt;&lt;br&gt;Your confirmation code is &lt;b&gt;&apos;0000&apos;&lt;/b&gt;.&lt;br&gt;&lt;br&gt;Have a nice day,&lt;br&gt;Signitsafe&lt;br&gt;</t:Body>\
<t:ToRecipients>\
<t:Mailbox>\
<t:EmailAddress>mortegam@nibsa.com.ni</t:EmailAddress>\
</t:Mailbox>\
</t:ToRecipients>\
<t:IsRead>false</t:IsRead>\
</t:Message>\
</Items>\
</CreateItem>\
</soap:Body>\
</soap:Envelope>";
	{
		STNBSslContextRef sslContext = NBSslContext_alloc(NULL);
		if(!NBSslContext_create(sslContext, NBSslContext_getClientMode)){
			PRINTF_ERROR("NBSslContext_create failed.\n");
		} else {
			STNBSocketRef socket = NBSocket_alloc(NULL);
			NBSocket_setNoSIGPIPE(&socket, TRUE);
			NBSocket_setDelayEnabled(&socket, FALSE);
			NBSocket_setCorkEnabled(&socket, FALSE);
			if(!NBSocket_connect(&socket, server, port)){
				PRINTF_ERROR("NBSocket_connect('%s:%d') failed.\n", server, port);
			} else {
				STNBSslRef ssl = NBSsl_alloc(NULL);
				if(!NBSsl_connect(ssl, &sslContext, &socket)){
					PRINTF_ERROR("NBSsl_connect('%s:%d') failed.\n", server, port);
				} else {
					//Send initial request
					STNBHttpRequest req;
					STNBHttpResponse resp;
					NBHttpRequest_init(&req);
					NBHttpResponse_init(&resp);
					if(!_sendRequest(&ssl, &req, server, port, target, httpBoundTag, &resp)){
						PRINTF_ERROR("_sendRequest('%s:%d') failed.\n", server, port);
					} else {
						if(resp.code != 401){
							PRINTF_ERROR("Expected 401 response.\n");
						} else {
							PRINTF_INFO("Response 401 received.\n");
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
									{
										STNBString tmp2;
										NBString_init(&tmp2);
										NBString_concatBytesHex(&tmp2, ngBin.str, ngBin.length);
										PRINTF_INFO("STNTLM_NEGOTIATE_MESSAGE: '%s'.\n", tmp2.str);
										NBString_release(&tmp2);
									}
								}
								NBHttpRequest_addHeader(&req, "Authorization", param.str);
								NBString_release(&param);
							}
							if(!_sendRequest(ssl, &req, server, port, target, httpBoundTag, &resp)){
								PRINTF_ERROR("_sendRequest('%s:%d') failed.\n", server, port);
							} else {
								if(resp.code != 401){
									PRINTF_ERROR("Expected 401 response.\n");
								} else {
									PRINTF_INFO("Response 401 received.\n");
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
															PRINTF_INFO("NTLM base payload: '%s'.\n", ch64.str);
														}
													}
												}
											}
										}
									}
									if(ch64.length <= 0){
										PRINTF_ERROR("Expected NTLM base64 payload from server.\n");
									} else {
										STNBString chBin; //STNTLM_CHALLENGE_MESSAGE with payload
										NBString_init(&chBin);
										if(!NBBase64_decodeBytes(&chBin, ch64.str, ch64.length)){
											PRINTF_ERROR("NTLM base64 payload is not valid.\n");
										} else if(chBin.length < sizeof(STNTLM_CHALLENGE_MESSAGE)){
											PRINTF_ERROR("NTLM base64 payload expected sizeof(NTLM_CHALLENGE_MESSAGE) minimun.\n");
										} else {
											const UI64 clientChallenge = (((UI64)rand()) << 16) | ((UI64)rand());
											STNBString authBin;
											NBString_init(&authBin);
											if(!NBNtln_concatAuthMessageBin(ngBin.str, ngBin.length, chBin.str, chBin.length, user, pass, domain, workstation, clientChallenge, &authBin)){
												PRINTF_ERROR("NTLM NBNtln_concatAuthMessageBin failed.\n");
											} else {
												PRINTF_INFO("NTLM NBNtln_concatAuthMessageBin success.\n");
												STNBHttpRequest req;
												STNBHttpResponse resp;
												NBString_init(&ngBin);
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
												if(!_sendRequest(&ssl, &req, server, port, target, httpBoundTag, &resp)){
													PRINTF_ERROR("_sendRequest('%s:%d') failed.\n", server, port);
												} else {
													if(resp.code < 200 || resp.code >= 300){
														PRINTF_ERROR("Expected 2XX response for authenticate, received: %d.\n", resp.code);
													} else {
														PRINTF_INFO("Response received: %d, sending mail...\n", resp.code);
														STNBHttpRequest req;
														STNBHttpResponse resp;
														NBString_init(&ngBin);
														NBHttpRequest_init(&req);
														NBHttpResponse_init(&resp);
														{
															NBHttpRequest_setContentType(&req, "text/xml");
															NBHttpRequest_setContent(&req, sendEmailReqXml);
														}
														/ *{
															STNBString param;
															NBString_init(&param);
															NBString_concat(&param, "NTLM ");
															NBBase64_codeBytes(&param, authBin.str, authBin.length);
															NBHttpRequest_addHeader(&req, "Authorization", param.str);
															NBString_release(&param);
														}* /
														if(!_sendRequest(&ssl, &req, server, port, target, httpBoundTag, &resp)){
															PRINTF_ERROR("_sendRequest('%s:%d') failed.\n", server, port);
														} else {
															if(resp.code < 200 || resp.code >= 300){
																PRINTF_ERROR("Expected 2XX response for mailSend, received: %d.\n", resp.code);
															} else {
																PRINTF_INFO("Response received: %d, mail sent...\n", resp.code);
															}
														}
														NBHttpResponse_release(&resp);
														NBHttpRequest_release(&req);
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
				}
				NBSsl_release(&ssl);
			}
			NBSocket_release(&socket);
		}
		NBSslContext_release(&sslContext);
	}
	//
	//
	//
	//
	//
	/ *STNBString authPlain, authB64, headResp;
	NBString_init(&authPlain);
	NBString_init(&authB64);
	NBString_init(&headResp);
	NBString_concat(&authPlain, "noreply@signitsafe.ch");
	NBString_concat(&authPlain, ":");
	NBString_concat(&authPlain, "Noreply2019");
	NBString_concat(&authB64, "Basic ");
	NBBase64_code(&authB64, authPlain.str);
	{
		STNBHttpRequest req;
		NBHttpRequest_init(&req);
		NBHttpRequest_setContentType(&req, "text/xml");
		//NBHttpRequest_addHeader(&req, "Authorization", authB64.str);
		//Authorization: Basic YWxhZGRpbjpvcGVuc2VzYW1l
		NBHttpRequest_setContent(&req, sendEmailReqXml);
		{
			STNBHttpClient clt;
			NBHttpClient_init(&clt);
			if(!NBHttpClient_executeSync(&clt, "exchange.omcolo.net", 443, "/EWS/Exchange.asmx", TRUE, &req, NULL, ENNBHttpRedirMode_None)){
				PRINTF_ERROR("NBHttpClient_executeSync failed.\n");
			} else {
				NBString_empty(&headResp);
				NBHttpHeader_concatHeader(clt.query.response., &headResp);
				NBString_replace(&headResp, "\r\n", "\n");
				PRINTF_INFO("Response received.\n", headResp.str);
			}
			NBHttpClient_release(&clt);
		}
		NBHttpRequest_release(&req);
	}
	NBString_release(&headResp);
	NBString_release(&authPlain);
	NBString_release(&authB64);* /
}

*/
