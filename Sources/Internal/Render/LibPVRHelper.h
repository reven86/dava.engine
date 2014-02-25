/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#ifndef __DAVAENGINE_LIBPVRHELPER_H__
#define __DAVAENGINE_LIBPVRHELPER_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"

#include "FileSystem/FilePath.h"

#include "Render/RenderBase.h"


#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
    #include "PVRDefines.h"
#else //#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
//    #include "libpvr/PVRTError.h"
//    #include "libpvr/PVRTDecompress.h"
//    #include "libpvr/PVRTMap.h"
    #include "libpvr/PVRTextureHeader.h"
//    #include "libpvr/PVRTexture.h"
#endif //#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)

namespace DAVA
{

    
#pragma pack(push,4)
struct PVRHeaderV3{
	uint32	u32Version;			//Version of the file header, used to identify it.
	uint32	u32Flags;			//Various format flags.
	uint64	u64PixelFormat;		//The pixel format, 8cc value storing the 4 channel identifiers and their respective sizes.
	uint32	u32ColourSpace;		//The Colour Space of the texture, currently either linear RGB or sRGB.
	uint32	u32ChannelType;		//Variable type that the channel is stored in. Supports signed/unsigned int/short/byte or float for now.
	uint32	u32Height;			//Height of the texture.
	uint32	u32Width;			//Width of the texture.
	uint32	u32Depth;			//Depth of the texture. (Z-slices)
	uint32	u32NumSurfaces;		//Number of members in a Texture Array.
	uint32	u32NumFaces;		//Number of faces in a Cube Map. Maybe be a value other than 6.
	uint32	u32MIPMapCount;		//Number of MIP Maps in the texture - NB: Includes top level.
	uint32	u32MetaDataSize;	//Size of the accompanying meta data.
    
	//Constructor for the header - used to make sure that the header is initialised usefully. The initial pixel format is an invalid one and must be set.
	PVRHeaderV3() :
    u32Version(0),u32Flags(0),
    u64PixelFormat(ePVRTPF_NumCompressedPFs),
    u32ColourSpace(0),u32ChannelType(0),
    u32Height(1),u32Width(1),u32Depth(1),
    u32NumSurfaces(1),u32NumFaces(1),
    u32MIPMapCount(1),u32MetaDataSize(0)
	{}
};
#pragma pack(pop)
#define PVRTEX3_HEADERSIZE 52
    
    
// V2 Header Identifiers.
const uint32 PVRTEX2_IDENT			= 0x21525650;	// 'P''V''R'!
const uint32 PVRTEX2_IDENT_REV		= 0x50565221;
    
/*!***************************************************************************
 Describes the Version 2 header of a PVR texture header.
 *****************************************************************************/
struct PVRHeaderV2
{
    uint32 dwHeaderSize;		/*!< size of the structure */
    uint32 dwHeight;			/*!< height of surface to be created */
    uint32 dwWidth;				/*!< width of input surface */
    uint32 dwMipMapCount;		/*!< number of mip-map levels requested */
    uint32 dwpfFlags;			/*!< pixel format flags */
    uint32 dwTextureDataSize;	/*!< Total size in bytes */
    uint32 dwBitCount;			/*!< number of bits per pixel  */
    uint32 dwRBitMask;			/*!< mask for red bit */
    uint32 dwGBitMask;			/*!< mask for green bits */
    uint32 dwBBitMask;			/*!< mask for blue bits */
    uint32 dwAlphaBitMask;		/*!< mask for alpha channel */
    uint32 dwPVR;				/*!< magic number identifying pvr file */
    uint32 dwNumSurfs;			/*!< the number of surfaces present in the pvr */
};

    
    
class Texture;
class Image;
class ImageSet;
class File;
class LibPVRHelper
{
public:

    static bool IsPvrFile(File *file);
    static uint32 GetMipMapLevelsCount(File *file);
	static uint32 GetCubemapFaceCount(File* file);
    
    static bool ReadFile(File *file, const Vector<Image *> &imageSet);
    
    static PixelFormat GetPixelFormat(const FilePath &filePathname);
    static uint32 GetDataSize(const FilePath &filePathname);
	
	static bool AddCRCIntoMetaData(const FilePath &filePathname);
	static uint32 GetCRCFromFile(const FilePath &filePathname);
    
protected:
		
	static uint32 ReadNextMetadata(DAVA::File* file, uint32* crc);
	
	static bool GetCRCFromMetaData(const FilePath &filePathname, uint32* outputCRC);

    static bool PreparePVRData(const char* pvrData, const int32 pvrDataSize);

    static uint32 GetBitsPerPixel(uint64 pixelFormat);
    static void GetFormatMinDims(uint64 pixelFormat, uint32 &minX, uint32 &minY, uint32 &minZ);
    static uint32 GetTextureDataSize(PVRHeaderV3 textureHeader, int32 mipLevel = PVRTEX_ALLMIPLEVELS, bool allSurfaces = true, bool allFaces = true);
    static void MapLegacyTextureEnumToNewFormat(PVRTPixelType OldFormat, uint64& newType, EPVRTColourSpace& newCSpace, EPVRTVariableType& newChanType, bool& isPreMult);
    static void ConvertOldTextureHeaderToV3(const PVRHeaderV2* LegacyHeader, PVRHeaderV3& NewHeader);
//    static bool IsGLExtensionSupported(const char * const extension);

    static const PixelFormat GetCompressedFormat(const uint64 PixelFormat);
    static const PixelFormat GetFloatTypeFormat(const uint64 PixelFormat);
    static const PixelFormat GetUnsignedByteFormat(const uint64 PixelFormat);
    static const PixelFormat GetUnsignedShortFormat(const uint64 PixelFormat);
    
    static const PixelFormat GetTextureFormat(const PVRHeaderV3& textureHeader);
    
    static PVRHeaderV3 GetHeader(const FilePath &filePathname);
    static PVRHeaderV3 GetHeader(File *file);
    static PVRHeaderV3 GetHeader(const uint8* pvrData, const int32 pvrDataSize);
	
	static uint32 GetCubemapLayout(PVRHeaderV3* pvrHeader, const char* pvrData, const int32 pvrDataSize);
	static bool IsCubemap(PVRHeaderV3* pvrHeader, const char* pvrData, const int32 pvrDataSize);
	static const char* GetCubemapMetadata(PVRHeaderV3* pvrHeader, const char* pvrData, const int32 pvrDataSize, uint32* outDataSize);
    
    static bool IsFormatSupported(const PixelFormatDescriptor &format);
    
    //static bool ReadMipMapLevel(const char* pvrData, const int32 pvrDataSize, Image *image, uint32 mipMapLevel);
    
	//load cubemap
	static bool ReadMipMapLevel(const char* pvrData, const int32 pvrDataSize, const Vector<Image*>& images, uint32 mipMapLevel);
    
    static bool CopyToImage(Image *image, uint32 mipMapLevel, uint32 faceIndex, const PVRHeaderV3 &header, const uint8 *pvrData);
    
    static PVRHeaderV3 CreateDecompressedHeader(const PVRHeaderV3 &compressedHeader);
    static bool AllocateImageData(Image *image, uint32 mipMapLevel, const PVRHeaderV3 &header);
    
    static int32 GetMipMapLayerOffset(uint32 mipMapLevel, uint32 faceIndex, const PVRHeaderV3 &header);
    
};
    
};


#endif //#ifndef __DAVAENGINE_LIBPVRHELPER_H__





