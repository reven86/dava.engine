//
//  UIPackageDocument.cpp
//  UIEditor
//
//  Created by Alexey Strokachuk on 9/17/14.
//
//

#include "PackageDocument.h"
#include <DAVAEngine.h>
#include <QLineEdit>
#include <QAction>

#include "UI/PackageView/UIPackageModel.h"
#include "UI/PackageView/UIFilteredPackageModel.h"
#include "UI/DavaGLWidget.h"
#include "UI/GraphicView/GraphicsViewContext.h"
#include "UI/PropertiesView/PropertiesViewContext.h"
#include "UI/LibraryView/LibraryModel.h"

#include "UIControls/PackageHierarchy/PackageNode.h"
#include "UIControls/PackageHierarchy/PackageControlsNode.h"
#include "UIControls/PackageHierarchy/ControlNode.h"

using namespace DAVA;

PackageDocument::PackageDocument(PackageNode *_package, QObject *parent)
: QObject(parent)
, package(SafeRetain(_package))
, graphicsContext(NULL)
{
    undoStack = new QUndoStack(this);

    treeContext.model = new UIPackageModel(this);
    treeContext.proxyModel = new UIFilteredPackageModel(this);
    treeContext.proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    treeContext.proxyModel->setSourceModel(treeContext.model);

    graphicsContext = new GraphicsViewContext();
    connect(this, SIGNAL(activeRootControlsChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)), graphicsContext, SLOT(OnActiveRootControlsChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)));

    propertiesContext = new PropertiesViewContext(this);
    
    libraryContext.model = new LibraryModel(package, this);

    PackageControlsNode *controlsNode = package->GetPackageControlsNode();
    for (int32 index = 0; index < controlsNode->GetCount(); ++index)
        activeRootControls.push_back(controlsNode->Get(index));

    if (!activeRootControls.empty())
        emit activeRootControlsChanged(activeRootControls, QList<ControlNode*>());
}

PackageDocument::~PackageDocument()
{
    treeContext.proxyModel->setSourceModel(NULL);

    SafeDelete(treeContext.model);
    SafeDelete(treeContext.proxyModel);
    disconnect(this, SIGNAL(activeRootControlsChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)), graphicsContext, SLOT(OnActiveRootControlsChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)));
    SafeDelete(graphicsContext);
    SafeDelete(propertiesContext);
    SafeDelete(libraryContext.model);
    
    SafeRelease(package);
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
    return package->GetPackage()->GetFilePath();
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
