//
// Prefix header for all source files
//

#ifndef NBFrameworkPch_h
#define NBFrameworkPch_h

#include "NBFrameworkCfg.h"

//Memory leak detection
#ifdef NB_MEM_LEAK_DETECT_ENABLED
#	ifdef _WIN32
		//Windows memory leak detection
		//https://docs.microsoft.com/en-us/visualstudio/debugger/finding-memory-leaks-using-the-crt-library?view=vs-2022
#		define _CRTDBG_MAP_ALLOC
#		include <stdlib.h>
#		include <crtdbg.h>
#	endif
#endif

//Basic
//#include <stdio.h>		//printf and generic calls
//#include <stdarg.h>		//for functions with variable parameters length (como el "printf")
//#include <stdlib.h>	//for "malloc", "free", "size_t" y rand()
//#include <assert.h>	//for assert

//__OBJC__ (defined when compiling a '.m' or '.mm' file)
//__ANDROID__ (defined for ANDROID)
//__MAC_OS_X_VERSION_MIN_REQUIRED (defined for OSX)
//__IPHONE_OS_VERSION_MIN_REQUIRED (defined for iOS)


#if defined(_WIN32) || defined(WIN32)
//#	include <winsock2.h>	//Include before windows.h
//#	include <windows.h>
//#	include <ws2tcpip.h>	//for sockaddr_in6
//
//	Validation
//#	if !defined(_WINSOCK2API_)
//#	if !defined(_WINSOCKAPI_)
//#		error No WINSOCK API was loaded by windows.h
//#	else
//#		warning WINSOCK_1 API was loaded by windows.h (not WINSOCK_2)
//#	endif
//#	endif
#	ifndef WIN32
#		define WIN32
#	endif
#endif


#if defined(__ANDROID__)
//#	include <android/log.h>
#endif

#ifdef __OBJC__
//#	import <Foundation/Foundation.h>
#endif

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#   define NB_IS_POSIX      //current system is posix (autodetection)
#endif

#endif //NBFrameworkPch_h
