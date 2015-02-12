#include "PackageDocument.h"
#include <DAVAEngine.h>
#include <QLineEdit>
#include <QAction>
#include <QItemSelection>

#include "UI/PackageView/UIPackageModel.h"
#include "UI/PackageView/UIFilteredPackageModel.h"
#include "UI/DavaGLWidget.h"
#include "UI/GraphicView/GraphicsViewContext.h"
#include "UI/PropertiesView/PropertiesViewContext.h"
#include "UI/LibraryView/LibraryModel.h"

#include "UIControls/PackageHierarchy/PackageNode.h"
#include "UIControls/PackageHierarchy/PackageControlsNode.h"
#include "UIControls/PackageHierarchy/ControlNode.h"
#include "UIControls/PackageHierarchy/PackageRef.h"

#include "QtModelPackageCommandExecutor.h"

using namespace DAVA;

PackageDocument::PackageDocument(Project *_project, PackageNode *_package, QObject *parent)
: QObject(parent)
, project(_project)
, package(SafeRetain(_package))
, graphicsContext(nullptr)
, commandExecutor(nullptr)
{
    undoStack = new QUndoStack(this);

    treeContext.model = new UIPackageModel(this);
    treeContext.proxyModel = new UIFilteredPackageModel(this);
    treeContext.proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    treeContext.proxyModel->setSourceModel(treeContext.model);

    treeContext.currentSelection = new QItemSelection();

    graphicsContext = new GraphicsViewContext();
    connect(this, SIGNAL(activeRootControlsChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)), graphicsContext, SLOT(OnActiveRootControlsChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)));
    connect(this, SIGNAL(controlsSelectionChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)), graphicsContext, SLOT(OnSelectedControlsChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)));

    connect(graphicsContext, SIGNAL(ControlNodeSelected(ControlNode*)), this, SLOT(OnControlSelectedInEditor(ControlNode*)));
    connect(graphicsContext, SIGNAL(AllControlsDeselected()), this, SLOT(OnAllControlDeselectedInEditor()));

    propertiesContext = new PropertiesViewContext(this);
    
    libraryContext.model = new LibraryModel(package, this);

    PackageControlsNode *controlsNode = package->GetPackageControlsNode();
    for (int32 index = 0; index < controlsNode->GetCount(); ++index)
        activeRootControls.push_back(controlsNode->Get(index));

    if (!activeRootControls.empty())
        emit activeRootControlsChanged(activeRootControls, QList<ControlNode*>());
    
    commandExecutor = new QtModelPackageCommandExecutor(this);
}

PackageDocument::~PackageDocument()
{
    treeContext.proxyModel->setSourceModel(NULL);

    SafeDelete(treeContext.model);
    SafeDelete(treeContext.proxyModel);
    SafeDelete(treeContext.currentSelection);

    disconnect(this, SIGNAL(activeRootControlsChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)), graphicsContext, SLOT(OnActiveRootControlsChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)));
    disconnect(this, SIGNAL(selectedRootControlsChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)), graphicsContext, SLOT(OnSelectedControlsChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)));

    disconnect(graphicsContext, SIGNAL(ControlNodeSelected(ControlNode*)), this, SLOT(OnControlSelectedInEditor(ControlNode*)));
    disconnect(graphicsContext, SIGNAL(AllControlsDeselected()), this, SLOT(OnAllControlDeselectedInEditor()));

    SafeDelete(graphicsContext);
    SafeDelete(propertiesContext);
    SafeDelete(libraryContext.model);
    
    SafeRelease(package);
    
    SafeRelease(commandExecutor);
}

bool PackageDocument::IsModified() const
{
    return !undoStack->isClean();
}

void PackageDocument::ClearModified()
{
    undoStack->setClean();
}

const DAVA::FilePath &PackageDocument::PackageFilePath() const
{
    return package->GetPackageRef()->GetPath();
}

QtModelPackageCommandExecutor *PackageDocument::GetCommandExecutor() const
{
    return commandExecutor;
}

void PackageDocument::OnSelectionRootControlChanged(const QList<ControlNode*> &activatedRootControls, const QList<ControlNode*> &deactivatedRootControls)
{
    activeRootControls.clear();
    foreach(ControlNode *control, activatedRootControls)
    {
        activeRootControls.push_back(control);
    }

    emit activeRootControlsChanged(activeRootControls, deactivatedRootControls);
}

void PackageDocument::OnSelectionControlChanged(const QList<ControlNode*> &activatedControls, const QList<ControlNode*> &deactivatedControls)
{
    selectedControls.clear();
    
    foreach(ControlNode *control, activatedControls)
    {
        selectedControls.push_back(control);
    }

    emit controlsSelectionChanged(activatedControls, deactivatedControls);
}

void PackageDocument::OnControlSelectedInEditor(ControlNode *activatedControl)
{
    emit controlSelectedInEditor(activatedControl);
}

void PackageDocument::OnAllControlDeselectedInEditor()
{
    emit allControlsDeselectedInEditor();
}
