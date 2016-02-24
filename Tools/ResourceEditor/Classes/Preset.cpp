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

#include "Preset.h"

#include "FileSystem/FileSystem.h"
#include "FileSystem/KeyedArchive.h"
#include "FileSystem/Logger.h"
#include "Render/TextureDescriptor.h"
#include "Render/Material/NMaterial.h"
#include "Utils/StringFormat.h"

#include "QtTools/FileDialog/FileDialog.h"

#include "Project/ProjectManager.h"

#include <QMessageBox>

namespace Preset
{
using namespace DAVA;

static const String texturePresetFolder = "Preset/Texture/";
static const String materialPresetFolder = "Preset/Material/";
static const QString presetFilter = "Preset (*.preset)";

namespace Internal
{
bool ArePresetDimensionsCorrect(const TextureDescriptor* descriptor, const KeyedArchive* preset, List<String>& warnings)
{
    DVASSERT(descriptor);
    DVASSERT(preset);

    const FilePath sourceImagePath = descriptor->GetSourceTexturePathname();
    const ImageInfo imageInfo = ImageSystem::Instance()->GetImageInfo(sourceImagePath);
    if (imageInfo.isEmpty())
    {
        String warn = DAVA::Format("Can't get image info for %s", sourceImagePath.GetAbsolutePathname().c_str());
        Logger::Warning("%s", warn.c_str());
        warnings.emplace_back(warn);
        return false;
    }

    bool dimensionsAreCorrect = true;
    bool imageIsSquare = (imageInfo.width == imageInfo.height);

    for (uint8 gpu = 0; gpu < GPU_FAMILY_COUNT; ++gpu)
    {
        String gpuName = GPUFamilyDescriptor::GetGPUName(static_cast<eGPUFamily>(gpu));
        const KeyedArchive* compressionArchive = preset->GetArchive(gpuName);
        if (compressionArchive != nullptr)
        {
            auto compressToWidth = static_cast<uint32>(compressionArchive->GetInt32("width"));
            auto compressToHeight = static_cast<uint32>(compressionArchive->GetInt32("height"));
            bool compressIsSquare = (compressToHeight == compressToWidth);

            if (compressToHeight != 0 && compressToWidth != 0)
            {
                if (imageIsSquare != compressIsSquare || (compressToWidth >= imageInfo.width) || (compressToHeight >= imageInfo.height))
                {
                    String warn = DAVA::Format("Preset compression size %u x %u for gpu %s doesn't fit for image size %u x %u",
                                               compressToWidth, compressToHeight, gpuName.c_str(), imageInfo.width, imageInfo.height);
                    Logger::Warning("%s", warn.c_str());
                    warnings.emplace_back(warn);
                    dimensionsAreCorrect = false;
                }
            }
        }
    }

    return dimensionsAreCorrect;
}

FilePath CreatePresetFolderPathname(const String& folder)
{
    const FilePath& projectPath = ProjectManager::Instance()->GetProjectPath();
    FilePath folderPath = projectPath + folder;
    folderPath.MakeDirectoryPathname();
    return folderPath;
}

QString GetSavePathname(const QString& caption, const String& folder)
{
    const FilePath folderPath = CreatePresetFolderPathname(folder);
    FileSystem::Instance()->CreateDirectory(folderPath, true);

    return FileDialog::getSaveFileName(nullptr, caption, folderPath.GetAbsolutePathname().c_str(), presetFilter);
}

QString GetOpenPathname(const QString& caption, const String& folder)
{
    const FilePath folderPath = CreatePresetFolderPathname(folder);
    return FileDialog::getOpenFileName(nullptr, caption, folderPath.GetAbsolutePathname().c_str(), presetFilter);
}

} // namespace Internal

bool SaveArchive(const KeyedArchive* presetArchive, const FilePath& path)
{ // We can change the way of saving: as text, as binary, etc..
    DVASSERT(presetArchive != nullptr);
    return presetArchive->SaveToYamlFile(path);
}

KeyedArchive* LoadArchive(const FilePath& path)
{ // We can change the way of loading: as text, as binary, etc..
    KeyedArchive* archive = new KeyedArchive;
    if (path.IsEmpty() || !archive->LoadFromYamlFile(path))
    {
        SafeRelease(archive);
    }

    return archive;
}

bool ApplyTexturePreset(TextureDescriptor* descriptor, const KeyedArchive* preset, bool applyWithCorrectDimensionsOnly)
{
    DVASSERT(descriptor != nullptr);
    DVASSERT(preset != nullptr);

    if (descriptor->IsPresetValid(preset) == false)
        return false;

    List<String> warnings;
    if (Internal::ArePresetDimensionsCorrect(descriptor, preset, warnings) == false && applyWithCorrectDimensionsOnly)
        return false;

    bool applied = descriptor->DeserializeFromPreset(preset);
    DVASSERT(applied);

    return true;
}

bool DialogSavePresetForTexture(const TextureDescriptor* descriptor)
{
    DVASSERT(descriptor != nullptr);

    const QString outputFile = Internal::GetSavePathname("Save Texture Preset", texturePresetFolder);
    if (outputFile.isEmpty())
    {
        return false;
    }

    ScopedPtr<KeyedArchive> presetArchive(new KeyedArchive());
    descriptor->SerializeToPreset(presetArchive);
    return SaveArchive(presetArchive, outputFile.toStdString());
}

bool DialogLoadPresetForTexture(TextureDescriptor* descriptor)
{
    DVASSERT(descriptor != nullptr);

    const QString inputFile = Internal::GetOpenPathname("Open Texture Preset", texturePresetFolder);
    if (inputFile.isEmpty())
    {
        return false;
    }

    ScopedPtr<KeyedArchive> presetArchive(LoadArchive(inputFile.toStdString()));
    if (!presetArchive || !descriptor->IsPresetValid(presetArchive))
    {
        return false;
    }

    bool toApply = true;
    List<String> warnings;
    if (Internal::ArePresetDimensionsCorrect(descriptor, presetArchive, warnings) == false)
    {
        QMessageBox msgBox;
        QString msg;
        for (String& warn : warnings)
        {
            msg.append(warn.c_str());
            msg.append("\n");
        }
        msg.append("\n");
        msg.append("Do you want to apply this preset?");
        msgBox.setText(msg);
        msgBox.setStandardButtons(QMessageBox::Apply | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        msgBox.setIcon(QMessageBox::Warning);
        int ret = msgBox.exec();
        toApply = (ret == QMessageBox::Apply);
    }

    if (toApply)
    {
        bool applied = descriptor->DeserializeFromPreset(presetArchive);
        DVASSERT(applied);
        descriptor->Save();
        return true;
    }
    else
    {
        return false;
    }
}

bool DialogLoadPresetForMaterial(NMaterial* material)
{
    return false;
}

bool DialogSavePresetForMaterial(NMaterial* material)
{
    return false;
}
}
