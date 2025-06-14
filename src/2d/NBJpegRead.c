//
//  NBJpeg.c
//  nbframework
//
//  Created by Marcos Ortega on 18/3/19.
//

#include "nb/NBFrameworkPch.h"
#include "nb/2d/NBJpegRead.h"
//
#include "nb/core/NBMemory.h"
//
#include <stddef.h>	//for size_t (required by 'jpeglib.h')
#include <stdio.h>	//for FILE (required by 'jpeglib.h')
#include "jpeglib.h"

//--------------------------
// Jpeg reading opq (definitions)
//--------------------------

#define NB_JPEG_BUFFER_LECTURA_TAM	4096

typedef struct NBJpegReadOpq_ {
	//Estado de flujo de archivo
	STNBFileRef archivo;
	BYTE* bufferLectura;			//NB_JPEG_BUFFER_LECTURA_TAM;
	UI32 posEnBufferLectura;
	UI32 usoBufferLectura;
	//Bufferes
	UI32 bufferDestinoLineasUso;	//Definidos automaticamente
	UI32 bufferDestinoLineasCaben;	//Definidos automaticamente
	BYTE* punterosLineasBase;		//Definidos automaticamente
	UI32 punterosLineasBaseTamano;	//Definidos automaticamente
	BYTE** punterosLineas;			//Definidos automaticamente
	//Propiedades de JPEG
	bool headLeido;					//Encabezado leido
	bool decmpIniciado;				//Decompresor inicializado
	bool decmpFinalizado;			//Decompresor finalizado
	UI16 decmpLineasLeidas;
	STNBBitmapProps propsMapaBits;
	//Objetos de la lib-jpeg
	struct jpeg_error_mgr* jpegErr;				//opaco, jpeg_error_mgr, manejo de errores
	struct jpeg_decompress_struct* jpegDecomp;	//opaco, jpeg_decompress_struct, estado de decompresor
	struct jpeg_source_mgr* jpegManager;		//opcao, jpeg_source_mgr, origen de datos
} NBJpegReadOpq;

//void NBGlueLibJpegRead_privStateInit(NBJpegReadOpq* opq, STNBFileRef stream);
//void NBGlueLibJpegRead_privStateFinish(NBJpegReadOpq* opq);

void	NBGlueLibJpegRead_jpeg_in_init_source(j_decompress_ptr cinfo);
boolean	NBGlueLibJpegRead_jpeg_in_fill_input_buffer(j_decompress_ptr cinfo);
void	NBGlueLibJpegRead_jpeg_in_skip_input_data(j_decompress_ptr cinfo, long num_bytes);
boolean	NBGlueLibJpegRead_jpeg_in_resync_to_restart(j_decompress_ptr cinfo, int desired);
void	NBGlueLibJpegRead_jpeg_in_term_source(j_decompress_ptr cinfo);

//--------------------------
// Jpeg reading opq (implementations)
//--------------------------

void NBJpegRead_init(STNBJpegRead* obj){
	NBJpegReadOpq* opq = obj->opaque = NBMemory_allocType(NBJpegReadOpq);
	NBMemory_setZeroSt(*opq, NBJpegReadOpq);
}

void NBJpegRead_release(STNBJpegRead* obj){
	NBJpegReadOpq* opq = (NBJpegReadOpq*)obj->opaque;
	if(opq != NULL){
		NBJpegRead_feedRelease(obj);
		NBMemory_free(opq);
		obj->opaque = NULL;
	}
}

BOOL NBJpegRead_feedStart(STNBJpegRead* obj, STNBFileRef stream){
	BOOL r = FALSE;
	NBJpegReadOpq* opq = (NBJpegReadOpq*)obj->opaque;
	if(opq != NULL){
		//Release previous
		NBJpegRead_feedRelease(obj);
		//Estado de flujo de archivo
		opq->archivo					= stream;
		opq->bufferLectura				= (BYTE*)NBMemory_alloc(sizeof(BYTE) * NB_JPEG_BUFFER_LECTURA_TAM);
		opq->posEnBufferLectura			= 0;
		opq->usoBufferLectura			= 0;
		//Bufferes
		opq->punterosLineasBase			= NULL;		//Definido automaticamente
		opq->punterosLineasBaseTamano	= 0;	//Definido automaticamente
		opq->punterosLineas				= NULL;		//Definido automaticamente
		//Propiedades del JPEG
		opq->headLeido					= false;
		opq->decmpIniciado				= false;
		opq->decmpFinalizado			= false;
		opq->decmpLineasLeidas			= 0;
		NBMemory_setZeroSt(opq->propsMapaBits, STNBBitmapProps);
		//
		struct jpeg_error_mgr* jpegErr				= NBMemory_allocType(struct jpeg_error_mgr);
		struct jpeg_decompress_struct* jpegDecomp	= NBMemory_allocType(struct jpeg_decompress_struct);
		struct jpeg_source_mgr* jpegManager			= NBMemory_allocType(struct jpeg_source_mgr);
		opq->jpegErr					= jpegErr;
		opq->jpegDecomp					= jpegDecomp;
		opq->jpegManager				= jpegManager;
		//Manager de la lib-jpeg
		jpeg_create_decompress(jpegDecomp);
		jpegDecomp->client_data			= opq;
		jpegDecomp->src					= jpegManager;
		jpegDecomp->err					= jpeg_std_error(jpegErr);
		jpegManager->next_input_byte	= opq->bufferLectura;
		jpegManager->bytes_in_buffer	= 0;
		jpegManager->init_source		= NBGlueLibJpegRead_jpeg_in_init_source;
		jpegManager->fill_input_buffer	= NBGlueLibJpegRead_jpeg_in_fill_input_buffer;
		jpegManager->skip_input_data	= NBGlueLibJpegRead_jpeg_in_skip_input_data;
		jpegManager->resync_to_restart	= NBGlueLibJpegRead_jpeg_in_resync_to_restart;
		jpegManager->term_source		= NBGlueLibJpegRead_jpeg_in_term_source;
		r = TRUE;
	}
	return r;
}

ENJpegReadResult NBJpegRead_feedRead(STNBJpegRead* obj, BYTE* dst, const UI32 dstSz, UI32* dstLinesReadCount){
	ENJpegReadResult r = ENJpegReadResult_error;
	NBJpegReadOpq* opq = (NBJpegReadOpq*)obj->opaque;
	if(opq != NULL){
		struct jpeg_decompress_struct* jpegDecomp = opq->jpegDecomp;
		//Leer encabezado
		if(!opq->headLeido){
			const int ret = jpeg_read_header(jpegDecomp, TRUE);
			if(ret == JPEG_HEADER_OK){
				opq->headLeido	= true;
				opq->propsMapaBits.size.width		= jpegDecomp->image_width; //nominal image width (from SOF marker)
				opq->propsMapaBits.size.height	= jpegDecomp->image_height; //nominal image height
				switch (jpegDecomp->jpeg_color_space) { //colorspace of JPEG image
					case JCS_GRAYSCALE:
						NBASSERT(jpegDecomp->num_components == 1) //# of color components in JPEG image
						opq->propsMapaBits.bitsPerPx	= 8;
						opq->propsMapaBits.bytesPerLine = opq->propsMapaBits.size.width; if((opq->propsMapaBits.bytesPerLine % 4) != 0) opq->propsMapaBits.bytesPerLine += 4 - (opq->propsMapaBits.bytesPerLine % 4);
						opq->propsMapaBits.color = ENNBBitmapColor_GRIS8;
						break;
					case JCS_RGB:
						NBASSERT(jpegDecomp->num_components == 3) //# of color components in JPEG image
						opq->propsMapaBits.bitsPerPx	= 24;
						opq->propsMapaBits.bytesPerLine = opq->propsMapaBits.size.width * 3; if((opq->propsMapaBits.bytesPerLine % 4) != 0) opq->propsMapaBits.bytesPerLine += 4 - (opq->propsMapaBits.bytesPerLine % 4);
						opq->propsMapaBits.color = ENNBBitmapColor_RGB8;
						break;
					case JCS_YCbCr:
						NBASSERT(jpegDecomp->num_components == 3) //# of color components in JPEG image
						opq->propsMapaBits.bitsPerPx	= 24;
						opq->propsMapaBits.bytesPerLine = opq->propsMapaBits.size.width * 3; if((opq->propsMapaBits.bytesPerLine % 4) != 0) opq->propsMapaBits.bytesPerLine += 4 - (opq->propsMapaBits.bytesPerLine % 4);
						opq->propsMapaBits.color = ENNBBitmapColor_RGB8;
						break;
					//case JCS_YCbCr:	 PRINTF_ERROR("jpeg_color_space no soportado: JCS_YCbCr componentes(%d).\n", opq->jpegDecomp.num_components); break;
					//case JCS_CMYK:	 PRINTF_ERROR("jpeg_color_space no soportado: JCS_CMYK componentes(%d).\n", jpegDecomp->num_components); break;
					//case JCS_YCCK:	 PRINTF_ERROR("jpeg_color_space no soportado: JCS_YCCK componentes(%d).\n", jpegDecomp->num_components); break;
					//case JCS_BG_RGB: PRINTF_ERROR("jpeg_color_space no soportado: JCS_BG_RGB componentes(%d).\n", jpegDecomp->num_components); break;
					//case JCS_BG_YCC: PRINTF_ERROR("jpeg_color_space no soportado: JCS_BG_YCC componentes(%d).\n", jpegDecomp->num_components); break;
					case JCS_UNKNOWN:
					default:
						PRINTF_ERROR("jpeg_color_space no identificado componentes(%d).\n", jpegDecomp->num_components);
						break;
				}
			}
		}
		//
		if(opq->headLeido){
			if(dst == NULL || dstSz == 0){
				r = (opq->propsMapaBits.bytesPerLine == 0 ? ENJpegReadResult_error : ENJpegReadResult_end);
			} else {
				//Init decomp
				if(!opq->decmpIniciado){
					const boolean ret = jpeg_start_decompress(jpegDecomp);
					if(ret == TRUE){
						opq->decmpIniciado = true;
					}
				}
				//Decomp
				if(opq->decmpIniciado){
					UI32 dstLineasUso = 0;
					UI32 dstLineasCaben = (dstSz / opq->propsMapaBits.bytesPerLine); NBASSERT(dstLineasCaben > 0)
					//Actualizar buffer de lineas (si es necesario)
					if(opq->punterosLineasBase != dst || opq->punterosLineasBaseTamano != dstSz){
						//Generar arreglo con los punteros de inicio de cada fila
						if(opq->punterosLineas != NULL) NBMemory_free(opq->punterosLineas); opq->punterosLineas = NULL;
						if(dstLineasCaben > 0){
							opq->punterosLineas = NBMemory_allocTypes(JSAMPLE*, dstLineasCaben);
						}
						//
						UI32 i;
						for(i = 0; i < dstLineasCaben; i++){
							opq->punterosLineas[i] = &dst[opq->propsMapaBits.bytesPerLine * i]; NBASSERT(opq->punterosLineas[i] < (dst + dstSz))
						}
						//
						opq->punterosLineasBase		= dst;
						opq->punterosLineasBaseTamano	= dstSz;
					}
					//Procesar lineas
					while (jpegDecomp->output_scanline < jpegDecomp->output_height && dstLineasUso < dstLineasCaben) {
						NBASSERT(opq->decmpLineasLeidas < opq->propsMapaBits.size.height)
						JDIMENSION lineasLeidas = jpeg_read_scanlines(jpegDecomp, &opq->punterosLineas[dstLineasUso], (dstLineasCaben - dstLineasUso));
						opq->decmpLineasLeidas += (UI16)lineasLeidas;
						dstLineasUso += lineasLeidas;
						NBASSERT(opq->decmpLineasLeidas <= opq->propsMapaBits.size.height)
					}
					NBASSERT(jpegDecomp->output_scanline == opq->decmpLineasLeidas)
					NBASSERT(jpegDecomp->output_scanline <= opq->propsMapaBits.size.height)
					NBASSERT(opq->decmpLineasLeidas <= opq->propsMapaBits.size.height)
					//Finalizar
					if(jpegDecomp->output_scanline == jpegDecomp->output_height && !opq->decmpFinalizado){
						const boolean ret = jpeg_finish_decompress(jpegDecomp);
						if(ret == TRUE){
							opq->decmpFinalizado = true;
						}
					}
					r = ((dstLineasUso == dstLineasCaben || opq->decmpFinalizado) ? ENJpegReadResult_end : ENJpegReadResult_partial);
					//
					if(dstLinesReadCount != NULL){
						*dstLinesReadCount = *dstLinesReadCount + dstLineasUso;
					}
				}
			}
		}
	}
	return r;
}

STNBBitmapProps NBJpegRead_feedGetProps(STNBJpegRead* obj){
	STNBBitmapProps r;
	NBMemory_setZeroSt(r, STNBBitmapProps);
	NBJpegReadOpq* opq = (NBJpegReadOpq*)obj->opaque;
	if(opq != NULL){
		r = opq->propsMapaBits;
	}
	return r;
}

void NBJpegRead_feedRelease(STNBJpegRead* obj){
	NBJpegReadOpq* opq = (NBJpegReadOpq*)obj->opaque;
	if(opq != NULL){
		if(opq->jpegDecomp != NULL){
			//Manager de la lib-jpeg
			if(opq->decmpIniciado && !opq->decmpFinalizado){
				jpeg_finish_decompress(opq->jpegDecomp);
				opq->decmpFinalizado = true;
			}
			jpeg_destroy_decompress(opq->jpegDecomp);
			NBMemory_free(opq->jpegDecomp);
			opq->jpegDecomp = NULL;
		}
		//
		if(opq->jpegErr != NULL) NBMemory_free(opq->jpegErr); opq->jpegErr = NULL;
		if(opq->jpegManager != NULL) NBMemory_free(opq->jpegManager); opq->jpegManager = NULL;
		//
		if(opq->bufferLectura != NULL) NBMemory_free(opq->bufferLectura); opq->bufferLectura = NULL;
		if(opq->punterosLineas != NULL) NBMemory_free(opq->punterosLineas); opq->punterosLineas = NULL;
	}
}

//

/*void NBGlueLibJpegRead_privStateInit(NBJpegReadOpq* opq, STNBFileRef stream){
	
}*/

/*void NBGlueLibJpegRead_privStateFinish(NBJpegReadOpq* opq){
	
}*/

void NBGlueLibJpegRead_jpeg_in_init_source(j_decompress_ptr cinfo){
	IF_NBASSERT(NBJpegReadOpq * opq = (NBJpegReadOpq*)cinfo->client_data;)
	NBASSERT(opq != NULL && NBFile_isSet(opq->archivo))
}

boolean NBGlueLibJpegRead_jpeg_in_fill_input_buffer(j_decompress_ptr cinfo){
    BOOL r = FALSE;
	NBJpegReadOpq* opq = (NBJpegReadOpq*)cinfo->client_data; NBASSERT(opq != NULL && NBFile_isSet(opq->archivo))
	const SI32 leidos = NBFile_read(opq->archivo, opq->bufferLectura, NB_JPEG_BUFFER_LECTURA_TAM);
	if(leidos >= 0){
		struct jpeg_source_mgr* mgr = opq->jpegManager;
		mgr->next_input_byte = opq->bufferLectura;
		mgr->bytes_in_buffer = leidos;
		r = (leidos > 0 ? TRUE : FALSE);
	}
	return (boolean)r;
}

void NBGlueLibJpegRead_jpeg_in_skip_input_data(j_decompress_ptr cinfo, long num_bytes){
	NBJpegReadOpq* opq = (NBJpegReadOpq*)cinfo->client_data; NBASSERT(opq != NULL && NBFile_isSet(opq->archivo))
	if (num_bytes > 0) {
		struct jpeg_source_mgr* mgr = opq->jpegManager;
		while (num_bytes > (long) mgr->bytes_in_buffer) {
			num_bytes -= (long) mgr->bytes_in_buffer;
			(void) (*mgr->fill_input_buffer) (cinfo);
		}
		mgr->next_input_byte += (size_t) num_bytes;
		mgr->bytes_in_buffer -= (size_t) num_bytes;
	}
}

boolean NBGlueLibJpegRead_jpeg_in_resync_to_restart(j_decompress_ptr cinfo, int desired){
	//PENDIENTE, necesario restablecer variables de buffer
	//Pendiente implementar acorde al metodo por defecto.
	return FALSE;
}

void NBGlueLibJpegRead_jpeg_in_term_source(j_decompress_ptr cinfo){
	IF_NBASSERT(NBJpegReadOpq* opq = (NBJpegReadOpq*)cinfo->client_data;)
	NBASSERT(opq != NULL && NBFile_isSet(opq->archivo))
}

//--------------------------
// Calls methods (implementations)
//--------------------------

/*void* NBGlueLibJpegRead_stateCreate(void* param, STNBFileRef stream){
	NBJpegReadOpq* opq = NBMemory_allocType(NBJpegReadOpq);
	NBGlueLibJpegRead_privStateInit(opq, stream);
	return opq;
}*/

/*void NBGlueLibJpegRead_stateDestroy(void* param, void* pState){
	NBJpegReadOpq* opq = (NBJpegReadOpq*)pState;
	if(opq != NULL){
		NBGlueLibJpegRead_privStateFinish(opq);
		NBMemory_free(opq);
	}
}*/

/*bool NBGlueLibJpegRead_GetProps(void* param, void* pState, STNBBitmapProps* dstProps){
	bool r = false;
	NBJpegReadOpq* opq = (NBJpegReadOpq*)pState;
	if(opq != NULL){
		if(opq->headLeido){
			if(dstProps != NULL) *dstProps = opq->propsMapaBits;
			r = true;
		}
	}
	return r;
}*/

/*ENJpegReadResult NBGlueLibJpegRead_stateRead(void* param, void* pState, BYTE* dst, const UI32 dstSz, UI32* dstLinesReadCount){
	
}*/
