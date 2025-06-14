//
//  NBAvcDecoderApple.c
//  nbframework
//
//  Created by Marcos Ortega on 9/21/20.
//

#include "nb/NBFrameworkPch.h"
#include "nb/2d/NBAvcDecoderApple.h"
//
#include "nb/core/NBMemory.h"
#include "nb/2d/NBBitmap.h"
#include "nb/2d/NBAvc.h"
//
#import <VideoToolbox/VideoToolbox.h>
//-----
//- https://developer.apple.com/documentation/videotoolbox/vtdecompressionsession?language=objc
//-----

typedef struct STNBAvcDecoderAppleOpq_ {
	BOOL dummy;
} STNBAvcDecoderAppleOpq;

#define NBAVC_DECOMP_APPLE_CTX_HEAD8	17
#define NBAVC_DECOMP_APPLE_CTX_FOOT8	37

typedef struct STNBAvcDecoderAppleCtx_ {
	UI8					head;
	//fmt (format description)
	struct {
		//sps (Sequence parameter set, NALU = 7)
		struct {
			void*		data;
			UI32		size;
		} sps;
		//pps (Picture parameter set, NALU = 8)
		struct {
			void*		data;
			UI32		size;
		} pps;
		//desc
		CMVideoFormatDescriptionRef desc;
	} fmt;
	//images
	struct {
		//idr
		struct {
			BOOL		found;
		} idr;
		//input
		struct {
			CMBlockBufferRef	block;
			CMSampleBufferRef	sample;
		} input;
		//output 
		struct {
			BOOL				isPopulated;
			UI32				seqNum;
			STNBBitmap			sample;
		} output;
	} imgs;
	VTDecompressionSessionRef	session;
	UI8					foot;
} STNBAvcDecoderAppleCtx;

//

void	NBAvcDecoderApple_create_(void** obj);
void	NBAvcDecoderApple_destroy_(void** obj);
//Context
void*	NBAvcDecoderApple_ctxCreate_(void* obj);
void	NBAvcDecoderApple_ctxDestroy_(void* obj, void* ctx);
BOOL	NBAvcDecoderApple_ctxFeedUnit_(void* obj, void* ctx, void* data, const UI32 dataSz);

void NBAvcDecoderApple_init(STNBAvcDecoderApple* obj){
	STNBAvcDecoderAppleOpq* opq = NBMemory_allocType(STNBAvcDecoderAppleOpq);
	NBMemory_setZeroSt(*opq, STNBAvcDecoderAppleOpq);
	obj->opaque = opq;
}

void NBAvcDecoderApple_release(STNBAvcDecoderApple* obj){
	STNBAvcDecoderAppleOpq* opq = (STNBAvcDecoderAppleOpq*)obj->opaque;
	//Release
	NBMemory_free(opq);
	obj->opaque = NULL;
}


void* NBAvcDecoderApple_ctxCreate(STNBAvcDecoderApple* obj){
	//STNBAvcDecoderAppleOpq* opq	= (STNBAvcDecoderAppleOpq*)obj->opaque;
	STNBAvcDecoderAppleCtx* ctx	= NBMemory_allocType(STNBAvcDecoderAppleCtx);
	NBMemory_setZeroSt(*ctx, STNBAvcDecoderAppleCtx);
	ctx->head = NBAVC_DECOMP_APPLE_CTX_HEAD8;
	ctx->foot = NBAVC_DECOMP_APPLE_CTX_FOOT8;
	//imgs
	{
		//output
		{
			NBBitmap_init(&ctx->imgs.output.sample);
		}
	}
	//
	return ctx;
}

void NBAvcDecoderApple_ctxDestroy(STNBAvcDecoderApple* obj, void* pCtx){
	//STNBAvcDecoderAppleOpq* opq = (STNBAvcDecoderAppleOpq*)obj->opaque;
	STNBAvcDecoderAppleCtx* ctx = (STNBAvcDecoderAppleCtx*)pCtx;
	NBASSERT(ctx == NULL || (ctx->head == NBAVC_DECOMP_APPLE_CTX_HEAD8 && ctx->foot == NBAVC_DECOMP_APPLE_CTX_FOOT8)) //Invalid pointer (released more than once?)
	if(ctx != NULL && ctx->head == NBAVC_DECOMP_APPLE_CTX_HEAD8 && ctx->foot == NBAVC_DECOMP_APPLE_CTX_FOOT8){
		if(ctx->fmt.desc != NULL){
			CFRelease(ctx->fmt.desc);
			ctx->fmt.desc = NULL;
		}
		if(ctx->session != NULL){
			CFRelease(ctx->session);
			ctx->session = NULL;
		}
		//imgs
		{
			//output
			{
				ctx->imgs.output.isPopulated = FALSE;
				NBBitmap_release(&ctx->imgs.output.sample);
			}
		}
		//free
		ctx->head = 0;
		ctx->foot = 0;
		NBMemory_free(ctx);
		ctx = NULL;
	}
}

void NBAvcDecoderApple_decompressionOutputCallback_(void *decompressionOutputRefCon, void *sourceFrameRefCon, OSStatus status, VTDecodeInfoFlags infoFlags, CVImageBufferRef imageBuffer, CMTime presentationTimeStamp, CMTime presentationDuration){
	STNBAvcDecoderAppleCtx* ctx = (STNBAvcDecoderAppleCtx*)decompressionOutputRefCon;
	if (status != noErr){
		PRINTF_ERROR("NBAvcDecoderApple, decompressionOutputCallback error(%d).\n", status);
	} else {
		PRINTF_ERROR("NBAvcDecoderApple, decompressionOutputCallback success.\n", status);
		//Extract image
		CVPixelBufferLockBaseAddress(imageBuffer, 0);
		{
			size_t bytesPerRow	= CVPixelBufferGetBytesPerRow(imageBuffer);
			size_t width		= CVPixelBufferGetWidth(imageBuffer);
			size_t height		= CVPixelBufferGetHeight(imageBuffer);
			OSType pxFmtType	= CVPixelBufferGetPixelFormatType(imageBuffer);
			void* baseAddress	= CVPixelBufferGetBaseAddress(imageBuffer);
			STNBBitmapProps props;
			NBMemory_setZeroSt(props, STNBBitmapProps);
			props.size.width	= (SI32)width;
			props.size.height	= (SI32)height;
			props.bytesPerLine	= (SI32)bytesPerRow;
			switch (pxFmtType) {
				case kCVPixelFormatType_24RGB:
					props.bitsPerPx = 24;
					props.color		= ENNBBitmapColor_RGB8;
					break;
				case kCVPixelFormatType_32ABGR:
					props.bitsPerPx = 32;
					props.color		= ENNBBitmapColor_ARGB8;
					break;	
				case kCVPixelFormatType_32RGBA:
					props.bitsPerPx = 32;
					props.color		= ENNBBitmapColor_RGBA8;
					break;
				default:
					break;
			}
			if(props.color == 0 || props.bitsPerPx <= 0 || width <= 0 || height <= 0 || bytesPerRow <= 0){
				PRINTF_ERROR("NBAvcDecoderApple, decompressionOutputCallback unexpected-props returned sz(%d x %d, %d bytes-per-row) fmt(%d aka '%c%c%c%c').\n", width, height, bytesPerRow, pxFmtType, ((char*)&pxFmtType)[3], ((char*)&pxFmtType)[2], ((char*)&pxFmtType)[1], ((char*)&pxFmtType)[0]);
			} else if(!NBBitmap_createWithBitmapData(&ctx->imgs.output.sample, props.color, props, baseAddress, NBST_P(STNBColor8, 255, 255, 255, 255))){
				PRINTF_ERROR("NBAvcDecoderApple, decompressionOutputCallback createWithBitmapData failed for sz(%d x %d, %d bytes-per-row) fmt(%d aka '%c%c%c%c').\n", width, height, bytesPerRow, pxFmtType, ((char*)&pxFmtType)[3], ((char*)&pxFmtType)[2], ((char*)&pxFmtType)[1], ((char*)&pxFmtType)[0]);
			} else {
				ctx->imgs.output.seqNum++;
				ctx->imgs.output.isPopulated = TRUE;
				PRINTF_INFO("NBAvcDecoderApple, decompressionOutputCallback returned sz(%d x %d, %d bytes-per-row) fmt(%d aka '%c%c%c%c').\n", width, height, bytesPerRow, pxFmtType, ((char*)&pxFmtType)[3], ((char*)&pxFmtType)[2], ((char*)&pxFmtType)[1], ((char*)&pxFmtType)[0]);
			}
		}
		CVPixelBufferUnlockBaseAddress(imageBuffer,0);
	}
}

BOOL NBAvcDecoderApple_ctxFeedUnit(STNBAvcDecoderApple* obj, void* pCtx, void* data, const UI32 dataSz){
	BOOL r = FALSE;
	//STNBAvcDecoderAppleOpq* opq = (STNBAvcDecoderAppleOpq*)obj->opaque;
	STNBAvcDecoderAppleCtx* ctx = (STNBAvcDecoderAppleCtx*)pCtx;
	NBASSERT(ctx == NULL || (ctx->head == NBAVC_DECOMP_APPLE_CTX_HEAD8 && ctx->foot == NBAVC_DECOMP_APPLE_CTX_FOOT8)) //Invalid pointer (released more than once?)
	if(ctx != NULL && ctx->head == NBAVC_DECOMP_APPLE_CTX_HEAD8 && ctx->foot == NBAVC_DECOMP_APPLE_CTX_FOOT8){
		if(dataSz == 0){
			r = TRUE;
		} else if(data != NULL && dataSz > 4){
			BYTE* bData = (BYTE*)data;
			//Expecting '[0x00, 0x00, 0x00, 0x01]' at start
			if(bData[0] == 0x00 && bData[1] == 0x00 && bData[2] == 0x00 && bData[3] == 0x01){
				const STNBAvcNaluHdr desc = NBAvc_getNaluHdr(bData[4]);
				if(ctx->fmt.desc == NULL){
					//Copy nalus
					if(desc.type == 7){
						//copy sps (Sequence parameter set, NALU = 7) 
						if(ctx->fmt.sps.data != NULL){
							NBMemory_free(ctx->fmt.sps.data);
						}
						ctx->fmt.sps.size = dataSz - 4;
						ctx->fmt.sps.data = NBMemory_alloc(ctx->fmt.sps.size);
						NBMemory_copy(ctx->fmt.sps.data, &bData[4], ctx->fmt.sps.size);
						r = TRUE;
					} else if(desc.type == 8){
						//copy pps (Picture parameter set, NALU = 8)
						if(ctx->fmt.pps.data != NULL){
							NBMemory_free(ctx->fmt.pps.data);
						}
						ctx->fmt.pps.size = dataSz - 4;
						ctx->fmt.pps.data = NBMemory_alloc(ctx->fmt.pps.size);
						NBMemory_copy(ctx->fmt.pps.data, &bData[4], ctx->fmt.pps.size);
						r = TRUE;
					}
					//create desc
					if(r && ctx->fmt.sps.data != NULL && ctx->fmt.pps.data != NULL){
						int nalUnitHeaderLength = 4; //1, 2 or 4
						CMVideoFormatDescriptionRef fmtDesc = NULL;
						VTDecompressionSessionRef	session = NULL;
						uint8_t* ptrs[2] = { ctx->fmt.sps.data, ctx->fmt.pps.data };
						size_t ptrsSz[2] = { ctx->fmt.sps.size, ctx->fmt.pps.size };
						OSStatus ret = CMVideoFormatDescriptionCreateFromH264ParameterSets(kCFAllocatorDefault, 2, (const uint8_t* const*)ptrs, ptrsSz, nalUnitHeaderLength, &fmtDesc);
						if(ret != noErr){
							PRINTF_ERROR("NBAvcDecoderApple, CMVideoFormatDescriptionCreateFromH264ParameterSets returned error(%d).\n", ret);
							r = FALSE;
						} else if(ctx->session == NULL){ 
							//create session
							CFMutableDictionaryRef attribs = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
							VTDecompressionOutputCallbackRecord callBackRecord;
							NBMemory_setZeroSt(callBackRecord, VTDecompressionOutputCallbackRecord);
							callBackRecord.decompressionOutputRefCon	= ctx;
							callBackRecord.decompressionOutputCallback	= NBAvcDecoderApple_decompressionOutputCallback_;
							//config attribs
							{
								OSType outFmt = kCVPixelFormatType_24RGB;
								CFDictionarySetValue(attribs, kCVPixelBufferPixelFormatTypeKey, CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &outFmt));
								//CFDictionarySetValue(attribs, kCVPixelBufferWidthKey, ...);
								//CFDictionarySetValue(attribs, kCVPixelBufferHeightKey, ...);
								//CFDictionarySetValue(attribs, kCVPixelBufferOpenGLCompatibilityKey, ...);
							}
							//
							ret = VTDecompressionSessionCreate(NULL, fmtDesc, NULL, attribs, &callBackRecord, &session);
							if(ret != noErr){
								PRINTF_ERROR("NBAvcDecoderApple, VTDecompressionSessionCreate returned error(%d).\n", ret);
								r = FALSE;
							} else {
								//Consume results
								if(ctx->fmt.desc != NULL){
									CFRelease(ctx->fmt.desc);
									ctx->fmt.desc = NULL;
								}
								if(ctx->session != NULL){
									CFRelease(ctx->session);
									ctx->session = NULL;
								}
								ctx->fmt.desc	= fmtDesc; fmtDesc = NULL;
								ctx->session	= session; session = NULL;
								PRINTF_INFO("NBAvcDecoderApple, VTDecompressionSessionCreate success.\n", ret);
								r = TRUE;
							}
							CFRelease(attribs);
						}
						//Release (if not consumed)
						if(fmtDesc != NULL){
							CFRelease(fmtDesc);
							fmtDesc = NULL;
						}
						if(session != NULL){
							CFRelease(session);
							session = NULL;
						}
					}
				} else if(dataSz > 4 && (desc.type == 5 || (desc.type == 1 && ctx->imgs.idr.found))){ //IDR image
					OSStatus ret;
					//change 4-bytes-header from [0x00 0x00 0x00 0x00] to len (Apple format)
					const uint32_t lenHdrOrg = *((uint32_t*)data);
 					const uint32_t lenHdr = htonl(dataSz - 4);
					memcpy(data, &lenHdr, sizeof(lenHdr));
					//
					ret = CMBlockBufferCreateWithMemoryBlock(NULL, data, (size_t)dataSz, kCFAllocatorNull, NULL, 0, (size_t)dataSz, 0, &ctx->imgs.input.block);
					if(ret != noErr){
						PRINTF_ERROR("NBAvcDecoderApple, CMBlockBufferCreateWithMemoryBlock returned error(%d).\n", ret);
						r = FALSE;
					} else {
						//
						size_t sampleSizeArray = (CMItemCount)dataSz;
						ret = CMSampleBufferCreate(kCFAllocatorDefault, ctx->imgs.input.block, true, NULL, NULL, ctx->fmt.desc, 1, 0, NULL, 1, &sampleSizeArray, &ctx->imgs.input.sample);
						if(ret != noErr){
							PRINTF_ERROR("NBAvcDecoderApple, CMSampleBufferCreate returned error(%d).\n", ret);
							r = FALSE;
						} else {
							// set some values of the sample buffer's attachments
							CFArrayRef attachments = CMSampleBufferGetSampleAttachmentsArray(ctx->imgs.input.sample, YES);
							CFMutableDictionaryRef dict = (CFMutableDictionaryRef)CFArrayGetValueAtIndex(attachments, 0);
							CFDictionarySetValue(dict, kCMSampleAttachmentKey_DisplayImmediately, kCFBooleanTrue);
							//Render
							VTDecodeFrameFlags flagsIn = 0; //0 = decompression will be executed in current thread
							ret = VTDecompressionSessionDecodeFrame(ctx->session, ctx->imgs.input.sample, flagsIn, NULL, NULL);
							if(ret != noErr){
								PRINTF_ERROR("NBAvcDecoderApple, VTDecompressionSessionDecodeFrame returned error(%d).\n", ret);
								r = FALSE;
							} else {
								//Extract image
								/*size_t bytesPerRow, width, height; void *baseAddress; OSType pxFmtType;
								//CGColorSpaceRef colorSpace; CGContextRef context; 
								CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(ctx->imgs.input.sample);
								CVPixelBufferLockBaseAddress(imageBuffer, 0);
								{
									baseAddress	= CVPixelBufferGetBaseAddress(imageBuffer);
									bytesPerRow	= CVPixelBufferGetBytesPerRow(imageBuffer);
									width		= CVPixelBufferGetWidth(imageBuffer);
									height		= CVPixelBufferGetHeight(imageBuffer);
									pxFmtType	= CVPixelBufferGetPixelFormatType(imageBuffer);
									//kCVPixelFormatType_24BGR
									PRINTF_INFO("NBAvcDecoderApple, sample returned sz(%d x %d, %d bytes-per-row) fmt('%c%c%c%c').\n", width, height, bytesPerRow, ((char*)&pxFmtType)[3], ((char*)&pxFmtType)[2], ((char*)&pxFmtType)[1], ((char*)&pxFmtType)[0]);
									colorSpace	= CGColorSpaceCreateDeviceRGB();
									 context		= CGBitmapContextCreate(baseAddress, 
									 width, 
									 height, 
									 8, 
									 bytesPerRow, 
									 colorSpace, 
									 kCGBitmapByteOrder32Little 
									 | kCGImageAlphaPremultipliedFirst);
									 // Create a Quartz image from the pixel data in the bitmap graphics context
									 CGImageRef quartzImage = CGBitmapContextCreateImage(context);
									 // Unlock the pixel buffer
									 CVPixelBufferUnlockBaseAddress(imageBuffer,0);
									 
									 // Free up the context and color space
									 CGContextRelease(context);
									 CGColorSpaceRelease(colorSpace);
									 
									 // Create an image object from the Quartz image
									 //UIImage *image = [UIImage imageWithCGImage:quartzImage];
									 UIImage *image = [UIImage imageWithCGImage:quartzImage 
									 scale:1.0f 
									 orientation:UIImageOrientationRight];
									 
									 // Release the Quartz image
									 CGImageRelease(quartzImage);
								}
								CVPixelBufferUnlockBaseAddress(imageBuffer,0);*/
								//
								if(desc.type == 5){
									ctx->imgs.idr.found = TRUE;
								}
								r = TRUE;
							}
							CFRelease(ctx->imgs.input.sample);
						}
						CFRelease(ctx->imgs.input.block);
					}
					//reset 4-bytes-header
					memcpy(data, &lenHdrOrg, sizeof(lenHdrOrg));
				}
			}
		}
	}
	return r;
}

BOOL NBAvcDecoderApple_ctxGetBuffer(STNBAvcDecoderApple* obj, void* pCtx, UI32* seqFilterAndDst, STNBBitmap* dst, const BOOL swapBuffersInsteadOfCloning){
	BOOL r = FALSE;
	//STNBAvcDecoderAppleOpq* opq = (STNBAvcDecoderAppleOpq*)obj->opaque;
	STNBAvcDecoderAppleCtx* ctx = (STNBAvcDecoderAppleCtx*)pCtx;
	NBASSERT(ctx == NULL || (ctx->head == NBAVC_DECOMP_APPLE_CTX_HEAD8 && ctx->foot == NBAVC_DECOMP_APPLE_CTX_FOOT8)) //Invalid pointer (released more than once?)
	if(ctx != NULL && ctx->head == NBAVC_DECOMP_APPLE_CTX_HEAD8 && ctx->foot == NBAVC_DECOMP_APPLE_CTX_FOOT8){
		if(dst != NULL){
			if(ctx->imgs.output.isPopulated && (seqFilterAndDst == NULL || *seqFilterAndDst != ctx->imgs.output.seqNum)){
				const STNBBitmapProps props = NBBitmap_getProps(&ctx->imgs.output.sample);
				if(props.size.width > 0 && props.size.height > 0 && props.bytesPerLine > 0 && props.bitsPerPx > 0 && props.color > 0){
					if(swapBuffersInsteadOfCloning){
						if(NBBitmap_swapData(dst, &ctx->imgs.output.sample)){
							if(seqFilterAndDst != NULL){
								*seqFilterAndDst = ctx->imgs.output.seqNum; 
							}
							ctx->imgs.output.isPopulated = FALSE;
							r = TRUE;
						}
					} else {
						if(NBBitmap_createWithBitmap(dst, props.color, &ctx->imgs.output.sample, NBST_P(STNBColor8, 255, 255, 255, 255))){
							if(seqFilterAndDst != NULL){
								*seqFilterAndDst = ctx->imgs.output.seqNum; 
							}
							r = TRUE;
						}
					}
				}
			}
		}
	}
	return r;
}

//Interface

STNBAVDecompItf NBAvcDecoderApple_getItf(void){
	STNBAVDecompItf r;
	NBMemory_setZeroSt(r, STNBAVDecompItf);
	r.create		= NBAvcDecoderApple_create_;
	r.destroy		= NBAvcDecoderApple_destroy_;
	//context
	r.ctxCreate		= NBAvcDecoderApple_ctxCreate_;
	r.ctxDestroy	= NBAvcDecoderApple_ctxDestroy_;
	//
	return r;
}

void NBAvcDecoderApple_create_(void** obj){
	STNBAvcDecoderApple* itfParam = NBMemory_allocType(STNBAvcDecoderApple);
	NBAvcDecoderApple_init(itfParam);
	*obj = itfParam;
}

void NBAvcDecoderApple_destroy_(void** obj){
	STNBAvcDecoderApple* itfParam = (STNBAvcDecoderApple*)*obj;
	NBAvcDecoderApple_release(itfParam);
	NBMemory_free(itfParam);
	*obj = NULL;
}

//Context

void* NBAvcDecoderApple_ctxCreate_(void* obj){
	return NBAvcDecoderApple_ctxCreate((STNBAvcDecoderApple*)obj);
}

void NBAvcDecoderApple_ctxDestroy_(void* obj, void* ctx){
	NBAvcDecoderApple_ctxDestroy((STNBAvcDecoderApple*)obj, ctx);
}

BOOL NBAvcDecoderApple_ctxFeedUnit_(void* obj, void* ctx, void* data, const UI32 dataSz){
	return NBAvcDecoderApple_ctxFeedUnit((STNBAvcDecoderApple*)obj, ctx, data, dataSz);
}
