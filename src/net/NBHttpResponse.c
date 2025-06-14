
#include "nb/NBFrameworkPch.h"
#include "nb/net/NBHttpResponse.h"
#include "nb/core/NBMemory.h"

//Factory
void NBHttpResponse_init(STNBHttpResponse* obj){
	obj->_transfType	= ENNBHttpTipoTransf_SinDefinir;
	obj->_transfEnded	= FALSE;
	obj->_zeroContentLenAsDef = FALSE;
    NBHttpResponseReadState_init(&obj->_transfState);
	//
	obj->bytesReceived	= 0;
	obj->bytesExpected	= 0;
	//
	obj->httpVersion	= 0.0f;
	obj->code			= 0;
	NBArray_init(&obj->headers, sizeof(STNBHttpRespHeadr), NULL);
    NBString_init(&obj->header);
    NBString_init(&obj->body);
	//
	obj->retainCount = 1;
}

void NBHttpResponse_retain(STNBHttpResponse* obj){
	obj->retainCount++;
}

void NBHttpResponse_release(STNBHttpResponse* obj){
    NBASSERT(obj->retainCount > 0)
	obj->retainCount--;
	if(obj->retainCount == 0){
	    NBHttpResponseReadState_release(&obj->_transfState);
		{
			UI32 i; for(i = 0; i < obj->headers.use; i++){
				STNBHttpRespHeadr* hdr = (STNBHttpRespHeadr*)NBArray_itemAtIndex(&obj->headers, i);
				NBString_release(&hdr->name);
				NBString_release(&hdr->value);
			}
			NBArray_empty(&obj->headers);
			NBArray_release(&obj->headers);
		}
	    NBString_release(&obj->header);
	    NBString_release(&obj->body);
	}
}

//Process data

BOOL NBHttpResponse_consumeHeader(STNBHttpResponse* obj, const char* protocol){
	BOOL r = FALSE;
	obj->_transfType = ENNBHttpTipoTransf_Desconocido;
	//Analyze first line
	{
		const SI32 lineEnd = NBString_strIndexOf(obj->header.str, "\r\n", 0); NBASSERT(lineEnd != -1) //At least one '\r\n' should exists
		if(lineEnd != -1){
			const SI32 protclEnd = NBString_strIndexOf(obj->header.str, "/", 0);
			NBASSERT(protclEnd == NBString_strLenBytes(protocol))
			NBASSERT(protclEnd != -1 && protclEnd < lineEnd)
			if(protclEnd == NBString_strLenBytes(protocol)){
				const char* strLine = obj->header.str;
				NBASSERT(NBString_strIsEqualStrBytes(protocol, strLine, protclEnd))
				if(NBString_strIsEqualStrBytes(protocol, strLine, protclEnd)){
					const SI32 verEnd = NBString_strIndexOf(obj->header.str, " ", protclEnd + 1);
					NBASSERT(verEnd != -1 && verEnd < lineEnd && protclEnd < verEnd)
					if(verEnd != -1 && verEnd < lineEnd && protclEnd < verEnd){
						const SI32 codeStart = NBString_strIndexOf(obj->header.str, " ", verEnd + 1);
						NBASSERT(codeStart != -1 && codeStart < lineEnd && verEnd < codeStart)
						if(codeStart != -1 && codeStart < lineEnd && verEnd < codeStart){
							STNBString strTmp;
							NBString_init(&strTmp);
							//Protocol version
							{
								NBString_empty(&strTmp);
								NBString_concatBytes(&strTmp, &strLine[protclEnd + 1], verEnd - protclEnd - 1);
								NBASSERT(NBString_strIsDecimal(strTmp.str))
								if(NBString_strIsDecimal(strTmp.str)){
									obj->httpVersion = NBString_strToFloat(strTmp.str);
									//Protocol response code
									{
										NBString_empty(&strTmp);
										NBString_concatBytes(&strTmp, &strLine[verEnd + 1], codeStart - verEnd - 1);
										NBASSERT(NBString_strIsInteger(strTmp.str))
										if(NBString_strIsInteger(strTmp.str)){
											obj->code = NBString_strToSI32(strTmp.str);
											r = TRUE;
											//Parse headers
											{
												SI32 lineStart = (lineEnd + 2); //+2 for '\r\n'
												while(lineStart < obj->header.length){
													SI32 lineEnd = NBString_strIndexOf(obj->header.str, "\r\n", lineStart); if(lineEnd == -1) lineEnd = obj->header.length;
													if(lineStart < lineEnd){
														SI32 valSep = NBString_strIndexOf(obj->header.str, ":", lineStart); if(valSep >= lineEnd) valSep = -1;
														if(valSep != -1){
															SI32 nameLen = (valSep - lineStart);
															SI32 valueStart = valSep + 1;
															//Remove spaces
															while(nameLen > 0){
																if(obj->header.str[lineStart + nameLen - 1] != ' '){
																	break;
																}
																nameLen--;
															}
															while(obj->header.str[valueStart] == ' '){
																valueStart++;
															}
															//'name: value' pair
															STNBHttpRespHeadr hdr;
															NBString_init(&hdr.name);
															NBString_init(&hdr.value);
															NBString_concatBytes(&hdr.name, &obj->header.str[lineStart], nameLen);
															NBString_concatBytes(&hdr.value, &obj->header.str[valueStart], (lineEnd - valueStart));
															//PRINTF_INFO("HttpResponse, header name('%s') value('%s').\n", hdr.name.str, hdr.value.str);
															if(obj->_transfType == ENNBHttpTipoTransf_Desconocido){
																if(NBString_strIsLike(hdr.name.str, "Content-Length")){
																	obj->_transfType		= ENNBHttpTipoTransf_ContentLength;
																	obj->bytesExpected		= obj->header.length + NBString_strToSI32(hdr.value.str);
																} else if(NBString_strIsLike(hdr.name.str, "Transfer-Encoding")){
																	if(NBString_strIsEqual(hdr.value.str, "chunked")){
																		obj->_transfType	= ENNBHttpTipoTransf_ChunckedEncoding;
																	} 
																}
															}
															NBArray_addValue(&obj->headers, hdr);
														} else {
															//not a name-value pair
															PRINTF_ERROR("HttpResponse, unexpected header line starting at: '...%s'.\n", &obj->header.str[lineStart]);
															r = FALSE; NBASSERT(FALSE)
															break;
														}
													}
													//Next line
													lineStart = (lineEnd + 2); //+2 for '\r\n'
												}
											}
                                            //204 No Content, return code
                                            if(obj->_transfType == ENNBHttpTipoTransf_Desconocido && obj->code == 204){
                                                obj->_transfType        = ENNBHttpTipoTransf_204NoContent;
                                                obj->bytesExpected      = obj->header.length;
                                            }
											//Default content-length
											if(obj->_transfType == ENNBHttpTipoTransf_Desconocido && obj->_zeroContentLenAsDef){
												obj->_transfType		= ENNBHttpTipoTransf_ContentLength;
												obj->bytesExpected		= obj->header.length;
											}
										}
									}
								}
							}
							NBString_release(&strTmp);
						}
					}
				}
			}
		}
	}
	//
	return r;
}

//Config

void NBHttpResponse_setZeroContentLenAsDefault(STNBHttpResponse* obj, const BOOL zeroContentLenAsDef){
	obj->_zeroContentLenAsDef = zeroContentLenAsDef;
}

//

BOOL NBHttpResponse_consumeBytes(STNBHttpResponse* obj, const char* buff, const SI32 bytesRcvd, const char* protocol){
	BOOL r = TRUE;
	obj->bytesReceived += bytesRcvd;
	//Consume header part
	SI32 bytesToConsume	= bytesRcvd;
	if(obj->_transfType == ENNBHttpTipoTransf_SinDefinir){
		NBString_concatBytes(&obj->header, buff, bytesRcvd); bytesToConsume = 0;
		const SI32 headEnd = NBString_strIndexOf(obj->header.str, "\r\n\r\n", 0);
		if(headEnd != -1){
			//Remove excess (+4 for '\r\n\r\n')
			if((headEnd + 4) < obj->header.length){
				bytesToConsume = (obj->header.length - headEnd - 4);
				NBString_removeLastBytes(&obj->header, bytesToConsume);
			}
			//Parse
			if(!NBHttpResponse_consumeHeader(obj, protocol)){
				PRINTF_ERROR("HttpResponse, could not parse header.\n");
				r = FALSE; NBASSERT(FALSE);
			}
		}
	}
	//Consume body part
	if(r){
		if(obj->_transfEnded){
			PRINTF_ERROR("HttpResponse, received extra (%d) bytes after response's body.\n", bytesToConsume);
			r = FALSE; NBASSERT(FALSE);
        } else {
            switch (obj->_transfType) {
                case ENNBHttpTipoTransf_ContentLength:
                    NBString_concatBytes(&obj->body, &buff[bytesRcvd - bytesToConsume], bytesToConsume);
                    if(obj->bytesReceived >= obj->bytesExpected){
                        if(obj->bytesReceived > obj->bytesExpected){
                            PRINTF_ERROR("HttpResponse, received(%d) from expected(%d) bytes.\n", obj->bytesReceived, obj->bytesExpected);
                            r = FALSE; //NBASSERT(FALSE);
                        }
                        obj->_transfEnded = TRUE;
                        //PRINTF_INFO("HttpResponse, all bytes received.\n");
                    }
                    break;
                case ENNBHttpTipoTransf_ChunckedEncoding:
                    if(!NBHttpResponseReadState_processData(&obj->_transfState, &buff[bytesRcvd - bytesToConsume], bytesToConsume, &obj->body)){
                        PRINTF_ERROR("HttpResponse, could not parse chuncked response.\n");
                        r = FALSE; NBASSERT(FALSE);
                    } else if(obj->_transfState.chucnkTamCeroEncontrado){
                        obj->_transfEnded = TRUE;
                        //PRINTF_INFO("HttpResponse, final chunck found.\n");
                    }
                    break;
                case ENNBHttpTipoTransf_204NoContent:
                    if(bytesToConsume != bytesRcvd){
                        PRINTF_ERROR("HttpResponse, 204-code implies zero-body-length (%d of %d bytes consumed).\n", bytesToConsume, bytesRcvd);
                        r = FALSE; NBASSERT(FALSE);
                    } else {
                        obj->_transfEnded = TRUE;
                        //PRINTF_INFO("HttpResponse, 204-code implicit zero-body-length.\n");
                    }
                    break;
                default:
                    break;
            }
		}
	}
	return r;
}

const char* NBHttpResponse_headerValue(const STNBHttpResponse* obj, const char* header, const char* defValue){
	const char* r = defValue;
	SI32 i; for(i = 0; i < obj->headers.use; i++){
		const STNBHttpRespHeadr* f = (const STNBHttpRespHeadr*)NBArray_itemAtIndex(&obj->headers, i);
		if(NBString_strIsLike(f->name.str, header)){
			r = f->value.str;
			break;
		}
	}
	return r;
}

//Cursor for reading header values:
//Multiple http headers can be combined in one separated by comma;
//quoted-strings "..." and comments (...) can be included.

BOOL NBHttpResponse_getNextFieldValue(const STNBHttpResponse* obj, const char* fieldName, const BOOL allowComments, STNBHttpHeaderFieldCursor* cursor){
	BOOL r = FALSE;
	if(cursor != NULL){
		SI32 i;
		//Init results
		cursor->value		= NULL;
		cursor->valueLen	= 0;
		//Search
		for(i = cursor->state.iField; i < obj->headers.use; i++){
			const STNBHttpRespHeadr* f = (const STNBHttpRespHeadr*)NBArray_itemAtIndex(&obj->headers, i);
			if(NBString_strIsLike(f->name.str, fieldName)){
				const UI32 iStart = cursor->state.iByte;
				const char* fValueStart	= &f->value.str[iStart];
				cursor->state.iByte = 0;
				if(iStart < f->value.length){
					if(NBHttpHeader_getNextFieldValueStr(fValueStart, allowComments, cursor)){
						NBASSERT(cursor->value != NULL)
						cursor->state.iByte += iStart;
						r = TRUE;
						break;
					}
				}
			}
		}
		//Keep state
		cursor->state.iField = i;
	}
	return r;
}

//Parse data

void NBHttpResponseReadState_init(STNBHttpLecturaChunckEstado* obj){
	obj->errorEncontrado			= FALSE;
	obj->chucnkTamCeroEncontrado	= FALSE;
	obj->modoLectura				= ENNBHttpLecturaChunckModo_TamanoHex;
    NBString_init(&obj->strBuffer);
	obj->chunckActualBytesEsperados = 0;
	obj->chucnkActualBytesLeidos	= 0;
}

void NBHttpResponseReadState_release(STNBHttpLecturaChunckEstado* obj){
	obj->errorEncontrado			= FALSE;
	obj->chucnkTamCeroEncontrado	= FALSE;
	obj->modoLectura				= ENNBHttpLecturaChunckModo_TamanoHex;
    NBString_release(&obj->strBuffer);
	obj->chunckActualBytesEsperados = 0;
	obj->chucnkActualBytesLeidos	= 0;
}

BOOL NBHttpResponseReadState_processData(STNBHttpLecturaChunckEstado* obj, const char* dataRead, const SI32 bytesCount, STNBString* saveContentAt){
	SI32 i = 0;
	if(obj->errorEncontrado) return FALSE; //Dont process data after ERROR state
	//
	obj->errorEncontrado = TRUE;
	while(i < bytesCount) {
		if(obj->modoLectura == ENNBHttpLecturaChunckModo_TamanoHex) {
			do {
				char c = dataRead[i++];
				//TODO: optimize this conditinal with ranges, not individual values
				if(c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || c == '7' || c == '8' || c == '9' || c == 'A' || c == 'a' || c == 'B' || c == 'b' || c == 'C' || c == 'c' || c == 'D' || c == 'd' || c == 'E' || c == 'e' || c == 'F' || c == 'f') {
				    NBString_concatByte(&obj->strBuffer, c);
				} else if(c == '\r') {
					if(obj->strBuffer.length == 0) {
						PRINTF_ERROR("HttpResponse, HEX value expected.\n");
						return FALSE;
					}
					obj->chunckActualBytesEsperados  = NBString_strToSI32FromHex(obj->strBuffer.str); if(obj->chunckActualBytesEsperados == 0) obj->chucnkTamCeroEncontrado = TRUE;
					obj->chucnkActualBytesLeidos     = 0;
					obj->modoLectura = ENNBHttpLecturaChunckModo_FinTamanoHex;
					//PRINTF_INFO("HttpResponse, chunck size: %d ('%s').\n", obj->chunckActualBytesEsperados, obj->strBuffer->str());
				    NBString_empty(&obj->strBuffer);
				    NBString_concatByte(&obj->strBuffer, c);
					break;
				} else {
					PRINTF_ERROR("HttpResponse, only HEX or '\\r' expected, with buffer('%s') we received ('%c').\n", obj->strBuffer.str, c);
					return FALSE;
				}
			} while(i < bytesCount);
		} else if(obj->modoLectura == ENNBHttpLecturaChunckModo_FinTamanoHex || obj->modoLectura == ENNBHttpLecturaChunckModo_FinContenido) {
			do {
				char c = dataRead[i++];
				if(c == '\r' || c == '\n') {
				    NBString_concatByte(&obj->strBuffer, c);
					if(obj->strBuffer.length == 2) {
						if(!NBString_strIsEqual(obj->strBuffer.str, "\r\n")){
							PRINTF_ERROR("HttpResponse, '\\r\\n' expected.\n");
							return FALSE;
						} else {
						    NBString_empty(&obj->strBuffer);
							obj->modoLectura = (obj->modoLectura == ENNBHttpLecturaChunckModo_FinTamanoHex ? ENNBHttpLecturaChunckModo_Contenido : ENNBHttpLecturaChunckModo_TamanoHex);
							break;
						}
					}
				} else {
					PRINTF_ERROR("HttpResponse, only '\\r' and '\\n' expected.\n");
					return FALSE;
				}
			} while(i < bytesCount);
		} else  if(obj->modoLectura == ENNBHttpLecturaChunckModo_Contenido) {
			SI32 bytesConsumibles = (bytesCount - i);
			if(bytesConsumibles > 0){
				int bytesConsumir = obj->chunckActualBytesEsperados - obj->chucnkActualBytesLeidos; if(bytesConsumir > bytesConsumibles) bytesConsumir = bytesConsumibles;
				if(bytesConsumir > 0){
				    NBString_concatBytes(saveContentAt, &dataRead[i], bytesConsumir);
					i += bytesConsumir;
					obj->chucnkActualBytesLeidos += bytesConsumir;
				} else if(bytesConsumir == 0){
				    NBString_empty(&obj->strBuffer);
					obj->modoLectura = ENNBHttpLecturaChunckModo_FinContenido;
				} else {
					PRINTF_ERROR("HttpResponse, bytes_to_consume is negative.\n");
					return FALSE;
				}
			}
		} else {
			PRINTF_ERROR("HttpResponse, unknown content type to read.\n");
			return FALSE;
		}
	}
	obj->errorEncontrado = FALSE;
	return TRUE;
}
