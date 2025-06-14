//
//  NBFilesPkgIndex.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/8/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/core/NBMemory.h"
#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"
#include "nb/core/NBArraySorted.h"
#include "nb/files/NBFilesPkgIndex.h"

#define NB_FILES_PKG_INDEX_VERIF_VALUE		101

typedef struct STNBFilesPkgRecord_ {
	UI16	filepath;
	UI32	dataStart;
	UI32	dataSize;
} STNBFilesPkgRecord;

typedef struct STNBFilesPkgIndexRecord_ {
	UI32				fileId;
	STNBFilesPkgRecord	record;
	STNBString*			strs;
} STNBFilesPkgIndexRecord;

BOOL NBCompare_NBFilesPkgIndexRecord(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
	NBASSERT(dataSz == sizeof(STNBFilesPkgIndexRecord))
	if(dataSz == sizeof(STNBFilesPkgIndexRecord)){
		const STNBFilesPkgIndexRecord* o1 = (const STNBFilesPkgIndexRecord*)data1;
		const STNBFilesPkgIndexRecord* o2 = (const STNBFilesPkgIndexRecord*)data2;
		const char* str1 = &o1->strs->str[o1->record.filepath];
		const char* str2 = &o2->strs->str[o2->record.filepath];
		switch (mode) {
			case ENCompareMode_Equal:
				return NBString_strIsEqual(str1, str2) ? TRUE : FALSE;
			case ENCompareMode_Lower:
				return NBString_strIsLower(str1, str2) ? TRUE : FALSE;
			case ENCompareMode_LowerOrEqual:
				return NBString_strIsLowerOrEqual(str1, str2) ? TRUE : FALSE;
			case ENCompareMode_Greater:
				return NBString_strIsGreater(str1, str2) ? TRUE : FALSE;
			case ENCompareMode_GreaterOrEqual:
				return NBString_strIsGreaterOrEqual(str1, str2) ? TRUE : FALSE;
			default:
				NBASSERT(FALSE)
				break;
		}
	}
	return FALSE;
}

typedef struct STNBFilesPkgIndexBySz_ {
	UI16				strsSz;
	STNBArraySorted		records;	//STNBFilesPkgIndexRecord
} STNBFilesPkgIndexBySz;

BOOL NBCompare_NBFilesPkgIndexBySz(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
	NBASSERT(dataSz == sizeof(STNBFilesPkgIndexBySz))
	if(dataSz == sizeof(STNBFilesPkgIndexBySz)){
		const STNBFilesPkgIndexBySz* o1 = (const STNBFilesPkgIndexBySz*)data1;
		const STNBFilesPkgIndexBySz* o2 = (const STNBFilesPkgIndexBySz*)data2;
		switch (mode) {
			case ENCompareMode_Equal:
				return o1->strsSz == o2->strsSz ? TRUE : FALSE;
			case ENCompareMode_Lower:
				return o1->strsSz < o2->strsSz ? TRUE : FALSE;
			case ENCompareMode_LowerOrEqual:
				return o1->strsSz <= o2->strsSz ? TRUE : FALSE;
			case ENCompareMode_Greater:
				return o1->strsSz > o2->strsSz ? TRUE : FALSE;
			case ENCompareMode_GreaterOrEqual:
				return o1->strsSz >= o2->strsSz ? TRUE : FALSE;
			default:
				NBASSERT(FALSE)
				break;
		}
	}
	return FALSE;
}

typedef struct STNBFilesPkgIndexOpq_ {
	UI32				curFileId;
	STNBArraySorted		recordsBySz;
	STNBString			strs;
	UI32				retainCount;
} STNBFilesPkgIndexOpq;

void NBFilesPkgIndex_init(STNBFilesPkgIndex* obj){
	STNBFilesPkgIndexOpq* opq = obj->opaque = NBMemory_allocType(STNBFilesPkgIndexOpq);
	NBMemory_setZeroSt(*opq, STNBFilesPkgIndexOpq);
	opq->curFileId		= 0;
	NBArraySorted_init(&opq->recordsBySz, sizeof(STNBFilesPkgIndexBySz), NBCompare_NBFilesPkgIndexBySz);
	NBString_init(&opq->strs);
	opq->retainCount	= 1;
}

void NBFilesPkgIndex_retain(STNBFilesPkgIndex* obj){
	STNBFilesPkgIndexOpq* opq = (STNBFilesPkgIndexOpq*)obj->opaque;
	NBASSERT(opq->retainCount > 0)
	opq->retainCount++;
}
	
void NBFilesPkgIndex_release(STNBFilesPkgIndex* obj){
	STNBFilesPkgIndexOpq* opq = (STNBFilesPkgIndexOpq*)obj->opaque;
	NBASSERT(opq->retainCount > 0)
	opq->retainCount--;
	if(opq->retainCount == 0){
		{
			UI32 i; for(i = 0; i < opq->recordsBySz.use; i++){
				STNBFilesPkgIndexBySz* grp = NBArraySorted_itmPtrAtIndex(&opq->recordsBySz, STNBFilesPkgIndexBySz, i);
				NBArraySorted_empty(&grp->records);
				NBArraySorted_release(&grp->records);
			}
			NBArraySorted_empty(&opq->recordsBySz);
			NBArraySorted_release(&opq->recordsBySz);
		}
		NBString_release(&opq->strs);
		NBMemory_free(opq);
		obj->opaque = NULL;
	}
}

//

UI32 NBFilesPkgIndex_getRecordId(STNBFilesPkgIndex* obj, const char* path){
	UI32 r = 0;
	if(path != NULL){
		if(path[0] != '\0'){
			STNBFilesPkgIndexOpq* opq = (STNBFilesPkgIndexOpq*)obj->opaque;
			const UI32 filepathSz	= NBString_strLenBytes(path);
			UI32 i; for(i = 0; i < opq->recordsBySz.use; i++){
				STNBFilesPkgIndexBySz* grp = NBArraySorted_itmPtrAtIndex(&opq->recordsBySz, STNBFilesPkgIndexBySz, i);
				if(filepathSz == grp->strsSz){
					UI32 i; for(i = 0; i < grp->records.use; i++){
						STNBFilesPkgIndexRecord* rc = NBArraySorted_itmPtrAtIndex(&grp->records, STNBFilesPkgIndexRecord, i);
						const char* fpath = &opq->strs.str[rc->record.filepath];
						if(NBString_strIsEqual(fpath, path)){
							r =rc->fileId;
							break;
						}
					}
					break;
				}
			}
		}
	}
	return r;
}

/*BOOL NBFilesPkgIndex_getFolders(STNBFilesPkgIndex* obj, const char* folderpath, STNBString* dstStrs, STNBArray* dstFiles / *STNBFilesystemFile* /){
	BOOL r = FALSE;
	if(folderpath != NULL){
		if(folderpath[0] != '\0'){
 			r = TRUE;
			STNBFilesPkgIndexOpq* opq = (STNBFilesPkgIndexOpq*)obj->opaque;
			const UI32 folderpathSz	= NBString_strLenBytes(folderpath);
			const UI32 folderWithSlashSz = (folderpath[folderpathSz - 1] == '/' ? folderpathSz : folderpathSz + 1);
			UI32 i; for(i = 0; i < opq->recordsBySz.use; i++){
				STNBFilesPkgIndexBySz* grp = NBArraySorted_itmPtrAtIndex(&opq->recordsBySz, STNBFilesPkgIndexBySz, i);
				if(folderWithSlashSz < grp->strsSz){
					UI32 i; for(i = 0; i < grp->records.use; i++){
						STNBFilesPkgIndexRecord* rc = NBArraySorted_itmPtrAtIndex(&grp->records, STNBFilesPkgIndexRecord, i);
						const char* filepath = &opq->strs.str[rc->record.filepath];
						if(NBString_strStartsWith(filepath, folderpath)){
							if(filepath[folderWithSlashSz - 1] == '/'){
								const SI32 nextSlash = NBString_strIndexOf(filepath, "/", folderWithSlashSz);
								if(nextSlash != -1){
									//PRINTF_INFO("Folder: '%s'.\n", &filepath[folderWithSlashSz]);
									if(dstStrs != NULL && dstFiles != NULL){
										STNBFilesystemFile f;
										NBMemory_setZeroSt(f, STNBFilesystemFile);
										f.isSymLink	= FALSE;
										f.name		= dstStrs->length;
 										f.root = ;
										NBString_concatBytes(dstStrs, &filepath[folderWithSlashSz], (nextSlash - folderWithSlashSz));
										NBString_concatByte(dstStrs, '\0');
										NBArray_addValue(dstFiles, f);
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return r;
}*/

BOOL NBFilesPkgIndex_getFiles(STNBFilesPkgIndex* obj, const char* folderpath, const BOOL includeStats, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/){
	BOOL r = FALSE;
	if(folderpath != NULL){
		if(folderpath[0] != '\0'){
			r = TRUE;
			STNBFilesPkgIndexOpq* opq = (STNBFilesPkgIndexOpq*)obj->opaque;
			const UI32 folderpathSz	= NBString_strLenBytes(folderpath);
			const UI32 folderWithSlashSz = (folderpath[folderpathSz - 1] == '/' ? folderpathSz : folderpathSz + 1);
			UI32 i; for(i = 0; i < opq->recordsBySz.use; i++){
				STNBFilesPkgIndexBySz* grp = NBArraySorted_itmPtrAtIndex(&opq->recordsBySz, STNBFilesPkgIndexBySz, i);
				if(folderWithSlashSz < grp->strsSz){
					UI32 i; for(i = 0; i < grp->records.use; i++){
						STNBFilesPkgIndexRecord* rc = NBArraySorted_itmPtrAtIndex(&grp->records, STNBFilesPkgIndexRecord, i);
						const char* filepath = &opq->strs.str[rc->record.filepath];
						if(NBString_strStartsWith(filepath, folderpath)){
							if(filepath[folderWithSlashSz - 1] == '/'){
								if(NBString_strIndexOf(filepath, "/", folderWithSlashSz) == -1){
									//PRINTF_INFO("File: '%s'.\n", &filepath[folderWithSlashSz]);
									if(dstStrs != NULL && dstFiles != NULL){
										STNBFilesystemFile f;
										NBMemory_setZeroSt(f, STNBFilesystemFile);
										f.isSymLink	= FALSE;
										f.name		= dstStrs->length;
										f.root		= ENNBFilesystemRoot_PkgsDir;
										NBString_concat(dstStrs, &filepath[folderWithSlashSz]);
										NBString_concatByte(dstStrs, '\0');
										NBArray_addValue(dstFiles, f);
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return r;
}

BOOL NBFilesPkgIndex_getFileRng(STNBFilesPkgIndex* obj, const char* filepath, UI32* dstStart, UI32* dstSize){
	BOOL r = FALSE;
	if(filepath != NULL){
		if(filepath[0] != '\0'){
			STNBFilesPkgIndexOpq* opq = (STNBFilesPkgIndexOpq*)obj->opaque;
			const UI32 filepathSz	= NBString_strLenBytes(filepath);
			UI32 i; for(i = 0; i < opq->recordsBySz.use; i++){
				STNBFilesPkgIndexBySz* grp = NBArraySorted_itmPtrAtIndex(&opq->recordsBySz, STNBFilesPkgIndexBySz, i);
				if(filepathSz == grp->strsSz){
					UI32 i; for(i = 0; i < grp->records.use; i++){
						STNBFilesPkgIndexRecord* rc = NBArraySorted_itmPtrAtIndex(&grp->records, STNBFilesPkgIndexRecord, i);
						const char* fpath = &opq->strs.str[rc->record.filepath];
						if(NBString_strIsEqual(fpath, filepath)){
							if(dstStart != NULL) *dstStart = rc->record.dataStart;
							if(dstSize != NULL) *dstSize = rc->record.dataSize;
							r = TRUE;
							break;
						}
					}
					break;
				}
			}
		}
	}
	return r;
}

BOOL NBFilesPkgIndex_fileExists(STNBFilesPkgIndex* obj, const char* filepath){
	BOOL r = FALSE;
	if(filepath != NULL){
		if(filepath[0] != '\0'){
			STNBFilesPkgIndexOpq* opq = (STNBFilesPkgIndexOpq*)obj->opaque;
			const UI32 filepathSz	= NBString_strLenBytes(filepath);
			UI32 i; for(i = 0; i < opq->recordsBySz.use; i++){
				STNBFilesPkgIndexBySz* grp = NBArraySorted_itmPtrAtIndex(&opq->recordsBySz, STNBFilesPkgIndexBySz, i);
				if(filepathSz == grp->strsSz){
					UI32 i; for(i = 0; i < grp->records.use; i++){
						STNBFilesPkgIndexRecord* rc = NBArraySorted_itmPtrAtIndex(&grp->records, STNBFilesPkgIndexRecord, i);
						const char* fpath = &opq->strs.str[rc->record.filepath];
						if(NBString_strIsEqual(fpath, filepath)){
							r = TRUE;
							break;
						}
					}
					break;
				}
			}
		}
	}
	return r;
}

//

BOOL NBFilesPkgIndex_addFile(STNBFilesPkgIndex* obj, const char* filepath, const UI32 start, const UI32 size){
	BOOL r = FALSE;
	if(filepath != NULL){
		if(filepath[0] != '\0'){
			STNBFilesPkgIndexOpq* opq = (STNBFilesPkgIndexOpq*)obj->opaque;
			const UI32 strsSz = NBString_strLenBytes(filepath);
			//Search group
			STNBFilesPkgIndexBySz srch;
			srch.strsSz	= (UI16)strsSz;
			SI32 iGrp = NBArraySorted_indexOf(&opq->recordsBySz, &srch, sizeof(srch), NULL);
			if(iGrp == -1){
				//Create group
				NBArraySorted_init(&srch.records, sizeof(STNBFilesPkgIndexRecord), NBCompare_NBFilesPkgIndexRecord);
				NBArraySorted_add(&opq->recordsBySz, &srch, sizeof(srch));
				iGrp = NBArraySorted_indexOf(&opq->recordsBySz, &srch, sizeof(srch), NULL);
				NBASSERT(iGrp >= 0)
			}
			STNBFilesPkgIndexBySz* grp = NBArraySorted_itmPtrAtIndex(&opq->recordsBySz, STNBFilesPkgIndexBySz, iGrp);
			NBASSERT(grp->strsSz == strsSz)
			//Search file
			
			/*typedef struct STNBFilesPkgIndexRecord_ {
				UI32				fileId;
				STNBFilesPkgRecord	record;
				STNBString*			strs;
			} STNBFilesPkgIndexRecord;*/
			STNBString srchStr;
			NBString_initWithStr(&srchStr, filepath);
			STNBFilesPkgIndexRecord srch2;
			srch2.record.filepath = 0;
			srch2.strs = &srchStr;
			const SI32 iFile = NBArraySorted_indexOf(&grp->records, &srch2, sizeof(srch2), NULL);
			if(iFile == -1){
				srch2.fileId			= ++opq->curFileId;
				srch2.record.filepath	= (UI16)opq->strs.length;
				srch2.record.dataStart	= start;
				srch2.record.dataSize	= size;
				srch2.strs				= &opq->strs;
				NBString_concat(&opq->strs, filepath);
				NBString_concatByte(&opq->strs, '\0');
				NBArraySorted_add(&grp->records, &srch2, sizeof(srch2));
				r = TRUE;
			}
			NBString_release(&srchStr);
		}
	}
	return r;
}

//File

BOOL NBFilesPkgIndex_writeToFile(const STNBFilesPkgIndex* obj, STNBFileRef file){
	BOOL r = TRUE;
	STNBFilesPkgIndexOpq* opq = (STNBFilesPkgIndexOpq*)obj->opaque;
	UI32 verif = NB_FILES_PKG_INDEX_VERIF_VALUE;
	NBFile_write(file, &verif, sizeof(verif));
	//Strings
	{
		const SI32 strsSz = opq->strs.length;
		NBFile_write(file, &strsSz, sizeof(strsSz));
		NBFile_write(file, opq->strs.str, strsSz);
		NBFile_write(file, &verif, sizeof(verif));
	}
	//Groups
	{
		const SI32 grpsCount = opq->recordsBySz.use;
		NBFile_write(file, &grpsCount, sizeof(grpsCount));
		SI32 iGrp;
		for(iGrp = 0; iGrp < grpsCount; iGrp++){
			STNBFilesPkgIndexBySz* grp = NBArraySorted_itmPtrAtIndex(&opq->recordsBySz, STNBFilesPkgIndexBySz, iGrp);
			UI16 strsSz		= grp->strsSz;
			NBFile_write(file, &strsSz, sizeof(strsSz));
			//Registros de grupo
			SI32 rcsCount	= grp->records.use;
			NBFile_write(file, &rcsCount, sizeof(rcsCount));
			SI32 iGrp;
			for(iGrp=0; iGrp < rcsCount; iGrp++){
				//Only save the base value
				const STNBFilesPkgIndexRecord* rc = NBArraySorted_itmPtrAtIndex(&grp->records, STNBFilesPkgIndexRecord, iGrp);
				NBFile_write(file, &rc->record, sizeof(rc->record));
			}
			NBFile_write(file, &verif, sizeof(verif));
		}
	}
	NBFile_write(file, &verif, sizeof(verif));
	return r;
}

BOOL NBFilesPkgIndex_loadFromFile(STNBFilesPkgIndex* obj, STNBFileRef file){
	BOOL r = FALSE;
	STNBFilesPkgIndexOpq* opq = (STNBFilesPkgIndexOpq*)obj->opaque;
	UI32 verif = 0; NBFile_read(file, &verif, sizeof(verif));
	if(verif != NB_FILES_PKG_INDEX_VERIF_VALUE){
		NBASSERT(FALSE);
	} else {
		//Read strs
		SI32 strsSz = -1; NBFile_read(file, &strsSz, sizeof(strsSz));
		if(strsSz < 0){
			NBASSERT(FALSE);
		} else {
			if(strsSz == 0){
				NBString_concatByte(&opq->strs, '\0');
			} else {
				char* buff = NBMemory_allocTypes(char, strsSz);
				NBFile_read(file, buff, strsSz);
				NBString_concatBytes(&opq->strs, buff, strsSz);
				NBMemory_free(buff);
			}
            verif = 0; NBFile_read(file, &verif, sizeof(verif));
			//Groups
			if(verif != NB_FILES_PKG_INDEX_VERIF_VALUE){
				NBASSERT(FALSE);
			} else {
				SI32 grpsCount = -1; NBFile_read(file, &grpsCount, sizeof(grpsCount));
				if(grpsCount < 0){
					NBASSERT(FALSE);
				} else {
					BOOL lgcErr = FALSE;
					SI32 iGrp;
					for(iGrp = 0; iGrp < grpsCount && !lgcErr; iGrp++){
						UI16 grpStrsSz = 0; NBFile_read(file, &grpStrsSz, sizeof(grpStrsSz));
						STNBFilesPkgIndexBySz grp;
						grp.strsSz	= grpStrsSz;
						NBArraySorted_init(&grp.records, sizeof(STNBFilesPkgIndexRecord), NBCompare_NBFilesPkgIndexRecord);
						//Registros de grupo
						SI32 ammRecs = -1; NBFile_read(file, &ammRecs, sizeof(ammRecs));
						if(ammRecs < 0){
							lgcErr = TRUE;
							NBASSERT(FALSE);
						} else {
							SI32 iRec;
							for(iRec = 0; iRec < ammRecs && !lgcErr; iRec++){
								STNBFilesPkgIndexRecord rc;
								NBFile_read(file, &rc.record, sizeof(rc.record));
								if(rc.record.filepath < 0 || rc.record.filepath >= strsSz){
									lgcErr = TRUE;
									NBASSERT(FALSE);
								} else {
									//PRINTF_INFO("File loaded: '%s'.\n", &opq->strs.str[rc.record.filepath]);
									rc.fileId	= ++opq->curFileId;
									rc.strs		= &opq->strs;
									NBArraySorted_add(&grp.records, &rc, sizeof(rc));
								}
							}
                            verif = 0; NBFile_read(file, &verif, sizeof(verif));
							if(verif != NB_FILES_PKG_INDEX_VERIF_VALUE){
								lgcErr = TRUE;
								NBASSERT(FALSE);
							}
						}
						NBArraySorted_add(&opq->recordsBySz, &grp, sizeof(grp));
					}
					//Valor de verificacion final
                    verif = 0; NBFile_read(file, &verif, sizeof(verif));
					if(verif != NB_FILES_PKG_INDEX_VERIF_VALUE){
						lgcErr = TRUE;
						NBASSERT(FALSE);
					}
					r = !lgcErr;
				}
			}
		}
	}
	return r;
}
