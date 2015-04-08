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

#include "SharedData.h"

#include "Ui/QtModelPackageCommandExecutor.h"

using namespace DAVA;

Document::Document(PackageNode *_package, QObject *parent)
    : QObject(parent)
    , package(SafeRetain(_package))
    , sharedData(new SharedData(this))
    , commandExecutor(new QtModelPackageCommandExecutor(this))
    , undoStack(new QUndoStack(this))
{
    InitSharedData();
    connect(sharedData, &SharedData::DataChanged, this, &Document::SharedDataChanged);
}

void Document::InitSharedData()
{
    sharedData->SetData("controlDeselected", false);
    sharedData->SetData("controlsDeselected", false);

    QList<ControlNode*> activeRootControls;
    PackageControlsNode *controlsNode = package->GetPackageControlsNode();
    for (int32 index = 0; index < controlsNode->GetCount(); ++index)
        activeRootControls.push_back(controlsNode->Get(index));

    sharedData->SetData("activeRootControls", QVariant::fromValue(activeRootControls));
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
    return reinterpret_cast<PropertiesModel*>(sharedData->GetData("propertiesModel").value<QAbstractItemModel*>()); //TODO this is ugly
}

PackageModel* Document::GetPackageModel() const
{
    return sharedData->GetData("packageModel").value<PackageModel*>();
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
