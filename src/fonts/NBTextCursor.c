//
//  NBTextCursor.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 16/11/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/fonts/NBTextCursor.h"

#include "nb/core/NBMemory.h"


void NBTextCursor_init(STNBTextCursor* obj){
	NBMemory_setZeroSt(*obj, STNBTextCursor);
}

void NBTextCursor_initWithOther(STNBTextCursor* obj, const STNBTextCursor* other){
	NBMemory_setZeroSt(*obj, STNBTextCursor);
	obj->rngCmp = other->rngCmp;
	obj->rngSel = other->rngSel;
}

void NBTextCursor_release(STNBTextCursor* obj){
	//
}

//Sync

void NBTextCursor_syncDataWithOther(STNBTextCursor* obj, const STNBTextCursor* other){
	if(obj != other){
		obj->rngCmp = other->rngCmp;
		obj->rngSel = other->rngSel;
	}
}

//

STNBRangeI NBTextCursor_getRngComposing(const STNBTextCursor* obj){
	return obj->rngCmp;
}

STNBRangeI NBTextCursor_getRngComposingAbs(const STNBTextCursor* obj){
	STNBRangeI r = obj->rngCmp;
	if(r.size < 0){
		r.start	= r.start + r.size;
		r.size	= -r.size;
	}
	NBASSERT(r.start >= 0 && r.size >= 0)
	return r;
}

STNBRangeI NBTextCursor_getRngSelection(const STNBTextCursor* obj){
	return obj->rngSel;
}

STNBRangeI NBTextCursor_getRngSelectionAbs(const STNBTextCursor* obj){
	STNBRangeI r = obj->rngSel;
	if(r.size < 0){
		r.start	= r.start + r.size;
		r.size	= -r.size;
	}
	NBASSERT(r.start >= 0 && r.size >= 0)
	return r;
}

void NBTextCursor_setRngComposing(STNBTextCursor* obj, const SI32 start, const SI32 size, const STNBTextMetrics* metrics){
	//Set
	obj->rngCmp.start	= start;
	obj->rngCmp.size	= size;
	//Validate
	if(metrics != NULL){
		STNBRangeI* rng = &obj->rngCmp;
		//Validate start
		if(rng->start < 0){
			PRINTF_INFO("NBTextCursor composing rangeStart moved to start-of-text.\n");
			rng->size	-= rng->start;
			rng->start	= 0;
		}
		if(rng->start > metrics->chars.use){
			PRINTF_INFO("NBTextCursor composing rangeStart moved to end-of-text.\n");
			rng->size	+= (rng->start - metrics->chars.use);
			rng->start	= metrics->chars.use;
		}
		//Validate size
		if((rng->start + rng->size) < 0){
			PRINTF_INFO("NBTextCursor composing rangeEnd moved to start-of-text.\n");
			rng->size = 0 - rng->start;
		}
		if((rng->start + rng->size) > metrics->chars.use){
			PRINTF_INFO("NBTextCursor composing rangeEnd moved to end-of-text.\n");
			rng->size = metrics->chars.use - rng->start;
		}
		NBASSERT(rng->start >= 0 && rng->start <= metrics->chars.use);
		NBASSERT((rng->start + rng->size) >= 0 && (rng->start + rng->size) <= metrics->chars.use);
#		ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
		if((rng->start + rng->size) < 0 || (rng->start + rng->size) > metrics->chars.use){
			PRINTF_ERROR("New-composing-range(%d, %s%d) out of range for %d charDefs.\n", rng->start, (rng->size < 0 ? "" : "+"), rng->size, metrics->chars.use);
		}
#		endif
	}
}

void NBTextCursor_setRngSelection(STNBTextCursor* obj, const SI32 start, const SI32 size, const STNBTextMetrics* metrics){
	//Set
	obj->rngSel.start	= start;
	obj->rngSel.size	= size;
	//Validate
	if(metrics != NULL){
		STNBRangeI* rng = &obj->rngSel;
		//Validate start
		if(rng->start < 0){
			PRINTF_INFO("NBTextCursor selection rangeStart moved to start-of-text.\n");
			rng->size	-= rng->start;
			rng->start	= 0;
		}
		if(rng->start > metrics->chars.use){
			PRINTF_INFO("NBTextCursor selection rangeStart moved to end-of-text.\n");
			rng->size	+= (rng->start - metrics->chars.use);
			rng->start	= metrics->chars.use;
		}
		//Validate size
		if((rng->start + rng->size) < 0){
			PRINTF_INFO("NBTextCursor selection rangeEnd moved to start-of-text.\n");
			rng->size = 0 - rng->start;
		}
		if((rng->start + rng->size) > metrics->chars.use){
			PRINTF_INFO("NBTextCursor selection rangeEnd moved to end-of-text.\n");
			rng->size = metrics->chars.use - rng->start;
		}
		NBASSERT(rng->start >= 0 && rng->start <= metrics->chars.use);
		NBASSERT((rng->start + rng->size) >= 0 && (rng->start + rng->size) <= metrics->chars.use);
#		ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
		if((rng->start + rng->size) < 0 || (rng->start + rng->size) > metrics->chars.use){
			PRINTF_ERROR("New-selection-range(%d, %s%d) out of range for %d charDefs.\n", rng->start, (rng->size < 0 ? "" : "+"), rng->size, metrics->chars.use);
		}
#		endif
	}
}

