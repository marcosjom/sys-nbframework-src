
#include "nb/NBFrameworkPch.h"
#include "nb/net/NBHttpRequest.h"
#include "nb/core/NBMemory.h"

//Factory
void NBHttpRequest_init(STNBHttpRequest* obj){
	NBMemory_setZeroSt(*obj, STNBHttpRequest);
	//
	NBString_initWithStrBytes(&obj->protocol, "HTTP", 4);
	obj->verMajor			= 1;
	obj->verMinor			= 1;
	obj->addImplicitParams	= TRUE;
	obj->addParamUserAgent	= TRUE;
	obj->addParamHost		= TRUE;
	NBString_init(&obj->method);
	obj->postFormat		= ENNBHttpPOSTFormat_urlencoded;
    NBArray_init(&obj->headers, sizeof(STNBHttpParam), NULL);
    NBArray_init(&obj->paramsGET, sizeof(STNBHttpParam), NULL);
    NBArray_init(&obj->paramsPOST, sizeof(STNBHttpParam), NULL);
    NBString_init(&obj->contentType);
    NBString_init(&obj->content);
	//
	obj->retainCount = 1;
}

void NBHttpRequest_initWithOther(STNBHttpRequest* obj, const STNBHttpRequest* other){
	if(other == NULL){
		NBHttpRequest_init(obj);
	} else {
		{
			NBString_initWithOther(&obj->protocol, &other->protocol);
		}
		obj->verMajor			= other->verMajor;
		obj->verMinor			= other->verMinor;
		obj->addImplicitParams	= other->addImplicitParams;
		obj->addParamUserAgent	= other->addParamUserAgent;
		obj->addParamHost		= other->addParamHost;
		{
			NBString_initWithOther(&obj->method, &other->method);
		}
		obj->postFormat	= other->postFormat;
		{
			NBArray_initWithSz(&obj->headers, sizeof(STNBHttpParam), NULL, other->headers.use, 8, 0.1f);
			NBHttpRequest_addParamsCopy(&obj->headers, &other->headers);
		}
		{
			NBArray_initWithSz(&obj->paramsGET, sizeof(STNBHttpParam), NULL, other->paramsGET.use, 8, 0.1f);
			NBHttpRequest_addParamsCopy(&obj->paramsGET, &other->paramsGET);
		}
		{
			NBArray_initWithSz(&obj->paramsPOST, sizeof(STNBHttpParam), NULL, other->paramsPOST.use, 8, 0.1f);
			NBHttpRequest_addParamsCopy(&obj->paramsPOST, &other->paramsPOST);
		}
		NBString_initWithOther(&obj->contentType, &other->contentType);
		NBString_initWithOther(&obj->content, &other->content);
		//
		obj->retainCount = 1;
	}
}

void NBHttpRequest_retain(STNBHttpRequest* obj){
    NBASSERT(obj->retainCount > 0) //Still alive
	obj->retainCount++;
}

void NBHttpRequest_release(STNBHttpRequest* obj){
    NBASSERT(obj->retainCount > 0)
	obj->retainCount--;
	if(obj->retainCount == 0){
		NBString_release(&obj->protocol);
		NBString_release(&obj->method);
	    NBString_release(&obj->contentType);
	    NBString_release(&obj->content);
		//
		{
			SI32 i; for(i = 0; i < obj->paramsGET.use; i++){
				STNBHttpParam* param = (STNBHttpParam*)NBArray_itemAtIndex(&obj->paramsGET, i);
				NBString_release(&param->name);
				if(param->valueStorageType == ENNBHttpParamStorage_Own){
					NBString_release(param->value);
					NBMemory_free(param->value);
				}
			}
			NBArray_release(&obj->paramsGET);
		}
		//
		{
			SI32 i; for(i = 0; i < obj->paramsPOST.use; i++){
				STNBHttpParam* param = (STNBHttpParam*)NBArray_itemAtIndex(&obj->paramsPOST, i);
				NBString_release(&param->name);
				if(param->valueStorageType == ENNBHttpParamStorage_Own){
					NBString_release(param->value);
					NBMemory_free(param->value);
				}
			}
			NBArray_release(&obj->paramsPOST);
		}
		//
		{
			SI32 i; for(i = 0; i < obj->headers.use; i++){
				STNBHttpParam* param = (STNBHttpParam*)NBArray_itemAtIndex(&obj->headers, i);
				NBString_release(&param->name);
				if(param->valueStorageType == ENNBHttpParamStorage_Own){
					NBString_release(param->value);
					NBMemory_free(param->value);
				}
			}
			NBArray_release(&obj->headers);
		}
	}
}

//Build

void NBHttpRequest_setProtocol(STNBHttpRequest* obj, const char* protocol){
	if(NBString_strIsEmpty(protocol)){
		NBString_empty(&obj->protocol);
	} else {
		NBString_set(&obj->protocol, protocol);
	}
}

void NBHttpRequest_setProtocolVer(STNBHttpRequest* obj, const ENNBHttpVer httpVer){
	switch (httpVer) {
	case ENNBHttpVer_1_0:
		obj->verMajor	= 1;
		obj->verMinor	= 0;
		break;
	case ENNBHttpVer_1_1:
		obj->verMajor	= 1;
		obj->verMinor	= 1;
		break;
	default:
		NBASSERT(FALSE)
		break;
	}
}

void NBHttpRequest_setProtocolVerX(STNBHttpRequest* obj, const UI8 majorVer, const UI8 minorVer){
	obj->verMajor	= majorVer;
	obj->verMinor	= minorVer;
}

void NBHttpRequest_setAddImplicitParams(STNBHttpRequest* obj, const BOOL addImplicitParams){
	obj->addImplicitParams = addImplicitParams;
}

void NBHttpRequest_setAddParamUserAgent(STNBHttpRequest* obj, const BOOL addParamUserAgent){
	obj->addParamUserAgent = addParamUserAgent;
}

void NBHttpRequest_setAddParamHost(STNBHttpRequest* obj, const BOOL addParamHost){
	obj->addParamHost = addParamHost;
}

void NBHttpRequest_setMethod(STNBHttpRequest* obj, const ENNBHttpMethod httpMethod){
	switch (httpMethod) {
	case ENNBHttpMethod_AUTO:
		NBString_empty(&obj->method);
		break;
	case ENNBHttpMethod_GET:
		NBString_set(&obj->method, "GET");
		break;	
	case ENNBHttpMethod_POST:
		NBString_set(&obj->method, "POST");
		break;
	case ENNBHttpMethod_PUT:
		NBString_set(&obj->method, "PUT");
		break;
	case ENNBHttpMethod_DELETE:
		NBString_set(&obj->method, "DELETE");
		break;
	default:
		NBASSERT(FALSE)
		break;
	}
}

void NBHttpRequest_setMethodX(STNBHttpRequest* obj, const char* method){
	if(NBString_strIsEmpty(method)){
		NBString_empty(&obj->method);
	} else {
		NBString_set(&obj->method, method);
	}
}

void NBHttpRequest_setContentType(STNBHttpRequest* obj, const char* contentType){
    NBString_empty(&obj->contentType);
    NBString_concat(&obj->contentType, contentType);
}

void NBHttpRequest_emptyContent(STNBHttpRequest* obj){
    NBString_empty(&obj->content);
}

void NBHttpRequest_setContent(STNBHttpRequest* obj, const char* body){
    NBString_empty(&obj->content);
    NBString_concat(&obj->content, body);
}

void NBHttpRequest_concatContent(STNBHttpRequest* obj, const char* bodyPart){
    NBString_concat(&obj->content, bodyPart);
}

void NBHttpRequest_concatContentBytes(STNBHttpRequest* obj, const char* bodyPart, const UI32 bytes){
    NBString_concatBytes(&obj->content, bodyPart, bytes);
}

void NBHttpRequest_concatContentUI64(STNBHttpRequest* obj, const UI64 val){
    NBString_concatUI64(&obj->content, val);
}

void NBHttpRequest_addHeader(STNBHttpRequest* obj, const char* name, const char* value){
	if(NBString_strIsLike(name, "Content-Type")){
		NBHttpRequest_setContentType(obj, value);
	} else {
		//Update if already exists
		SI32 i; for(i = 0; i < obj->headers.use; i++){
			STNBHttpParam* param = (STNBHttpParam*)NBArray_itemAtIndex(&obj->headers, i);
			if(NBString_strIsLike(param->name.str, name)){
				if(param->valueStorageType == ENNBHttpParamStorage_Own){
					NBString_empty(param->value);
					NBString_concat(param->value, value);
				} else {
					param->valueStorageType = ENNBHttpParamStorage_Own;
					param->value = (STNBString*)NBMemory_alloc(sizeof(STNBString));
					NBString_initWithStr(param->value, value);
				}
				break;
			}
		}
		//Add new param
		if(i == obj->headers.use){
			STNBHttpParam newParam;
			NBString_initWithStr(&newParam.name, name);
			newParam.valueStorageType = ENNBHttpParamStorage_Own;
			newParam.value = (STNBString*)NBMemory_alloc(sizeof(STNBString));
			NBString_initWithStr(newParam.value, value);
			NBArray_addValue(&obj->headers, newParam);
		}
	}
}

BOOL NBHttpRequest_headerIsPresent(const STNBHttpRequest* obj, const char* name){
	BOOL r = FALSE;
	if(NBString_strIsLike(name, "Content-Type")){
		r = (obj->contentType.length > 0);
	} else {
		//Search
		SI32 i; for(i = 0; i < obj->headers.use; i++){
			STNBHttpParam* param = (STNBHttpParam*)NBArray_itemAtIndex(&obj->headers, i);
			if(NBString_strIsLike(param->name.str, name)){
				r = TRUE;
				break;
			}
		}
	}
	return r;
}

void NBHttpRequest_addParamGET(STNBHttpRequest* obj, const char* name, const char* value){
	//Update if already exists
	SI32 i; for(i = 0; i < obj->paramsGET.use; i++){
		STNBHttpParam* param = (STNBHttpParam*)NBArray_itemAtIndex(&obj->paramsGET, i);
		if(NBString_strIsEqual(param->name.str, name)){
			if(param->valueStorageType == ENNBHttpParamStorage_Own){
			    NBString_empty(param->value);
			    NBString_concat(param->value, value);
			} else {
				param->valueStorageType = ENNBHttpParamStorage_Own;
				param->value = (STNBString*)NBMemory_alloc(sizeof(STNBString));
			    NBString_initWithStr(param->value, value);
			}
			break;
		}
	}
	//Add new param
	if(i == obj->paramsGET.use){
		STNBHttpParam newParam;
	    NBString_initWithStr(&newParam.name, name);
		newParam.valueStorageType = ENNBHttpParamStorage_Own;
		newParam.value = (STNBString*)NBMemory_alloc(sizeof(STNBString));
	    NBString_initWithStr(newParam.value, value);
	    NBArray_addValue(&obj->paramsGET, newParam);
	}
}

void NBHttpRequest_addParamPOST(STNBHttpRequest* obj, const char* name, const char* value){
	//Update if already exists
	SI32 i; for(i = 0; i < obj->paramsPOST.use; i++){
		STNBHttpParam* param = (STNBHttpParam*)NBArray_itemAtIndex(&obj->paramsPOST, i);
		if(NBString_strIsEqual(param->name.str, name)){
			if(param->valueStorageType == ENNBHttpParamStorage_Own){
			    NBString_empty(param->value);
			    NBString_concat(param->value, value);
			} else {
				param->valueStorageType = ENNBHttpParamStorage_Own;
				param->value = (STNBString*)NBMemory_alloc(sizeof(STNBString));
			    NBString_initWithStr(param->value, value);
			}
			break;
		}
	}
	//Add new param
	if(i == obj->paramsPOST.use){
		STNBHttpParam newParam;
	    NBString_initWithStr(&newParam.name, name);
		newParam.valueStorageType = ENNBHttpParamStorage_Own;
		newParam.value = (STNBString*)NBMemory_alloc(sizeof(STNBString));
	    NBString_initWithStr(newParam.value, value);
	    NBArray_addValue(&obj->paramsPOST, newParam);
	}
}

void NBHttpRequest_addHeaderStr(STNBHttpRequest* obj, const char* name, STNBString* value, const ENNBHttpParamStorage valueStorageType){
	if(NBString_strIsLike(name, "Content-Type")){
		NBHttpRequest_setContentType(obj, value->str);
	} else {
		//Update if already exists
		SI32 i; for(i = 0; i < obj->headers.use; i++){
			STNBHttpParam* param = (STNBHttpParam*)NBArray_itemAtIndex(&obj->headers, i);
			if(NBString_strIsLike(param->name.str, name)){
				//Release if necesary
				if(param->valueStorageType == ENNBHttpParamStorage_Own){
					NBString_empty(param->value);
					NBString_release(param->value);
					NBMemory_free(param->value);
					param->value = NULL;
				}
				//Set new value
				if(valueStorageType == ENNBHttpParamStorage_External) {
					param->valueStorageType = valueStorageType;
					param->value = value;
				} else {
					param->valueStorageType = ENNBHttpParamStorage_Own;
					param->value = (STNBString*)NBMemory_alloc(sizeof(STNBString));
					NBString_initWithOther(param->value, value);
				}
				break;
			}
		}
		//Add new param
		if(i == obj->headers.use){
			STNBHttpParam newParam;
			NBString_initWithStr(&newParam.name, name);
			if(valueStorageType == ENNBHttpParamStorage_External) {
				newParam.valueStorageType = valueStorageType;
				newParam.value = value;
			} else {
				newParam.valueStorageType = ENNBHttpParamStorage_Own;
				newParam.value = (STNBString*)NBMemory_alloc(sizeof(STNBString));
				NBString_initWithOther(newParam.value, value);
			}
			NBArray_addValue(&obj->headers, newParam);
		}
	}
}

void NBHttpRequest_addParamGETStr(STNBHttpRequest* obj, const char* name, STNBString* value, const ENNBHttpParamStorage valueStorageType){
	//TODO: update param if already exists
	STNBHttpParam newParam;
    NBString_initWithStr(&newParam.name, name);
	if(valueStorageType == ENNBHttpParamStorage_External) {
		newParam.valueStorageType = valueStorageType;
		newParam.value = value;
	} else {
		newParam.valueStorageType = ENNBHttpParamStorage_Own;
		newParam.value = (STNBString*)NBMemory_alloc(sizeof(STNBString));
	    NBString_initWithOther(newParam.value, value);
	}
    NBArray_addValue(&obj->paramsGET, newParam);
}

void NBHttpRequest_addParamPOSTStr(STNBHttpRequest* obj, const char* name, STNBString* value, const ENNBHttpParamStorage valueStorageType){
	//TODO: update param if already exists
	STNBHttpParam newParam;
    NBString_initWithStr(&newParam.name, name);
	if(valueStorageType == ENNBHttpParamStorage_External) {
		newParam.valueStorageType = valueStorageType;
		newParam.value = value;
	} else {
		newParam.valueStorageType = ENNBHttpParamStorage_Own;
		newParam.value = (STNBString*)NBMemory_alloc(sizeof(STNBString));
	    NBString_initWithOther(newParam.value, value);
	}
    NBArray_addValue(&obj->paramsPOST, newParam);
}

//

void NBHttpRequest_addParamsCopy(STNBArray* dst, const STNBArray* src){ //STNBHttpParam
	SI32 i; for(i = 0; i < src->use; i++){
		STNBHttpParam* param = (STNBHttpParam*)NBArray_itemAtIndex(src, i);
		{
			STNBHttpParam newParam;
			NBString_initWithStr(&newParam.name, param->name.str);
			newParam.valueStorageType = ENNBHttpParamStorage_Own;
			newParam.value = (STNBString*)NBMemory_alloc(sizeof(STNBString));
			if(param->value == NULL){
				NBString_init(newParam.value);
			} else {
				NBString_initWithOther(newParam.value, param->value);
			}
			NBArray_addValue(dst, newParam);
		}
	}
}

//Build request content

UI32 NBHttpRequest_concatHeaderStart(const STNBHttpRequest* obj, STNBString* dst, const char* server, const char* resource, const STNBUrl* optUrlParamsAndFragments, const char* httpBoundaryTag){
	const UI32 lenBefore = dst->length;
	const UI32 httpBoundaryTagLen = NBString_strLenBytes(httpBoundaryTag);
	{
		//body length
		UI32 bodyBytesCount = 0;
		if(obj->content.length > 0){
			bodyBytesCount = obj->content.length;
		} else if(obj->paramsPOST.use > 0){
			if(obj->postFormat == ENNBHttpPOSTFormat_urlencoded){
				SI32 i; const SI32 conteo = obj->paramsPOST.use;
				for(i = 0; i < conteo; i++){
					const STNBHttpParam* param = (const STNBHttpParam*)NBArray_itemAtIndex(&obj->paramsPOST, i);
					if(i != 0) bodyBytesCount++; //"&"
					bodyBytesCount += NBUrl_encodedLen(param->name.str);
					bodyBytesCount++; //"="
					bodyBytesCount += NBUrl_encodedLen(param->value->str);
				}
			} else {
				NBASSERT(obj->postFormat == ENNBHttpPOSTFormat_multipart)
				//POST params
				SI32 i; const SI32 conteo = obj->paramsPOST.use;
				for(i = 0; i < conteo; i++){
					const STNBHttpParam* param = (const STNBHttpParam*)NBArray_itemAtIndex(&obj->paramsPOST, i);
					bodyBytesCount += 2 + httpBoundaryTagLen + 2; //--[boundary]\r\n
					bodyBytesCount += NBString_strLenBytes("Content-Disposition: form-data; name=\"");
					bodyBytesCount += param->name.length;
					bodyBytesCount += NBString_strLenBytes("\"\r\n\r\n");
					bodyBytesCount += param->value->length;
					bodyBytesCount += 2; //\r\n
				}
				//Final boundary
				bodyBytesCount += 2 + httpBoundaryTagLen + 2; //--[boundary]--
			}
		}
		//Construir solicitud HTTP
		{
			const char* method = obj->method.str;
			if(NBString_strIsEmpty(method)){
				method = (obj->paramsPOST.use > 0 || obj->content.length > 0 ? "POST" : "GET");
			} NBASSERT(!NBString_strIsEmpty(method))
			NBString_concat(dst, method);
			NBString_concatByte(dst, ' ');
			NBString_concat(dst, resource);
			//Add params
			if(optUrlParamsAndFragments != NULL){
				SI32 i; for(i = 0; i < optUrlParamsAndFragments->params.use; i++){
					const STNBUrlPair* pair = (const STNBUrlPair*)NBArray_itemAtIndex(&optUrlParamsAndFragments->params, i);
					NBString_concatByte(dst, (i == 0 ? ';' : '&'));
					NBUrl_concatEncoded(dst, &optUrlParamsAndFragments->str.str[pair->iName]);
					NBString_concatByte(dst, '=');
					NBUrl_concatEncoded(dst, &optUrlParamsAndFragments->str.str[pair->iValue]);
				}
			}
			//Agregar GET queries
			{
				SI32 iAdded = 0;
				if(optUrlParamsAndFragments != NULL){
					SI32 i; for(i = 0; i < optUrlParamsAndFragments->queries.use; i++){
						const STNBUrlPair* pair = (const STNBUrlPair*)NBArray_itemAtIndex(&optUrlParamsAndFragments->queries, i);
						NBString_concatByte(dst, (iAdded == 0 ? '?' : '&'));
						NBUrl_concatEncoded(dst, &optUrlParamsAndFragments->str.str[pair->iName]);
						NBString_concatByte(dst, '=');
						NBUrl_concatEncoded(dst, &optUrlParamsAndFragments->str.str[pair->iValue]);
						iAdded++;
					}
				}
				{
					SI32 i; const SI32 conteo = obj->paramsGET.use;
					for(i = 0; i < conteo; i++){
						const STNBHttpParam* param = (const STNBHttpParam*)NBArray_itemAtIndex(&obj->paramsGET, i);
						NBString_concatByte(dst, (iAdded == 0 ? '?' : '&'));
						NBUrl_concatEncodedBytes(dst, param->name.str, param->name.length);
						NBString_concatByte(dst, '=');
						NBUrl_concatEncodedBytes(dst, param->value->str, param->value->length);
						iAdded++;
					}
				}
			}
			//Add fragments
			if(optUrlParamsAndFragments != NULL){
				SI32 i; for(i = 0; i < optUrlParamsAndFragments->fragments.use; i++){
					const STNBUrlPair* pair = (const STNBUrlPair*)NBArray_itemAtIndex(&optUrlParamsAndFragments->fragments, i);
					NBString_concatByte(dst, (i == 0 ? '#' : '&'));
					NBUrl_concatEncoded(dst, &optUrlParamsAndFragments->str.str[pair->iName]);
					if(optUrlParamsAndFragments->str.str[pair->iValue] != '\0'){
						NBString_concatByte(dst, '=');
						NBUrl_concatEncoded(dst, &optUrlParamsAndFragments->str.str[pair->iValue]);
					}
				}
			}
			NBString_concatByte(dst, ' ');
			NBString_concatBytes(dst, obj->protocol.str, obj->protocol.length);
			NBString_concatByte(dst, '/');
			NBString_concatUI32(dst, obj->verMajor);
			NBString_concatByte(dst, '.');
			NBString_concatUI32(dst, obj->verMinor);
			//PRINTF_INFO("Host URL: '%s'\n", strREQUEST.str);
			NBString_concat(dst, "\r\n");
			if(obj->addImplicitParams && obj->addParamHost){
				NBString_concat(dst, "Host: ");
				NBString_concat(dst, server);
				//DBString_concat(dst, ":");
				//DBString_concatSI32(dst, data->serverPort);
				NBString_concat(dst, "\r\n");
			}
			//Other headers
			{
				SI32 i; const SI32 conteo = obj->headers.use;
				for(i = 0; i < conteo; i++){
					const STNBHttpParam* param = (const STNBHttpParam*)NBArray_itemAtIndex(&obj->headers, i);
					NBString_concat(dst, param->name.str);
					NBString_concatBytes(dst, ": ", 2);
					NBString_concat(dst, param->value->str);
					NBString_concatBytes(dst, "\r\n", 2);
				}
			}
			if(obj->content.length > 0){
				if(obj->contentType.length == 0){
					NBString_concat(dst, "Content-Type: application/octet-stream\r\n");
				} else {
					NBString_concat(dst, "Content-Type: ");
					NBString_concat(dst, obj->contentType.str);
					NBString_concat(dst, "\r\n");
				}
				NBString_concat(dst, "Content-Length: ");
				NBString_concatSI32(dst, bodyBytesCount);
				NBString_concat(dst, "\r\n");
				NBASSERT(obj->content.length == bodyBytesCount)
			} else if(obj->paramsPOST.use > 0){
				if(obj->postFormat == ENNBHttpPOSTFormat_urlencoded){
					NBString_concat(dst, "Content-Type: application/x-www-form-urlencoded\r\n");
				} else {
					NBASSERT(obj->postFormat == ENNBHttpPOSTFormat_multipart)
					NBString_concat(dst, "Content-Type: multipart/form-data; boundary=");
					NBString_concat(dst, httpBoundaryTag);
					NBString_concat(dst, "\r\n");
				}
				NBString_concat(dst, "Content-Length: ");
				NBString_concatSI32(dst, bodyBytesCount);
				NBString_concat(dst, "\r\n");
			} else if(NBString_strIsEqual(method, "POST")){
				NBASSERT(bodyBytesCount == 0)
				if(obj->contentType.length == 0){
					NBString_concat(dst, "Content-Type: application/octet-stream\r\n");
				} else {
					NBString_concat(dst, "Content-Type: ");
					NBString_concat(dst, obj->contentType.str);
					NBString_concat(dst, "\r\n");
				}
				NBString_concat(dst, "Content-Length: ");
				NBString_concatSI32(dst, bodyBytesCount);
				NBString_concat(dst, "\r\n");
			}
			if(obj->addImplicitParams && obj->addParamUserAgent){
				NBString_concat(dst, "User-Agent: Nibsa/HttpClient\r\n");
			}
			NBString_concat(dst, "\r\n");
			//PRINTF_INFO("Sending http request:\n%s\n", strREQUEST.str);
		}
	}
	return (dst->length - lenBefore);
}

BOOL NBHttpRequest_concatBodyContinue(const STNBHttpRequest* obj, STNBString* dst, const char* httpBoundaryTag, void** dstPtrDataToSend, UI32* dstPtrDataToSendSize, const UI32 iStep){
	BOOL r = FALSE;
	//
	if(dstPtrDataToSend != NULL) *dstPtrDataToSend = NULL;
	if(dstPtrDataToSendSize != NULL) *dstPtrDataToSendSize = 0;
	{
		if(obj->content.length > 0){
			if(iStep == 0){
				if(dstPtrDataToSend != NULL && dstPtrDataToSendSize != NULL){
					//Do not use more memory, return current data pointer
					if(dstPtrDataToSend != NULL) *dstPtrDataToSend = obj->content.str;
					if(dstPtrDataToSendSize != NULL) *dstPtrDataToSendSize	= obj->content.length;
				} else {
					//Copy data (redundancy)
					NBString_concatBytes(dst, obj->content.str, obj->content.length);
					if(dstPtrDataToSend != NULL) *dstPtrDataToSend = dst->str;
					if(dstPtrDataToSendSize != NULL) *dstPtrDataToSendSize = dst->length;
				}
				r = TRUE;
			}
		} else if(obj->paramsPOST.use > 0){
			//POST params
			if(obj->paramsPOST.use > 0){
				if(obj->postFormat == ENNBHttpPOSTFormat_urlencoded){
					if(iStep < obj->paramsPOST.use){
						const STNBHttpParam* param = (const STNBHttpParam*)NBArray_itemAtIndex(&obj->paramsPOST, iStep);
						if(iStep > 0) NBString_concatByte(dst, '&');
						NBUrl_concatEncoded(dst, param->name.str);
						NBString_concatByte(dst, '=');
						NBUrl_concatEncoded(dst, param->value->str);
						r = TRUE;
					}
					if(r){
						if(dstPtrDataToSend != NULL) *dstPtrDataToSend = dst->str;
						if(dstPtrDataToSendSize != NULL) *dstPtrDataToSendSize = dst->length;
					}
				} else {
					const UI32 httpBoundaryTagLen = NBString_strLenBytes(httpBoundaryTag);
					NBASSERT(obj->postFormat == ENNBHttpPOSTFormat_multipart)
					if(iStep < obj->paramsPOST.use){
						const STNBHttpParam* param = (const STNBHttpParam*)NBArray_itemAtIndex(&obj->paramsPOST, iStep);
						NBString_concatBytes(dst, "--", 2);
						NBString_concatBytes(dst, httpBoundaryTag, httpBoundaryTagLen);
						NBString_concatBytes(dst, "\r\n", 2);
						//
						NBString_concat(dst, "Content-Disposition: form-data; name=\"");
						NBString_concatBytes(dst, param->name.str, param->name.length);
						NBString_concatBytes(dst, "\"\r\n\r\n", 5);
						NBString_concatBytes(dst, param->value->str, param->value->length);
						NBString_concatBytes(dst, "\r\n", 2);
						r = TRUE;
					}
					if((iStep == 0 && obj->paramsPOST.use == 0) || (iStep + 1) == obj->paramsPOST.use){
						//Final boundary
						NBString_concatBytes(dst, "--", 2);
						NBString_concatBytes(dst, httpBoundaryTag, httpBoundaryTagLen);
						NBString_concatBytes(dst, "--", 2);
						r = TRUE;
					}
					if(r){
						if(dstPtrDataToSend != NULL) *dstPtrDataToSend = dst->str;
						if(dstPtrDataToSendSize != NULL) *dstPtrDataToSendSize = dst->length;
					}
				}
			}
		}
	}
	return r;
}
