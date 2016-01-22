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


#ifndef QUICKED_DOCUMENTGROUP_H
#define QUICKED_DOCUMENTGROUP_H

#include <QObject>
#include <QSet>
#include <QList>
#include <QPointer>

class Document;
class QUndoGroup;
class PackageBaseNode;
class QAction;
class QTabBar;
class Project;

class DocumentGroup : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool canSave READ CanSave NOTIFY CanSaveChanged)
    Q_PROPERTY(bool canClose READ CanClose NOTIFY CanCloseChanged)
public:
    explicit DocumentGroup(Project* project, QObject* parent = nullptr);
    ~DocumentGroup();

    QList<Document*> GetDocuments() const;
    Document* GetActiveDocument() const;

    bool CanSave() const;
    bool CanClose() const;

    QAction* CreateUndoAction(QObject* parent, const QString& prefix = QString()) const;
    QAction* CreateRedoAction(QObject* parent, const QString& prefix = QString()) const;

    QAction* CreateSaveAction(QObject* parent) const;
    QAction* CreateSaveAllAction(QObject* parent) const;
    QAction* CreateCloseDocumentAction(QObject* parent) const;

    void ConnectToTabBar(QTabBar* tabBar);

signals:
    void ActiveIndexChanged(int index);
    void ActiveDocumentChanged(Document*);
    void CanSaveChanged(bool canSave);
    void CanCloseChanged(bool canClose);

public slots:
    void AddDocument(const QString& path);
    bool RemoveCurrentDocument();
    bool RemoveDocument(int index);
    bool RemoveDocument(Document* document);
    void ReloadDocument(int index);
    void ReloadDocument(Document* document);

    void SetActiveDocument(int index);
    void SetActiveDocument(Document* document);
    void SaveAllDocuments();
    void SaveCurrentDocument();
    void OnCanSaveChanged(bool canSave);

private slots:
    void OnApplicationStateChanged(Qt::ApplicationState state);
    void OnFileChanged(Document* document);

private:
    void OnFilesChanged(const QList<Document*>& changedFiles);
    void OnFilesRemoved(const QList<Document*>& removedFiles);
    void ApplyFileChanges();
    int GetIndexByPackagePath(const QString& davaPath) const;
    void InsertTab(QTabBar* tabBar, Document* document, int index);
    void SaveDocument(Document* document);
    Document* CreateDocument(const QString& path);
    void InsertDocument(Document* document, int pos);

    Document *active;
    QList<Document*> documents;
    QUndoGroup *undoGroup;
    QSet<Document*> changedFiles;
    QList<QPointer<QTabBar>> attachedTabBars;
    QPointer<Project> project;
};

#endif // QUICKED_DOCUMENTGROUP_H
