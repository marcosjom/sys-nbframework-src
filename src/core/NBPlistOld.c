//
//  XUXml.c
//  XUMonitorLogicaNegocios_OSX
//
//  Created by Marcos Ortega on 21/01/14.
//  Copyright (c) 2014 XurpasLatam. All rights reserved.
//

#include "nb/NBFrameworkPch.h"
#include "nb/core/NBMemory.h"
#include "nb/core/NBPlistOld.h"
#include "nb/core/NBPlistOldParser.h"
#include "nb/core/NBNumParser.h"
#include "nb/core/NBCompare.h"

#define NBPLISTOLD_VAL_NUMERIC_SIGNED(DST, STR_VALUE, TYPE)	\
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

#define NBPLISTOLD_VAL_NUMERIC_UNSIGNED(DST, STR_VALUE, TYPE)	\
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

void NBPlistOld_privNodeInit(STNBPlistOldNode* nodo);
void NBPlistOld_privNodeRelease(STNBPlistOldNode* nodo);
void NBPlistOld_privNodeEmpty(STNBPlistOldNode* nodo);

//

BOOL NBCompare_STNBPlistOldNode(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
	NBASSERT(dataSz == sizeof(STNBPlistOldNode))
	if(dataSz == sizeof(STNBPlistOldNode)){
		const STNBPlistOldNode* d1 = (const STNBPlistOldNode*)data1;
		const STNBPlistOldNode* d2 = (const STNBPlistOldNode*)data2;
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

void NBPlistOld_init(STNBPlistOld* obj){
    NBString_init(&obj->_strTags);
    NBString_init(&obj->_strVals);
    NBArray_initWithSz(&obj->_idxsTags, sizeof(UI32), NBCompareUI32, 128, 512, 0.5f);
	//
    NBString_concatByte(&obj->_strTags, (char)'\0');
    NBString_concatByte(&obj->_strVals, (char)'\0');
    NBPlistOld_privNodeInit(&obj->_docNode);
	obj->_docNode.type = ENPlistOldNodeTypee_Array;
}

void NBPlistOld_release(STNBPlistOld* obj){
    NBPlistOld_privNodeEmpty(&obj->_docNode);
    NBPlistOld_privNodeRelease(&obj->_docNode);
	//
    NBArray_release(&obj->_idxsTags);
    NBString_release(&obj->_strVals);
    NBString_release(&obj->_strTags);
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

void NBPlistOld_concatScaped(STNBString* dst, const char* unscaped){
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
			case '/': NBString_concatBytes(dst, "\\/", 2); break;
			//ToDo: four-digit-hex
			default: NBString_concatByte(dst, *c); break;
		}
		c++;
	}
}


void NBPlistOld_concatUnscaped(STNBString* dst, const char* scaped){
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
}

//

const char* NBPlistOld_nodeName(STNBPlistOld* obj, const STNBPlistOldNode* node){
    NBASSERT(node->iNameStart >= 0 && node->iNameStart < obj->_strTags.length)
	return (&obj->_strTags.str[node->iNameStart]);
}

const char* NBPlistOld_nodeValue(STNBPlistOld* obj, const STNBPlistOldNode* node){
    NBASSERT(node != NULL)
    NBASSERT(node->iValueStart >= 0 && node->iValueStart < obj->_strVals.length)
	return (&obj->_strVals.str[node->iValueStart]);
}

const STNBPlistOldNode* NBPlistOld_docNode(STNBPlistOld* obj){
	return &obj->_docNode;
}

//--------------
// Nodes childn
//--------------

const STNBPlistOldNode* NBPlistOld_childNode(STNBPlistOld* obj, const char* name, const STNBPlistOldNode* parent, const STNBPlistOldNode* afterThis){
	STNBPlistOldNode* nodoEcontrado = NULL;
	if(parent == NULL) parent = &obj->_docNode;
	if(parent->childn != NULL){
		//establecer el inicio de la busqueda
		UI32 i=0, iNodoConteo = parent->childn->use;
		if(afterThis != NULL){
		    NBASSERT(afterThis->hintChildIdx<parent->childn->use);
		    NBASSERT((STNBPlistOldNode*)NBArray_itemAtIndex(parent->childn, afterThis->hintChildIdx) == afterThis);
			i = afterThis->hintChildIdx + 1;
		}
		//busqueda por name
		for(; i < iNodoConteo; i++){
			STNBPlistOldNode* nodoHijo = (STNBPlistOldNode*)NBArray_itemAtIndex(parent->childn, i);
			if(NBString_strIsEqual(&obj->_strTags.str[nodoHijo->iNameStart], name)){
				nodoHijo->hintChildIdx = i; //para optimizar cuando esta salida sea brindada como parametro de entrada
				nodoEcontrado = nodoHijo;
				break;
			}
		}
	}
	return nodoEcontrado;
}

const STNBPlistOldNode* NBPlistOld_childNodeAfter(STNBPlistOld* obj, const STNBPlistOldNode* parent, const STNBPlistOldNode* afterThis){
	STNBPlistOldNode* nodoEcontrado = NULL;
	if(parent == NULL) parent = &obj->_docNode;
		if(parent->childn != NULL){
			//establecer el inicio de la busqueda
			if(afterThis != NULL){
				if((afterThis->hintChildIdx+1)<parent->childn->use){
				    NBASSERT(afterThis->hintChildIdx>=0 && afterThis->hintChildIdx<parent->childn->use)
				    NBASSERT((STNBPlistOldNode*)NBArray_itemAtIndex(parent->childn, afterThis->hintChildIdx) == afterThis)
					nodoEcontrado = (STNBPlistOldNode*)NBArray_itemAtIndex(parent->childn, afterThis->hintChildIdx + 1); //afterThis = NULL;
					nodoEcontrado->hintChildIdx = afterThis->hintChildIdx + 1; //para optimizar cuando esta salida sea brindada como parametro de entrada
				}
			} else {
				if(parent->childn->use != 0){
					nodoEcontrado = (STNBPlistOldNode*)NBArray_itemAtIndex(parent->childn, 0); //para optimizar cuando esta salida sea brindada como parametro de entrada
					nodoEcontrado->hintChildIdx = 0;
				}
			}
		}
	return nodoEcontrado;
}

const char* NBPlistOld_childStr(STNBPlistOld* obj, const char* name, const char* defValue, const STNBPlistOldNode* parent, const STNBPlistOldNode* afterThis){
	const char* r = defValue;
	const STNBPlistOldNode* nodoTmp = NBPlistOld_childNode(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		r = &obj->_strVals.str[nodoTmp->iValueStart];
	}
	return r;
}

char NBPlistOld_childChar(STNBPlistOld* obj, const char* name, char defValue, const STNBPlistOldNode* parent, const STNBPlistOldNode* afterThis){
	char r = defValue;
	const STNBPlistOldNode* nodoTmp = NBPlistOld_childNode(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		r = obj->_strVals.str[nodoTmp->iValueStart];
	}
	return r;
}

int NBPlistOld_childInt(STNBPlistOld* obj, const char* name, int defValue, const STNBPlistOldNode* parent, const STNBPlistOldNode* afterThis){
	int r = defValue;
	const STNBPlistOldNode* nodoTmp = NBPlistOld_childNode(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		const char* strVal = &obj->_strVals.str[nodoTmp->iValueStart];
		NBPLISTOLD_VAL_NUMERIC_SIGNED(r, strVal, int)
	}
	return r;
}

unsigned int NBPlistOld_childUint(STNBPlistOld* obj, const char* name, unsigned int defValue, const STNBPlistOldNode* parent, const STNBPlistOldNode* afterThis){
	unsigned int r = defValue;
	const STNBPlistOldNode* nodoTmp = NBPlistOld_childNode(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		const char* strVal = &obj->_strVals.str[nodoTmp->iValueStart];
		NBPLISTOLD_VAL_NUMERIC_UNSIGNED(r, strVal, unsigned int)
	}
	return r;
}

long NBPlistOld_childLong(STNBPlistOld* obj, const char* name, long defValue, const STNBPlistOldNode* parent, const STNBPlistOldNode* afterThis){
	long r = defValue;
	const STNBPlistOldNode* nodoTmp = NBPlistOld_childNode(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		const char* strVal = &obj->_strVals.str[nodoTmp->iValueStart];
		NBPLISTOLD_VAL_NUMERIC_SIGNED(r, strVal, long)
	}
	return r;
}

unsigned long NBPlistOld_childUlong(STNBPlistOld* obj, const char* name, unsigned long defValue, const STNBPlistOldNode* parent, const STNBPlistOldNode* afterThis){
	unsigned long r = defValue;
	const STNBPlistOldNode* nodoTmp = NBPlistOld_childNode(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		const char* strVal = &obj->_strVals.str[nodoTmp->iValueStart];
		NBPLISTOLD_VAL_NUMERIC_UNSIGNED(r, strVal, unsigned long)
	}
	return r;
}

long long NBPlistOld_childLongLong(STNBPlistOld* obj, const char* name, long long defValue, const STNBPlistOldNode* parent, const STNBPlistOldNode* afterThis){
	long long r = defValue;
	const STNBPlistOldNode* nodoTmp = NBPlistOld_childNode(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		const char* strVal = &obj->_strVals.str[nodoTmp->iValueStart];
		NBPLISTOLD_VAL_NUMERIC_SIGNED(r, strVal, long long)
	}
	return r;
}

unsigned long long NBPlistOld_childULongLong(STNBPlistOld* obj, const char* name, unsigned long long defValue, const STNBPlistOldNode* parent, const STNBPlistOldNode* afterThis){
	unsigned long long r = defValue;
	const STNBPlistOldNode* nodoTmp = NBPlistOld_childNode(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		const char* strVal = &obj->_strVals.str[nodoTmp->iValueStart];
		NBPLISTOLD_VAL_NUMERIC_UNSIGNED(r, strVal, unsigned long long)
	}
	return r;
}

BOOL NBPlistOld_childBOOL(STNBPlistOld* obj, const char* name, BOOL defValue, const STNBPlistOldNode* parent, const STNBPlistOldNode* afterThis){
	BOOL r = defValue;
	const STNBPlistOldNode* nodoTmp = NBPlistOld_childNode(obj, name, parent, afterThis);
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

float NBPlistOld_childFloat(STNBPlistOld* obj, const char* name, float defValue, const STNBPlistOldNode* parent, const STNBPlistOldNode* afterThis){
	float r = defValue;
	const STNBPlistOldNode* nodoTmp = NBPlistOld_childNode(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		const char* strVal = &obj->_strVals.str[nodoTmp->iValueStart];
		NBPLISTOLD_VAL_NUMERIC_SIGNED(r, strVal, float)
	}
	return r;
}

double NBPlistOld_childDouble(STNBPlistOld* obj, const char* name, double defValue, const STNBPlistOldNode* parent, const STNBPlistOldNode* afterThis){
	double r = defValue;
	const STNBPlistOldNode* nodoTmp = NBPlistOld_childNode(obj, name, parent, afterThis);
	if(nodoTmp != NULL){
		const char* strVal = &obj->_strVals.str[nodoTmp->iValueStart];
		NBPLISTOLD_VAL_NUMERIC_SIGNED(r, strVal, double)
	}
	return r;
}

//Save PLIST (old format)

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
			UI32 iNodo, iNodoConteo = _docNode.childn->use;
			for(iNodo=0; iNodo<iNodoConteo; iNodo++){
				strXML->vaciar();
				AUDatosJSONP<char>::cadenaJSONIdentadaDeNodo(this, &obj->_docNode.childn->elemento[iNodo], ENPlistOldNodeTypee_Array, strXML, 0, archivo);
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
			SI32 iNodo, iNodoConteo = _docNode.childn->use;
			for(iNodo=0; iNodo<iNodoConteo; iNodo++){
				strXML->vaciar();
				AUDatosJSONP<char>::cadenaJSONSinEspaciosDeNodo(this, &obj->_docNode.childn->elemento[iNodo], ENPlistOldNodeTypee_Array, strXML, archivo);
				archivo->escribir(strXML->str(), sizeof(char), strXML->tamano(), archivo);
			}
		}
		strXML->liberar(NB_RETENEDOR_THIS);
		r = TRUE;
	}
	return r;
}

long cadenaJSONIdentada(AUCadenaLargaMutable8* guardarEn){
	long r = AUDatosJSONP<char>::cadenaJSONIdentadaDeNodo(this, &obj->_docNode, ENPlistOldNodeTypee_Array, guardarEn, 1, NULL);
	return r;
}

long cadenaJSONSinEspacios(AUCadenaLargaMutable8* guardarEn){
	long r = AUDatosJSONP<char>::cadenaJSONSinEspaciosDeNodo(this, &obj->_docNode, ENPlistOldNodeTypee_Array, guardarEn);
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

static long cadenaJSONIdentadaDeNodo(AUDatosJSONP<char>* datosXml, STNBPlistOldNode* nodoXml, const ENPlistOldNodeTypee parentType, AUCadenaLargaMutable8* guardarEn, const long nivel, AUArchivo* archivoDst){
	long i;
	//Ident
	const char identChar = '\t';
	guardarEn->repetir(identChar, (const int)nivel);
	//Name
	if(parentType != ENPlistOldNodeTypee_Array){
		guardarEn->agregar('\"');
		guardarEn->agregar(&datosXml->_strTags.str[nodoXml->iNameStart]);
		guardarEn->agregar('\"');
		guardarEn->agregar(':');
		guardarEn->agregar(' ');
	}
	//
	if(nodoXml->type == ENPlistOldNodeTypee_Object){
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
	} else if(nodoXml->type == ENPlistOldNodeTypee_Array){
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
		if(nodoXml->valueType == ENPlistOldNodeValueType_Literal){
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

static long cadenaJSONSinEspaciosDeNodo(AUDatosJSONP<char>* datosXml, STNBPlistOldNode* nodoXml, const ENPlistOldNodeTypee parentType, AUCadenaLargaMutable8* guardarEn, AUArchivo* archivoDst){
	//Name
	if(parentType != ENPlistOldNodeTypee_Array){
		guardarEn->agregar('\"');
		guardarEn->agregar(&datosXml->_strTags.str[nodoXml->iNameStart]);
		guardarEn->agregar('\"');
		guardarEn->agregar(':');
	}
	//Value
	if(nodoXml->type == ENPlistOldNodeTypee_Object){
		guardarEn->agregar('{');
		if(nodoXml->childn != NULL){
			UI32 i, iNodoConteo = nodoXml->childn->use;
			for(i = 0; i < iNodoConteo; i++){
				if(i != 0) guardarEn->agregar(',');
				AUDatosJSONP<char>::cadenaJSONSinEspaciosDeNodo(datosXml, &(nodoXml->childn->elemento[i]), nodoXml->type, guardarEn, archivoDst);
			}
		}
		guardarEn->agregar('}');
	} else if(nodoXml->type == ENPlistOldNodeTypee_Array){
		guardarEn->agregar('[');
		if(nodoXml->childn != NULL){
			UI32 i, iNodoConteo = nodoXml->childn->use;
			for(i = 0; i < iNodoConteo; i++){
				if(i != 0) guardarEn->agregar(',');
				AUDatosJSONP<char>::cadenaJSONSinEspaciosDeNodo(datosXml, &(nodoXml->childn->elemento[i]), nodoXml->type, guardarEn, archivoDst);
			}
		}
		guardarEn->agregar(']');
	} else {
		if(nodoXml->valueType == ENPlistOldNodeValueType_Literal){
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

void NBPlistOld_privNodeInit(STNBPlistOldNode* nodo){
	if(nodo != NULL){
		nodo->iNameStart		= 0;
		nodo->iValueStart		= 0;
		nodo->type				= ENPlistOldNodeTypee_Undef;
		nodo->valueType			= ENPlistOldNodeValueType_Undef;
		nodo->childn		= NULL;
		//
		nodo->hintDeepLvl		= 0;
		nodo->hintChildIdx		= 0;
	}
}

void NBPlistOld_privNodeRelease(STNBPlistOldNode* nodo){
	if(nodo != NULL){
		nodo->iNameStart		= 0;
		nodo->iValueStart		= 0;
		nodo->type				= ENPlistOldNodeTypee_Undef;
		nodo->valueType			= ENPlistOldNodeValueType_Undef;
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

void NBPlistOld_privNodeEmpty(STNBPlistOldNode* nodo){
	if(nodo != NULL){
		if(nodo->childn != NULL){
			while(nodo->childn->use>0){
				STNBPlistOldNode* nodoHijo = (STNBPlistOldNode*)NBArray_itemAtIndex(nodo->childn, nodo->childn->use - 1);
				//Optiminzacion #1 (evitar 1 llamado recursivo)
				if(nodoHijo->childn != NULL){
					while(nodoHijo->childn->use>0){
						STNBPlistOldNode* nodoNieto = (STNBPlistOldNode*)NBArray_itemAtIndex(nodoHijo->childn, nodoHijo->childn->use - 1);
						//Optimizacion #2 (evitar otro llamado recursivo)
						if(nodoNieto->childn != NULL){
							while(nodoNieto->childn->use>0){
								STNBPlistOldNode* nodoBisNieto = (STNBPlistOldNode*)NBArray_itemAtIndex(nodoNieto->childn, nodoNieto->childn->use - 1);
							    NBPlistOld_privNodeEmpty(nodoBisNieto);
							    NBPlistOld_privNodeRelease(nodoBisNieto);
							    NBArray_removeItemAtIndex(nodoNieto->childn, nodoNieto->childn->use - 1);
							}
						}
					    NBPlistOld_privNodeRelease(nodoNieto);
					    NBArray_removeItemAtIndex(nodoHijo->childn, nodoHijo->childn->use - 1);
					}
				}
			    NBPlistOld_privNodeRelease(nodoHijo);
			    NBArray_removeItemAtIndex(nodo->childn, nodo->childn->use - 1);
			}
		}
	}
}

//------------------
// PLIST (old format) load
//------------------

typedef struct STNBPlistOldLoad_ {
	BOOL					plainDataStarted;
	BOOL					plainDataIgnoreCurrent;
	STNBArray				listaLIFO; // STNBPlistOldNode*
	STNBPlistOldParser		state;
	IPlistOldParserListener	listener;
	STNBPlistOld*			obj;
} STNBPlistOldLoad;

//Private

UI32 NBPlistOld_privIdxForTag(STNBPlistOld* obj, const char* tagName);
void NBPlistOld_privJsonLoadingOpenNewNode(const STNBPlistOldParser* state, void* listenerParam);
void NBPlistOld_privJsonLoadingRemoveLastNode(const STNBPlistOldParser* state, void* listenerParam);

//----------------------
// Json parser callbacks
//----------------------

void NBPlistOld_memberStarted(const STNBPlistOldParser* state, void* listenerParam){
	IF_NBASSERT(STNBPlistOldLoad* loadState = (STNBPlistOldLoad*)listenerParam;)
	IF_NBASSERT(STNBArray* listaLIFO = &loadState->listaLIFO;)
	NBASSERT(listaLIFO->use > 0)
#	ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
	//char cDeep[512]; PRINTF_INFO("%s NBPlistOld_memberStarted\n", NBString_strRepeatByte(cDeep, ' ', listaLIFO->use - 1));
#	endif
    NBPlistOld_privJsonLoadingOpenNewNode(state, listenerParam);
}

void NBPlistOld_consumeName(const STNBPlistOldParser* state, const char* unscapedName, void* listenerParam){
	//PRINTF_INFO("JSON, node's name: '%s'.\n", name);
	STNBPlistOldLoad* loadState = (STNBPlistOldLoad*)listenerParam;
	STNBArray* listaLIFO = &loadState->listaLIFO;
    NBASSERT(listaLIFO->use > 0)
#	ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
	//char cDeep[512]; PRINTF_INFO("%sNBPlistOld_consumeName('%s')\n", NBString_strRepeatByte(cDeep, ' ', listaLIFO->use), unscapedName);
#	endif
	if(listaLIFO->use > 0){
		STNBPlistOldNode* curNode = *((STNBPlistOldNode**)NBArray_itemAtIndex(listaLIFO, listaLIFO->use - 1));
	    NBASSERT(curNode->iNameStart == 0)
		if(curNode->iNameStart == 0){
			curNode->iNameStart = NBPlistOld_privIdxForTag(loadState->obj, unscapedName);
		}
	}
}

void NBPlistOld_consumePlain(const STNBPlistOldParser* state, const char* data, const SI32 dataSize, void* listenerParam){
	//PRINTF_INFO("JSON, plain value: %d bytes.\n", dataSize);
	STNBPlistOldLoad* loadState = (STNBPlistOldLoad*)listenerParam;
	STNBArray* listaLIFO = &loadState->listaLIFO;
    NBASSERT(listaLIFO->use > 0)
#	ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
	//char cDeep[512]; PRINTF_INFO("%sNBPlistOld_consumePlain: '%s'\n", NBString_strRepeatByte(cDeep, ' ', listaLIFO->use), data);
#	endif
	if(listaLIFO->use > 0){
		STNBPlistOld* obj = loadState->obj;
		STNBPlistOldNode* curNode = *((STNBPlistOldNode**)NBArray_itemAtIndex(listaLIFO, listaLIFO->use - 1));
	    NBASSERT(curNode->type == ENPlistOldNodeTypee_Undef || curNode->type == ENPlistOldNodeTypee_Pair)
	    NBASSERT(curNode->valueType == ENPlistOldNodeValueType_Undef || curNode->valueType == ENPlistOldNodeValueType_Plain)
		if(curNode->type == ENPlistOldNodeTypee_Undef){
			curNode->type = ENPlistOldNodeTypee_Pair;
			curNode->valueType = ENPlistOldNodeValueType_Plain;
			//Start content
		    NBASSERT(curNode->iValueStart == 0)
			if(curNode->iValueStart == 0){
			    NBString_concatByte(&obj->_strVals, '\0');
				curNode->iValueStart = obj->_strVals.length;
			    NBString_concatBytes(&obj->_strVals, data, dataSize);
			}
		} else if( curNode->type == ENPlistOldNodeTypee_Pair){
		    NBASSERT(curNode->valueType == ENPlistOldNodeValueType_Plain)
			if(curNode->valueType == ENPlistOldNodeValueType_Plain){
				//Continue content
			    NBASSERT(curNode->iValueStart != 0)
				if(curNode->iValueStart != 0){
				    NBString_concatBytes(&obj->_strVals, data, dataSize);
				}
			}
		}
	}
}

void NBPlistOld_consumeHexData(const STNBPlistOldParser* state, const char* hexData, const SI32 dataSize, void* listenerParam){
	//PRINTF_INFO("JSON, literal value: %d bytes.\n", dataSize);
	STNBPlistOldLoad* loadState = (STNBPlistOldLoad*)listenerParam;
	STNBArray* listaLIFO = &loadState->listaLIFO;
	NBASSERT(listaLIFO->use > 0)
#	ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
	//char cDeep[512]; PRINTF_INFO("%sNBPlistOld_consumeLiteral: '%s'\n", NBString_strRepeatByte(cDeep, ' ', listaLIFO->use), hexData);
#	endif
	if(listaLIFO->use > 0){
		STNBPlistOld* obj = loadState->obj;
		STNBPlistOldNode* curNode = *((STNBPlistOldNode**)NBArray_itemAtIndex(listaLIFO, listaLIFO->use - 1));
		NBASSERT(curNode->type == ENPlistOldNodeTypee_Undef || curNode->type == ENPlistOldNodeTypee_Pair)
		NBASSERT(curNode->valueType == ENPlistOldNodeValueType_Undef || curNode->valueType == ENPlistOldNodeValueType_HexData)
		if(curNode->type == ENPlistOldNodeTypee_Undef){
			curNode->type = ENPlistOldNodeTypee_Pair;
			curNode->valueType = ENPlistOldNodeValueType_HexData;
			//Start content
			NBASSERT(curNode->iValueStart == 0)
			if(curNode->iValueStart == 0){
				NBString_concatByte(&obj->_strVals, '\0');
				curNode->iValueStart = obj->_strVals.length;
				NBString_concatBytes(&obj->_strVals, hexData, dataSize);
			}
		} else if( curNode->type == ENPlistOldNodeTypee_Pair){
			NBASSERT(curNode->valueType == ENPlistOldNodeValueType_HexData)
			if(curNode->valueType == ENPlistOldNodeValueType_HexData){
				//Continue content
				NBASSERT(curNode->iValueStart != 0)
				if(curNode->iValueStart != 0){
					NBString_concatBytes(&obj->_strVals, hexData, dataSize);
				}
			}
		}
	}
}

void NBPlistOld_consumeLiteral(const STNBPlistOldParser* state, const char* unscapedData, const SI32 dataSize, void* listenerParam){
	//PRINTF_INFO("JSON, literal value: %d bytes.\n", dataSize);
	STNBPlistOldLoad* loadState = (STNBPlistOldLoad*)listenerParam;
	STNBArray* listaLIFO = &loadState->listaLIFO;
    NBASSERT(listaLIFO->use > 0)
#	ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
	//char cDeep[512]; PRINTF_INFO("%sNBPlistOld_consumeLiteral: '%s'\n", NBString_strRepeatByte(cDeep, ' ', listaLIFO->use), unscapedData);
#	endif
	if(listaLIFO->use > 0){
		STNBPlistOld* obj = loadState->obj;
		STNBPlistOldNode* curNode = *((STNBPlistOldNode**)NBArray_itemAtIndex(listaLIFO, listaLIFO->use - 1));
	    NBASSERT(curNode->type == ENPlistOldNodeTypee_Undef || curNode->type == ENPlistOldNodeTypee_Pair)
	    NBASSERT(curNode->valueType == ENPlistOldNodeValueType_Undef || curNode->valueType == ENPlistOldNodeValueType_Literal)
		if(curNode->type == ENPlistOldNodeTypee_Undef){
			curNode->type = ENPlistOldNodeTypee_Pair;
			curNode->valueType = ENPlistOldNodeValueType_Literal;
			//Start content
		    NBASSERT(curNode->iValueStart == 0)
			if(curNode->iValueStart == 0){
			    NBString_concatByte(&obj->_strVals, '\0');
				curNode->iValueStart = obj->_strVals.length;
			    NBString_concatBytes(&obj->_strVals, unscapedData, dataSize);
			}
		} else if( curNode->type == ENPlistOldNodeTypee_Pair){
		    NBASSERT(curNode->valueType == ENPlistOldNodeValueType_Literal)
			if(curNode->valueType == ENPlistOldNodeValueType_Literal){
				//Continue content
			    NBASSERT(curNode->iValueStart != 0)
				if(curNode->iValueStart != 0){
				    NBString_concatBytes(&obj->_strVals, unscapedData, dataSize);
				}
			}
		}
	}
}

void NBPlistOld_objectStarted(const STNBPlistOldParser* state, void* listenerParam){
	STNBPlistOldLoad* loadState = (STNBPlistOldLoad*)listenerParam;
	STNBArray* listaLIFO = &loadState->listaLIFO;
    NBASSERT(listaLIFO->use > 0)
#	ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
	//char cDeep[512]; PRINTF_INFO("%sNBPlistOld_objectStarted\n", NBString_strRepeatByte(cDeep, ' ', listaLIFO->use));
#	endif
	if(listaLIFO->use > 0){
		STNBPlistOldNode* curNode = *((STNBPlistOldNode**)NBArray_itemAtIndex(listaLIFO, listaLIFO->use - 1));
	    NBASSERT(curNode->type == ENPlistOldNodeTypee_Undef)
		if(curNode->type == ENPlistOldNodeTypee_Undef){
			curNode->type = ENPlistOldNodeTypee_Object;
		}
	}
}

void NBPlistOld_objectEnded(const STNBPlistOldParser* state, void* listenerParam){
	STNBPlistOldLoad* loadState = (STNBPlistOldLoad*)listenerParam;
	STNBArray* listaLIFO = &loadState->listaLIFO;
    NBASSERT(listaLIFO->use > 0)
#	ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
	//char cDeep[512]; PRINTF_INFO("%sNBPlistOld_objectEnded\n", NBString_strRepeatByte(cDeep, ' ', listaLIFO->use));
#	endif
	if(listaLIFO->use > 0){
		IF_NBASSERT(STNBPlistOldNode* curNode = *((STNBPlistOldNode**)NBArray_itemAtIndex(listaLIFO, listaLIFO->use - 1));)
	    NBASSERT(curNode->type == ENPlistOldNodeTypee_Object)
	}
}

void NBPlistOld_arrayStarted(const STNBPlistOldParser* state, void* listenerParam){
	STNBPlistOldLoad* loadState = (STNBPlistOldLoad*)listenerParam;
	STNBArray* listaLIFO = &loadState->listaLIFO;
    NBASSERT(listaLIFO->use > 0)
#	ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
	//char cDeep[512]; PRINTF_INFO("%sNBPlistOld_arrayStarted\n", NBString_strRepeatByte(cDeep, ' ', listaLIFO->use));
#	endif
	if(listaLIFO->use > 0){
		STNBPlistOldNode* curNode = *((STNBPlistOldNode**)NBArray_itemAtIndex(listaLIFO, listaLIFO->use - 1));
	    NBASSERT(curNode->type == ENPlistOldNodeTypee_Undef)
		if(curNode->type == ENPlistOldNodeTypee_Undef){
			curNode->type = ENPlistOldNodeTypee_Array;
		}
	}
}

void NBPlistOld_arrayEnded(const STNBPlistOldParser* state, void* listenerParam){
	STNBPlistOldLoad* loadState = (STNBPlistOldLoad*)listenerParam;
	STNBArray* listaLIFO = &loadState->listaLIFO;
    NBASSERT(listaLIFO->use > 0)
#	ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
	//char cDeep[512]; PRINTF_INFO("%sNBPlistOld_arrayEnded\n", NBString_strRepeatByte(cDeep, ' ', listaLIFO->use));
#	endif
	if(listaLIFO->use > 0){
		IF_NBASSERT(STNBPlistOldNode* curNode = *((STNBPlistOldNode**)NBArray_itemAtIndex(listaLIFO, listaLIFO->use - 1));)
	    NBASSERT(curNode->type == ENPlistOldNodeTypee_Array)
	}
}

void NBPlistOld_memberEnded(const STNBPlistOldParser* state, void* listenerParam){
	IF_NBASSERT(STNBPlistOldLoad* loadState = (STNBPlistOldLoad*)listenerParam;)
	IF_NBASSERT(STNBArray* listaLIFO = &loadState->listaLIFO;)
	NBASSERT(listaLIFO->use > 0)
#	ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
	//char cDeep[512]; PRINTF_INFO("%sNBPlistOld_memberEnded\n", NBString_strRepeatByte(cDeep, ' ', listaLIFO->use - 1));
#	endif
    NBPlistOld_privJsonLoadingRemoveLastNode(state, listenerParam);
}

//----------------------
// JSON load stream
//----------------------

BOOL NBPlistOld_loadStreamStart(STNBPlistOldLoad* state, STNBPlistOld* obj){
    NBPlistOld_privNodeEmpty(&obj->_docNode);
    NBPlistOld_privNodeRelease(&obj->_docNode);
    NBPlistOld_privNodeInit(&obj->_docNode);
	obj->_docNode.type = ENPlistOldNodeTypee_Array;
	//
	state->plainDataStarted					= FALSE;
	state->plainDataIgnoreCurrent			= FALSE;
	//
	STNBPlistOldNode* nodePtr				= &obj->_docNode;
    NBArray_init(&state->listaLIFO, sizeof(STNBPlistOldNode*), NULL);
    NBArray_addValue(&state->listaLIFO, nodePtr);
	//
    NBPlistOldParser_init(&state->state);
	state->listener.memberStarted			= NBPlistOld_memberStarted;
	state->listener.consumeName				= NBPlistOld_consumeName;
	state->listener.consumePlain			= NBPlistOld_consumePlain;
	state->listener.consumeHexData			= NBPlistOld_consumeHexData;
	state->listener.consumeLiteral			= NBPlistOld_consumeLiteral;
	state->listener.objectStarted			= NBPlistOld_objectStarted;
	state->listener.objectEnded				= NBPlistOld_objectEnded;
	state->listener.arrayStarted			= NBPlistOld_arrayStarted;
	state->listener.arrayEnded				= NBPlistOld_arrayEnded;
	state->listener.memberEnded				= NBPlistOld_memberEnded;
	state->obj								= obj;
	return NBPlistOldParser_feedStart(&state->state, &state->listener, state);
}

BOOL NBPlistOld_loadStreamFeed(STNBPlistOldLoad* state, const char* data, const SI32 dataSize){
	BOOL r = FALSE;
    NBASSERT(state != NULL)
	if(state != NULL){
		r = NBPlistOldParser_feed(&state->state, data, dataSize, &state->listener, state);
	}
	return r;
}

BOOL NBPlistOld_loadStreamEnd(STNBPlistOldLoad* state){
	BOOL r = FALSE;
	if(state != NULL){
		if(!NBPlistOldParser_feedEnd(&state->state, &state->listener, state)){
			PRINTF_ERROR("NBPlistOld, stream could not be parsed as a valid json.\n");
			if(state->listaLIFO.use > 0){
				IF_PRINTF(const STNBPlistOldNode* lastNode = (const STNBPlistOldNode*)NBArray_itemAtIndex(&state->listaLIFO, state->listaLIFO.use - 1);)
				PRINTF_ERROR("NBPlistOld, name of last node parsed: '%s'.\n", &state->obj->_strTags.str[lastNode->iNameStart]);
				if(state->listaLIFO.use > 1){
					IF_PRINTF(const STNBPlistOldNode* parentLastNode = (const STNBPlistOldNode*)NBArray_itemAtIndex(&state->listaLIFO, state->listaLIFO.use - 2);)
					PRINTF_ERROR("NBPlistOld, name of parent of last node parsed: '%s'.\n", &state->obj->_strTags.str[parentLastNode->iNameStart]);
				}
			}
		} else {
			//PRINTF_INFO("Lista lifo tiene %d elementos.\n", _loadState->listaLIFO->use);
			//NBASSERT(_loadState->listaLIFO->use == 1)
			r = TRUE;
		}
		NBPlistOldParser_release(&state->state);
	    NBArray_release(&state->listaLIFO);
	}
	return r;
}

//----------------------
// Plist (old format) load from file
//----------------------

BOOL NBPlistOld_loadFromFilePath(STNBPlistOld* obj, const char* filePath){
	BOOL r = FALSE;
    STNBFileRef stream = NBFile_alloc(NULL);
	if(NBFile_open(stream, filePath, ENNBFileMode_Read)){
		NBFile_lock(stream);
		if(!NBPlistOld_loadFromFile(obj, stream)){
			PRINTF_ERROR("interpretando JSON de: '%s'\n", filePath);
		} else {
			r = TRUE;
		}
		NBFile_unlock(stream);
	}
	NBFile_release(&stream);
	return r;
}

BOOL NBPlistOld_loadFromFile(STNBPlistOld* obj, STNBFileRef flujoArchivo){
	BOOL r = FALSE; NBASSERT(NBFile_isSet(flujoArchivo))
	//
	STNBPlistOldLoad state;
	if(NBPlistOld_loadStreamStart(&state, obj)){
		char buff[10240];
		SI32 read = 0;
		do {
			read = NBFile_read(flujoArchivo, buff, 10240);
			if(read > 0){
				if(!NBPlistOld_loadStreamFeed(&state, buff, read)){
					break;
				}
			}
		} while(read > 0);
		//Finish stream
		if(NBPlistOld_loadStreamEnd(&state)){
			r = TRUE;
		}
	}
	return r;
}

// Json load from string

BOOL NBPlistOld_loadFromStr(STNBPlistOld* obj, const char* strData){
	BOOL r = FALSE;
	SI32 elemsRead = 0;
	//Determine data size
	while(strData[elemsRead] != '\0') elemsRead++;
	//
	STNBPlistOldLoad state;
	if(NBPlistOld_loadStreamStart(&state, obj)){
		if(elemsRead > 0){
		    NBPlistOld_loadStreamFeed(&state, strData, elemsRead);
		}
		//Finish stream
		if(NBPlistOld_loadStreamEnd(&state)){
			r = TRUE;
		}
	}
	return r;
}

// Private


UI32 NBPlistOld_privIdxForTag(STNBPlistOld* obj, const char* tagName){
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

void NBPlistOld_privJsonLoadingOpenNewNode(const STNBPlistOldParser* state, void* listenerParam){
	STNBPlistOldLoad* loadState = (STNBPlistOldLoad*)listenerParam;
	STNBArray* listaLIFO = &loadState->listaLIFO;
	STNBPlistOld* obj = loadState->obj;
	//PRINTF_INFO("JSON, opening new node.\n");
	//Determine parent
	STNBPlistOldNode* parent = NULL;
	if(listaLIFO->use == 0){
		parent = &obj->_docNode;
	} else {
		parent = *((STNBPlistOldNode**)NBArray_itemAtIndex(listaLIFO, listaLIFO->use - 1));
	}
    NBASSERT(parent != NULL)
	//If parent is an Array, use his name.
	UI32 nameIdx = 0;
	if(parent->type == ENPlistOldNodeTypee_Array){
		nameIdx = parent->iNameStart;
	}
	//New node
	STNBPlistOldNode nuevoNodo;
    NBPlistOld_privNodeInit(&nuevoNodo);
	nuevoNodo.iNameStart		= nameIdx;
	nuevoNodo.iValueStart		= 0;
	nuevoNodo.type				= ENPlistOldNodeTypee_Undef;
	nuevoNodo.valueType			= ENPlistOldNodeValueType_Undef;
	nuevoNodo.childn		= NULL;
	nuevoNodo.hintDeepLvl 		= listaLIFO->use;
	//Add to root or parent
	{
	    NBASSERT(parent->type == ENPlistOldNodeTypee_Array || parent->type == ENPlistOldNodeTypee_Object)
		if(parent->type == ENPlistOldNodeTypee_Array || parent->type == ENPlistOldNodeTypee_Object){
			if(parent->childn == NULL){
				parent->childn = (STNBArray*)NBMemory_alloc(sizeof(STNBArray));
			    NBArray_init(parent->childn, sizeof(STNBPlistOldNode), NBCompare_STNBPlistOldNode);
			}
			//Add to parent
		    NBArray_addValue(parent->childn, nuevoNodo);
			//Add to loading stack
			STNBPlistOldNode* nodePtr = (STNBPlistOldNode*)NBArray_itemAtIndex(parent->childn, parent->childn->use - 1);
		    NBArray_addValue(listaLIFO, nodePtr);
		} else {
		    NBASSERT(FALSE)
		}
	}
}

void NBPlistOld_privJsonLoadingRemoveLastNode(const STNBPlistOldParser* state, void* listenerParam){
	STNBPlistOldLoad* loadState = (STNBPlistOldLoad*)listenerParam;
	STNBArray* listaLIFO = &loadState->listaLIFO;
	STNBPlistOld* obj = loadState->obj;
	//Remove node if is undefined
	if(listaLIFO->use > 0){
		STNBPlistOldNode* lastNode = *((STNBPlistOldNode**)NBArray_itemAtIndex(listaLIFO, listaLIFO->use - 1));
		if(lastNode->type == ENPlistOldNodeTypee_Undef){
			//TODO: optimization, avoid adding undefined nodes to tree.
			//if(lastNode->valueType != ENPlistOldNodeValueType_Undef){
			//	PRINTF_INFO("Undefined type node removed (value type: %d).\n", (int)lastNode->valueType);
			//}
			STNBPlistOldNode* parent = NULL;
			if(listaLIFO->use <= 1){
				parent = &obj->_docNode;
			} else {
				parent = *(STNBPlistOldNode**)NBArray_itemAtIndex(listaLIFO, listaLIFO->use - 2);
			}
		    NBASSERT(parent != NULL)
		    NBASSERT(parent->childn != NULL)
			if(parent->childn != NULL){
				const STNBPlistOldNode lastNodeCopy = *lastNode;
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


