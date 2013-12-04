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
#include "Classes/SceneEditor/EditorSettings.h"
#include "Classes/SceneEditor/SceneValidator.h"
#include "Classes/SceneEditor/EditorConfig.h"
#include "Classes/SceneEditor/SceneEditorScreenMain.h"

#include "Classes/CommandLine/TextureDescriptor/TextureDescriptorUtils.h"
#include "Classes/Qt/SpritesPacker/SpritePackerHelper.h"

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

			EditorSettings::Instance()->SetProjectPath(projectPath);
			EditorSettings::Instance()->SetDataSourcePath(dataSource3Dpathname);
			EditorSettings::Instance()->Save();

			EditorConfig::Instance()->ParseConfig(projectPath + "EditorConfig.yaml");

			SceneValidator::Instance()->SetPathForChecking(projectPath);
            SpritePackerHelper::Instance()->UpdateParticleSprites(EditorSettings::Instance()->GetTextureViewGPU());

            LoadProjectSettings();
            
            emit ProjectOpened(curProjectPath);
            
            DAVA::FilePath::AddTopResourcesFolder(projectPath);
		}
	}
}

void ProjectManager::ProjectOpenLast()
{
	DAVA::FilePath projectPath = EditorSettings::Instance()->GetProjectPath();

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
