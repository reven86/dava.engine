#include "Document.h"
#include <QLineEdit>
#include <QAction>
#include <QItemSelection>

#include "Base/BaseTypes.h"
#include "UI/Preview/EditScreen.h"

#include "UI/Package/PackageModel.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageRef.h"

#include "Model/ControlProperties/PropertiesRoot.h"
#include "Model/ControlProperties/PropertiesSection.h"
#include "Model/ControlProperties/ValueProperty.h"

#include "SharedData.h"
#include "Ui/QtModelPackageCommandExecutor.h"

#include "EditorCore.h"

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
    connect(GetEditorFontSystem(), &EditorFontSystem::BeginUpdatePreset, this, &Document::BeginUpdatePreset);
    connect(GetEditorFontSystem(), &EditorFontSystem::UpdateFontPreset, this, &Document::UpdateFontPreset);
}

void Document::InitSharedData()
{
    sharedData->SetData("controlDeselected", false);
    sharedData->SetData("controlsDeselected", false);

    QList<ControlNode*> rootControls;
    PackageControlsNode *controlsNode = package->GetPackageControlsNode();
    for (int32 index = 0; index < controlsNode->GetCount(); ++index)
        rootControls.push_back(controlsNode->Get(index));

    sharedData->SetData("activeRootControls", QVariant::fromValue(rootControls));
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

void Document::BeginUpdatePreset()
{
    BeginUpdateProperty("font");
}

void Document::UpdateFontPreset()
{
    UpdateProperty("font");
}

void Document::BeginUpdateProperty(const DAVA::String& property)
{
    PackageControlsNode *controlsNode = package->GetPackageControlsNode();
    for (int32 index = 0; index < controlsNode->GetCount(); ++index)
    {
        BeginUpdatePropertyRecuresively(controlsNode->Get(index), property);
    }
}

void Document::UpdateProperty(const String &property)
{
    PackageControlsNode *controlsNode = package->GetPackageControlsNode();
    for (int32 index = 0; index < controlsNode->GetCount(); ++index)
    {
        UpdatePropertyRecursively(controlsNode->Get(index), property);
    }
}

void Document::BeginUpdatePropertyRecuresively(ControlNode* node, const DAVA::String& property)
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
            if (property == valueProperty->GetMember()->Name())
            {
                valueProperty->SetDefaultValue(valueProperty->GetValue());
            }
        }
    }
    for (int index = 0; index < node->GetCount(); ++index)
    {
        BeginUpdatePropertyRecuresively(node->Get(index), property);
    }
}

void Document::UpdatePropertyRecursively(ControlNode* node, const DAVA::String &property)
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
            if (property == valueProperty->GetMember()->Name())
            {
                valueProperty->SetValue(valueProperty->GetDefaultValue()); //TODO: here must be ResetValue, but it not work, because clear "replaced" flag
            }
        }
    }
    for (int index = 0; index < node->GetCount(); ++index)
    {
        UpdatePropertyRecursively(node->Get(index), property);
    }
}
