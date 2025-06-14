
#-------------------------
# PROJECT
#-------------------------

$(eval $(call nbCall,nbInitProject))

NB_PROJECT_NAME             := nbframework

NB_PROJECT_CFLAGS           :=

NB_PROJECT_CXXFLAGS         :=

#Add debug specific
ifeq ($(NB_WORKSPACE_BLD_CFG),debug)
   NB_PROJECT_CFLAGS        += \
      -DDEBUG=1
   NB_PROJECT_CXXFLAGS      += \
      -DDEBUG=1
endif

NB_PROJECT_INCLUDES         := \
   include

#only if NB_LIB_Z_SYSTEM is not set
ifeq ($(NB_LIB_Z_SYSTEM),)
   NB_PROJECT_CFLAGS        += \
      -DZLIB_DEBUG=1
   NB_PROJECT_CXXFLAGS      += \
      -DZLIB_DEBUG=1
endif

#-------------------------
# TARGET
#-------------------------

$(eval $(call nbCall,nbInitTarget))

NB_TARGET_NAME              := nbframework

NB_TARGET_PREFIX            := lib

NB_TARGET_SUFIX             := .a

NB_TARGET_TYPE              := static
	
#-------------------------
# GROUP NB_TESSERACT glue (to source files)
#-------------------------

$(eval $(call nbCall,nbInitCodeGrp))

NB_CODE_GRP_NAME            := tesseract-glue-src

NB_CODE_GRP_FLAGS_REQUIRED  += NB_LIB_TESSERACT

NB_CODE_GRP_FLAGS_FORBIDDEN += NB_LIB_TESSERACT_SYSTEM

NB_CODE_GRP_FLAGS_ENABLES   += NB_LIB_LEPTONICA

NB_CODE_GRP_CFLAGS          += \
    -DHAVE_CONFIG_H \
    -DNDEBUG

NB_CODE_GRP_CXXFLAGS        += \
    -DHAVE_CONFIG_H \
    -DNDEBUG \
    -std=c++20

NB_CODE_GRP_INCLUDES        += \
    include/nb/ext \
    include/nb/ext/tesseract \
    include/nb/ext/leptonica \
    src/ext/leptonica/src \
    src/ext/tesseract/include \
    src/ext/tesseract/src/api \
    src/ext/tesseract/src/arch \
    src/ext/tesseract/src/ccmain \
    src/ext/tesseract/src/ccstruct \
    src/ext/tesseract/src/ccutil \
    src/ext/tesseract/src/classify \
    src/ext/tesseract/src/cutil \
    src/ext/tesseract/src/dict \
    src/ext/tesseract/src/textord \
    src/ext/tesseract/src/wordrec \
    src/ext/tesseract/src/lstm \
    src/ext/tesseract/src/viewer

NB_CODE_GRP_INCLUDES        += \
    include/nb/ext/leptonica \
    src/ext/leptonica/src
   
NB_CODE_GRP_SRCS            := \
    src/ocr/NBOcr.cpp

$(eval $(call nbCall,nbBuildCodeGrpRules))

#-------------------------
# GROUP NB_TESSERACT glue (to precompiled library)
#-------------------------

$(eval $(call nbCall,nbInitCodeGrp))

NB_CODE_GRP_NAME            := tesseract-glue-lib

NB_CODE_GRP_FLAGS_REQUIRED  += NB_LIB_TESSERACT NB_LIB_TESSERACT_SYSTEM

NB_CODE_GRP_FLAGS_ENABLES   += NB_LIB_LEPTONICA

NB_CODE_GRP_CFLAGS          += \
    -DHAVE_CONFIG_H \
    -DNDEBUG

NB_CODE_GRP_CXXFLAGS        += \
    -DHAVE_CONFIG_H \
    -DNDEBUG \
    -std=c++20
   
NB_CODE_GRP_SRCS            := \
    src/ocr/NBOcr.cpp

$(eval $(call nbCall,nbBuildCodeGrpRules))

#-------------------------
# GROUP NB_LIB_PNG glue
#-------------------------

$(eval $(call nbCall,nbInitCodeGrp))

NB_CODE_GRP_NAME            := libpng-glue

NB_CODE_GRP_FLAGS_REQUIRED  += NB_LIB_PNG

NB_CODE_GRP_FLAGS_ENABLES   += NB_LIB_Z

NB_CODE_GRP_SRCS            := \
    src/2d/NBPngChunk.c \
    src/2d/NBPng.c

$(eval $(call nbCall,nbBuildCodeGrpRules))

#-------------------------
# GROUP NB_LIBJPEG glue (to source)
#-------------------------

$(eval $(call nbCall,nbInitCodeGrp))

NB_CODE_GRP_NAME            := libjpeg-glue-src

NB_CODE_GRP_FLAGS_REQUIRED  += NB_LIB_JPEG

NB_CODE_GRP_FLAGS_FORBIDDEN += NB_LIB_JPEG_SYSTEM

NB_CODE_GRP_SRCS            := \
    src/2d/NBJpegRead.c \
    src/2d/NBJpegWrite.c \
    src/2d/NBJpeg.c
   
NB_CODE_GRP_INCLUDES        += \
    src/ext/libjpeg

$(eval $(call nbCall,nbBuildCodeGrpRules))

#-------------------------
# GROUP NB_LIBJPEG glue (to precompiled lib)
#-------------------------

$(eval $(call nbCall,nbInitCodeGrp))

NB_CODE_GRP_NAME            := libjpeg-glue-lib

NB_CODE_GRP_FLAGS_REQUIRED  += NB_LIB_JPEG NB_LIB_JPEG_SYSTEM

NB_CODE_GRP_SRCS            := \
   src/2d/NBJpegRead.c \
   src/2d/NBJpegWrite.c \
   src/2d/NBJpeg.c

$(eval $(call nbCall,nbBuildCodeGrpRules))

#-------------------------
# GROUP NB_OPENSSL glue (to source)
#-------------------------

$(eval $(call nbCall,nbInitCodeGrp))

NB_CODE_GRP_NAME            := openssl-glue-src

NB_CODE_GRP_FLAGS_REQUIRED  += NB_LIB_SSL

NB_CODE_GRP_FLAGS_FORBIDDEN += NB_LIB_SSL_SYSTEM

NB_CODE_GRP_INCLUDES        += \
    include/nb/ext/openssl/include \
    src/ext/openssl/include

NB_CODE_GRP_SRCS            := \
    src/crypto/NBAes256.c \
    src/crypto/NBPkcs12.c \
    src/crypto/NBPKey.c \
    src/crypto/NBX500Name.c \
    src/crypto/NBX509.c \
    src/crypto/NBX509Req.c \
    src/ssl/NBSsl.c \
    src/ssl/NBSslContext.c

$(eval $(call nbCall,nbBuildCodeGrpRules))

#-------------------------
# GROUP NB_OPENSSL glue (to precompiled lib)
#-------------------------

$(eval $(call nbCall,nbInitCodeGrp))

NB_CODE_GRP_NAME            := openssl-glue-lib

NB_CODE_GRP_FLAGS_REQUIRED  += NB_LIB_SSL NB_LIB_SSL_SYSTEM

NB_CODE_GRP_SRCS            := \
    src/crypto/NBAes256.c \
    src/crypto/NBPkcs12.c \
    src/crypto/NBPKey.c \
    src/crypto/NBX500Name.c \
    src/crypto/NBX509.c \
    src/crypto/NBX509Req.c \
    src/ssl/NBSsl.c \
    src/ssl/NBSslContext.c

$(eval $(call nbCall,nbBuildCodeGrpRules))

#-------------------------
# GROUP NB_FREETYPE glue (to source)
#-------------------------

$(eval $(call nbCall,nbInitCodeGrp))

NB_CODE_GRP_NAME            := freetype-glue-src

NB_CODE_GRP_FLAGS_REQUIRED  += NB_LIB_FREETYPE

NB_CODE_GRP_FLAGS_FORBIDDEN += NB_LIB_FREETYPE_SYSTEM

NB_CODE_GRP_INCLUDES        += \
    src/ext/freetype/include

NB_CODE_GRP_SRCS            := \
    src/fonts/NBFontBitmaps.c \
    src/fonts/NBFontCodesMap.c \
    src/fonts/NBFontGlyphs.c \
    src/fonts/NBFontLines.c \
    src/fonts/NBFontsBitmapsStore.c \
    src/fonts/NBFontsGlyphsStore.c \
    src/fonts/NBFontShapesMetricsMap.c \
    src/fonts/NBFontsLinesStore.c \
    src/fonts/NBText.c \
    src/fonts/NBTextCursor.c \
    src/fonts/NBTextMetrics.c \
    src/fonts/NBTextMetricsBuilder.c

$(eval $(call nbCall,nbBuildCodeGrpRules))

#-------------------------
# GROUP NB_FREETYPE glue (to precompiled lib)
#-------------------------

$(eval $(call nbCall,nbInitCodeGrp))

NB_CODE_GRP_NAME            := freetype-glue-lib

NB_CODE_GRP_FLAGS_REQUIRED  += NB_LIB_FREETYPE NB_LIB_FREETYPE_SYSTEM

NB_CODE_GRP_INCLUDES        += \
    /usr/include/freetype2 \
    /usr/include/freetype2/freetype

NB_CODE_GRP_SRCS            := \
    src/fonts/NBFontBitmaps.c \
    src/fonts/NBFontCodesMap.c \
    src/fonts/NBFontGlyphs.c \
    src/fonts/NBFontLines.c \
    src/fonts/NBFontsBitmapsStore.c \
    src/fonts/NBFontsGlyphsStore.c \
    src/fonts/NBFontShapesMetricsMap.c \
    src/fonts/NBFontsLinesStore.c \
    src/fonts/NBText.c \
    src/fonts/NBTextCursor.c \
    src/fonts/NBTextMetrics.c \
    src/fonts/NBTextMetricsBuilder.c

$(eval $(call nbCall,nbBuildCodeGrpRules))

#-------------------------
# CODE GRP CORE
#-------------------------

$(eval $(call nbCall,nbInitCodeGrp))

NB_CODE_GRP_NAME            := core
 
NB_CODE_GRP_LIBS            += pthread m

NB_CODE_GRP_SRCS            := \
  src/NBFrameworkDefs.c \
  src/NBFrameworkPch.c \
  src/NBClass.c \
  src/NBContext.c \
  src/NBObject.c \
  src/NBObjectLnk.c \
  src/core/NBLocale.c \
  src/core/NBLocaleCfg.c \
  src/core/NBLocaleCfgAnlz.c \
  src/core/NBArray.c \
  src/core/NBArraySorted.c \
  src/core/NBLinkedList.c \
  src/core/NBCallback.c \
  src/core/NBCompare.c \
  src/core/NBDate.c \
  src/core/NBDatetime.c \
  src/core/NBEncoding.c \
  src/core/NBEnumMap.c \
  src/core/NBJson.c \
  src/core/NBJsonParser.c \
  src/core/NBLog.c \
  src/core/NBLogQueue.c \
  src/core/NBMemory.c \
  src/core/NBNumParser.c \
  src/core/NBPlistOld.c \
  src/core/NBPlistOldParser.c \
  src/core/NBRange.c \
  src/core/NBString.c \
  src/core/NBStringsLib.c \
  src/core/NBStringsPool.c \
  src/core/NBStruct.c \
  src/core/NBStructMap.c \
  src/core/NBStructMapMember.c \
  src/core/NBStructMaps.c \
  src/core/NBMngrStructMaps.c \
  src/core/NBMngrProcess.c \
  src/core/NBThreadsPool.c \
  src/core/NBThread.c \
  src/core/NBThreadCond.c \
  src/core/NBThreadMutex.c \
  src/core/NBThreadMutexRW.c \
  src/core/NBThreadStorage.c \
  src/core/NBXml.c \
  src/core/NBXmlParser.c \
  src/core/NBDataPtr.c \
  src/core/NBDataPtrsPool.c \
  src/core/NBDataPtrsStats.c \
  src/core/NBDataChunk.c \
  src/core/NBDataBuff.c \
  src/core/NBDataBuffsProvider.c \
  src/core/NBDataBuffsPool.c \
  src/core/NBDataBuffsStats.c \
  src/core/NBDataCursor.c \
  src/core/NBBitstream.c \
  src/2d/NBAABox.c \
  src/2d/NBAtlasMap.c \
  src/2d/NBBezier2.c \
  src/2d/NBBezier3.c \
  src/2d/NBBitmap.c \
  src/2d/NBColor.c \
  src/2d/NBMatrix.c \
  src/2d/NBPoint.c \
  src/2d/NBRect.c \
  src/2d/NBSize.c \
  src/2d/NBAADetection.c \
  src/2d/NBAvc.c \
  src/2d/NBAvcParser.c \
  src/2d/NBAvcBitstream.c \
  src/pdf/NBPdfRenderDoc.c \
  src/pdf/NBPdfRenderPage.c \
  src/pdf/NBPdfRenderPageItm.c \
  src/files/NBFile.c \
  src/files/NBFilesPkg.c \
  src/files/NBFilesPkgIndex.c \
  src/files/NBFilesystem.c \
  src/files/NBFilesystemPkgs.c \
  src/files/NBMp4.c \
  src/storage/NBStorages.c \
  src/storage/NBStorageCache.c \
  src/net/NBHttp2Hpack.c \
  src/net/NBHttp2Parser.c \
  src/net/NBHttp2Peer.c \
  src/net/NBHttpBody.c \
  src/net/NBHttpBuilder.c \
  src/net/NBHttpChunkExt.c \
  src/net/NBHttpClient.c \
  src/net/NBHttpHeader.c \
  src/net/NBHttpMessage.c \
  src/net/NBHttpParser.c \
  src/net/NBHttpProxy.c \
  src/net/NBHttpRequest.c \
  src/net/NBHttpResponse.c \
  src/net/NBHttpReqRule.c \
  src/net/NBHttpReqRules.c \
  src/net/NBHttpServicePort.c \
  src/net/NBHttpServiceConn.c \
  src/net/NBHttpServiceResp.c \
  src/net/NBHttpServiceRespLnk.c \
  src/net/NBHttpServiceRespRawLnk.c \
  src/net/NBHttpCfg.c \
  src/net/NBHttpStats.c \
  src/net/NBHttpService.c \
  src/net/NBHttpTransferCoding.c \
  src/net/NBHttpTransferEncoding.c \
  src/net/NBWebSocket.c \
  src/net/NBWebSocketMessage.c \
  src/net/NBWebSocketFrame.c \
  src/net/NBMsExchangeClt.c \
  src/net/NBNtln.c \
  src/net/NBOAuthClient.c \
  src/net/NBRtcp.c \
  src/net/NBRtcpParser.c \
  src/net/NBRtcpClient.c \
  src/net/NBRtp.c \
  src/net/NBRtpHeader.c \
  src/net/NBRtpQueue.c \
  src/net/NBRtpClient.c \
  src/net/NBRtpClientStats.c \
  src/net/NBSdp.c \
  src/net/NBRtsp.c \
  src/net/NBRtspClient.c \
  src/net/NBRtspClientConn.c \
  src/net/NBRtspClientStats.c \
  src/net/NBSmtpBody.c \
  src/net/NBSmtpClient.c \
  src/net/NBSmtpHeader.c \
  src/net/NBSocket.c \
  src/core/NBAtomicVar.c \
  src/core/NBStopFlag.c \
  src/core/NBHndlNative.c \
  src/core/NBHndl.c \
  src/core/NBIO.c \
  src/core/NBIOLnk.c \
  src/core/NBIOPollster.c \
  src/core/NBIOPollstersPool.c \
  src/core/NBIOPollstersProvider.c \
  src/net/NBUriBuilder.c \
  src/net/NBUriParser.c \
  src/net/NBUrl.c \
  src/net/NBStompFrame.c \
  src/crypto/NBBase64.c \
  src/crypto/NBBase64Url.c \
  src/crypto/NBCrc32.c \
  src/crypto/NBRc4A.c \
  src/crypto/NBMd4.c \
  src/crypto/NBMd5.c \
  src/crypto/NBHmac.c \
  src/crypto/NBSha1.c \
  src/crypto/NBKeychain.c

#Specific OS
ifneq (,$(findstring Android,$(NB_CFG_HOST)))
  #Android
  NB_CODE_GRP_SRCS          += \
    src/files/NBFilesystemAndroid.c
else
ifeq ($(OS),Windows_NT)
  #Windows
else
  UNAME_S                   := $(shell uname -s)
  ifeq ($(UNAME_S),Linux)
    #Linux
  endif
  ifeq ($(UNAME_S),Darwin)
    #OSX
    NB_CODE_GRP_SRCS        += \
    src/files/NBFilesystemApple.m \
    src/crypto/NBKeychainApple.c
  endif
endif
endif

$(eval $(call nbCall,nbBuildCodeGrpRules))

#-------------------------
# GROUP NB_LIB_Z glue (to source)
#-------------------------

$(eval $(call nbCall,nbInitCodeGrp))

NB_CODE_GRP_NAME            := zlib-glue-src

NB_CODE_GRP_FLAGS_REQUIRED  += NB_LIB_Z

NB_CODE_GRP_FLAGS_FORBIDDEN += NB_LIB_Z_SYSTEM

NB_CODE_GRP_SRCS            := \
    src/compress/NBZlib.c \
    src/compress/NBZInflate.c \
    src/compress/NBZDeflate.c

NB_CODE_GRP_INCLUDES        += \
    include/nb/ext/zlib \
    src/ext/zlib

$(eval $(call nbCall,nbBuildCodeGrpRules))

#-------------------------
# GROUP NB_LIB_Z glue (to precompiled lib)
#-------------------------

$(eval $(call nbCall,nbInitCodeGrp))

NB_CODE_GRP_NAME            := zlib-glue-lib

NB_CODE_GRP_FLAGS_REQUIRED  += NB_LIB_Z NB_LIB_Z_SYSTEM
 
NB_CODE_GRP_SRCS            := \
  src/compress/NBZlib.c \
  src/compress/NBZInflate.c \
  src/compress/NBZDeflate.c

$(eval $(call nbCall,nbBuildCodeGrpRules))

#-------------------------
# GROUP NB_LIB_JPEG (from source)
#-------------------------

$(eval $(call nbCall,nbInitCodeGrp))

NB_CODE_GRP_NAME            := libjpeg-src

NB_CODE_GRP_FLAGS_REQUIRED  += NB_LIB_JPEG

NB_CODE_GRP_FLAGS_FORBIDDEN += NB_LIB_JPEG_SYSTEM

NB_CODE_GRP_INCLUDES        += \
    src/ext/libjpeg

NB_CODE_GRP_SRCS            := \
    src/ext/libjpeg/jaricom.c \
    src/ext/libjpeg/jcapimin.c \
    src/ext/libjpeg/jcapistd.c \
    src/ext/libjpeg/jcarith.c \
    src/ext/libjpeg/jccoefct.c \
    src/ext/libjpeg/jccolor.c \
    src/ext/libjpeg/jcdctmgr.c \
    src/ext/libjpeg/jchuff.c \
    src/ext/libjpeg/jcinit.c \
    src/ext/libjpeg/jcmainct.c \
    src/ext/libjpeg/jcmarker.c \
    src/ext/libjpeg/jcmaster.c \
    src/ext/libjpeg/jcomapi.c \
    src/ext/libjpeg/jcparam.c \
    src/ext/libjpeg/jcprepct.c \
    src/ext/libjpeg/jcsample.c \
    src/ext/libjpeg/jctrans.c \
    src/ext/libjpeg/jdapimin.c \
    src/ext/libjpeg/jdapistd.c \
    src/ext/libjpeg/jdarith.c \
    src/ext/libjpeg/jdatadst.c \
    src/ext/libjpeg/jdatasrc.c \
    src/ext/libjpeg/jdcoefct.c \
    src/ext/libjpeg/jdcolor.c \
    src/ext/libjpeg/jddctmgr.c \
    src/ext/libjpeg/jdhuff.c \
    src/ext/libjpeg/jdinput.c \
    src/ext/libjpeg/jdmainct.c \
    src/ext/libjpeg/jdmarker.c \
    src/ext/libjpeg/jdmaster.c \
    src/ext/libjpeg/jdmerge.c \
    src/ext/libjpeg/jdpostct.c \
    src/ext/libjpeg/jdsample.c \
    src/ext/libjpeg/jdtrans.c \
    src/ext/libjpeg/jerror.c \
    src/ext/libjpeg/jfdctflt.c \
    src/ext/libjpeg/jfdctfst.c \
    src/ext/libjpeg/jfdctint.c \
    src/ext/libjpeg/jidctflt.c \
    src/ext/libjpeg/jidctfst.c \
    src/ext/libjpeg/jidctint.c \
    src/ext/libjpeg/jmemmgr.c \
    src/ext/libjpeg/jmemnobs.c \
    src/ext/libjpeg/jquant1.c \
    src/ext/libjpeg/jquant2.c \
    src/ext/libjpeg/jutils.c

$(eval $(call nbCall,nbBuildCodeGrpRules))

#-------------------------
# GROUP NB_LIB_JPEG (from precompiled library)
#-------------------------

$(eval $(call nbCall,nbInitCodeGrp))

NB_CODE_GRP_NAME            := libjpeg-lib

NB_CODE_GRP_FLAGS_REQUIRED  += NB_LIB_JPEG NB_LIB_JPEG_SYSTEM

NB_CODE_GRP_LIBS            += jpeg

$(eval $(call nbCall,nbBuildCodeGrpRules))

#-------------------------
# GROUP NB_FREETYPE (from source)
#-------------------------

$(eval $(call nbCall,nbInitCodeGrp))

NB_CODE_GRP_NAME            := freetype-src

NB_CODE_GRP_FLAGS_REQUIRED  += NB_LIB_FREETYPE

NB_CODE_GRP_FLAGS_FORBIDDEN += NB_LIB_FREETYPE_SYSTEM

NB_CODE_GRP_INCLUDES        += \
    src/ext/freetype/include

NB_CODE_GRP_SRCS            := \
    src/ext/freetype/src/autofit/autofit.c \
    src/ext/freetype/src/base/ftbase.c \
    src/ext/freetype/src/base/ftbitmap.c \
    src/ext/freetype/src/base/ftinit.c \
    src/ext/freetype/src/base/ftsystem.c \
    src/ext/freetype/src/cff/cff.c \
    src/ext/freetype/src/gzip/ftgzip.c \
    src/ext/freetype/src/pshinter/pshinter.c \
    src/ext/freetype/src/psnames/psnames.c \
    src/ext/freetype/src/raster/raster.c \
    src/ext/freetype/src/sfnt/sfnt.c \
    src/ext/freetype/src/smooth/smooth.c \
    src/ext/freetype/src/truetype/truetype.c \
    src/ext/freetype/src/winfonts/winfnt.c
       
#only if NB_LIB_Z_SYSTEM is not set
ifeq ($(NB_LIB_Z_SYSTEM),)
    NB_CODE_GRP_CFLAGS      += \
        -DFT_CONFIG_OPTION_SYSTEM_ZLIB
    NB_CODE_GRP_CXXFLAGS    += \
        -DFT_CONFIG_OPTION_SYSTEM_ZLIB
    NB_CODE_GRP_INCLUDES    += \
        include/nb/ext/zlib \
        src/ext/zlib
endif

$(eval $(call nbCall,nbBuildCodeGrpRules))

#-------------------------
# GROUP NB_FREETYPE (from precompiled lib)
#-------------------------

$(eval $(call nbCall,nbInitCodeGrp))

NB_CODE_GRP_NAME            := freetype-lib

NB_CODE_GRP_FLAGS_REQUIRED  += NB_LIB_FREETYPE NB_LIB_FREETYPE_SYSTEM 

NB_CODE_GRP_LIBS            += freetype

$(eval $(call nbCall,nbBuildCodeGrpRules))

#-------------------------
# GROUP NB_OPENSSL (from source)
#-------------------------

$(eval $(call nbCall,nbInitCodeGrp))

NB_CODE_GRP_NAME            := openssl-src

NB_CODE_GRP_FLAGS_REQUIRED  += NB_LIB_SSL

NB_CODE_GRP_FLAGS_FORBIDDEN += NB_LIB_SSL_SYSTEM

NB_CODE_GRP_CFLAGS          += \
    -DOPENSSL_THREADS \
    -DOPENSSL_NO_DYNAMIC_ENGINE \
    -DOPENSSL_PIC \
    -DOPENSSLDIR="\".\""

NB_CODE_GRP_CXXFLAGS        += \
    -DOPENSSL_THREADS \
    -DOPENSSL_NO_DYNAMIC_ENGINE \
    -DOPENSSL_PIC \
    -DOPENSSLDIR="\".\""

NB_CODE_GRP_INCLUDES        += \
    include/nb/ext/openssl/include \
    include/nb/ext/openssl/crypto \
    include/nb/ext/openssl/crypto/include \
    src/ext/openssl \
    src/ext/openssl/crypto/include \
    src/ext/openssl/include \
    src/ext/openssl/crypto \
    src/ext/openssl/crypto/modes \
    src/ext/openssl/crypto/ec/curve448 \
    src/ext/openssl/crypto/ec/curve448/arch_32
    
NB_CODE_GRP_SRCS            := \
    src/ext/openssl/crypto/aes/aes_cbc.c \
    src/ext/openssl/crypto/aes/aes_cfb.c \
    src/ext/openssl/crypto/aes/aes_core.c \
    src/ext/openssl/crypto/aes/aes_ecb.c \
    src/ext/openssl/crypto/aes/aes_ige.c \
    src/ext/openssl/crypto/aes/aes_misc.c \
    src/ext/openssl/crypto/aes/aes_ofb.c \
    src/ext/openssl/crypto/aes/aes_wrap.c \
    src/ext/openssl/crypto/aria/aria.c \
    src/ext/openssl/crypto/asn1/a_bitstr.c \
    src/ext/openssl/crypto/asn1/a_d2i_fp.c \
    src/ext/openssl/crypto/asn1/a_digest.c \
    src/ext/openssl/crypto/asn1/a_dup.c \
    src/ext/openssl/crypto/asn1/a_gentm.c \
    src/ext/openssl/crypto/asn1/a_i2d_fp.c \
    src/ext/openssl/crypto/asn1/a_int.c \
    src/ext/openssl/crypto/asn1/a_mbstr.c \
    src/ext/openssl/crypto/asn1/a_object.c \
    src/ext/openssl/crypto/asn1/a_octet.c \
    src/ext/openssl/crypto/asn1/a_print.c \
    src/ext/openssl/crypto/asn1/a_sign.c \
    src/ext/openssl/crypto/asn1/a_strex.c \
    src/ext/openssl/crypto/asn1/a_strnid.c \
    src/ext/openssl/crypto/asn1/a_time.c \
    src/ext/openssl/crypto/asn1/a_type.c \
    src/ext/openssl/crypto/asn1/a_utctm.c \
    src/ext/openssl/crypto/asn1/a_utf8.c \
    src/ext/openssl/crypto/asn1/a_verify.c \
    src/ext/openssl/crypto/asn1/ameth_lib.c \
    src/ext/openssl/crypto/asn1/asn_mime.c \
    src/ext/openssl/crypto/asn1/asn_moid.c \
    src/ext/openssl/crypto/asn1/asn_mstbl.c \
    src/ext/openssl/crypto/asn1/asn_pack.c \
    src/ext/openssl/crypto/asn1/asn1_err.c \
    src/ext/openssl/crypto/asn1/asn1_gen.c \
    src/ext/openssl/crypto/asn1/asn1_item_list.c \
    src/ext/openssl/crypto/asn1/asn1_lib.c \
    src/ext/openssl/crypto/asn1/asn1_par.c \
    src/ext/openssl/crypto/asn1/bio_asn1.c \
    src/ext/openssl/crypto/asn1/bio_ndef.c \
    src/ext/openssl/crypto/asn1/d2i_pr.c \
    src/ext/openssl/crypto/asn1/d2i_pu.c \
    src/ext/openssl/crypto/asn1/evp_asn1.c \
    src/ext/openssl/crypto/asn1/f_int.c \
    src/ext/openssl/crypto/asn1/f_string.c \
    src/ext/openssl/crypto/asn1/i2d_pr.c \
    src/ext/openssl/crypto/asn1/i2d_pu.c \
    src/ext/openssl/crypto/asn1/n_pkey.c \
    src/ext/openssl/crypto/asn1/nsseq.c \
    src/ext/openssl/crypto/asn1/p5_pbe.c \
    src/ext/openssl/crypto/asn1/p5_pbev2.c \
    src/ext/openssl/crypto/asn1/p5_scrypt.c \
    src/ext/openssl/crypto/asn1/p8_pkey.c \
    src/ext/openssl/crypto/asn1/t_bitst.c \
    src/ext/openssl/crypto/asn1/t_pkey.c \
    src/ext/openssl/crypto/asn1/t_spki.c \
    src/ext/openssl/crypto/asn1/tasn_dec.c \
    src/ext/openssl/crypto/asn1/tasn_enc.c \
    src/ext/openssl/crypto/asn1/tasn_fre.c \
    src/ext/openssl/crypto/asn1/tasn_new.c \
    src/ext/openssl/crypto/asn1/tasn_prn.c \
    src/ext/openssl/crypto/asn1/tasn_scn.c \
    src/ext/openssl/crypto/asn1/tasn_typ.c \
    src/ext/openssl/crypto/asn1/tasn_utl.c \
    src/ext/openssl/crypto/asn1/x_algor.c \
    src/ext/openssl/crypto/asn1/x_bignum.c \
    src/ext/openssl/crypto/asn1/x_info.c \
    src/ext/openssl/crypto/asn1/x_int64.c \
    src/ext/openssl/crypto/asn1/x_long.c \
    src/ext/openssl/crypto/asn1/x_pkey.c \
    src/ext/openssl/crypto/asn1/x_sig.c \
    src/ext/openssl/crypto/asn1/x_spki.c \
    src/ext/openssl/crypto/asn1/x_val.c \
    src/ext/openssl/crypto/async/arch/async_null.c \
    src/ext/openssl/crypto/async/arch/async_posix.c \
    src/ext/openssl/crypto/async/arch/async_win.c \
    src/ext/openssl/crypto/async/async_err.c \
    src/ext/openssl/crypto/async/async_wait.c \
    src/ext/openssl/crypto/async/async.c \
    src/ext/openssl/crypto/bf/bf_cfb64.c \
    src/ext/openssl/crypto/bf/bf_ecb.c \
    src/ext/openssl/crypto/bf/bf_enc.c \
    src/ext/openssl/crypto/bf/bf_ofb64.c \
    src/ext/openssl/crypto/bf/bf_skey.c \
    src/ext/openssl/crypto/bio/b_addr.c \
    src/ext/openssl/crypto/bio/b_dump.c \
    src/ext/openssl/crypto/bio/b_print.c \
    src/ext/openssl/crypto/bio/b_sock.c \
    src/ext/openssl/crypto/bio/b_sock2.c \
    src/ext/openssl/crypto/bio/bf_buff.c \
    src/ext/openssl/crypto/bio/bf_lbuf.c \
    src/ext/openssl/crypto/bio/bf_nbio.c \
    src/ext/openssl/crypto/bio/bf_null.c \
    src/ext/openssl/crypto/bio/bio_cb.c \
    src/ext/openssl/crypto/bio/bio_err.c \
    src/ext/openssl/crypto/bio/bio_lib.c \
    src/ext/openssl/crypto/bio/bio_meth.c \
    src/ext/openssl/crypto/bio/bss_acpt.c \
    src/ext/openssl/crypto/bio/bss_bio.c \
    src/ext/openssl/crypto/bio/bss_conn.c \
    src/ext/openssl/crypto/bio/bss_dgram.c \
    src/ext/openssl/crypto/bio/bss_fd.c \
    src/ext/openssl/crypto/bio/bss_file.c \
    src/ext/openssl/crypto/bio/bss_log.c \
    src/ext/openssl/crypto/bio/bss_mem.c \
    src/ext/openssl/crypto/bio/bss_null.c \
    src/ext/openssl/crypto/bio/bss_sock.c \
    src/ext/openssl/crypto/blake2/blake2b.c \
    src/ext/openssl/crypto/blake2/blake2s.c \
    src/ext/openssl/crypto/blake2/m_blake2b.c \
    src/ext/openssl/crypto/blake2/m_blake2s.c \
    src/ext/openssl/crypto/bn/bn_add.c \
    src/ext/openssl/crypto/bn/bn_asm.c \
    src/ext/openssl/crypto/bn/bn_blind.c \
    src/ext/openssl/crypto/bn/bn_const.c \
    src/ext/openssl/crypto/bn/bn_ctx.c \
    src/ext/openssl/crypto/bn/bn_depr.c \
    src/ext/openssl/crypto/bn/bn_dh.c \
    src/ext/openssl/crypto/bn/bn_div.c \
    src/ext/openssl/crypto/bn/bn_err.c \
    src/ext/openssl/crypto/bn/bn_exp.c \
    src/ext/openssl/crypto/bn/bn_exp2.c \
    src/ext/openssl/crypto/bn/bn_gcd.c \
    src/ext/openssl/crypto/bn/bn_gf2m.c \
    src/ext/openssl/crypto/bn/bn_intern.c \
    src/ext/openssl/crypto/bn/bn_kron.c \
    src/ext/openssl/crypto/bn/bn_lib.c \
    src/ext/openssl/crypto/bn/bn_mod.c \
    src/ext/openssl/crypto/bn/bn_mont.c \
    src/ext/openssl/crypto/bn/bn_mpi.c \
    src/ext/openssl/crypto/bn/bn_mul.c \
    src/ext/openssl/crypto/bn/bn_nist.c \
    src/ext/openssl/crypto/bn/bn_prime.c \
    src/ext/openssl/crypto/bn/bn_print.c \
    src/ext/openssl/crypto/bn/bn_rand.c \
    src/ext/openssl/crypto/bn/bn_recp.c \
    src/ext/openssl/crypto/bn/bn_shift.c \
    src/ext/openssl/crypto/bn/bn_sqr.c \
    src/ext/openssl/crypto/bn/bn_sqrt.c \
    src/ext/openssl/crypto/bn/bn_srp.c \
    src/ext/openssl/crypto/bn/bn_word.c \
    src/ext/openssl/crypto/bn/bn_x931p.c \
    src/ext/openssl/crypto/buffer/buf_err.c \
    src/ext/openssl/crypto/buffer/buffer.c \
    src/ext/openssl/crypto/camellia/camellia.c \
    src/ext/openssl/crypto/camellia/cmll_cbc.c \
    src/ext/openssl/crypto/camellia/cmll_cfb.c \
    src/ext/openssl/crypto/camellia/cmll_ctr.c \
    src/ext/openssl/crypto/camellia/cmll_ecb.c \
    src/ext/openssl/crypto/camellia/cmll_misc.c \
    src/ext/openssl/crypto/camellia/cmll_ofb.c \
    src/ext/openssl/crypto/cast/c_cfb64.c \
    src/ext/openssl/crypto/cast/c_ecb.c \
    src/ext/openssl/crypto/cast/c_enc.c \
    src/ext/openssl/crypto/cast/c_ofb64.c \
    src/ext/openssl/crypto/cast/c_skey.c \
    src/ext/openssl/crypto/chacha/chacha_enc.c \
    src/ext/openssl/crypto/cmac/cm_ameth.c \
    src/ext/openssl/crypto/cmac/cm_pmeth.c \
    src/ext/openssl/crypto/cmac/cmac.c \
    src/ext/openssl/crypto/cms/cms_asn1.c \
    src/ext/openssl/crypto/cms/cms_att.c \
    src/ext/openssl/crypto/cms/cms_cd.c \
    src/ext/openssl/crypto/cms/cms_dd.c \
    src/ext/openssl/crypto/cms/cms_enc.c \
    src/ext/openssl/crypto/cms/cms_env.c \
    src/ext/openssl/crypto/cms/cms_err.c \
    src/ext/openssl/crypto/cms/cms_ess.c \
    src/ext/openssl/crypto/cms/cms_io.c \
    src/ext/openssl/crypto/cms/cms_kari.c \
    src/ext/openssl/crypto/cms/cms_lib.c \
    src/ext/openssl/crypto/cms/cms_pwri.c \
    src/ext/openssl/crypto/cms/cms_sd.c \
    src/ext/openssl/crypto/cms/cms_smime.c \
    src/ext/openssl/crypto/conf/conf_api.c \
    src/ext/openssl/crypto/conf/conf_def.c \
    src/ext/openssl/crypto/conf/conf_err.c \
    src/ext/openssl/crypto/conf/conf_lib.c \
    src/ext/openssl/crypto/conf/conf_mall.c \
    src/ext/openssl/crypto/conf/conf_mod.c \
    src/ext/openssl/crypto/conf/conf_sap.c \
    src/ext/openssl/crypto/conf/conf_ssl.c \
    src/ext/openssl/crypto/cpt_err.c \
    src/ext/openssl/crypto/cryptlib.c \
    src/ext/openssl/crypto/ct/ct_b64.c \
    src/ext/openssl/crypto/ct/ct_err.c \
    src/ext/openssl/crypto/ct/ct_log.c \
    src/ext/openssl/crypto/ct/ct_oct.c \
    src/ext/openssl/crypto/ct/ct_policy.c \
    src/ext/openssl/crypto/ct/ct_prn.c \
    src/ext/openssl/crypto/ct/ct_sct_ctx.c \
    src/ext/openssl/crypto/ct/ct_sct.c \
    src/ext/openssl/crypto/ct/ct_vfy.c \
    src/ext/openssl/crypto/ct/ct_x509v3.c \
    src/ext/openssl/crypto/ctype.c \
    src/ext/openssl/crypto/cversion.c \
    src/ext/openssl/crypto/des/cbc_cksm.c \
    src/ext/openssl/crypto/des/cbc_enc.c \
    src/ext/openssl/crypto/des/cfb_enc.c \
    src/ext/openssl/crypto/des/cfb64ede.c \
    src/ext/openssl/crypto/des/cfb64enc.c \
    src/ext/openssl/crypto/des/des_enc.c \
    src/ext/openssl/crypto/des/ecb_enc.c \
    src/ext/openssl/crypto/des/ecb3_enc.c \
    src/ext/openssl/crypto/des/fcrypt_b.c \
    src/ext/openssl/crypto/des/fcrypt.c \
    src/ext/openssl/crypto/des/ofb_enc.c \
    src/ext/openssl/crypto/des/ofb64ede.c \
    src/ext/openssl/crypto/des/ofb64enc.c \
    src/ext/openssl/crypto/des/pcbc_enc.c \
    src/ext/openssl/crypto/des/qud_cksm.c \
    src/ext/openssl/crypto/des/rand_key.c \
    src/ext/openssl/crypto/des/set_key.c \
    src/ext/openssl/crypto/des/str2key.c \
    src/ext/openssl/crypto/des/xcbc_enc.c \
    src/ext/openssl/crypto/dh/dh_ameth.c \
    src/ext/openssl/crypto/dh/dh_asn1.c \
    src/ext/openssl/crypto/dh/dh_check.c \
    src/ext/openssl/crypto/dh/dh_depr.c \
    src/ext/openssl/crypto/dh/dh_err.c \
    src/ext/openssl/crypto/dh/dh_gen.c \
    src/ext/openssl/crypto/dh/dh_kdf.c \
    src/ext/openssl/crypto/dh/dh_key.c \
    src/ext/openssl/crypto/dh/dh_lib.c \
    src/ext/openssl/crypto/dh/dh_meth.c \
    src/ext/openssl/crypto/dh/dh_pmeth.c \
    src/ext/openssl/crypto/dh/dh_prn.c \
    src/ext/openssl/crypto/dh/dh_rfc5114.c \
    src/ext/openssl/crypto/dh/dh_rfc7919.c \
    src/ext/openssl/crypto/dsa/dsa_ameth.c \
    src/ext/openssl/crypto/dsa/dsa_asn1.c \
    src/ext/openssl/crypto/dsa/dsa_depr.c \
    src/ext/openssl/crypto/dsa/dsa_err.c \
    src/ext/openssl/crypto/dsa/dsa_gen.c \
    src/ext/openssl/crypto/dsa/dsa_key.c \
    src/ext/openssl/crypto/dsa/dsa_lib.c \
    src/ext/openssl/crypto/dsa/dsa_meth.c \
    src/ext/openssl/crypto/dsa/dsa_ossl.c \
    src/ext/openssl/crypto/dsa/dsa_pmeth.c \
    src/ext/openssl/crypto/dsa/dsa_prn.c \
    src/ext/openssl/crypto/dsa/dsa_sign.c \
    src/ext/openssl/crypto/dsa/dsa_vrf.c \
    src/ext/openssl/crypto/dso/dso_dl.c \
    src/ext/openssl/crypto/dso/dso_dlfcn.c \
    src/ext/openssl/crypto/dso/dso_err.c \
    src/ext/openssl/crypto/dso/dso_lib.c \
    src/ext/openssl/crypto/dso/dso_openssl.c \
    src/ext/openssl/crypto/dso/dso_vms.c \
    src/ext/openssl/crypto/dso/dso_win32.c \
    src/ext/openssl/crypto/ebcdic.c \
    src/ext/openssl/crypto/ec/curve25519.c \
    src/ext/openssl/crypto/ec/curve448/arch_32/f_impl.c \
    src/ext/openssl/crypto/ec/curve448/curve448_tables.c \
    src/ext/openssl/crypto/ec/curve448/curve448.c \
    src/ext/openssl/crypto/ec/curve448/eddsa.c \
    src/ext/openssl/crypto/ec/curve448/f_generic.c \
    src/ext/openssl/crypto/ec/curve448/scalar.c \
    src/ext/openssl/crypto/ec/ec_ameth.c \
    src/ext/openssl/crypto/ec/ec_asn1.c \
    src/ext/openssl/crypto/ec/ec_check.c \
    src/ext/openssl/crypto/ec/ec_curve.c \
    src/ext/openssl/crypto/ec/ec_cvt.c \
    src/ext/openssl/crypto/ec/ec_err.c \
    src/ext/openssl/crypto/ec/ec_key.c \
    src/ext/openssl/crypto/ec/ec_kmeth.c \
    src/ext/openssl/crypto/ec/ec_lib.c \
    src/ext/openssl/crypto/ec/ec_mult.c \
    src/ext/openssl/crypto/ec/ec_oct.c \
    src/ext/openssl/crypto/ec/ec_pmeth.c \
    src/ext/openssl/crypto/ec/ec_print.c \
    src/ext/openssl/crypto/ec/ec2_oct.c \
    src/ext/openssl/crypto/ec/ec2_smpl.c \
    src/ext/openssl/crypto/ec/ecdh_kdf.c \
    src/ext/openssl/crypto/ec/ecdh_ossl.c \
    src/ext/openssl/crypto/ec/ecdsa_ossl.c \
    src/ext/openssl/crypto/ec/ecdsa_sign.c \
    src/ext/openssl/crypto/ec/ecdsa_vrf.c \
    src/ext/openssl/crypto/ec/eck_prn.c \
    src/ext/openssl/crypto/ec/ecp_mont.c \
    src/ext/openssl/crypto/ec/ecp_nist.c \
    src/ext/openssl/crypto/ec/ecp_nistp224.c \
    src/ext/openssl/crypto/ec/ecp_nistp256.c \
    src/ext/openssl/crypto/ec/ecp_nistp521.c \
    src/ext/openssl/crypto/ec/ecp_nistputil.c \
    src/ext/openssl/crypto/ec/ecp_oct.c \
    src/ext/openssl/crypto/ec/ecp_smpl.c \
    src/ext/openssl/crypto/ec/ecx_meth.c \
    src/ext/openssl/crypto/err/err_all.c \
    src/ext/openssl/crypto/err/err_prn.c \
    src/ext/openssl/crypto/err/err.c \
    src/ext/openssl/crypto/evp/bio_b64.c \
    src/ext/openssl/crypto/evp/bio_enc.c \
    src/ext/openssl/crypto/evp/bio_md.c \
    src/ext/openssl/crypto/evp/bio_ok.c \
    src/ext/openssl/crypto/evp/c_allc.c \
    src/ext/openssl/crypto/evp/c_alld.c \
    src/ext/openssl/crypto/evp/cmeth_lib.c \
    src/ext/openssl/crypto/evp/digest.c \
    src/ext/openssl/crypto/evp/e_aes_cbc_hmac_sha1.c \
    src/ext/openssl/crypto/evp/e_aes_cbc_hmac_sha256.c \
    src/ext/openssl/crypto/evp/e_aes.c \
    src/ext/openssl/crypto/evp/e_aria.c \
    src/ext/openssl/crypto/evp/e_bf.c \
    src/ext/openssl/crypto/evp/e_camellia.c \
    src/ext/openssl/crypto/evp/e_cast.c \
    src/ext/openssl/crypto/evp/e_chacha20_poly1305.c \
    src/ext/openssl/crypto/evp/e_des.c \
    src/ext/openssl/crypto/evp/e_des3.c \
    src/ext/openssl/crypto/evp/e_idea.c \
    src/ext/openssl/crypto/evp/e_null.c \
    src/ext/openssl/crypto/evp/e_old.c \
    src/ext/openssl/crypto/evp/e_rc2.c \
    src/ext/openssl/crypto/evp/e_rc4_hmac_md5.c \
    src/ext/openssl/crypto/evp/e_rc4.c \
    src/ext/openssl/crypto/evp/e_rc5.c \
    src/ext/openssl/crypto/evp/e_seed.c \
    src/ext/openssl/crypto/evp/e_sm4.c \
    src/ext/openssl/crypto/evp/e_xcbc_d.c \
    src/ext/openssl/crypto/evp/encode.c \
    src/ext/openssl/crypto/evp/evp_cnf.c \
    src/ext/openssl/crypto/evp/evp_enc.c \
    src/ext/openssl/crypto/evp/evp_err.c \
    src/ext/openssl/crypto/evp/evp_key.c \
    src/ext/openssl/crypto/evp/evp_lib.c \
    src/ext/openssl/crypto/evp/evp_pbe.c \
    src/ext/openssl/crypto/evp/evp_pkey.c \
    src/ext/openssl/crypto/evp/m_md2.c \
    src/ext/openssl/crypto/evp/m_md4.c \
    src/ext/openssl/crypto/evp/m_md5_sha1.c \
    src/ext/openssl/crypto/evp/m_md5.c \
    src/ext/openssl/crypto/evp/m_mdc2.c \
    src/ext/openssl/crypto/evp/m_null.c \
    src/ext/openssl/crypto/evp/m_ripemd.c \
    src/ext/openssl/crypto/evp/m_sha1.c \
    src/ext/openssl/crypto/evp/m_sha3.c \
    src/ext/openssl/crypto/evp/m_sigver.c \
    src/ext/openssl/crypto/evp/m_wp.c \
    src/ext/openssl/crypto/evp/names.c \
    src/ext/openssl/crypto/evp/p_dec.c \
    src/ext/openssl/crypto/evp/p_enc.c \
    src/ext/openssl/crypto/evp/p_lib.c \
    src/ext/openssl/crypto/evp/p_open.c \
    src/ext/openssl/crypto/evp/p_seal.c \
    src/ext/openssl/crypto/evp/p_sign.c \
    src/ext/openssl/crypto/evp/p_verify.c \
    src/ext/openssl/crypto/evp/p5_crpt.c \
    src/ext/openssl/crypto/evp/p5_crpt2.c \
    src/ext/openssl/crypto/evp/pbe_scrypt.c \
    src/ext/openssl/crypto/evp/pmeth_fn.c \
    src/ext/openssl/crypto/evp/pmeth_gn.c \
    src/ext/openssl/crypto/evp/pmeth_lib.c \
    src/ext/openssl/crypto/ex_data.c \
    src/ext/openssl/crypto/getenv.c \
    src/ext/openssl/crypto/hmac/hm_ameth.c \
    src/ext/openssl/crypto/hmac/hm_pmeth.c \
    src/ext/openssl/crypto/hmac/hmac.c \
    src/ext/openssl/crypto/idea/i_cbc.c \
    src/ext/openssl/crypto/idea/i_cfb64.c \
    src/ext/openssl/crypto/idea/i_ecb.c \
    src/ext/openssl/crypto/idea/i_ofb64.c \
    src/ext/openssl/crypto/idea/i_skey.c \
    src/ext/openssl/crypto/init.c \
    src/ext/openssl/crypto/kdf/hkdf.c \
    src/ext/openssl/crypto/kdf/kdf_err.c \
    src/ext/openssl/crypto/kdf/scrypt.c \
    src/ext/openssl/crypto/kdf/tls1_prf.c \
    src/ext/openssl/crypto/lhash/lh_stats.c \
    src/ext/openssl/crypto/lhash/lhash.c \
    src/ext/openssl/crypto/md4/md4_dgst.c \
    src/ext/openssl/crypto/md4/md4_one.c \
    src/ext/openssl/crypto/md5/md5_dgst.c \
    src/ext/openssl/crypto/md5/md5_one.c \
    src/ext/openssl/crypto/mdc2/mdc2_one.c \
    src/ext/openssl/crypto/mdc2/mdc2dgst.c \
    src/ext/openssl/crypto/mem_clr.c \
    src/ext/openssl/crypto/mem_dbg.c \
    src/ext/openssl/crypto/mem_sec.c \
    src/ext/openssl/crypto/mem.c \
    src/ext/openssl/crypto/modes/cbc128.c \
    src/ext/openssl/crypto/modes/ccm128.c \
    src/ext/openssl/crypto/modes/cfb128.c \
    src/ext/openssl/crypto/modes/ctr128.c \
    src/ext/openssl/crypto/modes/cts128.c \
    src/ext/openssl/crypto/modes/gcm128.c \
    src/ext/openssl/crypto/modes/ocb128.c \
    src/ext/openssl/crypto/modes/ofb128.c \
    src/ext/openssl/crypto/modes/wrap128.c \
    src/ext/openssl/crypto/modes/xts128.c \
    src/ext/openssl/crypto/o_dir.c \
    src/ext/openssl/crypto/o_fips.c \
    src/ext/openssl/crypto/o_fopen.c \
    src/ext/openssl/crypto/o_init.c \
    src/ext/openssl/crypto/o_str.c \
    src/ext/openssl/crypto/o_time.c \
    src/ext/openssl/crypto/objects/o_names.c \
    src/ext/openssl/crypto/objects/obj_dat.c \
    src/ext/openssl/crypto/objects/obj_err.c \
    src/ext/openssl/crypto/objects/obj_lib.c \
    src/ext/openssl/crypto/objects/obj_xref.c \
    src/ext/openssl/crypto/ocsp/ocsp_asn.c \
    src/ext/openssl/crypto/ocsp/ocsp_cl.c \
    src/ext/openssl/crypto/ocsp/ocsp_err.c \
    src/ext/openssl/crypto/ocsp/ocsp_ext.c \
    src/ext/openssl/crypto/ocsp/ocsp_ht.c \
    src/ext/openssl/crypto/ocsp/ocsp_lib.c \
    src/ext/openssl/crypto/ocsp/ocsp_prn.c \
    src/ext/openssl/crypto/ocsp/ocsp_srv.c \
    src/ext/openssl/crypto/ocsp/ocsp_vfy.c \
    src/ext/openssl/crypto/ocsp/v3_ocsp.c \
    src/ext/openssl/crypto/pem/pem_all.c \
    src/ext/openssl/crypto/pem/pem_err.c \
    src/ext/openssl/crypto/pem/pem_info.c \
    src/ext/openssl/crypto/pem/pem_lib.c \
    src/ext/openssl/crypto/pem/pem_oth.c \
    src/ext/openssl/crypto/pem/pem_pk8.c \
    src/ext/openssl/crypto/pem/pem_pkey.c \
    src/ext/openssl/crypto/pem/pem_sign.c \
    src/ext/openssl/crypto/pem/pem_x509.c \
    src/ext/openssl/crypto/pem/pem_xaux.c \
    src/ext/openssl/crypto/pem/pvkfmt.c \
    src/ext/openssl/crypto/pkcs12/p12_add.c \
    src/ext/openssl/crypto/pkcs12/p12_asn.c \
    src/ext/openssl/crypto/pkcs12/p12_attr.c \
    src/ext/openssl/crypto/pkcs12/p12_crpt.c \
    src/ext/openssl/crypto/pkcs12/p12_crt.c \
    src/ext/openssl/crypto/pkcs12/p12_decr.c \
    src/ext/openssl/crypto/pkcs12/p12_init.c \
    src/ext/openssl/crypto/pkcs12/p12_key.c \
    src/ext/openssl/crypto/pkcs12/p12_kiss.c \
    src/ext/openssl/crypto/pkcs12/p12_mutl.c \
    src/ext/openssl/crypto/pkcs12/p12_npas.c \
    src/ext/openssl/crypto/pkcs12/p12_p8d.c \
    src/ext/openssl/crypto/pkcs12/p12_p8e.c \
    src/ext/openssl/crypto/pkcs12/p12_sbag.c \
    src/ext/openssl/crypto/pkcs12/p12_utl.c \
    src/ext/openssl/crypto/pkcs12/pk12err.c \
    src/ext/openssl/crypto/pkcs7/bio_pk7.c \
    src/ext/openssl/crypto/pkcs7/pk7_asn1.c \
    src/ext/openssl/crypto/pkcs7/pk7_attr.c \
    src/ext/openssl/crypto/pkcs7/pk7_doit.c \
    src/ext/openssl/crypto/pkcs7/pk7_lib.c \
    src/ext/openssl/crypto/pkcs7/pk7_mime.c \
    src/ext/openssl/crypto/pkcs7/pk7_smime.c \
    src/ext/openssl/crypto/pkcs7/pkcs7err.c \
    src/ext/openssl/crypto/poly1305/poly1305_ameth.c \
    src/ext/openssl/crypto/poly1305/poly1305_pmeth.c \
    src/ext/openssl/crypto/poly1305/poly1305.c \
    src/ext/openssl/crypto/rand/drbg_ctr.c \
    src/ext/openssl/crypto/rand/drbg_lib.c \
    src/ext/openssl/crypto/rand/rand_egd.c \
    src/ext/openssl/crypto/rand/rand_err.c \
    src/ext/openssl/crypto/rand/rand_lib.c \
    src/ext/openssl/crypto/rand/rand_unix.c \
    src/ext/openssl/crypto/rand/rand_vms.c \
    src/ext/openssl/crypto/rand/rand_win.c \
    src/ext/openssl/crypto/rand/randfile.c \
    src/ext/openssl/crypto/rc2/rc2_cbc.c \
    src/ext/openssl/crypto/rc2/rc2_ecb.c \
    src/ext/openssl/crypto/rc2/rc2_skey.c \
    src/ext/openssl/crypto/rc2/rc2cfb64.c \
    src/ext/openssl/crypto/rc2/rc2ofb64.c \
    src/ext/openssl/crypto/rc4/rc4_enc.c \
    src/ext/openssl/crypto/rc4/rc4_skey.c \
    src/ext/openssl/crypto/ripemd/rmd_dgst.c \
    src/ext/openssl/crypto/ripemd/rmd_one.c \
    src/ext/openssl/crypto/rsa/rsa_ameth.c \
    src/ext/openssl/crypto/rsa/rsa_asn1.c \
    src/ext/openssl/crypto/rsa/rsa_chk.c \
    src/ext/openssl/crypto/rsa/rsa_crpt.c \
    src/ext/openssl/crypto/rsa/rsa_depr.c \
    src/ext/openssl/crypto/rsa/rsa_err.c \
    src/ext/openssl/crypto/rsa/rsa_gen.c \
    src/ext/openssl/crypto/rsa/rsa_lib.c \
    src/ext/openssl/crypto/rsa/rsa_meth.c \
    src/ext/openssl/crypto/rsa/rsa_mp.c \
    src/ext/openssl/crypto/rsa/rsa_none.c \
    src/ext/openssl/crypto/rsa/rsa_oaep.c \
    src/ext/openssl/crypto/rsa/rsa_ossl.c \
    src/ext/openssl/crypto/rsa/rsa_pk1.c \
    src/ext/openssl/crypto/rsa/rsa_pmeth.c \
    src/ext/openssl/crypto/rsa/rsa_prn.c \
    src/ext/openssl/crypto/rsa/rsa_pss.c \
    src/ext/openssl/crypto/rsa/rsa_saos.c \
    src/ext/openssl/crypto/rsa/rsa_sign.c \
    src/ext/openssl/crypto/rsa/rsa_ssl.c \
    src/ext/openssl/crypto/rsa/rsa_x931.c \
    src/ext/openssl/crypto/rsa/rsa_x931g.c \
    src/ext/openssl/crypto/seed/seed_cbc.c \
    src/ext/openssl/crypto/seed/seed_cfb.c \
    src/ext/openssl/crypto/seed/seed_ecb.c \
    src/ext/openssl/crypto/seed/seed_ofb.c \
    src/ext/openssl/crypto/seed/seed.c \
    src/ext/openssl/crypto/sha/keccak1600.c \
    src/ext/openssl/crypto/sha/sha1_one.c \
    src/ext/openssl/crypto/sha/sha1dgst.c \
    src/ext/openssl/crypto/sha/sha256.c \
    src/ext/openssl/crypto/sha/sha512.c \
    src/ext/openssl/crypto/siphash/siphash_ameth.c \
    src/ext/openssl/crypto/siphash/siphash_pmeth.c \
    src/ext/openssl/crypto/siphash/siphash.c \
    src/ext/openssl/crypto/sm2/sm2_crypt.c \
    src/ext/openssl/crypto/sm2/sm2_err.c \
    src/ext/openssl/crypto/sm2/sm2_pmeth.c \
    src/ext/openssl/crypto/sm2/sm2_sign.c \
    src/ext/openssl/crypto/sm3/m_sm3.c \
    src/ext/openssl/crypto/sm3/sm3.c \
    src/ext/openssl/crypto/sm4/sm4.c \
    src/ext/openssl/crypto/srp/srp_lib.c \
    src/ext/openssl/crypto/srp/srp_vfy.c \
    src/ext/openssl/crypto/stack/stack.c \
    src/ext/openssl/crypto/store/loader_file.c \
    src/ext/openssl/crypto/store/store_err.c \
    src/ext/openssl/crypto/store/store_init.c \
    src/ext/openssl/crypto/store/store_lib.c \
    src/ext/openssl/crypto/store/store_register.c \
    src/ext/openssl/crypto/store/store_strings.c \
    src/ext/openssl/crypto/threads_none.c \
    src/ext/openssl/crypto/threads_pthread.c \
    src/ext/openssl/crypto/threads_win.c \
    src/ext/openssl/crypto/ts/ts_asn1.c \
    src/ext/openssl/crypto/ts/ts_conf.c \
    src/ext/openssl/crypto/ts/ts_err.c \
    src/ext/openssl/crypto/ts/ts_lib.c \
    src/ext/openssl/crypto/ts/ts_req_print.c \
    src/ext/openssl/crypto/ts/ts_req_utils.c \
    src/ext/openssl/crypto/ts/ts_rsp_print.c \
    src/ext/openssl/crypto/ts/ts_rsp_sign.c \
    src/ext/openssl/crypto/ts/ts_rsp_utils.c \
    src/ext/openssl/crypto/ts/ts_rsp_verify.c \
    src/ext/openssl/crypto/ts/ts_verify_ctx.c \
    src/ext/openssl/crypto/txt_db/txt_db.c \
    src/ext/openssl/crypto/ui/ui_err.c \
    src/ext/openssl/crypto/ui/ui_lib.c \
    src/ext/openssl/crypto/ui/ui_null.c \
    src/ext/openssl/crypto/ui/ui_openssl.c \
    src/ext/openssl/crypto/ui/ui_util.c \
    src/ext/openssl/crypto/uid.c \
    src/ext/openssl/crypto/whrlpool/wp_block.c \
    src/ext/openssl/crypto/whrlpool/wp_dgst.c \
    src/ext/openssl/crypto/x509/by_dir.c \
    src/ext/openssl/crypto/x509/by_file.c \
    src/ext/openssl/crypto/x509/t_crl.c \
    src/ext/openssl/crypto/x509/t_req.c \
    src/ext/openssl/crypto/x509/t_x509.c \
    src/ext/openssl/crypto/x509/x_all.c \
    src/ext/openssl/crypto/x509/x_attrib.c \
    src/ext/openssl/crypto/x509/x_crl.c \
    src/ext/openssl/crypto/x509/x_exten.c \
    src/ext/openssl/crypto/x509/x_name.c \
    src/ext/openssl/crypto/x509/x_pubkey.c \
    src/ext/openssl/crypto/x509/x_req.c \
    src/ext/openssl/crypto/x509/x_x509.c \
    src/ext/openssl/crypto/x509/x_x509a.c \
    src/ext/openssl/crypto/x509/x509_att.c \
    src/ext/openssl/crypto/x509/x509_cmp.c \
    src/ext/openssl/crypto/x509/x509_d2.c \
    src/ext/openssl/crypto/x509/x509_def.c \
    src/ext/openssl/crypto/x509/x509_err.c \
    src/ext/openssl/crypto/x509/x509_ext.c \
    src/ext/openssl/crypto/x509/x509_lu.c \
    src/ext/openssl/crypto/x509/x509_obj.c \
    src/ext/openssl/crypto/x509/x509_r2x.c \
    src/ext/openssl/crypto/x509/x509_req.c \
    src/ext/openssl/crypto/x509/x509_set.c \
    src/ext/openssl/crypto/x509/x509_trs.c \
    src/ext/openssl/crypto/x509/x509_txt.c \
    src/ext/openssl/crypto/x509/x509_v3.c \
    src/ext/openssl/crypto/x509/x509_vfy.c \
    src/ext/openssl/crypto/x509/x509_vpm.c \
    src/ext/openssl/crypto/x509/x509cset.c \
    src/ext/openssl/crypto/x509/x509name.c \
    src/ext/openssl/crypto/x509/x509rset.c \
    src/ext/openssl/crypto/x509/x509spki.c \
    src/ext/openssl/crypto/x509/x509type.c \
    src/ext/openssl/crypto/x509v3/pcy_cache.c \
    src/ext/openssl/crypto/x509v3/pcy_data.c \
    src/ext/openssl/crypto/x509v3/pcy_lib.c \
    src/ext/openssl/crypto/x509v3/pcy_map.c \
    src/ext/openssl/crypto/x509v3/pcy_node.c \
    src/ext/openssl/crypto/x509v3/pcy_tree.c \
    src/ext/openssl/crypto/x509v3/v3_addr.c \
    src/ext/openssl/crypto/x509v3/v3_admis.c \
    src/ext/openssl/crypto/x509v3/v3_akey.c \
    src/ext/openssl/crypto/x509v3/v3_akeya.c \
    src/ext/openssl/crypto/x509v3/v3_alt.c \
    src/ext/openssl/crypto/x509v3/v3_asid.c \
    src/ext/openssl/crypto/x509v3/v3_bcons.c \
    src/ext/openssl/crypto/x509v3/v3_bitst.c \
    src/ext/openssl/crypto/x509v3/v3_conf.c \
    src/ext/openssl/crypto/x509v3/v3_cpols.c \
    src/ext/openssl/crypto/x509v3/v3_crld.c \
    src/ext/openssl/crypto/x509v3/v3_enum.c \
    src/ext/openssl/crypto/x509v3/v3_extku.c \
    src/ext/openssl/crypto/x509v3/v3_genn.c \
    src/ext/openssl/crypto/x509v3/v3_ia5.c \
    src/ext/openssl/crypto/x509v3/v3_info.c \
    src/ext/openssl/crypto/x509v3/v3_int.c \
    src/ext/openssl/crypto/x509v3/v3_lib.c \
    src/ext/openssl/crypto/x509v3/v3_ncons.c \
    src/ext/openssl/crypto/x509v3/v3_pci.c \
    src/ext/openssl/crypto/x509v3/v3_pcia.c \
    src/ext/openssl/crypto/x509v3/v3_pcons.c \
    src/ext/openssl/crypto/x509v3/v3_pku.c \
    src/ext/openssl/crypto/x509v3/v3_pmaps.c \
    src/ext/openssl/crypto/x509v3/v3_prn.c \
    src/ext/openssl/crypto/x509v3/v3_purp.c \
    src/ext/openssl/crypto/x509v3/v3_skey.c \
    src/ext/openssl/crypto/x509v3/v3_sxnet.c \
    src/ext/openssl/crypto/x509v3/v3_tlsf.c \
    src/ext/openssl/crypto/x509v3/v3_utl.c \
    src/ext/openssl/crypto/x509v3/v3err.c \
    src/ext/openssl/ssl/bio_ssl.c \
    src/ext/openssl/ssl/d1_lib.c \
    src/ext/openssl/ssl/d1_msg.c \
    src/ext/openssl/ssl/d1_srtp.c \
    src/ext/openssl/ssl/methods.c \
    src/ext/openssl/ssl/packet.c \
    src/ext/openssl/ssl/pqueue.c \
    src/ext/openssl/ssl/record/dtls1_bitmap.c \
    src/ext/openssl/ssl/record/rec_layer_d1.c \
    src/ext/openssl/ssl/record/rec_layer_s3.c \
    src/ext/openssl/ssl/record/ssl3_buffer.c \
    src/ext/openssl/ssl/record/ssl3_record_tls13.c \
    src/ext/openssl/ssl/record/ssl3_record.c \
    src/ext/openssl/ssl/s3_cbc.c \
    src/ext/openssl/ssl/s3_enc.c \
    src/ext/openssl/ssl/s3_lib.c \
    src/ext/openssl/ssl/s3_msg.c \
    src/ext/openssl/ssl/ssl_asn1.c \
    src/ext/openssl/ssl/ssl_cert.c \
    src/ext/openssl/ssl/ssl_ciph.c \
    src/ext/openssl/ssl/ssl_conf.c \
    src/ext/openssl/ssl/ssl_err.c \
    src/ext/openssl/ssl/ssl_init.c \
    src/ext/openssl/ssl/ssl_lib.c \
    src/ext/openssl/ssl/ssl_mcnf.c \
    src/ext/openssl/ssl/ssl_rsa.c \
    src/ext/openssl/ssl/ssl_sess.c \
    src/ext/openssl/ssl/ssl_stat.c \
    src/ext/openssl/ssl/ssl_txt.c \
    src/ext/openssl/ssl/ssl_utst.c \
    src/ext/openssl/ssl/statem/extensions_clnt.c \
    src/ext/openssl/ssl/statem/extensions_cust.c \
    src/ext/openssl/ssl/statem/extensions_srvr.c \
    src/ext/openssl/ssl/statem/extensions.c \
    src/ext/openssl/ssl/statem/statem_clnt.c \
    src/ext/openssl/ssl/statem/statem_dtls.c \
    src/ext/openssl/ssl/statem/statem_lib.c \
    src/ext/openssl/ssl/statem/statem_srvr.c \
    src/ext/openssl/ssl/statem/statem.c \
    src/ext/openssl/ssl/t1_enc.c \
    src/ext/openssl/ssl/t1_lib.c \
    src/ext/openssl/ssl/t1_trce.c \
    src/ext/openssl/ssl/tls_srp.c \
    src/ext/openssl/ssl/tls13_enc.c

$(eval $(call nbCall,nbBuildCodeGrpRules))

#-------------------------
# GROUP NB_OPENSSL (from precompiled lib)
#-------------------------

$(eval $(call nbCall,nbInitCodeGrp))

NB_CODE_GRP_NAME            := openssl-lib

NB_CODE_GRP_FLAGS_REQUIRED  += NB_LIB_SSL NB_LIB_SSL_SYSTEM
 
NB_CODE_GRP_LIBS            += ssl crypto

$(eval $(call nbCall,nbBuildCodeGrpRules))

#-------------------------
# GROUP NB_LEPTONICA (from source)
#-------------------------

$(eval $(call nbCall,nbInitCodeGrp))

NB_CODE_GRP_NAME            := leptonica-src

NB_CODE_GRP_FLAGS_REQUIRED  += NB_LIB_LEPTONICA

ifneq ($(NB_LIB_LEPTONICA_SYSTEM),)
    NB_CODE_GRP_LIBS        += lept
    NB_CODE_GRP_LIBS_PATHS  +=
else
    NB_CODE_GRP_CFLAGS      += \
        -DHAVE_CONFIG_H
    NB_CODE_GRP_CXXFLAGS    += \
        -DHAVE_CONFIG_H \
        -std=c++11
    NB_CODE_GRP_INCLUDES    += \
        include/nb/ext/leptonica \
        include/nb/ext/leptonica/private \
    src/ext/leptonica/src
    NB_CODE_GRP_SRCS        := \
    src/ext/leptonica/src/adaptmap.c \
    src/ext/leptonica/src/affine.c \
    src/ext/leptonica/src/affinecompose.c \
    src/ext/leptonica/src/arrayaccess.c \
    src/ext/leptonica/src/bardecode.c \
    src/ext/leptonica/src/baseline.c \
    src/ext/leptonica/src/bbuffer.c \
    src/ext/leptonica/src/bilateral.c \
    src/ext/leptonica/src/bilinear.c \
    src/ext/leptonica/src/binarize.c \
    src/ext/leptonica/src/binexpand.c \
    src/ext/leptonica/src/binreduce.c \
    src/ext/leptonica/src/blend.c \
    src/ext/leptonica/src/bmf.c \
    src/ext/leptonica/src/bmpio.c \
    src/ext/leptonica/src/bmpiostub.c \
    src/ext/leptonica/src/bootnumgen1.c \
    src/ext/leptonica/src/bootnumgen2.c \
    src/ext/leptonica/src/bootnumgen3.c \
    src/ext/leptonica/src/bootnumgen4.c \
    src/ext/leptonica/src/boxbasic.c \
    src/ext/leptonica/src/boxfunc1.c \
    src/ext/leptonica/src/boxfunc2.c \
    src/ext/leptonica/src/boxfunc3.c \
    src/ext/leptonica/src/boxfunc4.c \
    src/ext/leptonica/src/boxfunc5.c \
    src/ext/leptonica/src/bytearray.c \
    src/ext/leptonica/src/ccbord.c \
    src/ext/leptonica/src/ccthin.c \
    src/ext/leptonica/src/classapp.c \
    src/ext/leptonica/src/colorcontent.c \
    src/ext/leptonica/src/coloring.c \
    src/ext/leptonica/src/colormap.c \
    src/ext/leptonica/src/colormorph.c \
    src/ext/leptonica/src/colorquant1.c \
    src/ext/leptonica/src/colorquant2.c \
    src/ext/leptonica/src/colorseg.c \
    src/ext/leptonica/src/colorspace.c \
    src/ext/leptonica/src/compare.c \
    src/ext/leptonica/src/conncomp.c \
    src/ext/leptonica/src/convertfiles.c \
    src/ext/leptonica/src/convolve.c \
    src/ext/leptonica/src/correlscore.c \
    src/ext/leptonica/src/dewarp1.c \
    src/ext/leptonica/src/dewarp2.c \
    src/ext/leptonica/src/dewarp3.c \
    src/ext/leptonica/src/dewarp4.c \
    src/ext/leptonica/src/dnabasic.c \
    src/ext/leptonica/src/dnafunc1.c \
    src/ext/leptonica/src/dnahash.c \
    src/ext/leptonica/src/dwacomb.2.c \
    src/ext/leptonica/src/dwacomblow.2.c \
    src/ext/leptonica/src/edge.c \
    src/ext/leptonica/src/encoding.c \
    src/ext/leptonica/src/enhance.c \
    src/ext/leptonica/src/fhmtauto.c \
    src/ext/leptonica/src/fhmtgen.1.c \
    src/ext/leptonica/src/fhmtgenlow.1.c \
    src/ext/leptonica/src/finditalic.c \
    src/ext/leptonica/src/flipdetect.c \
    src/ext/leptonica/src/fliphmtgen.c \
    src/ext/leptonica/src/fmorphauto.c \
    src/ext/leptonica/src/fmorphgen.1.c \
    src/ext/leptonica/src/fmorphgenlow.1.c \
    src/ext/leptonica/src/fpix1.c \
    src/ext/leptonica/src/fpix2.c \
    src/ext/leptonica/src/gifio.c \
    src/ext/leptonica/src/gifiostub.c \
    src/ext/leptonica/src/gplot.c \
    src/ext/leptonica/src/graphics.c \
    src/ext/leptonica/src/graymorph.c \
    src/ext/leptonica/src/grayquant.c \
    src/ext/leptonica/src/heap.c \
    src/ext/leptonica/src/jbclass.c \
    src/ext/leptonica/src/jp2kheader.c \
    src/ext/leptonica/src/jp2kheaderstub.c \
    src/ext/leptonica/src/jp2kio.c \
    src/ext/leptonica/src/jp2kiostub.c \
    src/ext/leptonica/src/jpegio.c \
    src/ext/leptonica/src/jpegiostub.c \
    src/ext/leptonica/src/kernel.c \
    src/ext/leptonica/src/leptwin.c \
    src/ext/leptonica/src/libversions.c \
    src/ext/leptonica/src/list.c \
    src/ext/leptonica/src/map.c \
    src/ext/leptonica/src/maze.c \
    src/ext/leptonica/src/morph.c \
    src/ext/leptonica/src/morphapp.c \
    src/ext/leptonica/src/morphdwa.c \
    src/ext/leptonica/src/morphseq.c \
    src/ext/leptonica/src/numabasic.c \
    src/ext/leptonica/src/numafunc1.c \
    src/ext/leptonica/src/numafunc2.c \
    src/ext/leptonica/src/pageseg.c \
    src/ext/leptonica/src/paintcmap.c \
    src/ext/leptonica/src/parseprotos.c \
    src/ext/leptonica/src/partition.c \
    src/ext/leptonica/src/pdfio1.c \
    src/ext/leptonica/src/pdfio1stub.c \
    src/ext/leptonica/src/pdfio2.c \
    src/ext/leptonica/src/pdfio2stub.c \
    src/ext/leptonica/src/pix1.c \
    src/ext/leptonica/src/pix2.c \
    src/ext/leptonica/src/pix3.c \
    src/ext/leptonica/src/pix4.c \
    src/ext/leptonica/src/pix5.c \
    src/ext/leptonica/src/pixabasic.c \
    src/ext/leptonica/src/pixacc.c \
    src/ext/leptonica/src/pixafunc1.c \
    src/ext/leptonica/src/pixafunc2.c \
    src/ext/leptonica/src/pixalloc.c \
    src/ext/leptonica/src/pixarith.c \
    src/ext/leptonica/src/pixcomp.c \
    src/ext/leptonica/src/pixconv.c \
    src/ext/leptonica/src/pixlabel.c \
    src/ext/leptonica/src/pixtiling.c \
    src/ext/leptonica/src/pngio.c \
    src/ext/leptonica/src/pngiostub.c \
    src/ext/leptonica/src/pnmio.c \
    src/ext/leptonica/src/pnmiostub.c \
    src/ext/leptonica/src/projective.c \
    src/ext/leptonica/src/psio1.c \
    src/ext/leptonica/src/psio1stub.c \
    src/ext/leptonica/src/psio2.c \
    src/ext/leptonica/src/psio2stub.c \
    src/ext/leptonica/src/ptabasic.c \
    src/ext/leptonica/src/ptafunc1.c \
    src/ext/leptonica/src/ptafunc2.c \
    src/ext/leptonica/src/ptra.c \
    src/ext/leptonica/src/quadtree.c \
    src/ext/leptonica/src/queue.c \
    src/ext/leptonica/src/rank.c \
    src/ext/leptonica/src/rbtree.c \
    src/ext/leptonica/src/readbarcode.c \
    src/ext/leptonica/src/readfile.c \
    src/ext/leptonica/src/recogbasic.c \
    src/ext/leptonica/src/recogdid.c \
    src/ext/leptonica/src/recogident.c \
    src/ext/leptonica/src/recogtrain.c \
    src/ext/leptonica/src/regutils.c \
    src/ext/leptonica/src/rop.c \
    src/ext/leptonica/src/roplow.c \
    src/ext/leptonica/src/rotate.c \
    src/ext/leptonica/src/rotateam.c \
    src/ext/leptonica/src/rotateorth.c \
    src/ext/leptonica/src/rotateshear.c \
    src/ext/leptonica/src/runlength.c \
    src/ext/leptonica/src/sarray1.c \
    src/ext/leptonica/src/sarray2.c \
    src/ext/leptonica/src/scale1.c \
    src/ext/leptonica/src/scale2.c \
    src/ext/leptonica/src/seedfill.c \
    src/ext/leptonica/src/sel1.c \
    src/ext/leptonica/src/sel2.c \
    src/ext/leptonica/src/selgen.c \
    src/ext/leptonica/src/shear.c \
    src/ext/leptonica/src/skew.c \
    src/ext/leptonica/src/spixio.c \
    src/ext/leptonica/src/stack.c \
    src/ext/leptonica/src/stringcode.c \
    src/ext/leptonica/src/strokes.c \
    src/ext/leptonica/src/sudoku.c \
    src/ext/leptonica/src/textops.c \
    src/ext/leptonica/src/tiffio.c \
    src/ext/leptonica/src/tiffiostub.c \
    src/ext/leptonica/src/utils1.c \
    src/ext/leptonica/src/utils2.c \
    src/ext/leptonica/src/warper.c \
    src/ext/leptonica/src/watershed.c \
    src/ext/leptonica/src/webpio.c \
    src/ext/leptonica/src/webpiostub.c \
    src/ext/leptonica/src/writefile.c \
    src/ext/leptonica/src/zlibmem.c \
    src/ext/leptonica/src/zlibmemstub.c
endif

$(eval $(call nbCall,nbBuildCodeGrpRules))

#-------------------------
# GROUP NB_TESSERACT (from source)
#-------------------------

$(eval $(call nbCall,nbInitCodeGrp))

NB_CODE_GRP_NAME            := tesseract-src

NB_CODE_GRP_FLAGS_REQUIRED  += NB_LIB_TESSERACT

NB_CODE_GRP_FLAGS_FORBIDDEN += NB_LIB_TESSERACT_SYSTEM

NB_CODE_GRP_CFLAGS          += \
    -DHAVE_CONFIG_H \
    -DNDEBUG

NB_CODE_GRP_CXXFLAGS        += \
    -DHAVE_CONFIG_H \
    -DNDEBUG \
    -std=c++20

NB_CODE_GRP_INCLUDES        += \
    include/nb/ext \
    include/nb/ext/tesseract \
    include/nb/ext/leptonica \
    src/ext/leptonica/src \
    src/ext/tesseract/include \
    src/ext/tesseract/src/api \
    src/ext/tesseract/src/arch \
    src/ext/tesseract/src/ccmain \
    src/ext/tesseract/src/ccstruct \
    src/ext/tesseract/src/ccutil \
    src/ext/tesseract/src/classify \
    src/ext/tesseract/src/cutil \
    src/ext/tesseract/src/dict \
    src/ext/tesseract/src/textord \
    src/ext/tesseract/src/wordrec \
    src/ext/tesseract/src/lstm \
    src/ext/tesseract/src/viewer

#only if NB_LIB_LEPTONICA_SYSTEM is not set
ifeq ($(NB_LIB_LEPTONICA_SYSTEM),)
    NB_CODE_GRP_INCLUDES    += \
        include/nb/ext/leptonica \
        src/ext/leptonica/src
endif

NB_CODE_GRP_SRCS            := \
    src/ext/tesseract/src/api/altorenderer.cpp \
    src/ext/tesseract/src/api/baseapi.cpp \
    src/ext/tesseract/src/api/capi.cpp \
    src/ext/tesseract/src/api/hocrrenderer.cpp \
    src/ext/tesseract/src/api/lstmboxrenderer.cpp \
    src/ext/tesseract/src/api/pdfrenderer.cpp \
    src/ext/tesseract/src/api/renderer.cpp \
    src/ext/tesseract/src/api/wordstrboxrenderer.cpp \
    src/ext/tesseract/src/arch/dotproduct.cpp \
    src/ext/tesseract/src/arch/dotproductneon.cpp \
    src/ext/tesseract/src/arch/intsimdmatrix.cpp \
    src/ext/tesseract/src/arch/intsimdmatrixneon.cpp \
    src/ext/tesseract/src/arch/simddetect.cpp \
    src/ext/tesseract/src/ccmain/adaptions.cpp \
    src/ext/tesseract/src/ccmain/applybox.cpp \
    src/ext/tesseract/src/ccmain/control.cpp \
    src/ext/tesseract/src/ccmain/docqual.cpp \
    src/ext/tesseract/src/ccmain/equationdetect.cpp \
    src/ext/tesseract/src/ccmain/fixspace.cpp \
    src/ext/tesseract/src/ccmain/fixxht.cpp \
    src/ext/tesseract/src/ccmain/linerec.cpp \
    src/ext/tesseract/src/ccmain/ltrresultiterator.cpp \
    src/ext/tesseract/src/ccmain/mutableiterator.cpp \
    src/ext/tesseract/src/ccmain/osdetect.cpp \
    src/ext/tesseract/src/ccmain/output.cpp \
    src/ext/tesseract/src/ccmain/pageiterator.cpp \
    src/ext/tesseract/src/ccmain/pagesegmain.cpp \
    src/ext/tesseract/src/ccmain/pagewalk.cpp \
    src/ext/tesseract/src/ccmain/par_control.cpp \
    src/ext/tesseract/src/ccmain/paragraphs.cpp \
    src/ext/tesseract/src/ccmain/paramsd.cpp \
    src/ext/tesseract/src/ccmain/pgedit.cpp \
    src/ext/tesseract/src/ccmain/recogtraining.cpp \
    src/ext/tesseract/src/ccmain/reject.cpp \
    src/ext/tesseract/src/ccmain/resultiterator.cpp \
    src/ext/tesseract/src/ccmain/superscript.cpp \
    src/ext/tesseract/src/ccmain/tessbox.cpp \
    src/ext/tesseract/src/ccmain/tessedit.cpp \
    src/ext/tesseract/src/ccmain/tesseractclass.cpp \
    src/ext/tesseract/src/ccmain/tessvars.cpp \
    src/ext/tesseract/src/ccmain/tfacepp.cpp \
    src/ext/tesseract/src/ccmain/thresholder.cpp \
    src/ext/tesseract/src/ccmain/werdit.cpp \
    src/ext/tesseract/src/ccstruct/blamer.cpp \
    src/ext/tesseract/src/ccstruct/blobbox.cpp \
    src/ext/tesseract/src/ccstruct/blobs.cpp \
    src/ext/tesseract/src/ccstruct/blread.cpp \
    src/ext/tesseract/src/ccstruct/boxread.cpp \
    src/ext/tesseract/src/ccstruct/boxword.cpp \
    src/ext/tesseract/src/ccstruct/ccstruct.cpp \
    src/ext/tesseract/src/ccstruct/coutln.cpp \
    src/ext/tesseract/src/ccstruct/detlinefit.cpp \
    src/ext/tesseract/src/ccstruct/dppoint.cpp \
    src/ext/tesseract/src/ccstruct/fontinfo.cpp \
    src/ext/tesseract/src/ccstruct/image.cpp \
    src/ext/tesseract/src/ccstruct/imagedata.cpp \
    src/ext/tesseract/src/ccstruct/linlsq.cpp \
    src/ext/tesseract/src/ccstruct/matrix.cpp \
    src/ext/tesseract/src/ccstruct/mod128.cpp \
    src/ext/tesseract/src/ccstruct/normalis.cpp \
    src/ext/tesseract/src/ccstruct/ocrblock.cpp \
    src/ext/tesseract/src/ccstruct/ocrpara.cpp \
    src/ext/tesseract/src/ccstruct/ocrrow.cpp \
    src/ext/tesseract/src/ccstruct/otsuthr.cpp \
    src/ext/tesseract/src/ccstruct/pageres.cpp \
    src/ext/tesseract/src/ccstruct/params_training_featdef.cpp \
    src/ext/tesseract/src/ccstruct/pdblock.cpp \
    src/ext/tesseract/src/ccstruct/points.cpp \
    src/ext/tesseract/src/ccstruct/polyaprx.cpp \
    src/ext/tesseract/src/ccstruct/polyblk.cpp \
    src/ext/tesseract/src/ccstruct/quadlsq.cpp \
    src/ext/tesseract/src/ccstruct/quspline.cpp \
    src/ext/tesseract/src/ccstruct/ratngs.cpp \
    src/ext/tesseract/src/ccstruct/rect.cpp \
    src/ext/tesseract/src/ccstruct/rejctmap.cpp \
    src/ext/tesseract/src/ccstruct/seam.cpp \
    src/ext/tesseract/src/ccstruct/split.cpp \
    src/ext/tesseract/src/ccstruct/statistc.cpp \
    src/ext/tesseract/src/ccstruct/stepblob.cpp \
    src/ext/tesseract/src/ccstruct/werd.cpp \
    src/ext/tesseract/src/ccutil/ambigs.cpp \
    src/ext/tesseract/src/ccutil/bitvector.cpp \
    src/ext/tesseract/src/ccutil/ccutil.cpp \
    src/ext/tesseract/src/ccutil/clst.cpp \
    src/ext/tesseract/src/ccutil/elst.cpp \
    src/ext/tesseract/src/ccutil/elst2.cpp \
    src/ext/tesseract/src/ccutil/errcode.cpp \
    src/ext/tesseract/src/ccutil/indexmapbidi.cpp \
    src/ext/tesseract/src/ccutil/params.cpp \
    src/ext/tesseract/src/ccutil/scanutils.cpp \
    src/ext/tesseract/src/ccutil/serialis.cpp \
    src/ext/tesseract/src/ccutil/tessdatamanager.cpp \
    src/ext/tesseract/src/ccutil/tprintf.cpp \
    src/ext/tesseract/src/ccutil/unichar.cpp \
    src/ext/tesseract/src/ccutil/unicharcompress.cpp \
    src/ext/tesseract/src/ccutil/unicharmap.cpp \
    src/ext/tesseract/src/ccutil/unicharset.cpp \
    src/ext/tesseract/src/classify/adaptive.cpp \
    src/ext/tesseract/src/classify/adaptmatch.cpp \
    src/ext/tesseract/src/classify/blobclass.cpp \
    src/ext/tesseract/src/classify/classify.cpp \
    src/ext/tesseract/src/classify/cluster.cpp \
    src/ext/tesseract/src/classify/clusttool.cpp \
    src/ext/tesseract/src/classify/cutoffs.cpp \
    src/ext/tesseract/src/classify/featdefs.cpp \
    src/ext/tesseract/src/classify/float2int.cpp \
    src/ext/tesseract/src/classify/fpoint.cpp \
    src/ext/tesseract/src/classify/intfeaturespace.cpp \
    src/ext/tesseract/src/classify/intfx.cpp \
    src/ext/tesseract/src/classify/intmatcher.cpp \
    src/ext/tesseract/src/classify/intproto.cpp \
    src/ext/tesseract/src/classify/kdtree.cpp \
    src/ext/tesseract/src/classify/mf.cpp \
    src/ext/tesseract/src/classify/mfoutline.cpp \
    src/ext/tesseract/src/classify/mfx.cpp \
    src/ext/tesseract/src/classify/normfeat.cpp \
    src/ext/tesseract/src/classify/normmatch.cpp \
    src/ext/tesseract/src/classify/ocrfeatures.cpp \
    src/ext/tesseract/src/classify/outfeat.cpp \
    src/ext/tesseract/src/classify/picofeat.cpp \
    src/ext/tesseract/src/classify/protos.cpp \
    src/ext/tesseract/src/classify/shapeclassifier.cpp \
    src/ext/tesseract/src/classify/shapetable.cpp \
    src/ext/tesseract/src/classify/tessclassifier.cpp \
    src/ext/tesseract/src/classify/trainingsample.cpp \
    src/ext/tesseract/src/cutil/oldlist.cpp \
    src/ext/tesseract/src/dict/context.cpp \
    src/ext/tesseract/src/dict/dawg_cache.cpp \
    src/ext/tesseract/src/dict/dawg.cpp \
    src/ext/tesseract/src/dict/dict.cpp \
    src/ext/tesseract/src/dict/hyphen.cpp \
    src/ext/tesseract/src/dict/permdawg.cpp \
    src/ext/tesseract/src/dict/stopper.cpp \
    src/ext/tesseract/src/dict/trie.cpp \
    src/ext/tesseract/src/lstm/convolve.cpp \
    src/ext/tesseract/src/lstm/fullyconnected.cpp \
    src/ext/tesseract/src/lstm/functions.cpp \
    src/ext/tesseract/src/lstm/input.cpp \
    src/ext/tesseract/src/lstm/lstm.cpp \
    src/ext/tesseract/src/lstm/lstmrecognizer.cpp \
    src/ext/tesseract/src/lstm/maxpool.cpp \
    src/ext/tesseract/src/lstm/network.cpp \
    src/ext/tesseract/src/lstm/networkio.cpp \
    src/ext/tesseract/src/lstm/parallel.cpp \
    src/ext/tesseract/src/lstm/plumbing.cpp \
    src/ext/tesseract/src/lstm/recodebeam.cpp \
    src/ext/tesseract/src/lstm/reconfig.cpp \
    src/ext/tesseract/src/lstm/reversed.cpp \
    src/ext/tesseract/src/lstm/series.cpp \
    src/ext/tesseract/src/lstm/stridemap.cpp \
    src/ext/tesseract/src/lstm/tfnetwork.cpp \
    src/ext/tesseract/src/lstm/weightmatrix.cpp \
    src/ext/tesseract/src/textord/alignedblob.cpp \
    src/ext/tesseract/src/textord/baselinedetect.cpp \
    src/ext/tesseract/src/textord/bbgrid.cpp \
    src/ext/tesseract/src/textord/blkocc.cpp \
    src/ext/tesseract/src/textord/blobgrid.cpp \
    src/ext/tesseract/src/textord/ccnontextdetect.cpp \
    src/ext/tesseract/src/textord/cjkpitch.cpp \
    src/ext/tesseract/src/textord/colfind.cpp \
    src/ext/tesseract/src/textord/colpartition.cpp \
    src/ext/tesseract/src/textord/colpartitiongrid.cpp \
    src/ext/tesseract/src/textord/colpartitionset.cpp \
    src/ext/tesseract/src/textord/devanagari_processing.cpp \
    src/ext/tesseract/src/textord/drawtord.cpp \
    src/ext/tesseract/src/textord/edgblob.cpp \
    src/ext/tesseract/src/textord/edgloop.cpp \
    src/ext/tesseract/src/textord/equationdetectbase.cpp \
    src/ext/tesseract/src/textord/fpchop.cpp \
    src/ext/tesseract/src/textord/gap_map.cpp \
    src/ext/tesseract/src/textord/imagefind.cpp \
    src/ext/tesseract/src/textord/linefind.cpp \
    src/ext/tesseract/src/textord/makerow.cpp \
    src/ext/tesseract/src/textord/oldbasel.cpp \
    src/ext/tesseract/src/textord/pithsync.cpp \
    src/ext/tesseract/src/textord/pitsync1.cpp \
    src/ext/tesseract/src/textord/scanedg.cpp \
    src/ext/tesseract/src/textord/sortflts.cpp \
    src/ext/tesseract/src/textord/strokewidth.cpp \
    src/ext/tesseract/src/textord/tabfind.cpp \
    src/ext/tesseract/src/textord/tablefind.cpp \
    src/ext/tesseract/src/textord/tablerecog.cpp \
    src/ext/tesseract/src/textord/tabvector.cpp \
    src/ext/tesseract/src/textord/textlineprojection.cpp \
    src/ext/tesseract/src/textord/textord.cpp \
    src/ext/tesseract/src/textord/topitch.cpp \
    src/ext/tesseract/src/textord/tordmain.cpp \
    src/ext/tesseract/src/textord/tospace.cpp \
    src/ext/tesseract/src/textord/tovars.cpp \
    src/ext/tesseract/src/textord/underlin.cpp \
    src/ext/tesseract/src/textord/wordseg.cpp \
    src/ext/tesseract/src/textord/workingpartset.cpp \
    src/ext/tesseract/src/wordrec/associate.cpp \
    src/ext/tesseract/src/wordrec/chop.cpp \
    src/ext/tesseract/src/wordrec/chopper.cpp \
    src/ext/tesseract/src/wordrec/drawfx.cpp \
    src/ext/tesseract/src/wordrec/findseam.cpp \
    src/ext/tesseract/src/wordrec/gradechop.cpp \
    src/ext/tesseract/src/wordrec/language_model.cpp \
    src/ext/tesseract/src/wordrec/lm_consistency.cpp \
    src/ext/tesseract/src/wordrec/lm_pain_points.cpp \
    src/ext/tesseract/src/wordrec/lm_state.cpp \
    src/ext/tesseract/src/wordrec/outlines.cpp \
    src/ext/tesseract/src/wordrec/params_model.cpp \
    src/ext/tesseract/src/wordrec/pieces.cpp \
    src/ext/tesseract/src/wordrec/segsearch.cpp \
    src/ext/tesseract/src/wordrec/tface.cpp \
    src/ext/tesseract/src/wordrec/wordclass.cpp \
    src/ext/tesseract/src/wordrec/wordrec.cpp

$(eval $(call nbCall,nbBuildCodeGrpRules))

#-------------------------
# GROUP NB_TESSERACT (from precompiled lib)
#-------------------------

$(eval $(call nbCall,nbInitCodeGrp))

NB_CODE_GRP_NAME            := tesseract-lib

NB_CODE_GRP_FLAGS_REQUIRED  += NB_LIB_TESSERACT NB_LIB_TESSERACT_SYSTEM

NB_CODE_GRP_LIBS            += tesseract

$(eval $(call nbCall,nbBuildCodeGrpRules))

#-------------------------
# GROUP NB_ZLIB (from source)
#-------------------------

$(eval $(call nbCall,nbInitCodeGrp))

NB_CODE_GRP_NAME            := zlib-src

NB_CODE_GRP_FLAGS_REQUIRED  += NB_LIB_Z

NB_CODE_GRP_FLAGS_FORBIDDEN += NB_LIB_Z_SYSTEM

NB_CODE_GRP_INCLUDES        += \
    include/nb/ext/zlib \
    src/ext/zlib
    
NB_CODE_GRP_SRCS            := \
    src/ext/zlib/adler32.c \
    src/ext/zlib/compress.c \
    src/ext/zlib/crc32.c \
    src/ext/zlib/deflate.c \
    src/ext/zlib/gzclose.c \
    src/ext/zlib/gzlib.c \
    src/ext/zlib/gzread.c \
    src/ext/zlib/gzwrite.c \
    src/ext/zlib/infback.c \
    src/ext/zlib/inffast.c \
    src/ext/zlib/inflate.c \
    src/ext/zlib/inftrees.c \
    src/ext/zlib/trees.c \
    src/ext/zlib/uncompr.c \
    src/ext/zlib/zutil.c

$(eval $(call nbCall,nbBuildCodeGrpRules))

#-------------------------
# GROUP NB_ZLIB (from precompiled lib)
#-------------------------

$(eval $(call nbCall,nbInitCodeGrp))

NB_CODE_GRP_NAME            := zlib-lib

NB_CODE_GRP_FLAGS_REQUIRED  += NB_LIB_Z NB_LIB_Z_SYSTEM

NB_CODE_GRP_LIBS            += z

$(eval $(call nbCall,nbBuildCodeGrpRules))

#-------------------------
# GROUP NB_LIB_LZ4 (from source)
# (fast compression/decompression algorythm)
#-------------------------

$(eval $(call nbCall,nbInitCodeGrp))

NB_CODE_GRP_NAME            := lz4-src

NB_CODE_GRP_FLAGS_REQUIRED  += NB_LIB_LZ4

NB_CODE_GRP_FLAGS_FORBIDDEN += NB_LIB_LZ4_SYSTEM

NB_CODE_GRP_INCLUDES        += \
    src/ext/lz4/lib
        
NB_CODE_GRP_SRCS            := \
    src/ext/lz4/lib/lz4hc.c \
    src/ext/lz4/lib/lz4frame.c \
    src/ext/lz4/lib/xxhash.c \
    src/ext/lz4/lib/lz4.c

$(eval $(call nbCall,nbBuildCodeGrpRules))

#-------------------------
# GROUP NB_LIB_LZ4 (from precompiled lib)
# (fast compression/decompression algorythm)
#-------------------------

$(eval $(call nbCall,nbInitCodeGrp))

NB_CODE_GRP_NAME            := lz4-lib

NB_CODE_GRP_FLAGS_REQUIRED  += NB_LIB_LZ4 NB_LIB_LZ4_SYSTEM

NB_CODE_GRP_LIBS            += lz4

$(eval $(call nbCall,nbBuildCodeGrpRules))

#-------------------------
# GROUP NB_QUIRC (from source)
# (QRCode detection)
#-------------------------

$(eval $(call nbCall,nbInitCodeGrp))

NB_CODE_GRP_NAME           := quirc-src

NB_CODE_GRP_FLAGS_REQUIRED += NB_LIB_QUIRC

NB_CODE_GRP_INCLUDES       += \
   src/ext/quirc

NB_CODE_GRP_SRCS           := \
  src/ext/quirc/quirc.c \
  src/ext/quirc/version_db.c \
  src/ext/quirc/identify.c \
  src/ext/quirc/decode.c

$(eval $(call nbCall,nbBuildCodeGrpRules))

#-------------------------
# TARGET RULES
#-------------------------

$(eval $(call nbCall,nbBuildTargetRules))

#-------------------------
# PROJECT RULES
#-------------------------

$(eval $(call nbCall,nbBuildProjectRules))

