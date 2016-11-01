#include "Project/ProjectManager.h"
#include "Main/QtUtils.h"
#include "Settings/SettingsManager.h"
#include "Deprecated/SceneValidator.h"
#include "Deprecated/EditorConfig.h"

#include "Utils/TextureDescriptor/TextureDescriptorUtils.h"

#include "FileSystem/FileSystem.h"
#include "FileSystem/YamlParser.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"

#include "QtTools/FileDialogs/FileDialog.h"
#include "QtTools/ProjectInformation/ProjectStructure.h"


#include "SpritesPacker/SpritesPackerModule.h"

void ProjectManager::SetSpritesPacker(SpritesPackerModule* spritesPacker_)
{
    if (spritesPacker != nullptr)
    {
        disconnect(spritesPacker, &SpritesPackerModule::SpritesReloaded, this, &ProjectManager::OnSpritesReloaded);
    }

    spritesPacker = spritesPacker_;

    if (spritesPacker != nullptr)
    {
        connect(spritesPacker, &SpritesPackerModule::SpritesReloaded, this, &ProjectManager::OnSpritesReloaded);
    }
}

void ProjectManager::CloseProject()
{
}

void ProjectManager::OnSpritesReloaded()
{
    emit ProjectOpened(projectPath.GetAbsolutePathname().c_str());
}
