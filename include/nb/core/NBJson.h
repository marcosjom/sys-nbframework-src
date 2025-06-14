//
//  XUXml.h
//  XUMonitorLogicaNegocios_OSX
//
//  Created by Marcos Ortega on 21/01/14.
//  Copyright (c) 2014 XurpasLatam. All rights reserved.
//

#ifndef NB_JSON_H
#define NB_JSON_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"
#include "nb/files/NBFile.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef enum ENJsonNodeTypee_ {
		ENJsonNodeTypee_Undef = 0,
		ENJsonNodeTypee_Pair,		//A linear value
		ENJsonNodeTypee_Object,		//A parent of named values
		ENJsonNodeTypee_Array,		//A parent of unamed values
		ENJsonNodeTypee_Count
	} ENJsonNodeTypee;
	
	typedef enum ENJsonNodeValueType_ {
		ENJsonNodeValueType_Undef = 0,
		ENJsonNodeValueType_Plain,		//Plain value (number, true, false or null)
		ENJsonNodeValueType_Literal,	//Anything between double quotes
		ENJsonNodeValueType_Count
	} ENJsonNodeValueType;
	
	typedef struct STNBJsonNode_ {
		UI32				iNameStart;			//indice del primer caracter en la cadena compartida (optimizacion que evitar crear muchos objetos cadenas pequenos)
		UI32				iValueStart;		//indice del primer caracter en la cadena compartida (optimizacion que evitar crear muchos objetos cadenas pequenos)
		ENJsonNodeTypee		type;
		ENJsonNodeValueType	valueType;
		STNBArray*			childn;				//STNBJsonNode
		//
		UI32				hintDeepLvl;		//Deep level (just a hint)
		UI32				hintChildIdx;		//Index at parent childn (hint for quick search of next node)
	} STNBJsonNode;
	
	BOOL NBCompare_STNBJsonNode(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz);
	
	typedef struct STNBJson_ {
		STNBJsonNode	_rootMmbr;
		STNBString 		_strTags;
		STNBString 		_strVals;
		STNBArray		_idxsTags;	//UI32
	} STNBJson;
	
	//Init and release
	void NBJson_init(STNBJson* obj);
	void NBJson_release(STNBJson* obj);
	
	//Prepare buffer for parsing (optimization)
	void				NBJson_prepareStorageTags(STNBJson* obj, const UI32 strTotalSize);
	void				NBJson_prepareStorageValues(STNBJson* obj, const UI32 strTotalSize);
	
	//Scape
	void			    NBJson_concatScaped(STNBString* dst, const char* unscaped);
	void			    NBJson_concatScapedDQuotesOnly(STNBString* dst, const char* unscaped);
	void			    NBJson_concatScapedBytes(STNBString* dst, const char* unscaped, const UI32 unscapedSz);
	void			    NBJson_concatScapedBytesDQuotesOnly(STNBString* dst, const char* unscaped, const UI32 unscapedSz);
	void			    NBJson_concatUnscaped(STNBString* dst, const char* scaped);
	
	//Query
	const STNBJsonNode*	NBJson_rootMember(STNBJson* obj);
	BOOL				NBJson_nodeIsNull(const STNBJson* obj, const STNBJsonNode* node);
	const char*		    NBJson_nodeName(const STNBJson* obj, const STNBJsonNode* node);
	const char*		    NBJson_nodeStr(const STNBJson* obj, const STNBJsonNode* node, const char* defvalue);
	char				NBJson_nodeChar(const STNBJson* obj, const STNBJsonNode* node, const char defvalue);
	SI8					NBJson_nodeSI8(const STNBJson* obj, const STNBJsonNode* node, const SI8 defvalue);
	UI8					NBJson_nodeUI8(const STNBJson* obj, const STNBJsonNode* node, const UI8 defvalue);
	SI16				NBJson_nodeSI16(const STNBJson* obj, const STNBJsonNode* node, const SI16 defvalue);
	UI16				NBJson_nodeUI16(const STNBJson* obj, const STNBJsonNode* node, const UI16 defvalue);
	SI32				NBJson_nodeSI32(const STNBJson* obj, const STNBJsonNode* node, const SI32 defvalue);
	UI32				NBJson_nodeUI32(const STNBJson* obj, const STNBJsonNode* node, const UI32 defvalue);
	SI64				NBJson_nodeSI64(const STNBJson* obj, const STNBJsonNode* node, const SI64 defvalue);
	UI64				NBJson_nodeUI64(const STNBJson* obj, const STNBJsonNode* node, const UI64 defvalue);
	BOOL				NBJson_nodeBOOL(const STNBJson* obj, const STNBJsonNode* node, const BOOL defvalue);
	float				NBJson_nodeFloat(const STNBJson* obj, const STNBJsonNode* node, const float defvalue);
	double				NBJson_nodeDouble(const STNBJson* obj, const STNBJsonNode* node, const double defvalue);
	
	//Nodes tree
	const STNBJsonNode*	NBJson_childNode(STNBJson* obj, const char* name, const STNBJsonNode* parent, const STNBJsonNode* afterThis);
	const STNBJsonNode*	NBJson_childNodeAfter(STNBJson* obj, const STNBJsonNode* parent, const STNBJsonNode* afterThis);
	const char*		    NBJson_childStr(STNBJson* obj, const char* name, const char* defValue, const STNBJsonNode* parent, const STNBJsonNode* afterThis);
	char			    NBJson_childChar(STNBJson* obj, const char* name, char defValue, const STNBJsonNode* parent, const STNBJsonNode* afterThis);
	SI8					NBJson_childSI8(STNBJson* obj, const char* name, SI8 defValue, const STNBJsonNode* parent, const STNBJsonNode* afterThis);
	UI8		    		NBJson_childUI8(STNBJson* obj, const char* name, UI8 defValue, const STNBJsonNode* parent, const STNBJsonNode* afterThis);
	SI16				NBJson_childSI16(STNBJson* obj, const char* name, SI16 defValue, const STNBJsonNode* parent, const STNBJsonNode* afterThis);
	UI16	    		NBJson_childUI16(STNBJson* obj, const char* name, UI16 defValue, const STNBJsonNode* parent, const STNBJsonNode* afterThis);
	SI32				NBJson_childSI32(STNBJson* obj, const char* name, SI32 defValue, const STNBJsonNode* parent, const STNBJsonNode* afterThis);
	UI32	    		NBJson_childUI32(STNBJson* obj, const char* name, UI32 defValue, const STNBJsonNode* parent, const STNBJsonNode* afterThis);
	SI64		    	NBJson_childSI64(STNBJson* obj, const char* name, SI64 defValue, const STNBJsonNode* parent, const STNBJsonNode* afterThis);
	UI64				NBJson_childUI64(STNBJson* obj, const char* name, UI64 defValue, const STNBJsonNode* parent, const STNBJsonNode* afterThis);
	BOOL			    NBJson_childBOOL(STNBJson* obj, const char* name, BOOL defValue, const STNBJsonNode* parent, const STNBJsonNode* afterThis);
	float			    NBJson_childFloat(STNBJson* obj, const char* name, float defValue, const STNBJsonNode* parent, const STNBJsonNode* afterThis);
	double			    NBJson_childDouble(STNBJson* obj, const char* name, double defValue, const STNBJsonNode* parent, const STNBJsonNode* afterThis);
	
	//Load
	BOOL				NBJson_loadFromFilePath(STNBJson* obj, const char* filePath);
	BOOL				NBJson_loadFromFile(STNBJson* obj, STNBFileRef flujoArchivo);
	BOOL				NBJson_loadFromStr(STNBJson* obj, const char* strData);
	BOOL				NBJson_loadFromStrBytes(STNBJson* obj, const char* strData, const UI32 strSz);

	//Write
	void				NBJson_concat(const STNBJson* obj, const char padChar, STNBString* dst);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
