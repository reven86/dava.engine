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

#include "Preset/Preset.h"

#include "FileSystem/FileSystem.h"
#include "FileSystem/KeyedArchive.h"
#include "FileSystem/Logger.h"
#include "Render/TextureDescriptor.h"
#include "Render/Material/NMaterial.h"

#include "QtTools/FileDialog/FileDialog.h"

#include "Project/ProjectManager.h"

namespace Preset
{
static const DAVA::String texturePresetFolder = "Preset/Texture/";
static const DAVA::String materialPresetFolder = "Preset/Material/";
static const QString presetFilter = "Preset (*.preset)";

const DAVA::FilePath CreatePresetFolderPathname(const DAVA::String& folder)
{
    const DAVA::FilePath& projectPath = ProjectManager::Instance()->GetProjectPath();
    DAVA::FilePath folderPath = projectPath + folder;
    folderPath.MakeDirectoryPathname();
    return folderPath;
}

const QString GetSavePathname(const QString& caption, const DAVA::String& folder)
{
    const DAVA::FilePath folderPath = CreatePresetFolderPathname(folder);
    DAVA::FileSystem::Instance()->CreateDirectory(folderPath, true);

    return FileDialog::getSaveFileName(nullptr, caption, folderPath.GetAbsolutePathname().c_str(), presetFilter);
}

const QString GetOpenPathname(const QString& caption, const DAVA::String& folder)
{
    const DAVA::FilePath folderPath = CreatePresetFolderPathname(folder);
    return FileDialog::getOpenFileName(nullptr, caption, folderPath.GetAbsolutePathname().c_str(), presetFilter);
}

bool SaveArchieve(DAVA::KeyedArchive* archieve, const DAVA::FilePath& path)
{ // We can change the way of saving: as text, as binary, etc..
    DVASSERT(archieve != nullptr);
    return archieve->SaveToYamlFile(path);
}

bool LoadArchieve(DAVA::KeyedArchive* archieve, const DAVA::FilePath& path)
{ // We can change the way of loading: as text, as binary, etc..
    DVASSERT(archieve != nullptr);
    return archieve->LoadFromYamlFile(path);
}

bool SavePresetForTexture(const DAVA::TextureDescriptor* descriptor)
{
    DVASSERT(descriptor != nullptr);
    DVASSERT(descriptor->IsCompressedFile() == false);

    const QString outputFile = GetSavePathname("Save Texture Preset", texturePresetFolder);
    if (outputFile.isEmpty())
    {
        return false;
    }

    DAVA::ScopedPtr<DAVA::KeyedArchive> archieve(new DAVA::KeyedArchive());
    archieve->SetString("object", "TextureDescriptor");
    archieve->SetInt32("version", DAVA::TextureDescriptor::CURRENT_VERSION);

    const DAVA::TextureDescriptor::TextureDrawSettings& drawSettings = descriptor->drawSettings;
    archieve->SetInt32("wrapModeS", drawSettings.wrapModeS);
    archieve->SetInt32("wrapModeT", drawSettings.wrapModeT);
    archieve->SetInt32("minFilter", drawSettings.minFilter);
    archieve->SetInt32("magFilter", drawSettings.magFilter);
    archieve->SetInt32("mipFilter", drawSettings.mipFilter);

    const DAVA::TextureDescriptor::TextureDataSettings& dataSettings = descriptor->dataSettings;
    archieve->SetInt32("textureFlags", dataSettings.textureFlags);
    archieve->SetInt32("cubefaceFlags", dataSettings.cubefaceFlags);

    for (DAVA::uint32 gpu = 0; gpu < DAVA::GPU_FAMILY_COUNT; ++gpu)
    {
        DAVA::ScopedPtr<DAVA::KeyedArchive> compression(new DAVA::KeyedArchive());
        compression->SetInt32("format", descriptor->compression[gpu].format);
        compression->SetInt32("width", descriptor->compression[gpu].compressToWidth);
        compression->SetInt32("height", descriptor->compression[gpu].compressToHeight);

        const DAVA::String gpuName = DAVA::GPUFamilyDescriptor::GetGPUName(static_cast<DAVA::eGPUFamily>(gpu));
        archieve->SetArchive(gpuName, compression);
    }

    return SaveArchieve(archieve, outputFile.toStdString());
}

bool LoadPresetForTexture(DAVA::TextureDescriptor* descriptor)
{
    DVASSERT(descriptor != nullptr);
    DVASSERT(descriptor->IsCompressedFile() == false);

    const QString inputFile = GetOpenPathname("Open Texture Preset", texturePresetFolder);
    if (inputFile.isEmpty())
    {
        return false;
    }

    DAVA::ScopedPtr<DAVA::KeyedArchive> archieve(new DAVA::KeyedArchive());
    bool loaded = LoadArchieve(archieve, inputFile.toStdString());
    if (!loaded)
    {
        DAVA::Logger::Error("Cannot load archieve from %s", inputFile.toStdString().c_str());
        return false;
    }

    const DAVA::String object = archieve->GetString("object");
    if (object != "TextureDescriptor")
    {
        DAVA::Logger::Error("Cannot load %s archieve as TextureDescriptor", object.c_str());
        return false;
    }

    const DAVA::int32 version = archieve->GetInt32("version");
    if (version < DAVA::TextureDescriptor::CURRENT_VERSION)
    {
        DAVA::Logger::Warning("Trying to load old version %d", version);
    }
    else if (version > DAVA::TextureDescriptor::CURRENT_VERSION)
    {
        DAVA::Logger::Error("Trying to load newer version %d than have %d", version, DAVA::TextureDescriptor::CURRENT_VERSION);
        return false;
    }

    const DAVA::uint8 cubefaceFlags = archieve->GetInt32("cubefaceFlags");
    if (cubefaceFlags != descriptor->dataSettings.cubefaceFlags)
    {
        DAVA::Logger::Error("Cannot apply preset with different cubefaceFlags");
        return false;
    }

    DAVA::TextureDescriptor::TextureDrawSettings& drawSettings = descriptor->drawSettings;
    drawSettings.wrapModeS = static_cast<DAVA::int8>(archieve->GetInt32("wrapModeS", rhi::TEXADDR_WRAP));
    drawSettings.wrapModeT = static_cast<DAVA::int8>(archieve->GetInt32("wrapModeT", rhi::TEXADDR_WRAP));
    drawSettings.minFilter = static_cast<DAVA::int8>(archieve->GetInt32("minFilter", rhi::TEXFILTER_LINEAR));
    drawSettings.magFilter = static_cast<DAVA::int8>(archieve->GetInt32("magFilter", rhi::TEXFILTER_LINEAR));
    drawSettings.mipFilter = static_cast<DAVA::int8>(archieve->GetInt32("mipFilter", rhi::TEXMIPFILTER_LINEAR));

    DAVA::TextureDescriptor::TextureDataSettings& dataSettings = descriptor->dataSettings;
    dataSettings.textureFlags = static_cast<DAVA::int8>(archieve->GetInt32("textureFlags", DAVA::TextureDescriptor::TextureDataSettings::FLAG_DEFAULT));
    dataSettings.cubefaceFlags = cubefaceFlags;

    const DAVA::FilePath sourceImagePath = descriptor->GetSourceTexturePathname();
    const DAVA::ImageInfo imageInfo = DAVA::ImageSystem::Instance()->GetImageInfo(sourceImagePath);

    for (DAVA::uint32 gpu = 0; gpu < DAVA::GPU_FAMILY_COUNT; ++gpu)
    {
        const DAVA::String gpuName = DAVA::GPUFamilyDescriptor::GetGPUName(static_cast<DAVA::eGPUFamily>(gpu));
        const DAVA::KeyedArchive* compressionArchieve = archieve->GetArchive(gpuName);

        DAVA::TextureDescriptor::Compression& compression = descriptor->compression[gpu];
        if (compressionArchieve != nullptr)
        {
            const DAVA::int32 format = compressionArchieve->GetInt32("format", DAVA::FORMAT_INVALID);
            DAVA::int32 compressToWidth = compressionArchieve->GetInt32("width");
            DAVA::int32 compressToHeight = compressionArchieve->GetInt32("height");

            bool needResetCRC = false;
            if ((format != DAVA::FORMAT_INVALID) && (compression.format != format))
            { // apply format changes
                compression.format = format;
                needResetCRC = true;
            }

            bool canApplySizes = true;
            if (!imageInfo.isEmpty())
            { // correct compression sizes to image size

                if (compressToHeight != 0 && compressToWidth != 0)
                {
                    if (((imageInfo.width == imageInfo.height) && (compressToWidth == compressToHeight)) || ((imageInfo.width != imageInfo.height) && (compressToWidth != compressToHeight)))
                    {
                        compressToWidth = (compressToWidth >= imageInfo.width) ? 0 : compressToWidth;
                        compressToHeight = (compressToHeight >= imageInfo.height) ? 0 : compressToHeight;
                    }
                    else
                    {
                        DAVA::Logger::Warning("Trying to apply preset with wrong sizes for GPU %s", gpuName.c_str());
                        canApplySizes = false;
                    }
                }
            }

            if (canApplySizes && ((compression.compressToWidth != compressToWidth) || (compression.compressToHeight != compressToHeight)))
            { // apply size changes
                compression.compressToWidth = compressToWidth;
                compression.compressToHeight = compressToHeight;
                needResetCRC = true;
            }

            if (needResetCRC)
            {
                compression.convertedFileCrc = 0;
                compression.sourceFileCrc = 0;
            }
        }
    }

    descriptor->Save();
    return true;
}

bool LoadPresetForMaterial(DAVA::NMaterial* material)
{
    return false;
}

bool SavePresetForMaterial(DAVA::NMaterial* material)
{
    return false;
}

} //END of Preset
