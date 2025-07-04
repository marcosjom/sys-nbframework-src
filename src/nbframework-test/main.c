//
//  main.c
//  nbframework-test-osx
//
//  Created by Marcos Ortega on 3/3/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/NBFrameworkDefs.h"
//
#include "nb/core/NBMngrProcess.h"
#include "nb/core/NBMngrStructMaps.h"
#include "nb/core/NBStruct.h"
#include "nb/net/NBSocket.h"
//
#include <stdlib.h>	//for rand()
#include <time.h>	//for rand()
#include <sys/types.h> //for getid()
#ifdef _WIN32
	//
#else
#	include <pthread.h>	//for 'pthread_threadid_np()'
#endif

#ifndef _WIN32
/*void intHandler(int sig){
	if(sig == SIGINT){ //SIGQUIT
		PRINTF_INFO("-------------------------------------.\n");
		PRINTF_INFO("Captured signal SIGINT, clean-exit flag set (please wait ...).\n");
		PRINTF_INFO("-------------------------------------.\n");
		//NBHttpProxy_stopFlag(&srvc);
	} else if(sig == SIGQUIT){
		PRINTF_INFO("-------------------------------------.\n");
		PRINTF_INFO("Captured signal SIGQUIT, clean-exit flag set (please wait ...).\n");
		PRINTF_INFO("-------------------------------------.\n");
		//NBHttpProxy_stopFlag(&srvc);
	}
}*/
#endif

//Json parsing
#include "nb/core/NBMemory.h"
#include "nb/core/NBJsonParser.h"
#include "nb/files/NBFilesystem.h"

void text_json_parseFile_(const char* filepath, const char* name, const BOOL printBasic, const BOOL printParsedStruct);
void text_json_parseFiles_(STNBFilesystem* fs, const char* root, const BOOL printBasic, const BOOL printParsedStruct);

//Memory Block
#include "nb/core/NBMemoryBlock.h"

void memBlock_testsRun_(const UI32 ammTests);
void memblock_testRandomActions_(const UI32 blockSz, const UI32 ammPtrsMax, const UI32 ammActions);

//Memory Blocks
#include "nb/core/NBMemoryBlocks.h"

void memBlocks_testsRun_(const UI32 ammTests);
void memblocks_testRandomActions_(const UI32 blockSz, const UI32 ammPtrsMax, const UI32 ammActions);

//main

int main(int argc, const char * argv[]) {
    //These could be automatically called by 'AUFrameworkBaseInicializar'
    if(!NBMngrProcess_isInited()){
        NBMngrProcess_init();
        NBMngrStructMaps_init();
    }
	NBSocket_initEngine();
	NBSocket_initWSA();
	//
	PRINTF_INFO("Start-of-main.\n");
	{
		SI32 i; for(i=0; i < argc; i++){
			PRINTF_INFO("argv[%d] = '%s'.\n", i, argv[i]);
		}
	}
	//start random
	{
#		ifdef _WIN32
		{
			srand((unsigned int)time(NULL)); //start-randomizer
		}
#		else
		{
			time_t tm; uint64_t tid;
			tm = time(NULL);
			pthread_threadid_np(NULL, &tid);
			srand((unsigned int)(((uint64_t)tm + tid) % 0xFFFFFFFF)); //start-randomizer
		}
#		endif
	}
    //memblock test
    {
        PRINTF_INFO("Start-of-memBlock_testsRun_.\n");
        memBlock_testsRun_(100);
        PRINTF_INFO("End-of-memBlock_testsRun_.\n");
    }
    //memblocks test
    {
        PRINTF_INFO("Start-of-memBlocks_testsRun_.\n");
        memBlocks_testsRun_(100);
        PRINTF_INFO("End-of-memBlocks_testsRun_.\n");
    }
	//
	{
		STNBFilesystem fs;
		NBFilesystem_init(&fs);
		{
			const BOOL printBasic = TRUE;
			const BOOL printParsedStruct = FALSE;
			text_json_parseFiles_(&fs, "/Users/mortegam/Downloads/JSONTestSuite-master/test_parsing/", printBasic, printParsedStruct);
			text_json_parseFiles_(&fs, "/Users/mortegam/Downloads/JSONTestSuite-master/test_transform/", printBasic, printParsedStruct);
			//text_json_parseFile_("/Users/mortegam/Downloads/JSONTestSuite-master/test_parsing/y_object_long_strings.json", "y_object_long_strings.json", printBasic);
		}
		NBFilesystem_release(&fs);
	}
	//
	NBSocket_finishWSA();
	NBSocket_releaseEngine();
	NBMngrStructMaps_release();
	NBMngrProcess_release();
	printf("End-of-main.\n");
	return 0;
}


void text_json_parseFile_(const char* filepath, const char* name, const BOOL printBasic, const BOOL printParsedStruct){
	STNBJsonParser parser;
	NBJsonParser_init(&parser);
	NBJsonParser_feedStart(&parser, NULL, NULL);
	{
		STNBFileRef ff = NBFile_alloc(NULL);
		if(!NBFile_open(ff, filepath, ENNBFileMode_Read)){
			PRINTF_CONSOLE_ERROR("text_json_parseFiles, NBFile_open failed for '%s'.\n", filepath);
		} else {
			//PRINTF_INFO("text_json_parseFiles, testing file '%s'.\n", name);
			NBFile_lock(ff);
			{
				BOOL success = FALSE;
				char buff[4096];
				SI32 read = -1;
				while((read = NBFile_read(ff, buff, sizeof(buff) - 1)) > 0){
					buff[read] = '\0';
					if(!NBJsonParser_feed(&parser, buff, read, NULL, NULL)){
						//PRINTF_CONSOLE_ERROR("text_json_parseFiles, NBJsonParser_feed failed for '%s'.\n", name);
						break;
					}
				}
				if(!NBJsonParser_feedEnd(&parser, NULL, NULL)){
					//PRINTF_CONSOLE_ERROR("text_json_parseFiles, NBJsonParser_feedEnd failed for '%s'.\n", name);
				} else if(!NBJsonParser_feedIsComplete(&parser)){
					//PRINTF_CONSOLE_ERROR("text_json_parseFiles, NBJsonParser_feedIsComplete failed for '%s'.\n", name);
				} else {
					success = TRUE;
					//PRINTF_INFO("text_json_parseFiles, file parsed '%s'.\n", name);
				}
				//result
				if(NBString_strStartsWith(name, "y_")){
					if(!success){
						PRINTF_CONSOLE_ERROR("text_json_parseFiles, failed but should succed: '%s'.\n", name);
						if(parser.errDesc.length > 0){
							PRINTF_CONSOLE_ERROR("text_json_parseFiles, error: '%s'.\n", parser.errDesc.str);
						}
					} else if(printBasic){
						PRINTF_CONSOLE("text_json_parseFiles, OK-succed: '%s'.\n", name);
					}
				} else if(NBString_strStartsWith(name, "n_")){
					if(success){
						PRINTF_CONSOLE_ERROR("text_json_parseFiles, succed but should fail: '%s'.\n", name);
					} else if(printBasic){
						PRINTF_CONSOLE("text_json_parseFiles, OK-failed: '%s'.\n", name);
					}
				} else if(NBString_strStartsWith(name, "i_")){
					if(printBasic){
						PRINTF_CONSOLE("text_json_parseFiles, OK-%s as imp-dependant: '%s'.\n", (success ? "succed" : "failed"), name);
					}
				} else if(printBasic){
					PRINTF_CONSOLE("text_json_parseFiles, %s: '%s'.\n", (success ? "succed" : "failed"), name);
				}
				//load
				if(success){
					STNBJson json;
					NBJson_init(&json);
					//reset file-pos
					NBFile_seek(ff, 0, ENNBFileRelative_Start);
					//load
					if(!NBJson_loadFromFile(&json, ff)){
						PRINTF_CONSOLE_ERROR("text_json_parseFiles, NBJson_loadFromFile failed for: '%s'.\n", name);
					} else {
						//PRINTF_CONSOLE("text_json_parseFiles, NBJson_loadFromFile success for: '%s'.\n", name);
						if(printParsedStruct){
							STNBString strTmp;
							NBString_initWithSz(&strTmp, 1024, 1024, 0.10f);
							NBJson_concat(&json, '\t', &strTmp);
							PRINTF_CONSOLE("text_json_parseFiles, parsed-json: %s<--\n", strTmp.str);
							NBString_release(&strTmp);
						}
					}
					NBJson_release(&json);
				}
			}
			NBFile_unlock(ff);
		}
		NBFile_release(&ff);
	}
	NBJsonParser_release(&parser);
}
	
void text_json_parseFiles_(STNBFilesystem* fs, const char* root, const BOOL printBasic, const BOOL printParsedStruct){
	STNBArray files; STNBString strs, path;
	NBArray_init(&files, sizeof(STNBFilesystemFile), NULL);
	NBString_init(&strs);
	NBString_init(&path);
	if(!NBFilesystem_getFiles(fs, root, FALSE, &strs, &files)){
		PRINTF_CONSOLE_ERROR("text_json_parseFiles, NBFilesystem_getFiles failed for '%s'.\n", root);
	} else {
		SI32 i; for(i = 0; i < files.use; i++){
			const STNBFilesystemFile* f = NBArray_itmPtrAtIndex(&files, STNBFilesystemFile, i);
			const char* name = &strs.str[f->name];
			if(NBString_strEndsWith(name, ".json") || NBString_strEndsWith(name, ".Json") || NBString_strEndsWith(name, ".JSON")){
				NBString_set(&path, root);
				if(path.length > 0 && path.str[path.length - 1] != '\\' && path.str[path.length - 1] != '/'){
					NBString_concatByte(&path, '/');
				}
				NBString_concat(&path, name);
				text_json_parseFile_(path.str, name, printBasic, printParsedStruct);
			}
		}
	}
	NBArray_release(&files);
	NBString_release(&strs);
	NBString_release(&path);
}

//Memory Block

void memBlock_testsRun_(const UI32 ammTests){
    UI32 i; for(i = 0; i < ammTests; i++){
        memblock_testRandomActions_(rand() % 1024 * 128, 1 + (rand() % 1024), 1 + (rand() % 1024));
        if(((i + 1) % 10) == 0){
            PRINTF_INFO("#%d/%d runs-of-memblock_testRandomActions_ (%.1f%%).\n", i + 1, ammTests, (float)i / (float)ammTests * 100.f);
        }
    }
    PRINTF_INFO("%d/%d runs-of-memblock_testRandomActions_ (%.1f%%, END).\n", i, ammTests, (float)i / (float)ammTests * 100.f);
}

void memblock_testRandomActions_(const UI32 blockSz, const UI32 ammPtrsMax, const UI32 ammActions){
    STNBAbsPtr* ptrs = (STNBAbsPtr*)NBMemory_alloc(sizeof(STNBAbsPtr) * ammPtrsMax);
    NBMemory_set(ptrs, 0, sizeof(STNBAbsPtr) * ammPtrsMax);
    //
    {
        STNBMemoryBlockRef mb = NBMemoryBlock_alloc(NULL);
        STNBMemoryBlockCfg cfg = STNBMemoryBlockCfg_Zero;
        cfg.size            = blockSz;
        cfg.idxsAlign       = (rand() % 17);
        cfg.sizeAlign       = (rand() % 257);
        cfg.idxZeroIsValid  = (rand() % 2);
        NBMemoryBlock_prepare(mb, &cfg, NULL);
        {
            //random actions
            UI32 i; for(i = 0; i < ammActions; i++){
                const UI32 iAct = (rand() % 128);
                if(iAct < 124){
                    //random action
                    const UI32 iPtr = (rand() % ammPtrsMax);
                    if(ptrs[iPtr].ptr != NULL){
                        if(!NBMemoryBlock_mfree(mb, ptrs[iPtr])){
                            NBASSERT(FALSE); //pointer should be freed
                            break;
                        } else {
                            ptrs[iPtr].ptr = NULL;
                            NBASSERT(NBMemoryBlock_validateIndex(mb))
                        }
                    } else {
                        ptrs[iPtr] = NBMemoryBlock_malloc(mb, 1 + (rand() % (1 + blockSz)));
                        NBASSERT(NBMemoryBlock_validateIndex(mb))
                    }
                } else if(iAct < 126){
                    //remove all individually
                    UI32 availSz = 0;
                    UI32 i2; for(i2 = 0; i2 < ammPtrsMax && (availSz = NBMemoryBlock_mAvailSz(mb)) > 0; i2++){
                        if(ptrs[i2].ptr != NULL){
                            if(!NBMemoryBlock_mfree(mb, ptrs[i2])){
                                NBASSERT(FALSE);
                                break;
                            } else {
                                NBASSERT(NBMemoryBlock_validateIndex(mb))
                                ptrs[i2].ptr = NULL;
                            }
                        }
                    }
                } else if(iAct < 127){
                    //clear
                    NBMemoryBlock_clear(mb);
                    NBMemory_set(ptrs, 0, sizeof(STNBAbsPtr) * ammPtrsMax);
                    NBASSERT(NBMemoryBlock_validateIndex(mb))
                } else {
                    //fill all
                    STNBTimestampMicro end, start = NBTimestampMicro_getMonotonicFast();
                    UI32 availSzStart = NBMemoryBlock_mAvailSz(mb);
                    UI32 availSz = availSzStart;
                    //PRINTF_INFO("memblock_testRandomActions_ filling all memory (START, %d bytes).\n", availSzStart);
                    //
                    NBMemoryBlock_prepareForNewMallocsActions(mb, availSz);
                    //
                    UI32 i2; for(i2 = 0; i2 < ammPtrsMax && (availSz = NBMemoryBlock_mAvailSz(mb)) > 0; i2++){
                        if(ptrs[i2].ptr == NULL){
                            ptrs[i2] = NBMemoryBlock_malloc(mb, 1);
                            NBASSERT(NBMemoryBlock_validateIndex(mb))
                            if(ptrs[i2].ptr == NULL){ //this happens when the block is too fragmented
                                break;
                            }
                        }
                    }
                    while((availSz = NBMemoryBlock_mAvailSz(mb)) > 0){
                        STNBAbsPtr ptr = NBMemoryBlock_malloc(mb, 1);
                        NBASSERT(NBMemoryBlock_validateIndex(mb))
                        if(ptr.ptr == NULL){ //this happens when the block is too fragmented
                            break;
                        }
                    }
                    end = NBTimestampMicro_getMonotonicFast();
                    //PRINTF_INFO("memblock_testRandomActions_ filling all memory (DONE, %d bytes, %d ms).\n", availSzStart, NBTimestampMicro_getDiffInMs(&start, &end));
                }
            }
        }
        NBMemoryBlock_release(&mb);
    }
    //
    NBMemory_free(ptrs);
    ptrs = NULL;
}

//Memory Blocks

void memBlocks_testsRun_(const UI32 ammTests){
    UI32 i; for(i = 0; i < ammTests; i++){
        memblocks_testRandomActions_(rand() % 1024 * 128, 1 + (rand() % 1024), 1 + (rand() % 1024));
        if(((i + 1) % 10) == 0){
            PRINTF_INFO("#%d/%d runs-of-memblocks_testRandomActions_ (%.1f%%).\n", i + 1, ammTests, (float)i / (float)ammTests * 100.f);
        }
    }
    PRINTF_INFO("%d/%d runs-of-memblocks_testRandomActions_ (%.1f%%, END).\n", i, ammTests, (float)i / (float)ammTests * 100.f);
}

void memblocks_testRandomActions_(const UI32 blockSz, const UI32 ammPtrsMax, const UI32 ammActions){
    STNBAbsPtr* ptrs = (STNBAbsPtr*)NBMemory_alloc(sizeof(STNBAbsPtr) * ammPtrsMax);
    NBMemory_set(ptrs, 0, sizeof(STNBAbsPtr) * ammPtrsMax);
    //
    {
        STNBMemoryBlocksRef mb = NBMemoryBlocks_alloc(NULL);
        STNBMemoryBlocksCfg cfg = STNBMemoryBlocksCfg_Zero;
        cfg.sizePerBlock    = blockSz;
        cfg.sizeMax         = 0; //
        cfg.idxsAlign       = (rand() % 17);
        cfg.sizeAlign       = (rand() % 257);
        cfg.idxZeroIsValid  = (rand() % 2);
        if(blockSz > 0){
            cfg.sizeInitial  = (rand() % 2) == 0 ? 0 : (rand() % blockSz) * (rand() % 3);
        }
        NBMemoryBlocks_prepare(mb, &cfg, NULL);
        {
            //random actions
            UI32 i; for(i = 0; i < ammActions; i++){
                const UI32 iAct = (rand() % 128);
                if(iAct < 126){
                    //random action
                    const UI32 iPtr = (rand() % ammPtrsMax);
                    if(ptrs[iPtr].ptr != NULL){
                        if(!NBMemoryBlocks_mfree(mb, ptrs[iPtr])){
                            NBASSERT(FALSE); //pointer should be freed
                            break;
                        } else {
                            ptrs[iPtr].ptr = NULL;
                            NBASSERT(NBMemoryBlocks_validateIndex(mb))
                        }
                    } else {
                        ptrs[iPtr] = NBMemoryBlocks_malloc(mb, 1 + (rand() % (1 + blockSz)), NULL);
                        NBASSERT(ptrs[iPtr].ptr != NULL) //should be allocated
                        NBASSERT(NBMemoryBlocks_validateIndex(mb))
                    }
                } else if(iAct < 127){
                    //remove all individually
                    UI32 i2; for(i2 = 0; i2 < ammPtrsMax; i2++){
                        if(ptrs[i2].ptr != NULL){
                            if(!NBMemoryBlocks_mfree(mb, ptrs[i2])){
                                NBASSERT(FALSE);
                                break;
                            } else {
                                NBASSERT(NBMemoryBlocks_validateIndex(mb))
                                ptrs[i2].ptr = NULL;
                            }
                        }
                    }
                } else {
                    //clear
                    NBMemoryBlocks_clear(mb);
                    NBMemory_set(ptrs, 0, sizeof(STNBAbsPtr) * ammPtrsMax);
                    NBASSERT(NBMemoryBlocks_validateIndex(mb))
                }
            }
        }
        NBMemoryBlocks_release(&mb);
    }
    //
    NBMemory_free(ptrs);
    ptrs = NULL;
}
