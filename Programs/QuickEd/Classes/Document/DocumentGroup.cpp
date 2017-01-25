#include "DocumentGroup.h"

#include "Document/CommandsBase/CommandStackGroup.h"
#include "Document/Document.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/QuickEdPackageBuilder.h"
#include "Project/Project.h"
#include "UI/DocumentGroupView.h"
#include "UI/FileSystemView/FileSystemModel.h"

#include "Command/CommandStack.h"
#include "Debug/DVAssert.h"
#include "UI/UIPackageLoader.h"
#include "Project/Project.h"

#include "QtTools/FileDialogs/FileDialog.h"

#include <QApplication>
#include <QMutableListIterator>
#include <QAction>
#include <QTabBar>
#include <QMessageBox>

using namespace DAVA;

DocumentGroup::DocumentGroup(Project* project_, MainWindow::DocumentGroupView* view_, QObject* parent)
    : QObject(parent)
    , project(project_)
    , view(view_)
    , commandStackGroup(new CommandStackGroup())
{
    connect(qApp, &QApplication::applicationStateChanged, this, &DocumentGroup::OnApplicationStateChanged);
    commandStackGroup->canUndoChanged.Connect(this, &DocumentGroup::CanUndoChanged);
    commandStackGroup->canRedoChanged.Connect(this, &DocumentGroup::CanRedoChanged);
    commandStackGroup->undoCommandChanged.Connect([this](const DAVA::Command* command) { emit UndoTextChanged(GetUndoText()); });
    commandStackGroup->redoCommandChanged.Connect([this](const DAVA::Command* command) { emit RedoTextChanged(GetRedoText()); });

    connect(view, &MainWindow::DocumentGroupView::OpenPackageFile, this, &DocumentGroup::AddDocument);
    connect(this, &DocumentGroup::ActiveDocumentChanged, view, &MainWindow::DocumentGroupView::OnDocumentChanged);

    ConnectToTabBar(view->GetTabBar());

    AttachRedoAction(view->GetActionRedo());
    AttachUndoAction(view->GetActionUndo());

    AttachSaveAction(view->GetActionSaveDocument());
    AttachSaveAllAction(view->GetActionSaveAllDocuments());

    AttachCloseDocumentAction(view->GetActionCloseDocument());
    AttachReloadDocumentAction(view->GetActionReloadDocument());

    view->SetProject(project);
}

DocumentGroup::~DocumentGroup()
{
    view->SetProject(nullptr);
    DisconnectTabBar(view->GetTabBar());
};

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
    return active != nullptr && active->CanClose();
}

QString DocumentGroup::GetUndoText() const
{
    const DAVA::Command* undoCommand = commandStackGroup->GetUndoCommand();
    if (undoCommand != nullptr)
    {
        return QString::fromStdString(undoCommand->GetDescription());
    }
    return QString();
}

QString DocumentGroup::GetRedoText() const
{
    const DAVA::Command* redoCommand = commandStackGroup->GetRedoCommand();
    if (redoCommand != nullptr)
    {
        return QString::fromStdString(redoCommand->GetDescription());
    }
    return QString();
}

void DocumentGroup::AttachUndoAction(QAction* undoAction) const
{
    undoAction->setEnabled(commandStackGroup->CanUndo());
    connect(this, &DocumentGroup::UndoTextChanged, [undoAction](const QString& text) {
        QString actionText = text.isEmpty() ? "Undo" : "Undo: " + text;
        undoAction->setToolTip(actionText);
    });
    connect(this, &DocumentGroup::CanUndoChanged, undoAction, &QAction::setEnabled);
    connect(undoAction, &QAction::triggered, this, &DocumentGroup::Undo);
}

void DocumentGroup::AttachRedoAction(QAction* redoAction) const
{
    redoAction->setEnabled(commandStackGroup->CanRedo());
    connect(this, &DocumentGroup::RedoTextChanged, [redoAction](const QString& text) {
        QString actionText = text.isEmpty() ? "Redo" : "Redo: " + text;
        redoAction->setToolTip(actionText);
    });
    connect(this, &DocumentGroup::CanRedoChanged, redoAction, &QAction::setEnabled);
    connect(redoAction, &QAction::triggered, this, &DocumentGroup::Redo);
}

void DocumentGroup::AttachSaveAction(QAction* saveAction) const
{
    saveAction->setEnabled(CanSave());
    connect(this, &DocumentGroup::CanSaveChanged, saveAction, &QAction::setEnabled);
    connect(saveAction, &QAction::triggered, this, &DocumentGroup::SaveCurrentDocument);
}

void DocumentGroup::AttachSaveAllAction(QAction* saveAllAction) const
{
    saveAllAction->setEnabled(!documents.empty());
    connect(this, &DocumentGroup::CanSaveAllChanged, saveAllAction, &QAction::setEnabled);
    connect(saveAllAction, &QAction::triggered, this, &DocumentGroup::SaveAllDocuments);
}

void DocumentGroup::AttachCloseDocumentAction(QAction* closeDocumentAction) const
{
    closeDocumentAction->setEnabled(CanClose());
    connect(this, &DocumentGroup::CanCloseChanged, closeDocumentAction, &QAction::setEnabled);
    connect(closeDocumentAction, &QAction::triggered, this, &DocumentGroup::TryCloseCurrentDocument);
}

void DocumentGroup::AttachReloadDocumentAction(QAction* reloadDocumentAction) const
{
    reloadDocumentAction->setEnabled(CanClose());
    connect(this, &DocumentGroup::CanCloseChanged, reloadDocumentAction, &QAction::setEnabled);
    connect(reloadDocumentAction, &QAction::triggered, this, &DocumentGroup::ReloadCurrentDocument);
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

Document* DocumentGroup::AddDocument(const QString& path)
{
    DVASSERT(!path.isEmpty());
    if (path.isEmpty())
    {
        return nullptr;
    }

    int index = GetIndexByPackagePath(path);
    if (index == -1)
    {
        index = documents.size();
        Document* document = CreateDocument(path);
        if (nullptr != document)
        {
            InsertDocument(document, index);
        }
        else
        {
            QMessageBox::warning(qApp->activeWindow(), tr("Can not create document"), tr("Can not create document by path:\n%1").arg(path));
            return nullptr;
        }
    }
    SetActiveDocument(index);
    return documents.at(index);
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
            SaveDocument(document, false);
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
    Document* nextDocument = nullptr;
    if (document != active)
    {
        nextDocument = active;
    }
    else if (documents.size() > 1)
    {
        DVASSERT(nullptr != active);
        int activeIndex = documents.indexOf(active);
        DVASSERT(activeIndex != -1);
        activeIndex++;
        if (activeIndex < documents.size())
        {
            nextDocument = documents.at(activeIndex);
        }
        else
        {
            nextDocument = documents.at(documents.size() - 2); //last document will be removed
        }
    }

    const size_t removedCount = documents.removeAll(document);
    DVASSERT(removedCount == 1);
    emit CanSaveAllChanged(!documents.empty());

    commandStackGroup->RemoveStack(document->GetCommandStack());

    SetActiveDocument(nextDocument);
    delete document;
}

void DocumentGroup::ReloadDocument(int index, bool force)
{
    DVASSERT(index >= 0 && index < documents.size());
    QString path = documents.at(index)->GetPackageAbsolutePath();
    if (force)
    {
        CloseDocument(index);
    }
    else
    {
        if (!TryCloseDocument(index))
        {
            return;
        }
    }
    Document* document = CreateDocument(path);
    if (document != nullptr)
    {
        InsertDocument(document, index);
        SetActiveDocument(index);
    }
    else
    {
        QMessageBox::warning(qApp->activeWindow(), tr("Can not create document"), tr("Can not create document by path:\n%1").arg(path));
    }
}

void DocumentGroup::ReloadDocument(Document* document, bool force)
{
    DVASSERT(nullptr != document);
    ReloadDocument(documents.indexOf(document), force);
}

void DocumentGroup::ReloadCurrentDocument()
{
    DVASSERT(nullptr != active);
    ReloadDocument(active, false);
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
    if (active != nullptr)
    {
        disconnect(active, &Document::CanSaveChanged, this, &DocumentGroup::CanSaveChanged);
        disconnect(active, &Document::CanCloseChanged, this, &DocumentGroup::CanCloseChanged);
    }

    active = document;

    if (nullptr == active)
    {
        commandStackGroup->SetActiveStack(nullptr);
    }
    else
    {
        connect(active, &Document::CanSaveChanged, this, &DocumentGroup::CanSaveChanged);
        connect(active, &Document::CanCloseChanged, this, &DocumentGroup::CanCloseChanged);
        commandStackGroup->SetActiveStack(active->GetCommandStack());
    }

    view->SetDocumentActionsEnabled(active != nullptr);

    emit ActiveDocumentChanged(document);
    emit ActiveIndexChanged(documents.indexOf(document));
    emit CanSaveChanged(CanSave());
    emit CanCloseChanged(CanClose());
    emit CanUndoChanged(commandStackGroup->CanUndo());
    emit CanRedoChanged(commandStackGroup->CanRedo());
}

void DocumentGroup::SaveAllDocuments()
{
    for (Document* document : documents)
    {
        SaveDocument(document, true);
    }
}
void DocumentGroup::SaveCurrentDocument()
{
    DVASSERT(nullptr != active);
    SaveDocument(active, true);
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

void DocumentGroup::Undo()
{
    commandStackGroup->Undo();
}

void DocumentGroup::Redo()
{
    commandStackGroup->Redo();
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
    if (!document->CanSave() || qApp->applicationState() == Qt::ApplicationActive)
    {
        ApplyFileChanges();
    }
}

void DocumentGroup::OnFilesChanged(const QList<Document*>& changedFiles)
{
    static bool syncGuard = false;
    if (syncGuard)
    {
        return;
    }
    syncGuard = true;
    SCOPE_EXIT
    {
        syncGuard = false;
    };
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
            ReloadDocument(document, true);
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
    for (Document* document : changedFiles)
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
        QString absPath = documents.at(index)->GetPackageAbsolutePath();
        //normalize file path, because russian letter "й" will be decomposited
        QString normalizedAbsPath = absPath.normalized(QString::NormalizationForm_C);
        if (absPath == path || normalizedAbsPath == path)
        {
            return index;
        }
    }
    return -1;
}

void DocumentGroup::InsertTab(QTabBar* tabBar, Document* document, int index)
{
    bool blockSignals = tabBar->blockSignals(true); //block signals, because insertTab emit currentTabChanged
    int insertedIndex = tabBar->insertTab(index, document->GetName());
    tabBar->blockSignals(blockSignals);
    tabBar->setTabToolTip(insertedIndex, document->GetPackageAbsolutePath());
}

void DocumentGroup::SaveDocument(Document* document, bool force)
{
    DVASSERT(document != nullptr);
    QFileInfo fileInfo(document->GetPackageAbsolutePath());
    if (!fileInfo.exists())
    {
        QString saveFileName = FileDialog::getSaveFileName(qApp->activeWindow(), tr("Save document as"), document->GetPackageAbsolutePath(), "*" + Project::GetUiFileExtension());
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
    else if (!force && document->GetCommandStack()->IsClean())
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
    if (packageRef.Get() != nullptr)
    {
        Document* document = new Document(project, packageRef, this);
        connect(document, &Document::FileChanged, this, &DocumentGroup::OnFileChanged);
        connect(document, &Document::CanSaveChanged, this, &DocumentGroup::OnCanSaveChanged);
        connect(this, &DocumentGroup::FontPresetChanged, document, &Document::OnFontPresetChanged, Qt::DirectConnection);
        return document;
    }
    else
    {
        return nullptr;
    }
}

void DocumentGroup::InsertDocument(Document* document, int index)
{
    DVASSERT(nullptr != document);
    commandStackGroup->AddStack(document->GetCommandStack());
    if (documents.contains(document))
    {
        DVASSERT(false && "document already exists in document group");
        return;
    }
    documents.insert(index, document);
    emit CanSaveAllChanged(!documents.empty());
    for (auto& tabBar : attachedTabBars)
    {
        InsertTab(tabBar, document, index);
    }
}

RefPtr<PackageNode> DocumentGroup::OpenPackage(const FilePath& packagePath)
{
    QuickEdPackageBuilder builder;

    bool packageLoaded = UIPackageLoader(project->GetPrototypes()).LoadPackage(packagePath, &builder);

    if (packageLoaded)
        return builder.BuildPackage();

    return RefPtr<PackageNode>();
}

void DocumentGroup::RefreshControlsStuff()
{
    for (Document* document : documents)
    {
        document->RefreshAllControlProperties();
        document->RefreshLayout();
    }
}

void DocumentGroup::LanguageChanged()
{
    RefreshControlsStuff();
}

void DocumentGroup::RtlChanged()
{
    RefreshControlsStuff();
}

void DocumentGroup::BiDiSupportChanged()
{
    RefreshControlsStuff();
}

void DocumentGroup::GlobalStyleClassesChanged()
{
    RefreshControlsStuff();
}

bool DocumentGroup::TryCloseAllDocuments()
{
    while (!documents.empty())
    {
        if (!TryCloseDocument(documents.first()))
        {
            return false;
        }
    }

    return true;
}

bool DocumentGroup::HasUnsavedDocuments() const
{
    bool hasUnsaved = std::find_if(documents.begin(), documents.end(), [](const Document* document) { return document->CanSave(); }) != documents.end();

    return hasUnsaved;
}

QStringList DocumentGroup::GetUnsavedDocumentsNames() const
{
    QStringList unsavedDocumentsNames;
    for (const Document* document : documents)
    {
        if (document->CanSave())
        {
            unsavedDocumentsNames << document->GetName();
        }
    }
    return unsavedDocumentsNames;
}

void DocumentGroup::CloseAllDocuments()
{
    while (!documents.empty())
    {
        CloseDocument(documents.first());
    }
}
