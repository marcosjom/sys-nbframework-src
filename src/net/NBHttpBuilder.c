
#include "nb/NBFrameworkPch.h"
#include "nb/core/NBEncoding.h"
#include "nb/core/NBStruct.h"
#include "nb/core/NBNumParser.h"
#include "nb/net/NBHttpBuilder.h"
#include "nb/net/NBHttpParser.h"
#include "nb/net/NBUriParser.h"

void NBHttpBuilder_init(STNBHttpBuilder* obj){
	obj->bytesTotal	= 0;
}

void NBHttpBuilder_release(STNBHttpBuilder* obj){
	obj->bytesTotal	= 0;
}

//

void NBHttpBuilder_empty(STNBHttpBuilder* obj){
	obj->bytesTotal	= 0;
}

BOOL NBHttpBuilder_addHeader(STNBHttpBuilder* obj, STNBString* dst, const STNBHttpHeader* header){
	BOOL r = FALSE;
	//First line
	if(obj->bytesTotal == 0){
		if(header->requestLine != NULL){
			const char* method	= &header->strs.str[header->requestLine->method];
			const char* target	= &header->strs.str[header->requestLine->target];
			r = NBHttpBuilder_addRequestLine(obj, dst, method, target, header->requestLine->majorVer, header->requestLine->minorVer);
		} else if(header->statusLine != NULL){
			const char* reasonPhrase = &header->strs.str[header->statusLine->reasonPhrase];
			r = NBHttpBuilder_addStatusLine(obj, dst, header->statusLine->majorVer, header->statusLine->minorVer, header->statusLine->statusCode, reasonPhrase);
		}
		//Other fields
		if(r){
			r = NBHttpBuilder_addHeaderFields(obj, dst, header);
		}
	}
	return r;
}

BOOL NBHttpBuilder_addHeaderAndEnd(STNBHttpBuilder* obj, STNBString* dst, const STNBHttpHeader* header){
	BOOL r = FALSE;
	//First line
	if(obj->bytesTotal == 0){
		if(header->requestLine != NULL){
			const char* method	= &header->strs.str[header->requestLine->method];
			const char* target	= &header->strs.str[header->requestLine->target];
			r = NBHttpBuilder_addRequestLine(obj, dst, method, target, header->requestLine->majorVer, header->requestLine->minorVer);
		} else if(header->statusLine != NULL){
			const char* reasonPhrase = &header->strs.str[header->statusLine->reasonPhrase];
			r = NBHttpBuilder_addStatusLine(obj, dst, header->statusLine->majorVer, header->statusLine->minorVer, header->statusLine->statusCode, reasonPhrase);
		}
		//Other fields
		if(r){
			r = NBHttpBuilder_addHeaderFields(obj, dst, header);
		}
		//End
		if(r){
			r = NBHttpBuilder_addHeaderEnd(obj, dst);
		}
	}
	return r;
}

BOOL NBHttpBuilder_addHeaderFields(STNBHttpBuilder* obj, STNBString* dst, const STNBHttpHeader* header){
	BOOL r = TRUE;
	{
		UI32 i; for(i = 0; i < header->fields.use; i++){
			STNBHttpHeaderField* f = NBArray_itmPtrAtIndex(&header->fields, STNBHttpHeaderField, i);
			if(!NBHttpBuilder_addHeaderField(obj, dst, &header->strs.str[f->name], &header->strs.str[f->value])){
				r = FALSE; NBASSERT(FALSE)
				break;
			}
		}
	}
	return r;
}

BOOL NBHttpBuilder_addHeaderFieldsPlain(STNBHttpBuilder* obj, STNBString* dst, const STNBHttpHeader* header){
	BOOL r = TRUE;
	{
		UI32 i; for(i = 0; i < header->fields.use; i++){
			STNBHttpHeaderField* f = NBArray_itmPtrAtIndex(&header->fields, STNBHttpHeaderField, i);
			if(!NBHttpBuilder_addHeaderFieldPlain(obj, dst, &header->strs.str[f->name], &header->strs.str[f->value])){
				r = FALSE; NBASSERT(FALSE)
				break;
			}
		}
	}
	return r;
}

//requestLine: method SP request-target SP HTTP-version CRLF
BOOL NBHttpBuilder_addRequestLine(STNBHttpBuilder* obj, STNBString* dst, const char* method, const char* target, const UI8 majorVer, const UI8 minorVer){
	BOOL r = FALSE;
	const UI32 iStart = dst->length;
	if(majorVer < 9 && minorVer < 9){
		if(NBHttpParser_concatEncoded(dst, method, ENNBHttpParserType_method)){
			NBString_concatByte(dst, ' ');
			NBString_concat(dst, target);
			NBString_concatByte(dst, ' ');
			NBString_concatByte(dst, 'H');
			NBString_concatByte(dst, 'T');
			NBString_concatByte(dst, 'T');
			NBString_concatByte(dst, 'P');
			NBString_concatByte(dst, '/');
			NBString_concatUI32(dst, (UI32)majorVer);
			NBString_concatByte(dst, '.');
			NBString_concatUI32(dst, (UI32)minorVer);
			NBString_concatByte(dst, '\r');
			NBString_concatByte(dst, '\n');
			r = TRUE;
		}
	}
	//Remove added content
	if(!r){
		NBString_removeLastBytes(dst, (dst->length - iStart));
	} else {
		obj->bytesTotal += (dst->length - iStart);
	}
	return r;
}

BOOL NBHttpBuilder_addRequestLineWithProtocol(STNBHttpBuilder* obj, STNBString* dst, const char* protocol, const char* method, const char* target, const UI8 majorVer, const UI8 minorVer){
	BOOL r = FALSE;
	const UI32 iStart = dst->length;
	if(NBHttpParser_concatEncoded(dst, method, ENNBHttpParserType_method)){
		NBString_concatByte(dst, ' ');
		NBString_concat(dst, target);
		NBString_concatByte(dst, ' ');
		NBString_concat(dst, protocol);
		NBString_concatByte(dst, '/');
		NBString_concatUI32(dst, (UI32)majorVer);
		NBString_concatByte(dst, '.');
		NBString_concatUI32(dst, (UI32)minorVer);
		NBString_concatByte(dst, '\r');
		NBString_concatByte(dst, '\n');
		r = TRUE;
	}
	//Remove added content
	if(!r){
		NBString_removeLastBytes(dst, (dst->length - iStart));
	} else {
		obj->bytesTotal += (dst->length - iStart);
	}
	return r;
}

//statusLine: HTTP-version SP status-code SP reason-phrase CRLF
BOOL NBHttpBuilder_addStatusLine(STNBHttpBuilder* obj, STNBString* dst, const UI8 majorVer, const UI8 minorVer, const UI32 statusCode, const char* reasonPhrase){
	BOOL r = FALSE;
	const UI32 iStart = dst->length;
	if(majorVer < 9 && minorVer < 9 && (statusCode >= 100 && statusCode <= 999)){
		NBString_concatByte(dst, 'H');
		NBString_concatByte(dst, 'T');
		NBString_concatByte(dst, 'T');
		NBString_concatByte(dst, 'P');
		NBString_concatByte(dst, '/');
		NBString_concatUI32(dst, (UI32)majorVer);
		NBString_concatByte(dst, '.');
		NBString_concatUI32(dst, (UI32)minorVer);
		NBString_concatByte(dst, ' ');
		NBString_concatUI32(dst, statusCode);
		NBString_concatByte(dst, ' ');
		if(NBHttpParser_concatEncoded(dst, reasonPhrase, ENNBHttpParserType_reasonPhrase)){
			NBString_concatByte(dst, '\r');
			NBString_concatByte(dst, '\n');
			r = TRUE;
		}
	}
	//Remove added content
	if(!r){
		NBString_removeLastBytes(dst, (dst->length - iStart));
	} else {
		obj->bytesTotal += (dst->length - iStart);
	}
	return r;
}

//headerField:	field-name ":" OWS field-value OWS
BOOL NBHttpBuilder_addHeaderField(STNBHttpBuilder* obj, STNBString* dst, const char* name, const char* value){
	BOOL r = FALSE;
	const UI32 iStart = dst->length;
	if(NBHttpParser_concatEncoded(dst, name, ENNBHttpParserType_fieldName)){
		NBString_concatByte(dst, ':'); NBString_concatByte(dst, ' ');
		NBString_concat(dst, value);
		NBString_concatByte(dst, '\r');
		NBString_concatByte(dst, '\n');
		r = TRUE;
	}
	NBASSERT(r)
	//Remove added content
	if(!r){
		NBString_removeLastBytes(dst, (dst->length - iStart));
	} else {
		obj->bytesTotal += (dst->length - iStart);
	}
	return r;
}

BOOL NBHttpBuilder_addHeaderFieldPlain(STNBHttpBuilder* obj, STNBString* dst, const char* name, const char* value){
	BOOL r = FALSE;
	const UI32 iStart = dst->length;
	{
		NBString_concat(dst, name);
		NBString_concatByte(dst, ':'); NBString_concatByte(dst, ' ');
		NBString_concat(dst, value);
		NBString_concatByte(dst, '\r');
		NBString_concatByte(dst, '\n');
		r = TRUE;
	}
	NBASSERT(r)
	//Remove added content
	if(!r){
		NBString_removeLastBytes(dst, (dst->length - iStart));
	} else {
		obj->bytesTotal += (dst->length - iStart);
	}
	return r;
}

BOOL NBHttpBuilder_addHeaderFieldFromUnscaped(STNBHttpBuilder* obj, STNBString* dst, const char* name, const char* value){
	BOOL r = FALSE;
	const UI32 iStart = dst->length;
	if(NBHttpParser_concatEncoded(dst, name, ENNBHttpParserType_fieldName)){
		NBString_concatByte(dst, ':'); NBString_concatByte(dst, ' ');
		if(NBHttpParser_concatEncoded(dst, value, ENNBHttpParserType_fieldValue)){
			NBString_concatByte(dst, '\r');
			NBString_concatByte(dst, '\n');
			r = TRUE;
		}
	}
	NBASSERT(r)
	//Remove added content
	if(!r){
		NBString_removeLastBytes(dst, (dst->length - iStart));
	} else {
		obj->bytesTotal += (dst->length - iStart);
	}
	return r;
}

BOOL NBHttpBuilder_addHeaderFieldStruct(STNBHttpBuilder* obj, STNBString* dst, const char* name, const STNBStructMap* structMap, const void* src, const UI32 srcSz){
	BOOL r = FALSE;
	const UI32 iStart = dst->length;
	if(NBHttpParser_concatEncoded(dst, name, ENNBHttpParserType_fieldName)){
		NBString_concatByte(dst, ':'); NBString_concatByte(dst, ' ');
		if(NBStruct_stConcatAsJsonBase64(dst, structMap, src, srcSz)){
			NBString_concatByte(dst, '\r');
			NBString_concatByte(dst, '\n');
			r = TRUE;
		}
	}
	NBASSERT(r)
	//Remove added content
	if(!r){
		NBString_removeLastBytes(dst, (dst->length - iStart));
	} else {
		obj->bytesTotal += (dst->length - iStart);
	}
	return r;
}

//Host = uri-host [ ":" port ]
BOOL NBHttpBuilder_addHost(STNBHttpBuilder* obj, STNBString* dst, const char* host, const SI32 port){
	BOOL r = FALSE;
	const UI32 iStart = dst->length;
	if(*host != '\0'){
		NBString_concat(dst, "Host:");
		NBString_concat(dst, host);
		if(port > 0){
			NBString_concat(dst, ":");
			NBString_concatSI32(dst, port);
		}
		NBString_concatByte(dst, '\r');
		NBString_concatByte(dst, '\n');
		r = TRUE;
	}
	//Remove added content
	if(!r){
		NBString_removeLastBytes(dst, (dst->length - iStart));
	} else {
		obj->bytesTotal += (dst->length - iStart);
	}
	return r;
}

BOOL NBHttpBuilder_addHostFromUnscaped(STNBHttpBuilder* obj, STNBString* dst, const char* host){
	BOOL r = FALSE;
	const UI32 iStart = dst->length;
	if(*host != '\0'){
		STNBUriParser p;
		NBUriParser_init(&p);
		if(NBUriParser_feedStartWithType(&p, ENNBUriParserType_host)){
			while(*host != '\0') {
				if(!NBUriParser_feedChar(&p, *host)){
					break;
				}
				host++;
			}
			if(NBUriParser_feedEnd(&p)){
				NBString_concat(dst, "Host:");
				NBString_concat(dst, p.strAcum.str);
				if(*host == '\0'){
					NBString_concatByte(dst, '\r');
					NBString_concatByte(dst, '\n');
					r = TRUE; //Host only
				} else if(*host == ':'){
					host++;
					if(*host == '\0'){
						//Nothing after the ':' (ignore)
						NBString_concatByte(dst, '\r');
						NBString_concatByte(dst, '\n');
						r = TRUE; //Host only
					} else {
						NBString_concatByte(dst, ':');
						if(NBUriParser_feedStartWithType(&p, ENNBUriParserType_port)){
							while(*host != '\0') {
								if(!NBUriParser_feedChar(&p, *host)){
									break;
								}
								host++;
							}
							if(NBUriParser_feedEnd(&p)){
								if(*host == '\0'){
									NBString_concat(dst, p.strAcum.str);
									NBString_concatByte(dst, '\r');
									NBString_concatByte(dst, '\n');
									r = TRUE; //Port completed
								}
							}
						}
					}
				}
			}
		}
		NBUriParser_release(&p);
	}
	//Remove added content
	if(!r){
		NBString_removeLastBytes(dst, (dst->length - iStart));
	} else {
		obj->bytesTotal += (dst->length - iStart);
	}
	return r;
}

//contentLength: 1*DIGIT
BOOL NBHttpBuilder_addContentLength(STNBHttpBuilder* obj, STNBString* dst, const UI64 contentLength){
	const UI32 iStart = dst->length;
	NBString_concat(dst, "Content-Length:");
	NBString_concatUI64(dst, contentLength);
	NBString_concatByte(dst, '\r');
	NBString_concatByte(dst, '\n');
	obj->bytesTotal += (dst->length - iStart);
	return TRUE;
}

//transferEncoding: 1#transfer-coding == transfer-coding *( OWS "," OWS transfer-coding )
BOOL NBHttpBuilder_addTransferEncoding(STNBHttpBuilder* obj, STNBString* dst, const STNBHttpTransferEncoding* encoding){
	BOOL r = FALSE;
	const UI32 iStart = dst->length;
	if(encoding != NULL){
		if(encoding->codings.use > 0){
			r = TRUE;
			NBString_concat(dst, "Transfer-Encoding:");
			UI32 i; for(i = 0; i < encoding->codings.use && r; i++){
				const STNBHttpTransferCoding* c = NBArray_itmPtrAtIndex(&encoding->codings, STNBHttpTransferCoding, i);
				if(i != 0) NBString_concatByte(dst, ',');
				switch (c->type) {
					case ENNBHttpTransferCodingType_chunked: NBString_concat(dst, "chunked"); break;
					case ENNBHttpTransferCodingType_compress: NBString_concat(dst, "compress"); break;
					case ENNBHttpTransferCodingType_deflate: NBString_concat(dst, "deflate"); break;
					case ENNBHttpTransferCodingType_gzip: NBString_concat(dst, "gzip"); break;
					case ENNBHttpTransferCodingType_extension:
						//transferExtension: token *( OWS ";" OWS transfer-parameter )
						//transferParameter: token / token BWS "=" BWS ( token / quoted-string )
						if(!NBHttpParser_concatEncoded(dst, c->extensionType->str, ENNBHttpParserType_token)){
							r = FALSE; NBASSERT(FALSE)
						} else if(c->params != NULL){
							UI32 i; for(i = 0; i < c->params->use; i++){
								const STNBHttpTransferParam* p = NBArray_itmPtrAtIndex(c->params, STNBHttpTransferParam, i);
								NBString_concatByte(dst, ';');
								const char* pName = &c->paramsStrs->str[p->iName];
								const char* pValue = &c->paramsStrs->str[p->iValue];
								if(!NBHttpParser_concatEncoded(dst, pName, ENNBHttpParserType_token)){
									r = FALSE; NBASSERT(FALSE)
									break;
								} else if(*pValue != '\0'){
									NBString_concatByte(dst, '=');
									if(!NBHttpParser_concatEncoded(dst, pValue, ENNBHttpParserType_chunkExtVal)){ //token / quoted-string
										r = FALSE; NBASSERT(FALSE)
										break;
									}
								}
							}
						}
						break;
					default:
						r = FALSE; NBASSERT(FALSE)
						break;
				}
			}
			if(r){
				NBString_concatByte(dst, '\r');
				NBString_concatByte(dst, '\n');
			}
		}
	}
	//Remove added content
	if(!r){
		NBString_removeLastBytes(dst, (dst->length - iStart));
	} else {
		obj->bytesTotal += (dst->length - iStart);
	}
	return r;
}

//CRLF
BOOL NBHttpBuilder_addHeaderEnd(STNBHttpBuilder* obj, STNBString* dst){
	NBString_concatByte(dst, '\r');
	NBString_concatByte(dst, '\n');
	obj->bytesTotal += 2;
	return TRUE;
}

//Chunked body
BOOL NBHttpBuilder_addChunkHeader(STNBHttpBuilder* obj, STNBString* dst, const UI64 chunkSz){
	return NBHttpBuilder_addChunkHeaderWithExt(obj, dst, chunkSz, NULL);
}

//chunk-ext = *( BWS  ";" BWS chunk-ext-name [ BWS  "=" BWS chunk-ext-val ] )
BOOL NBHttpBuilder_addChunkHeaderWithExt(STNBHttpBuilder* obj, STNBString* dst, const UI64 chunkSz, const STNBHttpChunkExt* ext){
	BOOL r = TRUE;
	const UI32 iStart = dst->length;
	//Add hex-size
	{
		char hex[32]; UI32 iHex = 0;
		const char digs[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
		UI64 v = chunkSz;
		do {
			hex[iHex++] = digs[(v & 0xF)];
			v >>= 4; NBASSERT(chunkSz == 0 || chunkSz != v)
		} while(v != 0);
		NBASSERT(iHex > 0)
		do {
			NBString_concatByte(dst, hex[--iHex]);
		} while(iHex != 0);
	}
	//Add extensions
	if(ext != NULL){
		UI32 i; for(i = 0; i < ext->exts.use; i++){
			const STNBHttpChunkExtElem* e = NBArray_itmPtrAtIndex(&ext->exts, STNBHttpChunkExtElem, i);
			const char* name = &ext->strs.str[e->iName];
			const char* value = &ext->strs.str[e->iValue];
			NBString_concatByte(dst, ';');
			if(!NBHttpParser_concatEncoded(dst, name, ENNBHttpParserType_chunkExtName)){
				r = FALSE; NBASSERT(FALSE)
				break;
			} else {
				if(*value != '\0'){
					NBString_concatByte(dst, '=');
					if(!NBHttpParser_concatEncoded(dst, value, ENNBHttpParserType_chunkExtVal)){
						r = FALSE; NBASSERT(FALSE)
						break;
					}
				}
			}
		}
	}
	NBString_concatByte(dst, '\r');
	NBString_concatByte(dst, '\n');
	//Remove added content
	if(!r){
		NBString_removeLastBytes(dst, (dst->length - iStart));
	} else {
		obj->bytesTotal += (dst->length - iStart);
	}
	return r;
}

BOOL NBHttpBuilder_addChunkDataEnd(STNBHttpBuilder* obj, STNBString* dst){
	NBString_concatByte(dst, '\r');
	NBString_concatByte(dst, '\n');
	obj->bytesTotal += 2;
	return TRUE;
}

BOOL NBHttpBuilder_addChunkBodyEnd(STNBHttpBuilder* obj, STNBString* dst){
	NBString_concatByte(dst, '\r');
	NBString_concatByte(dst, '\n');
	obj->bytesTotal += 2;
	return TRUE;
}



