

#ifndef NBMp4_h
#define NBMp4_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBArray.h"
#include "nb/files/NBFile.h"

#ifdef __cplusplus
extern "C" {
#endif

//General use
#define NBMP4_SECS_FROM_1901_TO_UTC1970	2082844800
#define NBMp4_isChar4(SRC, CMP)			(NBString_strIsEqualBytes(SRC, 4, CMP, 4))
//file-read
BOOL NBMp4_readUI32File(UI32* dst, STNBFileRef f);
BOOL NBMp4_readBytesFile(void* dst, const UI32 bytes, STNBFileRef f);
//buff-read
BOOL NBMp4_readSI16(SI16* dst, const BYTE** buff, const BYTE* buffAfterEnd);
BOOL NBMp4_readUI16(UI16* dst, const BYTE** buff, const BYTE* buffAfterEnd);
BOOL NBMp4_readSI32(SI32* dst, const BYTE** buff, const BYTE* buffAfterEnd);
BOOL NBMp4_readUI32(UI32* dst, const BYTE** buff, const BYTE* buffAfterEnd);
BOOL NBMp4_readSI64(SI64* dst, const BYTE** buff, const BYTE* buffAfterEnd);
BOOL NBMp4_readUI64(UI64* dst, const BYTE** buff, const BYTE* buffAfterEnd);
BOOL NBMp4_readSI8s(SI8* dst, const UI32 amm, const BYTE** buff, const BYTE* buffAfterEnd);
BOOL NBMp4_readUI8s(UI8* dst, const UI32 amm, const BYTE** buff, const BYTE* buffAfterEnd);
BOOL NBMp4_readSI16s(SI16* dst, const UI32 amm, const BYTE** buff, const BYTE* buffAfterEnd);
BOOL NBMp4_readUI16s(UI16* dst, const UI32 amm, const BYTE** buff, const BYTE* buffAfterEnd);
BOOL NBMp4_readSI32s(SI32* dst, const UI32 amm, const BYTE** buff, const BYTE* buffAfterEnd);
BOOL NBMp4_readUI32s(UI32* dst, const UI32 amm, const BYTE** buff, const BYTE* buffAfterEnd);
BOOL NBMp4_readBytes(void* dst, const UI32 bytes, const BYTE** buff, const BYTE* buffAfterEnd);
BOOL NBMp4_readString(char** dst, const BYTE** buff, const BYTE* buffAfterEnd);
//buff-write
BOOL NBMp4_writeSI16(const SI16* src, STNBString* dst);
BOOL NBMp4_writeUI16(const UI16* src, STNBString* dst);
BOOL NBMp4_writeSI32(const SI32* src, STNBString* dst);
BOOL NBMp4_writeUI32(const UI32* src, STNBString* dst);
BOOL NBMp4_writeUI32At(const UI32* src, void* dst);
BOOL NBMp4_writeSI64(const SI64* src, STNBString* dst);
BOOL NBMp4_writeUI64(const UI64* src, STNBString* dst);
BOOL NBMp4_writeSI8s(const SI8* src, const UI32 amm, STNBString* dst);
BOOL NBMp4_writeUI8s(const UI8* src, const UI32 amm, STNBString* dst);
BOOL NBMp4_writeSI16s(const SI16* src, const UI32 amm, STNBString* dst);
BOOL NBMp4_writeUI16s(const UI16* src, const UI32 amm, STNBString* dst);
BOOL NBMp4_writeSI32s(const SI32* src, const UI32 amm, STNBString* dst);
BOOL NBMp4_writeUI32s(const UI32* src, const UI32 amm, STNBString* dst);
BOOL NBMp4_writeBytes(const void* src, const UI32 bytes, STNBString* dst);
BOOL NBMp4_writeString(const char* src, STNBString* dst);

//ISO-IEC-14496-12

struct STNBMp4Box_;

//NBMp4Id4

typedef struct STNBMp4Id4_ {
	union {
		BYTE str[4];
		UI32 ui32;
	};
} STNBMp4Id4;

//NBMp4Data

typedef struct STNBMp4Data_ {
	BYTE*	pay;
	UI32	use;
	UI32	allocSz;
} STNBMp4Data;

void NBMp4Data_init(STNBMp4Data* obj);
void NBMp4Data_release(STNBMp4Data* obj);

//NBMp4BoxHdr

typedef struct STNBMp4BoxHdr_ {
	UI32		size;
	STNBMp4Id4	type;
	UI64		largeSize;		//if size == 1
	BYTE		userType[16];	//if type == 'uuid'
	//
	UI32		lastWritePos;	//last index in STNBString last time written
} STNBMp4BoxHdr;

void NBMp4BoxHdr_init(STNBMp4BoxHdr* obj);
void NBMp4BoxHdr_release(STNBMp4BoxHdr* obj);
BOOL NBMp4BoxHdr_loadBitsFromFile(STNBMp4BoxHdr* obj, STNBFileRef f);
BOOL NBMp4BoxHdr_loadBits(STNBMp4BoxHdr* obj, const BYTE** buff, const BYTE* buffAfterEnd);
void NBMp4BoxHdr_concat(const STNBMp4BoxHdr* obj, const UI32 depthLvl, STNBString* dst);
UI32 NBMp4BoxHdr_getHdrBytesLen(const STNBMp4BoxHdr* obj);
BOOL NBMp4BoxHdr_writeBits(STNBMp4BoxHdr* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst);
BOOL NBMp4BoxHdr_writeHdrFinalSize(const STNBMp4BoxHdr* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst);
BOOL NBMp4BoxHdr_writeHdrFinalSizeExplicit(const STNBMp4BoxHdr* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst, const UI32 dstFinalSize);

typedef struct STNBMp4BoxStDef_ {
	UI32		size;
	struct {
		void	(*init)(void* obj);
		void	(*release)(void* obj);
		BOOL	(*loadBits)(void* obj, struct STNBMp4Box_* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd);
		void	(*concat)(const void* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst);
		BOOL	(*writeBits)(void* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst);
	} itf;
} STNBMp4BoxStDef;

#define NBMP4_BOX_PUBLIC_METHODS_DECLARE(BOX_NAME) \
	void NBMp4 ## BOX_NAME ## _init(STNBMp4 ## BOX_NAME* obj); \
	void NBMp4 ## BOX_NAME ## _release(STNBMp4 ## BOX_NAME* obj); \
	BOOL NBMp4 ## BOX_NAME ## _loadFromBuff(STNBMp4 ## BOX_NAME* obj, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd); \
	void NBMp4 ## BOX_NAME ## _concat(const STNBMp4 ## BOX_NAME* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst); \
	BOOL NBMp4 ## BOX_NAME ## _writeBits(STNBMp4 ## BOX_NAME* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst); \
	BOOL NBMp4 ## BOX_NAME ## _allocRef(STNBMp4BoxRef* dst); \

#define NBMP4_BOX_NULL_ST_DEF \
	{ \
		0 \
		, { \
			NULL \
			, NULL \
			, NULL \
			, NULL \
			, NULL \
		} \
	}

#define NBMP4_BOX_PARSEABLE_ST_DEF(BOX_NAME) \
	{ \
		sizeof(STNBMp4 ## BOX_NAME) \
		, { \
			NBMp4 ## BOX_NAME ##_init_ \
			, NBMp4 ## BOX_NAME ##_release_ \
			, NBMp4 ## BOX_NAME ##_loadBits_ \
			, NBMp4 ## BOX_NAME ##_concat_ \
			, NBMp4 ## BOX_NAME ##_writeBits_ \
		} \
	}

void NBMp4BoxStDef_init(STNBMp4BoxStDef* obj);
void NBMp4BoxStDef_release(STNBMp4BoxStDef* obj);

//NBMp4BoxRef

typedef struct STNBMp4BoxRef_ {
	STNBMp4BoxStDef		stDef;
	struct STNBMp4Box_*	box;
} STNBMp4BoxRef;

#define NBMP4_BOX_PARSEABLE_DEF(C0, C1, C2, C3, BOX_NAME) \
	{ \
		.boxId.str = { C0, C1, C2, C3 } \
		, ENNBMp4BoxTypeGrp_Parseable \
		, ENNBMp4BoxReadHint_Memory \
		, NBMP4_BOX_PARSEABLE_ST_DEF(BOX_NAME) \
		, { NULL, 0 } \
	} \

#define NBMP4_BOX_PARSEABLE_FILE_PREFERED_DEF(C0, C1, C2, C3) \
	{ \
		.boxId.str = { C0, C1, C2, C3 } \
		, ENNBMp4BoxTypeGrp_Parseable \
		, ENNBMp4BoxReadHint_File \
		, NBMP4_BOX_NULL_ST_DEF \
		, { NULL, 0 } \
	} \


#define NBMP4_BOX_CONTAINER_DEF(C0, C1, C2, C3, BOX_NAME) \
	{ \
		.boxId.str = { C0, C1, C2, C3 } \
		, ENNBMp4BoxTypeGrp_Container \
		, ENNBMp4BoxReadHint_Memory \
		, NBMP4_BOX_PARSEABLE_ST_DEF(BOX_NAME) \
		, { NBMp4 ## BOX_NAME ## Chldrn_, (sizeof(NBMp4 ## BOX_NAME ## Chldrn_) / sizeof(NBMp4 ## BOX_NAME ## Chldrn_[0])) } \
	} \

void NBMp4BoxRef_init(STNBMp4BoxRef* obj);
void NBMp4BoxRef_release(STNBMp4BoxRef* obj);
BOOL NBMp4BoxRef_allocBoxFromFile(STNBMp4BoxRef* dst, struct STNBMp4Box_* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, STNBFileRef f);
BOOL NBMp4BoxRef_allocBoxFromBuff(STNBMp4BoxRef* dst, struct STNBMp4Box_* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd);
void NBMp4BoxRef_concat(const STNBMp4BoxRef* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst);
BOOL NBMp4BoxRef_writeBits(STNBMp4BoxRef* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst);

//Definitions

typedef enum ENNBMp4BoxReadHint_ {
	ENNBMp4BoxReadHint_Memory = 0,	//can be loaded in memory (metadata)
	ENNBMp4BoxReadHint_File,		//should be read from file, if posible (payload)
	//Count
	ENNBMp4BoxReadHint_Count
} ENNBMp4BoxReadHint;

typedef enum ENNBMp4BoxTypeGrp_ {
	ENNBMp4BoxTypeGrp_Parseable = 0,	//box contains data
	ENNBMp4BoxTypeGrp_Container,		//box contains other boxes
	//Count
	ENNBMp4BoxTypeGrp_Count
} ENNBMp4BoxTypeGrp;

typedef struct STNBMp4BoxChildDef_ {
	STNBMp4Id4			boxId;
	const BYTE*			stPos;		//offset (stPos - stBase) in parent-struct to children-pointer
	const BYTE*			stBase;		//offset (stPos - stBase) in parent-struct to children-pointer
} STNBMp4BoxChildDef;

typedef struct STNBMp4BoxDef_ {
	STNBMp4Id4			boxId;
	ENNBMp4BoxTypeGrp	typeGrp;
	ENNBMp4BoxReadHint	readHint;
	//struct load/write
	STNBMp4BoxStDef		stDef;
	//children (if container)
	struct {
		const STNBMp4BoxChildDef* defs;
		UI32			defsSz;
	} children;
} STNBMp4BoxDef;

const STNBMp4BoxDef* NBMp4_getBoxDef(const BYTE id4[4]); 

//NBMp4Box

typedef struct STNBMp4Box_ {
	STNBMp4BoxHdr		hdr;
	struct STNBMp4Box_*	prnt;
	STNBArray			subBoxes;	//STNBMp4BoxRef
	//filePos
	struct {
		UI64			iStart;
		UI32			hdrSz;		//ToDo: define as UI8
		UI32			size;
	} filePos;
} STNBMp4Box;

void NBMp4Box_init(STNBMp4Box* obj);
void NBMp4Box_release(STNBMp4Box* obj);
void NBMp4Box_cloneHdr(STNBMp4Box* obj, STNBMp4Box* prnt, const UI64 boxFileStart, const UI8 hdrFileSz, const UI32 payFileSz, const STNBMp4BoxHdr* hdr);
void NBMp4Box_addChildBoxByOwning(STNBMp4Box* obj, STNBMp4BoxRef* refToOwn);
void NBMp4Box_concat(const STNBMp4Box* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst);
void NBMp4Box_concatHdr(const STNBMp4Box* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst);
void NBMp4Box_concatChildren(const STNBMp4Box* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst, const UI32 stSz, const STNBMp4BoxChildDef* cDefs, const UI32 cDefsSz);
BOOL NBMp4Box_writeBits(STNBMp4Box* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst);
BOOL NBMp4Box_writeHdrBits(STNBMp4Box* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst);
BOOL NBMp4Box_writeHdrFinalSize(const STNBMp4Box* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst);
BOOL NBMp4Box_allocBoxWithDef(STNBMp4BoxStDef* dstStDef, STNBMp4Box** dstBox, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd, const STNBMp4BoxDef* def);
STNBMp4Box* NBMp4Box_alloctIfMatchDef(const STNBMp4BoxDef* def, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const BYTE** buff, const BYTE* buffAfterEnd);

//STNBMp4FullBox

typedef struct STNBMp4FullBox_ {
	STNBMp4Box	prntBox;
	UI8			version;	//8 bits
	UI32		flags;		//24 bits
} STNBMp4FullBox;

void NBMp4FullBox_init(STNBMp4FullBox* obj);
void NBMp4FullBox_release(STNBMp4FullBox* obj);
BOOL NBMp4FullBox_loadBits(STNBMp4FullBox* obj, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd);
void NBMp4FullBox_concat(const STNBMp4FullBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst);
UI32 NBMp4FullBox_getHdrBytesLen(const STNBMp4FullBox* obj);
BOOL NBMp4FullBox_writeHdrBits(STNBMp4FullBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst);
BOOL NBMp4FullBox_writeHdrFinalSize(const STNBMp4FullBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst);

//NBMp4SampleFlags

typedef enum ENNBMp4SampleFlagsLeading_ {
	ENNBMp4SampleFlagsLeading_Unknown = 0,				//unknown
	ENNBMp4SampleFlagsLeading_Dependant = 1,			//(has a dependency before the referenced I‐picture (and is therefore not decodable))
	ENNBMp4SampleFlagsLeading_NonLeading = 2,			//(not a leading sample)
	ENNBMp4SampleFlagsLeading_NonDependent = 3,			//(has no dependency before the referenced I‐picture (and is therefore decodable))
	//
	ENNBMp4SampleFlagsLeading_Count
} ENNBMp4SampleFlagsLeading;

typedef enum ENNBMp4SampleFlagsDependedsOn_ {
	ENNBMp4SampleFlagsDependedsOn_Unknown = 0,			//unknown
	ENNBMp4SampleFlagsDependedsOn_Depends = 1,			//(non-I picture)
	ENNBMp4SampleFlagsDependedsOn_Independent = 2,		//(I picture)
	//
	ENNBMp4SampleFlagsDependedsOn_Count					//reserved
} ENNBMp4SampleFlagsDependedsOn;

typedef enum ENNBMp4SampleFlagsIsDependedOn_ {
	ENNBMp4SampleFlagsIsDependedOn_Unknown = 0,			//unknown
	ENNBMp4SampleFlagsIsDependedOn_NonDisposable = 1,	//(other samples may depend on this one)
	ENNBMp4SampleFlagsIsDependedOn_Disposable = 2,		//(no other sample depends on this one)
	//
	ENNBMp4SampleFlagsIsDependedOn_Count				//reserved
} ENNBMp4SampleFlagsIsDependedOn;

typedef enum ENNBMp4SampleFlagsHasRedundancy_ {
	ENNBMp4SampleFlagsHasRedundancy_Unknown = 0,		//unknown
	ENNBMp4SampleFlagsHasRedundancy_Redundant = 1,		//(there is redundant coding in this sample)
	ENNBMp4SampleFlagsHasRedundancy_NonRedundant = 2,	//(there is no redundant coding in this sample)
	//
	ENNBMp4SampleFlagsHasRedundancy_Count				//reserved
} ENNBMp4SampleFlagsHasRedundancy;

typedef struct STNBMp4SampleFlags_ {
	UI32 flags;
} STNBMp4SampleFlags;

/* 32-bits = {
	bit(4)   reserved=0;
	unsigned int(2) is_leading;
	unsigned int(2) sample_depends_on;
	unsigned int(2) sample_is_depended_on;
	unsigned int(2) sample_has_redundancy;
	bit(3)   sample_padding_value;
	bit(1)   sample_is_non_sync_sample;
	unsigned int(16)  sample_degradation_priority;
}*/

void NBMp4SampleFlags_init(STNBMp4SampleFlags* obj);
void NBMp4SampleFlags_release(STNBMp4SampleFlags* obj);
//
BOOL NBMp4SampleFlags_loadFromBuff(STNBMp4SampleFlags* obj, const BYTE** buff, const BYTE* buffAfterEnd);
BOOL NBMp4SampleFlags_writeBits(STNBMp4SampleFlags* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst);
void NBMp4SampleFlags_concat(const STNBMp4SampleFlags* obj, STNBString* dst, const char* prefix);
//
void NBMp4SampleFlags_setFromOther(STNBMp4SampleFlags* obj, const STNBMp4SampleFlags* other);
BOOL NBMp4SampleFlags_setIsLeading(STNBMp4SampleFlags* obj, const ENNBMp4SampleFlagsLeading v); //unsigned int(2)
BOOL NBMp4SampleFlags_setSampleDependsOn(STNBMp4SampleFlags* obj, const ENNBMp4SampleFlagsDependedsOn v); //unsigned int(2)
BOOL NBMp4SampleFlags_setSampleIsDependsOn(STNBMp4SampleFlags* obj, const ENNBMp4SampleFlagsIsDependedOn v); //unsigned int(2)
BOOL NBMp4SampleFlags_setSampleHasRedundancy(STNBMp4SampleFlags* obj, const ENNBMp4SampleFlagsHasRedundancy v); //unsigned int(2)
BOOL NBMp4SampleFlags_setSampleIsNonSyncSample(STNBMp4SampleFlags* obj, const BOOL v); //bit(1)
void NBMp4SampleFlags_setSampleDegradationPriority(STNBMp4SampleFlags* obj, const UI16 v); //unsigned int(16)

//NBMp4FileTypeBox (‘ftyp’)

typedef struct STNBMp4FileTypeBox_ {
	STNBMp4Box	prntBox;
	STNBMp4Id4	majorBrand;
	UI32		minorVersion; 
	STNBMp4Id4*	compatibleBrands;
	UI32		compatibleBrandsSz;	
} STNBMp4FileTypeBox;

BOOL NBMp4FileTypeBox_setMajorBrand(STNBMp4FileTypeBox* obj, const char* majorBrand, const UI32 minorVersion);
BOOL NBMp4FileTypeBox_addCompatibleBrand(STNBMp4FileTypeBox* obj, const char* brand);

NBMP4_BOX_PUBLIC_METHODS_DECLARE(FileTypeBox)

//NBMp4MovieHeaderBox

typedef struct STNBMp4MovieHeaderBox_ {
	STNBMp4FullBox prntFullBox;
	union {
		struct {
			UI64	creationTime;
			UI64	modificationTime;
			UI64	timescale;
			UI64	duration;
		} v1;
		struct {
			UI32	creationTime;
			UI32	modificationTime;
			UI32	timescale;
			UI32	duration;
		} v0;
	} hdr;
	SI32	rate;
	SI16	volume;
	SI16	reserved;		//0
	UI32	reserved2[2];	//[0, 0]
	SI32	matrix[9];
	SI32	preDefined[6];	//0
	UI32	nextTrackId;
} STNBMp4MovieHeaderBox;

void NBMp4MovieHeaderBox_setTimescale(STNBMp4MovieHeaderBox* obj, const UI64 timescale);
void NBMp4MovieHeaderBox_setDuration(STNBMp4MovieHeaderBox* obj, const UI64 duration);
void NBMp4MovieHeaderBox_setNextTrackId(STNBMp4MovieHeaderBox* obj, const UI32 nextTrackId);

NBMP4_BOX_PUBLIC_METHODS_DECLARE(MovieHeaderBox)

//NBMp4MediaHeaderBox ('mdhd')

typedef struct STNBMp4MediaHeaderBox_ {
	STNBMp4FullBox prntFullBox;
	union {
		struct {
			UI64	creationTime;
			UI64	modificationTime;
			UI64	timescale;
			UI64	duration;
		} v1;
		struct {
			UI32	creationTime;
			UI32	modificationTime;
			UI32	timescale;
			UI32	duration;
		} v0;
	} hdr;
	//BOOL	pad;			//1 bit
	char	language[3];	//5 bits each
	SI16	preDefined;		//0
} STNBMp4MediaHeaderBox;

void NBMp4MediaHeaderBox_setTimescale(STNBMp4MediaHeaderBox* obj, const UI64 timescale);
void NBMp4MediaHeaderBox_setDuration(STNBMp4MediaHeaderBox* obj, const UI64 duration);
BOOL NBMp4MediaHeaderBox_setLanguage(STNBMp4MediaHeaderBox* obj, const char* lang3);

NBMP4_BOX_PUBLIC_METHODS_DECLARE(MediaHeaderBox)

//NBMp4TrackHeaderBox ('tkhd')

typedef struct STNBMp4TrackHeaderBox_ {
	STNBMp4FullBox prntFullBox;
	union {
		struct {
			UI64	creationTime;
			UI64	modificationTime;
			UI32	trackId;
			UI32	reserved;
			UI64	duration;
		} v1;
		struct {
			UI32	creationTime;
			UI32	modificationTime;
			UI32	trackId;
			UI32	reserved;
			UI32	duration;
		} v0;
	} hdr;
	UI32	reserved2[2];
	SI16	layer;
	SI16	alternateGroup;
	SI16	volume;
	UI16	reserved;
	SI32	matrix[9];
	UI32	width;
	UI32	height;
} STNBMp4TrackHeaderBox;

NBMP4_BOX_PUBLIC_METHODS_DECLARE(TrackHeaderBox)

void NBMp4TrackHeaderBox_setIsEnabled(STNBMp4TrackHeaderBox* obj, const BOOL isEnabled);	//that the track is enabled
void NBMp4TrackHeaderBox_setIsInMovie(STNBMp4TrackHeaderBox* obj, const BOOL isInMovie);	//the track is used in the presentation.
void NBMp4TrackHeaderBox_setIsInPreview(STNBMp4TrackHeaderBox* obj, const BOOL isInPreview);	//the track is used when previewing the presentation.
void NBMp4TrackHeaderBox_setTrackId(STNBMp4TrackHeaderBox* obj, const UI32 trackId);
void NBMp4TrackHeaderBox_setDuration(STNBMp4TrackHeaderBox* obj, const UI64 duration);
void NBMp4TrackHeaderBox_setLayer(STNBMp4TrackHeaderBox* obj, const SI16 layer);
void NBMp4TrackHeaderBox_setAlternateGroup(STNBMp4TrackHeaderBox* obj, const SI16 alternateGroup);
void NBMp4TrackHeaderBox_setWidth(STNBMp4TrackHeaderBox* obj, const UI16 width);
void NBMp4TrackHeaderBox_setHeight(STNBMp4TrackHeaderBox* obj, const UI16 height);


//NBMp4HandlerBox ('hdlr')

/*aligned(8) class HandlerBox extends FullBox(‘hdlr’, version = 0, 0) {
   unsigned int(32)  pre_defined = 0;
   unsigned int(32)  handler_type;
   const unsigned int(32)[3]  reserved = 0;
   string   name;
}*/

typedef struct STNBMp4HandlerBox_ {
	STNBMp4FullBox	prntFullBox;
	UI32			preDefined;
	STNBMp4Id4		handlerType;
	UI32			reserved[3];
	char*			name;
} STNBMp4HandlerBox;

BOOL NBMp4HandlerBox_setHandlerType(STNBMp4HandlerBox* obj, const char* type4, const char* name);

NBMP4_BOX_PUBLIC_METHODS_DECLARE(HandlerBox)

//NBMp4DataEntryUrlBox ('url ')

typedef struct STNBMp4DataEntryUrlBox_ {
	STNBMp4FullBox	prntFullBox;
	char*			location;
} STNBMp4DataEntryUrlBox;

void NBMp4DataEntryUrlBox_setLocation(STNBMp4DataEntryUrlBox* obj, const char* location);

NBMP4_BOX_PUBLIC_METHODS_DECLARE(DataEntryUrlBox)

//NBMp4DataEntryUrnBox ('urn ')

typedef struct STNBMp4DataEntryUrnBox_ {
	STNBMp4FullBox	prntFullBox;
	char*			name;
	char*			location;
} STNBMp4DataEntryUrnBox;

void NBMp4DataEntryUrnBox_setLocation(STNBMp4DataEntryUrnBox* obj, const char* name, const char* location);

NBMP4_BOX_PUBLIC_METHODS_DECLARE(DataEntryUrnBox)

//NBMp4DataEntryBox ('url ' or 'urn ')

typedef struct STNBMp4DataEntryBox_ {
	union {
		STNBMp4DataEntryUrlBox	url;
		STNBMp4DataEntryUrnBox	urn;
	} entry;
} STNBMp4DataEntryBox;

void NBMp4DataEntryBox_setEntry(STNBMp4DataEntryBox* obj, const char* name, const char* location);

NBMP4_BOX_PUBLIC_METHODS_DECLARE(DataEntryBox)

//NBMp4DataReferenceBox ('dref')

typedef struct STNBMp4DataReferenceBox_ {
	STNBMp4FullBox	prntFullBox;
	UI32			entryCount;
	STNBMp4DataEntryBox* dataEntries;
} STNBMp4DataReferenceBox;

void NBMp4DataReferenceBox_addEntry(STNBMp4DataReferenceBox* obj, const char* name, const char* location);

NBMP4_BOX_PUBLIC_METHODS_DECLARE(DataReferenceBox)

//NBMp4SampleSizeBox ('stsz')

typedef struct STNBMp4SampleSizeBox_ {
	STNBMp4FullBox	prntFullBox;
	UI32			sampleSize;
	UI32			sampleCount;
	UI32*			entrySize;
	UI32			entrySizeSz;
} STNBMp4SampleSizeBox;

NBMP4_BOX_PUBLIC_METHODS_DECLARE(SampleSizeBox)

//NBMp4VideoMediaHeaderBox ('vmhd')

typedef struct STNBMp4VideoMediaHeaderBox_ {
	STNBMp4FullBox	prntFullBox;
	UI16			graphicsMode;
	UI16			opColor[3];
} STNBMp4VideoMediaHeaderBox;

NBMP4_BOX_PUBLIC_METHODS_DECLARE(VideoMediaHeaderBox)

//NBMp4BitRateBox ('btrt')

typedef struct STNBMp4BitRateBox_ {
	STNBMp4Box		prntBox;
	UI32 			bufferSizeDB;
	UI32			maxBitrate;
	UI32			avgBitrate;
} STNBMp4BitRateBox;

NBMP4_BOX_PUBLIC_METHODS_DECLARE(BitRateBox)

//NBMp4PixelAspectRatioBox ('pasp')

typedef struct STNBMp4PixelAspectRatioBox_ {
	STNBMp4Box		prntBox;
	UI32			hSpacing;
	UI32			vSpacing;
} STNBMp4PixelAspectRatioBox;

void NBMp4PixelAspectRatioBox_setHSpacing(STNBMp4PixelAspectRatioBox* obj, const UI32 hSpacing);
void NBMp4PixelAspectRatioBox_setVSpacing(STNBMp4PixelAspectRatioBox* obj, const UI32 vSpacing);

NBMP4_BOX_PUBLIC_METHODS_DECLARE(PixelAspectRatioBox)

//NBMp4CleanApertureBox ('clap')

typedef struct STNBMp4CleanApertureBox_ {
	STNBMp4Box		prntBox;
	UI32			cleanApertureWidthN;
	UI32			cleanApertureWidthD;
	UI32			cleanApertureHeightN;
	UI32			cleanApertureHeightD;
	UI32			horizOffN;
	UI32			horizOffD;
	UI32			vertOffN;
	UI32			vertOffD;
} STNBMp4CleanApertureBox;

NBMP4_BOX_PUBLIC_METHODS_DECLARE(CleanApertureBox)

//NBMp4SampleEntry (abstract)

typedef struct STNBMp4SampleEntry_ {
	STNBMp4Box		prntBox;
	UI8				reserved[6];	//reserved
	UI16			dataReferenceIndex;
	BYTE*			data;
	UI32			dataSz;
} STNBMp4SampleEntry;

void NBMp4SampleEntry_setDataRefIndex(STNBMp4SampleEntry* obj, const UI16 idx);

NBMP4_BOX_PUBLIC_METHODS_DECLARE(SampleEntry)

//NBMp4VisualSampleEntry (base)

typedef struct STNBMp4VisualSampleEntry_ {
	STNBMp4SampleEntry prntEntry;
	UI16	preDefined;			// = 0;
	UI16	reserved;			// = 0;
	UI32	preDefined2[3];		// = 0;
	UI16	width;
	UI16	height;
	UI32	horizResolution;	// = 0x00480000; // 72 dpi
	UI32	vertResolution;		//  = 0x00480000; // 72 dpi
	UI32	reserved2;			// = 0;
	UI16	frameCount;			// = 1;
	struct {
		char	len;
		char	value[32];		//includes the '\0' char at the 31 pos
	} compressorName;
	UI16	depth;				// = 0x0018;
	SI16	preDefined3;		// = -1;
	// other boxes from derived specifications
	STNBMp4CleanApertureBox*     clap;    // optional
	STNBMp4PixelAspectRatioBox*  pasp;    // optional
} STNBMp4VisualSampleEntry;

void NBMp4VisualSampleEntry_setWidth(STNBMp4VisualSampleEntry* obj, const UI16 width);
void NBMp4VisualSampleEntry_setHeight(STNBMp4VisualSampleEntry* obj, const UI16 height);
void NBMp4VisualSampleEntry_setFrameCount(STNBMp4VisualSampleEntry* obj, const UI16 frameCount);
void NBMp4VisualSampleEntry_setCompressorName(STNBMp4VisualSampleEntry* obj, const char* name31Max);
void NBMp4VisualSampleEntry_setDepth(STNBMp4VisualSampleEntry* obj, const UI16 depth);
BOOL NBMp4VisualSampleEntry_setClapByOwning(STNBMp4VisualSampleEntry* obj, STNBMp4BoxRef* ref);
BOOL NBMp4VisualSampleEntry_setPaspByOwning(STNBMp4VisualSampleEntry* obj, STNBMp4BoxRef* ref);

NBMP4_BOX_PUBLIC_METHODS_DECLARE(VisualSampleEntry)

//NBMp4SampleDescriptionBox ('stsd')

typedef struct STNBMp4SampleDescriptionBox_ {
	STNBMp4FullBox			prntFullBox;
	UI32					entryCount;
	STNBMp4BoxRef*			entries;
} STNBMp4SampleDescriptionBox;

BOOL NBMp4SampleDescriptionBox_addEntryByOwning(STNBMp4SampleDescriptionBox* obj, STNBMp4BoxRef* boxRef);

NBMP4_BOX_PUBLIC_METHODS_DECLARE(SampleDescriptionBox)

//NBMp4TimeToSampleBox ('stts')

typedef struct STNBMp4TimeToSampleEntry_ {
	UI32	sampleCount;
	UI32	sampleDelta;
} STNBMp4TimeToSampleEntry;

typedef struct STNBMp4TimeToSampleBox_ {
	STNBMp4FullBox				prntFullBox;
	UI32						entryCount;
	STNBMp4TimeToSampleEntry*	entries;
} STNBMp4TimeToSampleBox;

NBMP4_BOX_PUBLIC_METHODS_DECLARE(TimeToSampleBox)

//NBMp4SampleToChunkBox ('stsc')

typedef struct STNBMp4SampleToChunkEntry_ {
	UI32 firstChunk;
	UI32 samplesPerChunk;
	UI32 sampleDescriptionIndex;
} STNBMp4SampleToChunkEntry;

typedef struct STNBMp4SampleToChunkBox_ {
	STNBMp4FullBox				prntFullBox;
	UI32						entryCount;
	STNBMp4SampleToChunkEntry*	entries;
} STNBMp4SampleToChunkBox;

NBMP4_BOX_PUBLIC_METHODS_DECLARE(SampleToChunkBox)

//NBMp4ChunkOffsetBox ('stco')

typedef struct STNBMp4ChunkOffsetBox_ {
	STNBMp4FullBox	prntFullBox;
	UI32			entryCount;
	UI32*			entries;
} STNBMp4ChunkOffsetBox;

NBMP4_BOX_PUBLIC_METHODS_DECLARE(ChunkOffsetBox)

//NBMp4ChunkOffsetBox64 ('co64')

typedef struct STNBMp4ChunkOffsetBox64_ {
	STNBMp4FullBox	prntFullBox;
	UI32			entryCount;
	UI64*			entries;
} STNBMp4ChunkOffsetBox64;

NBMP4_BOX_PUBLIC_METHODS_DECLARE(ChunkOffsetBox64)

//NBMp4TrackExtendsBox ('trex')

typedef struct STNBMp4TrackExtendsBox_ {
	STNBMp4FullBox		prntFullBox;
	UI32				trackId;
	UI32				defaultSampleDescriptionIndex;
	UI32				defaultSampleDuration;
	UI32				defaultSampleSize;
	STNBMp4SampleFlags	defaultSampleFlags;
} STNBMp4TrackExtendsBox;

void NBMp4TrackExtendsBox_setTrackId(STNBMp4TrackExtendsBox* obj, const UI32 trackId);
void NBMp4TrackExtendsBox_setDefaultSampleDescriptionIndex(STNBMp4TrackExtendsBox* obj, const UI32 defaultSampleDescriptionIndex);
void NBMp4TrackExtendsBox_setDefaultSampleDuration(STNBMp4TrackExtendsBox* obj, const UI32 defaultSampleDuration);
void NBMp4TrackExtendsBox_setDefaultSampleSize(STNBMp4TrackExtendsBox* obj, const UI32 defaultSampleSize);
void NBMp4TrackExtendsBox_setDefaultSampleFlags(STNBMp4TrackExtendsBox* obj, const STNBMp4SampleFlags* flags);

NBMP4_BOX_PUBLIC_METHODS_DECLARE(TrackExtendsBox)

//NBMp4MovieFragmentHeaderBox ('mfhd')

typedef struct STNBMp4MovieFragmentHeaderBox_ {
	STNBMp4FullBox	prntFullBox;
	UI32 sequenceNumber;
} STNBMp4MovieFragmentHeaderBox;

void NBMp4MovieFragmentHeaderBox_setSeqNumber(STNBMp4MovieFragmentHeaderBox* obj, const UI32 seqNum);

NBMP4_BOX_PUBLIC_METHODS_DECLARE(MovieFragmentHeaderBox)

//NBMp4TrackFragmentHeaderBox ('tfhd')

#define NBMp4TrackFragmentHeaderBoxFlag_base_data_offset_present		0x000001
#define NBMp4TrackFragmentHeaderBoxFlag_sample_description_index_present 0x000002
#define NBMp4TrackFragmentHeaderBoxFlag_default_sample_duration_present	0x000008
#define NBMp4TrackFragmentHeaderBoxFlag_default_sample_size_present		0x000010
#define NBMp4TrackFragmentHeaderBoxFlag_default_sample_flags_present	0x000020
#define NBMp4TrackFragmentHeaderBoxFlag_duration_is_empty				0x010000
#define NBMp4TrackFragmentHeaderBoxFlag_default_base_is_moof			0x020000

typedef struct STNBMp4TrackFragmentHeaderBox_ {
	STNBMp4FullBox		prntFullBox;
	UI32				trackId;
	// all the following are optional fields
	UI64				baseDataOffset;
	UI32				sampleDescriptionIndex;
	UI32				defaultSampleDuration;
	UI32				defaultSampleSize;
	STNBMp4SampleFlags	defaultSampleFlags;
} STNBMp4TrackFragmentHeaderBox;

UI64 NBMp4TrackFragmentHeaderBox_getBaseDataOffset(const STNBMp4TrackFragmentHeaderBox* obj);
UI32 NBMp4TrackFragmentHeaderBox_getDefaultSampleSize(const STNBMp4TrackFragmentHeaderBox* obj);
//
void NBMp4TrackFragmentHeaderBox_setTrackId(STNBMp4TrackFragmentHeaderBox* obj, const UI32 trackId);
void NBMp4TrackFragmentHeaderBox_setBaseDataOffset(STNBMp4TrackFragmentHeaderBox* obj, const UI64 baseDataOffset);
void NBMp4TrackFragmentHeaderBox_setSampleDescriptionIndex(STNBMp4TrackFragmentHeaderBox* obj, const UI32 sampleDescriptionIndex);
void NBMp4TrackFragmentHeaderBox_setDefaultSampleDuration(STNBMp4TrackFragmentHeaderBox* obj, const UI32 defaultSampleDuration);
void NBMp4TrackFragmentHeaderBox_setDefaultSampleSize(STNBMp4TrackFragmentHeaderBox* obj, const UI32 defaultSampleSize);
void NBMp4TrackFragmentHeaderBox_setDefaultSampleFlags(STNBMp4TrackFragmentHeaderBox* obj, const STNBMp4SampleFlags* defaultSampleFlags);
void NBMp4TrackFragmentHeaderBox_setDurationIsEmpty(STNBMp4TrackFragmentHeaderBox* obj, const BOOL durIsEmpty);
void NBMp4TrackFragmentHeaderBox_setDefaultBaseIsMoof(STNBMp4TrackFragmentHeaderBox* obj, const BOOL defBaseIsMoof);

NBMP4_BOX_PUBLIC_METHODS_DECLARE(TrackFragmentHeaderBox)

//NBMp4TrackFragmentBaseMediaDecodeTimeBox ('tfdt')

typedef struct STNBMp4TrackFragmentBaseMediaDecodeTimeBox_ {
	STNBMp4FullBox	prntFullBox;
	union {
		UI32 v0;
		UI64 v1;
	} baseMediaDecodeTime;
} STNBMp4TrackFragmentBaseMediaDecodeTimeBox;

void NBMp4TrackFragmentBaseMediaDecodeTimeBox_setBaseMediaDecodeTime(STNBMp4TrackFragmentBaseMediaDecodeTimeBox* obj, const UI64 baseMediaDecodeTime);

NBMP4_BOX_PUBLIC_METHODS_DECLARE(TrackFragmentBaseMediaDecodeTimeBox)

//NBMp4TrackRunBox ('trun')

#define NBMp4TrackRunBoxFlag_data_offset_present		0x000001
#define NBMp4TrackRunBoxFlag_first_sample_flags_present	0x000004
#define NBMp4TrackRunBoxFlag_sample_duration_present	0x000100
#define NBMp4TrackRunBoxFlag_sample_size_present		0x000200
#define NBMp4TrackRunBoxFlag_sample_flags_present		0x000400
#define NBMp4TrackRunBoxFlag_sample_composition_time_offsets_present 0x000800 //sample-composition-time-offset = presentationTime - decodingTime.

typedef struct STNBMp4TrackRunEntry_ {
	UI32				sampleDuration;
	UI32				sampleSize;
	STNBMp4SampleFlags	sampleFlags;
	union {
		UI32 v0;
		SI32 vAny;
	} sampleCompositionTimeOffset;
} STNBMp4TrackRunEntry;

typedef struct STNBMp4TrackRunBox_ {
	STNBMp4FullBox			prntFullBox;
	UI32					sampleCount;
	// the following are optional fields
	SI32					dataOffset;
	STNBMp4SampleFlags		firstSampleFlags;
	// all fields in the following array are optional
	STNBMp4TrackRunEntry*	entries;
	UI32					entriesAlloc;
} STNBMp4TrackRunBox;

void NBMp4TrackRunBox_setDataOffset(STNBMp4TrackRunBox* obj, const SI32 dataOffset);
BOOL NBMp4TrackRunBox_overwriteDataOffset(STNBMp4TrackRunBox* obj, const SI32 dataOffset, STNBString* dst);
void NBMp4TrackRunBox_entriesEmpty(STNBMp4TrackRunBox* obj);
BOOL NBMp4TrackRunBox_entriesAdd(STNBMp4TrackRunBox* obj, const UI32 sampleDuration, const UI32 sampleSize, const STNBMp4SampleFlags* sampleFlags, const UI32 sampleCompositionTimeOffset);

NBMP4_BOX_PUBLIC_METHODS_DECLARE(TrackRunBox)

//---------------------
// Containers
//---------------------

//NBMp4SampleTableBox ('stbl')

typedef struct STNBMp4SampleTableBox_ {
	STNBMp4Box						prntBox;
	//Children boxes
	STNBMp4SampleDescriptionBox*	stsd;
	STNBMp4TimeToSampleBox*			stts;
	//STNBMp4CompositionOffsetBox*	ctts;
	//STNBMp4CompositionToDecodeBox* cslg;
	STNBMp4SampleToChunkBox*		stsc;
	STNBMp4SampleSizeBox*			stsz;
	//STNBMp4CompactSampleSizeBox*	stz2;
	STNBMp4ChunkOffsetBox*			stco;
	//STNBMp4ChunkLargeOffsetBox*	co64;
	//STNBMp4SyncSampleBox*			stss;
	//STNBMp4ShadowSyncSampleBox*	stsh;
	//STNBMp4PaddingBitsBox*		padb;
	//STNBMp4DegradationPriorityBox* stdp;
	//STNBMp4SampleDependencyTypeBox* sdtp;
	//STNBMp4SampleToGroupBox*		sbgp;
	//STNBMp4SampleGroupDescriptionBox* sgpd;
	//STNBMp4SubSampleInformationBox*	subs;
	//STNBMp4SampleAuxiliaryInformationSizesBox* saiz;
	//STNBMp4SampleAuxiliaryInformationOffsetsBox* saio;
} STNBMp4SampleTableBox;

NBMP4_BOX_PUBLIC_METHODS_DECLARE(SampleTableBox)

//NBMp4DataInformationBox ('dinf')

typedef struct STNBMp4DataInformationBox_ {
	STNBMp4Box		prntBox;
	//Children boxes
	STNBMp4DataReferenceBox*	dref;
} STNBMp4DataInformationBox;

NBMP4_BOX_PUBLIC_METHODS_DECLARE(DataInformationBox)

//NBMp4MediaInformationBox ('minf')

typedef struct STNBMp4MediaInformationBox_ {
	STNBMp4Box					prntBox;
	//Children boxes
	STNBMp4VideoMediaHeaderBox*	vmhd;
	//STNBMp4SoundMediaHeaderBox* smhd;
	//STNBMp4HintMediaHeaderBox* hmhd;
	//STNBMp4SubtitleMediaHeaderBox* sthd;
	//STNBMp4NullMediaHeaderBox* nmhd;
	STNBMp4DataInformationBox*	dinf;
	STNBMp4SampleTableBox*		stbl;
} STNBMp4MediaInformationBox;

NBMP4_BOX_PUBLIC_METHODS_DECLARE(MediaInformationBox)

//NBMp4MediaBox ('mdia')

typedef struct STNBMp4MediaBox_ {
	STNBMp4Box					prntBox;
	//Children boxes
	STNBMp4MediaHeaderBox*		mdhd;
	STNBMp4HandlerBox*			hdlr;
	//STNBMp4ExtendedLanguageBox elng;
	STNBMp4MediaInformationBox*	minf;
} STNBMp4MediaBox;

NBMP4_BOX_PUBLIC_METHODS_DECLARE(MediaBox)

//NBMp4TrackBox ('trak')

typedef struct STNBMp4TrackBox_ {
	STNBMp4Box				prntBox;
	//Children boxes
	STNBMp4TrackHeaderBox*	tkhd;
	//STNBMp4TrackReferenceBox* tref;
	//STNBMp4TrackGroupBox*	trgr
	//STNBMp4EditBox*		edts; //(container)
	//STNBMp4MetaBox*		meta;
	STNBMp4MediaBox*		mdia; //(container)
	//STNBMp4UserDataBox*	udta;
} STNBMp4TrackBox;

NBMP4_BOX_PUBLIC_METHODS_DECLARE(TrackBox)

//NBMp4MovieExtendsBox ('mvex')

typedef struct STNBMp4MovieExtendsBox_ {
	STNBMp4Box				prntBox;
	//Children boxes
	//STNBMp4MovieExtendsHeaderBox*	mehd;
	STNBMp4TrackExtendsBox* trex;
	//STNBMp4LevelAssignmentBox* leva;
} STNBMp4MovieExtendsBox;

NBMP4_BOX_PUBLIC_METHODS_DECLARE(MovieExtendsBox)

//NBMp4MovieBox ('moov')

typedef struct STNBMp4MovieBox_ {
	STNBMp4Box				prntBox;
	//Children boxes
	STNBMp4MovieHeaderBox*	mvhd;
	//STNBMp4MetaBox*		meta;
	STNBMp4TrackBox*		trak;
	//STNBMp4MovieExtendsBox* mvex;
} STNBMp4MovieBox;

NBMP4_BOX_PUBLIC_METHODS_DECLARE(MovieBox)

//NBMp4TrackFragmentBox ('traf')

typedef struct STNBMp4TrackFragmentBox_ {
	STNBMp4Box				prntBox;
	//Children boxes
	STNBMp4TrackFragmentHeaderBox* tfhd;
	STNBMp4TrackRunBox*		trun;
	//STNBMp4SampleToGroupBox* sbgp;
	//STNBMp4SampleGroupDescriptionBox* sgpd;
	//STNBMp4SubSampleInformationBox*	subs;
	//STNBMp4SampleAuxiliaryInformationSizesBox* saiz;
	//STNBMp4SampleAuxiliaryInformationOffsetsBox* saio;
	STNBMp4TrackFragmentBaseMediaDecodeTimeBox* tfdt;
	//STNBMp4MetaBox*		meta;
} STNBMp4TrackFragmentBox;

NBMP4_BOX_PUBLIC_METHODS_DECLARE(TrackFragmentBox)

//NBMp4MovieFragmentBox ('moof')

typedef struct STNBMp4MovieFragmentBox_ {
	STNBMp4Box				prntBox;
	//Children boxes
	STNBMp4MovieFragmentHeaderBox* mfhd;
	//STNBMp4MetaBox*		meta;
	STNBMp4TrackFragmentBox* traf;
} STNBMp4MovieFragmentBox;

NBMP4_BOX_PUBLIC_METHODS_DECLARE(MovieFragmentBox)

//ISO-IEC-14496-15

typedef struct STNBMp4AVCDecoderConfigurationRecord_ {
	UI8		configurationVersion;	// = 1;
	UI8		AVCProfileIndication;
	UI8		profileCompatibility;
	UI8		AVCLevelIndication;
	UI8		reserved;
	UI8		lengthSizeMinusOne; //bit(6) ‘111111’b +  bit(2)
	UI8		reserved1;
	UI8		numOfSequenceParameterSets; //bit(3) ‘111’b + bit(5)
	STNBMp4Data* sequenceParameterSetNALUnits;
	/*{
		unsigned int(16) sequenceParameterSetLength ;
		bit(8*sequenceParameterSetLength) sequenceParameterSetNALUnit;
	}*/
	UI8 numOfPictureParameterSets;
	STNBMp4Data* pictureParameterSetNALUnits;
	/*{
		unsigned int(16) pictureParameterSetLength;
		bit(8*pictureParameterSetLength) pictureParameterSetNALUnit;
	}*/
	//if( profile_idc  ==  100  ||  profile_idc  ==  110  || profile_idc  ==  122  ||  profile_idc  ==  144 ){
	UI8		reserved2;
	UI8		chromaFormat; //bit(6) ‘111111’b +  bit(2)
	UI8		reserved3;
	UI8		bitDepthLumaMinus8; //bit(5) ‘11111’b +  bit(3)
	UI8		reserved4;
	UI8		bitDepthChromaMinus8; //bit(5) ‘11111’b +  bit(3)
	UI8		numOfSequenceParameterSetExt;
	STNBMp4Data* sequenceParameterSetExtNALUnits;
		/*{
			unsigned int(16) sequenceParameterSetExtLength;
			bit(8*sequenceParameterSetExtLength) sequenceParameterSetExtNALUnit;
		}*/
	//}
} STNBMp4AVCDecoderConfigurationRecord;

NBMP4_BOX_PUBLIC_METHODS_DECLARE(AVCDecoderConfigurationRecord)

//NBMp4AVCConfigurationBox ('avcC')

typedef struct STNBMp4AVCConfigurationBox_ {
	STNBMp4Box		prntBox;
	STNBMp4AVCDecoderConfigurationRecord avcCfg;
} STNBMp4AVCConfigurationBox;

void NBMp4AVCConfigurationBox_setAVCProfileIndication(STNBMp4AVCConfigurationBox* obj, const UI8 avcProfileIndication);
void NBMp4AVCConfigurationBox_setProfileCompatibility(STNBMp4AVCConfigurationBox* obj, const UI8 profileCompatibility);
void NBMp4AVCConfigurationBox_setProfileLevelIndication(STNBMp4AVCConfigurationBox* obj, const UI8 profLevelIndication);
BOOL NBMp4AVCConfigurationBox_setLengthSize(STNBMp4AVCConfigurationBox* obj, const UI8 lengthSize); //bit(2) + 1
void NBMp4AVCConfigurationBox_emptySps(STNBMp4AVCConfigurationBox* obj);
BOOL NBMp4AVCConfigurationBox_addSps(STNBMp4AVCConfigurationBox* obj, const void* data, const UI16 size);
BOOL NBMp4AVCConfigurationBox_updateSps(STNBMp4AVCConfigurationBox* obj, const UI16 idx, const void* data, const UI16 size);
void NBMp4AVCConfigurationBox_emptyPps(STNBMp4AVCConfigurationBox* obj);
BOOL NBMp4AVCConfigurationBox_addPps(STNBMp4AVCConfigurationBox* obj, const void* data, const UI16 size);
BOOL NBMp4AVCConfigurationBox_updatePps(STNBMp4AVCConfigurationBox* obj, const UI16 idx, const void* data, const UI16 size);
BOOL NBMp4AVCConfigurationBox_setChromaFormat(STNBMp4AVCConfigurationBox* obj, const UI8 fmt); //bit(2)
BOOL NBMp4AVCConfigurationBox_setBitDepthLuma(STNBMp4AVCConfigurationBox* obj, const UI8 bitDepth); //bit(3) + 8
BOOL NBMp4AVCConfigurationBox_setBitDepthChroma(STNBMp4AVCConfigurationBox* obj, const UI8 bitDepth); //bit(3) + 8


NBMP4_BOX_PUBLIC_METHODS_DECLARE(AVCConfigurationBox)

//NBMp4MPEG4BitRateBox ('btrt')

typedef struct STNBMp4MPEG4BitRateBox_ {
	STNBMp4Box		prntBox;
	UI32 			bufferSizeDB;
	UI32			maxBitrate;
	UI32			avgBitrate;
} STNBMp4MPEG4BitRateBox;

NBMP4_BOX_PUBLIC_METHODS_DECLARE(MPEG4BitRateBox)

//NBMp4AVCSampleEntry ('avc1')

typedef struct STNBMp4AVCSampleEntry_ {
	STNBMp4VisualSampleEntry	prntVisualEntry;
	STNBMp4AVCConfigurationBox* avcC;	//required
	STNBMp4MPEG4BitRateBox*		btrt;	//optional 
	STNBMp4Box*					m4ds;	//optional (ToDo: implement)
} STNBMp4AVCSampleEntry;

void NBMp4AVCSampleEntry_setDataRefIndex(STNBMp4AVCSampleEntry* obj, const UI16 idx);
void NBMp4AVCSampleEntry_setWidth(STNBMp4AVCSampleEntry* obj, const UI16 width);
void NBMp4AVCSampleEntry_setHeight(STNBMp4AVCSampleEntry* obj, const UI16 height);
void NBMp4AVCSampleEntry_setFrameCount(STNBMp4AVCSampleEntry* obj, const UI16 frameCount);
void NBMp4AVCSampleEntry_setCompressorName(STNBMp4AVCSampleEntry* obj, const char* name31Max);
void NBMp4AVCSampleEntry_setDepth(STNBMp4AVCSampleEntry* obj, const UI16 depth);
BOOL NBMp4AVCSampleEntry_setClapByOwning(STNBMp4AVCSampleEntry* obj, STNBMp4BoxRef* ref);
BOOL NBMp4AVCSampleEntry_setPaspByOwning(STNBMp4AVCSampleEntry* obj, STNBMp4BoxRef* ref);
BOOL NBMp4AVCSampleEntry_setAvcCByOwning(STNBMp4AVCSampleEntry* obj, STNBMp4BoxRef* ref); 
BOOL NBMp4AVCSampleEntry_setBtrtByOwning(STNBMp4AVCSampleEntry* obj, STNBMp4BoxRef* ref);

NBMP4_BOX_PUBLIC_METHODS_DECLARE(AVCSampleEntry)

#ifdef __cplusplus
} //extern "C"
#endif

#endif
