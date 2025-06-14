//
//  XUXml.c
//  XUMonitorLogicaNegocios_OSX
//
//  Created by Marcos Ortega on 21/01/14.
//  Copyright (c) 2014 XurpasLatam. All rights reserved.
//

#include "nb/NBFrameworkPch.h"
#include "nb/core/NBMemory.h"
#include "nb/core/NBXml.h"
#include "nb/core/NBNumParser.h"
#include "nb/core/NBCompare.h"

#define NBXML_VAL_NUMERIC_SIGNED(DST, STR_VALUE, TYPE)	\
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

#define NBXML_VAL_NUMERIC_UNSIGNED(DST, STR_VALUE, TYPE)	\
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

void NBXml_privNodeInit(STNBXmlNode* nodo);
void NBXml_privNodeRelease(STNBXmlNode* nodo);
void NBXml_privNodeEmpty(STNBXmlNode* nodo);

//

void NBXml_init(STNBXml* obj){
    NBString_init(&obj->_strNames);
	NBArray_initWithSz(&obj->_idxsNames, sizeof(UI32), NBCompareUI32, 128, 512, 0.5f);
	NBString_init(&obj->_strAttrVals);
    NBString_init(&obj->_strContent);
	//
    NBString_concatByte(&obj->_strNames, (char)'\0');	//Index zero is empty string
    NBXml_privNodeInit(&obj->_docNode);
	//
	obj->loadState	= NULL;
}

void NBXml_initWithSizes(STNBXml* obj, const UI32 allTagsNamesSz, const UI32 allAttribsValuesSz, const UI32 contentSz){
	NBString_initWithSz(&obj->_strNames, allTagsNamesSz, 512, 1.5f);
	NBArray_initWithSz(&obj->_idxsNames, sizeof(UI32), NBCompareUI32, 128, 512, 0.5f);
	NBString_initWithSz(&obj->_strAttrVals, allAttribsValuesSz, 512, 1.5f);
	NBString_initWithSz(&obj->_strContent, contentSz, 512, 1.5f);
	//
	NBString_concatByte(&obj->_strNames, (char)'\0');	//Index zero is empty string
	NBXml_privNodeInit(&obj->_docNode);
	//
	obj->loadState	= NULL;
}

void NBXml_initWithContentSz(STNBXml* obj, const UI32 size){
	NBString_init(&obj->_strNames);
	NBArray_initWithSz(&obj->_idxsNames, sizeof(UI32), NBCompareUI32, 128, 512, 0.5f);
	NBString_init(&obj->_strAttrVals);
	NBString_initWithSz(&obj->_strContent, size, 512, 1.5f);
	//
	NBString_concatByte(&obj->_strNames, (char)'\0');	//Index zero is empty string
	NBXml_privNodeInit(&obj->_docNode);
	//
	obj->loadState	= NULL;
}

void NBXml_release(STNBXml* obj){
    NBXml_privNodeEmpty(&obj->_docNode);
    NBXml_privNodeRelease(&obj->_docNode);
	//
    NBString_release(&obj->_strContent);
	NBString_release(&obj->_strAttrVals);
	NBArray_release(&obj->_idxsNames);
    NBString_release(&obj->_strNames);
	//
	if(obj->loadState != NULL){
		NBArray_release(&obj->loadState->listaLIFO);
		NBXmlParser_release(&obj->loadState->parser);
		obj->loadState->obj = NULL;
		NBMemory_free(obj->loadState);
		obj->loadState = NULL;
	}
}

void NBXml_concatScaped(STNBString* dst, const char* unscaped){
	const char* c = unscaped;
	while(*c != '\0'){
		switch(*c){
			case '<': NBString_concatBytes(dst, "&lt;", 4); break;
			case '>': NBString_concatBytes(dst, "&gt;", 4); break;
			case '&': NBString_concatBytes(dst, "&amp;", 5); break;
			case '\'': NBString_concatBytes(dst, "&apos;", 6); break;
			case '\"': NBString_concatBytes(dst, "&quot;", 6); break;
			default: NBString_concatByte(dst, *c); break;
		}
		c++;
	}
}

void NBXml_concatScapedBytes(STNBString* dst, const char* unscaped, const UI32 sz){
	const char* c = unscaped;
	const char* cAfterLast = unscaped + sz;
	while(c < cAfterLast){
		NBASSERT(*c != '\0') //Not allowed
		switch(*c){
			case '<': NBString_concatBytes(dst, "&lt;", 4); break;
			case '>': NBString_concatBytes(dst, "&gt;", 4); break;
			case '&': NBString_concatBytes(dst, "&amp;", 5); break;
			case '\'': NBString_concatBytes(dst, "&apos;", 6); break;
			case '\"': NBString_concatBytes(dst, "&quot;", 6); break;
			default: NBString_concatByte(dst, *c); break;
		}
		c++;
	}
}


/*void NBXml_concatUnscaped(STNBString* dst, const char* scaped){
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
					case '/': NBString_concatByte(dst, '/'); break;
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
}*/

//

const char* NBXml_nodeName(STNBXml* obj, const STNBXmlNode* node){
	NBASSERT(node != NULL)
    NBASSERT(node->iNameStart >= 0 && node->iNameStart < obj->_strNames.length)
	return (&obj->_strNames.str[node->iNameStart]);
}

const char* NBXml_nodeContent(STNBXml* obj, const STNBXmlNode* node){
    NBASSERT(node != NULL)
    NBASSERT(node->iContentStart >= 0 && node->iContentStart < obj->_strContent.length)
	return (&obj->_strContent.str[node->iContentStart]);
}

const char* NBXml_nodeAttrbName(STNBXml* obj, const STNBXmlNodeAttrb* attrb){
	NBASSERT(attrb != NULL)
	NBASSERT(attrb->iNameStart >= 0 && attrb->iNameStart < obj->_strNames.length)
	return (&obj->_strNames.str[attrb->iNameStart]);
}

const char* NBXml_nodeAttrbValue(STNBXml* obj, const STNBXmlNodeAttrb* attrb){
	NBASSERT(attrb != NULL)
	NBASSERT(attrb->iValueStart >= 0 && attrb->iValueStart < obj->_strAttrVals.length)
	return (&obj->_strAttrVals.str[attrb->iValueStart]);
}

const STNBXmlNode* NBXml_rootNode(STNBXml* obj){
	return &obj->_docNode;
}

//--------------
// Nodes childn
//--------------

const STNBXmlNode* NBXml_childNode(STNBXml* obj, const char* name, const STNBXmlNode* parent, const STNBXmlNode* afterThis){
	STNBXmlNode* nodoEcontrado = NULL;
	if(parent == NULL) parent = &obj->_docNode;
	if(parent->childn != NULL){
		//establecer el inicio de la busqueda
		UI32 i=0, count = parent->childn->use;
		if(afterThis != NULL){
		    NBASSERT(afterThis->hintChildIdx<parent->childn->use);
		    NBASSERT((STNBXmlNode*)NBArray_itemAtIndex(parent->childn, afterThis->hintChildIdx) == afterThis);
			i = afterThis->hintChildIdx + 1;
		}
		//busqueda por name
		for(; i < count; i++){
			STNBXmlNode* nodoHijo = (STNBXmlNode*)NBArray_itemAtIndex(parent->childn, i);
			if(NBString_strIsEqual(&obj->_strNames.str[nodoHijo->iNameStart], name)){
				nodoHijo->hintChildIdx = i; //para optimizar cuando esta salida sea brindada como parametro de entrada
				nodoEcontrado = nodoHijo;
				break;
			}
		}
	}
	return nodoEcontrado;
}

const STNBXmlNode*	NBXml_childNodeWithEndOfname(STNBXml* obj, const char* endOfName, const STNBXmlNode* parent, const STNBXmlNode* afterThis){
	STNBXmlNode* nodoEcontrado = NULL;
	if(parent == NULL) parent = &obj->_docNode;
	if(parent->childn != NULL){
		//establecer el inicio de la busqueda
		UI32 i=0, count = parent->childn->use;
		if(afterThis != NULL){
			NBASSERT(afterThis->hintChildIdx<parent->childn->use);
			NBASSERT((STNBXmlNode*)NBArray_itemAtIndex(parent->childn, afterThis->hintChildIdx) == afterThis);
			i = afterThis->hintChildIdx + 1;
		}
		//busqueda por endOfName
		const UI32 endOfNameSz		= NBString_strLenBytes(endOfName);
		for(; i < count; i++){
			STNBXmlNode* nodoHijo	= (STNBXmlNode*)NBArray_itemAtIndex(parent->childn, i);
			const char* name		= &obj->_strNames.str[nodoHijo->iNameStart];
			const UI32 nameSz		= NBString_strLenBytes(name);
			if(nameSz >= endOfNameSz){
				if(NBString_strIsEqual(&obj->_strNames.str[nodoHijo->iNameStart + nameSz - endOfNameSz], endOfName)){
					nodoHijo->hintChildIdx = i; //para optimizar cuando esta salida sea brindada como parametro de entrada
					nodoEcontrado = nodoHijo;
					break;
				}
			}
		}
	}
	return nodoEcontrado;
}

const STNBXmlNode* NBXml_childNodeAfter(STNBXml* obj, const STNBXmlNode* parent, const STNBXmlNode* afterThis){
	STNBXmlNode* nodoEcontrado = NULL;
	if(parent == NULL) parent = &obj->_docNode;
		if(parent->childn != NULL){
			//establecer el inicio de la busqueda
			if(afterThis != NULL){
				if((afterThis->hintChildIdx+1)<parent->childn->use){
				    NBASSERT(afterThis->hintChildIdx>=0 && afterThis->hintChildIdx<parent->childn->use)
				    NBASSERT((STNBXmlNode*)NBArray_itemAtIndex(parent->childn, afterThis->hintChildIdx) == afterThis)
					nodoEcontrado = (STNBXmlNode*)NBArray_itemAtIndex(parent->childn, afterThis->hintChildIdx + 1); //afterThis = NULL;
					nodoEcontrado->hintChildIdx = afterThis->hintChildIdx + 1; //para optimizar cuando esta salida sea brindada como parametro de entrada
				}
			} else {
				if(parent->childn->use != 0){
					nodoEcontrado = (STNBXmlNode*)NBArray_itemAtIndex(parent->childn, 0); //para optimizar cuando esta salida sea brindada como parametro de entrada
					nodoEcontrado->hintChildIdx = 0;
				}
			}
		}
	return nodoEcontrado;
}

const char* NBXml_childStr(STNBXml* obj, const char* name, const char* defValue, const STNBXmlNode* parent, const STNBXmlNode* afterThis){
	const char* r = defValue;
	const STNBXmlNode* nodoTmp = NBXml_childNode(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		r = &obj->_strContent.str[nodoTmp->iContentStart];
	}
	return r;
}

char NBXml_childChar(STNBXml* obj, const char* name, char defValue, const STNBXmlNode* parent, const STNBXmlNode* afterThis){
	char r = defValue;
	const STNBXmlNode* nodoTmp = NBXml_childNode(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		r = obj->_strContent.str[nodoTmp->iContentStart];
	}
	return r;
}

int NBXml_childInt(STNBXml* obj, const char* name, int defValue, const STNBXmlNode* parent, const STNBXmlNode* afterThis){
	int r = defValue;
	const STNBXmlNode* nodoTmp = NBXml_childNode(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		const char* strVal = &obj->_strContent.str[nodoTmp->iContentStart];
		NBXML_VAL_NUMERIC_SIGNED(r, strVal, int)
	}
	return r;
}

unsigned int NBXml_childUint(STNBXml* obj, const char* name, unsigned int defValue, const STNBXmlNode* parent, const STNBXmlNode* afterThis){
	unsigned int r = defValue;
	const STNBXmlNode* nodoTmp = NBXml_childNode(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		const char* strVal = &obj->_strContent.str[nodoTmp->iContentStart];
		NBXML_VAL_NUMERIC_UNSIGNED(r, strVal, unsigned int)
	}
	return r;
}

long NBXml_childLong(STNBXml* obj, const char* name, long defValue, const STNBXmlNode* parent, const STNBXmlNode* afterThis){
	long r = defValue;
	const STNBXmlNode* nodoTmp = NBXml_childNode(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		const char* strVal = &obj->_strContent.str[nodoTmp->iContentStart];
		NBXML_VAL_NUMERIC_SIGNED(r, strVal, long)
	}
	return r;
}

unsigned long NBXml_childUlong(STNBXml* obj, const char* name, unsigned long defValue, const STNBXmlNode* parent, const STNBXmlNode* afterThis){
	unsigned long r = defValue;
	const STNBXmlNode* nodoTmp = NBXml_childNode(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		const char* strVal = &obj->_strContent.str[nodoTmp->iContentStart];
		NBXML_VAL_NUMERIC_UNSIGNED(r, strVal, unsigned long)
	}
	return r;
}

long long NBXml_childLongLong(STNBXml* obj, const char* name, long long defValue, const STNBXmlNode* parent, const STNBXmlNode* afterThis){
	long long r = defValue;
	const STNBXmlNode* nodoTmp = NBXml_childNode(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		const char* strVal = &obj->_strContent.str[nodoTmp->iContentStart];
		NBXML_VAL_NUMERIC_SIGNED(r, strVal, long long)
	}
	return r;
}

unsigned long long NBXml_childULongLong(STNBXml* obj, const char* name, unsigned long long defValue, const STNBXmlNode* parent, const STNBXmlNode* afterThis){
	unsigned long long r = defValue;
	const STNBXmlNode* nodoTmp = NBXml_childNode(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		const char* strVal = &obj->_strContent.str[nodoTmp->iContentStart];
		NBXML_VAL_NUMERIC_UNSIGNED(r, strVal, unsigned long long)
	}
	return r;
}

BOOL NBXml_childBOOL(STNBXml* obj, const char* name, BOOL defValue, const STNBXmlNode* parent, const STNBXmlNode* afterThis){
	BOOL r = defValue;
	const STNBXmlNode* nodoTmp = NBXml_childNode(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		const char* val = &obj->_strContent.str[nodoTmp->iContentStart];
		if(NBString_strIsEqual(val, "false") || NBString_strIsEqual(val, "0")){
			r = FALSE;
		} else {
			r = TRUE;
		}
	}
	return r;
}

float NBXml_childFloat(STNBXml* obj, const char* name, float defValue, const STNBXmlNode* parent, const STNBXmlNode* afterThis){
	float r = defValue;
	const STNBXmlNode* nodoTmp = NBXml_childNode(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		const char* strVal = &obj->_strContent.str[nodoTmp->iContentStart];
		NBXML_VAL_NUMERIC_SIGNED(r, strVal, float)
	}
	return r;
}

double NBXml_childDouble(STNBXml* obj, const char* name, double defValue, const STNBXmlNode* parent, const STNBXmlNode* afterThis){
	double r = defValue;
	const STNBXmlNode* nodoTmp = NBXml_childNode(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		const char* strVal = &obj->_strContent.str[nodoTmp->iContentStart];
		NBXML_VAL_NUMERIC_SIGNED(r, strVal, double)
	}
	return r;
}

//--------------
// Nodes attribs
//--------------

const STNBXmlNodeAttrb* NBXml_childAttrb(STNBXml* obj, const char* name, const STNBXmlNode* parent, const STNBXmlNodeAttrb* afterThis){
	STNBXmlNodeAttrb* nodoEcontrado = NULL;
	if(parent == NULL) parent = &obj->_docNode;
	if(parent->attribs != NULL){
		//establecer el inicio de la busqueda
		UI32 i=0, count = parent->attribs->use;
		if(afterThis != NULL){
			NBASSERT(afterThis->hintChildIdx<parent->attribs->use);
			NBASSERT((STNBXmlNodeAttrb*)NBArray_itemAtIndex(parent->attribs, afterThis->hintChildIdx) == afterThis);
			i = afterThis->hintChildIdx + 1;
		}
		//busqueda por name
		for(; i < count; i++){
			STNBXmlNodeAttrb* nodoHijo = (STNBXmlNodeAttrb*)NBArray_itemAtIndex(parent->attribs, i);
			if(NBString_strIsEqual(&obj->_strNames.str[nodoHijo->iNameStart], name)){
				nodoHijo->hintChildIdx = i; //para optimizar cuando esta salida sea brindada como parametro de entrada
				nodoEcontrado = nodoHijo;
				break;
			}
		}
	}
	return nodoEcontrado;
}

const STNBXmlNodeAttrb* NBXml_childAttrbAfter(STNBXml* obj, const STNBXmlNode* parent, const STNBXmlNodeAttrb* afterThis){
	STNBXmlNodeAttrb* nodoEcontrado = NULL;
	if(parent == NULL) parent = &obj->_docNode;
	if(parent->attribs != NULL){
		//establecer el inicio de la busqueda
		if(afterThis != NULL){
			if((afterThis->hintChildIdx+1)<parent->attribs->use){
				NBASSERT(afterThis->hintChildIdx>=0 && afterThis->hintChildIdx<parent->attribs->use)
				NBASSERT((STNBXmlNodeAttrb*)NBArray_itemAtIndex(parent->attribs, afterThis->hintChildIdx) == afterThis)
				nodoEcontrado = (STNBXmlNodeAttrb*)NBArray_itemAtIndex(parent->attribs, afterThis->hintChildIdx + 1); //afterThis = NULL;
				nodoEcontrado->hintChildIdx = afterThis->hintChildIdx + 1; //para optimizar cuando esta salida sea brindada como parametro de entrada
			}
		} else {
			if(parent->attribs->use != 0){
				nodoEcontrado = (STNBXmlNodeAttrb*)NBArray_itemAtIndex(parent->attribs, 0); //para optimizar cuando esta salida sea brindada como parametro de entrada
				nodoEcontrado->hintChildIdx = 0;
			}
		}
	}
	return nodoEcontrado;
}

const char* NBXml_attrbStr(STNBXml* obj, const char* name, const char* defValue, const STNBXmlNode* parent, const STNBXmlNodeAttrb* afterThis){
	const char* r = defValue;
	const STNBXmlNodeAttrb* nodoTmp = NBXml_childAttrb(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		r = &obj->_strAttrVals.str[nodoTmp->iValueStart];
	}
	return r;
}

char NBXml_attrbChar(STNBXml* obj, const char* name, char defValue, const STNBXmlNode* parent, const STNBXmlNodeAttrb* afterThis){
	char r = defValue;
	const STNBXmlNodeAttrb* nodoTmp = NBXml_childAttrb(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		r = obj->_strAttrVals.str[nodoTmp->iValueStart];
	}
	return r;
}

int NBXml_attrbInt(STNBXml* obj, const char* name, int defValue, const STNBXmlNode* parent, const STNBXmlNodeAttrb* afterThis){
	int r = defValue;
	const STNBXmlNodeAttrb* nodoTmp = NBXml_childAttrb(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		const char* strVal = &obj->_strAttrVals.str[nodoTmp->iValueStart];
		NBXML_VAL_NUMERIC_SIGNED(r, strVal, int)
	}
	return r;
}

unsigned int NBXml_attrbUint(STNBXml* obj, const char* name, unsigned int defValue, const STNBXmlNode* parent, const STNBXmlNodeAttrb* afterThis){
	unsigned int r = defValue;
	const STNBXmlNodeAttrb* nodoTmp = NBXml_childAttrb(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		const char* strVal = &obj->_strAttrVals.str[nodoTmp->iValueStart];
		NBXML_VAL_NUMERIC_UNSIGNED(r, strVal, unsigned int)
	}
	return r;
}

long NBXml_attrbLong(STNBXml* obj, const char* name, long defValue, const STNBXmlNode* parent, const STNBXmlNodeAttrb* afterThis){
	long r = defValue;
	const STNBXmlNodeAttrb* nodoTmp = NBXml_childAttrb(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		const char* strVal = &obj->_strAttrVals.str[nodoTmp->iValueStart];
		NBXML_VAL_NUMERIC_SIGNED(r, strVal, long)
	}
	return r;
}

unsigned long NBXml_attrbUlong(STNBXml* obj, const char* name, unsigned long defValue, const STNBXmlNode* parent, const STNBXmlNodeAttrb* afterThis){
	unsigned long r = defValue;
	const STNBXmlNodeAttrb* nodoTmp = NBXml_childAttrb(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		const char* strVal = &obj->_strAttrVals.str[nodoTmp->iValueStart];
		NBXML_VAL_NUMERIC_UNSIGNED(r, strVal, unsigned long)
	}
	return r;
}

long long NBXml_attrbLongLong(STNBXml* obj, const char* name, long long defValue, const STNBXmlNode* parent, const STNBXmlNodeAttrb* afterThis){
	long long r = defValue;
	const STNBXmlNodeAttrb* nodoTmp = NBXml_childAttrb(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		const char* strVal = &obj->_strAttrVals.str[nodoTmp->iValueStart];
		NBXML_VAL_NUMERIC_SIGNED(r, strVal, long long)
	}
	return r;
}

unsigned long long NBXml_attrbULongLong(STNBXml* obj, const char* name, unsigned long long defValue, const STNBXmlNode* parent, const STNBXmlNodeAttrb* afterThis){
	unsigned long long r = defValue;
	const STNBXmlNodeAttrb* nodoTmp = NBXml_childAttrb(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		const char* strVal = &obj->_strAttrVals.str[nodoTmp->iValueStart];
		NBXML_VAL_NUMERIC_UNSIGNED(r, strVal, unsigned long long)
	}
	return r;
}

BOOL NBXml_attrbBOOL(STNBXml* obj, const char* name, BOOL defValue, const STNBXmlNode* parent, const STNBXmlNodeAttrb* afterThis){
	BOOL r = defValue;
	const STNBXmlNodeAttrb* nodoTmp = NBXml_childAttrb(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		const char* val = &obj->_strAttrVals.str[nodoTmp->iValueStart];
		if(NBString_strIsEqual(val, "false") || NBString_strIsEqual(val, "0")){
			r = FALSE;
		} else {
			r = TRUE;
		}
	}
	return r;
}

float NBXml_attrbFloat(STNBXml* obj, const char* name, float defValue, const STNBXmlNode* parent, const STNBXmlNodeAttrb* afterThis){
	float r = defValue;
	const STNBXmlNodeAttrb* nodoTmp = NBXml_childAttrb(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		const char* strVal = &obj->_strAttrVals.str[nodoTmp->iValueStart];
		NBXML_VAL_NUMERIC_SIGNED(r, strVal, float)
	}
	return r;
}

double NBXml_attrbDouble(STNBXml* obj, const char* name, double defValue, const STNBXmlNode* parent, const STNBXmlNodeAttrb* afterThis){
	double r = defValue;
	const STNBXmlNodeAttrb* nodoTmp = NBXml_childAttrb(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		const char* strVal = &obj->_strAttrVals.str[nodoTmp->iValueStart];
		NBXML_VAL_NUMERIC_SIGNED(r, strVal, double)
	}
	return r;
}

//Save JSON

/*BOOL guardarJSONIdentadoHaciaArchivo(const char* rutaArchivo){
	BOOL r = FALSE;
	AUArchivo* archivo = NBGestorArchivos::flujoDeArchivo(ENMemoriaTipo_Temporal, rutaArchivo, ENArchivoModo_SoloEscritura);
	if(archivo != NULL){
		archivo->lock();
		r = guardarJSONIdentadoHaciaArchivo(archivo);
		archivo->unlock();
		archivo->cerrar();
	}
	return r;
}

BOOL guardarJSONIdentadoHaciaArchivo(AUArchivo* archivo){
	BOOL r = FALSE;
	if(archivo != NULL){
		AUCadenaLargaMutable8* strXML = new(ENMemoriaTipo_Temporal) AUCadenaLargaMutable8();
		//Datos XML
		if(_docNode.childn != NULL){
			UI32 iNodo, count = _docNode.childn->use;
			for(iNodo=0; iNodo<count; iNodo++){
				strXML->vaciar();
				AUDatosJSONP<char>::cadenaJSONIdentadaDeNodo(this, &obj->_docNode.childn->elemento[iNodo], ENXmlNodeTypee_Array, strXML, 0, archivo);
				archivo->escribir(strXML->str(), 1, strXML->tamano(), archivo);
			}
		}
		strXML->liberar(NB_RETENEDOR_THIS);
		r = TRUE;
	}
	return r;
}

BOOL guardarJSONSinEspaciosHaciaArchivo(const char* rutaArchivo){
	BOOL r = FALSE;
	AUArchivo* archivo = NBGestorArchivos::flujoDeArchivo(ENMemoriaTipo_Temporal, rutaArchivo, ENArchivoModo_SoloEscritura);
	if(archivo != NULL){
		archivo->lock();
		r = guardarJSONSinEspaciosHaciaArchivo(archivo);
		archivo->unlock();
		archivo->cerrar();
	}
	return r;
}

BOOL guardarJSONSinEspaciosHaciaArchivo(AUArchivo* archivo){
	BOOL r = FALSE;
	if(archivo != NULL){
		AUCadenaLargaMutable8* strXML = new(ENMemoriaTipo_Temporal) AUCadenaLargaMutable8();
		//Datos XML
		if(_docNode.childn != NULL){
			SI32 iNodo, count = _docNode.childn->use;
			for(iNodo=0; iNodo<count; iNodo++){
				strXML->vaciar();
				AUDatosJSONP<char>::cadenaJSONSinEspaciosDeNodo(this, &obj->_docNode.childn->elemento[iNodo], ENXmlNodeTypee_Array, strXML, archivo);
				archivo->escribir(strXML->str(), sizeof(char), strXML->tamano(), archivo);
			}
		}
		strXML->liberar(NB_RETENEDOR_THIS);
		r = TRUE;
	}
	return r;
}

long cadenaJSONIdentada(AUCadenaLargaMutable8* guardarEn){
	long r = AUDatosJSONP<char>::cadenaJSONIdentadaDeNodo(this, &obj->_docNode, ENXmlNodeTypee_Array, guardarEn, 1, NULL);
	return r;
}

long cadenaJSONSinEspacios(AUCadenaLargaMutable8* guardarEn){
	long r = AUDatosJSONP<char>::cadenaJSONSinEspaciosDeNodo(this, &obj->_docNode, ENXmlNodeTypee_Array, guardarEn);
	return r;
}

static long literalJSONScapeado(const char* str, AUCadenaLargaMutable8* dstBuffer, AUArchivo* dstFile){
	long r = 0;
	UI32 charValue = 0;
	SI32 pos = 0;
	while(str[pos] != '\0'){
		NBGESTORFUENTES_CHAR_DESDE_UTF8(charValue, str, pos)
		switch(charValue) {
			case '\"': dstBuffer->agregar("\\\"", 2); break;
			case '\\': dstBuffer->agregar("\\\\", 2); break;
			case '/': dstBuffer->agregar("\\/", 2); break;
			case '\b': dstBuffer->agregar("\\b", 2); break;
			case '\f': dstBuffer->agregar("\\f", 2); break;
			case '\n': dstBuffer->agregar("\\n", 2); break;
			case '\r': dstBuffer->agregar("\\r", 2); break;
			case '\t': dstBuffer->agregar("\\t", 2); break;
			default:
				if(charValue < 128){ //Utf8 first byte limit
					dstBuffer->agregar((char)charValue);
				} else {
					//TODO: scape non ASCII chars as "\uFFFF"
				}
				break;
		}
		//Optimize buffer usage
		//(flush buffer to file)
		if(dstFile != NULL){
			if(dstBuffer->tamano() > 1024){
				dstFile->escribir(dstBuffer->str(), sizeof(char), dstBuffer->tamano(), dstFile);
				dstBuffer->vaciar();
			}
		}
		r++;
	}
	return r;
}

static long cadenaJSONIdentadaDeNodo(AUDatosJSONP<char>* datosXml, STNBXmlNode* nodoXml, const ENXmlNodeTypee parentType, AUCadenaLargaMutable8* guardarEn, const long nivel, AUArchivo* archivoDst){
	long i;
	//Ident
	const char identChar = '\t';
	guardarEn->repetir(identChar, (const int)nivel);
	//Name
	if(parentType != ENXmlNodeTypee_Array){
		guardarEn->agregar('\"');
		guardarEn->agregar(&datosXml->_strNames.str[nodoXml->iNameStart]);
		guardarEn->agregar('\"');
		guardarEn->agregar(':');
		guardarEn->agregar(' ');
	}
	//
	if(nodoXml->type == ENXmlNodeTypee_Object){
		guardarEn->agregar('{');
		if(nodoXml->childn != 0){
			if(nodoXml->childn->use > 0){
				for(i = 0; i<nodoXml->childn->use; i++){
					if(i != 0){
						guardarEn->agregar(',');
					}
					guardarEn->agregar('\r');
					guardarEn->agregar('\n');
					AUDatosJSONP<char>::cadenaJSONIdentadaDeNodo(datosXml, &(nodoXml->childn->elemento[i]), nodoXml->type, guardarEn, (nivel + 1), archivoDst);
				}
				guardarEn->agregar('\r');
				guardarEn->agregar('\n');
				guardarEn->repetir(identChar, (const int)nivel);
			}
		}
		guardarEn->agregar('}');
	} else if(nodoXml->type == ENXmlNodeTypee_Array){
		guardarEn->agregar('[');
		if(nodoXml->childn != 0){
			if(nodoXml->childn->use > 0){
				for(i=0; i<nodoXml->childn->use; i++){
					if(i != 0){
						guardarEn->agregar(',');
					}
					guardarEn->agregar('\r');
					guardarEn->agregar('\n');
					AUDatosJSONP<char>::cadenaJSONIdentadaDeNodo(datosXml, &(nodoXml->childn->elemento[i]), nodoXml->type, guardarEn, (nivel + 1), archivoDst);
				}
				guardarEn->agregar('\r');
				guardarEn->agregar('\n');
				guardarEn->repetir(identChar, (const int)nivel);
			}
		}
		guardarEn->agregar(']');
	} else {
		if(nodoXml->valueType == ENXmlNodeValueType_Literal){
			//literal value
			guardarEn->agregar('\"');
			AUDatosJSONP<char>::literalJSONScapeado(&datosXml->_strVals.str[nodoXml->iValueStart], guardarEn, archivoDst);
			guardarEn->agregar('\"');
		} else {
			//plain value (number, TRUE, FALSE or null)
			guardarEn->agregar(&datosXml->_strVals.str[nodoXml->iValueStart]);
		}
	}
	//Flush to file
	if(archivoDst != NULL){
		archivoDst->escribir(guardarEn->str(), sizeof(char), guardarEn->tamano(), archivoDst);
		guardarEn->vaciar();
	}
	return 0;
}

static long cadenaJSONSinEspaciosDeNodo(AUDatosJSONP<char>* datosXml, STNBXmlNode* nodoXml, const ENXmlNodeTypee parentType, AUCadenaLargaMutable8* guardarEn, AUArchivo* archivoDst){
	//Name
	if(parentType != ENXmlNodeTypee_Array){
		guardarEn->agregar('\"');
		guardarEn->agregar(&datosXml->_strNames.str[nodoXml->iNameStart]);
		guardarEn->agregar('\"');
		guardarEn->agregar(':');
	}
	//Value
	if(nodoXml->type == ENXmlNodeTypee_Object){
		guardarEn->agregar('{');
		if(nodoXml->childn != NULL){
			UI32 i, count = nodoXml->childn->use;
			for(i = 0; i < count; i++){
				if(i != 0) guardarEn->agregar(',');
				AUDatosJSONP<char>::cadenaJSONSinEspaciosDeNodo(datosXml, &(nodoXml->childn->elemento[i]), nodoXml->type, guardarEn, archivoDst);
			}
		}
		guardarEn->agregar('}');
	} else if(nodoXml->type == ENXmlNodeTypee_Array){
		guardarEn->agregar('[');
		if(nodoXml->childn != NULL){
			UI32 i, count = nodoXml->childn->use;
			for(i = 0; i < count; i++){
				if(i != 0) guardarEn->agregar(',');
				AUDatosJSONP<char>::cadenaJSONSinEspaciosDeNodo(datosXml, &(nodoXml->childn->elemento[i]), nodoXml->type, guardarEn, archivoDst);
			}
		}
		guardarEn->agregar(']');
	} else {
		if(nodoXml->valueType == ENXmlNodeValueType_Literal){
			//literal value
			guardarEn->agregar('\"');
			AUDatosJSONP<char>::literalJSONScapeado(&datosXml->_strVals.str[nodoXml->iValueStart], guardarEn, archivoDst);
			guardarEn->agregar('\"');
		} else {
			//plain value (number, TRUE, FALSE or null)
			guardarEn->agregar(&datosXml->_strVals.str[nodoXml->iValueStart]);
		}
	}
	//Flush to file
	if(archivoDst != NULL){
		archivoDst->escribir(guardarEn->str(), sizeof(char), guardarEn->tamano(), archivoDst);
		guardarEn->vaciar();
	}
	return 0;
}*/

//private

void NBXml_privNodeInit(STNBXmlNode* nodo){
	if(nodo != NULL){
		nodo->iNameStart	= 0;
		nodo->iContentStart	= 0;
		nodo->iContentSz	= 0;
		nodo->childn		= NULL;
		nodo->attribs		= NULL;
		//
		nodo->hintDeepLvl	= 0;
		nodo->hintChildIdx	= 0;
	}
}

void NBXml_privNodeRelease(STNBXmlNode* nodo){
	if(nodo != NULL){
		nodo->iNameStart	= 0;
		nodo->iContentStart	= 0;
		nodo->iContentSz	= 0;
		if(nodo->childn != NULL){
		    NBArray_release(nodo->childn);
		    NBMemory_free(nodo->childn);
			nodo->childn	= NULL;
		}
		if(nodo->attribs != NULL){
			NBArray_release(nodo->attribs);
			NBMemory_free(nodo->attribs);
			nodo->attribs	= NULL;
		}
		//
		nodo->hintDeepLvl	= 0;
		nodo->hintChildIdx	= 0;
	}
}

void NBXml_privNodeEmpty(STNBXmlNode* nodo){
	if(nodo != NULL){
		//Children
		if(nodo->childn != NULL){
			while(nodo->childn->use>0){
				STNBXmlNode* nodoHijo = (STNBXmlNode*)NBArray_itemAtIndex(nodo->childn, nodo->childn->use - 1);
				//Optiminzacion #1 (evitar 1 llamado recursivo)
				if(nodoHijo->childn != NULL){
					while(nodoHijo->childn->use>0){
						STNBXmlNode* nodoNieto = (STNBXmlNode*)NBArray_itemAtIndex(nodoHijo->childn, nodoHijo->childn->use - 1);
						//Optimizacion #2 (evitar otro llamado recursivo)
						if(nodoNieto->childn != NULL){
							while(nodoNieto->childn->use>0){
								STNBXmlNode* nodoBisNieto = (STNBXmlNode*)NBArray_itemAtIndex(nodoNieto->childn, nodoNieto->childn->use - 1);
							    NBXml_privNodeEmpty(nodoBisNieto);
							    NBXml_privNodeRelease(nodoBisNieto);
							    NBArray_removeItemAtIndex(nodoNieto->childn, nodoNieto->childn->use - 1);
							}
						}
					    NBXml_privNodeRelease(nodoNieto);
					    NBArray_removeItemAtIndex(nodoHijo->childn, nodoHijo->childn->use - 1);
					}
				}
			    NBXml_privNodeRelease(nodoHijo);
			    NBArray_removeItemAtIndex(nodo->childn, nodo->childn->use - 1);
			}
		}
		//Attribs
		if(nodo->attribs != NULL){
			NBArray_empty(nodo->attribs);
		}
	}
}

//Private

UI32 NBXml_privIdxForTag(STNBXml* obj, const char* tagName){
	UI32 indiceEtiqueta = 0;
	SI32 iEtiq, iEtiqConteo = obj->_idxsNames.use;
	for(iEtiq=0; iEtiq<iEtiqConteo; iEtiq++){
		if(NBString_strIsEqual(&obj->_strNames.str[*(UI32*)NBArray_itemAtIndex(&obj->_idxsNames, iEtiq)], tagName)){
			//Etiqueta ya esta registrada
			indiceEtiqueta = *(UI32*)NBArray_itemAtIndex(&obj->_idxsNames, iEtiq);
			break;
		}
	}
	if(indiceEtiqueta == 0){
		//Primera vez que aparece la etiqueta (registrar)
		indiceEtiqueta = obj->_strNames.length;
		NBArray_addValue(&obj->_idxsNames, indiceEtiqueta);
		NBString_concat(&obj->_strNames, tagName);
		NBString_concatByte(&obj->_strNames, '\0');
	}
	NBASSERT(indiceEtiqueta != 0)
	return indiceEtiqueta;
}

//----------------------
// Xml parser callbacks
//----------------------

void NBXml_consumeNodeOpening(const STNBXmlParser* state, const char* nodePath, const UI32 nodePathSz, const char* tagName, void* listenerParam){
	//PRINTF_INFO("NBXml_consumeNodeOpening\n");
	STNBXmlLoad* loadState = (STNBXmlLoad*)listenerParam;
	STNBArray* listaLIFO = &loadState->listaLIFO;
	STNBXml* obj = loadState->obj;
	//PRINTF_INFO("XML, opening new node.\n");
	//Determine parent
	STNBXmlNode* parent = NULL;
	if(listaLIFO->use == 0){
		parent = &obj->_docNode;
	} else {
		parent = *((STNBXmlNode**)NBArray_itemAtIndex(listaLIFO, listaLIFO->use - 1));
	}
	NBASSERT(parent != NULL)
	//New node
	STNBXmlNode nuevoNodo;
	NBXml_privNodeInit(&nuevoNodo);
	nuevoNodo.iNameStart		= NBXml_privIdxForTag(obj, tagName);
	nuevoNodo.iContentStart		= obj->_strContent.length;
	nuevoNodo.iContentSz		= 0;
	nuevoNodo.hintDeepLvl 		= listaLIFO->use;
	//Add to root or parent
	{
		if(parent->childn == NULL){
			parent->childn = (STNBArray*)NBMemory_alloc(sizeof(STNBArray));
			NBArray_init(parent->childn, sizeof(STNBXmlNode), NULL);
		}
		//Add to parent
		nuevoNodo.hintChildIdx	= parent->childn->use;
		NBArray_addValue(parent->childn, nuevoNodo);
		//Add to loading stack
		STNBXmlNode* nodePtr = (STNBXmlNode*)NBArray_itemAtIndex(parent->childn, parent->childn->use - 1);
		NBArray_addValue(listaLIFO, nodePtr);
	}
}

void NBXml_consumeNodeContent(const STNBXmlParser* state, const char* nodePath, const UI32 nodePathSz, const char* tagName, const char* data, const SI32 dataSize, void* listenerParam){
	//PRINTF_INFO("NBXml_consumeNodeContent (%d bytes)\n", dataSize);
	STNBXmlLoad* loadState = (STNBXmlLoad*)listenerParam;
	STNBArray* listaLIFO = &loadState->listaLIFO;
	STNBXml* obj = loadState->obj;
	//PRINTF_INFO("XML, opening new node.\n");
	//Determine parent
	STNBXmlNode* parent = NULL;
	if(listaLIFO->use == 0){
		parent = &obj->_docNode;
	} else {
		parent = *((STNBXmlNode**)NBArray_itemAtIndex(listaLIFO, listaLIFO->use - 1));
	}
	NBASSERT(parent != NULL)
	//Append content
	NBString_concatBytes(&obj->_strContent, data, dataSize);
	//PRINTF_INFO("XmlContent: (%d bytes): '%s'.\n", dataSize, data);
}

void NBXml_consumeNodeAttrb(const STNBXmlParser* state, const char* nodePath, const UI32 nodePathSz, const char* tagName, const char* attrbName, const char* attrbValue, const UI32 attrbValueSz, void* listenerParam){
	STNBXmlLoad* loadState = (STNBXmlLoad*)listenerParam;
	STNBArray* listaLIFO = &loadState->listaLIFO;
	STNBXml* obj = loadState->obj;
	//Determine parent
	STNBXmlNode* parent = NULL;
	if(listaLIFO->use == 0){
		parent = &obj->_docNode;
	} else {
		parent = *((STNBXmlNode**)NBArray_itemAtIndex(listaLIFO, listaLIFO->use - 1));
	}
	//New attrib
	STNBXmlNodeAttrb newAttrb;
	newAttrb.iNameStart		= NBXml_privIdxForTag(obj, attrbName);
	newAttrb.iValueStart	= obj->_strAttrVals.length;
	newAttrb.hintChildIdx	= 0;
	//Add attrib value
	NBString_concatBytes(&obj->_strAttrVals, attrbValue, attrbValueSz);
	NBString_concatByte(&obj->_strAttrVals, '\0');
	//Add to root or parent
	{
		if(parent->attribs == NULL){
			parent->attribs = (STNBArray*)NBMemory_alloc(sizeof(STNBArray));
			NBArray_init(parent->attribs, sizeof(STNBXmlNodeAttrb), NULL);
		}
		//Add to parent
		newAttrb.hintChildIdx = parent->attribs->use;
		NBArray_addValue(parent->attribs, newAttrb);
	}
}

void NBXml_consumeNodeClosing(const STNBXmlParser* state, const char* nodePath, const UI32 nodePathSz, const char* tagName, void* listenerParam){
	STNBXmlLoad* loadState = (STNBXmlLoad*)listenerParam;
	STNBArray* listaLIFO = &loadState->listaLIFO;
	STNBXml* obj = loadState->obj;
	//Determine parent
	STNBXmlNode* parent = NULL;
	if(listaLIFO->use == 0){
		parent = &obj->_docNode;
	} else {
		parent = *((STNBXmlNode**)NBArray_itemAtIndex(listaLIFO, listaLIFO->use - 1));
	}
	//Close content's range
	NBASSERT(parent->iContentStart <= obj->_strContent.length)
	parent->iContentSz	= (obj->_strContent.length - parent->iContentStart);
	NBString_concatByte(&obj->_strContent, '\0');
	//Remove last object from stack
	NBArray_removeItemAtIndex(listaLIFO, listaLIFO->use - 1);
}

//----------------------
// Json load from file
//----------------------

BOOL NBXml_loadFromFilePath(STNBXml* obj, const char* filePath){
	BOOL r = FALSE;
    STNBFileRef stream = NBFile_alloc(NULL);
	if(NBFile_open(stream, filePath, ENNBFileMode_Read)){
		NBFile_lock(stream);
		if(!NBXml_loadFromFile(obj, stream)){
			PRINTF_ERROR("interpretando JSON de: '%s'\n", filePath);
		} else {
			r = TRUE;
		}
		NBFile_unlock(stream);
	}
	NBFile_release(&stream);
	return r;
}

BOOL NBXml_loadFromFile(STNBXml* obj, STNBFileRef flujoArchivo){
	BOOL r = FALSE; NBASSERT(NBFile_isSet(flujoArchivo))
	//
	NBXml_feedStart(obj);
	{
		char buff[10240];
		SI32 rd = 0;
		do {
			rd = NBFile_read(flujoArchivo, buff, 10240);
			if(rd > 0){
				if(!NBXml_feedBytes(obj, buff, rd)){
					break;
				}
			}
		} while(rd > 0);
	}
	if(NBXml_feedEnd(obj)){
		r = TRUE;
	}
	return r;
}

// Json load from string

BOOL NBXml_loadFromStr(STNBXml* obj, const char* strData){
	BOOL r = FALSE;
	SI32 elemsRead = 0;
	//Determine data size
	while(strData[elemsRead] != '\0') elemsRead++;
	//
	NBXml_feedStart(obj);
	if(elemsRead > 0){
	    NBXml_feedBytes(obj, strData, elemsRead);
	}
	//Finish stream
	if(NBXml_feedEnd(obj)){
		r = TRUE;
	}
	return r;
}

//

void NBXml_feedStart(STNBXml* obj){
	if(obj->loadState != NULL){
		NBArray_release(&obj->loadState->listaLIFO);
		NBXmlParser_release(&obj->loadState->parser);
		obj->loadState->obj = NULL;
	} else {
		obj->loadState = (STNBXmlLoad*)NBMemory_alloc(sizeof(STNBXmlLoad));
	}
	//
	NBArray_init(&obj->loadState->listaLIFO, sizeof(STNBXmlNode*), NULL);
	STNBXmlNode* nodePtr = &obj->_docNode;
	NBArray_addValue(&obj->loadState->listaLIFO, nodePtr);
	//
	IXmlParserListener listener;
	listener.consumeNodeOpening		= NBXml_consumeNodeOpening;
	listener.consumeNodeAttrb		= NBXml_consumeNodeAttrb;
	listener.consumeNodeContent		= NBXml_consumeNodeContent;
	listener.consumeNodeClosing		= NBXml_consumeNodeClosing;
	NBXmlParser_initWithListener(&obj->loadState->parser, &listener, obj->loadState);
	obj->loadState->obj				= obj;
	//
	NBXml_privNodeEmpty(&obj->_docNode);
	NBXml_privNodeRelease(&obj->_docNode);
	NBXml_privNodeInit(&obj->_docNode);
	//
	NBXmlParser_feedStart(&obj->loadState->parser);
}

BOOL NBXml_feedByte(STNBXml* obj, const char c){
	UI32 r = 0;
	if(obj->loadState != NULL){
		if(NBXmlParser_feedBytes(&obj->loadState->parser, &c, 1)){
			r = 1;
		}
	}
	return r;
}

UI32 NBXml_feed(STNBXml* obj, const char* str){
	UI32 r = 0;
	if(obj->loadState != NULL){
		const UI32 len = NBString_strLenBytes(str);
		if(NBXmlParser_feedBytes(&obj->loadState->parser, str, len)){
			r = len;
		}
	}
	return r;
}

UI32 NBXml_feedBytes(STNBXml* obj, const void* data, const UI32 dataSz){
	UI32 r = 0;
	if(obj->loadState != NULL){
		if(NBXmlParser_feedBytes(&obj->loadState->parser, data, dataSz)){
			r = dataSz;
		}
	}
	return r;
}

BOOL NBXml_feedIsComplete(STNBXml* obj){
	BOOL r = FALSE;
	if(obj->loadState != NULL){
		r = NBXmlParser_feedIsComplete(&obj->loadState->parser);
	}
	return r;
}

BOOL NBXml_feedEnd(STNBXml* obj){
	BOOL r = FALSE;
	if(obj->loadState != NULL){
		if(!NBXmlParser_feedEnd(&obj->loadState->parser)){
			PRINTF_ERROR("NBXml, stream could not be parsed as a valid json.\n");
			if(obj->loadState->listaLIFO.use > 0){
				IF_PRINTF(const STNBXmlNode* lastNode = (const STNBXmlNode*)NBArray_itemAtIndex(&obj->loadState->listaLIFO, obj->loadState->listaLIFO.use - 1);)
				PRINTF_ERROR("NBXml, name of last node parsed: '%s'.\n", &obj->loadState->obj->_strNames.str[lastNode->iNameStart]);
				if(obj->loadState->listaLIFO.use > 1){
					IF_PRINTF(const STNBXmlNode* parentLastNode = (const STNBXmlNode*)NBArray_itemAtIndex(&obj->loadState->listaLIFO, obj->loadState->listaLIFO.use - 2);)
					PRINTF_ERROR("NBXml, name of parent of last node parsed: '%s'.\n", &obj->_strNames.str[parentLastNode->iNameStart]);
				}
			}
		} else {
			//PRINTF_INFO("Lista lifo tiene %d elementos.\n", _loadState->listaLIFO->use);
			//NBASSERT(_loadState->listaLIFO->use == 1)
			r = TRUE;
		}
		{
			NBArray_release(&obj->loadState->listaLIFO);
			NBXmlParser_release(&obj->loadState->parser);
			obj->loadState->obj = NULL;
			NBMemory_free(obj->loadState);
			obj->loadState = NULL;
		}
		//Update root content size
		obj->_docNode.iContentSz = (obj->_strContent.length - obj->_docNode.iContentStart);
	}
	return r;
}

//

BOOL NBXml_concatContentTo(const STNBXml* obj, STNBString* dst){
	BOOL r = TRUE;
	NBString_concat(dst, "<?xml version=\"1.1\" encoding=\"UTF-8\" ?>");
	if(!NBXml_concatContentNodeTo(obj, &obj->_docNode, dst)){
		r = FALSE; NBASSERT(FALSE)
	}
	return r;
}

BOOL NBXml_concatContentNodeTo(const STNBXml* obj, const STNBXmlNode* node, STNBString* dst){
	BOOL r = TRUE;
	UI32 iCurStart = node->iContentStart;
	const UI32 iEndAfter = node->iContentStart + node->iContentSz;
	if(node->childn != NULL){
		UI32 i; const UI32 count = node->childn->use;
		for(i = 0; i < count; i++){
			const STNBXmlNode* child = NBArray_itmPtrAtIndex(node->childn, STNBXmlNode, i);
			NBASSERT(iCurStart <= child->iContentStart && (child->iContentStart + child->iContentSz) <= iEndAfter) //Child must be at parent's range
			//Append pending content (between last child and this child)
			if(iCurStart < child->iContentStart){
				NBASSERT(iCurStart < obj->_strContent.length && child->iContentStart < obj->_strContent.length)
				NBXml_concatScapedBytes(dst, &obj->_strContent.str[iCurStart], (child->iContentStart - iCurStart));
			}
			//Add this child
			{
				const BOOL isEmpty = (child->iContentSz == 0 && child->childn == NULL ? TRUE : FALSE);
				//Open tag
				{
					NBString_concatByte(dst, '<');
					NBString_concat(dst, &obj->_strNames.str[child->iNameStart]);
					//Attributes
					if(child->attribs != NULL){
						UI32 i; const UI32 count = child->attribs->use;
						for(i = 0; i < count; i++){
							const STNBXmlNodeAttrb* attrb = NBArray_itmPtrAtIndex(child->attribs, STNBXmlNodeAttrb, i);
							NBString_concatByte(dst, ' ');
							NBString_concat(dst, &obj->_strNames.str[attrb->iNameStart]);
							NBString_concatByte(dst, '=');
							NBString_concatByte(dst, '\"');
							NBXml_concatScaped(dst, &obj->_strAttrVals.str[attrb->iValueStart]);
							NBString_concatByte(dst, '\"');
						}
					}
					if(isEmpty){
						NBString_concatByte(dst, '/');
					}
					NBString_concatByte(dst, '>');
				}
				//Content
				if(!NBXml_concatContentNodeTo(obj, child, dst)){
					r = FALSE; NBASSERT(FALSE)
					break;
				}
				//Closing tag
				if(!isEmpty){
					NBString_concatByte(dst, '<');
					NBString_concatByte(dst, '/');
					NBString_concat(dst, &obj->_strNames.str[child->iNameStart]);
					NBString_concatByte(dst, '>');
				}
			}
			iCurStart = child->iContentStart + child->iContentSz + 1;
		}
	}
	//Append remaining content
	if(r && iCurStart < iEndAfter){
		NBASSERT(iCurStart < obj->_strContent.length && iEndAfter < obj->_strContent.length)
		NBXml_concatScapedBytes(dst, &obj->_strContent.str[iCurStart], (iEndAfter - iCurStart));
	}
	return r;
}



