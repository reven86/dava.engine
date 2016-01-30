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


#include "Platform/Qt5/QtLayer.h"
#include "UI/mainwindow.h"
#include "UI/Preview/ScrollAreaController.h"
#include "UI/Preview/Ruler/RulerController.h"
#include "DocumentGroup.h"
#include "Document.h"
#include "EditorCore.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "QtTools/ReloadSprites/DialogReloadSprites.h"
#include "QtTools/ReloadSprites/SpritesPacker.h"
#include "QtTools/DavaGLWidget/davaglwidget.h"
#include "EditorSettings.h"

#include <QSettings>
#include <QVariant>
#include <QByteArray>
#include <QFileSystemWatcher>

#include "UI/Layouts/UILayoutSystem.h"
#include "UI/Styles/UIStyleSheetSystem.h"
#include "UI/UIControlSystem.h"
#include "Utils/Utils.h"

using namespace DAVA;

EditorCore::EditorCore(QObject* parent)
    : QObject(parent)
    , Singleton<EditorCore>()
    , spritesPacker(std::make_unique<SpritesPacker>())
    , project(new Project(this))
    , documentGroup(new DocumentGroup(this))
    , mainWindow(std::make_unique<MainWindow>())
    , fileSystemWatcher(new QFileSystemWatcher(this))
{
    connect(qApp, &QApplication::applicationStateChanged, this, &EditorCore::OnApplicationStateChanged);
    connect(fileSystemWatcher, &QFileSystemWatcher::fileChanged, this, &EditorCore::OnFileChanged);

    mainWindow->setWindowIcon(QIcon(":/icon.ico"));
    mainWindow->CreateUndoRedoActions(documentGroup->GetUndoGroup());

    connect(mainWindow->actionReloadSprites, &QAction::triggered, this, &EditorCore::OnReloadSprites);
    connect(project, &Project::ProjectPathChanged, this, &EditorCore::OnProjectPathChanged);
    connect(mainWindow.get(), &MainWindow::TabClosed, this, &EditorCore::CloseOneDocument);
    connect(mainWindow.get(), &MainWindow::CurrentTabChanged, this, &EditorCore::OnCurrentTabChanged);
    connect(mainWindow.get(), &MainWindow::CloseProject, this, &EditorCore::CloseProject);
    connect(mainWindow.get(), &MainWindow::ActionExitTriggered, this, &EditorCore::Exit);
    connect(mainWindow.get(), &MainWindow::CloseRequested, this, &EditorCore::Exit);
    connect(mainWindow.get(), &MainWindow::RecentMenuTriggered, this, &EditorCore::RecentMenu);
    connect(mainWindow.get(), &MainWindow::ActionOpenProjectTriggered, this, &EditorCore::OpenProject);
    connect(mainWindow.get(), &MainWindow::OpenPackageFile, this, &EditorCore::OnOpenPackageFile);
    connect(mainWindow.get(), &MainWindow::SaveAllDocuments, this, &EditorCore::SaveAllDocuments);
    connect(mainWindow.get(), &MainWindow::SaveDocument, this, static_cast<void (EditorCore::*)(int)>(&EditorCore::SaveDocument));
    connect(mainWindow.get(), &MainWindow::RtlChanged, this, &EditorCore::OnRtlChanged);
    connect(mainWindow.get(), &MainWindow::BiDiSupportChanged, this, &EditorCore::OnBiDiSupportChanged);
    connect(mainWindow.get(), &MainWindow::GlobalStyleClassesChanged, this, &EditorCore::OnGlobalStyleClassesChanged);
    QComboBox* languageComboBox = mainWindow->GetComboBoxLanguage();
    EditorLocalizationSystem* editorLocalizationSystem = project->GetEditorLocalizationSystem();
    connect(languageComboBox, &QComboBox::currentTextChanged, editorLocalizationSystem, &EditorLocalizationSystem::SetCurrentLocale);
    connect(editorLocalizationSystem, &EditorLocalizationSystem::CurrentLocaleChanged, languageComboBox, &QComboBox::setCurrentText);
    
    auto previewWidget = mainWindow->previewWidget;
    
    connect(documentGroup, &DocumentGroup::ActiveDocumentChanged, previewWidget, &PreviewWidget::SaveSystemsContextAndClear); //this context will affect other widgets, so he must be updated before other widgets take new document
    connect(documentGroup, &DocumentGroup::ActiveDocumentChanged, previewWidget, &PreviewWidget::OnDocumentChanged);
    connect(mainWindow.get(), &MainWindow::EmulationModeChanged, previewWidget, &PreviewWidget::OnEmulationModeChanged);

    connect(documentGroup, &DocumentGroup::ActiveDocumentChanged, mainWindow.get(), &MainWindow::OnDocumentChanged);
    connect(documentGroup, &DocumentGroup::ActiveDocumentChanged, mainWindow->libraryWidget, &LibraryWidget::OnDocumentChanged);

    connect(documentGroup, &DocumentGroup::ActiveDocumentChanged, mainWindow->propertiesWidget, &PropertiesWidget::OnDocumentChanged);

    auto packageWidget = mainWindow->packageWidget;
    connect(documentGroup, &DocumentGroup::ActiveDocumentChanged, packageWidget, &PackageWidget::OnDocumentChanged);
    connect(packageWidget, &PackageWidget::CurrentIndexChanged, mainWindow->propertiesWidget, &PropertiesWidget::UpdateModel);
    connect(previewWidget, &PreviewWidget::DeleteRequested, packageWidget, &PackageWidget::OnDelete);
    connect(previewWidget, &PreviewWidget::ImportRequested, packageWidget, &PackageWidget::OnImport);
    connect(previewWidget, &PreviewWidget::CutRequested, packageWidget, &PackageWidget::OnCut);
    connect(previewWidget, &PreviewWidget::CopyRequested, packageWidget, &PackageWidget::OnCopy);
    connect(previewWidget, &PreviewWidget::PasteRequested, packageWidget, &PackageWidget::OnPaste);
    connect(previewWidget, &PreviewWidget::SelectionChanged, packageWidget, &PackageWidget::OnSelectionChanged);
    connect(packageWidget, &PackageWidget::SelectedNodesChanged, previewWidget, &PreviewWidget::OnSelectionChanged);

    connect(previewWidget->GetGLWidget(), &DavaGLWidget::Initialized, this, &EditorCore::OnGLWidgedInitialized);
    connect(project->GetEditorLocalizationSystem(), &EditorLocalizationSystem::CurrentLocaleChanged, this, &EditorCore::UpdateLanguage);
    
    connect(documentGroup, &DocumentGroup::ActiveDocumentChanged, previewWidget, &PreviewWidget::LoadSystemsContext); //this context will affect other widgets, so he must be updated when other widgets took new document
}

EditorCore::~EditorCore() = default;

MainWindow* EditorCore::GetMainWindow() const
{
    return mainWindow.get();
}

Project* EditorCore::GetProject() const
{
    return project;
}

void EditorCore::Start()
{
    mainWindow->show();
}

void EditorCore::OnReloadSprites()
{
    if (CloseAllDocuments())
    {
        mainWindow->ExecDialogReloadSprites(spritesPacker.get());
    }
}

void EditorCore::OnFilesChanged(const QStringList& changedFiles)
{
    bool yesToAll = false;
    bool noToAll = false;
    int changedCount = std::count_if(documents.begin(), documents.end(), [changedFiles](Document* document) {
        return !document->GetUndoStack()->isClean() && changedFiles.contains(document->GetPackageAbsolutePath());
    });
    for (Document* document : documents)
    {
        QString path = document->GetPackageAbsolutePath();
        if (changedFiles.contains(path))
        {
            documentGroup->SetActiveDocument(document);

            DVASSERT(QFileInfo::exists(path));
            QMessageBox::StandardButton button = QMessageBox::No;
            if (document->GetUndoStack()->isClean())
            {
                button = QMessageBox::Yes;
            }
            else
            {
                if (!yesToAll && !noToAll)
                {
                    QFileInfo fileInfo(path);
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
            int index = documents.indexOf(document);
            if (button == QMessageBox::Yes)
            {
                DAVA::FilePath davaPath = document->GetPackageFilePath();
                CloseDocument(index);
                RefPtr<PackageNode> package = project->OpenPackage(davaPath);
                DVASSERT(package.Get() != nullptr);
                CreateDocument(index, package);
            }
        }
    }
}

void EditorCore::OnFilesRemoved(const QStringList& removedFiles)
{
    for (Document* document : documents)
    {
        QString path = document->GetPackageAbsolutePath();
        if (removedFiles.contains(path))
        {
            documentGroup->SetActiveDocument(document);

            QMessageBox::StandardButton button = QMessageBox::No;
            QFileInfo fileInfo(path);
            button = QMessageBox::warning(
            qApp->activeWindow(), tr("File %1 is renamed or deleted").arg(fileInfo.fileName()), tr("%1\n\nThis file has been renamed or deleted outside of the editor. Do you want to close it?").arg(fileInfo.absoluteFilePath()), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
            if (button == QMessageBox::Yes)
            {
                CloseDocument(documents.indexOf(document));
            }
        }
    }
}

void EditorCore::OnGLWidgedInitialized()
{
    int32 projectCount = EditorSettings::Instance()->GetLastOpenedCount();
    if (projectCount > 0)
    {
        OpenProject(QDir::toNativeSeparators(QString(EditorSettings::Instance()->GetLastOpenedFile(0).c_str())));
    }
}

void EditorCore::OnOpenPackageFile(const QString &path)
{
    if (!path.isEmpty())
    {
        QString canonicalFilePath = QFileInfo(path).canonicalFilePath();
        FilePath davaPath(canonicalFilePath.toStdString());
        int index = GetIndexByPackagePath(davaPath);
        if (index == -1)
        {
            RefPtr<PackageNode> package = project->OpenPackage(davaPath);
            DVASSERT(package.Get() != nullptr);
            index = CreateDocument(documents.size(), package);
        }
        mainWindow->SetCurrentTab(index);
    }
}

void EditorCore::OnProjectPathChanged(const QString &projectPath)
{
    if (EditorSettings::Instance()->IsUsingAssetCache())
    {
        spritesPacker->SetCacheTool(
        EditorSettings::Instance()->GetAssetCacheIp(),
        EditorSettings::Instance()->GetAssetCachePort(),
        EditorSettings::Instance()->GetAssetCacheTimeoutSec());
    }
    else
    {
        spritesPacker->ClearCacheTool();
    }

    QRegularExpression searchOption("gfx\\d*$", QRegularExpression::CaseInsensitiveOption);
    spritesPacker->ClearTasks();
    QDirIterator it(projectPath + "/DataSource");
    while (it.hasNext())
    {
        const QFileInfo &fileInfo = it.fileInfo();
        it.next();
        if (fileInfo.isDir())
        {
            QString outputPath = fileInfo.absoluteFilePath();
            if (!outputPath.contains(searchOption))
            {
                continue;
            }
            outputPath.replace(outputPath.lastIndexOf("DataSource"), QString("DataSource").size(), "Data");
            QDir outputDir(outputPath);
            spritesPacker->AddTask(fileInfo.absoluteFilePath(), outputDir);
        }
    }
}

bool EditorCore::CloseAllDocuments()
{
    for (auto index = documents.size() - 1; index >= 0; --index)
    {
        if (!CloseOneDocument(index))
            return false;
    }
    return true;
}

bool EditorCore::CloseOneDocument(int index)
{
    DVASSERT(index >= 0);
    DVASSERT(index < documents.size());
    Document* document = documents.at(index);
    QUndoStack *undoStack = document->GetUndoStack();
    if (!undoStack->isClean())
    {
        QMessageBox::StandardButton ret = QMessageBox::question(
            qApp->activeWindow(),
            tr("Save changes"),
            tr("The file %1 has been modified.\n"
               "Do you want to save your changes?").arg(document->GetPackageFilePath().GetBasename().c_str()),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Save
            );
        if (ret == QMessageBox::Save)
        {
            SaveDocument(index);
        }
        else if (ret == QMessageBox::Cancel)
        {
            return false;
        }
    }
    CloseDocument(index);
    return true;
}

void EditorCore::SaveDocument(int index)
{
    DVASSERT(index >= 0);
    DVASSERT(index < documents.size());
    SaveDocument(documents[index]);
}

void EditorCore::SaveAllDocuments()
{
    for (int i = 0, size = documents.size(); i < size; ++i)
    {
        SaveDocument(i);
    }
}

void EditorCore::Exit()
{
    if (CloseProject())
    {
        QCoreApplication::exit();
    }
}

void EditorCore::RecentMenu(QAction *recentProjectAction)
{
    QString projectPath = recentProjectAction->data().toString();

    if (projectPath.isEmpty())
    {
        return;
    }
    OpenProject(projectPath);
}

void EditorCore::OnCurrentTabChanged(int index)
{
    documentGroup->SetActiveDocument(index == -1 ? nullptr : documents.at(index));
}

void EditorCore::UpdateLanguage()
{
    project->GetEditorFontSystem()->RegisterCurrentLocaleFonts();
    for(auto &document : documents)
    {
        document->RefreshAllControlProperties();
        document->RefreshLayout();
    }
}

void EditorCore::OnRtlChanged(bool isRtl)
{
    UIControlSystem::Instance()->SetRtl(isRtl);
    for(auto &document : documents)
    {
        document->RefreshAllControlProperties();
        document->RefreshLayout();
    }
}

void EditorCore::OnBiDiSupportChanged(bool support)
{
    UIControlSystem::Instance()->SetBiDiSupportEnabled(support);
    for (auto &document : documents)
    {
        document->RefreshAllControlProperties();
        document->RefreshLayout();
    }
}

void EditorCore::OnGlobalStyleClassesChanged(const QString &classesStr)
{
    Vector<String> tokens;
    Split(classesStr.toStdString(), " ", tokens);

    UIControlSystem::Instance()->GetStyleSheetSystem()->ClearGlobalClasses();
    for (String &token : tokens)
        UIControlSystem::Instance()->GetStyleSheetSystem()->AddGlobalClass(FastName(token));

    for(auto &document : documents)
    {
        document->RefreshAllControlProperties();
        document->RefreshLayout();
    }
}

void EditorCore::OnApplicationStateChanged(Qt::ApplicationState state)
{
    if (state == Qt::ApplicationActive)
    {
        ApplyFileChanges();
    }
}

void EditorCore::OnFileChanged(const QString& path)
{
    changedFiles.insert(path);
    Document* changedDocument = GetDocument(path);
    DVASSERT(nullptr != changedDocument);
    if ((QFileInfo::exists(path) && changedDocument->GetUndoStack()->isClean()) || qApp->applicationState() == Qt::ApplicationActive)
    {
        ApplyFileChanges();
    }
}

void EditorCore::ApplyFileChanges()
{
    QStringList changed;
    QStringList removed;
    for (const QString& filePath : changedFiles)
    {
        if (QFileInfo::exists(filePath))
        {
            changed << filePath;
        }
        else
        {
            removed << filePath;
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

Document* EditorCore::GetDocument(const QString& path) const
{
    FilePath davaPath(path.toStdString().c_str());
    for (Document* document : documents)
    {
        if (document->GetPackageFilePath() == davaPath)
        {
            return document;
        }
    }
    return nullptr;
}

void EditorCore::OpenProject(const QString &path)
{
    if (!CloseProject())
    {
        return;
    }

    if (!project->CheckAndUnlockProject(path))
    {
        return;
    }
    ResultList resultList;
    if (!project->Open(path))
    {
        resultList.AddResult(Result::RESULT_ERROR, "Error while loading project");
    }
    mainWindow->OnProjectOpened(resultList, project);
}

bool EditorCore::CloseProject()
{
    bool hasUnsaved = false;
    for (auto &doc : documents)
    {
        if (!doc->GetUndoStack()->isClean())
        {
            hasUnsaved = true;
            break;
        }
    }
    if (hasUnsaved)
    {
        int ret = QMessageBox::question(
            qApp->activeWindow(),
            tr("Save changes"),
            tr("Some files has been modified.\n"
                "Do you want to save your changes?"),
            QMessageBox::SaveAll | QMessageBox::NoToAll | QMessageBox::Cancel
            );
        if (ret == QMessageBox::Cancel)
        {
            return false;
        }
        else if (ret == QMessageBox::SaveAll)
        {
            SaveAllDocuments();
        }
    }

    while(!documents.isEmpty())
    {
        CloseDocument(0);
    }
    return true;
}

void EditorCore::CloseDocument(int index)
{
    DVASSERT(index >= 0);
    DVASSERT(index < documents.size());
    Document* activeDocument = documentGroup->GetActiveDocument();
    //we will set active document after check current document
    disconnect(mainWindow.get(), &MainWindow::CurrentTabChanged, this, &EditorCore::OnCurrentTabChanged);
    int newIndex = mainWindow->CloseTab(index);
    connect(mainWindow.get(), &MainWindow::CurrentTabChanged, this, &EditorCore::OnCurrentTabChanged);

    DVASSERT(activeDocument != nullptr);
    Document *detached = documents.takeAt(index);
    Document* nextDocument = nullptr;
    if (detached != activeDocument)
    {
        nextDocument = activeDocument;
    }
    else if (newIndex != -1)
    {
        nextDocument = documents.at(newIndex);
    }
    //sync document list with tab list
    fileSystemWatcher->removePath(detached->GetPackageAbsolutePath());

    documentGroup->SetActiveDocument(nextDocument);
    documentGroup->RemoveDocument(detached);
    delete detached; //some widgets hold this document inside :(
}

int EditorCore::CreateDocument(int index, const RefPtr<PackageNode> &package)
{    
    Document *document = new Document(package, this);
    documents.insert(index, document);
    documentGroup->InsertDocument(index, document);
    int insertedIndex = mainWindow->AddTab(document, index);
    OnCurrentTabChanged(insertedIndex);
    QString path = document->GetPackageAbsolutePath();
    if (!fileSystemWatcher->addPath(path))
    {
        DAVA::Logger::Error("can not add path to the file watcher: %s", path.toUtf8().data());
    }
    return index;
}

void EditorCore::SaveDocument(Document *document)
{
    DVASSERT(document);
    if (document->GetUndoStack()->isClean())
    {
        return;
    }
    QString path = document->GetPackageAbsolutePath();
    fileSystemWatcher->removePath(path);
    DVVERIFY(project->SavePackage(document->GetPackage())); //TODO:log here
    document->GetUndoStack()->setClean();
    if (!fileSystemWatcher->addPath(path))
    {
        DAVA::Logger::Error("can not add path to the file watcher: %s", path.toUtf8().data());
    }
}

int EditorCore::GetIndexByPackagePath(const FilePath& davaPath) const
{
    for (int index = 0; index < documents.size(); ++index)
    {
        if (documents.at(index)->GetPackageFilePath() == davaPath)
        {
            return index;
        }
    }
    return -1;
}

