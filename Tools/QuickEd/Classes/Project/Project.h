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


#ifndef QUICKED__PROJECT_H__
#define QUICKED__PROJECT_H__

#include <QObject>
#include "Model/LegacyEditorUIPackageLoader.h"
#include "Project/EditorFontSystem.h"
#include "Project/EditorLocalizationSystem.h"

class PackageNode;

class Project : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isOpen READ IsOpen NOTIFY IsOpenChanged)
    Q_PROPERTY(QString projectPath READ GetProjectPath WRITE SetProjectPath NOTIFY ProjectPathChanged)

public:
    explicit Project(QObject *parent = nullptr);
    virtual ~Project();
    QString GetProjectDir() const;
    bool Open(const QString &path);
    bool CheckAndUnlockProject(const QString& projectPath);

    DAVA::RefPtr<PackageNode> NewPackage(const QString &path);
    DAVA::RefPtr<PackageNode> OpenPackage(const QString &path);
    bool SavePackage(PackageNode *package);
    EditorFontSystem *GetEditorFontSystem() const;
    EditorLocalizationSystem *GetEditorLocalizationSystem() const;
signals:
    void ProjectOpened();

private:
    bool OpenInternal(const QString &path);
    
    LegacyControlData *legacyData;
    EditorFontSystem *editorFontSystem;
    EditorLocalizationSystem *editorLocalizationSystem;
    //properties
public:
    bool IsOpen() const;
signals:
    void IsOpenChanged(bool arg);
private:
    void SetIsOpen(bool arg);
    bool isOpen;

public:
    QString GetProjectPath() const;
public slots:
    void SetProjectPath(QString arg);
signals:
    void ProjectPathChanged(QString arg);
private:
    DAVA::FilePath projectPath;
};

inline EditorFontSystem* Project::GetEditorFontSystem() const
{
    return editorFontSystem;
}

inline EditorLocalizationSystem* Project::GetEditorLocalizationSystem() const
{
    return editorLocalizationSystem;
}

#endif // QUICKED__PROJECT_H__
