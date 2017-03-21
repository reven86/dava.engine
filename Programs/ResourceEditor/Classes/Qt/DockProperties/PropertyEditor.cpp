#include "Entity/Component.h"
#include "Main/mainwindow.h"

#include <QPushButton>
#include <QFile>
#include <QTextStream>

#include "DockProperties/PropertyEditor.h"
#include "MaterialEditor/MaterialEditor.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataIntrospection.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataDavaVariant.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataDavaKeyedArchive.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataKeyedArchiveMember.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataInspColl.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataInspMember.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataInspDynamic.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataMetaObject.h"
#include "Tools/QtPropertyEditor/QtPropertyDataValidator/HeightmapValidator.h"
#include "Tools/QtPropertyEditor/QtPropertyDataValidator/TexturePathValidator.h"
#include "Tools/QtPropertyEditor/QtPropertyDataValidator/ScenePathValidator.h"
#include "Commands2/Base/RECommandNotificationObject.h"
#include "Commands2/MetaObjModifyCommand.h"
#include "Commands2/InspMemberModifyCommand.h"
#include "Commands2/ConvertToShadowCommand.h"
#include "Commands2/DeleteRenderBatchCommand.h"
#include "Commands2/CloneLastBatchCommand.h"
#include "Commands2/AddComponentCommand.h"
#include "Commands2/RemoveComponentCommand.h"
#include "Commands2/RebuildTangentSpaceCommand.h"
#include "Commands2/ParticleEditorCommands.h"
#include "Commands2/SoundComponentEditCommands.h"
#include "Commands2/ConvertPathCommands.h"
#include "Commands2/ConvertToBillboardCommand.h"

#include "Classes/Application/REGlobal.h"
#include "Classes/Project/ProjectManagerData.h"

#include "TArc/DataProcessing/DataContext.h"
#include "Classes/Selection/Selection.h"
#include "Classes/Selection/SelectionData.h"

#include "Classes/Settings/SettingsManager.h"
#include "PropertyEditorStateHelper.h"

#include "ActionComponentEditor.h"
#include "SoundComponentEditor/SoundComponentEditor.h"
#include "Deprecated/SceneValidator.h"
#include "Tools/PathDescriptor/PathDescriptor.h"
#include "Scene/System/PathSystem.h"

#include "QtTools/Updaters/LazyUpdater.h"
#include "QtTools/WidgetHelpers/SharedIcon.h"

#include "TArc/Core/FieldBinder.h"

#include "Render/Highlevel/RenderObject.h"
#include "Scene3D/Components/Controller/SnapToLandscapeControllerComponent.h"

PropertyEditor::PropertyEditor(QWidget* parent /* = 0 */, bool connectToSceneSignals /*= true*/)
    : QtPropertyEditor(parent)
    , treeStateHelper(this, curModel)
{
    DAVA::Function<void()> fn(this, &PropertyEditor::ResetProperties);
    propertiesUpdater = new LazyUpdater(fn, this);
    selectionUpdater = new LazyUpdater(DAVA::MakeFunction(this, &PropertyEditor::UpdateSelectionLazy), this);

    if (connectToSceneSignals)
    {
        QObject::connect(SceneSignals::Instance(), &SceneSignals::Activated, this, &PropertyEditor::sceneActivated);
        QObject::connect(SceneSignals::Instance(), &SceneSignals::Deactivated, this, &PropertyEditor::sceneDeactivated);
        QObject::connect(SceneSignals::Instance(), &SceneSignals::CommandExecuted, this, &PropertyEditor::CommandExecuted);
        QObject::connect(SceneSignals::Instance(), &SceneSignals::ThemeChanged, this, &PropertyEditor::ResetProperties);

        selectionFieldBinder.reset(new DAVA::TArc::FieldBinder(REGlobal::GetAccessor()));
        {
            DAVA::TArc::FieldDescriptor fieldDescr;
            fieldDescr.type = DAVA::ReflectedTypeDB::Get<SelectionData>();
            fieldDescr.fieldName = DAVA::FastName(SelectionData::selectionPropertyName);
            selectionFieldBinder->BindField(fieldDescr, [this](const DAVA::Any&) { selectionUpdater->Update(); });
        }
    }

    posSaver.Attach(this, "DocPropetyEditor");

    DAVA::VariantType v = posSaver.LoadValue("splitPos");
    if (v.GetType() == DAVA::VariantType::TYPE_INT32)
    {
        header()->resizeSection(0, v.AsInt32());
    }

    SetUpdateTimeout(5000);
    SetEditTracking(true);
    setMouseTracking(true);

    LoadScheme("~doc:/PropEditorDefault.scheme");
}

PropertyEditor::~PropertyEditor()
{
    DAVA::VariantType v(header()->sectionSize(0));
    posSaver.SaveValue("splitPos", v);

    ClearCurrentNodes();
}

void PropertyEditor::Init(Ui::MainWindow* mainWindowUi, const std::shared_ptr<GlobalOperations>& globalOperations_)
{
    globalOperations = globalOperations_;
    connect(mainWindowUi->actionAddActionComponent, &QAction::triggered, this, &PropertyEditor::OnAddActionComponent);
    connect(mainWindowUi->actionAddQualitySettingsComponent, &QAction::triggered, this, &PropertyEditor::OnAddModelTypeComponent);
    connect(mainWindowUi->actionAddStaticOcclusionComponent, &QAction::triggered, this, &PropertyEditor::OnAddStaticOcclusionComponent);
    connect(mainWindowUi->actionAddSoundComponent, &QAction::triggered, this, &PropertyEditor::OnAddSoundComponent);
    connect(mainWindowUi->actionAddWaveComponent, &QAction::triggered, this, &PropertyEditor::OnAddWaveComponent);
    connect(mainWindowUi->actionAddSkeletonComponent, &QAction::triggered, this, &PropertyEditor::OnAddSkeletonComponent);
    connect(mainWindowUi->actionAddPathComponent, &QAction::triggered, this, &PropertyEditor::OnAddPathComponent);
    connect(mainWindowUi->actionAddRotationComponent, &QAction::triggered, this, &PropertyEditor::OnAddRotationControllerComponent);
    connect(mainWindowUi->actionAddSnapToLandscapeComponent, &QAction::triggered, this, &PropertyEditor::OnAddSnapToLandscapeControllerComponent);
    connect(mainWindowUi->actionAddWASDComponent, &QAction::triggered, this, &PropertyEditor::OnAddWASDControllerComponent);
    connect(mainWindowUi->actionAddVisibilityComponent, &QAction::triggered, this, &PropertyEditor::OnAddVisibilityComponent);
    connect(mainWindowUi->actionAddLodComponent, &QAction::triggered, this, &PropertyEditor::OnAddLodComponent);
}

void PropertyEditor::SetEntities(const SelectableGroup* selected)
{
    SelectableGroup group = ExtractEntities(selected);
    if (group != curNodes)
    {
        ClearCurrentNodes();
        curNodes = group;
        ResetProperties();
        SaveScheme("~doc:/PropEditorDefault.scheme");
    }
}

void PropertyEditor::SetViewMode(eViewMode mode)
{
    if (viewMode != mode)
    {
        viewMode = mode;
        ResetProperties();
    }
}

PropertyEditor::eViewMode PropertyEditor::GetViewMode() const
{
    return viewMode;
}

void PropertyEditor::SetFavoritesEditMode(bool set)
{
    if (favoritesEditMode != set)
    {
        favoritesEditMode = set;
        ResetProperties();
    }
}

bool PropertyEditor::GetFavoritesEditMode() const
{
    return favoritesEditMode;
}

void PropertyEditor::ClearCurrentNodes()
{
    curNodes.Clear();
}

void PropertyEditor::AddEntityProperties(DAVA::Entity* node, std::unique_ptr<QtPropertyData>& root,
                                         std::unique_ptr<QtPropertyData>& curEntityData, bool isFirstInList)
{
    // add info about components
    for (int ic = 0; ic < DAVA::Component::COMPONENT_COUNT; ic++)
    {
        const int nComponents = node->GetComponentCount(ic);
        for (int cidx = 0; cidx < nComponents; cidx++)
        {
            DAVA::Component* component = node->GetComponent(ic, cidx);
            if (component)
            {
                const DAVA::InspInfo* componentInspInfo = component->GetTypeInfo();
                std::unique_ptr<QtPropertyData> componentData(CreateInsp(componentInspInfo->Name(), component, componentInspInfo));
                PropEditorUserData* userData = GetUserData(componentData.get());
                userData->entity = node;

                bool isRemovable = true;
                switch (component->GetType())
                {
                case DAVA::Component::STATIC_OCCLUSION_DEBUG_DRAW_COMPONENT:
                case DAVA::Component::DEBUG_RENDER_COMPONENT:
                case DAVA::Component::TRANSFORM_COMPONENT:
                case DAVA::Component::CUSTOM_PROPERTIES_COMPONENT: // Disable removing, because custom properties are created automatically
                case DAVA::Component::WAYPOINT_COMPONENT: // disable remove, b/c waypoint entity doesn't make sence without waypoint component
                case DAVA::Component::EDGE_COMPONENT: // disable remove, b/c edge has to be removed directly from scene only
                    isRemovable = false;
                    break;
                }

                if (isRemovable)
                {
                    QtPropertyToolButton* deleteButton = CreateButton(componentData.get(), SharedIcon(":/QtIcons/remove.png"), "Remove Component");
                    deleteButton->setObjectName("RemoveButton");
                    deleteButton->setEnabled(true);
                    QObject::connect(deleteButton, SIGNAL(clicked()), this, SLOT(OnRemoveComponent()));
                }

                if (isFirstInList)
                {
                    root->ChildAdd(std::move(componentData));
                }
                else
                {
                    root->MergeChild(std::move(componentData));
                }
            }
        }
    }
}

void PropertyEditor::UpdateSelectionLazy()
{
    const SelectableGroup& selection = Selection::GetSelection();
    SetEntities(&selection);
}

SelectableGroup PropertyEditor::ExtractEntities(const SelectableGroup* selection)
{
    SelectableGroup entities;
    if (selection == nullptr || selection->IsEmpty())
        return entities;

    for (auto entity : selection->ObjectsOfType<DAVA::Entity>())
    {
        GetOrCreateCustomProperties(entity);
    }

    for (const auto& item : selection->GetContent())
    {
        entities.Add(item.GetContainedObject(), item.GetBoundingBox());
    }

    return entities;
}

void PropertyEditor::ResetProperties()
{
    // Store the current Property Editor Tree state before switching to the new node.
    // Do not clear the current states map - we are using one storage to share opened
    // Property Editor nodes between the different Scene Nodes.
    treeStateHelper.SaveTreeViewState(false);

    RemovePropertyAll();
    favoriteGroup = NULL;

    if (!curNodes.IsEmpty())
    {
        // create data tree, but don't add it to the property editor
        std::unique_ptr<QtPropertyData> root(new QtPropertyData(DAVA::FastName("root")));

        // add info about current entities
        bool isFirstItem = true;
        for (const auto& item : curNodes.GetContent())
        {
            auto node = item.GetContainedObject();

            const DAVA::InspInfo* inspInfo = node->GetTypeInfo();
            std::unique_ptr<QtPropertyData> curEntityData(CreateInsp(inspInfo->Name(), node, inspInfo));

            if (item.CanBeCastedTo<DAVA::Entity>())
            {
                PropEditorUserData* userData = GetUserData(curEntityData.get());
                userData->entity = item.AsEntity();
                root->MergeChild(std::move(curEntityData));

                AddEntityProperties(userData->entity, root, curEntityData, isFirstItem);
            }
            else
            {
                root->MergeChild(std::move(curEntityData));
            }
            isFirstItem = false;
        }

        ApplyFavorite(root.get());
        ApplyModeFilter(root.get());
        ApplyModeFilter(favoriteGroup);
        ApplyCustomExtensions(root.get());

        DAVA::Vector<std::unique_ptr<QtPropertyData>> properies;
        root->ChildrenExtract(properies);
        for (std::unique_ptr<QtPropertyData>& item : properies)
        {
            ApplyStyle(item.get(), QtPropertyEditor::HEADER_STYLE);
        }
        AppendProperties(std::move(properies));
        FinishTreeCreation();
    }

    // Restore back the tree view state from the shared storage.
    if (!treeStateHelper.IsTreeStateStorageEmpty())
    {
        treeStateHelper.RestoreTreeViewState();
    }
    else
    {
        // Expand the root elements as default value.
        expandToDepth(0);
    }
}

void PropertyEditor::ApplyModeFilter(QtPropertyData* parent)
{
    if (NULL != parent)
    {
        for (int i = 0; i < parent->ChildCount(); ++i)
        {
            bool toBeRemove = false;
            bool scanChilds = true;
            QtPropertyData* data = parent->ChildGet(i);

            // show only editable items and favorites
            if (viewMode == VIEW_NORMAL)
            {
                if (!data->IsEditable())
                {
                    toBeRemove = true;
                }
            }
            // show all editable/viewable items
            else if (viewMode == VIEW_ADVANCED)
            {
            }
            // show only favorite items
            else if (viewMode == VIEW_FAVORITES_ONLY)
            {
                QtPropertyData* favorite = NULL;
                PropEditorUserData* userData = GetUserData(data);

                if (userData->type == PropEditorUserData::ORIGINAL)
                {
                    toBeRemove = true;
                    favorite = userData->associatedData;
                }
                else if (userData->type == PropEditorUserData::COPY)
                {
                    favorite = data;
                    scanChilds = false;
                }

                if (favorite != nullptr && favorite->GetUserData() != nullptr)
                {
                    // remove from favorite data back link to the original data
                    // because original data will be removed from properties
                    static_cast<PropEditorUserData*>(favorite->GetUserData())->associatedData = nullptr;
                }
            }

            if (toBeRemove)
            {
                parent->ChildRemove(data);
                i--;
            }
            else if (scanChilds)
            {
                // apply mode to data childs
                ApplyModeFilter(data);
            }
        }
    }
}

void PropertyEditor::ApplyFavorite(QtPropertyData* data)
{
    if (NULL != data)
    {
        if (scheme.contains(data->GetPath()))
        {
            SetFavorite(data, true);
        }

        // go through childs
        for (int i = 0; i < data->ChildCount(); ++i)
        {
            ApplyFavorite(data->ChildGet(i));
        }
    }
}

void PropertyEditor::ApplyCustomExtensions(QtPropertyData* data)
{
    if (NULL != data)
    {
        const DAVA::MetaInfo* meta = data->MetaInfo();
        const bool isSingleSelection = data->GetMergedItemCount() == 0;

        if (NULL != meta)
        {
            if (DAVA::MetaInfo::Instance<DAVA::ActionComponent>() == meta)
            {
                // Add optional button to edit action component
                QtPropertyToolButton* editActions = CreateButton(data, SharedIcon(":/QtIcons/settings.png"), "Edit action component");
                editActions->setEnabled(isSingleSelection);
                QObject::connect(editActions, SIGNAL(clicked()), this, SLOT(ActionEditComponent()));
            }
            else if (DAVA::MetaInfo::Instance<DAVA::SoundComponent>() == meta)
            {
                QtPropertyToolButton* editSound = CreateButton(data, SharedIcon(":/QtIcons/settings.png"), "Edit sound component");
                editSound->setAutoRaise(true);
                QObject::connect(editSound, SIGNAL(clicked()), this, SLOT(ActionEditSoundComponent()));
            }
            else if (DAVA::MetaInfo::Instance<DAVA::WaveComponent>() == meta)
            {
                QtPropertyToolButton* triggerWave = data->AddButton();
                triggerWave->setIcon(SharedIcon(":/QtIcons/clone.png"));
                triggerWave->setAutoRaise(true);

                QObject::connect(triggerWave, SIGNAL(clicked()), this, SLOT(OnTriggerWaveComponent()));
            }
            else if (DAVA::MetaInfo::Instance<DAVA::RenderObject>() == meta)
            {
                QtPropertyDataIntrospection* introData = dynamic_cast<QtPropertyDataIntrospection*>(data);
                if (NULL != introData)
                {
                    DAVA::RenderObject* renderObject = (DAVA::RenderObject*)introData->object;
                    if (SceneValidator::IsObjectHasDifferentLODsCount(renderObject))
                    {
                        QtPropertyToolButton* cloneBatches = CreateButton(data, SharedIcon(":/QtIcons/clone_batches.png"), "Clone batches for LODs correction");
                        cloneBatches->setEnabled(isSingleSelection);
                        QObject::connect(cloneBatches, SIGNAL(clicked()), this, SLOT(CloneRenderBatchesToFixSwitchLODs()));
                    }

                    if ((renderObject->GetType() == DAVA::RenderObject::TYPE_MESH) ||
                        (renderObject->GetType() == DAVA::RenderObject::TYPE_RENDEROBJECT))
                    {
                        QtPropertyToolButton* convertRenderObject = CreateButton(data, SharedIcon(":/QtIcons/sphere.png"), "Make billboard");
                        QObject::connect(convertRenderObject, &QtPropertyToolButton::clicked, this, &PropertyEditor::OnConvertRenderObjectToBillboard);
                    }
                }
            }
            else if (DAVA::MetaInfo::Instance<DAVA::RenderBatch>() == meta)
            {
                QtPropertyToolButton* deleteButton = CreateButton(data, SharedIcon(":/QtIcons/remove.png"), "Delete RenderBatch");
                deleteButton->setEnabled(isSingleSelection);
                QObject::connect(deleteButton, SIGNAL(clicked()), this, SLOT(DeleteRenderBatch()));

                QtPropertyDataIntrospection* introData = dynamic_cast<QtPropertyDataIntrospection*>(data);
                if (NULL != introData)
                {
                    DAVA::RenderBatch* batch = (DAVA::RenderBatch*)introData->object;
                    if (batch != NULL)
                    {
                        DAVA::RenderObject* ro = batch->GetRenderObject();
                        if (ro != NULL && ConvertToShadowCommand::CanConvertBatchToShadow(batch))
                        {
                            QtPropertyToolButton* convertButton = CreateButton(data, SharedIcon(":/QtIcons/shadow.png"), "Convert To ShadowVolume");
                            convertButton->setEnabled(isSingleSelection);
                            connect(convertButton, SIGNAL(clicked()), this, SLOT(ConvertToShadow()));
                        }

                        DAVA::PolygonGroup* group = batch->GetPolygonGroup();
                        if (group != NULL)
                        {
                            bool isRebuildTsEnabled = true;
                            const DAVA::int32 requiredVertexFormat = (DAVA::EVF_TEXCOORD0 | DAVA::EVF_NORMAL);
                            isRebuildTsEnabled &= (group->GetPrimitiveType() == rhi::PRIMITIVE_TRIANGLELIST);
                            isRebuildTsEnabled &= ((group->GetFormat() & requiredVertexFormat) == requiredVertexFormat);

                            if (isRebuildTsEnabled)
                            {
                                QtPropertyToolButton* rebuildTangentButton = CreateButton(data, SharedIcon(":/QtIcons/external.png"), "Rebuild tangent space");
                                rebuildTangentButton->setEnabled(isSingleSelection);
                                connect(rebuildTangentButton, SIGNAL(clicked()), this, SLOT(RebuildTangentSpace()));
                            }
                        }
                    }
                }
            }
            else if (DAVA::MetaInfo::Instance<DAVA::NMaterial>() == meta)
            {
                QtPropertyToolButton* goToMaterialButton = CreateButton(data, SharedIcon(":/QtIcons/3d.png"), "Edit material");
                goToMaterialButton->setEnabled(isSingleSelection);
                QObject::connect(goToMaterialButton, SIGNAL(clicked()), this, SLOT(ActionEditMaterial()));
            }
            else if (DAVA::MetaInfo::Instance<DAVA::FilePath>() == meta)
            {
                QString dataName(data->GetName().c_str());
                PathDescriptor* pathDescriptor = &PathDescriptor::descriptors[0];

                for (auto descriptor : PathDescriptor::descriptors)
                {
                    if (descriptor.pathName == dataName)
                    {
                        pathDescriptor = &descriptor;
                        break;
                    }
                }

                QtPropertyDataDavaVariant* variantData = static_cast<QtPropertyDataDavaVariant*>(data);
                QString defaultPath = GetDefaultFilePath();
                variantData->SetDefaultOpenDialogPath(defaultPath);
                variantData->SetOpenDialogFilter(pathDescriptor->fileFilter);

                QStringList pathList;
                pathList.append(defaultPath);

                switch (pathDescriptor->pathType)
                {
                case PathDescriptor::PATH_HEIGHTMAP:
                    variantData->SetValidator(new HeightMapValidator(pathList));
                    break;
                case PathDescriptor::PATH_TEXTURE:
                    variantData->SetValidator(new TexturePathValidator(pathList));
                    break;
                case PathDescriptor::PATH_IMAGE:
                case PathDescriptor::PATH_TEXTURE_SHEET:
                    variantData->SetValidator(new PathValidator(pathList));
                    break;
                case PathDescriptor::PATH_SCENE:
                    pathList.append(GetDefaultFilePath(false));
                    variantData->SetValidator(new ScenePathValidator(pathList));
                    break;

                default:
                    break;
                }
            }
            else if (DAVA::MetaInfo::Instance<DAVA::QualitySettingsComponent>() == meta)
            {
                DAVA::QualitySettingsSystem* qss = DAVA::QualitySettingsSystem::Instance();
                QtPropertyDataDavaVariant* modelTypeData = (QtPropertyDataDavaVariant*)data->ChildGet(DAVA::FastName("modelType"));
                if (NULL != modelTypeData)
                {
                    modelTypeData->AddAllowedValue(DAVA::VariantType(DAVA::FastName()), "Undefined");
                    for (int i = 0; i < qss->GetOptionsCount(); ++i)
                    {
                        modelTypeData->AddAllowedValue(DAVA::VariantType(qss->GetOptionName(i)));
                    }
                }

                QtPropertyDataDavaVariant* groupData = (QtPropertyDataDavaVariant*)data->ChildGet(DAVA::FastName("requiredGroup"));
                if (NULL != groupData)
                {
                    groupData->AddAllowedValue(DAVA::VariantType(DAVA::FastName()), "Undefined");
                    for (size_t i = 0; i < qss->GetMaterialQualityGroupCount(); ++i)
                    {
                        groupData->AddAllowedValue(DAVA::VariantType(qss->GetMaterialQualityGroupName(i)));
                    }
                }

                DAVA::FastName curGroup = groupData->GetVariantValue().AsFastName();

                QtPropertyDataDavaVariant* requiredQualityData = (QtPropertyDataDavaVariant*)data->ChildGet(DAVA::FastName("requiredQuality"));
                if (NULL != requiredQualityData)
                {
                    requiredQualityData->AddAllowedValue(DAVA::VariantType(DAVA::FastName()), "Undefined");
                    for (size_t i = 0; i < qss->GetMaterialQualityCount(curGroup); ++i)
                    {
                        requiredQualityData->AddAllowedValue(DAVA::VariantType(qss->GetMaterialQualityName(curGroup, i)));
                    }
                }
            }
        }

        // go through childs
        for (int i = 0; i < data->ChildCount(); ++i)
        {
            ApplyCustomExtensions(data->ChildGet(i));
        }
    }
}

QtPropertyData* PropertyEditor::CreateInsp(const DAVA::FastName& name, void* object, const DAVA::InspInfo* info)
{
    QtPropertyData* ret = NULL;

    if (NULL != info)
    {
        bool hasMembers = false;
        const DAVA::InspInfo* baseInfo = info;

        // check if there are any members in introspection
        while (NULL != baseInfo)
        {
            if (baseInfo->MembersCount() > 0)
            {
                hasMembers = true;
                break;
            }
            baseInfo = baseInfo->BaseInfo();
        }

        ret = new QtPropertyDataIntrospection(name, object, info, false);

        // add members is there are some
        // and if we allow to view such introspection type
        if (hasMembers && IsInspViewAllowed(info))
        {
            while (NULL != baseInfo)
            {
                for (int i = 0; i < baseInfo->MembersCount(); ++i)
                {
                    const DAVA::InspMember* member = baseInfo->Member(i);

                    /// In our old style property panel Components vector in Entity processed by external loop
                    /// When i appended introspected collection into Entity with components, our property panel started containing two subtree with components.
                    /// This code leave out inner extraction of components vector in Entity.
                    if (baseInfo->Type() == DAVA::MetaInfo::Instance<DAVA::Entity>() && member->Name() == DAVA::FastName("components"))
                        continue;

                    std::unique_ptr<QtPropertyData> memberData(CreateInspMember(member->Name(), object, member));
                    ret->ChildAdd(std::move(memberData));
                }

                baseInfo = baseInfo->BaseInfo();
            }
        }
    }

    return ret;
}

QtPropertyData* PropertyEditor::CreateInspMember(const DAVA::FastName& name, void* object, const DAVA::InspMember* member)
{
    QtPropertyData* ret = NULL;

    if (NULL != member && (member->Flags() & DAVA::I_VIEW))
    {
        void* momberObject = member->Data(object);
        const DAVA::InspInfo* memberIntrospection = member->Type()->GetIntrospection(momberObject);
        bool isKeyedArchive = (member->Type() == DAVA::MetaInfo::Instance<DAVA::KeyedArchive*>());

        if (NULL != memberIntrospection && !isKeyedArchive)
        {
            ret = CreateInsp(name, momberObject, memberIntrospection);
        }
        else
        {
            if (member->Collection() && !isKeyedArchive)
            {
                ret = CreateInspCollection(name, momberObject, member->Collection());
            }
            else
            {
                ret = QtPropertyDataIntrospection::CreateMemberData(name, object, member);
            }
        }
    }

    return ret;
}

QtPropertyData* PropertyEditor::CreateInspCollection(const DAVA::FastName& name, void* object, const DAVA::InspColl* collection)
{
    QtPropertyData* ret = new QtPropertyDataInspColl(name, object, collection, false);

    if (NULL != collection && collection->Size(object) > 0)
    {
        int index = 0;
        DAVA::MetaInfo* valueType = collection->ItemType();
        DAVA::InspColl::Iterator i = collection->Begin(object);
        while (NULL != i)
        {
            DAVA::FastName childName(std::to_string(index));
            if (NULL != valueType->GetIntrospection())
            {
                void* itemObject = collection->ItemData(i);
                const DAVA::InspInfo* itemInfo = valueType->GetIntrospection(itemObject);

                std::unique_ptr<QtPropertyData> inspData(CreateInsp(childName, itemObject, itemInfo));
                ret->ChildAdd(std::move(inspData));
            }
            else
            {
                if (!valueType->IsPointer())
                {
                    std::unique_ptr<QtPropertyData> childData(new QtPropertyDataMetaObject(childName, collection->ItemPointer(i), valueType));
                    ret->ChildAdd(std::move(childData));
                }
                else
                {
                    DAVA::FastName localChildName = childName;
                    if (collection->ItemKeyType() == DAVA::MetaInfo::Instance<DAVA::FastName>())
                    {
                        localChildName = *reinterpret_cast<const DAVA::FastName*>(collection->ItemKeyData(i));
                    }
                    QString s;
                    std::unique_ptr<QtPropertyData> childData(new QtPropertyData(localChildName, s.sprintf("[%p] Pointer", collection->ItemData(i))));
                    childData->SetEnabled(false);
                    ret->ChildAdd(std::move(childData));
                }
            }

            index++;
            i = collection->Next(i);
        }
    }

    return ret;
}

QtPropertyData* PropertyEditor::CreateClone(QtPropertyData* original)
{
    QtPropertyDataIntrospection* inspData = dynamic_cast<QtPropertyDataIntrospection*>(original);
    if (NULL != inspData)
    {
        return CreateInsp(original->GetName(), inspData->object, inspData->info);
    }

    QtPropertyDataInspMember* memberData = dynamic_cast<QtPropertyDataInspMember*>(original);
    if (NULL != memberData)
    {
        return CreateInspMember(original->GetName(), memberData->object, memberData->member);
    }

    QtPropertyDataInspDynamic* memberDymanic = dynamic_cast<QtPropertyDataInspDynamic*>(original);
    if (NULL != memberData)
    {
        return CreateInspMember(original->GetName(), memberDymanic->ddata.object, memberDymanic->dynamicInfo->GetMember());
    }

    QtPropertyDataMetaObject* metaData = dynamic_cast<QtPropertyDataMetaObject*>(original);
    if (NULL != metaData)
    {
        return new QtPropertyDataMetaObject(original->GetName(), metaData->object, metaData->meta);
    }

    QtPropertyDataInspColl* memberCollection = dynamic_cast<QtPropertyDataInspColl*>(original);
    if (NULL != memberCollection)
    {
        return CreateInspCollection(original->GetName(), memberCollection->object, memberCollection->collection);
    }

    QtPropertyDataDavaKeyedArcive* memberArch = dynamic_cast<QtPropertyDataDavaKeyedArcive*>(original);
    if (NULL != memberArch)
    {
        return new QtPropertyDataDavaKeyedArcive(original->GetName(), memberArch->archive);
    }

    QtPropertyKeyedArchiveMember* memberArchMem = dynamic_cast<QtPropertyKeyedArchiveMember*>(original);
    if (NULL != memberArchMem)
    {
        return new QtPropertyKeyedArchiveMember(original->GetName(), memberArchMem->archive, memberArchMem->key);
    }

    QtPropertyData* retValue = new QtPropertyData(original->GetName(), QString(original->GetName().c_str()));
    retValue->SetFlags(original->GetFlags());
    return retValue;
}

void PropertyEditor::sceneActivated(SceneEditor2* scene)
{
    const SelectableGroup& selection = Selection::GetSelection();
    SetEntities(&selection);
}

void PropertyEditor::sceneDeactivated(SceneEditor2* scene)
{
    SetEntities(NULL);
}

template <typename T>
bool IsCurNodesContainsEntityFromCommand(const DAVA::Command* command, const SelectableGroup& curNodes)
{
    static_assert(std::is_base_of<DAVA::Command, T>::value, "it work only with commands, derived from DAVA::Command");
    const T* derivedCommand = static_cast<const T*>(command);
    DAVA::Entity* entity = derivedCommand->GetEntity();
    if ((entity == nullptr) || curNodes.ContainsObject(entity))
    {
        return true;
    }
    return false;
}

void PropertyEditor::CommandExecuted(SceneEditor2* scene, const RECommandNotificationObject& commandNotification)
{
    if (commandNotification.IsEmpty())
    {
        return;
    }

    std::function<bool(const RECommand*)> shouldResetPanel;
    shouldResetPanel = [this, &shouldResetPanel](const RECommand* cmd)
    {
        const DAVA::uint32 cmdID = cmd->GetID();
        if (cmdID == CMDID_COMPONENT_ADD)
        {
            return IsCurNodesContainsEntityFromCommand<AddComponentCommand>(cmd, curNodes);
        }
        else if (cmdID == CMDID_COMPONENT_REMOVE)
        {
            return IsCurNodesContainsEntityFromCommand<RemoveComponentCommand>(cmd, curNodes);
        }
        else if (cmdID == CMDID_CONVERT_TO_SHADOW)
        {
            return IsCurNodesContainsEntityFromCommand<ConvertToShadowCommand>(cmd, curNodes);
        }
        else if (cmdID == CMDID_PARTICLE_EMITTER_LOAD_FROM_YAML)
        {
            return true; //there is no "GetEntity" for this command
        }
        else if (cmdID == CMDID_SOUND_ADD_EVENT)
        {
            return IsCurNodesContainsEntityFromCommand<AddSoundEventCommand>(cmd, curNodes);
        }
        else if (cmdID == CMDID_SOUND_REMOVE_EVENT)
        {
            return IsCurNodesContainsEntityFromCommand<RemoveSoundEventCommand>(cmd, curNodes);
        }
        else if (cmdID == CMDID_DELETE_RENDER_BATCH)
        {
            return IsCurNodesContainsEntityFromCommand<DeleteRenderBatchCommand>(cmd, curNodes);
        }
        else if (cmdID == CMDID_CLONE_LAST_BATCH)
        {
            return true; //there is no "GetEntity" for this command
        }
        else if (cmdID == CMDID_EXPAND_PATH)
        {
            return IsCurNodesContainsEntityFromCommand<ExpandPathCommand>(cmd, curNodes);
        }
        else if (cmdID == CMDID_COLLAPSE_PATH)
        {
            return IsCurNodesContainsEntityFromCommand<CollapsePathCommand>(cmd, curNodes);
        }
        else if (cmdID == CMDID_CONVERT_TO_BILLBOARD)
        {
            return true;
        }
        return false;
    };

    bool shouldReset = false;
    if (commandNotification.batch != nullptr)
    {
        for (DAVA::uint32 i = 0, count = commandNotification.batch->Size(); i < count && !shouldReset; ++i)
        {
            shouldReset = shouldResetPanel(commandNotification.batch->GetCommand(i));
        }
    }
    else
    {
        shouldReset = shouldResetPanel(commandNotification.command);
    }

    if (shouldReset)
    {
        propertiesUpdater->Update();
    }
    else
    {
        OnUpdateTimeout();
    }
}

void PropertyEditor::OnItemEdited(const QModelIndex& index) // TODO: fix undo/redo
{
    QtPropertyEditor::OnItemEdited(index);

    SceneEditor2* curScene = sceneHolder.GetScene();
    if (curScene == nullptr)
        return;

    QtPropertyData* propData = GetProperty(index);
    if (nullptr != propData)
    {
        const int nMerged = propData->GetMergedItemCount();
        curScene->BeginBatch("Edit properties", nMerged + 1);

        curScene->Exec(propData->CreateLastCommand());

        propData->ForeachMergedItem([&curScene](QtPropertyData* item) {
            curScene->Exec(item->CreateLastCommand());
            return true;
        });

        curScene->EndBatch();
    }

    // this code it used to reload QualitySettingComponent field, when some changes made by user
    // because of QualitySettingComponent->requiredQuality directly depends from QualitySettingComponent->requiredGroup
    if (propData->GetName() == DAVA::FastName("requiredGroup"))
    {
        QtPropertyDataDavaVariant* requiredQualityData = (QtPropertyDataDavaVariant*)propData->Parent()->ChildGet(DAVA::FastName("requiredQuality"));
        if (NULL != requiredQualityData)
        {
            requiredQualityData->ClearAllowedValues();

            DAVA::FastName curGroup = ((QtPropertyDataDavaVariant*)propData)->GetVariantValue().AsFastName();
            requiredQualityData->AddAllowedValue(DAVA::VariantType(DAVA::FastName()), "Undefined");
            for (size_t i = 0; i < DAVA::QualitySettingsSystem::Instance()->GetMaterialQualityCount(curGroup); ++i)
            {
                requiredQualityData->AddAllowedValue(DAVA::VariantType(DAVA::QualitySettingsSystem::Instance()->GetMaterialQualityName(curGroup, i)));
            }
        }

        propData->Parent()->EmitDataChanged(QtPropertyData::VALUE_SOURCE_CHANGED);
    }
}

void PropertyEditor::mouseReleaseEvent(QMouseEvent* event)
{
    bool skipEvent = false;
    QModelIndex index = indexAt(event->pos());

    // handle favorite state toggle for item under mouse
    if (favoritesEditMode && index.parent().isValid() && index.column() == 0)
    {
        QRect rect = visualRect(index);
        rect.setX(0);
        rect.setWidth(16);

        if (rect.contains(event->pos()))
        {
            QtPropertyData* data = GetProperty(index);
            if (NULL != data && !IsParentFavorite(data))
            {
                SetFavorite(data, !IsFavorite(data));
                skipEvent = true;
            }
        }
    }

    if (!skipEvent)
    {
        QtPropertyEditor::mouseReleaseEvent(event);
    }
}

void PropertyEditor::drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    // custom draw for favorites edit mode
    QStyleOptionViewItemV4 opt = option;
    if (index.parent().isValid() && favoritesEditMode)
    {
        QtPropertyData* data = GetProperty(index);
        if (NULL != data)
        {
            if (!IsParentFavorite(data))
            {
                if (IsFavorite(data))
                {
                    SharedIcon(":/QtIcons/star.png").paint(painter, opt.rect.x(), opt.rect.y(), 16, opt.rect.height());
                }
                else
                {
                    SharedIcon(":/QtIcons/star_empty.png").paint(painter, opt.rect.x(), opt.rect.y(), 16, opt.rect.height());
                }
            }
        }
    }

    QtPropertyEditor::drawRow(painter, opt, index);
}

void PropertyEditor::ActionEditComponent()
{
    if (curNodes.GetSize() == 1)
    {
        DAVA::Entity* node = curNodes.GetFirst().AsEntity();
        if (node == nullptr)
            return;

        ActionComponentEditor editor(this);
        editor.SetComponent((DAVA::ActionComponent*)node->GetComponent(DAVA::Component::ACTION_COMPONENT));
        editor.exec();

        const SelectableGroup& selection = Selection::GetSelection();
        SetEntities(&selection);

        if (editor.IsModified())
        {
            SceneEditor2* curScene = sceneHolder.GetScene();
            curScene->SetChanged();
        }
    }
}

void PropertyEditor::ConvertToShadow()
{
    QtPropertyToolButton* btn = dynamic_cast<QtPropertyToolButton*>(QObject::sender());
    if (btn == nullptr)
        return;

    QtPropertyDataIntrospection* data = dynamic_cast<QtPropertyDataIntrospection*>(btn->GetPropertyData());
    if (data == nullptr)
        return;

    SceneEditor2* curScene = sceneHolder.GetScene();
    if (curScene == nullptr)
        return;

    auto findEntityFn = [this](DAVA::RenderBatch* batch) -> DAVA::Entity* {
        DVASSERT(batch);
        DAVA::RenderObject* renderObject = batch->GetRenderObject();
        for (auto entity : curNodes.ObjectsOfType<DAVA::Entity>())
        {
            if (GetRenderObject(entity) == renderObject)
            {
                return entity;
            }
        }
        DVASSERT(0, "UNABLE TO FIND RENDER BATCH");
        return nullptr;
    };

    DAVA::RenderBatch* batch = reinterpret_cast<DAVA::RenderBatch*>(data->object);
    curScene->BeginBatch("ConvertToShadow batch", 1);
    curScene->Exec(std::unique_ptr<DAVA::Command>(new ConvertToShadowCommand(findEntityFn(batch), batch)));
    data->ForeachMergedItem([&findEntityFn, curScene](QtPropertyData* item)
                            {
                                QtPropertyDataIntrospection* dynamicData = dynamic_cast<QtPropertyDataIntrospection*>(item);
                                if (dynamicData != nullptr)
                                {
                                    DAVA::RenderBatch* batch = reinterpret_cast<DAVA::RenderBatch*>(dynamicData->object);
                                    curScene->Exec(std::unique_ptr<DAVA::Command>(new ConvertToShadowCommand(findEntityFn(batch), batch)));
                                }
                                return true;
                            });
    curScene->EndBatch();
}

void PropertyEditor::RebuildTangentSpace()
{
    QtPropertyToolButton* btn = dynamic_cast<QtPropertyToolButton*>(QObject::sender());
    if (nullptr != btn)
    {
        QtPropertyDataIntrospection* data = dynamic_cast<QtPropertyDataIntrospection*>(btn->GetPropertyData());
        SceneEditor2* curScene = sceneHolder.GetScene();
        if (nullptr != data && nullptr != curScene)
        {
            DAVA::RenderBatch* batch = static_cast<DAVA::RenderBatch*>(data->object);
            curScene->Exec(std::unique_ptr<DAVA::Command>(new RebuildTangentSpaceCommand(batch, true)));
        }
    }
}

void PropertyEditor::DeleteRenderBatch()
{
    // Code for removing several render batches
    QtPropertyToolButton* btn = dynamic_cast<QtPropertyToolButton*>(QObject::sender());
    if (nullptr != btn)
    {
        QtPropertyDataIntrospection* data = dynamic_cast<QtPropertyDataIntrospection*>(btn->GetPropertyData());
        SceneEditor2* curScene = sceneHolder.GetScene();
        DAVA::Entity* node = curNodes.GetFirst().AsEntity();

        if (data != nullptr && curScene != nullptr && node != nullptr)
        {
            DAVA::uint32 count = data->GetMergedItemCount() + 1;
            curScene->BeginBatch("DeleteRenderBatch batch", count);

            auto createCommand = [&node, &curScene](QtPropertyDataIntrospection* dynamicData) {
                DAVA::RenderBatch* batch = reinterpret_cast<DAVA::RenderBatch*>(dynamicData->object);
                DAVA::RenderObject* ro = batch->GetRenderObject();
                DVASSERT(ro);

                DAVA::uint32 count = ro->GetRenderBatchCount();
                if (count == 1)
                {
                    // We don't allow to delete last render batch.
                    return;
                }

                for (DAVA::uint32 i = 0; i < count; ++i)
                {
                    DAVA::RenderBatch* b = ro->GetRenderBatch(i);
                    if (b == batch)
                    {
                        curScene->Exec(std::unique_ptr<DAVA::Command>(new DeleteRenderBatchCommand(node, batch->GetRenderObject(), i)));
                        break;
                    }
                }
            };

            createCommand(data);
            data->ForeachMergedItem([&createCommand, this](QtPropertyData* item) {
                QtPropertyDataIntrospection* dynamicData = dynamic_cast<QtPropertyDataIntrospection*>(item);
                if (dynamicData != nullptr)
                {
                    createCommand(dynamicData);
                }

                return true;
            });

            curScene->EndBatch();
        }
    }
}

void PropertyEditor::ActionEditMaterial()
{
    QtPropertyToolButton* btn = dynamic_cast<QtPropertyToolButton*>(QObject::sender());

    if (NULL != btn)
    {
        QtPropertyDataIntrospection* data = dynamic_cast<QtPropertyDataIntrospection*>(btn->GetPropertyData());
        if (NULL != data)
        {
            DVASSERT(globalOperations != nullptr);
            globalOperations->CallAction(GlobalOperations::ShowMaterial, DAVA::Any(static_cast<DAVA::NMaterial*>(data->object)));
        }
    }
}

void PropertyEditor::ActionEditSoundComponent()
{
    if (curNodes.GetSize() != 1)
        return;

    SceneEditor2* scene = sceneHolder.GetScene();
    if (scene == nullptr)
        return;

    DAVA::Entity* node = curNodes.GetFirst().AsEntity();
    DVASSERT(node != nullptr);

    scene->BeginBatch("Edit Sound Component", 1);
    {
        SoundComponentEditor editor(scene, globalOperations->GetGlobalParentWidget());
        editor.SetEditableEntity(node);
        editor.exec();
    }
    scene->EndBatch();

    ResetProperties();
}

bool PropertyEditor::IsParentFavorite(const QtPropertyData* data) const
{
    bool ret = false;

    QtPropertyData* parent = data->Parent();
    while (NULL != parent)
    {
        if (IsFavorite(parent))
        {
            ret = true;
            break;
        }
        else
        {
            parent = parent->Parent();
        }
    }

    return ret;
}

bool PropertyEditor::IsFavorite(QtPropertyData* data) const
{
    bool ret = false;

    if (NULL != data)
    {
        PropEditorUserData* userData = GetUserData(data);
        ret = userData->isFavorite;
    }

    return ret;
}

void PropertyEditor::SetFavorite(QtPropertyData* data, bool favorite)
{
    if (NULL == favoriteGroup)
    {
        favoriteGroup = GetProperty(InsertHeader("Favorites", 0));
    }

    if (NULL != data)
    {
        PropEditorUserData* userData = GetUserData(data);

        switch (userData->type)
        {
        case PropEditorUserData::ORIGINAL:
            if (userData->isFavorite != favorite)
            {
                // it is in favorite now, so we are going to remove it from favorites
                if (!favorite)
                {
                    DVASSERT(NULL != userData->associatedData);

                    QtPropertyData* favorite = userData->associatedData;
                    favoriteGroup->ChildRemove(favorite);
                    userData->associatedData = NULL;
                    userData->isFavorite = false;

                    scheme.remove(data->GetPath());
                    AddFavoriteChilds(data);
                }
                // new item should be added to favorites list
                else
                {
                    DVASSERT(NULL == userData->associatedData);

                    // check if it hasn't parent, that is already favorite
                    bool canBeAdded = true;
                    QtPropertyData* parent = data;
                    while (NULL != parent)
                    {
                        if (GetUserData(parent)->isFavorite)
                        {
                            canBeAdded = false;
                            break;
                        }

                        parent = parent->Parent();
                    }

                    if (canBeAdded)
                    {
                        std::unique_ptr<QtPropertyData> favorite(CreateClone(data));
                        data->ForeachMergedItem([&favorite, this](QtPropertyData* item) {
                            std::unique_ptr<QtPropertyData> mergedClone(CreateClone(item));
                            favorite->Merge(std::move(mergedClone));
                            return true;
                        });

                        userData->associatedData = favorite.get();
                        userData->isFavorite = true;

                        ApplyCustomExtensions(favorite.get());

                        // create user data for added favorite, that will have COPY type,
                        // and associatedData will point to the original property
                        PropEditorUserData* favUserData = new PropEditorUserData(PropEditorUserData::COPY, data, true);
                        favUserData->realPath = data->GetPath();

                        favorite->SetUserData(favUserData);
                        favoriteGroup->MergeChild(std::move(favorite));

                        scheme.insert(data->GetPath());
                        RemFavoriteChilds(data);
                    }
                }

                data->EmitDataChanged(QtPropertyData::VALUE_SET);
            }
            break;

        case PropEditorUserData::COPY:
            if (userData->isFavorite != favorite)
            {
                // copy of the original data can only be removed
                DVASSERT(!favorite);

                // copy of the original data should always have a pointer to the original property data
                QtPropertyData* original = userData->associatedData;
                if (NULL != original)
                {
                    PropEditorUserData* originalUserData = GetUserData(original);
                    originalUserData->associatedData = NULL;
                    originalUserData->isFavorite = false;

                    AddFavoriteChilds(original);
                }

                scheme.remove(userData->realPath);
                favoriteGroup->ChildRemove(data);
            }
            break;

        default:
            DVASSERT(false && "Unknown userData type");
            break;
        }
    }

    // if there is no favorite items - remove favorite group
    if (favoriteGroup->ChildCount() == 0)
    {
        RemoveProperty(favoriteGroup);
        favoriteGroup = NULL;
    }
}

void PropertyEditor::AddFavoriteChilds(QtPropertyData* data)
{
    if (NULL != data)
    {
        // go through childs
        for (int i = 0; i < data->ChildCount(); ++i)
        {
            QtPropertyData* child = data->ChildGet(i);
            if (scheme.contains(child->GetPath()))
            {
                SetFavorite(child, true);
            }
            else
            {
                AddFavoriteChilds(child);
            }
        }
    }
}

void PropertyEditor::RemFavoriteChilds(QtPropertyData* data)
{
    if (NULL != data)
    {
        if (GetUserData(data)->type == PropEditorUserData::ORIGINAL)
        {
            // go through childs
            for (int i = 0; i < data->ChildCount(); ++i)
            {
                QtPropertyData* child = data->ChildGet(i);
                PropEditorUserData* userData = GetUserData(child);
                if (NULL != userData->associatedData)
                {
                    favoriteGroup->ChildRemove(userData->associatedData);

                    userData->associatedData = NULL;
                    userData->isFavorite = false;
                }
                else
                {
                    RemFavoriteChilds(child);
                }
            }
        }
    }
}

PropEditorUserData* PropertyEditor::GetUserData(QtPropertyData* data) const
{
    PropEditorUserData* userData = static_cast<PropEditorUserData*>(data->GetUserData());
    if (NULL == userData)
    {
        userData = new PropEditorUserData(PropEditorUserData::ORIGINAL);
        data->SetUserData(userData);
    }

    return userData;
}

void PropertyEditor::LoadScheme(const DAVA::FilePath& path)
{
    // first, we open the file
    QFile file(path.GetAbsolutePathname().c_str());
    if (file.open(QIODevice::ReadOnly))
    {
        scheme.clear();

        QTextStream qin(&file);
        while (!qin.atEnd())
        {
            scheme.insert(qin.readLine());
        }

        file.close();
    }
}

void PropertyEditor::SaveScheme(const DAVA::FilePath& path)
{
    // first, we open the file
    QFile file(path.GetAbsolutePathname().c_str());
    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream qout(&file);
        foreach (const QString& value, scheme)
        {
            qout << value << endl;
        }
        file.close();
    }
}

bool PropertyEditor::IsInspViewAllowed(const DAVA::InspInfo* info) const
{
    bool ret = true;

    if (info->Type() == DAVA::MetaInfo::Instance<DAVA::NMaterial>())
    {
        // Don't show properties for NMaterial.
        // They should be edited in materialEditor.
        ret = false;
    }

    return ret;
}

QtPropertyToolButton* PropertyEditor::CreateButton(QtPropertyData* data, const QIcon& icon, const QString& tooltip)
{
    DVASSERT(data);

    QtPropertyToolButton* button = data->AddButton();
    button->setIcon(icon);
    button->setToolTip(tooltip);
    button->setIconSize(QSize(12, 12));
    button->setAutoRaise(true);

    return button;
}

void PropertyEditor::CloneRenderBatchesToFixSwitchLODs()
{
    QtPropertyToolButton* btn = dynamic_cast<QtPropertyToolButton*>(QObject::sender());
    if (nullptr != btn)
    {
        QtPropertyDataIntrospection* data = dynamic_cast<QtPropertyDataIntrospection*>(btn->GetPropertyData());
        if (nullptr != data)
        {
            DAVA::RenderObject* renderObject = (DAVA::RenderObject*)data->object;

            SceneEditor2* curScene = sceneHolder.GetScene();
            if (curScene && renderObject)
            {
                curScene->Exec(std::unique_ptr<DAVA::Command>(new CloneLastBatchCommand(renderObject)));
            }
        }
    }
}

void PropertyEditor::OnAddComponent(DAVA::Component::eType type)
{
    SceneEditor2* curScene = sceneHolder.GetScene();
    if (curScene == nullptr || curNodes.IsEmpty())
        return;

    curScene->BeginBatch(Format("Add Component: %d", type), curNodes.GetSize());
    for (auto entity : curNodes.ObjectsOfType<DAVA::Entity>())
    {
        auto c = DAVA::Component::CreateByType(type);
        curScene->Exec(std::unique_ptr<DAVA::Command>(new AddComponentCommand(entity, c)));
    }
    curScene->EndBatch();
}

void PropertyEditor::OnAddComponent(DAVA::Component* component)
{
    DVASSERT(component);
    SceneEditor2* curScene = sceneHolder.GetScene();
    if (curScene == nullptr || curNodes.IsEmpty())
        return;

    curScene->BeginBatch(DAVA::Format("Add Component: %d", component->GetType()), curNodes.GetSize());

    for (auto entity : curNodes.ObjectsOfType<DAVA::Entity>())
    {
        if (entity->GetComponentCount(component->GetType()) == 0)
        {
            DAVA::Component* c = component->Clone(entity);
            curScene->Exec(std::unique_ptr<DAVA::Command>(new AddComponentCommand(entity, c)));
        }
    }

    curScene->EndBatch();
}

void PropertyEditor::OnAddActionComponent()
{
    OnAddComponent(DAVA::Component::ACTION_COMPONENT);
}

void PropertyEditor::OnAddLodComponent()
{
    OnAddComponent(DAVA::Component::LOD_COMPONENT);
}

void PropertyEditor::OnAddStaticOcclusionComponent()
{
    OnAddComponent(DAVA::Component::STATIC_OCCLUSION_COMPONENT);
}

void PropertyEditor::OnAddSoundComponent()
{
    OnAddComponent(DAVA::Component::SOUND_COMPONENT);
}

void PropertyEditor::OnAddWaveComponent()
{
    OnAddComponent(DAVA::Component::WAVE_COMPONENT);
}

void PropertyEditor::OnAddModelTypeComponent()
{
    OnAddComponent(DAVA::Component::QUALITY_SETTINGS_COMPONENT);
}

void PropertyEditor::OnAddSkeletonComponent()
{
    OnAddComponent(DAVA::Component::SKELETON_COMPONENT);
}

void PropertyEditor::OnAddPathComponent()
{
    SceneEditor2* curScene = sceneHolder.GetScene();
    if (curScene == nullptr || curNodes.IsEmpty())
        return;

    curScene->BeginBatch(DAVA::Format("Add Component: %d", DAVA::Component::PATH_COMPONENT), curNodes.GetSize());

    for (auto entity : curNodes.ObjectsOfType<DAVA::Entity>())
    {
        if ((entity->GetComponentCount(DAVA::Component::PATH_COMPONENT) == 0) && (entity->GetComponentCount(DAVA::Component::WAYPOINT_COMPONENT) == 0))
        {
            DAVA::PathComponent* pathComponent = curScene->pathSystem->CreatePathComponent();
            curScene->Exec(std::unique_ptr<DAVA::Command>(new AddComponentCommand(entity, pathComponent)));
        }
    }

    curScene->EndBatch();
}

void PropertyEditor::OnAddRotationControllerComponent()
{
    OnAddComponent(DAVA::Component::ROTATION_CONTROLLER_COMPONENT);
}

void PropertyEditor::OnAddSnapToLandscapeControllerComponent()
{
    DAVA::SnapToLandscapeControllerComponent* snapComponent = static_cast<DAVA::SnapToLandscapeControllerComponent*>(DAVA::Component::CreateByType(DAVA::Component::SNAP_TO_LANDSCAPE_CONTROLLER_COMPONENT));

    DAVA::float32 height = SettingsManager::Instance()->GetValue(Settings::Scene_CameraHeightOnLandscape).AsFloat();
    snapComponent->SetHeightOnLandscape(height);

    OnAddComponent(snapComponent);

    SafeDelete(snapComponent);
}

void PropertyEditor::OnAddWASDControllerComponent()
{
    OnAddComponent(DAVA::Component::WASD_CONTROLLER_COMPONENT);
}

void PropertyEditor::OnAddVisibilityComponent()
{
    OnAddComponent(DAVA::Component::VISIBILITY_CHECK_COMPONENT);
}

void PropertyEditor::OnRemoveComponent()
{
    QtPropertyToolButton* btn = dynamic_cast<QtPropertyToolButton*>(QObject::sender());
    if (nullptr != btn)
    {
        QtPropertyDataIntrospection* data = dynamic_cast<QtPropertyDataIntrospection*>(btn->GetPropertyData());
        SceneEditor2* curScene = sceneHolder.GetScene();

        if (nullptr != data && nullptr != curScene)
        {
            curScene->BeginBatch("Remove Component", data->GetMergedItemCount() + 1);

            DAVA::Component* component = static_cast<DAVA::Component*>(data->object);
            PropEditorUserData* userData = GetUserData(data);
            curScene->Exec(std::unique_ptr<DAVA::Command>(new RemoveComponentCommand(userData->entity, component)));

            data->ForeachMergedItem([&curScene, this](QtPropertyData* item) {
                QtPropertyDataIntrospection* dynamicData = dynamic_cast<QtPropertyDataIntrospection*>(item);
                if (dynamicData != nullptr)
                {
                    DAVA::Component* component = static_cast<DAVA::Component*>(dynamicData->object);
                    PropEditorUserData* userData = GetUserData(dynamicData);
                    curScene->Exec(std::unique_ptr<DAVA::Command>(new RemoveComponentCommand(userData->entity, component)));
                }

                return true;
            });

            curScene->EndBatch();
        }
    }
}

void PropertyEditor::OnTriggerWaveComponent()
{
    for (auto entity : curNodes.ObjectsOfType<DAVA::Entity>())
    {
        DAVA::WaveComponent* component = GetWaveComponent(entity);
        if (component != nullptr)
        {
            component->Trigger();
        }
    }
}

void PropertyEditor::OnConvertRenderObjectToBillboard()
{
    QtPropertyToolButton* btn = dynamic_cast<QtPropertyToolButton*>(QObject::sender());
    DVASSERT(btn != nullptr);

    QtPropertyDataIntrospection* data = dynamic_cast<QtPropertyDataIntrospection*>(btn->GetPropertyData());
    if (data != nullptr)
    {
        bool anyConvertToBillboardCommandIssued = false;

        SceneEditor2* curScene = sceneHolder.GetScene();
        curScene->BeginBatch("Convert to billboard", curNodes.GetSize());
        for (Selectable& obj : curNodes.GetMutableContent())
        {
            DAVA::Entity* entity = obj.AsEntity();
            if (entity != nullptr)
            {
                DAVA::RenderComponent* component = DAVA::GetRenderComponent(entity);
                if (component != nullptr)
                {
                    DAVA::RenderObject* ro = component->GetRenderObject();
                    if (ro != nullptr)
                    {
                        anyConvertToBillboardCommandIssued = true;
                        curScene->Exec(std::unique_ptr<DAVA::Command>(new ConvertToBillboardCommand(ro, entity)));
                    }
                }
            }
        }
        curScene->EndBatch();
        DVASSERT(anyConvertToBillboardCommandIssued); // sanity check
    }
}

QString PropertyEditor::GetDefaultFilePath(bool withScenePath /*= true*/)
{
    ProjectManagerData* data = REGlobal::GetDataNode<ProjectManagerData>();
    DVASSERT(data != nullptr);
    QString defaultPath = data->GetProjectPath().GetAbsolutePathname().c_str();
    DAVA::FilePath dataSourcePath = data->GetDataSource3DPath();

    if (DAVA::FileSystem::Instance()->Exists(dataSourcePath))
    {
        defaultPath = dataSourcePath.GetAbsolutePathname().c_str();
    }

    SceneEditor2* editor = sceneHolder.GetScene();
    if (nullptr != editor && DAVA::FileSystem::Instance()->Exists(editor->GetScenePath()) && withScenePath)
    {
        DAVA::String scenePath = editor->GetScenePath().GetDirectory().GetAbsolutePathname();
        if (DAVA::String::npos != scenePath.find(dataSourcePath.GetAbsolutePathname()))
        {
            defaultPath = scenePath.c_str();
        }
    }

    return defaultPath;
}
