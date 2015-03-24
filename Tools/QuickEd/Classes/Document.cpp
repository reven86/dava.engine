#include "Document.h"
#include <DAVAEngine.h>
#include <QLineEdit>
#include <QAction>
#include <QItemSelection>

#include "UI/Package/PackageModel.h"
#include "UI/Library/LibraryModel.h"
#include "UI/Properties/PropertiesModel.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageRef.h"

#include "Ui/Package/PackageWidget.h"
#include "Ui/Preview/PreviewWidget.h"
#include "Ui/Properties/PropertiesWidget.h"
#include "Ui/Library/LibraryWidget.h"

#include "Ui/WidgetContext.h"

#include "Ui/QtModelPackageCommandExecutor.h"

using namespace DAVA;

Document::Document(PackageNode *_package, QObject *parent)
    : QObject(parent)
    , package(SafeRetain(_package))
    , libraryContext(new WidgetContext(this))
    , propertiesContext(new WidgetContext(this))
    , packageContext(new WidgetContext(this))
    , previewContext(new WidgetContext(this))
    , commandExecutor(new QtModelPackageCommandExecutor(this))
    , undoStack(new QUndoStack(this))
{
    InitWidgetContexts();
    //!! TODO: implement this
    /*connect(this, SIGNAL(activeRootControlsChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)), previewContext, SLOT(OnActiveRootControlsChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)));

    connect(previewContext, SIGNAL(ControlNodeSelected(ControlNode*)), this, SLOT(OnControlSelectedInEditor(ControlNode*)));
    connect(previewContext, SIGNAL(AllControlsDeselected()), this, SLOT(OnAllControlDeselectedInEditor()));*/

    
    PackageControlsNode *controlsNode = package->GetPackageControlsNode();
    for (int32 index = 0; index < controlsNode->GetCount(); ++index)
        activeRootControls.push_back(controlsNode->Get(index));

    if (!activeRootControls.empty())
        emit activeRootControlsChanged(activeRootControls, QList<ControlNode*>());
    ConnectWidgetContexts();
}

void Document::InitWidgetContexts()
{
    libraryContext->SetData(new LibraryModel(package, this), "model");

    PackageModel *packageModel = new PackageModel(package, commandExecutor, this);
    packageContext->SetData(packageModel, "model");
}

void Document::ConnectWidgetContexts() const
{
    connect(libraryContext, &WidgetContext::DataChanged, this, &Document::LibraryDataChanged, Qt::DirectConnection);
    connect(packageContext, &WidgetContext::DataChanged, this, &Document::PackageDataChanged, Qt::DirectConnection);
    connect(propertiesContext, &WidgetContext::DataChanged, this, &Document::PropertiesDataChanged, Qt::DirectConnection);
    connect(previewContext, &WidgetContext::DataChanged, this, &Document::PreviewDataChanged, Qt::DirectConnection);
}

Document::~Document()
{
    SafeDelete(packageContext);
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

WidgetContext *Document::GetLibraryContext() const
{
    return libraryContext;
}

WidgetContext *Document::GetPropertiesContext() const
{
    return propertiesContext;
}

PropertiesModel *Document::GetPropertiesModel() const
{
    return propertiesContext->GetData<PropertiesModel*>("model");
}

PackageModel* Document::GetPackageModel() const
{
    return packageContext->GetData<PackageModel*>("model");
}

WidgetContext* Document::GetPackageContext() const
{
    return packageContext;
}

WidgetContext *Document::GetPreviewContext() const
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


    QAbstractItemModel* model = activatedControls.empty() ? nullptr : new PropertiesModel(activatedControls.first(), this);
    propertiesContext->SetData(model, "model");

    //!!TODO: restore it previewContext->OnSelectedControlsChanged(activatedControls, deactivatedControls);

}

void Document::OnControlSelectedInEditor(ControlNode *activatedControl)
{
    emit controlSelectedInEditor(activatedControl);
}

void Document::OnAllControlDeselectedInEditor()
{
    emit allControlsDeselectedInEditor();
}