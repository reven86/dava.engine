#include "Document.h"
#include <QLineEdit>
#include <QAction>
#include <QItemSelection>
#include "UI/Preview/EditScreen.h"

#include "UI/Package/PackageModel.h"

#include "UI/Library/LibraryModel.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageRef.h"

#include "Model/ControlProperties/PropertiesRoot.h"
#include "Model/ControlProperties/PropertiesSection.h"
#include "Model/ControlProperties/ValueProperty.h"

#include "Ui/WidgetContext.h"

#include "Ui/QtModelPackageCommandExecutor.h"

using namespace DAVA;

Document::Document(PackageNode *_package, QObject *parent)
    : QObject(parent)
    , package(SafeRetain(_package))
    , dataContext(new WidgetContext(this))
    , commandExecutor(new QtModelPackageCommandExecutor(this))
    , undoStack(new QUndoStack(this))
{
    InitWidgetContexts();
    connect(dataContext, &WidgetContext::DataChanged, this, &Document::ContextDataChanged);
}

void Document::InitWidgetContexts()
{
    dataContext->SetData(QVariant::fromValue(new LibraryModel(package, this)), "libraryModel");

    dataContext->SetData(QVariant::fromValue(new PackageModel(package, commandExecutor, this)), "packageModel");

    dataContext->SetData(false, "controlDeselected");
    dataContext->SetData(false, "controlsDeselected");

    QList<ControlNode*> activeRootControls;
    PackageControlsNode *controlsNode = package->GetPackageControlsNode();
    for (int32 index = 0; index < controlsNode->GetCount(); ++index)
        activeRootControls.push_back(controlsNode->Get(index));

    dataContext->SetData(QVariant::fromValue(activeRootControls), "activeRootControls");
}

Document::~Document()
{   
    SafeRelease(package);
    
    SafeRelease(commandExecutor);
}

const DAVA::FilePath &Document::GetPackageFilePath() const
{
    return package->GetPackageRef()->GetPath();
}


PropertiesModel *Document::GetPropertiesModel() const
{
    return reinterpret_cast<PropertiesModel*>(dataContext->GetData("propertiesModel").value<QAbstractItemModel*>()); //TODO this is ugly
}

PackageModel* Document::GetPackageModel() const
{
    return dataContext->GetData("packageModel").value<PackageModel*>();
}

void Document::UpdateLanguage()
{
    QList<ControlNode*> activeRootControls;
    PackageControlsNode *controlsNode = package->GetPackageControlsNode();
    for (int32 index = 0; index < controlsNode->GetCount(); ++index)
        UpdateLanguageRecursively(controlsNode->Get(index));
}

void Document::UpdateLanguageRecursively(ControlNode *node)
{
    PropertiesRoot *propertiesRoot = node->GetPropertiesRoot();
    int propertiesCount = propertiesRoot->GetCount();
    for (int index = 0; index < propertiesCount; ++index)
    {
        PropertiesSection *section = dynamic_cast<PropertiesSection*>(propertiesRoot->GetProperty(index));
        int sectionCount = section->GetCount();
        for (int prop = 0; prop < sectionCount; ++prop)
        {
            ValueProperty *valueProperty = dynamic_cast<ValueProperty*>(section->GetProperty(prop));
            if (!strcmp(valueProperty->GetMember()->Name(), "text"))
            {
                valueProperty->SetValue(valueProperty->GetValue());
            }
        }
    }
    for (int index = 0; index < node->GetCount(); ++index)
    {
        UpdateLanguageRecursively(node->Get(index));
    }
}
