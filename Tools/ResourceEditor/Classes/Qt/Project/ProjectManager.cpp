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
#include "Qt/Settings/SettingsManager.h"
#include "Deprecated/SceneValidator.h"
#include "Deprecated/EditorConfig.h"

#include "CommandLine/TextureDescriptor/TextureDescriptorUtils.h"
#include "SpritesPacker/SpritePackerHelper.h"

#include "FileSystem/YamlParser.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"

#include "QtTools/FileDialog/FileDialog.h"

#include <QDebug>


ProjectManager::ProjectManager()
	: curProjectPath()
	, curProjectPathDataSource()
    , curProjectPathParticles()
    , useDelayInitialization(true)
    , isParticleSpritesUpdated(false)
{
}

ProjectManager::~ProjectManager()
{
}

bool ProjectManager::IsOpened() const
{
	return (!curProjectPath.IsEmpty());
}

const FilePath & ProjectManager::CurProjectPath() const
{
	return curProjectPath;
}

const FilePath & ProjectManager::CurProjectDataSourcePath() const
{
	return curProjectPathDataSource;
}

const FilePath & ProjectManager::CurProjectDataParticles() const
{
    return curProjectPathParticles;
}

const QVector<ProjectManager::AvailableMaterialTemplate>* ProjectManager::GetAvailableMaterialTemplates() const
{
    return &templates;
}

const QVector<ProjectManager::AvailableMaterialQuality>* ProjectManager::GetAvailableMaterialQualities() const
{
    return &qualities;
}

FilePath ProjectManager::ProjectOpenDialog()
{
    FilePath ret;

    QString newPathStr = FileDialog::getExistingDirectory(NULL, QString("Open Project Folder"), QString("/"));
    if(!newPathStr.isEmpty())
    {
        ret = FilePath(PathnameToDAVAStyle(newPathStr));
        ret.MakeDirectoryPathname();
    }
    
	return ret;
}

void ProjectManager::ProjectOpen(const QString &path)
{
    FilePath incomePath(PathnameToDAVAStyle(path));
    ProjectOpen(incomePath);
}

void ProjectManager::ProjectOpen(const FilePath & incomePath)
{
    if(incomePath.IsDirectoryPathname() && incomePath != curProjectPath)
	{
		ProjectClose();
        
        curProjectPath = incomePath;
        
		if(incomePath.Exists())
		{
			DAVA::FilePath dataSource3Dpathname = curProjectPath + "DataSource/3d/";
			curProjectPathDataSource = dataSource3Dpathname.GetAbsolutePathname().c_str();

            DAVA::FilePath particlesPathname = curProjectPath + "Data/Configs/Particles/";
			curProjectPathParticles = particlesPathname.GetAbsolutePathname().c_str();

			SettingsManager::SetValue(Settings::Internal_LastProjectPath, VariantType(curProjectPath));

			EditorConfig::Instance()->ParseConfig(curProjectPath + "EditorConfig.yaml");

			SceneValidator::Instance()->SetPathForChecking(curProjectPath);
            if (!useDelayInitialization)
            {
                UpdateParticleSprites();
            }

            DAVA::FilePath::AddTopResourcesFolder(curProjectPath);

            LoadProjectSettings();
            LoadMaterialsSettings();
            DAVA::QualitySettingsSystem::Instance()->Load("~res:/quality.yaml");
            DAVA::SoundSystem::Instance()->InitFromQualitySettings();

            emit ProjectOpened(curProjectPath.GetAbsolutePathname().c_str());
        }
	}
}

void ProjectManager::UpdateParticleSprites()
{
    useDelayInitialization = false;
    if (!isParticleSpritesUpdated)
    {
        SpritePackerHelper::Instance()->UpdateParticleSprites(static_cast<eGPUFamily>(SettingsManager::GetValue(Settings::Internal_TextureViewGPU).AsUInt32()));
        isParticleSpritesUpdated = true;
    }
}

void ProjectManager::ProjectOpenLast()
{
    DAVA::FilePath projectPath = SettingsManager::GetValue(Settings::Internal_LastProjectPath).AsFilePath();
	if(!projectPath.IsEmpty())
	{
		ProjectOpen(QString(projectPath.GetAbsolutePathname().c_str()));
	}
}

void ProjectManager::ProjectClose()
{
	if(!curProjectPath.IsEmpty())
	{
        isParticleSpritesUpdated = false;
		DAVA::FilePath::RemoveResourcesFolder(curProjectPath);
        curProjectPath = "";
        curProjectPathDataSource = "";
        curProjectPathParticles = "";
        SettingsManager::ResetPerProjectSettings();
        
        SettingsManager::SetValue(Settings::Internal_LastProjectPath, VariantType(DAVA::FilePath())); // reset last project path
        
        emit ProjectClosed();
	}
}

void ProjectManager::OnSceneViewInitialized()
{
    useDelayInitialization = false;
}

void ProjectManager::LoadProjectSettings()
{
	DAVA::FilePath prjPath = curProjectPath;
	prjPath.MakeDirectoryPathname();
	EditorConfig::Instance()->ParseConfig(prjPath + "EditorConfig.yaml");
}

void ProjectManager::LoadMaterialsSettings()
{
    templates.clear();
    qualities.clear();

    // parse available material templates
    DAVA::FilePath materialsListPath = DAVA::FilePath("~res:/Materials/assignable.yaml");
    if(materialsListPath.Exists())
    {
        DAVA::YamlParser *parser = DAVA::YamlParser::Create(materialsListPath);
        DAVA::YamlNode *rootNode = parser->GetRootNode();

        if(NULL != rootNode)
        {
            DAVA::FilePath materialsListDir = materialsListPath.GetDirectory();

            for(uint32 i = 0; i < rootNode->GetCount(); ++i)
            {
                const DAVA::YamlNode *templateNode = rootNode->Get(i);
                if(NULL != templateNode)
                {
                    const DAVA::YamlNode *name = templateNode->Get("name");
                    const DAVA::YamlNode *path = templateNode->Get("path");

                    if(NULL != name && NULL != path &&
                        name->GetType() == DAVA::YamlNode::TYPE_STRING &&
                        path->GetType() == DAVA::YamlNode::TYPE_STRING)
                    {
                        DAVA::FilePath templatePath = DAVA::FilePath(materialsListDir, path->AsString());
                        if(templatePath.Exists())
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

        parser->Release();
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
