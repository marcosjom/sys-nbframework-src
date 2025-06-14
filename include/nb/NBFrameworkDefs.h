//
// Prefix header for all source files
//

#ifndef NB_FRAMEWORK_DEFS_H
#define NB_FRAMEWORK_DEFS_H

#include "nb/NBFrameworkCfg.h"
//#include <stdio.h>	//printf and generic calls
//#include <stdlib.h>	//for "abort" (NBASSERT)
//#include <assert.h>	//for assert
#include <stdbool.h>	//only C99

//Cfg - validation

#if defined(CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT_ACCESO_MEMORIA) && !defined(CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT)
#	error "ERROR DE CONFIGURACION: se configura la validacion de acceso a memoria pero no se configura NBASSERT"
#endif

#if (defined(CONFIG_NB_GESTOR_MEMORIA_GUARDAR_PUNTEROS_RETENCIONES) || defined(CONFIG_NB_GESTOR_MEMORIA_GUARDAR_NOMBRES_PUNTEROS)) && !defined(CONFIG_NB_GESTOR_MEMORIA_REGISTRAR_BLOQUES)
#	error "ERROR DE CONFIGURACION: se configura guardar nombre de puntero o sus retenciones pero no se guardan punteros."
#endif

#if !defined(CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION) && defined(CONFIG_NB_INCLUDE_THREADS_LOCKS_RISK_VALIDATION)
#	error "CONFIG_NB_INCLUDE_THREADS_LOCKS_RISK_VALIDATION requieres CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION"
#endif

//Cfg - autodef

#if defined(CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION) || defined(CONFIG_NB_INCLUDE_THREADS_LOCKS_RISK_VALIDATION) || defined(CONFIG_NB_INCLUDE_THREADS_METRICS) || defined(CONFIG_NB_INCLUDE_THREADS_STORAGE_CLEANUP)
#	define CONFIG_NB_INCLUDE_THREADS_ENABLED
#endif

//NULL
#if !defined(__cplusplus) && !defined(NULL)
#	define NULL		((void*)0)
#endif

//PI and PIx2
#ifndef PI
#	define PI		3.14159265359f //3.14159265f
#endif
#ifndef PIx2
#	define PIx2		6.28318530718f //6.2831853f
#endif

#define RAD_2_DEG(RAD)		((RAD) * 180.0f / PI)
#define DEG_2_RAD(DEG)		((DEG) * PI / 180.0f)

/*#if !defined(__cplusplus) && !defined(__OBJC__) && !defined(BOOL)
#	define BOOL char
#endif*/
#ifndef TRUE
#	define TRUE		1
#endif
#ifndef FALSE
#	define FALSE	0
#endif

//Search reg-ex: \(([A-Za-z0-9 ]*)\)\{([A-Za-z0-9_,. =!?:+-/*->\[\]\(\)\t\n]*)\}
//Note: _WIN32 is defined on 32 and 64 bits, _WIN64 only on 64 bits.
#if defined(_WIN32) && defined(__cplusplus)
#	define NBST_P(TYPE, ...)	TYPE { __VA_ARGS__ }	//Visual-studio C++
#else
#	define NBST_P(TYPE, ...)	(TYPE){ __VA_ARGS__ }	//XCode
#endif

//Note: _WIN32 is defined on 32 and 64 bits, _WIN64 only on 64 bits.
#ifdef _WIN32
	typedef int					BOOL; //defined as in "minwindef.h"
#elif defined(__OBJC_BOOL_IS_BOOL)
#	if __OBJC_BOOL_IS_BOOL
	typedef bool				BOOL;
#	else
	typedef signed char			BOOL;
#	endif
#else
#	if TARGET_OS_OSX || (TARGET_OS_IOS && !__LP64__ && !__ARM_ARCH_7K)
	typedef signed char			BOOL;
#	else
	typedef bool				BOOL;
#	endif
#endif

#ifdef __LP64__
	typedef unsigned char 		BYTE;
	typedef signed char 		SI8;
	typedef unsigned char 		UI8;
	typedef	short int 			SI16;
	typedef	unsigned short int 	UI16;
	typedef	int 				SI32;
	typedef	unsigned int 		UI32;
	typedef	long long 			SI64;
	typedef	unsigned long long	UI64;
	typedef float				FLOAT;
	typedef double				DOUBLE;
#	define NBWORD				SI64	//type for atomic operation in current processor
#	define NBUWORD				UI64	//type for atomic operation in current processor
#	define NBUWORD_MAX			0xFFFFFFFFFFFFFFFFULL
#else
	typedef unsigned char 		BYTE;
	typedef signed char 		SI8;
	typedef unsigned char 		UI8;
	typedef	short int 			SI16;
	typedef	unsigned short int 	UI16;
	typedef	int 				SI32;
	typedef	unsigned int 		UI32;
	typedef	long long 			SI64;
	typedef	unsigned long long	UI64;
	typedef float				FLOAT;
	typedef double				DOUBLE;
#	define NBWORD				SI32	//type for atomic operation in current processor
#	define NBUWORD				UI32	//type for atomic operation in current processor
#	define NBUWORD_MAX			0xFFFFFFFFUL
#endif

//-------------------------------
//-- Definiciones Personales (MJOM)
//-------------------------------
#if defined(NB_COMPILE_DEVELOPMENT_VERSION) && defined(CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT)
#	define NB_CONFIG_INCLUDE_ASSERTS
#endif
//#define NB_CONFIG_INCLUDE_ASSERTS_LOADING_FILES
//#define NB_CONFIG_VERIFY_NOT_DUPLICATED_ON_ARRAY_ADD		//If defined, the method "DBStrgRecords_add" and others will verify if the new record will generate a duplicated Primarykey (integrity test, not necesarry on release)
//
//-------------------------------
//-- Definiciones Modulares (quita dependencias hacia archivos del mismo framework)
//-------------------------------
#define NB_HTTPCLIENT_USE_THREAD	//Si no-defined libera NBHttpClient de NBThread, pero deshabilita las consultas http asyncronas.
#define NB_ENABLE_OPENSSL

//-------------
//- PRINTF_*
//-------------

/*
#define NBASNSI_RESET		"\033[0]"
#define NBASNSI_FG_BLACK	"\033[30]"
#define NBASNSI_FG_RED		"\033[31]"
#define NBASNSI_FG_GREEN	"\033[32]"
#define NBASNSI_FG_YELLOW	"\033[33]"
#define NBASNSI_FG_BLUE		"\033[34]"
#define NBASNSI_FG_MAGENTA	"\033[35]"
#define NBASNSI_FG_CYAN		"\033[36]"
#define NBASNSI_FG_WHITE	"\033[37]"

#define NBASNSI_FG_BLACK_STR(STR)	NBASNSI_FG_BLACK	STR NBASNSI_RESET
#define NBASNSI_FG_RED_STR(STR)		NBASNSI_FG_RED		STR NBASNSI_RESET
#define NBASNSI_FG_GREEN_STR(STR)	NBASNSI_FG_GREN		STR NBASNSI_RESET
#define NBASNSI_FG_YELLOW_STR(STR)	NBASNSI_FG_YELLOW	STR NBASNSI_RESET
#define NBASNSI_FG_BLUE_STR(STR)	NBASNSI_FG_BLUE		STR NBASNSI_RESET
#define NBASNSI_FG_MAGENTA_STR(STR)	NBASNSI_FG_MAGENTA	STR NBASNSI_RESET
#define NBASNSI_FG_CYAN_STR(STR)	NBASNSI_FG_CYAN		STR NBASNSI_RESET
#define NBASNSI_FG_WHITE_STR(STR)	NBASNSI_FG_WHITE	STR NBASNSI_RESET
*/

#ifdef CONFIG_NB_DESHABILITAR_IMPRESIONES_PRINTF
#	define IF_PRINTF(CODE)					//nothing
#	define PRINTF_INFO( FMT, ... )			((void)0);
#	define PRINTF_WARNING( FMT, ... )		((void)0);
#	define PRINTF_ERROR( FMT, ... )			((void)0);
#else
#	define IF_PRINTF(CODE)					CODE
#	define PRINTF_INFO( FMT, ... )			__nbPrintfInfo(FMT, ##__VA_ARGS__);
#	define PRINTF_WARNING( FMT, ... )		__nbPrintfWarn(FMT, ##__VA_ARGS__);
#	define PRINTF_ERROR( FMT, ... )			__nbPrintfError(FMT, ##__VA_ARGS__);
#endif

#define PRINTF_CONSOLE( FMT, ... )			__nbPrintfInfo(FMT, ##__VA_ARGS__);
#define PRINTF_CONSOLE_ERROR( FMT, ... )	__nbPrintfError(FMT, ##__VA_ARGS__);

//-------------
//- VA_ARGS
//-------------

//ToDo: remove
#define NB_VA_ARGS_ARG16( _0,  _1,  _2,  _3,  _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, ...) _15
#define NB_VA_ARGS_HAS_COMMA(...) NB_VA_ARGS_ARG16(__VA_ARGS__, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0)
#define NB_VA_ARGS_TRIGGER_PARENTHESIS_ (...) ,
#define NB_VA_ARGS_ISEMPTY(...) \
	NB_VA_ARGS_ISEMPTY_( \
		NB_VA_ARGS_HAS_COMMA(__VA_ARGS__), \
		NB_VA_ARGS_HAS_COMMA(NB_VA_ARGS_TRIGGER_PARENTHESIS_ __VA_ARGS__),  \
		NB_VA_ARGS_HAS_COMMA(__VA_ARGS__ (/*empty*/)), \
		NB_VA_ARGS_HAS_COMMA(NB_VA_ARGS_TRIGGER_PARENTHESIS_ __VA_ARGS__ (/*empty*/)) \
	)
#define NB_VA_ARGS_IS_EMPTY_CASE_0001 ,
#define NB_VA_ARGS_PASTES(_0, _1, _2, _3, _4 ) _0 ## _1 ## _2 ## _3 ## _4
#define NB_VA_ARGS_ISEMPTY_(_0, _1, _2, _3) NB_VA_ARGS_HAS_COMMA(NB_VA_ARGS_PASTES(NB_VA_ARGS_IS_EMPTY_CASE_, _0, _1, _2, _3))

//-------------
//- ASSERT
//-------------
//CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT vs NB_CONFIG_INCLUDE_ASSERTS?

//ToDo: remove
/*#ifdef CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION
#	define NBASSERT_PRINT_STACK(PREFIX)		__nbMutexStackPrintAsErrorFatal(PREFIX)
#else
#	define NBASSERT_PRINT_STACK(PREFIX)
#endif*/

#ifdef NB_CONFIG_INCLUDE_ASSERTS
#	define NBASSERT(EXP)	if(!(EXP)){ __nbAssertFailed(#EXP, __FILE__, (SI32)__LINE__, __func__); }
#	define IF_NBASSERT(...)	__VA_ARGS__
#else
#	define NBASSERT(EXP)
#	define IF_NBASSERT(...)
#endif

//
#ifdef NB_CONFIG_INCLUDE_ASSERTS_LOADING_FILES
#	define NBASSERT_LOADFILE(VALIDATION)	NBASSERT(VALIDATION);
#else
#	define NBASSERT_LOADFILE(VALIDATION)
#endif

#ifdef __cplusplus
extern "C" {
#endif
	//
	void __nbPrintfInfo(const char* fmt, ...);
	void __nbPrintfWarn(const char* fmt, ...);
	void __nbPrintfError(const char* fmt, ...);
	//
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	void __nbAssertFailed(const char* expression, const char* filepath, const SI32 line, const char* func);
#	endif
	//
	int __nb_strerror_r(char *buf, const SI32 buflen, int errnum);
#ifdef __cplusplus
} //extern "C"
#endif



#endif
