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


#ifndef __PROJECT_MANAGER_H__
#define __PROJECT_MANAGER_H__

#include <QObject>
#include <QVector>
#include "DAVAEngine.h"

class ProjectManager
    : public QObject
    , public DAVA::Singleton<ProjectManager>
{
	Q_OBJECT

public:
    struct AvailableMaterialTemplate
    {
        QString name;
        QString path;
    };

    struct AvailableMaterialQuality
    {
        QString name;
        QString prefix;
        QVector<QString> values;
    };

    ProjectManager() = default;
    ~ProjectManager() = default;

    bool IsOpened() const;

    const DAVA::FilePath& GetProjectPath() const;
    const DAVA::FilePath& GetDataSourcePath() const;
    const DAVA::FilePath& GetParticlesPath() const;

    const DAVA::FilePath& GetWorkspacePath() const;

    const QVector<ProjectManager::AvailableMaterialTemplate>* GetAvailableMaterialTemplates() const;
    const QVector<ProjectManager::AvailableMaterialQuality>* GetAvailableMaterialQualities() const;

    static DAVA::FilePath CreateProjectPathFromPath(const DAVA::FilePath& pathname);

public slots:
    DAVA::FilePath ProjectOpenDialog() const;
    void OpenProject(const QString& path);
    void OpenProject(const DAVA::FilePath& path);
    void OpenLastProject();
    void CloseProject();
    void OnSceneViewInitialized();
    void UpdateParticleSprites();

signals:
	void ProjectOpened(const QString &path);
	void ProjectClosed();
    
private:
    void LoadProjectSettings();
    void LoadMaterialsSettings();

    void UpdateInternalValues();

    DAVA::FilePath projectPath;
    DAVA::FilePath dataSourcePath;
    DAVA::FilePath particlesPath;
    DAVA::FilePath workspacePath;

    QVector<AvailableMaterialTemplate> templates;
    QVector<AvailableMaterialQuality> qualities;
    bool useDelayInitialization = true;
    bool isParticleSpritesUpdated = false;
};

#endif // __PROJECT_MANAGER_H__ 
