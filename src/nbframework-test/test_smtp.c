//
//  test_smtp.c
//  nbframework-test-osx
//
//  Created by Marcos Ortega on 19/12/21.
//

#include "test_smtp.h"


//Testing MsExchangeClt
/*{
	const char* user			= "noreply@signitsafe.ch";
	const char* pass			= "Noreply2019";
	const char* domain			= "";
	const char* workstation		= "";
	const char* server			= "exchange.omcolo.net";
	const SI32 port				= 443;
	const char* target			= "/EWS/Exchange.asmx";
	STNBMsExchangeClt clt;
	NBMsExchangeClt_init(&clt);
	if(!NBMsExchangeClt_configure(&clt, server, port, target, user, pass, domain, workstation)){
		PRINTF_ERROR("NBMsExchangeClt_configure failed.\n");
	} else {
		STNBSmtpHeader head;
		STNBSmtpBody body;
		NBSmtpHeader_init(&head);
		NBSmtpBody_init(&body);
		{
			NBSmtpHeader_addTO(&head, "mortegam@nibsa.com.ni");
			NBSmtpHeader_addTO(&head, "info@nibsa.com.ni");
			NBSmtpHeader_addCC(&head, "marcosjom@hotmail.com");
			NBSmtpHeader_addCCO(&head, "marcosjom01@gmail.com");
			NBSmtpHeader_setSUBJECT(&head, "Test email!");
			NBSmtpBody_setContent(&body, "Hello! This is a test <b>message</b> from NBFramework!");
			//
			if(!NBMsExchangeClt_mailSendSync(&clt, &head, &body)){
				PRINTF_ERROR("NBMsExchangeClt_mailSendSync (1) failed.\n");
			} else {
				PRINTF_INFO("NBMsExchangeClt_mailSendSync (1) success.\n");
				NBSmtpHeader_setSUBJECT(&head, "Test email 2!");
				if(!NBMsExchangeClt_mailSendSync(&clt, &head, &body)){
					PRINTF_ERROR("NBMsExchangeClt_mailSendSync (2) failed.\n");
				} else {
					PRINTF_INFO("NBMsExchangeClt_mailSendSync (2) success.\n");
					NBSmtpHeader_setSUBJECT(&head, "Test email 3!");
					if(!NBMsExchangeClt_mailSendSync(&clt, &head, &body)){
						PRINTF_ERROR("NBMsExchangeClt_mailSendSync (3) failed.\n");
					} else {
						PRINTF_INFO("NBMsExchangeClt_mailSendSync (3) success.\n");
					}
				}
			}
		}
		NBSmtpBody_release(&body);
		NBSmtpHeader_release(&head);
	}
	NBMsExchangeClt_release(&clt);
}
*/

//SMTP test
/*{
	STNBSmtpClient smpt;
	NBSmtpClient_init(&smpt);
	{
		STNBSmtpHeader hdr;
		NBSmtpHeader_init(&hdr);
		NBSmtpHeader_setFROM(&hdr, "noreply@nibsa.com.ni");
		NBSmtpHeader_setSUBJECT(&hdr, "Test from NBSmtpClient");
		NBSmtpHeader_addTO(&hdr, "noreply@nibsa.com.ni");
		if(!NBSmtpClient_connect(&smpt, "mail.nibsa.com.ni", 465, TRUE, NULL)){ //587 no-ssl, 465 tls
			PRINTF_ERROR("Could not connect.\n");
		} else {
			if(!NBSmtpClient_sendHeader(&smpt, &hdr, "noreply@nibsa.com.ni", "KALvnnwQ5n8FpAans")){
				PRINTF_ERROR("Could not send header.\n");
			} else {
				PRINTF_INFO("Header sent.\n");
				STNBSmtpBody body;
				NBSmtpBody_init(&body);
				NBSmtpBody_setMimeType(&body, "text/html");
				NBSmtpBody_setContent(&body,
									  "<img src=\"https://www.signitsafe.ch/_logos/signit.logo.red@0.5x.png\" width=\"184\" height=\"48\"><br>\n"
									  "<b>Welcome,</b><br>\n<br>\n"
									  "Your confirmation code is '0000'.<br>\n<br>\n"
									  "Have a nice day,<br>\n"
									  "signit.safe dev-team<br>\n"
									  );
				if(!NBSmtpClient_sendBody(&smpt, &body)){
					PRINTF_ERROR("Could not send body.\n");
				} else {
					PRINTF_INFO("Body sent.\n");
				}
				NBSmtpBody_release(&body);
			}
			NBSmtpClient_disconnect(&smpt);
		}
		NBSmtpHeader_release(&hdr);
	}
	NBSmtpClient_release(&smpt);
}*/
