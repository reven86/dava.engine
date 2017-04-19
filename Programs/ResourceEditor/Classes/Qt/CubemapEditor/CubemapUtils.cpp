#include "CubemapEditor/CubemapUtils.h"
#include "Render/Texture.h"
#include "Classes/Settings/SettingsManager.h"
#include "Qt/Main/QtUtils.h"
#include "Classes/Application/REGlobal.h"
#include "Classes/Project/ProjectManagerData.h"

#include "TArc/DataProcessing/DataContext.h"

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
    ProjectManagerData* data = REGlobal::GetDataNode<ProjectManagerData>();
    DVASSERT(data != nullptr);
    DAVA::FilePath projectPath = data->GetProjectPath();
    bool isInProject = DAVA::FilePath::ContainPath(path, projectPath);

    if (!DAVA::FileSystem::Instance()->Exists(path) || !isInProject)
    {
        path = DAVA::FileSystem::Instance()->Exists(defaultPath) ? defaultPath : projectPath;
        SettingsManager::SetValue(key, DAVA::VariantType(path));
    }

    return path;
}
