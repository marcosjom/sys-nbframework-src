//
//  NBZlib.c
//  nbframework
//
//  Created by Marcos Ortega on 19/3/19.
//

#include "nb/NBFrameworkPch.h"
#include "nb/compress/NBZlib.h"
#include "nb/compress/NBZInflate.h"
#include "nb/compress/NBZDeflate.h"

//Read

BOOL NBZlib_inflateToStr(const void* pData, const UI32 dataSz, STNBString* dst){
	BOOL r = FALSE;
	const BYTE* data = (const BYTE*)pData;
	STNBZInflate z;
	NBZInflate_init(&z);
	if(NBZInflate_feedStart(&z)){
		ENZInfResult zr;
		NBMemory_setZeroSt(zr, ENZInfResult);
		{
			BYTE buff2[4096];
			UI32 iPos = 0;
			//Consume until no new output
			do {
				zr = NBZInflate_feed(&z, buff2, sizeof(buff2), &data[iPos], (dataSz - iPos), (iPos < dataSz ? ENZInfBlckType_Partial : ENZInfBlckType_Final));
				if(zr.outBytesProcessed > 0 && dst != NULL){
					NBASSERT(zr.outBytesProcessed <= sizeof(buff2))
					NBString_concatBytes(dst, (const char*)buff2, zr.outBytesProcessed);
				}
				iPos += zr.inBytesProcessed;
			} while(zr.resultCode >= 0 && zr.inBytesAvailable > 0 && zr.outBytesProcessed > 0);
			//Final result
			NBASSERT(iPos <= dataSz)
			if(zr.resultCode == 1 && iPos == dataSz){ //Z_STREAM_END
				r = TRUE;
			}
		}
	}
	NBZInflate_release(&z);
	return r;
}

BOOL NBZlib_inflateFromFile(STNBFileRef file, STNBString* dst){
	BOOL r = FALSE;
	STNBZInflate z;
	NBZInflate_init(&z);
	if(NBZInflate_feedStart(&z)){
		ENZInfResult zr;
		NBMemory_setZeroSt(zr, ENZInfResult);
		{
			BYTE buff[4096];
			BYTE buff2[4096];
			SI32 read, iPos = 0, fillSz = 0;
			do {
				read = NBFile_read(file, &buff[iPos], sizeof(buff) - iPos);
				if(read >= 0){
					fillSz	= iPos + read;
					iPos	= 0;
					//Consume until no new output
					do {
						zr = NBZInflate_feed(&z, buff2, sizeof(buff2), &buff[iPos], (fillSz - iPos), ENZInfBlckType_Partial);
						if(zr.outBytesProcessed > 0 && dst != NULL){
							NBASSERT(zr.outBytesProcessed <= sizeof(buff2))
							NBString_concatBytes(dst, (const char*)buff2, zr.outBytesProcessed);
						}
						iPos += zr.inBytesProcessed;
					} while(zr.resultCode >= 0 && zr.inBytesAvailable > 0 && zr.outBytesProcessed > 0);
					//Move remainig bytes to the left
					{
						NBASSERT(iPos <= fillSz)
						if(iPos < fillSz){
							NBMemory_copy(buff, &buff[iPos], (fillSz - iPos));
						}
						iPos = 0;
					}
				}
				//End of stream
				if(read <= 0){
					break;
				}
			} while(zr.resultCode >= 0); // < 0 means error
			//Final result
			if(zr.resultCode == 1 && iPos == 0){ //Z_STREAM_END
				r = TRUE;
			}
		}
	}
	NBZInflate_release(&z);
	return r;
}

BOOL NBZlib_inflateFromPath(const char* filepath, STNBString* dst){
	BOOL r = FALSE;
	STNBFileRef file = NBFile_alloc(NULL);
	if(NBFile_open(file, filepath, ENNBFileMode_Read)){
		NBFile_lock(file);
		r = NBZlib_inflateFromFile(file, dst);
		NBFile_unlock(file);
		NBFile_close(file);
	}
	NBFile_release(&file);
	return r;
}

BOOL NBZlib_inflateFromPathAtRoot(STNBFilesystem* fs, const ENNBFilesystemRoot root, const char* filepath, STNBString* dst){
	BOOL r = FALSE;
	STNBFileRef file = NBFile_alloc(NULL);
	if(NBFilesystem_openAtRoot(fs, root, filepath, ENNBFileMode_Read, file)){
		NBFile_lock(file);
		r = NBZlib_inflateFromFile(file, dst);
		NBFile_unlock(file);
		NBFile_close(file);
	}
	NBFile_release(&file);
	return r;
}

//Write

BOOL NBZlib_deflateToStr(STNBString* dst, const void* pData, const UI32 dataSz, const SI8 compLv9){
	BOOL r = FALSE;
	const BYTE* data = (const BYTE*)pData;
	STNBZDeflate z;
	NBZDeflate_init(&z);
	if(NBZDeflate_feedStart(&z, compLv9)){
		ENZDefResult zr;
		NBMemory_setZeroSt(zr, ENZDefResult);
		{
			BYTE buff2[4096];
			UI32 iPos = 0;
			do {
				zr = NBZDeflate_feed(&z, buff2, sizeof(buff2), &data[iPos], (dataSz - iPos), (iPos < dataSz ? ENZDefBlckType_Partial : ENZDefBlckType_Final));
				if(zr.outBytesProcessed > 0 && dst != NULL){
					NBASSERT(zr.outBytesProcessed <= sizeof(buff2))
					NBString_concatBytes(dst, (const char*)buff2, zr.outBytesProcessed);
				}
				iPos += zr.inBytesProcessed;
			} while(zr.resultCode >= 0 && zr.outBytesProcessed > 0);
			//Final result
			if(zr.resultCode == 1 && iPos == dataSz){ //Z_STREAM_END
				r = TRUE;
			}
		}
	}
	NBZDeflate_release(&z);
	return r;
}

BOOL NBZlib_deflateToFile(STNBFileRef file, const void* pData, const UI32 dataSz, const SI8 compLv9){
	BOOL r = FALSE;
	const BYTE* data = (const BYTE*)pData;
	STNBZDeflate z;
	NBZDeflate_init(&z);
	if(NBZDeflate_feedStart(&z, compLv9)){
		ENZDefResult zr;
		NBMemory_setZeroSt(zr, ENZDefResult);
		{
			BYTE buff2[4096];
			UI32 iPos = 0;
			do {
				zr = NBZDeflate_feed(&z, buff2, sizeof(buff2), &data[iPos], (dataSz - iPos), (iPos < dataSz ? ENZDefBlckType_Partial : ENZDefBlckType_Final));
				if(zr.outBytesProcessed > 0 && NBFile_isSet(file)){
					NBASSERT(zr.outBytesProcessed <= sizeof(buff2))
					if(NBFile_write(file, buff2, zr.outBytesProcessed) != zr.outBytesProcessed){
						PRINTF_ERROR("NBZlib_deflateToFile, NBFile_write failed.\n");
						break;
					}
				}
				iPos += zr.inBytesProcessed;
			} while(zr.resultCode >= 0 && zr.outBytesProcessed > 0);
			//Final result
			if(zr.resultCode == 1 && iPos == dataSz){ //Z_STREAM_END
				r = TRUE;
			}
		}
	}
	NBZDeflate_release(&z);
	return r;
}

BOOL NBZlib_deflateToPath(const char* filePath, const void* data, const UI32 dataSz, const SI8 compLv9){
	BOOL r = FALSE;
	STNBFileRef file = NBFile_alloc(NULL);
	if(NBFile_open(file, filePath, ENNBFileMode_Write)){
		NBFile_lock(file);
		r = NBZlib_deflateToFile(file, data, dataSz, compLv9);
		NBFile_unlock(file);
		NBFile_close(file);
	}
	NBFile_release(&file);
	return r;
}

BOOL NBZlib_deflateToPathAtRoot(STNBFilesystem* fs, const ENNBFilesystemRoot root, const char* filePath, const void* data, const UI32 dataSz, const SI8 compLv9){
	BOOL r = FALSE;
	STNBFileRef file = NBFile_alloc(NULL);
	if(NBFilesystem_openAtRoot(fs, root, filePath, ENNBFileMode_Write, file)){
		NBFile_lock(file);
		r = NBZlib_deflateToFile(file, data, dataSz, compLv9);
		NBFile_unlock(file);
		NBFile_close(file);
	}
	NBFile_release(&file);
	return r;
}
