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

bool SaveTexturePreset(const DAVA::KeyedArchive* presetArchive, const DAVA::FilePath& path)
{ // We can change the way of saving: as text, as binary, etc..
    DVASSERT(presetArchive != nullptr);
    return presetArchive->SaveToYamlFile(path);
}

bool LoadTexturePreset(DAVA::KeyedArchive* archieve, const DAVA::FilePath& path)
{ // We can change the way of loading: as text, as binary, etc..
    DVASSERT(archieve != nullptr);
    return archieve->LoadFromYamlFile(path);
}

bool DialogSavePresetForTexture(const DAVA::TextureDescriptor* descriptor)
{
    DVASSERT(descriptor != nullptr);

    const QString outputFile = GetSavePathname("Save Texture Preset", texturePresetFolder);
    if (outputFile.isEmpty())
    {
        return false;
    }

    DAVA::ScopedPtr<DAVA::KeyedArchive> presetArchive(new DAVA::KeyedArchive());
    descriptor->ExtractTexturePreset(presetArchive);
    return SaveTexturePreset(presetArchive, outputFile.toStdString());
}

bool DialogLoadPresetForTexture(DAVA::TextureDescriptor* descriptor)
{
    DVASSERT(descriptor != nullptr);

    const QString inputFile = GetOpenPathname("Open Texture Preset", texturePresetFolder);
    if (inputFile.isEmpty())
    {
        return false;
    }

    DAVA::ScopedPtr<DAVA::KeyedArchive> presetArchive(new DAVA::KeyedArchive());
    bool loaded = LoadTexturePreset(presetArchive, inputFile.toStdString());
    if (!loaded)
    {
        return false;
    }

    bool applied = descriptor->ApplyTexturePreset(presetArchive);
    if (!applied)
    {
        return false;
    }

    descriptor->Save();
    return true;
}

bool DialgoLoadPresetForMaterial(DAVA::NMaterial* material)
{
    return false;
}

bool DialogSavePresetForMaterial(DAVA::NMaterial* material)
{
    return false;
}

}
