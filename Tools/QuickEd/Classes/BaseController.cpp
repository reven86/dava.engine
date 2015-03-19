#include <QUndoStack>
#include <QApplication>
#include <QAction>
#include <QFileInfo>
#include <QCheckBox>
#include <QMessageBox>
#include "basecontroller.h"
#include "UI/Package/PackageWidget.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageNode.h"

#include "Platform/Qt5/QtLayer.h"


BaseController::BaseController(QObject *parent)
    : QObject(parent)
    , mainWindow(nullptr) //nullptr is parent
    , undoGroup(this)
    , count(0)
    , currentIndex(-1)
{
    mainWindow.CreateUndoRedoActions(undoGroup);
    connect(&mainWindow, &MainWindow::TabClosed, this, &BaseController::CloseOneDocument);
    connect(mainWindow.GetPackageWidget(), &PackageWidget::SelectionControlChanged, this, &BaseController::OnSelectionControlChanged);
    connect(mainWindow.GetPackageWidget(), &PackageWidget::SelectionRootControlChanged, this, &BaseController::OnSelectionRootControlChanged);
    connect(&mainWindow, &MainWindow::CloseProject, this, &BaseController::CloseProject);
    connect(&mainWindow, &MainWindow::ActionExitTriggered, this, &BaseController::Exit);
    connect(&mainWindow, &MainWindow::CloseRequested, this, &BaseController::Exit);
    connect(&mainWindow, &MainWindow::RecentMenuTriggered, this, &BaseController::RecentMenu);
    connect(&mainWindow, &MainWindow::ActionOpenProjectTriggered, this, &BaseController::OpenProject);
    connect(&mainWindow, &MainWindow::OpenPackageFile, this, &BaseController::OnOpenPackageFile);
    connect(&mainWindow, &MainWindow::SaveAllDocuments, this, &BaseController::SaveAllDocuments);
    connect(&mainWindow, &MainWindow::SaveDocument, this, static_cast<void(BaseController::*)(int)>(&BaseController::SaveDocument));
    connect(&mainWindow, &MainWindow::CurrentTabChanged, this, &BaseController::SetCurrentIndex);

    connect(this, &BaseController::CurrentIndexChanged, &mainWindow, &MainWindow::OnCurrentIndexChanged);
    connect(this, &BaseController::CountChanged, &mainWindow, &MainWindow::OnCountChanged);

    qApp->installEventFilter(this);
}

BaseController::~BaseController()
{

}

void BaseController::Start()
{
    mainWindow.show();
}

MainWindow* BaseController::GetMainWindow() const
{
    return const_cast<MainWindow*>( &mainWindow );
}

void BaseController::OnCleanChanged(bool clean)
{
    QUndoStack *undoStack = qobject_cast<QUndoStack*>(sender());
    for (int i = 0; i < documents.size(); ++i)
    {
        if (undoStack == documents.at(i)->GetUndoStack())
        {
            mainWindow.OnCleanChanged(i, clean);
        }
    }
}

void BaseController::OnSelectionRootControlChanged(const QList<ControlNode *> &activatedRootControls, const QList<ControlNode *> &deactivatedRootControls)
{
    Document *document = GetCurrentDocument();
    if (nullptr != document)
    {
        document->OnSelectionRootControlChanged(activatedRootControls, deactivatedRootControls);
    }
}

void BaseController::OnSelectionControlChanged(const QList<ControlNode *> &activatedControls, const QList<ControlNode *> &deactivatedControls)
{
    Document *document = GetCurrentDocument();
    if (nullptr != document)
    {
        document->OnSelectionControlChanged(activatedControls, deactivatedControls);
    }
}

void BaseController::OnControlSelectedInEditor(ControlNode *activatedControls)
{
    Document *document = GetCurrentDocument();
    if (nullptr != document)
    {
        document->OnControlSelectedInEditor(activatedControls);
    }
}

void BaseController::OnAllControlDeselectedInEditor()
{
    Document *document = GetCurrentDocument();
    if (nullptr != document)
    {
        document->OnAllControlDeselectedInEditor();
    }
}

void BaseController::OnOpenPackageFile(const QString &path)
{
    if (!path.isEmpty())
    {
        int index = GetIndexByPackagePath(path);
        if (index == -1)
        {
            DAVA::RefPtr<PackageNode> package = project.OpenPackage(path);
            if (nullptr != package)
            {
                index = CreateDocument(package.Get());
            }
        }
        mainWindow.SetCurrentTab(index);
    }
}

bool BaseController::CloseOneDocument(int index)
{
    DVASSERT(index >= 0);
    DVASSERT(index < Count());
    const Document *document = documents.at(index);
    QUndoStack *undoStack = document->GetUndoStack();
    if (!undoStack->isClean())
    {
        QMessageBox::StandardButton ret = QMessageBox::question(qApp->activeWindow(),
            tr("Save changes"),
            tr("The file %1 has been modified.\n"
            "Do you want to save your changes?").arg(document->PackageFilePath().GetBasename().c_str()),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Save);
        if (ret == QMessageBox::Cancel)
        {
            return false;
        }
        else if (ret == QMessageBox::Save)
        {
            SaveDocument(index);
        }
    }
    CloseDocument(index);
    return true;
}

void BaseController::SaveDocument(int index)
{
    DVASSERT(index >= 0);
    DVASSERT(index < Count());
    SaveDocument(documents[index]);
}

void BaseController::SaveAllDocuments()
{
    for (auto &document : documents)
    {
        DVVERIFY(project.SavePackage(document->GetPackage()));
        document->GetUndoStack()->setClean();
    }
}

void BaseController::Exit()
{
    if (CloseProject())
    {
        QCoreApplication::exit();
    }
}

void BaseController::RecentMenu(QAction *recentProjectAction)
{
    QString projectPath = recentProjectAction->data().toString();

    if (projectPath.isEmpty())
    {
        return;
    }
    OpenProject(projectPath);
}

void BaseController::OpenProject(const QString &path)
{
    if (!CloseProject())
    {
        return;
    }

    if (!project.CheckAndUnlockProject(path))
    {
        return;
    }
    Result result;
    if (!project.Open(path))
    {
        result.addError(Result::CriticalError, tr("Error while loading project"));
    }
    mainWindow.OnProjectOpened(result, path);
}

bool BaseController::CloseProject()
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
        int ret = QMessageBox::question(qApp->activeWindow(),
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

void BaseController::AttachDocument(Document *document)
{
    if (nullptr != document)
    {
        document->SetActive(true);
        connect(document, &Document::controlSelectedInEditor, mainWindow.GetPackageWidget(), &PackageWidget::OnControlSelectedInEditor);
        connect(document, &Document::allControlsDeselectedInEditor, mainWindow.GetPackageWidget(), &PackageWidget::OnAllControlsDeselectedInEditor);
    }
    mainWindow.SetDocumentToWidgets(document);
}
void BaseController::DetachDocument(Document *document)
{
    if (nullptr != document)
    {
        document->SetActive(false);
        disconnect(document, &Document::controlSelectedInEditor, mainWindow.GetPackageWidget(), &PackageWidget::OnControlSelectedInEditor);
        disconnect(document, &Document::allControlsDeselectedInEditor, mainWindow.GetPackageWidget(), &PackageWidget::OnAllControlsDeselectedInEditor);
    }
    mainWindow.SetDocumentToWidgets(nullptr);
}

void BaseController::CloseDocument(int index)
{
    DVASSERT(index >= 0);
    DVASSERT(index < Count());
    int newIndex = mainWindow.CloseTab(index);

    //sync document list with tab list
    Document *detached = documents.takeAt(index);
    undoGroup.removeStack(detached->GetUndoStack());
    DetachDocument(detached);

    //attach new doc
    if (-1 != newIndex)
    {
        AttachDocument(documents.at(newIndex));
    }
    delete detached; //some widgets hold this document inside :(

    SetCount(Count() - 1);
}

int BaseController::CreateDocument(PackageNode *package)
{
    Document *document = new Document(&project, package, this);
    connect(document->GetUndoStack(), &QUndoStack::cleanChanged, this, &BaseController::OnCleanChanged);
    undoGroup.addStack(document->GetUndoStack());
    documents.push_back(document);
    SetCount(Count() + 1);
    int index = mainWindow.AddTab(document->PackageFilePath().GetBasename().c_str());
    return index;
}

void BaseController::SaveDocument(Document *document)
{
    DVASSERT(document);
    if (document->GetUndoStack()->isClean())
    {
        return;
    }
    DVVERIFY(project.SavePackage(document->GetPackage())); //TODO:log here
    document->GetUndoStack()->setClean();
}

int BaseController::GetIndexByPackagePath(const QString &fileName) const
{
    QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();
    DAVA::FilePath davaPath(canonicalFilePath.toStdString());

    for (int index = 0; index < documents.size(); ++index)
    {
        if (documents.at(index)->PackageFilePath() == davaPath)
        {
            return index;
        }
    }
    return -1;
}

Document *BaseController::GetCurrentDocument() const
{
    if (0 == Count() || -1 == CurrentIndex())
    {
        return nullptr;
    }
    return documents.at(CurrentIndex());
}

//properties. must be at the end of file
int BaseController::Count() const
{
    return count;
}

int BaseController::CurrentIndex() const
{
    return currentIndex;
}

void BaseController::SetCount(int arg)
{
    if (count == arg)
        return;

    count = arg;
    emit CountChanged(arg);
}

void BaseController::SetCurrentIndex(int arg)
{
    if (currentIndex == arg) //arg = -1 when close last tab 
    {
        return;
    }
    DVASSERT(arg < documents.size());
    DetachDocument(GetCurrentDocument());
    currentIndex = arg;
    AttachDocument(GetCurrentDocument());
    emit CurrentIndexChanged(arg);
}

bool BaseController::eventFilter( QObject *obj, QEvent *event )
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