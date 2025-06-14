
#include "nb/NBFrameworkPch.h"
#include "nb/core/NBEncoding.h"
#include "nb/core/NBNumParser.h"
#include "nb/core/NBXmlParser.h"
#include "nb/core/NBCompare.h"

//XML 1.0 Specs (UNICODE)
#define NBXML_v10_IS_VALID_CHAR(C)					(C == 0x9 || C == 0xA || C == 0xD || (C >= 0x20 && C <= 0xD7FF) || (C >= 0xE000 && C <= 0xFFFD) || (C >= 0x10000 && C <= 0x10FFFF)) /* XML 1.0 spec, any Code character, excluding the surrogate blocks, FFFE, and FFFF. */
#define NBXML_v10_IS_SPACE_CHAR(C)					(C == ' ' || C == '\t' || C == '\r' || C == '\n')

//XML 1.1 Specs (UNICODE)
#define NBXML_v11_IS_VALID_CHAR(C)					((C >= 0x1 && C <= 0xD7FF) || (C >= 0xE000 && C <= 0xFFFD) || (C >= 0x10000 && C <= 0x10FFFF)) /* XML 1.1 spec, any Code character, excluding the surrogate blocks, FFFE, and FFFF. */
#define NBXML_v11_IS_RESTRICTED_CHAR(C)				((C >= 0x1 && C <= 0x8) || (C >= 0xB && C <= 0xC) || (C >= 0xE && C <= 0x1F) || (C >= 0x7F && C <= 0x84) || (C >= 0x86 && C <= 0x9F))
#define NBXML_v11_IS_SPACE_CHAR(C)					(C == ' ' || C == '\t' || C == '\r' || C == '\n')
//
#define NBXML_v11_IS_VALID_NAME_FIRST_CHAR(C)		(C == ':' || (C >= 'A' && C <= 'Z') || C == '_' || (C >= 'a' && C <= 'z') || (C >= 0xC0 && C <= 0xD6) || (C >= 0xD8 && C <= 0xF6) || (C >= 0xF8 && C <= 0x2FF) || (C >= 0x370 && C <= 0x37D) || (C >= 0x37F && C <= 0x1FFF) || (C >= 0x200C && C <= 0x200D) || (C >= 0x2070 && C <= 0x218F) || (C >= 0x2C00 && C <= 0x2FEF) || (C >= 0x3001 && C <= 0xD7FF) || (C >= 0xF900 && C <= 0xFDCF) || (C >= 0xFDF0 && C <= 0xFFFD) || (C >= 0x10000 && C <= 0xEFFFF))
#define NBXML_v11_IS_VALID_NAME_CHAR(C)				(NBXML_v11_IS_VALID_NAME_FIRST_CHAR(C) || C == '-' || C == '.' || ((C >= '0' && C <= '9')) || C == 0xB7 || (C >= 0x0300 && C <= 0x036F) || (C >= 0x203F && C <= 0x2040))


void NBXmlParser_init(STNBXmlParser* state){
	NBXmlParser_initWithListener(state, NULL, NULL);
}

void NBXmlParser_initWithListener(STNBXmlParser* state, const IXmlParserListener* listener, void* listenerParam){
	//Encoding load
	state->encBuff.bytesAcum	= 0;	//Current bytes accumulated
	state->encBuff.bytesExpct	= 0;	//Current bytes expected
	//
	state->fmtLogicError	= FALSE;
	NBArray_init(&state->tagsStackIdxs, sizeof(UI32), NBCompareUI32);
	NBString_init(&state->tagsStack);
	NBString_init(&state->curTagName);
	NBString_init(&state->curAttrbName);
	NBString_init(&state->curScapeSeq);
	NBString_init(&state->strAcum);
	//
	NBArray_init(&state->typesStack, sizeof(ENNBXmlParserType), NULL);
	state->curType			= ENNBXmlParserType_content;
	state->tagsCount		= 0;
	//
	if(listener == NULL){
		NBMemory_set(&state->listener, 0, sizeof(IXmlParserListener));
	} else {
		NBMemory_copy(&state->listener, listener, sizeof(IXmlParserListener));
	}
	state->listenerParam	= listenerParam;
}

void NBXmlParser_release(STNBXmlParser* state){
	//Encoding load
	state->encBuff.bytesAcum	= 0;	//Current bytes accumulated
	state->encBuff.bytesExpct	= 0;	//Current bytes expected
	//
	state->fmtLogicError	= FALSE;
	NBArray_release(&state->tagsStackIdxs);
	NBString_release(&state->tagsStack);
	NBString_release(&state->curTagName);
	NBString_release(&state->curAttrbName);
	NBString_release(&state->curScapeSeq);
	NBString_release(&state->strAcum);
	//
	NBArray_release(&state->typesStack);
	state->curType			= ENNBXmlParserType_content;
	state->tagsCount		= 0;
	//
	NBMemory_set(&state->listener, 0, sizeof(IXmlParserListener));
	state->listenerParam	= NULL;
}

BOOL NBXmlParser_feedStart(STNBXmlParser* state){
	//Encoding load
	state->encBuff.bytesAcum	= 0;	//Current bytes accumulated
	state->encBuff.bytesExpct	= 0;	//Current bytes expected
	//Init values
	state->fmtLogicError	= FALSE;
	NBArray_empty(&state->tagsStackIdxs);
	NBString_empty(&state->tagsStack);
	NBString_empty(&state->curTagName);
	NBString_empty(&state->curAttrbName);
	NBString_empty(&state->curScapeSeq);
	NBString_empty(&state->strAcum);
	//
	state->curType			= ENNBXmlParserType_content;
	state->tagsCount		= 0;
	NBArray_empty(&state->typesStack);
	NBArray_addValue(&state->typesStack, state->curType);
	//Add root tag to stack (for future validation)
	{
		const UI32 pos = state->tagsStack.length; NBASSERT(pos == 0)
		NBArray_addValue(&state->tagsStackIdxs, pos);
		//NBString_concatByte(&state->tagsStack, '/');
	}
	//
	return TRUE;
}

BOOL NBXmlParser_feedIsComplete(STNBXmlParser* state){
	//Only "root" node must remain
	return (state->encBuff.bytesAcum == 0 && state->encBuff.bytesExpct == 0 && !state->fmtLogicError && state->tagsStackIdxs.use == 1 ? TRUE : FALSE);
}

BOOL NBXmlParser_feedEnd(STNBXmlParser* state){
	//Validate encoding buffer state
	NBASSERT(state->encBuff.bytesAcum == 0 && state->encBuff.bytesExpct == 0)
#	ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
	if(state->encBuff.bytesAcum != 0 && state->encBuff.bytesExpct != 0){
		PRINTF_WARNING("XML parse error: incomplete encoded char (%d of %d bytes loaded).\n", state->encBuff.bytesAcum, state->encBuff.bytesExpct);
	}
#	endif
	//
	NBASSERT(state->typesStack.use == 1)					//Only the root type must remain
	NBASSERT(state->curType == ENNBXmlParserType_content)	//Only the root type must remain
#	ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
	if(!state->fmtLogicError && state->tagsStackIdxs.use != 1){
		PRINTF_WARNING("XML parse error: %d tags remained open at the end.\n", (state->tagsStackIdxs.use - 1));
		SI32 i; const SI32 count = state->tagsStackIdxs.use;
		for(i = 0; i < count; i++){
			const char* strTmp = &state->tagsStack.str[NBArray_itmValueAtIndex(&state->tagsStackIdxs, UI32, i)];
			PRINTF_WARNING("XML parse error: TAG STILL OPEN #%d '%s'.\n", (i + 1), strTmp);
		}
		//NBASSERT(0) //Do not assert, this is posible when user cancels the connection (or network problems).
	}
#	endif
	//Only "root" node must remain
	return (state->encBuff.bytesAcum == 0 && state->encBuff.bytesExpct == 0 && !state->fmtLogicError && state->tagsStackIdxs.use == 1 ? TRUE : FALSE);
}

void NBXmlParser_typeStackPush(STNBXmlParser* state, const ENNBXmlParserType type){
	state->curType		= type;
	NBArray_addValue(&state->typesStack, state->curType);
}

void NBXmlParser_typeStackReplace(STNBXmlParser* state, const ENNBXmlParserType type){
	NBASSERT(state->typesStack.use > 1)
	if(state->typesStack.use > 1){
		state->curType		= type;
		NBArray_setItemAt(&state->typesStack, state->typesStack.use - 1, &state->curType, sizeof(state->curType));
	} else {
		state->fmtLogicError = TRUE; NBASSERT(FALSE)
	}
}

void NBXmlParser_typeStackPop(STNBXmlParser* state){
	NBASSERT(state->typesStack.use > 1)
	if(state->typesStack.use > 1){
		NBArray_removeItemAtIndex(&state->typesStack, state->typesStack.use - 1);
		state->curType = NBArray_itmValueAtIndex(&state->typesStack, ENNBXmlParserType, state->typesStack.use - 1);
	} else {
		state->fmtLogicError = TRUE; NBASSERT(FALSE)
	}
}

BOOL NBXmlParser_feed_unicode_char(STNBXmlParser* state, const UI32 unicode, const char* encBytes, const UI8 encBytesSz){
	if(!NBXML_v11_IS_VALID_CHAR(unicode)){
		PRINTF_ERROR("XmlParser, not valid unicode char #%u found.\n", unicode);
		state->fmtLogicError = TRUE; NBASSERT(FALSE)
	} else {
		switch(state->curType) {
			//Plain content or document's root
			case ENNBXmlParserType_content:			//content outside markup
				if(unicode == '<'){
					//Flush content
					if(state->strAcum.length != 0){
						if(state->listener.consumeNodeContent != NULL){
							(*state->listener.consumeNodeContent)(state, state->tagsStack.str, state->tagsStack.length, state->curTagName.str, state->strAcum.str, state->strAcum.length, state->listenerParam);
						}
						NBString_empty(&state->strAcum);
					}
					//Push a markup
					NBString_empty(&state->curTagName);
					NBString_empty(&state->curAttrbName);
					NBXmlParser_typeStackPush(state, ENNBXmlParserType_tag);
					state->tagsCount++;
				} else if(state->tagsCount == 0){
					PRINTF_ERROR("XmlParser, expected '<' as first char at xml.\n")
					state->fmtLogicError = TRUE; NBASSERT(FALSE)
				} else {
					//Add to element's content
					NBString_concatBytes(&state->strAcum, encBytes, encBytesSz);
				}
				break;
			//Inside markup
			case ENNBXmlParserType_tag:			//found '<', expecting '/' or '?', tag-name, '!' (when tag name has been not defined); or attrib name when tagname is defined
				if(state->curTagName.length == 0){
					//No tag-name yet
					if(unicode == '/'){
						//This is a closing-tag (expecting tag-name only)
						NBXmlParser_typeStackReplace(state, ENNBXmlParserType_tagNameOnly);
					} else if(unicode == '?'){
						//This is the xml definition (document header)
						NBXmlParser_typeStackReplace(state, ENNBXmlParserType_declarOpen);
					} else if(unicode == '!'){
						//This is a comment or CDATA start
						NBXmlParser_typeStackReplace(state, ENNBXmlParserType_tagDataOpen);
					} else if(NBXML_v11_IS_VALID_NAME_FIRST_CHAR(unicode)){
						//Start reading tag's name
						NBString_concatBytes(&state->curTagName, encBytes, encBytesSz);
						NBXmlParser_typeStackPush(state, ENNBXmlParserType_tagName);
					} else {
						PRINTF_ERROR("XmlParser, tag name cannot start with #%u.\n", unicode);
						state->fmtLogicError = TRUE; NBASSERT(FALSE)
					}
				} else {
					//Tag-name already found
					if(unicode == '>'){
						//PRINTF_INFO("XmlParser opened tag %s.\n", state->tagsStack.str);
						NBXmlParser_typeStackPop(state);
					} else if(unicode == '/'){
						//Found '/' and expecting '>'
						//PRINTF_INFO("XmlParser opened tag %s.\n", state->tagsStack.str);
						NBXmlParser_typeStackReplace(state, ENNBXmlParserType_tagEndStart);
					} else if(NBXML_v11_IS_VALID_NAME_FIRST_CHAR(unicode)){
						//Start an attribute's name
						NBASSERT(state->curAttrbName.length == 0)
						NBString_concatBytes(&state->curAttrbName, encBytes, encBytesSz);
						NBXmlParser_typeStackPush(state, ENNBXmlParserType_tagAttrbName);
					}
				}
				break;
			//----------------------
			// '<![CDATA[' ... ']]>'
			//----------------------
			case ENNBXmlParserType_tagDataOpen:	//found '<!', expecting '[CDATA[' or '-' (for comment)
				if(unicode == '-' && state->strAcum.length == 0){
					//Change to comment
					NBString_concatBytes(&state->strAcum, encBytes, encBytesSz);
					NBXmlParser_typeStackReplace(state, ENNBXmlParserType_commOpen);
				} else {
					//Vaidate '[CDATA[' prefix
					if((state->strAcum.length == 0 && unicode == '[') || (state->strAcum.length == 1 && unicode == 'C')
					   || (state->strAcum.length == 2 && unicode == 'D') || (state->strAcum.length == 3 && unicode == 'A')
					   || (state->strAcum.length == 4 && unicode == 'T') || (state->strAcum.length == 5 && unicode == 'A')
					   ){
						//Acumulate data prefix
						NBString_concatBytes(&state->strAcum, encBytes, encBytesSz);
					} else if(state->strAcum.length == 6 && unicode == '['){
						//data prefix '[CDATA[' completed, move to data
						NBString_empty(&state->strAcum);
						NBXmlParser_typeStackReplace(state, ENNBXmlParserType_tagData);
					} else {
						PRINTF_ERROR("XmlParser, expected '--' or '[CDATA[' after '<!'.\n")
						state->fmtLogicError = TRUE; NBASSERT(FALSE)
					}
				}
				break;
			case ENNBXmlParserType_tagData:		//found '<![CDATA[', expecting anything but ']]'
				if(unicode == ']'){
					//Posible data end (ignore the ']')
					NBXmlParser_typeStackReplace(state, ENNBXmlParserType_tagDataEnd);
				} else {
					//Accum data
					NBString_concatBytes(&state->strAcum, encBytes, encBytesSz);
				}
				break;
			case ENNBXmlParserType_tagDataEnd:	//found ']', analyzing posible end
				if(unicode == ']'){
					//Data end, expect '>'
					NBXmlParser_typeStackReplace(state, ENNBXmlParserType_tagDataEnd2);
				} else {
					//Last ']' was part of the data, go back to data-mode
					NBString_concatByte(&state->strAcum, ']');	//Restore the ignored ']'
					NBString_concatBytes(&state->strAcum, encBytes, encBytesSz);		//Add current char
					NBXmlParser_typeStackReplace(state, ENNBXmlParserType_tagData);
				}
				break;
			case ENNBXmlParserType_tagDataEnd2:	//found ']]', expecting '>'
				if(unicode == '>'){
					//Flush content
					//PRINTF_INFO("XmlParser, CDATA has %d bytes.\n", state->strAcum.length);
					if(state->strAcum.length != 0){
						if(state->listener.consumeNodeContent != NULL){
							(*state->listener.consumeNodeContent)(state, state->tagsStack.str, state->tagsStack.length, state->curTagName.str, state->strAcum.str, state->strAcum.length, state->listenerParam);
						}
						NBString_empty(&state->strAcum);
					}
					//Pop
					NBXmlParser_typeStackPop(state);
				} else {
					PRINTF_ERROR("XmlParser, expected '>' after '<[CDATA[...]]'.\n");
					state->fmtLogicError = TRUE; NBASSERT(FALSE)
				}
				break;
			case ENNBXmlParserType_tagName:		//found '<a', expecting any sequence of valid chars for a name
			case ENNBXmlParserType_tagNameOnly:	//found '</a', expecting any sequence of valid chars for a name followed by '>'
				{
					BOOL added = FALSE;
					if(state->curTagName.length == 0){
						//Name's first char
						if(NBXML_v11_IS_VALID_NAME_FIRST_CHAR(unicode)){
							NBString_concatBytes(&state->curTagName, encBytes, encBytesSz);
							added = TRUE;
						}
					} else {
						//Name's additional chars
						if(NBXML_v11_IS_VALID_NAME_CHAR(unicode)){
							NBString_concatBytes(&state->curTagName, encBytes, encBytesSz);
							added = TRUE;
						}
					}
					if(!added){
						if(state->curTagName.length == 0){
							PRINTF_ERROR("XmlParser, expected a tag-name.\n");
							state->fmtLogicError = TRUE; NBASSERT(FALSE)
						} else {
							//Process tag-name
							//PRINTF_INFO("XmlParser, tag-name '%s'.\n", state->curTagName.str);
							if(state->curType == ENNBXmlParserType_tagName){
								NBXmlParser_typeStackPop(state);
								NBASSERT(state->curType == ENNBXmlParserType_tag)
								//Push node
								const UI32 nameIdx	= state->tagsStack.length;
								NBArray_addValue(&state->tagsStackIdxs, nameIdx);
								NBString_concatByte(&state->tagsStack, '/');
								NBString_concat(&state->tagsStack, state->curTagName.str);
								//Notify
								if(state->listener.consumeNodeOpening != NULL){
									(*state->listener.consumeNodeOpening)(state, state->tagsStack.str, state->tagsStack.length, state->curTagName.str, state->listenerParam);
								}
							} else {
								NBASSERT(state->curType == ENNBXmlParserType_tagNameOnly)
								NBXmlParser_typeStackReplace(state, ENNBXmlParserType_tagEndEnd);
							}
							//Process the unused-char (recursively)
							if(!NBXmlParser_feed_unicode_char(state, unicode, encBytes, encBytesSz)){
								state->fmtLogicError = TRUE; NBASSERT(FALSE)
							}
						}
					}
				}
				break;
			//----------------------
			// 'attrib=value'
			//----------------------
			case ENNBXmlParserType_tagAttrbName:	//found 'a', expecting any sequence of valid chars for a name until '='
				NBASSERT(state->curAttrbName.length != 0)
				if(NBXML_v11_IS_VALID_NAME_CHAR(unicode)){
					NBString_concatBytes(&state->curAttrbName, encBytes, encBytesSz);
				} else {
					//Process attrib name
					NBXmlParser_typeStackReplace(state, ENNBXmlParserType_tagAttrbEq);
					//Process the unused-char (recursively)
					if(!NBXmlParser_feed_unicode_char(state, unicode, encBytes, encBytesSz)){
						state->fmtLogicError = TRUE; NBASSERT(FALSE)
					}
				}
				break;
			case ENNBXmlParserType_tagAttrbEq:	//found 'a', expecting '='
				if(unicode == '='){
					NBString_empty(&state->strAcum);
					NBXmlParser_typeStackReplace(state, ENNBXmlParserType_tagAttrbVal);
				} else if(NBXML_v11_IS_SPACE_CHAR(unicode)){
					//Ignore
				} else {
					PRINTF_ERROR("XmlParser, expected '=' after attrib's name (spaces are allowed).\n");
					state->fmtLogicError = TRUE; NBASSERT(FALSE)
				}
				break;
			case ENNBXmlParserType_tagAttrbVal:	//found 'a=', expecting double-quote as first char untill next double-quote
				NBASSERT(state->curAttrbName.length != 0)
				if(unicode == '\''){
					NBXmlParser_typeStackReplace(state, ENNBXmlParserType_tagAttrbValS);
				} else if(unicode == '\"'){
					NBXmlParser_typeStackReplace(state, ENNBXmlParserType_tagAttrbValD);
				} else if(NBXML_v11_IS_SPACE_CHAR(unicode)){
					//Ignore
				} else {
					PRINTF_ERROR("XmlParser, expected '\"', \"'\" or space after attrib's value.\n");
					state->fmtLogicError = TRUE; NBASSERT(FALSE)
				}
				break;
			case ENNBXmlParserType_tagAttrbValS:	//found 'a=', expecting double-quote as first char untill next double-quote
				if(unicode == '\''){
					//Consume literal attrib
					//PRINTF_INFO("XmlParser, attrib '%s' with value '%s'.\n", state->curAttrbName.str, state->strAcum.str);
					NBString_empty(&state->curAttrbName);
					NBString_empty(&state->strAcum);
					NBXmlParser_typeStackPop(state);
				} else if(unicode == '&'){
					//Start scape squence
					NBString_empty(&state->curScapeSeq);
					NBXmlParser_typeStackPush(state, ENNBXmlParserType_scapeSeq);
				} else {
					NBString_concatBytes(&state->strAcum, encBytes, encBytesSz);
				}
				break;
			case ENNBXmlParserType_tagAttrbValD:	//found 'a=', expecting double-quote as first char untill next double-quote
				if(unicode == '\"'){
					//Consume literal attrib
					//PRINTF_INFO("XmlParser, attrib '%s' with value '%s'.\n", state->curAttrbName.str, state->strAcum.str);
					//Notify
					if(state->listener.consumeNodeAttrb != NULL){
						(*state->listener.consumeNodeAttrb)(state, state->tagsStack.str, state->tagsStack.length, state->curTagName.str, state->curAttrbName.str, state->strAcum.str, state->strAcum.length, state->listenerParam);
					}
					NBString_empty(&state->curAttrbName);
					NBString_empty(&state->strAcum);
					NBXmlParser_typeStackPop(state);
				} else if(unicode == '&'){
					//Start scape squence
					NBString_empty(&state->curScapeSeq);
					NBXmlParser_typeStackPush(state, ENNBXmlParserType_scapeSeq);
				} else {
					NBString_concatBytes(&state->strAcum, encBytes, encBytesSz);
				}
				break;
			case ENNBXmlParserType_tagEndStart:		//found '/', expecting '>'
			case ENNBXmlParserType_tagEndEnd:		//found '</name', expecting '>'
				if(unicode == '>'){
					//PRINTF_INFO("XmlParser, closd tag %s.\n", state->tagsStack.str);
					NBXmlParser_typeStackPop(state); //Pop the current tag
					//Validate tag name
					const UI32 lastNodeIdx	= NBArray_itmValueAtIndex(&state->tagsStackIdxs, UI32, state->tagsStackIdxs.use - 1);
					const char* lasNodeName = &state->tagsStack.str[lastNodeIdx + 1];
					if(!NBString_strIsEqual(state->curTagName.str, lasNodeName)){
						PRINTF_ERROR("XmlParser, closing node '%s' does not match with open node '%s'.\n", state->curTagName.str, lasNodeName);
						state->fmtLogicError = TRUE; NBASSERT(FALSE)
					} else {
						//Pop node name
						NBArray_removeItemAtIndex(&state->tagsStackIdxs, state->tagsStackIdxs.use - 1);
						NBString_removeLastBytes(&state->tagsStack, state->tagsStack.length - lastNodeIdx);
						//Notify
						if(state->listener.consumeNodeClosing != NULL){
							(*state->listener.consumeNodeClosing)(state, state->tagsStack.str, state->tagsStack.length, state->curTagName.str, state->listenerParam);
						}
					}
				} else {
					PRINTF_ERROR("XmlParser, expected '>' after '/'.\n")
					state->fmtLogicError = TRUE; NBASSERT(FALSE)
				}
				break;
			//----------------------
			// '<?xml' ... '?>'
			//----------------------
			case ENNBXmlParserType_declarOpen:		//found '<?' expecting 'xml'
				//Vaidate 'xml' prefix
				if((state->strAcum.length == 0 && unicode == 'x') || (state->strAcum.length == 1 && unicode == 'm')){
					//Acumulate data prefix
					NBString_concatBytes(&state->strAcum, encBytes, encBytesSz);
				} else if(state->strAcum.length == 2 && unicode == 'l'){
					//data prefix 'xml' completed, move to declar
					NBString_empty(&state->strAcum);
					NBXmlParser_typeStackReplace(state, ENNBXmlParserType_declar);
				} else {
					PRINTF_ERROR("XmlParser, expected 'xml' after '<?'.\n")
					state->fmtLogicError = TRUE; NBASSERT(FALSE)
				}
				break;
			case ENNBXmlParserType_declar:			//found '<?xml' expecting anything but '?'
				if(unicode == '?'){
					//PRINTF_INFO("XmlParser, xml declaration (header): '%s'.\n", state->strAcum.str);
					NBString_empty(&state->strAcum);
					NBXmlParser_typeStackReplace(state, ENNBXmlParserType_declarEnd);
				} else {
					//Accumulate or process attributes
					NBString_concatBytes(&state->strAcum, encBytes, encBytesSz);
				}
				break;
			case ENNBXmlParserType_declarEnd:		//found '?' expecting '>'
				if(unicode == '>'){
					NBXmlParser_typeStackPop(state);
				} else {
					PRINTF_ERROR("XmlParser, expected '>' after '<?xml ... ?'.\n")
					state->fmtLogicError = TRUE; NBASSERT(FALSE)
				}
				break;
			//----------------------
			// '<!--' ... '-->'
			//----------------------
			case ENNBXmlParserType_commOpen:	//found '<!', expecting '--'
				//Vaidate '--' prefix
				if((state->strAcum.length == 0 && unicode == '-')){
					//Acumulate comment prefix
					NBString_concatBytes(&state->strAcum, encBytes, encBytesSz);
				} else if(state->strAcum.length == 1 && unicode == '-'){
					//comment prefix '--' completed, move to comment
					NBString_empty(&state->strAcum);
					NBXmlParser_typeStackReplace(state, ENNBXmlParserType_comm);
				} else {
					PRINTF_ERROR("XmlParser, expected '--' or '[CDATA[' after '<!'.\n")
					state->fmtLogicError = TRUE; NBASSERT(FALSE)
				}
				break;
			case ENNBXmlParserType_comm:		//found '<!--', expecting anything except '--'
				//"--" (double-hyphen) is not allowed inside comments
				if(unicode == '-'){
					//Posible '--'
					NBXmlParser_typeStackReplace(state, ENNBXmlParserType_commEnd);
				} else {
					NBString_concatBytes(&state->strAcum, encBytes, encBytesSz);
				}
				break;
			case ENNBXmlParserType_commEnd:		//found '-', analyzing posible end
				if(unicode == '-'){
					//Comment's end
					NBXmlParser_typeStackReplace(state, ENNBXmlParserType_commEnd2);
				} else {
					//The last '-' was not part of the end "--"
					NBString_concatByte(&state->strAcum, '-'); //Add ignored char
					NBString_concatBytes(&state->strAcum, encBytes, encBytesSz); //Add current char
					NBXmlParser_typeStackReplace(state, ENNBXmlParserType_commEnd);
				}
				break;
			case ENNBXmlParserType_commEnd2:	//found '--', expecting '>'
				if(unicode == '>'){
					//Process comment and empty
					//PRINTF_INFO("XmlParser, comment has %d bytes.\n", state->strAcum.length);
					NBString_empty(&state->strAcum);
					//Pop
					NBXmlParser_typeStackPop(state);
				} else {
					PRINTF_ERROR("XmlParser, expected '>' after '--' inside a comment.\n")
					state->fmtLogicError = TRUE; NBASSERT(FALSE)
				}
				break;
			//----------------------
			// '&' ... ';'
			//----------------------
			case ENNBXmlParserType_scapeSeq:
				if(state->curScapeSeq.length == 0){
					if(unicode == '#'){
						//Numeric scape-seq
						NBXmlParser_typeStackReplace(state, ENNBXmlParserType_scapeSeqN);
					} else {
						//Start entity name's seq
						NBString_concatBytes(&state->curScapeSeq, encBytes, encBytesSz);
					}
				} else {
					if(unicode == ';'){
						if(NBString_isEqual(&state->curScapeSeq, "lt")){
							NBString_concatByte(&state->strAcum, '<');
							NBXmlParser_typeStackPop(state);
						} else if(NBString_isEqual(&state->curScapeSeq, "gt")){
							NBString_concatByte(&state->strAcum, '>');
							NBXmlParser_typeStackPop(state);
						} else if(NBString_isEqual(&state->curScapeSeq, "amp")){
							NBString_concatByte(&state->strAcum, '&');
							NBXmlParser_typeStackPop(state);
						} else if(NBString_isEqual(&state->curScapeSeq, "apos")){
							NBString_concatByte(&state->strAcum, '\'');
							NBXmlParser_typeStackPop(state);
						} else if(NBString_isEqual(&state->curScapeSeq, "quot")){
							NBString_concatByte(&state->strAcum, '\"');
							NBXmlParser_typeStackPop(state);
						} else {
							//ToDo: process entity reference
							PRINTF_ERROR("XmlParser, entity references are not supported yet for '#%s;'.\n", state->curScapeSeq.str);
							state->fmtLogicError = TRUE; NBASSERT(FALSE)
							NBXmlParser_typeStackPop(state);
						}
					} else {
						//Continue seq
						NBString_concatBytes(&state->curScapeSeq, encBytes, encBytesSz);
					}
				}
				break;
			case ENNBXmlParserType_scapeSeqN:
				if(state->curScapeSeq.length == 0){
					if(unicode == 'x'){
						//Numeric scape-seq
						NBString_concat(&state->curScapeSeq, "0x");
						NBXmlParser_typeStackReplace(state, ENNBXmlParserType_scapeSeqNX);
					} else if(unicode >= '0' && unicode <= '9'){
						//Start seq
						NBString_concatBytes(&state->curScapeSeq, encBytes, encBytesSz);
					} else {
						PRINTF_ERROR("XmlParser, expected digit after '&#'.\n")
						state->fmtLogicError = TRUE; NBASSERT(FALSE)
					}
				} else if(unicode >= '0' && unicode <= '9'){
					//Continue seq
					NBString_concatBytes(&state->curScapeSeq, encBytes, encBytesSz);
					if(state->curScapeSeq.length > 10){ //Max: '4294967295' on UI32
						PRINTF_ERROR("XmlParser, number after '&#' is too long '%s%c'.\n", state->curScapeSeq.str, (char)unicode);
						state->fmtLogicError = TRUE; NBASSERT(FALSE)
					}
				} else if(unicode == ';'){
					//Pop and feed unicode
					BOOL sucess = FALSE; char utf8[7];
					const UI32 unicode2  = NBNumParser_toUI32(state->curScapeSeq.str, &sucess); NBASSERT(sucess)
					NBEncoding_utf8FromUnicode(unicode2, utf8);
					NBString_concat(&state->strAcum, utf8);
					NBXmlParser_typeStackPop(state);
				} else {
					PRINTF_ERROR("XmlParser, expected a number after '&#', not '%s%c'.\n", state->curScapeSeq.str, (char)unicode);
					state->fmtLogicError = TRUE; NBASSERT(FALSE)
				}
				break;
			case ENNBXmlParserType_scapeSeqNX:
				if((unicode >= '0' && unicode <= '9') || (unicode >= 'A' && unicode <= 'F') || (unicode >= 'a' && unicode <= 'f')){
					//Continue seq
					NBString_concatBytes(&state->curScapeSeq, encBytes, encBytesSz);
					if(state->curScapeSeq.length > 10){ //Max: '0xFFFFFFFF' on UI32
						PRINTF_ERROR("XmlParser, hex-number after '&#' is too long '%s%c'.\n", state->curScapeSeq.str, (char)unicode);
						state->fmtLogicError = TRUE; NBASSERT(FALSE)
					}
				} else if(unicode == ';'){
					//Pop and feed unicode
					BOOL sucess = FALSE; char utf8[7];
					const UI32 unicode2  = NBNumParser_toUI32(state->curScapeSeq.str, &sucess); NBASSERT(sucess)
					NBEncoding_utf8FromUnicode(unicode2, utf8);
					NBString_concat(&state->strAcum, utf8);
					NBXmlParser_typeStackPop(state);
				} else {
					PRINTF_ERROR("XmlParser, expected a number after '&#', not '%s%c'.\n", state->curScapeSeq.str, (char)unicode);
					state->fmtLogicError = TRUE; NBASSERT(FALSE)
				}
				break;
			default:
				PRINTF_ERROR("XmlParser, program error, unexpected type.\n");
				state->fmtLogicError = TRUE; NBASSERT(FALSE)
				break;
		}
	}
	return (!state->fmtLogicError);
}

BOOL NBXmlParser_feedBytes(STNBXmlParser* state, const void* pData, const SI32 dataSz){
	const char* data = (const char*)pData;
	UI32 iData = 0;
	while(iData < dataSz && !state->fmtLogicError){
		const char c = data[iData++];
		if(state->encBuff.bytesExpct == 0){
			NBASSERT(state->encBuff.bytesAcum == 0)
			const UI8 sz = NBEncoding_utf8BytesExpected(c); NBASSERT(sz > 0 && sz <= 8)
			if(sz == 1){
				//One-byte char
				if(!NBXmlParser_feed_unicode_char(state, c, &c, 1)){
					state->fmtLogicError = TRUE; NBASSERT(FALSE)
					break;
				}
			} else if(sz == 0){
				PRINTF_ERROR("XmlParser, found malformed unicode char.\n");
				state->fmtLogicError = TRUE; NBASSERT(FALSE)
				break;
			} else {
				//Multi-byte char
				state->encBuff.data[0]		= c;
				state->encBuff.bytesAcum	= 1;
				state->encBuff.bytesExpct	= sz;
			}
		} else {
			//Accumulating multibyte
			state->encBuff.data[state->encBuff.bytesAcum++] = c;
			if(state->encBuff.bytesAcum == state->encBuff.bytesExpct){
				const UI32 unicode = NBEncoding_unicodeFromUtf8s(state->encBuff.data, state->encBuff.bytesAcum, 0);
				if(unicode == 0){
					PRINTF_ERROR("XmlParser, found malformed unicode char.\n");
					state->fmtLogicError = TRUE; NBASSERT(FALSE)
					break;
				} else {
					//Multi-byte char
					if(!NBXmlParser_feed_unicode_char(state, unicode, state->encBuff.data, state->encBuff.bytesAcum)){
						state->fmtLogicError = TRUE; NBASSERT(FALSE)
						break;
					} else {
						//Reset state
						state->encBuff.bytesAcum	= 0;
						state->encBuff.bytesExpct	= 0;
					}
				}
			}
		}
	}
	//
	return (!state->fmtLogicError);
}

