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


#pragma once


#include "Base/Platform.h"
#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"

#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)
#include <objc/objc.h>
#endif

#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
#include "Render/Image/Private/PVRDefines.h"
#else //#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
#if defined(__DAVAENGINE_WIN_UAP__)
#include "libpvr/PVRTextureDefines.h"
#else
#include "libpvr/PVRTextureHeader.h"
#endif

#endif //#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)

namespace DAVA
{
#pragma pack(push, 4)
struct PVRHeaderV3
{
    uint32 u32Version = PVRTEX3_IDENT; //Version of the file header, used to identify it.
    uint32 u32Flags = 0; //Various format flags.
    uint64 u64PixelFormat = ePVRTPF_NumCompressedPFs; //The pixel format, 8cc value storing the 4 channel identifiers and their respective sizes.
    uint32 u32ColourSpace = ePVRTCSpacelRGB; //The Colour Space of the texture, currently either linear RGB or sRGB.
    uint32 u32ChannelType = 0; //Variable type that the channel is stored in. Supports signed/unsigned int/short/byte or float for now.
    uint32 u32Height = 1; //Height of the texture.
    uint32 u32Width = 1; //Width of the texture.
    uint32 u32Depth = 1; //Depth of the texture. (Z-slices)
    uint32 u32NumSurfaces = 1; //Number of members in a Texture Array.
    uint32 u32NumFaces = 1; //Number of faces in a Cube Map. Maybe be a value other than 6.
    uint32 u32MIPMapCount = 1; //Number of MIP Maps in the texture - NB: Includes top level.
    uint32 u32MetaDataSize = 0; //Size of the accompanying meta data.
};
#pragma pack(pop)

class PVRFile
{
public:
    ~PVRFile();

    PVRHeaderV3 header;
    Vector<MetaDataBlock*> metaDatablocks;
    Vector<uint8> metaData;

    uint32 compressedDataSize = 0;
    uint8* compressedData = nullptr;
};

class File;
class Image;

namespace PVRFormatHelper
{
std::unique_ptr<PVRFile> ReadFile(const FilePath& pathname, bool readMetaData, bool readData);
std::unique_ptr<PVRFile> ReadFile(File* file, bool readMetaData, bool readData);

std::unique_ptr<PVRFile> GenerateHeader(const Vector<Image*>& imageSet);
std::unique_ptr<PVRFile> GenerateCubeHeader(const Vector<Vector<Image*>>& imageSet);

bool WriteFile(const FilePath& pathname, const PVRFile& pvrFile);

bool GetCRCFromMetaData(const PVRFile& pvrFile, uint32* outputCRC);
void AddCRCToMetaData(PVRFile& pvrFile, uint32 crc);

bool LoadImages(File* infile, Vector<Image*>& imageSet, uint32 fromMipMap, uint32 firstMipmapIndex);

Image* DecodeToRGBA8888(Image* encodedImage);

PixelFormat GetTextureFormat(const PVRHeaderV3& textureHeader);
}
}
