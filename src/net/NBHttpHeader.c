
#include "nb/NBFrameworkPch.h"
#include "nb/net/NBHttpHeader.h"
//
#include "nb/core/NBStruct.h"
#include "nb/net/NBHttpParser.h"


void NBHttpHeader_init(STNBHttpHeader* obj){
	NBMemory_setZeroSt(*obj, STNBHttpHeader);
	obj->isCompleted		= FALSE;
	obj->httpParser			= NULL;
	obj->requestLine		= NULL;		//If message is a request
	obj->statusLine			= NULL;		//If message is a status (response)
	NBArray_init(&obj->fields, sizeof(STNBHttpHeaderField), NULL);
	NBString_init(&obj->strs);
	NBString_concatByte(&obj->strs, '\0'); //Index 0 must be empty string
}

void NBHttpHeader_release(STNBHttpHeader* obj){
	NBHttpHeader_empty(obj);
	if(obj->httpParser != NULL){
		NBHttpParser_release(obj->httpParser);
		NBMemory_free(obj->httpParser);
		obj->httpParser = NULL;
	}
	NBArray_release(&obj->fields);
	NBString_release(&obj->strs);
}

//Listener

void NBHttpHeader_setListener(STNBHttpHeader* obj, const IHttpHeaderListener* listener, void* listenerParam){
	if(listener == NULL){
		NBMemory_setZeroSt(obj->listener, IHttpHeaderListener);
	} else {
		obj->listener = *listener;
	}
	obj->listenerParam = listenerParam;
}

//First line

BOOL NBHttpHeader_setRequestLine(STNBHttpHeader* obj, const char* method, const char* target, const UI8 majorVer, const UI8 minorVer){
	if(obj->requestLine == NULL){
		obj->requestLine = NBMemory_allocType(STNBHttpHeaderRequestLine);
	}
	//Empty
	NBMemory_setZeroSt(*obj->requestLine, STNBHttpHeaderRequestLine);
	//Method
	{
		obj->requestLine->method = obj->strs.length;
		NBString_concat(&obj->strs, method);
		NBString_concatByte(&obj->strs, '\0');
	}
	//Target
	{
		obj->requestLine->target = obj->strs.length;
		NBString_concat(&obj->strs, target);
		NBString_concatByte(&obj->strs, '\0');
	}
	//Http Version
	obj->requestLine->majorVer = majorVer;
	obj->requestLine->minorVer = minorVer;
	return TRUE;
}

BOOL NBHttpHeader_setStatusLine(STNBHttpHeader* obj, const UI8 majorVer, const UI8 minorVer, const UI32 statusCode, const char* reasonPhrase){
	if(obj->statusLine == NULL){
		obj->statusLine = NBMemory_allocType(STNBHttpHeaderStatusLine);
	}
	//Empty
	NBMemory_setZeroSt(*obj->statusLine, STNBHttpHeaderStatusLine);
	//Http Version
	obj->statusLine->majorVer	= majorVer;
	obj->statusLine->minorVer	= minorVer;
	//Status Code
	obj->statusLine->statusCode		= statusCode;
	//Reason phrase
	{
		obj->statusLine->reasonPhrase = obj->strs.length;
		NBString_concat(&obj->strs, reasonPhrase);
		NBString_concatByte(&obj->strs, '\0');
	}
	return TRUE;
}

BOOL NBHttpHeader_parseRequestTarget(const STNBHttpHeader* obj, STNBString* dstAbsPath, STNBString* dstQuery, STNBString* dstFragment){
    BOOL r = FALSE;
    if(obj->requestLine != NULL){
        r = NBHttpHeader_strParseRequestTarget(&obj->strs.str[obj->requestLine->target], dstAbsPath, dstQuery, dstFragment);
    }
    return r;
}

BOOL NBHttpHeader_strParseRequestTarget(const char* target, STNBString* dstAbsPath, STNBString* dstQuery, STNBString* dstFragment){
    BOOL r = FALSE;
    if(target != NULL){
        r = TRUE;
        //reset strings
        if(dstAbsPath != NULL) NBString_empty(dstAbsPath);
        if(dstQuery != NULL) NBString_empty(dstQuery);
        if(dstFragment != NULL) NBString_empty(dstFragment);
        //
        {
            const char* tPos = target;
            STNBUriParser uriParser;
            NBUriParser_init(&uriParser);
            //Parse uri parts as: path-absolute [ "?" query ] [ "#" fragment ]
            if(tPos != NULL){
                const char* tStart = tPos;
                NBUriParser_feedStartWithType(&uriParser, ENNBUriParserType_pathAbsolute);
                while(*tPos != '\0' && NBUriParser_feedChar(&uriParser, *tPos)){
                    tPos++;
                }
                if(tStart < tPos){
                    //set absPath
                    if(dstAbsPath != NULL) NBString_setBytes(dstAbsPath, tStart, (UI32)(tPos - tStart));
                    //parse query
                    if(*tPos == '?'){
                        const char* tStart = ++tPos; //ignore the '?'
                        NBUriParser_feedStartWithType(&uriParser, ENNBUriParserType_query);
                        while(*tPos != '\0' && NBUriParser_feedChar(&uriParser, *tPos)){
                            tPos++;
                        }
                        if(tStart < tPos){
                            if(dstQuery != NULL) NBString_setBytes(dstQuery, tStart, (UI32)(tPos - tStart));
                        }
                    }
                    //parse fragment
                    if(*tPos == '#'){
                        const char* tStart = ++tPos; //ignore the '#'
                        NBUriParser_feedStartWithType(&uriParser, ENNBUriParserType_fragment);
                        while(*tPos != '\0' && NBUriParser_feedChar(&uriParser, *tPos)){
                            tPos++;
                        }
                        if(tStart < tPos){
                            if(dstFragment != NULL) NBString_setBytes(dstFragment, tStart, (UI32)(tPos - tStart));
                        }
                    }
                    //target is not a valid uri (path-absolute [ "?" query ] [ "#" fragment ])
                    if(*tPos != '\0'){ //string not fully-consumed
                        if(dstAbsPath != NULL) NBString_empty(dstAbsPath);
                        if(dstQuery != NULL) NBString_empty(dstQuery);
                        if(dstFragment != NULL) NBString_empty(dstFragment);
                        r = FALSE;
                    }
                }
            }
            NBUriParser_release(&uriParser);
        }
    }
    return r;
}

//

void NBHttpHeader_empty(STNBHttpHeader* obj){
	obj->isCompleted	= FALSE;
	if(obj->requestLine != NULL){
		NBMemory_free(obj->requestLine);
		obj->requestLine = NULL;
	}
	if(obj->statusLine != NULL){
		NBMemory_free(obj->statusLine);
		obj->statusLine = NULL;
	}
	NBArray_empty(&obj->fields);
	NBString_empty(&obj->strs);
	NBString_concatByte(&obj->strs, '\0'); //Index 0 must be empty string
}

void NBHttpHeader_addField(STNBHttpHeader* obj, const char* fieldName, const char* fieldValue){
	STNBHttpHeaderField f;
	//
	f.name = obj->strs.length;
	NBString_concat(&obj->strs, fieldName);
	NBString_concatByte(&obj->strs, '\0');
	//
	f.value = obj->strs.length;
	NBString_concat(&obj->strs, fieldValue);
	NBString_concatByte(&obj->strs, '\0');
	//
	NBArray_addValue(&obj->fields, f);
}

void NBHttpHeader_addFieldContentLength(STNBHttpHeader* obj, const UI32 contentLength){
    STNBString lenStr;
    NBString_init(&lenStr);
    NBString_concatUI32(&lenStr, contentLength);
    {
        NBHttpHeader_addField(obj, "Content-Length", lenStr.str);
    }
    NBString_release(&lenStr);
}

void NBHttpHeader_removeField(STNBHttpHeader* obj, const char* fieldName){
	SI32 i;
	for(i = (obj->fields.use - 1); i >= 0; i--){
		STNBHttpHeaderField* f = NBArray_itmPtrAtIndex(&obj->fields, STNBHttpHeaderField, i);
		if(NBString_strIsLike(&obj->strs.str[f->name], fieldName)){
			NBArray_removeItemAtIndex(&obj->fields, i);
		}
	}
}

BOOL NBHttpHeader_hasFieldValue(const STNBHttpHeader* obj, const char* fieldName, const char* fieldValue, const BOOL allowComments){
	BOOL r = FALSE;
	STNBHttpHeaderFieldCursor cursor;
	NBHttpHeaderFieldCursor_init(&cursor);
	while(NBHttpHeader_getNextFieldValue(obj, fieldName, allowComments, &cursor)){
		if(cursor.value != NULL && cursor.valueLen > 0){
			if(NBString_strIsLikeStrBytes(fieldValue, cursor.value, cursor.valueLen)){
				r = TRUE;
				break;
			}
		}
	}
	NBHttpHeaderFieldCursor_release(&cursor);
	return r;
}

const char* NBHttpHeader_getField(const STNBHttpHeader* obj, const char* fieldName){
	const char* r = NULL;
	UI32 i; for(i = 0; i < obj->fields.use; i++){
		STNBHttpHeaderField* f = NBArray_itmPtrAtIndex(&obj->fields, STNBHttpHeaderField, i);
		if(NBString_strIsLike(&obj->strs.str[f->name], fieldName)){
			r = &obj->strs.str[f->value];
			break;
		}
	}
	return r;
}

const char* NBHttpHeader_getFieldOrDefault(const STNBHttpHeader* obj, const char* fieldName, const char* defValue, const BOOL allowEmptyValue){
	const char* r = NBHttpHeader_getField(obj, fieldName);
	if(r == NULL){
		r = defValue;
	} else if(r[0] == '\0' && !allowEmptyValue){
		r = defValue;
	}
	return r;
}

BOOL NBHttpHeader_getFieldAsStruct(const STNBHttpHeader* obj, const char* fieldName, const STNBStructMap* stMap, void* dst, const UI32 dstSz){
	BOOL r = FALSE;
	UI32 i; for(i = 0; i < obj->fields.use; i++){
		STNBHttpHeaderField* f = NBArray_itmPtrAtIndex(&obj->fields, STNBHttpHeaderField, i);
		if(NBString_strIsLike(&obj->strs.str[f->name], fieldName)){
			const char* v = &obj->strs.str[f->value];
			if(v[0] != '\0'){
				r = NBStruct_stReadFromJsonBase64Str(v, NBString_strLenBytes(v), stMap, dst, dstSz);
			}
			break;
		}
	}
	return r;
}

//Cursor for reading header values:
//Multiple http headers can be combined in one separated by comma;
//quoted-strings "..." and comments (...) can be included.

BOOL NBHttpHeader_getNextFieldValue(const STNBHttpHeader* obj, const char* fieldName, const BOOL allowComments, STNBHttpHeaderFieldCursor* cursor){
	BOOL r = FALSE;
	if(cursor != NULL){
		SI32 i;
		//Init results
		cursor->value		= NULL;
		cursor->valueLen	= 0;
		//Search
		for(i = cursor->state.iField; i < obj->fields.use; i++){
			STNBHttpHeaderField* f = NBArray_itmPtrAtIndex(&obj->fields, STNBHttpHeaderField, i);
			if(NBString_strIsLike(&obj->strs.str[f->name], fieldName)){
				const char* fValue	= &obj->strs.str[f->value];
				if(!NBHttpHeader_getNextFieldValueStr(fValue, allowComments, cursor)){
					NBASSERT(cursor->value == NULL)
					cursor->state.iByte = 0;
				} else {
					NBASSERT(cursor->value != NULL)
					r = TRUE;
					break;
				}
			}
		}
		//Keep state
		cursor->state.iField = i;
	}
	return r;
}

BOOL NBHttpHeader_getNextFieldValueStr(const char* fValue, const BOOL allowComments, STNBHttpHeaderFieldCursor* cursor){
	BOOL r = FALSE;
	if(cursor != NULL){
		char cPrev = '\0', grpOpener = '\0'; // '\0' for none, '(' for comment, '"' for quoted-string.
		const char* c		= fValue;
		const char* cFirst	= &c[cursor->state.iByte];
		//Move to start
		while(*c != '\0' && c < cFirst){
			c++;
		}
		//Read content
		if(*c != '\0'){
			UI32 skipAmount = 0;
			cursor->value = c;
			while(*c != '\0'){
				if(grpOpener == '('){
					if(cPrev != '\\' && *c == ')'){
						grpOpener = '\0';
					}
				} else if(grpOpener == '\"'){
					if(cPrev != '\\' && *c == '\"'){
						grpOpener = '\0';
					}
				} else if(*c == '\"'){
					grpOpener = '\"';
				} else if(*c == '(' && allowComments){
					grpOpener = '(';
				} else if(*c == ','){
					c++; skipAmount++; //skip the comma for next step
					break;
				}
				cPrev = *c;
				c++;
			}
			//Trim white-spaces
			{
				const char* cAfterEnd = c - skipAmount;
				//left-white-spaces
				while(cursor->value < cAfterEnd && (*cursor->value == ' ' || *cursor->value == '\t')){
					cursor->value++;
				}
				//right-white-spaces
				while(cursor->value < cAfterEnd && (*cAfterEnd == ' ' || *cAfterEnd == '\t')){
					cAfterEnd--;
				}
				//Calculate length
				cursor->valueLen = (UI32)(cAfterEnd - cursor->value);
			}
			r = TRUE;
		}
		//Keep state
		cursor->state.iByte = (UI32)(c - fValue);
	}
	return r;
}

//

BOOL NBHttpHeader_isConnectionClose(const STNBHttpHeader* obj){
	BOOL r = FALSE;
	UI32 i; const UI32 count = obj->fields.use;
	for(i = 0; i < count; i++){
		STNBHttpHeaderField* f = NBArray_itmPtrAtIndex(&obj->fields, STNBHttpHeaderField, i);
		if(NBString_strIsLike(&obj->strs.str[f->name], "Connection")){
			if(NBString_strIndexOf(&obj->strs.str[f->value], "close", 0) != -1){
				r = TRUE;
			}
			break;
		}
	}
	return r;
}

BOOL NBHttpHeader_isContentLength(const STNBHttpHeader* obj){
	BOOL r = FALSE;
	const char* fv = NBHttpHeader_getField(obj, "Content-Length");
	if(fv != NULL){
		if(fv[0] != '\0'){
			r = TRUE;
		}
	}
	return r;
}

UI64 NBHttpHeader_contentLength(const STNBHttpHeader* obj){
	UI64 r = 0;
	const char* fv = NBHttpHeader_getField(obj, "Content-Length");
	NBASSERT(fv != NULL)
	if(fv != NULL){
		NBASSERT(fv[0] != '\0')
		while(*fv != '\0'){
			NBASSERT(*fv >= '0' && *fv <= '9')
			r = (r * 10) + (*fv - '0');
			fv++;
		}
	}
	return r;
}

//Concat as string

void NBHttpHeader_concatHeader(const STNBHttpHeader* obj, STNBString* dst){
	//First line
	{
		if(obj->requestLine != NULL){
			const STNBHttpHeaderRequestLine* req = obj->requestLine;
			if(dst->length > 0) NBString_concat(dst, "\r\n");
			NBString_concat(dst, &obj->strs.str[req->method]);
			NBString_concatByte(dst, ' ');
			NBString_concat(dst, &obj->strs.str[req->target]);
			NBString_concatByte(dst, ' ');
			NBString_concat(dst, "HTTP/");
			NBString_concatUI32(dst, req->majorVer);
			NBString_concatByte(dst, '.');
			NBString_concatUI32(dst, req->minorVer);
		}
		if(obj->statusLine != NULL){
			const STNBHttpHeaderStatusLine* status = obj->statusLine;
			if(dst->length > 0) NBString_concat(dst, "\r\n");
			NBString_concat(dst, "HTTP/");
			NBString_concatUI32(dst, status->majorVer);
			NBString_concatByte(dst, '.');
			NBString_concatUI32(dst, status->minorVer);
			NBString_concatByte(dst, ' ');
			NBString_concatUI32(dst, status->statusCode);
			NBString_concatByte(dst, ' ');
			NBString_concat(dst, &obj->strs.str[status->reasonPhrase]);
		}
	}
	//Other lines
	{
		SI32 i; for(i = 0; i < obj->fields.use; i++){
			const STNBHttpHeaderField* fld = NBArray_itmPtrAtIndex(&obj->fields, STNBHttpHeaderField, i);
			if(dst->length > 0) NBString_concat(dst, "\r\n");
			NBString_concat(dst, &obj->strs.str[fld->name]);
			NBString_concatByte(dst, ':'); NBString_concatByte(dst, ' ');
			NBString_concat(dst, &obj->strs.str[fld->value]);
		}
	}
	//Empty line
	NBString_concat(dst, "\r\n");
}

//

BOOL NBHttpHeader_isBodyChunked(const STNBHttpHeader* obj){
	BOOL r = FALSE;
	const char* fv = NBHttpHeader_getField(obj, "Transfer-Encoding");
	if(fv != NULL){
		if(fv[0] != '\0'){
			STNBHttpTransferEncoding tEnc;
			NBHttpTransferEncoding_init(&tEnc);
			if(!NBHttpTransferEncoding_addFromLine(&tEnc, fv)){
				NBASSERT(FALSE)
			} else {
				r = NBHttpTransferEncoding_isChunked(&tEnc);
			}
			NBHttpTransferEncoding_release(&tEnc);
		}
	}
	return r;
}

const char* NBHttpHeader_getRequestMethod(const STNBHttpHeader* obj){
	const char* r = NULL;
	if(obj->requestLine != NULL){
		r = &obj->strs.str[obj->requestLine->method];
	}
	return r;
}

const char* NBHttpHeader_getRequestTarget(const STNBHttpHeader* obj){
    const char* r = NULL;
    if(obj->requestLine != NULL){
        r = &obj->strs.str[obj->requestLine->target];
    }
    return r;
}

//Parser interface
BOOL NBHttpHeader_consumeHttpVer_(const STNBHttpParser* obj, const STNBHttpParserNode* node, const char* httpVer, const UI32 httpVerSz, const UI32 majorVer, const UI32 minorVer, void* listenerParam){
	BOOL r = FALSE;
	if(listenerParam != NULL){
		STNBHttpHeader* obj = (STNBHttpHeader*)listenerParam;
		NBASSERT(!obj->isCompleted)
		if(!obj->isCompleted){
			if(obj->requestLine != NULL){
				//Request line has the http ver at the end
				obj->requestLine->majorVer	= (UI8)majorVer;
				obj->requestLine->minorVer	= (UI8)minorVer;
				//PRINTF_INFO("NBHttpHeader_consumeHttpVer_: %d.%d (request line)\n", majorVer, minorVer);
			} else {
				//Status line has the http ver at the start
				NBASSERT(obj->requestLine == NULL)
				NBASSERT(obj->statusLine == NULL)
				if(obj->statusLine == NULL){
					obj->statusLine = NBMemory_allocType(STNBHttpHeaderStatusLine);
					NBMemory_setZeroSt(*obj->statusLine, STNBHttpHeaderStatusLine);;
				}
				obj->statusLine->majorVer	= (UI8)majorVer;
				obj->statusLine->minorVer	= (UI8)minorVer;
				//PRINTF_INFO("NBHttpHeader_consumeHttpVer_: %d.%d (status line)\n", majorVer, minorVer);
			}
			r = TRUE;
		}
	}
	return r;
}
BOOL NBHttpHeader_consumeMethod_(const struct STNBHttpParser_* obj, const STNBHttpParserNode* node, const char* method, const UI32 methodSz, void* listenerParam){
	BOOL r = FALSE;
	if(listenerParam != NULL){
		STNBHttpHeader* obj = (STNBHttpHeader*)listenerParam;
		NBASSERT(!obj->isCompleted)
		if(!obj->isCompleted){
			NBASSERT(obj->statusLine == NULL)
			if(obj->requestLine == NULL){
				obj->requestLine = NBMemory_allocType(STNBHttpHeaderRequestLine);
				NBMemory_setZeroSt(*obj->requestLine, STNBHttpHeaderRequestLine);;
			}
			if(obj->requestLine->method == 0){
				obj->requestLine->method = obj->strs.length;
				NBString_concatBytes(&obj->strs, method, methodSz);
				NBString_concatByte(&obj->strs, '\0');
			} else {
				NBASSERT(NBString_strIsEqualStrBytes(&obj->strs.str[obj->requestLine->method], method, methodSz))
			}
			//PRINTF_INFO("NBHttpHeader_consumeMethod_: '%s'\n", &obj->strs.str[obj->requestLine->method]);
			r = TRUE;
		}
	}
	return r;
}
BOOL NBHttpHeader_consumeRequestTarget_(const STNBHttpParser* obj, const STNBHttpParserNode* node, const char* target, const UI32 targetSz, void* listenerParam){
	BOOL r = FALSE;
	if(listenerParam != NULL){
		STNBHttpHeader* obj = (STNBHttpHeader*)listenerParam;
		NBASSERT(!obj->isCompleted)
		if(!obj->isCompleted){
			NBASSERT(obj->statusLine == NULL)
			if(obj->requestLine == NULL){
				obj->requestLine = NBMemory_allocType(STNBHttpHeaderRequestLine);
				NBMemory_setZeroSt(*obj->requestLine, STNBHttpHeaderRequestLine);;
			}
			if(obj->requestLine->target == 0){
				obj->requestLine->target = obj->strs.length;
				NBString_concatBytes(&obj->strs, target, targetSz);
				NBString_concatByte(&obj->strs, '\0');
			} else {
				NBASSERT(NBString_strIsEqualStrBytes(&obj->strs.str[obj->requestLine->target], target, targetSz))
			}
			//PRINTF_INFO("NBHttpHeader_consumeRequestTarget_: '%s'\n", &obj->strs.str[obj->requestLine->target]);
			r = TRUE;
		}
	}
	return r;
}
BOOL NBHttpHeader_consumeRequestLine_(const STNBHttpParser* obj, const STNBHttpParserNode* node, const char* method, const UI32 methodSz, const char* target, const UI32 targetSz, const char* httpVer, const UI32 httpVerSz, const UI32 majorVer, const UI32 minorVer, void* listenerParam){
	BOOL r = FALSE;
	if(listenerParam != NULL){
		STNBHttpHeader* obj = (STNBHttpHeader*)listenerParam;
		NBASSERT(!obj->isCompleted)
		if(!obj->isCompleted){
			NBASSERT(obj->statusLine == NULL)
			if(obj->requestLine == NULL){
				obj->requestLine = NBMemory_allocType(STNBHttpHeaderRequestLine);
				NBMemory_setZeroSt(*obj->requestLine, STNBHttpHeaderRequestLine);;
			}
			//
			if(obj->requestLine->method == 0){
				obj->requestLine->method = obj->strs.length;
				NBString_concatBytes(&obj->strs, method, methodSz);
				NBString_concatByte(&obj->strs, '\0');
			} else {
				NBASSERT(NBString_strIsEqualStrBytes(&obj->strs.str[obj->requestLine->method], method, methodSz))
			}
			if(obj->requestLine->target == 0){
				obj->requestLine->target = obj->strs.length;
				NBString_concatBytes(&obj->strs, target, targetSz);
				NBString_concatByte(&obj->strs, '\0');
			} else {
				NBASSERT(NBString_strIsEqualStrBytes(&obj->strs.str[obj->requestLine->target], target, targetSz))
			}
			obj->requestLine->majorVer = (UI8)majorVer;
			obj->requestLine->minorVer = (UI8)minorVer;
			//PRINTF_INFO("NBHttpHeader_consumeRequestTarget_: '%s' '%s' %d.%d\n", &obj->strs.str[obj->requestLine->method], &obj->strs.str[obj->requestLine->target], majorVer, minorVer);
			r = TRUE;
			//Notify listener
			if(obj->listener.consumeStartLine != NULL){
				r = (*obj->listener.consumeStartLine)(obj, obj->listenerParam);
			}
		}
	}
	return r;
}
//Header status-start-line
BOOL NBHttpHeader_consumeStatusCode_(const STNBHttpParser* obj, const STNBHttpParserNode* node, const UI32 statusCode, void* listenerParam){
	BOOL r = FALSE;
	if(listenerParam != NULL){
		STNBHttpHeader* obj = (STNBHttpHeader*)listenerParam;
		NBASSERT(!obj->isCompleted)
		if(!obj->isCompleted){
			NBASSERT(obj->requestLine == NULL)
			if(obj->statusLine == NULL){
				obj->statusLine = NBMemory_allocType(STNBHttpHeaderStatusLine);
				NBMemory_setZeroSt(*obj->statusLine, STNBHttpHeaderStatusLine);;
			}
			obj->statusLine->statusCode = statusCode;
			//PRINTF_INFO("NBHttpHeader_consumeStatusCode_: '%d'\n", statusCode);
			r = TRUE;
		}
	}
	return r;
}
BOOL NBHttpHeader_consumeReasonPhrase_(const STNBHttpParser* obj, const STNBHttpParserNode* node, const char* reasonPhrase, const UI32 reasonPhraseSz, void* listenerParam){
	BOOL r = FALSE;
	if(listenerParam != NULL){
		STNBHttpHeader* obj = (STNBHttpHeader*)listenerParam;
		NBASSERT(!obj->isCompleted)
		if(!obj->isCompleted){
			NBASSERT(obj->requestLine == NULL)
			if(obj->statusLine == NULL){
				obj->statusLine = NBMemory_allocType(STNBHttpHeaderStatusLine);
				NBMemory_setZeroSt(*obj->statusLine, STNBHttpHeaderStatusLine);;
			}
			if(obj->statusLine->reasonPhrase == 0){
				obj->statusLine->reasonPhrase = obj->strs.length;
				NBString_concatBytes(&obj->strs, reasonPhrase, reasonPhraseSz);
				NBString_concatByte(&obj->strs, '\0');
			} else {
				NBASSERT(NBString_strIsEqualStrBytes(&obj->strs.str[obj->statusLine->reasonPhrase], reasonPhrase, reasonPhraseSz))
			}
			//PRINTF_INFO("NBHttpHeader_consumeReasonPhrase_: '%s'\n", &obj->strs.str[obj->statusLine->reasonPhrase]);
			r = TRUE;
		}
	}
	return r;
}
BOOL NBHttpHeader_consumeStatusLine_(const STNBHttpParser* obj, const STNBHttpParserNode* node, const char* httpVer, const UI32 httpVerSz, const UI32 majorVer, const UI32 minorVer, const UI32 statusCode, const char* reasonPhrase, const UI32 reasonPhraseSz, void* listenerParam){
	BOOL r = FALSE;
	if(listenerParam != NULL){
		STNBHttpHeader* obj = (STNBHttpHeader*)listenerParam;
		NBASSERT(!obj->isCompleted)
		if(!obj->isCompleted){
			NBASSERT(obj->requestLine == NULL)
			if(obj->statusLine == NULL){
				obj->statusLine = NBMemory_allocType(STNBHttpHeaderStatusLine);
				NBMemory_setZeroSt(*obj->statusLine, STNBHttpHeaderStatusLine);;
			}
			obj->statusLine->majorVer	= (UI8)majorVer;
			obj->statusLine->minorVer	= (UI8)minorVer;
			obj->statusLine->statusCode	= statusCode;
			if(obj->statusLine->reasonPhrase == 0){
				obj->statusLine->reasonPhrase = obj->strs.length;
				NBString_concatBytes(&obj->strs, reasonPhrase, reasonPhraseSz);
				NBString_concatByte(&obj->strs, '\0');
			} else {
				NBASSERT(NBString_strIsEqualStrBytes(&obj->strs.str[obj->statusLine->reasonPhrase], reasonPhrase, reasonPhraseSz))
			}
			//PRINTF_INFO("NBHttpHeader_consumeStatusLine_: %d.%d %d '%s'\n", majorVer, minorVer, statusCode, &obj->strs.str[obj->statusLine->reasonPhrase]);
			r = TRUE;
			//Notify listener
			if(obj->listener.consumeStartLine != NULL){
				r = (*obj->listener.consumeStartLine)(obj, obj->listenerParam);
			}
		}
	}
	return r;
}
//Header generic-fields
BOOL NBHttpHeader_consumeFieldName_(const STNBHttpParser* obj, const STNBHttpParserNode* node, const char* name, const UI32 nameSz, void* listenerParam){
	BOOL r = FALSE;
	if(listenerParam != NULL){
		STNBHttpHeader* obj = (STNBHttpHeader*)listenerParam;
		NBASSERT(!obj->isCompleted)
		if(!obj->isCompleted){
			STNBHttpHeaderField f;
			f.name	= obj->strs.length;
			NBString_concatBytes(&obj->strs, name, nameSz);
			NBString_concatByte(&obj->strs, '\0');
			f.value	= 0;
			NBArray_addValue(&obj->fields, f);
			//PRINTF_INFO("NBHttpHeader_consumeFieldName_: '%s'\n", &obj->strs.str[f.name]);
			r = TRUE;
		}
	}
	return r;
}
BOOL NBHttpHeader_consumeFieldLine_(const STNBHttpParser* obj, const STNBHttpParserNode* node, const char* name, const UI32 nameSz, const char* value, const UI32 valueSz, void* listenerParam){
	BOOL r = FALSE;
	if(listenerParam != NULL){
		STNBHttpHeader* obj = (STNBHttpHeader*)listenerParam;
		NBASSERT(!obj->isCompleted)
		if(!obj->isCompleted){
			//Complete prev added field
			if(obj->fields.use > 0){
				STNBHttpHeaderField* last = NBArray_itmPtrAtIndex(&obj->fields, STNBHttpHeaderField, obj->fields.use - 1);
				if(last->value == 0){
					if(NBString_strIsEqualStrBytes(&obj->strs.str[last->name], name, nameSz)){
						last->value = obj->strs.length;
						NBString_concatBytes(&obj->strs, value, valueSz);
						NBString_concatByte(&obj->strs, '\0');
						//PRINTF_INFO("NBHttpHeader_consumeFieldLine_: '%s': '%s' (pre-added)\n", &obj->strs.str[last->name], &obj->strs.str[last->value]);
						r = TRUE;
					}
				}
			}
			//Add new field
			if(!r){
				STNBHttpHeaderField f;
				f.name	= obj->strs.length;
				NBString_concatBytes(&obj->strs, name, nameSz);
				NBString_concatByte(&obj->strs, '\0');
				f.value	= obj->strs.length;
				NBString_concatBytes(&obj->strs, value, valueSz);
				NBString_concatByte(&obj->strs, '\0');
				NBArray_addValue(&obj->fields, f);
				//PRINTF_INFO("NBHttpHeader_consumeFieldLine_: '%s': '%s' (new)\n", &obj->strs.str[f.name], &obj->strs.str[f.value]);
				r = TRUE;
			}
			//Notify listener
			if(obj->listener.consumeFieldLine != NULL){
				STNBHttpHeaderField* last = NBArray_itmPtrAtIndex(&obj->fields, STNBHttpHeaderField, obj->fields.use - 1);
				r = (*obj->listener.consumeFieldLine)(obj, last, obj->listenerParam);
			}
		}
	}
	return r;
}
BOOL NBHttpHeader_consumeHeaderEnd_(const STNBHttpParser* obj, void* listenerParam){
	BOOL r = FALSE;
	if(listenerParam != NULL){
		STNBHttpHeader* obj = (STNBHttpHeader*)listenerParam;
		NBASSERT(!obj->isCompleted)
		if(!obj->isCompleted){
			obj->isCompleted = TRUE;
			r = TRUE;
			//Notify listener
			if(obj->listener.consumeEnd != NULL){
				r = (*obj->listener.consumeEnd)(obj, obj->listenerParam);
			}
		}
		//PRINTF_INFO("NBHttpHeader_consumeHeaderEnd_.\n");
	}
	return r;
}

//Feed

void NBHttpHeader_feedStart(STNBHttpHeader* obj){
	//Empty
	NBHttpHeader_empty(obj);
	//Start parser
	{
		if(obj->httpParser != NULL){
			NBHttpParser_release(obj->httpParser);
		} else {
			obj->httpParser = NBMemory_allocType(STNBHttpParser);
		}
		IHttpParserListener listnr;
		NBMemory_setZeroSt(listnr, IHttpParserListener);
		listnr.consumeHttpVer		= NBHttpHeader_consumeHttpVer_;
		listnr.consumeMethod		= NBHttpHeader_consumeMethod_;
		listnr.consumeRequestTarget	= NBHttpHeader_consumeRequestTarget_;
		listnr.consumeRequestLine	= NBHttpHeader_consumeRequestLine_;
		//Header status-start-line
		listnr.consumeStatusCode	= NBHttpHeader_consumeStatusCode_;
		listnr.consumeReasonPhrase	= NBHttpHeader_consumeReasonPhrase_;
		listnr.consumeStatusLine	= NBHttpHeader_consumeStatusLine_;
		//Header generic-fields
		listnr.consumeFieldName		= NBHttpHeader_consumeFieldName_;
		listnr.consumeFieldLine		= NBHttpHeader_consumeFieldLine_;
		//End of header
		listnr.consumeHeaderEnd		= NBHttpHeader_consumeHeaderEnd_;
		//
		NBHttpParser_initWithListener(obj->httpParser, &listnr, obj);
	}
	NBASSERT(obj->httpParser != NULL)
	NBHttpParser_feedStartWithType(obj->httpParser, ENNBHttpParserType_httpMessageHeader);
}

BOOL NBHttpHeader_feedByte(STNBHttpHeader* obj, const char c){
	BOOL r = FALSE;
	if(obj->httpParser != NULL){
		r = NBHttpParser_feedByte(obj->httpParser, c);
	}
	return r;
}

UI32 NBHttpHeader_feed(STNBHttpHeader* obj, const char* str){
	UI32 r = 0;
	if(obj->httpParser != NULL){
		r = NBHttpParser_feed(obj->httpParser, str);
	}
	return r;
}

UI32 NBHttpHeader_feedBytes(STNBHttpHeader* obj, const void* data, const UI32 dataSz){
	UI32 r = 0;
	if(obj->httpParser != NULL){
		r = NBHttpParser_feedBytes(obj->httpParser, data, dataSz);
	}
	return r;
}

BOOL NBHttpHeader_feedIsComplete(STNBHttpHeader* obj){
	BOOL r = obj->isCompleted;
	if(!r && obj->httpParser != NULL){
		r = NBHttpParser_feedIsComplete(obj->httpParser);
	}
	return r;
}

BOOL NBHttpHeader_feedEnd(STNBHttpHeader* obj){
	BOOL r = FALSE;
	if(obj->httpParser != NULL){
		r = NBHttpParser_feedEnd(obj->httpParser);
		//Release memory
		NBHttpParser_release(obj->httpParser);
		NBMemory_free(obj->httpParser);
		obj->httpParser = NULL;
	} else {
		r = obj->isCompleted;
	}
	return r;
}
