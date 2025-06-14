//
//  XUXml.c
//  XUMonitorLogicaNegocios_OSX
//
//  Created by Marcos Ortega on 21/01/14.
//  Copyright (c) 2014 XurpasLatam. All rights reserved.
//

#include "nb/NBFrameworkPch.h"
#include "nb/core/NBMemory.h"
#include "nb/core/NBJson.h"
#include "nb/core/NBJsonParser.h"
#include "nb/core/NBNumParser.h"
#include "nb/core/NBCompare.h"

#define NBJSON_VAL_NUMERIC_SIGNED(DST, STR_VALUE, TYPE)	\
{ \
	const UI32 firstChar	= (STR_VALUE[0] == '-' || STR_VALUE[0] == '+' ? 1 : 0); /*exclude sign char*/ \
	const BOOL isNeg		= (STR_VALUE[0] == '-'); \
	const STNBNumParser num = NBNumParser_strParseUnsigned(&STR_VALUE[firstChar]); \
	if(!num.isErr){ \
		switch (num.typeSub) { \
			case ENNumericTypeSub_Int: DST = (TYPE)(isNeg ? -num.valInt : num.valInt); break; \
			case ENNumericTypeSub_IntU: DST = (TYPE)(isNeg ? -num.valIntU : num.valIntU); break; \
			case ENNumericTypeSub_Long: DST = (TYPE)(isNeg ? -num.valLong : num.valLong); break; \
			case ENNumericTypeSub_LongU: DST = (TYPE)(isNeg ? -num.valLongU : num.valLongU); break; \
			case ENNumericTypeSub_LongLong: DST = (TYPE)(isNeg ? -num.valLongLong : num.valLongLong); break; \
			case ENNumericTypeSub_LongLongU: DST = (TYPE)(isNeg ? -num.valLongLongU : num.valLongLongU); break; \
			case ENNumericTypeSub_Float: DST = (TYPE)(isNeg ? -num.valFloat : num.valFloat); break; \
			case ENNumericTypeSub_Double: DST = (TYPE)(isNeg ? -num.valDouble : num.valDouble); break; \
			case ENNumericTypeSub_DoubleLong: DST = (TYPE)(isNeg ? -num.valDoubleLong : num.valDoubleLong); break; \
			default: \
			NBASSERT(FALSE) /*Inextpected subtype*/ \
			break; \
		} \
	} \
} \

#define NBJSON_VAL_NUMERIC_UNSIGNED(DST, STR_VALUE, TYPE)	\
{ \
	const UI32 firstChar	= (STR_VALUE[0] == '-' || STR_VALUE[0] == '+' ? 1 : 0); /*Exclude sign char*/ \
	const STNBNumParser num = NBNumParser_strParseUnsigned(&STR_VALUE[firstChar]); \
	if(!num.isErr){ \
		switch (num.typeSub) { \
			case ENNumericTypeSub_Int: DST = (TYPE)num.valInt; break; \
			case ENNumericTypeSub_IntU: DST = (TYPE)num.valIntU; break; \
			case ENNumericTypeSub_Long: DST = (TYPE)num.valLong; break; \
			case ENNumericTypeSub_LongU: DST = (TYPE)num.valLongU; break; \
			case ENNumericTypeSub_LongLong: DST = (TYPE)num.valLongLong; break; \
			case ENNumericTypeSub_LongLongU: DST = (TYPE)num.valLongLongU; break; \
			case ENNumericTypeSub_Float: DST = (TYPE)num.valFloat; break; \
			case ENNumericTypeSub_Double: DST = (TYPE)num.valDouble; break; \
			case ENNumericTypeSub_DoubleLong: DST = (TYPE)num.valDoubleLong; break; \
			default: \
			NBASSERT(FALSE) /*Inextpected subtype*/ \
			break; \
		} \
	} \
}

//Private

void NBJson_privNodeInit(STNBJsonNode* nodo);
void NBJson_privNodeRelease(STNBJsonNode* nodo);
void NBJson_privNodeEmpty(STNBJsonNode* nodo);

//

BOOL NBCompare_STNBJsonNode(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
	NBASSERT(dataSz == sizeof(STNBJsonNode))
	if(dataSz == sizeof(STNBJsonNode)){
		const STNBJsonNode* d1 = (const STNBJsonNode*)data1;
		const STNBJsonNode* d2 = (const STNBJsonNode*)data2;
		switch (mode) {
			case ENCompareMode_Equal:
				return (d1->hintDeepLvl == d2->hintDeepLvl && d1->hintChildIdx == d2->hintChildIdx && d1->iNameStart == d2->iNameStart && d1->iValueStart == d2->iValueStart && d1->type == d2->type && d1->valueType == d2->valueType /*&& d1->childn == d2->childn*/);
			case ENCompareMode_Lower:
				NBASSERT(FALSE)
				break;
			case ENCompareMode_LowerOrEqual:
				NBASSERT(FALSE)
				break;
			case ENCompareMode_Greater:
				NBASSERT(FALSE)
				break;
			case ENCompareMode_GreaterOrEqual:
				NBASSERT(FALSE)
				break;
			default:
				NBASSERT(FALSE)
				break;
		}
	}
	return FALSE;
}

//

void NBJson_init(STNBJson* obj){
	NBString_initWithSz(&obj->_strTags, 256, 256, 0.10f);
    NBString_initWithSz(&obj->_strVals, 1024, 4096, 0.10f);
    NBArray_initWithSz(&obj->_idxsTags, sizeof(UI32), NBCompareUI32, 128, 512, 0.5f);
	//
	NBString_concatByte(&obj->_strTags, (char)'\0');
    NBString_concatByte(&obj->_strVals, (char)'\0');
    NBJson_privNodeInit(&obj->_rootMmbr);
	obj->_rootMmbr.type = ENJsonNodeTypee_Undef;
}

void NBJson_release(STNBJson* obj){
    NBJson_privNodeEmpty(&obj->_rootMmbr);
    NBJson_privNodeRelease(&obj->_rootMmbr);
	//
    NBArray_release(&obj->_idxsTags);
    NBString_release(&obj->_strVals);
    NBString_release(&obj->_strTags);
}

//Prepare buffer for parsing (optimization)

void NBJson_prepareStorageTags(STNBJson* obj, const UI32 strTotalSize){
	STNBString str;
	NBString_initWithSz(&str, strTotalSize, 256, 0.10f);
	NBString_concatBytes(&str, obj->_strTags.str, obj->_strTags.length);
	NBString_swapContent(&str, &obj->_strTags);
	NBASSERT(obj->_strTags._buffSz == strTotalSize);
	NBString_release(&str);
}

void NBJson_prepareStorageValues(STNBJson* obj, const UI32 strTotalSize){
	STNBString str;
	NBString_initWithSz(&str, strTotalSize, 4096, 0.10f);
	NBString_concatBytes(&str, obj->_strVals.str, obj->_strVals.length);
	NBString_swapContent(&str, &obj->_strVals);
	NBASSERT(obj->_strVals._buffSz == strTotalSize);
	NBString_release(&str);
}

// Scape
/*
\"
\\
\/
\b
\f
\n
\r
\t
\u four-hex-digits*/

void NBJson_concatScaped(STNBString* dst, const char* unscaped){
	const char* c = unscaped;
	while(*c != '\0'){
		switch(*c){
			case '\b': NBString_concatBytes(dst, "\\b", 2); break;
			case '\f': NBString_concatBytes(dst, "\\f", 2); break;
			case '\n': NBString_concatBytes(dst, "\\n", 2); break;
			case '\r': NBString_concatBytes(dst, "\\r", 2); break;
			case '\t': NBString_concatBytes(dst, "\\t", 2); break;
			case '\"': NBString_concatBytes(dst, "\\\"", 2); break;
			case '\\': NBString_concatBytes(dst, "\\\\", 2); break;
			//case '/': NBString_concatBytes(dst, "\\/", 2); break;
			//ToDo: four-digit-hex
			default: NBString_concatByte(dst, *c); break;
		}
		c++;
	}
}

void NBJson_concatScapedDQuotesOnly(STNBString* dst, const char* unscaped){
	const char* c = unscaped;
	while(*c != '\0'){
		if(*c == '\"'){
			NBString_concatBytes(dst, "\\\"", 2);
		} else {
			NBString_concatByte(dst, *c);
		}
		c++;
	}
}



void NBJson_concatScapedBytes(STNBString* dst, const char* unscaped, const UI32 unscapedSz){
	const char* afterEnd = (unscaped + unscapedSz);
	const char* c = unscaped;
	while(c < afterEnd){
		switch(*c){
			case '\b': NBString_concatBytes(dst, "\\b", 2); break;
			case '\f': NBString_concatBytes(dst, "\\f", 2); break;
			case '\n': NBString_concatBytes(dst, "\\n", 2); break;
			case '\r': NBString_concatBytes(dst, "\\r", 2); break;
			case '\t': NBString_concatBytes(dst, "\\t", 2); break;
			case '\"': NBString_concatBytes(dst, "\\\"", 2); break;
			case '\\': NBString_concatBytes(dst, "\\\\", 2); break;
			//case '/': NBString_concatBytes(dst, "\\/", 2); break;
			//ToDo: four-digit-hex
			default: NBString_concatByte(dst, *c); break;
		}
		c++;
	}
}

void NBJson_concatScapedBytesDQuotesOnly(STNBString* dst, const char* unscaped, const UI32 unscapedSz){
	const char* afterEnd = (unscaped + unscapedSz);
	const char* c = unscaped;
	while(c < afterEnd){
		if(*c == '\"'){
			NBString_concatBytes(dst, "\\\"", 2);
		} else {
			NBString_concatByte(dst, *c);
		}
		c++;
	}
}

void NBJson_concatUnscaped(STNBString* dst, const char* scaped){
	const char* c = scaped;
	while(*c != '\0'){
		if(*c == '\\'){
			c++;
			if(*c != '\0'){
				switch(*c){
					case 'b': NBString_concatByte(dst, '\b'); break;
					case 'f': NBString_concatByte(dst, '\f'); break;
					case 'n': NBString_concatByte(dst, '\n'); break;
					case 'r': NBString_concatByte(dst, '\r'); break;
					case 't': NBString_concatByte(dst, '\t'); break;
					case '"': NBString_concatByte(dst, '\"'); break;
					case '\\': NBString_concatByte(dst, '\\'); break;
					//case '/': NBString_concatByte(dst, '/'); break;
					//ToDo: four-digit-hex
					default: NBString_concatByte(dst, '\\'); NBString_concatByte(dst, *c); break;
				}
				c++;
			}
		} else {
		    NBString_concatByte(dst, *c);
			c++;
		}
	}
}

//

const STNBJsonNode* NBJson_rootMember(STNBJson* obj){
	return &obj->_rootMmbr;
}

BOOL NBJson_nodeIsNull(const STNBJson* obj, const STNBJsonNode* node){
	NBASSERT(node != NULL)
	NBASSERT(node->iValueStart >= 0 && node->iValueStart <= obj->_strVals.length)
	const char* v = (&obj->_strVals.str[node->iValueStart]);
	return (node->valueType == ENJsonNodeValueType_Plain && NBString_strIsEqual(v, "null") ? TRUE : FALSE);
}

const char* NBJson_nodeName(const STNBJson* obj, const STNBJsonNode* node){
    NBASSERT(node->iNameStart >= 0 && node->iNameStart < obj->_strTags.length)
	return (&obj->_strTags.str[node->iNameStart]);
}

const char* NBJson_nodeStr(const STNBJson* obj, const STNBJsonNode* node, const char* defvalue){
	const char* r = NULL;
    NBASSERT(node != NULL)
    NBASSERT(node->iValueStart >= 0 && node->iValueStart <= obj->_strVals.length)
	if(node->valueType == ENJsonNodeValueType_Literal){
		r = &obj->_strVals.str[node->iValueStart];
	} else if(node->valueType == ENJsonNodeValueType_Plain){
		const char* v = &obj->_strVals.str[node->iValueStart];
		if(!NBString_strIsEqual(v, "null")){
			r = v;
		}
	}
	return r;
}

char NBJson_nodeChar(const STNBJson* obj, const STNBJsonNode* node, const char defvalue){
	char r = defvalue;
	if(obj->_strVals.str[node->iValueStart] != '\0'){
		r = obj->_strVals.str[node->iValueStart];
	}
	return r;
}

SI8 NBJson_nodeSI8(const STNBJson* obj, const STNBJsonNode* node, const SI8 defvalue){
	SI8 r = defvalue;
	const char* strVal = &obj->_strVals.str[node->iValueStart];
	NBJSON_VAL_NUMERIC_SIGNED(r, strVal, SI8)
	return r;
}

UI8 NBJson_nodeUI8(const STNBJson* obj, const STNBJsonNode* node, const UI8 defvalue){
	UI8 r = defvalue;
	const char* strVal = &obj->_strVals.str[node->iValueStart];
	NBJSON_VAL_NUMERIC_SIGNED(r, strVal, UI8)
	return r;
}

SI16 NBJson_nodeSI16(const STNBJson* obj, const STNBJsonNode* node, const SI16 defvalue){
	SI16 r = defvalue;
	const char* strVal = &obj->_strVals.str[node->iValueStart];
	NBJSON_VAL_NUMERIC_SIGNED(r, strVal, SI16)
	return r;
}

UI16 NBJson_nodeUI16(const STNBJson* obj, const STNBJsonNode* node, const UI16 defvalue){
	UI16 r = defvalue;
	const char* strVal = &obj->_strVals.str[node->iValueStart];
	NBJSON_VAL_NUMERIC_SIGNED(r, strVal, UI16)
	return r;
}

SI32 NBJson_nodeSI32(const STNBJson* obj, const STNBJsonNode* node, const SI32 defvalue){
	SI32 r = defvalue;
	const char* strVal = &obj->_strVals.str[node->iValueStart];
	NBJSON_VAL_NUMERIC_SIGNED(r, strVal, SI32)
	return r;
}

UI32 NBJson_nodeUI32(const STNBJson* obj, const STNBJsonNode* node, const UI32 defvalue){
	UI32 r = defvalue;
	const char* strVal = &obj->_strVals.str[node->iValueStart];
	NBJSON_VAL_NUMERIC_SIGNED(r, strVal, UI32)
	return r;
}

SI64 NBJson_nodeSI64(const STNBJson* obj, const STNBJsonNode* node, const SI64 defvalue){
	SI64 r = defvalue;
	const char* strVal = &obj->_strVals.str[node->iValueStart];
	NBJSON_VAL_NUMERIC_SIGNED(r, strVal, SI64)
	return r;
}

UI64 NBJson_nodeUI64(const STNBJson* obj, const STNBJsonNode* node, const UI64 defvalue){
	UI64 r = defvalue;
	const char* strVal = &obj->_strVals.str[node->iValueStart];
	NBJSON_VAL_NUMERIC_SIGNED(r, strVal, UI64)
	return r;
}

BOOL NBJson_nodeBOOL(const STNBJson* obj, const STNBJsonNode* node, const BOOL defvalue){
	BOOL r = defvalue;
	const char* val = &obj->_strVals.str[node->iValueStart];
	if(NBString_strIsEqual(val, "false")){
		r = FALSE;
	} else if(NBString_strIsEqual(val, "true")){
		r = TRUE;
	}
	return r;
}

float NBJson_nodeFloat(const STNBJson* obj, const STNBJsonNode* node, const float defvalue){
	float r = defvalue;
	const char* strVal = &obj->_strVals.str[node->iValueStart];
	NBJSON_VAL_NUMERIC_SIGNED(r, strVal, float)
	return r;
}

double NBJson_nodeDouble(const STNBJson* obj, const STNBJsonNode* node, const double defvalue){
	double r = defvalue;
	const char* strVal = &obj->_strVals.str[node->iValueStart];
	NBJSON_VAL_NUMERIC_SIGNED(r, strVal, double)
	return r;
}

//--------------
// Nodes childn
//--------------

const STNBJsonNode* NBJson_childNode(STNBJson* obj, const char* name, const STNBJsonNode* parent, const STNBJsonNode* afterThis){
	STNBJsonNode* nodoEcontrado = NULL;
	if(parent == NULL) parent = &obj->_rootMmbr;
	if(parent->childn != NULL){
		//establecer el inicio de la busqueda
		UI32 i = 0, iNodoConteo = parent->childn->use;
		if(afterThis != NULL){
		    NBASSERT(afterThis->hintChildIdx<parent->childn->use);
		    NBASSERT((STNBJsonNode*)NBArray_itemAtIndex(parent->childn, afterThis->hintChildIdx) == afterThis);
			i = afterThis->hintChildIdx + 1;
		}
		//busqueda por name
		for(; i < iNodoConteo; i++){
			STNBJsonNode* nodoHijo = (STNBJsonNode*)NBArray_itemAtIndex(parent->childn, i);
			if(NBString_strIsEqual(&obj->_strTags.str[nodoHijo->iNameStart], name)){
				nodoHijo->hintChildIdx = i; //para optimizar cuando esta salida sea brindada como parametro de entrada
				nodoEcontrado = nodoHijo;
				break;
			}
		}
	}
	return nodoEcontrado;
}

const STNBJsonNode* NBJson_childNodeAfter(STNBJson* obj, const STNBJsonNode* parent, const STNBJsonNode* afterThis){
	STNBJsonNode* nodoEcontrado = NULL;
	if(parent == NULL) parent = &obj->_rootMmbr;
		if(parent->childn != NULL){
			//establecer el inicio de la busqueda
			if(afterThis != NULL){
				if((afterThis->hintChildIdx+1)<parent->childn->use){
				    NBASSERT(afterThis->hintChildIdx>=0 && afterThis->hintChildIdx<parent->childn->use)
				    NBASSERT((STNBJsonNode*)NBArray_itemAtIndex(parent->childn, afterThis->hintChildIdx) == afterThis)
					nodoEcontrado = (STNBJsonNode*)NBArray_itemAtIndex(parent->childn, afterThis->hintChildIdx + 1); //afterThis = NULL;
					nodoEcontrado->hintChildIdx = afterThis->hintChildIdx + 1; //para optimizar cuando esta salida sea brindada como parametro de entrada
				}
			} else {
				if(parent->childn->use != 0){
					nodoEcontrado = (STNBJsonNode*)NBArray_itemAtIndex(parent->childn, 0); //para optimizar cuando esta salida sea brindada como parametro de entrada
					nodoEcontrado->hintChildIdx = 0;
				}
			}
		}
	return nodoEcontrado;
}

const char* NBJson_childStr(STNBJson* obj, const char* name, const char* defValue, const STNBJsonNode* parent, const STNBJsonNode* afterThis){
	const char* r = defValue;
	const STNBJsonNode* nodoTmp = NBJson_childNode(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		if(nodoTmp->valueType == ENJsonNodeValueType_Literal){
			r = &obj->_strVals.str[nodoTmp->iValueStart];
		} else if(nodoTmp->valueType == ENJsonNodeValueType_Plain){
			const char* v = &obj->_strVals.str[nodoTmp->iValueStart];
			if(!NBString_strIsEqual(v, "null")){
				r = v;
			}
		}
	}
	return r;
}

char NBJson_childChar(STNBJson* obj, const char* name, char defValue, const STNBJsonNode* parent, const STNBJsonNode* afterThis){
	char r = defValue;
	const STNBJsonNode* nodoTmp = NBJson_childNode(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		if(obj->_strVals.str[nodoTmp->iValueStart] != '\0'){
			r = obj->_strVals.str[nodoTmp->iValueStart];
		}
	}
	return r;
}

SI8 NBJson_childSI8(STNBJson* obj, const char* name, SI8 defValue, const STNBJsonNode* parent, const STNBJsonNode* afterThis){
	SI8 r = defValue;
	const STNBJsonNode* nodoTmp = NBJson_childNode(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		const char* strVal = &obj->_strVals.str[nodoTmp->iValueStart];
		NBJSON_VAL_NUMERIC_SIGNED(r, strVal, SI8)
	}
	return r;
}

UI8 NBJson_childUI8(STNBJson* obj, const char* name, UI8 defValue, const STNBJsonNode* parent, const STNBJsonNode* afterThis){
	UI8 r = defValue;
	const STNBJsonNode* nodoTmp = NBJson_childNode(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		const char* strVal = &obj->_strVals.str[nodoTmp->iValueStart];
		NBJSON_VAL_NUMERIC_UNSIGNED(r, strVal, UI8)
	}
	return r;
}

SI16 NBJson_childSI16(STNBJson* obj, const char* name, SI16 defValue, const STNBJsonNode* parent, const STNBJsonNode* afterThis){
	SI16 r = defValue;
	const STNBJsonNode* nodoTmp = NBJson_childNode(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		const char* strVal = &obj->_strVals.str[nodoTmp->iValueStart];
		NBJSON_VAL_NUMERIC_SIGNED(r, strVal, SI16)
	}
	return r;
}

UI16 NBJson_childUI16(STNBJson* obj, const char* name, UI16 defValue, const STNBJsonNode* parent, const STNBJsonNode* afterThis){
	UI16 r = defValue;
	const STNBJsonNode* nodoTmp = NBJson_childNode(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		const char* strVal = &obj->_strVals.str[nodoTmp->iValueStart];
		NBJSON_VAL_NUMERIC_UNSIGNED(r, strVal, UI16)
	}
	return r;
}

SI32 NBJson_childSI32(STNBJson* obj, const char* name, SI32 defValue, const STNBJsonNode* parent, const STNBJsonNode* afterThis){
	SI32 r = defValue;
	const STNBJsonNode* nodoTmp = NBJson_childNode(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		const char* strVal = &obj->_strVals.str[nodoTmp->iValueStart];
		NBJSON_VAL_NUMERIC_SIGNED(r, strVal, SI32)
	}
	return r;
}

UI32 NBJson_childUI32(STNBJson* obj, const char* name, UI32 defValue, const STNBJsonNode* parent, const STNBJsonNode* afterThis){
	UI32 r = defValue;
	const STNBJsonNode* nodoTmp = NBJson_childNode(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		const char* strVal = &obj->_strVals.str[nodoTmp->iValueStart];
		NBJSON_VAL_NUMERIC_UNSIGNED(r, strVal, UI32)
	}
	return r;
}

SI64 NBJson_childSI64(STNBJson* obj, const char* name, SI64 defValue, const STNBJsonNode* parent, const STNBJsonNode* afterThis){
	SI64 r = defValue;
	const STNBJsonNode* nodoTmp = NBJson_childNode(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		const char* strVal = &obj->_strVals.str[nodoTmp->iValueStart];
		NBJSON_VAL_NUMERIC_SIGNED(r, strVal, SI64)
	}
	return r;
}

UI64 NBJson_childUI64(STNBJson* obj, const char* name, UI64 defValue, const STNBJsonNode* parent, const STNBJsonNode* afterThis){
	UI64 r = defValue;
	const STNBJsonNode* nodoTmp = NBJson_childNode(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		const char* strVal = &obj->_strVals.str[nodoTmp->iValueStart];
		NBJSON_VAL_NUMERIC_UNSIGNED(r, strVal, UI64)
	}
	return r;
}

BOOL NBJson_childBOOL(STNBJson* obj, const char* name, BOOL defValue, const STNBJsonNode* parent, const STNBJsonNode* afterThis){
	BOOL r = defValue;
	const STNBJsonNode* nodoTmp = NBJson_childNode(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		const char* val = &obj->_strVals.str[nodoTmp->iValueStart];
		if(NBString_strIsEqual(val, "false")){
			r = FALSE;
		} else if(NBString_strIsEqual(val, "true")){
			r = TRUE;
		}
	}
	return r;
}

float NBJson_childFloat(STNBJson* obj, const char* name, float defValue, const STNBJsonNode* parent, const STNBJsonNode* afterThis){
	float r = defValue;
	const STNBJsonNode* nodoTmp = NBJson_childNode(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		const char* strVal = &obj->_strVals.str[nodoTmp->iValueStart];
		NBJSON_VAL_NUMERIC_SIGNED(r, strVal, float)
	}
	return r;
}

double NBJson_childDouble(STNBJson* obj, const char* name, double defValue, const STNBJsonNode* parent, const STNBJsonNode* afterThis){
	double r = defValue;
	const STNBJsonNode* nodoTmp = NBJson_childNode(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		const char* strVal = &obj->_strVals.str[nodoTmp->iValueStart];
		NBJSON_VAL_NUMERIC_SIGNED(r, strVal, double)
	}
	return r;
}

//Write

void NBJson_concatValue_(const STNBJson* obj, const STNBJsonNode* node, const char padChar, const UI32 depthLvl, STNBString* dst);
void NBJson_concatChildrenNamed_(const STNBJson* obj, const STNBJsonNode* node, const char padChar, const UI32 depthLvl, STNBString* dst);
void NBJson_concatChildrenUnnamed_(const STNBJson* obj, const STNBJsonNode* node, const char padChar, const UI32 depthLvl, STNBString* dst);

void NBJson_concatValue_(const STNBJson* obj, const STNBJsonNode* node, const char padChar, const UI32 depthLvl, STNBString* dst){
	SI32 lenStartGrp, lenStartVal; BOOL isMultiLine = FALSE;
	switch (node->type) {
		case ENJsonNodeTypee_Pair:
			switch(node->valueType){
				case ENJsonNodeValueType_Plain:
					NBString_concat(dst, &obj->_strVals.str[node->iValueStart]);
					break;
				case ENJsonNodeValueType_Literal:
					NBString_concatByte(dst, '\"'); NBJson_concatScaped(dst, &obj->_strVals.str[node->iValueStart]); NBString_concatByte(dst, '\"');
					break;
				default:
					NBASSERT(FALSE)
					break;
			}
			break;
		case ENJsonNodeTypee_Object:
			NBString_concatByte(dst, '{');
			lenStartGrp = dst->length;
			isMultiLine	= (node->childn != NULL && node->childn->use > 1);
			if(isMultiLine){
				NBString_concatByte(dst, '\n');
				if(padChar != '\0'){
					NBString_concatRepeatedByte(dst, padChar, depthLvl + 1);
				}
			}
			{
				lenStartVal = dst->length;
				NBJson_concatChildrenNamed_(obj, node, padChar, depthLvl + 1, dst);
			}
			if(lenStartVal == dst->length){
				//remove extra spaces and empty-lines
				NBString_removeLastBytes(dst, dst->length - lenStartGrp);
				NBString_concat(dst, " }");
			} else {
				if(isMultiLine){
					NBString_concatByte(dst, '\n');
					if(padChar != '\0' && depthLvl > 0){
						NBString_concatRepeatedByte(dst, padChar, depthLvl);
					}
				}
				NBString_concatByte(dst, '}');
			}
			break;
		case ENJsonNodeTypee_Array:
			NBString_concatByte(dst, '[');
			lenStartGrp = dst->length;
			isMultiLine	= (node->childn != NULL && node->childn->use > 1);
			if(isMultiLine){
				NBString_concatByte(dst, '\n');
				if(padChar != '\0'){
					NBString_concatRepeatedByte(dst, padChar, depthLvl + 1);
				}
			}
			{
				lenStartVal = dst->length;
				NBJson_concatChildrenUnnamed_(obj, node, padChar, depthLvl + 1, dst);
			}
			if(lenStartVal == dst->length){
				//remove extra spaces and empty-lines
				NBString_removeLastBytes(dst, dst->length - lenStartGrp);
				NBString_concat(dst, " ]");
			} else {
				if(isMultiLine){
					NBString_concatByte(dst, '\n');
					if(padChar != '\0' && depthLvl > 0){
						NBString_concatRepeatedByte(dst, padChar, depthLvl);
					}
				}
				NBString_concatByte(dst, ']');
			}
			break;	
		default:
			NBASSERT(FALSE)
			break;
	}
}

void NBJson_concatChildrenNamed_(const STNBJson* obj, const STNBJsonNode* node, const char padChar, const UI32 depthLvl, STNBString* dst){
	if(node->childn != NULL){
		SI32 i; for(i = 0; i < node->childn->use; i++){
			const STNBJsonNode* child = NBArray_itmPtrAtIndex(node->childn, STNBJsonNode, i);
			const char* name = &obj->_strTags.str[child->iNameStart];
			//NBASSERT(name[0] != '\0') //named
			//separator
			if(i != 0){
				NBString_concatByte(dst, '\n');
				if(padChar != '\0' && depthLvl > 0){
					NBString_concatRepeatedByte(dst, padChar, depthLvl);
				}
				NBString_concat(dst, ", ");
			}
			//name
			NBString_concatByte(dst, '\"'); NBJson_concatScaped(dst, name); NBString_concatByte(dst, '\"');
			NBString_concat(dst, ": ");
			//value
			NBJson_concatValue_(obj, child, padChar, depthLvl, dst);
		}
	}
}

void NBJson_concatChildrenUnnamed_(const STNBJson* obj, const STNBJsonNode* node, const char padChar, const UI32 depthLvl, STNBString* dst){
	if(node->childn != NULL){
		SI32 i; for(i = 0; i < node->childn->use; i++){
			const STNBJsonNode* child = NBArray_itmPtrAtIndex(node->childn, STNBJsonNode, i);
			//separator
			if(i != 0){
				NBString_concatByte(dst, '\n');
				if(padChar != '\0' && depthLvl > 0){
					NBString_concatRepeatedByte(dst, padChar, depthLvl);
				}
				NBString_concat(dst, ", ");
			}
			//value
			NBJson_concatValue_(obj, child, padChar, depthLvl, dst);
		}
	}
}

void NBJson_concat(const STNBJson* obj, const char padChar, STNBString* dst){
	NBJson_concatValue_(obj, &obj->_rootMmbr, padChar, 0, dst);
}

//private

void NBJson_privNodeInit(STNBJsonNode* nodo){
	NBMemory_setZeroSt(*nodo, STNBJsonNode);
}

void NBJson_privNodeRelease(STNBJsonNode* nodo){
	if(nodo != NULL){
		nodo->iNameStart		= 0;
		nodo->iValueStart		= 0;
		nodo->type				= ENJsonNodeTypee_Undef;
		nodo->valueType			= ENJsonNodeValueType_Undef;
		if(nodo->childn != NULL){
		    NBArray_release(nodo->childn);
		    NBMemory_free(nodo->childn);
			nodo->childn	= NULL;
		}
		//
		nodo->hintDeepLvl		= 0;
		nodo->hintChildIdx		= 0;
	}
}

void NBJson_privNodeEmpty(STNBJsonNode* nodo){
	if(nodo != NULL){
		if(nodo->childn != NULL){
			while(nodo->childn->use>0){
				STNBJsonNode* nodoHijo = (STNBJsonNode*)NBArray_itemAtIndex(nodo->childn, nodo->childn->use - 1);
				//Optiminzacion #1 (evitar 1 llamado recursivo)
				if(nodoHijo->childn != NULL){
					while(nodoHijo->childn->use>0){
						STNBJsonNode* nodoNieto = (STNBJsonNode*)NBArray_itemAtIndex(nodoHijo->childn, nodoHijo->childn->use - 1);
						//Optimizacion #2 (evitar otro llamado recursivo)
						if(nodoNieto->childn != NULL){
							while(nodoNieto->childn->use>0){
								STNBJsonNode* nodoBisNieto = (STNBJsonNode*)NBArray_itemAtIndex(nodoNieto->childn, nodoNieto->childn->use - 1);
							    NBJson_privNodeEmpty(nodoBisNieto);
							    NBJson_privNodeRelease(nodoBisNieto);
							    NBArray_removeItemAtIndex(nodoNieto->childn, nodoNieto->childn->use - 1);
							}
						}
					    NBJson_privNodeRelease(nodoNieto);
					    NBArray_removeItemAtIndex(nodoHijo->childn, nodoHijo->childn->use - 1);
					}
				}
			    NBJson_privNodeRelease(nodoHijo);
			    NBArray_removeItemAtIndex(nodo->childn, nodo->childn->use - 1);
			}
		}
	}
}

//------------------
// JSON load
//------------------

typedef struct STNBJsonLoad_ {
	BOOL				plainDataStarted;
	BOOL				plainDataIgnoreCurrent;
	STNBJsonNode*		rootNodePtr;	//Necesary to add to listaLIFO as pointer
	STNBArray			listaLIFO;	//STNBJsonNode*
	STNBJsonParser		state;
	IJsonParserListener	listener;
	STNBJson*			obj;
} STNBJsonLoad;

//Private

UI32 NBJson_privIdxForTag(STNBJson* obj, const char* tagName);
void NBJson_privJsonLoadingOpenNewNode(const STNBJsonParser* state, void* listenerParam);
void NBJson_privJsonLoadingRemoveLastNode(const STNBJsonParser* state, void* listenerParam);

//----------------------
// Json parser callbacks
//----------------------

void NBJson_memberStarted(const STNBJsonParser* state, void* listenerParam){
	//PRINTF_INFO("NBJson_memberStarted\n");
    NBJson_privJsonLoadingOpenNewNode(state, listenerParam);
}

void NBJson_consumeName(const STNBJsonParser* state, const char* unscapedName, void* listenerParam){
	//PRINTF_INFO("NBJson_consumeName('%s')\n", unscapedName);
	//PRINTF_INFO("JSON, node's name: '%s'.\n", name);
	STNBJsonLoad* loadState = (STNBJsonLoad*)listenerParam;
	STNBArray* listaLIFO = &loadState->listaLIFO;
    NBASSERT(listaLIFO->use > 0)
	if(listaLIFO->use > 0){
		STNBJsonNode* curNode = *((STNBJsonNode**)NBArray_itemAtIndex(listaLIFO, listaLIFO->use - 1));
	    NBASSERT(curNode->iNameStart == 0)
		if(curNode->iNameStart == 0){
			curNode->iNameStart = NBJson_privIdxForTag(loadState->obj, unscapedName);
		}
	}
}

void NBJson_consumePlain(const STNBJsonParser* state, const char* data, const SI32 dataSize, void* listenerParam){
	//PRINTF_INFO("NBJson_consumePlain: '%s'\n", data);
	//PRINTF_INFO("JSON, plain value: %d bytes.\n", dataSize);
	STNBJsonLoad* loadState = (STNBJsonLoad*)listenerParam;
	STNBArray* listaLIFO = &loadState->listaLIFO;
    NBASSERT(listaLIFO->use > 0)
	if(listaLIFO->use > 0){
		STNBJson* obj = loadState->obj;
		STNBJsonNode* curNode = *((STNBJsonNode**)NBArray_itemAtIndex(listaLIFO, listaLIFO->use - 1));
	    NBASSERT(curNode->type == ENJsonNodeTypee_Undef || curNode->type == ENJsonNodeTypee_Pair)
	    NBASSERT(curNode->valueType == ENJsonNodeValueType_Undef || curNode->valueType == ENJsonNodeValueType_Plain)
		if(curNode->type == ENJsonNodeTypee_Undef){
			curNode->type = ENJsonNodeTypee_Pair;
			curNode->valueType = ENJsonNodeValueType_Plain;
			//Start content
		    NBASSERT(curNode->iValueStart == 0)
			if(curNode->iValueStart == 0){
			    NBString_concatByte(&obj->_strVals, '\0');
				curNode->iValueStart = obj->_strVals.length;
			    NBString_concatBytes(&obj->_strVals, data, dataSize);
			}
		} else if( curNode->type == ENJsonNodeTypee_Pair){
		    NBASSERT(curNode->valueType == ENJsonNodeValueType_Plain)
			if(curNode->valueType == ENJsonNodeValueType_Plain){
				//Continue content
			    NBASSERT(curNode->iValueStart != 0)
				if(curNode->iValueStart != 0){
				    NBString_concatBytes(&obj->_strVals, data, dataSize);
				}
			}
		}
	}
}

void NBJson_consumeLiteral(const STNBJsonParser* state, const char* unscapedData, const SI32 dataSize, void* listenerParam){
	//PRINTF_INFO("NBJson_consumeLiteral: '%s'\n", unscapedData);
	//PRINTF_INFO("JSON, literal value: %d bytes.\n", dataSize);
	STNBJsonLoad* loadState = (STNBJsonLoad*)listenerParam;
	STNBArray* listaLIFO = &loadState->listaLIFO;
    NBASSERT(listaLIFO->use > 0)
	if(listaLIFO->use > 0){
		STNBJson* obj = loadState->obj;
		STNBJsonNode* curNode = *((STNBJsonNode**)NBArray_itemAtIndex(listaLIFO, listaLIFO->use - 1));
	    NBASSERT(curNode->type == ENJsonNodeTypee_Undef || curNode->type == ENJsonNodeTypee_Pair)
	    NBASSERT(curNode->valueType == ENJsonNodeValueType_Undef || curNode->valueType == ENJsonNodeValueType_Literal)
		if(curNode->type == ENJsonNodeTypee_Undef){
			curNode->type = ENJsonNodeTypee_Pair;
			curNode->valueType = ENJsonNodeValueType_Literal;
			//Start content
		    NBASSERT(curNode->iValueStart == 0)
			if(curNode->iValueStart == 0){
			    NBString_concatByte(&obj->_strVals, '\0');
				curNode->iValueStart = obj->_strVals.length;
			    NBString_concatBytes(&obj->_strVals, unscapedData, dataSize);
			}
		} else if( curNode->type == ENJsonNodeTypee_Pair){
		    NBASSERT(curNode->valueType == ENJsonNodeValueType_Literal)
			if(curNode->valueType == ENJsonNodeValueType_Literal){
				//Continue content
			    NBASSERT(curNode->iValueStart != 0)
				if(curNode->iValueStart != 0){
				    NBString_concatBytes(&obj->_strVals, unscapedData, dataSize);
				}
			}
		}
	}
}

void NBJson_objectStarted(const STNBJsonParser* state, void* listenerParam){
	//PRINTF_INFO("NBJson_objectStarted\n");
	STNBJsonLoad* loadState = (STNBJsonLoad*)listenerParam;
	STNBArray* listaLIFO = &loadState->listaLIFO;
    NBASSERT(listaLIFO->use > 0)
	if(listaLIFO->use > 0){
		STNBJsonNode* curNode = *((STNBJsonNode**)NBArray_itemAtIndex(listaLIFO, listaLIFO->use - 1));
	    NBASSERT(curNode->type == ENJsonNodeTypee_Undef)
		if(curNode->type == ENJsonNodeTypee_Undef){
			curNode->type = ENJsonNodeTypee_Object;
		}
	}
}

void NBJson_objectEnded(const STNBJsonParser* state, void* listenerParam){
	//PRINTF_INFO("NBJson_objectEnded\n");
	STNBJsonLoad* loadState = (STNBJsonLoad*)listenerParam;
	STNBArray* listaLIFO = &loadState->listaLIFO;
    NBASSERT(listaLIFO->use > 0)
	if(listaLIFO->use > 0){
		IF_NBASSERT(STNBJsonNode* curNode = *((STNBJsonNode**)NBArray_itemAtIndex(listaLIFO, listaLIFO->use - 1));)
	    NBASSERT(curNode->type == ENJsonNodeTypee_Object)
	}
}

void NBJson_arrayStarted(const STNBJsonParser* state, void* listenerParam){
	//PRINTF_INFO("NBJson_arrayStarted\n");
	STNBJsonLoad* loadState = (STNBJsonLoad*)listenerParam;
	STNBArray* listaLIFO = &loadState->listaLIFO;
    NBASSERT(listaLIFO->use > 0)
	if(listaLIFO->use > 0){
		STNBJsonNode* curNode = *((STNBJsonNode**)NBArray_itemAtIndex(listaLIFO, listaLIFO->use - 1));
	    NBASSERT(curNode->type == ENJsonNodeTypee_Undef)
		if(curNode->type == ENJsonNodeTypee_Undef){
			curNode->type = ENJsonNodeTypee_Array;
		}
	}
}

void NBJson_arrayEnded(const STNBJsonParser* state, void* listenerParam){
	//PRINTF_INFO("NBJson_arrayEnded\n");
	STNBJsonLoad* loadState = (STNBJsonLoad*)listenerParam;
	STNBArray* listaLIFO = &loadState->listaLIFO;
    NBASSERT(listaLIFO->use > 0)
	if(listaLIFO->use > 0){
		IF_NBASSERT(STNBJsonNode* curNode = *((STNBJsonNode**)NBArray_itemAtIndex(listaLIFO, listaLIFO->use - 1));)
	    NBASSERT(curNode->type == ENJsonNodeTypee_Array)
	}
}

void NBJson_memberEnded(const STNBJsonParser* state, void* listenerParam){
	//PRINTF_INFO("NBJson_memberEnded\n");
    NBJson_privJsonLoadingRemoveLastNode(state, listenerParam);
}

//----------------------
// JSON load stream
//----------------------

BOOL NBJson_loadStreamStart(STNBJsonLoad* state, STNBJson* obj){
    NBJson_privNodeEmpty(&obj->_rootMmbr);
    NBJson_privNodeRelease(&obj->_rootMmbr);
    NBJson_privNodeInit(&obj->_rootMmbr);
	obj->_rootMmbr.type = ENJsonNodeTypee_Undef;
	//
	state->plainDataStarted			= FALSE;
	state->plainDataIgnoreCurrent	= FALSE;
	//
	state->rootNodePtr				= &obj->_rootMmbr;
    NBArray_init(&state->listaLIFO, sizeof(STNBJsonNode*), NULL);
    NBArray_addValue(&state->listaLIFO, state->rootNodePtr);
	//parser
    NBJsonParser_init(&state->state);
	//lstnr itf
	{
		NBMemory_setZeroSt(state->listener, IJsonParserListener);
		state->listener.memberStarted	= NBJson_memberStarted;
		state->listener.consumeName		= NBJson_consumeName;
		state->listener.consumePlain	= NBJson_consumePlain;
		state->listener.consumeLiteral	= NBJson_consumeLiteral;
		state->listener.objectStarted	= NBJson_objectStarted;
		state->listener.objectEnded		= NBJson_objectEnded;
		state->listener.arrayStarted	= NBJson_arrayStarted;
		state->listener.arrayEnded		= NBJson_arrayEnded;
		state->listener.memberEnded		= NBJson_memberEnded;
		state->obj						= obj;
	}
	return NBJsonParser_feedStart(&state->state, &state->listener, state);
}

UI32 NBJson_loadStreamFeed(STNBJsonLoad* state, const char* data, const SI32 dataSize){
	UI32 r = 0;
    NBASSERT(state != NULL)
	if(state != NULL){
		r = NBJsonParser_feed(&state->state, data, dataSize, &state->listener, state);
#       ifdef NBASSERT
        if(state->state.errFnd){
            if(state->state.bytesFeed > 0){
                PRINTF_ERROR("NBJson, %d bytes stream could not be parsed as a valid json.\n", state->state.bytesFeed);
                if(state->listaLIFO.use > 0){
                    const STNBJsonNode* lastNode = (const STNBJsonNode*)NBArray_itemAtIndex(&state->listaLIFO, state->listaLIFO.use - 1);
                    if(lastNode->iNameStart < state->obj->_strTags.length){
                        PRINTF_ERROR("NBJson, name of last node parsed: '%s'.\n", &state->obj->_strTags.str[lastNode->iNameStart]);
                    }
                    if(state->listaLIFO.use > 1){
                        const STNBJsonNode* parentLastNode = (const STNBJsonNode*)NBArray_itemAtIndex(&state->listaLIFO, state->listaLIFO.use - 2);
                        if(parentLastNode->iNameStart < state->obj->_strTags.length){
                            PRINTF_ERROR("NBJson, name of parent of last node parsed: '%s'.\n", &state->obj->_strTags.str[parentLastNode->iNameStart]);
                        }
                    }
                }
            }
            if(state->state.errDesc.length > 0){
                PRINTF_ERROR("NBJson, parsing err-desc: '%s'.\n", state->state.errDesc.str);
            }
        }
#       endif
	}
	return r;
}

BOOL NBJson_loadStreamEnd(STNBJsonLoad* state){
	BOOL r = FALSE;
	if(state != NULL){
		if(!NBJsonParser_feedEnd(&state->state, &state->listener, state)){
#           ifdef NBASSERT
			if(state->state.bytesFeed > 0){
				PRINTF_ERROR("NBJson, %d bytes stream could not be parsed as a valid json.\n", state->state.bytesFeed);
				if(state->listaLIFO.use > 0){
					const STNBJsonNode* lastNode = (const STNBJsonNode*)NBArray_itemAtIndex(&state->listaLIFO, state->listaLIFO.use - 1);
					if(lastNode->iNameStart < state->obj->_strTags.length){
						PRINTF_ERROR("NBJson, name of last node parsed: '%s'.\n", &state->obj->_strTags.str[lastNode->iNameStart]);
					}
					if(state->listaLIFO.use > 1){
						const STNBJsonNode* parentLastNode = (const STNBJsonNode*)NBArray_itemAtIndex(&state->listaLIFO, state->listaLIFO.use - 2);
						if(parentLastNode->iNameStart < state->obj->_strTags.length){
							PRINTF_ERROR("NBJson, name of parent of last node parsed: '%s'.\n", &state->obj->_strTags.str[parentLastNode->iNameStart]);
						}
					}
				}
			}
            if(state->state.errDesc.length > 0){
                PRINTF_ERROR("NBJson, parsing err-desc: '%s'.\n", state->state.errDesc.str);
            }
#           endif
		} else {
			//PRINTF_INFO("Lista lifo tiene %d elementos.\n", _loadState->listaLIFO->use);
			//NBASSERT(_loadState->listaLIFO->use == 1)
			r = TRUE;
		}
		NBJsonParser_release(&state->state);
	    NBArray_release(&state->listaLIFO);
	}
	return r;
}

//----------------------
// Json load from file
//----------------------

BOOL NBJson_loadFromFilePath(STNBJson* obj, const char* filePath){
	BOOL r = FALSE;
    STNBFileRef stream = NBFile_alloc(NULL);
	if(!NBFile_open(stream, filePath, ENNBFileMode_Read)){
		PRINTF_ERROR("NBJson, could not open file: '%s'\n", filePath);
	} else {
		NBFile_lock(stream);
		if(!NBJson_loadFromFile(obj, stream)){
			PRINTF_ERROR("NBJson, could not load file: '%s'\n", filePath);
		} else {
			r = TRUE;
		}
		NBFile_unlock(stream);
	}
	NBFile_release(&stream);
	return r;
}

BOOL NBJson_loadFromFile(STNBJson* obj, STNBFileRef flujoArchivo){
	BOOL r = FALSE; NBASSERT(NBFile_isSet(flujoArchivo))
	//
	STNBJsonLoad state;
    NBMemory_setZeroSt(state, STNBJsonLoad);
	if(NBJson_loadStreamStart(&state, obj)){
		char buff[1024];
		SI32 read = 0, csmd = 0, totalRead = 0;
		r = TRUE;
		do {
			read = NBFile_read(flujoArchivo, buff, sizeof(buff));
			totalRead += read;
			if(read <= 0){
				break;
			} else if((csmd = NBJson_loadStreamFeed(&state, buff, read)) != read){
				r = FALSE;
				break;
			}
		} while(TRUE);
		//print
		if(!r && read > 0){
			SI32 iLast = 0;
			//'...' at start
			if(read < sizeof(buff)){
				buff[read] = '\0';
				iLast	= (read - 1); 
			} else {
				buff[sizeof(buff) - 1] = '\0';
				iLast	= sizeof(buff) - 2;
			}
			//'...' at end
			if(csmd > 3){
				buff[0] = buff[1] = buff[2] = '.';
			} else if(csmd > 2){
				buff[0] = buff[1] = '.';
			}
			if(iLast > 3){
				buff[iLast] = buff[iLast - 1] = buff[iLast - 2] = '.';
			} else if(iLast > 2){
				buff[iLast] = buff[iLast - 1] = '.';
			}
			PRINTF_ERROR("NBJson, only %d of %d bytes consumed at: '%s'.\n", csmd, read, buff);
		}
		//Finish stream
		if(r){
			r = NBJson_loadStreamEnd(&state);
		}
	}
	//
	return r;
}

// Json load from string

BOOL NBJson_loadFromStr(STNBJson* obj, const char* strData){
	//Determine data size
	UI32 strSz = 0; while(strData[strSz] != '\0') strSz++;
	return NBJson_loadFromStrBytes(obj, strData, strSz);
}

BOOL NBJson_loadFromStrBytes(STNBJson* obj, const char* strData, const UI32 strSz){
	BOOL r = FALSE;
	if(strSz > 0){
		STNBJsonLoad state; UI32 csmd = 0;
        NBMemory_setZeroSt(state, STNBJsonLoad);
		if(NBJson_loadStreamStart(&state, obj)){
			if((csmd = NBJson_loadStreamFeed(&state, strData, strSz)) == strSz){
				//Finish stream
				if(NBJson_loadStreamEnd(&state)){
					r = TRUE;
				}
			}
		}
	}
	return r;
}

// Private

UI32 NBJson_privIdxForTag(STNBJson* obj, const char* tagName){
	UI32 indiceEtiqueta = 0;
	SI32 iEtiq, iEtiqConteo = obj->_idxsTags.use;
	for(iEtiq=0; iEtiq<iEtiqConteo; iEtiq++){
		if(NBString_strIsEqual(&obj->_strTags.str[*(UI32*)NBArray_itemAtIndex(&obj->_idxsTags, iEtiq)], tagName)){
			//Etiqueta ya esta registrada
			indiceEtiqueta = *(UI32*)NBArray_itemAtIndex(&obj->_idxsTags, iEtiq);
			break;
		}
	}
	if(indiceEtiqueta == 0){
		//Primera vez que aparece la etiqueta (registrar)
		indiceEtiqueta = obj->_strTags.length;
	    NBArray_addValue(&obj->_idxsTags, indiceEtiqueta);
	    NBString_concat(&obj->_strTags, tagName);
	    NBString_concatByte(&obj->_strTags, '\0');
	}
    NBASSERT(indiceEtiqueta != 0)
	return indiceEtiqueta;
}

void NBJson_privJsonLoadingOpenNewNode(const STNBJsonParser* state, void* listenerParam){
	STNBJsonLoad* loadState = (STNBJsonLoad*)listenerParam;
	STNBArray* listaLIFO = &loadState->listaLIFO;
	STNBJson* obj = loadState->obj;
	//PRINTF_INFO("JSON, opening new node.\n");
	//Determine parent
	STNBJsonNode* parent = NULL;
	if(listaLIFO->use == 0){
		parent = &obj->_rootMmbr;
	} else {
		parent = *((STNBJsonNode**)NBArray_itemAtIndex(listaLIFO, listaLIFO->use - 1));
	}
    NBASSERT(parent != NULL)
	//If parent is an Array, use his name.
	UI32 nameIdx = 0;
	if(parent->type == ENJsonNodeTypee_Array){
		nameIdx = parent->iNameStart;
	}
	//New node
	STNBJsonNode nuevoNodo;
    NBJson_privNodeInit(&nuevoNodo);
	nuevoNodo.iNameStart		= nameIdx;
	nuevoNodo.iValueStart		= 0;
	nuevoNodo.type				= ENJsonNodeTypee_Undef;
	nuevoNodo.valueType			= ENJsonNodeValueType_Undef;
	nuevoNodo.childn			= NULL;
	nuevoNodo.hintDeepLvl 		= listaLIFO->use;
	//Add to root or parent
	{
	    NBASSERT(parent->type == ENJsonNodeTypee_Array || parent->type == ENJsonNodeTypee_Object)
		if(parent->type == ENJsonNodeTypee_Array || parent->type == ENJsonNodeTypee_Object){
			if(parent->childn == NULL){
				parent->childn = (STNBArray*)NBMemory_alloc(sizeof(STNBArray));
			    NBArray_init(parent->childn, sizeof(STNBJsonNode), NBCompare_STNBJsonNode);
			}
			//Add to parent
		    NBArray_addValue(parent->childn, nuevoNodo);
			//Add to loading stack
			STNBJsonNode* nodePtr = (STNBJsonNode*)NBArray_itemAtIndex(parent->childn, parent->childn->use - 1);
		    NBArray_addValue(listaLIFO, nodePtr);
		} else {
		    NBASSERT(FALSE)
		}
	}
}

void NBJson_privJsonLoadingRemoveLastNode(const STNBJsonParser* state, void* listenerParam){
	STNBJsonLoad* loadState = (STNBJsonLoad*)listenerParam;
	STNBArray* listaLIFO = &loadState->listaLIFO;
	STNBJson* obj = loadState->obj;
	//Remove node if is undefined
	if(listaLIFO->use > 0){
		STNBJsonNode* lastNode = *((STNBJsonNode**)NBArray_itemAtIndex(listaLIFO, listaLIFO->use - 1));
		if(lastNode->type == ENJsonNodeTypee_Undef){
			//TODO: optimization, avoid adding undefined nodes to tree.
			//if(lastNode->valueType != ENJsonNodeValueType_Undef){
			//	PRINTF_INFO("Undefined type node removed (value type: %d).\n", (int)lastNode->valueType);
			//}
			STNBJsonNode* parent = NULL;
			if(listaLIFO->use <= 1){
				parent = &obj->_rootMmbr;
			} else {
				parent = *(STNBJsonNode**)NBArray_itemAtIndex(listaLIFO, listaLIFO->use - 2);
			}
		    NBASSERT(parent != NULL)
		    NBASSERT(parent->childn != NULL)
			if(parent->childn != NULL){
				const STNBJsonNode lastNodeCopy = *lastNode;
				const SI32 itmIdx = NBArray_indexOf(parent->childn, &lastNodeCopy, sizeof(lastNodeCopy));
			    NBASSERT(itmIdx != -1)
				if(itmIdx != -1){
				    NBArray_removeItemAtIndex(parent->childn, itmIdx);
					if(parent->childn->use == 0){
					    NBArray_release(parent->childn);
					    NBMemory_free(parent->childn);
						parent->childn = NULL;
					}
				}
			}
		}
	}
	//Remove last object from stack
    NBArray_removeItemAtIndex(listaLIFO, listaLIFO->use - 1);
	//
	loadState->plainDataStarted = FALSE;
	loadState->plainDataIgnoreCurrent = FALSE;
}

