#include "Document.h"
#include <DAVAEngine.h>
#include <QLineEdit>
#include <QAction>
#include <QItemSelection>

#include "UI/Package/PackageModel.h"
#include "UI/Package/FilteredPackageModel.h"
#include "UI/Library/LibraryModel.h"
#include "UI/PreviewContext.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageRef.h"

#include "Package/PackageWidget.h"
#include "Preview/PreviewWidget.h"
#include "Properties/PropertiesWidget.h"
#include "Library/LibraryWidget.h"

#include "PackageContext.h"
#include "PropertiesContext.h"
#include "LibraryContext.h"
#include "PreviewContext.h"
#include "DocumentWidgets.h"

#include "QtModelPackageCommandExecutor.h"

using namespace DAVA;

Document::Document(Project *_project, PackageNode *_package, QObject *parent)
    : QObject(parent)
    , project(_project)
    , package(SafeRetain(_package))
    , packageContext(nullptr)
    , propertiesContext(nullptr)
    , previewContext(nullptr)
    , libraryContext(nullptr)
    , commandExecutor(nullptr)
{
    undoStack = new QUndoStack(this);

    packageContext = new PackageContext(this);
    propertiesContext = new PropertiesContext(this);
    libraryContext = new LibraryContext(this);
    previewContext = new PreviewContext(this);
    
    connect(this, SIGNAL(activeRootControlsChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)), previewContext, SLOT(OnActiveRootControlsChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)));
    connect(this, SIGNAL(controlsSelectionChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)), previewContext, SLOT(OnSelectedControlsChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)));

    connect(previewContext, SIGNAL(ControlNodeSelected(ControlNode*)), this, SLOT(OnControlSelectedInEditor(ControlNode*)));
    connect(previewContext, SIGNAL(AllControlsDeselected()), this, SLOT(OnAllControlDeselectedInEditor()));

    
    PackageControlsNode *controlsNode = package->GetPackageControlsNode();
    for (int32 index = 0; index < controlsNode->GetCount(); ++index)
        activeRootControls.push_back(controlsNode->Get(index));

    if (!activeRootControls.empty())
        emit activeRootControlsChanged(activeRootControls, QList<ControlNode*>());
    
    commandExecutor = new QtModelPackageCommandExecutor(this);
}

Document::~Document()
{
    SafeDelete(packageContext);
    SafeDelete(propertiesContext);
    SafeDelete(libraryContext);
    SafeDelete(previewContext);
    
    SafeRelease(package);
    
    SafeRelease(commandExecutor);
}

void Document::ConnectToWidgets(DocumentWidgets *widgets)
{
    widgets->SetWidgetsEnabled(true);
    
    widgets->GetPackageWidget()->SetDocument(this);
    widgets->GetPreviewWidget()->SetDocument(this);
    widgets->GetPropertiesWidget()->SetContext(propertiesContext);
    widgets->GetLibraryWidget()->SetDocument(this);
    
    connect(widgets->GetPackageWidget(), SIGNAL(SelectionControlChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)), this, SLOT(OnSelectionControlChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)));
    connect(widgets->GetPackageWidget(), SIGNAL(SelectionRootControlChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)), this, SLOT(OnSelectionRootControlChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)));
    
    connect(this, SIGNAL(controlSelectedInEditor(ControlNode*)), widgets->GetPackageWidget(), SLOT(OnControlSelectedInEditor(ControlNode*)));
    connect(this, SIGNAL(allControlsDeselectedInEditor()), widgets->GetPackageWidget(), SLOT(OnAllControlsDeselectedInEditor()));
    

    undoStack->setActive(true);
}

void Document::DisconnectFromWidgets(DocumentWidgets *widgets)
{
    undoStack->setActive(false);

    widgets->SetWidgetsEnabled(false);
    
    widgets->GetPackageWidget()->SetDocument(nullptr);
    widgets->GetPreviewWidget()->SetDocument(nullptr);
    widgets->GetPropertiesWidget()->SetContext(nullptr);
    widgets->GetLibraryWidget()->SetDocument(nullptr);
    
    disconnect(widgets->GetPackageWidget(), SIGNAL(SelectionRootControlChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)), this, SLOT(OnSelectionRootControlChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)));
    disconnect(widgets->GetPackageWidget(), SIGNAL(SelectionControlChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)), this, SLOT(OnSelectionControlChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)));
    
    disconnect(this, SIGNAL(controlSelectedInEditor(ControlNode*)), widgets->GetPackageWidget(), SLOT(OnControlSelectedInEditor(ControlNode*)));
    disconnect(this, SIGNAL(allControlsDeselectedInEditor()), widgets->GetPackageWidget(), SLOT(OnAllControlsDeselectedInEditor()));
}

bool Document::IsModified() const
{
    return !undoStack->isClean();
}

void Document::ClearModified()
{
    undoStack->setClean();
}

const DAVA::FilePath &Document::PackageFilePath() const
{
    return package->GetPackageRef()->GetPath();
}

QtModelPackageCommandExecutor *Document::GetCommandExecutor() const
{
    return commandExecutor;
}

void Document::OnSelectionRootControlChanged(const QList<ControlNode*> &activatedRootControls, const QList<ControlNode*> &deactivatedRootControls)
{
    activeRootControls.clear();
    foreach(ControlNode *control, activatedRootControls)
    {
        activeRootControls.push_back(control);
    }

    emit activeRootControlsChanged(activeRootControls, deactivatedRootControls);
}

void Document::OnSelectionControlChanged(const QList<ControlNode*> &activatedControls, const QList<ControlNode*> &deactivatedControls)
{
    selectedControls.clear();
    
    foreach(ControlNode *control, activatedControls)
    {
        selectedControls.push_back(control);
    }

    emit controlsSelectionChanged(activatedControls, deactivatedControls);
}

void Document::OnControlSelectedInEditor(ControlNode *activatedControl)
{
    emit controlSelectedInEditor(activatedControl);
}

void Document::OnAllControlDeselectedInEditor()
{
    emit allControlsDeselectedInEditor();
}
