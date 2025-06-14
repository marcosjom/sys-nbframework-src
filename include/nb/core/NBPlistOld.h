//
//  XUXml.h
//  XUMonitorLogicaNegocios_OSX
//
//  Created by Marcos Ortega on 21/01/14.
//  Copyright (c) 2014 XurpasLatam. All rights reserved.
//

#ifndef NB_PLIST_OLD_H
#define NB_PLIST_OLD_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"
#include "nb/files/NBFile.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef enum ENPlistOldNodeTypee_ {
		ENPlistOldNodeTypee_Undef = 0,
		ENPlistOldNodeTypee_Pair,		//A linear value
		ENPlistOldNodeTypee_Object,		//A parent of named values
		ENPlistOldNodeTypee_Array,		//A parent of unamed values
		ENPlistOldNodeTypee_Count
	} ENPlistOldNodeTypee;
	
	typedef enum ENPlistOldNodeValueType_ {
		ENPlistOldNodeValueType_Undef = 0,
		ENPlistOldNodeValueType_Plain,		//Plain value (number, true, false or null)
		ENPlistOldNodeValueType_HexData,	//Anything between double quotes
		ENPlistOldNodeValueType_Literal,	//Anything between double quotes
		ENPlistOldNodeValueType_Count
	} ENPlistOldNodeValueType;
	
	typedef struct STNBPlistOldNode_ {
		UI32					iNameStart;		//indice del primer caracter en la cadena compartida (optimizacion que evitar crear muchos objetos cadenas pequenos)
		UI32					iValueStart;	//indice del primer caracter en la cadena compartida (optimizacion que evitar crear muchos objetos cadenas pequenos)
		ENPlistOldNodeTypee		type;
		ENPlistOldNodeValueType	valueType;
		STNBArray*				childn;		//STNBPlistOldNode
		//
		UI32					hintDeepLvl;	//Deep level (just a hint)
		UI32					hintChildIdx;	//Index at parent childn (hint for quick search of next node)
	} STNBPlistOldNode;
	
	BOOL NBCompare_STNBPlistOldNode(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz);

	typedef struct STNBPlistOld_ {
		STNBPlistOldNode	_docNode;
		STNBString 			_strTags;
		STNBString 			_strVals;
		STNBArray			_idxsTags;	//UI32
	} STNBPlistOld;
	
	//Init and release
	void NBPlistOld_init(STNBPlistOld* obj);
	void NBPlistOld_release(STNBPlistOld* obj);
	
	//Scape
	void			    	NBPlistOld_concatScaped(STNBString* dst, const char* unscaped);
	void			    	NBPlistOld_concatUnscaped(STNBString* dst, const char* scaped);
	
	//Query
	const char*		   		NBPlistOld_nodeName(STNBPlistOld* obj, const STNBPlistOldNode* node);
	const char*		    	NBPlistOld_nodeValue(STNBPlistOld* obj, const STNBPlistOldNode* node);
	const STNBPlistOldNode*	NBPlistOld_docNode(STNBPlistOld* obj);
	
	//Nodes tree
	const STNBPlistOldNode*	NBPlistOld_childNode(STNBPlistOld* obj, const char* name, const STNBPlistOldNode* parent, const STNBPlistOldNode* afterThis);
	const STNBPlistOldNode*	NBPlistOld_childNodeAfter(STNBPlistOld* obj, const STNBPlistOldNode* parent, const STNBPlistOldNode* afterThis);
	const char*		    	NBPlistOld_childStr(STNBPlistOld* obj, const char* name, const char* defValue, const STNBPlistOldNode* parent, const STNBPlistOldNode* afterThis);
	char			    	NBPlistOld_childChar(STNBPlistOld* obj, const char* name, char defValue, const STNBPlistOldNode* parent, const STNBPlistOldNode* afterThis);
	int				    	NBPlistOld_childInt(STNBPlistOld* obj, const char* name, int defValue, const STNBPlistOldNode* parent, const STNBPlistOldNode* afterThis);
	unsigned int	    	NBPlistOld_childUint(STNBPlistOld* obj, const char* name, unsigned int defValue, const STNBPlistOldNode* parent, const STNBPlistOldNode* afterThis);
	long			    	NBPlistOld_childLong(STNBPlistOld* obj, const char* name, long defValue, const STNBPlistOldNode* parent, const STNBPlistOldNode* afterThis);
	unsigned long	    	NBPlistOld_childUlong(STNBPlistOld* obj, const char* name, unsigned long defValue, const STNBPlistOldNode* parent, const STNBPlistOldNode* afterThis);
	long long		    	NBPlistOld_childLongLong(STNBPlistOld* obj, const char* name, long long defValue, const STNBPlistOldNode* parent, const STNBPlistOldNode* afterThis);
	unsigned long long		NBPlistOld_childULongLong(STNBPlistOld* obj, const char* name, unsigned long long defValue, const STNBPlistOldNode* parent, const STNBPlistOldNode* afterThis);
	BOOL			    	NBPlistOld_childBOOL(STNBPlistOld* obj, const char* name, BOOL defValue, const STNBPlistOldNode* parent, const STNBPlistOldNode* afterThis);
	float			    	NBPlistOld_childFloat(STNBPlistOld* obj, const char* name, float defValue, const STNBPlistOldNode* parent, const STNBPlistOldNode* afterThis);
	double			    	NBPlistOld_childDouble(STNBPlistOld* obj, const char* name, double defValue, const STNBPlistOldNode* parent, const STNBPlistOldNode* afterThis);
	
	//Load
	BOOL NBPlistOld_loadFromFilePath(STNBPlistOld* obj, const char* filePath);
	BOOL NBPlistOld_loadFromFile(STNBPlistOld* obj, STNBFileRef flujoArchivo);
	BOOL NBPlistOld_loadFromStr(STNBPlistOld* obj, const char* strData);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
