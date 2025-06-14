//
//  XUCadena.c
//  MetricaMobilFramework
//
//  Created by Marcos Ortega on 07/10/13.
//  Copyright (c) 2013 logicoim@gmail.com. All rights reserved.
//

#include "nb/NBFrameworkPch.h"
#include "nb/core/NBString.h"
#include "nb/core/NBMemory.h"
#include "nb/core/NBEncoding.h"

#define NB_STRING_VERIF_START					638
#define NB_STRING_VERIF_END						837
//
#define NB_STRING_DEFAULT_GROWTH_MIN			128
#define NB_STRING_DEFAULT_GROWTH_REL_EXTRA		0.5		//After 1.0f
#define NB_STRING_DEFAULT_GROWTH_REL_PRECISION	16		//growthRel is stored in fixed precision

//

BOOL NBCompare_STNBString(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
	NBASSERT(dataSz == sizeof(STNBString))
	if(dataSz == sizeof(STNBString)){
		const STNBString* d1 = (const STNBString*)data1;
		const STNBString* d2 = (const STNBString*)data2;
		switch (mode) {
			case ENCompareMode_Equal:
				return NBString_isEqual(d1, d2->str);
			case ENCompareMode_Lower:
				return NBString_isLower(d1, d2->str);
			case ENCompareMode_LowerOrEqual:
				return NBString_isLowerOrEqual(d1, d2->str);
			case ENCompareMode_Greater:
				return NBString_isGreater(d1, d2->str);
			case ENCompareMode_GreaterOrEqual:
				return NBString_isGreaterOrEqual(d1, d2->str);
			default:
				NBASSERT(FALSE)
				break;
		}
	}
	return FALSE;
}

BOOL NBCompare_charPtr(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
	NBASSERT(dataSz == sizeof(char*))
	if(dataSz == sizeof(char*)){
		const char** d1 = (const char**)data1;
		const char** d2 = (const char**)data2;
		switch (mode) {
			case ENCompareMode_Equal:
				return NBString_strIsEqual(*d1, *d2);
			case ENCompareMode_Lower:
				return NBString_strIsLower(*d1, *d2);
			case ENCompareMode_LowerOrEqual:
				return NBString_strIsLowerOrEqual(*d1, *d2);
			case ENCompareMode_Greater:
				return NBString_strIsGreater(*d1, *d2);
			case ENCompareMode_GreaterOrEqual:
				return NBString_strIsGreaterOrEqual(*d1, *d2);
			default:
				NBASSERT(FALSE)
				break;
		}
	}
	return FALSE;
}

//Factory

static char _nbString_nullChar = '\0';

void NBString_init(STNBString* obj){
	obj->str			= &_nbString_nullChar;
	obj->length			= 0;
	//
	obj->_buffSz		= 1;
	obj->_buffGrowthMin	= NB_STRING_DEFAULT_GROWTH_MIN; NBASSERT(obj->_buffGrowthMin > 0)
	obj->_buffGrowthRel	= NB_STRING_DEFAULT_GROWTH_REL_EXTRA * NB_STRING_DEFAULT_GROWTH_REL_PRECISION; NBASSERT(obj->_buffGrowthRel >= 0.0f)
}

void NBString_initWithSz(STNBString* obj, const UI32 sz, const SI32 growthMin, const float growthRelExtra /*after 1.0*/){
	if(sz <= 1){
		obj->str		= &_nbString_nullChar;
		obj->length		= 0;
		obj->_buffSz	= 1;
	} else {
		obj->str		= (char*)NBMemory_alloc(sizeof(char) * sz);
		obj->length		= 0;
		obj->_buffSz	= sz;
		obj->str[0]		= '\0';
	}
	obj->_buffGrowthMin	= (growthMin < 1 ? 1 : growthMin); NBASSERT(obj->_buffGrowthMin > 0)
	obj->_buffGrowthRel	= (growthRelExtra < 0.0f ? 0.0f : growthRelExtra) * NB_STRING_DEFAULT_GROWTH_REL_PRECISION; NBASSERT(obj->_buffGrowthRel >= 0.0f)
}

void NBString_initWithOther(STNBString* obj, const STNBString* other){
	if(other->length == 0){
		obj->str		= &_nbString_nullChar;
		obj->length		= 0;
		obj->_buffSz	= 1;
	} else {
		obj->str		= (char*)NBMemory_alloc(sizeof(char) * (other->length + 1));
		obj->length		= other->length;
		obj->_buffSz	= other->length + 1;
		NBMemory_copy(obj->str, other->str, (other->length + 1));
	}
	obj->_buffGrowthMin	= other->_buffGrowthMin; NBASSERT(obj->_buffGrowthMin > 0)
	obj->_buffGrowthRel	= other->_buffGrowthRel; NBASSERT(obj->_buffGrowthRel >= 0.0f)
}

void NBString_initWithStringAndStr(STNBString* obj, const STNBString* other, const char* str){
	if(other->length == 0 && (str == NULL || str[0] == '\0')){
		obj->str		= &_nbString_nullChar;
		obj->length		= 0;
		obj->_buffSz	= 1;
	} else {
		const UI32 strSz = NBString_strLenBytes(str);
		obj->str		= (char*)NBMemory_alloc(sizeof(char) * (other->length + strSz + 1));
		obj->length		= other->length + strSz;
		obj->_buffSz	= other->length + strSz + 1;
		NBMemory_copy(obj->str, other->str, other->length);
		NBMemory_copy(&obj->str[other->length], str, strSz + 1);
	}
	obj->_buffGrowthMin	= other->_buffGrowthMin; NBASSERT(obj->_buffGrowthMin > 0)
	obj->_buffGrowthRel	= other->_buffGrowthRel; NBASSERT(obj->_buffGrowthRel >= 0.0f)
}

void NBString_initWithStr(STNBString* obj, const char* str){
	if(str == NULL){
		obj->str		= &_nbString_nullChar;
		obj->length		= 0;
		obj->_buffSz	= 1;
	} else if(str[0] == '\0'){
		obj->str		= &_nbString_nullChar;
		obj->length		= 0;
		obj->_buffSz	= 1;
	} else {
		const UI32 len	= NBString_strLenBytes(str);
		obj->str		= (char*)NBMemory_alloc(sizeof(char) * (len + 1));
		obj->length		= len;
		obj->_buffSz	= len + 1;
		NBMemory_copy(obj->str, str, (len + 1));
		NBASSERT(obj->str[len] == '\0');
	}
	obj->_buffGrowthMin	= NB_STRING_DEFAULT_GROWTH_MIN; NBASSERT(obj->_buffGrowthMin > 0)
	obj->_buffGrowthRel	= NB_STRING_DEFAULT_GROWTH_REL_EXTRA * NB_STRING_DEFAULT_GROWTH_REL_PRECISION; NBASSERT(obj->_buffGrowthRel >= 0.0f)
}

void NBString_initWithStrs(STNBString* obj, const char** arr, const UI32 arrSz){
	if(arr == NULL || arrSz == 0){
		NBString_init(obj);
	} else {
		UI32* lens = NBMemory_allocTypes(UI32, arrSz);
		UI32 i, totalLen = 0;
		//Get lengths
		for(i = 0; i < arrSz; i++){
			if(arr[i] == NULL){
				lens[i] = 0;
			} else {
				lens[i] = NBString_strLenBytes(arr[i]);
			}
			totalLen += lens[i];
		}
		//Create buffer
		if(totalLen == 0){
			obj->str		= &_nbString_nullChar;
			obj->length		= 0;
			obj->_buffSz	= 1;
		} else {
			obj->str		= (char*)NBMemory_alloc(sizeof(char) * (totalLen + 1));
			obj->length		= totalLen;
			obj->_buffSz	= totalLen + 1;
			//Copy content
			totalLen = 0;
			for(i = 0; i < arrSz; i++){
				if(lens[i] != 0){
					NBMemory_copy(&obj->str[totalLen], arr[i], lens[i]);
					totalLen += lens[i];
				}
			}
			obj->str[totalLen] = '\0';
			NBASSERT(totalLen == obj->length)
		}
		obj->_buffGrowthMin	= NB_STRING_DEFAULT_GROWTH_MIN; NBASSERT(obj->_buffGrowthMin > 0)
		obj->_buffGrowthRel	= NB_STRING_DEFAULT_GROWTH_REL_EXTRA * NB_STRING_DEFAULT_GROWTH_REL_PRECISION; NBASSERT(obj->_buffGrowthRel >= 0.0f)
		NBMemory_free(lens);
	}
}

void NBString_initWithStrsAndNull(STNBString* obj, const char** arrAndNull){
	if(arrAndNull == NULL){
		NBString_init(obj);
	} else if(arrAndNull[0] == NULL){
		NBString_init(obj);
	} else {
		UI32 i = 0;
		while(arrAndNull[i] != NULL){
			i++;
		}
		NBString_initWithStrs(obj, arrAndNull, i);
	}
}

void NBString_initWithStrBytes(STNBString* obj, const char* str, const UI32 strSz){
	if(strSz == 0){
		obj->str		= &_nbString_nullChar;
		obj->length		= 0;
		obj->_buffSz	= 1;
	} else {
		obj->str		= (char*)NBMemory_alloc(sizeof(char) * (strSz + 1));
		obj->length		= strSz;
		obj->_buffSz	= strSz + 1;
		NBMemory_copy(obj->str, str, strSz);
		obj->str[strSz]	= '\0';
	}
	obj->_buffGrowthMin	= NB_STRING_DEFAULT_GROWTH_MIN; NBASSERT(obj->_buffGrowthMin > 0)
	obj->_buffGrowthRel	= NB_STRING_DEFAULT_GROWTH_REL_EXTRA * NB_STRING_DEFAULT_GROWTH_REL_PRECISION; NBASSERT(obj->_buffGrowthRel >= 0.0f)
}

void NBString_release(STNBString* obj){
	if(obj->str != NULL){
		if(obj->str != &_nbString_nullChar){
    		NBMemory_free(obj->str);
		}
		obj->str		= NULL;
	}
	obj->length			= 0;
	obj->_buffSz		= 0;
	obj->_buffGrowthMin	= 0;
}

//

UI32 NBString_getBufferGrowthMin(STNBString* string){
    return string->_buffGrowthMin;
}

void NBString_increaseBuffer(STNBString* string, const UI32 additionalMinimunReq){
	string->_buffSz		+= (string->_buffGrowthMin < additionalMinimunReq ? additionalMinimunReq : string->_buffGrowthMin);
	char* newBuffer		= (char*) NBMemory_alloc(sizeof(char) * string->_buffSz);
	if(string->str != NULL){
		NBMemory_copy(newBuffer, string->str, string->length + 1);
		if(string->str != &_nbString_nullChar){
			NBMemory_free(string->str);
		}
	}
	string->str = newBuffer;
}

void NBString_resignToContent(STNBString* string){ //Leaves the buffer orphan
	string->str		= &_nbString_nullChar;
	string->length	= 0;
	string->_buffSz	= 1;
}

BOOL NBString_swapContent(STNBString* string, STNBString* string2){
	BOOL r = FALSE;
	if(string != NULL && string2 != NULL){
		STNBString tmp		= *string;
		//
		string->str			= string2->str;
		string->length		= string2->length;
		string->_buffSz		= string2->_buffSz;
		//
		string2->str		= tmp.str;
		string2->length		= tmp.length;
		string2->_buffSz	= tmp._buffSz;
		//
		r = TRUE;
	}
	return r;
}

BOOL NBString_swapContentBytes(STNBString* string, char* buff, const UI32 buffUse, const UI32 buffSz){
	BOOL r = FALSE;
	if(buff != NULL && buffUse <= buffSz){
		if(string->str != NULL && string->str != &_nbString_nullChar){
			NBMemory_free(string->str);
		}
		string->str		= buff;
		string->length	= buffUse;
		string->_buffSz	= buffSz;
		//
		r = TRUE;
	}
	return r;
}

void NBString_empty(STNBString* string){
	string->length	= 0;
	string->str[0]	= '\0';
}

BOOL NBString_truncate(STNBString* string, const UI32 strLen){
    BOOL r = FALSE;
    if(string->_buffSz > strLen){
        string->length      = strLen;
        string->str[strLen] = '\0';
        r = TRUE;
    }
    return r;
}

void NBString_set(STNBString* string, const char* str){
	//empty
	string->length	= 0;
	string->str[0]	= '\0';
	//concat
	NBString_concat(string, str);
}

void NBString_setByte(STNBString* string, const char c){
	//empty
	string->length	= 0;
	string->str[0]	= '\0';
	//concat
	NBString_concatByte(string, c);
}

void NBString_setBytes(STNBString* string, const char* str, const UI32 strSz){
	//empty
	string->length	= 0;
	string->str[0]	= '\0';
	//concat
	NBString_concatBytes(string, str, strSz);
}

UI32 NBString_replace(STNBString* string, const char* strFind, const char* strReplace){
	UI32 r = 0;
	const UI32 strFindSz = NBString_strLenBytes(strFind);
	const UI32 strReplaceSz = NBString_strLenBytes(strReplace);
	if(strFindSz > 0){
		STNBString strTmp;
		const char* strOrig = string->str;
		SI32 posBef = -1, posCur = 0;
		do {
			posCur = NBString_strIndexOf(strOrig, strFind, (posBef < 0 ? 0 : posBef + strFindSz));
			if(posCur >= 0){
				if(posBef < 0){
					//first find
					NBASSERT(r == 0)
					NBString_initWithSz(&strTmp, string->length, string->_buffGrowthMin, (float)string->_buffGrowthRel / (float)NB_STRING_DEFAULT_GROWTH_REL_PRECISION);
					NBString_concatBytes(&strTmp, strOrig, posCur);
				} else {
					//not first find
					NBString_concatBytes(&strTmp, &strOrig[posBef + strFindSz], posCur - (posBef + strFindSz));
				}
                if(strReplaceSz > 0){
                    NBString_concatBytes(&strTmp, strReplace, strReplaceSz);
                }
				posBef = posCur;
				r++;
			}
		} while(posCur >= 0);
		//restante de la cadena original
		NBASSERT(posBef >= 0 || r == 0)
		if(posBef >= 0){
			posCur = string->length;
			NBString_concatBytes(&strTmp, &strOrig[posBef + strFindSz], posCur - (posBef + strFindSz));
			NBString_swapContent(string, &strTmp);
			NBString_release(&strTmp);
		}
	}
	return r;
}

UI32 NBString_replaceByte(STNBString* string, const char cFind, const char cReplace){
	UI32 r = 0;
	char* str = string->str;
	if(str != NULL){
		while((*str) != '\0'){
			if((*str) == cFind){
				*str = cReplace;
				r++;
			}
			str++;
		}
	}
	return r;
}

UI32 NBString_replaceSI32(STNBString* string, const char* strFind, const SI32 strReplace){
	UI32 r = 0;
	STNBString tmp;
	NBString_initWithSz(&tmp, 32, 32, 0.10f);
	NBString_concatSI32(&tmp, strReplace);
	r = NBString_replace(string, strFind, tmp.str);
	NBString_release(&tmp);
	return r;
}

UI32 NBString_replaceUI32(STNBString* string, const char* strFind, const UI32 strReplace){
	UI32 r = 0;
	STNBString tmp;
	NBString_initWithSz(&tmp, 32, 32, 0.10f);
	NBString_concatUI32(&tmp, strReplace);
	r = NBString_replace(string, strFind, tmp.str);
	NBString_release(&tmp);
	return r;
}

UI32 NBString_replaceSI64(STNBString* string, const char* strFind, const SI64 strReplace){
	UI32 r = 0;
	STNBString tmp;
	NBString_initWithSz(&tmp, 32, 32, 0.10f);
	NBString_concatSI64(&tmp, strReplace);
	r = NBString_replace(string, strFind, tmp.str);
	NBString_release(&tmp);
	return r;
}

UI32 NBString_replaceUI64(STNBString* string, const char* strFind, const UI64 strReplace){
	UI32 r = 0;
	STNBString tmp;
	NBString_initWithSz(&tmp, 32, 32, 0.10f);
	NBString_concatUI64(&tmp, strReplace);
	r = NBString_replace(string, strFind, tmp.str);
	NBString_release(&tmp);
	return r;
}

UI32 NBString_replaceFloat(STNBString* string, const char* strFind, const float strReplace, const SI8 decimals){
	UI32 r = 0;
	STNBString tmp;
	NBString_initWithSz(&tmp, 32, 32, 0.10f);
	NBString_concatFloat(&tmp, strReplace, decimals);
	r = NBString_replace(string, strFind, tmp.str);
	NBString_release(&tmp);
	return r;
}

UI32 NBString_replaceDouble(STNBString* string, const char* strFind, const double strReplace, const SI8 decimals){
	UI32 r = 0;
	STNBString tmp;
	NBString_initWithSz(&tmp, 32, 32, 0.10f);
	NBString_concatDouble(&tmp, strReplace, decimals);
	r = NBString_replace(string, strFind, tmp.str);
	NBString_release(&tmp);
	return r;
}

char* NBString_strNewBuffer(const char* string){
	char* r = NULL;
	if(string != NULL){
		const UI32 len = NBString_strLenBytes(string);
		r = (char*)NBMemory_alloc(len + 1);
		NBMemory_copy(r, string, len + 1);
		NBASSERT(r[len] == '\0')
	}
	return r;
}

char* NBString_strNewBufferBytes(const char* string, const UI32 len){
	char* r = (char*) NBMemory_alloc(len + 1);
	if(string == NULL){
		r[0] = '\0';
	} else {
		NBMemory_copy(r, string, len);
		r[len] = '\0';
	}
	return r;
}

void NBString_strFreeAndNewBuffer(char** org, const char* string){
	if(org != NULL){
		if(*org != NULL){
			NBMemory_free(*org);
			*org = NULL;
		}
		if(string != NULL){
			*org = NBString_strNewBuffer(string);
		}
	}
}

void NBString_strFreeAndNewBufferBytes(char** org, const char* string, const UI32 len){
	if(org != NULL){
		if(*org != NULL) NBMemory_free(*org);
		*org = NBString_strNewBufferBytes(string, len);
	}
}

BOOL NBString_strIsEmpty(const char* str){
	if(str == NULL) return TRUE;
	return (str[0] == '\0' ? TRUE : FALSE);
}

UI32 NBString_strLenBytes(const char* string){
	if(string == NULL) return 0;
	const char* ptr = string;
	while(*ptr != '\0') ptr++;
	return (UI32)(ptr - string);
}

UI32 NBString_strLenUnicodes(const char* string){
	UI32 r = 0; UI8 bExp;
	const char* ptr = string;
	while(*ptr != '\0'){
		bExp = NBEncoding_utf8BytesExpected(*ptr);
		if(bExp <= 0) break; //Malformed UTF8
		ptr += bExp;
		r++;
	}
	return r;
}

SI32 NBString_strIndexOf(const char* haystack, const char* needle, SI32 posInitial){
	//Not empty string
	if(needle[0] != '\0'){
        //Search
        int posN = 0, posH = (posInitial < 0 ? 0 : posInitial);
        while(haystack[posH] != '\0'){
            if (haystack[posH] == needle[posN]) {
                //Advance in needle
                posN++;
                posH++;
                if (needle[posN] == '\0') return (posH - posN);
            } else {
                //Reset needle and haystack pos
                posH -= (posN - 1);
                posN = 0;
            }
        }
	}
	return -1;
}

SI32 NBString_strIndexOfBytes(const char* haystack, const UI32 haystackLen, const char* needle, SI32 posInitial){
	//Not empty string
	if(needle[0] != '\0' && posInitial < haystackLen){
        //Search
        int posN = 0, posH = (posInitial < 0 ? 0 : posInitial);
        while(posH < haystackLen){
            if (haystack[posH] == needle[posN]) {
                //Advance in needle
                posN++;
                posH++;
                if (needle[posN] == '\0') return (posH - posN);
            } else {
                //Reset needle and haystack pos
                posH -= (posN - 1);
                posN = 0;
            }
        }
	}
	return -1;
}

SI32 NBString_strIndexOfLike(const char* haystack, const char* needle, SI32 posInitial){
	//Not empty string
	if(needle[0] != '\0'){
		//Search
		char c1, c2;
        int posN = 0, posH = (posInitial < 0 ? 0 : posInitial);
		while(haystack[posH] != '\0'){
			c1 = haystack[posH];
			//needle
			c2 = needle[posN];
			if (c1 == c2
				|| ((c1 >= 'A' && c1 <= 'Z') && (c1 + 32) == c2)
				|| ((c1 >= 'a' && c1 <= 'z') && (c1 - 32) == c2)
				) {
				//Advance in needle
                posN++;
                posH++;
				if (needle[posN] == '\0') return (posH - posN);
			} else {
				//Reset needle and haystack pos
                posH -= (posN - 1);
                posN = 0;
			}
		}
	}
	return -1;
}

SI32 NBString_strLastIndexOf(const char* haystack, const char* needle, SI32 posInitialReverse){
    //Not empty string
    if(needle[0] != '\0'){
        //Search
        int needleSz = NBString_strLenBytes(needle);
        int posN = (needleSz - 1), posH = posInitialReverse;
        while(posH >= 0){
            if (haystack[posH] == needle[posN]) {
                //Advance in needle
                if (posN == 0) return posH;
                posN--;
                posH--;
            } else {
                //Reset needle and haystack pos
                posH += (needleSz - posN - 2);
                posN = (needleSz - 1);
            }
        }
    }
    return -1;
}

BOOL NBString_strStartsWith(const char* str1 /*stringCompare*/, const char* str2/*stringBase*/){
	BOOL r = FALSE;
	if(str2 == NULL){
		r = TRUE;
	} else if(*str2 == '\0'){
		r = TRUE;
	} else {
		r = TRUE;
		if(str1 == NULL || str2 == NULL) r = FALSE; //Validar nulos
		if(*str1 == '\0' || *str2 == '\0') r = FALSE; //Validar cadenas vacias
		if(r){
			while((*str1) != '\0' && (*str2) != '\0'){
				if((*str1) != (*str2)){
					r = FALSE; break;
				}
				str1++;	//mover puntero al siguiente caracter
				str2++;	//mover puntero al siguiente caracter
			}
			//Final result
			r = (r && (*str2) == '\0');
		}
	}
	return r; //si la cadena a comparar no termina, entonces es diferente en algun punto
}

BOOL NBString_strStartsWithBytes(const char* str1 /*stringCompare*/, const char* str2, const UI32 str2Sz /*stringBaseSz*/){
	BOOL r = FALSE;
	if(str2 == NULL){
		r = TRUE;
	} else if(*str2 == '\0'){
		r = TRUE;
	} else {
		r = TRUE;
		if(str1 == NULL || str2 == NULL) r = FALSE; //Validar nulos
		if(*str1 == '\0' || *str2 == '\0') r = FALSE; //Validar cadenas vacias
		if(r){
			const char* str2AfterEnd = str2 + str2Sz;
			while((*str1) != '\0' && str2 < str2AfterEnd){
				if((*str1) != (*str2)){
					r = FALSE; break;
				}
				str1++;	//mover puntero al siguiente caracter
				str2++;	//mover puntero al siguiente caracter
			}
			//Final result
			r = (r && str2 == str2AfterEnd);
		}
	}
	return r; //si la cadena a comparar no termina, entonces es diferente en algun punto
}

BOOL NBString_strEndsWith(const char* str1 /*stringCompare*/, const char* str2 /*stringEnd*/){
	BOOL r = FALSE;
	if(str1 != NULL){
		if(str2 == NULL){
			r = TRUE;
		} else if(*str2 == '\0'){
			r = TRUE;
		} else {
			const char* str1End = str1;
			const char* str2End = str2;
			//Move to end
			while(*str1End != '\0') str1End++;
			while(*str2End != '\0') str2End++;
			//Compare
			r = TRUE;
			if((str1End - str1) < (str2End - str2)){
				//too short
				r = FALSE;
			} else {
				//Compare
				do {
					if((*str1End) != (*str2End)){
						r = FALSE; break;
					}
					str1End--;
					str2End--;
				} while(str1End != str1 && str2End != str2);
			}
		}
	}
	return r; //si la cadena a comparar no termina, entonces es diferente en algun punto
}

//Errors are miss or extra unicode-chars
typedef struct STNBStringSrchImperfectHeader_ {
	UI32			maxErrors;		//max allowed errors (missing or added chars)
	ENStringFind	srchMode;		//stop at first find?
	const char* 	haystack;		//search-at
	const char* 	needle;			//searching
	UI32			needleBytesLen;	//searching len in bytes
	UI32			needleUniLen;	//searching len in unicode
} STNBStringSrchImperfectHeader;

typedef struct STNBStringSrchImperfectBody_ {
	UI32 iHaystack;	//current byte in haystack
	UI32 iByte;		//current byte idx
	UI32 countErrs;	//errors count (missing or added chars)
	UI32 iDepth;	//Depth in recursivity
} STNBStringSrchImperfectBody;

BOOL NBString_strIsLikeImperfect_(const STNBStringSrchImperfectHeader* hdr, const STNBStringSrchImperfectBody* body){
	//IF_NBASSERT(STNBString strSpaces;)
	//IF_NBASSERT(NBString_init(&strSpaces);)
	//IF_NBASSERT(NBString_concatRepeatedByte(&strSpaces, '\t', body->iDepth);)
	BOOL r = FALSE;
	NBASSERT(hdr->maxErrors < hdr->needleUniLen)
	NBASSERT(body->countErrs <= hdr->maxErrors)
	if(hdr->needle[body->iByte] == '\0'){
		//Needle found
		r = TRUE;
	} else {
		//Search all children posibilities
		//1-Search advance boths one-unichar
		if(hdr->haystack[body->iHaystack] != '\0'){
			STNBStringSrchImperfectBody newBody = *body;
			BOOL isEquivUnic = FALSE;
			const UI8 hSurgSz = NBEncoding_utf8BytesExpected(hdr->haystack[body->iHaystack]);
			const UI8 nSurgSz = NBEncoding_utf8BytesExpected(hdr->needle[body->iByte]);
			//Compare unichar
			{
				//PRINTF_INFO("%s(#%d) %d-err; Comparing: '%s' vs '%s'.\n", strSpaces.str, (body->iDepth + 1), body->countErrs, &hdr->needle[body->iByte], &hdr->haystack[body->iHaystack]);
				if(hSurgSz == nSurgSz){
					if(hSurgSz == 1){
						//One byte char
						if(NBEncoding_asciiLower(hdr->haystack[body->iHaystack]) == NBEncoding_asciiLower(hdr->needle[body->iByte])){
							//PRINTF_INFO("%s(#%d) %d-err; AreEquiv ascii haystack[%d]('%c') hystack[%d]('%c').\n", strSpaces.str, (body->iDepth + 1), body->countErrs, body->iHaystack, hdr->haystack[body->iHaystack], body->iByte, hdr->needle[body->iByte]);
							isEquivUnic = TRUE;
						}
					} else {
						//Compare bytes
						UI8 i; for(i = 0; i < hSurgSz; i++){
							if(hdr->haystack[body->iHaystack + i] != hdr->needle[body->iByte + i]){
								break;
							}
						}
						if(i == hSurgSz){
							//PRINTF_INFO("%s(#%d) %d-err; AreEquiv multibyte-char haystack[%d + %d] hystack[%d + %d].\n", strSpaces.str, (body->iDepth + 1), body->countErrs, body->iHaystack, hSurgSz, body->iByte, nSurgSz);
							isEquivUnic = TRUE;
						}
					}
				}
			}
			newBody.iDepth++;
			newBody.countErrs	+= (isEquivUnic ? 0 : 1);
			if(newBody.countErrs <= hdr->maxErrors){
				newBody.iHaystack	+= hSurgSz; NBASSERT(hSurgSz > 0)
				newBody.iByte		+= nSurgSz; NBASSERT(nSurgSz > 0)
				if(NBString_strIsLikeImperfect_(hdr, &newBody)){
					//PRINTF_INFO("%s(#%d) %d-err; Found-advancing-both haystack[%d + %d] hystack[%d + %d].\n", strSpaces.str, (body->iDepth + 1), body->countErrs, body->iHaystack, hSurgSz, body->iByte, nSurgSz);
					r = TRUE;
				}
			}
		}
		//2-Search advancing only needle
		if(!r || hdr->srchMode != ENStringFind_First){
			UI8 bExp; STNBStringSrchImperfectBody newBody = *body;
			newBody.iDepth++;
			while(hdr->needle[newBody.iByte] != '\0' && newBody.countErrs < hdr->maxErrors){
				bExp = NBEncoding_utf8BytesExpected(hdr->needle[newBody.iByte]);
				if(bExp <= 0) break; //Malformed UTF8
				newBody.iByte += bExp;
				newBody.countErrs++;
				if(NBString_strIsLikeImperfect_(hdr, &newBody)){
					//PRINTF_INFO("%s(#%d) %d-err; Found-advancing-needle(+%d) haystack[%d + %d] hystack[%d + %d].\n", strSpaces.str, (body->iDepth + 1), body->countErrs, (newBody.iByte - body->iByte), body->iHaystack, hSurgSz, body->iByte, nSurgSz);
					r = TRUE;
					if(hdr->srchMode == ENStringFind_First){
						break;
					}
				}
			}
		}
		//3-Search advancing only haystack
		if(!r || hdr->srchMode != ENStringFind_First){
			UI8 bExp; STNBStringSrchImperfectBody newBody = *body;
			newBody.iDepth++;
			while(hdr->haystack[newBody.iHaystack] != '\0' && newBody.countErrs < hdr->maxErrors){
				bExp = NBEncoding_utf8BytesExpected(hdr->haystack[newBody.iHaystack]);
				if(bExp <= 0) break; //Malformed UTF8
				newBody.iHaystack += bExp;
				newBody.countErrs++;
				if(NBString_strIsLikeImperfect_(hdr, &newBody)){
					//PRINTF_INFO("%s(#%d) %d-err; Found-advancing-haystack(+%d) haystack[%d + %d] hystack[%d + %d].\n", strSpaces.str, (body->iDepth + 1), body->countErrs, (newBody.iByte - body->iByte), body->iHaystack, hSurgSz, body->iByte, nSurgSz);
					r = TRUE;
					if(hdr->srchMode == ENStringFind_First){
						break;
					}
				}
			}
		}
	}
	//IF_NBASSERT(NBString_release(&strSpaces);)
	return r;
}

BOOL NBString_strContainsImperfect(const char* pHaystack, const char* pNeedle, const UI32 pMaxErrors, const ENStringFind pSrchMode){
	BOOL r = FALSE;
	if(pHaystack != NULL && pNeedle != NULL){
		//Create search header (recursivity optim, passing a pointer help to reduce call-stack size)
		STNBStringSrchImperfectHeader hdr;
		NBMemory_setZeroSt(hdr, STNBStringSrchImperfectHeader);
		hdr.maxErrors	= pMaxErrors;
		hdr.srchMode	= pSrchMode;
		hdr.haystack	= pHaystack;
		hdr.needle		= pNeedle;
		//Calculate needle len
		{
			UI8 bExp;
			while(hdr.needle[hdr.needleBytesLen] != '\0'){
				bExp = NBEncoding_utf8BytesExpected(hdr.needle[hdr.needleBytesLen]);
				if(bExp <= 0) break; //Malformed UTF8
				hdr.needleBytesLen += bExp;
				hdr.needleUniLen++;
			}
		}
		//Validate max errors
		if(hdr.maxErrors >= hdr.needleUniLen){
			//"maxErrors" will be found everywhere (no need to search)
			r = TRUE;
		} else {
			STNBStringSrchImperfectBody body;
			NBMemory_setZeroSt(body, STNBStringSrchImperfectBody);
			//Search all hayjack
			UI8 bExp;
			while(hdr.haystack[body.iHaystack] != '\0' && (!r || hdr.srchMode != ENStringFind_First)){
				//Search all start posibilities from this point in haystack
				{
					body.iByte = 0; body.countErrs = 0;
					while(hdr.needle[body.iByte] != '\0' && body.countErrs <= hdr.maxErrors){
						//Search tree of posibilities
						if(NBString_strIsLikeImperfect_(&hdr, &body)){
							r = TRUE;
							//Stop at first
							if(hdr.srchMode == ENStringFind_First){
								break;
							}
						}
						//Next
						bExp = NBEncoding_utf8BytesExpected(hdr.needle[body.iByte]);
						if(bExp <= 0) break; //Malformed UTF8
						body.iByte += bExp;
						body.countErrs++; //searching whith missing start-chars
					}
				}
				//Next
				bExp = NBEncoding_utf8BytesExpected(hdr.haystack[body.iHaystack]);
				if(bExp <= 0) break; //Malformed UTF8
				body.iHaystack += bExp;
			}
		}
	}
	return r;
}

//

BOOL NBString_strIsEqual(const char* str1, const char* str2){
	if(str1 == str2) return TRUE; //both can be NULL or the same pointer
	if((str1 == NULL && str2 != NULL) || (str1 != NULL && str2 == NULL)) return FALSE;
	//PRINTF_INFO("Comparando cadenas: '%s' vs '%s'\n", str1, str2);
	while(*str1 != '\0'){
		if(*str1 != *str2) return FALSE;
		str1++; str2++;
	}
	return (*str1 == *str2); //both should end in '\0'
}

BOOL NBString_strIsEqualBytes(const char* str1, const UI32 len1, const char* str2, const UI32 len2){
	if(str1 == str2) return TRUE; //both can be NULL or the same pointer
	if(len1 != len2 || (str1 == NULL && str2 != NULL) || (str1 != NULL && str2 == NULL)) return FALSE;
	const char* str2Afert = str2 + len2;
	while(str2 < str2Afert){
		if(*str1 != *str2) break;
		str1++; str2++;
	}
	return (str2 == str2Afert);
}

BOOL NBString_strIsEqualStrBytes(const char* str1, const char* str2, const UI32 len2){
	if(str1 == str2) return TRUE; //both can be NULL or the same pointer
	if((str1 == NULL && str2 != NULL) || (str1 != NULL && str2 == NULL)) return FALSE;
	const char* str2Afert = str2 + len2;
	while(*str1 != '\0' && str2 < str2Afert){
		if(*str1 != *str2) return FALSE;
		str1++; str2++;
	}
	return (*str1 == '\0' && str2 == str2Afert);
}

BOOL NBString_strIsLike(const char* c1, const char* c2){
	if(c1 == NULL && c2 == NULL) return TRUE;
	if((c1 == NULL && c2 != NULL) || (c1 != NULL && c2 == NULL)) return FALSE;
	while(TRUE){
		//A-Z: 65-90; a-z: 97-122
		if(*c1 != *c2){
			if(!((*c1 >= 'A' && *c1 <= 'Z') && (*c1 + 32) == *c2)
			   && !((*c1 >= 'a' && *c1 <= 'z') && (*c1 - 32) == *c2)){
				return FALSE;
			}
		}
		if(*c1 == '\0') break;
		c1++; c2++;
	}
	return TRUE;
}

BOOL NBString_strIsLikeBytes(const char* c1, const UI32 len1, const char* c2, const UI32 len2){
	if(c1 == NULL && c2 == NULL) return TRUE;
	if(len1 != len2 || (c1 == NULL && c2 != NULL) || (c1 != NULL && c2 == NULL)) return FALSE;
	const char* c2After = c2 + len2;
	while(c2 < c2After){
		//A-Z: 65-90; a-z: 97-122
		if(*c1 != *c2){
			if(!((*c1 >= 'A' && *c1 <= 'Z') && (*c1 + 32) == *c2)
			   && !((*c1 >= 'a' && *c1 <= 'z') && (*c1 - 32) == *c2)){
				break;
			}
		}
		c1++; c2++;
	}
	return (c2 == c2After);
}

BOOL NBString_strIsLikeStrBytes(const char* c1, const char* c2, const UI32 len2){
	if(c1 == NULL && c2 == NULL) return TRUE;
	if((c1 == NULL && c2 != NULL) || (c1 != NULL && c2 == NULL)) return FALSE;
	const char* c2After = c2 + len2;
	while(*c1 != '\0' && c2 < c2After){
		//A-Z: 65-90; a-z: 97-122
		if(*c1 != *c2){
			if(!((*c1 >= 'A' && *c1 <= 'Z') && (*c1 + 32) == *c2)
			   && !((*c1 >= 'a' && *c1 <= 'z') && (*c1 - 32) == *c2)){
				break;
			}
		}
		c1++; c2++;
	}
	return (*c1 == '\0' && c2 == c2After);
}

//

BOOL NBString_strIsLower(const char* str1, const char* str2){
	while(*str1 != 0 && *str2 != 0){
		if(*str1 < *str2){
			return TRUE;
		} else if(*str1 > *str2){
			return FALSE;
		}
		str1++; str2++;
	}
	return (*str1 < *str2);
}

BOOL NBString_strIsLowerBytes(const char* str1, const UI32 len1, const char* str2, const UI32 len2){
	const char* str1After = str1 + len1;
	const char* str2After = str2 + len2;
	while(str1 < str1After && str2 < str2After){
		if(*str1 < *str2){
			return TRUE;
		} else if(*str1 > *str2){
			return FALSE;
		}
		str1++; str2++;
	}
	return ((str1 == str1After ? '\0' : *str1) < (str2 == str2After ? '\0' : *str2));
}

BOOL NBString_strIsLowerStrBytes(const char* str1, const char* str2, const UI32 len2){
	const char* str2After = str2 + len2;
	while(*str1 != '\0' && str2 < str2After){
		if(*str1 < *str2){
			return TRUE;
		} else if(*str1 > *str2){
			return FALSE;
		}
		str1++; str2++;
	}
	return (*str1 < (str2 == str2After ? '\0' : *str2));
}

BOOL NBString_strIsLowerOrEqual(const char* str1, const char* str2){
	while(*str1 != 0 && *str2 != 0){
		if(*str1 < *str2){
			return TRUE;
		} else if(*str1 > *str2){
			return FALSE;
		}
		str1++; str2++;
	}
	return (*str1 <= *str2);
}

BOOL NBString_strIsLowerOrEqualBytes(const char* str1, const UI32 len1, const char* str2, const UI32 len2){
	const char* str1After = str1 + len1;
	const char* str2After = str2 + len2;
	while(str1 < str1After && str2 < str2After){
		if(*str1 < *str2){
			return TRUE;
		} else if(*str1 > *str2){
			return FALSE;
		}
		str1++; str2++;
	}
	return ((str1 == str1After ? '\0' : *str1) <= (str2 == str2After ? '\0' : *str2));
}

BOOL NBString_strIsLowerOrEqualStrBytes(const char* str1, const char* str2, const UI32 len2){
	const char* str2After = str2 + len2;
	while(*str1 != '\0' && str2 < str2After){
		if(*str1 < *str2){
			return TRUE;
		} else if(*str1 > *str2){
			return FALSE;
		}
		str1++; str2++;
	}
	return (*str1 <= (str2 == str2After ? '\0' : *str2));
}

BOOL NBString_strIsGreater(const char* str1, const char* str2){
	while(*str1 != 0 && *str2 != 0){
		if(*str1 > *str2){
			return TRUE;
		} else if(*str1 < *str2){
			return FALSE;
		}
		str1++; str2++;
	}
	return (*str1 > *str2);
}

BOOL NBString_strIsGreaterBytes(const char* str1, const UI32 len1, const char* str2, const UI32 len2){
	const char* str1After = str1 + len1;
	const char* str2After = str2 + len2;
	while(str1 < str1After && str2 < str2After){
		if(*str1 > *str2){
			return TRUE;
		} else if(*str1 < *str2){
			return FALSE;
		}
		str1++; str2++;
	}
	return ((str1 == str1After ? '\0' : *str1) > (str2 == str2After ? '\0' : *str2));
}

BOOL NBString_strIsGreaterStrBytes(const char* str1, const char* str2, const UI32 len2){
	const char* str2After = str2 + len2;
	while(*str1 != '\0' && str2 < str2After){
		if(*str1 > *str2){
			return TRUE;
		} else if(*str1 < *str2){
			return FALSE;
		}
		str1++; str2++;
	}
	return (*str1 > (str2 == str2After ? '\0' : *str2));
}

BOOL NBString_strIsGreaterOrEqual(const char* str1, const char* str2){
	while(*str1 != 0 && *str2 != 0){
		if(*str1 > *str2){
			return TRUE;
		} else if(*str1 < *str2){
			return FALSE;
		}
		str1++; str2++;
	}
	return (*str1 >= *str2);
}

BOOL NBString_strIsGreaterOrEqualBytes(const char* str1, const UI32 len1, const char* str2, const UI32 len2){
	const char* str1After = str1 + len1;
	const char* str2After = str2 + len2;
	while(str1 < str1After && str2 < str2After){
		if(*str1 > *str2){
			return TRUE;
		} else if(*str1 < *str2){
			return FALSE;
		}
		str1++; str2++;
	}
	return ((str1 == str1After ? '\0' : *str1) >= (str2 == str2After ? '\0' : *str2));
}

BOOL NBString_strIsGreaterOrEqualStrBytes(const char* str1, const char* str2, const UI32 len2){
	const char* str2After = str2 + len2;
	while(*str1 != '\0' && str2 < str2After){
		if(*str1 > *str2){
			return TRUE;
		} else if(*str1 < *str2){
			return FALSE;
		}
		str1++; str2++;
	}
	return (*str1 >= (str2 == str2After ? '\0' : *str2));
}

//

void NBString_concatDateTimeCompact(STNBString* string, const STNBDatetime dateTime){
	NBString_concatSI32(string, dateTime.year);
	if(dateTime.year < 1000) NBString_concatByte(string, '0');
	if(dateTime.year < 100) NBString_concatByte(string, '0');
	if(dateTime.year < 10) NBString_concatByte(string, '0');
	//
	if(dateTime.month < 10) NBString_concatByte(string, '0'); NBString_concatSI32(string, dateTime.month);
	//
	if(dateTime.day < 10) NBString_concatByte(string, '0'); NBString_concatSI32(string, dateTime.day);
	NBString_concatByte(string, ' ');
	if(dateTime.hour<10) NBString_concatByte(string, '0'); NBString_concatSI32(string, dateTime.hour);
	if(dateTime.min<10) NBString_concatByte(string, '0'); NBString_concatSI32(string, dateTime.min);
	if(dateTime.sec<10) NBString_concatByte(string, '0'); NBString_concatSI32(string, dateTime.sec);
}

void NBString_concatSqlDate(STNBString* string, const STNBDate date){
	NBString_concatSI32(string, date.year);
	if(date.year < 1000) NBString_concatByte(string, '0');
	if(date.year < 100) NBString_concatByte(string, '0');
	if(date.year < 10) NBString_concatByte(string, '0');
	NBString_concatByte(string, '-');
	if(date.month < 10) NBString_concatByte(string, '0');
	NBString_concatSI32(string, date.month);
	NBString_concatByte(string, '-');
	if(date.day < 10) NBString_concatByte(string, '0');
	NBString_concatSI32(string, date.day);
}

void NBString_concatSqlDatePart(STNBString* string, const STNBDatetime time){
	NBString_concatSI32(string, time.year);
	if(time.year < 1000) NBString_concatByte(string, '0');
	if(time.year < 100) NBString_concatByte(string, '0');
	if(time.year < 10) NBString_concatByte(string, '0');
	NBString_concatByte(string, '-');
	if(time.month < 10) NBString_concatByte(string, '0');
	NBString_concatSI32(string, time.month);
	NBString_concatByte(string, '-');
	if(time.day < 10) NBString_concatByte(string, '0');
	NBString_concatSI32(string, time.day);
}

void NBString_concatSqlDatetime(STNBString* string, const STNBDatetime time){
	NBString_concatSI32(string, time.year);
	if(time.year < 1000) NBString_concatByte(string, '0');
	if(time.year < 100) NBString_concatByte(string, '0');
	if(time.year < 10) NBString_concatByte(string, '0');
	NBString_concatByte(string, '-');
	if(time.month < 10) NBString_concatByte(string, '0'); NBString_concatSI32(string, time.month);
	NBString_concatByte(string, '-');
	if(time.day < 10) NBString_concatByte(string, '0'); NBString_concatSI32(string, time.day);
	NBString_concatByte(string, ' ');
	if(time.hour < 10) NBString_concatByte(string, '0'); NBString_concatSI32(string, time.hour);
	NBString_concatByte(string, ':');
	if(time.min < 10) NBString_concatByte(string, '0'); NBString_concatSI32(string, time.min);
	NBString_concatByte(string, ':');
	if(time.sec < 10) NBString_concatByte(string, '0'); NBString_concatSI32(string, time.sec);
	NBString_concatByte(string, '.');
	NBString_concatSI32(string, time.ms);
}

/*void NBString_concatDateTimeCurrent(STNBString* string){
    NBString_concatDateTime(string, time(NULL));
}*/

/*void NBString_concatDateTimeCompactCurrent(STNBString* string){
    NBString_concatDateTimeCompact(string, time(NULL));
}*/

/*void NBString_concatDateTime(STNBString* string, const time_t dateTime){
	/ *struct tm{
	    int     tm_sec;         / * Seconds: 0-59 (K&R says 0-61?) * /
	    int     tm_min;         / * Minutes: 0-59 * /
	    int     tm_hour;        / * Hours since midnight: 0-23 * /
	    int     tm_mday;        / * Day of the month: 1-31 * /
	    int     tm_mon;         / * Months *since* january: 0-11 * /
	    int     tm_year;        / * Years since 1900 * /
	    int     tm_wday;        / * Days since Sunday (0-6) * /
	    int     tm_yday;        / * Days since Jan. 1: 0-365 * /
	}* /
	SYSTEMTIME st = *localtime(&dateTime);
    NBString_concatSI32(string, 1900+st.tm_year);
    NBString_concatByte(string, '-');
	if(st.tm_mon<9) NBString_concatByte(string, '0'); NBString_concatSI32(string, st.tm_mon+1);
    NBString_concatByte(string, '-');
	if(st.tm_mday<10) NBString_concatByte(string, '0'); NBString_concatSI32(string, st.tm_mday);
    NBString_concatByte(string, ' ');
	if(st.tm_hour<10) NBString_concatByte(string, '0'); NBString_concatSI32(string, st.tm_hour);
    NBString_concatByte(string, ':');
	if(st.tm_min<10) NBString_concatByte(string, '0'); NBString_concatSI32(string, st.tm_min);
    NBString_concatByte(string, ':');
	if(st.tm_sec<10) NBString_concatByte(string, '0'); NBString_concatSI32(string, st.tm_sec);
}*/

void NBString_concat(STNBString* string, const char* str){
	if(str != NULL){
		if(str[0] != '\0'){
			UI32 i, use, sizeConcat; char* buffer;
			//Asegurar el tamano del buffer
			sizeConcat = 0; while(str[sizeConcat] != '\0') sizeConcat++;
			if((string->length + sizeConcat + 1) > (string->_buffSz)){
			    NBString_increaseBuffer(string, sizeConcat);
			}
			//
		    NBASSERT(string->length >= 0); //Por definicion una buffer debe tener por lo menos el caracter nulo
			use = string->length; buffer = string->str;
			for(i=0; i<sizeConcat; i++) buffer[use++] = str[i];
			buffer[use++] = '\0';
			string->length = (use - 1);
		}
	}
}

void NBString_concatLower(STNBString* string, const char* str){
	if(str != NULL){
		UI8 surg = 0;
		while(*str != '\0'){
			surg = NBEncoding_utf8BytesExpected(*str);
			if(surg <= 0) break;
			if(surg == 1){
				NBString_concatByte(string, NBEncoding_asciiLower(*str));
			} else {
				NBString_concatBytes(string, str, surg);
			}
			str += surg;
		}
	}
}

void NBString_concatLowerBytes(STNBString* string, const char* str, const UI32 strSz){
	if(str != NULL && strSz > 0){
		UI8 surg = 0;
		const char* afterLast = str + strSz;
		while(str < afterLast){
			surg = NBEncoding_utf8BytesExpected(*str);
			if(surg <= 0) break;
			if(surg == 1){
				NBString_concatByte(string, NBEncoding_asciiLower(*str));
			} else {
				NBString_concatBytes(string, str, surg);
			}
			str += surg;
		}
	}
}

void NBString_concatUpper(STNBString* string, const char* str){
	if(str != NULL){
		UI8 surg = 0;
		while(*str != '\0'){
			surg = NBEncoding_utf8BytesExpected(*str);
			if(surg <= 0) break;
			if(surg == 1){
				NBString_concatByte(string, NBEncoding_asciiUpper(*str));
			} else {
				NBString_concatBytes(string, str, surg);
			}
			str += surg;
		}
	}
}

void NBString_concatUpperBytes(STNBString* string, const char* str, const UI32 strSz){
	if(str != NULL && strSz > 0){
		UI8 surg = 0;
		const char* afterLast = str + strSz;
		while(str < afterLast){
			surg = NBEncoding_utf8BytesExpected(*str);
			if(surg <= 0) break;
			if(surg == 1){
				NBString_concatByte(string, NBEncoding_asciiUpper(*str));
			} else {
				NBString_concatBytes(string, str, surg);
			}
			str += surg;
		}
	}
}


void NBString_concatByte(STNBString* string, const char c){
	//Asegurar el tamano del buffer
	if((string->length + 1 + 1) > string->_buffSz){
	    NBString_increaseBuffer(string, 1);
	}
    NBASSERT(string->length >= 0); //Por definicion una buffer debe tener por lo menos el caracter nulo
	string->str[string->length++] = c;
	string->str[string->length] = '\0';
}

void NBString_concatBytes(STNBString* string, const void* data, const UI32 dataSz){
	UI32 i, use; char* buffer;
	const char* str = (const char*)data;
	//Asegurar el tamano del buffer
	if((string->length + dataSz + 1) > (string->_buffSz)){
		NBString_increaseBuffer(string, dataSz);
	}
	//
	NBASSERT(string->length >= 0); //Por definicion una buffer debe tener por lo menos el caracter nulo
	use = string->length; buffer = string->str;
	if(str != NULL){
		for(i = 0; i < dataSz; i++){
			buffer[use++] = str[i];
		}
	} else {
		use += dataSz;
	}
	buffer[use++] = '\0';
	string->length = (use - 1);
}

void NBString_concatBytesHex(STNBString* string, const void* data, const UI32 dataSz){
	const char* hex = "0123456789abcdef";
	const char* str = (const char*)data;
	UI32 i, use; char* buffer;
	//Asegurar el tamano del buffer
	if((string->length + (dataSz * 2) + 1) > (string->_buffSz)){
		NBString_increaseBuffer(string, (dataSz * 2));
	}
	//
	NBASSERT(string->length >= 0); //Por definicion una buffer debe tener por lo menos el caracter nulo
	use = string->length; buffer = string->str;
	if(str != NULL){
		for(i = 0; i < dataSz; i++){
			buffer[use++] = hex[(str[i] >> 4) & 15];
			buffer[use++] = hex[str[i] & 15];
		}
	} else {
		use += dataSz * 2;
	}
	buffer[use++] = '\0';
	string->length = (use - 1);
}

#define CONCAT_INTEGER_U(TYPENUMERIC, numberToConvert, SIZEBUFFER) \
	/*buffer donde se almacenan los digitos (enteros y decimales por separado)*/ \
	UI32 position; char dstIntegers[SIZEBUFFER]; \
	/*procesar parte entera*/ \
	dstIntegers[SIZEBUFFER - 1]		= '\0'; \
	position						= SIZEBUFFER - 1; \
	{ \
		TYPENUMERIC copyNumber	= (TYPENUMERIC)numberToConvert; \
		do { \
		    NBASSERT((copyNumber % (TYPENUMERIC)10) >= 0) \
			dstIntegers[--position] = (char)((TYPENUMERIC)'0' + (copyNumber % (TYPENUMERIC)10)); \
			copyNumber				/= (TYPENUMERIC)10; \
		} while(copyNumber != 0); \
	} \
	/*copiar resultado a destino*/ \
    NBString_concat(string, &(dstIntegers[position]));

#define CONCAT_INTEGER(TYPENUMERIC, numberToConvert, SIZEBUFFER) \
	/*buffer donde se almacenan los digitos (enteros y decimales por separado)*/ \
	char sign, dstIntegers[SIZEBUFFER]; UI32 position; TYPENUMERIC valueZero; \
	/*definir signo y asegurar positivo*/ \
	valueZero = 0; dstIntegers[SIZEBUFFER - 1] = '\0'; \
	if(numberToConvert < valueZero){ \
		sign = '-'; numberToConvert = -numberToConvert;			/*convertir a positivo (hay un bug cuando es negativo, causado por el bit de signo cuya posicion no esta definida en cada sistema)*/ \
		/*Nota: cuando el valor es el limite negativo (pe: -2147483648 en 32 bits, este no se puede convertir a positivo, porque el maximo positivo es una unidad menor, ej: 2147483647 en 32 bits)*/ \
	} else { \
		sign = '+'; \
	} \
    NBASSERT(numberToConvert >= valueZero) \
	/*procesar parte entera*/ \
	position						= SIZEBUFFER - 1; \
	{ \
		TYPENUMERIC copyNumber	= (TYPENUMERIC)numberToConvert; \
		do { \
		    NBASSERT((copyNumber % (TYPENUMERIC)10) >= 0) \
			dstIntegers[--position] = (char)((TYPENUMERIC)'0' + (copyNumber % (TYPENUMERIC)10)); \
			copyNumber				/= (TYPENUMERIC)10; \
		} while(copyNumber != 0); \
		/*sign*/ \
		if(sign == '-') dstIntegers[--position] = sign; \
	} \
	/*copiar resultado a destino*/ \
    NBString_concat(string, &(dstIntegers[position]));


#define CONCAT_DECIMAL(TYPENUMERIC, TYPEINTEGER, numberToConvert, SIZEBUFFER, posDecimals) \
	/*buffer donde se almacenan los digitos (enteros y decimales por separado)*/ \
	char sign, dstIntegers[SIZEBUFFER], dstDecimals[SIZEBUFFER]; UI32 digitsAtRightNotZero, position; TYPENUMERIC valueZero; \
	/*definir signo y asegurar positivo*/ \
	valueZero = 0; dstIntegers[SIZEBUFFER - 1] = dstDecimals[0] = '\0'; \
	if(numberToConvert < valueZero){ \
		sign = '-'; numberToConvert = -numberToConvert;			/*convertir a positivo (hay un bug cuando es negativo, causado por el bit de signo cuya posicion no esta definida en cada sistema)*/ \
	} else { \
		sign = '+'; \
	} \
    NBASSERT(numberToConvert >= valueZero) \
	/*procesar parte decimal*/ \
	digitsAtRightNotZero	= 0; \
	if(posDecimals > 0){ \
		SI32 added				= 0; \
		TYPENUMERIC copyNumber	= numberToConvert; \
		while(added < posDecimals){ \
			copyNumber				= copyNumber - (TYPENUMERIC)((TYPEINTEGER)copyNumber);	/*Quitar la parte entera*/ \
			copyNumber				*= (TYPENUMERIC)10;										/*Mover el punto decimal a la derecha*/ \
		    NBASSERT((TYPEINTEGER)copyNumber >= 0) \
			dstDecimals[added++]	= (char)((TYPEINTEGER)'0' + (TYPEINTEGER)copyNumber);	/*Guardar digito (despues de posicion ASCII del cero)*/ \
			if((TYPEINTEGER)copyNumber != 0) digitsAtRightNotZero = added; \
		} \
		if(posDecimals > 0){ \
			digitsAtRightNotZero = added; /*comment if you want auto-decimals-digits*/ \
		} \
		dstDecimals[digitsAtRightNotZero] = '\0'; \
	} \
	/*procesar parte entera*/ \
	position						= SIZEBUFFER - 1; \
	{ \
		TYPEINTEGER copyNumber		= (TYPEINTEGER)numberToConvert; \
		do { \
		    NBASSERT((copyNumber % (TYPEINTEGER)10) >= 0) \
			dstIntegers[--position] = (char)((TYPEINTEGER)'0' + (copyNumber % (TYPEINTEGER)10)); \
			copyNumber				/= (TYPEINTEGER)10; \
		} while(copyNumber != 0); \
		/*sign*/ \
		if(sign == '-') dstIntegers[--position] = sign; \
	} \
	/*copiar resultado a destino*/ \
    NBString_concat(string, &(dstIntegers[position])); \
	if(digitsAtRightNotZero > 0){ \
	    NBString_concat(string, "."); \
	    NBString_concat(string, dstDecimals); \
	}

void NBString_concatUI32(STNBString* string, UI32 number){
	CONCAT_INTEGER_U(UI32, number, 64)
}

void NBString_concatUI64(STNBString* string, UI64 number){
	CONCAT_INTEGER_U(UI64, number, 64)
}

void NBString_concatSI32(STNBString* string, SI32 number){
	//2015-11-03, Marcos Ortega.
	//Nota: cuando el valor es "-2147483648" el sistema nolo puede convertir a positivo.
	//Por eso es mejor procesarlo en el rango siguiente de valores (SI64)
	SI64 number64 = number;
	CONCAT_INTEGER(SI64, number64, 64)

}

void NBString_concatSI64(STNBString* string, SI64 number){
	CONCAT_INTEGER(SI64, number, 64)
}

void NBString_concatFloat(STNBString* string, float number, const SI8 decimals){
	CONCAT_DECIMAL(float, SI64, number, 64, decimals)
}

void NBString_concatDouble(STNBString* string, double number, const SI8 decimals){
	CONCAT_DECIMAL(double, SI64, number, 64, decimals)
}

void NBString_concatSecondsWithFormat(STNBString* string, UI64 seconds){
	if(seconds<60){
	    NBString_concatUI64(string, seconds); NBString_concat(string, " segundos");
	} else if(seconds < (60*60)){
		float number =  (float)seconds / 60.0f;
		CONCAT_DECIMAL(float, SI64, number, 64, 2)  NBString_concat(string, " minutos");
	} else if(seconds < (60*60*24)){
		float number =  (float)seconds / (60.0f * 60.0f);
		CONCAT_DECIMAL(float, SI64, number, 64, 2)  NBString_concat(string, " horas");
	} else if(seconds < (60*60*24*7)){
		float number =  (float)seconds / (60.0f * 60.0f * 24.0f);
		CONCAT_DECIMAL(float, SI64, number, 64, 2)  NBString_concat(string, " dias");
	} else if(seconds < (60*60*24*30)){
		float number =  (float)seconds / (60.0f * 60.0f * 24.0f * 7.0f);
		CONCAT_DECIMAL(float, SI64, number, 64, 2)  NBString_concat(string, " semanas");
	} else if(seconds < (60*60*24*365)){
		float number =  (float)seconds / (60.0f * 60.0f * 24.0f * 30.25f);
		CONCAT_DECIMAL(float, SI64, number, 64, 2)  NBString_concat(string, " meses");
	} else {
		float number =  (float)seconds / (60.0f * 60.0f * 24.0f * 365.0f);
		CONCAT_DECIMAL(float, SI64, number, 64, 2)  NBString_concat(string, " anios");
	}
}

void NBString_concatRepeatedByte(STNBString* string, const char charAdd, const UI32 qnt){
	UI32 i, use; char* buffer;
	//Asegurar el tamano del buffer
	if((string->length + qnt + 1) > (string->_buffSz)){
		NBString_increaseBuffer(string, qnt);
	}
	//
	NBASSERT(string->length >= 0); //Por definicion una buffer debe tener por lo menos el caracter nulo
	use = string->length; buffer = string->str;
	for(i = 0; i < qnt; i++){
		buffer[use++] = charAdd;
	}
	buffer[use++] = '\0';
	string->length = (use - 1);
}

//

void NBString_concatAsUtf16(STNBString* string, const char* str){
	UI8 surrExp = 0; UI32 unic = 0; UI16 u16 = 0;
	while(*str != '\0'){
		surrExp		= NBEncoding_utf8BytesExpected(*str);
		if(surrExp <= 0) break;
		unic		= NBEncoding_unicodeFromUtf8s(str, surrExp, '\0');
		u16			= (UI16)unic;
		if(u16 != '\0'){
			NBString_concatBytes(string, (const char*)&u16, sizeof(u16));
		}
		str += surrExp;
	}
}

void NBString_concatAsUtf16Bytes(STNBString* string, const char* str, const UI32 bytes){
	 UI8 surrExp = 0; UI32 unic = 0; UI16 u16 = 0;
	 const char* strAfterEnd = str + bytes;
	 while(str < strAfterEnd){
		 surrExp		= NBEncoding_utf8BytesExpected(*str);
		 if(surrExp <= 0) break;
		 unic		= NBEncoding_unicodeFromUtf8s(str, surrExp, '\0');
		 u16			= (UI16)unic;
		 if(u16 != '\0'){
			 NBString_concatBytes(string, (const char*)&u16, sizeof(u16));
		 }
		 str += surrExp;
	 }
}

void NBString_concatUtf16(STNBString* string, const UI16* str){
	UI8 surrExp = 0; UI32 unic = 0; char utf8[7];
	while(*str != '\0'){
		surrExp		= NBEncoding_utf16BytesExpected(*str) / 2;
		unic		= NBEncoding_unicodeFromUtf16s(str, surrExp, '?');
		NBEncoding_utf8FromUnicode(unic, utf8);
		NBString_concat(string, utf8);
		str += surrExp;
	}
}

void NBString_concatUtf16Chars(STNBString* string, const UI16* str, const UI32 chars){
	UI8 surrExp = 0; UI32 unic = 0; char utf8[7];
	const UI16* strAfterEnd = str + chars;
	while(str < strAfterEnd){
		surrExp		= NBEncoding_utf16BytesExpected(*str) / 2;
		if(surrExp <= 0) break;
		unic		= NBEncoding_unicodeFromUtf16s(str, surrExp, '?');
		NBEncoding_utf8FromUnicode(unic, utf8);
		NBString_concat(string, utf8);
		str += surrExp;
	}
}

void NBString_concatUnicode(STNBString* string, const UI32* str){
	char utf8[7];
	while(*str != '\0'){
		NBEncoding_utf8FromUnicode(*str, utf8);
		NBString_concat(string, utf8);
		str++;
	}
}

void NBString_concatUnicodeChars(STNBString* string, const UI32* str, const UI32 chars){
	char utf8[7];
	const UI32* strAfterEnd = str + chars;
	while(str < strAfterEnd){
		NBEncoding_utf8FromUnicode(*str, utf8);
		NBString_concat(string, utf8);
		str++;
	}
}

//

void NBString_removeBytes(STNBString* string, const UI32 start, const UI32 numBytes){
	NBString_replaceBytes(string, start, numBytes, NULL, 0);
}

void NBString_removeFirstBytes(STNBString* string, const UI32 numBytes){
	if(numBytes < string->length){
		UI32 i; for(i = numBytes; i < string->length; i++){ //the '\0' is also moved
			string->str[i - numBytes] = string->str[i];
		}
		string->length -= numBytes;
	} else {
		string->length = 0;
		string->str[0] = '\0';
	}
}

void NBString_removeLastBytes(STNBString* string, const UI32 numBytes){
	if(string->length >= numBytes){
		string->str[string->length - numBytes] = '\0';
		string->length -= numBytes;
	} else {
		string->length = 0;
		string->str[0] = '\0';
	}
}

//

void NBString_replaceBytes(STNBString* string, const UI32 pStart, const UI32 pNumBytes, const char* newStr, const UI32 newStrLen){
	UI32 start = pStart, numBytes = pNumBytes;
	//Validate-range
	{
		if(start > string->length){
			start = string->length;
		}
		if((start + numBytes) > string->length){
			NBASSERT(start <= string->length)
			numBytes = string->length - start;
		}
	}
	//Replace action (depending on buffer modification)
	if(newStrLen < numBytes){
		//Length will decrease
		//Make room from left to right (reduce space)
		const UI32 delta = numBytes - newStrLen;
		char* c0 = &string->str[start + delta];
		char* c1 = &string->str[string->length];
		NBASSERT(c0 <= c1)
		while(c0 <= c1){
			*(c0 - delta) = *c0;
			c0++;
		}
		//Copy content
		NBASSERT((start + newStrLen + 1) <= string->_buffSz)
		if(newStr != NULL && newStrLen > 0){
			NBMemory_copy(&string->str[start], newStr, newStrLen);
		}
		//Update length
		string->length = string->length + newStrLen - numBytes;
	} else if(numBytes < newStrLen){
		//Length will increase
		const UI32 delta = newStrLen - numBytes;
		if((string->length + delta + 1) > (string->_buffSz)){
			//Optimization: create bigger buffer and copy new-content.
			string->_buffSz		= (string->length + delta + 1);
			char* newBuffer		= (char*) NBMemory_alloc(sizeof(char) * string->_buffSz);
			//Copy content
			{
				//Copy left-content
				if(string->str != NULL && start > 0){
					NBASSERT(start <= string->length)
					NBMemory_copy(newBuffer, string->str, start);
				}
				//Copy replacement
				if(newStr != NULL && newStrLen > 0){
					NBMemory_copy(&newBuffer[start], newStr, newStrLen);
				}
				//Copy right-content (including the '\0')
				NBASSERT((start + numBytes) <= string->length)
				if(string->str != NULL && (start + numBytes) <= string->length){
					NBMemory_copy(&newBuffer[start + newStrLen], &string->str[start + numBytes], (string->length - numBytes - start + 1)); //+1 to include the '\0'
					NBASSERT(newBuffer[start + newStrLen + (string->length - numBytes - start + 1) - 1] == '\0')
				}
			}
			//Delete prev buffer
			if(string->str != NULL && string->str != &_nbString_nullChar){
				NBMemory_free(string->str);
			}
			string->str = newBuffer;
			//Update length
			string->length = string->length + newStrLen - numBytes;
		} else {
			//Make room from right to left (increase buffer)
			char* c0 = &string->str[start + numBytes];
			char* c1 = &string->str[string->length];
			NBASSERT(c0 <= c1)
			while(c1 >= c0){
				*(c1 + delta) = *c1;
				c1--;
			}
			//Copy content
			NBASSERT((start + newStrLen + 1) <= string->_buffSz)
			if(newStr != NULL && newStrLen > 0){
				NBMemory_copy(&string->str[start], newStr, newStrLen);
			}
			//Update length
			string->length = string->length + newStrLen - numBytes;
		}
	} else {
		//Length will remain
		//Just copy replacement
		if(string->str != &_nbString_nullChar && newStr != NULL && newStrLen > 0){
			NBMemory_copy(&string->str[start], newStr, newStrLen);
		}
	}
	NBASSERT(string->str[string->length] == '\0')
}

//

UI32 NBString_lenBytes(const STNBString* string){
	return string->length;
}

UI32 NBString_lenUnicodes(const STNBString* string){
	return NBString_strLenUnicodes(string->str);
}

SI32 NBString_indexOf(const STNBString* string, const char* needle, SI32 posInitial){
	return NBString_strIndexOf(string->str, needle, posInitial);
}

SI32 NBString_indexOfLike(const STNBString* string, const char* needle, SI32 posInitial){
	return NBString_strIndexOfLike(string->str, needle, posInitial);
}

SI32 NBString_lastIndexOf(const STNBString* string, const char* needle, SI32 posInitialReverse){
	return NBString_strLastIndexOf(string->str, needle, posInitialReverse);
}

BOOL NBString_startsWith(const STNBString* string, const char* stringBase){
	return NBString_strStartsWith(string->str, stringBase);
}

BOOL NBString_isEqual(const STNBString* string, const char* string2){
	return NBString_strIsEqual(string->str, string2);
}

BOOL NBString_isEqualBytes(const STNBString* string, const char* string2, const UI32 len){
	BOOL r = FALSE;
	if(string->length == len){
		r = NBString_strIsEqualBytes(string->str, string->length, string2, len);
	}
	return r;
}

BOOL NBString_isLike(const STNBString* string, const char* string2){
	return NBString_strIsLike(string->str, string2);
}

BOOL NBString_isLikeBytes(const STNBString* string, const char* string2, const UI32 len){
	BOOL r = FALSE;
	if(string->length == len){
		r = NBString_strIsLikeBytes(string->str, string->length, string2, len);
	}
	return r;
}

BOOL NBString_isLower(const STNBString* string, const char* str2){
	return NBString_strIsLower(string->str, str2);
}

BOOL NBString_isLowerOrEqual(const STNBString* string, const char* str2){
	return NBString_strIsLowerOrEqual(string->str, str2);
}

BOOL NBString_isGreater(const STNBString* string, const char* str2){
	return NBString_strIsGreater(string->str, str2);
}

BOOL NBString_isGreaterOrEqual(const STNBString* string, const char* str2){
	return NBString_strIsGreaterOrEqual(string->str, str2);
}

//

char* NBString_strRepeatByte(char* dst, const char c, const UI32 times){
	UI32 i = 0; while(i < times) dst[i++] = c; dst[i] = '\0';
	return dst;
}

//

UI8 NBString_strIsInteger(const char* string){
	if((*string) == '+' || (*string) == '-') string++;
	while((*string) != '\0'){
		if((*string)<48 || (*string)>57) return 0; //ASCII 48='0' ... 57='9'
		string++;
	}
	return 1;
}

UI8 NBString_strIsIntegerBytes(const char* string, const SI32 bytesCount){
	const char* afterLast = (string + bytesCount);
	if((*string) == '+' || (*string) == '-') string++;
	while(string < afterLast){
		if((*string)<48 || (*string)>57) return 0; //ASCII 48='0' ... 57='9'
		string++;
	}
	return 1;
}

UI8 NBString_strIsDecimal(const char* string){
	if((*string) == '+' || (*string) == '-') string++;
	while((*string) != '\0'){
		if(((*string)<48 || (*string)>57) && (*string) != '.') return 0; //ASCII 48='0' ... 57='9'
		string++;
	}
	return 1;
}

SI32 NBString_strToSI32(const char* string){
	SI32 value = 0;
	const char sign = (*string); if(sign == '-' || sign == '+') string++;
	while((*string) != '\0'){
		value = (value * 10) + ((*string) - 48); //ASCII 48='0' ... 57='9'
		string++;
	}
	if(sign == '-') value = -value; //negativo
	return value;
}

SI32 NBString_strToSI32Bytes(const char* string, const SI32 bytesCount){
	const char* afterLast = (string + bytesCount);
	SI32 value = 0;
	const char sign = (*string); if(sign == '-' || sign == '+') string++;
	while(string < afterLast){
		value = (value * 10) + ((*string) - 48); //ASCII 48='0' ... 57='9'
		string++;
	}
	if(sign == '-') value = -value; //negativo
	return value;
}

SI64 NBString_strToSI64(const char* string){
	SI64 value = 0;
	const char sign = (*string); if(sign == '-' || sign == '+') string++;
	while((*string) != '\0'){
		value = (value * 10) + ((*string) - 48); //ASCII 48='0' ... 57='9'
		string++;
	}
	if(sign == '-') value = -value; //negativo
	return value;
}

UI32 NBString_strToUI32(const char* string){
	UI32 value = 0;
	const char sign = (*string); if(sign == '-' || sign == '+') string++;
	while((*string) != '\0'){
		value = (value * 10) + ((*string) - 48); //ASCII 48='0' ... 57='9'
		string++;
	}
	return value;
}

UI32 NBString_strToUI32Bytes(const char* string, const SI32 bytesCount){
	const char* afterLast = (string + bytesCount);
	UI32 value = 0;
	const char sign = (*string); if(sign == '-' || sign == '+') string++;
	while(string < afterLast){
		value = (value * 10) + ((*string) - 48); //ASCII 48='0' ... 57='9'
		string++;
	}
	return value;
}

UI64 NBString_strToUI64(const char* string){
	UI64 value = 0;
	const char sign = (*string);
	if(sign == '-' || sign == '+') string++;
	while((*string) != '\0'){
		value = (value * 10) + ((*string) - 48); //ASCII 48='0' ... 57='9'
		string++;
	}
	return value;
}

float NBString_strToFloat(const char* string){
	const char sign= (*string);
	float value	= 0;
	if(sign == '-' || sign == '+') string++;
	//Parte entera
	while((*string) != '\0' && (*string) != '.'){
		value = (value * 10.0f) + ((*string) - 48.0f); //ASCII 48='0' ... 57='9'
		string++;
	}
	//Parte decimal
	if((*string) == '.'){
		float factorDivisor = 10.0f;
		string++;
		while((*string) != '\0'){
			value += (((*string) - 48.0f) / factorDivisor);
			factorDivisor *= 10.0f;
			string++;
		}
	}
	//Signo
	if(sign == '-') value = -value;
	//
	return value;
}

double NBString_strToDouble(const char* string){
	const char sign = (*string);
	double value	= 0;
	if(sign == '-' || sign == '+') string++;
	//Parte entera
	while((*string) != '\0' && (*string) != '.'){
		value = (value * 10.0f) + ((*string) - '0'); //ASCII 48='0' ... 57='9'
		string++;
	}
	//Parte decimal
	if((*string) == '.'){
		double factorDivisor = 10.0;
		string++;
		while((*string) != '\0'){
			value += (((*string) - '0') / factorDivisor);
			factorDivisor *= 10.0;
			string++;
		}
	}
	//Signo
	if(sign == '-') value = -value;
	//
	return value;
}

double NBString_strToDoubleBytes(const char* string, const SI32 bytesCount){
	double value	= 0;
	const char* strAfterEnd = string + bytesCount;
	if(string < strAfterEnd){
		const char sign = (*string);
		if(sign == '-' || sign == '+') string++;
		//Parte entera
		while(string < strAfterEnd && *string != '\0' && (*string) != '.'){
			value = (value * 10.0f) + ((*string) - '0'); //ASCII 48='0' ... 57='9'
			string++;
		}
		//Parte decimal
		if((*string) == '.'){
			double factorDivisor = 10.0;
			string++;
			while(string < strAfterEnd && *string != '\0'){
				value += (((*string) - '0') / factorDivisor);
				factorDivisor *= 10.0;
				string++;
			}
		}
		//Signo
		if(sign == '-') value = -value;
	}
	return value;
}

long double NBString_strToLongDouble(const char* string){
	long double value	= 0;
	const char sign = (*string);
	if(sign == '-' || sign == '+') string++;
	//Parte entera
	while((*string) != '\0' && (*string) != '.'){
		value = (value * 10.0f) + ((*string) - '0'); //ASCII 48='0' ... 57='9'
		string++;
	}
	//Parte decimal
	if((*string) == '.'){
		long double factorDivisor = 10.0;
		string++;
		while((*string) != '\0'){
			value += (((*string) - '0') / factorDivisor);
			factorDivisor *= 10.0;
			string++;
		}
	}
	//Signo
	if(sign == '-') value = -value;
	//
	return value;
}

long double NBString_strToLongDoubleBytes(const char* string, const SI32 bytesCount){
	long double value	= 0;
	const char* strAfterEnd = string + bytesCount;
	if(string < strAfterEnd){
		const char sign = (*string);
		if(sign == '-' || sign == '+') string++;
		//Parte entera
		while(string < strAfterEnd && *string != '\0' && (*string) != '.'){
			value = (value * 10.0f) + ((*string) - '0'); //ASCII 48='0' ... 57='9'
			string++;
		}
		//Parte decimal
		if((*string) == '.'){
			long double factorDivisor = 10.0;
			string++;
			while(string < strAfterEnd && *string != '\0'){
				value += (((*string) - '0') / factorDivisor);
				factorDivisor *= 10.0;
				string++;
			}
		}
		//Signo
		if(sign == '-') value = -value;
	}
	return value;
}

SI32 NBString_strToSI32FromHex(const char* string){
	SI32 v = 0; char c;
	while((*string) != '\0'){
		c = (*string); string++;
		if(c > 47 && c < 58) v = (v * 16) + (c - 48); //ASCII 48='0' ... 57='9'
		else if(c > 64 && c < 71) v = (v * 16) + (10 + (c - 65)); //ASCII 65 = 'A' ... 70 = 'F'
		else if(c > 96 && c < 103) v = (v * 16) + (10 + (c - 97)); //ASCII 97 = 'a' ... 102 = 'f'
	}
	return v;
}

SI32 NBString_strToSI32FromHexLen(const char* string, const SI32 strLen){
	SI32 v = 0; SI32 pos = 0; char c;
	while(pos < strLen){
		c = string[pos++];
		if(c > 47 && c < 58) v = (v * 16) + (c - 48); //ASCII 48='0' ... 57='9'
		else if(c > 64 && c < 71) v = (v * 16) + (10 + (c - 65)); //ASCII 65 = 'A' ... 70 = 'F'
		else if(c > 96 && c < 103) v = (v * 16) + (10 + (c - 97)); //ASCII 97 = 'a' ... 102 = 'f'
	}
	return v;
}

SI64 NBString_strToBytes(const char* str){
	SI64 r = 0;
	if(str != NULL){
		const char* c = str;
		//Move after numeric part
		while(*c == '-' || *c == '+' || *c == '.' || (*c >= '0' && *c <= '9')){
			c++;
		}
		//Process
		if(str < c){
			//Validate sufix
			if(*c == '\0'){
				//success
				r = (SI64)NBString_strToLongDoubleBytes(str, (SI32)(c - str));
			} else if(*c == 'b' || *c == 'B' || *c == 'K' || *c == 'M' || *c == 'G' || *c == 'T'){
				long double dd = NBString_strToLongDoubleBytes(str, (SI32)(c - str));
				if(*c == 'b'){
					dd /= 8.L;
				} else if(*c == 'B'){
					//dd = dd;
				} else if(*c == 'K'){
					dd *= 1024.L;
				} else if(*c == 'M'){
					dd *= 1024.L * 1024.L;
				} else if(*c == 'G'){
					dd *= 1024.L * 1024.L * 1024.L;
				} else if(*c == 'T'){
					dd *= 1024.L * 1024.L * 1024.L * 1024.L;
				} else {
					NBASSERT(FALSE) //program logic
				}
				//Valida sufix2
				c++;	
				if(*c == '\0'){
					//sucess
					r = (SI64)dd;
				} else if(*c == 'b' || *c == 'B'){
					if(*c == 'b'){
						r = (SI64)(dd / 8.0L);
					} else if(*c == 'B'){
						r = (SI64)dd;
					}
				}
			}
		}
	}
	return r;
}

SI64 NBString_strToSecs(const char* str){
	SI64 r = 0;
	if(str != NULL){
		const char* c = str;
		//Move after numeric part
		while(*c == '-' || *c == '+' || *c == '.' || (*c >= '0' && *c <= '9')){
			c++;
		}
		//Process
		if(str < c){
			//Validate sufix
			if(*c == '\0'){
				//success
				r = (SI64)NBString_strToLongDoubleBytes(str, (SI32)(c - str));
			} else {
				//YYYY-MM-DD hh:mm:ss[.nnnnnnn]
				long double dd = NBString_strToLongDoubleBytes(str, (SI32)(c - str));
				if(NBString_strIsLike(c, "s") || NBString_strIsLike(c, "sec") || NBString_strIsLike(c, "secs")){
					r = (SI64)dd;
				} else if(NBString_strIsLike(c, "m") || NBString_strIsLike(c, "min") || NBString_strIsLike(c, "mins")){
					r = (SI64)(dd * 60.L);
				} else if(NBString_strIsLike(c, "h") || NBString_strIsLike(c, "hour") || NBString_strIsLike(c, "hours")){
					r = (SI64)(dd * 60.L * 60.L);
				} else if(NBString_strIsLike(c, "d") || NBString_strIsLike(c, "D") || NBString_strIsLike(c, "day") || NBString_strIsLike(c, "days")){
					r = (SI64)(dd * 60.L * 60.L * 24.L);
				}
			}
		}
	}
	return r;
}

//

SI32 NBString_strToSI32IfValid(const char* string, const SI32 valueDefault){
	SI32 value = 0;
	const char sign = (*string);
	if((*string) == '\0') return valueDefault;
	if(sign == '-' || sign == '+') string++;
	while((*string) != '\0'){
		if((*string)<48 || (*string)>57) return valueDefault;
		value = (value * 10) + ((*string) - 48); //ASCII 48='0' ... 57='9'
		string++;
	}
	if(sign == '-') value = -value; //negativo
	return value;
}

SI64 NBString_strToSI64IfValid(const char* string, const SI64 valueDefault){
	SI64 value = 0;
	const char sign = (*string);
	if((*string) == '\0') return valueDefault;
	if(sign == '-' || sign == '+') string++;
	while((*string) != '\0'){
		if((*string)<48 || (*string)>57) return valueDefault;
		value = (value * 10) + ((*string) - 48); //ASCII 48='0' ... 57='9'
		string++;
	}
	if(sign == '-') value = -value; //negativo
	return value;
}

UI32 NBString_strToUI32IfValid(const char* string, const UI32 valueDefault){
	UI32 value = 0; const char sign = (*string);
	if((*string) == '\0') return valueDefault;
	if(sign == '-' || sign == '+') string++;
	while((*string) != '\0'){
		if((*string)<48 || (*string)>57) return valueDefault;
		value = (value * 10) + ((*string) - 48); //ASCII 48='0' ... 57='9'
		string++;
	}
	return value;
}

UI64 NBString_strToUI64IfValid(const char* string, const UI64 valueDefault){
	UI64 value = 0;
	const char sign = (*string);
	if((*string) == '\0') return valueDefault;
	if(sign == '-' || sign == '+') string++;
	while((*string) != '\0'){
		if((*string)<48 || (*string)>57) return valueDefault;
		value = (value * 10) + ((*string) - 48); //ASCII 48='0' ... 57='9'
		string++;
	}
	return value;
}

float NBString_strToFloatIfValid(const char* string, const float valueDefault){
	const char sign= (*string);
	float value	= 0;
	if((*string) == '\0') return valueDefault;
	if(sign == '-' || sign == '+') string++;
	//Parte entera
	while((*string) != '\0' && (*string) != '.'){
		if(((*string)<48 || (*string)>57) && (*string) != '.') return valueDefault;
		value = (value * 10.0f) + ((*string) - 48.0f); //ASCII 48='0' ... 57='9'
		string++;
	}
	//Parte decimal
	if((*string) == '.'){
		float factorDivisor = 10.0f;
		string++;
		while((*string) != '\0'){
			if(((*string)<48 || (*string)>57) && (*string) != '.') return valueDefault;
			value += (((*string) - 48.0f) / factorDivisor);
			factorDivisor *= 10.0f;
			string++;
		}
	}
	//Signo
	if(sign == '-') value = -value;
	//
	return value;
}

double NBString_strToDoubleIfValid(const char* string, const double valueDefault){
	const char sign= (*string);
	double value	= 0;
	if((*string) == '\0') return valueDefault;
	if(sign == '-' || sign == '+') string++;
	//Parte entera
	while((*string) != '\0' && (*string) != '.'){
		if(((*string)<48 || (*string)>57) && (*string) != '.') return valueDefault;
		value = (value * 10.0f) + ((*string) - 48.0f); //ASCII 48='0' ... 57='9'
		string++;
	}
	//Parte decimal
	if((*string) == '.'){
		float factorDivisor = 10.0f;
		string++;
		while((*string) != '\0'){
			if(((*string)<48 || (*string)>57) && (*string) != '.') return valueDefault;
			value += (((*string) - 48.0f) / factorDivisor);
			factorDivisor *= 10.0f;
			string++;
		}
	}
	//Signo
	if(sign == '-') value = -value;
	//
	return value;
}

SI32 NBString_strToSI32FromHexIfValid(const char* string, const SI32 valueDefault){
	SI32 v = 0; char c;
	if((*string) == '\0') return valueDefault;
	while((*string) != '\0'){
		c = (*string); string++;
		if(c > 47 && c < 58) v = (v * 16) + (c - 48); //ASCII 48='0' ... 57='9'
		else if(c > 64 && c < 71) v = (v * 16) + (10 + (c - 65)); //ASCII 65 = 'A' ... 70 = 'F'
		else if(c > 96 && c < 103) v = (v * 16) + (10 + (c - 97)); //ASCII 97 = 'a' ... 102 = 'f'
		else return valueDefault;
	}
	return v;
}

SI32 NBString_strToSI32FromHexLenIfValid(const char* string, const SI32 strLen, const SI32 valueDefault){
	SI32 v = 0; SI32 pos = 0; char c;
	if(string[0] == '\0') return valueDefault;
	while(pos < strLen){
		c = string[pos++];
		if(c > 47 && c < 58) v = (v * 16) + (c - 48); //ASCII 48='0' ... 57='9'
		else if(c > 64 && c < 71) v = (v * 16) + (10 + (c - 65)); //ASCII 65 = 'A' ... 70 = 'F'
		else if(c > 96 && c < 103) v = (v * 16) + (10 + (c - 97)); //ASCII 97 = 'a' ... 102 = 'f'
		else return valueDefault;
	}
	return v;
}

//File

SI32 NBString_writeToFile(const STNBString* obj, STNBFileRef file, const BOOL includeUnusedBuffer){
	const SI32 verfiStart		= NB_STRING_VERIF_START, verifEnd = NB_STRING_VERIF_END;
	const SI32 incUnusedBuff	= (includeUnusedBuffer != FALSE ? 1 : 0);
	const SI32 bytesToWrite		= (includeUnusedBuffer != FALSE ? obj->_buffSz : (obj->length + 1));
	if(obj == NULL || !NBFile_isSet(file)){
	    NBASSERT(0); return -1;
	}
    NBASSERT(obj->length == 0 || obj->str != NULL)
	if(NBFile_write(file, &verfiStart, sizeof(verfiStart)) != sizeof(verfiStart)){
	    NBASSERT(0); return -1;
	}
	if(NBFile_write(file, &obj->length, sizeof(obj->length)) != sizeof(obj->length)){
	    NBASSERT(0); return -1;
	}
	if(NBFile_write(file, &incUnusedBuff, sizeof(incUnusedBuff)) != sizeof(incUnusedBuff)){
	    NBASSERT(0); return -1;
	}
	if(incUnusedBuff){
		if(NBFile_write(file, &obj->_buffSz, sizeof(obj->_buffSz)) != sizeof(obj->_buffSz)){
		    NBASSERT(0); return -1;
		}
	}
	if(NBFile_write(file, &obj->_buffGrowthMin, sizeof(obj->_buffGrowthMin)) != sizeof(obj->_buffGrowthMin)){
	    NBASSERT(0); return -1;
	}
	if(NBFile_write(file, &obj->_buffGrowthRel, sizeof(obj->_buffGrowthRel)) != sizeof(obj->_buffGrowthRel)){
		NBASSERT(0); return -1;
	}
	if(bytesToWrite > 0){
		if(NBFile_write(file, obj->str, bytesToWrite) != bytesToWrite){
		    NBASSERT(0); return -1;
		}
	}
	if(NBFile_write(file, &verifEnd, sizeof(verifEnd)) != sizeof(verifEnd)){
	    NBASSERT(0); return -1;
	}
	return 0;
}

SI32 NBString_initFromFile(STNBString* obj, STNBFileRef file){
	SI32 verfiStart, verifEnd, includesUnusedBuffer = 0; UI32 bytesToLoad = 0;
	if(obj == NULL || !NBFile_isSet(file)){
	    NBASSERT_LOADFILE(0); return -1;
	}
	if(NBFile_read(file, &verfiStart, sizeof(verfiStart)) != sizeof(verfiStart)){
	    NBASSERT_LOADFILE(0); return -1;
	} else if(verfiStart != NB_STRING_VERIF_START){
	    NBASSERT_LOADFILE(0); return -1;
	}
	if(NBFile_read(file, &obj->length, sizeof(obj->length)) != sizeof(obj->length)){
	    NBASSERT_LOADFILE(0); return -1;
	}
	if(NBFile_read(file, &includesUnusedBuffer, sizeof(includesUnusedBuffer)) != sizeof(includesUnusedBuffer)){
	    NBASSERT_LOADFILE(0); return -1;
	}
	if(includesUnusedBuffer){
		if(NBFile_read(file, &obj->_buffSz, sizeof(obj->_buffSz)) != sizeof(obj->_buffSz)){
		    NBASSERT_LOADFILE(0); return -1;
		}
		bytesToLoad = obj->_buffSz;
	} else {
		bytesToLoad = obj->length + 1;
	}
	if(NBFile_read(file, &obj->_buffGrowthMin, sizeof(obj->_buffGrowthMin)) != sizeof(obj->_buffGrowthMin)){
	    NBASSERT_LOADFILE(0); return -1;
	}
	if(NBFile_read(file, &obj->_buffGrowthRel, sizeof(obj->_buffGrowthRel)) != sizeof(obj->_buffGrowthRel)){
		NBASSERT_LOADFILE(0); return -1;
	}
	if(obj->_buffGrowthMin <= 0){
	    NBASSERT_LOADFILE(0); return -1;
	}
    NBASSERT(bytesToLoad > 0)
	if(bytesToLoad == 1){
		obj->str		= &_nbString_nullChar;
	} else {
		obj->str		= (char*)NBMemory_alloc(obj->_buffSz);
	}
	obj->_buffSz		= bytesToLoad;
	if(obj->str == NULL){
	    NBASSERT_LOADFILE(0); return -1;
	}
	if(bytesToLoad > 0){
		if(NBFile_read(file, obj->str, bytesToLoad) != bytesToLoad){
			if(obj->str != &_nbString_nullChar){
		    	NBMemory_free(obj->str);
			}
		    NBASSERT_LOADFILE(0); return -1;
		}
	}
	obj->str[obj->length] = '\0';
	if(NBFile_read(file, &verifEnd, sizeof(verifEnd)) != sizeof(verifEnd)){
		if(obj->str != &_nbString_nullChar){
	    	NBMemory_free(obj->str);
		}
	    NBASSERT_LOADFILE(0); return -1;
	} else if(verifEnd != NB_STRING_VERIF_END){
		if(obj->str != &_nbString_nullChar){
	    	NBMemory_free(obj->str);
		}
	    NBASSERT_LOADFILE(0); return -1;
	}
	return 0;
}

#ifdef NB_CONFIG_INCLUDE_ASSERTS
#	include <stdlib.h>	//for rand() 
#endif

#ifdef NB_CONFIG_INCLUDE_ASSERTS
void NBString_dbgTestRemoveAndReplace(const UI32 totalTests, const UI32 ciclesPerTest, const UI8 printLevel){
	UI32 i, j;
	STNBString strTest, str2, str3;
	NBString_init(&strTest);
	NBString_init(&str2);
	NBString_init(&str3);
	//
	const UI32 origMaxLen = 512;
	const UI32 replcMaxLen = 512;
	const char* charsPosibs = "0123456789";
	const UI32 charsPosibsSz = NBString_strLenBytes(charsPosibs);
	const char* charsPosibs2 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	const UI32 charsPosibs2Sz = NBString_strLenBytes(charsPosibs2);
	//Tests
	for(i = 0; i < totalTests; i++){
		//Random reset string (to reset buffer)
		if((rand() % 100) < 2){
			NBString_release(&strTest);
			NBString_initWithSz(&strTest, (rand() % 100), 1 + (rand() % 7), 1.5f);
		}
		//Set initial value
		{
			UI32 i; const UI32 charsCount = (rand() % origMaxLen);
			NBString_empty(&strTest);
			for(i = 0; i < charsCount; i++){
				NBString_concatByte(&strTest, charsPosibs[(rand() % charsPosibsSz)]);
			}
		}
		//ciclesPerTest
		for(j = 0; j < ciclesPerTest; j++){
			if((rand() % 2) == 0){
				const UI32 orgLen = strTest.length;
				//Test remove
				UI32 start = 0, size = 0;
				if(strTest.length > 0){
					start = (rand() % strTest.length) + (rand() % ((strTest.length < 5 ? 5 : strTest.length) / 5));
					size = (rand() % (strTest.length * 5) / 4);
				} else {
					start = (rand() % 3);
					size = (rand() % 3);
				}
				//Build compare
				{
					NBString_empty(&str2);
					if(start > 0){
						NBString_concatBytes(&str2, &strTest.str[0], (start < strTest.length ? start : strTest.length));
					}
					if((start + size) < strTest.length){
						NBString_concatBytes(&str2, &strTest.str[start + size], strTest.length - start - size);
					}
				}
				//Replace
				if(printLevel > 1){
					PRINTF_INFO("-------------\n");
					PRINTF_INFO("Orig: %s.\n", strTest.str);
					PRINTF_INFO("Epct: %s.\n", str2.str);
				}
				if(printLevel > 0){
					PRINTF_INFO("#%d-test-#%d-cicle REMOVE(%d, +%d) on %d bytes%s%s%s.\n", (i + 1), (j + 1), start, size, orgLen, (start >= orgLen ? " start-out-of-rng" : ""), (start + size) == orgLen ? " remove-at-end" : "", (start + size) > orgLen ? " size-out-of-rng" : "");
				}
				NBString_removeBytes(&strTest, start, size);
				if(printLevel > 1){
					PRINTF_INFO("Rslt: %s.\n", strTest.str);
				}
				//Compare
				NBASSERT(strTest.length == str2.length)
				NBASSERT(NBString_strIsEqual(strTest.str, str2.str));
			} else {
				//Test replace
				const UI32 orgLen = strTest.length;
				//Test remove
				UI32 start = 0, size = 0;
				if(strTest.length > 0){
					start = (rand() % strTest.length) + (rand() % ((strTest.length < 5 ? 5 : strTest.length) / 5));
					size = (rand() % (strTest.length * 5) / 4);
				} else {
					start = (rand() % 3);
					size = (rand() % 3);
				}
				//Set replace value
				{
					UI32 i; const UI32 charsCount = (rand() % replcMaxLen);
					NBString_empty(&str3);
					for(i = 0; i < charsCount; i++){
						NBString_concatByte(&str3, charsPosibs2[(rand() % charsPosibs2Sz)]);
					}
				}
				//Build compare
				{
					NBString_empty(&str2);
					if(start > 0){
						NBString_concatBytes(&str2, &strTest.str[0], (start < strTest.length ? start : strTest.length));
					}
					NBString_concatBytes(&str2, str3.str, str3.length);
					if((start + size) < strTest.length){
						NBString_concatBytes(&str2, &strTest.str[start + size], strTest.length - start - size);
					}
				}
				//Replace
				if(printLevel > 1){
					PRINTF_INFO("-------------\n");
					PRINTF_INFO("Rplc: %s.\n", str3.str);
					PRINTF_INFO("Orig: %s.\n", strTest.str);
					PRINTF_INFO("Epct: %s.\n", str2.str);
				}
				if(printLevel > 0){
					PRINTF_INFO("#%d-test-#%d-cicle REPLACE(%d, +%d, with +%d [%s]) on %d bytes%s%s%s.\n", (i + 1), (j + 1), start, size, str3.length, (str3.length < size ? "reduce" : str3.length > size ? "growth" : "same"), orgLen, (start >= orgLen ? " start-out-of-rng" : ""), (start + size) == orgLen ? " remove-at-end" : "", (start + size) > orgLen ? " size-out-of-rng" : "");
				}
				NBString_replaceBytes(&strTest, start, size, str3.str, str3.length);
				if(printLevel > 1){
					PRINTF_INFO("Rslt: %s.\n", strTest.str);
				}
				//Compare
				NBASSERT(strTest.length == str2.length)
				NBASSERT(NBString_strIsEqual(strTest.str, str2.str));
			}
		}
	}
	NBString_release(&str3);
	NBString_release(&str2);
	NBString_release(&strTest);
}
#endif
