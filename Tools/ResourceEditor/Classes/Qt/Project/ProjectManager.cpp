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
#include "Tools/QtFileDialog/QtFileDialog.h"
#include "Qt/Settings/SettingsManager.h"
#include "Deprecated/SceneValidator.h"
#include "Deprecated/EditorConfig.h"

#include "CommandLine/TextureDescriptor/TextureDescriptorUtils.h"
#include "SpritesPacker/SpritePackerHelper.h"

#include "FileSystem/YamlParser.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"

ProjectManager::ProjectManager()
	: curProjectPath("")
	, curProjectPathDataSource("")
{

}

ProjectManager::~ProjectManager()
{

}

bool ProjectManager::IsOpened()
{
	return (curProjectPath != "");
}

QString ProjectManager::CurProjectPath()
{
	return curProjectPath;
}

QString ProjectManager::CurProjectDataSourcePath()
{
	return curProjectPathDataSource;
}

const QVector<ProjectManager::AvailableMaterialTemplate>* ProjectManager::GetAvailableMaterialTemplates() const
{
    return &templates;
}

const QVector<ProjectManager::AvailableMaterialQuality>* ProjectManager::GetAvailableMaterialQualities() const
{
    return &qualities;
}

QString ProjectManager::ProjectOpenDialog()
{
	return QtFileDialog::getExistingDirectory(NULL, QString("Open Project Folder"), QString("/"));
}

void ProjectManager::ProjectOpen(const QString &path)
{
	if(path != curProjectPath)
	{
		ProjectClose();
        
		curProjectPath = path;

		if(!curProjectPath.isEmpty())
		{
			DAVA::FilePath projectPath = PathnameToDAVAStyle(path);
            projectPath.MakeDirectoryPathname();

			DAVA::FilePath dataSource3Dpathname = projectPath + "DataSource/3d/";
			curProjectPathDataSource = dataSource3Dpathname.GetAbsolutePathname().c_str();

			SettingsManager::Instance()->SetValue("LastProjectPath",
				VariantType(projectPath.GetAbsolutePathname()), SettingsManager::INTERNAL);

			EditorConfig::Instance()->ParseConfig(projectPath + "EditorConfig.yaml");

			SceneValidator::Instance()->SetPathForChecking(projectPath);
            SpritePackerHelper::Instance()->UpdateParticleSprites((eGPUFamily)SettingsManager::Instance()->GetValue("TextureViewGPU", SettingsManager::INTERNAL).AsInt32());

            DAVA::FilePath::AddTopResourcesFolder(projectPath);

            LoadProjectSettings();
            LoadMaterialsSettings();

            emit ProjectOpened(curProjectPath);
        }
	}
}

void ProjectManager::ProjectOpenLast()
{
	DAVA::FilePath projectPath = FilePath(SettingsManager::Instance()->GetValue("LastProjectPath", SettingsManager::INTERNAL).AsString());

	if(!projectPath.IsEmpty())
	{
		ProjectOpen(QString(projectPath.GetAbsolutePathname().c_str()));
	}
}

void ProjectManager::ProjectClose()
{
	if("" != curProjectPath)
	{
		FilePath path = curProjectPath.toStdString();
		path.MakeDirectoryPathname();
        
		DAVA::FilePath::RemoveResourcesFolder(path);
        
        curProjectPath = "";
        emit ProjectClosed();
	}
}

void ProjectManager::LoadProjectSettings()
{
	DAVA::FilePath prjPath = DAVA::FilePath(curProjectPath.toStdString());
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

            for(int i = 0; i < rootNode->GetCount(); ++i)
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

    // parse available material qualities
    DAVA::QualitySettingsSystem::Instance()->Load("~res:/quality.yaml");
}