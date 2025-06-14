//
//  XUCadena.c
//  MetricaMobilFramework
//
//  Created by Marcos Ortega on 07/10/13.
//  Copyright (c) 2013 logicoim@gmail.com. All rights reserved.
//

#include "nb/NBFrameworkPch.h"
#include "nb/core/NBStruct.h"
#include "nb/crypto/NBBase64.h"

//Release

BOOL NBStruct_stReleaseOneMemberValue_(const STNBStructMap* structMap, const STNBStructMapMember* m, void* mSrc, const UI32 mSz);
BOOL NBStruct_stReleaseMember_(const STNBStructMap* structMap, void* pSrc, STNBStructMapMember* m);
BOOL NBStruct_stReleaseMembers_(const STNBStructMap* structMap, const UI16 mbrsMax, void* pSrc, const UI32 srcSz);

//Load from file

BOOL NBStruct_stReadFromJsonFilepath(const char* filepath, const STNBStructMap* structMap, void* dst, const UI32 dstSz){
	BOOL r = FALSE;
    STNBFileRef f = NBFile_alloc(NULL);
	if(NBFile_open(f, filepath, ENNBFileMode_Read)){
		NBFile_lock(f);
		r = NBStruct_stReadFromJsonFile(f, structMap, dst, dstSz);
		NBFile_unlock(f);
	}
	NBFile_release(&f);
	return r;
}

BOOL NBStruct_stReadFromJsonFilepathAtRoot(STNBFilesystem* fs, const ENNBFilesystemRoot root, const char* filepath, const STNBStructMap* structMap, void* dst, const UI32 dstSz){
	BOOL r = FALSE;
    STNBFileRef f = NBFile_alloc(NULL);
	if(NBFilesystem_openAtRoot(fs, root, filepath, ENNBFileMode_Read, f)){
		NBFile_lock(f);
		r = NBStruct_stReadFromJsonFile(f, structMap, dst, dstSz);
		NBFile_unlock(f);
	}
	NBFile_release(&f);
	return r;
}

BOOL NBStruct_stReadFromJsonFilepathStrsAndNullAtRoot(STNBFilesystem* fs, const ENNBFilesystemRoot root, const char** arrAndNull, const STNBStructMap* structMap, void* dst, const UI32 dstSz){
	BOOL r = FALSE;
	STNBString filepath;
	NBString_initWithStrsAndNull(&filepath, arrAndNull);
	{
		STNBFileRef f = NBFile_alloc(NULL);
		if(NBFilesystem_openAtRoot(fs, root, filepath.str, ENNBFileMode_Read, f)){
			NBFile_lock(f);
			r = NBStruct_stReadFromJsonFile(f, structMap, dst, dstSz);
			NBFile_unlock(f);
		}
		NBFile_release(&f);
	}
	NBString_release(&filepath);
	return r;
}

BOOL NBStruct_stReadFromJsonFile(STNBFileRef file, const STNBStructMap* structMap, void* dst, const UI32 dstSz){
	BOOL r = FALSE;
	if(structMap->stSize != dstSz){
		NBASSERT(FALSE)
	} else {
		STNBJson json;
		NBJson_init(&json);
		if(NBJson_loadFromFile(&json, file)){
			/*{
				STNBString str;
				NBString_init(&str);
				NBJson_concat(&json, '\t', &str);
				PRINTF_INFO("NBStruct, loaded: %s\n", str.str);
				NBString_release(&str);
			}*/
			r = NBStruct_stReadFromJsonNode(&json, NBJson_rootMember(&json), structMap, dst, dstSz);
		}
		NBJson_release(&json);
	}
	return r;
}

//Load from JSON node

BOOL NBStruct_stReadFromJsonNodeOneMemberChild_(STNBJson* json, const STNBJsonNode* pNode, const STNBStructMap* structMap, STNBStructMapMember* mm, void* mDst, const UI32 mDstSz){
	BOOL r = TRUE;
	if(mm->data.size != mDstSz){
		r = FALSE; NBASSERT(FALSE)
	} else {
		switch(mm->data.type) {
			case ENNBStructMapMemberType_Bool:
				switch(mm->data.size) {
					case 1: *((UI8*)mDst) = (UI8)NBJson_childBOOL(json, mm->name, 0, pNode, NULL); break;
					case 2: *((UI16*)mDst) = (UI16)NBJson_childBOOL(json, mm->name, 0, pNode, NULL); break;
					case 4: *((UI32*)mDst) = (UI32)NBJson_childBOOL(json, mm->name, 0, pNode, NULL); break;
					case 8: *((UI64*)mDst) = (UI64)NBJson_childBOOL(json, mm->name, 0, pNode, NULL); break;
#					ifdef NB_CONFIG_INCLUDE_ASSERTS
					default:
						NBASSERT(FALSE) 
						break;
#					endif
				}
				break;
			case ENNBStructMapMemberType_Int:
				switch(mm->data.size) {
					case 1: *((SI8*)mDst) = (SI8)NBJson_childSI8(json, mm->name, 0, pNode, NULL); break;
					case 2: *((SI16*)mDst) = (SI16)NBJson_childSI16(json, mm->name, 0, pNode, NULL); break;
					case 4: *((SI32*)mDst) = (SI32)NBJson_childSI32(json, mm->name, 0, pNode, NULL); break;
					case 8: *((SI64*)mDst) = (SI64)NBJson_childSI64(json, mm->name, 0, pNode, NULL); break;
#					ifdef NB_CONFIG_INCLUDE_ASSERTS
					default:
						NBASSERT(FALSE) 
						break;
#					endif
				}
				break;
			case ENNBStructMapMemberType_UInt:
				switch(mm->data.size) {
					case 1: *((UI8*)mDst) = (UI8)NBJson_childUI8(json, mm->name, 0, pNode, NULL); break;
					case 2: *((UI16*)mDst) = (UI16)NBJson_childUI16(json, mm->name, 0, pNode, NULL); break;
					case 4: *((UI32*)mDst) = (UI32)NBJson_childUI32(json, mm->name, 0, pNode, NULL); break;
					case 8: *((UI64*)mDst) = (UI64)NBJson_childUI64(json, mm->name, 0, pNode, NULL); break;
#					ifdef NB_CONFIG_INCLUDE_ASSERTS
					default:
						NBASSERT(FALSE) 
						break;
#					endif
				}
				break;
			case ENNBStructMapMemberType_Float:
				NBASSERT(mm->data.size == sizeof(float))
				*((float*)mDst) = NBJson_childFloat(json, mm->name, 0, pNode, NULL);
				break;
			case ENNBStructMapMemberType_Double:
				NBASSERT(mm->data.size == sizeof(double))
				*((double*)mDst) = NBJson_childDouble(json, mm->name, 0, pNode, NULL);
				break;
			case ENNBStructMapMemberType_Chars:
				{
					const char* str = NBJson_childStr(json, mm->name, NULL, pNode, NULL);
					if(str != NULL){
						const UI32 strSz = NBString_strLenBytes(str); NBASSERT(strSz <= mm->data.size)
						NBMemory_copy((char*)mDst, str, (strSz < mm->data.size ? strSz : mm->data.size));
					}
				}
				break;
			case ENNBStructMapMemberType_Bytes:
				{
					const char* base64 = NBJson_childStr(json, mm->name, NULL, pNode, NULL);
					if(base64 != NULL){
						const UI32 base64Sz = NBString_strLenBytes(base64);
						if((base64Sz % 4) != 0){
							r = FALSE; NBASSERT(FALSE)
						} else if(base64Sz > 0){
							UI32 i, i2 = 0; UI8 bytes; char buff[3]; char* dst = (char*)mDst;
							for(i = 0; i < base64Sz; i += 4){
								bytes = NBBase64_decode4Bytes(&base64[i], buff);
								if(bytes == 0 || (i2 + bytes) > mm->data.size){
									r = FALSE; NBASSERT(FALSE)
									break;
								} else {
									dst[i2++] = buff[0];
									if(bytes > 1) dst[i2++] = buff[1];
									if(bytes > 2) dst[i2++] = buff[2];
								}
							}
						}
					}
				}
				break;
			case ENNBStructMapMemberType_StrPtr:
				NBASSERT(mm->data.size == sizeof(void*))
				{
					const char* str = NBJson_childStr(json, mm->name, NULL, pNode, NULL);
					if(str == NULL){
						*((void**)mDst) = NULL;
					} else {
						const UI32 strSz = NBString_strLenBytes(str);
						char* cpy = (char*)NBMemory_alloc(strSz + 1);
						NBMemory_copy(cpy, str, strSz + 1);
						*((void**)mDst) = cpy;
					}
				}
				break;
			case ENNBStructMapMemberType_Enum:
				if(mm->data.enMap == NULL){
					r = FALSE; NBASSERT(FALSE)
				} else {
					SI64 intValue = 0;
					const char* val = NBJson_childStr(json, mm->name, NULL, pNode, NULL);
					if(val != NULL){
						UI32 i; for(i = 0; i < mm->data.enMap->recordsSz; i++){
							const STNBEnumMapRecord* r = &mm->data.enMap->records[i];
							if(NBString_strIsEqual(val, r->strValue)){
								intValue = r->intValue;
								break;
							}
						}
						if(i == mm->data.enMap->recordsSz){
							PRINTF_ERROR("Value '%s' was not found at enum '%s'.\n", val, mm->data.enMap->enumName);
							r = FALSE; NBASSERT(FALSE)
						}
					}
					switch(mm->data.size) {
						case 1: *((SI8*)mDst) = (SI8)intValue; break;
						case 2: *((SI16*)mDst) = (SI16)intValue; break;
						case 4: *((SI32*)mDst) = (SI32)intValue; break;
						case 8: *((SI64*)mDst) = (SI64)intValue; break;
#						ifdef NB_CONFIG_INCLUDE_ASSERTS
						default:
							NBASSERT(FALSE) 
							break;
#						endif
					}
				}
				break;
			case ENNBStructMapMemberType_Struct:
				if(mm->data.stMap == NULL){
					NBASSERT(FALSE)
					r = FALSE;
				} else {
					NBASSERT(mm->data.size == mm->data.stMap->stSize)
					const STNBJsonNode* chld = NBJson_childNode(json, mm->name, pNode, NULL);
					if(chld == NULL){
						NBMemory_set(mDst, 0, mm->data.stMap->stSize);
					} else {
						if(!NBStruct_stReadFromJsonNode(json, chld, mm->data.stMap, mDst, mm->data.stMap->stSize)){
							NBASSERT(FALSE)
							r = FALSE;
						}
					}
				}
				break;
			default:
				NBASSERT(FALSE)
				break;
		}
	}
	return r;
}

BOOL NBStruct_stReadFromJsonNodeOneMemberValue_(STNBJson* json, const STNBJsonNode* node, const STNBStructMap* structMap, STNBStructMapMember* mm, void* mDst, const UI32 mDstSz){
	BOOL r = TRUE;
	if(mm->data.size != mDstSz){
		r = FALSE; NBASSERT(FALSE)
	} else {
		switch(mm->data.type) {
			case ENNBStructMapMemberType_Bool:
				switch(mm->data.size) {
					case 1: *((UI8*)mDst) = (UI8)NBJson_nodeBOOL(json, node, 0); break;
					case 2: *((UI16*)mDst) = (UI16)NBJson_nodeBOOL(json, node, 0); break;
					case 4: *((UI32*)mDst) = (UI32)NBJson_nodeBOOL(json, node, 0); break;
					case 8: *((UI64*)mDst) = (UI64)NBJson_nodeBOOL(json, node, 0); break;
#					ifdef NB_CONFIG_INCLUDE_ASSERTS
					default:
						NBASSERT(FALSE) 
						break;
#					endif
				}
				break;
			case ENNBStructMapMemberType_Int:
				switch(mm->data.size) {
					case 1: *((SI8*)mDst) = (SI8)NBJson_nodeSI8(json, node, 0); break;
					case 2: *((SI16*)mDst) = (SI16)NBJson_nodeSI16(json, node, 0); break;
					case 4: *((SI32*)mDst) = (SI32)NBJson_nodeSI32(json, node, 0); break;
					case 8: *((SI64*)mDst) = (SI64)NBJson_nodeSI64(json, node, 0); break;
#					ifdef NB_CONFIG_INCLUDE_ASSERTS
					default:
						NBASSERT(FALSE) 
						break;
#					endif
				}
				break;
			case ENNBStructMapMemberType_UInt:
				switch(mm->data.size) {
					case 1: *((UI8*)mDst) = (UI8)NBJson_nodeUI8(json, node, 0); break;
					case 2: *((UI16*)mDst) = (UI16)NBJson_nodeUI16(json, node, 0); break;
					case 4: *((UI32*)mDst) = (UI32)NBJson_nodeUI32(json, node, 0); break;
					case 8: *((UI64*)mDst) = (UI64)NBJson_nodeUI64(json, node, 0); break;
#					ifdef NB_CONFIG_INCLUDE_ASSERTS
					default:
						NBASSERT(FALSE) 
						break;
#					endif
				}
				break;
			case ENNBStructMapMemberType_Float:
				NBASSERT(mm->data.size == sizeof(float))
				*((float*)mDst) = NBJson_nodeFloat(json, node, 0);
				break;
			case ENNBStructMapMemberType_Double:
				NBASSERT(mm->data.size == sizeof(double))
				*((double*)mDst) = NBJson_nodeDouble(json, node, 0);
				break;
			case ENNBStructMapMemberType_Chars:
				{
					const char* str = NBJson_nodeStr(json, node, NULL);
					if(str != NULL){
						const UI32 strSz = NBString_strLenBytes(str);
						NBMemory_copy((char*)mDst, str, (strSz < mm->data.size ? strSz : mm->data.size));
					}
				}
				break;
			case ENNBStructMapMemberType_Bytes:
				{
					const char* base64 = NBJson_nodeStr(json, node, NULL);
					if(base64 != NULL){
						const UI32 base64Sz = NBString_strLenBytes(base64);
						if((base64Sz % 4) != 0){
							r = FALSE; NBASSERT(FALSE)
						} else if(base64Sz > 0){
							UI32 i, i2 = 0; UI8 bytes; char buff[3]; char* dst = (char*)mDst;
							for(i = 0; i < base64Sz; i += 4){
								bytes = NBBase64_decode4Bytes(&base64[i], buff);
								if(bytes == 0 || (i2 + bytes) > mm->data.size){
									r = FALSE; NBASSERT(FALSE)
									break;
								} else {
									dst[i2++] = buff[0];
									if(bytes > 1) dst[i2++] = buff[1];
									if(bytes > 2) dst[i2++] = buff[2];
								}
							}
						}
					}
				}
				break;
			case ENNBStructMapMemberType_StrPtr:
				NBASSERT(mm->data.size == sizeof(void*))
				{
					const char* str = NBJson_nodeStr(json, node, NULL);
					if(str == NULL){
						*((void**)mDst) = NULL;
					} else {
						const UI32 strSz = NBString_strLenBytes(str);
						char* cpy = (char*)NBMemory_alloc(strSz + 1);
						NBMemory_copy(cpy, str, strSz + 1);
						*((void**)mDst) = cpy;
					}
				}
				break;
			case ENNBStructMapMemberType_Enum:
				if(mm->data.enMap == NULL){
					r = FALSE; NBASSERT(FALSE)
				} else {
					SI64 intValue = 0;
					const char* val = NBJson_nodeStr(json, node, NULL);
					if(val != NULL){
						UI32 i; for(i = 0; i < mm->data.enMap->recordsSz; i++){
							const STNBEnumMapRecord* r = &mm->data.enMap->records[i];
							if(NBString_strIsEqual(val, r->strValue)){
								intValue = r->intValue;
								break;
							}
						}
						if(i == mm->data.enMap->recordsSz){
							PRINTF_ERROR("Value '%s' was not found at enum '%s'.\n", val, mm->data.enMap->enumName);
							r = FALSE; NBASSERT(FALSE)
						}
					}
					switch(mm->data.size) {
						case 1: *((SI8*)mDst) = (SI8)intValue; break;
						case 2: *((SI16*)mDst) = (SI16)intValue; break;
						case 4: *((SI32*)mDst) = (SI32)intValue; break;
						case 8: *((SI64*)mDst) = (SI64)intValue; break;
#						ifdef NB_CONFIG_INCLUDE_ASSERTS
						default:
							NBASSERT(FALSE) 
							break;
#						endif
					}
				}
				break;
			case ENNBStructMapMemberType_Struct:
				if(mm->data.stMap == NULL){
					NBASSERT(FALSE)
					r = FALSE;
				} else {
					NBASSERT(mm->data.size == mm->data.stMap->stSize)
					if(node == NULL){
						NBMemory_set(mDst, 0, mm->data.stMap->stSize);
					} else {
						if(!NBStruct_stReadFromJsonNode(json, node, mm->data.stMap, mDst, mm->data.stMap->stSize)){
							NBASSERT(FALSE)
							r = FALSE;
						}
					}
				}
				break;
			default:
				NBASSERT(FALSE)
				break;
		}
	}
	return r;
}
	
BOOL NBStruct_stReadFromJsonNode(STNBJson* json, const STNBJsonNode* pNode, const STNBStructMap* structMap, void* pDst, const UI32 dstSz){
	BOOL r = FALSE;
	if(structMap->stSize != dstSz){
		NBASSERT(FALSE)
	} else {
		UI16 i = 0;
		//flags success
		r = TRUE;
		//Load members
		{
			BYTE* dst = (BYTE*)pDst;
			for(; i < structMap->mbrsSz && r; i++){
				STNBStructMapMember* m = &structMap->mbrs[i];
				NBASSERT(m->size > 0 && m->size == (m->elem.count * (m->elem.isPtr ? sizeof(void*) : m->data.size)))
				if(m->elem.count == 1){
					//------------------
					//- Only one element
					//------------------
					if(!m->elem.isPtr){
						//One embeded element
						r = NBStruct_stReadFromJsonNodeOneMemberChild_(json, pNode, structMap, m, &dst[m->iPos], m->data.size);
					} else {
						//One emeded pointer
						if(m->elem.vCount == NULL){
							//Pointer to one element
							const STNBJsonNode* n = NBJson_childNode(json, m->name, pNode, NULL);
							if(n == NULL){
								*((void**)&dst[m->iPos]) = NULL;
							} else {
								if(NBJson_nodeIsNull(json, n)){
									*((void**)&dst[m->iPos]) = NULL;
								} else {
									void* d = NBMemory_alloc(m->data.size);
									NBMemory_set(d, 0, m->data.size);
									if(NBStruct_stReadFromJsonNodeOneMemberChild_(json, pNode, structMap, m, d, m->data.size)){
										*((void**)&dst[m->iPos]) = d;
									} else {
										NBMemory_free(d);
										r = FALSE;
									}
								}
							}
						} else {
							//Pointer to an array
							STNBStructMapMember* mSz = m->elem.vCount;
							UI64 maxCount = 0;
							*((void**)&dst[m->iPos]) = NULL;
							//Get max posible count
							switch(mSz->data.type) {
							case ENNBStructMapMemberType_Int:
								switch(mSz->size) {
								case 1: maxCount = 127LL; break;
								case 2: maxCount = 32767LL; break;
								case 4: maxCount = 2147483647LL; break;
								case 8: maxCount = 9223372036854775807LL; break;
#								ifdef NB_CONFIG_INCLUDE_ASSERTS
								default:
									NBASSERT(FALSE) 
									break;
#								endif
								}
								break;
							case ENNBStructMapMemberType_UInt:
								switch(mSz->size) {
								case 1: maxCount = 255ULL; break;
								case 2: maxCount = 65535ULL; break;
								case 4: maxCount = 4294967295ULL; break;
								case 8: maxCount = 18446744073709551615ULL; break;
#								ifdef NB_CONFIG_INCLUDE_ASSERTS
								default:
									NBASSERT(FALSE) 
									break;
#								endif
								}
								break;
							default:
								r = FALSE; NBASSERT(FALSE)
								break;
							}
							//Count childs
							const STNBJsonNode* arrNode = NBJson_childNode(json, m->name, pNode, NULL);
							if(arrNode != NULL){
								BYTE* arrData = NULL;
								UI64 arrSz = 0;
								if(m->data.type == ENNBStructMapMemberType_Bytes && arrNode->type == ENJsonNodeTypee_Pair){
									//Optimization, process as base64
									const char* base64 = NBJson_nodeStr(json, arrNode, NULL);
									if(base64 != NULL){
										const UI32 base64Sz = NBString_strLenBytes(base64);
										if((base64Sz % 4) != 0){
											r = FALSE; NBASSERT(FALSE)
										} else if(base64Sz > 0){
											arrSz		= (base64Sz * 3) / 4;
											arrData		= NBMemory_alloc(arrSz);
											UI64 i, i2 = 0; UI8 bytes; char buff[3];
											for(i = 0; i < base64Sz; i += 4){
												bytes = NBBase64_decode4Bytes(&base64[i], buff);
												if(bytes == 0){
													r = FALSE; NBASSERT(FALSE)
													break;
												} else {
													NBASSERT((i2 + bytes) <= arrSz)
													arrData[i2++] = buff[0];
													if(bytes > 1) arrData[i2++] = buff[1];
													if(bytes > 2) arrData[i2++] = buff[2];
												}
											}
											if(r){
												arrSz = i2;
											} else {
												NBMemory_free(arrData);
												arrData = NULL;
												arrSz = 0;
											}
										}
									}
								} else {
									//Count childs
									{
										const STNBJsonNode* chld = NBJson_childNode(json, m->name, arrNode, NULL);
										while(chld != NULL && arrSz < maxCount){
											arrSz++;
											chld = NBJson_childNode(json, m->name, arrNode, chld);
										}
									}
									//Create and load childs
									if(arrSz > 0){
										UI64 i = 0;
										arrData = NBMemory_alloc(m->data.size * arrSz);
										NBMemory_set(arrData, 0, m->data.size * arrSz);
										{
											const STNBJsonNode* chld = NBJson_childNode(json, m->name, arrNode, NULL);
											while(chld != NULL && i < arrSz && r){
												NBASSERT(i < maxCount)
												BYTE* dst = &arrData[m->data.size * i];
												r = NBStruct_stReadFromJsonNodeOneMemberValue_(json, chld, structMap, m, dst, m->data.size);
												//Next
												i++;
												chld = NBJson_childNode(json, m->name, arrNode, chld);
											}
										}
										if(!r){
											NBMemory_free(arrData);
											arrData = NULL;
											NBASSERT(FALSE)
										} else {
											NBASSERT(i == arrSz)
											arrSz = i;
										}
									}
								}
								//Set values
								*((void**)&dst[m->iPos]) = arrData;
								switch(mSz->data.type) {
								case ENNBStructMapMemberType_Int:
									switch(mSz->data.size) {
									case 1: *((SI8*)&dst[mSz->iPos]) = (char)arrSz; break;
									case 2: *((SI16*)&dst[mSz->iPos]) = (SI16)arrSz; break;
									case 4: *((SI32*)&dst[mSz->iPos]) = (SI32)arrSz; break;
									case 8: *((SI64*)&dst[mSz->iPos]) = (SI64)arrSz; break;
									default: r = FALSE; NBASSERT(FALSE) break;
									}
									break;
								case ENNBStructMapMemberType_UInt:
									switch(mSz->data.size) {
									case 1: *((UI8*)&dst[mSz->iPos]) = (BYTE)arrSz; break;
									case 2: *((UI16*)&dst[mSz->iPos]) = (UI16)arrSz; break;
									case 4: *((UI32*)&dst[mSz->iPos]) = (UI32)arrSz; break;
									case 8: *((UI64*)&dst[mSz->iPos]) = (UI64)arrSz; break;
									default: r = FALSE; NBASSERT(FALSE) break;
									}
									break;
								default:
									r = FALSE; NBASSERT(FALSE)
									break;
								}
							}
						}
					}
				} else {
					//------------------
					//- Embeded array
					//------------------
					NBASSERT(m->elem.vCount == NULL)
					if(m->elem.isPtr){
						const STNBJsonNode* arr = NBJson_childNode(json, m->name, pNode, NULL);
						if(arr == NULL){
							UI32 i = 0; for(i = 0; i < m->elem.count; i++){
								void** pDst = (void**)&dst[m->iPos + (sizeof(void*) * i)];
								*pDst = NULL;
							}
						} else {
							UI32 i = 0;
							const STNBJsonNode* chld = NBJson_childNode(json, m->name, arr, NULL);
							while(chld != NULL && i < m->elem.count && r){
								void** pDst = (void**)&dst[m->iPos + (sizeof(void*) * i)];
								if(NBJson_nodeIsNull(json, chld)){
									*pDst = NULL;
								} else {
									*pDst = NBMemory_alloc(m->data.size);
									NBMemory_set(*pDst, 0, m->data.size);
									r = NBStruct_stReadFromJsonNodeOneMemberValue_(json, chld, structMap, m, *pDst, m->data.size);
								}
								//Next
								i++;
								chld = NBJson_childNode(json, m->name, arr, chld);
							}
						}
					} else {
						const STNBJsonNode* arr = NBJson_childNode(json, m->name, pNode, NULL);
						if(arr != NULL){
							if(m->data.type == ENNBStructMapMemberType_Bytes && arr->type == ENJsonNodeTypee_Pair){
								//Optimization, process as base64
								const char* base64 = NBJson_nodeStr(json, arr, NULL);
								if(base64 != NULL){
									const UI32 base64Sz = NBString_strLenBytes(base64);
									if((base64Sz % 4) != 0){
										r = FALSE; NBASSERT(FALSE)
									} else if(base64Sz > 0){
										BYTE* dstBuff = &dst[m->iPos];
										const UI32 dstBuffSz = (m->data.size * m->elem.count);
										UI32 i, i2 = 0; UI8 bytes; char buff[3];
										for(i = 0; i < base64Sz; i += 4){
											bytes = NBBase64_decode4Bytes(&base64[i], buff);
											if(bytes == 0 || (i2 + bytes) > dstBuffSz){
												r = FALSE; NBASSERT(FALSE)
												break;
											} else {
												dstBuff[i2++] = buff[0];
												if(bytes > 1) dstBuff[i2++] = buff[1];
												if(bytes > 2) dstBuff[i2++] = buff[2];
											}
										}
									}
								}
							} else {
								//Read one-by-one member
								UI32 i = 0;
								const STNBJsonNode* chld = NBJson_childNode(json, m->name, arr, NULL);
								while(chld != NULL && i < m->elem.count && r){
									r = NBStruct_stReadFromJsonNodeOneMemberValue_(json, chld, structMap, m, &dst[m->iPos + (m->data.size * i)], m->data.size);
									//Next
									i++;
									chld = NBJson_childNode(json, m->name, arr, chld);
								}
							}
						}
					}
				}
			}
		}
		//Release loaded members (if error found)
		if(!r && i > 0){
			NBStruct_stReleaseMembers_(structMap, i, pDst, dstSz);
		}
	}
	return r;
}

//Load from string

BOOL NBStruct_stReadFromJsonStr(const char* str, const UI32 strSz, const STNBStructMap* structMap, void* dst, const UI32 dstSz){
	BOOL r = FALSE;
	if(structMap->stSize != dstSz){
		NBASSERT(FALSE)
	} else {
		STNBJson json;
		NBJson_init(&json);
		NBJson_prepareStorageValues(&json, strSz);
		if(NBJson_loadFromStrBytes(&json, str, strSz)){
			r = NBStruct_stReadFromJsonNode(&json, NBJson_rootMember(&json), structMap, dst, dstSz);
		}
		NBJson_release(&json);
	}
	return r;
}

BOOL NBStruct_stReadFromJsonBase64Str(const char* str, const UI32 strSz, const STNBStructMap* structMap, void* dst, const UI32 dstSz){
	BOOL r = FALSE;
	if(structMap->stSize != dstSz){
		NBASSERT(FALSE)
	} else {
		STNBString plainStr;
		NBString_initWithSz(&plainStr, (strSz * 3 / 4), 256, 1.5f);
		if(!NBBase64_decodeBytes(&plainStr, str, strSz)){
			NBASSERT(FALSE)
		} else {
			STNBJson json;
			NBJson_init(&json);
			NBJson_prepareStorageValues(&json, plainStr.length);
			if(NBJson_loadFromStrBytes(&json, plainStr.str, plainStr.length)){
				r = NBStruct_stReadFromJsonNode(&json, NBJson_rootMember(&json), structMap, dst, dstSz);
			}
			NBJson_release(&json);
		}
		NBString_release(&plainStr);
	}
	return r;
}

//Concat as string

BOOL NBStruct_stConcatAsJsonMembers_(STNBString* dst, const STNBStructMap* structMap, const void* pSrc, const UI32 srcSz, const STNBStructConcatFormat* format, STNBString* dstBase64, STNBFileRef flushToFile);
BOOL NBStruct_stOneMemberValueIsZero_(const STNBStructMap* structMap, const STNBStructMapMember* mm, const void* mSrc, const UI32 mSz);
BOOL NBStruct_stAnyMemberValueIsNonZero_(const STNBStructMap* structMap, const void* pSrc, const UI32 srcSz);

BOOL NBStruct_stOneMemberValueIsZero_(const STNBStructMap* structMap, const STNBStructMapMember* mm, const void* mSrc, const UI32 mSz){
	BOOL r = FALSE;
	switch(mm->data.type) {
		case ENNBStructMapMemberType_Bool:
			switch(mSz) {
				case 1: r = (*((SI8*)mSrc) == 0); break;
				case 2: r = (*((SI16*)mSrc) == 0); break;
				case 4: r = (*((SI32*)mSrc) == 0); break;
				case 8: r = (*((SI64*)mSrc) == 0); break;
				default: r = FALSE; NBASSERT(FALSE) break;
			}
			break;
		case ENNBStructMapMemberType_Int:
			switch(mSz) {
				case 1: r = (*((SI8*)mSrc) == 0); break;
				case 2: r = (*((SI16*)mSrc) == 0); break;
				case 4: r = (*((SI32*)mSrc) == 0); break;
				case 8: r = (*((SI64*)mSrc) == 0); break;
				default: r = FALSE; NBASSERT(FALSE) break;
			}
			break;
		case ENNBStructMapMemberType_UInt:
			switch(mSz) {
				case 1: r = (*((UI8*)mSrc) == 0); break;
				case 2: r = (*((UI16*)mSrc) == 0); break;
				case 4: r = (*((UI32*)mSrc) == 0); break;
				case 8: r = (*((UI64*)mSrc) == 0); break;
				default: r = FALSE; NBASSERT(FALSE) break;
			}
			break;
		case ENNBStructMapMemberType_Float:
			NBASSERT(mSz == sizeof(float))
			r = (*((float*)mSrc) == 0);
			break;
		case ENNBStructMapMemberType_Double:
			NBASSERT(mSz == sizeof(double))
			r = (*((double*)mSrc) == 0);
			break;
		case ENNBStructMapMemberType_Chars:
			{
				const char* b = (const char*)mSrc;
				const char* bAfterEnd = b + mSz;
				r = TRUE;
				while(b < bAfterEnd){
					if(*(b++) != 0){
						r = FALSE;
						break;
					}
				}
			}
			break;
		case ENNBStructMapMemberType_Bytes:
			{
				const BYTE* b = (const BYTE*)mSrc;
				const BYTE* bAfterEnd = b + mSz;
				r = TRUE;
				while(b < bAfterEnd){
					if(*(b++) != 0){
						r = FALSE;
						break;
					}
				}
			}
			break;
		case ENNBStructMapMemberType_StrPtr:
			{
				NBASSERT(mSz == sizeof(char*))
				r = (*((const char**)mSrc) == NULL);
			}
			break;
		case ENNBStructMapMemberType_Enum:
			r = FALSE; //all enums are translated to text
			break;
		case ENNBStructMapMemberType_Struct:
			if(mm->data.stMap == NULL){
				NBASSERT(FALSE)
				r = FALSE;
			} else {
				NBASSERT(mSz == mm->data.stMap->stSize)
				r = !NBStruct_stAnyMemberValueIsNonZero_(mm->data.stMap, mSrc, mm->data.stMap->stSize);
			}
			break;
		default:
			NBASSERT(FALSE)
			break;
	}
	return r;
}

BOOL NBStruct_stAnyMemberValueIsNonZero_(const STNBStructMap* structMap, const void* pSrc, const UI32 srcSz){
	BOOL r = FALSE;
	if(structMap->stSize != srcSz){
		NBASSERT(FALSE)
	} else {
		const BYTE* src = (const BYTE*)pSrc;
		NBASSERT(structMap->stSize == srcSz)
		UI32 i; for(i = 0; i < structMap->mbrsSz && !r; i++){
			STNBStructMapMember* m = &structMap->mbrs[i];
			NBASSERT(m->size > 0 && m->size == (m->elem.count * (m->elem.isPtr ? sizeof(void*) : m->data.size)))
			if(m->elem.count == 1){
				//------------------
				//- Only one element
				//------------------
				if(!m->elem.isPtr){
					if(!NBStruct_stOneMemberValueIsZero_(structMap, m, &src[m->iPos], m->data.size)){
						r = TRUE;
						break;
					}
				} else {
					//One embeded pointer
					if(m->elem.vCount == NULL){
						//Pointer to one element
						if(*((void**)&src[m->iPos]) != NULL){
							r = TRUE;
							break;
						}
					} else {
						//Pointer to an array
						if(*((void**)&src[m->iPos]) != NULL){
							r = TRUE;
							break;
						}
					}
				}
			} else {
				//------------------
				//- Embeded array
				//------------------
				NBASSERT(m->elem.vCount == NULL)
				if(m->elem.isPtr){
					UI64 i; for(i = 0 ; i < m->elem.count && r; i++){
						const void** pSrc = (const void**)&src[m->iPos + (sizeof(void*) * i)];
						if(*pSrc != NULL){
							r = TRUE;
							break;
						}
					}
				} else {
					if(m->data.type == ENNBStructMapMemberType_Bytes){
						const BYTE* eSrc = &src[m->iPos];
						const BYTE* b = eSrc; 
						const BYTE* bAfterEnd = eSrc + (m->data.size * m->elem.count); 
						while(b < bAfterEnd){
							if(*(b++) != 0){
								r = TRUE;
								break;
							}
						}
					} else {
						//Concat as array
						UI64 i; for(i = 0 ; i < m->elem.count && r; i++){
							const BYTE* eSrc = &src[m->iPos + (m->data.size * i)];
							if(!NBStruct_stOneMemberValueIsZero_(structMap, m, eSrc, m->data.size)){
								r = TRUE;
								break;
							}
						}
					}
				}
			}
		}
	}
	return r;
}

BOOL NBStruct_stConcatAsJsonOneMemberValue_(STNBString* dst, const STNBStructMap* structMap, const STNBStructMapMember* mm, const void* mSrc, const UI32 mSz, const STNBStructConcatFormat* format, STNBString* dstBase64, STNBFileRef flushToFile){
	BOOL r = TRUE;
	//New line
	if(format->valuesInNewLine){
		NBString_concatByte(dst, '\n');
	}
	switch(mm->data.type) {
		case ENNBStructMapMemberType_Bool:
			switch(mSz) {
				case 1: NBString_concat(dst, *((SI8*)mSrc) ? "true" : "false"); break;
				case 2: NBString_concat(dst, *((SI16*)mSrc) ? "true" : "false"); break;
				case 4: NBString_concat(dst, *((SI32*)mSrc) ? "true" : "false"); break;
				case 8: NBString_concat(dst, *((SI64*)mSrc) ? "true" : "false"); break;
				default: r = FALSE; NBASSERT(FALSE) break;
			}
			break;
		case ENNBStructMapMemberType_Int:
			switch(mSz) {
				case 1: NBString_concatSI32(dst, *((SI8*)mSrc)); break;
				case 2: NBString_concatSI32(dst, *((SI16*)mSrc));  break;
				case 4: NBString_concatSI32(dst, *((SI32*)mSrc)); break;
				case 8: NBString_concatSI64(dst, *((SI64*)mSrc)); break;
				default: r = FALSE; NBASSERT(FALSE) break;
			}
			break;
		case ENNBStructMapMemberType_UInt:
			switch(mSz) {
				case 1: NBString_concatUI32(dst, *((UI8*)mSrc)); break;
				case 2: NBString_concatUI32(dst, *((UI16*)mSrc)); break;
				case 4: NBString_concatUI32(dst, *((UI32*)mSrc)); break;
				case 8: NBString_concatUI64(dst, *((UI64*)mSrc)); break;
				default: r = FALSE; NBASSERT(FALSE) break;
			}
			break;
		case ENNBStructMapMemberType_Float:
			NBASSERT(mSz == sizeof(float))
			NBString_concatFloat(dst, *((float*)mSrc), -10);
			break;
		case ENNBStructMapMemberType_Double:
			NBASSERT(mSz == sizeof(double))
			NBString_concatDouble(dst, *((double*)mSrc), -10);
			break;
		case ENNBStructMapMemberType_Chars:
			NBString_concatByte(dst, '\"');
			if(format->escapeOnlyDQuotes){
				NBJson_concatScapedBytesDQuotesOnly(dst, (const char*)mSrc, mSz);
			} else {
				NBJson_concatScapedBytes(dst, (const char*)mSrc, mSz);
			}
			NBString_concatByte(dst, '\"');
			break;
		case ENNBStructMapMemberType_Bytes:
			NBString_concatByte(dst, '\"'); NBBase64_codeBytes(dst, (const char*)mSrc, mSz); NBString_concatByte(dst, '\"');
			break;
		case ENNBStructMapMemberType_StrPtr:
			{
				const char* value = *((const char**)mSrc);
				if(value == NULL){
					NBString_concat(dst, "null");
				} else {
					NBString_concatByte(dst, '\"');
					if(format->escapeOnlyDQuotes){
						NBJson_concatScapedDQuotesOnly(dst, value);
					} else {
						NBJson_concatScaped(dst, value);
					}
					NBString_concatByte(dst, '\"');
				}
			}
			break;
		case ENNBStructMapMemberType_Enum:
			if(mm->data.enMap == NULL){
				NBASSERT(FALSE)
				r = FALSE;
			} else {
				SI64 intValue = 0;
				switch(mSz) {
					case 1: intValue = *((SI8*)mSrc); break;
					case 2: intValue = *((SI16*)mSrc); break;
					case 4: intValue = *((SI32*)mSrc); break;
					case 8: intValue = *((SI64*)mSrc); break;
#					ifdef NB_CONFIG_INCLUDE_ASSERTS
					default:
						NBASSERT(FALSE) 
						break;
#					endif
				}
				UI32 i; for(i = 0; i < mm->data.enMap->recordsSz; i++){
					const STNBEnumMapRecord* r = &mm->data.enMap->records[i];
					if(r->intValue == intValue){
						NBString_concatByte(dst, '\"');
						if(format->escapeOnlyDQuotes){
							NBJson_concatScapedDQuotesOnly(dst, r->strValue);
						} else {
							NBJson_concatScaped(dst, r->strValue);
						}
						NBString_concatByte(dst, '\"');
						break;
					}
				}
				if(i == mm->data.enMap->recordsSz){
					PRINTF_ERROR("Value %lld was not found at enum '%s'.\n", intValue, mm->data.enMap->enumName);
					r = FALSE; NBASSERT(FALSE)
				}
			}
			break;
		case ENNBStructMapMemberType_Struct:
			if(mm->data.stMap == NULL){
				NBASSERT(FALSE)
				r = FALSE;
			} else {
				NBASSERT(mSz == mm->data.stMap->stSize)
				//New line
				if(!format->valuesInNewLine && format->objectsInNewLine){
					NBString_concatByte(dst, '\n');
				}
				NBString_concatByte(dst, '{');
				NBStruct_stConcatAsJsonMembers_(dst, mm->data.stMap, mSrc, mm->data.stMap->stSize, format, dstBase64, flushToFile);
				NBString_concatByte(dst, '}');
			}
			break;
		default:
			NBASSERT(FALSE)
			break;
	}
	return r;
}

BOOL NBStruct_stConcatAsJsonMembers_(STNBString* dst, const STNBStructMap* structMap, const void* pSrc, const UI32 srcSz, const STNBStructConcatFormat* format, STNBString* dstBase64, STNBFileRef flushToFile){
	BOOL r = FALSE;
	if(structMap->stSize != srcSz){
		NBASSERT(FALSE)
	} else {
		r = TRUE;
		const BYTE* src = (const BYTE*)pSrc;
		NBASSERT(structMap->stSize == srcSz)
		UI32 i, printedCount = 0;
		for(i = 0; i < structMap->mbrsSz && r; i++){
			STNBStructMapMember* m = &structMap->mbrs[i];
			NBASSERT(m->size > 0 && m->size == (m->elem.count * (m->elem.isPtr ? sizeof(void*) : m->data.size)))
			if(m->elem.count == 1){
				//------------------
				//- Only one element
				//------------------
				if(!m->elem.isPtr){
					if(!format->ignoreZeroValues || !NBStruct_stOneMemberValueIsZero_(structMap, m, &src[m->iPos], m->data.size)){
						//New line
						if(format->namesInNewLine){
							NBString_concatByte(dst, '\n');
						}
						//One embeded element
						if(printedCount != 0) NBString_concatByte(dst, ','); printedCount++;
						//name
						NBString_concatByte(dst, '\"'); NBJson_concatScaped(dst, m->name); NBString_concatByte(dst, '\"'); NBString_concatByte(dst, ':');
						//value
						{
							IF_NBASSERT(const UI32 szBefore = dst->length;)
							r = NBStruct_stConcatAsJsonOneMemberValue_(dst, structMap, m, &src[m->iPos], m->data.size, format, dstBase64, flushToFile);
							NBASSERT(szBefore < dst->length || dst->length == 1) //dst->length == 1 if was flushed by struct member (only '}' remains)
						}
					}
				} else {
					//One embeded pointer
					if(m->elem.vCount == NULL){
						//Pointer to one element
						void* s = *((void**)&src[m->iPos]);
						if(!format->ignoreZeroValues || s != NULL){
							//New line
							if(format->namesInNewLine){
								NBString_concatByte(dst, '\n');
							}
							if(printedCount != 0) NBString_concatByte(dst, ','); printedCount++;
							//name
							NBString_concatByte(dst, '\"'); NBJson_concatScaped(dst, m->name); NBString_concatByte(dst, '\"'); NBString_concatByte(dst, ':');
							//value
							if(s == NULL){
								NBString_concat(dst, "null");
							} else {
								IF_NBASSERT(const UI32 szBefore = dst->length;)
								r = NBStruct_stConcatAsJsonOneMemberValue_(dst, structMap, m, s, m->data.size, format, dstBase64, flushToFile);
								NBASSERT(szBefore < dst->length || dst->length == 1) //dst->length == 1 if was flushed by struct member (only '}' remains)
							}
						}
					} else {
						//Pointer to an array
						void* v = *((void**)&src[m->iPos]);
						if(!format->ignoreZeroValues || v != NULL){
							STNBStructMapMember* mSz = m->elem.vCount;
							//New line
							if(format->namesInNewLine){
								NBString_concatByte(dst, '\n');
							}
							if(printedCount != 0) NBString_concatByte(dst, ','); printedCount++;
							//name
							NBString_concatByte(dst, '\"'); NBJson_concatScaped(dst, m->name); NBString_concatByte(dst, '\"'); NBString_concatByte(dst, ':');
							//value
							if(v == NULL){
								NBString_concat(dst, "null");
							} else {
								UI64 arrSz = 0;
								switch(mSz->data.type) {
									case ENNBStructMapMemberType_Int:
										switch(mSz->data.size) {
											case 1: { const SI8 c = *((SI8*)&src[mSz->iPos]); if(c > 0) arrSz = c; } break;
											case 2: { const SI16 c = *((SI16*)&src[mSz->iPos]); if(c > 0) arrSz = c; } break;
											case 4: { const SI32 c = *((SI32*)&src[mSz->iPos]); if(c > 0) arrSz = c; } break;
											case 8: { const SI64 c = *((SI64*)&src[mSz->iPos]); if(c > 0) arrSz = c; } break;
#											ifdef NB_CONFIG_INCLUDE_ASSERTS
											default:
												NBASSERT(FALSE) 
												break;
#											endif
										}
										break;
									case ENNBStructMapMemberType_UInt:
										switch(mSz->data.size) {
											case 1: arrSz = *((UI8*)&src[mSz->iPos]); break;
											case 2: arrSz = *((UI16*)&src[mSz->iPos]); break;
											case 4: arrSz = *((UI32*)&src[mSz->iPos]); break;
											case 8: arrSz = *((UI64*)&src[mSz->iPos]); break;
#											ifdef NB_CONFIG_INCLUDE_ASSERTS
											default:
												NBASSERT(FALSE) 
												break;
#											endif
										}
										break;
									default:
										NBASSERT(FALSE)
										break;
								}
								if(m->data.type == ENNBStructMapMemberType_Bytes){
									//Optimization, concat as one base64 string
									BYTE* src = &((BYTE*)v)[0];
									NBString_concatByte(dst, '\"');
									NBBase64_codeBytes(dst, (const char*)src, (UI32)(m->data.size * arrSz));
									NBString_concatByte(dst, '\"');
								} else {
									//Concat as array
									NBString_concatByte(dst, '[');
									UI64 i; for(i = 0 ; i < arrSz && r; i++){
										BYTE* src = &((BYTE*)v)[m->data.size * i];
										if(i != 0) NBString_concatByte(dst, ',');
										IF_NBASSERT(const UI32 szBefore = dst->length;)
										r = NBStruct_stConcatAsJsonOneMemberValue_(dst, structMap, m, src, m->data.size, format, dstBase64, flushToFile);
										NBASSERT(szBefore < dst->length || dst->length == 1) //dst->length == 1 if was flushed by struct member (only '}' remains)
									}
									NBString_concatByte(dst, ']');
								}
							}
						}
					}
				}
			} else {
				//------------------
				//- Embeded array
				//------------------
				NBASSERT(m->elem.vCount == NULL)
				BOOL ignore = FALSE;
				//detect non-zero values
				if(format->ignoreZeroValues){
					ignore = TRUE;
					if(m->elem.isPtr){
						UI64 i; for(i = 0 ; i < m->elem.count; i++){
							const void** pSrc = (const void**)&src[m->iPos + (sizeof(void*) * i)];
							if(*pSrc != NULL){
								ignore = FALSE;
								break;
							}
						}
					} else {
						if(m->data.type == ENNBStructMapMemberType_Bytes){
							//Optimization, concat as one base64 string
							const BYTE* eSrc = &src[m->iPos];
							const BYTE* b = eSrc; 
							const BYTE* bAfterEnd = b + (m->data.size * m->elem.count);
							while(b < bAfterEnd){
								if(*(b++) != 0){
									ignore = FALSE;
									break;
								}
							}
						} else {
							//Concat as array
							UI64 i; for(i = 0 ; i < m->elem.count; i++){
								const BYTE* eSrc = &src[m->iPos + (m->data.size * i)];
								if(!NBStruct_stOneMemberValueIsZero_(structMap, m, eSrc, m->data.size)){
									ignore = FALSE;
									break;
								}
							}
						}
					}
				}
				//add member
				if(!ignore){
					//New line
					if(format->namesInNewLine){
						NBString_concatByte(dst, '\n');
					}
					if(printedCount != 0) NBString_concatByte(dst, ','); printedCount++;
					//name
					NBString_concatByte(dst, '\"'); NBJson_concatScaped(dst, m->name); NBString_concatByte(dst, '\"'); NBString_concatByte(dst, ':');
					//value
					if(m->elem.isPtr){
						NBString_concatByte(dst, '[');
						UI64 i; for(i = 0 ; i < m->elem.count && r; i++){
							const void** pSrc = (const void**)&src[m->iPos + (sizeof(void*) * i)];
							if(i != 0) NBString_concatByte(dst, ',');
							if(*pSrc == NULL){
								NBString_concat(dst, "null");
							} else {
								IF_NBASSERT(const UI32 szBefore = dst->length;)
								r = NBStruct_stConcatAsJsonOneMemberValue_(dst, structMap, m, *pSrc, m->data.size, format, dstBase64, flushToFile);
								NBASSERT(szBefore < dst->length || dst->length == 1) //dst->length == 1 if was flushed by struct member (only '}' remains)
							}
						}
						NBString_concatByte(dst, ']');
					} else {
						if(m->data.type == ENNBStructMapMemberType_Bytes){
							//Optimization, concat as one base64 string
							const BYTE* eSrc = &src[m->iPos];
							NBString_concatByte(dst, '\"');
							NBBase64_codeBytes(dst, (const char*)eSrc, (UI32)(m->data.size * m->elem.count));
							NBString_concatByte(dst, '\"');
						} else {
							//Concat as array
							NBString_concatByte(dst, '[');
							UI64 i; for(i = 0 ; i < m->elem.count && r; i++){
								const BYTE* eSrc = &src[m->iPos + (m->data.size * i)];
								if(i != 0) NBString_concatByte(dst, ',');
								IF_NBASSERT(const UI32 szBefore = dst->length;)
								r = NBStruct_stConcatAsJsonOneMemberValue_(dst, structMap, m, eSrc, m->data.size, format, dstBase64, flushToFile);
								NBASSERT(szBefore < dst->length || dst->length == 1) //dst->length == 1 if was flushed by struct member (only '}' remains)
							}
							NBString_concatByte(dst, ']');
						}
					}
				}
			}
		}
		//Convert to base64
		if(r){
			if(dstBase64 == NULL){
				//Flush to file
				if(NBFile_isSet(flushToFile) && dst->length > 0){
					if(NBFile_write(flushToFile, dst->str, dst->length) != dst->length){
						r = FALSE; NBASSERT(false)
					} else {
						NBString_empty(dst);
					}
				}
			} else {
				//Convert to base64
				const UI32 blocks = (dst->length / 3);
				if(blocks > 0){
					char dst4[4];
					UI32 i; for(i = 0; i < blocks; i++){
						const char* src3 = &dst->str[i * 3]; NBASSERT((i * 3) < dst->length)
						NBBase64_code3Bytes(src3, 3, dst4);
						NBString_concatBytes(dstBase64, dst4, 4);
					}
					NBString_removeFirstBytes(dst, blocks * 3);
					//Flush to file
					if(NBFile_isSet(flushToFile) && dstBase64->length > 0){
						if(NBFile_write(flushToFile, dstBase64->str, dstBase64->length) != dstBase64->length){
							r = FALSE; NBASSERT(false)
						} else {
							NBString_empty(dstBase64);
						}
					}
				}
			}
		}
	}
	return r;
}
	
BOOL NBStruct_stConcatAsJson(STNBString* dst, const STNBStructMap* structMap, const void* src, const UI32 srcSz){
	BOOL r = FALSE;
	if(structMap->stSize != srcSz){
		NBASSERT(FALSE)
	} else {
		STNBStructConcatFormat format;
		NBMemory_setZeroSt(format, STNBStructConcatFormat);
		NBString_concatByte(dst, '{');
		r = NBStruct_stConcatAsJsonMembers_(dst, structMap, src, srcSz, &format, NULL, NB_OBJREF_NULL);
		NBString_concatByte(dst, '}');
	}
	return r;
}

BOOL NBStruct_stConcatAsJsonWithFormat(STNBString* dst, const STNBStructMap* structMap, const void* src, const UI32 srcSz, const STNBStructConcatFormat* format){
	BOOL r = FALSE;
	if(structMap->stSize != srcSz){
		NBASSERT(FALSE)
	} else {
		NBString_concatByte(dst, '{');
		r = NBStruct_stConcatAsJsonMembers_(dst, structMap, src, srcSz, format, NULL, NB_OBJREF_NULL);
		NBString_concatByte(dst, '}');
	}
	return r;
}

BOOL NBStruct_stConcatAsJsonBase64(STNBString* dst, const STNBStructMap* structMap, const void* src, const UI32 srcSz){
	BOOL r = FALSE;
	if(structMap->stSize != srcSz){
		NBASSERT(FALSE)
	} else {
		STNBStructConcatFormat format;
		NBMemory_setZeroSt(format, STNBStructConcatFormat);
		{
			STNBString strTmp;
			NBString_init(&strTmp);
			NBString_concatByte(&strTmp, '{');
			r = NBStruct_stConcatAsJsonMembers_(&strTmp, structMap, src, srcSz, &format, dst, NB_OBJREF_NULL);
			NBString_concatByte(&strTmp, '}');
			NBBase64_codeBytes(dst, strTmp.str, strTmp.length);
			NBString_release(&strTmp);
		}
	}
	return r;
}

BOOL NBStruct_stConcatAsJsonBase64WithFormat(STNBString* dst, const STNBStructMap* structMap, const void* src, const UI32 srcSz, const STNBStructConcatFormat* format){
	BOOL r = FALSE;
	if(structMap->stSize != srcSz){
		NBASSERT(FALSE)
	} else {
		STNBString strTmp;
		NBString_init(&strTmp);
		NBString_concatByte(&strTmp, '{');
		r = NBStruct_stConcatAsJsonMembers_(&strTmp, structMap, src, srcSz, format, dst, NB_OBJREF_NULL);
		NBString_concatByte(&strTmp, '}');
		NBBase64_codeBytes(dst, strTmp.str, strTmp.length);
		NBString_release(&strTmp);
	}
	return r;
}

//Write

BOOL NBStruct_stWriteToJsonFilepath(const char* filepath, const STNBStructMap* structMap, const void* src, const UI32 srcSz){
	BOOL r = FALSE;
	STNBFileRef f = NBFile_alloc(NULL);
	if(NBFile_open(f, filepath, ENNBFileMode_Write)){
		NBFile_lock(f);
		r = NBStruct_stWriteToJsonFile(f, structMap, src, srcSz);
		NBFile_unlock(f);
	}
	NBFile_release(&f);
	return r;
}

BOOL NBStruct_stWriteToJsonFilepathWithFormat(const char* filepath, const STNBStructMap* structMap, const void* src, const UI32 srcSz, const STNBStructConcatFormat* format){
	BOOL r = FALSE;
	STNBFileRef f = NBFile_alloc(NULL);
	if(NBFile_open(f, filepath, ENNBFileMode_Write)){
		NBFile_lock(f);
		r = NBStruct_stWriteToJsonFileWithFormat(f, structMap, src, srcSz, format);
		NBFile_unlock(f);
	}
	NBFile_release(&f);
	return r;
}

BOOL NBStruct_stWriteToJsonFilepathAtRoot(STNBFilesystem* fs, const ENNBFilesystemRoot root, const char* filepath, const STNBStructMap* structMap, const void* src, const UI32 srcSz){
	BOOL r = FALSE;
	STNBFileRef f = NBFile_alloc(NULL);
	if(NBFilesystem_openAtRoot(fs, root, filepath, ENNBFileMode_Write, f)){
		NBFile_lock(f);
		r = NBStruct_stWriteToJsonFile(f, structMap, src, srcSz);
		NBFile_unlock(f);
	}
	NBFile_release(&f);
	return r;
}

BOOL NBStruct_stWriteToJsonFilepathAtRootWithFormat(STNBFilesystem* fs, const ENNBFilesystemRoot root, const char* filepath, const STNBStructMap* structMap, const void* src, const UI32 srcSz, const STNBStructConcatFormat* format){
	BOOL r = FALSE;
	STNBFileRef f = NBFile_alloc(NULL);
	if(NBFilesystem_openAtRoot(fs, root, filepath, ENNBFileMode_Write, f)){
		NBFile_lock(f);
		r = NBStruct_stWriteToJsonFileWithFormat(f, structMap, src, srcSz, format);
		NBFile_unlock(f);
	}
	NBFile_release(&f);
	return r;
}

BOOL NBStruct_stWriteToJsonFilepathStrsAndNullAtRoot(STNBFilesystem* fs, const ENNBFilesystemRoot root, const char** arrAndNull, const STNBStructMap* structMap, const void* src, const UI32 srcSz){
	BOOL r = FALSE;
	STNBString filepath;
	NBString_initWithStrsAndNull(&filepath, arrAndNull);
	{
		STNBFileRef f = NBFile_alloc(NULL);
		if(NBFilesystem_openAtRoot(fs, root, filepath.str, ENNBFileMode_Write, f)){
			NBFile_lock(f);
			r = NBStruct_stWriteToJsonFile(f, structMap, src, srcSz);
			NBFile_unlock(f);
		}
		NBFile_release(&f);
	}
	NBString_release(&filepath);
	return r;
}

BOOL NBStruct_stWriteToJsonFilepathStrsAndNullAtRootWithFormat(STNBFilesystem* fs, const ENNBFilesystemRoot root, const char** arrAndNull, const STNBStructMap* structMap, const void* src, const UI32 srcSz, const STNBStructConcatFormat* format){
	BOOL r = FALSE;
	STNBString filepath;
	NBString_initWithStrsAndNull(&filepath, arrAndNull);
	{
		STNBFileRef f = NBFile_alloc(NULL);
		if(NBFilesystem_openAtRoot(fs, root, filepath.str, ENNBFileMode_Write, f)){
			NBFile_lock(f);
			r = NBStruct_stWriteToJsonFileWithFormat(f, structMap, src, srcSz, format);
			NBFile_unlock(f);
		}
		NBFile_release(&f);
	}
	NBString_release(&filepath);
	return r;
}

BOOL NBStruct_stWriteToJsonFile(STNBFileRef file, const STNBStructMap* structMap, const void* src, const UI32 srcSz){
	BOOL r = FALSE;
	if(structMap->stSize != srcSz){
		NBASSERT(FALSE)
	} else {
		STNBStructConcatFormat format;
		NBMemory_setZeroSt(format, STNBStructConcatFormat);
		{
			STNBString str;
			NBString_initWithSz(&str, 4096, 4096, 0.10f);
			NBString_concatByte(&str, '{');
			if(NBStruct_stConcatAsJsonMembers_(&str, structMap, src, srcSz, &format, NULL, file)){
				NBString_concatByte(&str, '}');
				if(NBFile_write(file, str.str, str.length) == str.length){
					r = TRUE;
				}
			}
			NBString_release(&str);
		}
	}
	return r;
}

BOOL NBStruct_stWriteToJsonFileWithFormat(STNBFileRef file, const STNBStructMap* structMap, const void* src, const UI32 srcSz, const STNBStructConcatFormat* format){
	BOOL r = FALSE;
	if(structMap->stSize != srcSz){
		NBASSERT(FALSE)
	} else {
		STNBString str;
		NBString_initWithSz(&str, 4096, 4096, 0.10f);
		NBString_concatByte(&str, '{');
		if(NBStruct_stConcatAsJsonMembers_(&str, structMap, src, srcSz, format, NULL, file)){
			NBString_concatByte(&str, '}');
			if(NBFile_write(file, str.str, str.length) == str.length){
				r = TRUE;
			}
		}
		NBString_release(&str);
	}
	return r;
}

//Verify

BOOL NBStruct_stCalculateCrcMembers_(const STNBStructMap* structMap, const void* src, const UI32 srcSz, STNBCrc32* job);

BOOL NBStruct_stCalculateCrcOneMemberValue_(const STNBStructMap* structMap, const STNBStructMapMember* m, const void* mSrc, const UI32 srcSz, STNBCrc32* job){
	BOOL r = TRUE;
	switch(m->data.type) {
		case ENNBStructMapMemberType_Bool:
		case ENNBStructMapMemberType_Int:
		case ENNBStructMapMemberType_UInt:
		case ENNBStructMapMemberType_Enum:
			NBASSERT(srcSz == 1 || srcSz == 2 || srcSz == 4 || srcSz == 8)
			NBCrc32_feed(job, mSrc, srcSz);
			break;
		case ENNBStructMapMemberType_Float:
			NBASSERT(srcSz == sizeof(float))
			NBCrc32_feed(job, mSrc, srcSz);
			break;
		case ENNBStructMapMemberType_Double:
			NBASSERT(srcSz == sizeof(double))
			NBCrc32_feed(job, mSrc, srcSz);
			break;
		case ENNBStructMapMemberType_Chars:
			NBCrc32_feed(job, mSrc, srcSz);
			break;
		case ENNBStructMapMemberType_Bytes:
			NBCrc32_feed(job, mSrc, srcSz);
			break;
		case ENNBStructMapMemberType_StrPtr:
			{
				NBASSERT(srcSz == sizeof(void*))
				char* value = *((char**)mSrc);
				//Ptr-jump
				{
					const UI32 jmpRef = 0;
					NBCrc32_feed(job, &jmpRef, sizeof(jmpRef));
				}
				//Value
				if(value != NULL){
					const UI32 sz	= NBString_strLenBytes(value);
					NBCrc32_feed(job, value, sz + 1);
				}
			}
			break;
		case ENNBStructMapMemberType_Struct:
			if(m->data.stMap == NULL){
				NBASSERT(FALSE)
				r = FALSE;
			} else {
				NBASSERT(srcSz == m->data.stMap->stSize)
				NBStruct_stCalculateCrcMembers_(m->data.stMap, mSrc, srcSz, job);
			}
			break;
		default:
			NBASSERT(FALSE)
			r = FALSE;
			break;
	}
	return r;
}

BOOL NBStruct_stCalculateCrcOneMemberValueArrNatives_(const STNBStructMap* structMap, const STNBStructMapMember* m, const void* mSrc, const UI32 srcSz, const UI64 arrSz, STNBCrc32* job){
	BOOL r = TRUE;
	switch(m->data.type) {
		case ENNBStructMapMemberType_Bool:
		case ENNBStructMapMemberType_Int:
		case ENNBStructMapMemberType_UInt:
		case ENNBStructMapMemberType_Enum:
			NBASSERT(srcSz == 1 || srcSz == 2 || srcSz == 4 || srcSz == 8)
			NBCrc32_feed(job, mSrc, (UI32)(srcSz * arrSz));
			break;
		case ENNBStructMapMemberType_Float:
			NBASSERT(srcSz == sizeof(float))
			NBCrc32_feed(job, mSrc, (UI32)(srcSz * arrSz));
			break;
		case ENNBStructMapMemberType_Double:
			NBASSERT(srcSz == sizeof(double))
			NBCrc32_feed(job, mSrc, (UI32)(srcSz * arrSz));
			break;
		case ENNBStructMapMemberType_Chars:
			NBCrc32_feed(job, mSrc, (UI32)(srcSz * arrSz));
			break;
		case ENNBStructMapMemberType_Bytes:
			NBCrc32_feed(job, mSrc, (UI32)(srcSz * arrSz));
			break;
		default:
			NBASSERT(FALSE)
			r = FALSE;
			break;
	}
	return r;
}

BOOL NBStruct_stCalculateCrcMembers_(const STNBStructMap* structMap, const void* pSrc, const UI32 srcSz, STNBCrc32* job){
	BOOL r = FALSE;
	if(structMap->stSize != srcSz){
		NBASSERT(FALSE)
	} else {
		r = TRUE;
		const BYTE* src = (const BYTE*)pSrc;
		UI32 i; for(i = 0; i < structMap->mbrsSz && r; i++){
			STNBStructMapMember* m = &structMap->mbrs[i];
			NBASSERT(m->size > 0 && m->size == (m->elem.count * (m->elem.isPtr ? sizeof(void*) : m->data.size)))
			if(m->elem.count == 1){
				//------------------
				//- Only one element
				//------------------
				if(!m->elem.isPtr){
					//One embeded element
					r = NBStruct_stCalculateCrcOneMemberValue_(structMap, m, &src[m->iPos], m->data.size, job);
				} else {
					//One emeded pointer
					if(m->elem.vCount == NULL){
						//Pointer to one element
						void* s = *((void**)&src[m->iPos]);
						//Ptr-jump
						{
							const UI32 jmpRef = 0;
							NBCrc32_feed(job, &jmpRef, sizeof(jmpRef));
						}
						//Value
						if(s != NULL){
							r = NBStruct_stCalculateCrcOneMemberValue_(structMap, m, s, m->data.size, job);
						}
					} else {
						//Pointer to an array
						STNBStructMapMember* mSz = m->elem.vCount;
						BYTE* arrData	= *((BYTE**)&src[m->iPos]);
						UI64 arrSz = 0;
						{
							switch(mSz->data.type) {
								case ENNBStructMapMemberType_Int:
									switch(mSz->size) {
										case 1: { const SI8 c = *((SI8*)&src[mSz->iPos]); if(c > 0) arrSz = c; } break;
										case 2: { const SI16 c = *((SI16*)&src[mSz->iPos]); if(c > 0) arrSz = c; } break;
										case 4: { const SI32 c = *((SI32*)&src[mSz->iPos]); if(c > 0) arrSz = c; } break;
										case 8: { const SI64 c = *((SI64*)&src[mSz->iPos]); if(c > 0) arrSz = c; } break;
#										ifdef NB_CONFIG_INCLUDE_ASSERTS
										default:
											NBASSERT(FALSE) 
											break;
#										endif
									}
									break;
								case ENNBStructMapMemberType_UInt:
									switch(mSz->size) {
										case 1: arrSz = *((UI8*)&src[mSz->iPos]); break;
										case 2: arrSz = *((UI16*)&src[mSz->iPos]); break;
										case 4: arrSz = *((UI32*)&src[mSz->iPos]); break;
										case 8: arrSz = *((UI64*)&src[mSz->iPos]); break;
#										ifdef NB_CONFIG_INCLUDE_ASSERTS
										default:
											NBASSERT(FALSE) 
											break;
#										endif
									}
									break;
								default:
									NBASSERT(FALSE)
									break;
							}
						}
						//Ptr-jump
						{
							const UI32 jmpRef = 0;
							NBCrc32_feed(job, &jmpRef, sizeof(jmpRef));
						}
						//Value
						if(arrData != NULL){
							switch(m->data.type) {
								case ENNBStructMapMemberType_Bool:
								case ENNBStructMapMemberType_Int:
								case ENNBStructMapMemberType_UInt:
								case ENNBStructMapMemberType_Enum:
								case ENNBStructMapMemberType_Float:
								case ENNBStructMapMemberType_Double:
								case ENNBStructMapMemberType_Chars:
								case ENNBStructMapMemberType_Bytes:
									//Optimization, clone multiple natives at once
									r = NBStruct_stCalculateCrcOneMemberValueArrNatives_(structMap, m, arrData, m->data.size, arrSz, job);
									break;
								default:
									//Copy one by one (strings and structs)
									{
										UI64 i; for(i = 0 ; i < arrSz && r; i++){
											const BYTE* src = &arrData[m->data.size * i];
											//Separator
											{
												const UI32 jmpRef = 0;
												NBCrc32_feed(job, &jmpRef, sizeof(jmpRef));
											}
											//Value
											{
												r = NBStruct_stCalculateCrcOneMemberValue_(structMap, m, src, m->data.size, job);
											}
										}
									}
									break;
							}
						}
					}
				}
			} else {
				//------------------
				//- Embeded array
				//------------------
				NBASSERT(m->elem.vCount == NULL)
				if(m->elem.isPtr){
					UI64 i; for(i = 0 ; i < m->elem.count && r; i++){
						const void** pSrc = (const void**)&src[m->iPos + (sizeof(void*) * i)];
						//Ptr-jump
						{
							const UI32 jmpRef = 0;
							NBCrc32_feed(job, &jmpRef, sizeof(jmpRef));
						}
						//Value
						if(*pSrc != NULL){
							r = NBStruct_stCalculateCrcOneMemberValue_(structMap, m, *pSrc, m->data.size, job);
						}
					}
				} else {
					switch(m->data.type) {
						case ENNBStructMapMemberType_Bool:
						case ENNBStructMapMemberType_Int:
						case ENNBStructMapMemberType_UInt:
						case ENNBStructMapMemberType_Enum:
						case ENNBStructMapMemberType_Float:
						case ENNBStructMapMemberType_Double:
						case ENNBStructMapMemberType_Chars:
						case ENNBStructMapMemberType_Bytes:
							//Optimization, copy multiple natives at once
							r = NBStruct_stCalculateCrcOneMemberValueArrNatives_(structMap, m, &src[m->iPos], m->data.size, m->elem.count, job);
							break;
						default:
							//Copy one by one (strings and structs)
							{
								UI64 i; for(i = 0 ; i < m->elem.count && r; i++){
									const BYTE* eSrc = &src[m->iPos + (m->data.size * i)];
									//Separator
									{
										const UI32 jmpRef = 0;
										NBCrc32_feed(job, &jmpRef, sizeof(jmpRef));
									}
									//Value
									{
										r = NBStruct_stCalculateCrcOneMemberValue_(structMap, m, eSrc, m->data.size, job);
									}
								}
							}
							break;
					}
				}
			}
		}
	}
	return r;
}

STNBStructCrc NBStruct_stCalculateCrc(const STNBStructMap* structMap, const void* src, const UI32 srcSz){
	STNBStructCrc r;
	NBMemory_setZeroSt(r, STNBStructCrc);
	if(structMap->stSize != srcSz){
		NBASSERT(FALSE)
	} else {
		STNBCrc32 job;
		NBCrc32_init(&job);
		if(NBStruct_stCalculateCrcMembers_(structMap, src, srcSz, &job)){
			NBCrc32_finish(&job);
			r.crc32		= job.hash;
			r.bytesFed	= job.totalBytesFed;
		}
		NBCrc32_release(&job);
	}
	return r;
}

BOOL NBStruct_stIsEqualByCrc(const STNBStructMap* structMap, const void* src, const UI32 srcSz, const void* src2, const UI32 src2Sz){
	BOOL r = FALSE;
	if(srcSz == src2Sz){
		if(src == src2){ //Are NULL or the same
			r = TRUE;
		} else {
			const STNBStructCrc crc = NBStruct_stCalculateCrc(structMap, src, srcSz);
			const STNBStructCrc crc2 = NBStruct_stCalculateCrc(structMap, src2, src2Sz);
			r = (crc.crc32 == crc2.crc32 && crc.bytesFed == crc2.bytesFed ? TRUE : FALSE);
		}
	}
	return r;
}

//Clone

BOOL NBStruct_stCloneMembers_(const STNBStructMap* structMap, const void* src, const UI32 srcSz, void* dst);

BOOL NBStruct_stCloneOneMemberValue_(const STNBStructMap* structMap, const STNBStructMapMember* m, const void* mSrc, const UI32 srcSz, void* mDst){
	BOOL r = TRUE;
	switch(m->data.type) {
		case ENNBStructMapMemberType_Bool:
		case ENNBStructMapMemberType_Int:
		case ENNBStructMapMemberType_UInt:
		case ENNBStructMapMemberType_Enum:
			NBASSERT(srcSz == 1 || srcSz == 2 || srcSz == 4 || srcSz == 8)
			NBMemory_copy(mDst, mSrc, srcSz);
			break;
		case ENNBStructMapMemberType_Float:
			NBASSERT(srcSz == sizeof(float))
			NBMemory_copy(mDst, mSrc, srcSz);
			break;
		case ENNBStructMapMemberType_Double:
			NBASSERT(srcSz == sizeof(double))
			NBMemory_copy(mDst, mSrc, srcSz);
			break;
		case ENNBStructMapMemberType_Chars:
			NBMemory_copy(mDst, mSrc, srcSz);
			break;
		case ENNBStructMapMemberType_Bytes:
			NBMemory_copy(mDst, mSrc, srcSz);
			break;
		case ENNBStructMapMemberType_StrPtr:
			{
				NBASSERT(srcSz == sizeof(void*))
				char* value = *((char**)mSrc);
				if(value == NULL){
					*((char**)mDst)	= NULL;
				} else {
					const UI32 sz	= NBString_strLenBytes(value);
					char* copy		= (char*)NBMemory_alloc(sz + 1);
					NBMemory_copy(copy, value, sz + 1);
					*((char**)mDst)	= copy;
				}
			}
			break;
		case ENNBStructMapMemberType_Struct:
			if(m->data.stMap == NULL){
				NBASSERT(FALSE)
				r = FALSE;
			} else {
				NBASSERT(srcSz == m->data.stMap->stSize)
				NBStruct_stCloneMembers_(m->data.stMap, mSrc, srcSz, mDst);
			}
			break;
		default:
			NBASSERT(FALSE)
			r = FALSE;
			break;
	}
	return r;
}

BOOL NBStruct_stCloneOneMemberValueArrNatives_(const STNBStructMap* structMap, const STNBStructMapMember* m, const void* mSrc, const UI32 srcSz, const UI64 arrSz, void* mDst){
	BOOL r = TRUE;
	switch(m->data.type) {
		case ENNBStructMapMemberType_Bool:
		case ENNBStructMapMemberType_Int:
		case ENNBStructMapMemberType_UInt:
		case ENNBStructMapMemberType_Enum:
			NBASSERT(srcSz == 1 || srcSz == 2 || srcSz == 4 || srcSz == 8)
			NBMemory_copy(mDst, mSrc, srcSz * arrSz);
			break;
		case ENNBStructMapMemberType_Float:
			NBASSERT(srcSz == sizeof(float))
			NBMemory_copy(mDst, mSrc, srcSz * arrSz);
			break;
		case ENNBStructMapMemberType_Double:
			NBASSERT(srcSz == sizeof(double))
			NBMemory_copy(mDst, mSrc, srcSz * arrSz);
			break;
		case ENNBStructMapMemberType_Chars:
			NBMemory_copy(mDst, mSrc, srcSz * arrSz);
			break;
		case ENNBStructMapMemberType_Bytes:
			NBMemory_copy(mDst, mSrc, srcSz * arrSz);
			break;
		default:
			NBASSERT(FALSE)
			r = FALSE;
			break;
	}
	return r;
}

BOOL NBStruct_stCloneMembers_(const STNBStructMap* structMap, const void* pSrc, const UI32 srcSz, void* pDst){
	BOOL r = FALSE;
	if(structMap->stSize != srcSz){
		NBASSERT(FALSE)
	} else {
		UI16 i = 0;
		r = TRUE;
		//clone
		{
			const BYTE* src = (const BYTE*)pSrc;
			BYTE* dst = (BYTE*)pDst;
			for(; i < structMap->mbrsSz && r; i++){
				STNBStructMapMember* m = &structMap->mbrs[i];
				NBASSERT(m->size > 0 && m->size == (m->elem.count * (m->elem.isPtr ? sizeof(void*) : m->data.size)))
				if(m->elem.count == 1){
					//------------------
					//- Only one element
					//------------------
					if(!m->elem.isPtr){
						//One embeded element
						r = NBStruct_stCloneOneMemberValue_(structMap, m, &src[m->iPos], m->data.size, &dst[m->iPos]);
					} else {
						//One emeded pointer
						if(m->elem.vCount == NULL){
							//Pointer to one element
							void* s = *((void**)&src[m->iPos]);
							if(s == NULL){
								*((void**)&dst[m->iPos]) = NULL;
							} else {
								void* d = NBMemory_alloc(m->data.size);
								NBMemory_set(d, 0, m->data.size);
								r = NBStruct_stCloneOneMemberValue_(structMap, m, s, m->data.size, d);
								*((void**)&dst[m->iPos]) = d;
							}
						} else {
							//Pointer to an array
							STNBStructMapMember* mSz = m->elem.vCount;
							BYTE* arrData	= *((BYTE**)&src[m->iPos]);
							UI64 arrSz = 0;
							{
								switch(mSz->data.type) {
								case ENNBStructMapMemberType_Int:
									switch(mSz->size) {
									case 1: { const SI8 c = *((SI8*)&dst[mSz->iPos]) = *((SI8*)&src[mSz->iPos]); if(c > 0) arrSz = c; } break;
									case 2: { const SI16 c = *((SI16*)&dst[mSz->iPos]) = *((SI16*)&src[mSz->iPos]); if(c > 0) arrSz = c; } break;
									case 4: { const SI32 c = *((SI32*)&dst[mSz->iPos]) = *((SI32*)&src[mSz->iPos]); if(c > 0) arrSz = c; } break;
									case 8: { const SI64 c = *((SI64*)&dst[mSz->iPos]) = *((SI64*)&src[mSz->iPos]); if(c > 0) arrSz = c; } break;
#									ifdef NB_CONFIG_INCLUDE_ASSERTS
									default:
										NBASSERT(FALSE) 
										break;
#									endif
									}
									break;
								case ENNBStructMapMemberType_UInt:
									switch(mSz->size) {
									case 1: arrSz = *((UI8*)&dst[mSz->iPos]) = *((UI8*)&src[mSz->iPos]); break;
									case 2: arrSz = *((UI16*)&dst[mSz->iPos]) = *((UI16*)&src[mSz->iPos]); break;
									case 4: arrSz = *((UI32*)&dst[mSz->iPos]) = *((UI32*)&src[mSz->iPos]); break;
									case 8: arrSz = *((UI64*)&dst[mSz->iPos]) = *((UI64*)&src[mSz->iPos]); break;
#									ifdef NB_CONFIG_INCLUDE_ASSERTS
									default:
										NBASSERT(FALSE) 
										break;
#									endif
									}
									break;
								default:
									NBASSERT(FALSE)
									break;
								}
							}
							if(arrData == NULL){
								NBASSERT(arrSz == 0)
								*((BYTE**)&dst[m->iPos]) = NULL;
							} else {
								BYTE* arrDst = NBMemory_alloc(m->data.size * arrSz);
								NBMemory_set(arrDst, 0, m->data.size * arrSz);
								switch(m->data.type) {
								case ENNBStructMapMemberType_Bool:
								case ENNBStructMapMemberType_Int:
								case ENNBStructMapMemberType_UInt:
								case ENNBStructMapMemberType_Enum:
								case ENNBStructMapMemberType_Float:
								case ENNBStructMapMemberType_Double:
								case ENNBStructMapMemberType_Chars:
								case ENNBStructMapMemberType_Bytes:
									//Optimization, clone multiple natives at once
									r = NBStruct_stCloneOneMemberValueArrNatives_(structMap, m, arrData, m->data.size, arrSz, arrDst);
									break;
								default:
									//Copy one by one (strings and structs)
									{
										UI64 i; for(i = 0 ; i < arrSz && r; i++){
											const BYTE* src = &arrData[m->data.size * i];
											BYTE* dst = &arrDst[m->data.size * i];
											r = NBStruct_stCloneOneMemberValue_(structMap, m, src, m->data.size, dst);
										}
									}
									break;
								}
								*((BYTE**)&dst[m->iPos]) = arrDst;
							}
						}
					}
				} else {
					//------------------
					//- Embeded array
					//------------------
					NBASSERT(m->elem.vCount == NULL)
					if(m->elem.isPtr){
						UI64 i; for(i = 0 ; i < m->elem.count && r; i++){
							const void** pSrc = (const void**)&src[m->iPos + (sizeof(void*) * i)];
							void** pDst = (void**)&dst[m->iPos + (sizeof(void*) * i)];
							if(*pSrc == NULL){
								*pDst = NULL;
							} else {
								*pDst = NBMemory_alloc(m->data.size);
								NBMemory_set(*pDst, 0, m->data.size);
								r = NBStruct_stCloneOneMemberValue_(structMap, m, *pSrc, m->data.size, *pDst);
							}
						}
					} else {
						switch(m->data.type) {
						case ENNBStructMapMemberType_Bool:
						case ENNBStructMapMemberType_Int:
						case ENNBStructMapMemberType_UInt:
						case ENNBStructMapMemberType_Enum:
						case ENNBStructMapMemberType_Float:
						case ENNBStructMapMemberType_Double:
						case ENNBStructMapMemberType_Chars:
						case ENNBStructMapMemberType_Bytes:
							//Optimization, copy multiple natives at once
							r = NBStruct_stCloneOneMemberValueArrNatives_(structMap, m, &src[m->iPos], m->data.size, m->elem.count, &dst[m->iPos]);
							break;
						default:
							//Copy one by one (strings and structs)
							{
								UI64 i; for(i = 0 ; i < m->elem.count && r; i++){
									const BYTE* eSrc = &src[m->iPos + (m->data.size * i)];
									BYTE* eDst = &dst[m->iPos + (m->data.size * i)];
									r = NBStruct_stCloneOneMemberValue_(structMap, m, eSrc, m->data.size, eDst);
								}
							}
							break;
						}
					}
				}
			}
		}
		//Release cloned members (if error)
		if(!r && i > 0){
			NBStruct_stReleaseMembers_(structMap, i, pDst, srcSz);
		}
	}
	return r;
}

BOOL NBStruct_stClone(const STNBStructMap* structMap, const void* src, const UI32 srcSz, void* dst, const UI32 dstSz){
	BOOL r = FALSE;
	if(structMap->stSize != srcSz || structMap->stSize != dstSz){
		NBASSERT(FALSE)
	} else {
		r = NBStruct_stCloneMembers_(structMap, src, srcSz, dst);
	}
	return r;
}

//Release

BOOL NBStruct_stReleaseOneMemberValue_(const STNBStructMap* structMap, const STNBStructMapMember* m, void* mSrc, const UI32 mSz){
	BOOL r = TRUE;
	switch(m->data.type) {
		case ENNBStructMapMemberType_StrPtr:
			{
				char* value = *((char**)mSrc);
				if(value != NULL){
					NBMemory_free(value);
					*((char**)mSrc) = NULL;
				}
			}
			break;
		case ENNBStructMapMemberType_Struct:
			if(m->data.stMap == NULL){
				r = FALSE; NBASSERT(FALSE)
			} else {
				NBStruct_stReleaseMembers_(m->data.stMap, m->data.stMap->mbrsSz, mSrc, m->data.stMap->stSize);
			}
			break;
		default:
			NBASSERT(FALSE)
			break;
	}
	return r;
}

BOOL NBStruct_stReleaseMember_(const STNBStructMap* structMap, void* pSrc, STNBStructMapMember* m){
	BOOL r = FALSE;
	BYTE* src = (BYTE*)pSrc;
	NBASSERT(m != NULL)
	NBASSERT(m >= structMap->mbrs && m < &structMap->mbrs[structMap->mbrsSz])
	NBASSERT(m->size > 0 && m->size == (m->elem.count * (m->elem.isPtr ? sizeof(void*) : m->data.size)))
	if(m->size > 0 && m->size == (m->elem.count * (m->elem.isPtr ? sizeof(void*) : m->data.size))){
		r = TRUE;
		if(m->elem.count == 1){
			//------------------
			//- Only one element
			//------------------
			if(!m->elem.isPtr){
				//One embeded element
				if(m->data.type == ENNBStructMapMemberType_StrPtr || m->data.type == ENNBStructMapMemberType_Struct){
					r = NBStruct_stReleaseOneMemberValue_(structMap, m, &src[m->iPos], m->data.size);
				}
			} else {
				//One embedded pointer
				if(m->elem.vCount == NULL){
					//Pointer to one element
					void* s = *((void**)&src[m->iPos]);
					if(s != NULL){
						if(m->data.type == ENNBStructMapMemberType_StrPtr || m->data.type == ENNBStructMapMemberType_Struct){
							r = NBStruct_stReleaseOneMemberValue_(structMap, m, s, m->data.size);
						}
						NBMemory_free(s);
						*((void**)&src[m->iPos]) = NULL;
					}
				} else {
					//Pointer to an array
					STNBStructMapMember* mSz = m->elem.vCount;
					UI64 arrSz = 0;
					//Obtain array-size and zero it
					{
						switch(mSz->data.type) {
							case ENNBStructMapMemberType_Int:
								switch(mSz->size) {
									case 1: { const SI8 c = *((SI8*)&src[mSz->iPos]); *((SI8*)&src[mSz->iPos]) = 0; if(c > 0) arrSz = c; } break;
									case 2: { const SI16 c = *((SI16*)&src[mSz->iPos]); *((SI16*)&src[mSz->iPos]) = 0; if(c > 0) arrSz = c; } break;
									case 4: { const SI32 c = *((SI32*)&src[mSz->iPos]); *((SI32*)&src[mSz->iPos]) = 0; if(c > 0) arrSz = c; } break;
									case 8: { const SI64 c = *((SI64*)&src[mSz->iPos]); *((SI64*)&src[mSz->iPos]) = 0; if(c > 0) arrSz = c; } break;
#									ifdef NB_CONFIG_INCLUDE_ASSERTS
									default:
										NBASSERT(FALSE) 
										break;
#									endif
								}
								break;
							case ENNBStructMapMemberType_UInt:
								switch(mSz->size) {
									case 1: arrSz = *((UI8*)&src[mSz->iPos]); *((UI8*)&src[mSz->iPos]) = 0; break;
									case 2: arrSz = *((UI16*)&src[mSz->iPos]); *((UI16*)&src[mSz->iPos]) = 0; break;
									case 4: arrSz = *((UI32*)&src[mSz->iPos]); *((UI32*)&src[mSz->iPos]) = 0; break;
									case 8: arrSz = *((UI64*)&src[mSz->iPos]); *((UI64*)&src[mSz->iPos]) = 0; break;
#									ifdef NB_CONFIG_INCLUDE_ASSERTS
									default:
										NBASSERT(FALSE) 
										break;
#									endif
								}
								break;
							default:
								NBASSERT(FALSE)
								break;
						}
					}
					//Release data
					{
						BYTE* arrData = *((BYTE**)&src[m->iPos]);
						if(arrData != NULL){
							if(m->data.type == ENNBStructMapMemberType_StrPtr || m->data.type == ENNBStructMapMemberType_Struct){
								UI64 i; for(i = 0 ; i < arrSz && r; i++){
									void* src = (void*)&arrData[m->data.size * i];
									r = NBStruct_stReleaseOneMemberValue_(structMap, m, src, m->data.size);
								}
							}
							NBMemory_free(arrData);
							*((BYTE**)&src[m->iPos]) = NULL;
						}
					}
				}
			}
		} else {
			//------------------
			//- Embeded array
			//------------------
			NBASSERT(m->elem.vCount == NULL)
			if(m->elem.isPtr){
				UI64 i; for(i = 0 ; i < m->elem.count && r; i++){
					void** pSrc = (void**)&src[m->iPos + (sizeof(void*) * i)];
					if(*pSrc != NULL){
						if(m->data.type == ENNBStructMapMemberType_StrPtr || m->data.type == ENNBStructMapMemberType_Struct){
							r = NBStruct_stReleaseOneMemberValue_(structMap, m, *pSrc, m->data.size);
						}
						NBMemory_free(*pSrc);
						*pSrc = NULL;
					}
				}
			} else {
				if(m->data.type == ENNBStructMapMemberType_StrPtr || m->data.type == ENNBStructMapMemberType_Struct){
					UI64 i; for(i = 0 ; i < m->elem.count && r; i++){
						void* eSrc = (void*)&src[m->iPos + (m->data.size * i)];
						r = NBStruct_stReleaseOneMemberValue_(structMap, m, eSrc, m->data.size);
					}
				}
			}
		}
	}
	return r;
}


//

BOOL NBStruct_stReleaseMemberInternalStructs_(const STNBStructMap* structMap, void* pSrc, STNBStructMapMember* m, const STNBStructMap* structMapFilter);
BOOL NBStruct_stReleaseInternalStructs_(const STNBStructMap* structMap, void* pSrc, const UI32 srcSz, const STNBStructMap* structMapFilter);

BOOL NBStruct_stReleaseOneMemberValueInternalStructs_(const STNBStructMap* structMap, const STNBStructMapMember* m, void* mSrc, const UI32 mSz, const STNBStructMap* structMapFilter){
	BOOL r = TRUE;
	switch(m->data.type) {
		case ENNBStructMapMemberType_Struct:
			if(m->data.stMap == NULL){
				r = FALSE; NBASSERT(FALSE)
			} else {
				if(structMapFilter != NULL && m->data.stMap == structMapFilter){
					NBStruct_stReleaseMembers_(m->data.stMap, m->data.stMap->mbrsSz, mSrc, m->data.stMap->stSize);
					NBMemory_set(mSrc, 0, m->data.stMap->stSize);
				} else {
					NBStruct_stReleaseInternalStructs_(m->data.stMap, mSrc, m->data.stMap->stSize, structMapFilter);
				}
			}
			break;
		default:
			NBASSERT(FALSE)
			break;
	}
	return r;
}

BOOL NBStruct_stReleaseMemberInternalStructs_(const STNBStructMap* structMap, void* pSrc, STNBStructMapMember* m, const STNBStructMap* structMapFilter){
	BOOL r = FALSE;
	BYTE* src = (BYTE*)pSrc;
	NBASSERT(m != NULL)
	NBASSERT(m >= structMap->mbrs && m < &structMap->mbrs[structMap->mbrsSz])
	NBASSERT(m->size > 0 && m->size == (m->elem.count * (m->elem.isPtr ? sizeof(void*) : m->data.size)))
	if(m->size > 0 && m->size == (m->elem.count * (m->elem.isPtr ? sizeof(void*) : m->data.size))){
		r = TRUE;
		if(m->elem.count == 1){
			//------------------
			//- Only one element
			//------------------
			if(!m->elem.isPtr){
				//One embeded element
				if(/*m->data.type == ENNBStructMapMemberType_StrPtr ||*/ m->data.type == ENNBStructMapMemberType_Struct){
					r = NBStruct_stReleaseOneMemberValueInternalStructs_(structMap, m, &src[m->iPos], m->data.size, structMapFilter);
				}
			} else {
				//One embedded pointer
				if(m->elem.vCount == NULL){
					//Pointer to one element
					void* s = *((void**)&src[m->iPos]);
					if(s != NULL){
						if(/*m->data.type == ENNBStructMapMemberType_StrPtr ||*/ m->data.type == ENNBStructMapMemberType_Struct){
							r = NBStruct_stReleaseOneMemberValueInternalStructs_(structMap, m, s, m->data.size, structMapFilter);
							if(structMapFilter != NULL && structMap == structMapFilter){
								NBMemory_free(s);
								*((void**)&src[m->iPos]) = NULL;
							}
						}
					}
				} else {
					//Pointer to an array
					STNBStructMapMember* mSz = m->elem.vCount;
					UI64 arrSz = 0;
					//Obtain array-size and zero it
					{
						switch(mSz->data.type) {
							case ENNBStructMapMemberType_Int:
								switch(mSz->size) {
									case 1: { const SI8 c = *((SI8*)&src[mSz->iPos]); /* *((SI8*)&src[mSz->iPos]) = 0;*/ if(c > 0) arrSz = c; } break;
									case 2: { const SI16 c = *((SI16*)&src[mSz->iPos]); /* *((SI16*)&src[mSz->iPos]) = 0;*/ if(c > 0) arrSz = c; } break;
									case 4: { const SI32 c = *((SI32*)&src[mSz->iPos]); /* *((SI32*)&src[mSz->iPos]) = 0;*/ if(c > 0) arrSz = c; } break;
									case 8: { const SI64 c = *((SI64*)&src[mSz->iPos]); /* *((SI64*)&src[mSz->iPos]) = 0;*/ if(c > 0) arrSz = c; } break;
#									ifdef NB_CONFIG_INCLUDE_ASSERTS
									default:
										NBASSERT(FALSE) 
										break;
#									endif
								}
								break;
							case ENNBStructMapMemberType_UInt:
								switch(mSz->size) {
									case 1: arrSz = *((UI8*)&src[mSz->iPos]); /* *((UI8*)&src[mSz->iPos]) = 0;*/ break;
									case 2: arrSz = *((UI16*)&src[mSz->iPos]); /* *((UI16*)&src[mSz->iPos]) = 0;*/ break;
									case 4: arrSz = *((UI32*)&src[mSz->iPos]); /* *((UI32*)&src[mSz->iPos]) = 0;*/ break;
									case 8: arrSz = *((UI64*)&src[mSz->iPos]); /* *((UI64*)&src[mSz->iPos]) = 0;*/ break;
#									ifdef NB_CONFIG_INCLUDE_ASSERTS
									default:
										NBASSERT(FALSE) 
										break;
#									endif
								}
								break;
							default:
								NBASSERT(FALSE)
								break;
						}
					}
					//Release data
					{
						BYTE* arrData = *((BYTE**)&src[m->iPos]);
						if(arrData != NULL){
							if(/*m->data.type == ENNBStructMapMemberType_StrPtr ||*/ m->data.type == ENNBStructMapMemberType_Struct){
								UI64 i; for(i = 0 ; i < arrSz && r; i++){
									void* src = (void*)&arrData[m->data.size * i];
									r = NBStruct_stReleaseOneMemberValueInternalStructs_(structMap, m, src, m->data.size, structMapFilter);
								}
							}
							//NBMemory_free(arrData);
							//*((BYTE**)&src[m->iPos]) = NULL;
						}
					}
				}
			}
		} else {
			//------------------
			//- Embeded array
			//------------------
			NBASSERT(m->elem.vCount == NULL)
			if(m->elem.isPtr){
				UI64 i; for(i = 0 ; i < m->elem.count && r; i++){
					void** pSrc = (void**)&src[m->iPos + (sizeof(void*) * i)];
					if(*pSrc != NULL){
						if(/*m->data.type == ENNBStructMapMemberType_StrPtr ||*/ m->data.type == ENNBStructMapMemberType_Struct){
							r = NBStruct_stReleaseOneMemberValueInternalStructs_(structMap, m, *pSrc, m->data.size, structMapFilter);
							if(structMapFilter != NULL && structMap == structMapFilter){
								NBMemory_free(*pSrc);
								*pSrc = NULL;
							}
						}
					}
				}
			} else {
				if(/*m->data.type == ENNBStructMapMemberType_StrPtr ||*/ m->data.type == ENNBStructMapMemberType_Struct){
					UI64 i; for(i = 0 ; i < m->elem.count && r; i++){
						void* eSrc = (void*)&src[m->iPos + (m->data.size * i)];
						r = NBStruct_stReleaseOneMemberValueInternalStructs_(structMap, m, eSrc, m->data.size, structMapFilter);
					}
				}
			}
		}
	}
	return r;
}

BOOL NBStruct_stReleaseMembers_(const STNBStructMap* structMap, const UI16 mbrsMax, void* pSrc, const UI32 srcSz){
	BOOL r = FALSE;
	NBASSERT(mbrsMax <= structMap->mbrsSz)
	if(structMap->stSize != srcSz){
		NBASSERT(FALSE)
	} else {
		r = TRUE;
		{
			UI16 i; for(i = 0; i < mbrsMax && r; i++){
				STNBStructMapMember* m = &structMap->mbrs[i];
				NBStruct_stReleaseMember_(structMap, pSrc, m);
			}
		}
	}
	return r;
}

BOOL NBStruct_stRelease(const STNBStructMap* structMap, void* src, const UI32 srcSz){
	BOOL r = FALSE;
	if(structMap->stSize != srcSz){
		NBASSERT(FALSE)
	} else {
		if(NBStruct_stReleaseMembers_(structMap, structMap->mbrsSz, src, srcSz)){
			NBMemory_set(src, 0, srcSz);
			r = TRUE;
		}
	}
	return r;
}

//Release only structs

BOOL NBStruct_stReleaseInternalStructs_(const STNBStructMap* structMap, void* pSrc, const UI32 srcSz, const STNBStructMap* structMapFilter){
	BOOL r = FALSE;
	if(structMap->stSize != srcSz){
		NBASSERT(FALSE)
	} else {
		r = TRUE;
		UI32 i;
		for(i = 0; i < structMap->mbrsSz && r; i++){
			STNBStructMapMember* m = &structMap->mbrs[i];
			NBStruct_stReleaseMemberInternalStructs_(structMap, pSrc, m, structMapFilter);
		}
	}
	return r;
}

BOOL NBStruct_stReleaseInternalStructs(const STNBStructMap* structMap, void* src, const UI32 srcSz, const STNBStructMap* structMapFilter){
	BOOL r = FALSE;
	if(structMap->stSize != srcSz){
		NBASSERT(FALSE)
	} else if(structMapFilter != NULL && structMap == structMapFilter){
		if(NBStruct_stReleaseMembers_(structMap, structMap->mbrsSz, src, srcSz)){
			NBMemory_set(src, 0, srcSz);
			r = TRUE;
		}
	} else {
		if(NBStruct_stReleaseInternalStructs_(structMap, src, srcSz, structMapFilter)){
			r = TRUE;
		}
	}
	return r;
}

//Individual tasks

STNBStructMapMember* NBStruct_stGetMember_(const STNBStructMap* structMap, const char* name, const void* pSrc, const UI32 srcSz){
	STNBStructMapMember* r = NULL;
	if(structMap->stSize != srcSz){
		NBASSERT(FALSE)
	} else {
		UI32 i;
		for(i = 0; i < structMap->mbrsSz; i++){
			STNBStructMapMember* m = &structMap->mbrs[i];
			NBASSERT(m->size > 0 && m->size == (m->elem.count * (m->elem.isPtr ? sizeof(void*) : m->data.size)))
			if(NBString_strIsEqual(m->name, name)){
				r = m;
				break;
			}
		}
	}
	NBASSERT(r != NULL) //Just testing
	return r;
}

void* NBStruct_stGetMemberValue(const STNBStructMap* structMap, const char* name, const void* pSrc, const UI32 srcSz, const UI32 mbrSz){
	void* r = NULL;
	STNBStructMapMember* m = NBStruct_stGetMember_(structMap, name, pSrc, srcSz); NBASSERT(m != NULL)
	if(m != NULL){
		NBASSERT(m->size == mbrSz)
		if(m->size == mbrSz){
			BYTE* src = (BYTE*)pSrc;
			r = &src[m->iPos];
		}
	}
	return r;
}
			   
void* NBStruct_stEmptyMemberValue(const STNBStructMap* structMap, const char* name, void* pSrc, const UI32 srcSz, const UI32 mbrSz){
	void* r = NULL;
	STNBStructMapMember* m = NBStruct_stGetMember_(structMap, name, pSrc, srcSz); NBASSERT(m != NULL)
	if(m != NULL){
		NBASSERT(m->size == mbrSz)
		if(m->size == mbrSz){
			BYTE* src = (BYTE*)pSrc;
			NBStruct_stReleaseMember_(structMap, pSrc, m);
			r = &src[m->iPos];
		}
	}
	return r;
}

BOOL NBStruct_stAddItmToArrayMember(const STNBStructMap* structMap, const char* name, void* pSrc, const UI32 srcSz, const UI32 mbrSz, void* newItm, const UI32 newItmSz){
	BOOL r = FALSE;
	STNBStructMapMember* m = NBStruct_stGetMember_(structMap, name, pSrc, srcSz); NBASSERT(m != NULL)
	if(m != NULL){
		NBASSERT(m->size == mbrSz)
		NBASSERT(m->data.size == newItmSz)
		if(m->size == mbrSz && m->data.size == newItmSz){
			BYTE* src = (BYTE*)pSrc;
			NBASSERT(m != NULL)
			NBASSERT(m >= structMap->mbrs && m < &structMap->mbrs[structMap->mbrsSz])
			NBASSERT(m->size > 0 && m->size == (m->elem.count * (m->elem.isPtr ? sizeof(void*) : m->data.size)))
			if(m->size > 0 && m->size == (m->elem.count * (m->elem.isPtr ? sizeof(void*) : m->data.size))){
				if(m->elem.count == 1){
					//------------------
					//- Only one element
					//------------------
					if(!m->elem.isPtr){
						//One embeded element (no mutable)
						NBASSERT(FALSE)
					} else {
						//One emeded pointer
						if(m->elem.vCount == NULL){
							//Pointer to one element (no mutable)
							NBASSERT(FALSE)
						} else {
							//Pointer to an array
							//Read current size (and increase one)
							UI64 arrSz = 0;
							{
								STNBStructMapMember* mSz = m->elem.vCount;
								switch(mSz->data.type) {
									case ENNBStructMapMemberType_Int:
										switch(mSz->size) {
											case 1: { const SI8 c = *((SI8*)&src[mSz->iPos]); if(c > 0) arrSz = c; *((SI8*)&src[mSz->iPos]) = (SI8)(arrSz + 1); NBASSERT(arrSz < *((SI8*)&src[mSz->iPos])) } break;
											case 2: { const SI16 c = *((SI16*)&src[mSz->iPos]); if(c > 0) arrSz = c; *((SI16*)&src[mSz->iPos]) = (SI16)(arrSz + 1); NBASSERT(arrSz < *((SI16*)&src[mSz->iPos])) } break;
											case 4: { const SI32 c = *((SI32*)&src[mSz->iPos]); if(c > 0) arrSz = c; *((SI32*)&src[mSz->iPos]) = (SI32)(arrSz + 1); NBASSERT(arrSz < *((SI32*)&src[mSz->iPos])) } break;
											case 8: { const SI64 c = *((SI64*)&src[mSz->iPos]); if(c > 0) arrSz = c; *((SI64*)&src[mSz->iPos]) = (SI64)(arrSz + 1); NBASSERT(arrSz < *((SI64*)&src[mSz->iPos])) } break;
#											ifdef NB_CONFIG_INCLUDE_ASSERTS
											default:
												NBASSERT(FALSE) 
												break;
#											endif
										}
										break;
									case ENNBStructMapMemberType_UInt:
										switch(mSz->size) {
											case 1: arrSz = *((UI8*)&src[mSz->iPos]); *((UI8*)&src[mSz->iPos]) = (UI8)(arrSz + 1); NBASSERT(arrSz < *((UI8*)&src[mSz->iPos])) break;
											case 2: arrSz = *((UI16*)&src[mSz->iPos]); *((UI16*)&src[mSz->iPos]) = (UI16)(arrSz + 1); NBASSERT(arrSz < *((UI16*)&src[mSz->iPos])) break;
											case 4: arrSz = *((UI32*)&src[mSz->iPos]); *((UI32*)&src[mSz->iPos]) = (UI32)(arrSz + 1); NBASSERT(arrSz < *((UI32*)&src[mSz->iPos])) break;
											case 8: arrSz = *((UI64*)&src[mSz->iPos]); *((UI64*)&src[mSz->iPos]) = (UI64)(arrSz + 1); NBASSERT(arrSz < *((UI64*)&src[mSz->iPos])) break;
#											ifdef NB_CONFIG_INCLUDE_ASSERTS
											default:
												NBASSERT(FALSE) 
												break;
#											endif
										}
										break;
									default:
										NBASSERT(FALSE)
										break;
								}
							}
							NBASSERT(m->data.size == newItmSz)
							//Create new array
							{
								BYTE* arrData = *((BYTE**)&src[m->iPos]);
								NBASSERT(arrData != NULL || arrSz == 0)
								BYTE* newArray = (BYTE*)NBMemory_alloc(m->data.size * (arrSz + 1));
								if(arrData != NULL){
									if(arrSz > 0){
										NBMemory_copy(newArray, arrData, m->data.size * arrSz);
									}
									NBMemory_free(arrData);
									arrData = NULL;
								}
								//Copy new element
								NBMemory_copy(&newArray[m->data.size * arrSz], newItm, m->data.size);
								//Set new array
								*((BYTE**)&src[m->iPos]) = newArray;
								r = TRUE;
							}
							//ToDo: remove
							/*if(arrData != NULL){
								if(m->data.type == ENNBStructMapMemberType_StrPtr || m->data.type == ENNBStructMapMemberType_Struct){
									UI64 i; for(i = 0 ; i < arrSz && r; i++){
										void* src = (void*)&arrData[m->data.size * i];
										r = NBStruct_..._(structMap, m, src, m->data.size);
									}
								}
								NBMemory_free(arrData);
								*((BYTE**)&src[m->iPos]) = NULL;
							}*/
						}
					}
				} else {
					//------------------
					//- Embeded array (no mutable)
					//------------------
					NBASSERT(FALSE)
				}
			}
		}
	}
	NBASSERT(r)
	return r;
}

//

BOOL NBStruct_test(void){
	BOOL r = FALSE;
	enum enSimple_ {
		enSimple_Morning = 0,
		enSimple_Noon,
		enSimple_Night
	};
	STNBEnumMapRecord enSimple_mapRecs[] = {
		{ enSimple_Morning, "enSimple_Morning", "morning" }
		, { enSimple_Noon, "enSimple_Noon", "noon" }
		, {	enSimple_Night, "enSimple_Night", "night" }
	};
	STNBEnumMap enSimple_map = {
		"enSimple_"
		, enSimple_mapRecs
		, (sizeof(enSimple_mapRecs) / sizeof(enSimple_mapRecs[0]))
	};
	struct stSimple_ {
		BOOL b;
		SI32 i;
		UI32 u;
		float f;
		double d;
		char c[5];
		const char* str;
		enum enSimple_ e;
		BYTE bytes[5];
		BYTE* bytes2;
		SI32 bytes2Sz;
		BYTE b1;
		BYTE* b12;
	};
	BOOL b = TRUE;
	SI32 i = -2147483648;
	UI32 u = 4294967295;
	float f = 9.99f;
	double d = 8.88;
	char c[5] = {'a', 'b', 'c', 'd', 'e'};
	BYTE b1 = 5;
	BYTE bbs[] = {12, 34, 2, 6, 78, 3, 45, 6, 0, 1};
	const char* str = "esta es cadena";
	enum enSimple_ e = enSimple_Morning;
	struct stSimple_ s = {
		TRUE
		, -1147483648
		, 2294967295
		, 1.1416f
		, 1.141599
		, {'h', 'o', 'l', 'a', '!'}
		, "jajajaja"
		, enSimple_Night
		, { 1, 2, 3, 4, 5 }
		, bbs
		, (sizeof(bbs) / sizeof(bbs[0]))
		, 0
		, &b1
	};
	struct stEmbededP_ {
		struct stSimple_ s;
		BOOL* bP;
		SI32* iP;
		UI32* uP;
		float* fP;
		double* dP;
		char (*cP)[5];
		const char** strP;
		enum enSimple_* eP;
		struct stSimple_* sP;
	};
	struct stEmbededP_ vEmbededAndNULL = {
		{
			TRUE
			, -1147483648
			, 2294967295
			, 1.1416f
			, 1.141599
			, {'h', 'o', 'l', 'a', '!'}
			, "chacha!"
			, enSimple_Night
		}
		, NULL
		, NULL
		, NULL
		, NULL
		, NULL
		, NULL
		, NULL
		, NULL
		, NULL
	};
	struct stEmbededP_ vEmbeded = {
		{
			TRUE
			, -1147483648
			, 2294967295
			, 1.1416f
			, 1.141599
			, {'h', 'o', 'l', 'a', '!'}
			, "como-como"
			, enSimple_Noon
		}
		, &b
		, &i
		, &u
		, &f
		, &d
		, &c
		, &str
		, &e
		, &s
	};
	BOOL bA[] = { TRUE, FALSE, TRUE };
	SI32 iA[] = { -32768, -22768 };
	UI32 uA[] = { 65535 };
	float fA[] = { 3.1416f, 2.1416f, 1.1416f };
	double dA[] = { 3.1416, 2.1416, 1.1416, 0.1416 };
	char cA[][5] = { {'a', 'd', 'i', 'o', '1'}, {'a', 'd', 'i', 'o', '2'}, {'a', 'd', 'i', 'o', '3'} };
	const char* strA[] = { "cadena1", "cadena2", "cadena3" };
	enum enSimple_ eA[] = { enSimple_Morning, enSimple_Noon, enSimple_Night };
	struct stEmbededP_ sA[] = { vEmbeded, vEmbededAndNULL };
	//
	STNBStructMap stSimple_map, stEmbededP_map;
	NBStructMap_init(&stSimple_map, sizeof(s));
	{
		NBStructMap_addBoolM(&stSimple_map, s, b);
		NBStructMap_addIntM(&stSimple_map, s, i);
		NBStructMap_addUIntM(&stSimple_map, s, u);
		NBStructMap_addFloatM(&stSimple_map, s, f);
		NBStructMap_addDoubleM(&stSimple_map, s, d);
		NBStructMap_addCharsM(&stSimple_map, s, c);
		NBStructMap_addStrPtrM(&stSimple_map, s, str);
		NBStructMap_addEnumM(&stSimple_map, s, e, &enSimple_map);
		NBStructMap_addArrayOfBytesM(&stSimple_map, s, bytes);
		NBStructMap_addPtrToArrayOfBytesM(&stSimple_map, s, bytes2, bytes2Sz, ENNBStructMapSign_Signed);
		NBStructMap_addBytesM(&stSimple_map, s, b1);
		NBStructMap_addBytesPtrM(&stSimple_map, s, b12);
	}
	NBStructMap_init(&stEmbededP_map, sizeof(vEmbeded));
	{
		NBStructMap_addStructM(&stEmbededP_map, vEmbeded, s, &stSimple_map);
		//
		NBStructMap_addBoolPtrM(&stEmbededP_map, vEmbeded, bP);
		NBStructMap_addIntPtrM(&stEmbededP_map, vEmbeded, iP);
		NBStructMap_addUIntPtrM(&stEmbededP_map, vEmbeded, uP);
		NBStructMap_addFloatPtrM(&stEmbededP_map, vEmbeded, fP);
		NBStructMap_addDoublePtrM(&stEmbededP_map, vEmbeded, dP);
		NBStructMap_addCharsPtrM(&stEmbededP_map, vEmbeded, cP);
		NBStructMap_addStrPtrPtrM(&stEmbededP_map, vEmbeded, strP);
		NBStructMap_addEnumPtrM(&stEmbededP_map, vEmbeded, eP, &enSimple_map);
		NBStructMap_addStructPtrM(&stEmbededP_map, vEmbeded, sP, &stSimple_map);
	}
	{
		//Embeded members (void)
		struct test1_ {
			BOOL b[3];
			SI16 i[2];
			UI16 u[1];
			float f[3];
			double d[4];
			char c[3][5];
			const char* str[4];
			enum enSimple_ e[3];
			struct stEmbededP_ s[2];
			//
			BOOL *bAP[3];
			SI32 *iAP[2];
			UI32 *uAP[1];
			float *fAP[3];
			double* dAP[4];
			char (*cAP[3])[5];
			const char** strAP[2];
			enum enSimple_* eAP[3];
			struct stSimple_* sAP[2];
			//
			BOOL* bA; UI32 bASz;
			SI32* iA; UI32 iASz;
			UI32* uA; UI32 uASz;
			float* fA; UI32 fASz;
			double* dA; UI32 dASz;
			char (*cA)[5]; UI32 cASz;
			const char** strA; UI32 strASz;
			enum enSimple_* eA; UI32 eASz;
			struct stEmbededP_* sA; UI32 sASz;
		} test1 = {
			{ TRUE, FALSE, TRUE }
			, { -32768, -22768 }
			, { 65535 }
			, { 3.1416f, 2.1416f, 1.1416f }
			, { 3.1416, 2.1416, 1.1416, 0.1416 }
			, { {'a', 'd', 'i', 'o', '4'}, {'a', 'd', 'i', 'o', '5'}, {'a', 'd', 'i', 'o', '6'} }
			, { "hola", "amigo", "mio", "!"}
			, { enSimple_Morning, enSimple_Noon, enSimple_Night }
			, { vEmbeded, vEmbededAndNULL }
			//
			, { &b, &b, &b }
			, { &i,&i }
			, { &u }
			, { &f, &f, &f }
			, { &d, &d, &d, &d }
			, { &c, &c, &c }
			, { &str, &str }
			, { &e, &e, &e }
			, { &s, &s }
			//
			, bA, sizeof(bA) / sizeof(bA[0])
			, iA, sizeof(iA) / sizeof(iA[0])
			, uA, sizeof(uA) / sizeof(uA[0])
			, fA, sizeof(fA) / sizeof(fA[0])
			, dA, sizeof(dA) / sizeof(dA[0])
			, cA, sizeof(cA) / sizeof(cA[0])
			, strA, sizeof(strA) / sizeof(strA[0])
			, eA, sizeof(eA) / sizeof(eA[0])
			, sA, sizeof(sA) / sizeof(sA[0])
		};
		STNBStructMap test1_map;
		NBStructMap_init(&test1_map, sizeof(test1));
		NBStructMap_addArrayOfBoolM(&test1_map, test1, b);
		NBStructMap_addArrayOfIntM(&test1_map, test1, i);
		NBStructMap_addArrayOfUIntM(&test1_map, test1, u);
		NBStructMap_addArrayOfFloatM(&test1_map, test1, f);
		NBStructMap_addArrayOfDoubleM(&test1_map, test1, d);
		NBStructMap_addArrayOfCharsM(&test1_map, test1, c);
		NBStructMap_addArrayOfStrPtrM(&test1_map, test1, str);
		NBStructMap_addArrayOfEnumM(&test1_map, test1, e, &enSimple_map);
		NBStructMap_addArrayOfStructM(&test1_map, test1, s, &stEmbededP_map);
		//
		NBStructMap_addArrayOfBoolPtrsM(&test1_map, test1, bAP);
		NBStructMap_addArrayOfIntPtrsM(&test1_map, test1, iAP);
		NBStructMap_addArrayOfUIntPtrsM(&test1_map, test1, uAP);
		NBStructMap_addArrayOfFloatPtrsM(&test1_map, test1, fAP);
		NBStructMap_addArrayOfDoublePtrsM(&test1_map, test1, dAP);
		NBStructMap_addArrayOfCharsPtrsM(&test1_map, test1, cAP);
		NBStructMap_addArrayOfStrPtrPtrsM(&test1_map, test1, strAP);
		NBStructMap_addArrayOfEnumPtrsM(&test1_map, test1, eAP, &enSimple_map);
		NBStructMap_addArrayOfStructPtrsM(&test1_map, test1, sAP, &stSimple_map);
		//
		NBStructMap_addPtrToArrayOfBoolM(&test1_map, test1, bA, bASz, ENNBStructMapSign_Unsigned);
		NBStructMap_addPtrToArrayOfIntM(&test1_map, test1, iA, iASz, ENNBStructMapSign_Unsigned);
		NBStructMap_addPtrToArrayOfUIntM(&test1_map, test1, uA, uASz, ENNBStructMapSign_Unsigned);
		NBStructMap_addPtrToArrayOfFloatM(&test1_map, test1, fA, fASz, ENNBStructMapSign_Unsigned);
		NBStructMap_addPtrToArrayOfDoubleM(&test1_map, test1, dA, dASz, ENNBStructMapSign_Unsigned);
		NBStructMap_addPtrToArrayOfCharsM(&test1_map, test1, cA, cASz, ENNBStructMapSign_Unsigned);
		NBStructMap_addPtrToArrayOfStrPtrM(&test1_map, test1, strA, strASz, ENNBStructMapSign_Unsigned);
		NBStructMap_addPtrToArrayOfEnumM(&test1_map, test1, eA, eASz, ENNBStructMapSign_Unsigned, &enSimple_map);
		NBStructMap_addPtrToArrayOfStructM(&test1_map, test1, sA, sASz, ENNBStructMapSign_Unsigned, &stEmbededP_map);
		{
			STNBString str;
			NBString_init(&str);
			if(!NBStruct_stConcatAsJson(&str, &test1_map, &test1, sizeof(test1))){
				NBASSERT(FALSE)
				r = FALSE;
			} else {
				struct test1_ test1cpy;
				NBMemory_setZeroSt(test1cpy, struct test1_);
				if(!NBStruct_stReadFromJsonStr(str.str, str.length, &test1_map, &test1cpy, sizeof(test1cpy))){
					NBASSERT(FALSE)
					r = FALSE;
				} else {
					STNBString str2;
					NBString_init(&str2);
					if(!NBStruct_stConcatAsJson(&str2, &test1_map, &test1cpy, sizeof(test1cpy))){
						NBASSERT(FALSE)
						r = FALSE;
					} else {
						if(!NBString_isEqual(&str, str2.str)){
							PRINTF_ERROR("JSON:\n%s\nvs JSON:\n%s\n", str.str, str2.str);
							NBASSERT(FALSE)
							r = FALSE;
						} else {
							PRINTF_INFO("Test1 json-write-load passed.\n");
						}
					}
					NBString_release(&str2);
				}
				NBMemory_setZeroSt(test1cpy, struct test1_);
				if(!NBStruct_stClone(&test1_map, &test1, sizeof(test1), &test1cpy, sizeof(test1cpy))){
					NBASSERT(FALSE)
					r = FALSE;
				} else {
					STNBString str2;
					NBString_init(&str2);
					if(!NBStruct_stConcatAsJson(&str2, &test1_map, &test1cpy, sizeof(test1cpy))){
						NBASSERT(FALSE)
						r = FALSE;
					} else {
						if(!NBString_isEqual(&str, str2.str)){
							PRINTF_ERROR("JSON:\n%s\nvs JSON:\n%s\n", str.str, str2.str);
							NBASSERT(FALSE)
							r = FALSE;
						} else {
							if(!NBStruct_stRelease(&test1_map, &test1cpy, sizeof(test1cpy))){
								NBASSERT(FALSE)
								r = FALSE;
							} else {
								PRINTF_INFO("JSON:\n%s\n", str2.str);
								PRINTF_INFO("SUCESS, test1 clone-json-write-load passed.\n");
								PRINTF_INFO("End-of-test.\n");
								r = TRUE;
							}
						}
					}
					NBString_release(&str2);
				}
			}
			NBString_release(&str);
		}
		NBStructMap_release(&test1_map);
	}
	NBStructMap_release(&stSimple_map);
	NBStructMap_release(&stEmbededP_map);
	return r;
}
