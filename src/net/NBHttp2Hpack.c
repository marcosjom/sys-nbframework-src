
#include "nb/NBFrameworkPch.h"
#include "nb/net/NBHttp2Hpack.h"
//
#include "nb/core/NBStruct.h"

void NBHttp2Hpack_addRecordToTableEnd(STNBHttp2Hpack* obj, STNBArray* table, const char* name, const char* value);
	
void NBHttp2Hpack_init(STNBHttp2Hpack* obj){
	NBArray_initWithSz(&obj->tblStatic, sizeof(STNBHttp2HpackTableItm), NULL, 64, 64, 0.10f);
	NBArray_initWithSz(&obj->tblDynamic, sizeof(STNBHttp2HpackTableItm), NULL, 64, 64, 0.10f);
	NBString_initWithSz(&obj->strs, 4096, 4096, 0.10f);
	//Static table
	{
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "", ""); //Index zero
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, ":authority", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, ":method", "GET");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, ":method", "POST");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, ":path", "/");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, ":path", "/index.html");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, ":scheme", "http");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, ":scheme", "https");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, ":status", "200");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, ":status", "204");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, ":status", "206");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, ":status", "304");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, ":status", "400");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, ":status", "404");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, ":status", "500");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "accept-charset", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "accept-encoding", "gzip, deflate");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "accept-language", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "accept-ranges", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "accept", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "access-control-allow-origin", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "age", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "allow", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "authorization", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "cache-control", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "content-disposition", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "content-encoding", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "content-language", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "content-length", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "content-location", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "content-range", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "content-type", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "cookie", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "date", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "etag", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "expect", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "expires", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "from", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "host", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "if-match", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "if-modified-since", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "if-none-match", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "if-range", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "if-unmodified-since", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "last-modified", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "link", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "location", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "max-forwards", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "proxy-authenticate", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "proxy-authorization", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "range", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "referer", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "refresh", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "retry-after", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "server", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "set-cookie", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "strict-transport-security", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "transfer-encoding", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "user-agent", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "vary", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "via", "");
		NBHttp2Hpack_addRecordToTableEnd(obj, &obj->tblStatic, "www-authenticate", "");
		NBASSERT(obj->tblStatic.use == 62)
	}
}

void NBHttp2Hpack_release(STNBHttp2Hpack* obj){
	NBString_release(&obj->strs);
	NBArray_release(&obj->tblDynamic);
	NBArray_release(&obj->tblStatic);
}

UI32 NBHttp2Hpack_tablesTotalSizes(STNBHttp2Hpack* obj){
	return (obj->tblStatic.use + obj->tblDynamic.use);
}

void NBHttp2Hpack_addRecordToTableEnd(STNBHttp2Hpack* obj, STNBArray* table, const char* name, const char* value){
	STNBHttp2HpackTableItm itm;
	{
		itm.iName = obj->strs.length;
		if(!NBString_strIsEmpty(name)){
			NBString_concat(&obj->strs, name);
		}
		NBString_concatByte(&obj->strs, '\0');
	}
	{
		itm.iValue = obj->strs.length;
		if(!NBString_strIsEmpty(value)){
			NBString_concat(&obj->strs, value);
		}
		NBString_concatByte(&obj->strs, '\0');
	}
	NBArray_addValue(table, itm);
}

void NBHttp2Hpack_addRecordToTableStart(STNBHttp2Hpack* obj, STNBArray* table, const char* name, const char* value){
	STNBHttp2HpackTableItm itm;
	{
		itm.iName = obj->strs.length;
		if(!NBString_strIsEmpty(name)){
			NBString_concat(&obj->strs, name);
		}
		NBString_concatByte(&obj->strs, '\0');
	}
	{
		itm.iValue = obj->strs.length;
		if(!NBString_strIsEmpty(value)){
			NBString_concat(&obj->strs, value);
		}
		NBString_concatByte(&obj->strs, '\0');
	}
	NBArray_addItemsAtIndex(table, 0, &itm, sizeof(itm), 1);
}

STNBHttp2HpackTableItmVal NBHttp2Hpack_getItmValue(STNBHttp2Hpack* obj, const UI32 idx){
	STNBHttp2HpackTableItmVal r;
	NBMemory_setZeroSt(r, STNBHttp2HpackTableItmVal);
	if(idx >= (obj->tblStatic.use + obj->tblDynamic.use)){
		r.isError	= TRUE;
	} else if(idx >= obj->tblStatic.use){
		STNBHttp2HpackTableItm* itm = NBArray_itmPtrAtIndex(&obj->tblDynamic, STNBHttp2HpackTableItm, idx - obj->tblStatic.use);
		NBASSERT((idx - obj->tblStatic.use) >= 0 && (idx - obj->tblStatic.use) < obj->tblDynamic.use)
		NBASSERT(itm->iName >= 0 && itm->iName < obj->strs.length)
		NBASSERT(itm->iValue >= 0 && itm->iValue < obj->strs.length)
		r.name		= &obj->strs.str[itm->iName];
		r.value		= &obj->strs.str[itm->iValue];
	} else if(idx <= 0){
		r.isError	= TRUE;
	} else {
		NBASSERT(idx >= 0 && idx < obj->tblStatic.use)
		STNBHttp2HpackTableItm* itm = NBArray_itmPtrAtIndex(&obj->tblStatic, STNBHttp2HpackTableItm, idx);
		NBASSERT(itm->iName >= 0 && itm->iName < obj->strs.length)
		NBASSERT(itm->iValue >= 0 && itm->iValue < obj->strs.length)
		r.name		= &obj->strs.str[itm->iName];
		r.value		= &obj->strs.str[itm->iValue];
	}
	return r;
}

STNBHttp2HpackSrchResult NBHttp2Hpack_getPairIndexes(STNBHttp2Hpack* obj, const char* name, const char* value){
	STNBHttp2HpackSrchResult r;
	NBMemory_setZeroSt(r, STNBHttp2HpackSrchResult);
	//Search in static
	if(r.iValue <= 0){
		SI32 i; for(i = 0; i < obj->tblStatic.use && r.iValue <= 0; i++){
			const STNBHttp2HpackTableItm* itm = NBArray_itmPtrAtIndex(&obj->tblStatic, STNBHttp2HpackTableItm, i);
			const char* itmName = &obj->strs.str[itm->iName];
			if(NBString_strIsEqual(itmName, name)){
				const char* itmValue = &obj->strs.str[itm->iValue];
				r.iName = i;
				if(NBString_strIsEqual(itmValue, value)){
					r.iValue = i;
				}
			}
		}
	}
	//Search in dynamic
	if(r.iValue <= 0){
		SI32 i; for(i = 0; i < obj->tblDynamic.use && r.iValue <= 0; i++){
			const STNBHttp2HpackTableItm* itm = NBArray_itmPtrAtIndex(&obj->tblDynamic, STNBHttp2HpackTableItm, i);
			const char* itmName = &obj->strs.str[itm->iName];
			if(NBString_strIsEqual(itmName, name)){
				const char* itmValue = &obj->strs.str[itm->iValue];
				r.iName = obj->tblStatic.use + i;
				if(NBString_strIsEqual(itmValue, value)){
					r.iValue = obj->tblStatic.use + i;
				}
			}
		}
	}
	return r;
}

//

BOOL NBHttp2Hpack_decodeBlock(STNBHttp2Hpack* obj, const void* pData, const UI32 dataSz, STNBHttpHeader* dst){
	BOOL r = TRUE;
	BOOL nameIsHuffman = FALSE, valueIsHuffman = FALSE;
	STNBString name, value;
	NBString_init(&name);
	NBString_init(&value);
	{
		const BYTE* data = (BYTE*)pData;
		UI32 iPos = 0;
		while(iPos < dataSz){
			const BOOL isIndex = ((data[iPos] & 0x80) != 0);
			//PRINTF_INFO("Pair at pos(%d) byteVal(%d).\n", iPos, data[iPos]);
			if(isIndex){
				UI64 idxVal = 0;
				if(!NBHttp2Hpack_decodeInteger(7, &iPos, pData, dataSz, &idxVal)){
					PRINTF_ERROR("Hpack, could not decode idx value\n");
					r = FALSE; NBASSERT(FALSE)
					break;
				} else if(idxVal <= 0){
					PRINTF_ERROR("Hpack, idx(0) is not allowed\n");
					r = FALSE; NBASSERT(FALSE)
					break;
				} else {
					const STNBHttp2HpackTableItmVal vals = NBHttp2Hpack_getItmValue(obj, (const UI32)idxVal);
					if(vals.isError){
						PRINTF_ERROR("Hpack, idx(%llu) not found\n", idxVal);
						r = FALSE; NBASSERT(FALSE)
						break;
					} else {
						//PRINTF_INFO("Hpack, field decoded to index(%llu): '%s' : '%s'\n", idxVal, vals.name, vals.value);
						if(dst != NULL){
							NBHttpHeader_addField(dst, vals.name, vals.value);
						}
					}
				}
			} else {
				const BOOL isIncIndexing = ((data[iPos] & 0x40) != 0);		// | 0 | 1 |      Index (6+)       |
				const BOOL isNotIndexing = ((data[iPos] & 0xF0) == 0);		//| 0 | 0 | 0 | 0 |  Index (4+)   |
				const BOOL isNeverIndxng = ((data[iPos] & 0xF0) == 0x10);	//| 0 | 0 | 0 | 1 |  Index (4+)   |
				const BOOL isTblSzUpdate = ((data[iPos] & 0xF0) == 0x20);	//| 0 | 0 | 1 |   Max size (5+)   |
				if(!(isIncIndexing || isNotIndexing || isNeverIndxng)){
					PRINTF_ERROR("Hpack, unexpected bits header pattern.\n");
					r = FALSE; NBASSERT(FALSE)
					break;
				} else {
					UI64 idxVal = 0;
					if(!NBHttp2Hpack_decodeInteger((isNotIndexing || isNeverIndxng ? 4 : isIncIndexing ? 6 : 5), &iPos, pData, dataSz, &idxVal)){
						PRINTF_ERROR("Hpack, could not decode idx for string\n");
						r = FALSE; NBASSERT(FALSE)
						break;
					} else {
						if(isTblSzUpdate){
							PRINTF_INFO("Hpack, Dynamic Table Size Update(%llu of %d).\n", idxVal, obj->tblDynamic.use);
							NBASSERT(FALSE) //ToDo: implement
						} else {
							//Decode name
							if(idxVal > 0){
								const STNBHttp2HpackTableItmVal vals = NBHttp2Hpack_getItmValue(obj, (const UI32)idxVal);
								if(vals.isError){
									PRINTF_ERROR("Hpack, idx(%llu) for field-name not found\n", idxVal);
									r = FALSE; NBASSERT(FALSE)
									break;
								} else {
									//PRINTF_INFO("Hpack, idx(%llu) for field-name = '%s': '%s'\n", idxVal, vals.name, vals.value);
									NBString_set(&name, vals.name);
								}
							} else {
								nameIsHuffman = FALSE;
								if(!NBHttp2Hpack_decodeStrLiteral(&iPos, pData, dataSz, &name, &nameIsHuffman)){
									PRINTF_ERROR("Hpack, field-name could not be decoded\n");
									r = FALSE; NBASSERT(FALSE)
									break;
								} else {
									PRINTF_INFO("Hpack, literal field-name: '%s'\n", (nameIsHuffman ? "[huffman]" : name.str));
								}
							}
							//Decode value
							{
								valueIsHuffman = FALSE;
								if(!NBHttp2Hpack_decodeStrLiteral(&iPos, pData, dataSz, &value, &valueIsHuffman)){
									PRINTF_ERROR("Hpack, field-value could not be decoded\n");
									r = FALSE; NBASSERT(FALSE)
									break;
								} else {
									PRINTF_INFO("Hpack, literal field-value: '%s'\n", (valueIsHuffman ? "[huffman]" : value.str));
									if(dst != NULL){
										NBHttpHeader_addField(dst, name.str, value.str);
									}
								}
							}
							//Add to dynamic table
							//PRINTF_INFO("Hpack, field decoded: '%s'%s : '%s'\n", (nameIsHuffman ? "[huffman]" : name.str), (idxVal > 0 ? " (from idx)" : ""), (valueIsHuffman ? "[huffman]" : value.str));
							if(isIncIndexing){
								NBHttp2Hpack_addRecordToTableStart(obj, &obj->tblDynamic, name.str, value.str);
							}
						}
					}
				}
			}
		}
	}
	NBString_release(&value);
	NBString_release(&name);
	return r;
}

BOOL NBHttp2Hpack_encodeBlock(STNBHttp2Hpack* obj, const STNBHttpHeader* pairs, STNBString* dst){
	BOOL r = TRUE;
	if(pairs == NULL){
		r = FALSE;
	} else {
		SI32 i; for(i = 0; i < pairs->fields.use; i++){
			const STNBHttpHeaderField* f = NBArray_itmPtrAtIndex(&pairs->fields, STNBHttpHeaderField, i);
			const char* name = &pairs->strs.str[f->name];
			const char* value = &pairs->strs.str[f->value];
			if(!NBHttp2Hpack_encodePair(obj, name, value, FALSE, FALSE, dst)){
				PRINTF_ERROR("NBHttp2Hpack_encodePair, failed for '%s': '%s'.\n", name, value);
				r = FALSE; NBASSERT(FALSE);
				break;
			}
		}
	}
	return r;
}

BOOL NBHttp2Hpack_encodePair(STNBHttp2Hpack* obj, const char* name, const char* value, const BOOL useTables, const BOOL allowTableStored, STNBString* dst){
	BOOL r = TRUE;
	STNBHttp2HpackSrchResult srch;
	NBMemory_setZeroSt(srch, STNBHttp2HpackSrchResult);
	//Search indexes
	if(useTables){
		srch = NBHttp2Hpack_getPairIndexes(obj, name, value);
	}
	if(srch.iValue > 0){
		NBASSERT(srch.iName == srch.iValue)
		//Add index only
		const UI32 lenStart = dst->length;
		if(!NBHttp2Hpack_encodeInteger(7, srch.iValue, dst)){
			r = FALSE; NBASSERT(FALSE);
		} else {
			//Activate bit
			NBASSERT((dst->str[lenStart] & 0x80) == 0)
			dst->str[lenStart] = dst->str[lenStart] | 0x80;
		}
	} else {
		UI8 prefixSz	= 0;
		UI32 prefixBits	= 0x00;
		UI8 prefixMask	= 0xFF;
		if(!allowTableStored){
			prefixSz	= 4;
			prefixBits	= 0x10;
			prefixMask	= 0xF;
		} else if(!useTables){
			prefixSz	= 4;
			prefixBits	= 0x0;
			prefixMask	= 0xF;
		} else {
			prefixSz	= 6;
			prefixBits	= 0x40;
			prefixMask	= 0x3F;
		}
		//Add name index (zero is allowed)
		if(r){
			const UI32 lenStart = dst->length;
			if(!NBHttp2Hpack_encodeInteger(prefixSz, srch.iName, dst)){
				r = FALSE; NBASSERT(FALSE);
			} else {
				NBASSERT((dst->str[lenStart] | prefixMask) == prefixMask) //all other bits must be zero
				dst->str[lenStart] = (dst->str[lenStart] | prefixBits); //activate bits
				//PRINTF_INFO("Name '%s' from idx(%d) prefixSz(%d).\n", name, srch.iName, prefixSz);
/*#				ifdef NB_CONFIG_INCLUDE_ASSERTS
				{
					UI32 iPos = lenStart; UI64 verifIdx = 0;
					NBASSERT(NBHttp2Hpack_decodeInteger(prefixSz, &iPos, dst->str, dst->length, &verifIdx))
					NBASSERT(verifIdx == srch.iName)
				}
#				endif*/
			}
		}
		//Add name (if no index)
		if(r){
			if(srch.iName <= 0){
				if(!NBHttp2Hpack_encodeStrLiteral(name, NBString_strLenBytes(name), FALSE, dst)){
					r = FALSE; NBASSERT(FALSE);
				}
			}
		}
		//Add value
		if(r){
			if(!NBHttp2Hpack_encodeStrLiteral(value, NBString_strLenBytes(value), FALSE, dst)){
				r = FALSE; NBASSERT(FALSE);
			}
		}
		//Add to dynamic
		if(r && useTables && allowTableStored){
			NBHttp2Hpack_addRecordToTableStart(obj, &obj->tblDynamic, name, value);
		}
	}
	return r;
}

//Primitive int


/*
 https://www.rfc-editor.org/rfc/rfc7541.txt
 encode I from the next N bits
 if I < 2^N - 1, encode I on N bits
 else
 encode (2^N - 1) on N bits
 I = I - (2^N - 1)
 while I >= 128
 encode (I % 128 + 128) on 8 bits
 I = I / 128
 encode I on 8 bits
 */

BOOL NBHttp2Hpack_encodeInteger(const UI8 prefixN, const UI64 pValue, STNBString* dst){
	BOOL r = TRUE;
	if(prefixN < 1 || prefixN > 8){
		NBASSERT(FALSE)
		r = FALSE; //decode error (N must be 1-8)
	} else {
		if(pValue < (0xFF >> (8 - prefixN))){
			const UI8 byte = (UI8)pValue & (0xFF >> (8 - prefixN));
			NBString_concatBytes(dst, (const char*)&byte, 1);
		} else {
			UI64 value	= pValue;
			UI8 byte	= (0xFF >> (8 - prefixN));
			NBString_concatBytes(dst, (const char*)&byte, 1);
			value		-= byte;
			while(value >= 128){
				byte = (value % 128 + 128);
				NBString_concatBytes(dst, (const char*)&byte, 1);
				value /= 128;
			}
			byte = (UI8)value;
			NBString_concatBytes(dst, (const char*)&byte, 1);
		}
	}
	return r;
}

/*
 https://www.rfc-editor.org/rfc/rfc7541.txt
 decode I from the next N bits
 if I < 2^N - 1, return I
 else
 M = 0
 repeat
 B = next octet
 I = I + (B & 127) * 2^M
 M = M + 7
 while B & 128 == 128
 return I
 */

BOOL NBHttp2Hpack_decodeInteger(const UI8 prefixN, UI32* iPos, const void* pData, const UI32 dataSz, UI64* dst){
	BOOL r = TRUE;
	if(*iPos >= dataSz){
		NBASSERT(FALSE)
		r = FALSE; //decode error (no octet available)
	} else if(prefixN < 1 || prefixN > 8){
		NBASSERT(FALSE)
		r = FALSE; //decode error (N must be 1-8)
	} else {
		const BYTE* data = (BYTE*)pData;
		//const UI32 iPosStart = *iPos;
		UI32 iPos2 = *iPos;
		//decode I from the next N bits
		UI64 I = (data[iPos2++] & (0xFF >> (8 - prefixN)));
		if(I < (0xFF >> (8 - prefixN))){
			//PRINTF_INFO("Hpack, Integer one-octet: %llu.\n", I);
		} else {
			BYTE B = 0;
			UI64 M = 0;
			do {
				if(iPos2 >= dataSz || M > (64 - 7)){
					NBASSERT(FALSE)
					r = FALSE; //decode error (no octet available or value wont fit on UI64 var)
					break;
				}
				B = data[iPos2++];;
				I = I + ((UI64)(B & 127) << M);
				M += 7;
			} while((B & 128) == 128);
			//PRINTF_INFO("Hpack, Integer %d-octets: %llu.\n", (iPos2 - iPosStart), I);
		}
		if(dst != NULL) *dst = I;
		*iPos = iPos2;
	}
	return r;
}


//Primitive string

/*
 https://www.rfc-editor.org/rfc/rfc7541.txt
 0   1   2   3   4   5   6   7
 +---+---+---+---+---+---+---+---+
 | H |    String Length (7+)     |
 +---+---------------------------+
 |  String Data (Length octets)  |
 +-------------------------------+
 */

BOOL NBHttp2Hpack_encodeStrLiteral(const void* str, const UI32 strLen, const BOOL doHuffman, STNBString* dst){
	BOOL r = TRUE;
	NBASSERT(!doHuffman)
	if(doHuffman){
		PRINTF_ERROR("Hpack, huffman not implemented yet.\n");
		r = FALSE; NBASSERT(FALSE)
	} else {
		const UI32 iLenStart = dst->length;
		if(!NBHttp2Hpack_encodeInteger(7, strLen, dst)){
			PRINTF_ERROR("Hpack, could not encode string header.\n");
			r = FALSE; NBASSERT(FALSE)
		} else {
			//Apply first bit mask
			dst->str[iLenStart] = (dst->str[iLenStart] & ~0x80) | (doHuffman ? 0x80 : 0x00);
			//Concat data
			NBString_concatBytes(dst, str, strLen);
		}
	}
	return r;
}

BOOL NBHttp2Hpack_decodeStrLiteral(UI32* iPos, const void* pData, const UI32 dataSz, STNBString* dstData, BOOL* dstIsHuffman){
	BOOL r = TRUE;
	if(*iPos >= dataSz){
		r = FALSE; //decode error (no octet available)
	} else {
		const BYTE* data = (BYTE*)pData;
		const BOOL isHuffman = ((data[*iPos] & 0x80) != 0);
		UI64 strLen = 0;
		if(!NBHttp2Hpack_decodeInteger(7, iPos, pData, dataSz, &strLen)){
			PRINTF_ERROR("Hpack, could not decode string length.\n");
			r = FALSE; NBASSERT(FALSE)
		} else if((dataSz - *iPos) < (UI32)strLen){
			PRINTF_ERROR("Hpack, string length is greather than available octets.\n");
			r = FALSE; NBASSERT(FALSE)
		} else {
			if(dstIsHuffman != NULL){
				*dstIsHuffman = isHuffman;
			}
			if(dstData != NULL){
				NBString_setBytes(dstData, (const char*)&data[*iPos], (const UI32)strLen);
				//PRINTF_INFO("Hpack, string(%llu bytes, %s) loaded: '%s'.\n", strLen, (isHuffman ? "huffman" : "raw"), &dstData->str[dstData->length - (const UI32)strLen]);
			} else {
				//PRINTF_INFO("Hpack, string(%llu bytes, %s) loaded: '%s'.\n", strLen, (isHuffman ? "huffman" : "raw"), "...");
			}
			*iPos += (UI32)strLen;
		}
	}
	return r;
}
