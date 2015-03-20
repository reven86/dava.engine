#include <QUndoStack>
#include <QApplication>
#include <QAction>
#include <QFileInfo>
#include <QCheckBox>
#include <QMessageBox>
#include <QAbstractItemModel>
#include "Basecontroller.h"
#include "Document.h"
#include "UI/Package/PackageWidget.h"
#include "Ui/Library/LibraryWidget.h"
#include "Ui/Properties/PropertiesWidget.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageNode.h"

BaseController::BaseController(QObject *parent)
    : QObject(parent)
    , documentGroup(this)
    , mainWindow(nullptr) //nullptr is parent
    , currentIndex(-1)
{
    mainWindow.CreateUndoRedoActions(documentGroup.GetUndoGroup());
    connect(&mainWindow, &MainWindow::TabClosed, this, &BaseController::CloseOneDocument);
    connect(mainWindow.GetPackageWidget(), &PackageWidget::SelectionControlChanged, &documentGroup, &DocumentGroup::OnSelectionControlChanged);
    connect(mainWindow.GetPackageWidget(), &PackageWidget::SelectionRootControlChanged, &documentGroup, &DocumentGroup::OnSelectionRootControlChanged);
    connect(&mainWindow, &MainWindow::CloseProject, this, &BaseController::CloseProject);
    connect(&mainWindow, &MainWindow::ActionExitTriggered, this, &BaseController::Exit);
    connect(&mainWindow, &MainWindow::CloseRequested, this, &BaseController::Exit);
    connect(&mainWindow, &MainWindow::RecentMenuTriggered, this, &BaseController::RecentMenu);
    connect(&mainWindow, &MainWindow::ActionOpenProjectTriggered, this, &BaseController::OpenProject);
    connect(&mainWindow, &MainWindow::OpenPackageFile, this, &BaseController::OnOpenPackageFile);
    connect(&mainWindow, &MainWindow::SaveAllDocuments, this, &BaseController::SaveAllDocuments);
    connect(&mainWindow, &MainWindow::SaveDocument, this, static_cast<void(BaseController::*)(int)>(&BaseController::SaveDocument));
    connect(&mainWindow, &MainWindow::CurrentTabChanged, this, &BaseController::SetCurrentIndex);

    connect(&documentGroup, &DocumentGroup::controlSelectedInEditor, mainWindow.GetPackageWidget(), &PackageWidget::OnControlSelectedInEditor);
    connect(&documentGroup, &DocumentGroup::allControlsDeselectedInEditor, mainWindow.GetPackageWidget(), &PackageWidget::OnAllControlsDeselectedInEditor);
}

BaseController::~BaseController()
{

}

void BaseController::Start()
{
    mainWindow.show();
}

void BaseController::OnCleanChanged(bool clean)
{
    // we need to update tab 
    QUndoStack *undoStack = qobject_cast<QUndoStack*>(sender());
    for (int i = 0; i < documents.size(); ++i)
    {
        if (undoStack == documents.at(i)->GetUndoStack())
        {
            mainWindow.OnCleanChanged(i, clean);
        }
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
    DVASSERT(index < documents.size());
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
    DVASSERT(index < documents.size());
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
    mainWindow.SetDocumentToWidgets(document);
    documentGroup.SetActiveDocument(document);
}
void BaseController::DetachDocument(Document *document)
{
    mainWindow.SetDocumentToWidgets(nullptr);
}

void BaseController::CloseDocument(int index)
{
    DVASSERT(index >= 0);
    DVASSERT(index < documents.size());
    int newIndex = mainWindow.CloseTab(index);

    //sync document list with tab list
    Document *detached = documents.takeAt(index);
    documentGroup.RemoveDocument(detached);
    documentGroup.SetActiveDocument(newIndex == -1 ? nullptr : documents.at(newIndex));
    documentGroup.RemoveDocument(detached);
    delete detached; //some widgets hold this document inside :(
}

int BaseController::CreateDocument(PackageNode *package)
{
    Document *document = new Document(package, this);
    connect(document->GetUndoStack(), &QUndoStack::cleanChanged, this, &BaseController::OnCleanChanged);
    documents.push_back(document);
    documentGroup.AddDocument(document);
    documents.push_back(document);
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

int BaseController::CurrentIndex() const
{
    return currentIndex;
}

void BaseController::SetCurrentIndex(int arg)
{
    Q_ASSERT(arg < documents.size());
    if (currentIndex == arg) //arg = -1 when close last tab 
    {
        return;
    }
    DVASSERT(arg < documents.size());
    currentIndex = arg;
    Document *document = arg == -1 ? nullptr : documents.at(arg);
    AttachDocument(document);
    emit CurrentIndexChanged(arg);
}
