#include "EditorMaterialSystem.h"
#include "Settings/SettingsManager.h"
#include "Project/ProjectManager.h"
#include "Scene3D/Scene.h"
#include "Commands2/Base/Command2.h"
#include "Commands2/Base/CommandBatch.h"
#include "Commands2/DeleteRenderBatchCommand.h"
#include "Commands2/ConvertToShadowCommand.h"
#include "Commands2/DeleteLODCommand.h"
#include "Commands2/CreatePlaneLODCommand.h"
#include "Commands2/CloneLastBatchCommand.h"
#include "Commands2/CopyLastLODCommand.h"
#include "Commands2/InspMemberModifyCommand.h"
#include "Scene3D/Systems/LandscapeSystem.h"

EditorMaterialSystem::MaterialMapping::MaterialMapping(DAVA::Entity* entity_, DAVA::RenderBatch* renderBatch_)
    : entity(entity_)
{
    renderBatch = DAVA::SafeRetain(renderBatch_);
}

EditorMaterialSystem::MaterialMapping::MaterialMapping(const MaterialMapping& other)
{
    *this = other;
}

EditorMaterialSystem::MaterialMapping::~MaterialMapping()
{
    DAVA::SafeRelease(renderBatch);
}

EditorMaterialSystem::MaterialMapping& EditorMaterialSystem::MaterialMapping::operator=(const MaterialMapping& other)
{
    if (this == &other)
        return *this;

    entity = other.entity;
    DAVA::SafeRelease(renderBatch);
    renderBatch = DAVA::SafeRetain(other.renderBatch);
    return *this;
}

EditorMaterialSystem::EditorMaterialSystem(DAVA::Scene* scene)
    : DAVA::SceneSystem(scene)
{
}

EditorMaterialSystem::~EditorMaterialSystem()
{
    while (!materialToObjectsMap.empty())
    {
        RemoveMaterial(materialToObjectsMap.begin()->first);
    }

    for (auto op : ownedParents)
    {
        DAVA::SafeRelease(op);
    }
}

DAVA::Entity* EditorMaterialSystem::GetEntity(DAVA::NMaterial* material) const
{
    auto it = materialToObjectsMap.find(material);
    return (it == materialToObjectsMap.end()) ? nullptr : it->second.entity;
}

const DAVA::RenderBatch* EditorMaterialSystem::GetRenderBatch(DAVA::NMaterial* material) const
{
    auto it = materialToObjectsMap.find(material);
    if (it == materialToObjectsMap.end())
        return nullptr;

    return it->second.renderBatch;
}

const DAVA::Set<DAVA::NMaterial*>& EditorMaterialSystem::GetTopParents() const
{
    return ownedParents;
}

int EditorMaterialSystem::GetLightViewMode()
{
    return curViewMode;
}

bool EditorMaterialSystem::GetLightViewMode(EditorMaterialSystem::MaterialLightViewMode viewMode) const
{
    return (bool)(curViewMode & viewMode);
}

void EditorMaterialSystem::SetLightViewMode(int fullViewMode)
{
    if (curViewMode != fullViewMode)
    {
        curViewMode = fullViewMode;
        ApplyViewMode();
    }
}

void EditorMaterialSystem::SetLightViewMode(EditorMaterialSystem::MaterialLightViewMode viewMode, bool set)
{
    int newMode = curViewMode;

    if (set)
    {
        newMode |= viewMode;
    }
    else
    {
        newMode &= ~viewMode;
    }

    SetLightViewMode(newMode);
}

void EditorMaterialSystem::SetLightmapCanvasVisible(bool enable)
{
    if (enable != showLightmapCanvas)
    {
        showLightmapCanvas = enable;
        ApplyViewMode();
    }
}

bool EditorMaterialSystem::IsLightmapCanvasVisible() const
{
    return showLightmapCanvas;
}

void EditorMaterialSystem::AddEntity(DAVA::Entity* entity)
{
    DAVA::RenderObject* ro = GetRenderObject(entity);
    if (nullptr != ro)
    {
        AddMaterials(entity);
    }
}

void EditorMaterialSystem::AddMaterials(DAVA::Entity* entity)
{
    DAVA::RenderObject* ro = GetRenderObject(entity);
    DVASSERT(nullptr != ro);

    for (DAVA::uint32 i = 0; i < ro->GetRenderBatchCount(); ++i)
    {
        DAVA::RenderBatch* renderBatch = ro->GetRenderBatch(i);
        AddMaterial(renderBatch->GetMaterial(), MaterialMapping(entity, renderBatch));
    }
}

void EditorMaterialSystem::RemoveEntity(DAVA::Entity* entity)
{
    DAVA::RenderObject* ro = GetRenderObject(entity);
    if (nullptr != ro)
    {
        for (DAVA::uint32 i = 0; i < ro->GetRenderBatchCount(); ++i)
        {
            DAVA::RenderBatch* rb = ro->GetRenderBatch(i);
            DAVA::NMaterial* material = rb->GetMaterial();
            RemoveMaterial(material);
        }
    }
}

void EditorMaterialSystem::ApplyViewMode()
{
    DAVA::Set<DAVA::NMaterial*>::const_iterator i = ownedParents.begin();
    DAVA::Set<DAVA::NMaterial*>::const_iterator end = ownedParents.end();

    for (; i != end; ++i)
    {
        ApplyViewMode(*i);
    }
}

void EditorMaterialSystem::ApplyViewMode(DAVA::NMaterial* material)
{
    auto SetMaterialFlag = [](DAVA::NMaterial* material, const DAVA::FastName& flagName, DAVA::int32 value) {
        if (value == 0)
        {
            if (material->HasLocalFlag(flagName))
            {
                material->RemoveFlag(flagName);
            }
        }
        else
        {
            if (material->HasLocalFlag(flagName))
            {
                material->SetFlag(flagName, value);
            }
            else
            {
                material->AddFlag(flagName, value);
            }
        }
    };

    if (curViewMode & LIGHTVIEW_ALBEDO)
    {
        SetMaterialFlag(material, DAVA::NMaterialFlagName::FLAG_LIGHTMAPONLY, 0);
    }
    else
    {
        SetMaterialFlag(material, DAVA::NMaterialFlagName::FLAG_LIGHTMAPONLY, 1);
    }

    if (showLightmapCanvas)
    {
        SetMaterialFlag(material, DAVA::NMaterialFlagName::FLAG_SETUPLIGHTMAP, 1);
    }
    else
    {
        SetMaterialFlag(material, DAVA::NMaterialFlagName::FLAG_SETUPLIGHTMAP, 0);
    }

    //materials
    auto UpdateFlags = [this, SetMaterialFlag](DAVA::NMaterial* material, DAVA::int32 mode, const DAVA::FastName& flagName) {
        if (curViewMode & mode)
        {
            SetMaterialFlag(material, flagName, 1);
        }
        else
        {
            SetMaterialFlag(material, flagName, 0);
        }
    };

    UpdateFlags(material, LIGHTVIEW_ALBEDO, DAVA::NMaterialFlagName::FLAG_VIEWALBEDO);
    UpdateFlags(material, LIGHTVIEW_DIFFUSE, DAVA::NMaterialFlagName::FLAG_VIEWDIFFUSE);
    UpdateFlags(material, LIGHTVIEW_SPECULAR, DAVA::NMaterialFlagName::FLAG_VIEWSPECULAR);
    UpdateFlags(material, LIGHTVIEW_AMBIENT, DAVA::NMaterialFlagName::FLAG_VIEWAMBIENT);
}

void EditorMaterialSystem::ProcessCommand(const Command2* command, bool redo)
{
    const DAVA::int32 commandID = command->GetId();
    if (commandID == CMDID_BATCH)
    {
        const CommandBatch* batch = static_cast<const CommandBatch*>(command);
        if (batch->MatchCommandIDs({ CMDID_LOD_DELETE, CMDID_LOD_CREATE_PLANE, CMDID_DELETE_RENDER_BATCH, CMDID_CONVERT_TO_SHADOW, CMDID_LOD_COPY_LAST_LOD, CMDID_INSP_MEMBER_MODIFY }))
        {
            const DAVA::uint32 count = batch->Size();
            for (DAVA::uint32 i = 0; i < count; ++i)
            {
                ProcessCommand(batch->GetCommand(i), redo);
            }
        }
    }
    else
    {
        switch (commandID)
        {
        case CMDID_LOD_DELETE:
        {
            const DeleteLODCommand* lodCommand = static_cast<const DeleteLODCommand*>(command);
            const DAVA::Vector<DeleteRenderBatchCommand*> batchCommands = lodCommand->GetRenderBatchCommands();

            const DAVA::uint32 count = (const DAVA::uint32)batchCommands.size();
            for (DAVA::uint32 i = 0; i < count; ++i)
            {
                DAVA::RenderBatch* batch = batchCommands[i]->GetRenderBatch();
                if (redo)
                {
                    RemoveMaterial(batch->GetMaterial());
                }
                else
                {
                    AddMaterial(batch->GetMaterial(), MaterialMapping(lodCommand->GetEntity(), batch));
                }
            }

            break;
        }
        case CMDID_LOD_CREATE_PLANE:
        {
            const CreatePlaneLODCommand* lodCommand = static_cast<const CreatePlaneLODCommand*>(command);
            DAVA::RenderBatch* batch = lodCommand->GetRenderBatch();
            if (redo)
            {
                AddMaterial(batch->GetMaterial(), MaterialMapping(lodCommand->GetEntity(), batch));
            }
            else
            {
                RemoveMaterial(batch->GetMaterial());
            }
            break;
        }
        case CMDID_DELETE_RENDER_BATCH:
        {
            const DeleteRenderBatchCommand* rbCommand = static_cast<const DeleteRenderBatchCommand*>(command);
            DAVA::RenderBatch* batch = rbCommand->GetRenderBatch();
            if (redo)
            {
                RemoveMaterial(batch->GetMaterial());
            }
            else
            {
                AddMaterial(batch->GetMaterial(), MaterialMapping(rbCommand->GetEntity(), batch));
            }
            break;
        }
        case CMDID_CONVERT_TO_SHADOW:
        {
            const ConvertToShadowCommand* swCommand = static_cast<const ConvertToShadowCommand*>(command);
            DAVA::RenderBatch* oldBatch = swCommand->oldBatch;
            DAVA::RenderBatch* newBatch = swCommand->newBatch;

            if (!redo)
            {
                std::swap(oldBatch, newBatch);
            }

            RemoveMaterial(oldBatch->GetMaterial());
            AddMaterial(newBatch->GetMaterial(), MaterialMapping(swCommand->GetEntity(), newBatch));
            break;
        }
        case CMDID_LOD_COPY_LAST_LOD:
        {
            const CopyLastLODToLod0Command* copyCommand = static_cast<const CopyLastLODToLod0Command*>(command);
            DAVA::uint32 batchCount = copyCommand->newBatches.size();
            for (DAVA::uint32 i = 0; i < batchCount; ++i)
            {
                DAVA::RenderBatch* batch = copyCommand->newBatches[i];
                DAVA::NMaterial* material = batch->GetMaterial();
                if (redo)
                {
                    AddMaterial(material, MaterialMapping(copyCommand->GetEntity(), batch));
                }
                else
                {
                    RemoveMaterial(material);
                }
            }
            break;
        }
        case CMDID_INSP_MEMBER_MODIFY:
        {
            const InspMemberModifyCommand* cmd = static_cast<const InspMemberModifyCommand*>(command);

            const DAVA::Vector<DAVA::Entity*>& landscapes = GetScene()->landscapeSystem->GetLandscapeEntities();
            for (DAVA::Entity* landEntity : landscapes)
            {
                DAVA::Landscape* landObject = GetLandscape(landEntity);
                if (landObject == cmd->object)
                {
                    RemoveMaterial(landObject->GetMaterial());
                    AddMaterials(landEntity);
                }
            }
        }

        default:
            break;
        }
    }
}

bool EditorMaterialSystem::HasMaterial(DAVA::NMaterial* material) const
{
    return (materialToObjectsMap.count(material) > 0) || (ownedParents.count(material) > 0);
}

void EditorMaterialSystem::AddMaterial(DAVA::NMaterial* material, const MaterialMapping& mapping)
{
    if (nullptr != material)
    {
        DAVA::NMaterial* curGlobalMaterial = nullptr;
        if ((nullptr != mapping.entity) && (nullptr != mapping.entity->GetScene()))
        {
            curGlobalMaterial = mapping.entity->GetScene()->GetGlobalMaterial();
        }

        materialToObjectsMap[material] = mapping;

        // remember parent material, if still isn't
        DAVA::NMaterial* parent = material;
        for (;;)
        {
            DAVA::NMaterial* nextParent = parent->GetParent();
            if (nullptr == nextParent || curGlobalMaterial == nextParent)
            {
                break;
            }
            else
            {
                parent = nextParent;
            }
        }

        if (0 == ownedParents.count(parent))
        {
            if (IsEditable(parent))
            {
                ownedParents.insert(parent);
                DAVA::SafeRetain(parent);
                ApplyViewMode(parent);
            }
        }
    }
}

void EditorMaterialSystem::RemoveMaterial(DAVA::NMaterial* material)
{
    materialToObjectsMap.erase(material);
}

bool EditorMaterialSystem::IsEditable(DAVA::NMaterial* material) const
{
    return (!material->IsRuntime());
}
