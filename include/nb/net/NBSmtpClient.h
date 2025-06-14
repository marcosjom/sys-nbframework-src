//
//  NBSmtpClient.h
//  nbframework
//
//  Created by Marcos Ortega on 28/2/19.
//

#ifndef NBSmtpClient_h
#define NBSmtpClient_h

#include "nb/net/NBSocket.h"
#include "nb/ssl/NBSsl.h"
#include "nb/net/NBSmtpHeader.h"
#include "nb/net/NBSmtpBody.h"

typedef struct STNBSmtpClient_ {
	STNBSocketRef		socket;
	STNBSslContextRef	sslCtxt;
	STNBSslRef			ssl;
} STNBSmtpClient;

void NBSmtpClient_init(STNBSmtpClient* obj);
void NBSmtpClient_release(STNBSmtpClient* obj);

//Connection
BOOL NBSmtpClient_connect(STNBSmtpClient* obj, const char* server, const SI32 port, const BOOL useSSL, STNBSslContextRef* optSslCtxt);
void NBSmtpClient_disconnect(STNBSmtpClient* obj);

//
BOOL NBSmtpClient_sendHeader(STNBSmtpClient* obj, STNBSmtpHeader* hdr, const char* userName, const char* passwrd);
BOOL NBSmtpClient_sendBody(STNBSmtpClient* obj, STNBSmtpBody* body);
BOOL NBSmtpClient_sendBodyStart(STNBSmtpClient* obj, STNBSmtpBody* body);
BOOL NBSmtpClient_sendBodyContent(STNBSmtpClient* obj, STNBSmtpBody* body);
BOOL NBSmtpClient_sendBodyEnd(STNBSmtpClient* obj, STNBSmtpBody* body);


#endif /* NBSmtpClient_h */
