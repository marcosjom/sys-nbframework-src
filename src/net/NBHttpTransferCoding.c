
#include "nb/NBFrameworkPch.h"
#include "nb/core/NBMemory.h"
#include "nb/net/NBHttpTransferCoding.h"

void NBHttpTransferCoding_init(STNBHttpTransferCoding* obj){
	obj->type			= ENNBHttpTransferCodingType_count;
	obj->extensionType	= NULL;
	obj->params			= NULL;
	obj->paramsStrs		= NULL;
}

void NBHttpTransferCoding_initWithOther(STNBHttpTransferCoding* obj, const STNBHttpTransferCoding* other){
	obj->type			= other->type;
	obj->extensionType	= NULL;
	obj->params			= NULL;
	obj->paramsStrs		= NULL;
	//
	if(other->extensionType != NULL){
		obj->extensionType = (STNBString*)NBMemory_alloc(sizeof(STNBString));
		NBString_initWithOther(obj->extensionType, other->extensionType);
	}
	if(other->params != NULL){
		obj->params = (STNBArray*)NBMemory_alloc(sizeof(STNBArray));
		NBArray_initWithOther(obj->params, other->params);
	}
	if(other->paramsStrs != NULL){
		obj->paramsStrs = (STNBString*)NBMemory_alloc(sizeof(STNBString));
		NBString_initWithOther(obj->paramsStrs, other->paramsStrs);
	}
}

void NBHttpTransferCoding_release(STNBHttpTransferCoding* obj){
	obj->type	= ENNBHttpTransferCodingType_count;
	if(obj->extensionType != NULL){
		NBString_release(obj->extensionType);
		NBMemory_free(obj->extensionType);
		obj->extensionType	= NULL;
	}
	if(obj->params != NULL){
		NBArray_release(obj->params);
		NBMemory_free(obj->params);
		obj->params	= NULL;
	}
	if(obj->paramsStrs != NULL){
		NBString_release(obj->paramsStrs);
		NBMemory_free(obj->paramsStrs);
		obj->paramsStrs	= NULL;
	}
}

//

void NBHttpTransferCoding_setType(STNBHttpTransferCoding* obj, const ENNBHttpTransferCodingType type){
	obj->type = type;
	if(obj->type == ENNBHttpTransferCodingType_extension){
		if(obj->extensionType == NULL){
			obj->extensionType = (STNBString*)NBMemory_alloc(sizeof(STNBString));
			NBString_init(obj->extensionType);
		}
		if(obj->params == NULL){
			obj->params = (STNBArray*)NBMemory_alloc(sizeof(STNBArray));
			NBArray_init(obj->params, sizeof(STNBHttpTransferParam), NULL);
		}
		if(obj->paramsStrs == NULL){
			obj->paramsStrs = (STNBString*)NBMemory_alloc(sizeof(STNBString));
			NBString_init(obj->paramsStrs);
			NBString_concatByte(obj->paramsStrs, '\0');
		}
	}
}

void NBHttpTransferCoding_setTypeExtension(STNBHttpTransferCoding* obj, const char* type){
	obj->type = ENNBHttpTransferCodingType_extension;
	if(obj->extensionType == NULL){
		obj->extensionType = (STNBString*)NBMemory_alloc(sizeof(STNBString));
		NBString_init(obj->extensionType);
	}
	if(obj->params == NULL){
		obj->params = (STNBArray*)NBMemory_alloc(sizeof(STNBArray));
		NBArray_init(obj->params, sizeof(STNBHttpTransferParam), NULL);
	}
	if(obj->paramsStrs == NULL){
		obj->paramsStrs = (STNBString*)NBMemory_alloc(sizeof(STNBString));
		NBString_init(obj->paramsStrs);
		NBString_concatByte(obj->paramsStrs, '\0');
	}
	NBString_set(obj->extensionType, type);
}

void NBHttpTransferCoding_setTypeExtensionBytes(STNBHttpTransferCoding* obj, const char* type, const UI32 typeSz){
	obj->type = ENNBHttpTransferCodingType_extension;
	if(obj->extensionType == NULL){
		obj->extensionType = (STNBString*)NBMemory_alloc(sizeof(STNBString));
		NBString_init(obj->extensionType);
	}
	if(obj->params == NULL){
		obj->params = (STNBArray*)NBMemory_alloc(sizeof(STNBArray));
		NBArray_init(obj->params, sizeof(STNBHttpTransferParam), NULL);
	}
	if(obj->paramsStrs == NULL){
		obj->paramsStrs = (STNBString*)NBMemory_alloc(sizeof(STNBString));
		NBString_init(obj->paramsStrs);
		NBString_concatByte(obj->paramsStrs, '\0');
	}
	NBString_setBytes(obj->extensionType, type, typeSz);
}

void NBHttpTransferCoding_addParam(STNBHttpTransferCoding* obj, const char* token, const char* value){
	STNBHttpTransferParam p;
	//
	if(obj->params == NULL){
		obj->params = (STNBArray*)NBMemory_alloc(sizeof(STNBArray));
		NBArray_init(obj->params, sizeof(STNBHttpTransferParam), NULL);
	}
	if(obj->paramsStrs == NULL){
		obj->paramsStrs = (STNBString*)NBMemory_alloc(sizeof(STNBString));
		NBString_init(obj->paramsStrs);
		NBString_concatByte(obj->paramsStrs, '\0');
	}
	//
	p.iName		= obj->paramsStrs->length; NBString_concat(obj->paramsStrs, token); NBString_concatByte(obj->paramsStrs, '\0');
	p.iValue	= obj->paramsStrs->length; NBString_concat(obj->paramsStrs, value); NBString_concatByte(obj->paramsStrs, '\0');
	NBArray_addValue(obj->params, p);
}

void NBHttpTransferCoding_addParamBytes(STNBHttpTransferCoding* obj, const char* token, const UI32 tokenSz, const char* value, const UI32 valueSz){
	STNBHttpTransferParam p;
	//
	if(obj->params == NULL){
		obj->params = (STNBArray*)NBMemory_alloc(sizeof(STNBArray));
		NBArray_init(obj->params, sizeof(STNBHttpTransferParam), NULL);
	}
	if(obj->paramsStrs == NULL){
		obj->paramsStrs = (STNBString*)NBMemory_alloc(sizeof(STNBString));
		NBString_init(obj->paramsStrs);
		NBString_concatByte(obj->paramsStrs, '\0');
	}
	//
	p.iName		= obj->paramsStrs->length; NBString_concatBytes(obj->paramsStrs, token, tokenSz); NBString_concatByte(obj->paramsStrs, '\0');
	p.iValue	= obj->paramsStrs->length; NBString_concatBytes(obj->paramsStrs, value, valueSz); NBString_concatByte(obj->paramsStrs, '\0');
	NBArray_addValue(obj->params, p);
}




