#include "CUbemapEditor/CubemapUtils.h"
#include "Render/Texture.h"
#include "Qt/Settings/SettingsManager.h"
#include "Qt/Main/QtUtils.h"
#include "Project/ProjectManager.h"
#include "Render/Texture.h"

void CubemapUtils::GenerateFaceNames(const DAVA::String& baseName, DAVA::Vector<DAVA::FilePath>& faceNames)
{
    faceNames.clear();

    DAVA::FilePath filePath(baseName);

    std::unique_ptr<DAVA::TextureDescriptor> descriptor(new DAVA::TextureDescriptor());
    if (!descriptor->Load(filePath))
        return;

    descriptor->GetFacePathnames(faceNames);
}

DAVA::FilePath CubemapUtils::GetDialogSavedPath(const DAVA::String& key, const DAVA::String& defaultValue)
{
    DAVA::VariantType settinsValue = SettingsManager::GetValue(key);
    DAVA::FilePath path = settinsValue.GetType() == DAVA::VariantType::TYPE_STRING ? settinsValue.AsString() : settinsValue.AsFilePath();

    DAVA::FilePath defaultPath(defaultValue);
    DAVA::FilePath projectPath = ProjectManager::Instance()->GetProjectPath();
    bool isInProject = DAVA::FilePath::ContainPath(path, projectPath);

    if (!DAVA::FileSystem::Instance()->Exists(path) || !isInProject)
    {
        path = DAVA::FileSystem::Instance()->Exists(defaultPath) ? defaultPath : projectPath;
        SettingsManager::SetValue(key, DAVA::VariantType(path));
    }

    return path;
}
