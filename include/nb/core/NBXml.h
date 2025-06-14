//
//  XUXml.h
//  XUMonitorLogicaNegocios_OSX
//
//  Created by Marcos Ortega on 21/01/14.
//  Copyright (c) 2014 XurpasLatam. All rights reserved.
//

#ifndef NB_XML_H
#define NB_XML_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"
#include "nb/files/NBFile.h"
#include "nb/core/NBXmlParser.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBXmlNodeAttrb_ {
		UI32		iNameStart;		//indice del primer caracter en la cadena compartida (optimizacion que evitar crear muchos objetos cadenas pequenos)
		UI32		iValueStart;	//indice del primer caracter en la cadena compartida (optimizacion que evitar crear muchos objetos cadenas pequenos)
		//
		UI32		hintChildIdx;	//Index at parent attribs (hint for quick search of next attrib)
	} STNBXmlNodeAttrb;
	
	typedef struct STNBXmlNode_ {
		UI32		iNameStart;		//indice del primer caracter en la cadena compartida (optimizacion que evitar crear muchos objetos cadenas pequenos)
		UI32		iContentStart;	//content's first char
		UI32		iContentSz;		//content's chars count (including multiple '\0' addded between nodes)
		STNBArray*	childn;			//STNBXmlNode
		STNBArray*	attribs;		//STNBXmlNodeAttrb
		//
		UI32		hintDeepLvl;	//Deep level (just a hint)
		UI32		hintChildIdx;	//Index at parent childn (hint for quick search of next node)
	} STNBXmlNode;
	
	struct STNBXml_;
	
	typedef struct STNBXmlLoad_ {
		STNBArray			listaLIFO; // STNBXmlNode*
		STNBXmlParser		parser;
		struct STNBXml_*	obj;
	} STNBXmlLoad;
	
	typedef struct STNBXml_ {
		STNBXmlNode	_docNode;
		STNBString 	_strNames;		//Names library (tags and attribs)
		STNBArray	_idxsNames;		//UI32
		STNBString 	_strAttrVals;	//Attribs values
		STNBString 	_strContent;	//Xml content
		STNBXmlLoad* loadState;
	} STNBXml;
	
	//Init and release
	void NBXml_init(STNBXml* obj);
	void NBXml_initWithSizes(STNBXml* obj, const UI32 allTagsNamesSz, const UI32 allAttribsValuesSz, const UI32 contentSz);
	void NBXml_initWithContentSz(STNBXml* obj, const UI32 size);
	void NBXml_release(STNBXml* obj);
	
	//Scape
	void			    NBXml_concatScaped(STNBString* dst, const char* unscaped);
	void			    NBXml_concatScapedBytes(STNBString* dst, const char* unscaped, const UI32 sz);
	//void			    NBXml_concatUnscaped(STNBString* dst, const char* scaped);
	
	//Query
	const char*		    NBXml_nodeName(STNBXml* obj, const STNBXmlNode* node);
	const char*		    NBXml_nodeContent(STNBXml* obj, const STNBXmlNode* node);
	const char*		    NBXml_nodeAttrbName(STNBXml* obj, const STNBXmlNodeAttrb* attrb);
	const char*		    NBXml_nodeAttrbValue(STNBXml* obj, const STNBXmlNodeAttrb* attrb);
	const STNBXmlNode*	NBXml_rootNode(STNBXml* obj);
	
	//Nodes tree
	const STNBXmlNode*	NBXml_childNode(STNBXml* obj, const char* name, const STNBXmlNode* parent, const STNBXmlNode* afterThis);
	const STNBXmlNode*	NBXml_childNodeWithEndOfname(STNBXml* obj, const char* endOfName, const STNBXmlNode* parent, const STNBXmlNode* afterThis);
	const STNBXmlNode*	NBXml_childNodeAfter(STNBXml* obj, const STNBXmlNode* parent, const STNBXmlNode* afterThis);
	const char*		    NBXml_childStr(STNBXml* obj, const char* name, const char* defValue, const STNBXmlNode* parent, const STNBXmlNode* afterThis);
	char			    NBXml_childChar(STNBXml* obj, const char* name, char defValue, const STNBXmlNode* parent, const STNBXmlNode* afterThis);
	int				    NBXml_childInt(STNBXml* obj, const char* name, int defValue, const STNBXmlNode* parent, const STNBXmlNode* afterThis);
	unsigned int	    NBXml_childUint(STNBXml* obj, const char* name, unsigned int defValue, const STNBXmlNode* parent, const STNBXmlNode* afterThis);
	long			    NBXml_childLong(STNBXml* obj, const char* name, long defValue, const STNBXmlNode* parent, const STNBXmlNode* afterThis);
	unsigned long	    NBXml_childUlong(STNBXml* obj, const char* name, unsigned long defValue, const STNBXmlNode* parent, const STNBXmlNode* afterThis);
	long long		    NBXml_childLongLong(STNBXml* obj, const char* name, long long defValue, const STNBXmlNode* parent, const STNBXmlNode* afterThis);
	unsigned long long	NBXml_childULongLong(STNBXml* obj, const char* name, unsigned long long defValue, const STNBXmlNode* parent, const STNBXmlNode* afterThis);
	BOOL			    NBXml_childBOOL(STNBXml* obj, const char* name, BOOL defValue, const STNBXmlNode* parent, const STNBXmlNode* afterThis);
	float			    NBXml_childFloat(STNBXml* obj, const char* name, float defValue, const STNBXmlNode* parent, const STNBXmlNode* afterThis);
	double			    NBXml_childDouble(STNBXml* obj, const char* name, double defValue, const STNBXmlNode* parent, const STNBXmlNode* afterThis);
	
	//Attrib
	const STNBXmlNodeAttrb*	NBXml_childAttrb(STNBXml* obj, const char* name, const STNBXmlNode* parent, const STNBXmlNodeAttrb* afterThis);
	const STNBXmlNodeAttrb*	NBXml_childAttrbAfter(STNBXml* obj, const STNBXmlNode* parent, const STNBXmlNodeAttrb* afterThis);
	const char*		    NBXml_attrbStr(STNBXml* obj, const char* name, const char* defValue, const STNBXmlNode* parent, const STNBXmlNodeAttrb* afterThis);
	char			    NBXml_attrbChar(STNBXml* obj, const char* name, char defValue, const STNBXmlNode* parent, const STNBXmlNodeAttrb* afterThis);
	int				    NBXml_attrbInt(STNBXml* obj, const char* name, int defValue, const STNBXmlNode* parent, const STNBXmlNodeAttrb* afterThis);
	unsigned int	    NBXml_attrbUint(STNBXml* obj, const char* name, unsigned int defValue, const STNBXmlNode* parent, const STNBXmlNodeAttrb* afterThis);
	long			    NBXml_attrbLong(STNBXml* obj, const char* name, long defValue, const STNBXmlNode* parent, const STNBXmlNodeAttrb* afterThis);
	unsigned long	    NBXml_attrbUlong(STNBXml* obj, const char* name, unsigned long defValue, const STNBXmlNode* parent, const STNBXmlNodeAttrb* afterThis);
	long long		    NBXml_attrbLongLong(STNBXml* obj, const char* name, long long defValue, const STNBXmlNode* parent, const STNBXmlNodeAttrb* afterThis);
	unsigned long long	NBXml_attrbULongLong(STNBXml* obj, const char* name, unsigned long long defValue, const STNBXmlNode* parent, const STNBXmlNodeAttrb* afterThis);
	BOOL			    NBXml_attrbBOOL(STNBXml* obj, const char* name, BOOL defValue, const STNBXmlNode* parent, const STNBXmlNodeAttrb* afterThis);
	float			    NBXml_attrbFloat(STNBXml* obj, const char* name, float defValue, const STNBXmlNode* parent, const STNBXmlNodeAttrb* afterThis);
	double			    NBXml_attrbDouble(STNBXml* obj, const char* name, double defValue, const STNBXmlNode* parent, const STNBXmlNodeAttrb* afterThis);
	
	//Load
	BOOL NBXml_loadFromFilePath(STNBXml* obj, const char* filePath);
	BOOL NBXml_loadFromFile(STNBXml* obj, STNBFileRef flujoArchivo);
	BOOL NBXml_loadFromStr(STNBXml* obj, const char* strData);
	
	//
	void NBXml_feedStart(STNBXml* obj);
	BOOL NBXml_feedByte(STNBXml* obj, const char c);
	UI32 NBXml_feed(STNBXml* obj, const char* str);
	UI32 NBXml_feedBytes(STNBXml* obj, const void* data, const UI32 dataSz);
	BOOL NBXml_feedIsComplete(STNBXml* obj);
	BOOL NBXml_feedEnd(STNBXml* obj);
	
	//
	BOOL NBXml_concatContentTo(const STNBXml* obj, STNBString* dst);
	BOOL NBXml_concatContentNodeTo(const STNBXml* obj, const STNBXmlNode* node, STNBString* dst);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
