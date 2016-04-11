/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QAction>
#include <QVariant>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QDebug>

#include "MaterialEditor.h"
#include "ui_materialeditor.h"

#include "MaterialModel.h"
#include "MaterialFilterModel.h"

#include "Main/mainwindow.h"
#include "Main/QtUtils.h"
#include "QualitySwitcher/QualitySwitcher.h"
#include "Project/ProjectManager.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataIntrospection.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataInspMember.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataInspDynamic.h"
#include "Tools/QtPropertyEditor/QtPropertyDataValidator/TexturePathValidator.h"
#include "Qt/Settings/SettingsManager.h"
#include "Commands2/MaterialGlobalCommand.h"
#include "Commands2/MaterialRemoveTexture.h"
#include "Commands2/MaterialConfigCommands.h"

#include "Scene3D/Systems/QualitySettingsSystem.h"

#include "CommandLine/TextureDescriptor/TextureDescriptorUtils.h"
#include "Tools/PathDescriptor/PathDescriptor.h"


#include "QtTools/FileDialog/FileDialog.h"

#include "QtTools/Updaters/LazyUpdater.h"
#include "QtTools/WidgetHelpers/SharedIcon.h"

#include "Base/Introspection.h"

namespace UIName
{
const DAVA::FastName Template("Template");

const DAVA::FastName Name("Name");
const DAVA::FastName Group("Group");

const DAVA::FastName Base("Base");
const DAVA::FastName Flags("Flags");
const DAVA::FastName Illumination("Illumination");
const DAVA::FastName Properties("Properties");
const DAVA::FastName Textures("Textures");
}

namespace NMaterialSectionName
{
const DAVA::FastName LocalFlags("localFlags");
const DAVA::FastName LocalProperties("localProperties");
const DAVA::FastName LocalTextures("localTextures");
}

namespace MaterialEditorLocal
{
const char* CURRENT_TAB_CHANGED_UPDATE = "CurrentTabChangedUpdate";
class PropertyLock
{
public:
    PropertyLock(QObject* propertyHolder_, const char* propertyName_)
        : propertyHolder(propertyHolder_)
        , propertyName(propertyName_)
    {
        if (!propertyHolder->property(propertyName).toBool())
        {
            locked = true;
            propertyHolder->setProperty(propertyName, true);
        }
    }

    ~PropertyLock()
    {
        if (locked)
        {
            propertyHolder->setProperty(propertyName, false);
        }
    }

    bool IsLocked() const
    {
        return locked;
    }

private:
    QObject* propertyHolder = nullptr;
    const char* propertyName = nullptr;
    bool locked = false;
};
}

class MaterialEditor::ConfigNameValidator : public QValidator
{
public:
    ConfigNameValidator(QObject* parent)
        : QValidator(parent)
    {
    }

    State validate(QString& input, int& pos) const override
    {
        DVASSERT(material != nullptr);
        if (input.isEmpty())
            return Intermediate;

        DAVA::uint32 index = material->FindConfigByName(DAVA::FastName(input.toStdString()));
        return (index < material->GetConfigCount()) ? Intermediate : Acceptable;
    }

    void SetCurrentMaterial(DAVA::NMaterial* material_)
    {
        material = material_;
    }

private:
    DAVA::NMaterial* material = nullptr;
};

MaterialEditor::MaterialEditor(QWidget* parent /* = 0 */)
    : QDialog(parent)
    , ui(new Ui::MaterialEditor)
    , templatesFilterModel(nullptr)
    , lastCheckState(CHECKED_ALL)
    , validator(new ConfigNameValidator(this))
{
    DAVA::Function<void()> fn(this, &MaterialEditor::RefreshMaterialProperties);
    materialPropertiesUpdater = new LazyUpdater(fn, this);

    ui->setupUi(this);
    setWindowFlags(WINDOWFLAG_ON_TOP_OF_APPLICATION);

    ui->tabbar->setNameValidator(validator);
    ui->tabbar->setUsesScrollButtons(true);
    ui->tabbar->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(ui->tabbar, &EditableTabBar::tabNameChanged, this, &MaterialEditor::onTabNameChanged);
    QObject::connect(ui->tabbar, &EditableTabBar::currentChanged, this, &MaterialEditor::onCurrentConfigChanged);
    QObject::connect(ui->tabbar, &EditableTabBar::tabCloseRequested, this, &MaterialEditor::onTabRemove);
    QObject::connect(ui->tabbar, &EditableTabBar::customContextMenuRequested, this, &MaterialEditor::onTabContextMenuRequested);

    ui->materialTree->setDragEnabled(true);
    ui->materialTree->setAcceptDrops(true);
    ui->materialTree->setDragDropMode(QAbstractItemView::DragDrop);

    ui->materialProperty->SetEditTracking(true);
    ui->materialProperty->setContextMenuPolicy(Qt::CustomContextMenu);

    baseRoot = AddSection(UIName::Base);
    flagsRoot = AddSection(UIName::Flags);
    illuminationRoot = AddSection(UIName::Illumination);
    propertiesRoot = AddSection(UIName::Properties);
    texturesRoot = AddSection(UIName::Textures);

    // global scene manager signals
    QObject::connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2*)), this, SLOT(sceneActivated(SceneEditor2*)));
    QObject::connect(SceneSignals::Instance(), SIGNAL(Deactivated(SceneEditor2*)), this, SLOT(sceneDeactivated(SceneEditor2*)));
    QObject::connect(SceneSignals::Instance(), SIGNAL(CommandExecuted(SceneEditor2*, const Command2*, bool)), this, SLOT(commandExecuted(SceneEditor2*, const Command2*, bool)));
    QObject::connect(SceneSignals::Instance(), SIGNAL(SelectionChanged(SceneEditor2*, const SelectableGroup*, const SelectableGroup*)), this, SLOT(autoExpand()));

    // material tree
    QObject::connect(ui->materialTree->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(materialSelected(const QItemSelection&, const QItemSelection&)));
    QObject::connect(ui->materialTree, SIGNAL(Updated()), this, SLOT(autoExpand()));
    QObject::connect(ui->materialTree, SIGNAL(ContextMenuPrepare(QMenu*)), this, SLOT(onContextMenuPrepare(QMenu*)));

    // material properties
    QObject::connect(ui->materialProperty, SIGNAL(PropertyEdited(const QModelIndex&)), this, SLOT(OnPropertyEdited(const QModelIndex&)));
    QObject::connect(ui->materialProperty, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnMaterialPropertyEditorContextMenuRequest(const QPoint&)));
    QObject::connect(ui->templateBox, SIGNAL(activated(int)), this, SLOT(OnTemplateChanged(int)));
    QObject::connect(ui->templateButton, SIGNAL(clicked()), this, SLOT(OnTemplateButton()));
    QObject::connect(ui->actionAddGlobalMaterial, SIGNAL(triggered(bool)), this, SLOT(OnMaterialAddGlobal(bool)));
    QObject::connect(ui->actionRemoveGlobalMaterial, SIGNAL(triggered(bool)), this, SLOT(OnMaterialRemoveGlobal(bool)));
    QObject::connect(ui->actionSaveMaterialPreset, SIGNAL(triggered(bool)), this, SLOT(OnMaterialSave(bool)));
    QObject::connect(ui->actionLoadMaterialPreset, SIGNAL(triggered(bool)), this, SLOT(OnMaterialLoad(bool)));

    posSaver.Attach(this);
    new QtPosSaver(ui->splitter);
    treeStateHelper = new PropertyEditorStateHelper(ui->materialProperty, (QtPropertyModel*)ui->materialProperty->model());

    DAVA::VariantType v1 = posSaver.LoadValue("splitPosProperties");
    DAVA::VariantType v2 = posSaver.LoadValue("splitPosPreview");
    if (v1.GetType() == DAVA::VariantType::TYPE_INT32)
        ui->materialProperty->header()->resizeSection(0, v1.AsInt32());
    if (v2.GetType() == DAVA::VariantType::TYPE_INT32)
        ui->materialProperty->header()->resizeSection(1, v2.AsInt32());

    DAVA::VariantType savePath = posSaver.LoadValue("lastSavePath");
    DAVA::VariantType loadState = posSaver.LoadValue("lastLoadState");
    if (savePath.GetType() == DAVA::VariantType::TYPE_FILEPATH)
        lastSavePath = savePath.AsFilePath();
    if (loadState.GetType() == DAVA::VariantType::TYPE_UINT32)
        lastCheckState = loadState.AsUInt32();

    expandMap[MaterialFilteringModel::SHOW_ALL] = false;
    expandMap[MaterialFilteringModel::SHOW_ONLY_INSTANCES] = true;
    expandMap[MaterialFilteringModel::SHOW_INSTANCES_AND_MATERIALS] = true;

    initActions();
}

MaterialEditor::~MaterialEditor()
{
    delete treeStateHelper;

    DAVA::VariantType v1(ui->materialProperty->header()->sectionSize(0));
    DAVA::VariantType v2(ui->materialProperty->header()->sectionSize(1));
    posSaver.SaveValue("splitPosProperties", v1);
    posSaver.SaveValue("splitPosPreview", v2);
    posSaver.SaveValue("lastSavePath", DAVA::VariantType(lastSavePath));
    posSaver.SaveValue("lastLoadState", DAVA::VariantType(lastCheckState));
}

QtPropertyData* MaterialEditor::AddSection(const DAVA::FastName& sectionName)
{
    QtPropertyData* section = new QtPropertyData(sectionName);
    ui->materialProperty->AppendProperty(std::unique_ptr<QtPropertyData>(section));
    ui->materialProperty->ApplyStyle(section, QtPropertyEditor::HEADER_STYLE);
    return section;
}

void MaterialEditor::initActions()
{
    ui->actionShowAll->setData(MaterialFilteringModel::SHOW_ALL);
    ui->actionInstances->setData(MaterialFilteringModel::SHOW_ONLY_INSTANCES);
    ui->actionMaterialsInstances->setData(MaterialFilteringModel::SHOW_INSTANCES_AND_MATERIALS);

    const int filterType = MaterialFilteringModel::SHOW_ALL;
    foreach (QAction* action, ui->filterType->actions())
    {
        QObject::connect(action, SIGNAL(triggered()), SLOT(onFilterChanged()));

        if (action->data().toInt() == filterType)
        {
            action->activate(QAction::Trigger);
        }
    }

    QObject::connect(ui->actionAutoExpand, SIGNAL(triggered(bool)), SLOT(onCurrentExpandModeChange(bool)));
}

void MaterialEditor::initTemplates()
{
    if (nullptr == templatesFilterModel)
    {
        QStandardItemModel* templatesModel = new QStandardItemModel(this);

        const QVector<ProjectManager::AvailableMaterialTemplate>* templates = ProjectManager::Instance()->GetAvailableMaterialTemplates();
        QStandardItem* emptyItem = new QStandardItem(QString());
        templatesModel->appendRow(emptyItem);

        for (int i = 0; i < templates->size(); ++i)
        {
            QStandardItem* item = new QStandardItem();
            item->setText(templates->at(i).name);
            item->setData(templates->at(i).path, Qt::UserRole);
            templatesModel->appendRow(item);
        }

        templatesFilterModel = new MaterialTemplateModel(this);
        templatesFilterModel->setSourceModel(templatesModel);

        ui->templateBox->setModel(templatesFilterModel);
    }
}

void MaterialEditor::setTemplatePlaceholder(const QString& text)
{
    QAbstractItemModel* model = ui->templateBox->model();
    const QModelIndex index = model->index(0, 0);
    model->setData(index, text, Qt::DisplayRole);
}

void MaterialEditor::autoExpand()
{
    QAction* action = ui->filterType->checkedAction();
    if (nullptr == action)
        return;

    const int filterType = action->data().toInt();

    if (expandMap[filterType])
        ui->materialTree->expandAll();
}

void MaterialEditor::onFilterChanged()
{
    QAction* action = ui->filterType->checkedAction();
    if (nullptr == action)
        return;

    const int filterType = action->data().toInt();
    ui->materialTree->setFilterType(filterType);

    ui->actionAutoExpand->setChecked(expandMap[filterType]);
    onCurrentExpandModeChange(expandMap[filterType]);
}

void MaterialEditor::SelectMaterial(DAVA::NMaterial* material)
{
    ui->materialTree->SelectMaterial(material);
}

void MaterialEditor::SelectEntities(DAVA::NMaterial* material)
{
    QList<DAVA::NMaterial*> materials;
    materials << material;
    ui->materialTree->SelectEntities(materials);
}

void MaterialEditor::SetCurMaterial(const QList<DAVA::NMaterial*>& materials)
{
    int curScrollPos = ui->materialProperty->verticalScrollBar()->value();

    curMaterials = materials;
    treeStateHelper->SaveTreeViewState(false);
    UpdateTabs();

    FillBase();
    FillDynamic(flagsRoot, NMaterialSectionName::LocalFlags);
    FillIllumination();
    FillDynamic(propertiesRoot, NMaterialSectionName::LocalProperties);
    FillDynamic(texturesRoot, NMaterialSectionName::LocalTextures);
    FillInvalidTextures();
    FillTemplates(materials);
    FinishCreation();

    // Restore back the tree view state from the shared storage.
    if (!treeStateHelper->IsTreeStateStorageEmpty())
    {
        treeStateHelper->RestoreTreeViewState();
    }
    else
    {
        // Expand the root elements as default value.
        ui->materialProperty->expandToDepth(0);
    }

    // check if there is global material and enable appropriate actions
    SceneEditor2* sceneEditor = QtMainWindow::Instance()->GetCurrentScene();
    if (nullptr != sceneEditor)
    {
        bool isGlobalMaterialPresent = (nullptr != sceneEditor->GetGlobalMaterial());
        ui->actionAddGlobalMaterial->setEnabled(!isGlobalMaterialPresent);
        ui->actionRemoveGlobalMaterial->setEnabled(isGlobalMaterialPresent);
    }
    else
    {
        ui->actionAddGlobalMaterial->setEnabled(false);
        ui->actionRemoveGlobalMaterial->setEnabled(false);
    }

    ui->materialProperty->verticalScrollBar()->setValue(curScrollPos);
}

void MaterialEditor::sceneActivated(SceneEditor2* scene)
{
    if (isVisible())
    {
        SetCurMaterial(QList<DAVA::NMaterial*>());
        ui->materialTree->SetScene(scene);
        autoExpand();
    }
}

void MaterialEditor::sceneDeactivated(SceneEditor2* scene)
{
    ui->materialTree->SetScene(nullptr);
    SetCurMaterial(QList<DAVA::NMaterial*>());
}

void MaterialEditor::materialSelected(const QItemSelection& selected, const QItemSelection& deselected)
{
    QList<DAVA::NMaterial*> materials;
    QItemSelectionModel* selection = ui->materialTree->selectionModel();
    const QModelIndexList selectedRows = selection->selectedRows();

    foreach (const QModelIndex& index, selectedRows)
    {
        if (index.column() == 0)
        {
            DAVA::NMaterial* material = ui->materialTree->GetMaterial(index);
            if (material)
                materials << material;
        }
    }

    SetCurMaterial(materials);
}

void MaterialEditor::commandExecuted(SceneEditor2* scene, const Command2* command, bool redo)
{
    if (scene != QtMainWindow::Instance()->GetCurrentScene())
    {
        return;
    }

    int curScrollPos = ui->materialProperty->verticalScrollBar()->value();
    SCOPE_EXIT
    {
        ui->materialProperty->verticalScrollBar()->setValue(curScrollPos);
    };

    if (command->MatchCommandID(CMDID_MATERIAL_GLOBAL_SET))
    {
        sceneActivated(scene);
        materialPropertiesUpdater->Update();
    }
    if (command->MatchCommandID(CMDID_MATERIAL_REMOVE_TEXTURE))
    {
        materialPropertiesUpdater->Update();
    }

    if (command->MatchCommandIDs({ CMDID_MATERIAL_CHANGE_CURRENT_CONFIG, CMDID_MATERIAL_CREATE_CONFIG, CMDID_MATERIAL_REMOVE_CONFIG }))
    {
        RefreshMaterialProperties();
    }

    if (command->MatchCommandIDs({ CMDID_INSP_MEMBER_MODIFY, CMDID_INSP_DYNAMIC_MODIFY }))
    {
        auto ProcessSingleCommand = [this](const Command2* command, bool redo)
        {
            if (command->MatchCommandID(CMDID_INSP_MEMBER_MODIFY))
            {
                const InspMemberModifyCommand* inspCommand = static_cast<const InspMemberModifyCommand*>(command);
                const DAVA::String memberName(inspCommand->member->Name().c_str());
                if (memberName == DAVA::NMaterialSerializationKey::QualityGroup || memberName == DAVA::NMaterialSerializationKey::FXName)
                {
                    for (auto& m : curMaterials)
                    {
                        m->InvalidateRenderVariants();
                    }
                    materialPropertiesUpdater->Update();
                }
                if (memberName == "configName")
                {
                    materialPropertiesUpdater->Update();
                }
            }
            else if (command->MatchCommandID(CMDID_INSP_DYNAMIC_MODIFY))
            {
                const InspDynamicModifyCommand* inspCommand = static_cast<const InspDynamicModifyCommand*>(command);
                // if material flag was changed we should rebuild list of all properties because their set can be changed
                if (inspCommand->dynamicInfo->GetMember()->Name() == NMaterialSectionName::LocalFlags)
                {
                    if (inspCommand->key == DAVA::NMaterialFlagName::FLAG_ILLUMINATION_USED)
                    {
                        DAVA::NMaterial* material = static_cast<DAVA::NMaterial*>(inspCommand->ddata.object);
                        if (material->HasLocalFlag(DAVA::NMaterialFlagName::FLAG_ILLUMINATION_USED) && material->GetLocalFlagValue(DAVA::NMaterialFlagName::FLAG_ILLUMINATION_USED) == 1)
                        {
                            AddMaterialFlagIfNeed(material, DAVA::NMaterialFlagName::FLAG_ILLUMINATION_SHADOW_CASTER);
                            AddMaterialFlagIfNeed(material, DAVA::NMaterialFlagName::FLAG_ILLUMINATION_SHADOW_RECEIVER);
                            material->InvalidateRenderVariants();
                        }
                    }

                    materialPropertiesUpdater->Update();
                }
                else
                {
                    UpdateAllAddRemoveButtons(ui->materialProperty->GetRootProperty());
                }
            }
        };

        if (command->GetId() == CMDID_BATCH)
        {
            const CommandBatch* batch = static_cast<const CommandBatch*>(command);
            const DAVA::uint32 count = batch->Size();
            for (DAVA::uint32 i = 0; i < count; ++i)
            {
                ProcessSingleCommand(batch->GetCommand(i), redo);
            }
        }
        else
        {
            ProcessSingleCommand(command, redo);
        }
    }
}

void MaterialEditor::AddMaterialFlagIfNeed(DAVA::NMaterial* material, const DAVA::FastName& flagName)
{
    bool hasFlag = false;
    DAVA::NMaterial* parent = material;
    while (parent)
    {
        if (parent->HasLocalFlag(flagName))
        {
            hasFlag = true;
            break;
        }

        parent = parent->GetParent();
    }

    if (!hasFlag)
    {
        material->AddFlag(flagName, 0);
    }
}

bool MaterialEditor::HasMaterialProperty(DAVA::NMaterial* material, const DAVA::FastName& paramName)
{
    while (material != nullptr)
    {
        if (material->HasLocalProperty(paramName))
        {
            return true;
        }

        material = material->GetParent();
    }

    return false;
}

void MaterialEditor::OnQualityChanged()
{
    RefreshMaterialProperties();
}

void MaterialEditor::onCurrentExpandModeChange(bool mode)
{
    QAction* action = ui->filterType->checkedAction();
    if (nullptr == action)
        return;

    const int filterType = action->data().toInt();
    expandMap[filterType] = mode;

    if (mode)
    {
        ui->materialTree->expandAll();
    }
    else
    {
        ui->materialTree->collapseAll();
    }
}

void MaterialEditor::showEvent(QShowEvent* event)
{
    FillTemplates(QList<DAVA::NMaterial*>());
    sceneActivated(QtMainWindow::Instance()->GetCurrentScene());
}

void MaterialEditor::FillBase()
{
    baseRoot->ChildRemoveAll();

    auto scene = QtMainWindow::Instance()->GetCurrentScene();
    auto globalMaterial = (nullptr == scene) ? nullptr : scene->GetGlobalMaterial();

    foreach (DAVA::NMaterial* material, curMaterials)
    {
        const DAVA::InspInfo* info = material->GetTypeInfo();

        // fill material name
        const DAVA::InspMember* nameMember = info->Member(DAVA::FastName("materialName"));
        if (nullptr != nameMember)
        {
            baseRoot->MergeChild(std::unique_ptr<QtPropertyData>(new QtPropertyDataInspMember(UIName::Name, material, nameMember)));
        }

        // fill material group, only for material type
        const DAVA::InspMember* groupMember = info->Member(DAVA::FastName("qualityGroup"));
        if ((nullptr != groupMember) && (globalMaterial != material))
        {
            QtPropertyDataInspMember* group = new QtPropertyDataInspMember(UIName::Group, material, groupMember);
            baseRoot->MergeChild(std::unique_ptr<QtPropertyData>(group));

            // Add unknown value:
            group->AddAllowedValue(DAVA::VariantType(DAVA::String()), "Unknown");

            // fill allowed values for material group
            for (size_t i = 0; i < DAVA::QualitySettingsSystem::Instance()->GetMaterialQualityGroupCount(); ++i)
            {
                DAVA::FastName groupName = DAVA::QualitySettingsSystem::Instance()->GetMaterialQualityGroupName(i);
                group->AddAllowedValue(DAVA::VariantType(groupName), groupName.c_str());
            }
        }
    }
}

void MaterialEditor::FillDynamic(QtPropertyData* root, const DAVA::FastName& dynamicName)
{
    DAVA::NMaterial* globalMaterial = nullptr;
    root->ChildRemoveAll();

    SceneEditor2* sceneEditor = QtMainWindow::Instance()->GetCurrentScene();
    if (nullptr != sceneEditor)
    {
        globalMaterial = sceneEditor->GetGlobalMaterial();
    }

    foreach (DAVA::NMaterial* material, curMaterials)
    {
        if (material == globalMaterial && dynamicName == NMaterialSectionName::LocalTextures)
        {
            continue;
        }

        const DAVA::InspInfo* info = material->GetTypeInfo();
        const DAVA::InspMember* materialMember = info->Member(dynamicName);

        // fill material flags
        if ((nullptr != materialMember) && (nullptr != materialMember->Dynamic()))
        {
            DAVA::InspInfoDynamic* dynamicInfo = materialMember->Dynamic()->GetDynamicInfo();
            FillDynamicMembers(root, dynamicInfo, material, material == globalMaterial);
        }
    }
}

namespace MELocal
{
struct InvalidTexturesCollector
{
    DAVA::Set<DAVA::FastName> validTextures;
    DAVA::Map<DAVA::FastName, DAVA::Vector<DAVA::FilePath>> invalidTextures;

    void CollectValidTextures(QtPropertyData* data)
    {
        DVASSERT(data != nullptr);
        validTextures.insert(data->GetName());
        for (int i = 0; i < data->ChildCount(); ++i)
        {
            CollectValidTextures(data->ChildGet(i));
        }
    }

    void CollectInvalidTextures(DAVA::NMaterial* material)
    {
        while (material != nullptr)
        {
            using TTexturesMap = DAVA::HashMap<DAVA::FastName, DAVA::MaterialTextureInfo*>;
            using TTextureItem = TTexturesMap::HashMapItem;

            const TTexturesMap& localTextures = material->GetLocalTextures();
            for (const TTextureItem& lc : localTextures)
            {
                if (validTextures.count(lc.first) == 0)
                {
                    invalidTextures[lc.first].push_back(lc.second->path);
                }
            }

            material = material->GetParent();
        }
    }
};
}

void MaterialEditor::FillInvalidTextures()
{
    MELocal::InvalidTexturesCollector collector;
    collector.CollectValidTextures(texturesRoot);
    for (int i = 0; i < curMaterials.size(); ++i)
    {
        collector.CollectInvalidTextures(curMaterials[i]);
    }

    for (const auto& t : collector.invalidTextures)
    {
        DVASSERT(!t.second.empty());
        for (size_t i = 0; i < t.second.size(); ++i)
        {
            QVariant qValue(QString::fromStdString(t.second[i].GetAbsolutePathname()));
            std::unique_ptr<QtPropertyData> textureSlot(new QtPropertyData(t.first, qValue));

            QtPropertyToolButton* addRemoveButton = textureSlot->AddButton();
            addRemoveButton->setObjectName("dynamicAddRemoveButton");
            addRemoveButton->setIconSize(QSize(14, 14));
            addRemoveButton->setIcon(SharedIcon(":/QtIcons/cminus.png"));
            addRemoveButton->setToolTip("Remove property");
            QObject::connect(addRemoveButton, SIGNAL(clicked()), this, SLOT(removeInvalidTexture()));

            textureSlot->SetEnabled(false);
            textureSlot->SetBackground(QBrush(QColor(255, 0, 0, 25)));
            texturesRoot->MergeChild(std::move(textureSlot));
        }
    }
}

void MaterialEditor::FillIllumination()
{
    illuminationRoot->ChildRemoveAll();

    for (auto& material : curMaterials)
    {
        if (material->GetEffectiveFlagValue(DAVA::NMaterialFlagName::FLAG_ILLUMINATION_USED) != 1)
        {
            continue;
        }

        const DAVA::InspInfo* info = material->GetTypeInfo();
        { // Add flags
            const DAVA::InspMember* flagsMember = info->Member(NMaterialSectionName::LocalFlags);
            if (flagsMember != nullptr && (nullptr != flagsMember->Dynamic()))
            {
                DAVA::InspInfoDynamic* dynamicInfo = flagsMember->Dynamic()->GetDynamicInfo();
                FillDynamicMember(illuminationRoot, dynamicInfo, material, DAVA::NMaterialFlagName::FLAG_ILLUMINATION_SHADOW_CASTER);
                FillDynamicMember(illuminationRoot, dynamicInfo, material, DAVA::NMaterialFlagName::FLAG_ILLUMINATION_SHADOW_RECEIVER);
            }
        }

        { // add properties
            const DAVA::InspMember* propertiesMember = info->Member(NMaterialSectionName::LocalProperties);
            if (propertiesMember != nullptr && (nullptr != propertiesMember->Dynamic()))
            {
                DAVA::InspInfoDynamic* dynamicInfo = propertiesMember->Dynamic()->GetDynamicInfo();
                FillDynamicMember(illuminationRoot, dynamicInfo, material, DAVA::NMaterialParamName::PARAM_LIGHTMAP_SIZE);
            }
        }
    }
}

void MaterialEditor::FillDynamicMember(QtPropertyData* root, DAVA::InspInfoDynamic* dynamic, DAVA::NMaterial* material, const DAVA::FastName& memberName)
{
    DAVA::InspInfoDynamic::DynamicData ddata = dynamic->Prepare(material, false);
    FillDynamicMemberInternal(root, dynamic, ddata, memberName);
}

void MaterialEditor::FillDynamicMembers(QtPropertyData* root, DAVA::InspInfoDynamic* dynamic, DAVA::NMaterial* material, bool isGlobal)
{
    DAVA::InspInfoDynamic::DynamicData ddata = dynamic->Prepare(material, isGlobal);
    DAVA::Vector<DAVA::FastName> membersList = dynamic->MembersList(ddata);

    // enumerate dynamic members and add them
    for (size_t i = 0; i < membersList.size(); ++i)
    {
        const DAVA::FastName& name = membersList[i];

        if ((root == flagsRoot) && (name == DAVA::NMaterialFlagName::FLAG_ILLUMINATION_SHADOW_CASTER || name == DAVA::NMaterialFlagName::FLAG_ILLUMINATION_SHADOW_RECEIVER))
        { // it will be shown in section illumination
            continue;
        }

        if ((root == propertiesRoot) && (name == DAVA::NMaterialParamName::PARAM_LIGHTMAP_SIZE))
        { // it will be shown in section illumination
            continue;
        }

        FillDynamicMemberInternal(root, dynamic, ddata, name);
    }
}

void MaterialEditor::FillDynamicMemberInternal(QtPropertyData* root, DAVA::InspInfoDynamic* dynamic, DAVA::InspInfoDynamic::DynamicData& ddata, const DAVA::FastName& memberName)
{
    QtPropertyDataInspDynamic* dynamicData = new QtPropertyDataInspDynamic(memberName, dynamic, ddata);

    // for all textures we should add texture path validator
    if (root == texturesRoot)
    {
        ApplyTextureValidator(dynamicData);
    }

    // update buttons state and enabled/disable state of this data
    UpdateAddRemoveButtonState(dynamicData);

    // merge created dynamic data into specified root
    root->MergeChild(std::unique_ptr<QtPropertyData>(dynamicData));
}

void MaterialEditor::ApplyTextureValidator(QtPropertyDataInspDynamic* data)
{
    QString defaultPath = ProjectManager::Instance()->GetProjectPath().GetAbsolutePathname().c_str();
    DAVA::FilePath dataSourcePath = ProjectManager::Instance()->GetDataSourcePath();

    // calculate appropriate default path
    if (DAVA::FileSystem::Instance()->Exists(dataSourcePath))
    {
        defaultPath = dataSourcePath.GetAbsolutePathname().c_str();
    }

    SceneEditor2* editor = QtMainWindow::Instance()->GetCurrentScene();
    if ((nullptr != editor) && DAVA::FileSystem::Instance()->Exists(editor->GetScenePath()))
    {
        DAVA::String scenePath = editor->GetScenePath().GetDirectory().GetAbsolutePathname();
        if (DAVA::String::npos != scenePath.find(dataSourcePath.GetAbsolutePathname()))
        {
            defaultPath = scenePath.c_str();
        }
    }

    // create validator
    data->SetDefaultOpenDialogPath(defaultPath);

    data->SetOpenDialogFilter(PathDescriptor::GetPathDescriptor(PathDescriptor::PATH_TEXTURE).fileFilter);
    QStringList path;
    path.append(dataSourcePath.GetAbsolutePathname().c_str());
    data->SetValidator(new TexturePathValidator(path));
}

void MaterialEditor::UpdateAddRemoveButtonState(QtPropertyDataInspDynamic* data)
{
    // don't create/update buttons for global material
    SceneEditor2* curScene = QtMainWindow::Instance()->GetCurrentScene();
    if (nullptr != curScene /*&& data->object != curScene->GetGlobalMaterial()*/)
    {
        // extract member flags from dynamic info
        int memberFlags = data->dynamicInfo->MemberFlags(data->ddata, data->name);

        QtPropertyToolButton* addRemoveButton = nullptr;

        // check if there is already our button
        for (int i = 0; i < data->GetButtonsCount(); ++i)
        {
            QtPropertyToolButton* btn = data->GetButton(i);
            if (btn->objectName() == "dynamicAddRemoveButton")
            {
                addRemoveButton = btn;
                break;
            }
        }

        // there is no our add/remove button - so create is
        if (nullptr == addRemoveButton)
        {
            addRemoveButton = data->AddButton();
            addRemoveButton->setObjectName("dynamicAddRemoveButton");
            addRemoveButton->setIconSize(QSize(14, 14));
            QObject::connect(addRemoveButton, SIGNAL(clicked()), this, SLOT(OnAddRemoveButton()));
        }

        QBrush bgColor;
        bool editEnabled = false;

        // self property - should be remove button
        if (memberFlags & DAVA::I_EDIT)
        {
            editEnabled = true;
            addRemoveButton->setIcon(SharedIcon(":/QtIcons/cminus.png"));
            addRemoveButton->setToolTip("Remove property");

            // isn't set in parent or shader
            if (!(memberFlags & DAVA::I_VIEW))
            {
                bgColor = QBrush(QColor(255, 0, 0, 25));
            }
        }
        // inherited from parent property - should be add button
        else
        {
            editEnabled = false;
            bgColor = QBrush(QColor(0, 0, 0, 25));
            addRemoveButton->setIcon(SharedIcon(":/QtIcons/cplus.png"));
            addRemoveButton->setToolTip("Add property");
        }

        // don't allow editing for members that are inherited
        data->SetEnabled(editEnabled);
        data->SetBackground(bgColor);
        for (int m = 0; m < data->ChildCount(); ++m)
        {
            data->ChildGet(m)->SetEnabled(editEnabled);
            data->ChildGet(m)->SetBackground(bgColor);
        }
    }
}

void MaterialEditor::UpdateAllAddRemoveButtons(QtPropertyData* root)
{
    QtPropertyDataInspDynamic* dynamicData = dynamic_cast<QtPropertyDataInspDynamic*>(root);
    if (nullptr != dynamicData)
    {
        UpdateAddRemoveButtonState(dynamicData);
    }

    for (int i = 0; i < root->ChildCount(); ++i)
    {
        UpdateAllAddRemoveButtons(root->ChildGet(i));
    }
}

void MaterialEditor::FillTemplates(const QList<DAVA::NMaterial*>& materials)
{
    initTemplates();

    if (1 == materials.size() && materials[0])
    {
        DAVA::NMaterial* material = materials[0];

        //Read material params
        auto scene = QtMainWindow::Instance()->GetCurrentScene();
        DAVA::NMaterial* globalMaterial = (nullptr == scene) ? nullptr : scene->GetGlobalMaterial();

        const bool isGlobalMaterial = (material == globalMaterial);
        if (isGlobalMaterial)
        { // reset state
            ui->templateBox->setCurrentIndex(-1);
            ui->templateBox->setEnabled(false);
            ui->templateButton->setEnabled(false);
            ui->templateButton->setIcon(SharedIcon(":/QtIcons/cplus.png"));
        }
        else
        {
            bool isAssignableFx = true;
            { //set fx name to fx template box
                int rowToSelect = -1;
                const DAVA::FastName fxName = material->GetEffectiveFXName();
                if (fxName.IsValid())
                {
                    QAbstractItemModel* model = ui->templateBox->model();
                    const int n = model->rowCount();
                    for (int i = 0; i < n; i++)
                    {
                        const QModelIndex index = model->index(i, 0);
                        if (index.data(Qt::UserRole).toString() == fxName.c_str())
                        {
                            rowToSelect = i;
                            break;
                        }
                    }

                    if (-1 == rowToSelect)
                    {
                        setTemplatePlaceholder(QString("NON-ASSIGNABLE: %1").arg(fxName.c_str()));
                        rowToSelect = 0;
                        isAssignableFx = false;
                    }
                }

                ui->templateBox->setCurrentIndex(rowToSelect);
            }

            { //update button state

                const bool hasLocalFxName = material->HasLocalFXName();

                DAVA::NMaterial* parentMaterial = material->GetParent();
                bool hasParentFx = false;
                if (parentMaterial != nullptr)
                {
                    hasParentFx = parentMaterial->HasLocalFXName();
                }

                if (hasLocalFxName)
                {
                    ui->templateButton->setIcon(SharedIcon(":/QtIcons/cminus.png"));
                }
                else
                {
                    ui->templateButton->setIcon(SharedIcon(":/QtIcons/cplus.png"));
                }

                if (parentMaterial == nullptr || parentMaterial == globalMaterial || isAssignableFx == false)
                {
                    ui->templateButton->setEnabled(false);
                }
                else
                {
                    ui->templateButton->setEnabled(true);
                }

                ui->templateBox->setEnabled(hasLocalFxName && isAssignableFx);
            }
        }
    }
    else
    { // reset state
        ui->templateBox->setCurrentIndex(-1);
        ui->templateBox->setEnabled(false);
        ui->templateButton->setEnabled(false);
        ui->templateButton->setIcon(SharedIcon(":/QtIcons/cplus.png"));
    }
}

void MaterialEditor::OnTemplateChanged(int index)
{
    if (curMaterials.size() == 1 && index > 0)
    {
        DAVA::NMaterial* material = curMaterials.at(0);
        QString newTemplatePath = GetTemplatePath(index);
        if (!newTemplatePath.isEmpty())
        {
            const DAVA::InspMember* templateMember = material->GetTypeInfo()->Member(DAVA::FastName("fxName"));

            if (nullptr != templateMember)
            {
                QtMainWindow::Instance()->GetCurrentScene()->Exec(Command2::Create<InspMemberModifyCommand>(templateMember, material,
                                                                                                            DAVA::VariantType(DAVA::FastName(newTemplatePath.toStdString().c_str()))));
            }
        }
    }

    RefreshMaterialProperties();
}

void MaterialEditor::OnTemplateButton()
{
    if (1 == curMaterials.size())
    {
        DAVA::NMaterial* material = curMaterials[0];
        const DAVA::InspMember* templateMember = material->GetTypeInfo()->Member(DAVA::FastName("fxName"));

        if (nullptr != templateMember)
        {
            SceneEditor2* scene = QtMainWindow::Instance()->GetCurrentScene();
            DVASSERT(scene != nullptr);
            if (material->HasLocalFXName())
            {
                // has local fxname, so button shoud remove it (by setting empty value)
                scene->Exec(Command2::Create<InspMemberModifyCommand>(templateMember, material, DAVA::VariantType(DAVA::FastName())));
            }
            else
            {
                // no local fxname, so button should add it
                scene->Exec(Command2::Create<InspMemberModifyCommand>(templateMember, material, DAVA::VariantType(material->GetEffectiveFXName())));
            }

            RefreshMaterialProperties();
        }
    }
}

QString MaterialEditor::GetTemplatePath(int index) const
{
    return ui->templateBox->itemData(index, Qt::UserRole).toString();
}

void MaterialEditor::OnAddRemoveButton()
{
    QtPropertyToolButton* btn = dynamic_cast<QtPropertyToolButton*>(QObject::sender());
    if (nullptr != btn)
    {
        QtPropertyDataInspDynamic* data = (QtPropertyDataInspDynamic*)btn->GetPropertyData();
        if (nullptr != data)
        {
            int memberFlags = data->dynamicInfo->MemberFlags(data->ddata, data->name);

            // pressed remove button
            if (memberFlags & DAVA::I_EDIT)
            {
                data->SetValue(QVariant(), QtPropertyData::VALUE_SOURCE_CHANGED);
            }
            // pressed add button
            else
            {
                data->SetValue(data->GetValue(), QtPropertyData::VALUE_EDITED);
            }

            data->EmitDataChanged(QtPropertyData::VALUE_EDITED);
            UpdateAddRemoveButtonState(data);
        }
    }
}

void MaterialEditor::OnPropertyEdited(const QModelIndex& index)
{
    QtPropertyEditor* editor = dynamic_cast<QtPropertyEditor*>(QObject::sender());
    SceneEditor2* curScene = QtMainWindow::Instance()->GetCurrentScene();
    if (editor != nullptr && curScene != nullptr)
    {
        QtPropertyData* propData = editor->GetProperty(index);
        if (nullptr != propData)
        {
            curScene->BeginBatch("Property multiedit", propData->GetMergedItemCount() + 1);

            auto commandsAccumulateFn = [&curScene](QtPropertyData* item) {
                Command2::Pointer command = item->CreateLastCommand();
                if (command)
                {
                    curScene->Exec(std::move(command));
                }
                return true;
            };

            propData->ForeachMergedItem(commandsAccumulateFn);
            commandsAccumulateFn(propData);

            curScene->EndBatch();
        }
    }
}

void MaterialEditor::OnMaterialAddGlobal(bool checked)
{
    SceneEditor2* curScene = QtMainWindow::Instance()->GetCurrentScene();
    if (nullptr != curScene)
    {
        DAVA::ScopedPtr<DAVA::NMaterial> global(new DAVA::NMaterial());

        global->SetMaterialName(DAVA::FastName("Scene_Global_Material"));
        curScene->Exec(Command2::Create<MaterialGlobalSetCommand>(curScene, global));

        sceneActivated(curScene);
        SelectMaterial(curScene->GetGlobalMaterial());
    }
}

void MaterialEditor::OnMaterialRemoveGlobal(bool checked)
{
    SceneEditor2* curScene = QtMainWindow::Instance()->GetCurrentScene();
    if (nullptr != curScene)
    {
        curScene->Exec(Command2::Create<MaterialGlobalSetCommand>(curScene, nullptr));
        sceneActivated(curScene);
    }
}

void MaterialEditor::onContextMenuPrepare(QMenu* menu)
{
    if (curMaterials.size() > 0)
    {
        ui->actionSaveMaterialPreset->setEnabled(curMaterials.size() == 1);
        menu->addSeparator();
        menu->addAction(ui->actionLoadMaterialPreset);
        menu->addAction(ui->actionSaveMaterialPreset);
    }
}

void MaterialEditor::OnMaterialPropertyEditorContextMenuRequest(const QPoint& pos)
{
    SceneEditor2* sceneEditor = QtMainWindow::Instance()->GetCurrentScene();
    if (nullptr != sceneEditor && curMaterials.size() == 1)
    {
        DAVA::NMaterial* globalMaterial = sceneEditor->GetGlobalMaterial();
        DAVA::NMaterial* material = curMaterials[0];

        QModelIndex index = ui->materialProperty->indexAt(pos);

        if (globalMaterial != material && index.column() == 0 && (nullptr != globalMaterial))
        {
            QtPropertyData* data = ui->materialProperty->GetProperty(index);
            if (nullptr != data && data->Parent() == propertiesRoot)
            {
                QtPropertyDataInspDynamic* dynamicData = (QtPropertyDataInspDynamic*)data;
                if (nullptr != dynamicData)
                {
                    const DAVA::FastName& propertyName = dynamicData->name;
                    bool hasProperty = material->HasLocalProperty(propertyName);
                    if (hasProperty)
                    {
                        QMenu menu;
                        menu.addAction("Add to Global Material");
                        QAction* resultAction = menu.exec(ui->materialProperty->viewport()->mapToGlobal(pos));

                        if (nullptr != resultAction)
                        {
                            globalMaterial->SetPropertyValue(propertyName, material->GetLocalPropValue(propertyName));
                            sceneEditor->SetChanged(true);
                        }
                    }
                }
            }
        }
    }
}

void MaterialEditor::OnMaterialSave(bool checked)
{
    if (curMaterials.size() == 1)
    {
        QString outputFile = FileDialog::getSaveFileName(this, "Save Material Preset", lastSavePath.GetAbsolutePathname().c_str(),
                                                         "Material Preset (*.mpreset)");

        SceneEditor2* curScene = QtMainWindow::Instance()->GetCurrentScene();

        if (!outputFile.isEmpty() && (nullptr != curScene))
        {
            lastSavePath = outputFile.toLatin1().data();

            DAVA::SerializationContext materialContext;
            materialContext.SetScene(curScene);
            materialContext.SetScenePath(ProjectManager::Instance()->GetProjectPath());
            materialContext.SetVersion(DAVA::VersionInfo::Instance()->GetCurrentVersion().version);

            DAVA::ScopedPtr<DAVA::KeyedArchive> materialArchive(new DAVA::KeyedArchive());
            StoreMaterialToPreset(curMaterials.front(), materialArchive, &materialContext);

            DAVA::ScopedPtr<DAVA::KeyedArchive> presetArchive(new DAVA::KeyedArchive());
            presetArchive->SetUInt32("serializationContextVersion", materialContext.GetVersion());
            presetArchive->SetArchive("content", materialArchive);
            presetArchive->SaveToYamlFile(lastSavePath);
        }
    }
    else
    {
        QMessageBox::warning(this, "Material properties save", "It is allowed to save only single material");
    }
}

void MaterialEditor::OnMaterialLoad(bool checked)
{
    if (curMaterials.size() > 0)
    {
        QString inputFile = FileDialog::getOpenFileName(this, "Load Material Preset",
                                                        lastSavePath.GetAbsolutePathname().c_str(), "Material Preset (*.mpreset)");

        SceneEditor2* curScene = QtMainWindow::Instance()->GetCurrentScene();

        if (!inputFile.isEmpty() && (nullptr != curScene))
        {
            lastSavePath = inputFile.toLatin1().data();

            DAVA::ScopedPtr<DAVA::KeyedArchive> presetArchive(new DAVA::KeyedArchive());
            presetArchive->LoadFromYamlFile(lastSavePath);

            // not checking version right now
            // version info is reserved for future use
            if (presetArchive->IsKeyExists("content"))
            {
                DAVA::KeyedArchive* materialArchive = presetArchive->GetArchive("content");
                DAVA::uint32 userChoiseWhatToLoad = ExecMaterialLoadingDialog(lastCheckState, inputFile);
                DAVA::SerializationContext materialContext;
                materialContext.SetScene(curScene);
                materialContext.SetScenePath(ProjectManager::Instance()->GetProjectPath());
                materialContext.SetVersion(DAVA::VersionInfo::Instance()->GetCurrentVersion().version);
                UpdateMaterialFromPresetWithOptions(curMaterials.front(), materialArchive, &materialContext, userChoiseWhatToLoad);
                materialContext.ResolveMaterialBindings();
                curScene->SetChanged(true);
            }
            else
            {
                QMessageBox::warning(this, "Material preset is not supported",
                                     "Material preset you are trying to open is either old or invalid.");
            }
        }
    }

    RefreshMaterialProperties();
}

void MaterialEditor::ClearDynamicMembers(DAVA::NMaterial* material, const DAVA::InspMemberDynamic* dynamicMember)
{
    if (nullptr != dynamicMember)
    {
        DAVA::InspInfoDynamic* dynamicInfo = dynamicMember->GetDynamicInfo();
        DAVA::InspInfoDynamic::DynamicData ddata = dynamicInfo->Prepare(material);
        DAVA::Vector<DAVA::FastName> membersList = dynamicInfo->MembersList(ddata); // this function can be slow
        for (size_t i = 0; i < membersList.size(); ++i)
        {
            dynamicInfo->MemberValueSet(ddata, membersList[i], DAVA::VariantType());
        }
    }
}

DAVA::uint32 MaterialEditor::ExecMaterialLoadingDialog(DAVA::uint32 initialState, const QString& inputFile)
{
    DAVA::uint32 ret = 0;

    QDialog* dlg = new QDialog(this);
    QVBoxLayout* dlgLayout = new QVBoxLayout();
    dlgLayout->setMargin(10);
    dlgLayout->setSpacing(15);

    dlg->setWindowTitle("Loading material preset...");
    dlg->setWindowFlags(Qt::Tool);
    dlg->setLayout(dlgLayout);

    QLineEdit* pathLine = new QLineEdit(dlg);
    pathLine->setText(inputFile);
    pathLine->setReadOnly(false);
    pathLine->setToolTip(inputFile);
    dlgLayout->addWidget(pathLine);

    QGroupBox* groupbox = new QGroupBox("Load parameters", dlg);
    dlgLayout->addWidget(groupbox);

    QCheckBox* templateChBox = new QCheckBox(QString(UIName::Template.c_str()), groupbox);
    QCheckBox* groupChBox = new QCheckBox(QString(UIName::Group.c_str()), groupbox);
    QCheckBox* propertiesChBox = new QCheckBox(QString(UIName::Properties.c_str()), groupbox);
    QCheckBox* texturesChBox = new QCheckBox(QString(UIName::Textures.c_str()), groupbox);

    templateChBox->setChecked((bool)(initialState & CHECKED_TEMPLATE));
    groupChBox->setChecked((bool)(initialState & CHECKED_GROUP));
    propertiesChBox->setChecked((bool)(initialState & CHECKED_PROPERTIES));
    texturesChBox->setChecked((bool)(initialState & CHECKED_TEXTURES));

    QGridLayout* gridLayout = new QGridLayout();
    groupbox->setLayout(gridLayout);
    gridLayout->setHorizontalSpacing(50);
    gridLayout->addWidget(templateChBox, 0, 0);
    gridLayout->addWidget(groupChBox, 1, 0);
    gridLayout->addWidget(propertiesChBox, 0, 1);
    gridLayout->addWidget(texturesChBox, 1, 1);

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, dlg);
    dlgLayout->addWidget(buttons);

    QObject::connect(buttons, SIGNAL(accepted()), dlg, SLOT(accept()));
    QObject::connect(buttons, SIGNAL(rejected()), dlg, SLOT(reject()));

    if (QDialog::Accepted == dlg->exec())
    {
        if (templateChBox->checkState() == Qt::Checked)
            ret |= CHECKED_TEMPLATE;
        if (groupChBox->checkState() == Qt::Checked)
            ret |= CHECKED_GROUP;
        if (propertiesChBox->checkState() == Qt::Checked)
            ret |= CHECKED_PROPERTIES;
        if (texturesChBox->checkState() == Qt::Checked)
            ret |= CHECKED_TEXTURES;
    }

    delete dlg;
    return ret;
}

void MaterialEditor::StoreMaterialTextures(DAVA::NMaterial* material, const DAVA::InspMember* materialMember,
                                           DAVA::KeyedArchive* texturesArchive, DAVA::SerializationContext* context) const
{
    DAVA::InspInfoDynamic* dynamicInfo = materialMember->Dynamic()->GetDynamicInfo();
    DAVA::InspInfoDynamic::DynamicData ddata = dynamicInfo->Prepare(material, false);

    DAVA::Vector<DAVA::FastName> membersList = dynamicInfo->MembersList(ddata);
    for (const auto& texName : membersList)
    {
        if (material->HasLocalTexture(texName))
        {
            auto texturePath = material->GetLocalTexture(texName)->GetPathname();
            if (!texturePath.IsEmpty())
            {
                DAVA::String textureRelativePath = texturePath.GetRelativePathname(context->GetScenePath());
                if (textureRelativePath.size() > 0)
                {
                    texturesArchive->SetString(texName.c_str(), textureRelativePath);
                }
            }
        }
    }
}

void MaterialEditor::StoreMaterialFlags(DAVA::NMaterial* material, const DAVA::InspMember* materialMember,
                                        DAVA::KeyedArchive* flagsArchive) const
{
    DAVA::InspInfoDynamic* dynamicInfo = materialMember->Dynamic()->GetDynamicInfo();
    DAVA::InspInfoDynamic::DynamicData ddata = dynamicInfo->Prepare(material, false);
    DAVA::Vector<DAVA::FastName> membersList = dynamicInfo->MembersList(ddata);
    for (const auto& flagName : membersList)
    {
        if (material->HasLocalFlag(flagName))
        {
            flagsArchive->SetInt32(flagName.c_str(), material->GetLocalFlagValue(flagName));
        }
    }
}

void MaterialEditor::StoreMaterialProperties(DAVA::NMaterial* material, const DAVA::InspMember* materialMember,
                                             DAVA::KeyedArchive* propertiesArchive) const
{
    DAVA::InspInfoDynamic* dynamicInfo = materialMember->Dynamic()->GetDynamicInfo();
    DAVA::InspInfoDynamic::DynamicData ddata = dynamicInfo->Prepare(material, false);
    DAVA::Vector<DAVA::FastName> membersList = dynamicInfo->MembersList(ddata);
    for (const auto& propertyName : membersList)
    {
        if (material->HasLocalProperty(propertyName))
        {
            auto propertyType = material->GetLocalPropType(propertyName);
            auto propertyValue = material->GetLocalPropValue(propertyName);
            auto arraySize = material->GetLocalPropArraySize(propertyName);
            auto dataSize = sizeof(DAVA::float32) * DAVA::ShaderDescriptor::CalculateDataSize(propertyType, 1);

            DAVA::ScopedPtr<DAVA::KeyedArchive> prop(new DAVA::KeyedArchive());
            prop->SetUInt32("type", static_cast<DAVA::uint32>(propertyType));
            prop->SetUInt32("size", arraySize);
            prop->SetByteArray("data", reinterpret_cast<const DAVA::uint8*>(propertyValue), dataSize);
            propertiesArchive->SetArchive(propertyName.c_str(), prop);
        }
    }
}

void MaterialEditor::StoreMaterialToPreset(DAVA::NMaterial* material, DAVA::KeyedArchive* archive,
                                           DAVA::SerializationContext* context) const
{
    const DAVA::InspInfo* info = material->GetTypeInfo();

    DAVA::ScopedPtr<DAVA::KeyedArchive> texturesArchive(new DAVA::KeyedArchive());
    DAVA::ScopedPtr<DAVA::KeyedArchive> flagsArchive(new DAVA::KeyedArchive());
    DAVA::ScopedPtr<DAVA::KeyedArchive> propertiesArchive(new DAVA::KeyedArchive());

    const DAVA::InspMember* materialMember = info->Member(DAVA::FastName("localTextures"));
    if ((nullptr != materialMember) && (nullptr != materialMember->Dynamic()))
        StoreMaterialTextures(material, materialMember, texturesArchive, context);

    materialMember = info->Member(DAVA::FastName("localFlags"));
    if ((nullptr != materialMember) && (nullptr != materialMember->Dynamic()))
        StoreMaterialFlags(material, materialMember, flagsArchive);

    materialMember = info->Member(DAVA::FastName("localProperties"));
    if ((nullptr != materialMember) && (nullptr != materialMember->Dynamic()))
        StoreMaterialProperties(material, materialMember, propertiesArchive);

    archive->SetArchive("flags", flagsArchive);
    archive->SetArchive("textures", texturesArchive);
    archive->SetArchive("properties", propertiesArchive);

    auto fxName = material->GetLocalFXName();
    if (fxName.IsValid())
        archive->SetFastName("fxname", fxName);

    auto qualityGroup = material->GetQualityGroup();
    if (qualityGroup.IsValid())
        archive->SetFastName("group", qualityGroup);
}

void MaterialEditor::UpdateMaterialPropertiesFromPreset(DAVA::NMaterial* material, DAVA::KeyedArchive* properitesArchive)
{
    const auto properties = properitesArchive->GetArchieveData();
    for (const auto& pm : properties)
    {
        DVASSERT(DAVA::VariantType::TYPE_KEYED_ARCHIVE == pm.second->type);

        DAVA::FastName propName(pm.first);
        DAVA::KeyedArchive* propertyArchive = pm.second->AsKeyedArchive();

        /*
         * Here we are checking if propData if valid, because yaml parser can 
         * completely delete (skip) byte array node if it contains invalid data
         */
        const DAVA::float32* propData = reinterpret_cast<const DAVA::float32*>(propertyArchive->GetByteArray("data"));
        if (nullptr != propData)
        {
            rhi::ShaderProp::Type propType = static_cast<rhi::ShaderProp::Type>(propertyArchive->GetUInt32("type"));
            DAVA::uint32 propSize = propertyArchive->GetUInt32("size");

            if (material->HasLocalProperty(propName))
            {
                auto existingType = material->GetLocalPropType(propName);
                auto existingSize = material->GetLocalPropArraySize(propName);
                if ((existingType == propType) && (existingSize == propSize))
                {
                    material->SetPropertyValue(propName, propData);
                }
            }
            else
            {
                material->AddProperty(propName, propData, propType, propSize);
            }
        }
    }
}

void MaterialEditor::UpdateMaterialFlagsFromPreset(DAVA::NMaterial* material, DAVA::KeyedArchive* flagsArchive)
{
    const auto flags = flagsArchive->GetArchieveData();
    for (const auto& fm : flags)
    {
        if (material->HasLocalFlag(DAVA::FastName(fm.first)))
            material->SetFlag(DAVA::FastName(fm.first), fm.second->AsInt32());
        else
            material->AddFlag(DAVA::FastName(fm.first), fm.second->AsInt32());
    }
}

void MaterialEditor::UpdateMaterialTexturesFromPreset(DAVA::NMaterial* material, DAVA::KeyedArchive* texturesArchive,
                                                      const DAVA::FilePath& scenePath)
{
    const auto& texturesMap = texturesArchive->GetArchieveData();
    for (const auto& tm : texturesMap)
    {
        auto texture = DAVA::Texture::CreateFromFile(scenePath + tm.second->AsString());

        DAVA::FastName textureName(tm.first);
        if (material->HasLocalTexture(textureName))
        {
            material->SetTexture(textureName, texture);
        }
        else
        {
            material->AddTexture(textureName, texture);
        }
    }
}

void MaterialEditor::UpdateMaterialFromPresetWithOptions(DAVA::NMaterial* material, DAVA::KeyedArchive* preset,
                                                         DAVA::SerializationContext* context, DAVA::uint32 options)
{
    if ((options & CHECKED_GROUP) && preset->IsKeyExists("group"))
    {
        material->SetQualityGroup(preset->GetFastName("group"));
    }

    if ((options & CHECKED_TEMPLATE) && preset->IsKeyExists("fxname"))
    {
        material->SetFXName(preset->GetFastName("fxname"));
    }

    if ((options & CHECKED_PROPERTIES) && preset->IsKeyExists("flags"))
    {
        UpdateMaterialFlagsFromPreset(material, preset->GetArchive("flags"));
    }

    if ((options & CHECKED_PROPERTIES) && preset->IsKeyExists("properties"))
    {
        UpdateMaterialPropertiesFromPreset(material, preset->GetArchive("properties"));
    }

    if ((options & CHECKED_TEXTURES) && preset->IsKeyExists("textures"))
    {
        UpdateMaterialTexturesFromPreset(material, preset->GetArchive("textures"), context->GetScenePath());
    }
}

void MaterialEditor::RefreshMaterialProperties()
{
    SetCurMaterial(curMaterials);
    UpdateAllAddRemoveButtons(ui->materialProperty->GetRootProperty());
}

void MaterialEditor::FinishCreation()
{
    baseRoot->FinishTreeCreation();
    flagsRoot->FinishTreeCreation();
    illuminationRoot->FinishTreeCreation();
    propertiesRoot->FinishTreeCreation();
    texturesRoot->FinishTreeCreation();
}

void MaterialEditor::removeInvalidTexture()
{
    QtPropertyToolButton* button = dynamic_cast<QtPropertyToolButton*>(sender());
    QtPropertyData* data = button->GetPropertyData();
    DAVA::FastName textureSlot = data->GetName();

    SceneEditor2* curScene = QtMainWindow::Instance()->GetCurrentScene();
    DVASSERT(curScene != nullptr);

    DAVA::uint32 count = static_cast<DAVA::uint32>(curMaterials.size());
    curScene->BeginBatch("Remove invalid texture from material", count);
    for (DAVA::uint32 i = 0; i < count; ++i)
    {
        DAVA::NMaterial* material = curMaterials[i];
        while (material != nullptr)
        {
            if (material->HasLocalTexture(textureSlot))
            {
                curScene->Exec(Command2::Create<MaterialRemoveTexture>(textureSlot, material));
                break;
            }

            material = material->GetParent();
        }
    }
    curScene->EndBatch();
}

void MaterialEditor::UpdateTabs()
{
    MaterialEditorLocal::PropertyLock propertyLock(ui->tabbar, MaterialEditorLocal::CURRENT_TAB_CHANGED_UPDATE);
    if (!propertyLock.IsLocked())
    {
        return;
    }

    while (ui->tabbar->count() > 0)
    {
        ui->tabbar->removeTab(0);
    }

    if (curMaterials.size() == 1)
    {
        DAVA::NMaterial* material = curMaterials.front();
        validator->SetCurrentMaterial(material);
        for (DAVA::uint32 i = 0; i < material->GetConfigCount(); ++i)
        {
            ui->tabbar->addTab(QString(material->GetConfigName(i).c_str()));
        }

        ui->tabbar->setCurrentIndex(material->GetCurrentConfigIndex());
        ui->tabbar->setTabsClosable(material->GetConfigCount() > 1);
    }
    else
    {
        validator->SetCurrentMaterial(nullptr);
    }

    ui->tabbar->setVisible(curMaterials.size() == 1);
}

void MaterialEditor::onTabNameChanged(int index)
{
    SceneEditor2* scene = QtMainWindow::Instance()->GetCurrentScene();

    DVASSERT(scene != nullptr);
    DVASSERT(curMaterials.size() == 1);

    DAVA::NMaterial* material = curMaterials.front();
    const DAVA::InspMember* configNameProperty = material->GetTypeInfo()->Member(DAVA::FastName("configName"));
    DVASSERT(configNameProperty != nullptr);
    DAVA::VariantType newValue(DAVA::FastName(ui->tabbar->tabText(index).toStdString()));
    scene->Exec(Command2::Create<InspMemberModifyCommand>(configNameProperty, material, newValue));
}

void MaterialEditor::onCreateConfig(int index)
{
    SceneEditor2* scene = QtMainWindow::Instance()->GetCurrentScene();
    DVASSERT(scene != nullptr);
    DVASSERT(curMaterials.size() == 1);
    DAVA::NMaterial* material = curMaterials.front();
    DAVA::MaterialConfig newConfig;
    if (index >= 0)
    {
        newConfig = material->GetConfig(static_cast<DAVA::uint32>(index));
        DAVA::String newConfigName = DAVA::String("_copy");
        if (newConfig.name.IsValid())
        {
            newConfigName = DAVA::String(newConfig.name.c_str()) + newConfigName;
        }

        newConfig.name = DAVA::FastName(newConfigName);
    }
    else
    {
        newConfig.name = DAVA::FastName("Empty");
        newConfig.fxName = material->GetEffectiveFXName();
    }

    DAVA::uint32 counter = 2;
    while (material->FindConfigByName(newConfig.name) < material->GetConfigCount())
    {
        newConfig.name = DAVA::FastName(DAVA::String(newConfig.name.c_str()) + std::to_string(counter));
    }
    scene->Exec(Command2::Create<MaterialCreateConfig>(material, newConfig));
}

void MaterialEditor::onCurrentConfigChanged(int index)
{
    if (index < 0)
        return;

    MaterialEditorLocal::PropertyLock propertyLock(ui->tabbar, MaterialEditorLocal::CURRENT_TAB_CHANGED_UPDATE);
    if (!propertyLock.IsLocked())
    {
        return;
    }

    SceneEditor2* curScene = QtMainWindow::Instance()->GetCurrentScene();
    DVASSERT(curScene);
    DVASSERT(curMaterials.size() == 1);
    DAVA::NMaterial* material = curMaterials.front();
    DVASSERT(static_cast<DAVA::uint32>(index) < material->GetConfigCount());
    curScene->Exec(Command2::Create<MaterialChangeCurrentConfig>(material, static_cast<DAVA::uint32>(index)));
}

void MaterialEditor::onTabRemove(int index)
{
    SceneEditor2* curScene = QtMainWindow::Instance()->GetCurrentScene();
    DVASSERT(curScene);
    DVASSERT(curMaterials.size() == 1);
    DAVA::NMaterial* material = curMaterials.front();

    curScene->Exec(Command2::Create<MaterialRemoveConfig>(material, static_cast<DAVA::uint32>(index)));
}

void MaterialEditor::onTabContextMenuRequested(const QPoint& pos)
{
    int tabIndex = ui->tabbar->tabAt(pos);

    std::unique_ptr<QMenu> contextMenu(new QMenu());

    if (tabIndex != -1)
    {
        QString actionText = QString("Create copy from %1").arg(ui->tabbar->tabText(tabIndex));
        QAction* createCopy = new QAction(actionText, contextMenu.get());
        QObject::connect(createCopy, &QAction::triggered, [this, tabIndex]() { onCreateConfig(tabIndex); });
        contextMenu->addAction(createCopy);
    }

    QAction* createEmpty = new QAction("Create empty config", contextMenu.get());
    QObject::connect(createEmpty, &QAction::triggered, [this]() { onCreateConfig(-1); });
    contextMenu->addAction(createEmpty);
    contextMenu->exec(ui->tabbar->mapToGlobal(pos));
}
