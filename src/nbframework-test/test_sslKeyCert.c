//
//  test_sslKeyCert.c
//  nbframework-test-osx
//
//  Created by Marcos Ortega on 19/12/21.
//

#include "test_sslKeyCert.h"

//Create server certificate (SSL and CertificateAuthority)
/*{
	const char* altNames[] = {
		"DNS:localhost"
	};
	const char* subjectNames[] = {
		"C", "NI"					//CountryName
		, "ST", "Managua"			//StateOrProvinceName
		, "L", "Managua"			//LocalityName
		, "CN", "127.0.0.1"			//CommonName
		, "O", "Signit"		//OrganizationName
		, "OU", "SignitNI"		//OrganizationalUnitName
	};
	STNBPKey pkey;
	STNBX509 cert;
	NBPKey_init(&pkey);
	NBX509_init(&cert);
	if(!NBX509_createSelfSigned(&cert, 2048, 365, subjectNames, (sizeof(subjectNames) / sizeof(subjectNames[0])), altNames, (sizeof(altNames) / sizeof(altNames[0])), &pkey)){
		PRINTF_ERROR("Could not generate certificate.\n");
	} else {
		STNBString certData, pkeyData;
		NBString_init(&certData);
		NBString_init(&pkeyData);
		if(!NBX509_getAsDERString(&cert, &certData)){
			PRINTF_ERROR("Could not generate ca-certificate's DER.\n");
		} else {
			FILE* f = fopen("localhost.cert.der", "wb+");
			if(f != NULL){
				fwrite(certData.str, certData.length, 1, f);
				fclose(f);
			}
		}
		if(!NBPKey_getAsDERString(&pkey, &pkeyData)){
			PRINTF_ERROR("Could not generate ca-pk's DER.\n");
		} else {
			FILE* f = fopen("localhost.key.der", "wb+");
			if(f != NULL){
				fwrite(pkeyData.str, pkeyData.length, 1, f);
				fclose(f);
			}
		}
		NBString_release(&certData);
		NBString_release(&pkeyData);
	}
	NBX509_release(&cert);
	NBPKey_release(&pkey);
}*/
//Create a CSR (Certificate Signing Request)
/*{
	const char* subjectNames[] = {
		"C", "NI"					//CountryName
		, "ST", "Managua"			//StateOrProvinceName
		, "L", "Managua"			//LocalityName
		, "CN", "User"				//CommonName
		, "O", "Signit"		//OrganizationName
		, "OU", "SignitNI"		//OrganizationalUnitName
	};
	STNBPKey pkey;
	STNBX509Req cert;
	NBPKey_init(&pkey);
	NBX509Req_init(&cert);
	if(!NBX509Req_createSelfSigned(&cert, 2048, subjectNames, (sizeof(subjectNames) / sizeof(subjectNames[0])), NULL, 0, &pkey)){
		PRINTF_ERROR("Could not generate certificate request.\n");
	} else {
		STNBString certData, pkeyData;
		NBString_init(&certData);
		NBString_init(&pkeyData);
		if(!NBX509Req_getAsDERString(&cert, &certData)){
			PRINTF_ERROR("Could not generate clt-sign-request's DER.\n");
		} else {
			FILE* f = fopen("user.req.der", "wb+");
			if(f != NULL){
				fwrite(certData.str, certData.length, 1, f);
				fclose(f);
			}
		}
		if(!NBPKey_getAsDERString(&pkey, &pkeyData)){
			PRINTF_ERROR("Could not generate clt-pk's DER.\n");
		} else {
			FILE* f = fopen("user.key.der", "wb+");
			if(f != NULL){
				fwrite(pkeyData.str, pkeyData.length, 1, f);
				fclose(f);
			}
		}
		NBString_release(&certData);
		NBString_release(&pkeyData);
	}
	NBX509Req_release(&cert);
	NBPKey_release(&pkey);
}*/
//Create certificate from request
/*{
	STNBX509 caCert; STNBX509Req req; STNBPKey caKey;
	NBX509Req_init(&req);
	NBX509_init(&caCert);
	NBPKey_init(&caKey);
	if(!NBX509Req_createFromDERFile(&req, "user.req.der")){
		PRINTF_ERROR("Could not load cert-req.\n");
	} else {
		if(!NBX509_createFromDERFile(&caCert, "localhost.cert.der")){
			PRINTF_ERROR("Could not load ca-cert.\n");
		} else {
			if(!NBPKey_createFromDERFile(&caKey, "localhost.key.der")){
				PRINTF_ERROR("Could not load ca-key.\n");
			} else {
				STNBX509 cert;
				NBX509_init(&cert);
				if(!NBX509_createFromRequest(&cert, &req, &caCert, &caKey, 2048, 365)){
					PRINTF_ERROR("Could not generate certificate.\n");
				} else {
					STNBString certData;
					NBString_init(&certData);
					if(!NBX509_getAsDERString(&cert, &certData)){
						PRINTF_ERROR("Could not generate clt-certificate's DER.\n");
					} else {
						FILE* f = fopen("user.cert.der", "wb+");
						if(f != NULL){
							fwrite(certData.str, certData.length, 1, f);
							fclose(f);
						}
					}
					NBString_release(&certData);
				}
				NBX509_release(&cert);
			}
		}
	}
	NBX509Req_release(&req);
	NBPKey_release(&caKey);
	NBX509_release(&caCert);
}*/
//Create P12 bundle
/*{
	STNBX509 caCert, usrCert; STNBPKey usrKey;
	NBX509_init(&caCert);
	NBX509_init(&usrCert);
	NBPKey_init(&usrKey);
	if(!NBX509_createFromDERFile(&caCert, "localhost.cert.der")){
		PRINTF_ERROR("Could not load ca-cert.\n");
	} else {
		if(!NBX509_createFromDERFile(&usrCert, "user.cert.der")){
			PRINTF_ERROR("Could not load user-cert.\n");
		} else {
			if(!NBPKey_createFromDERFile(&usrKey, "user.key.der")){
				PRINTF_ERROR("Could not load user-key.\n");
			} else {
				STNBPkcs12 bundle;
				NBPkcs12_init(&bundle);
				if(!NBPkcs12_createBundle(&bundle, "signit_user", &usrKey, &usrCert, &caCert, 1, "signit")){
					PRINTF_ERROR("Could not create p12 bundle.\n");
				} else {
					STNBString data;
					NBString_init(&data);
					if(!NBPkcs12_getAsDERString(&bundle, &data)){
						PRINTF_ERROR("Could not generate clt-certificate's DER.\n");
					} else {
						FILE* f = fopen("user.bundle.p12", "wb+");
						if(f != NULL){
							fwrite(data.str, data.length, 1, f);
							fclose(f);
						}
					}
					NBString_release(&data);
				}
				NBPkcs12_release(&bundle);
			}
		}
	}
	NBX509_release(&caCert);
	NBX509_release(&usrCert);
	NBPKey_release(&usrKey);
}*/
