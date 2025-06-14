
#ifndef NB_HTTPRESPONSE_H
#define NB_HTTPRESPONSE_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"
//#include "nb/net/NBHttpRequest.h"
#include "nb/net/NBHttpHeader.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef enum ENNBHttpTipoTransf_ {
		ENNBHttpTipoTransf_SinDefinir = 0,
		ENNBHttpTipoTransf_Desconocido,
		ENNBHttpTipoTransf_ContentLength,
		ENNBHttpTipoTransf_ChunckedEncoding,
        ENNBHttpTipoTransf_204NoContent, //Http 204 code was returned, empty body is implied
	} ENNBHttpTipoTransf;
	
	typedef enum ENNBHttpLecturaChunckModo_ {
		ENNBHttpLecturaChunckModo_TamanoHex = 0,
		ENNBHttpLecturaChunckModo_FinTamanoHex,
		ENNBHttpLecturaChunckModo_Contenido,
		ENNBHttpLecturaChunckModo_FinContenido
	} ENNBHttpLecturaChunckModo;
	
	typedef struct STNBHttpLecturaChunckEstado_ {
		BOOL errorEncontrado;
		BOOL chucnkTamCeroEncontrado;
		ENNBHttpLecturaChunckModo modoLectura;
		STNBString strBuffer;
		SI32 chunckActualBytesEsperados;
		SI32 chucnkActualBytesLeidos;
	} STNBHttpLecturaChunckEstado;
	
	typedef struct STNBHttpRespHeadr_ {
		STNBString		name;
		STNBString		value;
	} STNBHttpRespHeadr;
	
	typedef struct STNBHttpResponse_ {
		ENNBHttpTipoTransf			_transfType;
		BOOL						_transfEnded;
		BOOL						_zeroContentLenAsDef;
		STNBHttpLecturaChunckEstado	_transfState;
		//
		SI32			bytesReceived;
		SI32			bytesExpected;
		//
		float			httpVersion;
		SI32			code;
		STNBArray		headers;	//STNBHttpRespHeadr
		STNBString		header;
		STNBString		body;
		//
		SI32			retainCount;
	} STNBHttpResponse;
	
	//Factory
	void NBHttpResponse_init(STNBHttpResponse* obj);
	void NBHttpResponse_retain(STNBHttpResponse* obj);
	void NBHttpResponse_release(STNBHttpResponse* obj);
	
	//Config
	void NBHttpResponse_setZeroContentLenAsDefault(STNBHttpResponse* obj, const BOOL zeroContentLenAsDef); //When 'Content-Length' is not defined then zero will be the default value.

	//Process data
	BOOL NBHttpResponse_consumeBytes(STNBHttpResponse* obj, const char* buff, const SI32 bytesRcvd, const char* protocol);
	BOOL NBHttpResponse_consumeHeader(STNBHttpResponse* obj, const char* protocol);
	
	//Read data
	const char* NBHttpResponse_headerValue(const STNBHttpResponse* obj, const char* header, const char* defValue);
	//Cursor for reading header values:
	//Multiple http headers can be combined in one separated by comma;
	//quoted-strings "..." and comments (...) can be included.
	BOOL NBHttpResponse_getNextFieldValue(const STNBHttpResponse* obj, const char* fieldName, const BOOL allowComments, STNBHttpHeaderFieldCursor* cursor);
	
	//Parse data
	void NBHttpResponseReadState_init(STNBHttpLecturaChunckEstado* obj);
	void NBHttpResponseReadState_release(STNBHttpLecturaChunckEstado* obj);
	BOOL NBHttpResponseReadState_processData(STNBHttpLecturaChunckEstado* obj, const char* dataRead, const SI32 bytesCount, STNBString* saveContentAt);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
