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


#include "Document.h"
#include "DocumentGroup.h"
#include "Model/PackageHierarchy/PackageNode.h"

#include "Debug/DVAssert.h"
#include "UI/FileSystemView/FileSystemModel.h"
#include "QtTools/FileDialog/FileDialog.h"
#include "Model/EditorUIPackageBuilder.h"
#include "UI/UIPackageLoader.h"

#include <QUndoGroup>
#include <QApplication>
#include <QMutableListIterator>
#include <QAction>
#include <QTabBar>
#include <QMessageBox>

using namespace DAVA;

DocumentGroup::DocumentGroup(QObject* parent)
    : QObject(parent)
    , active(nullptr)
    , undoGroup(new QUndoGroup(this))
{
    connect(qApp, &QApplication::applicationStateChanged, this, &DocumentGroup::OnApplicationStateChanged);
}

DocumentGroup::~DocumentGroup() = default;

QList<Document*> DocumentGroup::GetDocuments() const
{
    return documents;
}

Document* DocumentGroup::GetActiveDocument() const
{
    return active;
}

bool DocumentGroup::CanSave() const
{
    if (active == nullptr)
    {
        return false;
    }
    return active->CanSave();
}

bool DocumentGroup::CanClose() const
{
    return active != nullptr;
}

QAction* DocumentGroup::CreateUndoAction(QObject* parent, const QString& prefix) const
{
    return undoGroup->createUndoAction(parent, prefix);
}

QAction* DocumentGroup::CreateRedoAction(QObject* parent, const QString& prefix) const
{
    return undoGroup->createRedoAction(parent, prefix);
}

void DocumentGroup::AttachSaveAction(QAction* saveAction) const
{
    saveAction->setEnabled(CanSave());
    connect(this, &DocumentGroup::CanSaveChanged, saveAction, &QAction::setEnabled);
    connect(saveAction, &QAction::triggered, this, &DocumentGroup::SaveCurrentDocument);
}

void DocumentGroup::AttachSaveAllAction(QAction* saveAllAction) const
{
    connect(saveAllAction, &QAction::triggered, this, &DocumentGroup::SaveAllDocuments);
}

void DocumentGroup::AttachCloseDocumentAction(QAction* closeDocumentAction) const
{
    closeDocumentAction->setEnabled(CanClose());
    connect(this, &DocumentGroup::CanCloseChanged, closeDocumentAction, &QAction::setEnabled);
    connect(closeDocumentAction, &QAction::triggered, this, &DocumentGroup::TryCloseCurrentDocument);
}

void DocumentGroup::ConnectToTabBar(QTabBar* tabBar)
{
    QPointer<QTabBar> tabPointer(tabBar);
    attachedTabBars.append(tabPointer);
    for (int i = 0, count = documents.size(); i < count; ++i)
    {
        InsertTab(tabBar, documents.at(i), i);
    }

    connect(this, &DocumentGroup::ActiveIndexChanged, tabBar, &QTabBar::setCurrentIndex);
    connect(tabBar, &QTabBar::currentChanged,
            this, static_cast<void (DocumentGroup::*)(int)>(&DocumentGroup::SetActiveDocument));
    connect(tabBar, &QTabBar::tabCloseRequested,
            this, static_cast<bool (DocumentGroup::*)(int)>(&DocumentGroup::TryCloseDocument));
}

void DocumentGroup::DisconnectTabBar(QTabBar* tabBar)
{
    bool found = false;
    QMutableListIterator<QPointer<QTabBar>> iter(attachedTabBars);
    while (iter.hasNext() && !found)
    {
        if (iter.next().data() == tabBar)
        {
            found = true;
            iter.remove();
        }
    }
    if (!found)
    {
        return;
    }
    while (tabBar->count() != 0)
    {
        tabBar->removeTab(0);
    }
    disconnect(this, &DocumentGroup::ActiveIndexChanged, tabBar, &QTabBar::setCurrentIndex);
    disconnect(tabBar, &QTabBar::currentChanged,
               this, static_cast<void (DocumentGroup::*)(int)>(&DocumentGroup::SetActiveDocument));
    disconnect(tabBar, &QTabBar::tabCloseRequested,
               this, static_cast<bool (DocumentGroup::*)(int)>(&DocumentGroup::TryCloseDocument));
}

void DocumentGroup::AddDocument(const QString& path)
{
    DVASSERT(!path.isEmpty());
    if (path.isEmpty())
    {
        return;
    }

    int index = GetIndexByPackagePath(path);
    if (index == -1)
    {
        index = documents.size();
        Document* document = CreateDocument(path);
        InsertDocument(document, index);
    }
    SetActiveDocument(index);
}

bool DocumentGroup::TryCloseCurrentDocument()
{
    if (CanClose())
    {
        return TryCloseDocument(active);
    }
    return false;
}

bool DocumentGroup::TryCloseDocument(int index)
{
    DVASSERT(index >= 0 && index < documents.size());
    return TryCloseDocument(documents.at(index));
}

bool DocumentGroup::TryCloseDocument(Document* document)
{
    DVASSERT(nullptr != document);
    if (document->CanSave())
    {
        QString status = document->IsDocumentExists() ? "modified" : "renamed or removed";
        QMessageBox::StandardButton ret = QMessageBox::question(
        qApp->activeWindow(),
        tr("Save changes"),
        tr("The file %1 has been %2.\n"
           "Do you want to save it?")
        .arg(document->GetPackageFilePath().GetBasename().c_str())
        .arg(status),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
        QMessageBox::Save);
        if (ret == QMessageBox::Save)
        {
            SaveDocument(document);
        }
        else if (ret == QMessageBox::Cancel)
        {
            return false;
        }
    }
    CloseDocument(document);
    return true;
}

void DocumentGroup::CloseDocument(int index)
{
    DVASSERT(index >= 0 && index < documents.size());
    CloseDocument(documents.at(index));
}

void DocumentGroup::CloseDocument(Document* document)
{
    int index = documents.indexOf(document);
    DVASSERT(index != -1);
    for (auto& tabBar : attachedTabBars)
    {
        bool signalsWasBlocked = tabBar->blockSignals(true);
        tabBar->removeTab(index);
        tabBar->blockSignals(signalsWasBlocked);
    }
    DVVERIFY(documents.removeAll(document) == 1);

    undoGroup->removeStack(document->GetUndoStack());

    Document* nextDocument = nullptr;
    if (document != active)
    {
        nextDocument = active;
    }
    else if (!documents.isEmpty())
    {
        nextDocument = documents.first();
    }
    SetActiveDocument(nextDocument);
    delete document;
}

void DocumentGroup::ReloadDocument(int index)
{
    DVASSERT(index >= 0 && index < documents.size());
    QString path = documents.at(index)->GetPackageAbsolutePath();
    CloseDocument(index);
    Document* document = CreateDocument(path);
    InsertDocument(document, index);
    SetActiveDocument(index);
}

void DocumentGroup::ReloadDocument(Document* document)
{
    DVASSERT(nullptr != document);
    ReloadDocument(documents.indexOf(document));
}

void DocumentGroup::SetActiveDocument(int index)
{
    DVASSERT(index >= 0 && index < documents.size());
    SetActiveDocument(documents.at(index));
}

void DocumentGroup::SetActiveDocument(Document* document)
{
    if (active == document)
    {
        return;
    }
    disconnect(active, &Document::CanSaveChanged, this, &DocumentGroup::CanSaveChanged);

    active = document;

    if (nullptr == active)
    {
        undoGroup->setActiveStack(nullptr);
    }
    else
    {
        connect(active, &Document::CanSaveChanged, this, &DocumentGroup::CanSaveChanged);
        undoGroup->setActiveStack(active->GetUndoStack());
    }
    emit ActiveDocumentChanged(document);
    emit ActiveIndexChanged(documents.indexOf(document));
    emit CanSaveChanged(CanSave());
    emit CanCloseChanged(CanClose());
}

void DocumentGroup::SaveAllDocuments()
{
    for (auto& document : documents)
    {
        SaveDocument(document);
    }
}
void DocumentGroup::SaveCurrentDocument()
{
    DVASSERT(nullptr != active);
    SaveDocument(active);
}

void DocumentGroup::OnCanSaveChanged(bool canSave)
{
    Document* document = qobject_cast<Document*>(sender());
    DVASSERT(nullptr != document);
    int index = documents.indexOf(document);
    DVASSERT(index != -1);
    for (auto& tabBar : attachedTabBars)
    {
        QString tabText = tabBar->tabText(index);
        if (canSave && !tabText.endsWith("*"))
        {
            tabText += "*";
        }
        else
        {
            tabText.chop(1);
        }
        tabBar->setTabText(index, tabText);
    }
}

void DocumentGroup::OnApplicationStateChanged(Qt::ApplicationState state)
{
    if (state == Qt::ApplicationActive)
    {
        ApplyFileChanges();
    }
}

void DocumentGroup::OnFileChanged(Document* document)
{
    DVASSERT(nullptr != document);
    changedFiles.insert(document);
    if (document->CanSave() || qApp->applicationState() == Qt::ApplicationActive)
    {
        ApplyFileChanges();
    }
}

void DocumentGroup::OnFilesChanged(const QList<Document*>& changedFiles)
{
    bool yesToAll = false;
    bool noToAll = false;
    int changedCount = std::count_if(changedFiles.begin(), changedFiles.end(), [changedFiles](Document* document) {
        return document->CanSave();
    });
    for (Document* document : changedFiles)
    {
        SetActiveDocument(document);

        QMessageBox::StandardButton button = QMessageBox::No;
        if (!document->CanSave())
        {
            button = QMessageBox::Yes;
        }
        else
        {
            if (!yesToAll && !noToAll)
            {
                QFileInfo fileInfo(document->GetPackageAbsolutePath());
                button = QMessageBox::warning(
                qApp->activeWindow(), tr("File %1 changed").arg(fileInfo.fileName()), tr("%1\n\nThis file has been modified outside of the editor. Do you want to reload it?").arg(fileInfo.absoluteFilePath()), changedCount > 1 ?
                QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll :
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::Yes);
                yesToAll = button == QMessageBox::YesToAll;
                noToAll = button == QMessageBox::NoToAll;
            }
            if (yesToAll || noToAll)
            {
                button = yesToAll ? QMessageBox::Yes : QMessageBox::No;
            }
        }
        if (button == QMessageBox::Yes)
        {
            ReloadDocument(document);
        }
    }
}

void DocumentGroup::OnFilesRemoved(const QList<Document*>& removedFiles)
{
    for (Document* document : removedFiles)
    {
        SetActiveDocument(document);

        QMessageBox::StandardButton button = QMessageBox::No;
        QFileInfo fileInfo(document->GetPackageAbsolutePath());
        button = QMessageBox::warning(
        qApp->activeWindow(),
        tr("File %1 is renamed or removed").arg(fileInfo.fileName()),
        tr("%1\n\nThis file has been renamed or removed. Do you want to close it?")
        .arg(fileInfo.absoluteFilePath()),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No
        );
        if (button == QMessageBox::Yes)
        {
            CloseDocument(document);
        }
    }
}

void DocumentGroup::ApplyFileChanges()
{
    QList<Document*> changed;
    QList<Document*> removed;
    for (const auto& document : changedFiles)
    {
        if (document->IsDocumentExists())
        {
            changed << document;
        }
        else
        {
            removed << document;
        }
    }
    changedFiles.clear();
    if (!changed.empty())
    {
        OnFilesChanged(changed);
    }
    if (!removed.empty())
    {
        OnFilesRemoved(removed);
    }
}

int DocumentGroup::GetIndexByPackagePath(const QString& path) const
{
    for (int index = 0; index < documents.size(); ++index)
    {
        if (documents.at(index)->GetPackageAbsolutePath() == path)
        {
            return index;
        }
    }
    return -1;
}

void DocumentGroup::InsertTab(QTabBar* tabBar, Document* document, int index)
{
    QFileInfo fileInfo(document->GetPackageAbsolutePath());
    QString tabText(fileInfo.fileName());
    bool blockSignals = tabBar->blockSignals(true); //block signals, because insertTab emit currentTabChanged
    int insertedIndex = tabBar->insertTab(index, tabText);
    tabBar->blockSignals(blockSignals);
    tabBar->setTabToolTip(insertedIndex, fileInfo.absoluteFilePath());
}

void DocumentGroup::SaveDocument(Document* document)
{
    DVASSERT(document != nullptr);
    QFileInfo fileInfo(document->GetPackageAbsolutePath());
    if (!fileInfo.exists())
    {
        QString saveFileName = FileDialog::getSaveFileName(qApp->activeWindow(), tr("Save document as"), document->GetPackageAbsolutePath(), "*" + FileSystemModel::GetYamlExtensionString());
        if (!saveFileName.isEmpty())
        {
            FilePath projectPath(saveFileName.toStdString().c_str());

            document->GetPackage()->SetPath(projectPath);
        }
        else
        {
            return;
        }
    }
    else if (document->GetUndoStack()->isClean())
    {
        return;
    }
    document->Save();
}

Document* DocumentGroup::CreateDocument(const QString& path)
{
    QString canonicalFilePath = QFileInfo(path).canonicalFilePath();
    FilePath davaPath(canonicalFilePath.toStdString());
    RefPtr<PackageNode> packageRef = OpenPackage(davaPath);

    Document* document = new Document(packageRef, this);
    connect(document, &Document::FileChanged, this, &DocumentGroup::OnFileChanged);
    connect(document, &Document::CanSaveChanged, this, &DocumentGroup::OnCanSaveChanged);
    return document;
}

void DocumentGroup::InsertDocument(Document* document, int index)
{
    DVASSERT(nullptr != document);
    undoGroup->addStack(document->GetUndoStack());
    if (documents.contains(document))
    {
        DVASSERT(false && "document already exists in document group");
        return;
    }
    documents.insert(index, document);
    for (auto& tabBar : attachedTabBars)
    {
        InsertTab(tabBar, document, index);
    }
}

RefPtr<PackageNode> DocumentGroup::OpenPackage(const FilePath& packagePath)
{
    EditorUIPackageBuilder builder;

    bool packageLoaded = UIPackageLoader().LoadPackage(packagePath, &builder);

    if (packageLoaded)
        return builder.BuildPackage();

    return RefPtr<PackageNode>();
}
