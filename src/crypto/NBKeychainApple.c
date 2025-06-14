//
//  NBKeychainApple.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/21/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/crypto/NBKeychainApple.h"
//
#include "nb/core/NBMemory.h"
#include <Security/Security.h>

//
//Interface
//

void NBKeychainApple_create_(void** obj);
void NBKeychainApple_destroy_(void** obj);
//
BOOL NBKeychainApple_addPassword_(void* obj, const char* account, const char* passwd, const BOOL allowDevicesSync);
BOOL NBKeychainApple_getPassword_(void* obj, const char* account, STNBString* dst);
BOOL NBKeychainApple_removePassword_(void* obj, const char* account);
//
BOOL NBKeychainApple_addCert_(void* obj, const STNBX509* cert, const BOOL allowDevicesSync);
BOOL NBKeychainApple_getCertByCN_(void* obj, const char* commonName, STNBX509* dst);
BOOL NBKeychainApple_getCertsIssuedByCert_(void* obj, const STNBX509* issuer, STNBArray* dst); //STNBX509
BOOL NBKeychainApple_removeCertByCN_(void* obj, const char* commonName);

//

typedef struct STNBKeychainAppleOpq_ {
	BOOL nothing;
} STNBKeychainAppleOpq;

void NBKeychainApple_init(STNBKeychainApple* obj){
	STNBKeychainAppleOpq* opq = obj->opaque = NBMemory_allocType(STNBKeychainAppleOpq);
	NBMemory_setZeroSt(*opq, STNBKeychainAppleOpq);
}

void NBKeychainApple_release(STNBKeychainApple* obj){
	if(obj->opaque != NULL){
		//STNBKeychainAppleOpq* opq = (STNBKeychainAppleOpq*)obj->opaque;
		NBMemory_free(obj->opaque);
		obj->opaque = NULL;
	}
}

//generic passwords

BOOL NBKeychainApple_getPassword(STNBKeychainApple* obj, const char* pAccount, STNBString* dst){
	BOOL r = FALSE;
	STNBKeychainAppleOpq* opq = (STNBKeychainAppleOpq*)obj->opaque;
	if(opq != NULL){
		CFStringRef account	= CFStringCreateWithCString(CFAllocatorGetDefault(), pAccount, kCFStringEncodingUTF8);
		CFMutableDictionaryRef attribs = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		CFDictionarySetValue(attribs, kSecClass, kSecClassGenericPassword);
		CFDictionarySetValue(attribs, kSecAttrAccount, account);
		CFDictionarySetValue(attribs, kSecMatchLimit, kSecMatchLimitOne);
		CFDictionarySetValue(attribs, kSecReturnData, (dst != NULL ? kCFBooleanTrue : kCFBooleanFalse));
		CFDictionarySetValue(attribs, kSecAttrSynchronizable, kSecAttrSynchronizableAny);
		CFDataRef passData = nil;
		OSStatus rr = SecItemCopyMatching(attribs, (CFTypeRef*)&passData);
		if(rr != errSecSuccess){
			if(rr != errSecItemNotFound){
				PRINTF_ERROR("NBKeychainApple, Could not search at keychain, error(%d).\n", (SI32)rr);
			} else {
				PRINTF_INFO("NBKeychainApple, Password didnt exists.\n");
			}
		} else {
			if(dst != NULL){
				NBString_concatBytes(dst, (const char*)CFDataGetBytePtr(passData), (UI32)CFDataGetLength(passData));
				//PRINTF_INFO("NBKeychainApple, Password found at keychain (%d bytes).\n", (UI32)dst->length);
			} else {
				//PRINTF_INFO("NBKeychainApple, Password found at keychain.\n");
			}
			r = TRUE;
		}
		if(passData){
			CFRelease(passData);
		}
		CFRelease(attribs);
		CFRelease(account);
		/*NSString *account	= [NSString stringWithUTF8String:pAccount];
		NSDictionary* query = @{
			(id)kSecClass: (id)kSecClassGenericPassword
			, (id)kSecAttrAccount: (id)account
			, (id)kSecMatchLimit: (id)kSecMatchLimitOne
			, (id)kSecReturnData: (id)(dst != NULL ? kCFBooleanTrue : kCFBooleanFalse)
			, (id)kSecAttrSynchronizable: (id)kSecAttrSynchronizableAny
		};
		OSStatus rr;
		CFDataRef passData = nil;
		if((rr = SecItemCopyMatching((__bridge CFDictionaryRef)query, (CFTypeRef*)&passData)) != errSecSuccess){
			if(rr != errSecItemNotFound){
				PRINTF_ERROR("NBKeychainApple, Could not search at keychain, error(%d).\n", rr);
			} else {
				PRINTF_INFO("NBKeychainApple, Password didnt exists.\n");
			}
		} else {
			if(dst != NULL){
				NSData *pswd = (__bridge NSData *)passData;
				NBString_concatBytes(dst, [pswd bytes], (UI32)[pswd length]);
				PRINTF_INFO("NBKeychainApple, Password found at keychain (%d bytes).\n", (UI32)[pswd length]);
			} else {
				PRINTF_INFO("NBKeychainApple, Password found at keychain.\n");
			}
			r = TRUE;
		}
		if(passData){
			CFRelease(passData);
		}*/
	}
	return r;
}

BOOL NBKeychainApple_addPassword(STNBKeychainApple* obj, const char* pAccount, const char* pPasswd, const BOOL allowDevicesSync){
	BOOL r = FALSE;
	STNBKeychainAppleOpq* opq = (STNBKeychainAppleOpq*)obj->opaque;
	if(opq != NULL){
		CFDataRef passData  = CFDataCreate(CFAllocatorGetDefault(), (const UInt8*)pPasswd, NBString_strLenBytes(pPasswd));
		CFStringRef account	= CFStringCreateWithCString(CFAllocatorGetDefault(), pAccount, kCFStringEncodingUTF8);
		CFMutableDictionaryRef attribs = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		CFDictionarySetValue(attribs, kSecClass, kSecClassGenericPassword);
		CFDictionarySetValue(attribs, kSecAttrAccount, account);
		CFDictionarySetValue(attribs, kSecAttrSynchronizable, (allowDevicesSync ? kCFBooleanTrue : kCFBooleanFalse));
		CFDictionarySetValue(attribs, kSecAttrAccessible, kSecAttrAccessibleWhenUnlocked);
		//CFDictionarySetValue(attribs, kSecAttrAccess, //ToDo: implement Access Control List (ACL));
		//CFDictionarySetValue(attribs, kSecAttrAccessGroupToken, //ToDo: implement Access Control List (ACL));
		CFDictionarySetValue(attribs, kSecValueData, passData);
		OSStatus rr = SecItemAdd(attribs, NULL);
		if(rr != errSecSuccess){
			PRINTF_ERROR("NBKeychainApple, Could not create password at keychain, error(%d).\n", (SI32)rr);
		} else {
			PRINTF_INFO("NBKeychainApple, Password create at keychain.\n");
			r = TRUE;
		}
		CFRelease(attribs);
		CFRelease(account);
		CFRelease(passData);
		/*NSData *passData	= [NSData dataWithBytesNoCopy:(void*)pPasswd length:NBString_strLenBytes(pPasswd)];
		NSString *account	= [NSString stringWithUTF8String:pAccount];
		NSDictionary* query = @{
			(id)kSecClass: (id)kSecClassGenericPassword
			, (id)kSecAttrAccount: (id)account
			, (id)kSecAttrSynchronizable: (id)(allowDevicesSync ? kCFBooleanTrue : kCFBooleanFalse)
			//, (id)kSecAttrAccess: (id) //ToDo: implement Access Control List (ACL)
			, (id)kSecAttrAccessible: (id)kSecAttrAccessibleWhenUnlocked //Required if 'allowDevicesSync' is TRUE
			//, (id)kSecAttrAccessGroup: (id)kSecAttrAccessGroupToken //Required if 'allowDevicesSync' is TRUE
			, (id)kSecValueData: (id)passData
		};
		OSStatus rr;
		if((rr = SecItemAdd((__bridge CFDictionaryRef)query, NULL)) != errSecSuccess){
			PRINTF_ERROR("NBKeychainApple, Could not create password at keychain, error(%d).\n", rr);
		} else {
			PRINTF_INFO("NBKeychainApple, Password create at keychain.\n");
			r = TRUE;
		}*/
	}
	return r;
}

BOOL NBKeychainApple_removePassword(STNBKeychainApple* obj, const char* pAccount){
	BOOL r = FALSE;
	STNBKeychainAppleOpq* opq = (STNBKeychainAppleOpq*)obj->opaque;
	if(opq != NULL){
		CFStringRef account	= CFStringCreateWithCString(CFAllocatorGetDefault(), pAccount, kCFStringEncodingUTF8);
		CFMutableDictionaryRef attribs = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		CFDictionarySetValue(attribs, kSecClass, kSecClassGenericPassword);
		CFDictionarySetValue(attribs, kSecAttrAccount, account);
		CFDictionarySetValue(attribs, kSecMatchLimit, kSecMatchLimitOne);
		CFDictionarySetValue(attribs, kSecAttrSynchronizable, kSecAttrSynchronizableAny);
		OSStatus rr = SecItemDelete(attribs);
		if(rr != errSecSuccess){
			if(rr != errSecItemNotFound){
				PRINTF_ERROR("NBKeychainApple, Could not delete password at keychain, error(%d).\n", (SI32)rr);
			} else {
				PRINTF_INFO("NBKeychainApple, Password didint exist (nothing to delete).\n");
			}
		} else {
			PRINTF_INFO("NBKeychainApple, Password deleted at keychain.\n");
			r = TRUE;
		}
		CFRelease(attribs);
		CFRelease(account);
		/*NSString *account	= [NSString stringWithUTF8String:pAccount];
		NSDictionary* query = @{
			(id)kSecClass: (id)kSecClassGenericPassword
			, (id)kSecAttrAccount: (id)account
			, (id)kSecMatchLimit: (id)kSecMatchLimitOne
			, (id)kSecAttrSynchronizable: (id)kSecAttrSynchronizableAny
		};
		OSStatus rr;
		if((rr = SecItemDelete((__bridge CFDictionaryRef)query)) != errSecSuccess){
			if(rr != errSecItemNotFound){
				PRINTF_ERROR("NBKeychainApple, Could not delete password at keychain, error(%d).\n", rr);
			} else {
				PRINTF_INFO("NBKeychainApple, Password didint exist (nothing to delete).\n");
			}
		} else {
			PRINTF_INFO("NBKeychainApple, Password deleted at keychain.\n");
			r = TRUE;
		}*/
	}
	return r;
}

//Public certificates

BOOL NBKeychainApple_addCert(STNBKeychainApple* obj, const STNBX509* cert, const BOOL allowDevicesSync){
	BOOL r = FALSE;
	/*STNBKeychainAppleOpq* opq = (STNBKeychainAppleOpq*)obj->opaque;
	if(opq != NULL){
		STNBString derStr, cn;
		NBString_init(&derStr);
		NBString_init(&cn);
		if(!NBX509_getAsDERString(cert, &derStr)){
			PRINTF_ERROR("NBKeychainApple, Could not get certificate's DER data.\n");
		} else if(!NBX509_concatSubjectCommName(cert, &cn)){
			PRINTF_ERROR("NBKeychainApple, Could not get certificate's CommonName.\n");
		} else {
			NSData *certData = [NSData dataWithBytesNoCopy:derStr.str length:derStr.length];
			SecCertificateRef cert = SecCertificateCreateWithData(NULL, (CFDataRef)certData);
			if(cert == NULL){
				PRINTF_ERROR("NBKeychainApple, Could not load certificate to SecCertificateRef.\n");
			} else {
				NSDictionary* query = @{
										(id)kSecValueRef: (__bridge id)cert
										, (id)kSecClass: (id)kSecClassCertificate
										, (id)kSecAttrLabel: [NSString stringWithUTF8String:cn.str]
										};
				OSStatus rr;
				if((rr = SecItemAdd((__bridge CFDictionaryRef)query, NULL)) != errSecSuccess){
					//errSecDuplicateItem = -25299
					switch (rr) {
						case errSecDuplicateItem: PRINTF_ERROR("NBKeychainApple, Could not create certificate at keychain, error(%d, errSecDuplicateItem).\n", rr); break;
						default: PRINTF_ERROR("NBKeychainApple, Could not create certificate at keychain, error(%d).\n", rr); break;
					}
				} else {
					PRINTF_INFO("NBKeychainApple, Certificate created at keychain.\n");
					r = TRUE;
				}
				CFRelease(cert);
			}
		}
		NBString_release(&cn);
		NBString_release(&derStr);
	}*/
	return r;
}

BOOL NBKeychainApple_getCertByCN(STNBKeychainApple* obj, const char* commonName, STNBX509* dst){
	BOOL r = FALSE;
	/*STNBKeychainAppleOpq* opq = (STNBKeychainAppleOpq*)obj->opaque;
	if(opq != NULL){
		NSDictionary* query = @{
				(id)kSecClass: (id)kSecClassCertificate
				, (id)kSecAttrLabel: [NSString stringWithUTF8String:commonName]
				, (id)kSecMatchLimit: (id)kSecMatchLimitOne
				, (id)kSecReturnRef: (id)(dst != NULL ? kCFBooleanTrue : kCFBooleanFalse)
				, (id)kSecAttrSynchronizable: (id)kSecAttrSynchronizableAny
		};
		OSStatus rr;
		SecCertificateRef cert = NULL;
		if((rr = SecItemCopyMatching((__bridge CFDictionaryRef)query, (CFTypeRef*)&cert)) != errSecSuccess){
			if(rr == errSecItemNotFound){
				PRINTF_INFO("NBKeychainApple, Cert not found.\n");
			} else {
				PRINTF_ERROR("NBKeychainApple, Could not search at keychain, error(%d).\n", rr);
			}
		} else {
			if(dst != NULL){
				NSData* certData = (NSData*)CFBridgingRelease(SecCertificateCopyData(cert));
				if(!NBX509_createFromDERBytes(dst, [certData bytes], (UI32)[certData length])){
					PRINTF_ERROR("NBKeychainApple, Cert found at keychain but error loading (%d bytes).\n", (UI32)[certData length]);
				} else {
					PRINTF_INFO("NBKeychainApple, Cert found at keychain and loaded (%d bytes).\n", (UI32)[certData length]);
					r = TRUE;
				}
			} else {
				PRINTF_INFO("NBKeychainApple, Cert found at keychain.\n");
				r = TRUE;
			}
		}
		if(cert){
			CFRelease(cert);
		}
	}*/
	return r;
}

BOOL NBKeychainApple_getCertsIssuedByCert(STNBKeychainApple* obj, const STNBX509* cIssuer, STNBArray* dst){
	BOOL r = FALSE;
	/*STNBKeychainAppleOpq* opq = (STNBKeychainAppleOpq*)obj->opaque;
	if(opq != NULL){
		STNBString issuer;
		NBString_init(&issuer);
		if(!NBX509_concatIssuerDER(cIssuer, &issuer)){
			PRINTF_ERROR("NBKeychainApple, Could not retrieve certificate's issuer in DER format.\n");
		} else {
			NSDictionary* query = @{
									(id)kSecClass: (id)kSecClassCertificate
									, (id)kSecAttrIssuer: [NSData dataWithBytesNoCopy:issuer.str length:issuer.length]
									, (id)kSecMatchLimit: (id)kSecMatchLimitAll
									, (id)kSecReturnRef: (id)(dst != NULL ? kCFBooleanTrue : kCFBooleanFalse)
									, (id)kSecAttrSynchronizable: (id)kSecAttrSynchronizableAny
									};
			OSStatus rr;
			CFArrayRef arr = NULL;
			if((rr = SecItemCopyMatching((__bridge CFDictionaryRef)query, (CFTypeRef*)&arr)) != errSecSuccess){
				if(rr == errSecItemNotFound){
					PRINTF_INFO("NBKeychainApple, Cert not found.\n");
				} else {
					PRINTF_ERROR("NBKeychainApple, Could not search at keychain, error(%d).\n", rr);
				}
			} else {
				if(dst != NULL){
					r = TRUE;
					CFIndex i; const CFIndex count = CFArrayGetCount(arr);
					for(i = 0 ; i < count; i++){
						SecCertificateRef cert = (SecCertificateRef)CFArrayGetValueAtIndex(arr, i);
						NSData* certData = (NSData*)CFBridgingRelease(SecCertificateCopyData(cert));
						STNBX509 x509;
						NBX509_init(&x509);
						if(!NBX509_createFromDERBytes(&x509, [certData bytes], (UI32)[certData length])){
							PRINTF_ERROR("NBKeychainApple, Cert found at keychain but error loading (%d bytes).\n", (UI32)[certData length]);
							NBX509_release(&x509);
						} else {
							PRINTF_INFO("NBKeychainApple, Cert found at keychain and loaded (%d bytes).\n", (UI32)[certData length]);
							NBArray_addValue(dst, x509);
						}
					}
				} else {
					PRINTF_INFO("NBKeychainApple, Cert found at keychain.\n");
					r = TRUE;
				}
			}
			if(arr){
				CFRelease(arr);
			}
		}
		NBString_release(&issuer);
	}*/
	return r;
}

BOOL NBKeychainApple_removeCertByCN(STNBKeychainApple* obj, const char* cn){
	BOOL r = FALSE;
	/*STNBKeychainAppleOpq* opq = (STNBKeychainAppleOpq*)obj->opaque;
	if(opq != NULL){
		NSDictionary* query = @{
			(id)kSecClass: (id)kSecClassCertificate
			, (id)kSecAttrLabel: [NSString stringWithUTF8String:cn]
			, (id)kSecMatchLimit: (id)kSecMatchLimitOne
			, (id)kSecAttrSynchronizable: (id)kSecAttrSynchronizableAny
		};
		OSStatus rr;
		if((rr = SecItemDelete((__bridge CFDictionaryRef)query)) != errSecSuccess){
			if(rr != errSecItemNotFound){
				PRINTF_ERROR("NBKeychainApple, Could not delete cert at keychain, error(%d).\n", rr);
			} else {
				PRINTF_INFO("NBKeychainApple, Cert didint exist (nothing to delete).\n");
			}
		} else {
			PRINTF_INFO("NBKeychainApple, Cert deleted at keychain.\n");
			r = TRUE;
		}
	}*/
	return r;
}

//Interface

STNBKeychainItf NBKeychainApple_getItf(void){
	STNBKeychainItf r;
	NBMemory_setZeroSt(r, STNBKeychainItf);
	r.param				= NULL;
	//
	r.create			= NBKeychainApple_create_;
	r.destroy			= NBKeychainApple_destroy_;
	//
	r.addPassword		= NBKeychainApple_addPassword_;
	r.getPassword		= NBKeychainApple_getPassword_;
	r.removePassword	= NBKeychainApple_removePassword_;
	//
	/*r.addCert			= NBKeychainApple_addCert_;
	r.getCertByCN		= NBKeychainApple_getCertByCN_;
	r.getCertsIssuedByCert	= NBKeychainApple_getCertsIssuedByCert_;
	r.removeCertByCN	= NBKeychainApple_removeCertByCN_;*/
	return r;
}

void NBKeychainApple_create_(void** obj){
	STNBKeychainApple* itfParam = NBMemory_allocType(STNBKeychainApple);
	NBKeychainApple_init(itfParam);
	*obj = itfParam;
}

void NBKeychainApple_destroy_(void** obj){
	STNBKeychainApple* itfParam = (STNBKeychainApple*)*obj;
	NBKeychainApple_release(itfParam);
	NBMemory_free(itfParam);
	*obj = NULL;
}

//

BOOL NBKeychainApple_addPassword_(void* obj, const char* account, const char* passwd, const BOOL allowDevicesSync){
	return NBKeychainApple_addPassword((STNBKeychainApple*)obj, account, passwd, allowDevicesSync);
}

BOOL NBKeychainApple_getPassword_(void* obj, const char* account, STNBString* dst){
	return NBKeychainApple_getPassword((STNBKeychainApple*)obj, account, dst);
}

BOOL NBKeychainApple_removePassword_(void* obj, const char* account){
	return NBKeychainApple_removePassword((STNBKeychainApple*)obj, account);
}

//

/*BOOL NBKeychainApple_getCertByCN_(void* obj, const char* commonName, STNBX509* dst){
	return NBKeychainApple_getCertByCN((STNBKeychainApple*)obj, commonName, dst);
}

BOOL NBKeychainApple_getCertsIssuedByCert_(void* obj, const STNBX509* issuer, STNBArray* dst){ //STNBX509
	return NBKeychainApple_getCertsIssuedByCert((STNBKeychainApple*)obj, issuer, dst);
}

BOOL NBKeychainApple_addCert_(void* obj, const STNBX509* cert, const BOOL allowDevicesSync){
	return NBKeychainApple_addCert((STNBKeychainApple*)obj, cert, allowDevicesSync);
}

BOOL NBKeychainApple_removeCertByCN_(void* obj, const char* commonName){
	return NBKeychainApple_removeCertByCN((STNBKeychainApple*)obj, commonName);
}*/

