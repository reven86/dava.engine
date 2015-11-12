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


#include "FileSystem/Logger.h"
#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/DynamicMemoryFile.h"
#include "Utils/Utils.h"
#include "Utils/CRC32.h"
#include "Render/Texture.h"
#include "Render/TextureDescriptor.h"
#include "Render/GPUFamilyDescriptor.h"
#include "Render/Image/ImageSystem.h"
#include "Render/Image/LibPVRHelper.h"
#include "Render/Image/LibDdsHelper.h"


namespace DAVA
{
namespace Validator
{
DAVA_DEPRECATED(void CopyDX11CompressionFormat(TextureDescriptor& descriptor))
{
    if (!descriptor.isCompressedFile)
    {
        if (descriptor.compression[GPU_DX11].format == FORMAT_INVALID && descriptor.compression[GPU_TEGRA].format != FORMAT_INVALID)
        {
            descriptor.compression[GPU_DX11] = descriptor.compression[GPU_TEGRA];
            if (descriptor.compression[GPU_DX11].format == FORMAT_ETC1)
            {
                descriptor.compression[GPU_DX11].format = FORMAT_DXT1;
            }
        }
    }
}

DAVA_DEPRECATED(void FixDX11CompressionFormat(TextureDescriptor& descriptor))
{
    if (!descriptor.isCompressedFile)
    {
        if (descriptor.compression[GPU_DX11].format == FORMAT_RGB888)
        {
            descriptor.compression[GPU_DX11].Clear();
            Logger::Error("[TextureDescriptor] Texture %s has unsupported RGB888 format for GPU_DX11", descriptor.pathname.GetStringValue().c_str());
        }
    }
}

DAVA_DEPRECATED(void ConvertV10orLessToV11(TextureDescriptor& descriptor))
{
    descriptor.drawSettings.wrapModeS = (descriptor.drawSettings.wrapModeS == 0) ? rhi::TEXADDR_CLAMP : rhi::TEXADDR_WRAP;
    descriptor.drawSettings.wrapModeT = (descriptor.drawSettings.wrapModeT == 0) ? rhi::TEXADDR_CLAMP : rhi::TEXADDR_WRAP;

    switch (descriptor.drawSettings.minFilter)
    {
    case 0:
        descriptor.drawSettings.minFilter = rhi::TEXFILTER_NEAREST;
        descriptor.drawSettings.mipFilter = rhi::TEXMIPFILTER_NONE;
        break; //FILTER_NEAREST
    case 1:
        descriptor.drawSettings.minFilter = rhi::TEXFILTER_LINEAR;
        descriptor.drawSettings.mipFilter = rhi::TEXMIPFILTER_NONE;
        break; //FILTER_LINEAR
    case 2:
        descriptor.drawSettings.minFilter = rhi::TEXFILTER_NEAREST;
        descriptor.drawSettings.mipFilter = rhi::TEXMIPFILTER_NEAREST;
        break; //FILTER_NEAREST_MIPMAP_NEAREST
    case 3:
        descriptor.drawSettings.minFilter = rhi::TEXFILTER_LINEAR;
        descriptor.drawSettings.mipFilter = rhi::TEXMIPFILTER_NEAREST;
        break; //FILTER_LINEAR_MIPMAP_NEAREST
    case 4:
        descriptor.drawSettings.minFilter = rhi::TEXFILTER_NEAREST;
        descriptor.drawSettings.mipFilter = rhi::TEXMIPFILTER_LINEAR;
        break; //FILTER_NEAREST_MIPMAP_LINEAR
    case 5:
        descriptor.drawSettings.minFilter = rhi::TEXFILTER_LINEAR;
        descriptor.drawSettings.mipFilter = rhi::TEXMIPFILTER_LINEAR;
        break; //FILTER_LINEAR_MIPMAP_LINEAR
    default:
        break;
    }
}
}

//================   TextureDrawSettings  ===================
void TextureDescriptor::TextureDrawSettings::SetDefaultValues()
{
    wrapModeS = rhi::TEXADDR_WRAP;
    wrapModeT = rhi::TEXADDR_WRAP;

    minFilter = rhi::TEXFILTER_LINEAR;
    magFilter = rhi::TEXFILTER_LINEAR;
    mipFilter = rhi::TEXMIPFILTER_LINEAR;
}
    
//================   TextureDataSettings  ===================
void TextureDescriptor::TextureDataSettings::SetDefaultValues()
{
    textureFlags = FLAG_DEFAULT;
    cubefaceFlags = 0;
    
    static ImageFormat defaultImageFormat = IMAGE_FORMAT_PNG;

    sourceFileFormat = defaultImageFormat;
    sourceFileExtension = ImageSystem::Instance()->GetExtensionsFor(defaultImageFormat)[0];
}

void TextureDescriptor::TextureDataSettings::SetGenerateMipmaps(const bool & generateMipmaps)
{
	EnableFlag(generateMipmaps, FLAG_GENERATE_MIPMAPS);
}

bool TextureDescriptor::TextureDataSettings::GetGenerateMipMaps() const
{
	return IsFlagEnabled(FLAG_GENERATE_MIPMAPS);
}

void TextureDescriptor::TextureDataSettings::SetIsNormalMap(const bool & isNormalMap)
{
    EnableFlag(isNormalMap, FLAG_IS_NORMAL_MAP);
}

bool TextureDescriptor::TextureDataSettings::GetIsNormalMap() const
{
    return IsFlagEnabled(FLAG_IS_NORMAL_MAP);
}

void TextureDescriptor::TextureDataSettings::EnableFlag( bool enable, int8 flag )
{
    if (enable)
    {
        textureFlags |= flag;
    }
    else
    {
        textureFlags &= ~flag;
    }
}

bool TextureDescriptor::TextureDataSettings::IsFlagEnabled( int8 flag ) const
{
	return ((textureFlags & flag) == flag);
}


//================   Compression  ===================
void TextureDescriptor::Compression::Clear()
{
    format = FORMAT_INVALID;
	sourceFileCrc = 0;
    convertedFileCrc = 0;

    compressToWidth = 0;
    compressToHeight = 0;
}
    
//================   TextureDescriptor  ===================
const String TextureDescriptor::DESCRIPTOR_EXTENSION = ".tex";
const String TextureDescriptor::DEFAULT_CUBEFACE_EXTENSION = ".png";

TextureDescriptor::TextureDescriptor()
{
	SetDefaultValues();
}

TextureDescriptor::~TextureDescriptor()
{
}

TextureDescriptor * TextureDescriptor::CreateFromFile(const FilePath &filePathname)
{
    if (filePathname.IsEmpty() || (filePathname.GetType() == FilePath::PATH_IN_MEMORY))
        return nullptr;

    TextureDescriptor* descriptor = new TextureDescriptor();
    if (!descriptor->Initialize(filePathname))
    {
        Logger::Error("[TextureDescriptor::CreateFromFile(]: there are no descriptor file (%s).", filePathname.GetAbsolutePathname().c_str());
        delete descriptor;
        return nullptr;
    }

    return descriptor;
}

TextureDescriptor* TextureDescriptor::CreateDescriptor(rhi::TextureAddrMode wrap, bool generateMipmaps)
{
    TextureDescriptor *descriptor = new TextureDescriptor();
	descriptor->Initialize(wrap, generateMipmaps);

	return descriptor;
}
    
    
void TextureDescriptor::SetDefaultValues()
{
	pathname = FilePath();
	format = FORMAT_INVALID;

	drawSettings.SetDefaultValues();
	dataSettings.SetDefaultValues();
    for (int32 i = 0; i < GPU_FAMILY_COUNT; ++i)
    {
        compression[i].Clear();
    }

    exportedAsGpuFamily = GPU_ORIGIN;
}

void TextureDescriptor::SetQualityGroup(const FastName &group)
{
    qualityGroup = group;
}

FastName TextureDescriptor::GetQualityGroup() const
{
    return qualityGroup;
}
    
bool TextureDescriptor::IsCompressedTextureActual(eGPUFamily forGPU) const
{
    const Compression* compressionForGPU = GetCompressionParams(forGPU);
    const uint32 sourceCRC = ReadSourceCRC();
    const uint32 convertedCRC = GetConvertedCRC(forGPU);

    const bool crcIsEqual = ((compressionForGPU->sourceFileCrc == sourceCRC) && (compressionForGPU->convertedFileCrc == convertedCRC));
    if (crcIsEqual)
    {
        //this code need until using of convertation params in crc
        const ImageFormat imageFormat = GetImageFormatForGPU(forGPU);
        const FilePath filePath = CreateCompressedTexturePathname(forGPU, imageFormat);
        ImageInfo imageInfo = ImageSystem::Instance()->GetImageInfo(filePath);

        const bool imageIsActual = (imageInfo.format == compressionForGPU->format) &&
        ((compressionForGPU->compressToWidth == 0) || (imageInfo.width == compressionForGPU->compressToWidth)) && ((compressionForGPU->compressToHeight == 0) || (imageInfo.height == compressionForGPU->compressToHeight));
        return imageIsActual;
    }

    return crcIsEqual;
}

bool TextureDescriptor::HasCompressionFor(eGPUFamily forGPU) const
{
    const Compression *compression = GetCompressionParams(forGPU);
    return (compression && compression->format != FORMAT_INVALID);
}

bool TextureDescriptor::UpdateCrcForFormat(eGPUFamily forGPU) const
{
    bool wasUpdated = false;
    const Compression *compression = GetCompressionParams(forGPU);

	uint32 sourceCRC = ReadSourceCRC();
    if (compression->sourceFileCrc != sourceCRC)
    {
        compression->sourceFileCrc = sourceCRC;
        wasUpdated = true;
    }

    uint32 convertedCRC = GetConvertedCRC(forGPU);
    if (compression->convertedFileCrc != convertedCRC)
    {
        compression->convertedFileCrc = convertedCRC;
        wasUpdated = true;
    }

    return wasUpdated;
}
    
bool TextureDescriptor::Load(const FilePath &filePathname)
{
    ScopedPtr<File> file(File::Create(filePathname, File::READ | File::OPEN));
    if (!file)
    {
        Logger::Error("[TextureDescriptor::Load] Can't open file: %s", filePathname.GetAbsolutePathname().c_str());
        return false;
    }
    
	pathname = filePathname;

    int32 signature;
	file->Read(&signature);

    int8 version = 0;
	file->Read(&version);

    if (signature == COMPRESSED_FILE)
    {
        isCompressedFile = true;
    }
    else if (signature == NOTCOMPRESSED_FILE)
    {
        isCompressedFile = false;
    }
    else
    {
        Logger::Error("[TextureDescriptor::Load] Signature '%X' is incorrect", signature);
        return false;
    }

    switch (version)
    {
    case 6:
        LoadVersion6(file);
        break;
    case 7:
        LoadVersion7(file);
        break;
    case 8:
        LoadVersion8(file);
        break;
    case 9:
        LoadVersion9(file);
        break;
    case 10:
        LoadVersion10(file);
        break;
    case 11:
        LoadVersion11(file);
        break;
    default:
    {
        Logger::Error("[TextureDescriptor::Load] Version %d is not supported", version);
        return false;
    }
    }

    if (version < 10)
    { //10 is version of adding of DX11 GPU
        Validator::CopyDX11CompressionFormat(*this);
    }
    Validator::FixDX11CompressionFormat(*this);

    return true;
}

void TextureDescriptor::Save() const
{
    DVASSERT_MSG(!pathname.IsEmpty(), "Can use this method only after calling Load()");
    Save(pathname);
}
    
void TextureDescriptor::Save(const FilePath &filePathname) const
{
    File *file = File::Create(filePathname, File::WRITE | File::CREATE);
    if (!file)
    {
        Logger::Error("[TextureDescriptor::Save] Can't open file: %s", filePathname.GetAbsolutePathname().c_str());
        return;
    }
    
    SaveInternal(file, NOTCOMPRESSED_FILE, GPU_FAMILY_COUNT);
    
    SafeRelease(file);
}
  
void TextureDescriptor::Export(const FilePath &filePathname) const
{
    File *file = File::Create(filePathname, File::WRITE | File::OPEN | File::CREATE);
    if (!file)
    {
        Logger::Error("[TextureDescriptor::Export] Can't open file: %s", filePathname.GetAbsolutePathname().c_str());
        return;
    }

    SaveInternal(file, COMPRESSED_FILE, 0);
    
    SafeRelease(file);
}

void TextureDescriptor::SaveInternal(File *file, const int32 signature, const uint8 compressionCount) const
{
    file->Write(&signature);

    int8 version = CURRENT_VERSION;
    file->Write(&version);
    
    //draw settings
    file->Write(&drawSettings.wrapModeS);
    file->Write(&drawSettings.wrapModeT);
    file->Write(&drawSettings.minFilter);
    file->Write(&drawSettings.magFilter);
    file->Write(&drawSettings.mipFilter);

    //data settings
    file->Write(&dataSettings.textureFlags);
    file->Write(&dataSettings.cubefaceFlags);
    file->Write(&dataSettings.sourceFileFormat);
    
    uint32 length = static_cast<uint32>(dataSettings.sourceFileExtension.length());
    file->Write(&length);
    file->Write(dataSettings.sourceFileExtension.c_str(), length);

    for (int i = 0; i < Texture::CUBE_FACE_COUNT; ++i)
    {
        if (dataSettings.cubefaceFlags & (1 << i))
        {
            String faceExt = GetFaceExtension(i);
            length = static_cast<uint32>(faceExt.length());
            file->Write(&length);
            file->Write(faceExt.c_str(), length);
        }
    }

    //compressions
    file->Write(&compressionCount);
    for(int32 i = 0; i < compressionCount; ++i)
    {
        WriteCompression(file, &compression[i]);
    }
    
    //export data
    file->Write(&exportedAsGpuFamily);
    int8 exportedAsPixelFormat = (COMPRESSED_FILE == signature) ? format : FORMAT_INVALID;
    file->Write(&exportedAsPixelFormat);
}

void TextureDescriptor::RecalculateCompressionSourceCRC()
{
    auto sourceCrcOld = ReadSourceCRC_V8_or_less();
    auto sourceCrcNew = ReadSourceCRC();

    for (auto& compr : compression)
    {
        if (compr.sourceFileCrc == sourceCrcOld)
            compr.sourceFileCrc = sourceCrcNew;
    }
}

void TextureDescriptor::LoadVersion6(DAVA::File *file)
{
	file->Read(&drawSettings.wrapModeS);
	file->Read(&drawSettings.wrapModeT);
	file->Read(&dataSettings.textureFlags);
	file->Read(&drawSettings.minFilter);
	file->Read(&drawSettings.magFilter);

    Validator::ConvertV10orLessToV11(*this);

    if (isCompressedFile)
    {
		file->Read(&exportedAsGpuFamily);
        exportedAsGpuFamily = GPUFamilyDescriptor::ConvertValueToGPU(exportedAsGpuFamily);
        
		int8 exportedAsPixelFormat = FORMAT_INVALID;
		file->Read(&exportedAsPixelFormat);
        format = static_cast<PixelFormat>(exportedAsPixelFormat);
    }
    else
    {
        for (auto i = 0; i < 5; ++i)
        {
            int8 format;
			file->Read(&format);
            compression[i].format = static_cast<PixelFormat>(format);

            file->Read(&compression[i].compressToWidth);
            file->Read(&compression[i].compressToHeight);
            file->Read(&compression[i].sourceFileCrc);
        }

        RecalculateCompressionSourceCRC();
    }
}

void TextureDescriptor::LoadVersion7(DAVA::File *file)
{
    file->Read(&drawSettings.wrapModeS);
    file->Read(&drawSettings.wrapModeT);
    file->Read(&dataSettings.textureFlags);
    file->Read(&drawSettings.minFilter);
    file->Read(&drawSettings.magFilter);

    Validator::ConvertV10orLessToV11(*this);

    if (isCompressedFile)
    {
        file->Read(&exportedAsGpuFamily);
        exportedAsGpuFamily = GPUFamilyDescriptor::ConvertValueToGPU(exportedAsGpuFamily);

        int8 exportedAsPixelFormat = FORMAT_INVALID;
        file->Read(&exportedAsPixelFormat);
        format = static_cast<PixelFormat>(exportedAsPixelFormat);
    }
    else
    {
        for (int32 i = 0; i < 5; ++i)
        {
            int8 format;
            file->Read(&format);
            compression[i].format = static_cast<PixelFormat>(format);

            file->Read(&compression[i].compressToWidth);
            file->Read(&compression[i].compressToHeight);
            file->Read(&compression[i].sourceFileCrc);
            file->Read(&compression[i].convertedFileCrc);
        }

        RecalculateCompressionSourceCRC();
    }
    
    file->Read(&dataSettings.cubefaceFlags);
}

void TextureDescriptor::LoadVersion8(File *file)
{
    file->Read(&drawSettings.wrapModeS);
    file->Read(&drawSettings.wrapModeT);
    file->Read(&dataSettings.textureFlags);
    file->Read(&drawSettings.minFilter);
    file->Read(&drawSettings.magFilter);

    Validator::ConvertV10orLessToV11(*this);

    if (isCompressedFile)
    {
        file->Read(&exportedAsGpuFamily);
        exportedAsGpuFamily = GPUFamilyDescriptor::ConvertValueToGPU(exportedAsGpuFamily);

        uint8 exportedAsPixelFormat = FORMAT_INVALID;
        file->Read(&exportedAsPixelFormat);
        format = static_cast<PixelFormat>(exportedAsPixelFormat);
    }
    else
    {
        uint8 compressionsCount = 0;
        file->Read(&compressionsCount);
        for (auto i = 0; i < 6; ++i)
        {
            uint8 format;
            file->Read(&format);
            compression[i].format = (PixelFormat)format;

            file->Read(&compression[i].compressToWidth);
            file->Read(&compression[i].compressToHeight);
            file->Read(&compression[i].sourceFileCrc);
            file->Read(&compression[i].convertedFileCrc);
        }

        RecalculateCompressionSourceCRC();
    }

    file->Read(&dataSettings.cubefaceFlags);
}

void TextureDescriptor::LoadVersion9(File *file)
{
    //draw settings
    file->Read(&drawSettings.wrapModeS);
    file->Read(&drawSettings.wrapModeT);
    file->Read(&drawSettings.minFilter);
    file->Read(&drawSettings.magFilter);

    Validator::ConvertV10orLessToV11(*this);

    //data settings
    file->Read(&dataSettings.textureFlags);
    file->Read(&dataSettings.cubefaceFlags);

    int8 sourceFileFormat = 0;
    file->Read(&sourceFileFormat);
    dataSettings.sourceFileFormat = static_cast<ImageFormat>(sourceFileFormat);
    
    uint32 length = 0;
    Array<char8, 20> extStr;

    file->Read(&length);
    file->Read(extStr.data(), length);
    dataSettings.sourceFileExtension = String(extStr.data(), length);

    for (int i = 0; i < Texture::CUBE_FACE_COUNT; ++i)
    {
        if (dataSettings.cubefaceFlags & (1 << i))
        {
            file->Read(&length);
            file->Read(extStr.data(), length);
            dataSettings.cubefaceExtensions[i] = String(extStr.data(), length);
        }
    }
    

    //compression
    uint8 compressionsCount = 0;
    file->Read(&compressionsCount);
    for (auto i = 0; i < compressionsCount; ++i)
    {
        auto &nextCompression = compression[i];

        uint8 format;
        file->Read(&format);
        nextCompression.format = static_cast<PixelFormat>(format);
        
        file->Read(&nextCompression.compressToWidth);
        file->Read(&nextCompression.compressToHeight);
        file->Read(&nextCompression.sourceFileCrc);
        file->Read(&nextCompression.convertedFileCrc);
    }

    //export data
    file->Read(&exportedAsGpuFamily);
    exportedAsGpuFamily = GPUFamilyDescriptor::ConvertValueToGPU(exportedAsGpuFamily);

    uint8 exportedAsPixelFormat = FORMAT_INVALID;
    file->Read(&exportedAsPixelFormat);
    format = static_cast<PixelFormat>(exportedAsPixelFormat);
}

void TextureDescriptor::LoadVersion10(File* file)
{
    // has no changes in format
    LoadVersion9(file);
}

void TextureDescriptor::LoadVersion11(File* file)
{
    //draw settings
    file->Read(&drawSettings.wrapModeS);
    file->Read(&drawSettings.wrapModeT);
    file->Read(&drawSettings.minFilter);
    file->Read(&drawSettings.magFilter);
    file->Read(&drawSettings.mipFilter);

    //data settings
    file->Read(&dataSettings.textureFlags);
    file->Read(&dataSettings.cubefaceFlags);

    int8 sourceFileFormat = 0;
    file->Read(&sourceFileFormat);
    dataSettings.sourceFileFormat = static_cast<ImageFormat>(sourceFileFormat);

    uint32 length = 0;
    Array<char8, 20> extStr;

    file->Read(&length);
    file->Read(extStr.data(), length);
    dataSettings.sourceFileExtension = String(extStr.data(), length);

    for (int i = 0; i < Texture::CUBE_FACE_COUNT; ++i)
    {
        if (dataSettings.cubefaceFlags & (1 << i))
        {
            file->Read(&length);
            file->Read(extStr.data(), length);
            dataSettings.cubefaceExtensions[i] = String(extStr.data(), length);
        }
    }

    //compression
    uint8 compressionsCount = 0;
    file->Read(&compressionsCount);
    for (auto i = 0; i < compressionsCount; ++i)
    {
        auto& nextCompression = compression[i];

        int8 format;
        file->Read(&format);
        nextCompression.format = static_cast<PixelFormat>(format);

        file->Read(&nextCompression.compressToWidth);
        file->Read(&nextCompression.compressToHeight);
        file->Read(&nextCompression.sourceFileCrc);
        file->Read(&nextCompression.convertedFileCrc);
    }

    //export data
    file->Read(&exportedAsGpuFamily);
    exportedAsGpuFamily = GPUFamilyDescriptor::ConvertValueToGPU(exportedAsGpuFamily);

    int8 exportedAsPixelFormat = FORMAT_INVALID;
    file->Read(&exportedAsPixelFormat);
    format = static_cast<PixelFormat>(exportedAsPixelFormat);
}

void TextureDescriptor::WriteCompression(File *file, const Compression *compression) const
{
    int8 format = compression->format;

	file->Write(&format);
	file->Write(&compression->compressToWidth);
	file->Write(&compression->compressToHeight);
	file->Write(&compression->sourceFileCrc);
	file->Write(&compression->convertedFileCrc);
}

bool TextureDescriptor::GetGenerateMipMaps() const
{
    return dataSettings.GetGenerateMipMaps();
}

const String& TextureDescriptor::GetSourceTextureExtension() const
{
    return dataSettings.sourceFileExtension;
}

FilePath TextureDescriptor::GetSourceTexturePathname() const
{
    if (pathname.IsEmpty())
    {
        return FilePath();
    }

    return FilePath::CreateWithNewExtension(pathname, GetSourceTextureExtension());
}

const String& TextureDescriptor::GetFaceExtension(uint32 face) const
{
    return (
        dataSettings.cubefaceExtensions[face].empty() ? 
        GetDefaultFaceExtension() : dataSettings.cubefaceExtensions[face]);
}

void TextureDescriptor::GetFacePathnames(Vector<FilePath>& faceNames) const
{
    GenerateFacePathnames(pathname, Texture::FACE_NAME_SUFFIX, faceNames);
}

void TextureDescriptor::GenerateFacePathnames(const FilePath & filePath, const Array<String, Texture::CUBE_FACE_COUNT>& faceNameSuffixes, Vector<FilePath>& faceNames) const
{
    faceNames.resize(Texture::CUBE_FACE_COUNT, FilePath());

    String baseName = filePath.GetBasename();
    for (auto face = 0; face < Texture::CUBE_FACE_COUNT; ++face)
    {
        if (dataSettings.cubefaceFlags & (1 << face))
        {
            faceNames[face] = filePath;
            faceNames[face].ReplaceFilename(baseName + faceNameSuffixes[face] + GetFaceExtension(face));
        }
    }
}
    
void TextureDescriptor::GenerateFacePathnames(const FilePath & filePath, Vector<FilePath>& faceNames, const String &extension)
{
    faceNames.resize(Texture::CUBE_FACE_COUNT, FilePath());
    
    const String baseName = filePath.GetBasename();
    for (auto face = 0; face < Texture::CUBE_FACE_COUNT; ++face)
    {
        faceNames[face] = filePath;
        faceNames[face].ReplaceFilename(baseName + Texture::FACE_NAME_SUFFIX[face] + extension);
    }
}
    

FilePath TextureDescriptor::GetDescriptorPathname(const FilePath &texturePathname)
{
    DVASSERT(!texturePathname.IsEmpty());

    if (0 == CompareCaseInsensitive(texturePathname.GetExtension(), GetDescriptorExtension()))
    {
        return texturePathname;
    }
    else
    {
        return FilePath::CreateWithNewExtension(texturePathname, GetDescriptorExtension());
    }
}


const String & TextureDescriptor::GetDescriptorExtension()
{
    return DESCRIPTOR_EXTENSION;
}



const String & TextureDescriptor::GetDefaultFaceExtension()
{
    return DEFAULT_CUBEFACE_EXTENSION;
}

const String & TextureDescriptor::GetLightmapTextureExtension()
{
    return ImageSystem::Instance()->GetExtensionsFor(IMAGE_FORMAT_PNG)[0];
}

const TextureDescriptor::Compression * TextureDescriptor::GetCompressionParams(eGPUFamily gpuFamily) const
{
    DVASSERT(gpuFamily >= 0 && gpuFamily < GPU_FAMILY_COUNT);
    return &compression[gpuFamily];
}

Array<ImageFormat, 5> TextureDescriptor::sourceTextureTypes = { { IMAGE_FORMAT_PNG, IMAGE_FORMAT_TGA, IMAGE_FORMAT_JPEG, IMAGE_FORMAT_DDS, IMAGE_FORMAT_WEBP } };
Array<ImageFormat, 2> TextureDescriptor::compressedTextureTypes = { { IMAGE_FORMAT_PVR, IMAGE_FORMAT_DDS } };

auto IsSupportedFor = [](ImageFormat format, const String& extension)
{
    auto& extensions = ImageSystem::Instance()->GetExtensionsFor(format);
    for (auto& ext : extensions)
    {
        if (CompareCaseInsensitive(ext, extension) == 0)
            return true;
    }
    return false;
};

bool TextureDescriptor::IsSupportedSourceFormat(ImageFormat imageFormat)
{
    auto found = std::find(sourceTextureTypes.begin(), sourceTextureTypes.end(), imageFormat);
    return (found != sourceTextureTypes.end());
}

bool TextureDescriptor::IsSupportedCompressedFormat(ImageFormat imageFormat)
{
    auto found = std::find(compressedTextureTypes.begin(), compressedTextureTypes.end(), imageFormat);
    return (found != compressedTextureTypes.end());
}

bool TextureDescriptor::IsSourceTextureExtension(const String& extension)
{
    for (auto& format : sourceTextureTypes)
    {
        if (IsSupportedFor(format, extension))
            return true;
    }

    return false;
}

bool TextureDescriptor::IsCompressedTextureExtension(const String& extension)
{
    for (auto& format : compressedTextureTypes)
    {
        if (IsSupportedFor(format, extension))
            return true;
    }

    return false;
}

bool TextureDescriptor::IsDescriptorExtension(const String& extension)
{
    return (CompareCaseInsensitive(GetDescriptorExtension(), extension) == 0);
}

bool TextureDescriptor::IsSupportedTextureExtension(const String& extension)
{
    return (IsSourceTextureExtension(extension) ||
            IsCompressedTextureExtension(extension) ||
            IsDescriptorExtension(extension));
}

bool TextureDescriptor::IsCompressedFile() const
{
    return isCompressedFile;
}

bool TextureDescriptor::IsCubeMap() const
{
	return (dataSettings.cubefaceFlags != 0);
}

uint32 TextureDescriptor::ReadSourceCRC_V8_or_less() const
{
    uint32 crc = 0;

    DAVA::File *f = DAVA::File::Create(GetSourceTexturePathname(), DAVA::File::OPEN | DAVA::File::READ);
    if (f != nullptr)
    {
        uint8 buffer[8];

        // Read PNG header
        f->Read(buffer, 8);

        // read chunk header
        while (0 != f->Read(buffer, 8))
        {
            int32 chunk_size = 0;
            chunk_size |= (buffer[0] << 24);
            chunk_size |= (buffer[1] << 16);
            chunk_size |= (buffer[2] << 8);
            chunk_size |= buffer[3];

            // jump thought data
            DVASSERT(chunk_size >= 0);
            f->Seek(chunk_size, File::SEEK_FROM_CURRENT);

            // read crc
            f->Read(buffer, 4);
            crc += ((uint32 *)buffer)[0];
        }

        f->Release();
    }

    return crc;
}

uint32 TextureDescriptor::ReadSourceCRC() const
{
    return CRC32::ForFile(GetSourceTexturePathname());
}
    
uint32 TextureDescriptor::GetConvertedCRC(eGPUFamily forGPU) const  
{
    if (compression[forGPU].format == FORMAT_INVALID)
        return 0;

    ImageFormat imageFormat = GetImageFormatForGPU(forGPU);
    FilePath filePath = CreateCompressedTexturePathname(forGPU, imageFormat);
    if (imageFormat == IMAGE_FORMAT_PVR)
	{
#ifdef __DAVAENGINE_WIN_UAP__
        Logger::Error("[TextureDescriptor::GetConvertedCRC] can't get compressed texture filename for %s; "
                      "LibPVR is unsupported", filePath.GetStringValue().c_str());
        DVASSERT(false);
        return 0;
#else
        LibPVRHelper helper;
        return helper.GetCRCFromFile(filePath) + GenerateDescriptorCRC(forGPU);
#endif
	}
    else if (imageFormat == IMAGE_FORMAT_DDS)
    {
        LibDdsHelper helper;
        return helper.GetCRCFromFile(filePath) + GenerateDescriptorCRC(forGPU);
    }
    else
    {
        Logger::Error("[TextureDescriptor::GetConvertedCRC] can't get compressed texture filename for %s", filePath.GetStringValue().c_str());
        DVASSERT(false);
        return 0;
    }
}

FilePath TextureDescriptor::CreatePathnameForGPU(const eGPUFamily gpuFamily) const
{
    ImageFormat imageFormat = GetImageFormatForGPU(gpuFamily);

    if (TextureDescriptor::IsSupportedCompressedFormat(imageFormat))
    {
        return CreateCompressedTexturePathname(gpuFamily, imageFormat);
    }
    else
    {
        return GetSourceTexturePathname();
    }
}

FilePath TextureDescriptor::CreateCompressedTexturePathname(const eGPUFamily gpuFamily, ImageFormat imageFormat) const
{
    String ext = GPUFamilyDescriptor::GetGPUPrefix(gpuFamily) + ImageSystem::Instance()->GetExtensionsFor(imageFormat)[0];
    return FilePath::CreateWithNewExtension(pathname, ext);
}

ImageFormat TextureDescriptor::GetImageFormatForGPU(const eGPUFamily gpuFamily) const
{
    if (GPU_INVALID == gpuFamily)
        return dataSettings.sourceFileFormat;

    eGPUFamily requestedGPU = gpuFamily;

    PixelFormat requestedFormat = FORMAT_INVALID;
    if (IsCompressedFile())
    {
        requestedGPU = static_cast<eGPUFamily>(exportedAsGpuFamily);
        requestedFormat = format;
    }
    else
    {
        requestedFormat = static_cast<PixelFormat>(compression[gpuFamily].format);
    }

    if (requestedGPU == GPU_ORIGIN)
        return dataSettings.sourceFileFormat;
    else
    {
        ImageFormat imageFormat = GPUFamilyDescriptor::GetCompressedFileFormat(gpuFamily, requestedFormat);
        if (imageFormat == IMAGE_FORMAT_UNKNOWN)
            return dataSettings.sourceFileFormat;
        else
            return imageFormat;
    }
}

    
PixelFormat TextureDescriptor::GetPixelFormatForGPU(eGPUFamily forGPU) const
{
    if (forGPU == GPU_INVALID)
        return FORMAT_INVALID;

    DVASSERT(0 <= forGPU && forGPU < GPU_FAMILY_COUNT);
    return static_cast<PixelFormat>(compression[forGPU].format);
}

void TextureDescriptor::Initialize(rhi::TextureAddrMode wrap, bool generateMipmaps)
{
	SetDefaultValues();

	drawSettings.wrapModeS = wrap;
	drawSettings.wrapModeT = wrap;

	dataSettings.SetGenerateMipmaps(generateMipmaps);
    drawSettings.minFilter = rhi::TEXFILTER_LINEAR;
    drawSettings.magFilter = rhi::TEXFILTER_LINEAR;

    if (generateMipmaps)
    {
        drawSettings.mipFilter = rhi::TEXMIPFILTER_LINEAR;
    }
    else
    {
        drawSettings.mipFilter = rhi::TEXMIPFILTER_NONE;
    }
}

void TextureDescriptor::Initialize( const TextureDescriptor *descriptor )
{
    if (nullptr == descriptor)
    {
        SetDefaultValues();
        return;
    }

    pathname = descriptor->pathname;

    drawSettings = descriptor->drawSettings;
    dataSettings = descriptor->dataSettings;

	isCompressedFile = descriptor->isCompressedFile;
	for(uint32 i = 0; i < GPU_FAMILY_COUNT; ++i)
	{
		compression[i] = descriptor->compression[i];
	}

    qualityGroup = descriptor->qualityGroup;
	exportedAsGpuFamily = descriptor->exportedAsGpuFamily;

	format = descriptor->format;
}

bool TextureDescriptor::Initialize(const FilePath &filePathname)
{
	SetDefaultValues();

	FilePath descriptorPathname = GetDescriptorPathname(filePathname);
	return Load(descriptorPathname);
}

void TextureDescriptor::SetGenerateMipmaps( bool generateMipmaps )
{
	dataSettings.SetGenerateMipmaps(generateMipmaps);
}

bool TextureDescriptor::Reload()
{
    if (FileSystem::Instance()->Exists(pathname))
    {
        FilePath descriptorPathname = pathname;
        SetDefaultValues();
        return Load(descriptorPathname);
    }

    return false;
}

uint32 TextureDescriptor::GenerateDescriptorCRC(eGPUFamily forGPU) const
{
	static const uint32 CRC_BUFFER_SIZE = 16;
	static const uint8 CRC_BUFFER_VALUE = 0x3F; //to create nonzero buffer

    Array<uint8, CRC_BUFFER_SIZE> crcBuffer; //this buffer need to calculate correct CRC of texture descriptor. I plan to fill this buffer with params that are important for compression of textures sush as textureFlags
    Memset(crcBuffer.data(), CRC_BUFFER_VALUE, crcBuffer.size());

    uint8* bufferPtr = crcBuffer.data();
    *bufferPtr++ = dataSettings.textureFlags;

#if 0   
    // We need to enable this code before global re-saving or reloading of texture descriptors. 
    // This code will affect "Not Relevant" image in texture descriptors and we need custom loading of old descripters

    // add convertation params
    *bufferPtr++ = compression[forGPU].format;

    const uint32 sizeOfWidth = sizeof(compression[forGPU].compressToWidth);
    Memcpy(bufferPtr, &compression[forGPU].compressToWidth, sizeOfWidth);
    bufferPtr += sizeOfWidth;

    const uint32 sizeOfHeight = sizeof(compression[forGPU].compressToHeight);
    Memcpy(bufferPtr, &compression[forGPU].compressToHeight, sizeOfHeight);
    bufferPtr += sizeOfHeight;
    //end of convertation params
#endif //

    return CRC32::ForBuffer((const char8*)crcBuffer.data(), CRC_BUFFER_SIZE);
}


};
