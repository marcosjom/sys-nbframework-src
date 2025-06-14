//
// Prefix header for all source files
//

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBMngrProcess.h"
//
#include <string.h>	//for 'strerror()'
#include <errno.h>  //for 'strerror()'
//
void __nbPrintfInfo(const char* fmt, ...){
	va_list vargs;
	va_start(vargs, fmt);
	{
		NBMngrProcess_printfInfoV(fmt, vargs);
	}
	va_end(vargs);
}

void __nbPrintfWarn(const char* fmt, ...){
	va_list vargs;
	va_start(vargs, fmt);
	{
		NBMngrProcess_printfWarnV(fmt, vargs);
	}
	va_end(vargs);
}

void __nbPrintfError(const char* fmt, ...){
	va_list vargs;
	va_start(vargs, fmt);
	{
		NBMngrProcess_printfErrorV(fmt, vargs);
	}
	va_end(vargs);
}

#ifdef NB_CONFIG_INCLUDE_ASSERTS
void __nbAssertFailed(const char* expression, const char* filepath, const SI32 line, const char* func){
	NBMngrProcess_assertFailed(expression, filepath, line, func);
}
#endif

int __nb_strerror_r(char *buf, const SI32 buflen, int errnum){
#	if defined(_MSC_VER) && _MSC_VER>=1400
	return !strerror_s(buf, buflen, errnum);
#	elif defined(_GNU_SOURCE)
	return strerror_r(errnum, buf, buflen) != NULL;
#	elif (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200112L) || \
	  (defined(_XOPEN_SOURCE) && _XOPEN_SOURCE >= 600)
	/*
	 * We can use "real" strerror_r. The OpenSSL version differs in that it
	 * gives 1 on success and 0 on failure for consistency with other OpenSSL
	 * functions. Real strerror_r does it the other way around
	 */
	return !strerror_r(errnum, buf, buflen);
#	else
	char *err;
	/* Fall back to non-thread safe strerror()...its all we can do */
	if (buflen < 2)
		return 0;
	err = strerror(errnum);
	/* Can this ever happen? */
	if (err == NULL)
		return 0;
	strncpy(buf, err, buflen - 1);
	buf[buflen - 1] = '\0';
	return 1;
#	endif
}
