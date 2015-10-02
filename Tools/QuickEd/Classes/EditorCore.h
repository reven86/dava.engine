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


#ifndef QUICKED_BASECONTROLLER_H
#define QUICKED_BASECONTROLLER_H

#include <QObject>
#include "UI/mainwindow.h"
#include "Project/Project.h"
#include "Base/BaseTypes.h"
#include "Base/Singleton.h"

class QAction;
class Document;
class DocumentGroup;
class Project;
class PackageNode;

class EditorCore final : public QObject, public DAVA::Singleton<EditorCore>
{
    Q_OBJECT
public:
    explicit EditorCore(QObject *parent = nullptr);
    ~EditorCore() = default;
    void Start();

    Project *GetProject() const;

protected slots:
    void OnCleanChanged(bool clean);
    void OnOpenPackageFile(const QString &path);
    void OnProjectPathChanged(const QString &path);

    bool CloseAllDocuments();
    bool CloseOneDocument(int index);
    void SaveDocument(int index);
    void SaveAllDocuments();

    void Exit();
    void RecentMenu(QAction *);
    void OnCurrentTabChanged(int index);
    
    void UpdateLanguage();
   
    void OnRtlChanged(bool isRtl);
    void OnGlobalStyleClassesChanged(const QString &classesStr);

protected:
    void OpenProject(const QString &path);
    bool CloseProject();
    int CreateDocument(PackageNode *package);
    void SaveDocument(Document *document);

private:
    bool eventFilter( QObject *obj, QEvent *event ) override;
    void CloseDocument(int index);
    int GetIndexByPackagePath(const QString &fileName) const;
    ///Return: pointer to currentDocument if exists, nullptr if not
    Project* project = nullptr;
    QList<Document*> documents;
    DocumentGroup* documentGroup = nullptr;
    std::unique_ptr<MainWindow> mainWindow = nullptr;
    DAVA::UIControl* rootControl = nullptr;
};

inline Project* EditorCore::GetProject() const
{
    return project;
}

inline EditorLocalizationSystem *GetEditorLocalizationSystem()
{
    return EditorCore::Instance()->GetProject()->GetEditorLocalizationSystem();
}

inline EditorFontSystem *GetEditorFontSystem()
{
    return EditorCore::Instance()->GetProject()->GetEditorFontSystem();
}

#endif // QUICKED_BASECONTROLLER_H
