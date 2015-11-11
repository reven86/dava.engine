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

#include "CommandLine/TextureDescriptor/TextureDescriptorUtils.h"
#include "ImageTools/ImageTools.h"
#include "Settings/SettingsManager.h"
#include "Qt/Preset/Preset.h"


using namespace DAVA;

namespace TextureDescriptorUtils {

bool IsCorrectDirectory(FileList *fileList, const int32 fileIndex)
{
    if (fileList->IsDirectory(fileIndex))
    {
        String name = fileList->GetFilename(fileIndex);
        if (0 != CompareCaseInsensitive(String(".svn"), name) && !fileList->IsNavigationDirectory(fileIndex))
        {
            return true;
        }
    }

    return false;
}

bool IsDescriptorPathname(const FilePath &pathname)
{
    return pathname.IsEqualToExtension(TextureDescriptor::GetDescriptorExtension());
}

void ResaveDescriptorsForFolder(const FilePath &folderPathname)
{
    ScopedPtr<FileList> fileList(new FileList(folderPathname));
    for (int32 fi = 0; fi < fileList->GetCount(); ++fi)
    {
        const FilePath &pathname = fileList->GetPathname(fi);
        if (IsCorrectDirectory(fileList, fi))
        {
            ResaveDescriptorsForFolder(pathname);
        }
        else if (IsDescriptorPathname(pathname))
        {
            ResaveDescriptor(pathname);
        }
    }
}

void ResaveDescriptor(const FilePath & descriptorPathname)
{
    std::unique_ptr<TextureDescriptor> descriptor(TextureDescriptor::CreateFromFile(descriptorPathname));
    descriptor->Save();
}

void CreateDescriptorsForFolder(const FilePath &folder, const FilePath& presetPath)
{
    ScopedPtr<FileList> fileList(new FileList(folder));
    for (int32 fi = 0; fi < fileList->GetCount(); ++fi)
    {
        const FilePath &pathname = fileList->GetPathname(fi);
        if (IsCorrectDirectory(fileList, fi))
        {
            CreateDescriptorsForFolder(pathname, presetPath);
        }
        else if (DAVA::TextureDescriptor::IsDescriptorExtension(pathname.GetExtension()))
        {
            CreateDescriptorIfNeed(pathname, presetPath);
        }
    }
}


bool CreateDescriptorIfNeed(const FilePath &texturePath, const FilePath& presetPath)
{
    FilePath descriptorPathname = TextureDescriptor::GetDescriptorPathname(texturePath);
    if (false == FileSystem::Instance()->IsFile(descriptorPathname))
    {
        std::unique_ptr<TextureDescriptor> descriptor(new TextureDescriptor());

        const String extension = texturePath.GetExtension();
        const ImageFormat sourceFormat = ImageSystem::Instance()->GetImageFormatForExtension(extension);
        if (sourceFormat != IMAGE_FORMAT_UNKNOWN)
        {
            descriptor->dataSettings.sourceFileFormat = sourceFormat;
            descriptor->dataSettings.sourceFileExtension = extension;
        }

        ScopedPtr<KeyedArchive> presetArchive(new KeyedArchive);
        if (!presetPath.IsEmpty() && Preset::LoadTexturePreset(presetArchive, presetPath))
        {
            descriptor->ApplyTexturePreset(presetArchive);
        }

        descriptor->Save(descriptorPathname);
        return true;
    }

    return false;
}

void SetCompressionParamsForFolder(const FilePath &folderPathname, const DAVA::Map<DAVA::eGPUFamily, DAVA::TextureDescriptor::Compression> & compressionParams, bool convertionEnabled, bool force, DAVA::TextureConverter::eConvertQuality quality, bool generateMipMaps)
{
    ScopedPtr<FileList> fileList(new FileList(folderPathname));

    for (int32 fi = 0; fi < fileList->GetCount(); ++fi)
    {
        const FilePath &pathname = fileList->GetPathname(fi);
        if (IsCorrectDirectory(fileList, fi))
        {
            SetCompressionParamsForFolder(pathname, compressionParams, convertionEnabled, force, quality, generateMipMaps);
        }
        else if (IsDescriptorPathname(pathname))
        {
            SetCompressionParams(pathname, compressionParams, convertionEnabled, force, quality, generateMipMaps);
        }
    }
}


void SetCompressionParams(const FilePath &descriptorPathname, const DAVA::Map<DAVA::eGPUFamily, DAVA::TextureDescriptor::Compression> & compressionParams, bool convertionEnabled, bool force, DAVA::TextureConverter::eConvertQuality quality, bool generateMipMaps)
{
    std::unique_ptr<TextureDescriptor> descriptor(TextureDescriptor::CreateFromFile(descriptorPathname));
    if (!descriptor) return;

    DVASSERT(descriptor->compression);

    auto endIt = compressionParams.end();
    for (auto it = compressionParams.begin(); it != endIt; ++it)
    {
        eGPUFamily gpu = it->first;

        if (force || (descriptor->compression[gpu].format == FORMAT_INVALID))
        {
            descriptor->compression[gpu] = it->second;

            if (convertionEnabled)
            {
                ImageTools::ConvertImage(descriptor.get(), gpu, (PixelFormat)descriptor->compression[gpu].format, quality);
            }
        }
    }

    descriptor->Save();
}

void SetPreset(const FilePath& descriptorPath, const KeyedArchive* preset, bool toConvert, TextureConverter::eConvertQuality quality)
{
    std::unique_ptr<TextureDescriptor> descriptor(TextureDescriptor::CreateFromFile(descriptorPath));
    if (!descriptor)
    {
        return;
    }

    descriptor->ApplyTexturePreset(preset);
    descriptor->Save();

    if (toConvert)
    {
        for (uint8 gpu = 0; gpu < GPU_FAMILY_COUNT; ++gpu)
        {
            DAVA::eGPUFamily eGPU = static_cast<eGPUFamily>(gpu);
            ImageTools::ConvertImage(descriptor.get(), eGPU, static_cast<PixelFormat>(descriptor->compression[eGPU].format), quality);
        }
    }
}

void SetPresetForFolder(const FilePath& folder, const KeyedArchive* preset, bool toConvert, TextureConverter::eConvertQuality quality)
{
    ScopedPtr<FileList> fileList(new FileList(folder));
    for (int32 fi = 0; fi < fileList->GetCount(); ++fi)
    {
        const FilePath &pathname = fileList->GetPathname(fi);
        if (IsCorrectDirectory(fileList, fi))
        {
            SetPresetForFolder(pathname, preset, toConvert, quality);
        }
        else if (DAVA::TextureDescriptor::IsDescriptorExtension(pathname.GetExtension()))
        {
            SetPreset(pathname, preset, toConvert, quality);
        }
    }
}

void SetPresetForFolder(const FilePath& folder, const FilePath& presetPath, bool toConvert, TextureConverter::eConvertQuality quality)
{
    ScopedPtr<KeyedArchive> presetArchive(new KeyedArchive);
    if (Preset::LoadTexturePreset(presetArchive, presetPath))
    {
        SetPresetForFolder(folder, presetArchive, toConvert, quality);
    }
}

void SetPreset(const FilePath& descriptorPath, const FilePath& presetPath, bool toConvert, TextureConverter::eConvertQuality quality)
{
    ScopedPtr<KeyedArchive> presetArchive(new KeyedArchive);
    if (Preset::LoadTexturePreset(presetArchive, presetPath))
    {
        SetPreset(descriptorPath, presetArchive, toConvert, quality);
    }
}

}
