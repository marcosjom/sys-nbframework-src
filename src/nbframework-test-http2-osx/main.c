//
//  main.c
//  nbframework-test-http2-osx
//
//  Created by Marcos Ortega on 4/23/20.
//

#include "nb/NBFrameworkPch.h"
#include "nb/NBFrameworkDefs.h"
//
#include "nb/core/NBMngrProcess.h"
#include "nb/core/NBMngrStructMaps.h"
#include "nb/net/NBSocket.h"
//
#include <pthread.h>		//for 'pthread_threadid_np()'

int main(int argc, const char * argv[]) {
	PRINTF_INFO("Start-of-main.\n");
	{
		SI32 i; for(i=0; i < argc; i++){
			PRINTF_INFO("argv[%d] = '%s'.\n", i, argv[i]);
		}
	}
	NBMngrProcess_init();
	NBMngrStructMaps_init();
	NBSocket_initEngine();
	NBSocket_initWSA();
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
		
	}
	//
	NBSocket_finishWSA();
	NBSocket_releaseEngine();
	NBMngrStructMaps_release();
	NBMngrProcess_release();
	PRINTF_INFO("End-of-main.\n");
	//
	return 0;
}
