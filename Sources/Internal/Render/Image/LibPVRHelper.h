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

#include "Base/Platform.h"

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"

#include "FileSystem/FilePath.h"

#include "Render/RenderBase.h"
#include "Render/PixelFormatDescriptor.h"
#include "Render/Image/ImageFormatInterface.h"
#include "Render/Image/CRCAdditionInterface.h"

#if defined (__DAVAENGINE_MACOS__) || defined (__DAVAENGINE_IPHONE__)
#include <objc/objc.h>
#endif

#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
#include "Render/PVRDefines.h"
#else //#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
//    #include "libpvr/PVRTError.h"
//    #include "libpvr/PVRTDecompress.h"
//    #include "libpvr/PVRTMap.h"
#if defined (__DAVAENGINE_WIN_UAP__)
#include "libpvr/PVRTextureDefines.h"
#else
#include "libpvr/PVRTextureHeader.h"
#endif

//    #include "libpvr/PVRTexture.h"
#endif //#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)

namespace DAVA
{
#pragma pack(push,4)
    struct PVRHeaderV3
    {
        uint32	u32Version;      //Version of the file header, used to identify it.
        uint32	u32Flags;        //Various format flags.
        uint64	u64PixelFormat;  //The pixel format, 8cc value storing the 4 channel identifiers and their respective sizes.
        uint32	u32ColourSpace;  //The Colour Space of the texture, currently either linear RGB or sRGB.
        uint32	u32ChannelType;  //Variable type that the channel is stored in. Supports signed/unsigned int/short/byte or float for now.
        uint32	u32Height;       //Height of the texture.
        uint32	u32Width;        //Width of the texture.
        uint32	u32Depth;        //Depth of the texture. (Z-slices)
        uint32	u32NumSurfaces;  //Number of members in a Texture Array.
        uint32	u32NumFaces;     //Number of faces in a Cube Map. Maybe be a value other than 6.
        uint32	u32MIPMapCount;  //Number of MIP Maps in the texture - NB: Includes top level.
        uint32	u32MetaDataSize; //Size of the accompanying meta data.
        
        //Constructor for the header - used to make sure that the header is initialised usefully. The initial pixel format is an invalid one and must be set.
        PVRHeaderV3()
            : u32Version(0)
            , u32Flags(0)
            , u64PixelFormat(ePVRTPF_NumCompressedPFs)
            , u32ColourSpace(0)
            ,u32ChannelType(0)
            , u32Height(1)
            ,u32Width(1)
            ,u32Depth(1)
            , u32NumSurfaces(1)
            ,u32NumFaces(1)
            , u32MIPMapCount(1)
            ,u32MetaDataSize(0)
        {
        }
    };
#pragma pack(pop)
#define PVRTEX3_HEADERSIZE 52


// V2 Header Identifiers.
const uint32 PVRTEX2_IDENT = 0x21525650; // 'P''V''R'!
const uint32 PVRTEX2_IDENT_REV = 0x50565221;

/*!***************************************************************************
 Describes the Version 2 header of a PVR texture header.
 *****************************************************************************/
struct PVRHeaderV2
{
    uint32 dwHeaderSize;      // size of the structure
    uint32 dwHeight;          // height of surface to be created
    uint32 dwWidth;           // width of input surface
    uint32 dwMipMapCount;     // number of mip-map levels requested
    uint32 dwpfFlags;         // pixel format flags
    uint32 dwTextureDataSize; // Total size in bytes
    uint32 dwBitCount;        // number of bits per pixel
    uint32 dwRBitMask;        // mask for red bit
    uint32 dwGBitMask;        // mask for green bits
    uint32 dwBBitMask;        // mask for blue bits
    uint32 dwAlphaBitMask;    // mask for alpha channel
    uint32 dwPVR;             // magic number identifying pvr file
    uint32 dwNumSurfs;        // the number of surfaces present in the pvr
};


class PVRFile
{
public:
    PVRFile();
    ~PVRFile();

    PVRHeaderV3 header;
    Vector<MetaDataBlock *> metaDatablocks;
    uint8 *metaData;

    uint32 compressedDataSize;
    uint8 *compressedData;
};

class Image;
class File;

class LibPVRHelper: public ImageFormatInterface, public CRCAdditionInterface
{
public:
    LibPVRHelper();

    ImageFormat GetImageFormat() const override;

    bool IsMyImage(File *file) const override;

    eErrorCode ReadFile(File *infile, Vector<Image *> &imageSet, int32 fromMipmap = 0) const override;

    eErrorCode WriteFile(const FilePath & fileName, const Vector<Image *> &imageSet, PixelFormat compressionFormat, ImageQuality quality) const override;

    eErrorCode WriteFileAsCubeMap(const FilePath & fileName, const Vector<Vector<Image *> > &imageSet, PixelFormat compressionFormat, ImageQuality quality) const override;

    ImageInfo GetImageInfo(File *infile) const override;

    bool AddCRCIntoMetaData(const FilePath &filePathname) const override;
    uint32 GetCRCFromFile(const FilePath &filePathname) const override;

    static bool WriteFileFromMipMapFiles(const FilePath & outputFile, const Vector<FilePath> & imgPaths);

protected:
    static PVRFile* ReadFile(const FilePath &filePathname, bool readMetaData = false, bool readData = false);
    static PVRFile* ReadFile(File *file, bool readMetaData = false, bool readData = false);
    static bool LoadImages(const PVRFile *pvrFile, Vector<Image *> &imageSet, int32 fromMipMap);

    static bool WriteFile(const PVRFile *pvrFile, File *outFile);

    static bool DetectIfNeedSwapBytes(const PVRHeaderV3 *header);
    static void PrepareHeader(PVRHeaderV3 *header, const bool swapBytes);
    static void SwapDataBytes(const PVRHeaderV3 &header, uint8 *data, const uint32 dataSize);

    static void ReadMetaData(File *file, PVRFile *pvrFile, const bool swapBytes);

    static bool LoadMipMapLevel(const PVRFile *pvrFile, const uint32 fileMipMapLevel, const uint32 imageMipMapLevel, Vector<Image *> &imageSet);
    static uint32 GetCubemapLayout(const PVRFile *pvrFile);
    static const MetaDataBlock * GetCubemapMetadata(const PVRFile *pvrFile);

    static bool GetCRCFromMetaData(const FilePath &filePathname, uint32* outputCRC);
    static bool GetCRCFromMetaData(const PVRFile *pvrFile, uint32* outputCRC);

    static uint32 GetBitsPerPixel(uint64 pixelFormat);
    static void GetFormatMinDims(uint64 pixelFormat, uint32 &minX, uint32 &minY, uint32 &minZ);
    static uint32 GetTextureDataSize(PVRHeaderV3 textureHeader, int32 mipLevel = PVRTEX_ALLMIPLEVELS, bool allSurfaces = true, bool allFaces = true);
    static int32 GetMipMapLayerOffset(uint32 mipMapLevel, uint32 faceIndex, const PVRHeaderV3 &header);
    static void MapLegacyTextureEnumToNewFormat(PVRTPixelType OldFormat, uint64& newType, EPVRTColourSpace& newCSpace, EPVRTVariableType& newChanType, bool& isPreMult);

    static const PixelFormat GetTextureFormat(const PVRHeaderV3& textureHeader);
    static const PixelFormat GetCompressedFormat(const uint64 PixelFormat);
    static const PixelFormat GetFloatTypeFormat(const uint64 PixelFormat);
    static const PixelFormat GetUnsignedByteFormat(const uint64 PixelFormat);
    static const PixelFormat GetUnsignedShortFormat(const uint64 PixelFormat);

    static bool IsFormatSupported(const PixelFormatDescriptor &format);

    static PVRHeaderV3 CreateDecompressedHeader(const PVRHeaderV3 &compressedHeader);

    static bool CopyToImage(Image *image, uint32 mipMapLevel, uint32 faceIndex, const PVRHeaderV3 &header, const uint8 *pvrData);
    static bool AllocateImageData(Image *image, uint32 mipMapLevel, const PVRHeaderV3 &header);
};

inline ImageFormat LibPVRHelper::GetImageFormat() const
{
    return IMAGE_FORMAT_PVR;
}

inline LibPVRHelper* CreateLibPVRHelper()
{
    return new LibPVRHelper;
}

};

#endif //#ifndef __DAVAENGINE_LIBPVRHELPER_H__