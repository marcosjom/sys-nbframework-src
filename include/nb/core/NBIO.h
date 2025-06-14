#ifndef NB_IO_H
#define NB_IO_H

#include "nb/NBFrameworkDefs.h"

#ifdef __cplusplus
extern "C" {
#endif

//NB_IO_ERR*

#define NB_IO_ERROR             -1  //generic error
#define NB_IO_ERR_EOF           -2  //end-of-file (this error can be temporal, if new data is written to the object on reading)
#define NB_IO_ERR_SHUTTED_DOWN  -3  //operation (read or write) is shutted down
//Note: update 'NBIO_getErrStr()' after adding NB_IO_ERR_* value.

//NB_IO_BIT
#define NB_IO_BIT_READ          1   //bit representing read action
#define NB_IO_BIT_WRITE         2   //bit representing write action
#define NB_IO_BITS_RDWR   (NB_IO_BIT_READ | NB_IO_BIT_WRITE)

const char* NBIO_getErrStr(const SI32 errCode);

#ifdef __cplusplus
} //extern "C"
#endif


#endif
