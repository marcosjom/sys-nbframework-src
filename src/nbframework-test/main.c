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

int main(int argc, const char * argv[]) {
	NBMngrProcess_init();
	NBMngrStructMaps_init();
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
