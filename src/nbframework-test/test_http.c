//
//  test_http.c
//  nbframework-test-osx
//
//  Created by Marcos Ortega on 19/12/21.
//

#include "test_http.h"

//Client
/*
BOOL Test_cltConnected_(STNBHttpServicePortRef ref, const STNBHttpServiceConn* cltRef, IHttpMessageListener* dstMsgListener, void** dstMsgListenerParam, void* lparam){
	return TRUE;
}
*/

/*
BOOL Test_cltDisconnected_(STNBHttpServicePortRef ref, void* msgListenerParam, void* lparam){
	return TRUE;
}
*/

//Actions
/*
void Test_cltWillDisconnect_(STNBHttpServicePortRef ref, void* msgListenerParam, void* lparam){
	//Nothing
}
*/

//Request
/*
BOOL _sendRequest(STNBSslRef ssl, STNBHttpRequest* req, const char* server, const SI32 port, const char* target, const char* httpBoundTag, STNBHttpResponse* dstRsp){
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
			NBString_replace(&tmpOut, "\r\n", "\n");
			PRINTF_INFO("Request sent:\n%s.\n", tmpOut.str);
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
						NBString_replace(&tmpIn, "\r\n", "\n");
						PRINTF_INFO("Response recvd:\n%s.\n", tmpIn.str);
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
*/

/*
BOOL Test_cltRequested_(STNBHttpServicePortRef ref, const STNBHttpMessage* msg, const IHttpServicePortSender* sendrItf, void* sendrParam, void* msgListenerParam, void* lparam){
	BOOL r = FALSE;
	STNBString str, resp;
	STNBHttpBuilder bldr;
	NBString_init(&str);
	NBString_init(&resp);
	NBHttpBuilder_init(&bldr);
	NBHttpBuilder_addStatusLine(&bldr, &str, 1, 1, 200, "OK");
	{
		NBString_concat(&resp, "");
		STNBString num;
		NBString_init(&num);
		NBString_concatUI32(&num, resp.length);
		NBHttpBuilder_addHeaderField(&bldr, &str, "Content-Length", num.str);
		NBString_release(&num);
	}
	NBHttpBuilder_addHeaderEnd(&bldr, &str);
	NBString_concat(&str, resp.str);
	if(sendrItf->sendBytes == NULL){
		r = FALSE; NBASSERT(FALSE)
	} else {
		if(!(*sendrItf->sendBytes)(sendrParam, str.str, str.length)){
			r = FALSE; NBASSERT(FALSE)
		} else {
			r = TRUE;
		}
	}
	NBHttpBuilder_release(&bldr);
	NBString_release(&resp);
	NBString_release(&str);
	return r;
}
*/
//Test HTTPS service
/*if(FALSE){
	//Temp
	SSL_library_init();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();
	ERR_load_crypto_strings();
	//
	STNBSslContextRef srvrCtxt = NBSslContext_alloc(NULL);
	if(!NBSslContext_create(&srvrCtxt, NBSslContext_getServerMode)){
		PRINTF_ERROR("Server, could not create SSLContext.\n");
	} else {
		if(!NBSslContext_attachCertAndkey(&srvrCtxt, "localhost.cert.der", "localhost.key.der", "signit")){
			PRINTF_ERROR("Server, could not attach certAndKey to context.\n");
		} else {
			PRINTF_INFO("Server, SSLContext ready.\n");
			STNBHttpServicePort srvc;
			NBHttpServicePort_init(&srvc);
			//Configure
			{
				STNBHttpPortCfg cfg;
				NBMemory_set(&cfg, 0, sizeof(cfg));
				cfg.maxs.connsIn			= 0; //1, testing thread in ssl
				cfg.maxs.bps.in				= 0; //(10 * 1024);
				cfg.maxs.bps.out			= 0; //(10 * 1024);
				cfg.maxs.secsIdle			= 30;
				cfg.maxsPerConn.secsIdle	= 10;
				NBHttpServicePort_loadFromConfig(&srvc, &cfg);
			}
			//Interface
			{
				STNBHttpServicePortLstnr itf;
				NBMemory_set(&itf, 0, sizeof(itf));
				itf.obj = NULL;
				itf.cltConnected			= Test_cltConnected_;
				itf.cltDisconnected		= Test_cltDisconnected_;
				itf.cltWillDisconnect	= Test_cltWillDisconnect_;
				itf.cltRequested			= Test_cltRequested_;
				NBHttpServicePort_setListener(&srvc, &itf);
			}
			NBHttpServicePort_setSslContext(&srvc, &srvrCtxt);
			//Start
			const UI32 port = 8080;
			if(!NBHttpServicePort_bind(&srvc, port)){
				PRINTF_ERROR("Could not bind port %d.\n", port);
			} else {
				if(!NBHttpServicePort_listen(&srvc)){
					PRINTF_ERROR("Could not listen.\n");
				} else {
					PRINTF_INFO("Listening port %d.\n", port);
					NBHttpServicePort_setBuffersSzs(&srvc, NB_PROXY_NET_READ_BUFF_SZ, NB_PROXY_HTTP_BODY_READ_BUFF_SZ, NB_PROXY_HTTP_BODY_STORG_BUFF_SZ);
					//LOG_INFO("MAIN, config loaded.\n");
#						ifndef _WIN32
					{
						//Ignore SIGPIPE for this thread (for unix-like systems)
						{
							struct sigaction act;
							act.sa_handler = SIG_IGN;
							act.sa_flags	= 0;
							sigemptyset(&act.sa_mask);
							sigaction(SIGPIPE, &act, NULL);
						}
						//activate signal handlers
						{
							struct sigaction act;
							act.sa_handler	= intHandler;
							act.sa_flags	= 0;
							sigemptyset(&act.sa_mask);
							sigaction(SIGINT, &act, NULL);
						}
						{
							struct sigaction act;
							act.sa_handler	= intHandler;
							act.sa_flags	= 0;
							sigemptyset(&act.sa_mask);
							sigaction(SIGQUIT, &act, NULL);
						}
					}
#						endif
					if(!NBHttpServicePort_execute(&srvc)){
						PRINTF_ERROR("Could not start server.\n");
					} else {
						NBHttpServicePort_stop(&srvc);
					}
				}
			}
			NBHttpServicePort_release(&srvc);
			PRINTF_INFO("End-of-server.\n");
		}
	}
	NBSslContext_release(&srvrCtxt);
}*/
//Test NBHttpProxy
/*if(FALSE){
	NBHttpProxy_init(&srvc);
	//Configure
	{
		STNBHttpProxyCfg cfg;
		NBMemory_set(&cfg, 0, sizeof(cfg));
		cfg.service.maxs.connsIn			= 0;
		cfg.service.maxs.bps.in				= 0; //(10 * 1024);
		cfg.service.maxs.bps.out			= 0; //(10 * 1024);
		cfg.service.maxs.secsIdle			= 30;
		cfg.service.maxsPerConn.secsIdle	= 10;
		NBHttpProxy_setConfig(&srvc, &cfg);
	}
	//Start
	const UI32 port = 4499;
	if(!NBHttpProxy_bind(&srvc, port)){
		PRINTF_ERROR("Could not bind port %d.\n", port);
	} else {
		if(!NBHttpProxy_listen(&srvc)){
			PRINTF_ERROR("Could not listen.\n");
		} else {
			PRINTF_INFO("Listening port %d.\n", port);
			NBHttpProxy_setBuffersSzs(&srvc, NB_PROXY_NET_READ_BUFF_SZ, NB_PROXY_HTTP_BODY_READ_BUFF_SZ, NB_PROXY_HTTP_BODY_STORG_BUFF_SZ);
			//LOG_INFO("MAIN, config loaded.\n");
#				ifndef _WIN32
			{
				//Ignore SIGPIPE for this thread (for unix-like systems)
				{
					struct sigaction act;
					act.sa_handler = SIG_IGN;
					act.sa_flags	= 0;
					sigemptyset(&act.sa_mask);
					sigaction(SIGPIPE, &act, NULL);
				}
				//activate signal handlers
				{
					struct sigaction act;
					act.sa_handler	= intHandler;
					act.sa_flags	= 0;
					sigemptyset(&act.sa_mask);
					sigaction(SIGINT, &act, NULL);
				}
				{
					struct sigaction act;
					act.sa_handler	= intHandler;
					act.sa_flags	= 0;
					sigemptyset(&act.sa_mask);
					sigaction(SIGQUIT, &act, NULL);
				}
			}
#				endif
			if(!NBHttpProxy_execute(&srvc)){
				PRINTF_ERROR("Could not start server.\n");
			} else {
				NBHttpProxy_waitForAll(&srvc);
			}
		}
	}
	NBHttpProxy_release(&srvc);
	PRINTF_INFO("End-of-server.\n");
}*/

//HTTP/2 HPACK test
/*{
	//Test primitives
	{
		STNBString str, str2, str3;
		NBString_init(&str);
		NBString_init(&str2);
		NBString_init(&str3);
		//Int encode/decode
		{
			UI32 iRun = 0, testRuns = 4096;
			while(iRun < testRuns){
				const UI64 valOrg = rand();
				const UI8 prefix = 1 + (rand() % 7);
				UI32 iPos = (rand() % 128);
				//Build random start string
				{
					const char* chars = "abcdefghijk1234567890";
					const UI32 charsSz = NBString_strLenBytes(chars);
					NBString_empty(&str);
					while(str.length < iPos){
						NBString_concatByte(&str, chars[rand() % charsSz]);
					}
				}
				//Dirty first byte
				if(!NBHttp2Hpack_encodeInteger(prefix, valOrg, &str)){
					NBASSERT(FALSE);
				} else {
					UI64 valDec = 0;
					if(!NBHttp2Hpack_decodeInteger(prefix, &iPos, str.str, str.length, &valDec)){
						NBASSERT(FALSE);
					} else {
						NBASSERT(valOrg == valDec)
						NBASSERT(iPos == str.length)
						PRINTF_INFO("TestRun %d success, value(%llu) prefix(%d), encoded(%d bytes).\n", iRun, valOrg, prefix, str.length);
					}
				}
				iRun++;
			}
		}
		//String encode/decode
		{
			const char* chars = "abcdefghijk1234567890";
			const UI32 charsSz = NBString_strLenBytes(chars);
			UI32 iRun = 0, testRuns = 4096;
			while(iRun < testRuns){
				NBString_empty(&str);
				NBString_empty(&str2);
				NBString_empty(&str3);
				//Original value
				{
					const UI32 len = (rand() % 64);
					SI32 i; for(i = 0; i < len; i++){
						NBString_concatByte(&str, chars[rand() % charsSz]);
					}
				}
				//Encode
				if(!NBHttp2Hpack_encodeStrLiteral(str.str, str.length, FALSE, &str2)){
					NBASSERT(FALSE);
				} else {
					//Decode
					UI32 iPos = 0;
					BOOL isHuffman = FALSE;
					if(!NBHttp2Hpack_decodeStrLiteral(&iPos, str2.str, str2.length, &str3, &isHuffman)){
						NBASSERT(FALSE);
					} else {
						NBASSERT(!isHuffman)
						NBASSERT(str.length == str3.length)
						NBASSERT(iPos == str2.length)
						NBASSERT(NBString_strIsEqual(str.str, str3.str))
						PRINTF_INFO("TestRun %d success, encoded(%d bytes): '%s'.\n", iRun, str2.length, str3.str);
					}
				}
				iRun++;
			}
		}
		//HPACK
		{
			STNBHttpBuilder bldr;
			STNBHttpHeader pairs, pairs2;
			STNBHttp2Hpack hpack, hpack2;
			NBHttpBuilder_init(&bldr);
			NBHttpHeader_init(&pairs);
			NBHttpHeader_init(&pairs2);
			NBHttp2Hpack_init(&hpack);
			NBHttp2Hpack_init(&hpack2);
			{
				const char* charsNames = "abcdef";
				const char* charsValues = "01234567890";
				const UI32 charsNamesSz = NBString_strLenBytes(charsNames);
				const UI32 charsValuesSz = NBString_strLenBytes(charsValues);
				UI32 iRun = 0, testRuns = 40961;
				while(iRun < testRuns){
					//Reset hpack states
					{
						NBHttp2Hpack_release(&hpack2);
						NBHttp2Hpack_release(&hpack);
						//
						NBHttp2Hpack_init(&hpack);
						NBHttp2Hpack_init(&hpack2);
					}
					//Run messages test
					{
						UI32 iMsg = 0, testMsgs = 256;
						while(iMsg < testMsgs){
							STNBHttp2Hpack* hpackSrc = ((iMsg % 2) == 0 ? &hpack : &hpack2);
							STNBHttp2Hpack* hpackDst = ((iMsg % 2) == 0 ? &hpack2 : &hpack);
							//Reset
							{
								NBHttpHeader_release(&pairs2);
								NBHttpHeader_release(&pairs);
								//
								NBHttpHeader_init(&pairs);
								NBHttpHeader_init(&pairs2);
							}
							//Build pairs
							{
								const UI32 pairsCount = 1 + (rand() % 32);
								SI32 i; for(i = 0; i < pairsCount; i++){
									NBString_empty(&str);
									NBString_empty(&str2);
									if((rand() % 5) <= 1){
										//Use index
										const UI32 idx = 1 + (rand() % (NBHttp2Hpack_tablesTotalSizes(hpackSrc) - 1));
										const STNBHttp2HpackTableItmVal val = NBHttp2Hpack_getItmValue(hpackSrc, idx); NBASSERT(!val.isError)
										NBString_concat(&str, val.name);
										NBString_concat(&str2, val.value);
									}
									//Build name
									if(str.length <= 0){
										const UI32 len = 1 + (rand() % 4);
										SI32 i; for(i = 0; i < len; i++){
											NBString_concatByte(&str, charsNames[rand() % charsNamesSz]);
										}
									}
									//Build value
									if(str2.length <= 0){
										const UI32 len = 1 + (rand() % 4);
										SI32 i; for(i = 0; i < len; i++){
											NBString_concatByte(&str2, charsValues[rand() % charsValuesSz]);
										}
									}
									//Add pair
									NBHttpHeader_addField(&pairs, str.str, str2.str);
								}
							}
							//Build to string
							{
								NBHttpBuilder_release(&bldr);
								NBHttpBuilder_init(&bldr);
								NBString_empty(&str);
								NBHttpBuilder_addHeaderFieldsPlain(&bldr, &str, &pairs);
								//PRINTF_INFO("Original list:\n\n-->\n%s<--\n\n", str.str);
							}
							//Encode
							{
								NBString_empty(&str2);
								NBHttp2Hpack_encodeBlock(hpackSrc, &pairs, &str2);
								/ *{
									STNBString arr;
									NBString_init(&arr);
									NBString_concat(&arr, "BYTE resp[] = {\n");
									{
										SI32 i; for(i = 0; i < str2.length; i++){
											const BYTE bb = *((BYTE*)&str2.str[i]);
											if(i != 0) NBString_concat(&arr, ", ");
											NBString_concatUI32(&arr, bb);
											if(((i + 1) % 8) == 0) NBString_concat(&arr, "\n");
										}
									}
									NBString_concat(&arr, "\n}; ");
									PRINTF_INFO("Encoded to %d bytes:\n%s\n", str2.length, arr.str);
									NBString_release(&arr);
								}* /
							}
							//Decode
							{
								NBHttp2Hpack_decodeBlock(hpackDst, str2.str, str2.length, &pairs2);
							}
							//Build to string
							{
								NBHttpBuilder_release(&bldr);
								NBHttpBuilder_init(&bldr);
								NBString_empty(&str3);
								NBHttpBuilder_addHeaderFieldsPlain(&bldr, &str3, &pairs2);
								//PRINTF_INFO("Decoded list:\n\n-->\n%s<--\n\n", str3.str);
							}
							//Validate
							{
								NBASSERT(hpack.tblDynamic.use == hpack2.tblDynamic.use)
								NBASSERT(hpack.strs.length == hpack2.strs.length)
								if(!NBString_strIsEqual(str.str, str3.str)){
									PRINTF_INFO("TestMsg %d/%d error, header-org(%d) encoded(%d bytes) header-dec(%d):\n\norig---->%s<----\n\ndecoded---->%s<----\n", iRun, iMsg, str.length, str2.length, str3.length, str.str, str3.str);
									NBASSERT(FALSE)
								} else {
									PRINTF_INFO("TestMsg %d/%d success, header(%d) encoded(%d bytes).\n", iRun, iMsg, str.length, str2.length);
								}
							}
							//Next
							iMsg++;
						}
					}
					iRun++;
				}
			}
			NBHttp2Hpack_release(&hpack2);
			NBHttp2Hpack_release(&hpack);
			NBHttpHeader_release(&pairs2);
			NBHttpHeader_release(&pairs);
			NBHttpBuilder_release(&bldr);
		}
		NBString_release(&str3);
		NBString_release(&str2);
		NBString_release(&str);
	}
	//
	//BOOL NBHttp2Hpack_encodeInteger(const UI8 prefixN, const UI64 pValue, STNBString* dst);
	//BOOL NBHttp2Hpack_decodeInteger(const UI8 prefixN, UI32* iPos, const void* pData, const UI32 dataSz, UI64* dst);
	//
	//SETTINGS FRAME (0x4)
	//const BYTE resp[] = { 0, 0, 24, 4, 0, 0, 0, 0, 0, 0, 1, 0, 0, 16, 0, 0, 3, 0, 0, 3, 232, 0, 5, 0, 0, 64, 0, 0, 6, 0, 0, 31, 64 };
	//
	//const BYTE resp[] = {0, 0, 33, 7, 0, 0, 0, 0, 0,      0, 0, 0, 0, 0, 0, 0, 1, 65, 32, 115, 116, 114, 101, 97, 109, 32, 73, 68, 32, 109, 117, 115, 116, 32, 98, 101, 32, 122, 101, 114, 111, 46 };
	/ *STNBHttp2Hpack hpack;
	NBHttp2Hpack_init(&hpack);
	if(!NBHttp2Hpack_decode(&hpack, resp, sizeof(resp))){
		PRINTF_ERROR("HPACK could not be decoded.\n");
	} else {
		PRINTF_INFO("HPACK decoded.\n");
	}
	NBHttp2Hpack_release(&hpack);* /
}*/
