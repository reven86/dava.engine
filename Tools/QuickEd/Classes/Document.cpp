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

#include "Ui/Package/PackageWidget.h"
#include "Ui/Preview/PreviewWidget.h"
#include "Ui/Properties/PropertiesWidget.h"
#include "Ui/Library/LibraryWidget.h"

#include "Ui/PackageContext.h"
#include "Ui/PropertiesContext.h"
#include "Ui/PreviewContext.h"

#include "Ui/QtModelPackageCommandExecutor.h"

using namespace DAVA;

Document::Document(Project *_project, PackageNode *_package, QObject *parent)
    : QObject(parent)
    , project(_project)
    , package(SafeRetain(_package))
    , packageContext(nullptr)
    , propertiesContext(nullptr)
    , previewContext(nullptr)
    , libraryModel(nullptr)
    , commandExecutor(nullptr)
    , undoStack(new QUndoStack(this))
{
    commandExecutor = new QtModelPackageCommandExecutor(this);

    packageContext = new PackageContext(this);
    propertiesContext = new PropertiesContext(this);
    previewContext = new PreviewContext(this);
    
    connect(this, SIGNAL(activeRootControlsChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)), previewContext, SLOT(OnActiveRootControlsChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)));

    connect(previewContext, SIGNAL(ControlNodeSelected(ControlNode*)), this, SLOT(OnControlSelectedInEditor(ControlNode*)));
    connect(previewContext, SIGNAL(AllControlsDeselected()), this, SLOT(OnAllControlDeselectedInEditor()));

    
    PackageControlsNode *controlsNode = package->GetPackageControlsNode();
    for (int32 index = 0; index < controlsNode->GetCount(); ++index)
        activeRootControls.push_back(controlsNode->Get(index));

    if (!activeRootControls.empty())
        emit activeRootControlsChanged(activeRootControls, QList<ControlNode*>());
}

Document::~Document()
{
    SafeDelete(packageContext);
    SafeDelete(propertiesContext);
    SafeDelete(previewContext);
    
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


PackageModel* Document::GetPackageModel() const
{
    return &packageContext->model;
}

PackageContext* Document::GetPackageContext() const
{
    return packageContext;
}

PropertiesModel *Document::GetPropertiesModel() const
{
    return propertiesContext->GetModel();
}

QAbstractItemModel *Document::GetLibraryModel() const
{
    return libraryModel;
}

PreviewContext *Document::GetPreviewContext() const
{
    return previewContext;
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
    for (ControlNode *control : deactivatedControls)
    {
        auto it = std::find(selectedControls.begin(), selectedControls.end(), control);
        if (it != selectedControls.end())
            selectedControls.erase(it);
    }
    
    for (ControlNode *control : activatedControls)
        selectedControls.push_back(control);
    
    
    propertiesContext->SetActiveNode(activatedControls.empty() ? nullptr : activatedControls.first());
    previewContext->OnSelectedControlsChanged(activatedControls, deactivatedControls);
}

void Document::OnControlSelectedInEditor(ControlNode *activatedControl)
{
    emit controlSelectedInEditor(activatedControl);
}

void Document::OnAllControlDeselectedInEditor()
{
    emit allControlsDeselectedInEditor();
}
