//
//  NBSmtpClient.c
//  nbframework
//
//  Created by Marcos Ortega on 28/2/19.
//

#include "nb/NBFrameworkPch.h"
#include "nb/net/NBSmtpClient.h"
//
#include "nb/crypto/NBBase64.h"
//

BOOL NBSmtpClient_sendBytes_(STNBSmtpClient* obj, const void* data, const SI32 dataSz);
BOOL NBSmtpClient_sendStr_(STNBSmtpClient* obj, const char* str);
BOOL NBSmtpClient_sendStr2_(STNBSmtpClient* obj, const char* prefix, const char* str, const char* post, STNBString* tmp);
UI32 NBSmtpClient_rcvBytesNotZero_(STNBSmtpClient* obj, char* dst, const SI32 dstSz);
UI32 NBSmtpClient_rcvBytes_(STNBSmtpClient* obj, char* dst, const SI32 dstSz);
UI32 NBSmtpClient_rcvLines_(STNBSmtpClient* obj, char* dst, const SI32 dstSz);
BOOL NBSmtpClient_hasToken_(STNBSmtpClient* obj, const char* lines, const char* token);

void NBSmtpClient_init(STNBSmtpClient* obj){
	NBMemory_setZeroSt(*obj, STNBSmtpClient);
}

void NBSmtpClient_release(STNBSmtpClient* obj){
	//Close connection
	if(NBSocket_isSet(obj->socket)){
		NBSocket_shutdown(obj->socket, NB_IO_BITS_RDWR);
		NBSocket_close(obj->socket);
	}
	//Release
	if(NBSsl_isSet(obj->ssl)){
		NBSsl_shutdown(obj->ssl);
		NBSsl_release(&obj->ssl);
		NBSsl_null(&obj->ssl);
	}
	if(NBSslContext_isSet(obj->sslCtxt)){
		NBSslContext_release(&obj->sslCtxt);
		NBSslContext_null(&obj->sslCtxt);
	}
	if(NBSocket_isSet(obj->socket)){
		NBSocket_shutdown(obj->socket, NB_IO_BITS_RDWR);
		NBSocket_release(&obj->socket);
		NBSocket_null(&obj->socket);
	}
}

//Connection

BOOL NBSmtpClient_connect(STNBSmtpClient* obj, const char* server, const SI32 port, const BOOL useSSL, STNBSslContextRef* optSslCtxt){
	BOOL r = FALSE;
	if(NBSsl_isSet(obj->ssl) || NBSocket_isSet(obj->socket)){
		PRINTF_ERROR("NBSmtpClient, already connected.\n");
	} else {
		STNBSocketRef socket = NBSocket_alloc(NULL);
		NBSocket_createHnd(socket);
		NBSocket_setNoSIGPIPE(socket, TRUE);
		NBSocket_setCorkEnabled(socket, FALSE);
		NBSocket_setDelayEnabled(socket, FALSE);
		if(!NBSocket_connect(socket, server, port)){
			PRINTF_ERROR("NBSmtpClient, could not connect to '%s':%d.\n", server, port);
		} else {
			//PRINTF_INFO("NBSmtpClient, connected to '%s':%d.\n", server, port);
			if(!useSSL){
				obj->socket = socket;
				NBSocket_null(&socket);
				r = TRUE;
			} else {
				STNBSslContextRef sslCtxt = NB_OBJREF_NULL;
				STNBSslRef ssl = NBSsl_alloc(NULL);
				if(optSslCtxt == NULL || !NBSslContext_isSet(*optSslCtxt)){
					sslCtxt = NBSslContext_alloc(NULL);
					if(!NBSslContext_create(sslCtxt, NBSslContext_getClientMode)){
						NBSslContext_release(&sslCtxt);
						NBSslContext_null(&sslCtxt);
					}
				}
				if(!NBSsl_connect(ssl, (optSslCtxt != NULL && NBSslContext_isSet(*optSslCtxt) ? *optSslCtxt : sslCtxt), socket)){
					PRINTF_ERROR("NBSmtpClient, could not start ssl at socket.\n");
				} else {
					//PRINTF_INFO("NBSmtpClient, ssl started.\n");
					obj->socket = socket; NBSocket_null(&socket);
					obj->sslCtxt = sslCtxt; NBSslContext_null(&sslCtxt);
					obj->ssl = ssl; NBSsl_null(&ssl);
					r = TRUE;
				}
				if(NBSslContext_isSet(sslCtxt)){
					NBSslContext_release(&sslCtxt);
					NBSslContext_null(&sslCtxt);
				}
				if(NBSsl_isSet(ssl)){
					NBSsl_release(&ssl);
					NBSsl_null(&ssl);
				}
			}
		}
		//Release (if not consumed)
		if(NBSocket_isSet(socket)){
			NBSocket_shutdown(socket, NB_IO_BITS_RDWR);
			NBSocket_release(&socket);
			NBSocket_null(&socket);
		}
	}
	return r;
}

void NBSmtpClient_disconnect(STNBSmtpClient* obj){
	//
	NBSmtpClient_sendStr_(obj, "QUIT\r\n");
	//
	//Close connection
	if(NBSocket_isSet(obj->socket)){
		NBSocket_shutdown(obj->socket, NB_IO_BITS_RDWR);
		NBSocket_close(obj->socket);
	}
	//Release
	if(NBSsl_isSet(obj->ssl)){
		NBSsl_shutdown(obj->ssl);
		NBSsl_release(&obj->ssl);
		NBSsl_null(&obj->ssl);
	}
	if(NBSslContext_isSet(obj->sslCtxt)){
		NBSslContext_release(&obj->sslCtxt);
		NBSslContext_null(&obj->sslCtxt);
	}
	if(NBSocket_isSet(obj->socket)){
		NBSocket_shutdown(obj->socket, NB_IO_BITS_RDWR);
		NBSocket_release(&obj->socket);
		NBSocket_null(&obj->socket);
	}
}

//

BOOL NBSmtpClient_sendHeader(STNBSmtpClient* obj, STNBSmtpHeader* hdr, const char* userName, const char* passwrd){
	BOOL r = FALSE;
	if(!NBSsl_isSet(obj->ssl) && !NBSocket_isSet(obj->socket)){
		PRINTF_ERROR("NBSmtpClient, could not send header, not connected.\n");
	} else {
		const char* from = NBSmtpHeader_getFROM(hdr);
		if(from[0] == '\0'){
			PRINTF_ERROR("NBSmtpClient, empty FROM.\n");
		} else {
			char buff[1024];
			STNBString tmp;
			NBString_init(&tmp);
			//FIRST LINE
			if(NBSmtpClient_rcvLines_(obj, buff, sizeof(buff)) <= 0){
				PRINTF_ERROR("NBSmtpClient, empty first line from server.\n");
			} else if(buff[0] != '2'){
				PRINTF_ERROR("NBSmtpClient, expected '2XX' first line from server: '%s'.\n", buff);
			} else {
				r = TRUE;
				//EHLO
				if(!NBSmtpClient_sendStr_(obj, "EHLO NBSmtpClient\r\n")){
					PRINTF_ERROR("NBSmtpClient, could not send EHLO.\n");
					r = FALSE;
				} else if(NBSmtpClient_rcvLines_(obj, buff, sizeof(buff)) <= 0){
					PRINTF_ERROR("NBSmtpClient, empty response to EHLO.\n");
					r = FALSE;
				} else if(buff[0] != '2'){
					PRINTF_ERROR("NBSmtpClient, expected '2XX' response to EHLO: '%s'.\n", buff);
					r = FALSE;
				} else {
					//AUTH
					if(userName != NULL && passwrd != NULL){
						r = FALSE;
						if(!r){
							STNBString usr64, pass64;
							NBString_init(&usr64);
							NBString_init(&pass64);
							NBBase64_code(&usr64, userName);
							NBBase64_code(&pass64, passwrd);
							{
								if(NBSmtpClient_hasToken_(obj, buff, "LOGIN")){
									if(!NBSmtpClient_sendStr_(obj, "AUTH LOGIN\r\n")){
										PRINTF_ERROR("NBSmtpClient, could not send AUTH LOGIN.\n");
										r = FALSE;
									} else if(NBSmtpClient_rcvLines_(obj, buff, sizeof(buff)) <= 0){
										PRINTF_ERROR("NBSmtpClient, empty response to AUTH LOGIN.\n");
										r = FALSE;
									} else if(buff[0] != '3'){
										PRINTF_ERROR("NBSmtpClient, expected '3XX' response to AUTH LOGIN: '%s'.\n", buff);
										r = FALSE;
									//Username
									} else if(!NBSmtpClient_sendStr2_(obj, "", usr64.str, "\r\n", &tmp)){
										PRINTF_ERROR("NBSmtpClient, could not send usr64.\n");
										r = FALSE;
									} else if(NBSmtpClient_rcvLines_(obj, buff, sizeof(buff)) <= 0){
										PRINTF_ERROR("NBSmtpClient, empty response to user64.\n");
										r = FALSE;
									} else if(buff[0] != '3'){
										PRINTF_ERROR("NBSmtpClient, expected '3XX' response to usr64: '%s'.\n", buff);
										r = FALSE;
									//Password
									} else if(!NBSmtpClient_sendStr2_(obj, "", pass64.str, "\r\n", &tmp)){
										PRINTF_ERROR("NBSmtpClient, could not send pass64.\n");
										r = FALSE;
									} else if(NBSmtpClient_rcvLines_(obj, buff, sizeof(buff)) <= 0){
										PRINTF_ERROR("NBSmtpClient, empty response to pass64.\n");
										r = FALSE;
									} else if(buff[0] != '2'){
										PRINTF_ERROR("NBSmtpClient, expected '2XX' response to pass64: '%s'.\n", buff);
										r = FALSE;
									} else {
										//PRINTF_INFO("NBSmtpClient, authenticated.\n");
										r = TRUE; //Authenticated
									}
								}
							}
							NBString_release(&usr64);
							NBString_release(&pass64);
						}
					}
					if(r){
						if(!NBSmtpClient_sendStr2_(obj, "MAIL FROM: ", from, "\r\n", &tmp)){
							PRINTF_ERROR("NBSmtpClient, could not send FROM.\n");
							r = FALSE;
						} else if(NBSmtpClient_rcvLines_(obj, buff, sizeof(buff)) <= 0){
							PRINTF_ERROR("NBSmtpClient, empty response to FROM.\n");
							r = FALSE;
						} else if(buff[0] != '2'){
							PRINTF_ERROR("NBSmtpClient, expected '2XX' response to FROM: '%s'.\n", buff);
							r = FALSE;
						} else {
							//PRINTF_INFO("NBSmtpClient, EHLO and FROM sent: '%s'.\n", buff);
							//TO
							if(r){
								UI32 i = 0; const char* mail = NULL;
								while((mail = NBSmtpHeader_getTO(hdr, i++)) != NULL){
									if(!NBSmtpClient_sendStr2_(obj, "RCPT TO:", mail, "\r\n", &tmp)){
										PRINTF_ERROR("NBSmtpClient, could not send TO.\n");
										r = FALSE;
									} else if(NBSmtpClient_rcvLines_(obj, buff, sizeof(buff)) <= 0){
										PRINTF_ERROR("NBSmtpClient, empty response to TO.\n");
										r = FALSE;
									} else if(buff[0] != '2'){
										PRINTF_ERROR("NBSmtpClient, expected '2XX' response to TO: '%s'.\n", buff);
										r = FALSE;
									} else {
										//PRINTF_INFO("NBSmtpClient, TO sent: '%s'.\n", buff);
									}
								}
							}
							//DATA
							if(r){
								if(!NBSmtpClient_sendStr_(obj, "DATA\r\n")){
									PRINTF_ERROR("NBSmtpClient, could not send DATA.\n");
									r = FALSE;
								} else if(NBSmtpClient_rcvLines_(obj, buff, sizeof(buff)) <= 0){
									PRINTF_ERROR("NBSmtpClient, empty response to DATA.\n");
									r = FALSE;
								} else if(buff[0] != '3'){
									PRINTF_ERROR("NBSmtpClient, expected '3XX' response to DATA: '%s'.\n", buff);
									r = FALSE;
								} else {
									//PRINTF_INFO("NBSmtpClient, DATA (header) sent: '%s'.\n", buff);
								}
							}
							//Build data (destinatary)
							if(r){
								NBString_empty(&tmp);
								//From
								{
									NBString_concat(&tmp, "From: ");
									NBString_concat(&tmp, from);
									NBString_concat(&tmp, "\r\n");
								}
								//To
								{
									UI32 i = 0; const char* mail = NULL;
									while((mail = NBSmtpHeader_getTO(hdr, i++)) != NULL){
										NBString_concat(&tmp, i == 1 ? "To: " : " ");
										NBString_concat(&tmp, mail);
										NBString_concat(&tmp, ";");
									}
									if(tmp.str[tmp.length - 1] != '\n'){
										NBString_concat(&tmp, "\r\n");
									}
								}
								//Cc
								{
									UI32 i = 0; const char* mail = NULL;
									while((mail = NBSmtpHeader_getCC(hdr, i++)) != NULL){
										NBString_concat(&tmp, i == 1 ? "Cc: " : " ");
										NBString_concat(&tmp, mail);
										NBString_concat(&tmp, ";");
									}
									if(tmp.str[tmp.length - 1] != '\n'){
										NBString_concat(&tmp, "\r\n");
									}
								}
								//Bcc
								{
									UI32 i = 0; const char* mail = NULL;
									while((mail = NBSmtpHeader_getCCO(hdr, i++)) != NULL){
										NBString_concat(&tmp, i == 1 ? "Bcc: " : " ");
										NBString_concat(&tmp, mail);
										NBString_concat(&tmp, ";");
									}
									if(tmp.str[tmp.length - 1] != '\n'){
										NBString_concat(&tmp, "\r\n");
									}
								}
								//Subject
								{
									NBString_concat(&tmp, "Subject: ");
									NBString_concat(&tmp, NBSmtpHeader_getSUBJECT(hdr));
									NBString_concat(&tmp, "\r\n");
								}
								//Send
								if(!NBSmtpClient_sendStr_(obj, tmp.str)){
									PRINTF_ERROR("NBSmtpClient, DATA header sent.\n");
									r = FALSE;
								}
							}
						}
					}
				}
			}
			NBString_release(&tmp);
		}
	}
	return r;
}

BOOL NBSmtpClient_sendBody(STNBSmtpClient* obj, STNBSmtpBody* body){
	BOOL r = FALSE;
	if(!NBSmtpClient_sendBodyStart(obj, body)){
		PRINTF_ERROR("NBSmtpClient, could not sendBodyStart.\n");
	} else if(!NBSmtpClient_sendBodyContent(obj, body)){
		PRINTF_ERROR("NBSmtpClient, could not sendBodyContent.\n");
	} else if(!NBSmtpClient_sendBodyEnd(obj, body)){
		PRINTF_ERROR("NBSmtpClient, could not sendBodyEnd.\n");
	} else {
		r = TRUE;
	}
	return r;
}

BOOL NBSmtpClient_sendBodyStart(STNBSmtpClient* obj, STNBSmtpBody* body){
	BOOL r = FALSE;
	STNBString tmp;
	NBString_init(&tmp);
	{
		const char* boundary =  NBSmtpBody_getBoundary(body);
		const char* mimeType = NBSmtpBody_getMimeType(body);
		NBString_concat(&tmp, "MIME-Version: 1.0\r\n");
		NBString_concat(&tmp, "Content-Type: multipart/mixed; boundary=\"");
		NBString_concat(&tmp, boundary);
		NBString_concat(&tmp, "\"\r\n");
		NBString_concat(&tmp, "\r\n");
		NBString_concat(&tmp, "--");
		NBString_concat(&tmp, boundary);
		NBString_concat(&tmp, "\r\n");
		NBString_concat(&tmp, "Content-Type:");
		NBString_concat(&tmp, mimeType);
		NBString_concat(&tmp, "\r\n\r\n");
		if(!NBSmtpClient_sendStr_(obj, tmp.str)){
			PRINTF_ERROR("NBSmtpClient, could not sendBodyStart.\n");
		} else {
			r = TRUE;
		}
	}
	NBString_release(&tmp);
	return r;
}

BOOL NBSmtpClient_sendBodyContent(STNBSmtpClient* obj, STNBSmtpBody* body){
	BOOL r = FALSE;
	STNBString tmp;
	NBString_init(&tmp);
	{
		//Content
		const char* content = NBSmtpBody_getContent(body);
		if(!NBSmtpClient_sendStr_(obj, content)){
			PRINTF_ERROR("NBSmtpClient, could not sendBodyContent.\n");
		} else {
			//End-of-content
			NBString_concat(&tmp, "\r\n\r\n");
			if(!NBSmtpClient_sendStr_(obj, tmp.str)){
				PRINTF_ERROR("NBSmtpClient, could not sendBodyContent.\n");
			} else {
				r = TRUE;
			}
		}
	}
	NBString_release(&tmp);
	return r;
}

BOOL NBSmtpClient_sendBodyEnd(STNBSmtpClient* obj, STNBSmtpBody* body){
	BOOL r = FALSE;
	STNBString tmp;
	NBString_init(&tmp);
	{
		const char* boundary =  NBSmtpBody_getBoundary(body);
		//Last boundary
		NBString_concat(&tmp, "--");
		NBString_concat(&tmp, boundary);
		NBString_concat(&tmp, "--\r\n");
		NBString_concat(&tmp, "\r\n.\r\n");
		if(!NBSmtpClient_sendStr_(obj, tmp.str)){
			PRINTF_ERROR("NBSmtpClient, could not sendBodyStart.\n");
		} else {
			r = TRUE;
		}
	}
	NBString_release(&tmp);
	return r;
}

//

BOOL NBSmtpClient_sendBytes_(STNBSmtpClient* obj, const void* data, const SI32 dataSz){
	BOOL r = FALSE;
	if(NBSsl_isSet(obj->ssl)){
        const SI32 rr = NBSsl_write(obj->ssl, data, dataSz);
        r = (rr == dataSz);
	} else if(NBSocket_isSet(obj->socket)){
		const SI32 rr = NBSocket_send(obj->socket, data, dataSz);
		r = (rr == dataSz);
	}
	return r;
}

BOOL NBSmtpClient_sendStr_(STNBSmtpClient* obj, const char* str){
	return NBSmtpClient_sendBytes_(obj, str, NBString_strLenBytes(str));
}

BOOL NBSmtpClient_sendStr2_(STNBSmtpClient* obj, const char* prefix, const char* str, const char* post, STNBString* tmp){
	NBString_set(tmp, prefix);
	NBString_concat(tmp, str);
	NBString_concat(tmp, post);
	return NBSmtpClient_sendBytes_(obj, tmp->str, tmp->length);
}

UI32 NBSmtpClient_rcvBytesNotZero_(STNBSmtpClient* obj, char* dst, const SI32 dstSz){
	SI32 r = 0;
	if(NBSsl_isSet(obj->ssl)){
		const SI32 rr = NBSsl_read(obj->ssl, dst, dstSz);
        r = (rr >= 0 ? rr : 0);
	} else if(NBSocket_isSet(obj->socket)){
		const SI32 rr = NBSocket_recv(obj->socket, dst, dstSz);
		r = (rr >= 0 ? rr : 0);
	}
	return r;
}

UI32 NBSmtpClient_rcvBytes_(STNBSmtpClient* obj, char* dst, const SI32 dstSz){
	NBMemory_set(dst, 0, dstSz);
	return NBSmtpClient_rcvBytesNotZero_(obj, dst, dstSz);
}

UI32 NBSmtpClient_rcvLines_(STNBSmtpClient* obj, char* dst, const SI32 dstSz){
	UI32 r = 0, iLine = 0, rPrev, rr; BOOL nextCharStartLine = FALSE;
	NBMemory_set(dst, 0, dstSz);
	while(TRUE){
		rr = NBSmtpClient_rcvBytesNotZero_(obj, &dst[r], (dstSz - r));
		if(rr <= 0){
			break;
		} else {
			rPrev = r;
			r += rr;
			//Analyze lines
			while(rPrev < r){
				if(nextCharStartLine){
					iLine = rPrev;
					nextCharStartLine = FALSE;
				}
				if(rPrev > 1){
					if(dst[rPrev - 2] == '\r' && dst[rPrev - 1] == '\n'){
						nextCharStartLine = TRUE;
					}
				}
				rPrev++;
			}
			//Analyze '\r\n'
			if(r > 1 && (r - iLine) > 3){
				//Lines XXX-.... are continued by:
				//Lines XXX .... are final.
				if(dst[iLine + 3] != '-' && dst[r - 2] == '\r' && dst[r - 1] == '\n'){
					dst[r - 2] = dst[r - 1] = '\0';
					break;
				}
			}
		}
	}
	//PRINTF_INFO("NBSmtpClient, lines-rcvd: '%s' (%d bytes).\n", dst, r);
	return r;
}

BOOL NBSmtpClient_hasToken_(STNBSmtpClient* obj, const char* lines, const char* token){
	BOOL r = FALSE;
	SI32 pos = -1;
	while(TRUE){
		pos = NBString_strIndexOf(lines, token, pos + 1);
		if(pos >= 0){
			const char nxtChar = lines[pos + NBString_strLenBytes(token)];
			char prvChar = ' '; if(pos > 0) prvChar = lines[pos - 1];
			if((prvChar == ' ' || prvChar == '\n') && (nxtChar == ' ' || nxtChar == '\r')){
				r = TRUE;
				break;
			}
		} else {
			break;
		}
	}
	return r;
}
