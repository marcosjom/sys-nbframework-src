#ifndef NB_RTP_PARSER_H
#define NB_RTP_PARSER_H

#include "nb/NBFrameworkDefs.h"
#include "nb/net/NBRtp.h"

//

#ifdef __cplusplus
extern "C" {
#endif
	
	//-----------------
	//-- RTP - RFC3550
	//-- https://tools.ietf.org/html/rfc3550
	//-----------------
	
	typedef struct STNBRtpHdrBasic_ {
		//Header
		STNBRtpPacketHead	head;
		//Payload
		struct {
			UI32			iStart;		//zero if not set
			UI32			sz;
		} payload;
	} STNBRtpHdrBasic;

#	define NBRtpHdrBasic_init(PTR)					NBMemory_setZeroSt(*(PTR), STNBRtpHdrBasic)
#	define NBRtpHdrBasic_release(PTR)				//Nothing
#	define NBRtpHdrBasic_preResign(OBJ)				//Nothing
#	define NBRtpHdrBasic_hdrsRelease(PTR, COUNT)	//Nothing
#	define NBRtpHdrBasic_copy(PTR, OTHER)			*(PTR) = *(OTHER)
	BOOL NBRtpHdrBasic_parse(STNBRtpHdrBasic* obj, const void* data, const UI32 dataSz);

	//-----------------
	//-- RTP - RFC3550
	//-- https://tools.ietf.org/html/rfc3550
	//-----------------

	typedef struct STNBRtpHeader_ {
		//Header
		struct {
			STNBRtpPacketHead	def;
			UI32*				csrcs;
			UI32				csrcsSz;
		} head;
		//Header extension
		struct {
			STNBRtpPacketHeadExt def;	
			UI32				iStart;	//zero if not set
			UI32				sz;
		} headExt;
		//Payload
		struct {
			UI32				iStart; //zero if not set
			UI32				sz;
		} payload;
		//Optimization (reduces locks and internal data cloning)
		BOOL					preResigned;	//owner flagged internal data as 'free to take by other'
		BOOL					preResignTaken;	//other took internal data as own (not)
	} STNBRtpHeader;
	
	//

	void NBRtpHeader_init(STNBRtpHeader* obj);
	void NBRtpHeader_release(STNBRtpHeader* obj);
	void NBRtpHeader_preResign(STNBRtpHeader* obj); //Optimization (reduces locks and internal data cloning)

	//
	
	BOOL NBRtpHeader_parse(STNBRtpHeader* obj, const void* data, const UI32 dataSz);
	BOOL NBRtpHeader_copy(STNBRtpHeader* obj, const STNBRtpHeader* other);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
