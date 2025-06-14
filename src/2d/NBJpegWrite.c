//
//  NBJpeg.c
//  nbframework
//
//  Created by Marcos Ortega on 18/3/19.
//

#include "nb/NBFrameworkPch.h"
#include "nb/2d/NBJpegWrite.h"
//
#include "nb/core/NBMemory.h"
//
#include <stddef.h>	//for size_t (required by 'jpeglib.h')
#include <stdio.h>	//for FILE (required by 'jpeglib.h')
#include "jpeglib.h"

//--------------------------
// Jpeg writing opq (definitions)
//--------------------------

#define NB_JPEG_BUFFER_ESCRITURA_TAM	4096

typedef struct NBJpegWriteOpq_ {
	//Estado de destino
	STNBFileRef		archivo;				//Definido manualmente por la funcion usuaria
	UI8				calidadBase100;			//Definido manualmente por la funcion usuaria
	UI8				suavizadoBase100;		//Definido manualmente por la funcion usuaria
	//Propiedades de origen
	STNBBitmapProps	propsMapaBits;			//Definido manualmente por la funcion usuaria
	const BYTE*		bufferDatos;			//Definido manualmente por la funcion usuaria
	UI32			bufferDatosTam;			//Definido manualmente por la funcion usuaria
	UI32			bufferDatosUso;			//Definido manualmente por la funcion usuaria
	//Buffer
	BYTE*			bufferEscritura;		//Definido automaticamente
	UI32			bufferEscrituraTam;		//Definido automaticamente
	BYTE**			punterosLineas;			//Definido automaticamente
	UI16			punterosLineasTamano;	//Definido automaticamente
	UI16			lineasTotalVolcadas;	//Definido automaticamente
	//Objetos de la lib-jpeg
	struct jpeg_error_mgr*			jpegErr;			//opaco, jpeg_error_mgr, manejo de errores
	struct jpeg_compress_struct*	jpegComp;			//opcado, jpeg_compress_struct, estado de decompresor
	bool							jpegCompStarted;
	bool							jpegCompFinished;
	struct jpeg_destination_mgr*	jpegManager;		//opaco, jpeg_destination_mgr, origen de datos
} NBJpegWriteOpq;

//

//void		NBGlueLibJpegWrite_privStateInit(NBJpegWriteOpq* opq, const BYTE* data, const UI32 dataSz, const STNBBitmapProps dataFmt, STNBFileRef flujo, const UI8 calidadBase100, const UI8 suavizadoBase100, const bool createBuffers);
//void		NBGlueLibJpegWrite_privStateFinish(NBJpegWriteOpq* opq, const bool releaseBuffers);

//

void		NBGlueLibJpegWrite_jpeg_out_init_destination(j_compress_ptr cinfo);
boolean		NBGlueLibJpegWrite_jpeg_out_empty_output_buffer(j_compress_ptr cinfo);
void		NBGlueLibJpegWrite_jpeg_out_term_destination(j_compress_ptr cinfo);


//--------------------------
// Jpeg writing opq (implementations)
//--------------------------

void NBJpegWrite_init(STNBJpegWrite* obj){
	NBJpegWriteOpq* opq = obj->opaque = NBMemory_allocType(NBJpegWriteOpq);
	NBMemory_setZeroSt(*opq, NBJpegWriteOpq);
}

void NBJpegWrite_release(STNBJpegWrite* obj){
	NBJpegWriteOpq* opq = (NBJpegWriteOpq*)obj->opaque;
	if(opq != NULL){
		NBJpegWrite_feedRelease(obj);
		NBMemory_free(opq);
		obj->opaque = NULL;
	}
}

void NBJpegWrite_feedRelease_(NBJpegWriteOpq* opq, const BOOL releaseBuffers){
	if(opq != NULL){
		//Release JPEG instances
		if(opq->jpegComp != NULL){
			if(opq->jpegCompStarted && !opq->jpegCompFinished){
				jpeg_finish_compress((struct jpeg_compress_struct*)opq->jpegComp);
				opq->jpegCompFinished = true;
			}
			jpeg_destroy_compress((struct jpeg_compress_struct*)opq->jpegComp);
			NBMemory_free(opq->jpegComp);
			opq->jpegComp = NULL;
		}
		if(opq->jpegErr != NULL){
			NBMemory_free(opq->jpegErr);
			opq->jpegErr = NULL;
		}
		if(opq->jpegManager != NULL){
			NBMemory_free(opq->jpegManager);
			opq->jpegManager = NULL;
		}
		//Release buffers
		if(releaseBuffers){
			if(opq->bufferEscritura != NULL) NBMemory_free(opq->bufferEscritura); opq->bufferEscritura = NULL;
			if(opq->punterosLineas != NULL) NBMemory_free(opq->punterosLineas); opq->punterosLineas = NULL;
			opq->punterosLineasTamano = 0;
		}
	}
}

BOOL NBJpegWrite_feedStart_(NBJpegWriteOpq* opq, const void* data, const UI32 dataSz, const STNBBitmapProps dataFmt, STNBFileRef stream, const UI8 quality100, const UI8 smooth100, const BOOL createBuffers){
	BOOL r = FALSE;
	if(opq != NULL){
		if(dataFmt.color != ENNBBitmapColor_RGB8 && dataFmt.color != ENNBBitmapColor_GRIS8){
			PRINTF_ERROR("NBJpegWrite only supports RGB8 and GRAY8 bitmaps formats.\n")
		} else {
			//Release previous
			NBJpegWrite_feedRelease_(opq, createBuffers);
			//Propiedades de destino
			opq->archivo					= stream;
			opq->calidadBase100				= quality100;
			opq->suavizadoBase100			= smooth100;
			//Propiedades de origen
			opq->propsMapaBits				= dataFmt;
			opq->bufferDatos				= data;
			opq->bufferDatosUso				= dataSz;
			//Buffers
			if(createBuffers){
				opq->bufferEscrituraTam		= NB_JPEG_BUFFER_ESCRITURA_TAM;
				opq->bufferEscritura		= (BYTE*)NBMemory_alloc(opq->bufferEscrituraTam);
				opq->punterosLineasTamano	= 0;
				opq->punterosLineas			= NULL;
			}
			opq->lineasTotalVolcadas			= 0;
			//
			struct jpeg_error_mgr* jpegErr		= NBMemory_allocType(struct jpeg_error_mgr);
			struct jpeg_compress_struct* jpegComp		= NBMemory_allocType(struct jpeg_compress_struct);
			struct jpeg_destination_mgr* jpegManager	= NBMemory_allocType(struct jpeg_destination_mgr);
			opq->jpegErr						= jpegErr;
			opq->jpegComp						= jpegComp;
			opq->jpegCompStarted				= false;
			opq->jpegCompFinished				= false;
			opq->jpegManager					= jpegManager;
			//
			jpeg_create_compress(jpegComp);
			jpegComp->client_data				= opq;
			jpegComp->dest						= jpegManager;
			jpegComp->err						= jpeg_std_error(jpegErr);
			jpegManager->next_output_byte		= opq->bufferEscritura;
			jpegManager->free_in_buffer			= opq->bufferEscrituraTam;
			jpegManager->init_destination		= NBGlueLibJpegWrite_jpeg_out_init_destination;
			jpegManager->empty_output_buffer	= NBGlueLibJpegWrite_jpeg_out_empty_output_buffer;
			jpegManager->term_destination		= NBGlueLibJpegWrite_jpeg_out_term_destination;
			r = TRUE;
		}
	}
	return r;
}

BOOL NBJpegWrite_feedStart(STNBJpegWrite* obj, const void* data, const UI32 dataSz, const STNBBitmapProps dataFmt, STNBFileRef stream, const UI8 quality100, const UI8 smooth100){
	BOOL r = FALSE;
	NBJpegWriteOpq* opq = (NBJpegWriteOpq*)obj->opaque;
	if(opq != NULL){
		r = NBJpegWrite_feedStart_(opq, data, dataSz, dataFmt, stream, quality100, smooth100, TRUE);
	}
	return r;
}

BOOL NBJpegWrite_feedReset(STNBJpegWrite* obj, const void* data, const UI32 dataSz, const STNBBitmapProps dataFmt, STNBFileRef stream, const UI8 quality100, const UI8 smooth100){
	BOOL r = FALSE;
	NBJpegWriteOpq* opq = (NBJpegWriteOpq*)obj->opaque;
	if(opq != NULL){
		r = NBJpegWrite_feedStart_(opq, data, dataSz, dataFmt, stream, quality100, smooth100, FALSE);
	}
	return r;
}

ENJpegWriteResult NBJpegWrite_feedWrite(STNBJpegWrite* obj){
	ENJpegWriteResult r = ENJpegWriteResult_error;
	NBJpegWriteOpq* opq = (NBJpegWriteOpq*)obj->opaque;
	if(opq != NULL){
		struct jpeg_compress_struct* jpegComp	= opq->jpegComp;
		//PRINTF_INFO("NBJpegWrite_feedWrite.\n");
		//Inicializar compresor
		if(!opq->jpegCompStarted){
			jpegComp->image_width		= opq->propsMapaBits.size.width;
			jpegComp->image_height		= opq->propsMapaBits.size.height;
			jpegComp->input_components	= (opq->propsMapaBits.color == ENNBBitmapColor_RGB8 ? 3 : 1);
			jpegComp->in_color_space	= (opq->propsMapaBits.color == ENNBBitmapColor_RGB8 ? JCS_RGB : JCS_GRAYSCALE);
			jpeg_set_defaults(jpegComp);
			jpegComp->smoothing_factor	= (opq->suavizadoBase100 > 100 ? 100 : opq->suavizadoBase100);
			jpeg_set_quality(jpegComp, (opq->calidadBase100 > 100 ? 100 : opq->calidadBase100), TRUE);
			jpeg_start_compress(jpegComp, TRUE);
			opq->jpegCompStarted = true;
			r = ENJpegWriteResult_partial;
		}
		//Procesar contenido
		if(opq->jpegCompStarted && !opq->jpegCompFinished && opq->bufferDatos != NULL && opq->bufferDatosUso > 0){
			//Procesar lineas
			const UI16 cantLineasEnBuffer = (opq->bufferDatosUso / opq->propsMapaBits.bytesPerLine); NBASSERT((opq->bufferDatosUso % opq->propsMapaBits.bytesPerLine) == 0) //El buffer debe contener lineas completas
			if(cantLineasEnBuffer > 0){
				//Asegurar el buffer de punteros de lineas
				if(opq->punterosLineasTamano < cantLineasEnBuffer){
					if(opq->punterosLineas != NULL) NBMemory_free(opq->punterosLineas);
					opq->punterosLineas			= NBMemory_allocTypes(JSAMPLE*, cantLineasEnBuffer);
					opq->punterosLineasTamano	= cantLineasEnBuffer;
				}
				//Establecer punteros de lineas
				UI16 i;
				for(i = 0; i < cantLineasEnBuffer; i++){
					opq->punterosLineas[i] = (JSAMPLE*)&opq->bufferDatos[opq->propsMapaBits.bytesPerLine * i];
				}
				//Volcar lineas
				//while (opq->jpegComp.next_scanline < cantLineasEnBuffer) { //Pendiente, verificar si funciona con buffer parcial
				jpeg_write_scanlines(jpegComp, opq->punterosLineas, cantLineasEnBuffer);
				r = ENJpegWriteResult_partial;
				//}
				opq->lineasTotalVolcadas += cantLineasEnBuffer;
			}
			//Finalizar volcado
			//PRINTF_INFO("jpegCompFinished(%d) cantLineasEnBuffer(%d) opq->lineasTotalVolcadas(%d) opq->propsMapaBits.size.height(%d).\n", opq->jpegCompFinished, cantLineasEnBuffer, opq->lineasTotalVolcadas, opq->propsMapaBits.size.height);
			if(opq->lineasTotalVolcadas == opq->propsMapaBits.size.height){
				//Liberar arreglo de punteros de filas
				/* ToDo: delete this block of code
				 if(opq->punterosLineas != NULL){
				 NBMemory_free(opq->punterosLineas);
				 opq->punterosLineas			= NULL;
				 opq->punterosLineasTamano	= 0;
				 exito = true;
				 }*/
				//Finalizar compresor
				if(!opq->jpegCompFinished){
					jpeg_finish_compress(jpegComp);
					opq->jpegCompFinished = true;
					r = ENJpegWriteResult_end;
				}
			}
		}
	}
	return r;
}

void NBJpegWrite_feedEnd(STNBJpegWrite* obj){
	NBJpegWriteOpq* opq = (NBJpegWriteOpq*)obj->opaque;
	if(opq != NULL){
		NBJpegWrite_feedRelease_(opq, FALSE);
	}
}

void NBJpegWrite_feedRelease(STNBJpegWrite* obj){
	NBJpegWriteOpq* opq = (NBJpegWriteOpq*)obj->opaque;
	if(opq != NULL){
		NBJpegWrite_feedRelease_(opq, TRUE);
	}
}


/*void NBGlueLibJpegWrite_privStateInit(NBJpegWriteOpq* opq, const BYTE* data, const UI32 dataSz, const STNBBitmapProps dataFmt, STNBFileRef flujo, const UI8 calidadBase100, const UI8 suavizadoBase100, const bool createBuffers){
	
}*/

/*void NBGlueLibJpegWrite_privStateFinish(NBJpegWriteOpq* opq, const bool releaseBuffers){
	
}*/

void NBGlueLibJpegWrite_jpeg_out_init_destination(j_compress_ptr cinfo){
	NBASSERT((NBJpegWriteOpq*)cinfo->client_data != NULL)
	NBASSERT(NBFile_isSet(((NBJpegWriteOpq*)cinfo->client_data)->archivo))
}

boolean NBGlueLibJpegWrite_jpeg_out_empty_output_buffer(j_compress_ptr cinfo){
	NBJpegWriteOpq* opq = (NBJpegWriteOpq*)cinfo->client_data; NBASSERT(opq != NULL && NBFile_isSet(opq->archivo))
	struct jpeg_destination_mgr* jpegManager	= opq->jpegManager;
	if(NBFile_write(opq->archivo, opq->bufferEscritura, opq->bufferEscrituraTam) == opq->bufferEscrituraTam){
		jpegManager->next_output_byte	= opq->bufferEscritura;
		jpegManager->free_in_buffer		= opq->bufferEscrituraTam;
		return TRUE;
	}
	return FALSE;
}

void NBGlueLibJpegWrite_jpeg_out_term_destination(j_compress_ptr cinfo){
	NBJpegWriteOpq* opq = (NBJpegWriteOpq*)cinfo->client_data; NBASSERT(opq != NULL && NBFile_isSet(opq->archivo))
	struct jpeg_destination_mgr* jpegManager = opq->jpegManager;
	//Volcar datos restantes
	const SI32 bytesEnUso = (SI32)(opq->bufferEscrituraTam - jpegManager->free_in_buffer);
	if(bytesEnUso > 0){
		NBFile_write(opq->archivo, opq->bufferEscritura, bytesEnUso);
	}
}

//--------------------------
// Calls methods (implementations)
//--------------------------

/*void* NBGlueLibJpegWrite_stateCreate(void* param, const BYTE* data, const UI32 dataSz, const STNBBitmapProps dataFmt, STNBFileRef stream, const UI8 quality100, const UI8 smooth100){
	NBJpegWriteOpq* opq = NBMemory_allocType(NBJpegWriteOpq);
	NBGlueLibJpegWrite_privStateInit(opq, data, dataSz, dataFmt, stream, quality100, smooth100, true);
	return opq;
}*/

/*bool NBGlueLibJpegWrite_stateReset(void* param, void* pState, const BYTE* data, const UI32 dataSz, const STNBBitmapProps dataFmt,STNBFileRef stream, const UI8 quality100, const UI8 smooth100){
	bool r = false;
	NBJpegWriteOpq* opq = (NBJpegWriteOpq*)pState;
	if(opq != NULL){
		NBGlueLibJpegWrite_privStateInit(opq, data, dataSz, dataFmt, stream, quality100, smooth100, false);
		r = true;
	}
	return r;
}*/

/*void NBGlueLibJpegWrite_stateFinish(void* param, void* pState){
	NBJpegWriteOpq* opq = (NBJpegWriteOpq*)pState;
	if(opq != NULL){
		NBGlueLibJpegWrite_privStateFinish(opq, false);
	}
}*/

/*void NBGlueLibJpegWrite_stateDestroy(void* param, void* pState){
	NBJpegWriteOpq* opq = (NBJpegWriteOpq*)pState;
	if(opq != NULL){
		NBGlueLibJpegWrite_privStateFinish(opq, true);
		NBMemory_free(opq);
	}
}*/

/*ENJpegWriteResult NBGlueLibJpegWrite_stateWrite(void* param, void* pState){
	
}*/
