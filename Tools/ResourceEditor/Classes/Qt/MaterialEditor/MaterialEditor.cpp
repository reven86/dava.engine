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

#include "Scene3D/Systems/QualitySettingsSystem.h"

#include "CommandLine/TextureDescriptor/TextureDescriptorUtils.h"
#include "Tools/PathDescriptor/PathDescriptor.h"


#include "QtTools/FileDialog/FileDialog.h"


#define MATERIAL_NAME_LABEL "Name"
#define MATERIAL_GROUP_LABEL "Group"
#define MATERIAL_BASE_LABEL "Base"
#define MATERIAL_FLAGS_LABEL "Flags"
#define MATERIAL_PROPERTIES_LABEL "Properties"
#define MATERIAL_TEXTURES_LABEL "Textures"
#define MATERIAL_ILLUMINATION_LABEL "Illumination"
#define MATERIAL_TEMPLATE_LABEL "Template"

MaterialEditor::MaterialEditor(QWidget *parent /* = 0 */)
    : QDialog(parent)
    , ui(new Ui::MaterialEditor)
    , templatesFilterModel(nullptr)
    , lastCheckState(CHECKED_ALL)
{
    ui->setupUi(this);
    setWindowFlags(WINDOWFLAG_ON_TOP_OF_APPLICATION);

    ui->materialTree->setDragEnabled(true);
    ui->materialTree->setAcceptDrops(true);
    ui->materialTree->setDragDropMode(QAbstractItemView::DragDrop);

    ui->materialProperty->SetEditTracking(true);
    ui->materialProperty->setContextMenuPolicy(Qt::CustomContextMenu);
    //ui->materialProperty->setSortingEnabled(true);
    //ui->materialProperty->header()->setSortIndicator(0, Qt::AscendingOrder);

    baseRoot = new QtPropertyData();
    flagsRoot = new QtPropertyData();
    propertiesRoot = new QtPropertyData();
    illuminationRoot = new QtPropertyData();
    texturesRoot = new QtPropertyData();

    ui->materialProperty->AppendProperty(MATERIAL_BASE_LABEL, baseRoot);
    ui->materialProperty->AppendProperty(MATERIAL_FLAGS_LABEL, flagsRoot);
    ui->materialProperty->AppendProperty(MATERIAL_PROPERTIES_LABEL, propertiesRoot);
    ui->materialProperty->AppendProperty(MATERIAL_ILLUMINATION_LABEL, illuminationRoot);
    ui->materialProperty->AppendProperty(MATERIAL_TEXTURES_LABEL, texturesRoot);

    ui->materialProperty->ApplyStyle(baseRoot, QtPropertyEditor::HEADER_STYLE);
    ui->materialProperty->ApplyStyle(flagsRoot, QtPropertyEditor::HEADER_STYLE);
    ui->materialProperty->ApplyStyle(propertiesRoot, QtPropertyEditor::HEADER_STYLE);
    ui->materialProperty->ApplyStyle(illuminationRoot, QtPropertyEditor::HEADER_STYLE);
    ui->materialProperty->ApplyStyle(texturesRoot, QtPropertyEditor::HEADER_STYLE);

    // global scene manager signals
    QObject::connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2 *)), this, SLOT(sceneActivated(SceneEditor2 *)));
    QObject::connect(SceneSignals::Instance(), SIGNAL(Deactivated(SceneEditor2 *)), this, SLOT(sceneDeactivated(SceneEditor2 *)));
    QObject::connect(SceneSignals::Instance(), SIGNAL(CommandExecuted(SceneEditor2 *, const Command2*, bool)), this, SLOT(commandExecuted(SceneEditor2 *, const Command2 *, bool)));
    QObject::connect(SceneSignals::Instance(), SIGNAL(SelectionChanged(SceneEditor2 *, const EntityGroup *, const EntityGroup *)), this, SLOT(autoExpand()));

    // material tree
    QObject::connect(ui->materialTree->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(materialSelected(const QItemSelection &, const QItemSelection &)));
    QObject::connect(ui->materialTree, SIGNAL(Updated()), this, SLOT(autoExpand()));
    QObject::connect(ui->materialTree, SIGNAL(ContextMenuPrepare(QMenu *)), this, SLOT(onContextMenuPrepare(QMenu *)));

    // material properties
    QObject::connect(ui->materialProperty, SIGNAL(PropertyEdited(const QModelIndex &)), this, SLOT(OnPropertyEdited(const QModelIndex &)));
    QObject::connect(ui->materialProperty, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(OnMaterialPropertyEditorContextMenuRequest(const QPoint &)));
    QObject::connect(ui->templateBox, SIGNAL(activated(int)), this, SLOT(OnTemplateChanged(int)));
    QObject::connect(ui->templateButton, SIGNAL(clicked()), this, SLOT(OnTemplateButton()));
    QObject::connect(ui->actionAddGlobalMaterial, SIGNAL(triggered(bool)), this, SLOT(OnMaterialAddGlobal(bool)));
    QObject::connect(ui->actionRemoveGlobalMaterial, SIGNAL(triggered(bool)), this, SLOT(OnMaterialRemoveGlobal(bool)));
    QObject::connect(ui->actionSaveMaterialPreset, SIGNAL(triggered(bool)), this, SLOT(OnMaterialSave(bool)));
    QObject::connect(ui->actionLoadMaterialPreset, SIGNAL(triggered(bool)), this, SLOT(OnMaterialLoad(bool)));

    posSaver.Attach(this);
    new QtPosSaver(ui->splitter);
    treeStateHelper = new PropertyEditorStateHelper(ui->materialProperty, (QtPropertyModel *) ui->materialProperty->model());

    DAVA::VariantType v1 = posSaver.LoadValue("splitPosProperties");
    DAVA::VariantType v2 = posSaver.LoadValue("splitPosPreview");
    if (v1.GetType() == DAVA::VariantType::TYPE_INT32) ui->materialProperty->header()->resizeSection(0, v1.AsInt32());
    if (v2.GetType() == DAVA::VariantType::TYPE_INT32) ui->materialProperty->header()->resizeSection(1, v2.AsInt32());

    DAVA::VariantType savePath = posSaver.LoadValue("lastSavePath");
    DAVA::VariantType loadState = posSaver.LoadValue("lastLoadState");
    if (savePath.GetType() == DAVA::VariantType::TYPE_FILEPATH) lastSavePath = savePath.AsFilePath();
    if (loadState.GetType() == DAVA::VariantType::TYPE_UINT32) lastCheckState = loadState.AsUInt32();

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

void MaterialEditor::initActions()
{
    ui->actionShowAll->setData(MaterialFilteringModel::SHOW_ALL);
    ui->actionInstances->setData(MaterialFilteringModel::SHOW_ONLY_INSTANCES);
    ui->actionMaterialsInstances->setData(MaterialFilteringModel::SHOW_INSTANCES_AND_MATERIALS);

    const int filterType =  MaterialFilteringModel::SHOW_ALL;
    foreach(QAction *action, ui->filterType->actions())
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
        QStandardItemModel *templatesModel = new QStandardItemModel(this);

        const QVector<ProjectManager::AvailableMaterialTemplate> *templates = ProjectManager::Instance()->GetAvailableMaterialTemplates();
        QStandardItem *emptyItem = new QStandardItem(QString());
        templatesModel->appendRow(emptyItem);

        for(int i = 0; i < templates->size(); ++i)
        {
            QStandardItem *item = new QStandardItem();
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
    QAbstractItemModel *model = ui->templateBox->model();
    const QModelIndex index = model->index(0, 0);
    model->setData(index, text, Qt::DisplayRole);
}

void MaterialEditor::autoExpand()
{
    QAction *action = ui->filterType->checkedAction();
    if (nullptr == action)
        return;

    const int filterType = action->data().toInt();

    if (expandMap[filterType])
        ui->materialTree->expandAll();
}

void MaterialEditor::onFilterChanged()
{
    QAction *action = ui->filterType->checkedAction();
    if (nullptr == action)
        return;

    const int filterType = action->data().toInt();
    ui->materialTree->setFilterType(filterType);

    ui->actionAutoExpand->setChecked(expandMap[filterType]);
    onCurrentExpandModeChange(expandMap[filterType]);
}

void MaterialEditor::SelectMaterial(DAVA::NMaterial *material)
{
    ui->materialTree->SelectMaterial(material);
}

void MaterialEditor::SelectEntities(DAVA::NMaterial *material)
{
    QList<DAVA::NMaterial *> materials;
    materials << material;
    ui->materialTree->SelectEntities(materials);
}

void MaterialEditor::SetCurMaterial(const QList< DAVA::NMaterial *>& materials)
{
    int curScrollPos = ui->materialProperty->verticalScrollBar()->value();

    curMaterials = materials;
    treeStateHelper->SaveTreeViewState(false);
    
    FillBase();
    FillDynamic(flagsRoot, DAVA::FastName("localFlags"));
    FillDynamic(propertiesRoot, DAVA::FastName("localProperties"));
    FillDynamic(texturesRoot, DAVA::FastName("localTextures"));
    FillTemplates(materials);

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
    SceneEditor2 *sceneEditor = QtMainWindow::Instance()->GetCurrentScene();
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

void MaterialEditor::sceneActivated(SceneEditor2 *scene)
{
    if (isVisible())
    {
        SetCurMaterial(QList< DAVA::NMaterial *>());
        ui->materialTree->SetScene(scene);
        autoExpand();
    }
}

void MaterialEditor::sceneDeactivated(SceneEditor2 *scene)
{ 
    ui->materialTree->SetScene(nullptr);
    SetCurMaterial(QList< DAVA::NMaterial *>());
}

void MaterialEditor::materialSelected(const QItemSelection & selected, const QItemSelection & deselected)
{
    QList< DAVA::NMaterial *> materials;
    QItemSelectionModel *selection = ui->materialTree->selectionModel();
    const QModelIndexList selectedRows = selection->selectedRows();

    foreach (const QModelIndex& index, selectedRows)
    {
        if (index.column() == 0)
        {
            DAVA::NMaterial *material = ui->materialTree->GetMaterial(index);
            if (material)
                materials << material;
        }
    }

    SetCurMaterial(materials);
}

void MaterialEditor::commandExecuted(SceneEditor2 *scene, const Command2 *command, bool redo)
{
    if (scene == QtMainWindow::Instance()->GetCurrentScene())
    {
        int curScrollPos = ui->materialProperty->verticalScrollBar()->value();
        int cmdId = command->GetId();

        switch (cmdId)
        {
        case CMDID_INSP_MEMBER_MODIFY:
            {
                InspMemberModifyCommand *inspCommand = (InspMemberModifyCommand *)command;

                const QString memberName = inspCommand->member->Name().c_str();
                if (memberName == "materialGroup" || memberName == "fxName")
                {
                    SetCurMaterial(curMaterials);
                }
            }
            break;
        case CMDID_INSP_DYNAMIC_MODIFY:
            {
                InspDynamicModifyCommand *inspCommand = (InspDynamicModifyCommand *) command;

                // if material flag was changed we should rebuild list of all properties
                // because their set can be changed
                if (inspCommand->dynamicInfo->GetMember()->Name() == DAVA::FastName("localFlags"))
                {
                    FillDynamic(propertiesRoot, DAVA::FastName("localProperties"));
                    FillDynamic(texturesRoot, DAVA::FastName("localTextures"));
                }

                UpdateAllAddRemoveButtons(ui->materialProperty->GetRootProperty());
            }
            break;
        case CMDID_MATERIAL_GLOBAL_SET:
            {
                sceneActivated(scene);
                SetCurMaterial(curMaterials);
            }
            break;
        default:
            break;
        }

        ui->materialProperty->verticalScrollBar()->setValue(curScrollPos);
    }
}

void MaterialEditor::OnQualityChanged()
{
    SetCurMaterial(curMaterials);
}

void MaterialEditor::onCurrentExpandModeChange(bool mode)
{
    QAction *action = ui->filterType->checkedAction();
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

void MaterialEditor::showEvent(QShowEvent * event)
{
    FillTemplates(QList<DAVA::NMaterial *>());
    sceneActivated(QtMainWindow::Instance()->GetCurrentScene());
}

void MaterialEditor::FillBase()
{
    baseRoot->ChildRemoveAll();
    illuminationRoot->ChildRemoveAll();

    foreach(DAVA::NMaterial *material, curMaterials)
    {
        const DAVA::InspInfo *info = material->GetTypeInfo();

        // fill material name
        const DAVA::InspMember *nameMember = info->Member(DAVA::FastName("materialName"));
        if (nullptr != nameMember)
        {
            QtPropertyDataInspMember *name = new QtPropertyDataInspMember(material, nameMember);
            baseRoot->MergeChild(name, MATERIAL_NAME_LABEL);
        }

        // fill material group, only for material type
        const DAVA::InspMember *groupMember = info->Member(DAVA::FastName("qualityGroup"));
        if (nullptr != groupMember)
        {
            QtPropertyDataInspMember *group = new QtPropertyDataInspMember(material, groupMember);
            baseRoot->MergeChild(group, MATERIAL_GROUP_LABEL);

            // Add unknown value:
            group->AddAllowedValue(VariantType(String()), "Unknown");

            // fill allowed values for material group
            for(size_t i = 0; i < DAVA::QualitySettingsSystem::Instance()->GetMaterialQualityGroupCount(); ++i)
            {
                DAVA::FastName groupName = DAVA::QualitySettingsSystem::Instance()->GetMaterialQualityGroupName(i);
                group->AddAllowedValue(DAVA::VariantType(groupName), groupName.c_str());
            }
        }
    }
}

void MaterialEditor::FillDynamic(QtPropertyData *root, const FastName& dynamicName)
{
    DAVA::NMaterial *globalMaterial = nullptr;
    root->ChildRemoveAll();

    SceneEditor2 *sceneEditor = QtMainWindow::Instance()->GetCurrentScene();
    if (nullptr != sceneEditor)
    {
        globalMaterial = sceneEditor->GetGlobalMaterial();
    }

    foreach(DAVA::NMaterial *material, curMaterials)
    {
        const DAVA::InspInfo *info = material->GetTypeInfo();
        const DAVA::InspMember *materialMember = info->Member(dynamicName);

        // fill material flags
        if ((nullptr != materialMember) && (nullptr != materialMember->Dynamic()))
        {
            DAVA::InspInfoDynamic *dynamicInfo = materialMember->Dynamic()->GetDynamicInfo();
            FillDynamicMembers(root, dynamicInfo, material, material == globalMaterial);
        }
    }
}

void MaterialEditor::FillDynamicMembers(QtPropertyData *root, DAVA::InspInfoDynamic *dynamic, DAVA::NMaterial *material, bool isGlobal)
{
    DAVA::InspInfoDynamic::DynamicData ddata = dynamic->Prepare(material, isGlobal);
    DAVA::Vector<DAVA::FastName> membersList = dynamic->MembersList(ddata); 

    // enumerate dynamic members and add them
    for(size_t i = 0; i < membersList.size(); ++i)
    {
        QtPropertyDataInspDynamic *dynamicData = new QtPropertyDataInspDynamic(dynamic, ddata, membersList[i]);

        // for all textures we should add texture path validator
        if (root == texturesRoot)
        {
            ApplyTextureValidator(dynamicData);
        }

        // update buttons state and enabled/disable state of this data
        UpdateAddRemoveButtonState(dynamicData);

        // merge created dynamic data into specified root
        root->MergeChild(dynamicData, membersList[i].c_str());
    }
}

void MaterialEditor::ApplyTextureValidator(QtPropertyDataInspDynamic *data)
{
    QString defaultPath = ProjectManager::Instance()->CurProjectPath().GetAbsolutePathname().c_str();
    FilePath dataSourcePath = ProjectManager::Instance()->CurProjectDataSourcePath();

    // calculate appropriate default path
    if (dataSourcePath.Exists())
    {
        defaultPath = dataSourcePath.GetAbsolutePathname().c_str();
    }

    SceneEditor2* editor = QtMainWindow::Instance()->GetCurrentScene();
    if ((nullptr != editor) && editor->GetScenePath().Exists())
    {
        DAVA::String scenePath = editor->GetScenePath().GetDirectory().GetAbsolutePathname();
        if (String::npos != scenePath.find(dataSourcePath.GetAbsolutePathname()))
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

void MaterialEditor::UpdateAddRemoveButtonState(QtPropertyDataInspDynamic *data)
{
    // don't create/update buttons for global material
    SceneEditor2 *curScene = QtMainWindow::Instance()->GetCurrentScene();
    if (nullptr != curScene /*&& data->object != curScene->GetGlobalMaterial()*/)
    {
        // extract member flags from dynamic info
        int memberFlags = data->dynamicInfo->MemberFlags(data->ddata, data->name);
    
        QtPropertyToolButton *addRemoveButton = nullptr;

        // check if there is already our button
        for(int i = 0; i < data->GetButtonsCount(); ++i)
        {
            QtPropertyToolButton *btn = data->GetButton(i);
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
            addRemoveButton->setIcon(QIcon(":/QtIcons/cminus.png"));
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
            addRemoveButton->setIcon(QIcon(":/QtIcons/cplus.png"));
            addRemoveButton->setToolTip("Add property");
        }

        // don't allow editing for members that are inherited
        data->SetEnabled(editEnabled);
        data->SetBackground(bgColor);
        for(int m = 0; m < data->ChildCount(); ++m)
        {
            data->ChildGet(m)->SetEnabled(editEnabled);
            data->ChildGet(m)->SetBackground(bgColor);
        }
    }
}

void MaterialEditor::UpdateAllAddRemoveButtons(QtPropertyData *root)
{
    QtPropertyDataInspDynamic *dynamicData = dynamic_cast<QtPropertyDataInspDynamic *>(root);
    if (nullptr != dynamicData)
    {
        UpdateAddRemoveButtonState(dynamicData);
    }

    for (int i = 0; i < root->ChildCount(); ++i)
    {
        UpdateAllAddRemoveButtons(root->ChildGet(i));
    }
}

void MaterialEditor::FillTemplates(const QList<DAVA::NMaterial *>& materials)
{
    initTemplates();

    if (1 == materials.size() && materials[0])
    {
        DAVA::NMaterial* material = materials[0];
        DAVA::FastName fxName = material->GetEffectiveFXName();
        bool isLocalFxName = material->HasLocalFXName();
        bool hasParentFx = false;

        if (nullptr != material->GetParent())
        {
            hasParentFx = material->GetParent()->GetEffectiveFXName().IsValid();
        }

        if (isLocalFxName)
        {
            int rowToSelect = -1;
            QAbstractItemModel *model = ui->templateBox->model();
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
            }

            ui->templateBox->setCurrentIndex(rowToSelect);
            ui->templateBox->setEnabled(true);

            ui->templateButton->setIcon(QIcon(":/QtIcons/cminus.png"));
            ui->templateButton->setEnabled(hasParentFx);
        }
        else
        {
            ui->templateBox->setEnabled(false);
            ui->templateButton->setIcon(QIcon(":/QtIcons/cplus.png"));
            ui->templateButton->setEnabled(true);
        }
    }
    else
    {
        ui->templateBox->setCurrentIndex(-1);
        ui->templateBox->setEnabled(false);
        ui->templateButton->setEnabled(false);
        ui->templateButton->setIcon(QIcon(":/QtIcons/cplus.png"));
    }
}

void MaterialEditor::OnTemplateChanged(int index)
{
    if (curMaterials.size() == 1 && index > 0)
    {
        DAVA::NMaterial *material = curMaterials.at(0);
        QString newTemplatePath = GetTemplatePath(index);
        if (!newTemplatePath.isEmpty())
        {
            const DAVA::InspMember *templateMember = material->GetTypeInfo()->Member(DAVA::FastName("fxName"));

            if (nullptr != templateMember)
            {
                QtMainWindow::Instance()->GetCurrentScene()->Exec(new InspMemberModifyCommand(templateMember, material, 
                    DAVA::VariantType(DAVA::FastName(newTemplatePath.toStdString().c_str()))));
            }
        }
    }

    SetCurMaterial(curMaterials);
}

void MaterialEditor::OnTemplateButton()
{
    if (1 == curMaterials.size())
    {
        DAVA::NMaterial *material = curMaterials[0];
        const DAVA::InspMember *templateMember = material->GetTypeInfo()->Member(DAVA::FastName("fxName"));

        if (nullptr != templateMember)
        {
            Command2 *cmd = nullptr;
            if (material->HasLocalFXName())
            {
                // has local fxname, so button shoud remove it (by setting empty value)
                cmd = new InspMemberModifyCommand(templateMember, material, DAVA::VariantType(DAVA::FastName()));
            }
            else
            {
                // no local fxname, so button should add it
                cmd = new InspMemberModifyCommand(templateMember, material, DAVA::VariantType(material->GetEffectiveFXName()));
            }

            QtMainWindow::Instance()->GetCurrentScene()->Exec(cmd);
            SetCurMaterial(curMaterials);
        }
    }
}

QString MaterialEditor::GetTemplatePath(int index) const
{
    return ui->templateBox->itemData(index, Qt::UserRole).toString();
}

void MaterialEditor::OnAddRemoveButton()
{
    QtPropertyToolButton *btn = dynamic_cast<QtPropertyToolButton *>(QObject::sender());
    if (nullptr != btn)
    {
        QtPropertyDataInspDynamic *data = (QtPropertyDataInspDynamic *) btn->GetPropertyData();
        if (nullptr != data)
        {
            int memberFlags = data->dynamicInfo->MemberFlags(data->ddata, data->name);

            // pressed remove button
            if (memberFlags & I_EDIT)
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

void MaterialEditor::OnPropertyEdited(const QModelIndex &index)
{
    QtPropertyEditor *editor = dynamic_cast<QtPropertyEditor *>(QObject::sender());
    if (nullptr != editor)
    {
        QtPropertyData *propData = editor->GetProperty(index);

        if (nullptr != propData)
        {
            QList<QtPropertyData *> propDatList; //propData->GetMergedData();
            propDatList.reserve(propData->GetMergedCount() + 1);
            for (int i = 0; i < propData->GetMergedCount(); i++)
                propDatList << propData->GetMergedData(i);
            propDatList << propData;

            QList<Command2 *> commands;
            foreach (QtPropertyData *data, propDatList)
            {
                Command2 *command = (Command2 *) data->CreateLastCommand();
                if (command)
                {
                    commands << command;
                }
            }
            const bool usebatch = (commands.count() > 1);

            SceneEditor2 *curScene = QtMainWindow::Instance()->GetCurrentScene();
            if (curScene)
            {
                if (usebatch)
                {
                    QObject::disconnect(SceneSignals::Instance(), SIGNAL(CommandExecuted(SceneEditor2 *, const Command2*, bool)), this, SLOT(commandExecuted(SceneEditor2 *, const Command2 *, bool)));
                    curScene->BeginBatch("Property multiedit");
                }

                for (int i = 0; i < commands.size(); i++)
                {
                    Command2 *cmd = commands.at(i);
                    curScene->Exec(cmd);
                }

                if (usebatch)
                {
                    curScene->EndBatch();
                    QObject::connect(SceneSignals::Instance(), SIGNAL(CommandExecuted(SceneEditor2 *, const Command2*, bool)), this, SLOT(commandExecuted(SceneEditor2 *, const Command2 *, bool)));

                    // emulate that only one signal was emited, after batch of commands executed
                    commandExecuted(curScene, commands.last(), true);
                }
            }
        }
    }
}

void MaterialEditor::OnMaterialAddGlobal(bool checked)
{
    SceneEditor2 *curScene = QtMainWindow::Instance()->GetCurrentScene();
    if (nullptr != curScene)
    {
        DAVA::NMaterial *global = new DAVA::NMaterial();
        SCOPE_EXIT{ SafeRelease(global); };

        global->SetMaterialName(FastName("Scene_Global_Material"));
        curScene->Exec(new MaterialGlobalSetCommand(curScene, global));
        SafeRelease(global);

        sceneActivated(curScene);
        SelectMaterial(curScene->GetGlobalMaterial());
    }
}

void MaterialEditor::OnMaterialRemoveGlobal(bool checked)
{
    SceneEditor2 *curScene = QtMainWindow::Instance()->GetCurrentScene();
    if (nullptr != curScene)
    {
        curScene->Exec(new MaterialGlobalSetCommand(curScene, nullptr));
        sceneActivated(curScene);
    }
}

void MaterialEditor::onContextMenuPrepare(QMenu *menu)
{
    if (curMaterials.size() > 0)
    {
        ui->actionSaveMaterialPreset->setEnabled(curMaterials.size() == 1);
        menu->addSeparator();
        menu->addAction(ui->actionLoadMaterialPreset);
        menu->addAction(ui->actionSaveMaterialPreset);
    }
}

void MaterialEditor::OnMaterialPropertyEditorContextMenuRequest(const QPoint & pos)
{
    SceneEditor2 *sceneEditor = QtMainWindow::Instance()->GetCurrentScene();
    if (nullptr != sceneEditor && curMaterials.size() == 1)
    {
        DAVA::NMaterial *globalMaterial = sceneEditor->GetGlobalMaterial();
        DAVA::NMaterial *material = curMaterials[0];

        QModelIndex index = ui->materialProperty->indexAt(pos);

        if (globalMaterial != material && index.column() == 0 && (nullptr != globalMaterial))
        {
            QtPropertyData *data = ui->materialProperty->GetProperty(index);
            if (nullptr != data && data->Parent() == propertiesRoot)
            {
                QtPropertyDataInspDynamic *dynamicData = (QtPropertyDataInspDynamic *) data;
                if (nullptr != dynamicData)
                {
                    const DAVA::FastName & propertyName = dynamicData->name;
                    bool hasProperty = material->HasLocalProperty(propertyName);
                    if (hasProperty)
                    {
                        QMenu menu;
                        menu.addAction("Add to Global Material");
                        QAction *resultAction = menu.exec(ui->materialProperty->viewport()->mapToGlobal(pos));

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

        SceneEditor2 *curScene = QtMainWindow::Instance()->GetCurrentScene();

        if (!outputFile.isEmpty() && (nullptr != curScene))
        {
            lastSavePath = outputFile.toLatin1().data();

            DAVA::SerializationContext materialContext;
            materialContext.SetScene(curScene);
            materialContext.SetScenePath(ProjectManager::Instance()->CurProjectPath());
			materialContext.SetVersion(VersionInfo::Instance()->GetCurrentVersion().version);

            ScopedPtr<DAVA::KeyedArchive> materialArchive(new DAVA::KeyedArchive());
			StoreMaterialToPreset(curMaterials.front(), materialArchive, &materialContext);

            ScopedPtr<DAVA::KeyedArchive> presetArchive(new DAVA::KeyedArchive());
			presetArchive->SetUInt32("serializationContextVersion", materialContext.GetVersion());
			presetArchive->SetArchive("content", materialArchive);
            presetArchive->Save(lastSavePath);
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

        SceneEditor2 *curScene = QtMainWindow::Instance()->GetCurrentScene();

        if (!inputFile.isEmpty() && (nullptr != curScene))
        {
            lastSavePath = inputFile.toLatin1().data();

            ScopedPtr<DAVA::KeyedArchive> presetArchive(new DAVA::KeyedArchive());
            presetArchive->Load(lastSavePath);

			// not checking version right now
			// version info is reserved for future use
			if (presetArchive->IsKeyExists("content"))
			{
				DAVA::KeyedArchive* materialArchive = presetArchive->GetArchive("content");
				DAVA::uint32 userChoiseWhatToLoad = ExecMaterialLoadingDialog(lastCheckState, inputFile);
				DAVA::SerializationContext materialContext;
				materialContext.SetScene(curScene);
				materialContext.SetScenePath(ProjectManager::Instance()->CurProjectPath());
				materialContext.SetVersion(VersionInfo::Instance()->GetCurrentVersion().version);
				UpdateMaterialFromPresetWithOptions(curMaterials.front(), materialArchive, &materialContext, userChoiseWhatToLoad);
				materialContext.ResolveMaterialBindings();
				curScene->SetChanged(true);
			}
			else 
			{
		        QMessageBox::warning(this, "Material preset not supported",
					"Material preset you are trying to open is either old or invalid.");
			}
        }
    }

    SetCurMaterial(curMaterials);
}

void MaterialEditor::ClearDynamicMembers(DAVA::NMaterial *material, const DAVA::InspMemberDynamic *dynamicMember)
{
    if (nullptr != dynamicMember)
    {
        DAVA::InspInfoDynamic *dynamicInfo = dynamicMember->GetDynamicInfo();
        DAVA::InspInfoDynamic::DynamicData ddata = dynamicInfo->Prepare(material);
        DAVA::Vector<DAVA::FastName> membersList = dynamicInfo->MembersList(ddata); // this function can be slow
        for(size_t i = 0; i < membersList.size(); ++i)
        {
            dynamicInfo->MemberValueSet(ddata, membersList[i], DAVA::VariantType());
        }
    }
}

DAVA::uint32 MaterialEditor::ExecMaterialLoadingDialog(DAVA::uint32 initialState, const QString &inputFile)
{
    DAVA::uint32 ret = 0;

    QDialog *dlg = new QDialog(this);
    QVBoxLayout *dlgLayout = new QVBoxLayout();
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

    QGroupBox *groupbox = new QGroupBox("Load parameters", dlg);
    dlgLayout->addWidget(groupbox);

    QCheckBox *templateChBox = new QCheckBox(MATERIAL_TEMPLATE_LABEL, groupbox);
    QCheckBox *groupChBox = new QCheckBox(MATERIAL_GROUP_LABEL, groupbox);
    QCheckBox *propertiesChBox = new QCheckBox(MATERIAL_PROPERTIES_LABEL, groupbox);
    QCheckBox *texturesChBox = new QCheckBox(MATERIAL_TEXTURES_LABEL, groupbox);

    templateChBox->setChecked((bool) (initialState & CHECKED_TEMPLATE));
    groupChBox->setChecked((bool) (initialState & CHECKED_GROUP));
    propertiesChBox->setChecked((bool) (initialState & CHECKED_PROPERTIES));
    texturesChBox->setChecked((bool) (initialState & CHECKED_TEXTURES));

    QGridLayout *gridLayout = new QGridLayout();
    groupbox->setLayout(gridLayout);
    gridLayout->setHorizontalSpacing(50);
    gridLayout->addWidget(templateChBox, 0, 0);
    gridLayout->addWidget(groupChBox, 1, 0);
    gridLayout->addWidget(propertiesChBox, 0, 1);
    gridLayout->addWidget(texturesChBox, 1, 1);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, dlg);
    dlgLayout->addWidget(buttons);

    QObject::connect(buttons, SIGNAL(accepted()), dlg, SLOT(accept()));
    QObject::connect(buttons, SIGNAL(rejected()), dlg, SLOT(reject()));

    if (QDialog::Accepted == dlg->exec())
    {
        if (templateChBox->checkState() == Qt::Checked) ret |= CHECKED_TEMPLATE;
        if (groupChBox->checkState() == Qt::Checked) ret |= CHECKED_GROUP;
        if (propertiesChBox->checkState() == Qt::Checked) ret |= CHECKED_PROPERTIES;
        if (texturesChBox->checkState() == Qt::Checked) ret |= CHECKED_TEXTURES;
    }

    delete dlg;
    return ret;
}

void MaterialEditor::StoreMaterialToPreset(DAVA::NMaterial* material, DAVA::KeyedArchive* archive, DAVA::SerializationContext* context)
{
    const DAVA::InspInfo* info = material->GetTypeInfo();

	ScopedPtr<DAVA::KeyedArchive> texturesArchive(new DAVA::KeyedArchive());
	ScopedPtr<DAVA::KeyedArchive> propertiesArchive(new DAVA::KeyedArchive());
	ScopedPtr<DAVA::KeyedArchive> flagsArchive(new DAVA::KeyedArchive());

    const DAVA::InspMember* materialMember = info->Member(FastName("localTextures"));
    if ((nullptr != materialMember) && (nullptr != materialMember->Dynamic()))
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
					String textureRelativePath = texturePath.GetRelativePathname(context->GetScenePath());
					if (textureRelativePath.size() > 0)
					{
						texturesArchive->SetString(texName.c_str(), textureRelativePath);
					}
				}
			}
		}
	}

    materialMember = info->Member(FastName("localFlags"));
    if ((nullptr != materialMember) && (nullptr != materialMember->Dynamic()))
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

    materialMember = info->Member(FastName("localProperties"));
    if ((nullptr != materialMember) && (nullptr != materialMember->Dynamic()))
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
				auto dataSize = sizeof(float32) * DAVA::ShaderDescriptor::CalculateDataSize(propertyType, 1);

				ScopedPtr<KeyedArchive> prop(new KeyedArchive());
				prop->SetUInt32("type", static_cast<uint32>(propertyType));
				prop->SetUInt32("size", arraySize);
				prop->SetByteArray("data", reinterpret_cast<const uint8*>(propertyValue), dataSize);
				propertiesArchive->SetArchive(propertyName.c_str(), prop);
			}
		}
	}

	archive->SetArchive("flags", flagsArchive);
	archive->SetArchive("textures", texturesArchive);
	archive->SetArchive("properties", propertiesArchive);

	auto fxName = material->GetLocalFXName();
	if (fxName.IsValid())
		archive->SetString("fxname", fxName.c_str());
	
	auto qualityGroup = material->GetQualityGroup();
	if (qualityGroup.IsValid())
		archive->SetString("group", qualityGroup.c_str());
}

void MaterialEditor::UpdateMaterialFromPresetWithOptions(DAVA::NMaterial* material, DAVA::KeyedArchive* preset,
	DAVA::SerializationContext* context, uint32 options)
{
	if ((options & CHECKED_GROUP) && preset->IsKeyExists("group"))
	{
		FastName qualityGroup(preset->GetString("materialGroup").c_str());
		material->SetQualityGroup(qualityGroup);
	}

	if ((options & CHECKED_TEMPLATE) && preset->IsKeyExists("fxname"))
	{
		FastName fxName(preset->GetString("fxname").c_str());
		material->SetFXName(fxName);
	}

	if ((options & CHECKED_PROPERTIES) && preset->IsKeyExists("flags"))
	{
		const auto flags = preset->GetArchive("flags")->GetArchieveData();
		for (const auto& fm : flags)
		{
			if (material->HasLocalFlag(FastName(fm.first)))
				material->SetFlag(FastName(fm.first), fm.second->AsInt32());
			else
				material->AddFlag(FastName(fm.first), fm.second->AsInt32());
		}
	}

    if ((options & CHECKED_PROPERTIES) && preset->IsKeyExists("properties"))
    {
        const auto properties = preset->GetArchive("properties")->GetArchieveData();
        for (const auto& pm : properties)
        {
            DVASSERT(VariantType::TYPE_KEYED_ARCHIVE == pm.second->type);

            FastName propName(pm.first);
			KeyedArchive* propertyArchive = pm.second->AsKeyedArchive();
			rhi::ShaderProp::Type propType = static_cast<rhi::ShaderProp::Type>(propertyArchive->GetUInt32("type"));
			uint32 propSize = propertyArchive->GetUInt32("size");
			const float32* propData = reinterpret_cast<const float32*>(propertyArchive->GetByteArray("data"));
            
			if (material->HasLocalProperty(propName))
			{
				auto existingType = material->GetLocalPropType(propName);
				auto existingSize = material->GetLocalPropArraySize(propName);

				// do we need this check?
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

    if ((options & CHECKED_TEXTURES) && preset->IsKeyExists("textures"))
    {
        const auto& texturesMap = preset->GetArchive("textures")->GetArchieveData();
        for (const auto& tm : texturesMap)
        {
			FastName textureName(tm.first);
			auto pathToTexture = context->GetScenePath() + tm.second->AsString();
			auto texture = Texture::CreateFromFile(pathToTexture);
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
}
