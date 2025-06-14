
#include "nb/NBFrameworkPch.h"
#include "nb/core/NBMemory.h"
#include "nb/net/NBHttpChunkExt.h"

void NBHttpChunkExt_init(STNBHttpChunkExt* obj){
	NBArray_init(&obj->exts, sizeof(STNBHttpChunkExtElem), NULL);
	NBString_init(&obj->strs);
}

void NBHttpChunkExt_initWithOther(STNBHttpChunkExt* obj, const STNBHttpChunkExt* other){
	NBArray_initWithOther(&obj->exts, &other->exts);
	NBString_initWithOther(&obj->strs, &other->strs);
}

void NBHttpChunkExt_release(STNBHttpChunkExt* obj){
	NBArray_release(&obj->exts);
	NBString_release(&obj->strs);
}

//chunk-ext      = *( BWS  ";" BWS chunk-ext-name [ BWS  "=" BWS chunk-ext-val ] )
//chunk-ext-name = token
//chunk-ext-val  = token / quoted-string
void NBHttpChunkExt_addExt(STNBHttpChunkExt* obj, const char* name, const char* value){
	STNBHttpChunkExtElem e;
	e.iName		= obj->strs.length; NBString_concat(&obj->strs, name); NBString_concatByte(&obj->strs, '\0');
	e.iValue	= obj->strs.length;
	if(value != NULL){
		if(value[0] != '\0'){
			NBString_concat(&obj->strs, value);
		}
	}
	NBString_concatByte(&obj->strs, '\0');
	NBArray_addValue(&obj->exts, e);
}

