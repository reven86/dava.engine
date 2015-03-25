#include "Document.h"
#include <QLineEdit>
#include <QAction>
#include <QItemSelection>

#include "UI/Package/PackageModel.h"

#include "UI/Library/LibraryModel.h"

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

    QList<ControlNode*> activeRootControls;
    PackageControlsNode *controlsNode = package->GetPackageControlsNode();
    for (int32 index = 0; index < controlsNode->GetCount(); ++index)
        activeRootControls.push_back(controlsNode->Get(index));

    previewContext->SetData(QVariant::fromValue(activeRootControls), "activeRootControls");
    ConnectWidgetContexts();
}

void Document::InitWidgetContexts()
{
    libraryContext->SetData(QVariant::fromValue(new LibraryModel(package, this)), "model");

    packageContext->SetData(QVariant::fromValue(new PackageModel(package, commandExecutor, this)), "model");

    DAVA::UIControl *view = new DAVA::UIControl();
    DAVA::UIControl *canvas = new DAVA::UIControl();
    view->AddControl(canvas);
    previewContext->SetData(QVariant::fromValue(view), "view");
    previewContext->SetData(QVariant::fromValue(canvas), "canvas");
    previewContext->SetData(false, "controlDeselected");
    packageContext->SetData(false, "controlsDeselected");
    
}

void Document::ConnectWidgetContexts() const
{
    connect(packageContext, &WidgetContext::DataChanged, this, &Document::OnPackageContextDataChanged);
    connect(previewContext, &WidgetContext::DataChanged, this, &Document::OnPreviewContextDataChanged);

    connect(libraryContext, &WidgetContext::DataChanged, this, &Document::LibraryDataChanged);
    connect(packageContext, &WidgetContext::DataChanged, this, &Document::PackageDataChanged);
    connect(propertiesContext, &WidgetContext::DataChanged, this, &Document::PropertiesDataChanged);
    connect(previewContext, &WidgetContext::DataChanged, this, &Document::PreviewDataChanged);
}

Document::~Document()
{   
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
    return propertiesContext->GetData("model").value<PropertiesModel*>();
}

PackageModel* Document::GetPackageModel() const
{
    return packageContext->GetData("model").value<PackageModel*>();
}

WidgetContext* Document::GetPackageContext() const
{
    return packageContext;
}

WidgetContext *Document::GetPreviewContext() const
{
    return previewContext;
}

void Document::OnPreviewContextDataChanged(const QByteArray &role)
{
    if (role == "selectedNode")
    {
        packageContext->SetData(previewContext->GetData(role), role);
    }
    else if (role == "controlDeselected")
    {
        packageContext->SetData(!packageContext->GetData(role).toBool(), role);
    }
}

void Document::OnPackageContextDataChanged(const QByteArray &role)
{
    if (role == "activatedControls")
    {
        QVariant selected = packageContext->GetData("activatedControls");
        QList<ControlNode*> &activatedControls = selected.value<QList<ControlNode*> >();
        QAbstractItemModel* model = activatedControls.empty() ? nullptr : new PropertiesModel(activatedControls.first(), this);
        propertiesContext->SetData(QVariant::fromValue(model), "model");

        previewContext->SetData(selected, "activatedControls");
    }
    else if (role == "deactivatedControls")
    {
        previewContext->SetData(packageContext->GetData("deactivatedControls"), "deactivatedControls");
    }
    else if (role == "activeRootControls")
    {
        previewContext->SetData(packageContext->GetData("activeRootControls"), "activeRootControls");
    }
}