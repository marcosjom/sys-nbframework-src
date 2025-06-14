#ifndef NB_NTLN_H
#define NB_NTLN_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBString.h"

#ifdef __cplusplus
extern "C" {
#endif
	
#define NTLMSSP_NEGOTIATE_UNICODE						(0x1 << 0)
#define NTLM_NEGOTIATE_OEM								(0x1 << 1)
#define NTLMSSP_REQUEST_TARGET							(0x1 << 2)
#define NTLN_R10										(0x1 << 3)
#define NTLMSSP_NEGOTIATE_SIGN							(0x1 << 4)
#define NTLMSSP_NEGOTIATE_SEAL							(0x1 << 5)
#define NTLMSSP_NEGOTIATE_DATAGRAM						(0x1 << 6)
#define NTLMSSP_NEGOTIATE_LM_KEY						(0x1 << 7)
#define NTLN_R9											(0x1 << 8)
#define NTLMSSP_NEGOTIATE_NTLM							(0x1 << 9)
#define NTLN_R8											(0x1 << 10)
#define NTLN_CONN_ANONYMOUS								(0x1 << 11)
#define NTLMSSP_NEGOTIATE_OEM_DOMAIN_SUPPLIED			(0x1 << 12)
#define NTLMSSP_NEGOTIATE_OEM_WORKSTATION_SUPPLIED		(0x1 << 13)
#define NTLN_R7											(0x1 << 14)
#define NTLMSSP_NEGOTIATE_ALWAYS_SIGN					(0x1 << 15)
#define NTLMSSP_TARGET_TYPE_DOMAIN						(0x1 << 16)
#define NTLMSSP_TARGET_TYPE_SERVER						(0x1 << 17)
#define NTLN_R6											(0x1 << 18)
#define NTLMSSP_NEGOTIATE_EXTENDED_SESSIONSECURITY		(0x1 << 19)
#define NTLMSSP_NEGOTIATE_IDENTIFY						(0x1 << 20)
#define NTLN_R5											(0x1 << 21)
#define NTLMSSP_REQUEST_NON_NT_SESSION_KEY				(0x1 << 22)
#define NTLMSSP_NEGOTIATE_TARGET_INFO					(0x1 << 23)
#define NTLN_R4											(0x1 << 24)
#define NTLMSSP_NEGOTIATE_VERSION						(0x1 << 25)
#define NTLN_R3											(0x1 << 26)
#define NTLN_R2											(0x1 << 27)
#define NTLN_R1											(0x1 << 28)
#define NTLMSSP_NEGOTIATE_128							(0x1 << 29)
#define NTLMSSP_NEGOTIATE_KEY_EXCH						(0x1 << 30)
#define NTLMSSP_NEGOTIATE_56							(0x1 << 31)

//2.2.2.10 VERSION
typedef struct STNTLM_VERSION_ {
	BYTE	productMajorVersion;
	BYTE	productMinorVersion;
	UI16	productBuild;
	BYTE	r0;
	BYTE	r1;
	BYTE	r2;
	BYTE	nTLMRevisionCurrent;
} STNTLM_VERSION;

//2.2.1.1 NEGOTIATE_MESSAGE
typedef struct STNTLM_NEGOTIATE_MESSAGE_ {
	char signature[8];		//must be ['N', 'T', 'L', 'M', 'S', 'S', 'P', '\0']
	UI32 messageType;		//must be  0x00000001
	UI32 negotiateFlags;
	UI16 domainNameLen;
	UI16 domainNameMaxLen;
	UI32 domainNameBufferOffset;
	UI16 workstationLen;
	UI16 workstationMaxLen;
	UI32 workstationBufferOffset;
	STNTLM_VERSION version;			//used only for debig purposes
	//Payload follows
} STNTLM_NEGOTIATE_MESSAGE;

//2.2.1.2 CHALLENGE_MESSAGE
typedef struct STNTLM_CHALLENGE_MESSAGE_ {
	char signature[8];		//must be ['N', 'T', 'L', 'M', 'S', 'S', 'P', '\0']
	UI32 messageType;		//must be  0x00000002
	UI16 targetNameLen;
	UI16 targetNameMaxLen;
	UI32 targetNameBufferOffset;
	UI32 negotiateFlags;
	UI64 serverChallenge;
	UI64 reserved;			//must be zero
	UI16 targetInfoLen;
	UI16 targetInfoMaxLen;
	UI32 targetInfoBufferOffset;
	STNTLM_VERSION version;			//used only for debig purposes
	//Payload follows
} STNTLM_CHALLENGE_MESSAGE;

//2.2.1.3 AUTHENTICATE_MESSAGE
typedef struct STNTLM_AUTHENTICATE_MESSAGE_ {
	char signature[8];		//must be ['N', 'T', 'L', 'M', 'S', 'S', 'P', '\0']
	UI32 messageType;		//must be  0x00000003
	//
	UI16 lmChallengeResponseLen;
	UI16 lmChallengeResponseMaxLen;
	UI32 lmChallengeResponseBufferOffset;
	//
	UI16 ntChallengeResponseLen;
	UI16 ntChallengeResponseMaxLen;
	UI32 ntChallengeResponseBufferOffset;
	//
	UI16 domainNameLen;
	UI16 domainNameMaxLen;
	UI32 domainNameBufferOffset;
	//
	UI16 userNameLen;
	UI16 userNameMaxLen;
	UI32 userNameBufferOffset;
	//
	UI16 workstationNameLen;
	UI16 workstationNameMaxLen;
	UI32 workstationNameBufferOffset;
	//
	UI16 encRandSessionKeyLen;
	UI16 encRandSessionKeyMaxLen;
	UI32 encRandSessionKeyBufferOffset;
	//
	UI32 negotiateFlags;
	STNTLM_VERSION version;		//zero, used only for debig purposes
	BYTE mic[16];				//message integrity for the NTLM NEGOTIATE_MESSAGE, CHALLENGE_MESSAGE, and AUTHENTICATE_MESSAGE
	//Payload follows
} STNTLM_AUTHENTICATE_MESSAGE;

//2.2.2.1 AV_PAIR
typedef struct STNTLM_AVPAIR_ {
	UI16 avId;
	UI16 avLen;
	//Payload follows
} STNTLM_AVPAIR;

//2.2.2.2
typedef struct STNTLM_Single_Host_Data_ {
	UI32	size;
	UI32	z4;
	char	customData[8];
	char	machineId[32];
} STNTLM_Single_Host_Data;

//2.2.2.6 NTLM_RESPONSE
typedef struct STNTLM_RESPONSE_ {
	BYTE resp[24];
} STNTLM_RESPONSE;

typedef struct STNTLM_FILETIME_ {
	UI32 dwLowDateTime;	//DWORD
	UI32 dwHighDateTime;	//DWORD
} STNTLM_FILETIME;
	
void NBNtln_concatNTOWFv2(STNBString* dst, const char* passUtf8, const char* userUtf8, const char* domainUtf8);
void NBNtln_concatLMOWFv2(STNBString* dst, const char* passUtf8, const char* userUtf8, const char* domainUtf8);
BOOL NBNtln_concatSIGNKEY(STNBString* dst, const UI32 negFlg, const void* exportedSessionKey, const UI32 exportedSessionKeySz, const char* mode);
BOOL NBNtln_concatAuthMessageBin(const void* cltNegBin, const UI32 cltNegBinSz, const void* srvChallengeBin, const UI32 srvChallengeBinSz, const char* user, const char* pass, const char* domain, const char* workstation, const UI64 clientChallenge, STNBString* dstAuthBin);

STNTLM_VERSION NBNtln_getDefaultVersionClt(void);

//

void NBNtln_printFlags(const UI32 negFlags);

#ifdef __cplusplus
} //extern "C"
#endif


#endif
