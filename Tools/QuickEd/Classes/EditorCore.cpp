#include "Platform/Qt5/QtLayer.h"

#include "UI/mainwindow.h"
#include "DocumentGroup.h"
#include "Document.h"
#include "EditorCore.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "SharedData.h"

using namespace DAVA;

EditorCore::EditorCore(QObject *parent)
    : QObject(parent)
    , Singleton<EditorCore>()
    , project(new Project(this))
    , documentGroup(new DocumentGroup(this))
    , mainWindow(new MainWindow())
{
    mainWindow->CreateUndoRedoActions(documentGroup->GetUndoGroup());
     
    connect(mainWindow, &MainWindow::TabClosed, this, &EditorCore::CloseOneDocument);
    connect(mainWindow, &MainWindow::CurrentTabChanged, this, &EditorCore::OnCurrentTabChanged);
    connect(mainWindow, &MainWindow::CloseProject, this, &EditorCore::CloseProject);
    connect(mainWindow, &MainWindow::ActionExitTriggered, this, &EditorCore::Exit);
    connect(mainWindow, &MainWindow::CloseRequested, this, &EditorCore::Exit);
    connect(mainWindow, &MainWindow::RecentMenuTriggered, this, &EditorCore::RecentMenu);
    connect(mainWindow, &MainWindow::ActionOpenProjectTriggered, this, &EditorCore::OpenProject);
    connect(mainWindow, &MainWindow::OpenPackageFile, this, &EditorCore::OnOpenPackageFile);
    connect(mainWindow, &MainWindow::SaveAllDocuments, this, &EditorCore::SaveAllDocuments);
    connect(mainWindow, &MainWindow::SaveDocument, this, static_cast<void(EditorCore::*)(int)>(&EditorCore::SaveDocument));

    connect(documentGroup, &DocumentGroup::DocumentChanged, mainWindow->libraryWidget, &LibraryWidget::OnDocumentChanged);

    connect(documentGroup, &DocumentGroup::DocumentChanged, mainWindow->propertiesWidget, &PropertiesWidget::OnDocumentChanged);
    connect(documentGroup, &DocumentGroup::SharedDataChanged, mainWindow->propertiesWidget, &PropertiesWidget::OnDataChanged);

    connect(documentGroup, &DocumentGroup::DocumentChanged, mainWindow->packageWidget, &PackageWidget::OnDocumentChanged);
    connect(documentGroup, &DocumentGroup::SharedDataChanged, mainWindow->packageWidget, &PackageWidget::OnDataChanged);

    connect(documentGroup, &DocumentGroup::DocumentChanged, mainWindow->previewWidget, &PreviewWidget::OnDocumentChanged);
    connect(documentGroup, &DocumentGroup::SharedDataChanged, mainWindow->previewWidget, &PreviewWidget::OnDataChanged);
    
    connect(project->GetEditorLocalizationSystem(), &EditorLocalizationSystem::LocaleChanged, this, &EditorCore::UpdateLanguage);

    qApp->installEventFilter(this);
}
    
EditorCore::~EditorCore()
{
    delete mainWindow;
}

void EditorCore::Start()
{
    mainWindow->show();
}

void EditorCore::OnCleanChanged(bool clean)
{
    QUndoStack *undoStack = qobject_cast<QUndoStack*>(sender());
    for (int i = 0; i < documents.size(); ++i)
    {
        if (undoStack == documents.at(i)->GetUndoStack())
        {
            mainWindow->OnCleanChanged(i, clean);
        }
    }
}

void EditorCore::OnOpenPackageFile(const QString &path)
{
    if (!path.isEmpty())
    {
        int index = GetIndexByPackagePath(path);
        if (index == -1)
        {
            DAVA::RefPtr<PackageNode> package = project->OpenPackage(path);
            if (nullptr != package)
            {
                index = CreateDocument(package.Get());
            }
        }
        mainWindow->SetCurrentTab(index);
    }
}

bool EditorCore::CloseOneDocument(int index)
{
    DVASSERT(index >= 0);
    DVASSERT(index < documents.size());
    const Document *document = documents.at(index);
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
    for (auto &document : documents)
    {
        DVVERIFY(project->SavePackage(document->GetPackage()));
        document->GetUndoStack()->setClean();
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
    project->GetEditorFontSystem()->BeginUpdatePreset();
    project->GetEditorFontSystem()->RegisterCurrentLocaleFonts();
    for(auto &document : documents)
    {
        document->BeginUpdateProperty("text");
        document->UpdateProperty("text");
    }
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
    Result result;
    if (!project->Open(path))
    {
        result.addError(Result::CriticalError, tr("Error while loading project"));
    }
    mainWindow->OnProjectOpened(result, path);
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
    int newIndex = mainWindow->CloseTab(index);

    //sync document list with tab list
    Document *detached = documents.takeAt(index);
    documentGroup->SetActiveDocument(newIndex == -1 ? nullptr : documents.at(newIndex));
    documentGroup->RemoveDocument(detached);
    delete detached; //some widgets hold this document inside :(
}

int EditorCore::CreateDocument(PackageNode *package)
{
    Document *document = new Document(package, this);
    connect(document->GetUndoStack(), &QUndoStack::cleanChanged, this, &EditorCore::OnCleanChanged);
    documents.push_back(document);
    documentGroup->AddDocument(document);
    int index = mainWindow->AddTab(document->GetPackageFilePath().GetBasename().c_str());
    OnCurrentTabChanged(index);
    return index;
}

void EditorCore::SaveDocument(Document *document)
{
    DVASSERT(document);
    if (document->GetUndoStack()->isClean())
    {
        return;
    }
    DVVERIFY(project->SavePackage(document->GetPackage())); //TODO:log here
    document->GetUndoStack()->setClean();
}

int EditorCore::GetIndexByPackagePath(const QString &fileName) const
{
    QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();
    DAVA::FilePath davaPath(canonicalFilePath.toStdString());

    for (int index = 0; index < documents.size(); ++index)
    {
        if (documents.at(index)->GetPackageFilePath() == davaPath)
        {
            return index;
        }
    }
    return -1;
}

bool EditorCore::eventFilter( QObject *obj, QEvent *event )
{
    QEvent::Type eventType = event->type();

    if ( qApp == obj )
    {
        if ( QEvent::ApplicationStateChange == eventType )
        {
            QApplicationStateChangeEvent* stateChangeEvent = static_cast<QApplicationStateChangeEvent*>( event );
            Qt::ApplicationState state = stateChangeEvent->applicationState();
            switch ( state )
            {
            case Qt::ApplicationInactive:
            {
                if ( DAVA::QtLayer::Instance() )
                {
                    DAVA::QtLayer::Instance()->OnSuspend();
                }
                break;
            }
            case Qt::ApplicationActive:
            {
                if ( DAVA::QtLayer::Instance() )
                {
                    DAVA::QtLayer::Instance()->OnResume();
                }
                break;
            }
            default:
                break;
            }
        }
    }

    return QObject::eventFilter( obj, event );
}