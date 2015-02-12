#include "Document.h"
#include <DAVAEngine.h>
#include <QLineEdit>
#include <QAction>
#include <QItemSelection>

#include "UI/PackageView/UIPackageModel.h"
#include "UI/PackageView/UIFilteredPackageModel.h"
#include "UI/DavaGLWidget.h"
#include "UI/GraphicView/GraphicsViewContext.h"
#include "UI/LibraryView/LibraryModel.h"

#include "UIControls/PackageHierarchy/PackageNode.h"
#include "UIControls/PackageHierarchy/PackageControlsNode.h"
#include "UIControls/PackageHierarchy/ControlNode.h"
#include "UIControls/PackageHierarchy/PackageRef.h"

#include "PackageContext.h"
#include "PropertiesContext.h"
#include "LibraryContext.h"

#include "QtModelPackageCommandExecutor.h"

using namespace DAVA;

Document::Document(Project *_project, PackageNode *_package, QObject *parent)
    : QObject(parent)
    , project(_project)
    , package(SafeRetain(_package))
    , graphicsContext(nullptr)
    , commandExecutor(nullptr)
{
    undoStack = new QUndoStack(this);

    packageContext = new PackageContext(this);
    propertiesContext = new PropertiesContext(this);
    libraryContext = new LibraryContext(this);
    graphicsContext = new GraphicsViewContext();
    
    connect(this, SIGNAL(activeRootControlsChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)), graphicsContext, SLOT(OnActiveRootControlsChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)));
    connect(this, SIGNAL(controlsSelectionChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)), graphicsContext, SLOT(OnSelectedControlsChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)));

    connect(graphicsContext, SIGNAL(ControlNodeSelected(ControlNode*)), this, SLOT(OnControlSelectedInEditor(ControlNode*)));
    connect(graphicsContext, SIGNAL(AllControlsDeselected()), this, SLOT(OnAllControlDeselectedInEditor()));

    
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
    SafeDelete(graphicsContext);
    
    SafeRelease(package);
    
    SafeRelease(commandExecutor);
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
