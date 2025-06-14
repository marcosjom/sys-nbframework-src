
#include "nb/NBFrameworkPch.h"
#include "nb/net/NBHttpTransferEncoding.h"
#include "nb/net/NBHttpParser.h"

void NBHttpTransferEncoding_init(STNBHttpTransferEncoding* obj){
	NBArray_init(&obj->codings, sizeof(STNBHttpTransferCoding), NULL);
}

void NBHttpTransferEncoding_release(STNBHttpTransferEncoding* obj){
	{
		UI32 i; for(i = 0; i < obj->codings.use; i++){
			STNBHttpTransferCoding* c = NBArray_itmPtrAtIndex(&obj->codings, STNBHttpTransferCoding, i);
			NBHttpTransferCoding_release(c);
		}
		NBArray_empty(&obj->codings);
		NBArray_release(&obj->codings);
	}
}

BOOL NBHttpTransferEncoding_isChunked(STNBHttpTransferEncoding* obj){
	BOOL r = FALSE;
	UI32 i; for(i = 0; i < obj->codings.use; i++){
		STNBHttpTransferCoding* c = NBArray_itmPtrAtIndex(&obj->codings, STNBHttpTransferCoding, i);
		if(c->type == ENNBHttpTransferCodingType_chunked){
			r = TRUE; break;
		}
	}
	return r;
}

BOOL NBHttpTransferEncoding_addFromHttpParser(STNBHttpTransferEncoding* obj, STNBHttpParser* subpar){
	BOOL r = FALSE;
	//transferEncoding: 1#transfer-coding == transfer-coding *( OWS "," OWS transfer-coding )
	if(subpar->nodesStack.use > 0){
		const STNBHttpParserNode* start		= NBArray_dataPtr(&subpar->nodesStack, STNBHttpParserNode);
		const STNBHttpParserNode* afterEnd	= start + subpar->nodesStack.use;
		const STNBHttpParserNode* tEnc		= NBArray_itmPtrAtIndex(&subpar->nodesStack, STNBHttpParserNode, 0);
		NBASSERT(tEnc->type == ENNBHttpParserType_transferEncoding)
		if((tEnc + 1) < afterEnd){
			UI32 iCoding = 0, iTotalParam = 0;
			const STNBHttpParserNode* tCod = (tEnc + 1);
			NBASSERT(tCod->type == ENNBHttpParserType_transferCoding)
			if(tCod->type == ENNBHttpParserType_transferCoding){
				r = TRUE;
				do {
					//transferCoding:	"chunked" / "compress" / "deflate" / "gzip" / transfer-extension
					const char* valStr	= &subpar->strAcum.str[tCod->rng.iStart];
					const UI32 valLen	= tCod->rng.count;
					BOOL isExt			= FALSE;
					if((tCod + 1) < afterEnd){
						const STNBHttpParserNode* next = (tCod + 1);
						if(next->type == ENNBHttpParserType_transferExtension){
							isExt = TRUE;
						}
					}
					if(!isExt){
						if(NBString_strIsEqualStrBytes("chunked", valStr, valLen)){
							NBHttpTransferEncoding_addCodingType(obj, ENNBHttpTransferCodingType_chunked);
							//PRINTF_INFO("NBHttpTransferEncoding, chunked\n");
						} else if(NBString_strIsEqualStrBytes("compress", valStr, valLen)){
							NBHttpTransferEncoding_addCodingType(obj, ENNBHttpTransferCodingType_compress);
							//PRINTF_INFO("NBHttpTransferEncoding, compress\n");
						} else if(NBString_strIsEqualStrBytes("deflate", valStr, valLen)){
							NBHttpTransferEncoding_addCodingType(obj, ENNBHttpTransferCodingType_deflate);
							//PRINTF_INFO("NBHttpTransferEncoding, deflate\n");
						} else if(NBString_strIsEqualStrBytes("gzip", valStr, valLen)){
							NBHttpTransferEncoding_addCodingType(obj, ENNBHttpTransferCodingType_gzip);
							//PRINTF_INFO("NBHttpTransferEncoding, gzip\n");
						} else {
							r = FALSE; NBASSERT(FALSE) //Unexpected coding (not extension)
							break;
						}
					} else {
						//transferExtension:	token *( OWS ";" OWS transfer-parameter )
						NBASSERT((tCod + 2) < afterEnd)
						const STNBHttpParserNode* tExt		= (tCod + 1);	NBASSERT(tExt->type == ENNBHttpParserType_transferExtension)
						const STNBHttpParserNode* tExtName	= (tCod + 2);	NBASSERT(tExtName->type == ENNBHttpParserType_token)
						const char* nameStr	= &subpar->strAcum.str[tExtName->rng.iStart];
						const UI32 nameLen	= tExtName->rng.count;
						//
						PRINTF_INFO("NBHttpTransferEncoding, extension.\n");
						//
						STNBHttpTransferCoding c;
						NBHttpTransferCoding_init(&c);
						NBHttpTransferCoding_setTypeExtensionBytes(&c, nameStr, nameLen);
						//Add parameters
						if((tExtName + 1) < afterEnd){
							const STNBHttpParserNode* tParam = (tExtName + 1); NBASSERT(tParam->type == ENNBHttpParserType_transferCoding || tParam->type == ENNBHttpParserType_transferParameter)
							if(tParam->type == ENNBHttpParserType_transferParameter){
								UI32 iParam = 0;
								do {
									if(tParam->type != ENNBHttpParserType_transferParameter){
										r = FALSE; NBASSERT(FALSE)
										break;
									} else {
										//transferParameter:	token / token BWS "=" BWS ( token / quoted-string )
										const STNBHttpParserNode* tParamName = tParam + 1; NBASSERT(tParamName->type == ENNBHttpParserType_token)
										const char* nameStr	= &subpar->strAcum.str[tParamName->rng.iStart];
										const UI32 nameLen	= tParamName->rng.count;
										const STNBHttpParserNode* tParamValue = NBHttpParser_nextNode(subpar, tParam, tParamName);
										if(tParamValue == NULL){
											NBHttpTransferCoding_addParamBytes(&c, nameStr, nameLen, NULL, 0);
											PRINTF_INFO("NBHttpTransferEncoding, extension-param-empty.\n");
										} else {
											if(tParamValue->type == ENNBHttpParserType_token){
												const char* valStr	= &subpar->strAcum.str[tParamValue->rng.iStart];
												const UI32 valLen	= tParamValue->rng.count;
												NBHttpTransferCoding_addParamBytes(&c, nameStr, nameLen, valStr, valLen);
												PRINTF_INFO("NBHttpTransferEncoding, extension-param-with-token.\n");
											} else if(tParamValue->type == ENNBHttpParserType_quotedString){
												STNBString unscaped;
												NBString_init(&unscaped);
												NBHttpParser_concatUnencodedNode(subpar, tParamValue, &unscaped);
												NBHttpTransferCoding_addParamBytes(&c, nameStr, nameLen, unscaped.str, unscaped.length);
												PRINTF_INFO("NBHttpTransferEncoding, extension-param-with-quoted '%s'.\n", unscaped.str);
												NBString_release(&unscaped);
											} else {
												r = FALSE; NBASSERT(FALSE)
												break;
											}
										}
									}
									//Next param
									tParam = NBHttpParser_nextNodeWithType(subpar, tExt, tParam, ENNBHttpParserType_transferParameter);
									iParam++; iTotalParam++;
								} while(tParam != NULL && r);
							}
						}
						//Add
						NBHttpTransferEncoding_addCoding(obj, &c);
						NBHttpTransferCoding_release(&c);
					}
					//Next Coding
					tCod = NBHttpParser_nextNodeWithType(subpar, tEnc, tCod, ENNBHttpParserType_transferCoding);
					iCoding++;
				} while(tCod != NULL && r);
			}
		}
	}
	return r;
}


BOOL NBHttpTransferEncoding_addFromLine(STNBHttpTransferEncoding* obj, const char* line){
	BOOL r = FALSE;
	STNBHttpParser subpar;
	NBHttpParser_init(&subpar);
	NBHttpParser_feedStartWithType(&subpar, ENNBHttpParserType_transferEncoding);
	const UI32 consumed = NBHttpParser_feed(&subpar, line);
	if(line[consumed] != '\0'){
		r = FALSE; NBASSERT(FALSE)
	} else if(!NBHttpParser_feedEnd(&subpar)){
		r = FALSE; NBASSERT(FALSE)
	} else {
		r = NBHttpTransferEncoding_addFromHttpParser(obj, &subpar);
	}
	NBHttpParser_release(&subpar);
	return r;
}

BOOL NBHttpTransferEncoding_addFromLineBytes(STNBHttpTransferEncoding* obj, const char* line, const UI32 lineSz){
	BOOL r = FALSE;
	STNBHttpParser subpar;
	NBHttpParser_init(&subpar);
	NBHttpParser_feedStartWithType(&subpar, ENNBHttpParserType_transferEncoding);
	if(NBHttpParser_feedBytes(&subpar, line, lineSz) != lineSz){
		r = FALSE; NBASSERT(FALSE)
	} else if(!NBHttpParser_feedEnd(&subpar)){
		r = FALSE; NBASSERT(FALSE)
	} else {
		r = NBHttpTransferEncoding_addFromHttpParser(obj, &subpar);
	}
	NBHttpParser_release(&subpar);
	return r;
}

BOOL NBHttpTransferEncoding_addCodingType(STNBHttpTransferEncoding* obj, const ENNBHttpTransferCodingType type){
	BOOL r = FALSE;
	//No extension allowed
	if(type == ENNBHttpTransferCodingType_chunked || type == ENNBHttpTransferCodingType_compress || type == ENNBHttpTransferCodingType_deflate || type == ENNBHttpTransferCodingType_gzip){
		STNBHttpTransferCoding c;
		NBHttpTransferCoding_init(&c);
		NBHttpTransferCoding_setType(&c, type);
		NBArray_addValue(&obj->codings, c);
	}
	return r;
}

BOOL NBHttpTransferEncoding_addCoding(STNBHttpTransferEncoding* obj, const STNBHttpTransferCoding* coding){
	STNBHttpTransferCoding c;
	NBHttpTransferCoding_initWithOther(&c, coding);
	NBArray_addValue(&obj->codings, c);
	return TRUE;
}

