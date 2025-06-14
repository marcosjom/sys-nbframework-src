//
//  NBTextCursor.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 16/11/18.
//

#ifndef NBTextCursor_h
#define NBTextCursor_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBRange.h"
#include "nb/fonts/NBTextMetrics.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct STNBTextCursor_ {
		STNBRangeI	rngCmp;	//in logic-char units (not byte unit)
		STNBRangeI	rngSel;	//in logic-char units (not byte unit)
	} STNBTextCursor;
	
	void NBTextCursor_init(STNBTextCursor* obj);
	void NBTextCursor_initWithOther(STNBTextCursor* obj, const STNBTextCursor* other);
	void NBTextCursor_release(STNBTextCursor* obj);
	
	//Sync
	void NBTextCursor_syncDataWithOther(STNBTextCursor* obj, const STNBTextCursor* other);
	
	//
	STNBRangeI NBTextCursor_getRngComposing(const STNBTextCursor* obj);
	STNBRangeI NBTextCursor_getRngComposingAbs(const STNBTextCursor* obj);
	STNBRangeI NBTextCursor_getRngSelection(const STNBTextCursor* obj);
	STNBRangeI NBTextCursor_getRngSelectionAbs(const STNBTextCursor* obj);
	
	void NBTextCursor_setRngComposing(STNBTextCursor* obj, const SI32 start, const SI32 size, const STNBTextMetrics* metrics);
	void NBTextCursor_setRngSelection(STNBTextCursor* obj, const SI32 start, const SI32 size, const STNBTextMetrics* metrics);
	
#ifdef __cplusplus
}
#endif

#endif /* NBTextCursor_h */
