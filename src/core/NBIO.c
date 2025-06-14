
#include "nb/NBFrameworkPch.h"
#include "nb/core/NBIO.h"

const char* NBIO_getErrStr(const SI32 errCode){
    switch (errCode) {
        case NB_IO_ERROR: return "ERROR"; //generic error
        case NB_IO_ERR_EOF: return "EOF"; //end-of-file (this error can be temporal, if new data is written to the object on reading)
        case NB_IO_ERR_SHUTTED_DOWN: return "SHUTTED_DOWN"; //operation (read or write) is shutted down
        default:
            if(errCode >= 0) return "OK";
            return "UNKNOWN";
    }
    return NULL;
}
