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


#include "Project/ProjectManager.h"
#include "Main/QtUtils.h"
#include "Settings/SettingsManager.h"
#include "Deprecated/SceneValidator.h"
#include "Deprecated/EditorConfig.h"

#include "CommandLine/TextureDescriptor/TextureDescriptorUtils.h"

#include "FileSystem/FileSystem.h"
#include "FileSystem/YamlParser.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"

#include "QtTools/FileDialog/FileDialog.h"

#include "SpritesPacker/SpritesPackerModule.h"

bool ProjectManager::IsOpened() const
{
    return (!projectPath.IsEmpty());
}

const DAVA::FilePath& ProjectManager::GetProjectPath() const
{
    return projectPath;
}

const DAVA::FilePath& ProjectManager::GetDataSourcePath() const
{
    return dataSourcePath;
}

const DAVA::FilePath& ProjectManager::GetParticlesConfigPath() const
{
    return particlesConfigPath;
}

const DAVA::FilePath& ProjectManager::GetParticlesDataPath() const
{
    return particlesDataPath;
}

const DAVA::FilePath& ProjectManager::GetWorkspacePath() const
{
    return workspacePath;
}

const QVector<ProjectManager::AvailableMaterialTemplate>* ProjectManager::GetAvailableMaterialTemplates() const
{
    return &templates;
}

const QVector<ProjectManager::AvailableMaterialQuality>* ProjectManager::GetAvailableMaterialQualities() const
{
    return &qualities;
}

DAVA::FilePath ProjectManager::ProjectOpenDialog() const
{
    QString newPathStr = FileDialog::getExistingDirectory(NULL, QString("Open Project Folder"), QString("/"));
    if (!newPathStr.isEmpty())
    {
        DAVA::FilePath path = DAVA::FilePath(PathnameToDAVAStyle(newPathStr));
        path.MakeDirectoryPathname();
        return path;
    }

    return DAVA::FilePath();
}

void ProjectManager::OpenProject(const QString& path)
{
    OpenProject(PathnameToDAVAStyle(path));
}

void ProjectManager::OpenProject(const DAVA::FilePath& incomePath)
{
    if (incomePath.IsDirectoryPathname() && incomePath != projectPath)
    {
        CloseProject();

        projectPath = incomePath;

        if (DAVA::FileSystem::Instance()->Exists(incomePath))
        {
            DAVA::FilePath::AddTopResourcesFolder(projectPath);

            SettingsManager::SetValue(Settings::Internal_LastProjectPath, DAVA::VariantType(projectPath));

            UpdateInternalValues();
            LoadProjectSettings();
            LoadMaterialsSettings();

            DAVA::FileSystem::Instance()->CreateDirectory(workspacePath, true);

            SceneValidator::Instance()->SetPathForChecking(projectPath);

            DAVA::QualitySettingsSystem::Instance()->Load("~res:/quality.yaml");
            DAVA::SoundSystem::Instance()->InitFromQualitySettings();

            if (spritesPacker != nullptr)
            {
                //emit ProjectOpened will be done later
                spritesPacker->RepackImmediately(projectPath, static_cast<DAVA::eGPUFamily>(SettingsManager::GetValue(Settings::Internal_SpriteViewGPU).AsUInt32()));
            }
            else
            {
                emit ProjectOpened(projectPath.GetAbsolutePathname().c_str());
            }
        }
    }
}

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

void ProjectManager::OpenLastProject()
{
    DAVA::FilePath path = SettingsManager::GetValue(Settings::Internal_LastProjectPath).AsFilePath();
    if (!path.IsEmpty())
    {
        OpenProject(path);
    }
}

void ProjectManager::CloseProject()
{
    if (!projectPath.IsEmpty())
    {
        DAVA::FilePath::RemoveResourcesFolder(projectPath);

        projectPath = "";
        UpdateInternalValues();

        SettingsManager::ResetPerProjectSettings();
        SettingsManager::SetValue(Settings::Internal_LastProjectPath, DAVA::VariantType(DAVA::FilePath())); // reset last project path

        emit ProjectClosed();
    }
}

void ProjectManager::OnSpritesReloaded()
{
    emit ProjectOpened(projectPath.GetAbsolutePathname().c_str());
}

void ProjectManager::LoadProjectSettings()
{
    EditorConfig::Instance()->ParseConfig(projectPath + "EditorConfig.yaml");
}

void ProjectManager::LoadMaterialsSettings()
{
    templates.clear();
    qualities.clear();

    // parse available material templates
    const DAVA::FilePath materialsListPath = DAVA::FilePath("~res:/Materials/assignable.yaml");
    if (DAVA::FileSystem::Instance()->Exists(materialsListPath))
    {
        DAVA::ScopedPtr<DAVA::YamlParser> parser(DAVA::YamlParser::Create(materialsListPath));
        DAVA::YamlNode* rootNode = parser->GetRootNode();

        if (nullptr != rootNode)
        {
            DAVA::FilePath materialsListDir = materialsListPath.GetDirectory();

            for (DAVA::uint32 i = 0; i < rootNode->GetCount(); ++i)
            {
                const DAVA::YamlNode* templateNode = rootNode->Get(i);
                if (nullptr != templateNode)
                {
                    const DAVA::YamlNode* name = templateNode->Get("name");
                    const DAVA::YamlNode* path = templateNode->Get("path");

                    if (nullptr != name && nullptr != path &&
                        name->GetType() == DAVA::YamlNode::TYPE_STRING &&
                        path->GetType() == DAVA::YamlNode::TYPE_STRING)
                    {
                        const DAVA::FilePath templatePath = materialsListDir + path->AsString();
                        if (DAVA::FileSystem::Instance()->Exists(templatePath))
                        {
                            AvailableMaterialTemplate amt;
                            amt.name = name->AsString().c_str();
                            amt.path = templatePath.GetFrameworkPath().c_str();

                            templates.append(amt);
                        }
                    }
                }
            }
        }
    }
}

void ProjectManager::UpdateInternalValues()
{
    if (projectPath.IsEmpty())
    {
        dataSourcePath = "";
        particlesConfigPath = "";
        particlesDataPath = "";
        workspacePath = "";
    }
    else
    {
        dataSourcePath = projectPath + "DataSource/3d/";
        particlesConfigPath = projectPath + "Data/Configs/Particles/";
        particlesDataPath = projectPath + "Data/Gfx/Particles/";
        workspacePath = "~doc:/ResourceEditor/" + projectPath.GetLastDirectoryName() + "/";
    }
}

DAVA::FilePath ProjectManager::CreateProjectPathFromPath(const DAVA::FilePath& pathname)
{
    DAVA::String fullPath = pathname.GetAbsolutePathname();
    DAVA::String::size_type pos = fullPath.find("/Data");
    if (pos != DAVA::String::npos)
    {
        return fullPath.substr(0, pos + 1);
    }

    return DAVA::FilePath();
}
