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
#include "Model/ControlProperties/LocalizedTextValueProperty.h"
#include "Model/ControlProperties/FontValueProperty.h"

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
    connect(GetEditorFontSystem(), &EditorFontSystem::UpdateFontPreset, this, &Document::UpdateFonts);
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

void Document::UpdateLanguage()
{
    PackageControlsNode *controlsNode = package->GetPackageControlsNode();
    for (int32 index = 0; index < controlsNode->GetCount(); ++index)
        UpdateLanguageRecursively(controlsNode->Get(index));
}

void Document::UpdateFonts()
{
    PackageControlsNode *controlsNode = package->GetPackageControlsNode();
    for (int32 index = 0; index < controlsNode->GetCount(); ++index)
        UpdateFontsRecursively(controlsNode->Get(index));
}

void Document::UpdateLanguageRecursively(ControlNode *node)
{
    PropertiesRoot *propertiesRoot = node->GetPropertiesRoot();
    int propertiesCount = propertiesRoot->GetCount();
    for (int index = 0; index < propertiesCount; ++index)
    {
        PropertiesSection *section = dynamic_cast<PropertiesSection*>(propertiesRoot->GetProperty(index));
        if (nullptr != section)
        {
            int sectionCount = section->GetCount();
            for (int prop = 0; prop < sectionCount; ++prop)
            {
                ValueProperty *valueProperty = dynamic_cast<ValueProperty*>(section->GetProperty(prop));
                if (nullptr != valueProperty && strcmp(valueProperty->GetMember()->Name(), "text") == 0)
                {
                    LocalizedTextValueProperty *textValueProperty = dynamic_cast<LocalizedTextValueProperty*>(valueProperty);
                    if (nullptr != textValueProperty)
                    {
                        textValueProperty->RefreshLocalizedValue();
                    }
                }
            }
        }
    }
    for (int index = 0; index < node->GetCount(); ++index)
    {
        UpdateLanguageRecursively(node->Get(index));
    }
}

void Document::UpdateFontsRecursively(ControlNode *node)
{
    PropertiesRoot *propertiesRoot = node->GetPropertiesRoot();
    int propertiesCount = propertiesRoot->GetCount();
    for (int index = 0; index < propertiesCount; ++index)
    {
        PropertiesSection *section = dynamic_cast<PropertiesSection*>(propertiesRoot->GetProperty(index));
        if (nullptr != section)
        {
            int sectionCount = section->GetCount();
            for (int prop = 0; prop < sectionCount; ++prop)
            {
                ValueProperty *valueProperty = dynamic_cast<ValueProperty*>(section->GetProperty(prop));
                if (nullptr != valueProperty && strcmp(valueProperty->GetMember()->Name(), "font") == 0)
                {
                    FontValueProperty *fontValueProperty = dynamic_cast<FontValueProperty*>(valueProperty);
                    if (nullptr != fontValueProperty)
                    {
                        fontValueProperty->RefreshFontValue();
                    }
                }
            }
        }
    }
    for (int index = 0; index < node->GetCount(); ++index)
    {
        UpdateFontsRecursively(node->Get(index));
    }
}
