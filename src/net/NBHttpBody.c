
#include "nb/NBFrameworkPch.h"
#include "nb/net/NBHttpBody.h"
#include "nb/net/NBHttpParser.h"

void NBHttpBody_init(STNBHttpBody* obj){
	obj->isCompleted		= FALSE;
	obj->readBuffSz			= 4096;
	obj->storgBuffSz		= 4096;
	obj->mode				= ENNBHttpBodyMode_untillClose;
	obj->expctLength		= 0;
	obj->dataTotalSz		= 0;
	obj->httpParser			= NULL;
	NBArray_init(&obj->chunks, sizeof(STNBHttpBodyChunk), NULL);
	NBArray_init(&obj->fields, sizeof(STNBHttpBodyField), NULL);
	NBString_init(&obj->strs);
	NBString_concatByte(&obj->strs, '\0'); //Index 0 must be empty string
	NBMemory_set(&obj->listnr, 0, sizeof(obj->listnr));
	obj->listnrParam		= NULL;
}

void NBHttpBody_release(STNBHttpBody* obj){
	NBHttpBody_empty(obj);
	if(obj->httpParser != NULL){
		NBHttpParser_release(obj->httpParser);
		NBMemory_free(obj->httpParser);
		obj->httpParser = NULL;
	}
	NBArray_release(&obj->chunks);
	NBArray_release(&obj->fields);
	NBString_release(&obj->strs);
	NBMemory_set(&obj->listnr, 0, sizeof(obj->listnr));
	obj->listnrParam		= NULL;
}

//

void NBHttpBody_emptyChuncksData(STNBHttpBody* obj){
	{
		UI32 i; const UI32 count = obj->chunks.use;
		for(i = 0 ; i < count; i++){
			STNBHttpBodyChunk* c = NBArray_itmPtrAtIndex(&obj->chunks, STNBHttpBodyChunk, i);
			//NBASSERT(c->data != NULL) //ToDo: remove.
			if(c->data != NULL){
				NBMemory_free(c->data);
				c->data = NULL;
			}
		}
	}
}
	
void NBHttpBody_empty(STNBHttpBody* obj){
	obj->isCompleted		= FALSE;
	obj->readBuffSz			= 4096;
	obj->storgBuffSz		= 4096;
	obj->mode				= ENNBHttpBodyMode_untillClose;
	obj->expctLength		= 0;
	obj->dataTotalSz		= 0;
	{
		NBHttpBody_emptyChuncksData(obj);
		NBArray_empty(&obj->chunks);
	}
	NBArray_empty(&obj->fields);
	NBString_empty(&obj->strs);
	NBString_concatByte(&obj->strs, '\0'); //Index 0 must be empty string
}

//Listener

void NBHttpBody_setListener(STNBHttpBody* obj, const IHttpBodyListener* listener, void* listenerParam){
	if(listener != NULL){
		obj->listnr	= *listener;
	} else {
		NBMemory_set(&obj->listnr, 0, sizeof(obj->listnr));
	}
	obj->listnrParam = listenerParam;
}

//Chunks

UI32 NBHttpBody_chunksTotalBytes(const STNBHttpBody* obj){
	UI32 r = 0;
	UI32 i; const UI32 count = obj->chunks.use;
	for(i = 0 ; i < count; i++){
		STNBHttpBodyChunk* c = NBArray_itmPtrAtIndex(&obj->chunks, STNBHttpBodyChunk, i);
		if(c->data != NULL){
			r += (UI32)c->use;
		}
	}
	return r;
}

void NBHttpBody_chunksConcatAll(const STNBHttpBody* obj, STNBString* dst){
	if(dst != NULL){
		UI32 i; const UI32 count = obj->chunks.use;
		for(i = 0 ; i < count; i++){
			STNBHttpBodyChunk* c = NBArray_itmPtrAtIndex(&obj->chunks, STNBHttpBodyChunk, i);
			if(c->data != NULL){
				NBString_concatBytes(dst, (const char*)c->data, (UI32)c->use);
			}
		}
	}
}

UI32 NBHttpBody_storeData(STNBHttpBody* obj, const void* pData, const UI32 dataSz){
	UI32 r = 0;
	const BYTE* data = (const BYTE*)pData;
	NBASSERT(obj->storgBuffSz > 0) //Optimization, do not call if storage is not enabled
	if(obj->storgBuffSz > 0 && dataSz > 0){
		//Create first chunk (if necesary)
		if(obj->chunks.use == 0){
			STNBHttpBodyChunk k;
			k.notif		= 0;
			k.use		= 0;
			k.size		= obj->storgBuffSz; NBASSERT(k.size > 0)
			k.data		= (BYTE*)NBMemory_alloc((UI32)k.size);
			NBArray_addValue(&obj->chunks, k);
			//PRINTF_INFO("NBHttpBody_storeData created first-chunk %llu bytes.\n", k.size);
		}
		//Add data
		while(r < dataSz){
			//Add to last chunk
			STNBHttpBodyChunk* last = NBArray_itmPtrAtIndex(&obj->chunks, STNBHttpBodyChunk, obj->chunks.use - 1);
			const UI32 consumeSz = ((last->use + (dataSz - r)) <= last->size ? (dataSz - r) : (UI32)(last->size - last->use));
			if(consumeSz > 0){
				//Copy data to chunk
				NBASSERT(last->data != NULL)
				NBMemory_copy(&last->data[last->use], &data[r], consumeSz);
				last->use			+= consumeSz; NBASSERT(last->use <= last->size)
				r					+= consumeSz;
				//Notify-data (of recently filled chunk)
				if(last->use >= last->size){
					NBASSERT(last->notif == 0)
					NBASSERT(last->use == last->size)
					if(obj->listnr.consumeData != NULL){
						if(!(*obj->listnr.consumeData)(obj, &last->data[last->notif], (last->use - last->notif), obj->listnrParam)){
							break;
						}
						last->notif = last->use;
					}
				}
			} else {
				//Create new chunk
				STNBHttpBodyChunk k;
				k.notif		= 0;
				k.use		= 0;
				k.size		= obj->storgBuffSz; NBASSERT(k.size > 0)
				k.data		= (BYTE*)NBMemory_alloc((UI32)k.size);
				NBArray_addValue(&obj->chunks, k);
				//PRINTF_INFO("NBHttpBody_storeData created chunk %llu bytes.\n", k.size);
			}
		}
		NBASSERT(r == dataSz)
	}
	return r;
}


//Header generic-fields

BOOL NBHttpBody_consumeFieldName_(const STNBHttpParser* obj, const STNBHttpParserNode* node, const char* name, const UI32 nameSz, void* listenerParam){
	BOOL r = FALSE;
	if(listenerParam != NULL){
		STNBHttpBody* obj = (STNBHttpBody*)listenerParam;
		NBASSERT(!obj->isCompleted && obj->mode == ENNBHttpBodyMode_chunked)
		if(!obj->isCompleted && obj->mode == ENNBHttpBodyMode_chunked){
			STNBHttpBodyField f;
			f.name	= obj->strs.length;
			NBString_concatBytes(&obj->strs, name, nameSz);
			NBString_concatByte(&obj->strs, '\0');
			f.value	= 0;
			NBArray_addValue(&obj->fields, f);
			//PRINTF_INFO("NBHttpBody_consumeFieldName_: '%s'\n", &obj->strs.str[f.name]);
			r = TRUE;
		}
	}
	return r;
}
BOOL NBHttpBody_consumeFieldLine_(const STNBHttpParser* obj, const STNBHttpParserNode* node, const char* name, const UI32 nameSz, const char* value, const UI32 valueSz, void* listenerParam){
	BOOL r = FALSE;
	if(listenerParam != NULL){
		STNBHttpBody* obj = (STNBHttpBody*)listenerParam;
		NBASSERT(!obj->isCompleted && obj->mode == ENNBHttpBodyMode_chunked)
		if(!obj->isCompleted && obj->mode == ENNBHttpBodyMode_chunked){
			//Complete prev added field
			if(obj->fields.use > 0){
				STNBHttpBodyField* last = NBArray_itmPtrAtIndex(&obj->fields, STNBHttpBodyField, obj->fields.use - 1);
				if(last->value == 0){
					if(NBString_strIsEqualStrBytes(&obj->strs.str[last->name], name, nameSz)){
						last->value = obj->strs.length;
						NBString_concatBytes(&obj->strs, value, valueSz);
						NBString_concatByte(&obj->strs, '\0');
						//PRINTF_INFO("NBHttpBody_consumeFieldLine_: '%s': '%s' (pre-added)\n", &obj->strs.str[last->name], &obj->strs.str[last->value]);
						r = TRUE;
						//Notify
						if(obj->listnr.consumeTrailerField != NULL){
							if(!(*obj->listnr.consumeTrailerField)(obj, last, obj->listnrParam)){
								r = FALSE;
							}
						}
					}
				}
			}
			//Add new field
			if(!r){
				STNBHttpBodyField f;
				f.name	= obj->strs.length;
				NBString_concatBytes(&obj->strs, name, nameSz);
				NBString_concatByte(&obj->strs, '\0');
				f.value	= obj->strs.length;
				NBString_concatBytes(&obj->strs, value, valueSz);
				NBString_concatByte(&obj->strs, '\0');
				NBArray_addValue(&obj->fields, f);
				//PRINTF_INFO("NBHttpBody_consumeFieldLine_: '%s': '%s' (new)\n", &obj->strs.str[f.name], &obj->strs.str[f.value]);
				r = TRUE;
				//Notify
				if(obj->listnr.consumeTrailerField != NULL){
					STNBHttpBodyField* last = NBArray_itmPtrAtIndex(&obj->fields, STNBHttpBodyField, obj->fields.use - 1);
					if(!(*obj->listnr.consumeTrailerField)(obj, last, obj->listnrParam)){
						r = FALSE;
					}
				}
			}
		}
	}
	return r;
}

//Chunked-body
BOOL NBHttpBody_chunkStarted_(const STNBHttpParser* obj, const STNBHttpParserNode* node, const UI64 chunkSz, void* listenerParam){
	BOOL r = FALSE;
	if(listenerParam != NULL){
		STNBHttpBody* obj = (STNBHttpBody*)listenerParam;
		NBASSERT(!obj->isCompleted && obj->mode == ENNBHttpBodyMode_chunked)
		if(!obj->isCompleted && obj->mode == ENNBHttpBodyMode_chunked){
			//PRINTF_INFO("NBHttpBody_chunkStarted_: %llu\n", chunkSz);
			r = TRUE;
		}
	}
	return r;
}

BOOL NBHttpBody_consumeChunkData_(const STNBHttpParser* obj, const STNBHttpParserNode* node, const char* data, const UI64 dataSz, const UI64 chunkReadPos, const UI64 chunkTotalSz, void* listenerParam){
	BOOL r = FALSE;
	if(listenerParam != NULL){
		STNBHttpBody* obj = (STNBHttpBody*)listenerParam;
		NBASSERT(!obj->isCompleted && obj->mode == ENNBHttpBodyMode_chunked)
		if(!obj->isCompleted && obj->mode == ENNBHttpBodyMode_chunked){
			if(obj->storgBuffSz > 0){
				//Feed to storage (will be notified inside)
				const UI32 csmd		= NBHttpBody_storeData(obj, data, (UI32)dataSz); NBASSERT(csmd == dataSz)
				obj->dataTotalSz	+= csmd;
				r = (csmd == dataSz ? TRUE : FALSE);
			} else {
				obj->dataTotalSz	+= dataSz;
				r = TRUE;
				//Just notify
				if(obj->listnr.consumeData != NULL){
					if(!(*obj->listnr.consumeData)(obj, data, dataSz, obj->listnrParam)){
						r = FALSE;
					}
				}
			}
		}
	}
	return r;
}

BOOL NBHttpBody_chunkEnded_(const STNBHttpParser* obj, const STNBHttpParserNode* node, const UI64 chunkTotalSz, void* listenerParam){
	BOOL r = FALSE;
	if(listenerParam != NULL){
		STNBHttpBody* obj = (STNBHttpBody*)listenerParam;
		NBASSERT(!obj->isCompleted && obj->mode == ENNBHttpBodyMode_chunked)
		if(!obj->isCompleted && obj->mode == ENNBHttpBodyMode_chunked){
			r = TRUE;
		}
	}
	return r;
}

BOOL NBHttpBody_consumeChunkedBodyEnd_(const struct STNBHttpParser_* obj, void* listenerParam){
	BOOL r = FALSE;
	if(listenerParam != NULL){
		STNBHttpBody* obj = (STNBHttpBody*)listenerParam;
		NBASSERT(!obj->isCompleted && obj->mode == ENNBHttpBodyMode_chunked)
		if(!obj->isCompleted && obj->mode == ENNBHttpBodyMode_chunked){
			//PRINTF_INFO("NBHttpBody_consumeChunkedBodyEnd_: %llu\n", chunkTotalSz);
			obj->isCompleted = TRUE;
			r = TRUE;
			//Notify-data (if necesary)
			{
				UI64 dataTotalSz = 0;
				UI32 i; for(i = 0; i < obj->chunks.use; i++){
					STNBHttpBodyChunk* c = NBArray_itmPtrAtIndex(&obj->chunks, STNBHttpBodyChunk, i);
					if(c->data != NULL && c->notif < c->use){
						if(obj->listnr.consumeData != NULL){
							if(!(*obj->listnr.consumeData)(obj, &c->data[c->notif], (c->use - c->notif), obj->listnrParam)){
								r = FALSE;
							}
							c->notif = c->use;
						}
					}
					dataTotalSz += c->use;
				}
				NBASSERT(obj->storgBuffSz == 0 || dataTotalSz == obj->dataTotalSz)
			}
			//Notify-end
			if(obj->listnr.consumeEnd != NULL){
				if(!(*obj->listnr.consumeEnd)(obj, obj->listnrParam)){
					r = FALSE;
				}
			}
		}
	}
	return r;
}

//Feed

void NBHttpBody_feedStartWithHeader(STNBHttpBody* obj, STNBHttpHeader* header, const UI32 readBuffSize, const UI32 storgBuffSz, const char* optReqMethod){
    NBASSERT(header->requestLine != NULL || header->statusLine != NULL)
    //
	if(header->requestLine != NULL){
        //reading request message
		/*
		https://tools.ietf.org/html/rfc7230
		The presence of a message body in a request is signaled by a
		Content-Length or Transfer-Encoding header field.  Request message
		framing is independent of method semantics, even if the method does
		not define any use for a message body.*/
		if(NBHttpHeader_isBodyChunked(header)){
			//PRINTF_INFO("feedStartChunked.\n");
			NBHttpBody_feedStartChunked(obj, readBuffSize, storgBuffSz);
		} else if(NBHttpHeader_isContentLength(header)){
			//PRINTF_INFO("feedStartWithContentLength(%llu).\n", (UI64)(*header->contentLength));
			NBHttpBody_feedStartWithContentLength(obj, NBHttpHeader_contentLength(header), readBuffSize, storgBuffSz);
		} else {
			//PRINTF_INFO("feedStartWithContentLength(implicit-empty).\n");
			NBHttpBody_feedStartWithContentLength(obj, 0, readBuffSize, storgBuffSz);
		}
	} else if(header->statusLine != NULL){
        //reading response message
		/*
		https://tools.ietf.org/html/rfc7230
		The presence of a message body in a response depends on both the
		request method to which it is responding and the response status code
		(Section 3.1.2).  Responses to the HEAD request method (Section 4.3.2
		of [RFC7231]) never include a message body because the associated
		response header fields (e.g., Transfer-Encoding, Content-Length,
		etc.), if present, indicate only what their values would have been if
		the request method had been GET (Section 4.3.1 of [RFC7231]). 2xx
		(Successful) responses to a CONNECT request method (Section 4.3.6 of
		[RFC7231]) switch to tunnel mode instead of having a message body.
		All 1xx (Informational), 204 (No Content), and 304 (Not Modified)
		responses do not include a message body.  All other responses do
		include a message body, although the body might be of zero length.
		*/
		BOOL wasDefined = FALSE;
        //define body-type by request-method
		if(optReqMethod != NULL){
			if(NBString_strIsEqual(optReqMethod, "HEAD")){
                //no-body expected
				NBHttpBody_feedStartWithContentLength(obj, 0, readBuffSize, storgBuffSz);
                wasDefined = TRUE;
			} else if(NBString_strIsEqual(optReqMethod, "CONNECT") && header->statusLine->statusCode >= 200 && header->statusLine->statusCode <= 299){
                //no-body expected
				NBHttpBody_feedStartWithContentLength(obj, 0, readBuffSize, storgBuffSz);
                wasDefined = TRUE;
			} else if((header->statusLine->statusCode >= 100 && header->statusLine->statusCode <= 199) || header->statusLine->statusCode == 204 || header->statusLine->statusCode == 304){
                //no-body expected
				NBHttpBody_feedStartWithContentLength(obj, 0, readBuffSize, storgBuffSz);
                wasDefined = TRUE;
			}
		}
        //define body-type by content-length (if present)
		if(!wasDefined){
			if(NBHttpHeader_isBodyChunked(header)){
                //read chunks untill last-chunk found
				NBHttpBody_feedStartChunked(obj, readBuffSize, storgBuffSz);
                wasDefined = TRUE;
			} else if(NBHttpHeader_isContentLength(header)){
                //read specific ammount of bytes
				NBHttpBody_feedStartWithContentLength(obj, NBHttpHeader_contentLength(header), readBuffSize, storgBuffSz);
                wasDefined = TRUE;
			} else {
                //default: read untill connection is closed
				NBHttpBody_feedStartUntillClose(obj, readBuffSize, storgBuffSz);
                wasDefined = TRUE;
			}
		}
	}
}

void NBHttpBody_feedSetStorageBuffSize(STNBHttpBody* obj, const UI32 storgBuffSz){
	obj->storgBuffSz = storgBuffSz;
}

void NBHttpBody_feedStartUntillClose(STNBHttpBody* obj, const UI32 readBuffSize, const UI32 storgBuffSz){
	//PRINTF_INFO("NBHttpBody_feedStart (untill close).\n");
	//Empty
	NBHttpBody_empty(obj);
	//Configure
	obj->mode			= ENNBHttpBodyMode_untillClose;
	obj->readBuffSz		= readBuffSize;
	obj->storgBuffSz	= storgBuffSz;
	if(obj->httpParser != NULL){
		NBHttpParser_release(obj->httpParser);
		NBMemory_free(obj->httpParser);
		obj->httpParser = NULL;
	}
}

void NBHttpBody_feedStartWithContentLength(STNBHttpBody* obj, const UI64 contentLength, const UI32 readBuffSize, const UI32 storgBuffSz){
	//PRINTF_INFO("NBHttpBody_feedStart (contentLength).\n");
	//Empty
	NBHttpBody_empty(obj);
	//Configure
	obj->mode			= ENNBHttpBodyMode_contentLength;
	obj->expctLength	= contentLength;
	obj->readBuffSz		= readBuffSize;
	obj->storgBuffSz	= storgBuffSz;
	if(obj->httpParser != NULL){
		NBHttpParser_release(obj->httpParser);
		NBMemory_free(obj->httpParser);
		obj->httpParser = NULL;
	}
	//Create first chunk (if necesary)
	if(obj->storgBuffSz > 0){
		if(obj->chunks.use == 0 && obj->expctLength > 0){
			STNBHttpBodyChunk k;
			k.notif		= 0;
			k.use		= 0;
			k.size		= obj->expctLength; NBASSERT(k.size > 0)
			k.data		= (BYTE*)NBMemory_alloc((UI32)k.size);
			NBArray_addValue(&obj->chunks, k);
			//PRINTF_INFO("NBHttpBody_feedStartWithContentLength created chunk %llu bytes.\n", k.size);
		}
	}
}

void NBHttpBody_feedStartChunked(STNBHttpBody* obj, const UI32 readBuffSize, const UI32 storgBuffSz){
	//PRINTF_INFO("NBHttpBody_feedStartChunked.\n");
	//Empty
	NBHttpBody_empty(obj);
	//Configure
	obj->mode			= ENNBHttpBodyMode_chunked;
	obj->readBuffSz		= readBuffSize;
	obj->storgBuffSz	= storgBuffSz;
	//Start parser
	{
		if(obj->httpParser != NULL){
			NBHttpParser_release(obj->httpParser);
		} else {
			obj->httpParser = (STNBHttpParser*)NBMemory_alloc(sizeof(STNBHttpParser));
		}
		IHttpParserListener listnr;
		NBMemory_set(&listnr, 0, sizeof(listnr));
		//Header generic-fields
		listnr.consumeFieldName			= NBHttpBody_consumeFieldName_;
		listnr.consumeFieldLine			= NBHttpBody_consumeFieldLine_;
		//Chunks
		listnr.chunkStarted				= NBHttpBody_chunkStarted_;
		listnr.consumeChunkData			= NBHttpBody_consumeChunkData_;
		listnr.chunkEnded				= NBHttpBody_chunkEnded_;
		//End of body
		listnr.consumeChunkedBodyEnd	=  NBHttpBody_consumeChunkedBodyEnd_;
		NBHttpParser_initWithListener(obj->httpParser, &listnr, obj);
		NBHttpParser_setDataBuffSz(obj->httpParser, obj->readBuffSz);
	}
	NBHttpParser_feedStartWithType(obj->httpParser, ENNBHttpParserType_chunkedBody);
}

BOOL NBHttpBody_feedByte(STNBHttpBody* obj, const char c){
	BOOL r = FALSE;
	switch (obj->mode) {
		case ENNBHttpBodyMode_untillClose:
			if(obj->storgBuffSz > 0){
				//Feed to storage (will be notified inside)
				const UI32 csmd = NBHttpBody_storeData(obj, &c, 1); NBASSERT(csmd == 1)
				obj->dataTotalSz += csmd;
				r = (csmd == 1 ? TRUE : FALSE);
			} else {
				obj->dataTotalSz++;
				r = TRUE;
				//Just notify
				if(obj->listnr.consumeData != NULL){
					if(!(*obj->listnr.consumeData)(obj, &c, 1, obj->listnrParam)){
						r = FALSE;
					}
				}
			}
			break;
		case ENNBHttpBodyMode_chunked:
			if(obj->httpParser != NULL){
				//Work done at interfaces
				r = NBHttpParser_feedByte(obj->httpParser, c);
			}
			break;
		case ENNBHttpBodyMode_contentLength:
			if((obj->dataTotalSz + 1) <= obj->expctLength){
				if(obj->storgBuffSz > 0){
					//Feed to storage (will be notified inside)
					const UI32 csmd = NBHttpBody_storeData(obj, &c, 1); NBASSERT(csmd == 1)
					obj->dataTotalSz += csmd;
					r = (csmd == 1 ? TRUE : FALSE);
				} else {
					obj->dataTotalSz++;
					r = TRUE;
					//Just notify
					if(obj->listnr.consumeData != NULL){
						if(!(*obj->listnr.consumeData)(obj, &c, 1, obj->listnrParam)){
							r = FALSE;
						}
					}
				}
			}
			break;
		default:
			NBASSERT(FALSE)
			break;
	}
	return r;
}

UI32 NBHttpBody_feed(STNBHttpBody* obj, const char* str){
	return NBHttpBody_feedBytes(obj, str, NBString_strLenBytes(str));
}

UI32 NBHttpBody_feedBytes(STNBHttpBody* obj, const void* pData, const UI32 dataSz){
	UI32 r = 0;
	const char* data = (const char*)pData;
	switch (obj->mode) {
		case ENNBHttpBodyMode_untillClose:
			if(obj->storgBuffSz > 0){
				//Feed to storage (will be notified inside)
				const UI32 csmd		= NBHttpBody_storeData(obj, data, dataSz); NBASSERT(csmd == dataSz)
				obj->dataTotalSz	+= csmd;
				r = csmd;
			} else {
				obj->dataTotalSz	+= dataSz;
				r = dataSz;
				//Just notify
				if(obj->listnr.consumeData != NULL){
					if(!(*obj->listnr.consumeData)(obj, data, dataSz, obj->listnrParam)){
						r = 0; NBASSERT(FALSE) //Tmp
					}
				}
			}
			break;
		case ENNBHttpBodyMode_chunked:
			if(obj->httpParser != NULL){
				//Work done at interfaces
				r = NBHttpParser_feedBytes(obj->httpParser, data, dataSz);
			}
			break;
		case ENNBHttpBodyMode_contentLength:
			{
				NBASSERT(obj->dataTotalSz <= obj->expctLength)
				const UI32 consumeSz = ((obj->dataTotalSz + dataSz) <= obj->expctLength ? dataSz : (UI32)(obj->expctLength - obj->dataTotalSz));
				//PRINTF_INFO("Consuming %d of %d, total %llu of %llu.\n", consumeSz, dataSz, obj->dataTotalSz, obj->expctLength);
				if(consumeSz > 0){
					if(obj->storgBuffSz > 0){
						//Feed to storage (will be notified inside)
						const UI32 csmd		= NBHttpBody_storeData(obj, data, consumeSz); NBASSERT(csmd == consumeSz)
						obj->dataTotalSz	+= csmd;
						r = csmd;
					} else {
						obj->dataTotalSz	+= consumeSz;
						r = consumeSz;
						//Just notify
						if(obj->listnr.consumeData != NULL){
							if(!(*obj->listnr.consumeData)(obj, data, consumeSz, obj->listnrParam)){
								r = 0; NBASSERT(FALSE) //Tmp
							}
						}
					}
					NBASSERT(obj->dataTotalSz <= obj->expctLength)
				}
			}
			break;
		default:
			NBASSERT(FALSE)
			break;
	}
	return r;
}

BOOL NBHttpBody_feedIsComplete(STNBHttpBody* obj){
	BOOL r = obj->isCompleted;
	if(!r){
		switch (obj->mode) {
			case ENNBHttpBodyMode_untillClose:
				r = FALSE; //always return FALSE
				break;
			case ENNBHttpBodyMode_chunked:
				//Work done at interfaces
				if(obj->httpParser != NULL){
					r = NBHttpParser_feedIsComplete(obj->httpParser);
					NBASSERT(r == obj->isCompleted)
				}
				break;
			case ENNBHttpBodyMode_contentLength:
				NBASSERT(obj->dataTotalSz <= obj->expctLength);
				r = (obj->dataTotalSz >= obj->expctLength ? TRUE : FALSE);
				break;
			default:
				NBASSERT(FALSE)
				break;
		}
	}
	return r;
}

BOOL NBHttpBody_feedEnd(STNBHttpBody* obj){
	BOOL r = FALSE;
	const BOOL wasCompleted = obj->isCompleted;
	switch (obj->mode) {
		case ENNBHttpBodyMode_untillClose:
			obj->isCompleted = TRUE;
			r = TRUE;
			break;
		case ENNBHttpBodyMode_chunked:
			//Work done at interfaces
			if(obj->httpParser != NULL){
				r = NBHttpParser_feedEnd(obj->httpParser);
				NBASSERT(r == obj->isCompleted)
				//Release memory
				NBHttpParser_release(obj->httpParser);
				NBMemory_free(obj->httpParser);
				obj->httpParser = NULL;
			} else {
				r = obj->isCompleted;
			}
			break;
		case ENNBHttpBodyMode_contentLength:
			NBASSERT(obj->dataTotalSz <= obj->expctLength);
			if(obj->dataTotalSz >= obj->expctLength){
				obj->isCompleted = TRUE;
				r = TRUE;
			}
			break;
		default:
			NBASSERT(FALSE)
			break;
	}
	//Notify
	if(!wasCompleted && obj->isCompleted){
		//Notify-data (if necesary)
		{
			UI64 dataTotalSz = 0;
			UI32 i; for(i = 0; i < obj->chunks.use; i++){
				STNBHttpBodyChunk* c = NBArray_itmPtrAtIndex(&obj->chunks, STNBHttpBodyChunk, i);
				if(c->data != NULL && c->notif < c->use){
					if(obj->listnr.consumeData != NULL){
						if(!(*obj->listnr.consumeData)(obj, &c->data[c->notif], (c->use - c->notif), obj->listnrParam)){
							r = FALSE;
						}
						c->notif = c->use;
					}
				}
				dataTotalSz += c->use;
			}
			NBASSERT(obj->storgBuffSz == 0 || dataTotalSz == obj->dataTotalSz)
		}
		//Notify-end
		if(obj->listnr.consumeEnd != NULL){
			if(!(*obj->listnr.consumeEnd)(obj, obj->listnrParam)){
				r = FALSE;
			}
		}
	}
	return r;
}

